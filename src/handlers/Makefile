#
#	Makefile for the handlers
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

#
#	PHP has warnings for 64-32 conversions
#
_CFLAGS			:= $(patsubst -Wshorten-64-to-32,,$(_CFLAGS))
MAKE_IFLAGS		:= $(BLD_PHP_IFLAGS) $(BLD_EJS_IFLAGS) $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)

ifeq	($(BLD_FEATURE_PHP),1)
	IMPORTS 	+= $(BLD_OPENSSL_IMPORTS) $(BLD_PHP_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))
endif

compileExtra: 	$(IMPORTED) export

$(IMPORTED): $(IMPORTS)
	getlib $^
	$(call log) "[Import]" "$(IMPORTS)"
	
export:
ifeq	($(BLD_FEATURE_PHP),1)
	find $(BLD_TOP)/projects/MACOSX -name Debug | while read d ; do \
		cp $(BLD_PHP_IMPORTS) $$d ; done
endif

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
