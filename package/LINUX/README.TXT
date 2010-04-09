Embedthis Embedthis Appweb(TM)
README.TXT for Installing Appweb on Linux

Welcome to the Embedthis Appweb distribution. 

This document contains describes the procedure for installing Appweb. This package contains a pre-built stand-alone (binary) web server and the development documentation, libraries and headers needed to embed Appweb in an application.

Once installed, you may read the Appweb online documentation and samples for further information. You can also read the documentation for the latest release online at:

    http://www.embedthis.com/products/appweb/doc/index.html


This software is copyrighted and distributed under license. Please read the LICENSE.TXT for details.


System Requirements

To install Embedthis Appweb on a development system, you will need at least the following:

    * Linux 2.6 or later
    * 256MB RAM
    * 10 MB free disk space
    * TCP/IP network stack

To rebuild Appweb from source code, you will need at least:

    * 30 MB disk space


Development Environment Platform Support

If you wish to embed Appweb in your application, build the samples or rebuild Embedthis Appweb from source code, you will need to use a C/C++ compiler and associated development tools. On some linux systems they are not installed by default. 


Installing Embedthis Appweb

The installation distribution includes sub-packages containing pre-built binaries, development libraries, headers, documentation and samples. You can selectively install the sub-packages or you can install the complete distribution.

For those looking to build from source code, you will need to download the separate source code distribution. The product documentation describes how to build from source. 

The Embedthis Appweb install distribution for LINUX is published in three formats:

    * Debian (*.deb) format for debian distributions including Ubuntu

    * RPM (*.rpm) format for RPM distributions including Fedora and RHEL

    * Tar format for custom installs.

It is recommended that you use the RPM or Debian formats if your system supports those package standards.  The RPM and Debian formats will register the software installation with the system's package manager so that you can query, verify, and easily manage the software on your system. 

The tar format consists of tar archives that you can manually extract for maximum control over the placement of the product files. You can however, still use the install script with the tar install distribution if your system does not support RPM.

To install the Linux RPM package:

   1. If you are installing the binary package, you will need to login as root or attain root privilege when you execute the install script.

   2. Uncompress and extract the top level files by typing

      tar xvfz appweb-VERSION-NUMBER-LINUX-i386.rpm.tar.gz

   3. ./install

   4. If required, edit /etc/appweb.conf to suit your environment and then restart Appweb via:

      service appweb restart


To install the Linux Debian package:

   1. If you are installing the binary package, you will need to login as root or attain root privilege via sudo when you execute the install script.

   2. Uncompress and extract the top level files by typing

      tar xvfz appweb-VERSION-NUMBER-LINUX-i386.deb.tar.gz

   3. Install the package via:

      dpkg -i appweb-VERSION-NUMBER-LINUX-i386.deb

   4. If required, edit /etc/appweb.conf to suit your environment and then restart Appweb via:

      invoke-rc.d appweb restart


To install the Tar Package:

   1. If you are installing the binary package, you will need to login as root or attain root privilege when you execute the install script.

   2. tar xvfz appweb-VERSION-NUMBER-LINUX-i386.tar.tar.gz

   3. cd {Desired-Directory-For-Binaries}

   4. tar xvfz appweb-VERSION-NUMBER-LINUX-i386.tar.gz


Removing Embedthis Appweb

   1. Login as root

   2. Change directory to the installation directory for Appweb. This is usually /etc/appweb

   3. Run the remove script in the install directory for Appweb by typing 

      ./remove

      or use your system removal tool. For RPM:

      rpm -e appweb

      or Debian:

      dpkg -r appweb


Running Appweb

The Appweb product installation will automatically start Appweb before it completes. Thereafter, Appweb will automatically start if the system is rebooted.

If you wish to stop Appweb, on Red Hat type:

    service appweb stop

    or on Debian, type:

    invoke-rc.d appweb stop

You can restart Appweb on Red Hat by:

    service appweb start

    or on Debian:

    invoke-rc.d appweb start

Alternatively, you may also run Appweb manually (once you have stopped the Appweb service as described above)

   1. cd /etc/appweb

   2. ./appweb --log stdout:2

Appweb is configured by default to listen on port 7777. To test that it is serving pages, point your browser at http://localhost:7777. To tailor the Embedthis Appweb configuration, including the default port, you may wish to edit the appweb.conf configuration file. The --log switch directs logging output to the standard output (stdout) at level 2. Levels range between 0 and 9 with zero being the least verbose.


Building the Source Code

If you wish to rebuild the Appweb product from the distributed source code, please download the source code distribution from the Embedthis web site. Then read "Building from Source" in the Appweb product documentation for an explanation of the necessary steps to successfully rebuild the product.


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
