# 
#	Makefile -- Top level Makefile for Appweb
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#
#	Standard Make targets supported are:
#	
#		make 						# Does a "make compile"
#		make clean					# Removes generated objects
#		make compile				# Compiles the source
#		make depend					# Generates the make dependencies
#		make test 					# Runs unit tests
#		make package				# Creates an installable package
#
#	Installation targets. Use "make ROOT_DIR=myDir" to do a custom local install:
#
#		make install				# Call install-binary + install-dev
#		make install-binary			# Install binary files
#		make install-dev			# Install development libraries and headers
#
#	To remove, use make uninstall-ITEM, where ITEM is a component above.
#

include			build/make/Makefile.top
include			build/make/Makefile.appweb

testCleanup:
	killall testAppweb >/dev/null 2>&1 ; true

diff import sync:
	if [ ! -x $(BLD_TOOLS_DIR)/edep$(BLD_BUILD_EXE) -a "$(BUILDING_CROSS)" != 1 ] ; then \
		$(MAKE) -S --no-print-directory _RECURSIVE_=1 -C $(BLD_TOP)/build/src compile ; \
	fi
	if [ "`git branch`" != "* master" ] ; then echo "Sync only in default branch" ; echo 255 ; fi
	$(BLD_TOOLS_DIR)/import.sh --$@ ../tools/releases/tools-all.tgz
	$(BLD_TOOLS_DIR)/import.sh --$@ ../ejs/releases/ejs-all.tgz
	if [ ../ejs/doc/index.html -nt doc/ejs/index.html ] ; then \
		set -x ; \
		echo "#  import ejs doc"  \
		chmod -R +w doc/ejs doc/man ; \
		rm -fr doc/ejs ; \
		mkdir -p doc/ejs ; \
		( cd ../ejs/doc ; find . -type f | \
			egrep -v '/xml/|/html/|/dsi/|.makedep|.DS_Store|.pptx|\/Archive' | cpio -pdum ../../appweb/doc/ejs ) ; \
		chmod +w doc/man/* ; \
		cp doc/ejs/man/*.1 doc/man ; \
		chmod -R +w doc/ejs ; \
		rm -f doc/api/ejsBare.html ; \
	fi
	echo

testExtra: test-projects

test-projects:
ifeq    ($(BLD_HOST_OS),WIN)
	if [ "$(BUILD_DEPTH)" -ge 3 ] ; then \
		$(BLD_TOOLS_DIR)/nativeBuild ; \
	fi
endif
