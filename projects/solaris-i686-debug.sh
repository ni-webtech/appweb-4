#
#   solaris-i686-debug.sh -- Build It Shell Script to build Embedthis Appweb
#

CONFIG="solaris-i686-debug"
CC="cc"
LD="ld"
CFLAGS="-Wall -fPIC -g -mcpu=i686"
DFLAGS="-D_REENTRANT -DCPU=${ARCH} -DPIC"
IFLAGS="-Isolaris-i686-debug/inc"
LDFLAGS="-g"
LIBPATHS="-L${CONFIG}/lib"
LIBS="-llxnet -lrt -lsocket -lpthread -lm"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin
cp projects/buildConfig.${CONFIG} ${CONFIG}/inc/buildConfig.h

rm -rf solaris-i686-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h solaris-i686-debug/inc/mpr.h

${CC} -c -o ${CONFIG}/obj/mprLib.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/mpr/mprLib.c

${CC} -shared -o ${CONFIG}/lib/libmpr.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprLib.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/manager.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/mpr/manager.c

${CC} -o ${CONFIG}/bin/appman ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.o ${LIBS} -lmpr ${LDFLAGS}

${CC} -c -o ${CONFIG}/obj/makerom.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/mpr/makerom.c

${CC} -o ${CONFIG}/bin/makerom ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.o ${LIBS} -lmpr ${LDFLAGS}

rm -rf solaris-i686-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h solaris-i686-debug/inc/pcre.h

${CC} -c -o ${CONFIG}/obj/pcre.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/pcre/pcre.c

${CC} -shared -o ${CONFIG}/lib/libpcre.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf solaris-i686-debug/inc/http.h
cp -r src/deps/http/http.h solaris-i686-debug/inc/http.h

${CC} -c -o ${CONFIG}/obj/httpLib.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/http/httpLib.c

${CC} -shared -o ${CONFIG}/lib/libhttp.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${CONFIG}/obj/http.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/http/http.c

${CC} -o ${CONFIG}/bin/http ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre ${LDFLAGS}

rm -rf solaris-i686-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h solaris-i686-debug/inc/sqlite3.h

${CC} -c -o ${CONFIG}/obj/sqlite3.o -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${CC} -shared -o ${CONFIG}/lib/libsqlite3.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite3.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/sqlite.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${CC} -o ${CONFIG}/bin/sqlite ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.o ${LIBS} -lsqlite3 ${LDFLAGS}

rm -rf solaris-i686-debug/inc/appweb.h
cp -r src/appweb.h solaris-i686-debug/inc/appweb.h

rm -rf solaris-i686-debug/inc/customize.h
cp -r src/customize.h solaris-i686-debug/inc/customize.h

${CC} -c -o ${CONFIG}/obj/config.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/config.c

${CC} -c -o ${CONFIG}/obj/convenience.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/convenience.c

${CC} -c -o ${CONFIG}/obj/dirHandler.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/dirHandler.c

${CC} -c -o ${CONFIG}/obj/fileHandler.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/fileHandler.c

${CC} -c -o ${CONFIG}/obj/log.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/log.c

${CC} -c -o ${CONFIG}/obj/server.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/server.c

${CC} -shared -o ${CONFIG}/lib/libappweb.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/config.o ${CONFIG}/obj/convenience.o ${CONFIG}/obj/dirHandler.o ${CONFIG}/obj/fileHandler.o ${CONFIG}/obj/log.o ${CONFIG}/obj/server.o ${LIBS} -lmpr -lhttp -lpcre -lpcre

rm -rf solaris-i686-debug/inc/edi.h
cp -r src/esp/edi.h solaris-i686-debug/inc/edi.h

rm -rf solaris-i686-debug/inc/esp-app.h
cp -r src/esp/esp-app.h solaris-i686-debug/inc/esp-app.h

rm -rf solaris-i686-debug/inc/esp.h
cp -r src/esp/esp.h solaris-i686-debug/inc/esp.h

rm -rf solaris-i686-debug/inc/mdb.h
cp -r src/esp/mdb.h solaris-i686-debug/inc/mdb.h

${CC} -c -o ${CONFIG}/obj/edi.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/edi.c

${CC} -c -o ${CONFIG}/obj/espAbbrev.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/espAbbrev.c

${CC} -c -o ${CONFIG}/obj/espFramework.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/espFramework.c

${CC} -c -o ${CONFIG}/obj/espHandler.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/espHandler.c

${CC} -c -o ${CONFIG}/obj/espHtml.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/espHtml.c

${CC} -c -o ${CONFIG}/obj/espSession.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/espSession.c

${CC} -c -o ${CONFIG}/obj/espTemplate.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/espTemplate.c

${CC} -c -o ${CONFIG}/obj/mdb.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/mdb.c

${CC} -c -o ${CONFIG}/obj/sdb.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/sdb.c

${CC} -shared -o ${CONFIG}/lib/mod_esp.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.o ${CONFIG}/obj/espAbbrev.o ${CONFIG}/obj/espFramework.o ${CONFIG}/obj/espHandler.o ${CONFIG}/obj/espHtml.o ${CONFIG}/obj/espSession.o ${CONFIG}/obj/espTemplate.o ${CONFIG}/obj/mdb.o ${CONFIG}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${CONFIG}/obj/esp.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/esp/esp.c

${CC} -o ${CONFIG}/bin/esp ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.o ${CONFIG}/obj/esp.o ${CONFIG}/obj/espAbbrev.o ${CONFIG}/obj/espFramework.o ${CONFIG}/obj/espHandler.o ${CONFIG}/obj/espHtml.o ${CONFIG}/obj/espSession.o ${CONFIG}/obj/espTemplate.o ${CONFIG}/obj/mdb.o ${CONFIG}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre ${LDFLAGS}

rm -rf solaris-i686-debug/lib/esp.conf
cp -r src/esp/esp.conf solaris-i686-debug/lib/esp.conf

rm -rf solaris-i686-debug/lib/esp-www
cp -r src/esp/www solaris-i686-debug/lib/esp-www

${CC} -c -o ${CONFIG}/obj/cgiHandler.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/modules/cgiHandler.c

${CC} -shared -o ${CONFIG}/lib/mod_cgi.so ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${CONFIG}/obj/auth.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/utils/auth.c

${CC} -o ${CONFIG}/bin/auth ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/auth.o ${LIBS} -lmpr ${LDFLAGS}

${CC} -c -o ${CONFIG}/obj/cgiProgram.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/utils/cgiProgram.c

${CC} -o ${CONFIG}/bin/cgiProgram ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiProgram.o ${LIBS} ${LDFLAGS}

${CC} -c -o ${CONFIG}/obj/setConfig.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/utils/setConfig.c

${CC} -o ${CONFIG}/bin/setConfig ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/setConfig.o ${LIBS} -lmpr ${LDFLAGS}

rm -rf solaris-i686-debug/inc/appwebMonitor.h
cp -r src/server/appwebMonitor.h solaris-i686-debug/inc/appwebMonitor.h

${CC} -c -o ${CONFIG}/obj/appweb.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/server/appweb.c

${CC} -o ${CONFIG}/bin/appweb ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/appweb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre ${LDFLAGS}

${CC} -c -o ${CONFIG}/obj/simpleServer.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/samples/c/simpleServer/simpleServer.c

${CC} -c -o ${CONFIG}/obj/simpleHandler.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/samples/c/simpleHandler/simpleHandler.c

${CC} -c -o ${CONFIG}/obj/simpleClient.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/samples/c/simpleClient/simpleClient.c

${CC} -c -o ${CONFIG}/obj/cppHandler.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/samples/cpp/cppHandler/cppHandler.cpp

${CC} -c -o ${CONFIG}/obj/cppModule.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc src/samples/cpp/cppModule/cppModule.cpp

rm -rf solaris-i686-debug/inc/testAppweb.h
cp -r test/testAppweb.h solaris-i686-debug/inc/testAppweb.h

${CC} -c -o ${CONFIG}/obj/testAppweb.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc test/testAppweb.c

${CC} -c -o ${CONFIG}/obj/testHttp.o -Wall -fPIC ${LDFLAGS} -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -I${CONFIG}/inc test/testHttp.c

${CC} -o ${CONFIG}/bin/testAppweb ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/testAppweb.o ${CONFIG}/obj/testHttp.o ${LIBS} -lappweb -lmpr -lhttp -lpcre ${LDFLAGS}

echo '#!${CONFIG}/bin/cgiProgram' >test/cgi-bin/testScript ; chmod +x test/cgi-bin/testScript
echo -e '#!`type -p sh`' >test/web/caching/cache.cgi
echo -e '' >>test/web/caching/cache.cgi
echo -e 'echo HTTP/1.0 200 OK' >>test/web/caching/cache.cgi
echo -e 'echo Content-Type: text/plain' >>test/web/caching/cache.cgi
echo -e 'date' >>test/web/caching/cache.cgi
chmod +x test/web/caching/cache.cgi
echo -e '#!`type -p sh`' >test/web/basic/basic.cgi
echo -e '' >>test/web/basic/basic.cgi
echo -e 'echo Content-Type: text/plain' >>test/web/basic/basic.cgi
echo -e '/usr/bin/env' >>test/web/basic/basic.cgi
chmod +x test/web/basic/basic.cgi
cp ${CONFIG}/bin/cgiProgram test/cgi-bin/cgiProgram
cp ${CONFIG}/bin/cgiProgram test/cgi-bin/nph-cgiProgram
cp ${CONFIG}/bin/cgiProgram 'test/cgi-bin/cgi Program'
cp ${CONFIG}/bin/cgiProgram test/web/cgiProgram.cgi
chmod +x test/cgi-bin/* test/web/cgiProgram.cgi
