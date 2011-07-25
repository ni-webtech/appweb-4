#
#	Makefile for the appweb-all project
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

include     .makedep

compileExtra: Product.xcconfig

Product.xcconfig: $(BLD_INC_DIR)/buildConfig.h Makefile
	cp Product.xcconfig /tmp/$$$$.tmp ; \
	sed 's/CURRENT_PROJECT_VERSION = .*/CURRENT_PROJECT_VERSION = $(BLD_VERSION)/' </tmp/$$$$.tmp >Product.xcconfig ; \
	rm -f /tmp/$$$$.tmp

#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
