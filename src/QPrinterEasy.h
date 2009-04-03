/**********************************************************************************
 *   QPrinterEasy                                                                 *
 *   Copyright (C) 2009                                                           *
 *   eric.maeker@free.fr, wathek@gmail.com, aurelien.french@gmail.com             *
 *                                                                                *
 * Copyright (c) 1998, Regents of the University of California                    *
 * All rights reserved.                                                           *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted provided that the following conditions are met:    *
 *                                                                                *
 *     * Redistributions of source code must retain the above copyright           *
 *       notice, this list of conditions and the following disclaimer.            *
 *     * Redistributions in binary form must reproduce the above copyright        *
 *       notice, this list of conditions and the following disclaimer in the      *
 *       documentation and/or other materials provided with the distribution.     *
 *     * Neither the name of the University of California, Berkeley nor the       *
 *       names of its contributors may be used to endorse or promote products     *
 *       derived from this software without specific prior written permission.    *
 *                                                                                *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY    *
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE         *
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY   *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES     *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;   *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND    *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS  *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                   *
 **********************************************************************************/
#ifndef QPRINTEREASY_H
#define QPRINTEREASY_H

#include "QPrinterEasy_global.h"
class QPrinterEasyPrivate;

#include <QObject>
#include <QFlags>
#include <QPixmap>
#include <QTextDocument>
#include <QPrinter>

class Q_EXPORT QPrinterEasy : public QObject
{
    Q_OBJECT
public:
    /** \brief This enum is used to define the presence of headers, footers, watermarks */
    enum Presence {
        OnEachPages,
        FirstPageOnly,
        SecondPageOnly,
        OnePageOnly             // must set the page number
    };

    QPrinterEasy( QObject * parent = 0);
    ~QPrinterEasy();

    bool askForPrinter( QWidget *parent = 0 );
    bool useDefaultPrinter();
    bool previewDialog( QWidget *parent = 0, bool test = false );

    void setHeader( const QString & html, const Presence p = OnEachPages );
    void setFooter( const QString & html, const Presence p = OnEachPages );
    void setContent( const QString & html );

    void addWatermark( const QPixmap & pix,
                       Presence p = OnEachPages,
                       Qt::AlignmentFlag alignement = Qt::AlignCenter)   {}

    bool print( const QTextDocument & docToPrint );

public Q_SLOTS:
    bool print( QPrinter *printer = 0 );  // used by QPrintPreviewDialog

private:
    QPrinterEasyPrivate *d;
};

#endif // QPRINTEREASY_H
