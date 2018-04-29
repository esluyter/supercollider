/************************************************************************
*
* Copyright 2010 Jakob Leben (jakob.leben@gmail.com)
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

#ifndef QC_CODE_EDIT
#define QC_CODE_EDIT

#include "../QcHelper.h"

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

class PostHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    PostHighlighter(QTextDocument *parent = 0);
    void setSuccessColor(const QColor &);
    void setWarningColor( const QColor & );
    void setErrorColor( const QColor & );

    void setCustomColor( const QString &, const QColor & );
    void clearCustomColors();

protected:
    void highlightBlock(const QString &text) override;

private:
    void buildRules();

    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat successFormat;
    QTextCharFormat warningFormat;
    QTextCharFormat errorFormat;

    QVector<HighlightingRule> customRules;
};

class QcPostView : public QPlainTextEdit, QcHelper
{
  Q_OBJECT
  Q_PROPERTY( int selectionStart READ selectionStart );
  Q_PROPERTY( int selectionSize READ selectionSize );
  Q_PROPERTY( QString selectedString READ selectedString );
  Q_PROPERTY( QFont textFont READ dummyFont WRITE setTextFont );
  Q_PROPERTY( QColor textColor READ dummyColor WRITE setTextColor );
  Q_PROPERTY( QString post WRITE post );

  Q_PROPERTY( QColor successColor READ dummyColor WRITE setSuccessColor );
  Q_PROPERTY( QColor warningColor READ dummyColor WRITE setWarningColor );
  Q_PROPERTY( QColor errorColor READ dummyColor WRITE setErrorColor );

  Q_PROPERTY( QVariantList userColor READ dummyVariantList WRITE setUserColor );
  Q_PROPERTY( QVariantList clearUserColors READ dummyVariantList WRITE clearUserColors );


  public:
    QcPostView();
    int selectionStart() const;
    int selectionSize() const;
    Q_INVOKABLE void select( int start, int size );
    QString selectedString() const;
    void setTextFont( const QFont & );
    void setTextColor( const QColor & );
    void post ( const QString & );

    void setSuccessColor( const QColor & ); // for syntax hiliting
    void setWarningColor( const QColor & );
    void setErrorColor( const QColor & );

    void setUserColor( const QVariantList & );
    void clearUserColors( const QVariantList & );

  private:
    QString & prepareText( QString & str ) const;

    // highlighting
    PostHighlighter *highlighter;
};

#endif
