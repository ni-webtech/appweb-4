#
#	Makefile for the Appweb source code.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

# LIBS			:= appweb http mpr
CGI				:= mod_cgi
FILE			:= mod_file
EJS				:= mod_ejs
ESP				:= mod_esp
PHP				:= mod_php
SSL				:= mod_ssl

include 		.makedep

PRE_DIRS		+= include deps
PRE_DIRS		+= handlers filters modules
ifeq	($(BLD_FEATURE_ESP),1)
	PRE_DIRS	+= handlers/esp
endif
POST_DIRS		+= utils server test
TARGETS			+= $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
SOURCES			+= $(shell ls *.c handlers/fileHandler.c | egrep -v 'deps')
ifeq ($(BLD_FEATURE_DIR),1)
	SOURCES		+= $(shell ls handlers/dirHandler.c)
endif
OBJECTS			= $(patsubst %.c,$(BLD_OBJ_DIR)/%$(BLD_OBJ),$(notdir $(SOURCES)))
MAKE_IFLAGS		+= $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)
LIBS			:= $(BLD_APPWEB_LIBS)

#
#	Order of modules matters. Dependencies must be earlier in the list.
#
ifeq	($(BLD_FEATURE_CGI),1)
	MODULES		+= $(BLD_MOD_DIR)/$(CGI)$(BLD_SHOBJ)
endif
ifneq	($(BLD_EJS_IFLAGS),)
	MODULES		+= $(BLD_MOD_DIR)/$(EJS)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_ESP),1)
	MODULES		+= $(BLD_MOD_DIR)/$(ESP)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	MODULES		+= $(BLD_MOD_DIR)/$(PHP)$(BLD_SHOBJ)
endif
ifeq ($(BLD_FEATURE_SSL),1)
	MODULES		+= $(BLD_MOD_DIR)/$(SSL)$(BLD_SHOBJ)
endif
# TARGETS			+= $(BLD_LIB_DIR)/appweb.conf

compileFirst: tools

tools:
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) -C deps compile
ifeq ($(BLD_CROSS),1)
	echo
	$(call log) [Notice] "Building $(BLD_NAME) for ($(BLD_HOST_SYSTEM))"
endif

compileExtra: $(TARGETS)
	@T=modules D=deps/ejs ; $(DO_RECURSE)

compileFinal: $(MODULES)

#
#	Build the appweb library. Includes dirHandler, fileHandler.
#
$(BLD_LIB_DIR)/libappweb$(BLD_LIB): Makefile $(OBJECTS) $(BLD_MPR_MAKEDEP)
	bld --library $(BLD_LIB_DIR)/libappweb --search "$(BLD_APPWEB_LIBPATHS)" --libs "http $(BLD_APPWEB_WITHLIBS)" $(OBJECTS)

#
#	Build Modules
#
$(BLD_MOD_DIR)/$(CGI)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(CGI) --libs "$(LIBS)" $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)

$(BLD_MOD_DIR)/$(EJS)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(EJS) --search "$(BLD_EJS_LIBPATHS)" --libs "$(LIBS) $(BLD_EJS_LIBS)" \
		$(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ)

$(BLD_MOD_DIR)/$(ESP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/espFramework$(BLD_OBJ) $(BLD_OBJ_DIR)/espHandler$(BLD_OBJ) \
		$(BLD_OBJ_DIR)/espTemplate$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(ESP) --libs "$(LIBS) $(BLD_ESP_LIBS)" $(BLD_OBJ_DIR)/esp*$(BLD_OBJ)

$(BLD_MOD_DIR)/$(PHP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(PHP) --rpath "$(BLD_MOD_PREFIX)" \
		--search "$(BLD_PHP_LIBPATHS)" --libs "$(BLD_PHP_LIBS) $(LIBS)" $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ)

$(BLD_MOD_DIR)/$(SSL)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(SSL) --search "$(BLD_SSL_LIBPATHS)" --libs "$(LIBS) $(BLD_SSL_LIBS)" \
		$(BLD_OBJ_DIR)/sslModule$(BLD_OBJ)

cleanExtra:
	rm -f $(MODULES)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
