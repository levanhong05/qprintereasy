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
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QTextLayout>
#include <QTextFrame>
#include <QTextTable>

// For test
#include <QTextBrowser>
#include <QDialog>
#include <QGridLayout>
#include <QDebug>
#include <QDialogButtonBox>
// End

/** \brief Only keeps some private members and private datas */
class QPrinterEasyPrivate
{
public:
    bool simpleDraw();
    bool complexDraw();

    // used by complexDraw()
    // creates a new page into the painter, recalculates all sizes and return the pageNumber of the created one.
    // all its params will be modified
    int complexDrawNewPage( QPainter *p, QSizeF & headerSize, QSizeF & footerSize,
                            QSizeF & pageSize, QSizeF & actualRect, QRectF & drawnedRect,
                            const int currentPageNumber );


public:
    QPrinterEasyPrivate() : m_Watermark(0), m_Printer(0) {}

    QTextDocument m_Header, m_Footer, m_Content;
    QPixmap *m_Watermark;
    QPrinter *m_Printer;
    int m_HeaderPresence, m_FooterPresence;
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

void QPrinterEasy::setHeader( const QString & html, const Presence p )
{
    d->m_Header.setHtml( html );
    d->m_HeaderPresence = p;
}

void QPrinterEasy::setFooter( const QString & html, const Presence p )
{
    d->m_Footer.setHtml( html );
    d->m_FooterPresence = p;
}

void QPrinterEasy::setContent( const QString & html )
{
    d->m_Content.setHtml( html );
}


bool QPrinterEasy::useDefaultPrinter()
{
    // TODO
    return true;
}

bool QPrinterEasy::previewDialog( QWidget *parent, bool test )
{
    if (!d->m_Printer)
        return false;

    // For test
    if ( test ) {
        QDialog dial;
        QGridLayout g(&dial);
        QTextBrowser t(&dial);
        t.setDocument( &d->m_Content );
        g.addWidget( &t );

        QTextBrowser h(&dial);
        h.setDocument( &d->m_Header );
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
        // end of test
    }

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

    // test if we can use the simpleDraw member to add header/footer to each pages
    if ( ( d->m_HeaderPresence == OnEachPages) &&
         ( d->m_FooterPresence == OnEachPages) &&
         ( d->m_Watermark == 0 ) )
        return d->simpleDraw();
    else
        return d->complexDraw();
}

bool QPrinterEasyPrivate::complexDraw()
{
    qWarning() << "complexDraw";
    QPainter painter(m_Printer);
    QTextFrame *frame = m_Content.rootFrame();

    // Here we have to print different documents :
    // 'content' is the main document to print
    // 'header' the header, see m_HeaderPresence to know when to add the header
    // 'footer' the footer, see m_FooterPresence to know when to add the footer

    // This function draw the content block by block
    // These values do not change during the printing process of blocks
    int pageWidth = m_Printer->pageRect().width() - 20;                     //TODO add margins
    m_Header.setTextWidth( pageWidth );
    m_Footer.setTextWidth( pageWidth );
    m_Content.setTextWidth( pageWidth );

    // These values change during the printing process of blocks
    QSizeF pageSize, headerSize, footerSize, blockSize, actualSize;
    QRectF drawnedRect, blockRect;
    int i = 0;
    int actualHeight = 0;
    int pageNumber = 1;
    bool newPage = true;

    QTextBlock block;
    painter.save();

    QTextFrame::iterator it;
    for (it = frame->begin(); !(it.atEnd()); ++it) {

        QTextFrame *childFrame = it.currentFrame();
        block = it.currentBlock();

        if (childFrame) {
            QTextTable *table = qobject_cast<QTextTable*>(childFrame);

            if (table) {
                qWarning() << "--> drawTable"<< childFrame->format().toFrameFormat().height();

                // calculate table height
                int tableHeight = m_Content.documentLayout()->frameBoundingRect(childFrame).height();

                // table fits on current page ? or new page ?
                if ( tableHeight + drawnedRect.height() > pageSize.height() )
                    pageNumber = complexDrawNewPage( &painter, headerSize, footerSize, pageSize,
                                                     actualSize, drawnedRect, pageNumber );

                qWarning() << "    --> Table rect" << m_Content.documentLayout()->frameBoundingRect(childFrame);
                // get position of the table into the painter
                // draw all frames/blocks of the table
            }

        }

        //            else
        //                processFrame(frameElement, childFrame);
        //
        //        } else if (childBlock.isValid())
        //            processBlock(frameElement, childBlock);

        if ( (i%15) == 0 ) {
            pageNumber = complexDrawNewPage( &painter, headerSize, footerSize, pageSize,
                                             actualSize, drawnedRect, pageNumber );
        } // end newPage

        if ( block.isValid() ) {
            // warn boundingrect
            blockRect = m_Content.documentLayout()->blockBoundingRect(block);

            //        painter.restore();
            //        painter.translate( blockRect.x(), blockRect.y() );
            painter.drawRect( blockRect );
            block.layout()->draw( &painter, QPointF(0,0) );
            //        block.layout()->draw( &painter, QPointF(blockRect.x(), blockRect.y()) );//,
            //                              QVector<QTextLayout::FormatRange> (), blockRect);
            actualHeight += blockRect.height();
            //            qWarning() << blockRect << block.text().left(50);
            //            qWarning() << block.layout()->boundingRect() << block.layout()->position();
            //            qWarning() << QPointF(blockRect.x(), blockRect.y()) << actualHeight;

            // next block
            block = block.next();
            ++i;
            //        if (i == 15) break;
        }
    }

    painter.restore();
    painter.end();

    return true;
}

int QPrinterEasyPrivate::complexDrawNewPage( QPainter *p, QSizeF & headerSize, QSizeF & footerSize,
                                             QSizeF & pageSize, QSizeF & actualSize, QRectF & drawnedRect,
                                             const int currentPageNumber )
{
    // do we have to include the header ?
    if ( ( m_HeaderPresence == QPrinterEasy::OnEachPages ) ||
         ( m_HeaderPresence == QPrinterEasy::FirstPageOnly && currentPageNumber == 1 ) ||
         ( m_HeaderPresence == QPrinterEasy::SecondPageOnly && currentPageNumber == 2 ))
        headerSize = m_Header.size();
    else
        headerSize = QSizeF(0,0);

    // do we have to include the footer ?
    if ( ( m_FooterPresence == QPrinterEasy::OnEachPages ) ||
         ( m_FooterPresence == QPrinterEasy::FirstPageOnly && currentPageNumber == 1 ) ||
         ( m_FooterPresence == QPrinterEasy::SecondPageOnly && currentPageNumber == 2 ))
        footerSize = m_Footer.size();
    else
        footerSize = QSizeF(0,0);

    // recalculate the content size of the content page
    pageSize = QSizeF( m_Printer->pageRect().width(),
                       m_Printer->pageRect().height() - headerSize.height() - footerSize.height());

    // reset drawnedSize (nothing is drawned into the new page)
    drawnedRect = QRectF(0,0,0,0);

    // do we have to create a newpage into printer ?
    if ( currentPageNumber != 1 ) {
        m_Printer->newPage();
        p->restore();
        p->translate( 0, -actualSize.height() );  // TODO <--- this is totally wrong
        p->save();
        return currentPageNumber + 1;
    }
    return currentPageNumber;
}


bool QPrinterEasyPrivate::simpleDraw()
{
    // This private member is used when there is only one header and one footer to add on each pages
    // All pages measures
    // pageWidth, margins of document, margins of printer
    int pageWidth = m_Printer->pageRect().width() - 20;                     //TODO add margins

    // Need to be recalculated for each pages
    // headerHeight, footerHeight, innerRect, currentRect, contentRect
    m_Header.setTextWidth( pageWidth );
    m_Footer.setTextWidth( pageWidth );
    int headerHeight = m_Header.size().height();
    int footerHeight = m_Footer.size().height();
    QSize size;
    size.setHeight( m_Printer->pageRect().height() - headerHeight - footerHeight );
    size.setWidth( pageWidth );
    m_Content.setPageSize(size);
    m_Content.setUseDesignMetrics(true);

    // prepare drawing areas
    QRect innerRect = m_Printer->pageRect();                                     // the content area
    innerRect.setTop(innerRect.top() + headerHeight );
    innerRect.setBottom(innerRect.bottom() - footerHeight );
    QRect currentRect = QRect(QPoint(0,0), innerRect.size());                  // content area
    QRect contentRect = QRect(QPoint(0,0), m_Content.size().toSize() );     // whole document drawing rectangle

    QPainter painter(m_Printer);
    int pageNumber = 0;

    // moving into Painter : go under the header
    painter.save();
    painter.translate(0, headerHeight);
    while (currentRect.intersects(contentRect)) {
        // draw content for this page
        m_Content.drawContents(&painter, currentRect);
        pageNumber++;
        currentRect.translate(0, currentRect.height());
        // moving into Painter : return to the beginning of the page
        painter.restore();

        // draw header
        QRectF headRect = QRectF(QPoint(0,0), m_Header.size() );
        //        painter.drawRect( headRect );
        m_Header.drawContents(&painter, headRect );

        // draw footer
        painter.save();
        painter.translate(0,m_Printer->pageRect().bottom() - footerHeight - 10);
        QRectF footRect = QRectF(QPoint(0,0), m_Footer.size() );
        //        painter.drawRect( footRect );
        m_Footer.drawContents(&painter, footRect);
        painter.restore();

        // calculate new page
        painter.save();
        painter.translate(0, -currentRect.height() * pageNumber + headerHeight);

        if (currentRect.intersects(contentRect))
            m_Printer->newPage();
    }
    painter.restore();
    painter.end();
    return true;
}
