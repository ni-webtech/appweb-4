#
# 	Makefile for utilities
#
# 	opyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 		.makedep

ifeq ($(BUILDING_DEV),1)
	TARGETS		+= $(BLD_BIN_DIR)/auth$(BLD_EXE)
	TARGETS		+= $(BLD_BIN_DIR)/setConfig$(BLD_EXE)
endif
ifeq ($(BUILDING_TARGET),1)
	ifeq	($(BLD_FEATURE_CGI),1)
		TARGETS	+= $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
	endif
endif

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

ifeq ($(BUILDING_DEV),1)
$(BLD_OBJ_DIR)/cgiProgram$(BLD_OBJ):
	true
endif

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
