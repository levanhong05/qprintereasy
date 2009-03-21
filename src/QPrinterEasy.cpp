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


/** \brief Only keeps private datas */
class QPrinterEasyPrivate
{
public:
    QTextDocument m_Header, m_Footer;
    QPixmap m_Watermark;
};


QPrinterEasy::QPrinterEasy( QObject * parent )
    : QObject(parent),
    d(0)
{
    d = new QPrinterEasyPrivate();
}

QPrinterEasy::~QPrinterEasy()
{
    if (d) delete d;
    d=0;
}

/*
    QTextDocument td;
    td.setHtml(content);
    QPrinter p;
    QPrintDialog pd(&p, 0);
    pd.exec();
    td.setPageSize(p.pageRect().size());
    QRect innerRect = p.pageRect();
    innerRect.setTop(innerRect.top() + 20);
    innerRect.setBottom(innerRect.bottom() - 30);
    QRect contentRect = QRect(QPoint(0,0), td.size().toSize());
    QRect currentRect = QRect(QPoint(0,0), innerRect.size());
    QPainter painter(&p);
    int count = 0;
    painter.save();
    painter.translate(0, 30);
    while (currentRect.intersects(contentRect)) {
        td.drawContents(&painter, currentRect);
        count++;
        currentRect.translate(0, currentRect.height());
        painter.restore();
        painter.drawText(10, 10, QString("Header %1").arg(count));
        painter.drawText(10, p.pageRect().bottom() - 10, QString("Footer %1").arg(count));
        painter.save();
        painter.translate(0, -currentRect.height() * count + 30);
        if (currentRect.intersects(contentRect))
            p.newPage();
    }
    painter.restore();
    painter.end();
    */


