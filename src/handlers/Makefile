#
#	Makefile for the handlers
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

OBJECTS				= 

include 			.makedep

OBJECTS				:= \
	$(BLD_OBJ_DIR)/fileHandler$(BLD_OBJ)

ifeq	($(BLD_FEATURE_CGI),1)
	OBJECTS 		+= $(BLD_OBJ_DIR)/cgiHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_DIR),1)
	OBJECTS 		+= $(BLD_OBJ_DIR)/dirHandler$(BLD_OBJ)
endif
ifneq	($(BLD_EJS_IFLAGS),)
	OBJECTS 		+= $(BLD_OBJ_DIR)/ejsHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_PHP),1)
	OBJECTS			+= $(BLD_OBJ_DIR)/phpHandler$(BLD_OBJ)
endif
ifeq	($(BLD_FEATURE_ESP),1)
	PRE_DIRS		+= esp
endif

ejsHandler_IFLAGS	+= $(BLD_EJS_IFLAGS)
phpHandler_IFLAGS	+= $(BLD_PHP_IFLAGS)
DEPEND_FLAGS		+= $(BLD_PHP_IFLAGS)

compileExtra: $(OBJECTS)

#
#	Remove shorten-64 warnings because PHP headers trigger this
#
_CFLAGS				:= $(patsubst -Wshorten-64-to-32,,$(_CFLAGS))

ifeq	($(BLD_FEATURE_PHP),1)
	IMPORTS 	+= $(BLD_OPENSSL_IMPORTS) $(BLD_PHP_IMPORTS)
	IMPORTED	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))
endif

compileExtra: 	$(IMPORTED)

$(IMPORTED): $(IMPORTS)
	getlib $^
	$(call log) "[Import]" "$(IMPORTS)"
	
#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
