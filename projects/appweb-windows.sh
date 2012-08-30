#
#   appweb-windows.sh -- Build It Shell Script to build Embedthis Appweb
#

export PATH="$(SDK)/Bin:$(VS)/VC/Bin:$(VS)/Common7/IDE:$(VS)/Common7/Tools:$(VS)/SDK/v3.5/bin:$(VS)/VC/VCPackages;$(PATH)"
export INCLUDE="$(INCLUDE);$(SDK)/Include:$(VS)/VC/INCLUDE"
export LIB="$(LIB);$(SDK)/Lib:$(VS)/VC/lib"

ARCH="x86"
ARCH="$(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/')"
OS="windows"
PROFILE="debug"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="cl.exe"
LD="link.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd"
DFLAGS="-D_REENTRANT -D_MT -DBIT_DEBUG"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -debug -machine:x86"
LIBPATHS="-libpath:${CONFIG}/bin"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib shell32.lib"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin

[ ! -f ${CONFIG}/inc/bit.h ] && cp projects/appweb-${OS}-bit.h ${CONFIG}/inc/bit.h
if ! diff ${CONFIG}/inc/bit.h projects/appweb-${OS}-bit.h >/dev/null ; then
	cp projects/appweb-${OS}-bit.h ${CONFIG}/inc/bit.h
fi

rm -rf ${CONFIG}/inc/mpr.h
cp -r src/deps/mpr/mpr.h ${CONFIG}/inc/mpr.h

"${CC}" -c -Fo${CONFIG}/obj/mprLib.obj -Fd${CONFIG}/obj/mprLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

"${LD}" -dll -out:${CONFIG}/bin/libmpr.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libmpr.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprLib.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/mprSsl.obj -Fd${CONFIG}/obj/mprSsl.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

"${LD}" -dll -out:${CONFIG}/bin/libmprssl.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libmprssl.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/mprSsl.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${CONFIG}/obj/manager.obj -Fd${CONFIG}/obj/manager.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/manager.c

"${LD}" -out:${CONFIG}/bin/appman.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/manager.obj ${LIBS} libmpr.lib

"${CC}" -c -Fo${CONFIG}/obj/makerom.obj -Fd${CONFIG}/obj/makerom.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/makerom.c

"${LD}" -out:${CONFIG}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.obj ${LIBS} libmpr.lib

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

"${CC}" -c -Fo${CONFIG}/obj/pcre.obj -Fd${CONFIG}/obj/pcre.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

"${LD}" -dll -out:${CONFIG}/bin/libpcre.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libpcre.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/pcre.obj ${LIBS}

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

"${CC}" -c -Fo${CONFIG}/obj/httpLib.obj -Fd${CONFIG}/obj/httpLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

"${LD}" -dll -out:${CONFIG}/bin/libhttp.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libhttp.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/httpLib.obj ${LIBS} libmpr.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/http.obj -Fd${CONFIG}/obj/http.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

"${LD}" -out:${CONFIG}/bin/http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

"${CC}" -c -Fo${CONFIG}/obj/sqlite3.obj -Fd${CONFIG}/obj/sqlite3.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

"${LD}" -dll -out:${CONFIG}/bin/libsqlite3.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libsqlite3.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite3.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/sqlite.obj -Fd${CONFIG}/obj/sqlite.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

"${LD}" -out:${CONFIG}/bin/sqlite.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.obj ${LIBS} libsqlite3.lib

rm -rf ${CONFIG}/inc/appweb.h
cp -r src/appweb.h ${CONFIG}/inc/appweb.h

rm -rf ${CONFIG}/inc/customize.h
cp -r src/customize.h ${CONFIG}/inc/customize.h

"${CC}" -c -Fo${CONFIG}/obj/config.obj -Fd${CONFIG}/obj/config.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/config.c

"${CC}" -c -Fo${CONFIG}/obj/convenience.obj -Fd${CONFIG}/obj/convenience.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/convenience.c

"${CC}" -c -Fo${CONFIG}/obj/dirHandler.obj -Fd${CONFIG}/obj/dirHandler.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/dirHandler.c

"${CC}" -c -Fo${CONFIG}/obj/fileHandler.obj -Fd${CONFIG}/obj/fileHandler.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/fileHandler.c

"${CC}" -c -Fo${CONFIG}/obj/log.obj -Fd${CONFIG}/obj/log.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/log.c

"${CC}" -c -Fo${CONFIG}/obj/server.obj -Fd${CONFIG}/obj/server.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/server.c

"${LD}" -dll -out:${CONFIG}/bin/libappweb.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libappweb.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/config.obj ${CONFIG}/obj/convenience.obj ${CONFIG}/obj/dirHandler.obj ${CONFIG}/obj/fileHandler.obj ${CONFIG}/obj/log.obj ${CONFIG}/obj/server.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib

rm -rf ${CONFIG}/inc/edi.h
cp -r src/esp/edi.h ${CONFIG}/inc/edi.h

rm -rf ${CONFIG}/inc/esp-app.h
cp -r src/esp/esp-app.h ${CONFIG}/inc/esp-app.h

rm -rf ${CONFIG}/inc/esp.h
cp -r src/esp/esp.h ${CONFIG}/inc/esp.h

rm -rf ${CONFIG}/inc/mdb.h
cp -r src/esp/mdb.h ${CONFIG}/inc/mdb.h

"${CC}" -c -Fo${CONFIG}/obj/edi.obj -Fd${CONFIG}/obj/edi.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/edi.c

"${CC}" -c -Fo${CONFIG}/obj/espAbbrev.obj -Fd${CONFIG}/obj/espAbbrev.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espAbbrev.c

"${CC}" -c -Fo${CONFIG}/obj/espFramework.obj -Fd${CONFIG}/obj/espFramework.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espFramework.c

"${CC}" -c -Fo${CONFIG}/obj/espHandler.obj -Fd${CONFIG}/obj/espHandler.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espHandler.c

"${CC}" -c -Fo${CONFIG}/obj/espHtml.obj -Fd${CONFIG}/obj/espHtml.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espHtml.c

"${CC}" -c -Fo${CONFIG}/obj/espSession.obj -Fd${CONFIG}/obj/espSession.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espSession.c

"${CC}" -c -Fo${CONFIG}/obj/espTemplate.obj -Fd${CONFIG}/obj/espTemplate.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/espTemplate.c

"${CC}" -c -Fo${CONFIG}/obj/mdb.obj -Fd${CONFIG}/obj/mdb.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/mdb.c

"${CC}" -c -Fo${CONFIG}/obj/sdb.obj -Fd${CONFIG}/obj/sdb.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/sdb.c

"${LD}" -dll -out:${CONFIG}/bin/mod_esp.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/mod_esp.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.obj ${CONFIG}/obj/espAbbrev.obj ${CONFIG}/obj/espFramework.obj ${CONFIG}/obj/espHandler.obj ${CONFIG}/obj/espHtml.obj ${CONFIG}/obj/espSession.obj ${CONFIG}/obj/espTemplate.obj ${CONFIG}/obj/mdb.obj ${CONFIG}/obj/sdb.obj ${LIBS} libappweb.lib libhttp.lib libmpr.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/esp.obj -Fd${CONFIG}/obj/esp.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/esp/esp.c

"${LD}" -out:${CONFIG}/bin/esp.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/edi.obj ${CONFIG}/obj/esp.obj ${CONFIG}/obj/espAbbrev.obj ${CONFIG}/obj/espFramework.obj ${CONFIG}/obj/espHandler.obj ${CONFIG}/obj/espHtml.obj ${CONFIG}/obj/espSession.obj ${CONFIG}/obj/espTemplate.obj ${CONFIG}/obj/mdb.obj ${CONFIG}/obj/sdb.obj ${LIBS} libappweb.lib libhttp.lib libmpr.lib libpcre.lib

rm -rf ${CONFIG}/bin/esp.conf
cp -r src/esp/esp.conf ${CONFIG}/bin/esp.conf

rm -rf ${CONFIG}/bin/esp-www
cp -r src/esp/www ${CONFIG}/bin/esp-www

rm -rf ${CONFIG}/bin/esp-appweb.conf
cp -r src/esp/esp-appweb.conf ${CONFIG}/bin/esp-appweb.conf

rm -rf ${CONFIG}/inc/ejs.h
cp -r src/deps/ejs/ejs.h ${CONFIG}/inc/ejs.h

rm -rf ${CONFIG}/inc/ejs.slots.h
cp -r src/deps/ejs/ejs.slots.h ${CONFIG}/inc/ejs.slots.h

rm -rf ${CONFIG}/inc/ejsByteGoto.h
cp -r src/deps/ejs/ejsByteGoto.h ${CONFIG}/inc/ejsByteGoto.h

"${CC}" -c -Fo${CONFIG}/obj/ejsLib.obj -Fd${CONFIG}/obj/ejsLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsLib.c

"${LD}" -dll -out:${CONFIG}/bin/libejs.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/libejs.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsLib.obj ${LIBS} libhttp.lib libmpr.lib libpcre.lib libsqlite3.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/ejs.obj -Fd${CONFIG}/obj/ejs.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejs.c

"${LD}" -out:${CONFIG}/bin/ejs.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejs.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libsqlite3.lib

"${CC}" -c -Fo${CONFIG}/obj/ejsc.obj -Fd${CONFIG}/obj/ejsc.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsc.c

"${LD}" -out:${CONFIG}/bin/ejsc.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsc.obj ${LIBS} libejs.lib libhttp.lib libmpr.lib libpcre.lib libsqlite3.lib

cd src/deps/ejs >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.mod --optimize 9 --bind --require null ejs.es ;\
cd - >/dev/null 

"${CC}" -c -Fo${CONFIG}/obj/cgiHandler.obj -Fd${CONFIG}/obj/cgiHandler.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/modules/cgiHandler.c

"${LD}" -dll -out:${CONFIG}/bin/mod_cgi.dll -entry:_DllMainCRTStartup@12 -def:${CONFIG}/bin/mod_cgi.def ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiHandler.obj ${LIBS} libappweb.lib libhttp.lib libmpr.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/authpass.obj -Fd${CONFIG}/obj/authpass.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/utils/authpass.c

"${LD}" -out:${CONFIG}/bin/authpass.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/authpass.obj ${LIBS} libappweb.lib libhttp.lib libmpr.lib libpcre.lib

"${CC}" -c -Fo${CONFIG}/obj/cgiProgram.obj -Fd${CONFIG}/obj/cgiProgram.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/utils/cgiProgram.c

"${LD}" -out:${CONFIG}/bin/cgiProgram.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/cgiProgram.obj ${LIBS}

"${CC}" -c -Fo${CONFIG}/obj/setConfig.obj -Fd${CONFIG}/obj/setConfig.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/utils/setConfig.c

"${LD}" -out:${CONFIG}/bin/setConfig.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/setConfig.obj ${LIBS} libmpr.lib

rm -rf ${CONFIG}/inc/appwebMonitor.h
cp -r src/server/appwebMonitor.h ${CONFIG}/inc/appwebMonitor.h

"${CC}" -c -Fo${CONFIG}/obj/appweb.obj -Fd${CONFIG}/obj/appweb.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/server/appweb.c

"${LD}" -out:${CONFIG}/bin/appweb.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/appweb.obj ${LIBS} libappweb.lib libhttp.lib libmpr.lib libpcre.lib

"rc.exe" -nologo -Fo ${CONFIG}/obj/appwebMonitor.res src/server/windows/appwebMonitor.rc

"${CC}" -c -Fo${CONFIG}/obj/appwebMonitor.obj -Fd${CONFIG}/obj/appwebMonitor.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/server/windows/appwebMonitor.c

"${LD}" -out:${CONFIG}/bin/appwebMonitor.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/appwebMonitor.res ${CONFIG}/obj/appwebMonitor.obj shell32.lib libappweb.lib ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib libhttp.lib libmpr.lib libpcre.lib

rm -rf ${CONFIG}/bin/appwebMonitor.ico
cp -r src/server/windows/appwebMonitor.ico ${CONFIG}/bin/appwebMonitor.ico

rm -rf ${CONFIG}/inc/testAppweb.h
cp -r test/testAppweb.h ${CONFIG}/inc/testAppweb.h

"${CC}" -c -Fo${CONFIG}/obj/testAppweb.obj -Fd${CONFIG}/obj/testAppweb.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc test/testAppweb.c

"${CC}" -c -Fo${CONFIG}/obj/testHttp.obj -Fd${CONFIG}/obj/testHttp.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc test/testHttp.c

"${LD}" -out:${CONFIG}/bin/testAppweb.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/testAppweb.obj ${CONFIG}/obj/testHttp.obj ${LIBS} libappweb.lib libhttp.lib libmpr.lib libpcre.lib

cd test >/dev/null ;\
echo '#!../${CONFIG}/bin/cgiProgram.exe' >cgi-bin/testScript ; chmod +x cgi-bin/testScript ;\
cd - >/dev/null 

cd test >/dev/null ;\
echo "#!`type -p sh`" >web/caching/cache.cgi ;\
echo '' >>web/caching/cache.cgi ;\
echo 'echo HTTP/1.0 200 OK' >>web/caching/cache.cgi ;\
echo 'echo Content-Type: text/plain' >>web/caching/cache.cgi ;\
echo 'date' >>web/caching/cache.cgi ;\
chmod +x web/caching/cache.cgi ;\
cd - >/dev/null 

cd test >/dev/null ;\
echo "#!`type -p sh`" >web/auth/basic/basic.cgi ;\
echo '' >>web/auth/basic/basic.cgi ;\
echo 'echo Content-Type: text/plain' >>web/auth/basic/basic.cgi ;\
echo 'echo' >>web/auth/basic/basic.cgi ;\
echo '/usr/bin/env' >>web/auth/basic/basic.cgi ;\
chmod +x web/auth/basic/basic.cgi ;\
cd - >/dev/null 

cd test >/dev/null ;\
cp ../${CONFIG}/bin/cgiProgram cgi-bin/cgiProgram.exe ;\
cp ../${CONFIG}/bin/cgiProgram cgi-bin/nph-cgiProgram.exe ;\
cp ../${CONFIG}/bin/cgiProgram 'cgi-bin/cgi Program.exe' ;\
cp ../${CONFIG}/bin/cgiProgram web/cgiProgram.cgi ;\
chmod +x cgi-bin/* web/cgiProgram.cgi ;\
cd - >/dev/null 

cd test >/dev/null ;\
cp -r ../src/esp/www/files/static/js 'web/js' ;\
cd - >/dev/null 

"${CC}" -c -Fo${CONFIG}/obj/removeFiles.obj -Fd${CONFIG}/obj/removeFiles.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc package/windows/removeFiles.c

"${LD}" -out:${CONFIG}/bin/removeFiles.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/removeFiles.obj ${LIBS} libmpr.lib

