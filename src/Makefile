#
#	Makefile for the Appweb source code.
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

LIBS			:= appweb http mpr
CGI				:= mod_cgi
FILE			:= mod_file
DIR				:= mod_dir
EJS				:= mod_ejs
PHP				:= mod_php
UPLOAD			:= mod_upload
AUTH			:= mod_auth
CHUNK			:= mod_chunk
RANGE			:= mod_range
SSL				:= mod_ssl

include 		.makedep

PRE_DIRS		+= include deps
PRE_DIRS		+= handlers filters modules
POST_DIRS		+= utils server test

TARGETS			+= $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
ifeq ($(BLD_STATIC),1)
	SOURCES		+= $(shell ls *.c handlers/*.c modules/*.c | egrep -v 'deps')
else
	SOURCES		+= $(shell ls *.c handlers/fileHandler.c handlers/dirHandler.c handlers/egiHandler.c | egrep -v 'deps')
endif
OBJECTS			= $(patsubst %.c,$(BLD_OBJ_DIR)/%$(BLD_OBJ),$(notdir $(SOURCES)))

#
#	Order of modules matters. Dependencies must be earlier in the list.
#
ifeq	($(BLD_FEATURE_CGI),1)
	MODULES		+= $(BLD_MOD_DIR)/$(CGI)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_EJS),1)
	MODULES		+= $(BLD_MOD_DIR)/$(EJS)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	MODULES		+= $(BLD_MOD_DIR)/$(PHP)$(BLD_SHOBJ)
endif
ifeq ($(BLD_FEATURE_SSL),1)
	MODULES		+= $(BLD_MOD_DIR)/$(SSL)$(BLD_SHOBJ)
endif
TARGETS			+= $(BLD_LIB_DIR)/appweb.conf

compileExtra: $(TARGETS)
ifeq ($(BLD_STATIC),0)
	@T=modules D=deps/ejs ; $(DO_RECURSE)
endif

ifeq ($(BLD_STATIC),0)
compileFinal: $(MODULES)
endif

#
#	Build the appweb library. Includes dirHandler, fileHandler
#
$(BLD_LIB_DIR)/libappweb$(BLD_LIB): Makefile $(OBJECTS) $(BLD_MPR_MAKEDEP)
	bld --library $(BLD_LIB_DIR)/libappweb --search "$(BLD_APPWEB_LIBPATHS)" --libs "http $(BLD_APPWEB_WITHLIBS)" $(OBJECTS)

#
#	Build Modules -- Always build all modules -- even if statically linking
#
$(BLD_MOD_DIR)/$(CGI)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(CGI) --libs "$(LIBS)" $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)

$(BLD_MOD_DIR)/$(EJS)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(EJS) --libs "$(LIBS) $(BLD_EJS_LIBS)" $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ)

$(BLD_MOD_DIR)/$(PHP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(PHP) --rpath "$(BLD_MOD_PREFIX)" \
		--search "$(BLD_PHP_LIBPATHS)" --libs "$(BLD_PHP_WITHLIBS) $(LIBS)" $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ)

$(BLD_MOD_DIR)/$(SSL)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_MOD_DIR)/$(SSL) --libs "$(LIBS) $(BLD_SSL_LIBS)" $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ)

$(BLD_LIB_DIR)/appweb.conf: default.conf
	$(call log) "[Install]" "$(BLD_LIB_DIR)/appweb.conf"
	cp default.conf $(BLD_LIB_DIR)/appweb.conf

cleanExtra:
	rm -f $(MODULES)
