#
#	Makefile for the ESP web framework
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

PRE_DIRS	= js

ifeq	($(BLD_FEATURE_ESP),1)
	TARGETS		+= $(BLD_BIN_DIR)/esp$(BLD_EXE)
	TARGETS		+= $(BLD_LIB_DIR)/esp-www
	ESP_CMD		+= $(BLD_LIB_DIR)/esp.conf
endif

compileExtra: $(OBJECTS) 
	
modules: $(ESP_CMD) $(TARGETS)

$(BLD_BIN_DIR)/esp$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/esp$(BLD_EXE) --search "$(BLD_APPWEB_LIBPATHS)" --libs "$(BLD_APPWEB_LIBS)" \
		$(BLD_LIB_DIR)/mod_esp$(BLD_SHLIB) esp$(BLD_OBJ)

$(BLD_LIB_DIR)/esp-www: www/* www/files/*/*
	$(call log) [Copy] "www to $(BLD_LIB_DIR)/esp-www"
	rm -fr $(BLD_LIB_DIR)/esp-www
	mkdir -p $(BLD_LIB_DIR)/esp-www/
	cp -r www/* $(BLD_LIB_DIR)/esp-www

$(BLD_LIB_DIR)/esp.conf: esp.conf
	$(call log) [Copy] "esp.conf to $(BLD_LIB_DIR)/esp.conf"
	cp -r esp.conf $(BLD_LIB_DIR)/esp.conf

cleanExtra:
	rm -rf $(BLD_LIB_DIR)/esp-www 
	rm -rf $(TARGETS)
	rm -rf $(ESP_CMD)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
