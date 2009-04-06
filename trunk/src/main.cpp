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

#include <QFile>
#include <QApplication>
#include <QDir>
#include <QtDebug>
#include <QTextFrame>

#include "QPrinterEasy.h"

QByteArray readEntireFile(const QString &fileName)
{
	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly))
		return f.readAll();

	qCritical("Error in loading %s", qPrintable(fileName));
	return QByteArray();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDir dir(app.applicationDirPath());
    QString header = readEntireFile(dir.filePath("header.html"));
    QString header2 = readEntireFile(dir.filePath("header_2.html"));
    QString footer = readEntireFile(dir.filePath("footer.html"));
    QString watermark = readEntireFile(dir.filePath("watermark.html"));
    QString document = QString::fromUtf8(readEntireFile(dir.filePath("document.html")));

    QTextDocument td(document);
    qWarning() << "document blockCount" << td.blockCount();

    QTextFrame::iterator it;
    int i = 0;
    for (it = td.rootFrame()->begin(); !(it.atEnd()); ++it) {
        ++i;
    }
    qWarning() << "document frameCount" << i;

    QPrinterEasy pe;
    // for test
    pe.askForPrinter();
    pe.addWatermarkHtml( watermark,
                         QPrinterEasy::EachPages,
                         Qt::AlignCenter );
    // end test

//    pe.setHeader( header, QPrinterEasy::FirstPageOnly );
//    pe.setFooter( footer, QPrinterEasy::SecondPageOnly );
//    pe.setContent( document );
//    pe.previewDialog();

    return 0;
}
