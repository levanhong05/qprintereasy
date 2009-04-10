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
#include <QPrinter>
#include <QSizeF>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QTextLayout>
#include <QTextFrame>
#include <QTextTable>
#include <QPointer>
#include <math.h>

#include "qprintereasy.h"
#include "qtextdocumentheader.h"


/** \brief Only keeps some private members and private datas */
class QPrinterEasyPrivate
{
public:
    QPrinterEasyPrivate()
                : m_Printer(0) {}//, m_header(0), m_footer(0) {}
    ~QPrinterEasyPrivate();

    QTextDocument &content() { return m_content; }
    QList<QTextDocumentHeader*> headers(int pageNumber);
    QTextDocument *header(QPrinterEasy::Presence p); // Returns 0 if there is no header for the presence
    QList<QTextDocumentHeader*> footers(int pageNumber);
    QTextDocument *footer(QPrinterEasy::Presence p); // Returns 0 if there is no footer for the presence

    void setHeader( const QString & html, QPrinterEasy::Presence p, QPrinterEasy::Priority prior );
    void setHeader( const QString & html, int pageNumber );

    void setFooter( const QString & html, QPrinterEasy::Presence p, QPrinterEasy::Priority prior );
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
    static double medianAngle( const QRectF & rect )
    {
        // calculate the median angle of the page
        double pi = 3.14159265;
        double calculatedTang = rect.height() / rect.width();
        return -atan( calculatedTang ) * 180.0 / pi;
    }
    // calculate rotation angle of watermark using the alignment (return the angle)
    int calculateWatermarkRotation( QRectF & textRect, const QRectF pageRect, const Qt::Alignment watermarkAlignment );

    // used by all
    bool presenceIsRequiredAtPage( const int page, const int presence);

private:
    // use simpleDraw or complexDraw method ?
    bool isSimple() const;// { return m_pageHeaders.isEmpty() && m_pageFooters.isEmpty() && m_Watermark.isNull(); }

    // simpleDraw method
    bool simpleDraw();
    // complexDraw method
    bool complexDraw();

public:
    QPixmap m_Watermark; // null watermark at constructor time
    int m_WatermarkPresence;
    QPrinter *m_Printer;
    QList< QPointer<QTextDocumentHeader> > m_Headers;
    QList< QPointer<QTextDocumentHeader> > m_Footers;

private:
    QTextDocument m_content;                             // TODO transform to QPointer<QTextDocument> ?
//    QPointer<QTextDocument> m_header, m_footer;          // TODO this should become obsolete
//    QMap<int, QPointer<QTextDocument> > m_pageHeaders;   // TODO --> QSet< QPointer<QTextDocumentHeader> >
//    QMap<int, QPointer<QTextDocument> > m_pageFooters;   // TODO --> QSet< QPointer<QTextDocumentHeader> >


};