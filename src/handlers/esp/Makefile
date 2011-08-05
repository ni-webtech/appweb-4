#
#	Makefile for the ESP handler
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

ESP					:= mod_esp

include 			.makedep

ifeq	($(BLD_FEATURE_ESP),1)
	MODULES		+= $(BLD_LIB_DIR)/$(ESP)$(BLD_SHOBJ)
endif

compileExtra: $(OBJECTS) $(MODULES)

$(BLD_LIB_DIR)/$(ESP)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/esp*$(BLD_OBJ) $(BLD_LIB_DIR)/libappweb$(BLD_LIB)
	bld --shared --library $(BLD_LIB_DIR)/$(ESP) --libs "$(BLD_APPWEB_LIBS)" $(BLD_OBJ_DIR)/esp*$(BLD_OBJ)

cleanExtra:
	rm -f $(MODULES)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
