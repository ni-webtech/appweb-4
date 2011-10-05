#
#	Makefile for the Http Library
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#	This is an all-in-one build where all the Http source files are catenated into as few files as possible. 
#	See https://github.com/embedthis/http if you require the full source for Http.
#

include 			.makedep

HTTP_LIB			+= $(BLD_LIB_DIR)/libhttp$(BLD_LIB)
HTTP_OBJECTS		+= httpLib
TARGETS				+= $(HTTP_LIB) $(BLD_BIN_DIR)/http$(BLD_EXE)

compileTargets: $(TARGETS)

$(HTTP_LIB): $(OBJECTS)
	bld --library $(HTTP_LIB) --search "$(BLD_MPR_LIBPATHS) $(BLD_SSL_LIBPATHS)" \
		--libs "$(BLD_HTTP_WITHLIBS) $(BLD_MPR_LIBS) $(BLD_SSL_LIBS)" $(HTTP_OBJECTS)

$(BLD_BIN_DIR)/http$(BLD_EXE): $(OBJECTS) $(HTTP_LIB)
	bld --exe $(BLD_BIN_DIR)/http$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS) $(BLD_SSL_LIBPATHS)" \
		--libs "$(BLD_HTTP_LIBS) $(BLD_SSL_LIBS) $(BLD_MPR_LIBS)" http
