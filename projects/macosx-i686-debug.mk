#
#   build.mk -- Build It Makefile to build Embedthis Appweb for macosx on i686
#

PLATFORM  := macosx-i686-debug
CC        := cc
CFLAGS    := -fPIC -Wall -g
DFLAGS    := -DPIC -DCPU=I686
IFLAGS    := -I$(PLATFORM)/inc
LDFLAGS   := -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/appweb/$(PLATFORM)/lib -g
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
        src/test/cgi-bin/testScript \
        src/test/web/caching/cache.cgi \
        src/test/web/basic/basic.cgi \
        src/test/cgi-bin/cgiProgram

.PHONY: prep

prep:
	@[ ! -x $(PLATFORM)/inc ] && mkdir -p $(PLATFORM)/inc $(PLATFORM)/obj $(PLATFORM)/lib $(PLATFORM)/bin ; true
	@[ ! -f $(PLATFORM)/inc/buildConfig.h ] && cp src/buildConfig.default $(PLATFORM)/inc/buildConfig.h ; true

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
	rm -rf $(PLATFORM)/bin/testAppweb
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
	rm -rf $(PLATFORM)/obj/testAppweb.o
	rm -rf $(PLATFORM)/obj/testHttp.o

$(PLATFORM)/inc/mpr.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/mpr.h
	cp -r /Users/mob/git/appweb/src/deps/mpr/mpr.h /Users/mob/git/appweb/macosx-i686-debug/inc/mpr.h

$(PLATFORM)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/mprLib.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/mprLib.c

$(PLATFORM)/lib/libmpr.dylib:  \
        $(PLATFORM)/inc/mpr.h \
        $(PLATFORM)/obj/mprLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libmpr.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libmpr.dylib $(PLATFORM)/obj/mprLib.o $(LIBS)

$(PLATFORM)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/manager.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/manager.c

$(PLATFORM)/bin/appman:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/manager.o
	$(CC) -o $(PLATFORM)/bin/appman -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/manager.o $(LIBS) -lmpr

$(PLATFORM)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/makerom.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/mpr/makerom.c

$(PLATFORM)/bin/makerom:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/makerom.o
	$(CC) -o $(PLATFORM)/bin/makerom -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/makerom.o $(LIBS) -lmpr

$(PLATFORM)/inc/pcre.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/pcre.h
	cp -r /Users/mob/git/appweb/src/deps/pcre/pcre.h /Users/mob/git/appweb/macosx-i686-debug/inc/pcre.h

$(PLATFORM)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/pcre.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/pcre/pcre.c

$(PLATFORM)/lib/libpcre.dylib:  \
        $(PLATFORM)/inc/pcre.h \
        $(PLATFORM)/obj/pcre.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libpcre.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libpcre.dylib $(PLATFORM)/obj/pcre.o $(LIBS)

$(PLATFORM)/inc/http.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/http.h
	cp -r /Users/mob/git/appweb/src/deps/http/http.h /Users/mob/git/appweb/macosx-i686-debug/inc/http.h

$(PLATFORM)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/httpLib.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/httpLib.c

$(PLATFORM)/lib/libhttp.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/inc/http.h \
        $(PLATFORM)/obj/httpLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libhttp.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libhttp.dylib $(PLATFORM)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(PLATFORM)/obj/http.o: \
        src/deps/http/http.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/http.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/deps/http/http.c

$(PLATFORM)/bin/http:  \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/obj/http.o
	$(CC) -o $(PLATFORM)/bin/http -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre

$(PLATFORM)/inc/sqlite3.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/sqlite3.h
	cp -r /Users/mob/git/appweb/src/deps/sqlite/sqlite3.h /Users/mob/git/appweb/macosx-i686-debug/inc/sqlite3.h

$(PLATFORM)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/sqlite3.o -arch i686 -fPIC -g $(DFLAGS) -I$(PLATFORM)/inc src/deps/sqlite/sqlite3.c

$(PLATFORM)/lib/libsqlite3.dylib:  \
        $(PLATFORM)/inc/sqlite3.h \
        $(PLATFORM)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libsqlite3.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libsqlite3.dylib $(PLATFORM)/obj/sqlite3.o $(LIBS)

$(PLATFORM)/inc/appweb.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/appweb.h
	cp -r /Users/mob/git/appweb/src/appweb.h /Users/mob/git/appweb/macosx-i686-debug/inc/appweb.h

$(PLATFORM)/inc/customize.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/customize.h
	cp -r /Users/mob/git/appweb/src/customize.h /Users/mob/git/appweb/macosx-i686-debug/inc/customize.h

$(PLATFORM)/obj/config.o: \
        src/config.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/config.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/config.c

$(PLATFORM)/obj/convenience.o: \
        src/convenience.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/convenience.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/convenience.c

$(PLATFORM)/obj/dirHandler.o: \
        src/dirHandler.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/dirHandler.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/dirHandler.c

$(PLATFORM)/obj/fileHandler.o: \
        src/fileHandler.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/fileHandler.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/fileHandler.c

$(PLATFORM)/obj/log.o: \
        src/log.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/log.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/log.c

$(PLATFORM)/obj/server.o: \
        src/server.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/server.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/server.c

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
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libappweb.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/libappweb.dylib $(PLATFORM)/obj/config.o $(PLATFORM)/obj/convenience.o $(PLATFORM)/obj/dirHandler.o $(PLATFORM)/obj/fileHandler.o $(PLATFORM)/obj/log.o $(PLATFORM)/obj/server.o $(LIBS) -lmpr -lhttp -lpcre -lpcre

$(PLATFORM)/inc/edi.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/edi.h
	cp -r /Users/mob/git/appweb/src/esp/edi.h /Users/mob/git/appweb/macosx-i686-debug/inc/edi.h

$(PLATFORM)/inc/esp-app.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/esp-app.h
	cp -r /Users/mob/git/appweb/src/esp/esp-app.h /Users/mob/git/appweb/macosx-i686-debug/inc/esp-app.h

$(PLATFORM)/inc/esp.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/esp.h
	cp -r /Users/mob/git/appweb/src/esp/esp.h /Users/mob/git/appweb/macosx-i686-debug/inc/esp.h

$(PLATFORM)/inc/mdb.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/mdb.h
	cp -r /Users/mob/git/appweb/src/esp/mdb.h /Users/mob/git/appweb/macosx-i686-debug/inc/mdb.h

$(PLATFORM)/obj/edi.o: \
        src/esp/edi.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/edi.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/edi.c

$(PLATFORM)/obj/espAbbrev.o: \
        src/esp/espAbbrev.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/espAbbrev.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espAbbrev.c

$(PLATFORM)/obj/espFramework.o: \
        src/esp/espFramework.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/espFramework.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espFramework.c

$(PLATFORM)/obj/espHandler.o: \
        src/esp/espHandler.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/espHandler.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espHandler.c

$(PLATFORM)/obj/espHtml.o: \
        src/esp/espHtml.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/espHtml.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espHtml.c

$(PLATFORM)/obj/espSession.o: \
        src/esp/espSession.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/espSession.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espSession.c

$(PLATFORM)/obj/espTemplate.o: \
        src/esp/espTemplate.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/espTemplate.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/espTemplate.c

$(PLATFORM)/obj/mdb.o: \
        src/esp/mdb.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/mdb.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/mdb.c

$(PLATFORM)/obj/sdb.o: \
        src/esp/sdb.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/sdb.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/sdb.c

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
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_esp.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/mod_esp.dylib $(PLATFORM)/obj/edi.o $(PLATFORM)/obj/espAbbrev.o $(PLATFORM)/obj/espFramework.o $(PLATFORM)/obj/espHandler.o $(PLATFORM)/obj/espHtml.o $(PLATFORM)/obj/espSession.o $(PLATFORM)/obj/espTemplate.o $(PLATFORM)/obj/mdb.o $(PLATFORM)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/esp.o: \
        src/esp/esp.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/esp.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/esp/esp.c

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
	$(CC) -o $(PLATFORM)/bin/esp -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/edi.o $(PLATFORM)/obj/esp.o $(PLATFORM)/obj/espAbbrev.o $(PLATFORM)/obj/espFramework.o $(PLATFORM)/obj/espHandler.o $(PLATFORM)/obj/espHtml.o $(PLATFORM)/obj/espSession.o $(PLATFORM)/obj/espTemplate.o $(PLATFORM)/obj/mdb.o $(PLATFORM)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/lib/esp.conf: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/lib/esp.conf
	cp -r /Users/mob/git/appweb/src/esp/esp.conf /Users/mob/git/appweb/macosx-i686-debug/lib/esp.conf

$(PLATFORM)/lib/esp-www: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/lib/esp-www
	cp -r /Users/mob/git/appweb/src/esp/www /Users/mob/git/appweb/macosx-i686-debug/lib/esp-www

$(PLATFORM)/obj/cgiHandler.o: \
        src/modules/cgiHandler.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/cgiHandler.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/modules/cgiHandler.c

$(PLATFORM)/lib/mod_cgi.dylib:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/cgiHandler.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_cgi.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -install_name @rpath/mod_cgi.dylib $(PLATFORM)/obj/cgiHandler.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/auth.o: \
        src/utils/auth.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/auth.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/utils/auth.c

$(PLATFORM)/bin/auth:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/auth.o
	$(CC) -o $(PLATFORM)/bin/auth -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/auth.o $(LIBS) -lmpr

$(PLATFORM)/obj/cgiProgram.o: \
        src/utils/cgiProgram.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/cgiProgram.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/utils/cgiProgram.c

$(PLATFORM)/bin/cgiProgram:  \
        $(PLATFORM)/obj/cgiProgram.o
	$(CC) -o $(PLATFORM)/bin/cgiProgram -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/cgiProgram.o $(LIBS)

$(PLATFORM)/obj/setConfig.o: \
        src/utils/setConfig.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/setConfig.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/utils/setConfig.c

$(PLATFORM)/bin/setConfig:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/setConfig.o
	$(CC) -o $(PLATFORM)/bin/setConfig -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/setConfig.o $(LIBS) -lmpr

$(PLATFORM)/inc/appwebMonitor.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/appwebMonitor.h
	cp -r /Users/mob/git/appweb/src/server/appwebMonitor.h /Users/mob/git/appweb/macosx-i686-debug/inc/appwebMonitor.h

$(PLATFORM)/obj/appweb.o: \
        src/server/appweb.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/appweb.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/server/appweb.c

$(PLATFORM)/bin/appweb:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/inc/appwebMonitor.h \
        $(PLATFORM)/obj/appweb.o
	$(CC) -o $(PLATFORM)/bin/appweb -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/appweb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/inc/testAppweb.h: 
	rm -fr /Users/mob/git/appweb/macosx-i686-debug/inc/testAppweb.h
	cp -r /Users/mob/git/appweb/src/test/testAppweb.h /Users/mob/git/appweb/macosx-i686-debug/inc/testAppweb.h

$(PLATFORM)/obj/testAppweb.o: \
        src/test/testAppweb.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/testAppweb.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/test/testAppweb.c

$(PLATFORM)/obj/testHttp.o: \
        src/test/testHttp.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/testHttp.o -arch i686 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc src/test/testHttp.c

$(PLATFORM)/bin/testAppweb:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/inc/testAppweb.h \
        $(PLATFORM)/obj/testAppweb.o \
        $(PLATFORM)/obj/testHttp.o
	$(CC) -o $(PLATFORM)/bin/testAppweb -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -L$(PLATFORM)/lib $(PLATFORM)/obj/testAppweb.o $(PLATFORM)/obj/testHttp.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

src/test/cgi-bin/testScript: 
	echo '#!/Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram' >/Users/mob/git/appweb/src/test/cgi-bin/testScript ; chmod +x /Users/mob/git/appweb/src/test/cgi-bin/testScript

src/test/web/caching/cache.cgi: 
	echo -e '#!`type -p sh`' >/Users/mob/git/appweb/src/test/web/caching/cache.cgi
	echo -e '' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
	echo -e 'echo HTTP/1.0 200 OK' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
	echo -e 'echo Content-Type: text/plain' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
	echo -e 'date' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
	chmod +x /Users/mob/git/appweb/src/test/web/caching/cache.cgi

src/test/web/basic/basic.cgi: 
	echo -e '#!`type -p sh`' >/Users/mob/git/appweb/src/test/web/basic/basic.cgi
	echo -e '' >>/Users/mob/git/appweb/src/test/web/basic/basic.cgi
	echo -e 'echo Content-Type: text/plain' >>/Users/mob/git/appweb/src/test/web/basic/basic.cgi
	echo -e '/usr/bin/env' >>/Users/mob/git/appweb/src/test/web/basic/basic.cgi
	chmod +x /Users/mob/git/appweb/src/test/web/basic/basic.cgi

src/test/cgi-bin/cgiProgram: 
	cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/cgiProgram
	cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/nph-cgiProgram
	cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram '/Users/mob/git/appweb/src/test/cgi-bin/cgi Program'
	cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
	chmod +x /Users/mob/git/appweb/src/test/cgi-bin/* /Users/mob/git/appweb/src/test/web/cgiProgram.cgi

