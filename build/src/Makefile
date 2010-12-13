#
#	Makefile for the build src directory
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include			.makedep

TARGETS			:= 	\
	$(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE) \
	$(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE)

compileFirst: tools 
	
tools:
	$(call log) [Notice] "Building required tools"
	$(MAKE) -S --no-print-directory BUILDING_NATIVE=1 TRACE=$(TRACE) -C . targets

targets: check $(TARGETS)

$(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE): $(BLD_OBJ_DIR)/dsi$(BLD_OBJ) dsi.c
	bld --exe $(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE) dsi$(BLD_OBJ)

$(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE): $(BLD_OBJ_DIR)/edep$(BLD_OBJ) edep.c
	bld --exe $(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE) edep$(BLD_OBJ)
