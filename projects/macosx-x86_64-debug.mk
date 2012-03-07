#
#   build.mk -- Build It Makefile to build Embedthis Appweb for macosx on x86_64
#

PLATFORM  := macosx-x86_64-debug
CC        := /usr/bin/cc
CFLAGS    := -fPIC -Wall -g -Wshorten-64-to-32
DFLAGS    := -DPIC -DCPU=X86_64
IFLAGS    := -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc
LDFLAGS   := -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/appweb/$(PLATFORM)/lib -g -ldl
LIBS      := -lpthread -lm

all: prep \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/bin/manager \
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
        $(PLATFORM)/lib/mod_php.dylib \
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
	@if [ ! -x $(PLATFORM)/inc ] ; then \
		mkdir -p $(PLATFORM)/inc $(PLATFORM)/obj $(PLATFORM)/lib $(PLATFORM)/bin ; \
		cp src/buildConfig.default $(PLATFORM)/inc/buildConfig.h ; \
	fi

clean:
	rm -rf $(PLATFORM)/lib/libmpr.dylib
	rm -rf $(PLATFORM)/lib/libmprssl.dylib
	rm -rf $(PLATFORM)/bin/manager
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

$(PLATFORM)/obj/mprLib.o: \
        src/deps/mpr/mprLib.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/mprLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/mprLib.c

$(PLATFORM)/lib/libmpr.dylib:  \
        $(PLATFORM)/obj/mprLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libmpr.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/libmpr.dylib $(PLATFORM)/obj/mprLib.o $(LIBS)

$(PLATFORM)/obj/manager.o: \
        src/deps/mpr/manager.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/manager.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/manager.c

$(PLATFORM)/bin/manager:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/manager.o
	$(CC) -o $(PLATFORM)/bin/manager -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/manager.o $(LIBS) -lmpr

$(PLATFORM)/obj/makerom.o: \
        src/deps/mpr/makerom.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/makerom.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/makerom.c

$(PLATFORM)/bin/makerom:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/makerom.o
	$(CC) -o $(PLATFORM)/bin/makerom -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/makerom.o $(LIBS) -lmpr

$(PLATFORM)/obj/pcre.o: \
        src/deps/pcre/pcre.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/pcre/pcre.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/pcre.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/pcre/pcre.c

$(PLATFORM)/lib/libpcre.dylib:  \
        $(PLATFORM)/obj/pcre.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libpcre.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/libpcre.dylib $(PLATFORM)/obj/pcre.o $(LIBS)

$(PLATFORM)/obj/httpLib.o: \
        src/deps/http/httpLib.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/http/http.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/httpLib.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/http/httpLib.c

$(PLATFORM)/lib/libhttp.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/obj/httpLib.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libhttp.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/libhttp.dylib $(PLATFORM)/obj/httpLib.o $(LIBS) -lmpr -lpcre

$(PLATFORM)/obj/http.o: \
        src/deps/http/http.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/http/http.h
	$(CC) -c -o $(PLATFORM)/obj/http.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/http/http.c

$(PLATFORM)/bin/http:  \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/obj/http.o
	$(CC) -o $(PLATFORM)/bin/http -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/http.o $(LIBS) -lhttp -lmpr -lpcre

$(PLATFORM)/obj/sqlite3.o: \
        src/deps/sqlite/sqlite3.c \
        $(PLATFORM)/inc/buildConfig.h \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/sqlite/sqlite3.h
	$(CC) -c -o $(PLATFORM)/obj/sqlite3.o -arch x86_64 -fPIC -g $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/sqlite/sqlite3.c

$(PLATFORM)/lib/libsqlite3.dylib:  \
        $(PLATFORM)/obj/sqlite3.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libsqlite3.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/libsqlite3.dylib $(PLATFORM)/obj/sqlite3.o $(LIBS)

$(PLATFORM)/obj/config.o: \
        src/config.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/config.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/config.c

$(PLATFORM)/obj/convenience.o: \
        src/convenience.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/convenience.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/convenience.c

$(PLATFORM)/obj/dirHandler.o: \
        src/dirHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/dirHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/dirHandler.c

$(PLATFORM)/obj/fileHandler.o: \
        src/fileHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/fileHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/fileHandler.c

$(PLATFORM)/obj/log.o: \
        src/log.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/log.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/log.c

$(PLATFORM)/obj/server.o: \
        src/server.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/server.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/server.c

$(PLATFORM)/lib/libappweb.dylib:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/lib/libhttp.dylib \
        $(PLATFORM)/lib/libpcre.dylib \
        $(PLATFORM)/obj/config.o \
        $(PLATFORM)/obj/convenience.o \
        $(PLATFORM)/obj/dirHandler.o \
        $(PLATFORM)/obj/fileHandler.o \
        $(PLATFORM)/obj/log.o \
        $(PLATFORM)/obj/server.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/libappweb.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/libappweb.dylib $(PLATFORM)/obj/config.o $(PLATFORM)/obj/convenience.o $(PLATFORM)/obj/dirHandler.o $(PLATFORM)/obj/fileHandler.o $(PLATFORM)/obj/log.o $(PLATFORM)/obj/server.o $(LIBS) -lmpr -lhttp -lpcre -lpcre

$(PLATFORM)/obj/edi.o: \
        src/esp/edi.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/edi.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/edi.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/edi.c

$(PLATFORM)/obj/espAbbrev.o: \
        src/esp/espAbbrev.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espAbbrev.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espAbbrev.c

$(PLATFORM)/obj/espFramework.o: \
        src/esp/espFramework.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espFramework.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espFramework.c

$(PLATFORM)/obj/espHandler.o: \
        src/esp/espHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h \
        src/esp/esp.h \
        src/esp/edi.h
	$(CC) -c -o $(PLATFORM)/obj/espHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espHandler.c

$(PLATFORM)/obj/espHtml.o: \
        src/esp/espHtml.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/esp.h \
        src/esp/edi.h
	$(CC) -c -o $(PLATFORM)/obj/espHtml.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espHtml.c

$(PLATFORM)/obj/espSession.o: \
        src/esp/espSession.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espSession.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espSession.c

$(PLATFORM)/obj/espTemplate.o: \
        src/esp/espTemplate.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/esp.h
	$(CC) -c -o $(PLATFORM)/obj/espTemplate.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espTemplate.c

$(PLATFORM)/obj/mdb.o: \
        src/esp/mdb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h \
        src/esp/edi.h \
        src/esp/mdb.h \
        src/deps/pcre/pcre.h
	$(CC) -c -o $(PLATFORM)/obj/mdb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/mdb.c

$(PLATFORM)/obj/sdb.o: \
        src/esp/sdb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h \
        src/esp/edi.h
	$(CC) -c -o $(PLATFORM)/obj/sdb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/sdb.c

$(PLATFORM)/lib/mod_esp.dylib:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/edi.o \
        $(PLATFORM)/obj/espAbbrev.o \
        $(PLATFORM)/obj/espFramework.o \
        $(PLATFORM)/obj/espHandler.o \
        $(PLATFORM)/obj/espHtml.o \
        $(PLATFORM)/obj/espSession.o \
        $(PLATFORM)/obj/espTemplate.o \
        $(PLATFORM)/obj/mdb.o \
        $(PLATFORM)/obj/sdb.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_esp.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/mod_esp.dylib $(PLATFORM)/obj/edi.o $(PLATFORM)/obj/espAbbrev.o $(PLATFORM)/obj/espFramework.o $(PLATFORM)/obj/espHandler.o $(PLATFORM)/obj/espHtml.o $(PLATFORM)/obj/espSession.o $(PLATFORM)/obj/espTemplate.o $(PLATFORM)/obj/mdb.o $(PLATFORM)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/esp.o: \
        src/esp/esp.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/esp/esp.h
	$(CC) -c -o $(PLATFORM)/obj/esp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/esp.c

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
	$(CC) -o $(PLATFORM)/bin/esp -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/edi.o $(PLATFORM)/obj/esp.o $(PLATFORM)/obj/espAbbrev.o $(PLATFORM)/obj/espFramework.o $(PLATFORM)/obj/espHandler.o $(PLATFORM)/obj/espHtml.o $(PLATFORM)/obj/espSession.o $(PLATFORM)/obj/espTemplate.o $(PLATFORM)/obj/mdb.o $(PLATFORM)/obj/sdb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

macosx-x86_64-debug/lib/esp.conf: 
	rm -fr /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp.conf
	cp -r /Users/mob/git/appweb/src/esp/esp.conf /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp.conf

macosx-x86_64-debug/lib/esp-www: 
	rm -fr /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp-www
	cp -r /Users/mob/git/appweb/src/esp/www /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp-www

$(PLATFORM)/obj/cgiHandler.o: \
        src/modules/cgiHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/cgiHandler.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/modules/cgiHandler.c

$(PLATFORM)/lib/mod_cgi.dylib:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/cgiHandler.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_cgi.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -install_name @rpath/mod_cgi.dylib $(PLATFORM)/obj/cgiHandler.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/phpHandler.o: \
        src/modules/phpHandler.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/phpHandler.o -arch x86_64 -fPIC -g $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -I../packages-macosx-x86_64/php/php-5.3.8 -I../packages-macosx-x86_64/php/php-5.3.8/main -I../packages-macosx-x86_64/php/php-5.3.8/Zend -I../packages-macosx-x86_64/php/php-5.3.8/TSRM src/modules/phpHandler.c

$(PLATFORM)/lib/mod_php.dylib:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/phpHandler.o
	$(CC) -dynamiclib -o $(PLATFORM)/lib/mod_php.dylib -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L/Users/mob/git/packages-macosx-x86_64/php/php-5.3.8/.libs -install_name @rpath/mod_php.dylib $(PLATFORM)/obj/phpHandler.o $(LIBS) -lphp5 -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/auth.o: \
        src/utils/auth.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/http/http.h
	$(CC) -c -o $(PLATFORM)/obj/auth.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/auth.c

$(PLATFORM)/bin/auth:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/auth.o
	$(CC) -o $(PLATFORM)/bin/auth -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/auth.o $(LIBS) -lmpr

$(PLATFORM)/obj/cgiProgram.o: \
        src/utils/cgiProgram.c \
        $(PLATFORM)/inc/buildConfig.h
	$(CC) -c -o $(PLATFORM)/obj/cgiProgram.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/cgiProgram.c

$(PLATFORM)/bin/cgiProgram:  \
        $(PLATFORM)/obj/cgiProgram.o
	$(CC) -o $(PLATFORM)/bin/cgiProgram -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/cgiProgram.o $(LIBS)

$(PLATFORM)/obj/setConfig.o: \
        src/utils/setConfig.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/deps/mpr/mpr.h
	$(CC) -c -o $(PLATFORM)/obj/setConfig.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/setConfig.c

$(PLATFORM)/bin/setConfig:  \
        $(PLATFORM)/lib/libmpr.dylib \
        $(PLATFORM)/obj/setConfig.o
	$(CC) -o $(PLATFORM)/bin/setConfig -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/setConfig.o $(LIBS) -lmpr

$(PLATFORM)/obj/appweb.o: \
        src/server/appweb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/appweb.h
	$(CC) -c -o $(PLATFORM)/obj/appweb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/server/appweb.c

$(PLATFORM)/bin/appweb:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/appweb.o
	$(CC) -o $(PLATFORM)/bin/appweb -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/appweb.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

$(PLATFORM)/obj/testAppweb.o: \
        src/test/testAppweb.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/test/testAppweb.h
	$(CC) -c -o $(PLATFORM)/obj/testAppweb.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Isrc/test src/test/testAppweb.c

$(PLATFORM)/obj/testHttp.o: \
        src/test/testHttp.c \
        $(PLATFORM)/inc/buildConfig.h \
        src/test/testAppweb.h
	$(CC) -c -o $(PLATFORM)/obj/testHttp.o -arch x86_64 $(CFLAGS) $(DFLAGS) -I$(PLATFORM)/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Isrc/test src/test/testHttp.c

$(PLATFORM)/bin/testAppweb:  \
        $(PLATFORM)/lib/libappweb.dylib \
        $(PLATFORM)/obj/testAppweb.o \
        $(PLATFORM)/obj/testHttp.o
	$(CC) -o $(PLATFORM)/bin/testAppweb -arch x86_64 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L$(PLATFORM)/lib -g -ldl -L$(PLATFORM)/lib $(PLATFORM)/obj/testAppweb.o $(PLATFORM)/obj/testHttp.o $(LIBS) -lappweb -lmpr -lhttp -lpcre

src/test/cgi-bin/testScript: 
	echo '#!/Users/mob/git/appweb/macosx-x86_64-debug/bin/cgiProgram' >/Users/mob/git/appweb/src/test/cgi-bin/testScript ; chmod +x /Users/mob/git/appweb/src/test/cgi-bin/testScript

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
	cp /Users/mob/git/appweb/macosx-x86_64-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/cgiProgram
	cp /Users/mob/git/appweb/macosx-x86_64-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/nph-cgiProgram
	cp /Users/mob/git/appweb/macosx-x86_64-debug/bin/cgiProgram '/Users/mob/git/appweb/src/test/cgi-bin/cgi Program'
	cp /Users/mob/git/appweb/macosx-x86_64-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
	chmod +x /Users/mob/git/appweb/src/test/cgi-bin/* /Users/mob/git/appweb/src/test/web/cgiProgram.cgi

