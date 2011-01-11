#
# 	Makefile for utilities for the http web server 
#
# 	opyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

ifeq	($(BLD_FEATURE_CGI),1)
	TARGETS		+= $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
endif
TARGETS			+= $(BLD_BIN_DIR)/auth$(BLD_EXE)
MAKE_IFLAGS		+= $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/auth$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/auth$(BLD_EXE) --libs "$(BLD_MPR_LIBS)" $(BLD_OBJ_DIR)/auth$(BLD_OBJ)

$(BLD_BIN_DIR)/cgiProgram$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) $(BLD_OBJ_DIR)/cgiProgram$(BLD_OBJ) $(BLD_OBJ_DIR)/mprLib$(BLD_OBJ)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
