Embedthis Appweb 4.X
===

The fast, little web server for embedded applications. 

Licensing
---
Appweb is dual-licensed under a GPLv2 license and commercial licenses are offered by Embedthis Software.
See http://appwebserver.org/downloads/licensing.html for licensing details.

### To Read Documentation:

  See http://appwebserver.org/products/appweb/doc/product/index.html

### To Build:

    ./configure
    make

Images are built into */bin. The build configuration is saved in */inc/bit.h.

### To Test:

    make test

### To Run:

    sudo make -C src/server run

This will run appweb in the src/server directory using the src/server/appweb.conf configuration file.

### To Install:

    make install

### To Create Packages:

    make package

Resources
---
  - [Appweb web site](http://appwebserver.org/)
  - [Embedthis web site](http://embedthis.com/)
  - [Appweb GitHub repository](http://github.com/embedthis/appweb-4)
