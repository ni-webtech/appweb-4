#
#	Makefile for Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

include     .makedep
include		buildConfig.override

SRC			= ../../../src
PLATFORM	= Pocket PC 2003 (ARMV4)
MPR_HDR 	= $(BLD_TOP)/buildConfig.h $(shell find $(SRC)/include -name 'mpr*.h')
EJS_HDR 	= $(shell find $(SRC)/include -name 'ejs*.h')
EC_HDR  	= $(shell find $(SRC)/include -name 'ec*.h')
APP_HDR 	= $(SRC)/include/appweb.h $(SRC)/include/http.h
MPR_SRC 	= $(shell find $(SRC)/mpr -name '*.c' | egrep -v 'http.c|makerom.c')
EJS_SRC 	= $(SRC)/ejs/ejsLib.c $(SRC)/ejs/sqlite3.c
EC_SRC 		= $(SRC)/ejs/ecLib.c
APPWEB_SRC 	= $(shell find $(SRC)/http -name '*.c' | egrep -v 'XXXXX.c')

CROSS_DFLAGS= _WIN32_WCE=\$$(CEVER);UNDER_CE;\$$(PLATFORMDEFINES);\$$(ARCHFAM);\$$(_ARCHFAM_)
CROSS_LIBS	= winsock.lib secchk.lib coredll.lib corelibc.lib ole32.lib oleaut32.lib uuid.lib commctrl.lib
NATIVE_LIBS	= ws2_32.lib

#
#	NOT CURRENTLY USED
#
MAKE_IFLAGS	+= $(BLD_REGEXP_IFLAGS) $(BLD_MPR_IFLAGS)
ifeq ($(BLD_FEATURE_OPENSSL),1)
	SSL_IFLAGS	+= $(BLD_OPENSSL_IFLAGS)
endif
ifeq ($(BLD_FEATURE_MATRIXSSL),1)
	SSL_IFLAGS	+= $(BLD_MATRIXSSL_IFLAGS)
endif
ifeq ($(BLD_FEATURE_PHP),1)
	PHP_IFLAGS	+= $(BLD_PHP_IFLAGS)
endif

ifeq ($(BUILDING_CROSS),1)
compileExtra: appweb.vcproj
else
compileExtra: ajsmod.vcproj ajsc.vcproj ajs.vcproj ajssql.vcproj ajsweb.vcproj http.vcproj httpPassword.vcproj
endif

appweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project appweb.vcproj --exe appweb.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --libs "$(CROSS_LIBS)" $(MPR_HDR) $(EJS_HDR) $(APP_HDR) $(EJS_SRC) $(MPR_SRC) \
		$(APPWEB_SRC) $(SRC)/server/appweb.c $(SRC)/ejs/ejsAppweb.c

ajs.vcproj: Makefile buildConfig.override
	@makeVsProj --project ajs.vcproj --exe ajs.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(NATIVE_DFLAGS)" \
		--libs "$(NATIVE_LIBS)" $(MPR_HDR) $(EJS_HDR) $(EJS_SRC) $(MPR_SRC) $(SRC)/ejs/ejs.c

ajsmod.vcproj: Makefile buildConfig.override
	@makeVsProj --project ajsmod.vcproj --exe ajsmod.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(NATIVE_DFLAGS)" \
		--libs "$(NATIVE_LIBS)" $(MPR_HDR) $(EJS_HDR) $(EC_HDR) $(EC_SRC) $(EJS_SRC) \
		$(MPR_SRC) $(SRC)/ejs/ejsmod.c

ajsc.vcproj: Makefile buildConfig.override
	@makeVsProj --project ajsc.vcproj --exe ajsc.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(NATIVE_DFLAGS)" \
		--custom ajsc.custom --libs "$(NATIVE_LIBS)" $(MPR_HDR) $(EJS_HDR) $(EC_HDR) \
		$(EC_SRC) $(EJS_SRC) $(MPR_SRC) $(SRC)/ejs/ejsc.c

ajssql.vcproj: Makefile buildConfig.override
	@makeVsProj --project ajssql.vcproj --exe ajssql.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(NATIVE_DFLAGS)" \
		--libs "$(NATIVE_LIBS)" $(MPR_HDR) $(MPR_SRC) $(SRC)/ejs/ejssql.c $(SRC)/ejs/sqlite3.c

ajsweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project ajsweb.vcproj --exe ajsweb.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(NATIVE_DFLAGS)" \
		--libs "$(NATIVE_LIBS)" $(MPR_HDR) $(EJS_HDR) $(EC_HDR) $(EC_SRC) $(EJS_SRC) $(MPR_SRC) \
		$(SRC)/ejs/ejsweb.c

http.vcproj: Makefile buildConfig.override
	@makeVsProj --project http.vcproj --exe http.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(NATIVE_DFLAGS)" \
		--libs "$(NATIVE_LIBS)" $(MPR_HDR) $(MPR_SRC) $(SRC)/mpr/http.c

httpPassword.vcproj: Makefile buildConfig.override
	@makeVsProj --project httpPassword.vcproj --exe httpPassword.exe --search "$(BLD_MPR_LIBPATHS)" \
		--dflags "$(NATIVE_DFLAGS)" --libs "$(NATIVE_LIBS)" $(MPR_HDR) $(MPR_SRC) $(SRC)/utils/httpPassword.c
