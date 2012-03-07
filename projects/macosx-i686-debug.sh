#
#   build.sh -- Build It Shell Script to build Embedthis Appweb
#

PLATFORM="macosx-i686-debug"
CC="cc"
CFLAGS="-fPIC -Wall -g"
DFLAGS="-DPIC -DCPU=I686"
IFLAGS="-Imacosx-i686-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc"
LDFLAGS="-Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L/Users/mob/git/appweb/${PLATFORM}/lib -g"
LIBS="-lpthread -lm"

${CC} -c -o ${PLATFORM}/obj/mprLib.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/mprLib.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libmpr.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/libmpr.dylib ${PLATFORM}/obj/mprLib.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/manager.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/manager.c

${CC} -o ${PLATFORM}/bin/manager -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/manager.o ${LIBS} -lmpr

${CC} -c -o ${PLATFORM}/obj/makerom.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/makerom.c

${CC} -o ${PLATFORM}/bin/makerom -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/makerom.o ${LIBS} -lmpr

${CC} -c -o ${PLATFORM}/obj/pcre.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/pcre/pcre.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libpcre.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/libpcre.dylib ${PLATFORM}/obj/pcre.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/httpLib.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/http/httpLib.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libhttp.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/libhttp.dylib ${PLATFORM}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/http.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/http/http.c

${CC} -o ${PLATFORM}/bin/http -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/sqlite3.o -arch i686 -fPIC -g ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/sqlite/sqlite3.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libsqlite3.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/libsqlite3.dylib ${PLATFORM}/obj/sqlite3.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/config.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/config.c

${CC} -c -o ${PLATFORM}/obj/convenience.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/convenience.c

${CC} -c -o ${PLATFORM}/obj/dirHandler.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/dirHandler.c

${CC} -c -o ${PLATFORM}/obj/fileHandler.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/fileHandler.c

${CC} -c -o ${PLATFORM}/obj/log.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/log.c

${CC} -c -o ${PLATFORM}/obj/server.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/server.c

${CC} -dynamiclib -o ${PLATFORM}/lib/libappweb.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/libappweb.dylib ${PLATFORM}/obj/config.o ${PLATFORM}/obj/convenience.o ${PLATFORM}/obj/dirHandler.o ${PLATFORM}/obj/fileHandler.o ${PLATFORM}/obj/log.o ${PLATFORM}/obj/server.o ${LIBS} -lmpr -lhttp -lpcre -lpcre

${CC} -c -o ${PLATFORM}/obj/edi.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/edi.c

${CC} -c -o ${PLATFORM}/obj/espAbbrev.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espAbbrev.c

${CC} -c -o ${PLATFORM}/obj/espFramework.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espFramework.c

${CC} -c -o ${PLATFORM}/obj/espHandler.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espHandler.c

${CC} -c -o ${PLATFORM}/obj/espHtml.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espHtml.c

${CC} -c -o ${PLATFORM}/obj/espSession.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espSession.c

${CC} -c -o ${PLATFORM}/obj/espTemplate.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espTemplate.c

${CC} -c -o ${PLATFORM}/obj/mdb.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/mdb.c

${CC} -c -o ${PLATFORM}/obj/sdb.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/sdb.c

${CC} -dynamiclib -o ${PLATFORM}/lib/mod_esp.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/mod_esp.dylib ${PLATFORM}/obj/edi.o ${PLATFORM}/obj/espAbbrev.o ${PLATFORM}/obj/espFramework.o ${PLATFORM}/obj/espHandler.o ${PLATFORM}/obj/espHtml.o ${PLATFORM}/obj/espSession.o ${PLATFORM}/obj/espTemplate.o ${PLATFORM}/obj/mdb.o ${PLATFORM}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${PLATFORM}/obj/esp.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/esp.c

${CC} -o ${PLATFORM}/bin/esp -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/edi.o ${PLATFORM}/obj/esp.o ${PLATFORM}/obj/espAbbrev.o ${PLATFORM}/obj/espFramework.o ${PLATFORM}/obj/espHandler.o ${PLATFORM}/obj/espHtml.o ${PLATFORM}/obj/espSession.o ${PLATFORM}/obj/espTemplate.o ${PLATFORM}/obj/mdb.o ${PLATFORM}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

rm -rf /Users/mob/git/appweb/macosx-i686-debug/lib/esp.conf

cp -r /Users/mob/git/appweb/src/esp/esp.conf /Users/mob/git/appweb/macosx-i686-debug/lib/esp.conf

rm -rf /Users/mob/git/appweb/macosx-i686-debug/lib/esp-www

cp -r /Users/mob/git/appweb/src/esp/www /Users/mob/git/appweb/macosx-i686-debug/lib/esp-www

${CC} -c -o ${PLATFORM}/obj/cgiHandler.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/modules/cgiHandler.c

${CC} -dynamiclib -o ${PLATFORM}/lib/mod_cgi.dylib -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -install_name @rpath/mod_cgi.dylib ${PLATFORM}/obj/cgiHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${PLATFORM}/obj/auth.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/auth.c

${CC} -o ${PLATFORM}/bin/auth -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/auth.o ${LIBS} -lmpr

${CC} -c -o ${PLATFORM}/obj/cgiProgram.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/cgiProgram.c

${CC} -o ${PLATFORM}/bin/cgiProgram -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/cgiProgram.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/setConfig.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/setConfig.c

${CC} -o ${PLATFORM}/bin/setConfig -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/setConfig.o ${LIBS} -lmpr

${CC} -c -o ${PLATFORM}/obj/appweb.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/server/appweb.c

${CC} -o ${PLATFORM}/bin/appweb -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/appweb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${PLATFORM}/obj/testAppweb.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Isrc/test src/test/testAppweb.c

${CC} -c -o ${PLATFORM}/obj/testHttp.o -arch i686 ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Isrc/test src/test/testHttp.c

${CC} -o ${PLATFORM}/bin/testAppweb -arch i686 -Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/testAppweb.o ${PLATFORM}/obj/testHttp.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

echo '#!/Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram' >/Users/mob/git/appweb/src/test/cgi-bin/testScript ; chmod +x /Users/mob/git/appweb/src/test/cgi-bin/testScript
echo -e '#!`type -p sh`' >/Users/mob/git/appweb/src/test/web/caching/cache.cgi
echo -e '' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
echo -e 'echo HTTP/1.0 200 OK' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
echo -e 'echo Content-Type: text/plain' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
echo -e 'date' >>/Users/mob/git/appweb/src/test/web/caching/cache.cgi
chmod +x /Users/mob/git/appweb/src/test/web/caching/cache.cgi
echo -e '#!`type -p sh`' >/Users/mob/git/appweb/src/test/web/basic/basic.cgi
echo -e '' >>/Users/mob/git/appweb/src/test/web/basic/basic.cgi
echo -e 'echo Content-Type: text/plain' >>/Users/mob/git/appweb/src/test/web/basic/basic.cgi
echo -e '/usr/bin/env' >>/Users/mob/git/appweb/src/test/web/basic/basic.cgi
chmod +x /Users/mob/git/appweb/src/test/web/basic/basic.cgi
cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/cgiProgram
cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/nph-cgiProgram
cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram '/Users/mob/git/appweb/src/test/cgi-bin/cgi Program'
cp /Users/mob/git/appweb/macosx-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
chmod +x /Users/mob/git/appweb/src/test/cgi-bin/* /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
