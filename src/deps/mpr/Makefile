#
#	Makefile for the MPR (Multithreaded Portable Runtime)
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#	This is an all-in-one build where all the MPR source files are catenated into as few files as possible. 
#	See http://hg.embedthis.com/mpr if you require the full source for the MPR.
#

include 			.makedep

ifeq ($(BLD_MPR_BUILTIN),1)
	MPR_LIB				+= $(BLD_LIB_DIR)/libmpr$(BLD_LIB)
	MPRSSL_LIB			+= $(BLD_LIB_DIR)/libmprssl$(BLD_LIB)
	TARGETS				+= $(MPR_LIB)
	MPR_OBJECTS			+= mprLib
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
	TARGETS				+= $(BLD_BIN_DIR)/makerom$(BLD_EXE)
else
	COMPILE				:= manager.c
endif

ifneq ($(BLD_HOST_VXWORKS),1)
	TARGETS				+= $(BLD_BIN_DIR)/manager$(BLD_EXE)
endif

ifeq ($(MACOSX),1)
	mprSsl_IFLAGS   +=  -Wno-deprecated-declarations
endif

compileTargets: $(TARGETS)

$(MPR_LIB): $(OBJECTS)
	bld --library $(MPR_LIB) $(MPR_OBJECTS)

$(MPRSSL_LIB): $(OBJECTS)
	bld --library $(MPRSSL_LIB) --search "$(BLD_SSL_WITHPATHS)" --libs "mpr $(BLD_SSL_WITHLIBS)" $(SSL_OBJECTS)

$(BLD_BIN_DIR)/makerom$(BLD_EXE): $(OBJECTS) $(MPR_LIB)
	bld --exe $(BLD_BIN_DIR)/makerom$(BLD_EXE) --search "$(BLD_SSL_LIBPATHS)" --libs "mpr $(BLD_SSL_LIBS)" makerom

$(BLD_BIN_DIR)/manager$(BLD_EXE): $(BLD_LIB_DIR)/libmpr$(BLD_LIB) $(OBJECTS)
ifeq ($(BLD_HOST_UNIX),1)
	bld --exe $(BLD_BIN_DIR)/manager$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS)" --libs "$(BLD_MPR_LIBS)" manager$(BLD_OBJ)
endif
ifeq ($(BLD_HOST_WIN),1)
	bld --graphical --exe $(BLD_BIN_DIR)/manager$(BLD_EXE) --rpath "$(BLD_LIB_PREFIX)" --rpath "../bin" \
		--search "$(LIBPATH)" $(MODE) --libs "mpr" --syslibs "$(SYSLIBS)" manager$(BLD_OBJ) 
endif

cleanExtra:
	rm -f $(IMPORTED)
