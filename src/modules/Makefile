#
#	Makefile for the http modules
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

OBJECTS			=
SSL				:= mod_ssl

include 		.makedep

ifeq ($(BLD_FEATURE_SSL),1)
	OBJECTS		= $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ)
endif

ifeq ($(BLD_FEATURE_SSL),1)
	MAKE_IFLAGS	+= $(BLD_OPENSSL_IFLAGS) $(BLD_MATRIXSSL_IFLAGS) 
	MODULES		+= $(BLD_LIB_DIR)/$(SSL)$(BLD_SHOBJ)
	IMPORTS 	+= $(BLD_OPENSSL_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))
endif
ifeq ($(MACOSX),1)
	sslModule_IFLAGS   +=  -Wno-deprecated-declarations
endif

compileExtra: $(OBJECTS) $(MODULES) $(IMPORTED)

$(BLD_LIB_DIR)/$(SSL)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(SSL) --search "$(BLD_SSL_LIBPATHS)" --libs "$(BLD_APPWEB_LIBS) $(BLD_SSL_LIBS)" \
		$(BLD_OBJ_DIR)/sslModule$(BLD_OBJ)

$(IMPORTED): $(IMPORTS)
	getlib $^
	$(call log) "[Import]" "$(IMPORTS)"
	
cleanExtra:
	rm -f $(MODULES)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
