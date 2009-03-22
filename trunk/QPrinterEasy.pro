TARGET = QPrinterEasy
TEMPLATE = lib
DEFINES += QPRINTEREASY_LIBRARY

# decomment to create a staticlib
# CONFIG *= staticlib

# include configuration file
include( src/config.pri )

# include files to compile
include( src/src.pri )
