#
#	Makefile for Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

include     .makedep

# include     buildConfig.override

SRC			= ../../../src
MPR_HDR 	= $(BLD_TOP)/buildConfig.h $(shell find $(SRC)/include -name 'mpr*.h')
EJS_HDR 	= $(shell find $(SRC)/include -name 'ejs*.h')
HTTP_HDR 	= $(shell find $(SRC)/include -name 'http*.h')
APPWEB_HDR 	= $(SRC)/include/appweb.h
MPR_SRC		= $(SRC)/deps/mpr/mprLib.c
APPWEB_SRC	= $(shell find $(SRC) -name '*.c' | egrep -v 'deps|test|utils' | egrep -v 'server/|filters|handlers|modules') \
              $(SRC)/handlers/dirHandler.c $(SRC)/handlers/egiHandler.c $(SRC)/handlers/fileHandler.c
LIBS		= ws2_32.lib
PROJECTS	= libmpr.vcproj libsqlite3.vcproj libmprssl.vcproj libpcre.vcproj libhttp.vcproj libejs.vcproj \
              libappweb.vcproj appweb.vcproj angel.vcproj appwebMonitor.vcproj ejsmod.vcproj ejsc.vcproj ejs.vcproj \
              ejssql.vcproj ejsweb.vcproj http.vcproj httpPassword.vcproj \
			  mod_cgi.vcproj mod_ejs.vcproj mod_php.vcproj mod_ssl.vcproj

MAKE_IFLAGS	= -I$(BLD_TOP)/src/include/master $(BLD_REGEXP_IFLAGS) $(BLD_MPR_IFLAGS)
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
	if ls ../../../bin/*.def >/dev/null 2>&1 ; then \
		cp ../../../bin/*.def . ; \
	fi

libmpr.vcproj: Makefile buildConfig.override
	@makeVsProj --project libmpr.vcproj --library mpr --dflags "$(DFLAGS)" \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(MPR_SRC)
	
libmprssl.vcproj: Makefile buildConfig.override
	@makeVsProj --project libmprssl.vcproj --library mprssl --dflags "$(DFLAGS)" \
		--iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(LIBS)" \
		$(MPR_HDR) $(SRC)/deps/mpr/mprSsl.c
	
libpcre.vcproj: Makefile buildConfig.override
	@makeVsProj --project libpcre.vcproj --library pcre --dflags "$(DFLAGS)" \
		--def libpcre.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(SRC)/deps/pcre/pcre.c
	
libsqlite3.vcproj: Makefile buildConfig.override
	@makeVsProj --project libsqlite3.vcproj --library sqlite3 --dflags "$(DFLAGS)" \
		--def libsqlite3.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(SRC)/deps/ejs/sqlite3.c
	
libejs.vcproj: Makefile buildConfig.override
	@makeVsProj --project libejs.vcproj --library ejs --dflags "$(DFLAGS)" \
		--def libejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) \
        $(SRC)/deps/ejs/ejsLib.c
	
libhttp.vcproj: Makefile buildConfig.override
	@makeVsProj --project libhttp.vcproj --library http --dflags "$(DFLAGS)" \
		--def libhttp.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(HTTP_HDR) $(SRC)/deps/http/httpLib.c
	
libappweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project libappweb.vcproj --library appweb --dflags "$(DFLAGS)" \
		--def libappweb.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(APPWEB_HDR) $(APPWEB_SRC)

#
#	Modules
#
mod_cgi.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_cgi.vcproj --library cgi --dflags "$(DFLAGS)" \
		--def mod_cgi.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libejs.lib libappweb.lib $(LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/handlers/cgiHandler.c

mod_ejs.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_ejs.vcproj --library ejs --dflags "$(DFLAGS)" \
		--def mod_ejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libejs.lib libappweb.lib $(LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/handlers/ejsHandler.c

mod_php.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_php.vcproj --library php --dflags "$(DFLAGS)" \
		--iflags "$(PHP_IFLAGS)" --def mod_php.def --libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" \
        $(MPR_HDR) $(EJS_HDR) $(SRC)/handlers/phpHandler.c

mod_ssl.vcproj: Makefile buildConfig.override
	@makeVsProj --project mod_ssl.vcproj --library ssl --dflags "$(DFLAGS)" \
		--def mod_ssl.def --libs "libmpr.lib libmprssl.lib libhttp.lib libappweb.lib $(LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/modules/sslModule.c

#
#	Executables
#
appweb.vcproj: Makefile buildConfig.override
	@makeVsProj --project appweb.vcproj --exe appweb.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(APPWEB_HDR) $(SRC)/server/appweb.c

angel.vcproj: Makefile buildConfig.override
	@makeVsProj --project angel.vcproj --graphical --exe angel.exe --search "$(BLD_EJS_LIBPATHS)" \
		--dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(APPWEB_HDR) $(SRC)/server/WIN/angel.c

appwebMonitor.vcproj: Makefile buildConfig.override
	@makeVsProj --project appwebMonitor.vcproj --graphical --exe appwebMonitor.exe --search "$(BLD_EJS_LIBPATHS)" \
		--dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(APPWEB_HDR) \
        $(SRC)/include/appwebMonitor.h $(SRC)/server/WIN/appwebMonitor.c

ejs.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejs.vcproj --exe ejs.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libejs.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(SRC)/deps/ejs/ejs.c

ejsmod.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejsmod.vcproj --exe ejsmod.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libejs.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) \
		$(SRC)/deps/ejs/ejsmod.c

ejsc.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejsc.vcproj --exe ejsc.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--custom ejsc.custom --libs "libmpr.lib libejs.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) \
		$(SRC)/deps/ejs/ejsc.c

ejssql.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejssql.vcproj --exe ejssql.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(MPR_SRC) $(SRC)/deps/ejs/ejssql.c $(SRC)/deps/ejs/sqlite3.c

ejsmvc.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejsmvc.vcproj --exe ejsmvc.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libejs.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(SRC)/deps/ejs/ejsmvc.c

ejspage.vcproj: Makefile buildConfig.override
	@makeVsProj --project ejspage.vcproj --exe ejspage.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libejs.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(SRC)/deps/ejs/ejspage.c

http.vcproj: Makefile buildConfig.override
	@makeVsProj --project http.vcproj --exe http.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libhttp.lib $(LIBS)" $(HTTP_HDR) $(SRC)/deps/http/http.c

httpPassword.vcproj: Makefile buildConfig.override
	@makeVsProj --project httpPassword.vcproj --exe httpPassword.exe --search "$(BLD_MPR_LIBPATHS)" \
        --dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(SRC)/utils/httpPassword.c

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
