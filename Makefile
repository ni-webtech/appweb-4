# 
#	Makefile -- Top level Makefile for Appweb
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
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
	$(BLD_TOOLS_DIR)/import.sh --$@ ../tools/out/releases/tools-dist.tgz
	$(BLD_TOOLS_DIR)/import.sh --$@ ../mpr/out/releases/mpr-dist.tgz
	$(BLD_TOOLS_DIR)/import.sh --$@ ../http/out/releases/http-dist.tgz
	$(BLD_TOOLS_DIR)/import.sh --$@ ../pcre/out/releases/pcre-dist.tgz
	$(BLD_TOOLS_DIR)/import.sh --$@ ../ejs/out/releases/ejs-dist.tgz
	if [ ../ejs/doc/index.html -nt doc/ejs/index.html ] ; then \
		echo "#  import ejs doc"  \
		chmod -R +w doc/ejs doc/man ; \
		rm -fr doc/ejs ; \
		mkdir -p doc/ejs ; \
		( cd ../ejs/doc ; find . -type f | \
			egrep -v '/xml/|/html/|/dsi/|.makedep|.DS_Store|.pptx|\/Archive' | cpio --quiet -pdum ../../appweb/doc/ejs ) ; \
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

ext:
	./configure --with-mpr=../mpr --with-ssl=../mpr --with-http=../http  --with-ejs=../ejs --with-sqlite=../ejs --with-pcre=../pcre

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
