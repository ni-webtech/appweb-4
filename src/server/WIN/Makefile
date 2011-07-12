#
# Makefile for the Windows Appweb server components
#
# Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

BUILD_CROSS	:= 0

include 	.makedep 

TARGETS		+= $(BLD_PRODUCT)Monitor$(BLD_EXE)
ifeq	($(BLD_OS),WIN)
	SYSLIBS	+= shell32.lib
	MONITOR_RES = --resources appwebMonitor.rc
endif
//MOB MAKE_IFLAGS	+= $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)

compileExtra: $(TARGETS) 

#
#	Link the monitor
#
$(BLD_PRODUCT)Monitor$(BLD_EXE): $(BLD_LIB_DIR)/libmpr$(BLD_LIB) $(OBJECTS)
	@bld --graphical --exe $(BLD_PRODUCT)Monitor$(BLD_EXE) --rpath "$(BLD_LIB_PREFIX)" --rpath "../bin" \
		--search "$(LIBPATH)" $(MODE) --libs "mpr" $(MONITOR_RES) --syslibs "$(SYSLIBS)" \
		$(BLD_OBJ_DIR)/$(BLD_PRODUCT)Monitor$(BLD_OBJ) 
	cp $(BLD_PRODUCT)Monitor$(BLD_EXE) $(BLD_BIN_DIR)/$(BLD_PRODUCT)Monitor$(BLD_EXE) 
	cp $(BLD_PRODUCT)Monitor.ico $(BLD_BIN_DIR)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
