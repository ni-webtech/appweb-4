#
#	Makefile for the ESP web framework
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 		.makedep

PRE_DIRS		= js
ESP				:= mod_esp

ifeq	($(BLD_FEATURE_ESP),1)
	MODULES		+= $(BLD_LIB_DIR)/$(ESP)$(BLD_SHOBJ)
	ifeq ($(BUILDING_DEV),1)
		TARGETS	+= $(BLD_BIN_DIR)/esp$(BLD_EXE)
	endif
	TARGETS		+= $(BLD_LIB_DIR)/esp-www
	TARGETS		+= $(BLD_LIB_DIR)/esp.conf
endif

compileExtra: $(MODULES) $(TARGETS)

$(BLD_LIB_DIR)/$(ESP)$(BLD_SHOBJ): $(OBJECTS)
	bld --shared --library $(BLD_LIB_DIR)/$(ESP) --search "$(BLD_ESP_LIBPATHS)" \
		--libs "$(LIBS) $(BLD_APPWEB_LIBS)" $(OBJECTS)

$(BLD_BIN_DIR)/esp$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/esp$(BLD_EXE) --search "$(BLD_APPWEB_LIBPATHS)" --libs "$(BLD_APPWEB_LIBS)" $(OBJECTS)

$(BLD_LIB_DIR)/esp-www: www/* www/files/*/*
	$(call log) [Copy] "$(BLD_LIB_DIR)/esp-www"
	rm -fr $(BLD_LIB_DIR)/esp-www
	mkdir -p $(BLD_LIB_DIR)/esp-www/
	cp -r www/* $(BLD_LIB_DIR)/esp-www

$(BLD_LIB_DIR)/esp.conf: esp.conf
	$(call log) [Copy] "$(BLD_LIB_DIR)/esp.conf"
	cp -r esp.conf $(BLD_LIB_DIR)/esp.conf

cleanExtra:
	rm -rf $(BLD_LIB_DIR)/esp-www 
	rm -f $(BLD_LIB_DIR)/esp.conf
	rm -f $(MODULES)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
