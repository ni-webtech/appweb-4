#
#   solaris-i686-debug.sh -- Build It Shell Script to build Embedthis Appweb
#

CONFIG="solaris-i686-debug"
CC="cc"
LD="ld"
CFLAGS="-Wall -fPIC -O3 -mcpu=i686 -fPIC -O3 -mcpu=i686"
DFLAGS="-D_REENTRANT -DCPU=${ARCH} -DPIC -DPIC"
IFLAGS="-Isolaris-i686-debug/inc -Isolaris-i686-debug/inc"
LDFLAGS=""
LIBPATHS="-L${CONFIG}/lib -L${CONFIG}/lib"
LIBS="-llxnet -lrt -lsocket -lpthread -lm -lpthread -lm"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin
cp projects/buildConfig.${CONFIG} ${CONFIG}/inc/buildConfig.h

rm -rf solaris-i686-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h solaris-i686-debug/inc/mpr.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/mprLib.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/mprLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/libmpr.so ${LIBPATHS} ${CONFIG}/obj/mprLib.o ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/manager.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/manager.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/appman ${LIBPATHS} ${CONFIG}/obj/manager.o ${LIBS} -lmpr 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/makerom.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/makerom.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/makerom ${LIBPATHS} ${CONFIG}/obj/makerom.o ${LIBS} -lmpr 

rm -rf solaris-i686-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h solaris-i686-debug/inc/pcre.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/pcre.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/pcre/pcre.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/libpcre.so ${LIBPATHS} ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf solaris-i686-debug/inc/http.h
cp -r src/deps/http/http.h solaris-i686-debug/inc/http.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/httpLib.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/http/httpLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/libhttp.so ${LIBPATHS} ${CONFIG}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/http.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/http/http.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/http ${LIBPATHS} ${CONFIG}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre 

rm -rf solaris-i686-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h solaris-i686-debug/inc/sqlite3.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sqlite3.o -fPIC -O3 -mcpu=i686 -fPIC -O3 -mcpu=i686 -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/libsqlite3.so ${LIBPATHS} ${CONFIG}/obj/sqlite3.o ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sqlite.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/sqlite ${LIBPATHS} ${CONFIG}/obj/sqlite.o ${LIBS} -lsqlite3 

rm -rf solaris-i686-debug/inc/appweb.h
cp -r src/appweb.h solaris-i686-debug/inc/appweb.h

rm -rf solaris-i686-debug/inc/customize.h
cp -r src/customize.h solaris-i686-debug/inc/customize.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/config.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/config.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/convenience.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/convenience.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/dirHandler.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/dirHandler.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/fileHandler.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/fileHandler.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/log.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/log.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/server.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/server.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/libappweb.so ${LIBPATHS} ${CONFIG}/obj/config.o ${CONFIG}/obj/convenience.o ${CONFIG}/obj/dirHandler.o ${CONFIG}/obj/fileHandler.o ${CONFIG}/obj/log.o ${CONFIG}/obj/server.o ${LIBS} -lmpr -lhttp -lpcre -lpcre

rm -rf solaris-i686-debug/inc/edi.h
cp -r src/esp/edi.h solaris-i686-debug/inc/edi.h

rm -rf solaris-i686-debug/inc/esp-app.h
cp -r src/esp/esp-app.h solaris-i686-debug/inc/esp-app.h

rm -rf solaris-i686-debug/inc/esp.h
cp -r src/esp/esp.h solaris-i686-debug/inc/esp.h

rm -rf solaris-i686-debug/inc/mdb.h
cp -r src/esp/mdb.h solaris-i686-debug/inc/mdb.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/edi.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/edi.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/espAbbrev.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espAbbrev.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/espFramework.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espFramework.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/espHandler.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espHandler.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/espHtml.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espHtml.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/espSession.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espSession.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/espTemplate.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espTemplate.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/mdb.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/mdb.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sdb.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/sdb.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/mod_esp.so ${LIBPATHS} ${CONFIG}/obj/edi.o ${CONFIG}/obj/espAbbrev.o ${CONFIG}/obj/espFramework.o ${CONFIG}/obj/espHandler.o ${CONFIG}/obj/espHtml.o ${CONFIG}/obj/espSession.o ${CONFIG}/obj/espTemplate.o ${CONFIG}/obj/mdb.o ${CONFIG}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/esp.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/esp/esp.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/esp ${LIBPATHS} ${CONFIG}/obj/edi.o ${CONFIG}/obj/esp.o ${CONFIG}/obj/espAbbrev.o ${CONFIG}/obj/espFramework.o ${CONFIG}/obj/espHandler.o ${CONFIG}/obj/espHtml.o ${CONFIG}/obj/espSession.o ${CONFIG}/obj/espTemplate.o ${CONFIG}/obj/mdb.o ${CONFIG}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre 

rm -rf solaris-i686-debug/lib/esp.conf
cp -r src/esp/esp.conf solaris-i686-debug/lib/esp.conf

rm -rf solaris-i686-debug/lib/esp-www
cp -r src/esp/www solaris-i686-debug/lib/esp-www

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/cgiHandler.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/modules/cgiHandler.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/mod_cgi.so ${LIBPATHS} ${CONFIG}/obj/cgiHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/auth.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/utils/auth.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/auth ${LIBPATHS} ${CONFIG}/obj/auth.o ${LIBS} -lmpr 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/cgiProgram.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/utils/cgiProgram.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/cgiProgram ${LIBPATHS} ${CONFIG}/obj/cgiProgram.o ${LIBS} 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/setConfig.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/utils/setConfig.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/setConfig ${LIBPATHS} ${CONFIG}/obj/setConfig.o ${LIBS} -lmpr 

rm -rf solaris-i686-debug/inc/appwebMonitor.h
cp -r src/server/appwebMonitor.h solaris-i686-debug/inc/appwebMonitor.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/appweb.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/server/appweb.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/appweb ${LIBPATHS} ${CONFIG}/obj/appweb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/simpleServer.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/simpleServer/simpleServer.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/simpleServer ${LIBPATHS} ${CONFIG}/obj/simpleServer.o ${LIBS} -lappweb -lmpr -lhttp -lpcre 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/simpleHandler.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/simpleHandler/simpleHandler.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/lib/simpleHandler.so ${LIBPATHS} ${CONFIG}/obj/simpleHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/simpleClient.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/simpleClient/simpleClient.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/simpleClient ${LIBPATHS} ${CONFIG}/obj/simpleClient.o ${LIBS} -lappweb -lmpr -lhttp -lpcre 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/cppHandler.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/samples/cpp/cppHandler/cppHandler.cpp

${LDFLAGS}${LDFLAGS}${CC} -shared -o src/samples/cpp/cppHandler/cppModule.so ${LIBPATHS} ${CONFIG}/obj/cppHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/cppModule.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc src/samples/cpp/cppModule/cppModule.cpp

${LDFLAGS}${LDFLAGS}${CC} -shared -o src/samples/cpp/cppModule/cppModule.so ${LIBPATHS} ${CONFIG}/obj/cppModule.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

rm -rf solaris-i686-debug/inc/testAppweb.h
cp -r test/testAppweb.h solaris-i686-debug/inc/testAppweb.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/testAppweb.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc test/testAppweb.c

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/testHttp.o ${CFLAGS} -D_REENTRANT -DCPU=i686 -DPIC -DPIC -I${CONFIG}/inc -I${CONFIG}/inc test/testHttp.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/testAppweb ${LIBPATHS} ${CONFIG}/obj/testAppweb.o ${CONFIG}/obj/testHttp.o ${LIBS} -lappweb -lmpr -lhttp -lpcre 

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
