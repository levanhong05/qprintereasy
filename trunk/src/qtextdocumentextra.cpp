/**********************************************************************************
 *   QPrinterEasy                                                                 *
 *   Copyright (C) 2009                                                           *
 *   eric.maeker@free.fr                                                          *
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
#include "qtextdocumentextra.h"

#include <QDomElement>
#include <QDomDocument>

/**
  \class QTextDocumentExtra
  \brief This class is used by tkPrinter for header/footer/watermark management.
  Xml structure looks like :\n
  <tkConstants::QDOCUMENT_GENERAL_XML_TAG> </idem>\n
    <tkConstants::QDOCUMENT_VERSION_XML_TAG> </idem>\n
    <tkConstants::QDOCUMENT_PRESENCE_XML_TAG> </idem>\n
    <tkConstants::QDOCUMENT_PRIORITY_XML_TAG> </idem>\n
    <tkConstants::QDOCUMENT_EXTRA_XML_TAG> </idem>\n
    <tkConstants::QDOCUMENT_HTML_XML_TAG> </idem>\n
  </tkConstants::QDOCUMENT_GENERAL_XML_TAG>\n
  You can :
  \li get a pointer to the referenced QTextDocument using document().\n
  \li set the Presence of the document according to tkPrinter::Presence\n
  \li set the Priority of the document according to tkPrinter::Priority\n
  \li get/set to/from Xml using toXml() , fromXml()\n
  \sa QPrinterEasy
*/

using namespace QTextDocumentExtraConstants;

QTextDocumentExtra::~QTextDocumentExtra()
{
    if (m_Doc) delete m_Doc;
    m_Doc = 0;
}

QString QTextDocumentExtra::toHtml() const
{
    if (m_Doc) {
        return document()->toHtml();
    }
    return m_Html;
}


QTextDocument *QTextDocumentExtra::document() const
{
    if (!m_Doc) {
        m_Doc = new QTextDocument();
        m_Doc->setHtml(m_Html);
        m_DocCreated = true;
    }
    return m_Doc;
}

void QTextDocumentExtra::setTextWidth( qreal width )
{
    document()->setTextWidth(width);
}

QString QTextDocumentExtra::toXml() const
{
    QHash<QString,QString> t;
    t.insert(QDOCUMENT_VERSION_XML_TAG, xmlVersion);
    t.insert(QDOCUMENT_PRESENCE_XML_TAG,QString::number(presence()));
    t.insert(QDOCUMENT_PRIORITY_XML_TAG, QString::number(priority()));
    t.insert(QDOCUMENT_EXTRA_XML_TAG, "");
    t.insert(QDOCUMENT_HTML_XML_TAG, toHtml());
    QDomDocument doc;
    QDomElement main = doc.createElement(QDOCUMENT_GENERAL_XML_TAG);
    doc.appendChild(main);
    foreach( const QString &k, t.keys() ) {
        QDomElement data = doc.createElement(k);
        QDomText dataText = doc.createTextNode(t.value(k));
        main.appendChild(data);
        data.appendChild(dataText);
    }
    return doc.toString(4);
}

QTextDocumentExtra *QTextDocumentExtra::fromXml(const QString &xml)
{
    if (!xml.contains(QString("<%1>").arg(QDOCUMENT_GENERAL_XML_TAG)))
        return new QTextDocumentExtra();
    QHash<QString, QString> t;

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement root = doc.documentElement();
    QDomElement paramElem = root.firstChildElement();
    while ( !paramElem.isNull() ) {
        if (!paramElem.tagName().compare( QDOCUMENT_GENERAL_XML_TAG, Qt::CaseInsensitive ) ) {
            paramElem = paramElem.nextSiblingElement();
            continue;
        }
        t.insert(paramElem.tagName(), paramElem.text().trimmed().toAscii());
        paramElem = paramElem.nextSiblingElement();
    }
    return new QTextDocumentExtra(t.value(QDOCUMENT_HTML_XML_TAG),
                                  QPrinterEasy::Presence(t.value(QDOCUMENT_PRESENCE_XML_TAG).toInt()),
                                  QPrinterEasy::Priority(t.value(QDOCUMENT_PRIORITY_XML_TAG).toInt()),
                                  t.value(QDOCUMENT_VERSION_XML_TAG));
}
