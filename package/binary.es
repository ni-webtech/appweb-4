/*
 	binary.files - Files needed for the binary installation 

 	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
 */

var top = App.dir.findAbove("configure").dirname

load(top.join("package/copy.es"))
var bare: Boolean = App.args[3] == "1"
var options = copySetup({task: App.args[1], root: Path(App.args[2])})
var build = options.build
var os = build.BLD_HOST_OS
var product = build.BLD_PRODUCT
var debug: Boolean = build.BLD_DEBUG
var strip = !debug

/*
    Sources
 */
var sout: Path = build.BLD_OUT_DIR
var sbin: Path = build.BLD_BIN_DIR
var sinc: Path = build.BLD_INC_DIR
var slib: Path = build.BLD_LIB_DIR

/*
    Destinations
 */
var root: Path = build.BLD_ROOT_PREFIX
var bin: Path = build.BLD_BIN_PREFIX
var inc: Path = build.BLD_INC_PREFIX
var lib: Path = build.BLD_LIB_PREFIX
var ver: Path = build.BLD_VER_PREFIX
var cfg: Path = build.BLD_CFG_PREFIX
var log: Path = build.BLD_LOG_PREFIX
var man: Path = build.BLD_MAN_PREFIX
var spl: Path = build.BLD_SPL_PREFIX
var web: Path = build.BLD_WEB_PREFIX
var ssl: Path = cfg.join("ssl")
var etc: Path = root.join("etc")
var init: Path = etc.join("init")
var initd: Path = etc.join("init.d")
var cache: Path = spl.join("cache")

let user = 0, group = 0
if (Config.OS != "WIN") {
    let passwords = Path("/etc/passwd").readString()
    for each (u in ["www-data", "_www", "nobody", "Administrator"]) {
        if (passwords.contains(u + ":")) {
            user = u
            break
        }
    }
    let groups = Path("/etc/group").readString()
    for each (g in ["www-data", "_www", "nobody", "nogroup", "Administrator"]) {
        if (groups.contains(g + ":")) {
            group = g
            break
        }
    }
    if (!user || !group) {
        App.log.error("Can't find acceptable user or group for files")
        App.exit(1)
    }
}

var lowperms = {permissions: 0755, owner: user, group: group }
var dperms = {permissions: 0755, owner: 0, group: 0 }
bin.makeDir(dperms)
inc.makeDir(dperms)
lib.makeDir(dperms)
ver.makeDir(dperms)
cfg.makeDir(dperms)
cfg.join("config").makeDir(dperms)
web.makeDir(dperms)
var saveLink : Path

if (!bare) {
    man.join("man1").makeDir(dperms)
    lib.join("www").makeDir(dperms)

    if (options.task == "Remove" && bin.join("linkup").exists) {
        saveLink = Path(".").temp()
        bin.join("linkup").copy(saveLink)
        saveLink.attributes = {permissions: 0755}
    }
}

if (!bare) {
    copy("setConfig*", bin, {from: sbin, permissions: 0755, strip: strip})
    copy("LICENSE.TXT", ver, { from: "doc/licenses", fold: true, expand: true })
    copy("*.TXT", ver, { from: "doc/product", fold: true, expand: true })
    copy("uninstall.sh", bin.join("uninstall"), {from: "package", permissions: 0755, expand: true})
    copy("linkup", bin.join("linkup"), {from: "package", permissions: 0755, expand: true})

    let cmdFilter
    if (Config.OS == "WIN") {
        cmdFilter = /\.cmd|\.sln|\.suo/
    } else {
        cmdFilter = /undefined/
    }
    //  This will pickup .lib files on windows
    copy("*", bin, {
        from: sbin,
        include: /appman|esp|http|auth|makerom|libappweb|libmpr/
        exclude: cmdFilter,
        permissions: 0755,
    })
    log.makeDir(lowperms)
    let dummy = log.join("error.log")
    dummy.write("")
    dummy.attributes = dperms
    spl.makeDir(lowperms)
    cache.makeDir(lowperms)
    dummy = cache.join(".dummy")
    dummy.write("")
    dummy.attributes = dperms

    copy("*", web, {
        from: "src/server/web",
        exclude: /mgmt\//,
        recurse: true,
    })
    copy("*", web.join("test"), {
        from: "src/server/web/test",
        include: /.cgi|test.pl|test.py/,
        recurse: true,
        permissions: 0755,
    })
    copy("*", inc, {
        from: sinc,
        exclude: /appwebMonitor.h|testAppweb.h/,
    })
    copy("*", cfg, {
        from: "src/server",
        include: /mime.types/,
        permissions: 0644,
    })
    copy("*", lib, {
        from: slib,
        include: /esp-www/,
        exclude: /files.save/,
        permissions: 0644,
        recurse: true,
    })

} else {
    copy("src/server/web/min-index.html", web.join("appweb.html"))
}

copy("appweb*", bin, {from: sbin, permissions: 0755, strip: strip})
copy("*" + build.BLD_SHOBJ, lib, {
    from: slib, 
    exclude: /simple|sample/,
    permissions: 0755, 
    strip: strip
})
copy("*", lib, {
    from: slib,
    include: /esp.conf/,
    permissions: 0644,
})

if (options.task != "Remove" && build.BLD_FEATURE_SSL == 1 && os == "LINUX") {
    /* 
        Symlink to sonames for openssl
     */
    copy("*" + build.BLD_SHOBJ + ".*", lib, {from: slib, permissions: 0755, strip: strip})
    for each (f in slib.find("*.so.*")) {
        let withver = f.basename
        let nover = withver.name.replace(/\.[0-9]*.*/, ".so")
        Cmd.sh("rm -f " + lib.join(nover))
        Cmd.sh("ln -s " + withver + " " + lib.join(nover))
    }
}

copy("*", cfg, {
    from: "src/server",
    include: /php.ini|appweb.conf/,
    exclude: /appweb.conf.bak/,
    permissions: 0644,
})

if (build.BLD_HOST_OS == "WIN") {
    if (options.task != "Remove") {
        Cmd(["setConfig", 
            "--home", ".", 
            "--documents", "web", 
            "--logs", "logs", 
            "--port", build.BLD_HTTP_PORT, 
            "--ssl", build.BLD_SSL_PORT, 
            "--cache", "cache", 
            "--modules", "bin", 
            cfg.join("appweb.conf")])
        cfg.join("appweb.conf.bak").remove()
    }
} else {
    if (options.task != "Remove") {
        Cmd(["setConfig", 
            "--home", build.ORIG_BLD_CFG_PREFIX, 
            "--documents", build.ORIG_BLD_WEB_PREFIX, 
            "--logs", build.ORIG_BLD_LOG_PREFIX, 
            "--port", build.BLD_HTTP_PORT, 
            "--ssl", build.BLD_SSL_PORT, 
            "--user", user,
            "--group", group,
            "--cache", Path(build.ORIG_BLD_SPL_PREFIX).join("cache"), 
            "--modules", build.ORIG_BLD_LIB_PREFIX, 
            cfg.join("appweb.conf")])
        cfg.join("appweb.conf.bak").remove()
    }
}

if (build.BLD_FEATURE_EJSCRIPT == 1) {
    copy("ejs*.mod", lib, {from: slib})
}

/*
    Service startup scripts
 */
if (!bare) {
    if (os == "MACOSX") {
        let daemons = root.join("Library/LaunchDaemons")
        daemons.makeDir(dperms)
        copy("com.embedthis.appweb.plist", daemons, {from: "package/MACOSX", permissions: 0644, expand: true})

    } else if (os == "LINUX") {
        if (options.task == "Package") {
            init.makeDir(dperms)
            initd.makeDir(dperms)
        }
        copy("package/LINUX/" + product + ".init", initd.join(product), {permissions: 0755, expand: true})
        copy("package/LINUX/" + product + ".upstart", init.join(product).joinExt("conf"), 
            {permissions: 0644, expand: true})
        if (options.task != "Package") {
            if (App.uid == 0 && options.root != "") {
                if (options.openwrt) {
                    root.join("CONTROL").makeDir(dperms)
                    copy("p*", root.join("CONTROL"), {from: "package/LINUX/deb.bin", permissions: 0755, expand: true})
                    copy("appweb.openwrt", initd.join(product), {from: "package/LINUX", permissions: 0755, expand: true})
                }
            }
        }
    }
    if (build.BLD_HOST_OS == "WIN") {
        if (build.BLD_CC_CL_VERSION == 16) {
            copy("msvcrt.lib", bin, {from: build.BLD_VS})
            copy("msvcr100.dll", bin, {from: build.BLD_VS.parent.join("redist/x86/Microsoft.VC100.CRT")})
        }
        copy("removeFiles*", bin, {from: sbin, permissions: 0755})
        copy("setConf*", bin, {from: sbin, permissions: 0755})
    }
}

if (build.BLD_UNIX_LIKE == 1) {
    copy("*.1", man.join("man1"), {from: "doc/man", compress: true })
}

if (options.task == "Install") {
    if (!bare && build.BLD_HOST_OS != "WIN") {
        Cmd.sh([bin.join("linkup"), options.task, options.root])
    }
} else if (saveLink && saveLink.exists) {
    if (build.BLD_HOST_OS != "WIN") {
        Cmd.sh([saveLink, options.task, options.root])
    }
    saveLink.remove()
}
