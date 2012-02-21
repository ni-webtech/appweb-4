/*
    Support functions for Embedthis products

    Exporting: install(), package()

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.unix

public function getWebUser(): String {
    let passwdFile: Path = Path("/etc/passwd")
    if (passwdFile.exists) {
        let passwords = passwdFile.readString()
        for each (u in ["www-data", "_www", "nobody", "Administrator"]) {
            if (passwords.contains(u + ":")) {
                return u
            }
        }
    }
    return '0'
}

public function getWebGroup(): String {
    let groupFile: Path = Path("/etc/group")
    if (groupFile.exists) {
        let groups = groupFile.readString()
        for each (g in ["www-data", "_www", "nobody", "nogroup", "Administrator"]) {
            if (groups.contains(g + ":")) {
                return g
            }
        }
    }
    return '0'
}


function installCallback(src: Path, dest: Path, options = {}): Boolean {
    options.task ||= 'install'

    src = src.relative.portable
    if (options.exclude && src.match(options.exclude)) {
        return false
    }
    if (options.copytemp && src.match(TempFilter)) {
        return false
    }
    if (options.include && !src.match(options.include)) {
        return false
    }
    if (options.task == 'uninstall') {
        if (options.compress) {
            dest = Path(dest.name + '.gz')
        }
        vtrace(options.task.toPascal(), dest.relative)
        dest.remove()
        return true
    }
    let attributes = { 
        owner: options.owner || -1, 
        group: options.group || -1, 
        permissions: options.permissions || (src.extension.match(/exe|lib|so|dylib|sh|es/) ? 0755 : 0644)
    }
    if (options.cat) {
        vtrace('Combine', dest.relative + ' += ' + src.relative)
        if (!dest.exists) {
            if (options.title) {
                dest.write('/*\n' +
                           '    ' + dest.basename + ' -- ' + options.title + '\n\n' +
                           '    This file is a catenation of all the source code. Amalgamating into a\n' +
                           '    single file makes embedding simpler and the resulting application faster.\n */\n\n')
            }
            if (options.header) {
                dest.append(options.header + '\n')
            }
        }
        dest.append('\n' +
           '/************************************************************************/\n' +
           '/*\n    Start of file \"' + src.relative + '\"\n */\n' +
           '/************************************************************************/\n\n')
        let data = src.readString()
        if (options.filter) {
            data = data.replace(options.filter, '')
        }
        dest.append(data)
        dest.attributes = attributes
    } else {
        vtrace(options.task.toPascal(), dest.relative)
        dest.parent.makeDir()
        src.copy(dest, attributes)
    }
    if (options.patch) {
        trace('Patch', dest)
        dest.write(dest.readString().expand(bit))
        dest.attributes = attributes
    }
    if (options.fold && bit.platform.like == 'windows') {
        trace('Fold', dest)
        fold(dest, options)
        dest.attributes = attributes
    }
    if (options.strip && bit.packs.strip) {
        trace('Strip', dest)
        Cmd.run(bit.packs.strip.path + ' ' + dest)
    }
    if (options.compress) {
        trace('Compress', dest)
        dest = Path(dest.name + '.gz')
        dest.remove()
        Cmd.run('gzip --best ' + dest)
    }
    if (App.uid == 0 && dest.extension == 'so' && Config.OS == 'LINUX' && options.task == 'install') {
        trace('Ldconfig', dest)
        Cmd.run('ldconfig ' + dest)
    }
    return true
}

/*
    Install and uninstall files.
    If options.task is 'install' or 'package', the files are installed. If the options.task is 'uninstall', the 
        files are removed.
    @param src Source path. May contain glob style wild cards including '*', '?' and '**'. May also be an array
        of source paths.
    @param dest Destination path
    @param options Process options
    @option compress Compress target file
    @option copytemp Copy files that look like temp files
    @option exclude Exclude files that match the pattern. The pattern should be in portable file format.
    @option expand Object hash containing properties to use when replacing tokens of the form ${token} in the
        src and dest filenames. Defaults to 'bit'.
    @option fold Fold long lines on windows at column 80 and convert new line endings.
    @option group Set file group
    @option include Include files that match the pattern. The pattern should be in portable file format.
    @option owner Set file owner
    @option patch Object hash containing properties to use when replacing tokens of the form ${token} in the
        destination file contents.  
    @option perms Set file perms
    @option strip Strip object or executable
 */
public function install(src, dest: Path, options = {}) {
    if (!(src is Array)) src = [src]
    if (options.cat) {
        let files = []
        for each (pat in src) {
            files += Path('.').glob(pat)
        }
        src = files.unique()
    }
    cp(src, dest, blend({process: this.installCallback, expand: bit, warn: true}, options))
}

/*
    Fold long lines at column 80. On windows, will also convert line terminatations to <CR><LF>.
 */
function fold(path: Path, options) {
    let lines = path.readLines()
    let out = new TextStream(new File(path, 'wt'))
    for (l = 0; l < lines.length; l++) {
        let line = lines[l]
        if (options.fold && line.length > 80) {
            for (i = 79; i >= 0; i--) {
                if (line[i] == ' ') {
                    lines[l] = line.slice(0, i)
                    lines.insert(l + 1, line.slice(i + 1))
                    break
                }
            }
            if (i == 0) {
                lines[l] = line.slice(0, 80)
                lines.insert(l + 1, line.slice(80))
            }
        }
        out.writeLine(lines[l])
    }
    out.close()
}

public function package(formats) {
    let rel = bit.dir.rel
    let flat = bit.dir.flat
    let s = bit.settings
    let name, generic
    let pkg
    if (!(formats is Array)) formats = [formats]

    for each (fmt in formats) {
        pkg = bit.dir.pkg
        if (fmt == 'flat') {
            safeRemove(flat)
            flat.makeDir()
            for each (f in pkg.glob("**", {nodirs: true})) {
                f.copy(flat.join(f.basename))
            }
            pkg = flat
        }
        name = rel.join(s.product + '-' + s.version + '-' + s.buildNumber + '-' + fmt + '.tgz')
        generic = rel.join(s.product + '-' + fmt + '.tgz')
        if (bit.platform.os == 'linux') {
            run('tar -C ' + pkg + ' --owner 0 --group 0 -czf ' + name + ' .')
        } else {
            run('tar -C ' + pkg + ' -czf ' + name + ' .')
        }
        //MOB  Path(name).symlink(generic)
        Path(generic).remove()
        run('ln -s ' + name + ' ' + generic)
        trace('Package', name)
        trace('Package', generic)
    }
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
