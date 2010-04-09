#
#	Makefile for the http handlers
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

MAKE_IFLAGS		:= $(BLD_PHP_IFLAGS) 

ifeq	($(BLD_FEATURE_PHP),1)
	IMPORTS 	+= $(BLD_OPENSSL_IMPORTS) $(BLD_PHP_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))
endif

compileExtra: 	$(IMPORTED) export

$(IMPORTED): $(IMPORTS)
	getlib $^
	$(call log) "[Export]" "$(BLD_PHP_IMPORTS)"
	$(call log) "[Export]" "$(BLD_OPENSSL_IMPORTS)"
	
export:
ifeq	($(BLD_FEATURE_PHP),1)
	find $(BLD_TOP)/projects/MACOSX -name Debug | while read d ; do \
		cp $(BLD_PHP_IMPORTS) $$d ; done
endif
