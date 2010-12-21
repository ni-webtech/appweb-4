#
#	Makefile for the Appweb dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

ifeq ($(BLD_MPR_DIR),)
	PRE_DIRS	+= mpr
endif
PRE_DIRS		+= pcre 
PRE_DIRS		+= http 
ifeq ($(BLD_FEATURE_EJS),1)
ifeq ($(BLD_EJS_DIR),)
	PRE_DIRS	+= ejs 
endif
endif
