#
#	Makefile for Ejscript
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#	This is an all-in-one build where all the Ejscript source files are catenated into as few files as possible. 
#	See http://hg.embedthis.com/ejs if you require the full source for the Ejscript.
#

include 		.makedep

#
#   Cygwin can't handle -g for sqlite3
#
ifeq ($(BLD_HOST_OS),CYGWIN)
_CFLAGS 		:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/;s/-g3//')
endif

ESFLAGS 			+= $(_ESFLAGS)

ifeq ($(BLD_EJS_DIR),)
	EJS_OBJECTS		+= ejsLib
	ifeq ($(BLD_FEATURE_SQLITE),1)
		TARGETS		+= $(BLD_LIB_DIR)/libsqlite3$(BLD_LIB)
	endif
	TARGETS			+= $(BLD_LIB_DIR)/libejs$(BLD_LIB)

	ifneq ($(BUILDING_CROSS),1)
		BUILD_COMPILER=1
	endif
	ifeq ($(BUILD_NATIVE_OR_COMPLETE_CROSS),1)
		BUILD_COMPILER=1
	endif
	ifeq ($(BLD_FEATURE_EJS_CROSS_COMPILER),1)
		BUILD_COMPILER=1
	endif

	TARGETS			+= $(BLD_BIN_DIR)/ejs$(BLD_EXE)

	ifeq ($(BUILD_COMPILER),1)
		TARGETS		+= $(BLD_BIN_DIR)/ejsc$(BLD_EXE)
		TOOLS		+= $(BLD_BIN_DIR)/ejsc$(BLD_EXE)
	endif

	ifeq ($(BUILD_NATIVE_OR_COMPLETE_CROSS),1)
		TARGETS		+= $(BLD_BIN_DIR)/ejsmod$(BLD_EXE)
		TOOLS		+= $(BLD_BIN_DIR)/ejsmod$(BLD_EXE)
		ifeq ($(BLD_FEATURE_SQLITE),1)
			TARGETS	+= $(BLD_BIN_DIR)/ejssql$(BLD_EXE)
		endif
	endif

	TARGETS		+= $(BLD_MOD_DIR)/ejs.mod
	ifeq ($(BLD_FEATURE_STATIC),1) 
		TARGETS	+= $(BLD_LIB_DIR)/libejs$(BLD_LIB)
	endif
	# MODULES += $(BLD_MOD_DIR)/mod_ejs$(BLD_SHOBJ)
	GATE_OBJECTS += ejsAppwebHandler$(BLD_OBJ)
	ifeq ($(BLD_OS),VXWORKS)
	ifeq ($(BLD_FEATURE_STATIC),0)
		EXTRA_GATEWAY_OBJECTS += $(BLD_LIB_DIR)/libejs.out
		ifeq ($(BLD_FEATURE_SQLITE),1)
			EXTRA_GATEWAY_OBJECTS += $(BLD_LIB_DIR)/libsqlite3.out
		endif
	endif
	endif
	TARGETS		+= $(BLD_LIB_DIR)/www
	ifeq ($(BLD_HOST_OS),WIN)
		TARGETS	+= $(BLD_BIN_DIR)/ejsmvc.cmd
		TARGETS	+= $(BLD_BIN_DIR)/ejspage.cmd
		TARGETS	+= $(BLD_BIN_DIR)/jem.cmd
	endif
endif
ifeq	($(BLD_CC_EDITLINE),1)
	EJS_LIBS	+= edit curses
endif

#	TODO - remove this
DOC_DIR		:= $(BLD_TOP)/doc/api/ejscript

#
#	Suppress warnings for sqlite
#
_CFLAGS 	:= $(shell echo $(_CFLAGS) | sed 's/-W3/-W1/;s/-Wall/-w/')

compileExtra: $(TARGETS)

#
#	Modules can only be built after the appweb library is built. May build twice if the slots change.
#
modules: $(MODULES) dummy

$(BLD_MOD_DIR)/ejs.mod: $(BLD_INC_DIR)/ejs.slots.h *.es $(TOOLS)
	$(call log) "[Generate]" ejs.mod
	$(call setlibpath) ; \
		ejsc $(ESFLAGS) --bind --require "" --out $(BLD_MOD_DIR)/ejs.mod ejs.es
	$(call setlibpath) ; ejsmod --showDebug --cslots --require "" --out .ejs.slots.h $(BLD_MOD_DIR)/ejs.mod \
		|| rm -f $(BLD_MOD_DIR)/ejs.mod
	if [ "`grep CHECKSUM .ejs.slots.h | sum`" != "`grep CHECKSUM $(BLD_INC_DIR)/ejs.slots.h | sum`" ] ; then \
		chmod +w $(BLD_INC_DIR)/ejs.slots.h ; \
		cp .ejs.slots.h $(BLD_INC_DIR)/ejs.slots.h ; \
		touch .ejs.slots.h ; \
		if [ "$(RETRY)" = "" ] ; then \
			make TRACE=$(TRACE) RETRY=1 compile; [ $$? != 0 ] && exit 255 ; true ; \
		fi ; \
	else true ; \
	fi

#
#	Build the ejs, ejsc, and sqlite libraries
#	
$(BLD_LIB_DIR)/libejs$(BLD_LIB): $(OBJECTS) $(call lib, mpr)
	bld --library $(BLD_LIB_DIR)/libejs --search "$(BLD_EJS_WITHPATHS)" \
		--libs "mpr http $(BLD_EJS_WITHLIBS)" $(EJS_OBJECTS)

$(BLD_LIB_DIR)/libsqlite3$(BLD_LIB): $(OBJECTS) $(call lib, mpr)
	bld --library $(BLD_LIB_DIR)/libsqlite3 --search "$(BLD_SQLITE_WITHPATHS)" --libs "$(BLD_SQLITE_WITHLIBS)" \
		sqlite3$(BLD_OBJ)

$(BLD_BIN_DIR)/ejsc$(BLD_EXE): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --exe $(BLD_BIN_DIR)/ejsc$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS)" ejsc$(BLD_OBJ)

$(BLD_BIN_DIR)/ejs$(BLD_EXE): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --exe $(BLD_BIN_DIR)/ejs$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS) $(EJS_LIBS)" ejs$(BLD_OBJ)

$(BLD_BIN_DIR)/ejsmod$(BLD_EXE): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --exe $(BLD_BIN_DIR)/ejsmod$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS)" ejsmod$(BLD_OBJ)

$(BLD_BIN_DIR)/ejssql$(BLD_EXE): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
	bld --exe $(BLD_BIN_DIR)/ejssql$(BLD_EXE) --search "$(BLD_EJS_LIBPATHS)" --libs "$(BLD_EJS_LIBS)" ejssql$(BLD_OBJ)

# $(BLD_MOD_DIR)/mod_ejs$(BLD_SHOBJ): $(OBJECTS) $(call lib, mpr) $(call vlib, ejs)
# 	bld --shared --library $(BLD_MOD_DIR)/mod_ejs --search "$(BLD_APPWEB_LIBPATHS)" \
# 		--libs "$(BLD_APPWEB_LIBS) $(BLD_EJS_LIBS)" ejsAppwebHandler$(BLD_OBJ) $(EXTRA_GATEWAY_OBJECTS)

$(BLD_LIB_DIR)/www: Makefile www/*
	mkdir -p $(BLD_LIB_DIR)/www/images
	mkdir -p $(BLD_LIB_DIR)/www/themes
	cp -r www/* $(BLD_LIB_DIR)/www

$(BLD_BIN_DIR)/ejsmvc.cmd: ejsmvc.cmd
	$(call log) "[Generate]" "ejsmvc.cmd"
	cp ejsmvc.cmd $(BLD_BIN_DIR)/ejsmvc.cmd
	chmod 755 $(BLD_BIN_DIR)/ejsmvc.cmd

$(BLD_BIN_DIR)/ejspage.cmd: ejspage.cmd
	$(call log) "[Generate]" "ejspage.cmd"
	cp ejspage.cmd $(BLD_BIN_DIR)/ejspage.cmd
	chmod 755 $(BLD_BIN_DIR)/ejspage.cmd

$(BLD_BIN_DIR)/jem.cmd: jem.cmd
	$(call log) "[Generate]" "jem.cmd"
	cp jem.cmd $(BLD_BIN_DIR)/jem.cmd
	chmod 755 $(BLD_BIN_DIR)/jem.cmd

cleanExtra:
	rm -fr $(BLD_LIB_DIR)/www
