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

#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QRect>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QSizeF>
#include <QPointer>

// For test
#include <QTextBrowser>
#include <QDialog>
#include <QGridLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QLabel>
// End

#include "qprintereasy.h"
#include "qprintereasy_p.h"


void QPrinterEasy::clearHeaders()
{
    qDeleteAll(d->m_Headers);
}

void QPrinterEasy::clearFooters()
{
    qDeleteAll(d->m_Footers);
}

///////////////////////////////////////////////////////////
///////////// QPrinterEasy ////////////////////////////////
///////////////////////////////////////////////////////////

QPrinterEasy::QPrinterEasy( QObject * parent )
    : QObject(parent),
    d(0)
{
    setObjectName("QPrinterEasy");
    d = new QPrinterEasyPrivate;
}

QPrinterEasy::~QPrinterEasy()
{
    clearHeaders();
    clearFooters();
    if (d) delete d;
    d = 0;
}

bool QPrinterEasy::askForPrinter( QWidget *parent )
{
    d->renewPrinter();
    QPrintDialog dialog(d->m_Printer, parent);
    dialog.setWindowTitle(tr("Print Document"));
    if (dialog.exec() != QDialog::Accepted)
        return true;
    return false;
}

void QPrinterEasy::setHeader( const QString & html, Presence presence, QPrinterEasy::Priority prior )
{
    QTextDocumentHeader *doc = new QTextDocumentHeader;
    doc->setHtml(html);
    doc->setPresence( presence );
    doc->setPriority( prior );
    d->m_Headers.append(doc);
}

void QPrinterEasy::setFooter( const QString & html, Presence presence, QPrinterEasy::Priority prior )
{
    QTextDocumentHeader *doc = new QTextDocumentHeader;
    doc->setHtml(html);
    doc->setPresence( presence );
    doc->setPriority( prior );
    d->m_Footers.append(doc);
}

void QPrinterEasy::setContent( const QString & html )
{
    d->content().setHtml( html );
}

bool QPrinterEasy::useDefaultPrinter()
{
    // TODO
    return true;
}

bool QPrinterEasy::previewDialog( QWidget *parent, bool test )
{
    if (!d->m_Printer)
        d->renewPrinter();

#ifdef QPRINTEREASY_DEBUG
    // For test
    if ( test ) {
        QDialog dial;
        QGridLayout g(&dial);
        QTextBrowser t(&dial);
        t.setDocument( &d->content() );
        g.addWidget( &t );

        QTextBrowser h(&dial);
        h.setDocument( d->header(EachPages) );
        g.addWidget( &h );

        QTextBrowser f(&dial);
        f.setDocument( d->footer(EachPages) );
        g.addWidget( &f );

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                           | QDialogButtonBox::Cancel);

        connect(buttonBox, SIGNAL(accepted()), &dial, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dial, SLOT(reject()));
        g.addWidget( buttonBox );
        dial.exec();
        // end of test
    }
#else
    Q_UNUSED(test);
#endif

    QPrintPreviewDialog dialog(d->m_Printer, parent);
    connect( &dialog, SIGNAL(paintRequested(QPrinter *)), this, SLOT(print(QPrinter *)) );
    dialog.exec();
    return true;
}

bool QPrinterEasy::print( QPrinter *printer )
{
    if (!printer)
        printer = d->m_Printer;

    if (!printer)
        return false;

    return d->draw();
}

void QPrinterEasy::addWatermarkPixmap( const QPixmap & pix, const Presence p , const Qt::AlignmentFlag watermarkAlign)
{
    // TODO TO TEST
    if ( ! d->m_Printer )
        return;
    d->m_WatermarkPresence = p;
    // TODO page margins, calculate rotation of the pixmap ?
    Q_UNUSED(watermarkAlign);
    d->m_Watermark = pix;
}

void QPrinterEasy::addWatermarkHtml( const QString & html,
                                     const Presence p,
                                     const Qt::Alignment watermarkAlignment,
                                     const int orientation )
{
    if ( ! d->m_Printer )
        return;
    d->m_WatermarkPresence = p;

    // TODO Manage page margins +++

    // get some values about the printing page and prepare the pixmap
    QRectF pageRect = d->m_Printer->pageRect();
    int rotationAngle = orientation;

    d->m_Watermark = QPixmap( pageRect.width(), pageRect.height() );
    d->m_Watermark.fill();

    QPointF pageCenter( pageRect.center() );

    // Calculates the painting area for the text
    QTextDocument wm;
    wm.setHtml( html );
    wm.setTextWidth( pageRect.width() );
    QRectF textRect = QRectF( QPointF(0,0), wm.size() );

    rotationAngle = d->calculateWatermarkRotation( textRect, pageRect, watermarkAlignment );

    // Prepare painter
    QPainter painter;
    painter.begin( &d->m_Watermark );
    painter.translate( -pageRect.topLeft() );  // TODO : this is wrong because we loose the margins
    painter.save();
    // rotate the painter from its middle
    if ( orientation != 0 ) {
        painter.translate( textRect.center() );
        painter.rotate( rotationAngle );
        // scale textRect to feet inside the pageRect - margins
        QRectF boundingRect = d->rotatedBoundingRect(textRect, rotationAngle);
        double scale = qMin( pageRect.width() / boundingRect.width(), pageRect.height() / boundingRect.height() );
        painter.scale( scale, scale );
        painter.translate( -textRect.center() );
    }
    painter.translate( textRect.topLeft() );
    wm.drawContents( &painter );//, textRect );
    painter.translate( -textRect.topLeft() );
    painter.drawRect( textRect );

    painter.restore();
    painter.end();
}

void QPrinterEasy::addWatermarkText( const QString & plainText,
                                     const Presence p,
                                     const Qt::Alignment watermarkAlignment,
                                     const Qt::Alignment textAlignment,
                                     const QFont & font,
                                     const QColor & color,
                                     const int orientation )
{
    if ( ! d->m_Printer )
        return;
    d->m_WatermarkPresence = p;

    // TODO Manage page margins +++

    // get some values about the printing page and prepare the pixmap
    QRectF pageRect = d->m_Printer->pageRect();
    int rotationAngle = orientation;

    d->m_Watermark = QPixmap( pageRect.width(), pageRect.height() );
    d->m_Watermark.fill();

    QPointF pageCenter( pageRect.center() );

    // Calculates the painting area for the text
    QFontMetrics fm( font );
    QRectF textRect = fm.boundingRect( pageRect.toRect(), textAlignment | Qt::TextWordWrap, plainText );

    rotationAngle = d->calculateWatermarkRotation( textRect, pageRect, watermarkAlignment );

    // Prepare painter
    QPainter painter;
    painter.begin( &d->m_Watermark );
    painter.translate( -pageRect.topLeft() );  // TODO : this is wrong because we loose the margins
    painter.save();
    painter.setPen( color );
    painter.setFont( font );

    QTextOption opt;
    opt.setAlignment( textAlignment );
    opt.setWrapMode( QTextOption::WordWrap );

    // rotate the painter from its middle
    if ( orientation != 0 ) {
        painter.translate( textRect.center() );
        painter.rotate( rotationAngle );
        // scale textRect to feet inside the pageRect - margins
        QRectF boundingRect = d->rotatedBoundingRect(textRect, rotationAngle);
        double scale = qMin( pageRect.width() / boundingRect.width(), pageRect.height() / boundingRect.height() );
        painter.scale( scale, scale );
        painter.translate( -textRect.center() );
    }
    painter.drawText( textRect, plainText, opt );
    painter.restore();
    painter.end();
}
