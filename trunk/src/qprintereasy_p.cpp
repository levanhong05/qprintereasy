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
 
#include "qprintereasy_p.h"

#include <QDebug>

///////////////////////////////////////////////////////////
///////////// QPrinterEasyPrivate /////////////////////////
///////////////////////////////////////////////////////////
QPrinterEasyPrivate::~QPrinterEasyPrivate()
{
    if (m_Printer)
        delete m_Printer;
//    if (m_header)
//        delete m_header;
//    if (m_footer)
//        delete m_footer;
//    qDeleteAll(m_pageHeaders);
//    qDeleteAll(m_pageFooters);
}

bool QPrinterEasyPrivate::presenceIsRequiredAtPage( const int page, const int presence)
{
    if (presence == QPrinterEasy::EachPages)
        return true;
    if ( ( presence == QPrinterEasy::OddPages ) && ((page % 2) == 1) )
        return true;
    if ( ( presence == QPrinterEasy::EventPages ) && ((page % 2) == 0) )
        return true;
    if ( (presence == QPrinterEasy::FirstPageOnly) && (page==1) )
        return true;
    if ( (presence == QPrinterEasy::SecondPageOnly) && (page==2) )
        return true;
    if ( (presence == QPrinterEasy::ButFirstPage) && (page!=1) )
        return true;
    // TODO LastPageOnly need to know the pageCount of the doc too
    return false;

    return true;
}

void QPrinterEasyPrivate::renewPrinter()
{
    if (m_Printer) {
        delete m_Printer;
        m_Printer=0;
    }
    m_Printer = new QPrinter;
}

void QPrinterEasyPrivate::setTextWidth(int width) {
    m_content.setTextWidth(width);
    // OBSOLETE
//    if (m_header)
//        m_header->setTextWidth(width);
//    if (m_footer)
//        m_footer->setTextWidth(width);
//    foreach (QTextDocument *doc, m_pageHeaders)
//        doc->setTextWidth(width);
//    foreach (QTextDocument *doc, m_pageFooters)
//        doc->setTextWidth(width);
    // END OBSOLETE
    foreach (QTextDocument *doc, m_Headers)
        doc->setTextWidth(width);
    foreach (QTextDocument *doc, m_Footers)
        doc->setTextWidth(width);


}

bool QPrinterEasyPrivate::isSimple() const // { return m_pageHeaders.isEmpty() && m_pageFooters.isEmpty() && m_Watermark.isNull(); }
{
    return ((m_Headers.count()==1) && (m_Footers.count()==1) && m_Watermark.isNull());
}

bool QPrinterEasyPrivate::complexDraw()
{
    qWarning() << "complexDraw";
    QPainter painter(m_Printer);
    QTextFrame *frame = m_content.rootFrame();

    // Here we have to print different documents :
    // 'content' is the main document to print
    // 'header' the header, see m_HeaderPresence to know when to add the header
    // 'footer' the footer, see m_FooterPresence to know when to add the footer

    // This function draw the content block by block
    // These values do not change during the printing process of blocks
    int pageWidth = m_Printer->pageRect().width() - 20;                     //TODO add margins
    setTextWidth(pageWidth);

    // These values change during the printing process of blocks
    QSizeF pageSize, headerSize, footerSize, blockSize, actualSize, drawnedSize;
    QRectF lastDrawnedRect, blockRect;
    int correctedY = 0;
    int pageNumber = 0;

    QTextBlock block;
    painter.save();

    QTextFrame::iterator it;

    for (it = frame->begin(); !(it.atEnd()); ++it) {
        QTextFrame *table = qobject_cast<QTextTable*>( it.currentFrame() );
        block = it.currentBlock();

        if (table) {
            // calculate table height
            QRectF tableRect = m_content.documentLayout()->frameBoundingRect(it.currentFrame());
            painter.drawRect(tableRect);
            painter.drawText(tableRect, QString("\n Tables are not yet supported in complex drawing.") );

            // need new page ?
            if ( tableRect.height() + drawnedSize.height() > pageSize.height() )
                pageNumber = complexDrawNewPage( painter, headerSize, footerSize, pageSize,
                                                 correctedY, drawnedSize, pageNumber );

            QPointF TablePos = QPointF( tableRect.x(), drawnedSize.height() );
            // get position of the table into the painter
            // draw all frames/blocks of the table
            // modify drawnedRect / actualRect...
            drawnedSize.setHeight( drawnedSize.height() + tableRect.size().height() +
                                   (tableRect.top() - lastDrawnedRect.bottom() ) );
            lastDrawnedRect = tableRect;

        } else if ( block.isValid() ) {

            blockRect = m_content.documentLayout()->blockBoundingRect(block);

            // need new page ?
            if ( (drawnedSize.height() + blockRect.size().height()) > pageSize.height() ) {

                int i = 0;
                QTextLayout *layout = block.layout();
                if ( layout->lineCount() > 1 ) {


                    // TODO --> draw line by line


                    qWarning() << "lines in block" << block.layout()->lineCount();
                    int heightSave = drawnedSize.height();
                    // draw the maximum lines into the page before creating a new one
                    while (layout->lineAt(i).height() + drawnedSize.height() < pageSize.height()) {
//                        layout->lineAt(i).draw( &painter, layout->lineAt(i).position() );
                        drawnedSize.setHeight( drawnedSize.height() + layout->lineAt(i).height());
                        qWarning() << "draw line" << i;
                        ++i;
                    }
                    drawnedSize.setHeight( heightSave );


                    // END TODO

                }
                pageNumber = complexDrawNewPage( painter, headerSize, footerSize, pageSize,
                                                 correctedY, drawnedSize, pageNumber );
            }

            block.layout()->draw( &painter, QPointF(0,0) );

            drawnedSize.setHeight( drawnedSize.height() + blockRect.size().height() +
                                   ( blockRect.top() - lastDrawnedRect.bottom() ) );
            lastDrawnedRect = blockRect;
        }
    }
    painter.restore();
    painter.end();
    return true;
}

int QPrinterEasyPrivate::complexDrawNewPage( QPainter &p, QSizeF & headerSize, QSizeF & footerSize,
                                             QSizeF & pageSize, int & correctedY, QSizeF & drawnedSize,
                                             int currentPageNumber )
{
    bool headerDrawned = false;
    bool footerDrawned = false;

    // correctedY --> translate painter to  (0, correctedY)  in order to paint with pageRect coordonnates

    // do we have to create a newpage into printer ?
    if ( currentPageNumber != 0 ) {
        m_Printer->newPage();
        p.restore();
        int previousHeaderHeight = 0;
        foreach( QTextDocument *doc, headers(currentPageNumber) ) {
            previousHeaderHeight += doc->size().height();
        }
        p.translate( 0, -drawnedSize.height() - previousHeaderHeight );
        correctedY += drawnedSize.height();
        p.save();
        // painter points at the beginning of the page
    }

    // do we have to include a watermark ?
    if ( presenceIsRequiredAtPage( currentPageNumber+1 , m_WatermarkPresence ) ) {
        p.save();
        p.translate(0, correctedY );
        p.drawPixmap( m_Printer->pageRect(), m_Watermark );
        p.restore();
    }

    // do we have to include the header ?
    int specialY = correctedY;
    int headerHeight = 0;
    foreach( QTextDocument *doc, headers(currentPageNumber + 1) ) {
        headerSize = doc->size();
        // draw all headers
        p.save();
        p.translate(0, specialY );
        specialY = 0;
        headerHeight += doc->size().height();
        QRectF headRect = QRectF(QPoint(0,0), headerSize );
        doc->drawContents( &p, headRect );
        p.restore();
        // translate painter under the header
        p.restore();
        p.translate( 0, doc->size().height() );
        p.save();
        headerDrawned = true;
    }
    headerSize.setHeight( headerHeight );


    // do we have to include the footer ?
    int footHeight = 0;
    foreach( QTextDocument *doc, footers(currentPageNumber + 1) ) {
        footerSize = QSizeF(doc->size().width(),0);
        footHeight += doc->size().height();
        p.save();
        p.translate(0, m_Printer->pageRect().bottom() + correctedY - footHeight - headerSize.height() - 10);
        QRectF footRect = QRectF(QPoint(0,0), QSizeF( doc->size().width(), footHeight) );
        doc->drawContents(&p, footRect);
        p.restore();
        footerDrawned = true;
    }
    footerSize.setHeight( footHeight );

    // recalculate the content size of the content page
    pageSize = QSizeF( m_Printer->pageRect().width(),
                       m_Printer->pageRect().height() - headerSize.height() - footerSize.height() - 10 );

    // reset drawnedSize (nothing is drawned into the new page)
    drawnedSize = QSizeF(0,0);

    return currentPageNumber + 1;
}

bool QPrinterEasyPrivate::simpleDraw()
{
    // This private member is used when there is only one header and one footer to add on each pages
    // All pages measures
    // pageWidth, margins of document, margins of printer
    int pageWidth = m_Printer->pageRect().width() - 20;                     //TODO add margins

    QSize headerSize(pageWidth, 0);
    QSize footerSize(pageWidth, 0);

    // Need to be recalculated for each pages
    // headerHeight, footerHeight, innerRect, currentRect, contentRect
    QTextDocument *headerDoc = header(QPrinterEasy::EachPages);
    if (headerDoc) {
        headerDoc->setTextWidth( pageWidth );
        headerSize.setHeight(headerDoc->size().height());
    }
    QTextDocument *footerDoc = footer(QPrinterEasy::EachPages);
    if (footerDoc) {
        footerDoc->setTextWidth( pageWidth );
        footerSize.setHeight(footerDoc->size().height());
    }

    QSize size;
    size.setHeight( m_Printer->pageRect().height() - headerSize.height() - footerSize.height() );
    size.setWidth( pageWidth );
    m_content.setPageSize(size);
    m_content.setUseDesignMetrics(true);

    // prepare drawing areas
    QRect innerRect = m_Printer->pageRect();                                     // the content area
    innerRect.setTop(innerRect.top() + headerSize.height() );
    innerRect.setBottom(innerRect.bottom() - footerSize.height() );
    QRect currentRect = QRect(QPoint(0,0), innerRect.size());                  // content area
    QRect contentRect = QRect(QPoint(0,0), m_content.size().toSize() );     // whole document drawing rectangle

    QPainter painter(m_Printer);
    int pageNumber = 0;

    // moving into Painter : go under the header
    painter.save();
    painter.translate(0, headerSize.height());
    while (currentRect.intersects(contentRect)) {
        // draw content for this page
        m_content.drawContents(&painter, currentRect);
        pageNumber++;
        currentRect.translate(0, currentRect.height());
        // moving into Painter : return to the beginning of the page
        painter.restore();

        // draw header
        QRectF headRect = QRectF(QPoint(0,0), headerSize );
        //        painter.drawRect( headRect );
        if (headerDoc)
            headerDoc->drawContents(&painter, headRect );

        // draw footer
        painter.save();
        painter.translate(0,m_Printer->pageRect().bottom() - footerSize.height() - 10);
        QRectF footRect = QRectF(QPoint(0,0), footerSize );
        //        painter.drawRect( footRect );
        if (footerDoc)
            footerDoc->drawContents(&painter, footRect);
        painter.restore();

        // calculate new page
        painter.save();
        painter.translate(0, -currentRect.height() * pageNumber + headerSize.height());

        if (currentRect.intersects(contentRect))
            m_Printer->newPage();
    }
    painter.restore();
    painter.end();
    return true;
}

bool QPrinterEasyPrivate::draw()
{
    if (isSimple())
        return simpleDraw();
    else
        return complexDraw();
}

QList<QTextDocumentHeader*> QPrinterEasyPrivate::headers( int pageNumber )
{
    // TODO returns a QList< QTextDocumentHeader *> sorted by priority
    QList< QTextDocumentHeader *> list;
    foreach( QTextDocumentHeader *doc, m_Headers ) {
        qWarning() << "QPrinterEasyPrivate::headers" << doc->presence();
        if (presenceIsRequiredAtPage(pageNumber, doc->presence()))
            list << doc;
    }
    return list;
//
//    // if header was setted ForEachPages
//    if (m_header)
//        return m_header;
//    // else return from map
//    if (m_pageHeaders.find(pageNumber) == m_pageHeaders.end())
//        return m_header;
//    return m_pageHeaders[pageNumber];
}

QTextDocument *QPrinterEasyPrivate::header(QPrinterEasy::Presence p)
{
//    // TODO : OBSOLETE
//    switch (p) {
//    case QPrinterEasy::EachPages: return m_header;
//    case QPrinterEasy::FirstPageOnly: return header(1);
//    case QPrinterEasy::SecondPageOnly: return header(2);
//    case QPrinterEasy::LastPageOnly: return header(-1);
//    default: return 0;
//    }

    return 0;
}

QList<QTextDocumentHeader*> QPrinterEasyPrivate::footers(int pageNumber)
{
    // TODO returns a QList< QTextDocumentHeader *> sorted by priority
    QList< QTextDocumentHeader *> list;
    foreach( QTextDocumentHeader *doc, m_Footers ) {
        qWarning() << "QPrinterEasyPrivate::footers" << doc->presence();
        if (presenceIsRequiredAtPage(pageNumber, doc->presence()))
            list << doc;
    }
    return list;
//    if (m_pageFooters.find(pageNumber) == m_pageFooters.end())
//        return m_footer;
//    return m_pageFooters[pageNumber];
}

QTextDocument *QPrinterEasyPrivate::footer(QPrinterEasy::Presence p)
{
//    switch (p) {
//    case QPrinterEasy::EachPages: return m_footer;
//    case QPrinterEasy::FirstPageOnly: return footer(1);
//    case QPrinterEasy::SecondPageOnly: return footer(2);
//    case QPrinterEasy::LastPageOnly: return footer(-1);
//    default: return 0;
//    }
}

void QPrinterEasyPrivate::setHeader( const QString & html, QPrinterEasy::Presence presence, QPrinterEasy::Priority prior )
{
//    switch (p) {
//    case QPrinterEasy::EachPages:
//        if (!m_header)
//            m_header = new QTextDocument;
//        m_header->setHtml( html );
//        break;
//    case QPrinterEasy::FirstPageOnly:
//        setHeader( html, 1 );
//        break;
//    case QPrinterEasy::SecondPageOnly:
//        setHeader( html, 2 );
//        break;
//    case QPrinterEasy::LastPageOnly:
//        setHeader( html, -1 );
//        break;
//    }
//    QTextDocumentHeader *doc = new QTextDocumentHeader(html);
//    doc->setPresence( presence );
//    doc->setPriority( prior );
//    m_Headers.append(doc);// << QPointer<QTextDocumentHeader>(doc) ;
}

void QPrinterEasyPrivate::setHeader( const QString & html, int pageNumber )
{
//    if (!m_pageHeaders[pageNumber])
//        m_pageHeaders[pageNumber] = new QTextDocument;
//    m_pageHeaders[pageNumber]->setHtml(html);
}

void QPrinterEasyPrivate::setFooter( const QString &html, QPrinterEasy::Presence presence, QPrinterEasy::Priority prior )
{
//    QTextDocumentHeader *doc = new QTextDocumentHeader(html);
//    doc->setPresence( presence );
//    doc->setPriority( prior );
//    m_Footers.append( doc );
}

void QPrinterEasyPrivate::setFooter( const QString & html, int pageNumber )
{
//    if (!m_pageFooters[pageNumber])
//        m_pageFooters[pageNumber] = new QTextDocument;
//    m_pageFooters[pageNumber]->setHtml(html);
}

QRectF QPrinterEasyPrivate::rotatedBoundingRect(const QRectF &rect, int rotation)
{
    QRectF centeredRect = rect.translated( - rect.center() );
    QPolygonF polygon(centeredRect);
    QTransform transform;
    transform.rotate(rotation);
    polygon = polygon * transform;
    return polygon.boundingRect().translated(rect.center());
}

