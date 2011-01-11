#
#	Makefile for the Appweb dependencies.
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 		.makedep

ifeq ($(BLD_MPR_BUILTIN),1)
	PRE_DIRS    += mpr
endif
ifeq ($(BLD_PCRE_BUILTIN),1)
	PRE_DIRS    += pcre
endif
ifeq ($(BLD_HTTP_BUILTIN),1)
	PRE_DIRS    += http
endif
ifeq ($(BLD_EJS_BUILTIN),1)
	PRE_DIRS    += ejs
endif

clobberExtra:
ifeq ($(BLD_MPR_BUILTIN),1)
	rm -f mpr/*.c mpr/*.h
endif
ifeq ($(BLD_PCRE_BUILTIN),1)
	rm -f pcre/*.c pcre/*.h
endif
ifeq ($(BLD_HTTP_BUILTIN),1)
	rm -f http/*.c http/*.h
endif
ifeq ($(BLD_EJS_BUILTIN),1)
	rm -f ejs/*.c ejs/*.h
endif

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
