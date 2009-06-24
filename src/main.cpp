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
#include <QString>
#include <QTextFrame>
#include <QtDebug>

#include "qprintereasy.h"

QString document;
QString header;
QString header2;
QString footer;
QString footer2;
QString watermark;


QByteArray readEntireFile(const QString &fileName)
{
	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly))
		return f.readAll();

	qCritical("Error in loading %s", qPrintable(fileName));
	return QByteArray();
}

void example1()
{
    qWarning() << "example 1 : One header, one footer on each pages";
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header );
    pe.setFooter( footer );
    pe.setContent( document );
    pe.previewDialog();
}

void example2()
{
    qWarning() << "example 2 : One header on first page only, one footer on each pages";
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header, QPrinterEasy::FirstPageOnly );
    pe.setFooter( footer );
    pe.setContent( document );
    pe.previewDialog();
}

void example3()
{
    qWarning() << "example 3 : One header on first page only, one footer on the second page only";
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header, QPrinterEasy::FirstPageOnly );
    pe.setFooter( footer, QPrinterEasy::SecondPageOnly );
    pe.setContent( document );
    pe.previewDialog();
}

void example4()
{
    qWarning() << "example 4 : Header and footer with centered plain text Watermark";
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header );
    pe.setFooter( footer );
    pe.setContent( document );
    pe.addWatermarkText( "Adding a plain text\nWATERMARK" );
    pe.previewDialog();
}


void example5()
{
    qWarning() << "example 5 : Header and footer with centered plain text Watermark on Even Pages";
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header );
    pe.setFooter( footer );
    pe.setContent( document );
    pe.addWatermarkText( "Adding a plain text\nWATERMARK", QPrinterEasy::EvenPages );
    pe.previewDialog();
}

void example6()
{
    qWarning() << "example 6 : Header and footer with a pixmap watermark on EachPages";
    QDir dir(qApp->applicationDirPath());
    QPixmap pixWatermark;
    pixWatermark.load( dir.filePath("pixmapWatermark.png") );
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header );
    pe.setFooter( footer );
    pe.setContent( document );
    pe.addWatermarkPixmap( pixWatermark, QPrinterEasy::EachPages );
    pe.previewDialog();
}

void example7()
{
    qWarning() << "example 7 : MultiHeaders, one footer, watermark on EvenPages";
    QPrinterEasy pe;
    pe.askForPrinter();
    pe.setHeader( header, QPrinterEasy::FirstPageOnly );
    pe.setHeader( header2, QPrinterEasy::EachPages );
    pe.setFooter( footer, QPrinterEasy::ButFirstPage );
    pe.setContent( document );
    pe.addWatermarkText( "Adding a plain text\nWATERMARK", QPrinterEasy::EvenPages );
    pe.previewDialog();
}

void warnDocumentBlockCount()
{
    QTextDocument td(document);
    qWarning() << "document blockCount" << td.blockCount();

    QTextFrame::iterator it;
    int i = 0;
    for (it = td.rootFrame()->begin(); !(it.atEnd()); ++it) {
        ++i;
    }
    qWarning() << "document frameCount" << i;
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDir dir(app.applicationDirPath());
    header = readEntireFile(dir.filePath("header.html"));
    header2 = readEntireFile(dir.filePath("header_2.html"));
    footer = readEntireFile(dir.filePath("footer.html"));
    footer2 = readEntireFile(dir.filePath("footer_2.html"));
    watermark = readEntireFile(dir.filePath("watermark.html"));

    QPixmap pixWatermark;
    pixWatermark.load( dir.filePath("pixmapWatermark.png") );
    document = QString::fromUtf8(readEntireFile(dir.filePath("document.html")));

    example1();
    example2();
    example3();
    example4();
    example5();
    example6();
    example7();

//    QPrinterEasy pe;
//    pe.askForPrinter();
////    pe.addWatermarkPixmap( pixWatermark, QPrinterEasy::EachPages );
//    pe.addWatermarkText( "Adding a plain text\nWATERMARK", QPrinterEasy::EvenPages, Qt::AlignCenter );
//    pe.setHeader( header );//, QPrinterEasy::FirstPageOnly );
////    pe.setHeader( header2, QPrinterEasy::EachPages );
//    pe.setFooter( footer );
////    pe.setFooter( footer2 );
////    pe.setFooter( footer );
//    pe.setContent( document );
//    pe.setOrientation(QPrinter::Portrait);
//    pe.setPaperSize(QPrinter::A4);
//    pe.previewDialog();

    return 0;
}
