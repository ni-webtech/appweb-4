/**
    esp.c -- Embedded Server Pages (ESP) utility program

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.

    MOB - what about romming
 */

/********************************* Includes ***********************************/

#include    "esp.h"

#if BLD_FEATURE_ESP
/********************************** Locals ************************************/
/*
    Global application object. Provides the top level roots of all data objects for the GC.
 */
typedef struct App {
    Mpr         *mpr;
    MaAppweb    *appweb;
    MaServer    *server;
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
    EspRoute    *eroute;
    MprList     *files;                 /* List of files to process */
    char        *routeName;             /* Name of route to use for ESP configuration */
    char        *routePrefix;           /* Route prefix to use for ESP configuration */
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

static App       *app;
static Esp       *esp;
static HttpRoute *route;
static EspRoute  *eroute;
static Http      *http;
static MaAppweb  *appweb;

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

MAIN(espgen, int argc, char **argv)
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
        if (smatch(argp, "--config")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->configFile = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "--flat") || smatch(argp, "-f")) {
            app->flat = 1;

        } else if (smatch(argp, "--listen") || smatch(argp, "-l")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->listen = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "--log") || smatch(argp, "-l")) {
            if (argind >= argc) {
                usageError();
            } else {
                maStartLogging(NULL, argv[++argind]);
                mprSetCmdlineLogging(1);
            }

        } else if (smatch(argp, "--overwrite")) {
            app->overwrite = 1;

        } else if (smatch(argp, "--quiet") || smatch(argp, "-q")) {
            app->quiet = 1;

        } else if (smatch(argp, "--routeName")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->routeName = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "--routePrefix")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->routePrefix = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            maStartLogging(NULL, "stderr:2");
            mprSetCmdlineLogging(1);

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
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
        mprMark(app->appName);
        mprMark(app->appweb);
        mprMark(app->cacheName);
        mprMark(app->command);
        mprMark(app->configFile);
        mprMark(app->csource);
        mprMark(app->currentDir);
        mprMark(app->eroute);
        mprMark(app->files);
        mprMark(app->flatFile);
        mprMark(app->flatItems);
        mprMark(app->flatPath);
        mprMark(app->libDir);
        mprMark(app->listen);
        mprMark(app->module);
        mprMark(app->mpr);
        mprMark(app->pathEnv);
        mprMark(app->routeName);
        mprMark(app->routePrefix);
        mprMark(app->server);
        mprMark(app->serverRoot);
        mprMark(app->source);
        mprMark(app->wwwDir);
    }
}


static void setDirs(cchar *path)
{
    if ((app->eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return;
    }
    eroute = app->eroute;
    eroute->dir = sclone(path);
    eroute->cacheDir = mprJoinPath(eroute->dir, "cache");
    eroute->controllersDir = mprJoinPath(eroute->dir, "controllers");
    eroute->layoutsDir = mprJoinPath(eroute->dir, "layouts");
    eroute->staticDir = mprJoinPath(eroute->dir, "static");
    eroute->viewsDir = mprJoinPath(eroute->dir, "views");
}


static void initialize()
{
    app->currentDir = mprGetCurrentPath();
    app->appName = mprGetPathBase(app->currentDir);
    app->libDir = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
    app->wwwDir = mprJoinPath(app->libDir, "esp-www");

    setDirs(app->currentDir);

#if BLD_UNIX_LIKE
    app->pathEnv = sjoin("PATH=", getenv("PATH"), ":", mprGetAppDir(), NULL);
    putenv(app->pathEnv);
#endif
}


static void getAppDirs()
{
    HttpHost    *host;
    HttpRoute   *rp;
    char        *routeName, *routePrefix;
    int         next;

    if ((host = mprGetFirstItem(http->hosts)) == 0) {
        error("Can't find default host");
        return;
    }
    routeName = app->routeName;
    routePrefix = app->routePrefix ? app->routePrefix : "/";

    for (next = 0; (rp = mprGetNextItem(host->routes, &next)) != 0; ) {
        if (routeName) {
            if (strcmp(routeName, rp->name) != 0) {
                continue;
            }
        }
        if (strcmp(routePrefix, rp->literalPattern) != 0) {
            continue;
        }
        if ((app->eroute = httpGetRouteData(rp, ESP_NAME)) != 0 && app->eroute->compile) {
            break;
        }
    }
    if (rp == 0) {
        error("Can't find ESP configuration in config file");
        return;
    }
    route = rp;
    eroute = app->eroute;
    eroute->dir = route->dir;
    app->appName = mprGetPathBase(eroute->dir);
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

    if ((app->server = maCreateServer(appweb, "default")) == 0) {
        error("Can't create HTTP server for %s", mprGetAppName());
        return;
    }
    if (maParseConfig(app->server, app->configFile) < 0) {
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
    if (smatch(cmd, "generate")) {
        generate(argc - 1, &argv[1]);
        return;
    }
    if (app->error) {
        return;
    }
    if (smatch(cmd, "clean")) {
        readConfig();
        clean(argc - 1, &argv[1]);

    } else if (smatch(cmd, "compile")) {
        readConfig();
        compile(argc - 1, &argv[1]);

    } else if (smatch(cmd, "run")) {
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

    files = mprGetPathFiles(eroute->cacheDir, 0);
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0; ) {
        path = mprJoinPath(eroute->cacheDir, dp->name);
        trace("CLEAN", "%s", path);
        mprDeletePath(path);
    }
    trace("TASK", "Complete");
}


static void run(int argc, char **argv)
{
    MprCmd      *cmd;

    cmd = mprCreateCmd(0);
    if (mprRunCmd(cmd, "appweb -v", NULL, NULL, MPR_CMD_DETACH) != 0) {
        error("Can't run command %s", app->command);
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
    if (eroute->env) {
        mprAddNullItem(eroute->env);
        mprSetCmdDefaultEnv(cmd, (cchar**) &eroute->env->items[0]);
    }
    if (eroute->searchPath) {
        mprSetCmdSearchPath(cmd, eroute->searchPath);
    }
    mprLog(1, "Run: %s", app->command);

    //  WARNING: GC will run here
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
    app->cacheName = mprGetMD5WithPrefix(source, slen(source), (kind == ESP_CONTROLLER) ? "controller_" : "view_");
    app->module = mprGetNormalizedPath(sfmt("%s/%s%s", eroute->cacheDir, app->cacheName, BLD_SHOBJ));

    if (kind == ESP_CONTROLLER) {
        app->csource = source;
        if (app->flatFile) {
            if ((data = mprReadPath(source, &len)) == 0) {
                error("Can't read %s", source);
                return;
            }
            if (mprWriteFile(app->flatFile, data, slen(data)) < 0) {
                error("Can't write compiled script file %s", app->flatFile->path);
                return;
            }
            mprWriteFileFormat(app->flatFile, "\n\n");
            mprAddItem(app->flatItems, sfmt("espInit_controller_%s", mprTrimPathExt(mprGetPathBase(source))));
        }
    }
    if (kind & (ESP_PAGE | ESP_VIEW)) {
        layout = mprJoinPath(mprJoinPath(eroute->dir, eroute->layoutsDir), "default.esp");
        if ((page = mprReadPath(source, &len)) == 0) {
            error("Can't read %s", source);
            return;
        }
        /* No yield here */
        if ((script = espBuildScript(eroute, page, source, app->cacheName, layout, &err)) == 0) {
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
            app->csource = mprJoinPathExt(mprTrimPathExt(app->module), ".c");
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
        if (!eroute->compile) {
            error("Missing EspCompile directive for %s", app->csource);
            return;
        }
        if (runCommand(eroute->compile, app->csource, app->module) < 0) {
            return;
        }
        if (eroute->link) {
            trace("LINK", "%s", app->module);
            if (runCommand(eroute->link, app->csource, app->module) < 0) {
                return;
            }
#if !(BLD_DEBUG && MACOSX)
            /*
                MAC needs the object for debug information
             */
            mprDeletePath(mprJoinPathExt(mprTrimPathExt(app->module), BLD_OBJ));
#endif
        }
        if (!eroute->keepSource && (kind & (ESP_VIEW | ESP_PAGE))) {
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
        if (smatch(kind, "flat")) {
            app->flat = 1;
        }
    }
    if (smatch(kind, "*")) {
        /*
            Build all items separately
         */
        app->files = files = mprGetPathFiles(eroute->controllersDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (smatch(mprGetPathExt(dp->name), "c")) {
                path = mprJoinPath(eroute->controllersDir, dp->name);
                compileFile(path, ESP_CONTROLLER);
            }
        }
        app->files = files = mprGetPathFiles(eroute->viewsDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            path = mprJoinPath(eroute->viewsDir, dp->name);
            compileFile(path, ESP_VIEW);
        }

        //  MOB - Is the name right?
        //  MOB - what about pattern matching?
        app->files = files = mprFindFiles(eroute->staticDir, 0);
        for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(path, ESP_PAGE);
            }
        }

    } else if (smatch(kind, "flat")) {
        /*
            Catenate all source
         */
        app->flatItems = mprCreateList(-1, 0);
        app->flatPath = path = mprJoinPath(eroute->cacheDir, "app.c");
        if ((app->flatFile = mprOpenFile(path, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
            error("Can't open %s", path);
            return;
        }
        mprWriteFileFormat(app->flatFile, "/*\n *  Flat compilation of %s\n */\n\n", app->appName);

        app->files = files = mprGetPathFiles(eroute->controllersDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (smatch(mprGetPathExt(dp->name), "c")) {
                path = mprJoinPath(eroute->controllersDir, dp->name);
                compileFile(path, ESP_CONTROLLER);
            }
        }
        app->files = files = mprGetPathFiles(eroute->viewsDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            path = mprJoinPath(eroute->viewsDir, dp->name);
            compileFile(path, ESP_VIEW);
        }
        //  MOB - Is the name right?
        //  MOB - what about pattern matching?
        app->files = files = mprFindFiles(eroute->staticDir, 0);
        for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(path, ESP_PAGE);
            }
        }
        mprWriteFileFormat(app->flatFile, "\nESP_EXPORT int espInit_app_%s(EspRoute *el, MprModule *module) {\n", 
            app->appName);
        for (next = 0; (line = mprGetNextItem(app->flatItems, &next)) != 0; ) {
            mprWriteFileFormat(app->flatFile, "    %s(el, module);\n", line);
        }
        mprWriteFileFormat(app->flatFile, "    return 0;\n}\n");
        mprCloseFile(app->flatFile);

        app->module = mprGetNormalizedPath(sfmt("%s/app%s", eroute->cacheDir, BLD_SHOBJ));
        trace("COMPILE", "%s", app->csource);
        if (runCommand(eroute->compile, app->flatPath, app->module) < 0) {
            return;
        }
        if (eroute->link) {
            trace("LINK", "%s", app->module);
            if (runCommand(eroute->link, app->flatPath, app->module) < 0) {
                return;
            }
        }
        app->flatItems = 0;
        app->flatFile = 0;
        app->flatPath = 0;

    } else {
        for (i = 0; i < argc; i++) {
            if (scmp(mprGetPathExt(argv[i]), "esp") != 0) {
                error("Command arguments must be files with a \".esp\" extension");
                return;
            }
            path = mprJoinPath(eroute->dir, argv[i]);
            compileFile(path, ESP_PAGE);
        }
    }
    if (!app->error) {
        trace("TASK", "Complete");
    }
}


/*
    generate app path
 */
static void generate(int argc, char **argv)
{
    char    *kind;

    if (argc < 2) {
        usageError();
        return;
    }
    kind = argv[0];

    if (smatch(kind, "app")) {
        setDirs(argv[1]);
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
    ssize   len;
    char    *data, *tmp;

    path = mprJoinPath(eroute->dir, path);
    if ((data = mprReadPath(path, &len)) == 0) {
        error("Can't read %s", path);
        return;
    }
    data = sreplace(data, "${NAME}", app->appName);
    data = sreplace(data, "${DIR}", eroute->dir);
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
    copyDir(mprJoinPath(app->wwwDir, "files"), eroute->dir);
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
    ssize   len;

    from = mprJoinPath(app->wwwDir, "appweb.conf");
    if ((conf = mprReadPath(from, &len)) == 0) {
        error("Can't read %s", from);
        return;
    }
    to = mprJoinPath(eroute->dir, "appweb.conf");
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

    path = mprJoinPath(eroute->dir, dir);
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
    "    --routeName name       # Route name in appweb.conf to use \n"
    "    --routePrefix prefix   # Route prefix in appweb.conf to use \n"
    "    --static               # Compile static content into C code\n"
    "    --verbose              # Emit verbose trace \n"
    "\n"
    "  Commands:\n"
    "    esp clean              # Clean cached files\n"
    "    esp compile            # Compile all controllers, views and pages\n"
    "    esp compile path/*.esp # Compile a single web page\n"
    "    esp generate app name  # Generate a new application\n"
    "    esp run                # Run appweb and the ESP app\n"
    "", name);
    app->error++;
}


static void error(cchar *fmt, ...)
{
    va_list     args;
    char        *msg;

    va_start(args, fmt);
    msg = sfmt(fmt, args);
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
        msg = sfmtv(fmt, args);
        tag = sfmt("[%s]", tag);
        mprRawLog(0, "%12s %s\n", tag, msg);
        va_end(args);
    }
}

#endif /* BLD_FEATURE_ESP */

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
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
