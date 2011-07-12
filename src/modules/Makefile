#
#	Makefile for the http modules
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

OBJECTS			=

include 		.makedep

ifeq ($(BLD_FEATURE_SSL),1)
	OBJECTS		= $(BLD_OBJ_DIR)/sslModule$(BLD_OBJ)
endif

MAKE_IFLAGS		:= $(BLD_OPENSSL_IFLAGS) $(BLD_MATRIXSSL_IFLAGS) 

compileExtra: $(OBJECTS)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
