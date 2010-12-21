Embedthis Ejscript README
-------------------------

Welcome to the Embedthis Ejscript distribution. Ejscript is a powerful server-side JavaScript framework for web applications. It includes an enhanced implementation of the JavaScript language and fully supports ECMAScript Edition 3 and adds enhancments for classes, modules and an extensive system library and powerful web framework.

This document describes the installation procedure for the binary Ejscript package. This package contains pre-built language tools including an interpreter, compiler, vm, documentation generator and assembler listing program as well as the development libraries and headers needed to embed Ejscript in an application. 

You can read the documentation for the latest release online at:

    http://www.embedthis.com/products/ejs/doc/product/index.html

This software is copyrighted and distributed under license. Please read the LICENSE.TXT for details.


Table of Contents
-----------------

* System Requirements
* Installation Package Formats
* Development Environment Platform Support
* Installating the Tar Self-Install Image
* Removing Ejscript
* Apple MAC OS X  Release Details
* Linux Release Details
* Windows Release Details
* Running Ejscript
* Building the Source Code
* License and Copyright Information

System Requirements
-------------------

Operating System Support

    * Windows XP, Vista, Windows-7 (x86,x86_64)
    * Mac OS X 10.4, 10.5 (ppc,x86,x86_64)
    * Linux 2.4, 2.6 (x86,,x86_64,arm,ppc,sparc,xscale,mips)
    * VxWorks (x86,arm,ppc,sparc,xscale,mips)
    * Solaris (sparc,x86,x86_64)

To install Ejscript, your system will need at least the following:

    * 20 MB Disk
    * 4 MB RAM

To deploy Ejscript in an embedded environment, you will need at least the following:

    * 800 KB Disk / ROM space
    * 1MB RAM

To rebuild Ejscript from source code, you will need at least:

    * 50 MB disk space


Installation Package Formats
----------------------------

The Ejscript binary installation is published in several formats:

    * Native Apple MAC OS X DMG/PKG format
    * Native Linux RPM format
    * Native Linux Debian format
    * Native Windows installer format
    * Tar format for custom self installs.

All formats are wrapped in a compressed Tar archive with this README.TXT file.  It is recommended that you use one of the native O/S formats if your system supports it. The native formats will typically register the software installation with your system's software manager so that you can query, verify, and easily manage the software on your system. 

The Tar format consists of tar archives that you can manually extract for maximum control over the placement of various product components.

The installation package includes binaries, development libraries, headers and documentation. You can selectively install sub-packages or you can install the complete distribution.

For those looking to build from source code, you will need to download the separate source code distribution. The product documentation describes how to build from source. 

See below for operating system specific installation information.


Development Environment Platform Support
----------------------------------------

If you wish to embed Ejscript in your application or build Ejscript from source code, you will need to use a C compiler and associated development tools.  Several development environments are supported.  You may choose any of the following:

    * Linux/Unix/Solaris GNU Tools

    * Windows Visual Studio (Visual Studio 2008 (Express) or the Cygwin UNIX 
      emulation tools. See www.cygwin.com.

    * WindRiver Tornado Development Tools


Installating the Tar Self-Install Image
---------------------------------------

This the procedure for the self-install Tar format image:

   1. Uncompress and extract the image by typing

      tar xvf ejs-VERSION-BUILD-DIST-OS-ARCH.FMT.tar.gz

      where FMT is "tar".

   2. Invoke the installation script with administrator privilege

      sudo ./install


Removing Ejscript
-----------------

This is the procedure to remove Ejscript on all systems except Windows.

   1. Change directory to the installation directory for Ejscript. This is 
      usually /usr/lib/ejs

   2. Run the uninstall script in the install directory for Ejscript by typing 

      sudo ./uninstall


Apple Mac OS X Release Details
-------------------------------

To install the native MAC OS X DMG image:

   1. Download the DMG image.

   2. Double click on the DMG file image. This will mount a new drive called
      ejs-VERSION. Double click on this drive to display its contents.
 
   3. Double click on the "ejs.mpkg" image.

   4. Answer the questions as prompted.


Linux Release Details
---------------------

The installation images all include an "install" script that will intelligently invoke the appropriate native software package manager such as "rpm" or dpkg as required.

To install a Linux image:

   1. Uncompress and extract the image by typing:

      tar xvfz ejs-VERSION-BUILD-DIST-LINUX-ARCH.FMT.tar.gz

      Where FMT is "rpm" or "deb".

   2. Run the install script which will invoke the native package manager. If
      you are doing a Tar self-install, the install script will complete the
      installation for you.

      sudo ./install


Windows Release Details
-----------------------

To install the Windows Installer image:

   1. Login with administrator privileges. This is necessary as the binary 
      installation needs to install a Windows Service.

   2. Use WinZip or equivalent software to extract the installation image. This
      will extract the README.TXT and setup.exe from the zip archive.

   3. Double click on the setup.exe image to run it.


Removing Ejscript on Windows
----------------------------

If you installed the Ejscript Windows Installer image, follow these steps to 
remove Ejscript. If you installed the Tar self-install image, you will need 
to remove Ejscript manually. 

   1. Login with administrator privileges

   2. Run the Add / Remove Programs applet from the control panel.

   3. Select the Embedthis Ejscript and choose remove.


Running Ejscript
----------------

The Ejscript product installation includes a simple command line interpreter that can be invoked by typing:

    ejs

This command responds with the following prompt:

ejs>

Ejscript commands can be entered and results will be echoed to the console.  See the user documentation for further details.

Running Ejscript on Windows
---------------------------

On windows, the installation creates a Programs shortcut for the "ejs" Ejscript command shell.  Clicking on it will run "ejs" in a command window.  To run "ejs" from a command window manually, you may need to add the Ejscript installation bin directory to your path. This is usually the C:/Program Files/Embedthis Ejscript/bin directory. You can do this by running the System applet from the Control Panel and selecting the Advanced tab and then the Environment Variables button at the bottom of the page. Add this directory to the setting for the PATH use variable.


Building from the Source Code
-----------------------------

If you wish to rebuild the Ejscript product from the distributed source code, please download the source code distribution from the Embedthis web site. Then read "Building from Source" in the Ejscript product documentation for an explanation of the necessary steps to successfully rebuild the product.


License and Copyright Information
---------------------------------

This software is supplied under license. Both a GNU and commercial license are available. Please read LICENSE.TXT for details. This documentation and the software described in this document may be used and copied only in accordance with the terms of the accompanying license agreement. Product and technical information in this document is subject to change without notice and does not represent a commitment on the part of Embedthis Software LLC.

Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.

Embedthis is a trademark of Embedthis Software LLC. Embedthis, Appweb and Ejscript are trademarks of Embedthis Software. All other brand or product names are the trademarks or registered trademarks of their respective holders.


Embedthis Software, LLC.
4616 25th Ave NE, #733
Seattle, WA 98105.
+1 (425) 329-3490
www.embedthis.com
info@embedthis.com
