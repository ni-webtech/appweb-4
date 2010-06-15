#
#	Makefile to build the all-in-one Appweb distribution
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep

SRC				:= ../../src
JEMS			:= $(SRC)/jems
STAGING			:= staging
DEST			:= $(STAGING)/src/deps/appweb
INC_DEST		:= $(STAGING)/src/include
DOC_DEST		:= $(STAGING)/doc
ARCHIVE			+= $(BLD_TOP)/releases/appweb-all.tgz
TARGETS			+= $(patsubst %,$(INC_DEST)/%, appweb.h)
TARGETS			+= $(patsubst %,$(DEST)/%, appwebLib.c appweb.c)

APPWEB_HEADERS	+= $(patsubst %,$(SRC)/include/%.h, appweb appwebMonitor http)
LIB_SOURCES		+= $(shell find $(SRC) -name '*.c' | egrep -v 'deps|server/|test|utils|sav') 

packageExtra:	prep $(TARGETS) $(ARCHIVE)
	
prep:
	rm -fr $(STAGING)
	mkdir -p $(STAGING)/src/include $(STAGING)/src/deps/appweb $(STAGING)/doc/api $(STAGING)/doc/man

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

$(ARCHIVE): Makefile $(TARGETS)
	cp $(BLD_TOP)/doc/man/*.1 $(STAGING)/doc/man
	@$(call log) "[Generate]" "tar cfz $(ARCHIVE) -C $(STAGING) ."
	rm -f $(ARCHIVE)
	tar cfz $(ARCHIVE) -C $(STAGING) .
	rm -fr $(STAGING)

cleanExtra:
	rm -fr $(STAGING)
