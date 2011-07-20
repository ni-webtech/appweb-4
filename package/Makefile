#
#	Makefile for the packages
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep 

PRE_DIRS	= $(BLD_HOST_OS) all

dependExtra:
	[ "$(shell echo *.sh)" != "*.sh" ] && chmod +x *.sh ; true

packageFinal:
	$(MAKE) -C .. packageRelease
	$(MAKE) -C .. packageCombo


#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
