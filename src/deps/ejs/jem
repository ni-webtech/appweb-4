#!/usr/bin/env ejs

/*
    BUGS
    - Inconsistent trace. Build does, publish doesnt
 */
GC.enabled = false

/*
    Background
        - Jem Repository contents are a git repository
        - Packages are a versioned directory containing package description file and *.jem
        - Catalogue
    - Web App
        /admin/     - 
        /module/    - Control modules: enable/disable, publish/retract, flag latest version
        /lookup/name?min=
            - Download jem given a jem name 
            - Upload a new jem (have authorized publishers)
            - jemrc can have publishers key
    - Doc
        - Package
            [Stability: prototype, evolving, development, production, stable, mature, deprecated]
        - Categories
            [samples, comms, databases, ...]
    PUNCH LIST
    - Man page
    - Think through catalog
        - Don't keep local catalog
        - Perhaps Remote ops should download catalog every time?
            install
            search
    - install from http
    - install to/from switches
    - Test install.es, uninstall.es, build.es
        - Need to use onmessage technique like utest does to get config into install/uninstall.es
    - Install binaries to wherever
    - Build doc
        - Generate doc from source
        - How is doc aggregated from jems
            - Need one top level UI from which all jem doc can be read
    - Run unit tests
    - Test package with a native module
        - Create 2 sample jems (one native, one not)
        - copy doc to /usr/lib/ejs/???
        - Test versioning
    - Consider creating a build.es (aka Jake)
    - Jem Commands
        - jem --from repository is the same as jem --repository
            *** Standardize on --to --from and don't use --repository for anything
        - jem search {keyword, .., .., -keyword}
          jem --local search ...
        - jem --add repository URL
        - jem --remove repository URL
        - jem rebuild   # what does this do?
        - jem server    [ run a local jem web server]
        - jem reinstall
        - jem update        If updating a local catalog is required
        - jem publish --user user:password
    - Sign, compress, md5
    - Need more detailed help
    - Need a single user lock to ensure only one person 
        - Locking so that a jem won't be found in a partial state
            - Only one may use at a time
            - Must be ^C safe
    - Implement / Test
        install
            --code
            --from repository
            --repository
        publish
            --to
        retract
        rebuild
        upgrade
        list
            --remote
    - Create jems.embedthis.com
        /ejs
        /commonjs

 */

module ejs.jem {

require ejs.unix
require ejs.tar

const MAX_VER: Number = 1000000000
const VER_FACTOR: Number = 1000

var dirs: Object
var filenames: Object
//  MOB -- should be extensions
var ext: Object

class JemCmd 
{
    private const RC: String = ".jemrc"
    private const DIR_PERMS: Number = 0775

    private var appName: String = "jem"
    private var cmd: CmdArgs
    private var config: Object
    //  MOB -- rename to extensions, move with dirs?
    private var libext: String
    //  MOB -- should use logger
    private var logFile: String = "stdout"
    private var logLevel: Number = 1
    private var out: File = App.outputStream
    private var searchPath: String
    private var tempFile: Path

    private var requiredKeywords = [ "author", "bugReports", "categories", "cpu", "dependencies", "description", 
        "contributors", "keywords", "license", "name", "os", "patchable", 
        "location", "stability", "version",
    ]
    private var requiredDirs = [
        "bin", "doc", "lib", "cache", "src", "test",
    ]
    /* List of system dirs as added protection - must not remove */
    private var sysdirs = [ "", "/", ".", "..", "/bin", "/usr/bin", "/tmp", "/etc", "/var", "/Program Files",
    ]

    /* This layers over App.config */
//  MOB -- remove "."
    private var defaultConfig = {
        extensions: {
            es: ".es",
            js: ".js",
            ejs: ".ejs",
            jem: ".jem",
            mod: ".mod",
            lib: ".so"
        },
        directories: {
            //  MOB -- /usr/lib/ejs should come from a single constant
            _ejs: Path("/usr/lib/ejs"),
            _jems: Path("/usr/lib/ejs/jems"),
            _modules: Path("/usr/lib/ejs/modules"),
            _lib: Path("/usr/lib/ejs/lib"),
            _bin: Path("/usr/lib/ejs/bin"),
            _repositories: [ Path("http://jems.ejscript.org") ],

            //  TODO - is this used?
            ejs: Path("XXXXX"),
            jems: Path("/Users/mob/git/ejs/jlocal"),
            modules: Path("/Users/mob/git/ejs/modules"),
            lib: Path("/Users/mob/git/ejs/lib"),
            bin: Path("/Users/mob/git/ejs/bin"),
            repositories: [ Path("/Users/mob/git/ejs/jremote") ],
        },
        filenames: {
            package: Path("package.json"),
            cache: Path("cache")
        },
    }

    function JemCmd() {
        loadDefaults()
//  MOB -- should update App.config
        blend(config, defaultConfig, false)
        dirs = config.directories
        dirs.home = App.dir
//  MOB -- rename ext to extensions
        ext = config.extensions
        filenames = config.filenames
        for (d in dirs) {
            if (d != "repositories") {
                dirs[d] = Path(dirs[d])
            }
        }
        for (d in dirs.repositories) {
            dirs.repositories[d] = Path(dirs.repositories[d])
        }
        //  MOB -- should not have "." prefix
        // TODO - should be in Config
        if (Config.OS == "MACOSX") {
            ext.lib = ".dylib"
        } else if (Config.OS == "WIN") {
            ext.lib = ".dll"
        } else {
            ext.lib = ".so"
        }
    }

//  MOB -- these options dont match usage below
    private var cmdOptions = [
        [ "all" ],
        [ "code", String ],
        [ "debug" ],
        [ "details" ],
        [ "dir", String ],
        [ "field", String ],
        [ "force" ],
        [ "from", String ],
        [ "log", /\w+(:\d)/, "stdout:1" ],
        [ "repository", String ],
        [ ["quiet", "q"] ],
        [ "to", String ],
        [ "user" ],
        [ "user", String ],
        [ "value", String ],
        [ ["verbose", "v"] ],
        [ "version", String ],
        [ "versions" ],
    ]

    function usage(): Void {
        print("\nUsage: " + appName + " [options] [commands] ...\n" +
            "  Commands:\n" + 
    //MOB -- missing build
            "    create files ...     # Create a new jem package\n" +
            "    depend               # Display jem dependencies\n" +
            "    edit                 # Edit a package description file \n" +
            "    help                 # Display this usage help\n" +
            "    install              # Install a jem on the local system\n" +
            "    list                 # list installed jems\n" +
            "    publish              # Upload and publish a jem in a repository\n" +
            "    rebuild              # Rebuild all local jems\n" +
            "    retract              # Remove a jem from a repository\n" +
            "    uninstall            # Uninstall a jem on the local system\n" +
            "    upgrade              # Upgrade all installed jems\n" +
            "  General options:\n" + 
            "    --dir                # Change to directory before running\n" +
            "    --force              # Ignore dependencies and continue\n" +
            "    --from repository\n" +
            "    --to repository\n" +
            "    --log file:level     # Send log output to a file at a given level\n" + 
//  MOB - is this the same as --from repository?
            "    --repository path    # Use specified repository for all actions\n" +
            "    --quiet              # Run in quiet mode\n" +
            "    --verbose            # Run in verbose mode (default)\n" +
            "  Edit options:\n" + 
            "    --field name         # Package field name to edit \n" +
            "    --value value        # Value to set field to\n" +
            "  List options:\n" + 
            "    --all                # Show all versions for a jem individually\n" +
            "    --details            # Show jem details\n" +
            "    --remote             # List jems in remote repositories\n" + 
            "    --versions           # Show jem version information\n" +
            "  Install options:\n" + 
            "    --code code          # Download authorization code\n" + 
            "    --force              # Ignore dependencies\n" +
            "  Publish/retract options:\n" + 
            "   --user user:password\n" + 
            "")
        /*
            Private commands
            setdeps                   # Set dependencies from mod files
         */
        App.exit(1)
    }

    function main() {
        //  MOB -- refactor. Shouldn't need to use try with CmdArgs every time
        try {
            cmd = CmdArgs(cmdOptions)
        } catch (e) {
            error(e)
            usage()
        }
        try {
            processOptions(cmd)
            if (cmd.args.length == 0) {
                usage()
            }
            process()
        } catch (e) {
print(e)
            if (e is String) {
                msg = e
                error("jem: Error: " + msg + "\n")
            } else {
                msg = e.message
                error("jem: Error: " + msg + "\n")
                if (!cmd || cmd.options.debug) {
                    error(e.stack + "\n")
                }
            }
            App.exit(2)
        } finally {
            cleanup()
        }
    }

    function processOptions(cmd: CmdArgs) {
        let options = cmd.options
        if (options.version) {
            print(Config.version)
            App.exit(0)
        }
        if (options.search) {
            searchPath = App.searchPath = options.search.split(Path.SearchSeparator)
        }
        if (options.log) {
            //  MOB -- should use logger
            setLog(options.log)
        }
        if (options.quiet) {
            logLevel = 0
        }
        if (options.verbose) {
            logLevel = 2
        }
        if (options.version) {
            let parts = version.split(":")
            //  TODO - rethink. Is this the right place for this? Name should be minVer, maxVer
            options.min = parts[0]
            options.max = parts[1]
        } else {
            options.min = 0
            options.max = MAX_VER
        }
    }

    function process() {
        let args = cmd.args
        let task = args.shift()

        switch (task) {
        case "build":
            //  MOB -- what are build args?
            build(args)
            break
        case "catalog":
        case "catalogue":
            catalog(args)
            break
        case "create":
            create(args)
            break
        case "depend":
            if (args.length == 0) {
                dependAll()
            } else for each (jem in args) {
                depend(Jem(jem))
            }
            break
        case "dir":
            App.dir = args[0]
            break
        case "edit":
            if (args.length == 0) {
                edit(".")
            } else {
                for each (path in args) {
                    edit(path)
                }
            }
            break
        case "help":
            usage()
            break
        case "install":
            for each (jem in args) {
                install(Jem(jem))
            }
            break
        case "list":
            list(args)
            break
        case "publish":
            for each (jem in args) {
                publish(Jem(jem))
            }
            break
        case "rebuild":
            rebuild()
            break
        case "retract":
            for each (jem in args) {
                retract(Jem(jem))
            }
            break
        case "uninstall":
            for each (jem in args) {
                uninstall(Jem(jem))
            }
            break
        case "upgrade":
            if (args.length == 0) {
                upgradeAll()
            } else for each (jem in args) {
                upgrade(Jem(jem))
            }
            break

        //  Private commands
        case "setdeps":
            setdeps()
            break

        default:
            usage()
            break
        }
        return true
    }

    /*
        Add a file to a jem tar archive
     */
    function addToTar(tar: Tar, file: Path, optional: Boolean = true) {
        if (!file.exists) {
            if (optional) return
            error("File " + file + " is missing")
        }
        log(2, "Append", file)
        tar.add(file)
    }

/* UNUSED
    function bestInstalled(jem: Jem, min: Number = 0, max: Number = MAX_VER): Path {
        let best
        for each (path in find(dirs.jems, jem + "*", false).sort()) {
            //  TODO add Path.contains()
            if (path.toString().contains("-")) {
                version = parseVersionedPath(path)
            } else {
                version = 0
            }
            if (min <= version && version <= max) {
                best = path
            }
        }
        return best
    }
*/

    function buildJem(jem: Jem) {
        log(1, "Building", jem.vername)
        let dir = jem.local
        let path: Path = dir.join("build.es")
        if (path.exists) {
            log(1, "Building", "build.es")
            Worker().load(path)
        } else {
            let files = []
            //  TODO - remove 1 ||
            if (jem.package && (1 || jem.package.ejs)) {
                for each (e in [ext.es, ext.js]) {
                    files += find(dir, "*" + e, true)
                }
            } else {
                files = find(dir.join("lib"), "*" + ext.js, true)
            }
            //  TODO - our should be just do lib as per CommonJS
            files -= ["build.es", "install.es", "remove.es"]
            if (files.length > 0) {
                makeDir(dir.join(filenames.cache))
                let out = dir.join(filenames.cache).join(jem.vername + ext.mod)
                if (logLevel > 1) {
                    log(1, "Compiling", "ejsc --out " + out + " " + files.join(" "))
                } else {
                    log(1, "Compiling", jem.vername)
                }
                Cmd.sh("ejsc --out " + out + " " + files.join(" "))
            }
        }
    }

    /*
        Build a jem file
        @param files Optional list of files to include in the package. Otherwise, build() discovers the files in the
            current directory.
     */
    function build(files: Array): Void {
        //  TODO - should build dependency list from the "require" statements - ejsmod to create
        if (!filenames.package.exists) {
            error("Missing package.json. Run jem in the directory containing the package description")
        }
        let package = loadPackageDescription(".")
        let path = package.name + "-" + package.version + ext.jem
        validatePackage(package)
        try {
            tar = new Tar(path)
            addToTar(tar, filenames.package, true)
            if (files.length > 0) {
                //  TODO - should validate files. Must contain ...
                for each (f in files) {
                    addToTar(tar, f)
                }
            } else {
                //  MOB -- need better filtering. Perhaps a file list would be better
                files =  find(".", "*" + libext, true)
                files += find(".", "*.mod", true)
                files += find(".", "*.es", true)
                files += find("doc", "*", true)
                files += find("bin", "*", true)
                files += find("lib", "*", true)
                files += find("test", "*", true)
                for each (f in files) {
                    if (f.isRegular) {
                        addToTar(tar, f)
                    }
                }
                addToTar(tar, "install.es")
                addToTar(tar, "uninstall.es")
            }
            tar.create()
            log(1, "Building", path)
        } catch (e) {
            print(e)
        }
    }

    /*
        Check if a jem is installed. This will check the required versions specified by jem.min and jem.max if specified.
     */
    function checkInstalled(jem: Jem): Boolean {
        log(2, "CHECK", "Checking \"" + jem + "\" min " + jem.min + " max " + jem.max)
        for each (path in find(dirs.jems, jem.name + "*", false).sort()) {
            let name = path.basename.toString()
            if (name == jem.name) {
                return true
            }
            if (name.split("-")[0] == jem.name) {
                if (name.contains("-")) {
                    version = parseVersionedPath(path)
                } else {
                    version = 0
                }
                if (jem.min <= version && version <= jem.max) {
                    return true
                }
            }
        }
        return false
    }

    private function cleanup() {
        if (tempFile) {
            tempFile.remove()
            tempFile = null
        }
    }

    function catalog() {
        breakpoint()
        let file = File("catalog.json", "w")
        file.write("/*\n *  Catalog generated by \"jem\" on " + Date() + "\n */\n")
        for each (path in ls(dirs.jems, true)) {
            jem = Jem(path)
            let package = loadPackageDescription(path)
            file.write(serialize(package, {pretty: true}) + "\n")
        }
        file.close()
    }

    function create() {
        if (ls(".", true).length != 0 && !cmd.options.force) {
            throw "Create must be run in an empty directory"
        }
        let path = Path(".")
        if (filenames.package.exists) {
            throw "Package description \"" + filenames.package + "\" .already exists in this directory"
        }
        for each (d in requiredDirs) {
            makeDir(d)
        }
        Path(filenames.package).write(PackageTemplate)

        // Path("build.es").write("/* Sample build script */")
        // Path("install.es").write("/* Sample install script */")
        // Path("uninstall.es").write(/* Sample uninstall script */)
    }

    function dependAll() {
        for each (path in ls(dirs.jems, true)) {
            depend(Jem(path))
        }
    }

    /*
        Print jem dependencies. Jem is a bare jem name or a versioned jem name
     */
    function depend(jem: Jem): Void {
        if (!jem.version) {
            jem = latest(dirs.jems, jem)
        }
        if (!checkInstalled(jem)) {
            throw "Jem " + jem + " is not installed"
        }
        printDeps(jem, (logLevel > 0) ? "    " : "")
    }

    function edit(path: Path): Void {
        if (!cmd.options.field) {
            erorr("Missing --field option")
        }
        if (!cmd.options.value) {
            erorr("Missing --value option")
        }
        let package = loadPackageDescription(path)
        package[cmd.options.field] = cmd.options.value
        path.join(filenames.package).write(serialize(package, {pretty: true}))
    }

    private function fetchLocal(rep: Path, jem: Jem): Path {
        let matches = []
        for each (path in find(rep, jem.vername + "*", false)) {
            if (!path.isDir) {
                continue
            }
            let candidate = Jem(path)
            if (matchJem(candidate, [jem.vername])) {
                matches.append(candidate)
            }
        }
        if (matches.length == 0) {
            return null
        }
        matches.sort()
        return rep.join(matches.pop().toString())
    }

    /*
        Fetch a jem from a remote repository using http
        @options version Qualifying version string
     */
    private function fetchRemote(rep: String, jem: Jem): Path {
        let url: String = rep + "/" + jem.name + "?min=" + options.min + "&max=" + options.max
        let http = new Http
        log(2, "FETCH", "Try repository " + url)
        try {
            http.get(url)
            if (http.code != 200) {
                log(0, "WARN", "Can't retrieve " + url + " Code " + http.code)
                return null
            }
        } catch (e) {
print(e)
            log(0, "WARN", "Can't retrieve " + url + "\n" + e.message)
            return null
        }
        tempFile = Path(".").tempDir()
        tempFile.save(http.resposne)
        return tempFile
    }

    /*
        Install a jem
        @param jem Name of the jem. can be a bare-jem name, versioned jem-name or a path to a physical jem file
     */
    function install(jem: Jem) {
        if (checkInstalled(jem)) {
            if (!cmd.options.force) {
                throw "Jem \"" + jem + "\" is already installed"
            }
        } else {
            log(4, "DEBUG", jem + " is not yet installed")
        }
        if (!jem.local) {
            let path = search(jem)
            if (!path) {
                throw "Can't find jem \"" + jem + "\" to install"
            }
            jem = Jem(path)
        }
        installJemFile(jem)
    }

    function upgradeAll() {
        for each (path in ls(dirs.jems, true)) {
            upgrade(Jem(path))
        }
    }

    function upgrade(jem: Jem? = null) {
/*
        if (!checkInstalled(jem)) {
            throw "Jem \"" + jem + "\" is not installed"
        }
        if (!jem.local) {
            throw "Can't find jem \"" + jem + "\" to upgrade"
        }
        path = queryRemote(rep, jem)
        */
    }

    private function installDependencies(jem: Jem): Boolean {
        let package = jem.package
        for each (other in package.dependencies) {
            let dep = parseDep(other)
            if (!checkInstalled(dep)) {
                log(1, "NOTE", "Installing required dependency " + dep.name)
                try {
                    install(dep.name)
                } catch (e) {
print(e)
                    if (cmd.options.force) {
                        log(0, "WARN", "Can't install required dependency \"" + dep.name + "\"" )
                    } else {
                        throw "Can't install required dependency \"" + dep.name + "\"" 
                    }
                }
            } else {
                log(3, "DEBUG", "dependency \"" + dep.name + "\" is installed")
            }
        }
        return true
    }

/*
    function copyTree(from: Path, to: Path, pattern: String? = null) {
        from = from.relative
        makeDir(to.join(from.dirname))
        for each (f in find(from, pattern, true)) {
            cp(f, to.join(f))
        }
    }
*/

    function installModules(jem: Jem): Void {
        for each (f in find(jem.local.join(filenames.cache), "*" + ext.mod, true)) {
            let dest = dirs.modules.join(Path(f).basename)
            log(1, "INSTALL", dest)
            cp(f, dest)
        }
    }

    function installNativeModules(jem: Jem): Void {
        for each (f in find(jem.local.join(filenames.cache), "*" + ext.so, true)) {
            let dest = dirs.lib.join(Path(f).basename)
            log(1, "INSTALL", dest)
            cp(f, dest)
        }
    }

    function installDocumentation(dir): Void {
        // print("TODO")
        //  TODO
        // log(1, "INSTALL", dest)
    }

    /*
        Install a jem from a physical jem file
     */
    private function installJemFile(jem: Jem): Void {
        if (jem.vername == jem.name) {
            log(1, "INSTALL", "Jem mismatch: ", serialize(jem))
            throw "Jem package is missing version information"
        }
        let path = jem.local.join(jem.vername + ext.jem)
        let tar = new Tar(path)
        let data = tar.cat(filenames.package)
        let package = deserialize(data)
        validatePackage(package)

        let dir: Path = dirs.jems.join(jem.vername)
        log(2, "NOTE", "Installing jem \"" + jem + "\" from \"" + path + "\" to \"" + dir + "\"")
        if (dir.exists) {
            log(3, "RMDIR", dir)
            removeDir(dir, true)
        }
        log(3, "MKDIR", dir)
        let home = App.dir
        mkdir(dir)
        try {
            chdir(dir)
            log(3, "EXTRACT", "Extract to " + dir)
            tar.extract()
        } finally {
            chdir(home)
        }
        jem = Jem(dir)
        jem.package = package
        if (!validateJem(jem)) {
            return
        }
        installDependencies(jem)

        let script = jem.local.join("install.es")
        if (script.exists) {
            try {
                log(2, "DEBUG", "Run installation script: " + script)
                //  TODO - need to trap onerror
                //  TODO - need to set dirs.jems, inside the script env
                chdir(jem.local)
                Worker().load(script)
                Worker.join()
            } catch (e) {
                throw "Can't run installion script \"" + script + "\" for " + jem + "\n" + e
            } finally {
                chdir(home)
            }
        } else {
            buildJem(jem)
            if (cmd.options.test) {
                // runTests(jem)
            }
            installModules(jem)
            installNativeModules(jem)
            installDocumentation(jem)
        }
        //  TODO build source
        //  TODO run tests
        log(1, "NOTE", "Jem \"" + jem + "\" successfully installed")
    }

    /*
     *  Find the most recent version of a jem
     */
    private function latest(dir: Path, jem: Jem): Jem {
        let jems = find(dir, jem.name + "-*", false)
        jems += find(dir, jem.name, false)
        jems = jems.sort()
        if (jems.length > 0) {
            return Jem(jems[0])
        }
        return null
    }

    /*
        Show list of locally installed jems
            --all          # Independently list all versions of a module instead of just the most recent
            --versions     # Show versions appended to each jem
            --details      # List jem details
     */
    function list(args: Array): Void {
        let options = cmd.options
        if (options.help) {
            listHelp()
        }
        if (options.all) {
            options.versions = true
        }
        let sets = {}
        for each (path in ls(dirs.jems, true)) {
            jem = Jem(path)
            if (matchJem(jem, args)) {
                let index = (options.all) ? jem.vername : jem.name
                /* Aggregate the set of installed versions of each jem */
                let jemset = sets[index]
                jemset ||= []
                sets[index] = jemset
                jemset.append(jem)
            }
        }
        for each (jemset in sets) {
            if (options.versions) {
                versions = []
                for each (jem in jemset) {
                    versions.append(jem.version)
                }
                jem = jemset[0]
                out.write(jem.name + " [" + versions.join(", ") + "]")
            } else {
                jem = jemset[0]
                out.write(jem.name)
            }
            if (options.details) {
                let package = loadPackageDescription(jem.local)
                out.write(":\n")
                let deps = []
                for each (dep in package.dependencies) {
                    if (dep is String) {
                        deps.append(dep)
                    } else {
                        deps.append(dep[0])
                    }
                }
                print("    Description: ", package.description)
                print("    Author: ", package.author)
                print("    Stability: ", package.stability)
                print("    Location: ", package.location)
                print("    Dependencies: ", deps.join(", "))
            }
            print()
        }
    }

    private function loadPackageDescription(path: Path): Object 
        deserialize(path.join(filenames.package).readString())

    //  MOB -- should use logger
    function log(level: Number, tag: String, ...args): Void {
        if (level <= logLevel) {
            out.write("  [" + tag + "] " + args.join(" ") + "\n")
        }
    }


    /*
        Given a filename with an optional version component, return the version number.
     */
    function parseVersionedPath(filename: String): Number
        makeVersion(filename.split("-")[1])

    /*
        Make a version number from a version string (Maj.Min.Patch)
     */
    function makeVersion(version: String): Number {
        let parts = version.trim().split(".")
        let patch = 0, minor = 0
        let major = parts[0] cast Number
        if (parts.length > 1) {
            minor = parts[1] cast Number
        }
        if (parts.length > 2) {
            patch = parts[2] cast Number
        }
        return (((major * VER_FACTOR) + minor) * VER_FACTOR) + patch
    }

    function makeVersionString(version: Number): String {
        if (version == 0) {
            return ""
        }
        let patch = version % VER_FACTOR
        version /= VER_FACTOR
        let minor = version % VER_FACTOR
        version /= VER_FACTOR
        let major = version % VER_FACTOR
        version /= VER_FACTOR
        return "" + major + "." + minor + "." + patch
    }

    /*
        Match a jem name against user specified patterns
     */
    private function matchJem(jem: Jem, patterns: Array): Boolean {
        if (!patterns || patterns.length == 0) {
            return true
        }
        for each (pat in patterns) {
            //  TODO - what about version?
            if (jem.name.match(pat)) {
                return true
            }
        }
        return false
    }

    /*
        Parse a dependency from a package file and define the max/min version properties
        @return the options object with fields set: name, min, max
        Format: [ "name", "min", "max"]
     */
    private function parseDep(dep: Array): Jem {
        let max, min
        max = min = 0
        let name = dep.shift()
        for each (cond in dep) {
            cond = cond.toString()
            if (cond.contains("<=")) {
                max = makeVersion(cond.split("<=")[1])
            } else if (cond.contains("<")) {
                max = makeVersion(cond.split("<")[1]) -1
            } else if (cond.contains(">=")) {
                min = makeVersion(cond.split(">=")[1])
            } else if (cond.contains(">")) {
                min = makeVersion(cond.split(">")[1]) + 1
            } else if (cond.contains("==")) {
                max = min = makeVersion(cond.split("==")[1])
            } else {
                max = min = makeVersion(cond)
            }
        }
        if (max == 0) {
            max = MAX_VER
        }
        return new Jem(name, min, max)
    }

    /*
        Given a package description file, print the jem dependencies
     */
    private function printDeps(jem: Jem, prefix: String = "") {
        let package = loadPackageDescription(jem.local)
        for each (other in package.dependencies) {
            out.write(prefix)
            let dep = parseDep(other)
            if (dep.min == undefined) {
                print(name)
            } else if (dep.max == undefined || dep.min == dep.max) {
                print(dep.name + " [" + makeVersionString(dep.min) + "]")
            } else if (dep.max == MAX_VER) {
                print(dep.name + " [>= " + makeVersionString(dep.min) + "]")
            } else {
                let min = dep.min
                let max = dep.max + 1
                print(dep.name + " [>= " + makeVersionString(min) + ", <= " + makeVersionString(max) + "]")
            }
        }
    }

    function publish(jem: Jem): Void {
        let repo = Path(cmd.options.to)
        if (!repo) {
            repo = dirs.repositories[0]
        }
        if (repo.contains("http://")) {
            //  MOB TODO
            publishRemote()
            // MOB path = fetchRemote(rep, jem)
        } else {
            //  TODO - really need to update the catalog
            let from = jem.path
            let dir = repo.join(jem.vername)
            let target = dir.join(jem.path.basename)
            //  MOB -- must cleanup if cp or makedir fails
            makeDir(target.dirname)
            cp(from, target)
            tar = new Tar(target)
            let home = App.dir
            try {
                chdir(dir)
                tar.extract("package.json")
            } finally {
                chdir(home)
            }
            //  MOB publishLocal()
        }
    }

    /*
        Check if this jem is required by any others
        TODO OPT. Should get this from the index?
     */
    private function requiredJem(jem: Jem): Boolean {
        let version = makeVersion(jem.version)
        let consumers = []
        for each (path in ls(dirs.jems, true)) {
            let name = path.basename.toString()
            if (name.split("-")[0] != jem.name) {
                let package = loadPackageDescription(path)
                for each (other in package.dependencies) {
                    let dep = parseDep(other)
                    if (dep.name == jem.name && dep.min <= version && version <= dep.max) {
                        consumers.append(name)
                    }
                }
            }
        }
        if (consumers.length > 0) {
            throw "Can't remove \"" + jem + "\". It is required by: " + consumers.join(", ")
        }
        return false
    }

    function removeJem(jem: Jem): Void {
        if (!cmd.options.force && requiredJem(jem)) {
            return
        }
        log(1, "REMOVE", "Removing \"" + jem + "\"")
        let script = jem.local.join("uninstall.es")
        if (script.exists) {
            try {
                log(1, "RUN", "Run uninstall script: " + script)
                Worker().load(script)
                Worker.join()
            } catch (e) {
                throw "Can't uinstall \"" + jem + "\"\n" + e
            }
        }
        log(1, "RMDIR", "Removing jem contents at \"" + jem.local + "\"")
        removeDir(jem.local, true)
    }

    function rebuild(): Void {
        for each (path in ls(dirs.jems, true)) {
            buildJem(Jem(path))
        }
    }

    //  TODO - check jem usage
    function retract(jem: Jem): Void {
    }

    /*
        Search for a jem in local or remote jem repositories
        jem can be a path to a physical jem, a bare jem name or a versioned jem name.
     */
    private function search(jem: Jem): Path {
        let options = cmd.options
        log(3, "DEBUG", "Searching repositories: for " + jem)
        for each (rep in dirs.repositories) {
            log(4, "DEBUG", "Checking \"" + rep + "\" repository")
            let path
            if (rep.contains("http://")) {
                path = fetchRemote(rep, jem)
            } else {
                path = fetchLocal(rep, jem)
            }
            if (path) {
                return path
            }
        }
        return null
    }

    /*
        Set package dependencies based on module files. Uses exact versioning.
     */
    private function setdeps() {
        if (!filenames.package.exists) {
            throw "Missing " + filenames.package + ". Run jem in the directory containing the package description file"
        }
        moddeps = []
        for each (f in cmd.args) {
            moddeps += Cmd.sh("ejsmod --depends " + f).trim().split(" ")
        }
        let package = loadPackageDescription(".")
        deps = []
        for each (mod in moddeps) {
            let parts = mod.split("-")
            let name = parts[0]
            let min = parts[1]
            dep = [name, "== " + min]
            deps.append(dep)
        }
        package.dependencies = deps
        // TODO - need safe rewrite with a move and backup
        filenames.package.write(serialize(package, {pretty: true}))
    }

    function setLog(logSpec: String): Void {
        if (logSpec) {
            let parts = logSpec.split(":")
            logFile = parts[0]
            logLevel = parts[1]
            if (logFile == "stdout") {
                out = App.errorStream
            } else {
                out = Path(logFile).open("w")
            }
        }
    }

    /*
        Parse sub command options
    function subargs(switches): Object {
        options = {}
        let first = MAX_VER
        let last = 0
        let i = 0
        for each (arg in args) {
            if (arg.startsWith("-")) {
                for each (sw in switches) {
                    if (arg == ("--" + sw)) {
                        options[sw] = true
                    }
                }
                last = i
                first = first.min(i)
            }
            i++
        }
        if (first < args.length) {
            args.remove(first, last - first)
        }
        return options
    }
     */

    //  TODO - should this take a jem parameter?
    function validateJem(jem: Jem): Boolean {
        let requiredFiles = [ filenames.package ]
        for each (file in requiredFiles) {
            let path = jem.local.join(file)
            if (!exists(path) && !path.isDir) {
                throw "Jem is missing required files \"" + file + "\""
            }
        }
        let requiredDirs = [ "modules", "doc", "test" ]
        for each (file in requiredFiles) {
            let path = jem.local.join(file)
            if (!exists(path) && !path.isDir) {
                throw "Jem is missing required directory \"" + file + "\""
            }
        }
        return true
    }

    //  TODO - should validate field contents: non-empty, valid version, valid email
    function validatePackage(package: Object): Boolean {
        if (!package) {
            throw "Invalid package description file"
        }
        for each (field in requiredKeywords) {
            if (package[field] == undefined) {
                throw "Package does not validate. Missing or empty required field \"" + field + "\""
            }
        }
        return true
    }

    function uninstall(jem: Jem): Void {
        if (!checkInstalled(jem)) {
            if (!cmd.options.force) {
                throw "Jem \"" + jem + "\" is not installed"
            }
        }
        let jems = []
        if (jem.version) { 
            jems += jem
        } else {
            let files = dirs.jems.find(jem + "*", false)
            if (files.length == 0) {
                throw "Jem \"" + jem + "\" is not installed"
            } else {
                let count = 0
                for each (candidate in files) {
                    j = Jem(candidate)
                    if (matchJem(j, [ jem.name ])) {
                        jems.append(j)
                        count++
                    }
                }
                if (count > 1 && !cmd.options.force) {
                    throw "Multiple matching jems: " + jems
                }
            }
        }
        for each (jem in jems) {
            removeJem(jem)
        }
    }

    function loadConfig(path: Path, overwrite: Boolean = true, mandatory: Boolean = false): Boolean {
        if (!path.exists) {
            if (mandatory) {
                throw new IOError("Can't open required configuration file: " + path)
            } else {
                return false
            }
        }
        let settings = deserialize(path.readString())
        blend(config, settings, overwrite)
        return true
    }

    function loadDefaults(): Void {
        let homeEnv = App.getenv("HOME")
        if (homeEnv) {
            let home = Path(homeEnv)
            //  MOB -- should come from App.confg
            config = {}
            loadConfig(RC) || loadConfig(home.join(RC))
        } else {
            //  MOB -- should come from App.confg
            config = {}
        }
    }

    function makeDir(path: String): Void {
        if (isDir(path)) {
            return
        }
        mkdir(path, DIR_PERMS)
    }

    function removeDir(path: Path, contents: Boolean = false) {
        for each (d in sysdirs) {
            if (d == path) {
                throw "Internal error. Attempting to remove " + path
            }
        }
        rmdir(path, contents)
    }

    function error(msg) {
        App.log.error(msg)
    }

    /*
        Templates
     */
    private var PackageTemplate = {
      "name": "NAME",
      "version": "1.0.0",
      "description": "Package Description",
      "keywords": [
        "Put keywords here",
      ],
      "categories": [ "Put categories here" ],
      "author": "NAME (URL) <MAIL>",
      "contributors": [
        "NAME (URL) <MAIL>",
      ],
      "bugReports": "dev@embedthis.com",
      "license": [
        "GPLv2 (URL to license file)",
      ],
      "location": "http://hg.embedthis.com/ejs",
      "dependencies": [

        [ "ejs", "== 1.0.0" ],
      ],
      "stability": "evolving",
      "os": "*",
      "cpu": "*",
      "patchable": false,

/*
OS: []
CPU: []
MAINTAINERS: [{ name, email, web}]
CONTRIBUTORS: [{ name, email, web}]
LICENSES: [{ type, url }]
REPOSITORIES [{type, url, path }]
HOMEPAGE
ENGINE: []
DIRECTORIES: { lib: , src:, doc:, test:, bin:, }
IMPLEMENTS:  ["CommonJS/Modules/1.1", "CommonJS/JSGI/1.0"]
SCRIPTS: {
    install:
    uninstall
    build
    doc
    test
}
*/

    }
}

/*
    Hold the best available information available about a jem
 */
class Jem {
    use default namespace internal
    var name: String            //  Bare name without version information
    var vername: String         //  Versioned name if a version supplied, otherwise == name
    var local: Path             //  Path to local location in local repository (if known)
    var path: Path              //  Path to Jem outside a repository (if installing)
    var remote: String          //  Remote repository location MOB - does this include name
    var version: String         //  Actual version of the jem (if known)
    var min: String             //  Minimum acceptable version (if searching)
    var max: String             //  Maximum acceptable version (if searching)
    var package: Object         //  Package description file data

    /*
     *  Create a new jem object. Pathname may be a path, bare jem name or a versioned jem name
     */
    function Jem(pathname: String, min: Number = 0, max: Number = MAX_VER) {
        path = Path(pathname.trimEnd(ext.jem) + ext.jem)
        if (pathname.contains("http://")) {
            remote  = pathname
        } else if (exists(pathname) || pathname.contains("/")) {
            local = Path(pathname)
        } else if (dirs.jems.join(pathname).exists) {
            local = dirs.jems.join(pathname)
        } else {
            remote = pathname
        }
        let parts = basename(pathname).toString().split("-")
        name = parts[0]
        if (parts.length == 2) {
            version = parts[1]
            vername = name + "-" + version
        } else {
            vername = name
        }
        this.min = min
        this.max = max
    }

    public override function toString(): String {
        if (version) {
            return name + "-" + version 
        } else {
            return name
        }
    }
}


JemCmd().main()

} /* ejs.jem module */


/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
  
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
