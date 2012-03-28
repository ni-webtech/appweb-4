#
#   win-i686-debug.sh -- Build It Shell Script to build Embedthis Appweb
#

export VS="$(PROGRAMFILES)\Microsoft Visual Studio 10.0"
export SDK="$(PROGRAMFILES)\Microsoft SDKs\Windows\v7.0A"
export PATH="$(SDK)\Bin:$(VS)\VC\Bin:$(VS)\Common7\IDE:$(VS)\Common7\Tools:$(VS)\SDK\v3.5\bin:$(VS)\VC\VCPackages"
export INCLUDE="$(SDK)\INCLUDE:$(VS)\VC\INCLUDE"
export LIB="$(SDK)\lib:$(VS)\VC\lib"

PLATFORM="win-i686-debug"
CC="cl.exe"
LD="link.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib"

[ ! -x ${PLATFORM}/inc ] && mkdir -p ${PLATFORM}/inc ${PLATFORM}/obj ${PLATFORM}/lib ${PLATFORM}/bin
[ ! -f ${PLATFORM}/inc/buildConfig.h ] && cp projects/buildConfig.${PLATFORM} ${PLATFORM}/inc/buildConfig.h

rm -rf win-i686-debug/inc/mpr.h
cp -r src/deps/mpr/mpr.h win-i686-debug/inc/mpr.h

"${CC}" -c -Fo${PLATFORM}/obj/mprLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/mprLib.c

"${LD}" -dll -out:${PLATFORM}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libmpr.def ${LDFLAGS} ${PLATFORM}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/manager.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/manager.c

"${LD}" -out:${PLATFORM}/bin/appman -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${PLATFORM}/obj/manager.obj ${LIBS} mpr.lib shell32.lib

"${CC}" -c -Fo${PLATFORM}/obj/makerom.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/makerom.c

"${LD}" -out:${PLATFORM}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/makerom.obj ${LIBS} mpr.lib

rm -rf win-i686-debug/inc/pcre.h
cp -r src/deps/pcre/pcre.h win-i686-debug/inc/pcre.h

"${CC}" -c -Fo${PLATFORM}/obj/pcre.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/pcre/pcre.c

"${LD}" -dll -out:${PLATFORM}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libpcre.def ${LDFLAGS} ${PLATFORM}/obj/pcre.obj ${LIBS}

rm -rf win-i686-debug/inc/http.h
cp -r src/deps/http/http.h win-i686-debug/inc/http.h

"${CC}" -c -Fo${PLATFORM}/obj/httpLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/httpLib.c

"${LD}" -dll -out:${PLATFORM}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libhttp.def ${LDFLAGS} ${PLATFORM}/obj/httpLib.obj ${LIBS} mpr.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/http.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/http.c

"${LD}" -out:${PLATFORM}/bin/http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/http.obj ${LIBS} http.lib mpr.lib pcre.lib

rm -rf win-i686-debug/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h win-i686-debug/inc/sqlite3.h

"${CC}" -c -Fo${PLATFORM}/obj/sqlite3.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/sqlite/sqlite3.c

"${LD}" -dll -out:${PLATFORM}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libsqlite3.def ${LDFLAGS} ${PLATFORM}/obj/sqlite3.obj ${LIBS}

rm -rf win-i686-debug/inc/appweb.h
cp -r src/appweb.h win-i686-debug/inc/appweb.h

rm -rf win-i686-debug/inc/customize.h
cp -r src/customize.h win-i686-debug/inc/customize.h

"${CC}" -c -Fo${PLATFORM}/obj/config.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/config.c

"${CC}" -c -Fo${PLATFORM}/obj/convenience.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/convenience.c

"${CC}" -c -Fo${PLATFORM}/obj/dirHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/dirHandler.c

"${CC}" -c -Fo${PLATFORM}/obj/fileHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/fileHandler.c

"${CC}" -c -Fo${PLATFORM}/obj/log.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/log.c

"${CC}" -c -Fo${PLATFORM}/obj/server.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/server.c

"${LD}" -dll -out:${PLATFORM}/bin/libappweb.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libappweb.def ${LDFLAGS} ${PLATFORM}/obj/config.obj ${PLATFORM}/obj/convenience.obj ${PLATFORM}/obj/dirHandler.obj ${PLATFORM}/obj/fileHandler.obj ${PLATFORM}/obj/log.obj ${PLATFORM}/obj/server.obj ${LIBS} mpr.lib http.lib pcre.lib pcre.lib

rm -rf win-i686-debug/inc/edi.h
cp -r src/esp/edi.h win-i686-debug/inc/edi.h

rm -rf win-i686-debug/inc/esp-app.h
cp -r src/esp/esp-app.h win-i686-debug/inc/esp-app.h

rm -rf win-i686-debug/inc/esp.h
cp -r src/esp/esp.h win-i686-debug/inc/esp.h

rm -rf win-i686-debug/inc/mdb.h
cp -r src/esp/mdb.h win-i686-debug/inc/mdb.h

"${CC}" -c -Fo${PLATFORM}/obj/edi.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/edi.c

"${CC}" -c -Fo${PLATFORM}/obj/espAbbrev.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espAbbrev.c

"${CC}" -c -Fo${PLATFORM}/obj/espFramework.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espFramework.c

"${CC}" -c -Fo${PLATFORM}/obj/espHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espHandler.c

"${CC}" -c -Fo${PLATFORM}/obj/espHtml.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espHtml.c

"${CC}" -c -Fo${PLATFORM}/obj/espSession.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espSession.c

"${CC}" -c -Fo${PLATFORM}/obj/espTemplate.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espTemplate.c

"${CC}" -c -Fo${PLATFORM}/obj/mdb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/mdb.c

"${CC}" -c -Fo${PLATFORM}/obj/sdb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/sdb.c

"${LD}" -dll -out:${PLATFORM}/bin/mod_esp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/mod_esp.def ${LDFLAGS} ${PLATFORM}/obj/edi.obj ${PLATFORM}/obj/espAbbrev.obj ${PLATFORM}/obj/espFramework.obj ${PLATFORM}/obj/espHandler.obj ${PLATFORM}/obj/espHtml.obj ${PLATFORM}/obj/espSession.obj ${PLATFORM}/obj/espTemplate.obj ${PLATFORM}/obj/mdb.obj ${PLATFORM}/obj/sdb.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/esp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/esp.c

"${LD}" -out:${PLATFORM}/bin/esp.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/edi.obj ${PLATFORM}/obj/esp.obj ${PLATFORM}/obj/espAbbrev.obj ${PLATFORM}/obj/espFramework.obj ${PLATFORM}/obj/espHandler.obj ${PLATFORM}/obj/espHtml.obj ${PLATFORM}/obj/espSession.obj ${PLATFORM}/obj/espTemplate.obj ${PLATFORM}/obj/mdb.obj ${PLATFORM}/obj/sdb.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

rm -rf win-i686-debug/bin/esp.conf
cp -r src/esp/esp.conf win-i686-debug/bin/esp.conf

rm -rf win-i686-debug/bin/esp-www
cp -r src/esp/www win-i686-debug/bin/esp-www

"${CC}" -c -Fo${PLATFORM}/obj/cgiHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/modules/cgiHandler.c

"${LD}" -dll -out:${PLATFORM}/bin/mod_cgi.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/mod_cgi.def ${LDFLAGS} ${PLATFORM}/obj/cgiHandler.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/auth.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/utils/auth.c

"${LD}" -out:${PLATFORM}/bin/auth.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/auth.obj ${LIBS} mpr.lib

"${CC}" -c -Fo${PLATFORM}/obj/cgiProgram.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/utils/cgiProgram.c

"${LD}" -out:${PLATFORM}/bin/cgiProgram.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/cgiProgram.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/setConfig.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/utils/setConfig.c

"${LD}" -out:${PLATFORM}/bin/setConfig.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${PLATFORM}/obj/setConfig.obj ${LIBS} mpr.lib shell32.lib

rm -rf win-i686-debug/inc/appwebMonitor.h
cp -r src/server/appwebMonitor.h win-i686-debug/inc/appwebMonitor.h

"${CC}" -c -Fo${PLATFORM}/obj/appweb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/server/appweb.c

"${LD}" -out:${PLATFORM}/bin/appweb.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/appweb.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/appwebMonitor.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/server/WIN/appwebMonitor.c

"${LD}" -out:${PLATFORM}/bin/appwebMonitor.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${PLATFORM}/obj/appwebMonitor.obj shell32.lib appweb.lib ${LIBS} mpr.lib http.lib pcre.lib shell32.lib

rm -rf win-i686-debug/bin/appwebMonitor.ico
cp -r src/server/WIN/appwebMonitor.ico win-i686-debug/bin/appwebMonitor.ico

rm -rf win-i686-debug/inc/testAppweb.h
cp -r test/testAppweb.h win-i686-debug/inc/testAppweb.h

"${CC}" -c -Fo${PLATFORM}/obj/testAppweb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc test/testAppweb.c

"${CC}" -c -Fo${PLATFORM}/obj/testHttp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc test/testHttp.c

"${LD}" -out:${PLATFORM}/bin/testAppweb.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${PLATFORM}/obj/testAppweb.obj ${PLATFORM}/obj/testHttp.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

echo '#!${PLATFORM}/bin/cgiProgram.exe' >test/cgi-bin/testScript ; chmod +x test/cgi-bin/testScript
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
cp ${PLATFORM}/bin/cgiProgram test/cgi-bin/cgiProgram.exe
cp ${PLATFORM}/bin/cgiProgram test/cgi-bin/nph-cgiProgram.exe
cp ${PLATFORM}/bin/cgiProgram 'test/cgi-bin/cgi Program.exe'
cp ${PLATFORM}/bin/cgiProgram test/web/cgiProgram.cgi
chmod +x test/cgi-bin/* test/web/cgiProgram.cgi
"${CC}" -c -Fo${PLATFORM}/obj/removeFiles.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc package/WIN/removeFiles.c

"${LD}" -out:${PLATFORM}/bin/removeFiles.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${PLATFORM}/obj/removeFiles.obj ${LIBS} mpr.lib shell32.lib

