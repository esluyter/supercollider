/************************************************************************
*
* Copyright 2010-2012 Jakob Leben (jakob.leben@gmail.com)
*
* This file is part of SuperCollider Qt GUI.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
************************************************************************/

#include "QcCodeEdit.h"
#include "../QcWidgetFactory.h"
#include "../hacks/hacks_qt.hpp"

#include <QFile>
#include <QUrl>
#include <QKeyEvent>
#include <QApplication>
#include <QTextBlock>


class QcCodeEditFactory : public QcWidgetFactory<QcCodeEdit>
{
  void initialize( QWidgetProxy *p, QcCodeEdit *w ) {
    p->setMouseEventWidget( w->viewport() );
  }
};

QC_DECLARE_FACTORY( QcCodeEdit, QcCodeEditFactory );



Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
  HighlightingRule rule;

  keywordFormat.setForeground(QColor(168, 28, 166));
  keywordFormat.setFontWeight(QFont::Bold);
  QStringList keywordPatterns;
  keywordPatterns << "\\bvar\\b" << "\\barg\\b" << "\\bthis\\b"
                  << "\\btrue\\b" << "\\bfalse\\b" << "\\bcurrentEnvironment\\b"
                  << "\\btopEnvironment\\b" << "\\bthisProcess\\b" << "\\bthisThread\\b"
                  << "\\bthisFunction\\b" << "\\bthisMethod\\b" << "\\bthisCuelist\\b";

  for (int i = 0; i < keywordPatterns.size(); i++) {
    QString pattern = keywordPatterns.at(i);
    rule.pattern = QRegExp(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
  }

  quotationFormat.setForeground(QColor(77, 162, 75));
  rule.pattern = QRegExp("\".*\"");
  rule.format = quotationFormat;
  highlightingRules.append(rule);

  symbolFormat.setForeground(QColor(0, 131, 190));
  rule.pattern = QRegExp("\'.*\'|(\\s|^)\\\\\\w+\\b");
  rule.format = symbolFormat;
  highlightingRules.append(rule);

  numberFormat.setForeground(QColor(156, 109, 0));
  rule.pattern = QRegExp("(\\b|((\\s|^)\\-))((\\d+(\\.\\d+)?)|pi|inf)\\b");
  rule.format = numberFormat;
  highlightingRules.append(rule);

  envvarFormat.setForeground(QColor(230, 85, 68));
  rule.pattern = QRegExp("\\~\\w+");
  rule.format = envvarFormat;
  highlightingRules.append(rule);

  keyFormat.setForeground(QColor(0, 128, 128));
  rule.pattern = QRegExp("(\\w+):");
  rule.format = keyFormat;
  highlightingRules.append(rule);



  //functionFormat.setFontItalic(true);
  functionFormat.setForeground(QColor(60, 116, 246));
  rule.pattern = QRegExp("(\\.[a-z]\\w*)|(\\b[a-z]\\w*(?=(\\s*[\\(\\{])))" /*"(?=\\()"*/);
  rule.format = functionFormat;
  highlightingRules.append(rule);


  singleLineCommentFormat.setForeground(QColor(160, 161, 167));
  singleLineCommentFormat.setFontItalic(true);
  rule.pattern = QRegExp("//[^\n]*");
  rule.format = singleLineCommentFormat;
  highlightingRules.append(rule);

  classFormat.setFontWeight(QFont::Bold);
  classFormat.setForeground(QColor(194, 133, 0));
  rule.pattern = QRegExp("\\b[A-Z]\\w+\\b");
  rule.format = classFormat;
  highlightingRules.append(rule);

  multiLineCommentFormat.setForeground(QColor(160, 161, 167));
  multiLineCommentFormat.setFontItalic(true);

  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    for (int i = 0; i < highlightingRules.size(); i++) {
      HighlightingRule rule = highlightingRules.at(i);
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    setCurrentBlockState(0);

  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = commentStartExpression.indexIn(text);

  while (startIndex >= 0) {
    int endIndex = commentEndExpression.indexIn(text, startIndex);
    int commentLength;
    if (endIndex == -1) {
       setCurrentBlockState(1);
       commentLength = text.length() - startIndex;
    } else {
       commentLength = endIndex - startIndex
                       + commentEndExpression.matchedLength();
    }
    setFormat(startIndex, commentLength, multiLineCommentFormat);
    startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
  }
}






class LineNumberArea : public QWidget
{
public:
    LineNumberArea(QcCodeEdit *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    QcCodeEdit *codeEditor;
};


QcCodeEdit::QcCodeEdit() : _interpretSelection(true)
{
  setAttribute(Qt::WA_AcceptTouchEvents);

  connect( this, SIGNAL(interpret(QString)),
           qApp, SLOT(interpret(QString)),
           Qt::QueuedConnection );

  // line numbers
  lineNumberArea = new LineNumberArea(this);

  highlightColor = QColor(QColor(200, 200, 200));

  highlighter = new Highlighter(document());

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}


int QcCodeEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void QcCodeEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void QcCodeEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void QcCodeEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void QcCodeEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = highlightColor;

        lineColor.setAlpha(50);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void QcCodeEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), highlightColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    QFont font = painter.font() ;
    /* twice the size than the current font size */
    font.setPointSize(font.pointSize() * 7 / 8);
    /* set the modified font to the painter */
    painter.setFont(font);

    while (block.isValid() && top <= event->rect().bottom()) {
      if (block.isVisible() && bottom >= event->rect().top()) {

          QString number = QString::number(blockNumber + 1);
          painter.setPen(Qt::black);
          painter.drawText(0, top + (font.pointSize() / 7 * 3), lineNumberArea->width() - 1, fontMetrics().height(),
                           Qt::AlignRight, number);
      }

      block = block.next();
      top = bottom;
      bottom = top + (int) blockBoundingRect(block).height();
      ++blockNumber;
  }
}

QString QcCodeEdit::documentFilename() const
{
  return _document;
}

void QcCodeEdit::setDocument( const QString &filename )
{
  QFile file( filename );
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QByteArray data = file.readAll();
  //TODO setText( data );
  _document = filename;
}

int QcCodeEdit::selectionStart() const
{
  return textCursor().selectionStart();
}

int QcCodeEdit::selectionSize() const
{
  QTextCursor cursor = textCursor();
  return cursor.selectionEnd() - cursor.selectionStart();
}

void QcCodeEdit::select( int start, int size )
{
  if( start < 0 ) start = 0;

  QTextCursor cursor( document() );

  cursor.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, start );
  cursor.movePosition( size > 0 ? QTextCursor::Right : QTextCursor::Left,
                       QTextCursor::KeepAnchor, qAbs(size) );

  setTextCursor( cursor );
}

QString QcCodeEdit::selectedString() const
{
  QString selection( textCursor().selectedText() );
  return prepareText(selection);
}

void QcCodeEdit::replaceSelectedText( const QString &string )
{
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() ) {
    cursor.removeSelectedText();
  };
  cursor.insertText( string );
}

QString QcCodeEdit::currentLine() const
{
    return textCursor().block().text();
}

void QcCodeEdit::setTextFont( const QFont &f )
{
  QTextCharFormat format;
  format.setFont( f );

  QTextCursor cursor( document() );
  cursor.select( QTextCursor::Document );
  cursor.mergeCharFormat( format );

  QPlainTextEdit::setFont(f);
}

void QcCodeEdit::setTextColor( const QColor &color )
{
  QTextCharFormat format;
  format.setForeground( color );

  QTextCursor cursor( document() );
  cursor.select( QTextCursor::Document );
  cursor.mergeCharFormat( format );
}

void QcCodeEdit::setHighlightColor( const QColor &color )
{
  highlightColor = color;
  highlightCurrentLine();
}

void QcCodeEdit::setRangeColor( const QVariantList &list )
{
  if( list.count() < 3 ) return;
  QColor c = list[0].value<QColor>();
  int start = list[1].toInt();
  int size = list[2].toInt();

  QTextCharFormat format;
  format.setForeground( c );

  QTextCursor cursor( document() );
  cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::MoveAnchor, start );
  cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor, size );
  cursor.mergeCharFormat( format );
}

void QcCodeEdit::setRangeFont( const QVariantList & list )
{
  if( list.count() < 3 ) return;
  QFont f = list[0].value<QFont>();
  int start = list[1].toInt();
  int size = list[2].toInt();

  QTextCharFormat format;
  format.setFont( f );

  QTextCursor cursor( document() );
  cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::MoveAnchor, start );
  cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor, size );
  cursor.mergeCharFormat( format );
}

void QcCodeEdit::setRangeText( const QVariantList & list )
{
  if( list.count() < 3 ) return;
  QString text = list[0].value<QString>();
  int start = list[1].toInt();
  int size = list[2].toInt();

  QTextCursor cursor( document() );
  cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::MoveAnchor, start );
  cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor, size );
  cursor.insertText( text );
}

void QcCodeEdit::keyPressEvent( QKeyEvent *e )
{
  if( _interpretSelection && e->modifiers() & (Qt::ControlModifier|Qt::ShiftModifier)
      && ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter ) )
  {
    QTextCursor c(textCursor());
    QString code;

    if ( c.hasSelection() ) {
      code = c.selectedText();
    } {
      code = c.block().text();
    }

    Q_EMIT( interpret( prepareText(code) ) );

    return;
  }

  QPlainTextEdit::keyPressEvent( e );
}

void QcCodeEdit::insertFromMimeData ( const QMimeData * data )
{
  if(data->hasUrls()) {
    QTextCursor c( textCursor() );
    QList<QUrl> urls = data->urls();
    int n = urls.count();
    for(int i = 0; i < n; ++i)
    {
      QUrl &url = urls[i];
      QString text = QURL_IS_LOCAL_FILE(url) ? url.toLocalFile() : url.toString();
      c.insertText(text);
      if(n > 1) c.insertText("\n");
    }
  }
  else
    QPlainTextEdit::insertFromMimeData(data);
}

QString & QcCodeEdit::prepareText( QString & text ) const
{
  // NOTE: QTextDocument contains unicode paragraph separators U+2029
  // instead of newline \n characters
  return text.replace( QChar( 0x2029 ), QChar( '\n' ) );
}
