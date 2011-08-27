#
#	Makefile for the Appweb dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

#
#	If building with EJS, must use ejs versions of libmpr, libhttp, libpcre
#
ifeq ($(BLD_FEATURE_EJSCRIPT),0)
	ifeq ($(BLD_MPR_BUILTIN),1)
		PRE_DIRS	+= mpr
	endif
	ifeq ($(BLD_PCRE_BUILTIN),1)
		PRE_DIRS	+= pcre
	endif
	ifeq ($(BLD_HTTP_BUILTIN),1)
		PRE_DIRS	+= http
	endif
endif

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
