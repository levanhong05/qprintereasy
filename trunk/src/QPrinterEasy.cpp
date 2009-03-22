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
#include "QPrinterEasy.h"

#include <QPainter>
#include <QRectF>
#include <QRect>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPixmap>
#include <QSizeF>

// For test
#include <QTextBrowser>
#include <QDialog>
#include <QGridLayout>
#include <QDebug>
#include <QDialogButtonBox>
// End

/** \brief Only keeps private datas */
class QPrinterEasyPrivate
{
public:
    QPrinterEasyPrivate() : m_Watermark(0), m_Printer(0) {}

    QTextDocument m_Header, m_Footer, m_Content;
    QPixmap *m_Watermark;
    QPrinter *m_Printer;
};


QPrinterEasy::QPrinterEasy( QObject * parent )
    : QObject(parent),
    d(0)
{
    setObjectName("QPrinterEasy");
    d = new QPrinterEasyPrivate();
}

QPrinterEasy::~QPrinterEasy()
{
    if (d) {
        if ( d->m_Printer )
            delete d->m_Printer;
        d->m_Printer = 0;
        delete d;
    }
    d=0;
}

bool QPrinterEasy::askForPrinter( QWidget *parent )
{
     if ( d->m_Printer )
         delete d->m_Printer;
     d->m_Printer = new QPrinter();
     QPrintDialog *dialog = new QPrintDialog(d->m_Printer, parent);
     dialog->setWindowTitle(tr("Print Document"));
     if (dialog->exec() != QDialog::Accepted)
         return true;
     return false;
}

void QPrinterEasy::setHeader( const QString & html, Presence p )
{
    d->m_Header.setHtml( html );
    qWarning() << html << d->m_Header.toHtml();
    d->m_Header.setTextWidth( d->m_Printer->paperSize(QPrinter::DevicePixel).rwidth() );
    // TODO presence
}

void QPrinterEasy::setFooter( const QString & html, Presence p )
{
    d->m_Footer.setHtml( html );
    d->m_Footer.setTextWidth( d->m_Printer->paperSize(QPrinter::DevicePixel).rwidth() );
}

void QPrinterEasy::setContent( const QString & html )
{
    d->m_Content.setHtml( html );
}


bool QPrinterEasy::useDefaultPrinter()
{
    // TODO
}

bool QPrinterEasy::previewDialog( QWidget *parent)
{
    if (!d->m_Printer)
        return false;

    QDialog dial;
    QGridLayout g(&dial);
    QTextBrowser t(&dial);
    t.setDocument( &d->m_Content );
    g.addWidget( &t );

    QTextBrowser h(&dial);
    h.setDocument( &d->m_Header );
    qWarning() << d->m_Header.toHtml();
    g.addWidget( &h );

    QTextBrowser f(&dial);
    f.setDocument( &d->m_Footer );
    g.addWidget( &f );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);

     connect(buttonBox, SIGNAL(accepted()), &dial, SLOT(accept()));
     connect(buttonBox, SIGNAL(rejected()), &dial, SLOT(reject()));
    g.addWidget( buttonBox );
    dial.exec();

    QPrintPreviewDialog dialog(d->m_Printer, parent);
    connect( &dialog, SIGNAL(paintRequested(QPrinter *)), this, SLOT(print(QPrinter *)) );
    dialog.exec();
    return true;
}

bool QPrinterEasy::print( QPrinter *printer )
{
    if ( (!printer) && (!d->m_Printer) )
        return false;
    if (!printer)
        printer = d->m_Printer;

    // prepare drawing areas
    int headerHeight = d->m_Header.size().height();
    int footerHeight = d->m_Footer.size().height();
    QRect innerRect = printer->pageRect(); // the content area
    innerRect.setTop(innerRect.top() + headerHeight );
    innerRect.setBottom(innerRect.bottom() - footerHeight );
    QRect contentRect = QRect(QPoint(0,0), d->m_Content.size().toSize() );
    QRect currentRect = QRect(QPoint(0,0), innerRect.size());

    // This is a test
    QSize size( printer->pageRect().size() );
    size.setHeight( size.height() - headerHeight - footerHeight );
    d->m_Content.setPageSize(size);
    // End of test

    QPainter painter(printer);
    int count = 0;
    painter.save();
    painter.translate(0, headerHeight); // go under the header
    while (currentRect.intersects(contentRect)) {
        d->m_Content.drawContents(&painter, currentRect);
        count++;
        currentRect.translate(0, currentRect.height()); // go under header+content
        painter.restore();  // return to the beginning of the painter

//        d->m_Header.drawContents(&painter, QRect(QPoint(10,10), d->m_Header.size().toSize() ) );
        painter.drawText(10, 10, d->m_Header.toHtml() );
//        d->m_Footer.drawContents(&painter, contentRect);
        painter.drawText(10, printer->pageRect().bottom() - 10, QString("Footer %1").arg(count));
        painter.save();
        painter.translate(0, -currentRect.height() * count + 30);
        if (currentRect.intersects(contentRect))
            printer->newPage();
    }
    painter.restore();
    painter.end();


    return true;
}


/*
    // create printer
    QPrinter printer;
    QPrintDialog pd(&printer, 0);
    pd.exec();

    // create text document
    QTextDocument texdoc;
    texdoc.setHtml(content);
    texdoc.setPageSize(printer.pageRect().size());

    QRect innerRect = printer.pageRect();
    innerRect.setTop(innerRect.top() + 20);
    innerRect.setBottom(innerRect.bottom() - 30);
    QRect contentRect = QRect(QPoint(0,0), texdoc.size().toSize());
    QRect currentRect = QRect(QPoint(0,0), innerRect.size());
    QPainter painter(&printer);
    int count = 0;
    painter.save();
    painter.translate(0, 30);
    while (currentRect.intersects(contentRect)) {
        texdoc.drawContents(&painter, currentRect);
        count++;
        currentRect.translate(0, currentRect.height());
        painter.restore();
        painter.drawText(10, 10, QString("Header %1").arg(count));
        painter.drawText(10, p.pageRect().bottom() - 10, QString("Footer %1").arg(count));
        painter.save();
        painter.translate(0, -currentRect.height() * count + 30);
        if (currentRect.intersects(contentRect))
            printer.newPage();
    }
    painter.restore();
    painter.end();
    */


