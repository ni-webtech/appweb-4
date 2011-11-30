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
#		make deploy					# Deploy binary files to a directory
#
#	To remove, use make uninstall-ITEM, where ITEM is a component above.
#

DEPS		= tools mpr pcre http sqlite

include		build/make/Makefile.top
include		build/make/Makefile.appweb
include		out/inc/Makefile.import

testCleanup:
	killall testAppweb >/dev/null 2>&1 ; true

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
