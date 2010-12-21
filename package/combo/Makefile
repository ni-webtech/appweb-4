#
#	Makefile to build the combo and flat Appweb distributions
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep


STAGING			:= $(PWD)/staging
TDIR			:= $(STAGING)/appweb-$(BLD_VERSION)
REL_DIR			:= ../../releases
COMBO_VER_NAME	:= appweb-$(BLD_VERSION)-combo.tgz
COMBO_PERM_NAME	:= appweb-combo.tgz
COMBO_VER		:= $(REL_DIR)/$(COMBO_VER_NAME)
COMBO_PERM		:= $(REL_DIR)/$(COMBO_PERM_NAME)
FLAT_VER_NAME	:= appweb-$(BLD_VERSION)-flat.tgz
FLAT_PERM_NAME	:= appweb-flat.tgz
FLAT_VER		:= $(REL_DIR)/$(FLAT_VER_NAME)
FLAT_PERM		:= $(REL_DIR)/$(FLAT_PERM_NAME)
SRC				:= ../../src
JEMS			:= $(SRC)/jems
DEST			:= $(TDIR)/src/deps/appweb
INC_DEST		:= $(TDIR)/src/include
DOC_DEST		:= $(TDIR)/doc
TARGETS			+= $(patsubst %,$(INC_DEST)/%, appweb.h)
TARGETS			+= $(patsubst %,$(DEST)/%, appwebLib.c appweb.c)

APPWEB_HEADERS	+= $(patsubst %,$(SRC)/include/%.h, appweb appwebMonitor)
LIB_SOURCES		+= $(shell find $(SRC) -name '*.c' | egrep -v 'deps|server/|test|utils|sav') 

combo packageExtra:	prep $(TARGETS) $(COMBO_VER) $(FLAT_VER)
	
prep:
	@$(call log) "[Prepare]" "All-in-one combo package"
	rm -fr $(STAGING) $(COMBO_VER) $(COMBO_PERM) $(FLAT_VER) $(FLAT_PERM)
	mkdir -p $(TDIR)/src/include $(TDIR)/src/deps/appweb $(TDIR)/doc/api $(TDIR)/doc/man

$(INC_DEST)/appweb.h: Makefile $(APPWEB_HEADERS)
	echo '#include "appwebConfig.h"' >$(INC_DEST)/appweb.h
	echo '#include "http.h"' >>$(INC_DEST)/appweb.h
	cat $(APPWEB_HEADERS) | egrep -v '#include.*appweb|#include.*http' >>$(INC_DEST)/appweb.h

$(DEST)/appwebLib.c: Makefile $(APPWEB_HEADERS) $(LIB_SOURCES)
	echo '#include "appweb.h"' >$(DEST)/appwebLib.c
	all-in-one $(LIB_SOURCES) | egrep -v '#include.*appweb|#include.*http' >>$(DEST)/appwebLib.c

$(DEST)/appweb.c: Makefile $(APPWEB_HEADERS) $(SRC)/server/appweb.c
	echo '#include "appweb.h"' >$(DEST)/appweb.c
	all-in-one $(SRC)/server/appweb.c | egrep -v '#include.*appweb|#include.*http' >>$(DEST)/appweb.c

$(COMBO_VER):
	cp $(BLD_TOP)/doc/man/appweb.1 $(TDIR)/doc/man
	cp $(BLD_TOP)/doc/man/appwebMonitor.1 $(TDIR)/doc/man
	cp $(BLD_TOP)/doc/man/httpPassword.1 $(TDIR)/doc/man
	@$(call log) "[Generate]" "tar cfz $(COMBO_VER) -C $(STAGING) ."
	tar cfz $(COMBO_VER) -C $(STAGING) .

flat $(FLAT_VER):
	mv $(TDIR)/src/deps/ejs/www $(TDIR)
	find $(TDIR)/src -type f | xargs -I % mv % $(TDIR) 
	cp $(SRC)/include/pcre.h $(SRC)/include/http.h $(SRC)/include/mpr.h $(SRC)/include/mprSsl.h $(TDIR) 
	cp $(SRC)/include/master/*.h $(TDIR) 
	cp $(SRC)/deps/mpr/mprLib.c $(SRC)/deps/mpr/mprSsl.c $(TDIR)
	cp $(SRC)/deps/http/httpLib.c $(SRC)/deps/pcre/pcre.c $(TDIR)
	cp $(SRC)/deps/ejs/*.c $(TDIR)
	rm -fr $(TDIR)/doc $(TDIR)/src $(TDIR)/Makefile
	$(call log) "[Generate]" "tar cfz $(FLAT_VER) -C $(STAGING) ."
	tar cfz $(FLAT_VER) -C $(STAGING) .
	cd $(REL_DIR) ; ln -s $(FLAT_VER_NAME) $(FLAT_PERM_NAME)

cleanExtra:
	rm -fr $(STAGING)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4
#
