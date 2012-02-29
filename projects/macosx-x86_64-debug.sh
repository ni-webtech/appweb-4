#
#   build.sh -- Build It Shell Script to build Embedthis Appweb
#

CC="/usr/bin/cc"
CFLAGS="-fPIC -Wall -g -Wshorten-64-to-32"
DFLAGS="-DPIC -DCPU=X86_64"
IFLAGS="-Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc"
LDFLAGS="-Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/appweb/macosx-x86_64-debug/lib -g -ldl"
LIBS="-lpthread -lm"

${CC} -c -o macosx-x86_64-debug/obj/mprLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/mpr/mprLib.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libmpr.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libmpr.dylib macosx-x86_64-debug/obj/mprLib.o ${LIBS}

${CC} -c -o macosx-x86_64-debug/obj/manager.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/mpr/manager.c

${CC} -o macosx-x86_64-debug/bin/manager -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/manager.o ${LIBS} -lmpr

${CC} -c -o macosx-x86_64-debug/obj/makerom.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/mpr/makerom.c

${CC} -o macosx-x86_64-debug/bin/makerom -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/makerom.o ${LIBS} -lmpr

${CC} -c -o macosx-x86_64-debug/obj/pcre.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/pcre/pcre.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libpcre.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libpcre.dylib macosx-x86_64-debug/obj/pcre.o ${LIBS}

${CC} -c -o macosx-x86_64-debug/obj/httpLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/http/httpLib.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libhttp.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libhttp.dylib macosx-x86_64-debug/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o macosx-x86_64-debug/obj/http.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/http/http.c

${CC} -o macosx-x86_64-debug/bin/http -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/http.o ${LIBS} -lhttp -lmpr -lpcre

${CC} -c -o macosx-x86_64-debug/obj/sqlite3.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/deps/sqlite/sqlite3.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libsqlite3.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libsqlite3.dylib macosx-x86_64-debug/obj/sqlite3.o ${LIBS}

${CC} -c -o macosx-x86_64-debug/obj/config.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/config.c

${CC} -c -o macosx-x86_64-debug/obj/convenience.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/convenience.c

${CC} -c -o macosx-x86_64-debug/obj/dirHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/dirHandler.c

${CC} -c -o macosx-x86_64-debug/obj/fileHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/fileHandler.c

${CC} -c -o macosx-x86_64-debug/obj/log.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/log.c

${CC} -c -o macosx-x86_64-debug/obj/server.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/server.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/libappweb.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/libappweb.dylib macosx-x86_64-debug/obj/config.o macosx-x86_64-debug/obj/convenience.o macosx-x86_64-debug/obj/dirHandler.o macosx-x86_64-debug/obj/fileHandler.o macosx-x86_64-debug/obj/log.o macosx-x86_64-debug/obj/server.o ${LIBS} -lmpr -lhttp -lpcre -lpcre

${CC} -c -o macosx-x86_64-debug/obj/edi.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/edi.c

${CC} -c -o macosx-x86_64-debug/obj/espAbbrev.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/espAbbrev.c

${CC} -c -o macosx-x86_64-debug/obj/espFramework.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/espFramework.c

${CC} -c -o macosx-x86_64-debug/obj/espHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/espHandler.c

${CC} -c -o macosx-x86_64-debug/obj/espHtml.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/espHtml.c

${CC} -c -o macosx-x86_64-debug/obj/espSession.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/espSession.c

${CC} -c -o macosx-x86_64-debug/obj/espTemplate.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/espTemplate.c

${CC} -c -o macosx-x86_64-debug/obj/mdb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/mdb.c

${CC} -c -o macosx-x86_64-debug/obj/sdb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/sdb.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/mod_esp.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/mod_esp.dylib macosx-x86_64-debug/obj/edi.o macosx-x86_64-debug/obj/espAbbrev.o macosx-x86_64-debug/obj/espFramework.o macosx-x86_64-debug/obj/espHandler.o macosx-x86_64-debug/obj/espHtml.o macosx-x86_64-debug/obj/espSession.o macosx-x86_64-debug/obj/espTemplate.o macosx-x86_64-debug/obj/mdb.o macosx-x86_64-debug/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o macosx-x86_64-debug/obj/esp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/esp/esp.c

${CC} -o macosx-x86_64-debug/bin/esp -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/edi.o macosx-x86_64-debug/obj/esp.o macosx-x86_64-debug/obj/espAbbrev.o macosx-x86_64-debug/obj/espFramework.o macosx-x86_64-debug/obj/espHandler.o macosx-x86_64-debug/obj/espHtml.o macosx-x86_64-debug/obj/espSession.o macosx-x86_64-debug/obj/espTemplate.o macosx-x86_64-debug/obj/mdb.o macosx-x86_64-debug/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

rm -f /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp.conf

cp /Users/mob/git/appweb/src/esp/esp.conf /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp.conf

rm -f /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp-www

cp /Users/mob/git/appweb/src/esp/www /Users/mob/git/appweb/macosx-x86_64-debug/lib/esp-www

${CC} -c -o macosx-x86_64-debug/obj/cgiHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/modules/cgiHandler.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/mod_cgi.dylib -arch x86_64 ${LDFLAGS} -install_name @rpath/mod_cgi.dylib macosx-x86_64-debug/obj/cgiHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o macosx-x86_64-debug/obj/phpHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc -I../packages-macosx-x86_64/php/php-5.3.8 -I../packages-macosx-x86_64/php/php-5.3.8/main -I../packages-macosx-x86_64/php/php-5.3.8/Zend -I../packages-macosx-x86_64/php/php-5.3.8/TSRM src/modules/phpHandler.c

${CC} -dynamiclib -o macosx-x86_64-debug/lib/mod_php.dylib -arch x86_64 ${LDFLAGS} -L/Users/mob/git/packages-macosx-x86_64/php/php-5.3.8/.libs -install_name @rpath/mod_php.dylib macosx-x86_64-debug/obj/phpHandler.o ${LIBS} -lphp5 -lappweb -lmpr -lhttp -lpcre

${CC} -c -o macosx-x86_64-debug/obj/auth.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/utils/auth.c

${CC} -o macosx-x86_64-debug/bin/auth -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/auth.o ${LIBS} -lmpr

${CC} -c -o macosx-x86_64-debug/obj/cgiProgram.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/utils/cgiProgram.c

${CC} -o macosx-x86_64-debug/bin/cgiProgram -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/cgiProgram.o ${LIBS}

${CC} -c -o macosx-x86_64-debug/obj/setConfig.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/utils/setConfig.c

${CC} -o macosx-x86_64-debug/bin/setConfig -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/setConfig.o ${LIBS} -lmpr

${CC} -c -o macosx-x86_64-debug/obj/appweb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/server/appweb.c

${CC} -o macosx-x86_64-debug/bin/appweb -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/appweb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o macosx-x86_64-debug/obj/testAppweb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/test -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/test/testAppweb.c

${CC} -c -o macosx-x86_64-debug/obj/testHttp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -Isrc/test -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Imacosx-x86_64-debug/inc src/test/testHttp.c

${CC} -o macosx-x86_64-debug/bin/testAppweb -arch x86_64 ${LDFLAGS} -Lmacosx-x86_64-debug/lib macosx-x86_64-debug/obj/testAppweb.o macosx-x86_64-debug/obj/testHttp.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

#  Omit script /Users/mob/git/appweb/src/test/cgi-bin/testScript undefined
#  Omit script /Users/mob/git/appweb/src/test/web/caching/cache.cgi undefined
#  Omit script /Users/mob/git/appweb/src/test/web/basic/basic.cgi undefined
#  Omit script /Users/mob/git/appweb/src/test/cgi-bin/cgiProgram undefined
