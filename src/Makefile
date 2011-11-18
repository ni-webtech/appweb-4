#
#	Makefile for the Appweb source code.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

CGI				:= mod_cgi
FILE			:= mod_file
EJS				:= mod_ejs
PHP				:= mod_php
SSL				:= mod_ssl

include 		.makedep

PRE_DIRS		+= deps
ifeq ($(BLD_FEATURE_ESP),1)
	PRE_DIRS	+= esp
endif
PRE_DIRS		+= modules
POST_DIRS		+= utils server test samples
TARGETS			+= $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
SOURCES			+= $(shell ls *.c | egrep -v 'deps')
OBJECTS			+= $(BLD_OBJ_DIR)/fileHandler$(BLD_OBJ)
OBJECTS			+= $(BLD_OBJ_DIR)/dirHandler$(BLD_OBJ)

compileFirst: prep tools

tools:
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/deps -C deps compile
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) DIR=$(DIR)/utils -C utils compile
ifeq ($(BLD_CROSS),1)
	echo
	$(call log) [Notice] "Building $(BLD_NAME) for ($(BLD_HOST_SYSTEM))"
endif

compileExtra: $(TARGETS)
	@T=modules D=modules ; $(DO_RECURSE)
	@T=modules D=esp ; $(DO_RECURSE)

#
#	Build the appweb library (includes dirHandler, fileHandler)
#
$(BLD_LIB_DIR)/libappweb$(BLD_LIB): Makefile $(OBJECTS) $(BLD_MPR_MAKEDEP)
	bld --library $(BLD_LIB_DIR)/libappweb --search "$(BLD_APPWEB_LIBPATHS)" --libs "http $(BLD_APPWEB_WITHLIBS)" $(OBJECTS)

prep:
	if [ -f $(BLD_BIN_DIR)/.ideBuild ] ; then \
		rm -f $(BLD_BIN_DIR)/* $(BLD_BIN_DIR)/.ideBuild ; \
		make -C ../build/src ; \
	fi

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
