TARGET = QPrinterEasy
TEMPLATE = app
DEFINES += QPRINTEREASY_LIBRARY

# include configuration file
include( src/config.pri )

# include files to compile
include( src/src.pri )

# This project builds the testing application
SOURCES *= src/main.cpp
