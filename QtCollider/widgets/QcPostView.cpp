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

#include "QcPostView.h"
#include "../QcWidgetFactory.h"
#include "../hacks/hacks_qt.hpp"

#include <QFile>
#include <QUrl>
#include <QKeyEvent>
#include <QApplication>
#include <QTextBlock>


class QcPostViewFactory : public QcWidgetFactory<QcPostView>
{
  void initialize( QWidgetProxy *p, QcPostView *w ) {
    p->setMouseEventWidget( w->viewport() );
  }
};

QC_DECLARE_FACTORY( QcPostView, QcPostViewFactory );



PostHighlighter::PostHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{

  keywordFormat.setForeground(QColor(168, 28, 166));
  keywordFormat.setFontWeight(QFont::Bold);

  builtinFormat.setForeground(QColor(168, 28, 166));

  numberFormat.setForeground(QColor(156, 109, 0));

  envvarFormat.setForeground(QColor(230, 85, 68));

  keyFormat.setForeground(QColor(0, 131, 190));

  //functionFormat.setFontItalic(true);
  functionFormat.setForeground(QColor(60, 116, 246));

  //classFormat.setFontWeight(QFont::Bold);
  classFormat.setForeground(QColor(194, 133, 0));

  punctuationFormat.setForeground(QColor(90, 108, 126));

  quotationFormat.setForeground(QColor(77, 162, 75));

  symbolFormat.setForeground(QColor(0, 131, 190));

  singleLineCommentFormat.setForeground(QColor(160, 161, 167));
  singleLineCommentFormat.setFontItalic(true);
  multiLineCommentFormat.setForeground(QColor(160, 161, 167));
  multiLineCommentFormat.setFontItalic(true);

  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");

  buildRules();
}

void PostHighlighter::buildRules()
{
  HighlightingRule rule;
  highlightingRules.clear();

  QStringList keywordPatterns;
  keywordPatterns << "\\bvar\\b" << "\\barg\\b" << "\\bthis\\b";
  for (int i = 0; i < keywordPatterns.size(); i++) {
    QString pattern = keywordPatterns.at(i);
    rule.pattern = QRegExp(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
  }

  QStringList builtinPatterns;
  builtinPatterns << "\\bnil\\b" << "\\btrue\\b" << "\\binf\\b" << "\\bfalse\\b" << "\\bcurrentEnvironment\\b"
                  << "\\btopEnvironment\\b" << "\\bthisProcess\\b" << "\\bthisThread\\b"
                  << "\\bthisFunction\\b" << "\\bthisMethod\\b" << "\\bthisCuelist\\b";
  for (int i = 0; i < builtinPatterns.size(); i++) {
    QString pattern = builtinPatterns.at(i);
    rule.pattern = QRegExp(pattern);
    rule.format = builtinFormat;
    highlightingRules.append(rule);
  }

  rule.pattern = QRegExp("\\~\\w+");
  rule.format = envvarFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("(\\w+):");
  rule.format = keyFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("(\\.[a-z]\\w*)|(\\b[a-z]\\w*(?=(\\s*[\\(\\{])))");
  rule.format = functionFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\\b[A-Z]\\w*\\b");
  rule.format = classFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("[<>\\&\\{\\}\\(\\)\\[\\]\\.\\,\\;:!\\=\\+\\-\\*\\/\\%\\|]");
  rule.format = punctuationFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("(\\b|((\\s|^)\\-))((\\d+(\\.\\d+)?)|pi)\\b");
  rule.format = numberFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\"(?:[^\"\\\\]|\\\\.)*\"");
  rule.format = quotationFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("\'.*\'|\\\\\\w+\\b");
  rule.format = symbolFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("//[^\n]*");
  rule.format = singleLineCommentFormat;
  highlightingRules.append(rule);

  for (int i = 0; i < customRules.size(); i++) {
    rule = customRules.at(i);
    highlightingRules.append(rule);
  }
}

void PostHighlighter::highlightBlock(const QString &text)
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

    for (int i = 0; i < customCommentRules.size(); i++) {
      HighlightingRule rule = customCommentRules.at(i);
      QRegExp expression(rule.pattern);
      int index = expression.indexIn(text, startIndex);
      while (index >= 0 && (endIndex == -1 || index < endIndex)) {
        int length = expression.matchedLength();
        setFormat(index, length, rule.format);
        index = expression.indexIn(text, index + length);
      }
    }

    startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
  }
}

void PostHighlighter::setBuiltinColor(const QColor &color)
{
  builtinFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setKeywordColor(const QColor &color)
{
  keywordFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setNumberColor(const QColor &color)
{
  numberFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setEnvvarColor(const QColor &color)
{
  envvarFormat.setForeground(color);
}

void PostHighlighter::setSymbolColor(const QColor &color)
{
  keyFormat.setForeground(color);
  symbolFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setMethodColor(const QColor &color)
{
  functionFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setClassColor(const QColor &color)
{
  classFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setPunctuationColor(const QColor &color)
{
  punctuationFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setStringColor(const QColor &color)
{
  quotationFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setCommentColor(const QColor &color)
{
  singleLineCommentFormat.setForeground(color);
  multiLineCommentFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setCustomColor(const QString &string, const QColor &color)
{
  HighlightingRule rule;
  QTextCharFormat format;
  format.setForeground(color);
  rule.pattern = QRegExp(string);
  rule.format = format;
  customRules.append(rule);
  buildRules();
  rehighlight();
}

void PostHighlighter::setCustomCommentColor(const QString &string, const QColor &color)
{
  HighlightingRule rule;
  QTextCharFormat format;
  format.setForeground(color);
  format.setFontItalic(true);
  rule.pattern = QRegExp(string);
  rule.format = format;
  customCommentRules.append(rule);
  buildRules();
  rehighlight();
}

void PostHighlighter::clearCustomColors()
{
  customRules.clear();
  customCommentRules.clear();
  buildRules();
  rehighlight();
}


QcPostView::QcPostView()
{
  setAttribute(Qt::WA_AcceptTouchEvents);
  setMaximumBlockCount(5);
  setReadOnly(true);

  highlighter = new PostHighlighter(document());

  setCursorWidth(2);
}

int QcPostView::selectionStart() const
{
  return textCursor().selectionStart();
}

int QcPostView::selectionSize() const
{
  QTextCursor cursor = textCursor();
  return cursor.selectionEnd() - cursor.selectionStart();
}

void QcPostView::select( int start, int size )
{
  if( start < 0 ) start = 0;

  QTextCursor cursor( document() );

  cursor.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, start );
  cursor.movePosition( size > 0 ? QTextCursor::Right : QTextCursor::Left,
                       QTextCursor::KeepAnchor, qAbs(size) );

  setTextCursor( cursor );
}

QString QcPostView::selectedString() const
{
  QString selection( textCursor().selectedText() );
  return prepareText(selection);
}

void QcPostView::setTextFont( const QFont &f )
{
  QTextCharFormat format;
  format.setFont( f );

  QTextCursor cursor( document() );
  cursor.select( QTextCursor::Document );
  cursor.mergeCharFormat( format );

  QPlainTextEdit::setFont(f);
}

void QcPostView::setTextColor( const QColor &color )
{
  QTextCharFormat format;
  format.setForeground( color );

  QTextCursor cursor( document() );
  cursor.select( QTextCursor::Document );
  cursor.mergeCharFormat( format );
}

void QcPostView::post( const QString &string )
{
  moveCursor( QTextCursor::End );
  insertPlainText( string );
  moveCursor( QTextCursor::End );
}


void QcPostView::setBuiltinColor( const QColor &color )
{
  highlighter->setBuiltinColor( color );
}

void QcPostView::setKeywordColor( const QColor &color )
{
  highlighter->setKeywordColor( color );
}

void QcPostView::setNumberColor( const QColor &color )
{
  highlighter->setNumberColor( color );
}

void QcPostView::setEnvvarColor( const QColor &color )
{
  highlighter->setEnvvarColor( color );
}

void QcPostView::setSymbolColor( const QColor &color )
{
  highlighter->setSymbolColor( color );
}

void QcPostView::setMethodColor( const QColor &color )
{
  highlighter->setMethodColor( color );
}

void QcPostView::setClassColor( const QColor &color )
{
  highlighter->setClassColor( color );
}

void QcPostView::setPunctuationColor( const QColor &color )
{
  highlighter->setPunctuationColor( color );
}

void QcPostView::setStringColor( const QColor &color )
{
  highlighter->setStringColor( color );
}

void QcPostView::setCommentColor( const QColor &color )
{
  highlighter->setCommentColor( color );
}


void QcPostView::setUserColor( const QVariantList & list )
{
  if (list.count() < 2) return;
  QString regex = list[0].value<QString>();
  QColor color = list[1].value<QColor>();
  highlighter->setCustomColor(regex, color);
}

void QcPostView::setUserCommentColor( const QVariantList & list )
{
  if (list.count() < 2) return;
  QString regex = list[0].value<QString>();
  QColor color = list[1].value<QColor>();
  highlighter->setCustomCommentColor(regex, color);
}

void QcPostView::clearUserColors( const QVariantList & list )
{
  highlighter->clearCustomColors();
}

/*
void QcPostView::replaceSelectedText( const QString &string )
{
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() ) {
    cursor.removeSelectedText();
  };
  cursor.insertText( string );
}
*/

QString & QcPostView::prepareText( QString & text ) const
{
  // NOTE: QTextDocument contains unicode paragraph separators U+2029
  // instead of newline \n characters
  return text.replace( QChar( 0x2029 ), QChar( '\n' ) );
}
