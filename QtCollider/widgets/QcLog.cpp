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

#include "QcLog.h"
#include "../QcWidgetFactory.h"
#include "../hacks/hacks_qt.hpp"

#include <QFile>
#include <QUrl>
#include <QKeyEvent>
#include <QApplication>
#include <QTextBlock>


class QcLogFactory : public QcWidgetFactory<QcLog>
{
  void initialize( QWidgetProxy *p, QcLog *w ) {
    p->setMouseEventWidget( w->viewport() );
  }
};

QC_DECLARE_FACTORY( QcLog, QcLogFactory );




QcLog::QcLog()
{
  setAttribute(Qt::WA_AcceptTouchEvents);
  setMaximumBlockCount(5);

  //highlighter = new Highlighter(document());
}

int QcLog::selectionStart() const
{
  return textCursor().selectionStart();
}

int QcLog::selectionSize() const
{
  QTextCursor cursor = textCursor();
  return cursor.selectionEnd() - cursor.selectionStart();
}

void QcLog::select( int start, int size )
{
  if( start < 0 ) start = 0;

  QTextCursor cursor( document() );

  cursor.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, start );
  cursor.movePosition( size > 0 ? QTextCursor::Right : QTextCursor::Left,
                       QTextCursor::KeepAnchor, qAbs(size) );

  setTextCursor( cursor );
}

QString QcLog::selectedString() const
{
  QString selection( textCursor().selectedText() );
  return prepareText(selection);
}

void QcLog::replaceSelectedText( const QString &string )
{
  QTextCursor cursor( textCursor() );
  if( cursor.hasSelection() ) {
    cursor.removeSelectedText();
  };
  cursor.insertText( string );
}

QString QcLog::currentLine() const
{
    return textCursor().block().text();
}

void QcLog::setTextFont( const QFont &f )
{
  QTextCharFormat format;
  format.setFont( f );

  QTextCursor cursor( document() );
  cursor.select( QTextCursor::Document );
  cursor.mergeCharFormat( format );

  QPlainTextEdit::setFont(f);
}

void QcLog::setTextColor( const QColor &color )
{
  QTextCharFormat format;
  format.setForeground( color );

  QTextCursor cursor( document() );
  cursor.select( QTextCursor::Document );
  cursor.mergeCharFormat( format );
}


void QcLog::setBuiltinColor( const QColor &color )
{
  //highlighter->setBuiltinColor( color );
}

void QcLog::setKeywordColor( const QColor &color )
{
  //highlighter->setKeywordColor( color );
}

void QcLog::setNumberColor( const QColor &color )
{
  //highlighter->setNumberColor( color );
}

void QcLog::setEnvvarColor( const QColor &color )
{
  //highlighter->setEnvvarColor( color );
}

void QcLog::setSymbolColor( const QColor &color )
{
  //highlighter->setSymbolColor( color );
}

void QcLog::setMethodColor( const QColor &color )
{
  //highlighter->setMethodColor( color );
}

void QcLog::setClassColor( const QColor &color )
{
  //highlighter->setClassColor( color );
}

void QcLog::setPunctuationColor( const QColor &color )
{
  //highlighter->setPunctuationColor( color );
}

void QcLog::setStringColor( const QColor &color )
{
  //highlighter->setStringColor( color );
}

void QcLog::setCommentColor( const QColor &color )
{
  //highlighter->setCommentColor( color );
}


void QcLog::setUserColor( const QVariantList & list )
{
  if (list.count() < 2) return;
  QString regex = list[0].value<QString>();
  QColor color = list[1].value<QColor>();
  //highlighter->setCustomColor(regex, color);
}

void QcLog::setUserCommentColor( const QVariantList & list )
{
  if (list.count() < 2) return;
  QString regex = list[0].value<QString>();
  QColor color = list[1].value<QColor>();
  //highlighter->setCustomCommentColor(regex, color);
}

void QcLog::clearUserColors( const QVariantList & list )
{
  //highlighter->clearCustomColors();
}


void QcLog::setRangeColor( const QVariantList &list )
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

void QcLog::setRangeFont( const QVariantList & list )
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

void QcLog::setRangeText( const QVariantList & list )
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

void QcLog::insertFromMimeData ( const QMimeData * data )
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

QString & QcLog::prepareText( QString & text ) const
{
  // NOTE: QTextDocument contains unicode paragraph separators U+2029
  // instead of newline \n characters
  return text.replace( QChar( 0x2029 ), QChar( '\n' ) );
}
