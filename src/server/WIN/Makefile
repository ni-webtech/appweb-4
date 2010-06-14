#
# Makefile for the Windows Appweb server components
#
# Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

BUILD_CROSS	:= 0

include 	.makedep 

#
#	Extra libraries
#
ifeq	($(BLD_OS),WIN)
	SYSLIBS	+= shell32.lib
	MONITOR_RES = --resources appwebMonitor.rc
endif

ifeq ($(BUILD_CROSS_OR_COMPLETE_NATIVE),1)
	TARGETS	:= angel$(BLD_EXE) $(BLD_PRODUCT)Monitor$(BLD_EXE)
endif

compileExtra: $(TARGETS) 

#
#	Link the service angel
#
angel$(BLD_EXE): $(BLD_LIB_DIR)/libmpr$(BLD_LIB) $(OBJECTS)
	@bld --graphical --exe angel$(BLD_EXE) --rpath "$(BLD_LIB_PREFIX)" --rpath "../bin" \
		--search "$(LIBPATH)" $(MODE) --libs "mpr" --syslibs "$(SYSLIBS)" $(BLD_OBJ_DIR)/angel$(BLD_OBJ) 
	cp angel$(BLD_EXE) $(BLD_BIN_DIR)/angel$(BLD_EXE) 

#
#	Link the monitor
#
$(BLD_PRODUCT)Monitor$(BLD_EXE): $(BLD_LIB_DIR)/libmpr$(BLD_LIB) $(OBJECTS)
	@bld --graphical --exe $(BLD_PRODUCT)Monitor$(BLD_EXE) --rpath "$(BLD_LIB_PREFIX)" --rpath "../bin" \
		--search "$(LIBPATH)" $(MODE) --libs "mpr" $(MONITOR_RES) --syslibs "$(SYSLIBS)" \
		$(BLD_OBJ_DIR)/$(BLD_PRODUCT)Monitor$(BLD_OBJ) 
	cp $(BLD_PRODUCT)Monitor$(BLD_EXE) $(BLD_BIN_DIR)/$(BLD_PRODUCT)Monitor$(BLD_EXE) 
	cp $(BLD_PRODUCT)Monitor.ico $(BLD_BIN_DIR)
