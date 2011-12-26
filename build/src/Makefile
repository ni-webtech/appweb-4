#
#	Makefile for the build src directory
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

NATIVE_ONLY	= 1

include		.makedep

TARGETS		= 	\
	$(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE) \
	$(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE) \
	$(BLD_BUILD_BIN_DIR)/getpath$(BLD_EXE)

compileFirst: tools 
	
tools:
ifeq ($(BLD_CROSS),1)
	$(call log) [Notice] "Building Required Toolchain for ($(BLD_BUILD_SYSTEM))"
endif
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) -C . targets

targets: check $(TARGETS)

$(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE): $(BLD_OBJ_DIR)/dsi$(BLD_OBJ) dsi.c
	bld --exe $(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE) dsi$(BLD_OBJ)

$(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE): $(BLD_OBJ_DIR)/edep$(BLD_OBJ) edep.c
	bld --exe $(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE) edep$(BLD_OBJ)

$(BLD_BUILD_BIN_DIR)/getpath$(BLD_EXE): $(BLD_OBJ_DIR)/getpath$(BLD_OBJ) getpath.c
	bld --exe $(BLD_BUILD_BIN_DIR)/getpath$(BLD_EXE) getpath$(BLD_OBJ)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
