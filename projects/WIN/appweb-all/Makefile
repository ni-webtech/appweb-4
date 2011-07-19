#
#	Makefile for Windows Visual Studio projects
#
#	Copyright (c) Embedthis Software LLC, 2003-2009. All Rights Reserved.
#

CONFIG		= buildConfig.default

include     .makedep

include     $(CONFIG)

MVS			= makeVsProj
SRC			= $(BLD_TOP)/src
OUT			= $(BLD_OUT_DIR)
MPR_HDR 	= $(OUT)/inc/buildConfig.h $(shell find $(OUT)/inc -name 'mpr*.h')
EJS_HDR 	= $(shell find $(OUT)/inc -name 'ejs*.h')
HTTP_HDR 	= $(shell find $(OUT)/inc -name 'http*.h')
APPWEB_HDR 	= $(OUT)/inc/appweb.h
MPR_SRC		= $(SRC)/deps/mpr/mprLib.c
APPWEB_SRC	= $(shell find $(SRC) -name '*.c' | egrep -v 'deps|test|utils' | egrep -v 'server/|filters|handlers|modules') \
              $(SRC)/handlers/dirHandler.c $(SRC)/handlers/egiHandler.c $(SRC)/handlers/fileHandler.c
LIBS		= ws2_32.lib
PROJECTS	= libmpr.vcproj libmprssl.vcproj libpcre.vcproj libhttp.vcproj libejs.vcproj \
              libappweb.vcproj appweb.vcproj angel.vcproj appwebMonitor.vcproj \
			  http.vcproj auth.vcproj mod_cgi.vcproj mod_ejs.vcproj mod_esp.vcproj mod_php.vcproj mod_ssl.vcproj
OTHER_PROJECTS= libsqlite3.vcproj ejsmod.vcproj ejsc.vcproj ejs.vcproj ejssql.vcproj mvc.vcproj 
DFLAGS		+= -DDEBUG_IDE
SSL_IFLAGS	+= $(BLD_OPENSSL_IFLAGS)
PHP_IFLAGS	+= $(BLD_PHP_IFLAGS)

compileExtra: prep $(PROJECTS)

prep:
	if ls $(BLD_BIN_DIR)/*.def >/dev/null 2>&1 ; then \
		for f in $(BLD_BIN_DIR)/*.def ; do \
			base=`basename "$$f"` ; \
			cp $$f $$.tmp ; cat $$.tmp | grep -v mask@@NegDouble >"$$base" ; rm -f $$.tmp ; \
		done ; \
	fi

appweb.sln: Makefile $(CONFIG)

setup.vcproj: Makefile $(CONFIG)
	# hand crafted
	#   $(MVS) --project setup.vcproj --custom setup.custom

x.vcxproj: Makefile $(CONFIG)
	echo $(MVS) --project x.vcxproj --library mpr --dflags "$(DFLAGS)" \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(MPR_SRC)
	$(MVS) --project x.vcxproj --library mpr --dflags "$(DFLAGS)" \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(MPR_SRC)

libmpr.vcproj: Makefile $(CONFIG)
	@$(MVS) --project libmpr.vcproj --library mpr --dflags "$(DFLAGS)" \
		--def libmpr.def --libs "$(LIBS)" $(MPR_HDR) $(MPR_SRC)
	
libmprssl.vcproj: Makefile $(CONFIG)
	@$(MVS) --project libmprssl.vcproj --library mprssl --dflags "$(DFLAGS)" \
		--iflags "$(SSL_IFLAGS)" --def libmprssl.def --libs "libmpr.lib $(LIBS)" \
		$(MPR_HDR) $(SRC)/deps/mpr/mprSsl.c
	
libpcre.vcproj: Makefile $(CONFIG)
	@$(MVS) --project libpcre.vcproj --library pcre --dflags "$(DFLAGS)" \
		--def libpcre.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(SRC)/deps/pcre/pcre.c
	
libsqlite3.vcproj: Makefile $(CONFIG)
	@$(MVS) --project libsqlite3.vcproj --library sqlite3 --dflags "$(DFLAGS)" \
		--def libsqlite3.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(SRC)/deps/ejs/sqlite3.c
	
libhttp.vcproj: Makefile $(CONFIG)
	@$(MVS) --project libhttp.vcproj --library http --dflags "$(DFLAGS)" \
		--def libhttp.def --libs "libmpr.lib libmprssl.lib $(LIBS)" $(MPR_HDR) $(HTTP_HDR) $(SRC)/deps/http/httpLib.c
	
libappweb.vcproj: Makefile $(CONFIG)
	@$(MVS) --project libappweb.vcproj --library appweb --dflags "$(DFLAGS)" \
		--def libappweb.def --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(APPWEB_HDR) $(APPWEB_SRC)

#
#	Modules
#
mod_cgi.vcproj: Makefile $(CONFIG)
	@$(MVS) --project mod_cgi.vcproj --library cgi --dflags "$(DFLAGS)" \
		--def mod_cgi.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libejs.lib libappweb.lib $(LIBS)" \
		$(MPR_HDR) $(SRC)/handlers/cgiHandler.c

mod_ejs.vcproj: Makefile $(CONFIG)
	@$(MVS) --project mod_ejs.vcproj --library ejs --dflags "$(DFLAGS)" \
		--def mod_ejs.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libejs.lib libappweb.lib $(LIBS)" \
		$(MPR_HDR) $(SRC)/handlers/ejsHandler.c

mod_esp.vcproj: Makefile $(CONFIG)
	@$(MVS) --project mod_esp.vcproj --library esp --dflags "$(DFLAGS)" \
		--def mod_esp.def --libs "libmpr.lib libhttp.lib libsqlite3.lib libesp.lib libappweb.lib $(LIBS)" $(MPR_HDR) \
		$(SRC)/handlers/esp/espHandler.c $(SRC)/handlers/esp/espFramework.c $(SRC)/handlers/esp/espTemplate.c

mod_php.vcproj: Makefile $(CONFIG)
	@$(MVS) --project mod_php.vcproj --library php --dflags "$(DFLAGS)" \
		--iflags "$(PHP_IFLAGS)" --def mod_php.def --libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" \
        $(MPR_HDR) $(EJS_HDR) $(SRC)/handlers/phpHandler.c

mod_ssl.vcproj: Makefile $(CONFIG)
	@$(MVS) --project mod_ssl.vcproj --library ssl --dflags "$(DFLAGS)" \
		--def mod_ssl.def --libs "libmpr.lib libmprssl.lib libhttp.lib libappweb.lib $(LIBS)" \
		$(MPR_HDR) $(EJS_HDR) $(SRC)/modules/sslModule.c

#
#	Executables
#
appweb.vcproj: Makefile $(CONFIG)
	@$(MVS) --project appweb.vcproj --exe appweb.exe --search "$(BLD_EJS_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libhttp.lib libappweb.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(APPWEB_HDR) $(SRC)/server/appweb.c

angel.vcproj: Makefile $(CONFIG)
	@$(MVS) --project angel.vcproj --graphical --exe angel.exe --search "$(BLD_EJS_LIBPATHS)" \
		--dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(APPWEB_HDR) $(SRC)/deps/mpr/angel.c

appwebMonitor.vcproj: Makefile $(CONFIG)
	@$(MVS) --project appwebMonitor.vcproj --graphical --exe appwebMonitor.exe --search "$(BLD_EJS_LIBPATHS)" \
		--dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(APPWEB_HDR) \
        $(SRC)/include/appwebMonitor.h $(SRC)/server/WIN/appwebMonitor.c

auth.vcproj: Makefile $(CONFIG)
	@$(MVS) --project auth.vcproj --exe auth.exe --search "$(BLD_MPR_LIBPATHS)" \
        --dflags "$(DFLAGS)" --libs "libmpr.lib $(LIBS)" $(MPR_HDR) $(SRC)/utils/auth.c

http.vcproj: Makefile $(CONFIG)
	@$(MVS) --project http.vcproj --exe http.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
		--libs "libmpr.lib libhttp.lib $(LIBS)" $(HTTP_HDR) $(SRC)/deps/http/http.c

#ejspage.vcproj: Makefile $(CONFIG)
#	@$(MVS) --project ejspage.vcproj --exe ejspage.exe --search "$(BLD_MPR_LIBPATHS)" --dflags "$(DFLAGS)" \
#		--libs "libmpr.lib libejs.lib $(LIBS)" $(MPR_HDR) $(EJS_HDR) $(SRC)/deps/ejs/ejspage.c

cleanExtra:
	rm -fr Upgrade* _Upgrade* x64 Win32 *.suo *.sdf *.filters *.vcxproj*

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
