#ifndef QPRINTEREASY_H
#define QPRINTEREASY_H

#include "QPrinterEasy_global.h"
#include <QObject>
#include <QFlags>

class Q_EXPORT QPrinterEasy
{
public:
    /** \brief This enum is used to define the presence of headers, footers, watermarks */
    enum Presence {
        OnEachPages,
        FirstPageOnly,
        SecondPageOnly,
        OnePageOnly             // must set the page number
    };

    QPrinterEasy( QObject * parent );

    void addHtmlHeader( const QString & html, Presence p = OnEachPages );
    void addHtmlFooter( const QString & html, Presence p = OnEachPages );

    void addWatermark( const QPixmap & pix,
                       Presence p = OnEachPages,
                       Qt::AlignmentFlag alignement = Qt::AlignCenter);

    bool print( const QTextDocument & docToPrint );

};

#endif // QPRINTEREASY_H
