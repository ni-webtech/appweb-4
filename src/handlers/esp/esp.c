/**
    esp.c -- Embedded Server Pages (ESP) utility program

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.

    usage: %s [options] [commands]
            --config configFile     # Use given config file instead 
            --debugger              # Disable timeouts to make debugging easier
            --ejs name:path         # Create an ejs application at the path
            --home path             # Set the home working directory
            --log logFile:level     # Log to file file at verbosity level
            --threads maxThreads    # Set maximum worker threads
            --version               # Output version information
            -v                      # Same as --log stderr:2

Usage: mvc [options] [commands] ...
  Options:
    --apply                      # Apply migrations
    --database [sqlite | mysql]  # Sqlite only currently supported adapter
    --full
    --keep
    --layout layoutPage
    --listen port
    --min
    --overwrite
    --quiet
    --verbose

  Commands:
    mvc generate controller name [action [, action] ...]
        # generate a new controller with names
    mvc run

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
    char        *documentRoot;
    char        *serverRoot;
    char        *configFile;
    char        *pathEnv;

    char        *libDir;                /* Appweb lib directory */
    char        *wwwDir;                /* Appweb esp-www default files directory */
    char        *confDir;               /* Appweb etc config directory */

    char        *appName;               /* Application name */
    char        *appDir;                /* Application top level base directory */
    char        *appCacheDir;           /* Cache directory */
    char        *appControllersDir;     /* Controllers directory */
    char        *appViewsDir;           /* Views directory */

    int         overwrite;
    int         quiet;
    int         error;
} App;

static App      *app;
static Esp      *esp;
static Http     *http;
static MaAppweb *appweb;

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
static void trace(cchar *tag, cchar *fmt, ...);
static void usageError();

#ifndef BLD_SERVER_ROOT
    #define BLD_SERVER_ROOT mprGetCurrentPath()
#endif
#ifndef BLD_CONFIG_FILE
    #define BLD_CONFIG_FILE NULL
#endif

/*********************************** Code *************************************/

MAIN(appweb, int argc, char **argv)
{
    Mpr     *mpr;
    cchar   *argp;
    int     argind, rc;

    if ((mpr = mprCreate(argc, argv, MPR_USER_EVENTS_THREAD)) == NULL) {
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
    findConfigFile();
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
        mprMark(app->documentRoot);
        mprMark(app->configFile);
        mprMark(app->pathEnv);
        mprMark(app->libDir);
        mprMark(app->wwwDir);
        mprMark(app->confDir);
        mprMark(app->appDir);
        mprMark(app->appName);
        mprMark(app->appCacheDir);
        mprMark(app->appControllersDir);
        mprMark(app->appViewsDir);
    }
}


static void getDirs()
{
    app->libDir = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
    app->wwwDir = mprJoinPath(app->libDir, "esp-www");
    app->confDir = mprGetPathDir(app->configFile);

    app->appDir = mprGetCurrentPath();
    app->appName = mprGetPathBase(app->appDir);

    //  MOB - these dir names must come from appweb.conf
    app->appCacheDir = mprJoinPath(app->appDir, "cache");
    app->appControllersDir = mprJoinPath(app->appDir, "controllers");
    app->appViewsDir = mprJoinPath(app->appDir, "views");

#if BLD_UNIX_LIKE
    app->pathEnv = sjoin("PATH=", getenv("PATH"), ":", mprGetAppDir(), NULL);
    putenv(app->pathEnv);
#endif
}


static void initialize()
{
    HttpStage   *stage;

    if ((app->appweb = maCreateAppweb()) == 0) {
        error("Can't create HTTP service for %s", mprGetAppName());
        return;
    }
    appweb = app->appweb;
    http = app->appweb->http;

    if ((app->meta = maCreateMeta(appweb, "default", NULL, NULL, -1)) == 0) {
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
    getDirs();
}


static void process(int argc, char **argv)
{
    cchar   *cmd;

    cmd = argv[0];

    if (scmp(cmd, "clean") == 0) {
        clean(argc - 1, &argv[1]);

    } else if (scmp(cmd, "compile") == 0) {
        compile(argc - 1, &argv[1]);

    } else if (scmp(cmd, "generate") == 0) {
        generate(argc - 1, &argv[1]);

    } else {
        error("Unknown command %s", cmd);
    }
}


static void clean(int argc, char **argv)
{
    MprList         *files;
    MprDirEntry     *dp;
    char            *path;
    int             next;

    files = mprGetPathFiles(app->appCacheDir, 0);
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0; ) {
        path = mprJoinPath(app->appCacheDir, dp->name);
        mprDeletePath(path);
    }
}


static void compileFile(cchar *path, bool isController)
{
    cchar   *cacheName, *module;

    if (isController) {
        cacheName = mprGetMD5Hash(path, slen(path), "controller_");
        module = mprGetNormalizedPath(sfmt("%s/%s%s", app->appCacheDir, cacheName, BLD_SHOBJ));
    } else {
    }
    // espCompile(conn, 
}


/*
    mvc compile [flat | app | controller_names | view_names]
        ## [] - compile controllers and views separately into cache
        ## [controller_names/view_names] - compile single file
        ## [app] - compile all files into a single source file with one init that calls all sub-init files
    mvc compile path/name.ejs ...
        ## [controller_names/view_names] - compile single file
    mvc compile static
        ## use makerom code and compile static into a file in cache
 */
static void compile(int argc, char **argv)
{
    MprList     *files;
    MprDirEntry *dp;
    char        *path, *kind;
    int         next;

    if (argc == 0) {
        kind = "*";
    } else {
        kind = argv[0];
    }
    if (scmp(kind, "*") == 0) {
        /*
            Build all items separately
         */
        files = mprGetPathFiles(app->appControllersDir, 1);
        for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
            path = mprJoinPath(app->appControllersDir, dp->name);
            compileFile(path, 1);
        }
    }
}


static void generate(int argc, char **argv)
{
    char    *kind, *path;

    /*
        generate app path
     */
    if (argc < 2) {
        usageError();
        return;
    }
    kind = argv[0];

    if (scmp(kind, "app") == 0) {
        path = argv[1];
        app->appName = mprGetPathBase(path);
        app->appDir = mprJoinPath(app->confDir, path);
        app->appCacheDir = mprJoinPath(app->appDir, "cache");
        app->appControllersDir = mprJoinPath(app->appDir, "controllers");
        app->appViewsDir = mprJoinPath(app->appDir, "views");
        generateAppDirs();
        generateAppFiles();
        generateAppConfigFile();

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

    path = mprJoinPath(app->appDir, path);
    if ((data = mprReadPath(path)) == 0) {
        error("Can't read %s", path);
        return;
    }
    data = sreplace(data, "${NAME}", app->appName);
    data = sreplace(data, "${DIR}", app->appDir);

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
    copyDir(mprJoinPath(app->wwwDir, "files"), app->appDir);
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


static void generateAppConfigFile()
{
    char    *from, *to, *conf;

    from = mprJoinPath(app->wwwDir, "app.conf");
    if ((conf = mprReadPath(from)) == 0) {
        error("Can't read %s", from);
        return;
    }
    to = sfmt("%s/conf/%s.conf", app->confDir, mprGetPathBase(app->appName));
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


static void findConfigFile()
{
    cchar   *userPath;

    userPath = app->configFile;
    if (app->configFile == 0) {
        app->configFile = mprJoinPathExt(BLD_PRODUCT, ".conf");
    }
    if (mprPathExists(app->configFile, R_OK)) {
        return;
    }
    if (!userPath) {
        app->configFile = mprJoinPath(mprGetAppDir(), mprAsprintf("../%s/%s.conf", BLD_LIB_NAME, mprGetAppName()));
        if (mprPathExists(app->configFile, R_OK)) {
            return;
        }
    }
    error("Can't open config file %s", app->configFile);
}


static void makeDir(cchar *dir)
{
    char    *path;

    path = mprJoinPath(app->appDir, dir);
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

    mprPrintfError("\n%s Usage:\n\n"
    "  %s [options] [IPaddress][:port] [documentRoot]\n\n"
    "  Options:\n"
    "    --config configFile    # Use named config file instead appweb.conf\n"
    "    --log logFile:level    # Log to file file at verbosity level\n"
    "    --version              # Output version information\n\n"
    "  Without IPaddress, %s will read the appweb.conf configuration file.\n\n",
        mprGetAppTitle(), name, name, name, name);

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
