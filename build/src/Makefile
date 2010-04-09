#
#	Makefile for the build src directory
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#	Note: we do not include .makedep here so we can bootstrap ourselves
#
BLD_TOP 		:= ../..
BUILD_CROSS		:= 0
EXPORT_OBJECTS	:= 0
MAKE_IFLAGS		:= -I..

#
#	Makefile.top always ensures the bootstrap is built. Prevent recursive calling.
#
IN_BOOTSTRAP	:= 1

all: compile

#
#	Read the build configuration settings and make variable definitions.
#
include $(BLD_TOP)/buildConfig.make

depend: $(BLD_TOOLS_DIR)/edep$(BLD_EXE)

FILES = \
	$(BLD_OBJ_DIR)/dsi$(BLD_OBJ) 	\
	$(BLD_OBJ_DIR)/edep$(BLD_OBJ) 	\
	$(BLD_OBJ_DIR)/getpath$(BLD_OBJ)

#
#	Read the Makefile rules
#
include $(BLD_TOP)/build/make/make.rules
include $(BLD_TOP)/build/make/make.$(BLD_BUILD_OS)

TARGETS		:= 	$(BLD_TOOLS_DIR)/dsi$(BLD_EXE) \
				$(BLD_TOOLS_DIR)/edep$(BLD_EXE) \
				$(BLD_TOOLS_DIR)/getpath$(BLD_EXE)

compileExtra: $(TARGETS)

$(BLD_TOOLS_DIR)/dsi$(BLD_EXE): $(BLD_OBJ_DIR)/dsi$(BLD_OBJ)
	bld --exe $(BLD_TOOLS_DIR)/dsi$(BLD_EXE) dsi$(BLD_OBJ)

$(BLD_TOOLS_DIR)/edep$(BLD_EXE): $(BLD_OBJ_DIR)/edep$(BLD_OBJ)
	bld --exe $(BLD_TOOLS_DIR)/edep$(BLD_EXE) edep$(BLD_OBJ)

$(BLD_TOOLS_DIR)/getpath$(BLD_EXE): $(BLD_OBJ_DIR)/getpath$(BLD_OBJ)
	bld --exe $(BLD_TOOLS_DIR)/getpath$(BLD_EXE) getpath$(BLD_OBJ)

cleanExtra:
	rm -f $(TARGETS) ../*.manifest *.o *.pdb *.obj 2>/dev/null ; true
