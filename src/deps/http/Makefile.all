#
#	Makefile for the Http Library
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#	This is an all-in-one build where all the Http source files are catenated into as few files as possible. 
#	See http://hg.embedthis.com/mpr if you require the full source for the Http.
#

include 			.makedep

HTTP_LIB			+= $(BLD_LIB_DIR)/libhttp$(BLD_LIB)
TARGETS				+= $(HTTP_LIB)
HTTP_OBJECTS		+= httpLib

MAKE_IFLAGS			+= $(BLD_HTTP_IFLAGS)

ifeq ($(BUILD_NATIVE_OR_COMPLETE_CROSS),1)
	TARGETS			+= $(BLD_BIN_DIR)/http$(BLD_EXE)
endif

compileTargets: $(TARGETS)

$(HTTP_LIB): $(OBJECTS)
	bld --library $(HTTP_LIB) --search "$(BLD_MPR_LIBPATHS) $(BLD_SSL_LIBPATHS)" \
		--libs "$(BLD_MPR_LIBS) $(BLD_SSL_LIBS)" $(HTTP_OBJECTS)

$(BLD_BIN_DIR)/http$(BLD_EXE): $(OBJECTS) $(HTTP_LIB)
	bld --exe $(BLD_BIN_DIR)/http$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS) $(BLD_SSL_LIBPATHS)" \
		--libs "$(BLD_HTTP_LIBS) $(BLD_SSL_LIBS) $(BLD_MPR_LIBS)" http
