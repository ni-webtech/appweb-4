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
INS_DIR			:= c:/tmp/testInsAppweb

install install-%:
	packs=$* ; [ "$$packs" = "" ] && packs="binary dev"; \
	[ "$$packs" = "binary dev" ] && BROWSER=--browser ; \
	priv OPTIONS=$(OPTIONS) QUIET=$(QUIET) TRACE=$(TRACE) $(BLD_TOP)/build/bin/makeInstall \
		$$BROWSER --root=$(ROOT_DIR) --install --with-conf="$(CONF)" --with-testweb pre $$packs post

uninstall uninstall-%:
	packs=$* ; [ "$$packs" = "" ] && packs="binary dev" ; \
	priv OPTIONS=$(OPTIONS) QUIET=$(QUIET) TRACE=$(TRACE) $(BLD_TOP)/build/bin/makeInstall \
		--root=$(ROOT_DIR) --remove --with-testweb pre $$packs post
	
packageExtra:
	export OPTIONS=$(OPTIONS) TRACE=$(TRACE) ; \
	fakePriv "$(BLD_TOP)/build/bin/makeInstall --root=$(PKG_DIR) --with-testweb --package $(PACKS) && \
        $(BLD_TOOLS_DIR)/makePackage --root=$(PKG_DIR) $(PACKS)"

package-only:
	fakePriv $(BLD_TOOLS_DIR)/makePackage --root=$(PKG_DIR) $(PACKS)

test-install-package:
    name="$(BASE_NAME).tar.tar.gz" ; \
    cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
    priv "$(BLD_PRODUCT)_HEADLESS=1 ./install" ; \

test-uninstall-package:
    name="$(BASE_NAME).tar.tar.gz" ; \
    cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
    priv "$(BLD_PRODUCT)_HEADLESS=1 ./uninstall" ; \

test-install-native-package:
	if [ $(BLD_HOST_OS) = WIN ] ; then \
		rm -fr "$(INS_DIR)" ; mkdir -p "$(INS_DIR)" ; \
		name="$(BASE_NAME).exe.zip" ; \
		cp WIN/unattended.ans ../releases/$$name "$(INS_DIR)" ; cd "$(INS_DIR)" ; \
		unzip -q -o $$name ; \
		echo "Dir=$(INS_DIR)" >>unattended.ans ; \
		./$(BASE_NAME).exe /loadinf=/tmp/unattended.ans /silent ; \
	elif [ $(BLD_HOST_OS) = MACOSX ] ; then \
		hdiutil eject /Volumes/$(BLD_PRODUCT)-$(BLD_VERSION) >/dev/null 2>&1 ; \
		hdid $(BLD_TOP)/releases/$(BASE_NAME).dmg >/dev/null ; \
		priv installer -package /Volumes/$(BLD_PRODUCT)-$(BLD_VERSION)/$(BLD_PRODUCT).mpkg -target /; \
		hdiutil eject /Volumes/$(BLD_PRODUCT)-$(BLD_VERSION) >/dev/null 2>&1 ; \
	else \
		name="$(BASE_NAME).tar.tar.gz" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
		priv $(BLD_PRODUCT)_HEADLESS=1 ./install ; \
	fi

test-uninstall-native-package:
	if [ $(BLD_HOST_OS) = WIN ] ; then \
		if [ ! -x "$(INS_DIR)/unins000.exe" ] ; then \
			echo "Can't find $(INS_DIR)/unins000.exe to uninstall" ; \
			exit 255 ; \
		fi ; \
		"$(INS_DIR)/unins000.exe" /silent ; \
		rm -fr "$(INS_DIR)" ; \
	else \
		name="$(BASE_NAME).tar.tar.gz" ; \
		cd $(BLD_TOP); top=`pwd` ; cd /tmp ; tar xvfz $$top/releases/$$name ; \
		priv "$(BLD_PRODUCT)_HEADLESS=1 ./uninstall" ; \
	fi

# SAM SRC
test-installed:
	complete=1 ; \
	for dir in $(BLD_CFG_PREFIX) $(BLD_BIN_PREFIX) $(BLD_DOC_PREFIX) $(BLD_INC_PREFIX) $(BLD_JEM_PREFIX) $(BLD_LIB_PREFIX) $(BLD_LOG_PREFIX) $(BLD_MAN_PREFIX) $(BLD_MOD_PREFIX) $(BLD_PRD_PREFIX) $(BLD_VER_PREFIX) $(BLD_WEB_PREFIX) ; \
	do \
		if [ ! -x $$dir ] ; then \
			echo Missing directory: $$dir ; \
			complete=0 ; \
		fi ; \
	done ; \
	[ $$complete = 0 ] && exit 255 ; \
	port=$(BLD_HTTP_PORT) ; \
	http -q $$port/index.html ; \
	http -q $$port/test/test.php ; \
	http -q $$port/test/test.cgi ; \
	http -q $$port/test/test.pl ; \
	http -q $$port/test/test.py ; \
	echo Installed
	# http -q $$port/test/test.ejs

test-removed:
	partial=0 ; \
	for dir in $(BLD_CFG_PREFIX) $(BLD_BIN_PREFIX) $(BLD_DOC_PREFIX) $(BLD_INC_PREFIX) $(BLD_JEM_PREFIX) $(BLD_LIB_PREFIX) $(BLD_LOG_PREFIX) $(BLD_MAN_PREFIX) $(BLD_MOD_PREFIX) $(BLD_PRD_PREFIX) $(BLD_VER_PREFIX) $(BLD_WEB_PREFIX) ; \
	do \
		if [ -x $$dir ] ; then \
			partial=1 ; \
			echo Directory present: $$dir ; \
		fi ; \
	done ; \
	[ $$partial = 1 ] && exit 255 ; \
	echo Removed

checkInstalled: installed-files

installed-files:
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

dependExtra:
	[ "$(shell echo *.sh)" != "*.sh" ] && chmod +x *.sh ; true
	mkdir -p ../releases
