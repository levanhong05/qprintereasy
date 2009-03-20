#ifndef QPRINTEREASY_GLOBAL_H
#define QPRINTEREASY_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QPRINTEREASY_LIBRARY)
#  define Q_EXPORT Q_DECL_EXPORT
#else
#  define Q_EXPORT Q_DECL_IMPORT
#endif

#endif // QPRINTEREASY_GLOBAL_H
