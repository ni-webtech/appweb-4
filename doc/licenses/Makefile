#
#	Makefile -- for licenses
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include		.makedep

compileExtra: LICENSE.TXT

LICENSE.TXT: 
ifeq ($(BLD_LICENSE),commercial)
	cp eval.txt LICENSE.TXT
else
	cp gpl.txt LICENSE.TXT
endif

cleanExtra:
	rm -f LICENSE.TXT

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
