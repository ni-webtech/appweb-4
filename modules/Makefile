#
#  Makefile -- Modules Makefile
#
# Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

dependExtra:
	mkdir -p $(BLD_MOD_DIR)

cleanExtra:
	rm -f $(BLD_MOD_DIR)/*.mod 
	rm -f $(BLD_MOD_DIR)/*$(BLD_OBJ)
	rm -f $(BLD_MOD_DIR)/*$(BLD_LIB) 
	rm -f $(BLD_MOD_DIR)/*$(BLD_SHOBJ)

clobberExtra: cleanExtra
	[ "$(BUILD_CROSS)" = 1 ] && rm -fr "./$(BLD_HOST_SYSTEM)" ; true
