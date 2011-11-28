#
#	Makefile for Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

CONFIG		= buildConfig.default

include     .makedep
include     $(CONFIG)

HEADERS		=
VSP			= makeVsProj
VSS			= makeVsSol
SRC			= $(BLD_TOP)/src
OUT			= $(BLD_OUT_DIR)
MPR_HDR 	= $(OUT)/inc/buildConfig.h $(shell find $(OUT)/inc -name 'mpr*.h')
EJS_HDR 	= $(shell find $(OUT)/inc -name 'ejs*.h')
HTTP_HDR 	= $(shell find $(OUT)/inc -name 'http*.h')
APPWEB_HDR 	= $(OUT)/inc/appweb.h
MPR_SRC		= $(SRC)/deps/mpr/mprLib.c
APPWEB_SRC	= $(shell find $(SRC) -name '*.c' | egrep -v 'deps|test|utils' | \
			  egrep -v 'server/|filters|handlers|modules|esp|samples') \
              $(SRC)/modules/dirHandler.c $(SRC)/modules/fileHandler.c
LIBS		= ws2_32.lib
SSL_IFLAGS	+= $(BLD_OPENSSL_IFLAGS)
PHP_IFLAGS	+= $(BLD_PHP_IFLAGS)

PROJECTS	= setup.vcxproj libmpr.vcxproj libmprssl.vcxproj libpcre.vcxproj libhttp.vcxproj \
              libappweb.vcxproj appweb.vcxproj appman.vcxproj appwebMonitor.vcxproj \
			  http.vcxproj auth.vcxproj mod_cgi.vcxproj mod_ejs.vcxproj mod_esp.vcxproj mod_php.vcxproj mod_ssl.vcxproj \
			  esp.vcxproj
OTHER_PROJECTS= libejs.vcxproj ejsmod.vcxproj ejsc.vcxproj ejs.vcxproj sqlite.vcxproj mvc.vcxproj libsqlite3.vcxproj
SOLUTIONS	= appweb-all.sln

PROPS		+= product.props

#
#	Don't want to do this every build, just on demand
#
gen: prep $(PROJECTS) $(SOLUTIONS)

prep:
	touch Makefile
	if ls $(BLD_BIN_DIR)/*.def >/dev/null 2>&1 ; then \
		for f in $(BLD_BIN_DIR)/*.def ; do \
			base=`basename "$$f"` ; \
			cp $$f $$.tmp ; cat $$.tmp | grep -v mask@@NegDouble >"$$base" ; rm -f $$.tmp ; \
		done ; \
	fi

appweb-all.sln: Makefile $(CONFIG)
	$(VSS) --solution appweb-all.sln \
		--deps "libmpr setup" \
		--deps "libpcre setup" \
		$(PROJECTS)

setup.vcxproj: setup.custom Makefile $(CONFIG)
	$(VSP) --project setup.vcxproj --props "$(PROPS)" --custom setup.custom

libmpr.vcxproj: libmpr.custom Makefile $(CONFIG)
	$(VSP) --project libmpr.vcxproj --library mpr --dflags "$(DFLAGS)" \
		--props "$(PROPS)" \
		--custom libmpr.custom \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(MPR_SRC)
	
libmprssl.vcxproj: libmprssl.custom Makefile $(CONFIG)
	$(VSP) --project libmprssl.vcxproj --library mprssl --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libmpr \
		--custom libmprssl.custom \
		--iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(LIBS)" \
		$(MPR_HDR) $(SRC)/deps/mpr/mprSsl.c
	
libpcre.vcxproj: libpcre.custom Makefile $(CONFIG)
	$(VSP) --project libpcre.vcxproj --library pcre --dflags "$(DFLAGS)" \
		--props "$(PROPS)" \
		--custom libpcre.custom \
		--def libpcre.def --libs "$(LIBS)" $(SRC)/deps/pcre/pcre.c
	
libhttp.vcxproj: libhttp.custom Makefile $(CONFIG)
	$(VSP) --project libhttp.vcxproj --library http --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libmprssl libpcre" \
		--custom libhttp.custom \
		--def libhttp.def --libs "libmpr.lib libmprssl.lib $(LIBS)" $(HTTP_HDR) $(SRC)/deps/http/httpLib.c
	
libsqlite3.vcxproj: libsqlite3.custom Makefile $(CONFIG)
	$(VSP) --project libsqlite3.vcxproj --library sqlite --dflags "$(DFLAGS)" \
		--props "$(PROPS)" \
		--custom libsqlite3.custom \
		--def libsqlite3.def --libs "$(LIBS)" $(SRC)/deps/sqlite/sqlite3.c
	
libappweb.vcxproj: libappweb.custom Makefile $(CONFIG)
	$(VSP) --project libappweb.vcxproj --library appweb --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libmpr libhttp libpcre" \
		--custom libappweb.custom \
		--def libappweb.def --libs "libmpr.lib $(LIBS)" $(APPWEB_HDR) $(APPWEB_SRC)

#
#	Modules
#
mod_cgi.vcxproj: Makefile $(CONFIG)
	$(VSP) --project mod_cgi.vcxproj --library cgi --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libappweb \
		--condition "'$$\(BLD_FEATURE_CGI)' == '1'" \
		--def mod_cgi.def --libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" \
		$(SRC)/modules/cgiHandler.c

mod_ejs.vcxproj: Makefile $(CONFIG)
	$(VSP) --project mod_ejs.vcxproj --library ejs --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libappweb \
		--condition "'$$\(BLD_FEATURE_EJS)' == '1'" \
		--def mod_ejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libejs.lib libappweb.lib $(LIBS)" \
		$(EJS_HDR) $(SRC)/modules/ejsHandler.c

mod_esp.vcxproj: Makefile $(CONFIG)
	$(VSP) --project mod_esp.vcxproj --library esp --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps "libappweb libpcre" \
		--custom mod_esp.custom \
		--def mod_esp.def --libs "libmpr.lib libhttp.lib libpcre.lib libappweb.lib $(LIBS)" \
		$(SRC)/modules/espHandler.c $(SRC)/esp/espFramework.c $(SRC)/esp/espSession.c $(SRC)/esp/espTemplate.c \
		$(SRC)/esp/espAbbrev.c $(SRC)/esp/espHtml.c $(SRC)/esp/edi.c $(SRC)/esp/mdb.c $(SRC)/esp/sdb.c

mod_php.vcxproj: Makefile $(CONFIG)
	$(VSP) --project mod_php.vcxproj --library php --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libappweb \
		--condition "'$$\(BLD_FEATURE_PHP)' == '1'" \
		--iflags "$(PHP_IFLAGS)" --def mod_php.def --libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" \
        $(SRC)/modules/phpHandler.c

mod_ssl.vcxproj: Makefile $(CONFIG)
	$(VSP) --project mod_ssl.vcxproj --library ssl --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libappweb \
		--condition "'$$\(BLD_FEATURE_SSL)' == '1'" \
		--def mod_ssl.def --libs "libmpr.lib libmprssl.lib libhttp.lib libappweb.lib $(LIBS)" \
		$(SRC)/modules/sslModule.c

#
#	Executables
#
appweb.vcxproj: Makefile $(CONFIG)
	$(VSP) --project appweb.vcxproj --exe appweb.exe --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libappweb \
		--libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" $(APPWEB_HDR) $(SRC)/server/appweb.c

appman.vcxproj: Makefile $(CONFIG)
	$(VSP) --project appman.vcxproj --graphical --exe appman.exe \
		--props "$(PROPS)" --deps libmpr \
		--dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(APPWEB_HDR) $(SRC)/deps/mpr/manager.c

appwebMonitor.vcxproj: appwebMonitor.custom Makefile $(CONFIG)
	$(VSP) --project appwebMonitor.vcxproj --graphical --exe appwebMonitor.exe \
		--props "$(PROPS)" --deps libmpr \
		--custom appwebMonitor.custom \
		--resources $(SRC)/server/appweb.rc \
		--dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(APPWEB_HDR) \
        $(SRC)/include/appwebMonitor.h $(SRC)/server/WIN/appwebMonitor.c

auth.vcxproj: Makefile $(CONFIG)
	$(VSP) --project auth.vcxproj --exe auth.exe --search "$(BLD_MPR_LIBPATHS)" \
		--props "$(PROPS)" --deps libmpr \
        --dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(SRC)/utils/auth.c

esp.vcxproj: Makefile $(CONFIG)
	$(VSP) --project esp.vcxproj --exe esp.exe --search "$(BLD_MPR_LIBPATHS)" \
		--props "$(PROPS)" --deps "libmpr mod_esp" \
		--libs "libmpr.lib libhttp.lib libpcre.lib libappweb.lib $(LIBS)" \
        --dflags "$(DFLAGS)" --libs "mod_esp.lib libmpr.lib $(LIBS)" $(SRC)/esp/esp.c

http.vcxproj: Makefile $(CONFIG)
	$(VSP) --project http.vcxproj --exe http.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps libhttp \
		--libs "libmpr.lib libhttp.lib $(LIBS)" $(HTTP_HDR) $(SRC)/deps/http/http.c

#
#	Unused
#
ejspage.vcxproj: Makefile $(CONFIG)
	$(VSP) --project ejspage.vcxproj --exe ejspage.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--props "$(PROPS)" --deps setup \
		--libs "libmpr.lib libejs.lib $(LIBS)" $(SRC)/deps/ejs/ejspage.c

cleanExtra:
	rm -fr Upgrade* _Upgrade* x64 Win32 *.suo *.sdf *.filters

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
