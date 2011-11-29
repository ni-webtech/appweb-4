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
	if [ `id -u` = 0 ] ; then \
		if logname >/dev/null 2>&1 ; then \
			priv chown -R `logname` $(BLD_OUT_DIR) ; \
		fi ; \
	fi
	$(MAKE) -C .. packageCombo
	$(MAKE) -C .. packageFlat

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
