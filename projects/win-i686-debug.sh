#
#   build.sh -- Build It Shell Script to build Embedthis Appweb
#

PLATFORM="win-i686-debug"
CC="cl"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT"
IFLAGS="-Iwin-i686-debug/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -libpath:/Users/mob/git/appweb/${PLATFORM}/bin -debug -machine:x86"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib"

export PATH="%VS%/Bin:%VS%/VC/Bin:%VS%/Common7/IDE:%VS%/Common7/Tools:%VS%/SDK/v3.5/bin:%VS%/VC/VCPackages"
export INCLUDE="%VS%/INCLUDE:%VS%/VC/INCLUDE"
export LIB="%VS%/lib:%VS%/VC/lib"
[ ! -x ${PLATFORM}/inc ] && mkdir -p ${PLATFORM}/inc ${PLATFORM}/obj ${PLATFORM}/lib ${PLATFORM}/bin
[ ! -f ${PLATFORM}/inc/buildConfig.h ] && cp src/buildConfig.default ${PLATFORM}/inc/buildConfig.h

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/mpr.h
cp -r /Users/mob/git/appweb/src/deps/mpr/mpr.h /Users/mob/git/appweb/win-i686-debug/inc/mpr.h

"${CC}" -c -Fo${PLATFORM}/obj/mprLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/mprLib.c

"link" -dll -out:${PLATFORM}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libmpr.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/manager.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/manager.c

"link" -out:${PLATFORM}/bin/appman -entry:WinMainCRTStartup -subsystem:Windows -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/manager.obj ${LIBS} mpr.lib shell32.lib

"${CC}" -c -Fo${PLATFORM}/obj/makerom.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/mpr/makerom.c

"link" -out:${PLATFORM}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/makerom.obj ${LIBS} mpr.lib

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/pcre.h
cp -r /Users/mob/git/appweb/src/deps/pcre/pcre.h /Users/mob/git/appweb/win-i686-debug/inc/pcre.h

"${CC}" -c -Fo${PLATFORM}/obj/pcre.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/pcre/pcre.c

"link" -dll -out:${PLATFORM}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libpcre.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/pcre.obj ${LIBS}

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/http.h
cp -r /Users/mob/git/appweb/src/deps/http/http.h /Users/mob/git/appweb/win-i686-debug/inc/http.h

"${CC}" -c -Fo${PLATFORM}/obj/httpLib.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/httpLib.c

"link" -dll -out:${PLATFORM}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libhttp.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/httpLib.obj ${LIBS} mpr.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/http.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/http/http.c

"link" -out:${PLATFORM}/bin/http.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/http.obj ${LIBS} http.lib mpr.lib pcre.lib

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/sqlite3.h
cp -r /Users/mob/git/appweb/src/deps/sqlite/sqlite3.h /Users/mob/git/appweb/win-i686-debug/inc/sqlite3.h

"${CC}" -c -Fo${PLATFORM}/obj/sqlite3.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/deps/sqlite/sqlite3.c

"link" -dll -out:${PLATFORM}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libsqlite3.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/sqlite3.obj ${LIBS}

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/appweb.h
cp -r /Users/mob/git/appweb/src/appweb.h /Users/mob/git/appweb/win-i686-debug/inc/appweb.h

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/customize.h
cp -r /Users/mob/git/appweb/src/customize.h /Users/mob/git/appweb/win-i686-debug/inc/customize.h

"${CC}" -c -Fo${PLATFORM}/obj/config.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/config.c

"${CC}" -c -Fo${PLATFORM}/obj/convenience.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/convenience.c

"${CC}" -c -Fo${PLATFORM}/obj/dirHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/dirHandler.c

"${CC}" -c -Fo${PLATFORM}/obj/fileHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/fileHandler.c

"${CC}" -c -Fo${PLATFORM}/obj/log.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/log.c

"${CC}" -c -Fo${PLATFORM}/obj/server.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/server.c

"link" -dll -out:${PLATFORM}/bin/libappweb.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/libappweb.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/config.obj ${PLATFORM}/obj/convenience.obj ${PLATFORM}/obj/dirHandler.obj ${PLATFORM}/obj/fileHandler.obj ${PLATFORM}/obj/log.obj ${PLATFORM}/obj/server.obj ${LIBS} mpr.lib http.lib pcre.lib pcre.lib

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/edi.h
cp -r /Users/mob/git/appweb/src/esp/edi.h /Users/mob/git/appweb/win-i686-debug/inc/edi.h

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/esp-app.h
cp -r /Users/mob/git/appweb/src/esp/esp-app.h /Users/mob/git/appweb/win-i686-debug/inc/esp-app.h

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/esp.h
cp -r /Users/mob/git/appweb/src/esp/esp.h /Users/mob/git/appweb/win-i686-debug/inc/esp.h

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/mdb.h
cp -r /Users/mob/git/appweb/src/esp/mdb.h /Users/mob/git/appweb/win-i686-debug/inc/mdb.h

"${CC}" -c -Fo${PLATFORM}/obj/edi.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/edi.c

"${CC}" -c -Fo${PLATFORM}/obj/espAbbrev.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espAbbrev.c

"${CC}" -c -Fo${PLATFORM}/obj/espFramework.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espFramework.c

"${CC}" -c -Fo${PLATFORM}/obj/espHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espHandler.c

"${CC}" -c -Fo${PLATFORM}/obj/espHtml.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espHtml.c

"${CC}" -c -Fo${PLATFORM}/obj/espSession.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espSession.c

"${CC}" -c -Fo${PLATFORM}/obj/espTemplate.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/espTemplate.c

"${CC}" -c -Fo${PLATFORM}/obj/mdb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/mdb.c

"${CC}" -c -Fo${PLATFORM}/obj/sdb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/sdb.c

"link" -dll -out:${PLATFORM}/bin/mod_esp.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/mod_esp.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/edi.obj ${PLATFORM}/obj/espAbbrev.obj ${PLATFORM}/obj/espFramework.obj ${PLATFORM}/obj/espHandler.obj ${PLATFORM}/obj/espHtml.obj ${PLATFORM}/obj/espSession.obj ${PLATFORM}/obj/espTemplate.obj ${PLATFORM}/obj/mdb.obj ${PLATFORM}/obj/sdb.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/esp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/esp/esp.c

"link" -out:${PLATFORM}/bin/esp.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/edi.obj ${PLATFORM}/obj/esp.obj ${PLATFORM}/obj/espAbbrev.obj ${PLATFORM}/obj/espFramework.obj ${PLATFORM}/obj/espHandler.obj ${PLATFORM}/obj/espHtml.obj ${PLATFORM}/obj/espSession.obj ${PLATFORM}/obj/espTemplate.obj ${PLATFORM}/obj/mdb.obj ${PLATFORM}/obj/sdb.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

rm -rf /Users/mob/git/appweb/win-i686-debug/bin/esp.conf
cp -r /Users/mob/git/appweb/src/esp/esp.conf /Users/mob/git/appweb/win-i686-debug/bin/esp.conf

rm -rf /Users/mob/git/appweb/win-i686-debug/bin/esp-www
cp -r /Users/mob/git/appweb/src/esp/www /Users/mob/git/appweb/win-i686-debug/bin/esp-www

"${CC}" -c -Fo${PLATFORM}/obj/cgiHandler.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/modules/cgiHandler.c

"link" -dll -out:${PLATFORM}/bin/mod_cgi.dll -entry:_DllMainCRTStartup@12 -def:${PLATFORM}/bin/mod_cgi.def -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/cgiHandler.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

"${CC}" -c -Fo${PLATFORM}/obj/auth.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/utils/auth.c

"link" -out:${PLATFORM}/bin/auth.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/auth.obj ${LIBS} mpr.lib

"${CC}" -c -Fo${PLATFORM}/obj/cgiProgram.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/utils/cgiProgram.c

"link" -out:${PLATFORM}/bin/cgiProgram.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/cgiProgram.obj ${LIBS}

"${CC}" -c -Fo${PLATFORM}/obj/setConfig.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/utils/setConfig.c

"link" -out:${PLATFORM}/bin/setConfig.exe -entry:WinMainCRTStartup -subsystem:Windows -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/setConfig.obj ${LIBS} mpr.lib shell32.lib

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/appwebMonitor.h
cp -r /Users/mob/git/appweb/src/server/appwebMonitor.h /Users/mob/git/appweb/win-i686-debug/inc/appwebMonitor.h

"${CC}" -c -Fo${PLATFORM}/obj/appweb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/server/appweb.c

"link" -out:${PLATFORM}/bin/appweb.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/appweb.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

rm -rf /Users/mob/git/appweb/win-i686-debug/inc/testAppweb.h
cp -r /Users/mob/git/appweb/src/test/testAppweb.h /Users/mob/git/appweb/win-i686-debug/inc/testAppweb.h

"${CC}" -c -Fo${PLATFORM}/obj/testAppweb.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/test/testAppweb.c

"${CC}" -c -Fo${PLATFORM}/obj/testHttp.obj -Fd${PLATFORM}/obj ${CFLAGS} ${DFLAGS} -I${PLATFORM}/inc src/test/testHttp.c

"link" -out:${PLATFORM}/bin/testAppweb.exe -entry:mainCRTStartup -subsystem:console -nologo -nodefaultlib -incremental:no -libpath:${PLATFORM}/bin -debug -machine:x86 ${PLATFORM}/obj/testAppweb.obj ${PLATFORM}/obj/testHttp.obj ${LIBS} appweb.lib mpr.lib http.lib pcre.lib

echo '#!/Users/mob/git/appweb/win-i686-debug/bin/cgiProgram.exe' >/Users/mob/git/appweb/src/test/cgi-bin/testScript ; chmod +x /Users/mob/git/appweb/src/test/cgi-bin/testScript
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
cp /Users/mob/git/appweb/win-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/cgiProgram.exe
cp /Users/mob/git/appweb/win-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/cgi-bin/nph-cgiProgram.exe
cp /Users/mob/git/appweb/win-i686-debug/bin/cgiProgram '/Users/mob/git/appweb/src/test/cgi-bin/cgi Program.exe'
cp /Users/mob/git/appweb/win-i686-debug/bin/cgiProgram /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
chmod +x /Users/mob/git/appweb/src/test/cgi-bin/* /Users/mob/git/appweb/src/test/web/cgiProgram.cgi
