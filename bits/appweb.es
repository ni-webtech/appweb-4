/*
    Support functions for Embedthis products
    Exporting: install(), package()

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix

/*
    Copy binary files to package staging area
 */
public function packageBinaryFiles() {
    let settings = bit.settings
    let bin = bit.dir.pkg.join('bin')
    safeRemove(bit.dir.pkg)
    let vname = settings.product + '-' + settings.version + '-' + settings.buildNumber
    let pkg = bin.join(vname)
    pkg.makeDir()

    /* These three files are replicated outside the data directory */
    install('doc/product/README.TXT', pkg, {fold: true, expand: true})
    install('package/install.sh', pkg.join('install'), {permissions: 0755, expand: true})
    install('package/uninstall.sh', pkg.join('uninstall'), {permissions: 0755, expand: true})
    let contents = pkg.join('contents')

    let prefixes = bit.prefixes;
    let p = {}
    for (prefix in bit.prefixes) {
        p[prefix] = Path(contents.name + bit.prefixes[prefix])
        p[prefix].makeDir()
    }
    let strip = settings.profile == 'debug'

    install('LICENSE.md', p.product, {fold: true, expand: true})
    install('doc/product/README.TXT', p.product, {fold: true, expand: true})
    install('package/uninstall.sh', p.bin.join('uninstall'), {permissions: 0755, expand: true})
    install('package/linkup', p.bin, {permissions: 0755})
    install(bit.dir.bin + '/*', p.bin, {
        include: /appweb|appman|esp|http|auth|makerom|libappweb|libmpr|setConfig/,
        permissions: 0755, 
    })
    install(bit.dir.bin.join('appweb'), p.bin, {
        permissions: 0755, 
        strip: strip,
    })
    install(bit.dir.lib.join('*'), p.lib, {
        permissions: 0755, 
        exclude: /file-save|www|simple|sample/,
    })
    install('src/server/appweb.conf', p.config)
    install('src/server/mime.types', p.config)
    install('src/server/php.ini', p.config)
    install('src/server/web/**/*', p.web, {exclude: /mgmt/ })
    install('src/server/web/test/*', p.web.join('test'))
    install('src/server/web/test/*', p.web.join('test'), {
        include: /.cgi|test.pl|test.py/,
        permissions: 0755,
    })
    install(bit.dir.lib + '/esp.conf', p.lib)
    install(bit.dir.lib + '/esp-www', p.lib)
    let user = getWebUser(), group = getWebUser()
    p.spool.join('cache').makeDir()
    let tmp = p.spool.join('cache/.dummy')
    tmp.write()
    tmp.setAttributes({permissions: 0755, uid: user, gid: group})
    tmp = p.log.join('error.log')
    tmp.write()
    tmp.setAttributes({permissions: 0755, uid: user, gid: group})

    //  MOB - should this apply to non-openssl ?
    if (bit.packs.ssl.enable && bit.platform.os == 'linux') {
        install(bit.dir.lib.join('*.' + bit.ext.shobj + '*'), p.lib, {strip: strip, permissions: 0755})
        for each (f in p.lib.glob('*.so.*')) {
            let withver = f.basename
            let nover = withver.name.replace(/\.[0-9]*.*/, '.so')
            let link = p.lib.join(nover)
            f.remove()
            f.symlink(link.basename)
        }
    }
    let conf = Path(contents.name + bit.prefixes.config + '/appweb.conf')
    if (bit.platform.os == 'win') {
        run(['setConfig', '--home', '.', '--documents', 'web', '--logs', 'logs', '--port', settings.http_port,
            '--ssl', settings.ssl_port, '--cache', 'cache', '--modules', 'bin', conf])
    } else {
        run(['setConfig', '--home', prefixes.config, '--documents', prefixes.web, '--logs', prefixes.log,
            '--port', settings.http_port, '--ssl', settings.ssl_port, '--user', user, '--group', group,
            '--cache', prefixes.spool.join('cache'), '--modules', prefixes.lib, conf])
    }
    bit.dir.cfg.join('appweb.conf.bak').remove()

    if (bit.packs.ejscript.enable) {
        install(bit.dir.lib.join('ejs*.mod'), p.lib);
    }
    if (OS == 'macosx') {
        let daemons = contents.join('Library/LaunchDaemons')
        daemons.makeDir()
        install('package/MACOSX/com.embedthis.appweb.plist', daemons, {permissions: 0644, expand: true})

    } else if (OS == 'linux') {
        install('package/LINUX/' + settings.product + '.init', 
            contents.join('etc/init.d', settings.product), 
            {permissions: 0755, expand: true})
        install('package/LINUX/' + settings.product + '.upstart', 
            contents.join('init', settings.product).joinExt('conf'),
            {permissions: 0644, expand: true})

    } else if (OS == 'win') {
        install('msvcrt.lib', bin, {from: bit.packs.compiler.parent.join('redist/x86/Microsoft.VC100.CRT')})
        install('removeFiles*', bin, {from: bit.dir.bin, permissions: 0755})
        install('setConf*', bin, {from: bit.dir.bin, permissions: 0755})
    }
    if (bit.platform.like == 'posix') {
        install('doc/man/*.1', p.productver.join('doc/man/man1'), {compress: true})
    }
}

/*
    Create symlinks for binaries and man pages
 */
public function createLinks() {
    let programs = ['appman', 'appweb', 'http', 'auth', 'esp']
    let localbin = Path('/usr/local/bin')
    let bin = bit.prefixes.bin
    let target: Path
    let log = []

    for each (program in programs) {
        let link = Path(localbin.join(program))
        bin.join(program).symlink(link)
        log.push(link)
    }
    for each (page in bit.prefixes.productver.join('doc/man').glob('**/*.1.gz')) {
        let link = Path('/usr/share/man/man1/' + page.basename)
        page.symlink(link)
        log.push(link)
    }
    bit.prefixes.productver.join('files.log').append(log.join('\n') + '\n')
}

public function startAppweb() {
    trace('Start', 'appman install enable start')
    Cmd.run('/usr/local/bin/appman install enable start')
}


/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
