#
#	Makefile for the PCRE - Regular Expression Library (based on pcre)
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#
#	This is an all-in-one build where all the re source files are catenated into as few files as possible. 
#	See http://hg.embedthis.com/re if you require the full source for the RE.
#

include 			.makedep

PCRE_LIB			+= $(BLD_LIB_DIR)/libpcre$(BLD_LIB)

MAKE_IFLAGS			+= $(BLD_REGEXP_IFLAGS) $(BLD_MPR_IFLAGS)
ifeq ($(BLD_FEATURE_REGEXP),1)
	TARGETS			+= $(PCRE_LIB)
endif

compileTargets: $(TARGETS)

#
#	Regular Expression (pcre) library
#
$(PCRE_LIB): $(OBJECTS)
	bld --library $(PCRE_LIB) pcre
