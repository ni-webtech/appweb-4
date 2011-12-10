/*
 	source.files - Files needed for the source installation
 	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
 */

var top = App.dir.findAbove("configure").dirname
load(top.join("package/copy.es"))
var bare: Boolean = App.args[3] == "1"
var options = copySetup({task: App.args[1], root: Path(App.args[2])})
var build = options.build
var src: Path = build.BLD_SRC_PREFIX

src.makeDir()
copy("Makefile", src)
copy("LICENSE.TXT", src, {from: "doc/licenses", fold: true, expand: true})
copy("*.TXT", src, {from: "doc/product", fold: true, expand: true})
copy("configure", src, {permissions: 0755})

copy("build/*", src, {
    recurse: true,
    include: /Makefile|Makefile.top|configure\.|\.defaults|config\.|make\.|\.config/,
})

copy("build/bin/*", src, {
    permissions: 0755,
    include: /getImports|getlib|makedep|priv|fakePriv|utest*|bld$|make.*Package.*|fakePriv|mkdmg|makeInstall|combo|makedep|cacheConfig|patchAppweb|patchFile/,
    trace: true,
})

copy("build/src/*", src, {
    include: /^Makefile$|\.c$|\.h$/,
})

copy("build/components/*", src)

copy("package/*", src, {
    include: /Makefile|\.files|\.es/,
})

copy("package/*", src, {
    include: /\.sh|makeInstall\.|linkup/
    permissions: 0755,
})

copy("package/*", src, {
    recurse: true,
    include: /LINUX|MACOSX|WIN/
})

copy("src/*.c", src, {})

copy("src/*", src, {
    recurse: true,
    include: /^src\/Makefile|^src\/deps|^src\/utils|^src\/server|^src\/test|^src\/modules|^src\/esp|^src\/samples|\.h$/,
    exclude: /\.log$|fcgi|ejs.debugger|ejs.cgi|\.lst$|ejs.zip|\.stackdump$|\/cache\/view_|huge.txt|\.swp$|^src\/deps\/ejs/
})

copy("doc/*", src, {
    recurse: true,
    exclude: /\/xml\/|\/html\/|Archive|\.mod$|\.so$|\.dylib$|\.o$/
})

copy("projects/*", src, {
    recurse: true,
    exclude: /\/Debug\/|\/Release\/|\.ncb|\.mode1v3|\.pbxuser/,
})

if (build.BLD_HOST_OS != "WIN") {
    Cmd.sh("chown -R 0 " + src)
    Cmd.sh("chgrp -R 0 " + src)
}
