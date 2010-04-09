Embedthis Embedthis Appweb(TM)
README.TXT for Installing Appweb on MAC OS X.

Welcome to the Embedthis Appweb distribution. 

This document contains describes the procedure for installing Appweb. This package contains a pre-built stand-alone (binary) web server and the development documentation, libraries and headers needed to embed Appweb in an application. It also contains the complete source code for Appweb.

Once installed, you may read the Appweb online documentation and samples for further information. You can also read the documentation for the latest release online at:

    http://www.embedthis.com/products/appweb/doc/index.html


This software is copyrighted and distributed under license. Please read the LICENSE.TXT for details.


System Requirements

    * MAC OS X 10.4 or later
    * 256MB RAM
    * 10 MB free disk space
    * TCP/IP network stack

To rebuild Appweb from source code, you will need at least:

    * 30 MB disk space


Development Environment Platform Support

If you wish to embed Appweb in your application, build the samples or rebuild Embedthis Appweb from source code, you will need to install the Apple Xcode C/C++ compiler and associated development tools. 


    * VxWorks Tornado II or later


Installing Embedthis Appweb

The installation distribution includes sub-packages containing pre-built binaries, development libraries, headers, documentation and samples. You can selectively install the sub-packages or you can install the complete distribution.


To install the Appweb package:

   1. Download and run the installation package. It is typically named something like:

        appweb-VERSION-BUILD-Apple-MACOSX-i386.dmg

   2. You will be prompted to enter the administrators password when you install the Appweb package.


Removing Embedthis Appweb

   1. Run a terminal window

   2. Change directory to the installation directory for Appweb. This is usually /etc/appweb

   3. Run the remove script in the install directory for Appweb by typing 

      sudo ./remove


Running Appweb

The Appweb product installation will automatically start Appweb before it completes. Thereafter, Appweb will automatically start if the system is rebooted.

If you wish to stop Appweb run a terminal window and enter the following command:

    launchctl stop com.embedthis.appweb

You can restart Appweb on Red Hat by:

    launchctl start com.embedthis.appweb

Alternatively, you may also run Appweb manually (once you have stopped the Appweb service as described above)

   1. cd /etc/appweb

   2. ./appweb --log stdout:2

Appweb is configured by default to listen on port 7777. To test that it is serving pages, point your browser at http://localhost:7777. To tailor the Embedthis Appweb configuration, including the default port, you may wish to edit the appweb.conf configuration file. The --log switch directs logging output to the standard output (stdout) at level 2. Levels range between 0 and 9 with zero being the least verbose.


Building the Source Code

If you wish to rebuild the Appweb product from the distributed source code, please read "Building from Source" in the Appweb product documentation for an explanation of the necessary steps to successfully rebuild the product.


Exporting
This distribution includes cryptographic software.  The country in which you currently reside may have restrictions on the import, possession, use, and/or re-export to another country, of encryption software. BEFORE using any encryption software, please check your country's laws, regulations and policies concerning the import, possession, or use, and re-export of encryption software, to see if this is permitted.  See <http://www.wassenaar.org/> for more information.

The U.S. Government Department of Commerce, Bureau of Industry and Security (BIS), has classified this software as Export Commodity Control Number (ECCN) 5D002.C.1, which includes information security software using or performing cryptographic functions with asymmetric algorithms. This release is eligible for export under the License Exception ENC Technology Software Unrestricted (TSU) exception (see the BIS Export Administration Regulations, Section 740.13) for both object code and source code.


License and Copyright Information

This software is supplied under license. Both a GNU and commercial license are available. Please read LICENSE.TXT for details. This documentation and the software described in this document may be used and copied only in accordance with the terms of the accompanying license agreement. Product and technical information in this document is subject to change without notice and does not represent a commitment on the part of Embedthis Software LLC.


Copyright (c) Embedthis Software LLC, 2003-2007. All Rights Reserved.

Embedthis is a trademark of Embedthis Software LLC. Embedthis, Appweb, ESP and EGI are trademarks of Embedthis Software. All other brand or product names are the trademarks or registered trademarks of their respective holders.


Embedthis Software, LLC.
4616 25th Ave NE 
#733
Seattle, WA 98105.
+1 (425) 329-3490
www.embedthis.com
info@embedthis.com
