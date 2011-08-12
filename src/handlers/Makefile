#
#	Makefile for the handlers
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

OBJECTS				= 
CGI				:= mod_cgi
FILE			:= mod_file
EJS				:= mod_ejs
PHP				:= mod_php

include 		.makedep

ifeq	($(BLD_FEATURE_ESP),1)
	POST_DIRS		+= esp
endif
ifeq	($(BLD_FEATURE_CGI),1)
	OBJECTS		+= $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_EJSCRIPT),1)
	OBJECTS 	+= $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	OBJECTS		+= $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ)
endif

#
#	Order of modules matters. Dependencies must be earlier in the list.
#
ifeq	($(BLD_FEATURE_CGI),1)
	MODULES		+= $(BLD_LIB_DIR)/$(CGI)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_EJSCRIPT),1)
	MODULES		+= $(BLD_LIB_DIR)/$(EJS)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	MODULES		+= $(BLD_LIB_DIR)/$(PHP)$(BLD_SHOBJ)
	IMPORTS 	+= $(BLD_PHP_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))
endif

ejsHandler_IFLAGS	+= $(BLD_EJSCRIPT_IFLAGS)
phpHandler_IFLAGS	+= $(BLD_PHP_IFLAGS)
DEPEND_FLAGS		+= $(BLD_PHP_IFLAGS)
LIBS				+= $(BLD_APPWEB_LIBS)

#
#	Remove shorten-64 warnings because PHP headers trigger this
#
_CFLAGS				:= $(patsubst -Wshorten-64-to-32,,$(_CFLAGS))

compileExtra: $(IMPORTED) $(OBJECTS) $(MODULES)

$(IMPORTED): $(IMPORTS)
	getlib $^
	$(call log) "[Import]" "$(IMPORTS)"
	
#
#	Build Modules (esp does its own build, see ./esp/Makefile)
#
$(BLD_LIB_DIR)/$(CGI)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(CGI) --libs "$(LIBS)" $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)

$(BLD_LIB_DIR)/$(EJS)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(EJS) --search "$(BLD_EJSCRIPT_LIBPATHS)" --libs "$(LIBS) $(BLD_EJSCRIPT_LIBS)" \
		$(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ)

$(BLD_LIB_DIR)/$(PHP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(PHP) --rpath "$(BLD_LIB_PREFIX)" \
		--search "$(BLD_PHP_LIBPATHS)" --libs "$(BLD_PHP_LIBS) $(LIBS)" $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ)

cleanExtra:
	rm -f $(MODULES)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
