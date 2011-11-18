#
# 	Makefile for utilities for the http web server 
#
# 	opyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

OBJECTS			=

include 		.makedep

ifeq	($(BLD_FEATURE_CGI),1)
	TARGETS		+= $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
	OBJECTS		+= $(BLD_OBJ_DIR)/cgiProgram$(BLD_OBJ)
endif
TARGETS			+= $(BLD_BIN_DIR)/auth$(BLD_EXE)
OBJECTS			+= $(BLD_OBJ_DIR)/auth$(BLD_OBJ)

ifeq ($(BUILDING_NATIVE),1)
compileExtra: $(BLD_BIN_DIR)/auth$(BLD_EXE)
else
compileExtra: $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
endif

$(BLD_BIN_DIR)/auth$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/auth$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS)" --libs "$(BLD_MPR_LIBS)" \
		$(BLD_OBJ_DIR)/auth$(BLD_OBJ)

$(BLD_BIN_DIR)/cgiProgram$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) $(BLD_OBJ_DIR)/cgiProgram$(BLD_OBJ)
	cp $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) ../server/cgi-bin
	cp $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) ../test/cgi-bin
	chmod +x ../server/cgi-bin/cgiProgram$(BLD_EXE) ../test/cgi-bin/cgiProgram$(BLD_EXE)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
