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
#include <QRectF>
#include <QRect>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QSizeF>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QTextLayout>
#include <QTextFrame>
#include <QTextTable>
#include <math.h>

// For test
#include <QTextBrowser>
#include <QDialog>
#include <QGridLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QLabel>
// End

#include "QPrinterEasy.h"

/** \brief Only keeps some private members and private datas */
class QPrinterEasyPrivate
{
public:
    QPrinterEasyPrivate()
                : m_Printer(0),
                m_header(0), m_footer(0) {}
    ~QPrinterEasyPrivate();

    QTextDocument &content() { return m_content; }
    QTextDocument *header(int pageNumber); // Returns 0 if there is no header for the page
    QTextDocument *header(QPrinterEasy::Presence p); // Returns 0 if there is no header for the presence
    QTextDocument *footer(int pageNumber); // Returns 0 if there is no footer for the page
    QTextDocument *footer(QPrinterEasy::Presence p); // Returns 0 if there is no footer for the presence

    void setHeader( const QString & html, QPrinterEasy::Presence p );
    void setHeader( const QString & html, int pageNumber );

    void setFooter( const QString & html, QPrinterEasy::Presence p );
    void setFooter( const QString & html, int pageNumber );

    // Affect the width in argument to all QTextDocument
    void setTextWidth(int width);

    // Draw all
    bool draw();

    // Destroy the current printer and recrete a brand new one
    void renewPrinter();

    // used by complexDraw()
    // creates a new page into the painter, recalculates all sizes and return the pageNumber of the created one.
    // all its params will be modified
    int complexDrawNewPage( QPainter &p, QSizeF & headerSize, QSizeF & footerSize,
                            QSizeF & pageSize, int & correctedY, QSizeF & drawnedSize,
                            int currentPageNumber );

    // Used by Watermark
    static QRectF rotatedBoundingRect(const QRectF &rect, int rotation);


public:
    QPixmap m_Watermark; // null watermark at constructor time
    QPrinter *m_Printer;

private:
    QTextDocument m_content;
    QTextDocument *m_header, *m_footer;
    QMap<int, QTextDocument*> m_pageHeaders;
    QMap<int, QTextDocument*> m_pageFooters;

    bool isSimple() const { return m_pageHeaders.isEmpty() && m_pageFooters.isEmpty() && m_Watermark.isNull(); }
    bool simpleDraw();
    bool complexDraw();
};

QPrinterEasyPrivate::~QPrinterEasyPrivate()
{
    if (m_Printer)
        delete m_Printer;
    if (m_header)
        delete m_header;
    if (m_footer)
        delete m_footer;
    qDeleteAll(m_pageHeaders);
    qDeleteAll(m_pageFooters);
}

void QPrinterEasyPrivate::renewPrinter()
{
    if (m_Printer)
        delete m_Printer;
    m_Printer = new QPrinter;
}

void QPrinterEasyPrivate::setTextWidth(int width) {
    m_content.setTextWidth(width);
    if (m_header)
        m_header->setTextWidth(width);
    if (m_footer)
        m_footer->setTextWidth(width);
    foreach (QTextDocument *doc, m_pageHeaders)
        doc->setTextWidth(width);
    foreach (QTextDocument *doc, m_pageFooters)
        doc->setTextWidth(width);
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

    // do we have to create a newpage into printer ?
    if ( currentPageNumber != 0 ) {
        m_Printer->newPage();
        p.restore();
        int previousHeaderHeight = 0;
        QTextDocument *doc = header(currentPageNumber);
        if (doc)
            previousHeaderHeight = doc->size().height();
        p.translate( 0, -drawnedSize.height() - previousHeaderHeight );
        correctedY += drawnedSize.height();
        p.save();
        // painter points at the beginning of the page
    }

    // do we have to include the header ?
    QTextDocument *doc = header(currentPageNumber + 1);
    if ( doc ) {
        headerSize = doc->size();
        // draw header
        QRectF headRect = QRectF(QPoint(0,0), headerSize );
        doc->drawContents( &p, headRect );
        // translate painter under the header
        p.restore();
        p.translate( 0, headerSize.height() );
        p.save();
        headerDrawned = true;
    } else
        headerSize = QSizeF(0,0);

    // do we have to include the footer ?
    doc = footer(currentPageNumber + 1);
    if ( doc ) {
        footerSize = doc->size();
        p.save();
        p.translate(0, m_Printer->pageRect().bottom() + correctedY - footerSize.height() - headerSize.height() - 10);
        QRectF footRect = QRectF(QPoint(0,0), footerSize );
        doc->drawContents(&p, footRect);
        p.restore();
        footerDrawned = true;
    }
    else
        footerSize = QSizeF(0,0);

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
        footerSize.setHeight(m_footer->size().height());
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

bool QPrinterEasyPrivate::draw() {
    if (isSimple())
        return simpleDraw();
    else
        return complexDraw();
}

QTextDocument *QPrinterEasyPrivate::header(int pageNumber) {
    if (m_pageHeaders.find(pageNumber) == m_pageHeaders.end())
        return m_header;
    return m_pageHeaders[pageNumber];
}

QTextDocument *QPrinterEasyPrivate::header(QPrinterEasy::Presence p) {
    switch (p) {
    case QPrinterEasy::EachPages: return m_header;
    case QPrinterEasy::FirstPageOnly: return header(1);
    case QPrinterEasy::SecondPageOnly: return header(2);
    case QPrinterEasy::LastPageOnly: return header(-1);
    default: return 0;
    }
}

QTextDocument *QPrinterEasyPrivate::footer(int pageNumber) {
    if (m_pageFooters.find(pageNumber) == m_pageFooters.end())
        return m_footer;
    return m_pageFooters[pageNumber];
}

QTextDocument *QPrinterEasyPrivate::footer(QPrinterEasy::Presence p) {
    switch (p) {
    case QPrinterEasy::EachPages: return m_footer;
    case QPrinterEasy::FirstPageOnly: return footer(1);
    case QPrinterEasy::SecondPageOnly: return footer(2);
    case QPrinterEasy::LastPageOnly: return footer(-1);
    default: return 0;
    }
}

void QPrinterEasyPrivate::setHeader( const QString & html, QPrinterEasy::Presence p )
{
    switch (p) {
    case QPrinterEasy::EachPages:
        if (!m_header)
            m_header = new QTextDocument;
        m_header->setHtml( html );
        break;
    case QPrinterEasy::FirstPageOnly:
        setHeader( html, 1 );
        break;
    case QPrinterEasy::SecondPageOnly:
        setHeader( html, 2 );
        break;
    case QPrinterEasy::LastPageOnly:
        setHeader( html, -1 );
        break;
    }
}

void QPrinterEasyPrivate::setHeader( const QString & html, int pageNumber )
{
    if (!m_pageHeaders[pageNumber])
        m_pageHeaders[pageNumber] = new QTextDocument;
    m_pageHeaders[pageNumber]->setHtml(html);
}

void QPrinterEasyPrivate::setFooter( const QString &html, QPrinterEasy::Presence p )
{
    switch (p) {
    case QPrinterEasy::EachPages:
        if (!m_footer)
            m_footer = new QTextDocument;
        m_footer->setHtml( html );
        break;
    case QPrinterEasy::FirstPageOnly:
        setFooter( html, 1 );
        break;
    case QPrinterEasy::SecondPageOnly:
        setFooter( html, 2 );
        break;
    case QPrinterEasy::LastPageOnly:
        setFooter( html, -1 );
        break;
    }
}

void QPrinterEasyPrivate::setFooter( const QString & html, int pageNumber )
{
    if (!m_pageFooters[pageNumber])
        m_pageFooters[pageNumber] = new QTextDocument;
    m_pageFooters[pageNumber]->setHtml(html);
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
    delete d;
}

bool QPrinterEasy::askForPrinter( QWidget *parent )
{
    d->renewPrinter();
    QPrintDialog *dialog = new QPrintDialog(d->m_Printer, parent);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return true;
    return false;
}

void QPrinterEasy::setHeader( const QString & html, Presence p )
{
    d->setHeader( html, p );
}

void QPrinterEasy::setHeader( const QString & html, int pageNumber )
{
    d->setHeader( html, pageNumber );
}

void QPrinterEasy::setFooter( const QString & html, Presence p )
{
    d->setFooter( html, p );
}

void QPrinterEasy::setFooter( const QString & html, int pageNumber )
{
    d->setFooter( html, pageNumber );
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
        return false;

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

void QPrinterEasy::addWatermarkPixmap( const QPixmap & , const Presence, const Qt::AlignmentFlag)
{
}

void QPrinterEasy::addWatermarkText( const QString & plainText, const QFont & font,
                                     const Presence p,
                                     const Qt::AlignmentFlag alignement,
                                     const int orientation )
{
    if ( ! d->m_Printer )
        return;

    // get some values about the printing page
    QRectF pageRect = d->m_Printer->pageRect();
    // for test
    pageRect.setWidth( pageRect.width() / 2 );
    pageRect.setHeight( pageRect.height() / 2 );
    // end test
    QPointF pageCenter( pageRect.center() );

    // for test
    QDialog dialog;
    QVBoxLayout * layout = new QVBoxLayout();
    dialog.setLayout(layout);
    QLabel label;
    QPixmap pixmap( pageRect.width(), pageRect.height() );
    pixmap.fill();
    // end test

    // Calculates the painting area for the text
    QFontMetrics fm( font );
    QRectF textRect = fm.boundingRect( pageRect.toRect(), alignement | Qt::TextWordWrap, plainText );
    textRect.moveCenter( pageRect.center() );

    // Prepare painter
    QPainter painter;
    painter.begin(&pixmap);
    painter.translate( -pageRect.topLeft() );
    painter.save();
    painter.setPen("gray");
    painter.setFont( font );

    QTextOption opt;
    opt.setAlignment( alignement );
    opt.setWrapMode( QTextOption::WordWrap );

    // rotate the painter from its middle
    if ( orientation != 0 ) {
        painter.translate( textRect.center() );
        painter.rotate( orientation );
        // scale textRect to feet inside the pageRect - margins
        QRectF boundingRect = d->rotatedBoundingRect(textRect, orientation);
        double scale = qMin( pageRect.width() / boundingRect.width(), pageRect.height() / boundingRect.height() );
        painter.scale( scale, scale );
        painter.translate( -textRect.center() );
    }
    painter.drawText( textRect, plainText, opt );
    painter.drawRect( textRect );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                       | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    label.setPixmap( pixmap );
    layout->addWidget(&label);
    layout->addWidget( buttonBox );

    painter.restore();
    painter.end();

    dialog.exec();

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
