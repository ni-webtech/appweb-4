#
#	Makefile for src/include
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

HEADERS			:= ejs.slots.h

include 		.makedep

compileExtra: $(HEADERS)

#
#   Update buildConfig.h version information. This is used in windows packages for building with VS.
#
dependExtra:
	for f in master/*slots* ; do \
		cp $$f . ; \
	done
ifeq ($(BLD_OS),WIN)
	cat master/buildConfig.h | \
        egrep -v 'BLD_VERSION|BLD_NUMBER|BLD_.*_VERSION|BLD_VNUM' | grep -v BLD_CC_CL_VERSION >versions.h
	cat buildConfig.h | egrep 'BLD_VERSION|BLD_NUMBER|BLD_.*_VERSION|BLD_VNUM' | grep -v BLD_CC_CL_VERSION >>versions.h
	mv versions.h master/buildConfig.h
endif

cleanExtra: 
	rm -f $(HEADERS)

%.h: master/%.h
	cp $< .
