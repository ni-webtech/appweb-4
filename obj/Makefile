#
#	Makefile -- for the obj directory
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include		.makedep

dependExtra:
	mkdir -p $(BLD_OBJ_DIR)

cleanExtra:
	rm -f *.obj *.o */*.obj */*.o

clobberExtra: 
	[ "$(BUILD_CROSS)" = 1 ] && rm -fr "./$(BLD_HOST_SYSTEM)" ; true
