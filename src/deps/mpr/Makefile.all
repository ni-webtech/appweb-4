#
#	Makefile for the MPR - Michael's Portable Runtime
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#	This is an all-in-one build where all the MPR source files are catenated into as few files as possible. 
#	See http://hg.embedthis.com/mpr if you require the full source for the MPR.
#

include 			.makedep

MPR_LIB				+= $(BLD_LIB_DIR)/libmpr$(BLD_LIB)
MPRSSL_LIB			+= $(BLD_LIB_DIR)/libmprssl$(BLD_LIB)

TARGETS				+= $(MPR_LIB)
MPR_OBJECTS			+= mprLib
IMPORTS 			+= $(BLD_OPENSSL_IMPORTS) $(BLD_MATRIXSSL_IMPORTS) $(BLD_REGEXP_IMPORTS) $(BLD_MPR_IMPORTS)
IMPORTED			+= $(patsubst %,$(BLD_LIB_DIR)/%, $(notdir $(IMPORTS)))

MAKE_IFLAGS			+= $(BLD_REGEXP_IFLAGS) $(BLD_MPR_IFLAGS)
ifeq ($(BLD_FEATURE_OPENSSL),1)
	MAKE_IFLAGS		+= $(BLD_OPENSSL_IFLAGS)
endif
ifeq ($(BLD_FEATURE_MATRIXSSL),1)
	MAKE_IFLAGS		+= $(BLD_MATRIXSSL_IFLAGS)
endif

ifeq ($(BLD_FEATURE_SSL),1)
	TARGETS			+= $(MPRSSL_LIB)
	SSL_OBJECTS		+= mprSsl
endif

ifeq ($(BUILD_NATIVE_OR_COMPLETE_CROSS),1)
	TARGETS			+= $(BLD_BIN_DIR)/makerom$(BLD_EXE)
endif

compileTargets: imports $(TARGETS)

#
#	MPR library
#
$(MPR_LIB): $(OBJECTS)
	bld --library $(MPR_LIB) $(MPR_OBJECTS)

#
#	SSL library
#
$(MPRSSL_LIB): $(OBJECTS)
	bld --library $(MPRSSL_LIB) --search "$(BLD_SSL_WITHLIBPATHS)" --libs "mpr $(BLD_SSL_WITHLIBS)" $(SSL_OBJECTS)

#
#	makerom utility
#
$(BLD_BIN_DIR)/makerom$(BLD_EXE): $(OBJECTS) $(MPR_LIB)
	bld --exe $(BLD_BIN_DIR)/makerom$(BLD_EXE) --search "$(BLD_SSL_LIBPATHS)" --libs "mpr $(BLD_SSL_LIBS)" makerom

#
#	Import required libraries
#
imports: $(IMPORTED)

$(IMPORTED): $(IMPORTS)
	getlib $^
ifeq ($(BLD_FEATURE_MATRIXSSL),1)
	[ -x $(BLD_TOP)/projects/MACOSX ] && find $(BLD_TOP)/projects/MACOSX -name Debug | \
		while read d ; do cp $(BLD_MATRIXSSL_IMPORTS) $$d ; done
endif

cleanExtra:
	rm -f $(IMPORTED)
