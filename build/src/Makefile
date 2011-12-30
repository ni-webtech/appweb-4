#
#	Makefile for the build src directory
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

AUTO_COMPILE	:=

include			.makedep

ifeq ($(BUILDING_DEV),0)
    OBJECTS 	:= 
else
	TARGETS		+= $(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE)
	TARGETS		+= $(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE)
	TARGETS		+= $(BLD_BUILD_BIN_DIR)/getpath$(BLD_EXE)
dependExtra: $(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE)
endif

compileExtra: $(TARGETS)

keep_dependFirst keep_compileFirst:
	echo $(BUILDING_DEV) $(BUILDING_TARGET)
	echo TARGETS $(TARGETS)
	echo OBJECTS $(OBJECTS)

$(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE): $(BLD_OBJ_DIR)/dsi$(BLD_OBJ)
	bld --exe $(BLD_BUILD_BIN_DIR)/dsi$(BLD_EXE) dsi$(BLD_OBJ)

$(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE): $(BLD_OBJ_DIR)/edep$(BLD_OBJ)
	bld --exe $(BLD_BUILD_BIN_DIR)/edep$(BLD_EXE) edep$(BLD_OBJ)

$(BLD_BUILD_BIN_DIR)/getpath$(BLD_EXE): $(BLD_OBJ_DIR)/getpath$(BLD_OBJ)
	bld --exe $(BLD_BUILD_BIN_DIR)/getpath$(BLD_EXE) getpath$(BLD_OBJ)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
