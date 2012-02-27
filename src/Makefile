#
#	Makefile for the Appweb source code.
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 		.makedep

CGI				:= mod_cgi
FILE			:= mod_file
EJS				:= mod_ejs
PHP				:= mod_php
SSL				:= mod_ssl
PRE_DIRS		+= deps
POST_DIRS		+= esp modules utils server test samples
TARGETS			+= $(BLD_LIB_DIR)/libappweb$(BLD_LIB)

compileFirst:
	if [ -f $(BLD_BIN_DIR)/.ideBuild ] ; then \
		rm -f $(BLD_BIN_DIR)/* $(BLD_BIN_DIR)/.ideBuild ; \
	fi

compileExtra: $(TARGETS)

$(BLD_LIB_DIR)/libappweb$(BLD_LIB): Makefile $(OBJECTS) $(BLD_MPR_MAKEDEP)
	bld --library $(BLD_LIB_DIR)/libappweb --search "$(BLD_APPWEB_LIBPATHS)" \
		--libs "http $(BLD_APPWEB_WITHLIBS)" $(OBJECTS)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
