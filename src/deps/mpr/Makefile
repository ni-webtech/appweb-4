#
#	Makefile for the MPR (Multithreaded Portable Runtime)
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
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
TARGETS				+= $(BLD_BIN_DIR)/angel$(BLD_EXE)
TARGETS				+= $(BLD_BIN_DIR)/makerom$(BLD_EXE)

compileTargets: imports $(TARGETS)

$(MPR_LIB): $(OBJECTS)
	bld --library $(MPR_LIB) $(MPR_OBJECTS)

$(MPRSSL_LIB): $(OBJECTS)
	bld --library $(MPRSSL_LIB) --search "$(BLD_SSL_WITHPATHS)" --libs "mpr $(BLD_SSL_WITHLIBS)" $(SSL_OBJECTS)

$(BLD_BIN_DIR)/makerom$(BLD_EXE): $(OBJECTS) $(MPR_LIB)
	bld --exe $(BLD_BIN_DIR)/makerom$(BLD_EXE) --search "$(BLD_SSL_LIBPATHS)" --libs "mpr $(BLD_SSL_LIBS)" makerom

$(BLD_BIN_DIR)/angel$(BLD_EXE): $(BLD_LIB_DIR)/libmpr$(BLD_LIB) $(OBJECTS)
ifeq ($(BLD_UNIX_LIKE),1)
	bld --exe $(BLD_BIN_DIR)/angel$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS)" --libs "$(BLD_MPR_LIBS)" angel$(BLD_OBJ)
endif
ifeq ($(BLD_WIN_LIKE),1)
	bld --graphical --exe $(BLD_BIN_DIR)/angel$(BLD_EXE) --rpath "$(BLD_LIB_PREFIX)" --rpath "../bin" \
		--search "$(LIBPATH)" $(MODE) --libs "mpr" --syslibs "$(SYSLIBS)" angel$(BLD_OBJ) 
endif

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
