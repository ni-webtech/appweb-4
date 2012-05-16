#
# Makefile for the Windows Appweb server components
#
# Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

BUILD_CROSS	:= 0

include 	.makedep 

ifeq ($(BUILDING_TARGET),1)
TARGETS		+= $(BIT_PRODUCT)Monitor$(BIT_EXE)
ifeq	($(BIT_OS),WIN)
	SYSLIBS	+= shell32.lib
	MONITOR_RES = --resources appwebMonitor.rc
endif
endif

compileExtra: $(TARGETS) 

#
#	Link the monitor
#
$(BIT_PRODUCT)Monitor$(BIT_EXE): $(BIT_LIB_DIR)/libmpr$(BIT_LIB) $(OBJECTS)
	@bld --graphical --exe $(BIT_PRODUCT)Monitor$(BIT_EXE) --rpath "$(BIT_LIB_PREFIX)" --rpath "../bin" \
		--search "$(LIBPATH)" $(MODE) --libs "mpr" $(MONITOR_RES) --syslibs "$(SYSLIBS)" \
		$(BIT_OBJ_DIR)/$(BIT_PRODUCT)Monitor$(BIT_OBJ) 
	cp $(BIT_PRODUCT)Monitor$(BIT_EXE) $(BIT_BIN_DIR)/$(BIT_PRODUCT)Monitor$(BIT_EXE) 
	cp $(BIT_PRODUCT)Monitor.ico $(BIT_BIN_DIR)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
