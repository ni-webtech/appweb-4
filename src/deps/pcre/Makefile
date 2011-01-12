#
#	Makefile for the PCRE - Regular Expression Library (based on pcre)
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#
#	This is an all-in-one build where all the re source files are catenated into as few files as possible. 
#

include 			.makedep

CHAR				= char
ifeq ($(BLD_CHAR_LEN),2)
	CHAR			= short
endif
ifeq ($(BLD_CHAR_LEN),4)
	CHAR			= int32
endif
MAKE_DFLAGS			+= -DPCRE_CHAR=$(CHAR)
MAKE_IFLAGS			+= $(BLD_PCRE_IFLAGS) $(BLD_MPR_IFLAGS)
PCRE_LIB			+= $(BLD_LIB_DIR)/libpcre$(BLD_LIB)
ifeq ($(BLD_FEATURE_PCRE),1)
	TARGETS			+= $(PCRE_LIB)
endif

compileTargets: $(TARGETS)

#
#	Regular Expression (pcre) library
#
$(PCRE_LIB): $(OBJECTS)
	bld --library $(PCRE_LIB) pcre
