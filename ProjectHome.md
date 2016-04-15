# Who are QPrinterEasy ? #

  * Main developper :
    * [Eric Maeker, MD (eric\_tux on IRC)](mailto:eric.maeker@free.fr)

  * Main contributors :
    * Guillaume Denry (guid)
    * Filipe Azevedo (for the constant help he provides - PasNox on IRC)
    * Romain Jourdan (roms18)

  * Contributors :
    * Wathek Loued (wathek)
    * Aurelien Michon (aurelien)

  * You can talk to us on IRC channel #qt-fr.

# What for ? #

  * This code aims at ease the print process of documents including headers, footers, left side bar and watermarks.
  * You can freely use this code in your productions without any restrictions. All code is published under BSD revised licence.

# What you can already do with it ? #

  * talking about SVN version [r66](https://code.google.com/p/qprintereasy/source/detail?r=66)
  * you can add multiple headers/footers on documents
  * you can add plain text, html or pixmap watermarks on documents
  * you can print it all without any effort
  * please take a look at the samples to see the complete features

# Dependencies #
  * Some parts use new functions of Qt4.5, so compilation won't work with a lower version of Qt.

# Building instructions #

Choose the build process :
  * app.pro builds an testing application
  * QPrinterEasy.pro builds a lib (static or dynamic you have to correctly configure the project file).

Run `qmake QPrinterEasy.pro && make`.

Please report the building errors into our [mailing list](mailto:qprintereasy@googlegroups.com).

# UsagesAndSamples #

# How to contribute? #

  * Anyone can contribute to this project. All contributions are released under the same licence as the project.
  * Before coding new functions please inform mainteners using our [mailing list](mailto:qprintereasy@googlegroups.com).
  * Read the issues.


# Links #

Some parts of the code are inspired of :
http://www.qtsoftware.com/developer/faqs/is-it-possible-to-set-a-header-and-footer-when-printing-a-qtextdocument

