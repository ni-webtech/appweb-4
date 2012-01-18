#
#	Makefile for the packages
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep 

PRE_DIRS	= $(BLD_HOST_OS)

dependExtra:
	[ "$(shell echo *.sh)" != "*.sh" ] && chmod +x *.sh ; true

ifeq ($(BUILDING_TARGET),1)
packageFinal:
	$(MAKE) -C .. packageRelease
	if [ `id -u` = 0 ] ; then \
		if logname >/dev/null 2>&1 ; then \
			priv chown -R `logname` $(BLD_OUT_DIR) ; \
		fi ; \
	fi
	$(MAKE) -C .. packageCombo
	$(MAKE) -C .. packageFlat
endif

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
