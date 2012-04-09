#
#   macosx-x86_64-debug.mk -- Build It Makefile to build Embedthis Appweb for macosx on x86_64
#

CONFIG   := macosx-x86_64-debug
CC       := /usr/bin/cc
LD       := /usr/bin/ld
CFLAGS   := -fPIC -Wall -g -Wno-unused-result -Wshorten-64-to-32
DFLAGS   := -DPIC -DCPU=X86_64
IFLAGS   := -I$(CONFIG)/inc
LDFLAGS  := '-Wl,-rpath,@executable_path/../lib' '-Wl,-rpath,@executable_path/' '-Wl,-rpath,@loader_path/' '-g'
LIBPATHS := -L$(CONFIG)/lib
LIBS     := -lpthread -lm -ldl

all: prep \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/bin/appman \
        $(CONFIG)/bin/makerom \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/bin/http \
        $(CONFIG)/lib/libsqlite3.dylib \
        $(CONFIG)/bin/sqlite \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/lib/mod_esp.dylib \
        $(CONFIG)/bin/esp \
        $(CONFIG)/lib/esp.conf \
        $(CONFIG)/lib/esp-www \
        $(CONFIG)/lib/mod_cgi.dylib \
        $(CONFIG)/bin/auth \
        $(CONFIG)/bin/cgiProgram \
        $(CONFIG)/bin/setConfig \
        $(CONFIG)/bin/appweb \
        $(CONFIG)/bin/simpleServer \
        $(CONFIG)/lib/simpleHandler.dylib \
        $(CONFIG)/bin/simpleClient \
        src/samples/cpp/cppHandler/cppModule.dylib \
        src/samples/cpp/cppModule/cppModule.dylib \
        $(CONFIG)/bin/testAppweb \
        test/cgi-bin/testScript \
        test/web/caching/cache.cgi \
        test/web/basic/basic.cgi \
        test/cgi-bin/cgiProgram

.PHONY: prep

prep:
	@[ ! -x $(CONFIG)/inc ] && mkdir -p $(CONFIG)/inc $(CONFIG)/obj $(CONFIG)/lib $(CONFIG)/bin ; true
	@[ ! -f $(CONFIG)/inc/buildConfig.h ] && cp projects/buildConfig.$(CONFIG) $(CONFIG)/inc/buildConfig.h ; true
	@if ! diff $(CONFIG)/inc/buildConfig.h projects/buildConfig.$(CONFIG) >/dev/null ; then\
		echo cp projects/buildConfig.$(CONFIG) $(CONFIG)/inc/buildConfig.h  ; \
		cp projects/buildConfig.$(CONFIG) $(CONFIG)/inc/buildConfig.h  ; \
	fi; true

clean:
	rm -rf $(CONFIG)/lib/libmpr.dylib
	rm -rf $(CONFIG)/lib/libmprssl.dylib
	rm -rf $(CONFIG)/bin/appman
	rm -rf $(CONFIG)/bin/makerom
	rm -rf $(CONFIG)/lib/libpcre.dylib
	rm -rf $(CONFIG)/lib/libhttp.dylib
	rm -rf $(CONFIG)/bin/http
	rm -rf $(CONFIG)/lib/libsqlite3.dylib
	rm -rf $(CONFIG)/bin/sqlite
	rm -rf $(CONFIG)/lib/libappweb.dylib
	rm -rf $(CONFIG)/lib/mod_esp.dylib
	rm -rf $(CONFIG)/bin/esp
	rm -rf $(CONFIG)/lib/esp.conf
	rm -rf $(CONFIG)/lib/esp-www
	rm -rf $(CONFIG)/lib/mod_cgi.dylib
	rm -rf $(CONFIG)/lib/mod_ejs.dylib
	rm -rf $(CONFIG)/lib/mod_php.dylib
	rm -rf $(CONFIG)/lib/mod_ssl.dylib
	rm -rf $(CONFIG)/bin/auth
	rm -rf $(CONFIG)/bin/cgiProgram
	rm -rf $(CONFIG)/bin/setConfig
	rm -rf $(CONFIG)/bin/appweb
	rm -rf $(CONFIG)/bin/appwebMonitor
	rm -rf $(CONFIG)/lib/appwebMonitor.ico
	rm -rf $(CONFIG)/bin/simpleServer
	rm -rf $(CONFIG)/lib/simpleHandler.dylib
	rm -rf $(CONFIG)/bin/simpleClient
	rm -rf $(CONFIG)/bin/simpleEjs
	rm -rf src/samples/cpp/cppHandler/cppModule.dylib
	rm -rf src/samples/cpp/cppModule/cppModule.dylib
	rm -rf $(CONFIG)/bin/testAppweb
	rm -rf test/cgi-bin/testScript
	rm -rf test/web/caching/cache.cgi
	rm -rf test/web/basic/basic.cgi
	rm -rf test/cgi-bin/cgiProgram
	rm -rf $(CONFIG)/bin/removeFiles
	rm -rf $(CONFIG)/obj/mprLib.o
	rm -rf $(CONFIG)/obj/mprSsl.o
	rm -rf $(CONFIG)/obj/manager.o
	rm -rf $(CONFIG)/obj/makerom.o
	rm -rf $(CONFIG)/obj/pcre.o
	rm -rf $(CONFIG)/obj/httpLib.o
	rm -rf $(CONFIG)/obj/http.o
	rm -rf $(CONFIG)/obj/sqlite3.o
	rm -rf $(CONFIG)/obj/sqlite.o
	rm -rf $(CONFIG)/obj/config.o
	rm -rf $(CONFIG)/obj/convenience.o
	rm -rf $(CONFIG)/obj/dirHandler.o
	rm -rf $(CONFIG)/obj/fileHandler.o
	rm -rf $(CONFIG)/obj/log.o
	rm -rf $(CONFIG)/obj/server.o
	rm -rf $(CONFIG)/obj/edi.o
	rm -rf $(CONFIG)/obj/espAbbrev.o
	rm -rf $(CONFIG)/obj/espFramework.o
	rm -rf $(CONFIG)/obj/espHandler.o
	rm -rf $(CONFIG)/obj/espHtml.o
	rm -rf $(CONFIG)/obj/espSession.o
	rm -rf $(CONFIG)/obj/espTemplate.o
	rm -rf $(CONFIG)/obj/mdb.o
	rm -rf $(CONFIG)/obj/sdb.o
	rm -rf $(CONFIG)/obj/esp.o
	rm -rf $(CONFIG)/obj/cgiHandler.o
	rm -rf $(CONFIG)/obj/ejsHandler.o
	rm -rf $(CONFIG)/obj/phpHandler.o
	rm -rf $(CONFIG)/obj/sslModule.o
	rm -rf $(CONFIG)/obj/auth.o
	rm -rf $(CONFIG)/obj/cgiProgram.o
	rm -rf $(CONFIG)/obj/setConfig.o
	rm -rf $(CONFIG)/obj/appweb.o
	rm -rf $(CONFIG)/obj/appwebMonitor.o
	rm -rf $(CONFIG)/obj/simpleServer.o
	rm -rf $(CONFIG)/obj/simpleHandler.o
	rm -rf $(CONFIG)/obj/simpleClient.o
	rm -rf $(CONFIG)/obj/simpleEjs.o
	rm -rf $(CONFIG)/obj/cppHandler.o
	rm -rf $(CONFIG)/obj/cppModule.o
	rm -rf $(CONFIG)/obj/testAppweb.o
	rm -rf $(CONFIG)/obj/testHttp.o
	rm -rf $(CONFIG)/obj/removeFiles.o

clobber: clean
	rm -fr ./$(CONFIG)

$(CONFIG)/inc/mpr.h: 
	rm -fr macosx-x86_64-debug/inc/mpr.h
	cp -r src/deps/mpr/mpr.h macosx-x86_64-debug/inc/mpr.h

$(CONFIG)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/mprLib.c

$(CONFIG)/lib/libmpr.dylib:  \
        $(CONFIG)/inc/mpr.h \
        $(CONFIG)/obj/mprLib.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libmpr.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libmpr.dylib $(CONFIG)/obj/mprLib.o $(LIBS)

$(CONFIG)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/manager.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/manager.c

$(CONFIG)/bin/appman:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/obj/manager.o
	$(CC) -o $(CONFIG)/bin/appman -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/manager.o $(LIBS) -lmpr

$(CONFIG)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/mpr/makerom.c

$(CONFIG)/bin/makerom:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/obj/makerom.o
	$(CC) -o $(CONFIG)/bin/makerom -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/makerom.o $(LIBS) -lmpr

$(CONFIG)/inc/pcre.h: 
	rm -fr macosx-x86_64-debug/inc/pcre.h
	cp -r src/deps/pcre/pcre.h macosx-x86_64-debug/inc/pcre.h

$(CONFIG)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/pcre/pcre.c

$(CONFIG)/lib/libpcre.dylib:  \
        $(CONFIG)/inc/pcre.h \
        $(CONFIG)/obj/pcre.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libpcre.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libpcre.dylib $(CONFIG)/obj/pcre.o $(LIBS)

$(CONFIG)/inc/http.h: 
	rm -fr macosx-x86_64-debug/inc/http.h
	cp -r src/deps/http/http.h macosx-x86_64-debug/inc/http.h

$(CONFIG)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/httpLib.c

$(CONFIG)/lib/libhttp.dylib:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/inc/http.h \
        $(CONFIG)/obj/httpLib.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libhttp.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libhttp.dylib $(CONFIG)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(CONFIG)/obj/http.o: \
        src/deps/http/http.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/http/http.c

$(CONFIG)/bin/http:  \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/obj/http.o
	$(CC) -o $(CONFIG)/bin/http -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre

$(CONFIG)/inc/sqlite3.h: 
	rm -fr macosx-x86_64-debug/inc/sqlite3.h
	cp -r src/deps/sqlite/sqlite3.h macosx-x86_64-debug/inc/sqlite3.h

$(CONFIG)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite3.o -arch x86_64 -fPIC -g -Wno-unused-result $(DFLAGS) -I$(CONFIG)/inc src/deps/sqlite/sqlite3.c

$(CONFIG)/lib/libsqlite3.dylib:  \
        $(CONFIG)/inc/sqlite3.h \
        $(CONFIG)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libsqlite3.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libsqlite3.dylib $(CONFIG)/obj/sqlite3.o $(LIBS)

$(CONFIG)/obj/sqlite.o: \
        src/deps/sqlite/sqlite.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/sqlite3.h
	$(CC) -c -o $(CONFIG)/obj/sqlite.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/deps/sqlite/sqlite.c

$(CONFIG)/bin/sqlite:  \
        $(CONFIG)/lib/libsqlite3.dylib \
        $(CONFIG)/obj/sqlite.o
	$(CC) -o $(CONFIG)/bin/sqlite -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/sqlite.o $(LIBS) -lsqlite3

$(CONFIG)/inc/appweb.h: 
	rm -fr macosx-x86_64-debug/inc/appweb.h
	cp -r src/appweb.h macosx-x86_64-debug/inc/appweb.h

$(CONFIG)/inc/customize.h: 
	rm -fr macosx-x86_64-debug/inc/customize.h
	cp -r src/customize.h macosx-x86_64-debug/inc/customize.h

$(CONFIG)/obj/config.o: \
        src/config.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/config.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/config.c

$(CONFIG)/obj/convenience.o: \
        src/convenience.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/convenience.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/convenience.c

$(CONFIG)/obj/dirHandler.o: \
        src/dirHandler.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/dirHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/dirHandler.c

$(CONFIG)/obj/fileHandler.o: \
        src/fileHandler.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/fileHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/fileHandler.c

$(CONFIG)/obj/log.o: \
        src/log.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/log.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/log.c

$(CONFIG)/obj/server.o: \
        src/server.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/server.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/server.c

$(CONFIG)/lib/libappweb.dylib:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/lib/libhttp.dylib \
        $(CONFIG)/lib/libpcre.dylib \
        $(CONFIG)/inc/appweb.h \
        $(CONFIG)/inc/customize.h \
        $(CONFIG)/obj/config.o \
        $(CONFIG)/obj/convenience.o \
        $(CONFIG)/obj/dirHandler.o \
        $(CONFIG)/obj/fileHandler.o \
        $(CONFIG)/obj/log.o \
        $(CONFIG)/obj/server.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/libappweb.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/libappweb.dylib $(CONFIG)/obj/config.o $(CONFIG)/obj/convenience.o $(CONFIG)/obj/dirHandler.o $(CONFIG)/obj/fileHandler.o $(CONFIG)/obj/log.o $(CONFIG)/obj/server.o $(LIBS) -lmpr -lhttp -lpcre -lpcre

$(CONFIG)/inc/edi.h: 
	rm -fr macosx-x86_64-debug/inc/edi.h
	cp -r src/esp/edi.h macosx-x86_64-debug/inc/edi.h

$(CONFIG)/inc/esp-app.h: 
	rm -fr macosx-x86_64-debug/inc/esp-app.h
	cp -r src/esp/esp-app.h macosx-x86_64-debug/inc/esp-app.h

$(CONFIG)/inc/esp.h: 
	rm -fr macosx-x86_64-debug/inc/esp.h
	cp -r src/esp/esp.h macosx-x86_64-debug/inc/esp.h

$(CONFIG)/inc/mdb.h: 
	rm -fr macosx-x86_64-debug/inc/mdb.h
	cp -r src/esp/mdb.h macosx-x86_64-debug/inc/mdb.h

$(CONFIG)/obj/edi.o: \
        src/esp/edi.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/edi.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/edi.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/edi.c

$(CONFIG)/obj/espAbbrev.o: \
        src/esp/espAbbrev.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/esp.h
	$(CC) -c -o $(CONFIG)/obj/espAbbrev.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/espAbbrev.c

$(CONFIG)/obj/espFramework.o: \
        src/esp/espFramework.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/esp.h
	$(CC) -c -o $(CONFIG)/obj/espFramework.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/espFramework.c

$(CONFIG)/obj/espHandler.o: \
        src/esp/espHandler.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h \
        $(CONFIG)/inc/esp.h \
        $(CONFIG)/inc/edi.h
	$(CC) -c -o $(CONFIG)/obj/espHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/espHandler.c

$(CONFIG)/obj/espHtml.o: \
        src/esp/espHtml.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/esp.h \
        $(CONFIG)/inc/edi.h
	$(CC) -c -o $(CONFIG)/obj/espHtml.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/espHtml.c

$(CONFIG)/obj/espSession.o: \
        src/esp/espSession.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/esp.h
	$(CC) -c -o $(CONFIG)/obj/espSession.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/espSession.c

$(CONFIG)/obj/espTemplate.o: \
        src/esp/espTemplate.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/esp.h
	$(CC) -c -o $(CONFIG)/obj/espTemplate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/espTemplate.c

$(CONFIG)/obj/mdb.o: \
        src/esp/mdb.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h \
        $(CONFIG)/inc/edi.h \
        $(CONFIG)/inc/mdb.h \
        $(CONFIG)/inc/pcre.h
	$(CC) -c -o $(CONFIG)/obj/mdb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/mdb.c

$(CONFIG)/obj/sdb.o: \
        src/esp/sdb.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h \
        $(CONFIG)/inc/edi.h
	$(CC) -c -o $(CONFIG)/obj/sdb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/sdb.c

$(CONFIG)/lib/mod_esp.dylib:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/inc/edi.h \
        $(CONFIG)/inc/esp-app.h \
        $(CONFIG)/inc/esp.h \
        $(CONFIG)/inc/mdb.h \
        $(CONFIG)/obj/edi.o \
        $(CONFIG)/obj/espAbbrev.o \
        $(CONFIG)/obj/espFramework.o \
        $(CONFIG)/obj/espHandler.o \
        $(CONFIG)/obj/espHtml.o \
        $(CONFIG)/obj/espSession.o \
        $(CONFIG)/obj/espTemplate.o \
        $(CONFIG)/obj/mdb.o \
        $(CONFIG)/obj/sdb.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/mod_esp.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/mod_esp.dylib $(CONFIG)/obj/edi.o $(CONFIG)/obj/espAbbrev.o $(CONFIG)/obj/espFramework.o $(CONFIG)/obj/espHandler.o $(CONFIG)/obj/espHtml.o $(CONFIG)/obj/espSession.o $(CONFIG)/obj/espTemplate.o $(CONFIG)/obj/mdb.o $(CONFIG)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/esp.o: \
        src/esp/esp.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/esp.h
	$(CC) -c -o $(CONFIG)/obj/esp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/esp/esp.c

$(CONFIG)/bin/esp:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/edi.o \
        $(CONFIG)/obj/esp.o \
        $(CONFIG)/obj/espAbbrev.o \
        $(CONFIG)/obj/espFramework.o \
        $(CONFIG)/obj/espHandler.o \
        $(CONFIG)/obj/espHtml.o \
        $(CONFIG)/obj/espSession.o \
        $(CONFIG)/obj/espTemplate.o \
        $(CONFIG)/obj/mdb.o \
        $(CONFIG)/obj/sdb.o
	$(CC) -o $(CONFIG)/bin/esp -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/edi.o $(CONFIG)/obj/esp.o $(CONFIG)/obj/espAbbrev.o $(CONFIG)/obj/espFramework.o $(CONFIG)/obj/espHandler.o $(CONFIG)/obj/espHtml.o $(CONFIG)/obj/espSession.o $(CONFIG)/obj/espTemplate.o $(CONFIG)/obj/mdb.o $(CONFIG)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/lib/esp.conf: 
	rm -fr macosx-x86_64-debug/lib/esp.conf
	cp -r src/esp/esp.conf macosx-x86_64-debug/lib/esp.conf

$(CONFIG)/lib/esp-www: 
	rm -fr macosx-x86_64-debug/lib/esp-www
	cp -r src/esp/www macosx-x86_64-debug/lib/esp-www

$(CONFIG)/obj/cgiHandler.o: \
        src/modules/cgiHandler.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/cgiHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/modules/cgiHandler.c

$(CONFIG)/lib/mod_cgi.dylib:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/cgiHandler.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/mod_cgi.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/mod_cgi.dylib $(CONFIG)/obj/cgiHandler.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/auth.o: \
        src/utils/auth.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/http.h
	$(CC) -c -o $(CONFIG)/obj/auth.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/utils/auth.c

$(CONFIG)/bin/auth:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/obj/auth.o
	$(CC) -o $(CONFIG)/bin/auth -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/auth.o $(LIBS) -lmpr

$(CONFIG)/obj/cgiProgram.o: \
        src/utils/cgiProgram.c \
        $(CONFIG)/inc/buildConfig.h
	$(CC) -c -o $(CONFIG)/obj/cgiProgram.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/utils/cgiProgram.c

$(CONFIG)/bin/cgiProgram:  \
        $(CONFIG)/obj/cgiProgram.o
	$(CC) -o $(CONFIG)/bin/cgiProgram -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/cgiProgram.o $(LIBS)

$(CONFIG)/obj/setConfig.o: \
        src/utils/setConfig.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/mpr.h
	$(CC) -c -o $(CONFIG)/obj/setConfig.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/utils/setConfig.c

$(CONFIG)/bin/setConfig:  \
        $(CONFIG)/lib/libmpr.dylib \
        $(CONFIG)/obj/setConfig.o
	$(CC) -o $(CONFIG)/bin/setConfig -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/setConfig.o $(LIBS) -lmpr

$(CONFIG)/inc/appwebMonitor.h: 
	rm -fr macosx-x86_64-debug/inc/appwebMonitor.h
	cp -r src/server/appwebMonitor.h macosx-x86_64-debug/inc/appwebMonitor.h

$(CONFIG)/obj/appweb.o: \
        src/server/appweb.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/appweb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/server/appweb.c

$(CONFIG)/bin/appweb:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/inc/appwebMonitor.h \
        $(CONFIG)/obj/appweb.o
	$(CC) -o $(CONFIG)/bin/appweb -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/appweb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/simpleServer.o: \
        src/samples/c/simpleServer/simpleServer.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/simpleServer.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/samples/c/simpleServer/simpleServer.c

$(CONFIG)/bin/simpleServer:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/simpleServer.o
	$(CC) -o $(CONFIG)/bin/simpleServer -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/simpleServer.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/simpleHandler.o: \
        src/samples/c/simpleHandler/simpleHandler.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/simpleHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/samples/c/simpleHandler/simpleHandler.c

$(CONFIG)/lib/simpleHandler.dylib:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/simpleHandler.o
	$(CC) -dynamiclib -o $(CONFIG)/lib/simpleHandler.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/simpleHandler.dylib $(CONFIG)/obj/simpleHandler.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/simpleClient.o: \
        src/samples/c/simpleClient/simpleClient.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/simpleClient.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/samples/c/simpleClient/simpleClient.c

$(CONFIG)/bin/simpleClient:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/simpleClient.o
	$(CC) -o $(CONFIG)/bin/simpleClient -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/simpleClient.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/cppHandler.o: \
        src/samples/cpp/cppHandler/cppHandler.cpp \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/cppHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/samples/cpp/cppHandler/cppHandler.cpp

src/samples/cpp/cppHandler/cppModule.dylib:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/cppHandler.o
	$(CC) -dynamiclib -o src/samples/cpp/cppHandler/cppModule.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/cppModule.dylib $(CONFIG)/obj/cppHandler.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/obj/cppModule.o: \
        src/samples/cpp/cppModule/cppModule.cpp \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/appweb.h
	$(CC) -c -o $(CONFIG)/obj/cppModule.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc src/samples/cpp/cppModule/cppModule.cpp

src/samples/cpp/cppModule/cppModule.dylib:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/obj/cppModule.o
	$(CC) -dynamiclib -o src/samples/cpp/cppModule/cppModule.dylib -arch x86_64 $(LDFLAGS) $(LIBPATHS) -install_name @rpath/cppModule.dylib $(CONFIG)/obj/cppModule.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(CONFIG)/inc/testAppweb.h: 
	rm -fr macosx-x86_64-debug/inc/testAppweb.h
	cp -r test/testAppweb.h macosx-x86_64-debug/inc/testAppweb.h

$(CONFIG)/obj/testAppweb.o: \
        test/testAppweb.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/testAppweb.h
	$(CC) -c -o $(CONFIG)/obj/testAppweb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc test/testAppweb.c

$(CONFIG)/obj/testHttp.o: \
        test/testHttp.c \
        $(CONFIG)/inc/buildConfig.h \
        $(CONFIG)/inc/testAppweb.h
	$(CC) -c -o $(CONFIG)/obj/testHttp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(CONFIG)/inc test/testHttp.c

$(CONFIG)/bin/testAppweb:  \
        $(CONFIG)/lib/libappweb.dylib \
        $(CONFIG)/inc/testAppweb.h \
        $(CONFIG)/obj/testAppweb.o \
        $(CONFIG)/obj/testHttp.o
	$(CC) -o $(CONFIG)/bin/testAppweb -arch x86_64 $(LDFLAGS) $(LIBPATHS) $(CONFIG)/obj/testAppweb.o $(CONFIG)/obj/testHttp.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

test/cgi-bin/testScript: 
	echo '#!$(CONFIG)/bin/cgiProgram' >test/cgi-bin/testScript ; chmod +x test/cgi-bin/testScript

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
	cp $(CONFIG)/bin/cgiProgram test/cgi-bin/cgiProgram
	cp $(CONFIG)/bin/cgiProgram test/cgi-bin/nph-cgiProgram
	cp $(CONFIG)/bin/cgiProgram 'test/cgi-bin/cgi Program'
	cp $(CONFIG)/bin/cgiProgram test/web/cgiProgram.cgi
	chmod +x test/cgi-bin/* test/web/cgiProgram.cgi

