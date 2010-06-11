#
#	Makefile for the Appweb library
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

MAKE_IFLAGS		+= $(BLD_APPWEB_IFLAGS)
MAKE_IFLAGS		+= $(BLD_OPENSSL_IFLAGS) $(BLD_MATRIXSSL_IFLAGS)

TARGETS			+= $(BLD_LIB_DIR)/ejsweb.conf
TARGETS			+= $(BLD_LIB_DIR)/start.es
# MODULES		+= $(BLD_BIN_DIR)/ejsweb$(BLD_EXE)
ifeq ($(BLD_UNIX_LIKE),1)
    TARGETS		+= $(BLD_BIN_DIR)/angel$(BLD_EXE)
endif
TARGETS			+= $(BLD_LIB_DIR)/mime.types
WEB_OBJECTS		+= $(patsubst %,$(BLD_OBJ_DIR)/%$(BLD_OBJ),appweb appwebLib)

compileExtra:	$(TARGETS)

# modules:		$(MODULES)

$(BLD_BIN_DIR)/ejsweb$(BLD_EXE): $(OBJECTS)
	bld --exe $(BLD_BIN_DIR)/ejsweb$(BLD_EXE) --libs "$(BLD_EJS_LIBS)" \
		--modules "ejs.web$(BLD_SHOBJ)" $(WEB_OBJECTS)

$(BLD_BIN_DIR)/angel$(BLD_EXE): $(OBJECTS) $(patsubst %,$(BLD_OBJ_DIR)/%$(BLD_OBJ),angel)
	bld --exe $(BLD_BIN_DIR)/angel$(BLD_EXE) --libs "$(BLD_MPR_LIBS)" angel$(BLD_OBJ)

romFiles.c: ../mime.types ../ejsweb.conf default-web/*.html
	$(call log) "[Generate]" "romFiles.c"
	echo -e ".\nejsweb.conf\nmime.types" >rom.files
	find default-web -print | egrep -v 'CVS|\.svn|\.cvsignore|\.ignore|\.tmp|\.sav|\.php|\.ejs|php' >>rom.files
	$(call setlibpath) ; sort rom.files | uniq | xargs makerom --name romFiles >romFiles.c

$(BLD_LIB_DIR)/mime.types: mime.types
	$(call log) "[Generate]" "mime.types"
	rm -f $(BLD_LIB_DIR)/mime.types
	cp mime.types $(BLD_LIB_DIR)

$(BLD_LIB_DIR)/ejsweb.conf: ejsweb.conf
	$(call log) "[Generate]" "ejsweb.conf"
	cp ejsweb.conf $(BLD_LIB_DIR)

$(BLD_LIB_DIR)/start.es: start.es
	$(call log) "[Generate]" "start.es"
	cp start.es $(BLD_LIB_DIR)
