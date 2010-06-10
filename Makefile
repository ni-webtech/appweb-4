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

#
#	MOB -- this should be moved out into a script in build/bin
#
diff import sync:
	if [ ! -x $(BLD_TOOLS_DIR)/edep$(BLD_BUILD_EXE) -a "$(BUILDING_CROSS)" != 1 ] ; then \
		$(MAKE) -S --no-print-directory _RECURSIVE_=1 -C $(BLD_TOP)/build/src compile ; \
	fi
	if [ "`git branch`" != "* master" ] ; then echo "Sync only in default branch" ; echo 255 ; fi
	import.ksh --$@ --src ../tools --dir . ../tools/build/export/export.gen
	import.ksh --$@ --src ../tools --dir . ../tools/build/export/export.configure
	import.ksh --$@ --src ../mpr --dir . ../mpr/build/export/export.gen
	import.ksh --$@ --src ../mpr --dir ./src/include --strip ./all/ ../mpr/build/export/export.h
	import.ksh --$@ --src ../mpr --dir ./src/deps/mpr --strip ./all/ ../mpr/build/export/export.c
	import.ksh --$@ --src ../pcre --dir . ../pcre/build/export/export.gen
	import.ksh --$@ --src ../pcre --dir ./src/include --strip ./all/ ../pcre/build/export/export.h
	import.ksh --$@ --src ../pcre --dir ./src/deps/pcre --strip ./all/ ../pcre/build/export/export.c
	import.ksh --$@ --src ../http --dir . ../http/build/export/export.gen
	import.ksh --$@ --src ../http --dir ./src/include --strip ./all/ ../http/build/export/export.h
	import.ksh --$@ --src ../http --dir ./src/deps/http --strip ./all/ ../http/build/export/export.c
	import.ksh --$@ --src ../ejs --dir . ../ejs/build/export/export.gen
	import.ksh --$@ --src ../ejs --dir ./src/include --strip ./all/ ../ejs/build/export/export.h
	import.ksh --$@ --src ../ejs --dir ./src/deps/ejs --strip ./all/ ../ejs/build/export/export.c
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
