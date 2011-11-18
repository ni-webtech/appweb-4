#
#	Makefile for the Appweb dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

PRE_DIRS		+= mpr pcre http sqlite
APPMAN			:= $(BLD_BIN_DIR)/appman$(BLD_EXE)

ifneq ($(BLD_OS),VXWORKS)
	TARGETS		+= $(APPMAN)
endif

compileExtra: 	$(TARGETS)

$(APPMAN): $(BLD_BIN_DIR)/manager$(BLD_EXE)
	cp $(BLD_BIN_DIR)/manager$(BLD_EXE) $(APPMAN)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
