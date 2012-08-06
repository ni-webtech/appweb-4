Embedthis Appweb 4.X
===

The fast, little web server for embedded applications. 

Licensing
---
See LICENSE.md for details.

### To Read Documentation:

  See http://appwebserver.org/products/appweb/doc/product/index.html

### Prerequisites:
    Ejscript (http://www.ejscript.org/downloads/ejs/download.ejs) for the Bit and Utest tools to configure and build.

### To Build:

    ./configure
    bit

    Alternatively to build without Ejscript:

    make

Images are built into */bin. The build configuration is saved in */inc/bit.h.

### To Test:

    bit test

### To Run:

    bit run

This will run appweb in the src/server directory using the src/server/appweb.conf configuration file.

### To Install:

    bit install

### To Create Packages:

    bit package

Resources
---
  - [Appweb web site](http://appwebserver.org/)
  - [Embedthis web site](http://embedthis.com/)
  - [Appweb GitHub repository](http://github.com/embedthis/appweb-4)
