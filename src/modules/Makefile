#
#	Makefile for the http modules
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

OBJECTS			=
CGI				:= mod_cgi
FILE			:= mod_file
EJS				:= mod_ejs
ESP				:= mod_esp
PHP				:= mod_php
SSL				:= mod_ssl

include 		.makedep

OBJECTS			= $(BLD_OBJ_DIR)/dirHandler$(BLD_OBJ)
OBJECTS			+= $(BLD_OBJ_DIR)/fileHandler$(BLD_OBJ)
ifeq	($(BLD_FEATURE_CGI),1)
	OBJECTS		+= $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_ESP),1)
	OBJECTS 	+= $(BLD_OBJ_DIR)/espHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_EJSCRIPT),1)
	OBJECTS 	+= $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	OBJECTS		+= $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ)
endif
ifeq ($(BLD_FEATURE_SSL),1)
	OBJECTS		+= $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ)
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
ifeq	($(BLD_FEATURE_ESP),1)
	MODULES		+= $(BLD_LIB_DIR)/$(ESP)$(BLD_SHOBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	MODULES		+= $(BLD_LIB_DIR)/$(PHP)$(BLD_SHOBJ)
	IMPORTS 	+= $(BLD_PHP_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(BLD_PHP_IMPORTS)))
endif
ifeq ($(BLD_FEATURE_SSL),1)
	MODULES		+= $(BLD_LIB_DIR)/$(SSL)$(BLD_SHOBJ)
	IMPORTS 	+= $(BLD_OPENSSL_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(BLD_OPENSSL_IMPORTS)))
endif
ifeq ($(MACOSX),1)
	sslModule_IFLAGS   +=  -Wno-deprecated-declarations
endif

sslModule_IFLAGS	+= $(BLD_OPENSSL_IFLAGS) $(BLD_MATRIXSSL_IFLAGS) 
ejsHandler_IFLAGS	+= $(BLD_EJSCRIPT_IFLAGS)
phpHandler_IFLAGS	+= $(BLD_PHP_IFLAGS)
DEPEND_FLAGS		+= $(BLD_PHP_IFLAGS) $(BLD_OPENSSL_IFLAGS) $(BLD_MATRIXSSL_IFLAGS) 
LIBS				+= $(BLD_APPWEB_LIBS)

#
#	Remove shorten-64 warnings because PHP headers trigger this (Ugh!)
#
_CFLAGS				:= $(patsubst -Wshorten-64-to-32,,$(_CFLAGS))

compileExtra: $(IMPORTED) $(OBJECTS) 
	
modules: $(MODULES) $(IMPORTED)

#
#	Build Modules (esp does its own build, see ./esp/Makefile)
#
$(BLD_LIB_DIR)/$(CGI)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(CGI) --libs "$(LIBS)" $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)

$(BLD_LIB_DIR)/$(EJS)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(EJS) --search "$(BLD_EJSCRIPT_LIBPATHS)" --libs "$(LIBS) \
		$(BLD_EJSCRIPT_LIBS)" ejsHandler$(BLD_OBJ) 

$(BLD_LIB_DIR)/$(ESP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/espHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB) \
		$(BLD_OBJ_DIR)/espAbbrev$(BLD_OBJ) $(BLD_OBJ_DIR)/espHandler$(BLD_OBJ) $(BLD_OBJ_DIR)/espFramework$(BLD_OBJ) \
		$(BLD_OBJ_DIR)/espSession$(BLD_OBJ) $(BLD_OBJ_DIR)/espTemplate$(BLD_OBJ) $(BLD_OBJ_DIR)/espHtml$(BLD_OBJ) \
		$(BLD_OBJ_DIR)/edi$(BLD_OBJ) $(BLD_OBJ_DIR)/mdb$(BLD_OBJ)
	bld --shared --library $(BLD_LIB_DIR)/$(ESP) --search "$(BLD_ESP_LIBPATHS)" --libs "$(LIBS) $(BLD_ESP_LIBS)" \
		espAbbrev$(BLD_OBJ) espHandler$(BLD_OBJ) espFramework$(BLD_OBJ) espSession$(BLD_OBJ) espTemplate$(BLD_OBJ) \
		espHtml$(BLD_OBJ) edi$(BLD_OBJ) mdb$(BLD_OBJ) sdb$(BLD_OBJ)

$(BLD_LIB_DIR)/$(PHP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(PHP) --rpath "$(BLD_LIB_PREFIX)" \
		--search "$(BLD_PHP_LIBPATHS)" --libs "$(BLD_PHP_LIBS) $(LIBS)" phpHandler$(BLD_OBJ)

$(BLD_LIB_DIR)/$(SSL)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(SSL) --search "$(BLD_SSL_LIBPATHS)" --libs "$(BLD_APPWEB_LIBS) \
		$(BLD_SSL_LIBS)" sslModule$(BLD_OBJ)

$(IMPORTED): $(IMPORTS)
	getlib $^
	$(call log) "[Import]" "$(IMPORTS)"
	
cleanExtra:
	rm -f $(MODULES) $(OBJECTS)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
