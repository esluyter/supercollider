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

  errorFormat.setForeground(QColor(230, 85, 68));
  //classFormat.setFontWeight(QFont::Bold);
  warningFormat.setForeground(QColor(194, 133, 0));
  successFormat.setForeground(QColor(77, 162, 75));

  buildRules();
}

void PostHighlighter::buildRules()
{
  HighlightingRule rule;
  highlightingRules.clear();

  rule.pattern = QRegExp("^\\->\\s.*$");
  rule.format = successFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("^WARNING:\\s.*$");
  rule.format = warningFormat;
  highlightingRules.append(rule);

  rule.pattern = QRegExp("^ERROR:\\s.*$");
  rule.format = errorFormat;
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
}

void PostHighlighter::setSuccessColor(const QColor &color)
{
  successFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setWarningColor(const QColor &color)
{
  warningFormat.setForeground(color);
  buildRules();
  rehighlight();
}

void PostHighlighter::setErrorColor(const QColor &color)
{
  errorFormat.setForeground(color);
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

void PostHighlighter::clearCustomColors()
{
  customRules.clear();
  buildRules();
  rehighlight();
}


QcPostView::QcPostView()
{
  setAttribute(Qt::WA_AcceptTouchEvents);
  setMaximumBlockCount(100);
  setReadOnly(true);
  setCursorWidth(2);

  highlighter = new PostHighlighter(document());
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


void QcPostView::setSuccessColor( const QColor &color )
{
  highlighter->setSuccessColor( color );
}

void QcPostView::setWarningColor( const QColor &color )
{
  highlighter->setWarningColor( color );
}

void QcPostView::setErrorColor( const QColor &color )
{
  highlighter->setErrorColor( color );
}


void QcPostView::setUserColor( const QVariantList & list )
{
  if (list.count() < 2) return;
  QString regex = list[0].value<QString>();
  QColor color = list[1].value<QColor>();
  highlighter->setCustomColor(regex, color);
}

void QcPostView::clearUserColors( const QVariantList & list )
{
  highlighter->clearCustomColors();
}

QString & QcPostView::prepareText( QString & text ) const
{
  // NOTE: QTextDocument contains unicode paragraph separators U+2029
  // instead of newline \n characters
  return text.replace( QChar( 0x2029 ), QChar( '\n' ) );
}
