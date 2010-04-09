#
#	Makefile for the packages
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include 		.makedep 

PRE_DIRS		= $(BLD_HOST_OS)
BASE_NAME		:= "$(BLD_PRODUCT)-$(BLD_VERSION)-$(BLD_NUMBER)-$(BLD_HOST_DIST)-$(BLD_HOST_OS)-$(BLD_HOST_CPU)"
PACKS			:= binary dev source
PKG_DIR			:= `pwd`/../staging
WIN_INS_DIR		:= c:/tmp/testInsAppweb

install install-%: prep
	packs=$* ; [ "$$packs" = "" ] && packs="binary dev"; \
	[ "$$packs" = "binary dev" ] && BROWSER=--browser ; \
	priv OPTIONS=$(OPTIONS) QUIET=$(QUIET) TRACE=$(TRACE) $(BLD_TOP)/build/bin/makeInstall \
	$$BROWSER --root=$(ROOT_DIR) --install --with-conf="$(CONF)" --with-testweb pre $$packs post

uninstall uninstall-%: prep
	packs=$* ; [ "$$packs" = "" ] && packs="binary dev" ; \
	priv OPTIONS=$(OPTIONS) QUIET=$(QUIET) TRACE=$(TRACE) $(BLD_TOP)/build/bin/makeInstall \
	--root=$(ROOT_DIR) --remove --with-testweb pre $$packs post
	
packageExtra:
	export OPTIONS=$(OPTIONS) TRACE=$(TRACE) ; \
	fakePriv "$(BLD_TOP)/build/bin/makeInstall --root=$(PKG_DIR) --with-testweb --package $(PACKS) && \
        $(BLD_TOOLS_DIR)/makePackage --root=$(PKG_DIR) $(PACKS)"

packageOnly:
	fakePriv $(BLD_TOOLS_DIR)/makePackage --root=$(PKG_DIR) $(PACKS)

install-package: prep
	if [ $(BLD_HOST_OS) = WIN ] ; then \
		name="$(BASE_NAME).exe.zip" ; \
		rm -fr "$(WIN_INS_DIR)" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; unzip -o $$top/releases/$(BLD_PRODUCT)/$$name ; \
		name="${BLD_PRODUCT}-${BLD_VERSION}-${BLD_NUMBER}-${BLD_HOST_OS}-${BLD_HOST_CPU}.exe" ; \
		cp $$top/package/WIN/unattended.ans /tmp ; \
		echo "Dir=$(WIN_INS_DIR)" >>/tmp/unattended.ans ; \
		$$name /loadinf=/tmp/unattended.ans /silent ; \
	else \
		name="$(BASE_NAME).tar.tar.gz" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
		priv "$(BLD_PRODUCT)_HEADLESS=1 ./install" ; \
	fi

uninstall-package: prep
	if [ $(BLD_HOST_OS) = WIN ] ; then \
		if [ ! -x "$(WIN_INS_DIR)/unins000.exe" ] ; then \
			echo "Can't find $(WIN_INS_DIR)/unins000.exe to uninstall" ; \
			exit 255 ; \
		fi ; \
		"$(WIN_INS_DIR)/unins000.exe" /silent ; \
	else \
		name="$(BASE_NAME).tar.tar.gz" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
		priv "$(BLD_PRODUCT)_HEADLESS=1 ./uninstall" ; \
	fi

install-native-package: prep
	if [ $(BLD_HOST_OS) = WIN ] ; then \
		name="$(BASE_NAME).exe.zip" ; \
		rm -fr "$(WIN_INS_DIR)" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; unzip -o $$top/releases/$(BLD_PRODUCT)/$$name ; \
		name="${BLD_PRODUCT}-${BLD_VERSION}-${BLD_NUMBER}-${BLD_HOST_OS}-${BLD_HOST_CPU}.exe" ; \
		cp $$top/package/WIN/unattended.ans /tmp ; \
		echo "Dir=$(WIN_INS_DIR)" >>/tmp/unattended.ans ; \
		$$name /loadinf=/tmp/unattended.ans /silent ; \
	elif [ $(BLD_HOST_OS) = MACOSX ] ; then \
		hdiutil eject /Volumes/$(BLD_PRODUCT)-$(BLD_VERSION) >/dev/null ; \
		hdid $(BLD_TOP)/releases/$(BASE_NAME).dmg >/dev/null ; \
		priv installer -package /Volumes/$(BLD_PRODUCT)-$(BLD_VERSION)/$(BLD_PRODUCT).mpkg -target /; \
		hdiutil eject /Volumes/$(BLD_PRODUCT)-$(BLD_VERSION) >/dev/null ; \
	else \
		name="$(BASE_NAME).tar.tar.gz" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
		priv $(BLD_PRODUCT)_HEADLESS=1 ./install ; \
	fi

uninstall-native-package: prep
	if [ $(BLD_HOST_OS) = WIN ] ; then \
		if [ ! -x "$(WIN_INS_DIR)/unins000.exe" ] ; then \
			echo "Can't find $(WIN_INS_DIR)/unins000.exe to uninstall" ; \
			exit 255 ; \
		fi ; \
		"$(WIN_INS_DIR)/unins000.exe" /silent ; \
	else \
		name="$(BASE_NAME).tar.tar.gz" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
		priv "$(BLD_PRODUCT)_HEADLESS=1 ./uninstall" ; \
	fi

checkInstalled:
	( \
		for d in /etc /usr/src /usr/share /usr/share/doc /usr/lib /usr/lib/$(BLD_PRODUCT) /usr/include \
			/usr/local/bin /usr/local/bin/$(BLD_PRODUCT) /usr/bin /usr/bin/$(BLD_PRODUCT) \
			/usr/include/$(BLD_PRODUCT) /var/log /var/www ; \
		do \
			if [ "`echo $$d/$(BLD_PRODUCT)*`" != $$d/$(BLD_PRODUCT)'*' ] ; then \
				eval echo "$$d/$(BLD_PRODUCT)*" ; \
				continue ; \
			fi ; \
			if [ -x "$$d/$(BLD_PRODUCT)" ] ; then \
				echo "$$d/$(BLD_PRODUCT)" ; \
				continue ; \
			fi ; \
		done ; \
		for d in "$(BLD_CFG_PREFIX)" "$(BLD_DOC_PREFIX)" "$(BLD_INC_PREFIX)" "$(BLD_LIB_PREFIX)" "$(BLD_LOG_PREFIX)" \
			"$(BLD_SAM_PREFIX)" "$(BLD_SRC_PREFIX)" ; \
		do \
			[ -x "$$d" ] && echo "$$d" || true; \
		done ; \
	) | sort | uniq

prep:
	if [ ! -x $(BLD_TOOLS_DIR)/edep$(BLD_BUILD_EXE) -a "$(BUILDING_CROSS)" != 1 ] ; then \
		$(MAKE) -S --no-print-directory _RECURSIVE_=1 -C $(BLD_TOP)/build/src compile ; \
	fi

dependExtra:
	[ "$(shell echo *.sh)" != "*.sh" ] && chmod +x *.sh ; true
	mkdir -p ../releases
