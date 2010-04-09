#
#	Makefile for Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

BUILD_NATIVE	:= 0

include     .makedep
include		buildConfig.override

SRC			:= ../../../src
PLATFORM	:= Pocket PC 2003 (ARMV4)
MPR_HDR 	= $(BLD_TOP)/buildConfig.h $(shell find $(SRC)/include -name 'mpr*.h')
EJS_HDR 	= $(shell find $(SRC)/include -name 'ejs*.h')
EC_HDR  	= $(shell find $(SRC)/include -name 'ec*.h')
APPWEB_HDR 	= $(SRC)/include/appweb.h $(SRC)/include/http.h
MPR_SRC		= $(SRC)/mpr/mprLib.c
APPWEB_SRC	= $(SRC)/http/handlers/passHandler.c $(shell find $(SRC)/http -name '*.c' | \
				egrep -v 'filters|handlers|modules')
MPR_SRC 	= $(shell find $(SRC)/mpr -name '*.c' | egrep -v 'http.c|makerom.c')
EJS_SRC 	= $(SRC)/ejs/ejsLib.c $(SRC)/ejs/sqlite3.c
EC_SRC 		= $(SRC)/ejs/ecLib.c
APPWEB_SRC 	= $(shell find $(SRC)/http -name '*.c' | egrep -v 'XXXXX.c')

ifeq ($(BUILDING_CROSS),1)
PROJECTS	= libmpr.vcproj libsqlite3.vcproj libmprssl.vcproj libpcre.vcproj libajs.vcproj libac.vcproj \
              libappweb.vcproj mod_auth.vcproj mod_cgi.vcproj mod_chunk.vcproj mod_dir.vcproj mod_egi.vcproj \
              mod_ejs.vcproj mod_file.vcproj mod_php.vcproj mod_range.vcproj mod_ssl.vcproj mod_upload.vcproj \
			  appweb.vcproj 
else
PROJECTS	= ajsmod.vcproj ajsc.vcproj ajs.vcproj ajssql.vcproj ajsweb.vcproj http.vcproj httpPassword.vcproj
endif

#
#   Forced override. The default project doesn't require or support SSL or PHP. This makes it simpler for most folks.
#
BLD_FEATURE_MATRIXSSL = 0
BLD_FEATURE_OPENSSL = 0
BLD_FEATURE_PHP = 0
BLD_FEATURE_STATIC = 0

CROSS_DFLAGS= _WIN32_WCE=\$$(CEVER);UNDER_CE;\$$(PLATFORMDEFINES);\$$(ARCHFAM);\$$(_ARCHFAM_)
CROSS_LIBS	= winsock.lib secchk.lib coredll.lib corelibc.lib ole32.lib oleaut32.lib uuid.lib commctrl.lib
NATIVE_LIBS	= ws2_32.lib

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

compileExtra: prep $(PROJECTS)

prep:
	cp defs/*.def .

libmpr.vcproj: Makefile buildConfig.override
	@makeVsProj --project libmpr.vcproj --library mpr --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def libmpr.def --libs "$(CROSS_LIBS)" $(MPR_HDR) $(SRC)/mpr/mprLib.c
	
libmprssl.vcproj: Makefile buildConfig.override
	@makeVsProj --project libmprssl.vcproj --library mprSsl --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(SRC)/mpr/mprSsl.c
	
libpcre.vcproj: Makefile buildConfig.override
	@makeVsProj --project libpcre.vcproj --library pcre --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def libpcre.def --libs "libmpr.lib $(CROSS_LIBS)" $(MPR_HDR) $(SRC)/mpr/mprPcre.c
	
libsqlite3.vcproj: Makefile buildConfig.override
	@makeVsProj --project libsqlite3.vcproj --library sqlite3 --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def libsqlite3.def --libs "libmpr.lib $(CROSS_LIBS)" $(MPR_HDR) $(SRC)/ejs/sqlite3.c
	
#
#	Ejscript core library
#
libajs.vcproj: Makefile buildConfig.override
	@makeVsProj --project libajs.vcproj --library ajs --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def libajs.def --libs "libmpr.lib libsqlite3.lib $(CROSS_LIBS)" $(MPR_HDR) \
		$(EJS_HDR) $(SRC)/ejs/ejsLib.c
	
#
#	Ejscript compiler library
#
libac.vcproj: Makefile buildConfig.override
	@makeVsProj --project libac.vcproj --library ac --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def libac.def --libs "libmpr.lib libsqlite3.lib libajs.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/ejs/ecLib.c

libappweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project libappweb.vcproj --library appweb --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def libappweb.def --libs "libmpr.lib $(CROSS_LIBS)" $(MPR_HDR) $(EJS_HDR) \
		$(APPWEB_HDR) $(APPWEB_SRC)

#
#	Modules
#
mod_auth.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_auth.vcproj --library auth --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_auth.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/filters/authFilter.c

mod_cgi.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_cgi.vcproj --library cgi --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_cgi.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/handlers/cgiHandler.c

mod_chunk.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_chunk.vcproj --library chunk --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_chunk.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/filters/chunkFilter.c

mod_dir.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_dir.vcproj --library dir --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_dir.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/handlers/dirHandler.c

mod_egi.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_egi.vcproj --library egi --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_egi.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/handlers/egiHandler.c

mod_ejs.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_ejs.vcproj --library ejs --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_ejs.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/ejs/ejsAppweb.c

mod_file.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_file.vcproj --library file --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_file.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/handlers/fileHandler.c

mod_php.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_php.vcproj --library php --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --iflags "$(PHP_IFLAGS)" --def mod_php.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/handlers/phpHandler.c

mod_range.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_range.vcproj --library range --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_range.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/filters/rangeFilter.c

mod_ssl.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_ssl.vcproj --library ssl --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_ssl.def \
		--libs "libmpr.lib libmprssl.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/modules/sslModule.c

mod_upload.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_upload.vcproj --library upload --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --def mod_upload.def \
		--libs "libmpr.lib libsqlite3.lib libajs.lib libappweb.lib $(CROSS_LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/http/filters/uploadFilter.c

#
#	Cross-Executables
#
appweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project appweb.vcproj --exe appweb.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(CROSS_DFLAGS)" \
		--platform "$(PLATFORM)" --libs "libmpr.lib libappweb.lib $(CROSS_LIBS)" $(MPR_HDR) $(EJS_HDR) $(APPWEB_HDR) \
		$(SRC)/server/appweb.c

#
#	Native tools (build statically with source)
#
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
