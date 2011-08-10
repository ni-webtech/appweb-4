/**
    esp.c -- Embedded Server Pages (ESP) utility program

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.

  Commands:
    esp generate controller name [action [, action] ...]
        # generate a new controller with names
    esp run

    MOB - what about romming
 */

/********************************* Includes ***********************************/

#include    "appweb.h"
#include    "esp.h"

/********************************** Locals ************************************/
/*
    Global application object. Provides the top level roots of all data objects for the GC.
 */
typedef struct App {
    Mpr         *mpr;
    MaAppweb    *appweb;
    MaMeta      *meta;
    char        *serverRoot;
    char        *configFile;
    char        *pathEnv;
    char        *listen;

    char        *currentDir;            /* Initial starting current directory */
    char        *libDir;                /* Appweb lib directory */
    char        *wwwDir;                /* Appweb esp-www default files directory */
    char        *appName;               /* Application name */

    char        *flatPath;              /* Output filename for flat compilations */
    MprFile     *flatFile;              /* Output file for flat compilations */
    MprList     *flatItems;             /* Items to invoke from Init */

    /*
        GC retention
     */
    EspLoc      *loc;
    MprList     *files;                 /* List of files to process */
    char        *command;               /* Compilation or link command */
    char        *cacheName;             /* MD5 name of cached component */
    cchar       *csource;               /* Name of "C" source for controller or view */
    char        *module;                /* Compiled module name */
    char        *source;

    int         error;
    int         flat;
    int         overwrite;
    int         quiet;
} App;

static App      *app;
static Esp      *esp;
static EspLoc   *loc;
static Http     *http;
static MaAppweb *appweb;

#define ESP_CONTROLLER  0x1
#define ESP_VIEW        0x2
#define ESP_PAGE        0x4

/***************************** Forward Declarations ***************************/

static void clean(int argc, char **argv);
static void compile(int argc, char **argv);
static void copyDir(cchar *fromDir, cchar *toDir);
static void error(cchar *fmt, ...);
static void findConfigFile();
static void generate(int argc, char **argv);
static void generateAppDirs();
static void generateAppFiles();
static void generateAppConfigFile();
static void initialize();
static void makeDir(cchar *dir);
static void manageApp(App *app, int flags);
static void process(int argc, char **argv);
static void readConfig();
static void run(int argc, char **argv);
static void trace(cchar *tag, cchar *fmt, ...);
static void usageError();

#ifndef BLD_SERVER_ROOT
    #define BLD_SERVER_ROOT mprGetCurrentPath()
#endif
#ifndef BLD_CONFIG_FILE
    #define BLD_CONFIG_FILE NULL
#endif

/*********************************** Code *************************************/

MAIN(esp, int argc, char **argv)
{
    Mpr     *mpr;
    cchar   *argp;
    int     argind, rc;

    if ((mpr = mprCreate(argc, argv, 0)) == NULL) {
        exit(1);
    }
    if ((app = mprAllocObj(App, manageApp)) == NULL) {
        exit(2);
    }
    mprAddRoot(app);
    mprAddStandardSignals();
    
    argc = mpr->argc;
    argv = mpr->argv;
    app->mpr = mpr;
    app->configFile = BLD_CONFIG_FILE;
    app->listen = sclone(ESP_LISTEN);

    for (argind = 1; argind < argc; argind++) {
        argp = argv[argind];
        if (*argp != '-') {
            break;
        }
        if (scmp(argp, "--config") == 0) {
            if (argind >= argc) {
                usageError();
            } else {
                app->configFile = sclone(argv[++argind]);
            }

        } else if (scmp(argp, "--flat") == 0 || scmp(argp, "-f") == 0) {
            app->flat = 1;

        } else if (scmp(argp, "--listen") == 0 || scmp(argp, "-l") == 0) {
            if (argind >= argc) {
                usageError();
            } else {
                app->listen = sclone(argv[++argind]);
            }

        } else if (scmp(argp, "--log") == 0 || scmp(argp, "-l") == 0) {
            if (argind >= argc) {
                usageError();
            } else {
                maStartLogging(NULL, argv[++argind]);
                mprSetCmdlineLogging(1);
            }

        } else if (scmp(argp, "--overwrite") == 0) {
            app->overwrite = 1;

        } else if (scmp(argp, "--quiet") == 0 || scmp(argp, "-q") == 0) {
            app->quiet = 1;

        } else if (scmp(argp, "--verbose") == 0 || scmp(argp, "-v") == 0) {
            maStartLogging(NULL, "stderr:2");
            mprSetCmdlineLogging(1);

        } else if (scmp(argp, "--version") == 0 || scmp(argp, "-V") == 0) {
            mprPrintf("%s %s-%s\n", mprGetAppTitle(), BLD_VERSION, BLD_NUMBER);
            exit(0);

        } else {
            error("Unknown switch \"%s\"", argp);
            usageError();
        }
    }
    if (app->error) {
        return app->error;
    }
    if (mprStart() < 0) {
        mprUserError("Can't start MPR for %s", mprGetAppName());
        mprDestroy(MPR_EXIT_DEFAULT);
        return MPR_ERR_CANT_INITIALIZE;
    }
    initialize();
    if (!app->error) {
        process(argc - argind, &argv[argind]);
    }
    rc = app->error;
    mprLog(1, "Exit complete");
    mprDestroy(MPR_EXIT_DEFAULT);
    return rc;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->mpr);
        mprMark(app->appweb);
        mprMark(app->meta);
        mprMark(app->serverRoot);
        mprMark(app->configFile);
        mprMark(app->pathEnv);
        mprMark(app->listen);
        mprMark(app->currentDir);
        mprMark(app->libDir);
        mprMark(app->wwwDir);
        mprMark(app->appName);
        mprMark(app->command);
        mprMark(app->cacheName);
        mprMark(app->csource);
        mprMark(app->module);
        mprMark(app->source);
        mprMark(app->files);
        mprMark(app->loc);
        mprMark(app->flatItems);
        mprMark(app->flatFile);
        mprMark(app->flatPath);
    }
}


static void setLocDirs(cchar *path)
{
    if ((app->loc = mprAllocObj(EspLoc, espManageEspLoc)) == 0) {
        return;
    }
    loc = app->loc;
    loc->dir = sclone(path);
    loc->cacheDir = mprJoinPath(loc->dir, "cache");
    loc->controllersDir = mprJoinPath(loc->dir, "controllers");
    loc->layoutsDir = mprJoinPath(loc->dir, "layouts");
    loc->staticDir = mprJoinPath(loc->dir, "static");
    loc->viewsDir = mprJoinPath(loc->dir, "views");
}


static void initialize()
{
    app->currentDir = mprGetCurrentPath();
    app->appName = mprGetPathBase(app->currentDir);
    app->libDir = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
    app->wwwDir = mprJoinPath(app->libDir, "esp-www");

    setLocDirs(app->currentDir);

#if BLD_UNIX_LIKE
    app->pathEnv = sjoin("PATH=", getenv("PATH"), ":", mprGetAppDir(), NULL);
    putenv(app->pathEnv);
#endif
}


static void getAppDirs()
{
    HttpHost    *host;
    HttpLoc     *hp;
    char        *prefix;

    //  MOB - must support non-default hosts
    host = app->meta->defaultHost;
    //  MOB - must support more complex prefixes
    //  MOB - but the /app is not used if doing a stand-alone appweb.conf
    prefix = sfmt("/%s", app->appName);
    if ((hp = httpLookupBestLocation(host, prefix)) == 0) {
        error("Can't find EspAlias or Location block for %s", prefix);
    }
    if ((app->loc = httpGetLocationData(hp, ESP_NAME)) == 0) {
        error("Can't find ESP data for Location block for %s", prefix);
    }
    loc = app->loc;
    app->appName = mprGetPathBase(loc->dir);
}


static void readConfig()
{
    HttpStage   *stage;

    if ((app->appweb = maCreateAppweb()) == 0) {
        error("Can't create HTTP service for %s", mprGetAppName());
        return;
    }
    appweb = app->appweb;
    http = app->appweb->http;

    findConfigFile();

    if ((app->meta = maCreateMeta(appweb, "default", app->serverRoot, NULL, -1)) == 0) {
        error("Can't create HTTP server for %s", mprGetAppName());
        return;
    }
    if (maParseConfig(app->meta, app->configFile) < 0) {
        error("Can't configure the server, exiting.");
        return;
    }
    if ((stage = httpLookupStage(http, "espHandler")) == 0) {
        error("Can't find ESP handler in %s", app->configFile);
        return;
    }
    esp = stage->stageData;
    getAppDirs();
}


static void process(int argc, char **argv)
{
    cchar   *cmd;

    cmd = argv[0];
    if (scmp(cmd, "generate") == 0) {
        generate(argc - 1, &argv[1]);
        return;
    }
    if (app->error) {
        return;
    }
    if (scmp(cmd, "clean") == 0) {
        readConfig();
        clean(argc - 1, &argv[1]);

    } else if (scmp(cmd, "compile") == 0) {
        readConfig();
        compile(argc - 1, &argv[1]);

    } else if (scmp(cmd, "run") == 0) {
        run(argc - 1, &argv[1]);

    } else {
        if (cmd && *cmd) {
            error("Unknown command %s", cmd);
        } else {
            usageError();
        }
    }
}


static void clean(int argc, char **argv)
{
    MprList         *files;
    MprDirEntry     *dp;
    char            *path;
    int             next;

    files = mprGetPathFiles(loc->cacheDir, 0);
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0; ) {
        path = mprJoinPath(loc->cacheDir, dp->name);
        trace("CLEAN", "%s", path);
        mprDeletePath(path);
    }
    trace("TASK", "Complete", path);
}


static void run(int argc, char **argv)
{
    MprCmd      *cmd;
    char        *err, *out;

    cmd = mprCreateCmd(0);
    if (mprRunCmd(cmd, "appweb -v", NULL, NULL, MPR_CMD_DETACH) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
        error("Can't run command %s, error %s", app->command, err);
    }
    mprWaitForCmd(cmd, -1);
}

static int runCommand(cchar *command, cchar *csource, cchar *module)
{
    MprCmd      *cmd;
    char        *err, *out;

    cmd = mprCreateCmd(0);
    if ((app->command = espExpandCommand(command, csource, module)) == 0) {
        error("Missing EspCompile directive for %s", csource);
        return MPR_ERR_CANT_READ;
    }
    mprLog(4, "ESP command: %s\n", app->command);
    if (loc->env) {
        mprAddNullItem(loc->env);
        mprSetCmdDefaultEnv(cmd, (cchar**) &loc->env->items[0]);
    }
    if (loc->searchPath) {
        mprSetCmdSearchPath(cmd, loc->searchPath);
    }
    //  WARNING: GC will run here
    mprLog(1, "Run: %s", app->command);
	if (mprRunCmd(cmd, app->command, &out, &err, 0) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
        error("Can't run command %s, error %s", app->command, err);
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}


static void compileFile(cchar *source, int kind)
{
    cchar   *script, *page, *layout, *data;
    char    *err;
    ssize   len;

    layout = 0;
    app->cacheName = mprGetMD5Hash(source, slen(source), (kind == ESP_CONTROLLER) ? "controller_" : "view_");
    app->module = mprGetNormalizedPath(sfmt("%s/%s%s", loc->cacheDir, app->cacheName, BLD_SHOBJ));

    if (kind == ESP_CONTROLLER) {
        app->csource = source;
        if (app->flatFile) {
            if ((data = mprReadPath(source)) == 0) {
                error("Can't read %s", source);
                return;
            }
            if (mprWriteFile(app->flatFile, data, slen(data)) < 0) {
                error("Can't write compiled script file %s", app->flatFile->path);
                return;
            }
            mprWriteFileFormat(app->flatFile, "\n\n");
            mprAddItem(app->flatItems, sfmt("espInit_controller_%s", mprTrimPathExtension(mprGetPathBase(source))));
        }
    }
    if (kind & (ESP_PAGE | ESP_VIEW)) {
        layout = mprJoinPath(mprJoinPath(loc->dir, loc->layoutsDir), "default.esp");
        if ((page = mprReadPath(source)) == 0) {
            error("Can't read %s", source);
            return;
        }
        /* No yield here */
        if ((script = espBuildScript(loc, page, source, app->cacheName, layout, &err)) == 0) {
            error("Can't build %s, error %s", source, err);
            return;
        }
        len = slen(script);
        if (app->flatFile) {
            if (mprWriteFile(app->flatFile, script, len) < 0) {
                error("Can't write compiled script file %s", app->flatFile->path);
                return;
            }
            mprWriteFileFormat(app->flatFile, "\n\n");
            mprAddItem(app->flatItems, sfmt("espInit_%s", app->cacheName));

        } else {
            trace("BUILD", "%s", source);
            app->csource = mprJoinPathExt(mprTrimPathExtension(app->module), ".c");
            if (mprWritePath(app->csource, script, len, 0664) < 0) {
                error("Can't write compiled script file %s", app->csource);
                return;
            }
        }
    }
    if (! app->flatFile) {
        /*
            WARNING: GC yield here
         */
        trace("COMPILE", "%s", app->csource);
        if (!loc->compile) {
            error("Missing EspCompile directive for %s", app->csource);
            return;
        }
        if (runCommand(loc->compile, app->csource, app->module) < 0) {
            return;
        }
        if (loc->link) {
            trace("LINK", "%s", app->module);
            if (runCommand(loc->link, app->csource, app->module) < 0) {
                return;
            }
#if !(BLD_DEBUG && MACOSX)
            /*
                MAC needs the object for debug information
             */
            mprDeletePath(mprJoinPathExt(mprTrimPathExtension(app->module), BLD_OBJ));
#endif
        }
        if (!loc->keepSource && (kind & (ESP_VIEW | ESP_PAGE))) {
            mprDeletePath(app->csource);
        }
    }
}


/*
    esp compile [flat | app | controller_names | view_names]
        ## [] - compile controllers and views separately into cache
        ## [controller_names/view_names] - compile single file
        ## [app] - compile all files into a single source file with one init that calls all sub-init files
    esp compile path/name.ejs ...
        ## [controller_names/view_names] - compile single file
    esp compile static
        ## use makerom code and compile static into a file in cache
 */
static void compile(int argc, char **argv)
{
    MprList     *files;
    MprDirEntry *dp;
    char        *path, *kind, *line;
    int         i, next;

    if (argc == 0) {
        if (app->flat) {
            kind = "flat";
        } else {
            kind = "*";
        }
    } else {
        kind = argv[0];
        if (scmp(kind, "flat") == 0) {
            app->flat = 1;
        }
    }
    if (scmp(kind, "*") == 0) {
        /*
            Build all items separately
         */
        app->files = files = mprGetPathFiles(loc->controllersDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (scmp(mprGetPathExtension(dp->name), "c") == 0) {
                path = mprJoinPath(loc->controllersDir, dp->name);
                compileFile(path, ESP_CONTROLLER);
            }
        }
        app->files = files = mprGetPathFiles(loc->viewsDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            path = mprJoinPath(loc->viewsDir, dp->name);
            compileFile(path, ESP_VIEW);
        }

        //  MOB - Is the name right?
        //  MOB - what about pattern matching?
        app->files = files = mprFindFiles(loc->staticDir, 0);
        for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (scmp(mprGetPathExtension(path), "esp") == 0) {
                compileFile(path, ESP_PAGE);
            }
        }

    } else if (scmp(kind, "flat") == 0) {
        /*
            Catenate all source
         */
        app->flatItems = mprCreateList(-1, 0);
        app->flatPath = path = mprJoinPath(loc->cacheDir, "app.c");
        if ((app->flatFile = mprOpenFile(path, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
            error("Can't open %s", path);
            return;
        }
        mprWriteFileFormat(app->flatFile, "/*\n *  Flat compilation of %s\n */\n\n", app->appName);

        app->files = files = mprGetPathFiles(loc->controllersDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (scmp(mprGetPathExtension(dp->name), "c") == 0) {
                path = mprJoinPath(loc->controllersDir, dp->name);
                compileFile(path, ESP_CONTROLLER);
            }
        }
        app->files = files = mprGetPathFiles(loc->viewsDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            path = mprJoinPath(loc->viewsDir, dp->name);
            compileFile(path, ESP_VIEW);
        }
        //  MOB - Is the name right?
        //  MOB - what about pattern matching?
        app->files = files = mprFindFiles(loc->staticDir, 0);
        for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (scmp(mprGetPathExtension(path), "esp") == 0) {
                compileFile(path, ESP_PAGE);
            }
        }
        mprWriteFileFormat(app->flatFile, "\nESP_EXPORT int espInit_app_%s(EspLoc *el, MprModule *module) {\n", 
            app->appName);
        for (next = 0; (line = mprGetNextItem(app->flatItems, &next)) != 0; ) {
            mprWriteFileFormat(app->flatFile, "    %s(el, module);\n", line);
        }
        mprWriteFileFormat(app->flatFile, "    return 0;\n}\n");
        mprCloseFile(app->flatFile);

        app->module = mprGetNormalizedPath(sfmt("%s/app%s", loc->cacheDir, BLD_SHOBJ));
        trace("COMPILE", "%s", app->csource);
        if (runCommand(loc->compile, app->flatPath, app->module) < 0) {
            return;
        }
        if (loc->link) {
            trace("LINK", "%s", app->module);
            if (runCommand(loc->link, app->flatPath, app->module) < 0) {
                return;
            }
        }
        app->flatItems = 0;
        app->flatFile = 0;
        app->flatPath = 0;

    } else {
        for (i = 0; i < argc; i++) {
            if (scmp(mprGetPathExtension(argv[i]), "esp") != 0) {
                error("Command arguments must be files with a \".esp\" extension");
                return;
            }
            path = mprJoinPath(loc->dir, argv[i]);
            compileFile(path, ESP_PAGE);
        }
    }
    if (!app->error) {
        trace("TASK", "Complete");
    }
}


static void generate(int argc, char **argv)
{
    char    *kind;

    /*
        generate app path
     */
    if (argc < 2) {
        usageError();
        return;
    }
    kind = argv[0];

    if (scmp(kind, "app") == 0) {
        setLocDirs(argv[1]);
        generateAppDirs();
        generateAppFiles();
        generateAppConfigFile();
        if (!app->error) {
            trace("TASK", "Complete");
        }

    } else {
        mprError("Unknown generation kind %s", kind);
        usageError();
    }
}


static void generateAppDirs()
{
    makeDir("");
    makeDir("cache");
    makeDir("controllers");
    makeDir("layouts");
    makeDir("static");
    makeDir("static/images");
    makeDir("static/js");
    makeDir("static/themes");
    makeDir("views");
}


static void fixupFile(cchar *path)
{
    char    *data, *tmp;

    path = mprJoinPath(loc->dir, path);
    if ((data = mprReadPath(path)) == 0) {
        error("Can't read %s", path);
        return;
    }
    data = sreplace(data, "${NAME}", app->appName);
    data = sreplace(data, "${DIR}", loc->dir);
    data = sreplace(data, "${LISTEN}", app->listen);
    data = sreplace(data, "${LIBDIR}", app->libDir);

    tmp = mprGetTempPath(0);
    if (mprWritePath(tmp, data, slen(data), 0644) < 0) {
        error("Can't write %s", path);
        return;
    }
    if (rename(tmp, path) < 0) {
        error("Can't rename %s to %s", tmp, path);
    }
}


static void generateAppFiles()
{
    copyDir(mprJoinPath(app->wwwDir, "files"), loc->dir);
    fixupFile("layouts/default.esp");
}


static void copyDir(cchar *fromDir, cchar *toDir)
{
    MprList     *files;
    MprDirEntry *dp;
    char        *from, *to;
    int         next;

    files = mprGetPathFiles(fromDir, 1);
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
        from = mprJoinPath(fromDir, dp->name);
        to = mprJoinPath(toDir, dp->name);
        if (dp->isDir) {
            if (!mprPathExists(to, R_OK)) {
                if (mprMakeDir(to, 0755, 1) < 0) {
                    error("Can't make directory %s", to);
                    return;
                }
                trace("CREATE",  "Directory: %s", to);
            }
            copyDir(from, to);
        
        } else {
            if (mprMakeDir(mprGetPathDir(to), 0755, 1) < 0) {
                error("Can't make directory %s", mprGetPathDir(to));
                return;
            }
            if (mprCopyPath(from, to, 0644) < 0) {
                error("Can't copy file %s to %s", from, to);
                return;
            }
            if (mprPathExists(to, R_OK) && !app->overwrite) {
                trace("EXISTS",  "File: %s", to);
            } else {
                trace("CREATED",  "Static web file: %s", to);
            }
        }
    }
}


#if UNUSED
static void installAppConf()
{
    char    *from, *to, *conf;

    if (0) {
        from = mprJoinPath(app->wwwDir, "app.conf");
    } else {
        from = mprJoinPath(app->wwwDir, "appweb.conf");
    }
    if ((conf = mprReadPath(from)) == 0) {
        error("Can't read %s", from);
        return;
    }
    to = sfmt("%s/conf/%s.conf", app->serverRoot, mprGetPathBase(app->appName));
    if (mprPathExists(to, R_OK) && !app->overwrite) {
        trace("EXISTS",  "Config file: %s", to);
        return;

    } else if (mprWritePath(to, conf, slen(conf), 0644) < 0) {
        error("Can't write %s", to);
        return;
    }
    fixupFile(to);
    trace("CREATED",  "Config file: %s", to);
}
#endif


static void generateAppConfigFile()
{
    char    *from, *to, *conf;

    from = mprJoinPath(app->wwwDir, "appweb.conf");
    if ((conf = mprReadPath(from)) == 0) {
        error("Can't read %s", from);
        return;
    }
    to = mprJoinPath(loc->dir, "appweb.conf");
    if (mprPathExists(to, R_OK) && !app->overwrite) {
        trace("EXISTS",  "Config file: %s", to);
        return;

    } else if (mprWritePath(to, conf, slen(conf), 0644) < 0) {
        error("Can't write %s", to);
        return;
    }
    fixupFile("appweb.conf");
    trace("CREATED",  "File: %s", to);
}


static void findConfigFile()
{
    cchar   *userPath;

    userPath = app->configFile;
    if (app->configFile == 0) {
        app->configFile = mprJoinPathExt(BLD_PRODUCT, ".conf");
    }
    if (!mprPathExists(app->configFile, R_OK)) {
        if (userPath) {
            error("Can't open config file %s", app->configFile);
            return;
        }
        app->configFile = mprJoinPath(mprGetAppDir(), sfmt("../%s/%s.conf", BLD_LIB_NAME, BLD_PRODUCT));
        if (!mprPathExists(app->configFile, R_OK)) {
            error("Can't open config file %s", app->configFile);
            return;
        }
        //  MOB -- should search up for config.
    }
    app->serverRoot = mprGetAbsPath(mprGetPathDir(app->configFile));
}


static void makeDir(cchar *dir)
{
    char    *path;

    path = mprJoinPath(loc->dir, dir);
    if (mprPathExists(path, X_OK)) {
        trace("EXISTS",  "Directory: %s", path);
    } else {
        if (mprMakeDir(path, 0755, 1) < 0) {
            app->error++;
        } else {
            trace("CREATE",  "Directory: %s", path);
        }
    }
}


static void usageError(Mpr *mpr)
{
    cchar   *name;

    name = mprGetAppName();

    mprPrintfError("\nESP Usage:\n\n"
    "  %s [options] [commands]\n\n"
    "  Options:\n"
    "    --config configFile    # Use named config file instead appweb.conf\n"
    "    --flat                 # Compile into a single module\n"
    "    --listen [ip:]port     # Listen on specified address \n"
    "    --log logFile:level    # Log to file file at verbosity level\n"
    "    --overwrite            # Overwrite existing files \n"
    "    --quiet                # Don't emit trace \n"
    "    --static               # Compile static content into C code\n"
    "    --verbose              # Emit verbose trace \n"
    "\n"
    "  Commands:\n"
    "    esp clean              # Clean cached files\n"
    "    esp compile            # Compile all controllers, views and pages\n"
    "    esp compile path/*.esp # Compile a single web page\n"
    "    esp run                # Run appweb and the ESP app\n"
    "", name);
    app->error++;
}


static void error(cchar *fmt, ...)
{
    va_list     args;
    char        *msg;

    va_start(args, fmt);
    msg = mprAsprintfv(fmt, args);
    mprError("%s", msg);
    va_end(args);
    app->error++;
}


static void trace(cchar *tag, cchar *fmt, ...)
{
    va_list     args;
    char        *msg;

    if (!app->quiet) {
        va_start(args, fmt);
        msg = mprAsprintfv(fmt, args);
        tag = sfmt("[%s]", tag);
        mprRawLog(0, "%12s %s\n", tag, msg);
        va_end(args);
    }
}


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
