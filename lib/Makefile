#
#  Makefile -- Libraries Makefile
#
# Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 	.makedep

dependExtra:
	mkdir -p $(BLD_LIB_DIR)

cleanExtra:
	rm -f *.o *.so *.dylib *.a *.dll *.lib
	rm -fr default-web *.conf mime.types
	rm -f $(BLD_LIB_DIR)/*.o $(BLD_LIB_DIR)/*.so $(BLD_LIB_DIR)/*.dylib $(BLD_LIB_DIR)/*.a 
	rm -f $(BLD_LIB_DIR)/*.dll $(BLD_LIB_DIR)/*.lib
	rm -f $(BLD_LIB_DIR)/*.conf $(BLD_LIB_DIR)/*.types
	rm -fr $(BLD_LIB_DIR)/default-web 

clobberExtra: cleanExtra
	[ "$(BUILD_CROSS)" = 1 ] && rm -fr "./$(BLD_HOST_SYSTEM)" ; true
