#
#  Makefile -- Binaries Makefile
#
# Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

dependExtra:
	mkdir -p $(BLD_BIN_DIR)

cleanExtra:
	rm -f angel* cgi* http* makerom* testAppweb* appweb* ajs* *.out *.def *.exe *.lib *.dylib *.a *.so *.mod
	rm -f $(BLD_BIN_DIR)/ajs* $(BLD_BIN_DIR)/ajs* 
	rm -f $(BLD_BIN_DIR)/*.def $(BLD_BIN_DIR)/*.out $(BLD_BIN_DIR)/*.exe $(BLD_BIN_DIR)/*.lib 
	rm -f $(BLD_BIN_DIR)/*.dylib $(BLD_BIN_DIR)/*.a $(BLD_BIN_DIR)/*.so $(BLD_BIN_DIR)/*.mod
	[ "$(BUILD_CROSS)" = 1 ] && rm -f "./$(BLD_HOST_SYSTEM)/*" ; true

clobberExtra: cleanExtra
	[ "$(BUILD_CROSS)" = 1 ] && rm -fr "./$(BLD_HOST_SYSTEM)" ; true
