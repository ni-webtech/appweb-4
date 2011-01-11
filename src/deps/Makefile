#
#	Makefile for the Appweb dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

ifeq ($(BLD_MPR_BUILTIN),1)
	PRE_DIRS    += mpr
else
	CLOBBER		+= mpr/*.c mpr/*.h
	IMPORTS     += $(BLD_MPR_IMPORTS)
endif

ifeq ($(BLD_SSL_BUILTIN),1)
	
else
	IMPORTS     += $(BLD_SSL_IMPORTS) $(BLD_MATRIXSSL_IMPORTS) $(BLD_OPENSSL_IMPORTS)
endif

ifeq ($(BLD_HTTP_BUILTIN),1)
	PRE_DIRS    += http
else
	CLOBBER		+= http/*.c http/*.h
	IMPORTS     += $(BLD_HTTP_IMPORTS)
endif

ifeq ($(BLD_PCRE_BUILTIN),1)
	PRE_DIRS    += pcre
else
	CLOBBER		+= pcre/*.c pcre/*.h
	IMPORTS     += $(BLD_PCRE_IMPORTS)
endif

ifeq ($(BLD_EJS_BUILTIN),1)
	PRE_DIRS    += ejs
else
	CLOBBER		+= ejs/*.c ejs/*.h
	IMPORTS     += $(BLD_EJS_IMPORTS)
endif

IMPORTED    	+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))

compileFirst:	$(IMPORTED) export

$(IMPORTED): $(IMPORTS)
	getlib $^

export:

xexport:
ifeq    ($(BLD_FEATURE_PHP),1)
	find $(BLD_TOP)/projects/MACOSX -name Debug | while read d ; do \
		cp $(BLD_PHP_IMPORTS) $$d ; \
	done
endif

clobberExtra:
	rm -f $(CLOBBERIT)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
