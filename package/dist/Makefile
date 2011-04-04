#
#	Makefile to build Appweb distributions
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep


STAGE_DIR		:= $(BLD_OUT_DIR)/dist
REL_DIR			:= $(BLD_OUT_DIR)/releases
TDIR			:= $(STAGE_DIR)/appweb-$(BLD_VERSION)
DIST_VER_NAME	:= appweb-$(BLD_VERSION)-dist.tgz
DIST_PERM_NAME	:= appweb-dist.tgz
DIST_VER		:= $(REL_DIR)/$(DIST_VER_NAME)
DIST_PERM		:= $(REL_DIR)/$(DIST_PERM_NAME)
FLAT_VER_NAME	:= appweb-$(BLD_VERSION)-flat.tgz
FLAT_PERM_NAME	:= appweb-flat.tgz
FLAT_VER		:= $(REL_DIR)/$(FLAT_VER_NAME)
FLAT_PERM		:= $(REL_DIR)/$(FLAT_PERM_NAME)
SRC				:= ../../src
JEMS			:= $(SRC)/jems
DEST			:= $(TDIR)/src/deps/appweb
# INC_DEST		:= $(TDIR)/src/include
INC_DEST		:= $(DEST)
DOC_DEST		:= $(TDIR)/doc
TARGETS			+= $(patsubst %,$(INC_DEST)/%, appweb.h)
TARGETS			+= $(patsubst %,$(DEST)/%, appwebLib.c appweb.c)

APPWEB_HEADERS	+= $(patsubst %,$(SRC)/include/%.h, appweb appwebMonitor)
LIB_SOURCES		+= $(shell find $(SRC) -name '*.c' | egrep -v 'deps|server/|test|utils|sav') 

dist packageExtra:	clean prep $(TARGETS) $(DIST_VER) $(FLAT_VER)
	
prep:
	@$(call log) "[Prepare]" "Package directories"
	mkdir -p $(STAGE_DIR) $(REL_DIR) $(TDIR)/src/include $(TDIR)/src/deps/appweb $(TDIR)/doc/api $(TDIR)/doc/man

$(INC_DEST)/appweb.h: Makefile $(APPWEB_HEADERS)
	echo '#include "appwebConfig.h"' >$(INC_DEST)/appweb.h
	echo '#include "http.h"' >>$(INC_DEST)/appweb.h
	cat $(APPWEB_HEADERS) | egrep -v '#include.*appweb|#include.*http' >>$(INC_DEST)/appweb.h

$(DEST)/appwebLib.c: Makefile $(APPWEB_HEADERS) $(LIB_SOURCES)
	echo '#include "appweb.h"' >$(DEST)/appwebLib.c
	dist $(LIB_SOURCES) | egrep -v '#include.*appweb|#include.*http' >>$(DEST)/appwebLib.c

$(DEST)/appweb.c: Makefile $(APPWEB_HEADERS) $(SRC)/server/appweb.c
	echo '#include "appweb.h"' >$(DEST)/appweb.c
	dist $(SRC)/server/appweb.c | egrep -v '#include.*appweb|#include.*http' >>$(DEST)/appweb.c

$(DIST_VER):
	cp $(BLD_TOP)/doc/man/appweb.1 $(TDIR)/doc/man
	cp $(BLD_TOP)/doc/man/appwebMonitor.1 $(TDIR)/doc/man
	cp $(BLD_TOP)/doc/man/auth.1 $(TDIR)/doc/man
	@$(call log) "[Generate]" "tar cfz $(DIST_VER) -C $(STAGE_DIR) ."
	tar cfz $(DIST_VER) -C $(STAGE_DIR) .
	cd $(REL_DIR) ; ln -s $(DIST_VER_NAME) $(DIST_PERM_NAME)

flat $(FLAT_VER):
	cp -r $(BLD_LIB_DIR)/www $(TDIR)
	find $(TDIR)/src -type f | xargs -I % mv % $(TDIR) 
	cp $(SRC)/deps/pcre/pcre.h $(SRC)/deps/http/http.h $(SRC)/deps/mpr/mpr.h $(SRC)/deps/mpr/mprSsl.h $(TDIR) 
	cp $(SRC)/include/master/*.h $(TDIR) 
	cp $(SRC)/deps/mpr/mprLib.c $(SRC)/deps/mpr/mprSsl.c $(TDIR)
	cp $(SRC)/deps/http/httpLib.c $(SRC)/deps/pcre/pcre.c $(TDIR)
	cp $(SRC)/deps/ejs/*.c $(TDIR)
	rm -fr $(TDIR)/doc $(TDIR)/src $(TDIR)/Makefile
	$(call log) "[Generate]" "tar cfz $(FLAT_VER) -C $(STAGE_DIR) ."
	tar cfz $(FLAT_VER) -C $(STAGE_DIR) .
	cd $(REL_DIR) ; ln -s $(FLAT_VER_NAME) $(FLAT_PERM_NAME)

cleanup:
	rm -fr $(STAGE_DIR)

cleanExtra:
	rm -fr $(STAGE_DIR) $(REL_DIR)/$(DIST_VER_NAME) $(REL_DIR)/$(DIST_PERM_NAME)
	rm -fr $(REL_DIR)/$(FLAT_VER_NAME) $(REL_DIR)/$(FLAT_PERM_NAME)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
