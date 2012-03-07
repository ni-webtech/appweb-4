#
#   build.sh -- Build It Shell Script to build Embedthis Appweb
#

PLATFORM="linux-i686-debug"
CC="cc"
CFLAGS="-fPIC -g -mcpu=i686"
DFLAGS="-DPIC"
IFLAGS="-Ilinux-i686-debug/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc"
LDFLAGS="-L/Users/mob/git/appweb/${PLATFORM}/lib -g"
LIBS="-lpthread -lm"

${CC} -c -o ${PLATFORM}/obj/mprLib.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/mprLib.c

${CC} -shared -o ${PLATFORM}/lib/libmpr.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/mprLib.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/manager.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/manager.c

${CC} -o ${PLATFORM}/bin/manager -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/manager.o ${LIBS} -lmpr -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/makerom.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/mpr/makerom.c

${CC} -o ${PLATFORM}/bin/makerom -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/makerom.o ${LIBS} -lmpr -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/pcre.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/pcre/pcre.c

${CC} -shared -o ${PLATFORM}/lib/libpcre.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/pcre.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/httpLib.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/http/httpLib.c

${CC} -shared -o ${PLATFORM}/lib/libhttp.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${PLATFORM}/obj/http.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/http/http.c

${CC} -o ${PLATFORM}/bin/http -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/sqlite3.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/deps/sqlite/sqlite3.c

${CC} -shared -o ${PLATFORM}/lib/libsqlite3.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/sqlite3.o ${LIBS}

${CC} -c -o ${PLATFORM}/obj/config.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/config.c

${CC} -c -o ${PLATFORM}/obj/convenience.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/convenience.c

${CC} -c -o ${PLATFORM}/obj/dirHandler.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/dirHandler.c

${CC} -c -o ${PLATFORM}/obj/fileHandler.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/fileHandler.c

${CC} -c -o ${PLATFORM}/obj/log.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/log.c

${CC} -c -o ${PLATFORM}/obj/server.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/server.c

${CC} -shared -o ${PLATFORM}/lib/libappweb.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/config.o ${PLATFORM}/obj/convenience.o ${PLATFORM}/obj/dirHandler.o ${PLATFORM}/obj/fileHandler.o ${PLATFORM}/obj/log.o ${PLATFORM}/obj/server.o ${LIBS} -lmpr -lhttp -lpcre -lpcre

${CC} -c -o ${PLATFORM}/obj/edi.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/edi.c

${CC} -c -o ${PLATFORM}/obj/espAbbrev.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espAbbrev.c

${CC} -c -o ${PLATFORM}/obj/espFramework.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espFramework.c

${CC} -c -o ${PLATFORM}/obj/espHandler.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espHandler.c

${CC} -c -o ${PLATFORM}/obj/espHtml.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espHtml.c

${CC} -c -o ${PLATFORM}/obj/espSession.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espSession.c

${CC} -c -o ${PLATFORM}/obj/espTemplate.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/espTemplate.c

${CC} -c -o ${PLATFORM}/obj/mdb.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/mdb.c

${CC} -c -o ${PLATFORM}/obj/sdb.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/sdb.c

${CC} -shared -o ${PLATFORM}/lib/mod_esp.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/edi.o ${PLATFORM}/obj/espAbbrev.o ${PLATFORM}/obj/espFramework.o ${PLATFORM}/obj/espHandler.o ${PLATFORM}/obj/espHtml.o ${PLATFORM}/obj/espSession.o ${PLATFORM}/obj/espTemplate.o ${PLATFORM}/obj/mdb.o ${PLATFORM}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${PLATFORM}/obj/esp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/esp/esp.c

${CC} -o ${PLATFORM}/bin/esp -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/edi.o ${PLATFORM}/obj/esp.o ${PLATFORM}/obj/espAbbrev.o ${PLATFORM}/obj/espFramework.o ${PLATFORM}/obj/espHandler.o ${PLATFORM}/obj/espHtml.o ${PLATFORM}/obj/espSession.o ${PLATFORM}/obj/espTemplate.o ${PLATFORM}/obj/mdb.o ${PLATFORM}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre -L${PLATFORM}/lib -g

rm -rf /Users/mob/git/appweb/linux-i686-debug/lib/esp.conf

cp -r /Users/mob/git/appweb/src/esp/esp.conf /Users/mob/git/appweb/linux-i686-debug/lib/esp.conf

rm -rf /Users/mob/git/appweb/linux-i686-debug/lib/esp-www

cp -r /Users/mob/git/appweb/src/esp/www /Users/mob/git/appweb/linux-i686-debug/lib/esp-www

${CC} -c -o ${PLATFORM}/obj/cgiHandler.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/modules/cgiHandler.c

${CC} -shared -o ${PLATFORM}/lib/mod_cgi.so -L${PLATFORM}/lib -g ${PLATFORM}/obj/cgiHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${PLATFORM}/obj/auth.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/auth.c

${CC} -o ${PLATFORM}/bin/auth -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/auth.o ${LIBS} -lmpr -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/cgiProgram.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/cgiProgram.c

${CC} -o ${PLATFORM}/bin/cgiProgram -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/cgiProgram.o ${LIBS} -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/setConfig.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/utils/setConfig.c

${CC} -o ${PLATFORM}/bin/setConfig -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/setConfig.o ${LIBS} -lmpr -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/appweb.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc src/server/appweb.c

${CC} -o ${PLATFORM}/bin/appweb -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/appweb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre -L${PLATFORM}/lib -g

${CC} -c -o ${PLATFORM}/obj/testAppweb.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Isrc/test src/test/testAppweb.c

${CC} -c -o ${PLATFORM}/obj/testHttp.o ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc -Isrc/deps/mpr -Isrc/deps/pcre -Isrc/deps/http -Isrc/deps/sqlite -Isrc -Isrc/esp -Isrc -Isrc/test src/test/testHttp.c

${CC} -o ${PLATFORM}/bin/testAppweb -L${PLATFORM}/lib -g -L${PLATFORM}/lib ${PLATFORM}/obj/testAppweb.o ${PLATFORM}/obj/testHttp.o ${LIBS} -lappweb -lmpr -lhttp -lpcre -L${PLATFORM}/lib -g

echo '#!/Users/mob/git/appweb/linux-i686-debug/bin/cgiProgram' >/Users/mob/git/appweb/src/test/cgi-bin/testScript ; chmod +x /Users/mob/git/appweb/src/test/cgi-bin/testScript
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
cp /Users/mob/git/appweb/linux-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/cgiProgram
cp /Users/mob/git/appweb/linux-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/nph-cgiProgram
cp /Users/mob/git/appweb/linux-i686-debug/bin/cgiProgram '/Users/mob/git/appweb/src/test/cgi-bin/cgi Program'
cp /Users/mob/git/appweb/linux-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
chmod +x /Users/mob/git/appweb/src/test/cgi-bin/* /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
