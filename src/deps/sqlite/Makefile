#
#	Makefile to build SQLite
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 			.makedep

ifeq ($(BLD_FEATURE_SQLITE),1)
ifeq ($(BLD_SQLITE_BUILTIN),1)
	TARGETS		+= $(BLD_LIB_DIR)/libsqlite3$(BLD_LIB)
	TARGETS		+= $(BLD_BIN_DIR)/sqlite$(BLD_EXE)
endif
endif

MAKE_IFLAGS		+= $(BLD_SQLITE_IFLAGS) $(BLD_MPR_IFLAGS) $(BLD_HTTP_IFLAGS)
ifeq ($(BLD_HOST_WIN),1)
	MAKE_DFLAGS	+= -D_CRT_SECURE_NO_WARNINGS
endif
ifeq ($(BLD_HOST_OS),CYGWIN)
	_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/;s/-g3//')
else
	_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/')
endif

compileExtra:   $(TARGETS)

$(BLD_LIB_DIR)/libsqlite3$(BLD_LIB): $(BLD_OBJ_DIR)/sqlite3$(BLD_OBJ)
	bld --library $(BLD_LIB_DIR)/libsqlite3 --libs "$(BLD_MPR_LIBS)" sqlite3

$(BLD_BIN_DIR)/sqlite$(BLD_EXE): $(BLD_LIB_DIR)/libsqlite3$(BLD_LIB)
	bld --exe $(BLD_BIN_DIR)/sqlite$(BLD_EXE) --libs "$(BLD_MPR_LIBS) sqlite3" sqlite

dependExtra: $(BLD_INC_DIR)/sqlite3.h

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
