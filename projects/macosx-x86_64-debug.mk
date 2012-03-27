#
#   build.mk -- Build It Makefile to build Embedthis Appweb for macosx on x86_64
#

PLATFORM  := macosx-x86_64-debug
CC        := cc
CFLAGS    := -fPIC -Wall -g
DFLAGS    := -DPIC -DCPU=X86_64
IFLAGS    := -I$(PLATFORM)/inc
LDFLAGS   := -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g
LIBS      := -lpthread -lm


all: prep \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/bin/appman \
        $(PLATFORM)/bin/makerom \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/bin/http \
        $(PLATFORM)/lib/libsqlite3.dylib \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/lib/mod_esp.dylib \
        $(PLATFORM)/bin/esp \
        $(PLATFORM)/lib/esp.conf \
        $(PLATFORM)/lib/esp-www \
        $(PLATFORM)/lib/mod_cgi.dylib \
        $(PLATFORM)/bin/auth \
        $(PLATFORM)/bin/cgiProgram \
        $(PLATFORM)/bin/setConfig \
        $(PLATFORM)/bin/appweb \
        $(PLATFORM)/bin/testAppweb \
        test/cgi-bin/testScript \
        test/web/caching/cache.cgi \
        test/web/basic/basic.cgi \
        test/cgi-bin/cgiProgram

.PHONY: prep

prep:
	@[ ! -x $(PLATFORM)/inc ] && mkdir -p $(PLATFORM)/inc $(PLATFORM)/obj $(PLATFORM)/lib $(PLATFORM)/bin ; true
	@[ ! -f $(PLATFORM)/inc/buildConfig.h ] && cp projects/buildConfig.$(PLATFORM) $(PLATFORM)/inc/buildConfig.h ; true

clean:
	rm -rf $(PLATFORM)/lib/libmpr.dylib
	rm -rf $(PLATFORM)/lib/libmprssl.dylib
	rm -rf $(PLATFORM)/bin/appman
	rm -rf $(PLATFORM)/bin/makerom
	rm -rf $(PLATFORM)/lib/libpcre.dylib
	rm -rf $(PLATFORM)/lib/libhttp.dylib
	rm -rf $(PLATFORM)/bin/http
	rm -rf $(PLATFORM)/lib/libsqlite3.dylib
	rm -rf $(PLATFORM)/lib/libappweb.dylib
	rm -rf $(PLATFORM)/lib/mod_esp.dylib
	rm -rf $(PLATFORM)/bin/esp
	rm -rf $(PLATFORM)/lib/esp.conf
	rm -rf $(PLATFORM)/lib/esp-www
	rm -rf $(PLATFORM)/lib/mod_cgi.dylib
	rm -rf $(PLATFORM)/lib/mod_ejs.dylib
	rm -rf $(PLATFORM)/lib/mod_php.dylib
	rm -rf $(PLATFORM)/lib/mod_ssl.dylib
	rm -rf $(PLATFORM)/bin/auth
	rm -rf $(PLATFORM)/bin/cgiProgram
	rm -rf $(PLATFORM)/bin/setConfig
	rm -rf $(PLATFORM)/bin/appweb
	rm -rf $(PLATFORM)/bin/appwebMonitor
	rm -rf $(PLATFORM)/lib/appwebMonitor.ico
	rm -rf $(PLATFORM)/bin/testAppweb
	rm -rf $(PLATFORM)/bin/removeFiles
	rm -rf $(PLATFORM)/obj/mprLib.o
	rm -rf $(PLATFORM)/obj/mprSsl.o
	rm -rf $(PLATFORM)/obj/manager.o
	rm -rf $(PLATFORM)/obj/makerom.o
	rm -rf $(PLATFORM)/obj/pcre.o
	rm -rf $(PLATFORM)/obj/httpLib.o
	rm -rf $(PLATFORM)/obj/http.o
	rm -rf $(PLATFORM)/obj/sqlite3.o
	rm -rf $(PLATFORM)/obj/config.o
	rm -rf $(PLATFORM)/obj/convenience.o
	rm -rf $(PLATFORM)/obj/dirHandler.o
	rm -rf $(PLATFORM)/obj/fileHandler.o
	rm -rf $(PLATFORM)/obj/log.o
	rm -rf $(PLATFORM)/obj/server.o
	rm -rf $(PLATFORM)/obj/edi.o
	rm -rf $(PLATFORM)/obj/espAbbrev.o
	rm -rf $(PLATFORM)/obj/espFramework.o
	rm -rf $(PLATFORM)/obj/espHandler.o
	rm -rf $(PLATFORM)/obj/espHtml.o
	rm -rf $(PLATFORM)/obj/espSession.o
	rm -rf $(PLATFORM)/obj/espTemplate.o
	rm -rf $(PLATFORM)/obj/mdb.o
	rm -rf $(PLATFORM)/obj/sdb.o
	rm -rf $(PLATFORM)/obj/esp.o
	rm -rf $(PLATFORM)/obj/cgiHandler.o
	rm -rf $(PLATFORM)/obj/ejsHandler.o
	rm -rf $(PLATFORM)/obj/phpHandler.o
	rm -rf $(PLATFORM)/obj/sslModule.o
	rm -rf $(PLATFORM)/obj/auth.o
	rm -rf $(PLATFORM)/obj/cgiProgram.o
	rm -rf $(PLATFORM)/obj/setConfig.o
	rm -rf $(PLATFORM)/obj/appweb.o
	rm -rf $(PLATFORM)/obj/appwebMonitor.o
	rm -rf $(PLATFORM)/obj/testAppweb.o
	rm -rf $(PLATFORM)/obj/testHttp.o
	rm -rf $(PLATFORM)/obj/removeFiles.o

$(PLATFORM)/inc/mpr.h: 
	rm -fr macosx-x86_64-debug/inc/mpr.h
	cp -r src/deps/mpr/mpr.h macosx-x86_64-debug/inc/mpr.h

$(PLATFORM)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/mprLib.c

$(PLATFORM)/lib/libmpr.dylib:  \
        $(PLATFORM)/inc/mpr.h \
        $(PLATFORM)/obj/mprLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libmpr.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libmpr.dylib $(PLATFORM)/obj/mprLib.o $(LIBS)

$(PLATFORM)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/manager.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/manager.c

$(PLATFORM)/bin/appman:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/manager.o
	$(CC) -o $(PLATFORM)/bin/appman -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/manager.o $(LIBS) -lmpr

$(PLATFORM)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/makerom.c

$(PLATFORM)/bin/makerom:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/makerom.o
	$(CC) -o $(PLATFORM)/bin/makerom -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/makerom.o $(LIBS) -lmpr

$(PLATFORM)/inc/pcre.h: 
	rm -fr macosx-x86_64-debug/inc/pcre.h
	cp -r src/deps/pcre/pcre.h macosx-x86_64-debug/inc/pcre.h

$(PLATFORM)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/pcre/pcre.c

$(PLATFORM)/lib/libpcre.dylib:  \
        $(PLATFORM)/inc/pcre.h \
        $(PLATFORM)/obj/pcre.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libpcre.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libpcre.dylib $(PLATFORM)/obj/pcre.o $(LIBS)

$(PLATFORM)/inc/http.h: 
	rm -fr macosx-x86_64-debug/inc/http.h
	cp -r src/deps/http/http.h macosx-x86_64-debug/inc/http.h

$(PLATFORM)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/http.h \
        $(PLATFORM)/inc/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/httpLib.c

$(PLATFORM)/lib/libhttp.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/inc/http.h \
        $(PLATFORM)/obj/httpLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libhttp.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libhttp.dylib $(PLATFORM)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(PLATFORM)/obj/http.o: \
        src/deps/http/http.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/http.h
	$(CC) -c -o $(PLATFORM)/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/http.c

$(PLATFORM)/bin/http:  \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/obj/http.o
	$(CC) -o $(PLATFORM)/bin/http -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre

$(PLATFORM)/inc/sqlite3.h: 
	rm -fr macosx-x86_64-debug/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h macosx-x86_64-debug/inc/sqlite3.h

$(PLATFORM)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/sqlite3.h
	$(CC) -c -o $(PLATFORM)/obj/sqlite3.o -arch x86_64 -fPIC -g $(DFLAGS) -I$(PLATFORM)/inc src/deps/sqlite/sqlite3.c

$(PLATFORM)/lib/libsqlite3.dylib:  \
        $(PLATFORM)/inc/sqlite3.h \
        $(PLATFORM)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libsqlite3.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libsqlite3.dylib $(PLATFORM)/obj/sqlite3.o $(LIBS)

$(PLATFORM)/inc/appweb.h: 
	rm -fr macosx-x86_64-debug/inc/appweb.h
	cp -r src/appweb.h macosx-x86_64-debug/inc/appweb.h

$(PLATFORM)/inc/customize.h: 
	rm -fr macosx-x86_64-debug/inc/customize.h
	cp -r src/customize.h macosx-x86_64-debug/inc/customize.h

$(PLATFORM)/obj/config.o: \
        src/config.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h \
        $(PLATFORM)/inc/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/config.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/config.c

$(PLATFORM)/obj/convenience.o: \
        src/convenience.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/convenience.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/convenience.c

$(PLATFORM)/obj/dirHandler.o: \
        src/dirHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/dirHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/dirHandler.c

$(PLATFORM)/obj/fileHandler.o: \
        src/fileHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/fileHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/fileHandler.c

$(PLATFORM)/obj/log.o: \
        src/log.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/log.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/log.c

$(PLATFORM)/obj/server.o: \
        src/server.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/server.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/server.c

$(PLATFORM)/lib/libappweb.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/inc/appweb.h \
        $(PLATFORM)/inc/customize.h \
        $(PLATFORM)/obj/config.o \
        $(PLATFORM)/obj/convenience.o \
        $(PLATFORM)/obj/dirHandler.o \
        $(PLATFORM)/obj/fileHandler.o \
        $(PLATFORM)/obj/log.o \
        $(PLATFORM)/obj/server.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libappweb.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libappweb.dylib $(PLATFORM)/obj/config.o $(PLATFORM)/obj/convenience.o $(PLATFORM)/obj/dirHandler.o $(PLATFORM)/obj/fileHandler.o $(PLATFORM)/obj/log.o $(PLATFORM)/obj/server.o $(LIBS) -lmpr -lhttp -lpcre -lpcre

$(PLATFORM)/inc/edi.h: 
	rm -fr macosx-x86_64-debug/inc/edi.h
	cp -r src/esp/edi.h macosx-x86_64-debug/inc/edi.h

$(PLATFORM)/inc/esp-app.h: 
	rm -fr macosx-x86_64-debug/inc/esp-app.h
	cp -r src/esp/esp-app.h macosx-x86_64-debug/inc/esp-app.h

$(PLATFORM)/inc/esp.h: 
	rm -fr macosx-x86_64-debug/inc/esp.h
	cp -r src/esp/esp.h macosx-x86_64-debug/inc/esp.h

$(PLATFORM)/inc/mdb.h: 
	rm -fr macosx-x86_64-debug/inc/mdb.h
	cp -r src/esp/mdb.h macosx-x86_64-debug/inc/mdb.h

$(PLATFORM)/obj/edi.o: \
        src/esp/edi.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/edi.h \
        $(PLATFORM)/inc/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/edi.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/edi.c

$(PLATFORM)/obj/espAbbrev.o: \
        src/esp/espAbbrev.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espAbbrev.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espAbbrev.c

$(PLATFORM)/obj/espFramework.o: \
        src/esp/espFramework.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espFramework.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espFramework.c

$(PLATFORM)/obj/espHandler.o: \
        src/esp/espHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h \
        $(PLATFORM)/inc/esp.h \
        $(PLATFORM)/inc/edi.h
	$(CC) -c -o $(PLATFORM)/obj/espHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espHandler.c

$(PLATFORM)/obj/espHtml.o: \
        src/esp/espHtml.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/esp.h \
        $(PLATFORM)/inc/edi.h
	$(CC) -c -o $(PLATFORM)/obj/espHtml.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espHtml.c

$(PLATFORM)/obj/espSession.o: \
        src/esp/espSession.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espSession.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espSession.c

$(PLATFORM)/obj/espTemplate.o: \
        src/esp/espTemplate.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espTemplate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espTemplate.c

$(PLATFORM)/obj/mdb.o: \
        src/esp/mdb.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h \
        $(PLATFORM)/inc/edi.h \
        $(PLATFORM)/inc/mdb.h \
        $(PLATFORM)/inc/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/mdb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/mdb.c

$(PLATFORM)/obj/sdb.o: \
        src/esp/sdb.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h \
        $(PLATFORM)/inc/edi.h
	$(CC) -c -o $(PLATFORM)/obj/sdb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/sdb.c

$(PLATFORM)/lib/mod_esp.dylib:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/inc/edi.h \
        $(PLATFORM)/inc/esp-app.h \
        $(PLATFORM)/inc/esp.h \
        $(PLATFORM)/inc/mdb.h \
        $(PLATFORM)/obj/edi.o \
        $(PLATFORM)/obj/espAbbrev.o \
        $(PLATFORM)/obj/espFramework.o \
        $(PLATFORM)/obj/espHandler.o \
        $(PLATFORM)/obj/espHtml.o \
        $(PLATFORM)/obj/espSession.o \
        $(PLATFORM)/obj/espTemplate.o \
        $(PLATFORM)/obj/mdb.o \
        $(PLATFORM)/obj/sdb.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_esp.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/mod_esp.dylib $(PLATFORM)/obj/edi.o $(PLATFORM)/obj/espAbbrev.o $(PLATFORM)/obj/espFramework.o $(PLATFORM)/obj/espHandler.o $(PLATFORM)/obj/espHtml.o $(PLATFORM)/obj/espSession.o $(PLATFORM)/obj/espTemplate.o $(PLATFORM)/obj/mdb.o $(PLATFORM)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/esp.o: \
        src/esp/esp.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/esp.h
	$(CC) -c -o $(PLATFORM)/obj/esp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/esp.c

$(PLATFORM)/bin/esp:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/edi.o \
        $(PLATFORM)/obj/esp.o \
        $(PLATFORM)/obj/espAbbrev.o \
        $(PLATFORM)/obj/espFramework.o \
        $(PLATFORM)/obj/espHandler.o \
        $(PLATFORM)/obj/espHtml.o \
        $(PLATFORM)/obj/espSession.o \
        $(PLATFORM)/obj/espTemplate.o \
        $(PLATFORM)/obj/mdb.o \
        $(PLATFORM)/obj/sdb.o
	$(CC) -o $(PLATFORM)/bin/esp -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/edi.o $(PLATFORM)/obj/esp.o $(PLATFORM)/obj/espAbbrev.o $(PLATFORM)/obj/espFramework.o $(PLATFORM)/obj/espHandler.o $(PLATFORM)/obj/espHtml.o $(PLATFORM)/obj/espSession.o $(PLATFORM)/obj/espTemplate.o $(PLATFORM)/obj/mdb.o $(PLATFORM)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/lib/esp.conf: 
	rm -fr macosx-x86_64-debug/lib/esp.conf
	cp -r src/esp/esp.conf macosx-x86_64-debug/lib/esp.conf

$(PLATFORM)/lib/esp-www: 
	rm -fr macosx-x86_64-debug/lib/esp-www
	cp -r src/esp/www macosx-x86_64-debug/lib/esp-www

$(PLATFORM)/obj/cgiHandler.o: \
        src/modules/cgiHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/cgiHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/modules/cgiHandler.c

$(PLATFORM)/lib/mod_cgi.dylib:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/cgiHandler.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_cgi.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/mod_cgi.dylib $(PLATFORM)/obj/cgiHandler.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/auth.o: \
        src/utils/auth.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/http.h
	$(CC) -c -o $(PLATFORM)/obj/auth.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/utils/auth.c

$(PLATFORM)/bin/auth:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/auth.o
	$(CC) -o $(PLATFORM)/bin/auth -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/auth.o $(LIBS) -lmpr

$(PLATFORM)/obj/cgiProgram.o: \
        src/utils/cgiProgram.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/cgiProgram.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/utils/cgiProgram.c

$(PLATFORM)/bin/cgiProgram:  \
        $(PLATFORM)/obj/cgiProgram.o
	$(CC) -o $(PLATFORM)/bin/cgiProgram -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/cgiProgram.o $(LIBS)

$(PLATFORM)/obj/setConfig.o: \
        src/utils/setConfig.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/setConfig.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/utils/setConfig.c

$(PLATFORM)/bin/setConfig:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/setConfig.o
	$(CC) -o $(PLATFORM)/bin/setConfig -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/setConfig.o $(LIBS) -lmpr

$(PLATFORM)/inc/appwebMonitor.h: 
	rm -fr macosx-x86_64-debug/inc/appwebMonitor.h
	cp -r src/server/appwebMonitor.h macosx-x86_64-debug/inc/appwebMonitor.h

$(PLATFORM)/obj/appweb.o: \
        src/server/appweb.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/appweb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/server/appweb.c

$(PLATFORM)/bin/appweb:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/inc/appwebMonitor.h \
        $(PLATFORM)/obj/appweb.o
	$(CC) -o $(PLATFORM)/bin/appweb -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/appweb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/inc/testAppweb.h: 
	rm -fr macosx-x86_64-debug/inc/testAppweb.h
	cp -r test/testAppweb.h macosx-x86_64-debug/inc/testAppweb.h

$(PLATFORM)/obj/testAppweb.o: \
        test/testAppweb.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/testAppweb.h
	$(CC) -c -o $(PLATFORM)/obj/testAppweb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc test/testAppweb.c

$(PLATFORM)/obj/testHttp.o: \
        test/testHttp.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/testAppweb.h
	$(CC) -c -o $(PLATFORM)/obj/testHttp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc test/testHttp.c

$(PLATFORM)/bin/testAppweb:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/inc/testAppweb.h \
        $(PLATFORM)/obj/testAppweb.o \
        $(PLATFORM)/obj/testHttp.o
	$(CC) -o $(PLATFORM)/bin/testAppweb -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/testAppweb.o $(PLATFORM)/obj/testHttp.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

test/cgi-bin/testScript: 
	echo '#!$(PLATFORM)/bin/cgiProgram' >test/cgi-bin/testScript ; chmod +x test/cgi-bin/testScript

test/web/caching/cache.cgi: 
	echo -e '#!`type -p sh`' >test/web/caching/cache.cgi
	echo -e '' >>test/web/caching/cache.cgi
	echo -e 'echo HTTP/1.0 200 OK' >>test/web/caching/cache.cgi
	echo -e 'echo Content-Type: text/plain' >>test/web/caching/cache.cgi
	echo -e 'date' >>test/web/caching/cache.cgi
	chmod +x test/web/caching/cache.cgi

test/web/basic/basic.cgi: 
	echo -e '#!`type -p sh`' >test/web/basic/basic.cgi
	echo -e '' >>test/web/basic/basic.cgi
	echo -e 'echo Content-Type: text/plain' >>test/web/basic/basic.cgi
	echo -e '/usr/bin/env' >>test/web/basic/basic.cgi
	chmod +x test/web/basic/basic.cgi

test/cgi-bin/cgiProgram: 
	cp $(PLATFORM)/bin/cgiProgram test/cgi-bin/cgiProgram
	cp $(PLATFORM)/bin/cgiProgram test/cgi-bin/nph-cgiProgram
	cp $(PLATFORM)/bin/cgiProgram 'test/cgi-bin/cgi Program'
	cp $(PLATFORM)/bin/cgiProgram test/web/cgiProgram.cgi
	chmod +x test/cgi-bin/* test/web/cgiProgram.cgi

