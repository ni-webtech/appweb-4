#
#   win-i686-debug.sh -- Build It Shell Script to build Embedthis Appweb
#

VS="${VSINSTALLDIR}"
: ${VS:="$(VS)"}
SDK="${WindowsSDKDir}"
: ${SDK:="$(SDK)"}

export SDK VS
export PATH="$(SDK)/Bin:$(VS)/VC/Bin:$(VS)/Common7/IDE:$(VS)/Common7/Tools:$(VS)/SDK/v3.5/bin:$(VS)/VC/VCPackages;$(PATH)"
export INCLUDE="$(INCLUDE);$(SDK)/INCLUDE:$(VS)/VC/INCLUDE"
export LIB="$(LIB);$(SDK)/lib:$(VS)/VC/lib"

CONFIG="win-i686-debug"
CC="cl.exe"
LD="link.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug/inc -Iwin-i686-debug/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -machine:x86 -machine:x86"
LIBPATHS="-libpath:${CONFIG}/bin -libpath:${CONFIG}/bin"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib shell32.lib"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin
cp projects/buildConfig.${CONFIG} ${CONFIG}/inc/buildConfig.h

rm -rf win-i686-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h win-i686-debug/inc/mpr.h

"${CC}" -c -Fo${CONFIG}/obj/mprLib.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/mprLib.c

"${LD}" -dll -out:${CONFIG}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libmpr.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/manager.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/manager.c

"${LD}" -out:${CONFIG}/bin/appman -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${CONFIG}/obj/makerom.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/mpr/makerom.c

"${LD}" -out:${CONFIG}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.obj ${LIBS} libmpr.lib

rm -rf win-i686-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h win-i686-debug/inc/pcre.h

"${CC}" -c -Fo${CONFIG}/obj/pcre.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/pcre/pcre.c

"${LD}" -dll -out:${CONFIG}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libpcre.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/pcre.obj ${LIBS}

rm -rf win-i686-debug/inc/http.h
cp -r src/deps/http/http.h win-i686-debug/inc/http.h

"${CC}" -c -Fo${CONFIG}/obj/httpLib.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/http/httpLib.c

"${LD}" -dll -out:${CONFIG}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libhttp.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/httpLib.obj ${LIBS} libmpr.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/http.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/http/http.c

"${LD}" -out:${CONFIG}/bin/http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib

rm -rf win-i686-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h win-i686-debug/inc/sqlite3.h

"${CC}" -c -Fo${CONFIG}/obj/sqlite3.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

"${LD}" -dll -out:${CONFIG}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libsqlite3.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite3.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/sqlite.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/deps/sqlite/sqlite.c

"${LD}" -out:${CONFIG}/bin/sqlite.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.obj ${LIBS} libsqlite3.lib

rm -rf win-i686-debug/inc/appweb.h
cp -r src/appweb.h win-i686-debug/inc/appweb.h

rm -rf win-i686-debug/inc/customize.h
cp -r src/customize.h win-i686-debug/inc/customize.h

"${CC}" -c -Fo${CONFIG}/obj/config.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/config.c

"${CC}" -c -Fo${CONFIG}/obj/convenience.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/convenience.c

"${CC}" -c -Fo${CONFIG}/obj/dirHandler.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/dirHandler.c

"${CC}" -c -Fo${CONFIG}/obj/fileHandler.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/fileHandler.c

"${CC}" -c -Fo${CONFIG}/obj/log.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/log.c

"${CC}" -c -Fo${CONFIG}/obj/server.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/server.c

"${LD}" -dll -out:${CONFIG}/bin/libappweb.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libappweb.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/config.obj ${CONFIG}/obj/convenience.obj ${CONFIG}/obj/dirHandler.obj ${CONFIG}/obj/fileHandler.obj ${CONFIG}/obj/log.obj ${CONFIG}/obj/server.obj ${LIBS} libmpr.lib libhttp.lib libpcre.lib libpcre.lib

rm -rf win-i686-debug/inc/edi.h
cp -r src/esp/edi.h win-i686-debug/inc/edi.h

rm -rf win-i686-debug/inc/esp-app.h
cp -r src/esp/esp-app.h win-i686-debug/inc/esp-app.h

rm -rf win-i686-debug/inc/esp.h
cp -r src/esp/esp.h win-i686-debug/inc/esp.h

rm -rf win-i686-debug/inc/mdb.h
cp -r src/esp/mdb.h win-i686-debug/inc/mdb.h

"${CC}" -c -Fo${CONFIG}/obj/edi.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/edi.c

"${CC}" -c -Fo${CONFIG}/obj/espAbbrev.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espAbbrev.c

"${CC}" -c -Fo${CONFIG}/obj/espFramework.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espFramework.c

"${CC}" -c -Fo${CONFIG}/obj/espHandler.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espHandler.c

"${CC}" -c -Fo${CONFIG}/obj/espHtml.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espHtml.c

"${CC}" -c -Fo${CONFIG}/obj/espSession.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espSession.c

"${CC}" -c -Fo${CONFIG}/obj/espTemplate.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/espTemplate.c

"${CC}" -c -Fo${CONFIG}/obj/mdb.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/mdb.c

"${CC}" -c -Fo${CONFIG}/obj/sdb.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/sdb.c

"${LD}" -dll -out:${CONFIG}/bin/mod_esp.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/mod_esp.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.obj ${CONFIG}/obj/espAbbrev.obj ${CONFIG}/obj/espFramework.obj ${CONFIG}/obj/espHandler.obj ${CONFIG}/obj/espHtml.obj ${CONFIG}/obj/espSession.obj ${CONFIG}/obj/espTemplate.obj ${CONFIG}/obj/mdb.obj ${CONFIG}/obj/sdb.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/esp.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/esp/esp.c

"${LD}" -out:${CONFIG}/bin/esp.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.obj ${CONFIG}/obj/esp.obj ${CONFIG}/obj/espAbbrev.obj ${CONFIG}/obj/espFramework.obj ${CONFIG}/obj/espHandler.obj ${CONFIG}/obj/espHtml.obj ${CONFIG}/obj/espSession.obj ${CONFIG}/obj/espTemplate.obj ${CONFIG}/obj/mdb.obj ${CONFIG}/obj/sdb.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

rm -rf win-i686-debug/bin/esp.conf
cp -r src/esp/esp.conf win-i686-debug/bin/esp.conf

rm -rf win-i686-debug/bin/esp-www
cp -r src/esp/www win-i686-debug/bin/esp-www

"${CC}" -c -Fo${CONFIG}/obj/cgiHandler.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/modules/cgiHandler.c

"${LD}" -dll -out:${CONFIG}/bin/mod_cgi.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/mod_cgi.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiHandler.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/auth.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/utils/auth.c

"${LD}" -out:${CONFIG}/bin/auth.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/auth.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${CONFIG}/obj/cgiProgram.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/utils/cgiProgram.c

"${LD}" -out:${CONFIG}/bin/cgiProgram.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiProgram.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/setConfig.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/utils/setConfig.c

"${LD}" -out:${CONFIG}/bin/setConfig.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/setConfig.obj ${LIBS} libmpr.lib

rm -rf win-i686-debug/inc/appwebMonitor.h
cp -r src/server/appwebMonitor.h win-i686-debug/inc/appwebMonitor.h

"${CC}" -c -Fo${CONFIG}/obj/appweb.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/server/appweb.c

"${LD}" -out:${CONFIG}/bin/appweb.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/appweb.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"rc" -Fo ${CONFIG}/obj/appwebMonitor.res src/server/WIN/appwebMonitor.rc

"${CC}" -c -Fo${CONFIG}/obj/appwebMonitor.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/server/WIN/appwebMonitor.c

"${LD}" -out:${CONFIG}/bin/appwebMonitor.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/appwebMonitor.res ${CONFIG}/obj/appwebMonitor.obj shell32.lib libappweb.lib ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib libmpr.lib libhttp.lib libpcre.lib

rm -rf win-i686-debug/bin/appwebMonitor.ico
cp -r src/server/WIN/appwebMonitor.ico win-i686-debug/bin/appwebMonitor.ico

"${CC}" -c -Fo${CONFIG}/obj/simpleServer.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/simpleServer/simpleServer.c

"${LD}" -out:${CONFIG}/bin/simpleServer.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/simpleServer.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/simpleHandler.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/simpleHandler/simpleHandler.c

"${LD}" -dll -out:${CONFIG}/bin/simpleHandler.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/simpleHandler.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/simpleHandler.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/simpleClient.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/c/simpleClient/simpleClient.c

"${LD}" -out:${CONFIG}/bin/simpleClient.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/simpleClient.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/cppHandler.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/cpp/cppHandler/cppHandler.cpp

"${LD}" -dll -out:src/samples/cpp/cppHandler/cppModule.dll -entry:_DllMainCRTStartup@12 -def:src/samples/cpp/cppHandler/cppModule.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cppHandler.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/cppModule.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc src/samples/cpp/cppModule/cppModule.cpp

"${LD}" -dll -out:src/samples/cpp/cppModule/cppModule.dll -entry:_DllMainCRTStartup@12 -def:src/samples/cpp/cppModule/cppModule.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cppModule.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

rm -rf win-i686-debug/inc/testAppweb.h
cp -r test/testAppweb.h win-i686-debug/inc/testAppweb.h

"${CC}" -c -Fo${CONFIG}/obj/testAppweb.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc test/testAppweb.c

"${CC}" -c -Fo${CONFIG}/obj/testHttp.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc test/testHttp.c

"${LD}" -out:${CONFIG}/bin/testAppweb.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/testAppweb.obj ${CONFIG}/obj/testHttp.obj ${LIBS} libappweb.lib libmpr.lib libhttp.lib libpcre.lib

echo '#!${CONFIG}/bin/cgiProgram.exe' >test/cgi-bin/testScript ; chmod +x test/cgi-bin/testScript
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
cp ${CONFIG}/bin/cgiProgram test/cgi-bin/cgiProgram.exe
cp ${CONFIG}/bin/cgiProgram test/cgi-bin/nph-cgiProgram.exe
cp ${CONFIG}/bin/cgiProgram 'test/cgi-bin/cgi Program.exe'
cp ${CONFIG}/bin/cgiProgram test/web/cgiProgram.cgi
chmod +x test/cgi-bin/* test/web/cgiProgram.cgi
"${CC}" -c -Fo${CONFIG}/obj/removeFiles.obj -Fd${CONFIG}/obj ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc -I${CONFIG}/inc package/WIN/removeFiles.c

"${LD}" -out:${CONFIG}/bin/removeFiles.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/removeFiles.obj ${LIBS} libmpr.lib

