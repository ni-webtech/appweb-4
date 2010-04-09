#
#	Makefile for the Windows release
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep 

compileExtra: $(BLD_BIN_DIR)/removeFiles$(BLD_EXE)

$(BLD_BIN_DIR)/removeFiles$(BLD_EXE): $(BLD_OBJ_DIR)/removeFiles$(BLD_OBJ)
	bld --graphical --exe $(BLD_BIN_DIR)/removeFiles$(BLD_EXE) removeFiles$(BLD_OBJ)
