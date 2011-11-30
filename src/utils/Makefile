#
# 	Makefile for utilities for the http web server 
#
# 	opyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

## MOB FIX
ifneq ($(BUILDING_NATIVE),0)
	TARGETS		+= $(BLD_BIN_DIR)/auth$(BLD_EXE)
	TARGETS		+= $(BLD_BIN_DIR)/setConfig$(BLD_EXE)
else
	ifeq	($(BLD_FEATURE_CGI),1)
		TARGETS	+= $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
	endif
endif
	TARGETS		+= $(BLD_BIN_DIR)/setConfig$(BLD_EXE)

compileExtra: $(TARGETS)

$(BLD_BIN_DIR)/auth$(BLD_EXE): $(BLD_OBJ_DIR)/auth$(BLD_OBJ)
	bld --exe $(BLD_BIN_DIR)/auth$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS)" --libs "$(BLD_MPR_LIBS)" \
		$(BLD_OBJ_DIR)/auth$(BLD_OBJ)

$(BLD_BIN_DIR)/cgiProgram$(BLD_EXE): $(BLD_OBJ_DIR)/cgiProgram$(BLD_OBJ)
	bld --exe $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) $(BLD_OBJ_DIR)/cgiProgram$(BLD_OBJ)
	cp $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) ../test/cgi-bin
	chmod +x ../test/cgi-bin/cgiProgram$(BLD_EXE)

$(BLD_BIN_DIR)/setConfig$(BLD_EXE): $(BLD_OBJ_DIR)/setConfig$(BLD_OBJ)
	bld --graphical --exe $(BLD_BIN_DIR)/setConfig$(BLD_EXE) --search "$(BLD_MPR_LIBPATHS)" --libs "$(BLD_MPR_LIBS)" \
		$(BLD_OBJ_DIR)/setConfig$(BLD_OBJ)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
