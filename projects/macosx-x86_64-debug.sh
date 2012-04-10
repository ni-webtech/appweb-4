#
#   macosx-x86_64-debug.sh -- Build It Shell Script to build Embedthis Appweb
#

CONFIG="macosx-x86_64-debug"
CC="/usr/bin/cc"
LD="/usr/bin/ld"
CFLAGS="-fPIC -Wall -g -Wno-unused-result -Wshorten-64-to-32"
DFLAGS="-DPIC -DCPU=X86_64"
IFLAGS="-Imacosx-x86_64-debug/inc"
LDFLAGS="-Wl,-rpath,@executable_path/../lib -Wl,-rpath,@executable_path/ -Wl,-rpath,@loader_path/ -g"
LIBPATHS="-L${CONFIG}/lib"
LIBS="-lpthread -lm -ldl"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin
cp projects/buildConfig.${CONFIG} ${CONFIG}/inc/buildConfig.h

rm -rf macosx-x86_64-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h macosx-x86_64-debug/inc/mpr.h

${CC} -c -o ${CONFIG}/obj/mprLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

${CC} -dynamiclib -o ${CONFIG}/lib/libmpr.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libmpr.dylib ${CONFIG}/obj/mprLib.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/manager.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/manager.c

${CC} -o ${CONFIG}/bin/appman -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.o ${LIBS} -lmpr

${CC} -c -o ${CONFIG}/obj/makerom.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/makerom.c

${CC} -o ${CONFIG}/bin/makerom -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.o ${LIBS} -lmpr

rm -rf macosx-x86_64-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h macosx-x86_64-debug/inc/pcre.h

${CC} -c -o ${CONFIG}/obj/pcre.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

${CC} -dynamiclib -o ${CONFIG}/lib/libpcre.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libpcre.dylib ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf macosx-x86_64-debug/inc/http.h
cp -r src/deps/http/http.h macosx-x86_64-debug/inc/http.h

${CC} -c -o ${CONFIG}/obj/httpLib.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

${CC} -dynamiclib -o ${CONFIG}/lib/libhttp.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libhttp.dylib ${CONFIG}/obj/httpLib.o ${LIBS} -lmpr -lpcre

${CC} -c -o ${CONFIG}/obj/http.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

${CC} -o ${CONFIG}/bin/http -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.o ${LIBS} -lhttp -lmpr -lpcre

rm -rf macosx-x86_64-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h macosx-x86_64-debug/inc/sqlite3.h

${CC} -c -o ${CONFIG}/obj/sqlite3.o -arch x86_64 -fPIC -g -Wno-unused-result ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${CC} -dynamiclib -o ${CONFIG}/lib/libsqlite3.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libsqlite3.dylib ${CONFIG}/obj/sqlite3.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/sqlite.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${CC} -o ${CONFIG}/bin/sqlite -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.o ${LIBS} -lsqlite3

rm -rf macosx-x86_64-debug/inc/appweb.h
cp -r src/appweb.h macosx-x86_64-debug/inc/appweb.h

rm -rf macosx-x86_64-debug/inc/customize.h
cp -r src/customize.h macosx-x86_64-debug/inc/customize.h

${CC} -c -o ${CONFIG}/obj/config.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/config.c

${CC} -c -o ${CONFIG}/obj/convenience.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/convenience.c

${CC} -c -o ${CONFIG}/obj/dirHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/dirHandler.c

${CC} -c -o ${CONFIG}/obj/fileHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/fileHandler.c

${CC} -c -o ${CONFIG}/obj/log.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/log.c

${CC} -c -o ${CONFIG}/obj/server.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/server.c

${CC} -dynamiclib -o ${CONFIG}/lib/libappweb.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/libappweb.dylib ${CONFIG}/obj/config.o ${CONFIG}/obj/convenience.o ${CONFIG}/obj/dirHandler.o ${CONFIG}/obj/fileHandler.o ${CONFIG}/obj/log.o ${CONFIG}/obj/server.o ${LIBS} -lmpr -lhttp -lpcre -lpcre

rm -rf macosx-x86_64-debug/inc/edi.h
cp -r src/esp/edi.h macosx-x86_64-debug/inc/edi.h

rm -rf macosx-x86_64-debug/inc/esp-app.h
cp -r src/esp/esp-app.h macosx-x86_64-debug/inc/esp-app.h

rm -rf macosx-x86_64-debug/inc/esp.h
cp -r src/esp/esp.h macosx-x86_64-debug/inc/esp.h

rm -rf macosx-x86_64-debug/inc/mdb.h
cp -r src/esp/mdb.h macosx-x86_64-debug/inc/mdb.h

${CC} -c -o ${CONFIG}/obj/edi.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/edi.c

${CC} -c -o ${CONFIG}/obj/espAbbrev.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espAbbrev.c

${CC} -c -o ${CONFIG}/obj/espFramework.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espFramework.c

${CC} -c -o ${CONFIG}/obj/espHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espHandler.c

${CC} -c -o ${CONFIG}/obj/espHtml.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espHtml.c

${CC} -c -o ${CONFIG}/obj/espSession.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espSession.c

${CC} -c -o ${CONFIG}/obj/espTemplate.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espTemplate.c

${CC} -c -o ${CONFIG}/obj/mdb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/mdb.c

${CC} -c -o ${CONFIG}/obj/sdb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/sdb.c

${CC} -dynamiclib -o ${CONFIG}/lib/mod_esp.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/mod_esp.dylib ${CONFIG}/obj/edi.o ${CONFIG}/obj/espAbbrev.o ${CONFIG}/obj/espFramework.o ${CONFIG}/obj/espHandler.o ${CONFIG}/obj/espHtml.o ${CONFIG}/obj/espSession.o ${CONFIG}/obj/espTemplate.o ${CONFIG}/obj/mdb.o ${CONFIG}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${CONFIG}/obj/esp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/esp.c

${CC} -o ${CONFIG}/bin/esp -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.o ${CONFIG}/obj/esp.o ${CONFIG}/obj/espAbbrev.o ${CONFIG}/obj/espFramework.o ${CONFIG}/obj/espHandler.o ${CONFIG}/obj/espHtml.o ${CONFIG}/obj/espSession.o ${CONFIG}/obj/espTemplate.o ${CONFIG}/obj/mdb.o ${CONFIG}/obj/sdb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

rm -rf macosx-x86_64-debug/lib/esp.conf
cp -r src/esp/esp.conf macosx-x86_64-debug/lib/esp.conf

rm -rf macosx-x86_64-debug/lib/esp-www
cp -r src/esp/www macosx-x86_64-debug/lib/esp-www

${CC} -c -o ${CONFIG}/obj/cgiHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/modules/cgiHandler.c

${CC} -dynamiclib -o ${CONFIG}/lib/mod_cgi.dylib -arch x86_64 ${LDFLAGS} ${LIBPATHS} -install_name @rpath/mod_cgi.dylib ${CONFIG}/obj/cgiHandler.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${CONFIG}/obj/auth.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/utils/auth.c

${CC} -o ${CONFIG}/bin/auth -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/auth.o ${LIBS} -lmpr

${CC} -c -o ${CONFIG}/obj/cgiProgram.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/utils/cgiProgram.c

${CC} -o ${CONFIG}/bin/cgiProgram -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiProgram.o ${LIBS}

${CC} -c -o ${CONFIG}/obj/setConfig.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/utils/setConfig.c

${CC} -o ${CONFIG}/bin/setConfig -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/setConfig.o ${LIBS} -lmpr

rm -rf macosx-x86_64-debug/inc/appwebMonitor.h
cp -r src/server/appwebMonitor.h macosx-x86_64-debug/inc/appwebMonitor.h

${CC} -c -o ${CONFIG}/obj/appweb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/server/appweb.c

${CC} -o ${CONFIG}/bin/appweb -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/appweb.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

${CC} -c -o ${CONFIG}/obj/simpleServer.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/samples/c/simpleServer/simpleServer.c

${CC} -c -o ${CONFIG}/obj/simpleHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/samples/c/simpleHandler/simpleHandler.c

${CC} -c -o ${CONFIG}/obj/simpleClient.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/samples/c/simpleClient/simpleClient.c

${CC} -c -o ${CONFIG}/obj/cppHandler.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/samples/cpp/cppHandler/cppHandler.cpp

${CC} -c -o ${CONFIG}/obj/cppModule.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/samples/cpp/cppModule/cppModule.cpp

rm -rf macosx-x86_64-debug/inc/testAppweb.h
cp -r test/testAppweb.h macosx-x86_64-debug/inc/testAppweb.h

${CC} -c -o ${CONFIG}/obj/testAppweb.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc test/testAppweb.c

${CC} -c -o ${CONFIG}/obj/testHttp.o -arch x86_64 ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc test/testHttp.c

${CC} -o ${CONFIG}/bin/testAppweb -arch x86_64 ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/testAppweb.o ${CONFIG}/obj/testHttp.o ${LIBS} -lappweb -lmpr -lhttp -lpcre

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
