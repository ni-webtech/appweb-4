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
    mvc clean
        ## remove cache/ *
    mvc compile [flat | app | controller_names | view_names]
        ## [] - compile controllers and views separately into cache
        ## [controller_names/view_names] - compile single file
        ## [app] - compile all files into a single source file with one init that calls all sub-init files
    mvc compile path/name.ejs ...
        ## [controller_names/view_names] - compile single file
    mvc compile static
        ## use makerom code and compile static into a file in cache
    mvc generate app name
        # mkdir cache views controllers layouts
        # generate *.conf
        # generate layouts/default.esp, 
        #   ./layouts/default.esp
        #   ./static/favicon.ico
        #   ./static/images/banner.jpg          (need these rommed)
        #   ./static/images/splash.jpg
        #   ./static/index.esp
        #   ./static/js/jquery.esp.js
        #   ./static/js/jquery.js
        #   ./static/js/jquery.simplemodal.js
        #   ./static/js/jquery.tablesorter.js
        #   ./static/layout.css
        #   ./static/themes/default.css
        #
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
    char        *pathVar;

    char        *libDir;
    char        *appwebDir;
    char        *appDir;
} App;

static App *app;
static Http *http;
static MaAppweb *appweb;
static Esp *esp;

/***************************** Forward Declarations ***************************/

static int  clean(int argc, char **argv);
static int  compile(int argc, char **argv);
static int  findConfigFile();
static int  generate(int argc, char **argv);
static int  generateAppStaticFiles(cchar *name);
static int  generateAppConfigFile(cchar *name);
static int  initialize();
static void manageApp(App *app, int flags);
static int  process(int argc, char **argv);
static int  usageError();

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
    int     argind;

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
        if (strcmp(argp, "--config") == 0) {
            if (argind >= argc) {
                return usageError();
            }
            app->configFile = sclone(argv[++argind]);

        } else if (strcmp(argp, "--log") == 0 || strcmp(argp, "-l") == 0) {
            if (argind >= argc) {
                return usageError();
            }
            maStartLogging(NULL, argv[++argind]);
            mprSetCmdlineLogging(1);

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            maStartLogging(NULL, "stderr:2");
            mprSetCmdlineLogging(1);

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintf("%s %s-%s\n", mprGetAppTitle(), BLD_VERSION, BLD_NUMBER);
            exit(0);

        } else {
            mprError("Unknown switch \"%s\"", argp);
            return usageError();
        }
    }
    if (mprStart() < 0) {
        mprUserError("Can't start MPR for %s", mprGetAppName());
        mprDestroy(MPR_EXIT_DEFAULT);
        return MPR_ERR_CANT_INITIALIZE;
    }
    if (argc > argind) {
        if (argc > (argind + 2)) {
            return usageError();
        }
    } else if (findConfigFile() < 0) {
        exit(7);
    }
    if (initialize() < 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    if (process(argc - argind, &argv[argind]) < 0) {
        //  MOB - RC
        return MPR_ERR_CANT_INITIALIZE;
    }
    mprLog(1, "Exit complete");
    mprDestroy(MPR_EXIT_DEFAULT);
    return 0;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->configFile);
        mprMark(app->documentRoot);
        mprMark(app->pathVar);
        mprMark(app->libDir);
        mprMark(app->meta);
    }
}


static void getDirs()
{
    app->libDir = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
    app->appwebDir = mprGetPathDir(app->configFile);
#if BLD_UNIX_LIKE
    app->pathVar = sjoin("PATH=", getenv("PATH"), ":", mprGetAppDir(), NULL);
    putenv(app->pathVar);
#endif
}


static int initialize()
{
    HttpStage   *stage;

    getDirs();
    if ((app->appweb = maCreateAppweb()) == 0) {
        mprUserError("Can't create HTTP service for %s", mprGetAppName());
        return MPR_ERR_CANT_CREATE;
    }
    appweb = app->appweb;
    http = app->appweb->http;

    if ((app->meta = maCreateMeta(appweb, "default", NULL, NULL, -1)) == 0) {
        mprUserError("Can't create HTTP server for %s", mprGetAppName());
        return MPR_ERR_CANT_CREATE;
    }
    if (maParseConfig(app->meta, app->configFile) < 0) {
        mprUserError("Can't configure the server, exiting.");
        return MPR_ERR_CANT_CREATE;
    }
    if ((stage = httpLookupStage(http, "espHandler")) == 0) {
        mprUserError("Can't find ESP handler in %s", app->configFile);
        return MPR_ERR_CANT_CREATE;
    }
    esp = stage->stageData;
    return 0;
}


static int process(int argc, char **argv)
{
    cchar   *cmd;

    cmd = argv[0];

    if (scmp(cmd, "clean") == 0) {
        clean(argc, argv);

    } else if (scmp(cmd, "compile") == 0) {
        compile(argc, argv);

    } else if (scmp(cmd, "generate") == 0) {
        generate(argc, argv);

    } else {
        mprError("Unknown command %s", cmd);
        return MPR_ERR_CANT_FIND;
    }
    return 0;
}


static int clean(int argc, char **argv)
{
    // files = mprGetPathFiles(app->cacheDir, 0);
    return 0;
}


static int compile(int argc, char **argv)
{
    return 0;
}


/*
 mvc generate app name
 # mkdir cache views controllers layouts
 # generate *.conf
 # generate layouts/default.esp, 
 #   ./layouts/default.esp
 #   ./static/favicon.ico
 #   ./static/images/banner.jpg          (need these rommed)
 #   ./static/images/splash.jpg
 #   ./static/index.esp
 #   ./static/js/jquery.esp.js
 #   ./static/js/jquery.js
 #   ./static/js/jquery.simplemodal.js
 #   ./static/js/jquery.tablesorter.js
 #   ./static/layout.css
 #   ./static/themes/default.css
 #

 */
static int generate(int argc, char **argv)
{
    char    *kind, *name;

    /*
        generate app name
     */
    if (argc < 2) {
        return usageError();
    }
    kind = argv[0];
    name = argv[1];

    if (scmp(kind, "app") == 0) {
        generateAppStaticFiles(name);
        generateAppConfigFile(name);
    } else {
        mprError("Unknown generation kind %s", kind);
        return usageError();
    }
    return 0;
}


static int generateAppStaticFiles(cchar *name)
{
    MprList     *files;
    char        *from, *to;
    int         next;

    files = mprGetPathFiles(mprJoinPath(app->libDir, "www"), 1);
    for (next = 0; (from = mprGetNextItem(files, &next)) != 0; ) {
        to = mprJoinPath(name, from);
        mprMakeDir(mprGetPathDir(to), 0755, 1);
        mprCopyPath(from, to, 0644);
    }
    return 0;
}


static int generateAppConfigFile(cchar *name)
{
    char    *directive;

    directive = sfmt("/*\n *    ESP Configuration for %s\n */\n\nEspAlias %s %s mvc\n", name, "/mvc", name);
#if UNUSED
    mprWritePath(sfmt("%s/conf/%s", app->appwebDir, name), directive);
#endif
    return 0;
}


static int findConfigFile()
{
    cchar   *userPath;

    userPath = app->configFile;
    if (app->configFile == 0) {
        app->configFile = mprJoinPathExt(BLD_PRODUCT, ".conf");
    }
    if (mprPathExists(app->configFile, R_OK)) {
        return 0;
    }
    if (!userPath) {
        app->configFile = mprJoinPath(mprGetAppDir(), mprAsprintf("../%s/%s.conf", BLD_LIB_NAME, mprGetAppName()));
        if (mprPathExists(app->configFile, R_OK)) {
            return 0;
        }
    }
    mprError("Can't open config file %s", app->configFile);
    return MPR_ERR_CANT_OPEN;
}


static int usageError(Mpr *mpr)
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
    return MPR_ERR_BAD_STATE;
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
