/**
    esp.c -- Embedded Server Pages (ESP) utility program

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
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
    char        *database;              /* Database provider "mdb" | "sqlite" */
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
    HttpRoute   *route;
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
    int         isMvc;
    int         minified;
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

/********************************* Templates **********************************/

static cchar *AppHeader = "\
/*\n\
    esp-app.h -- User header for ESP Applications and Pages\n\
\n\
    This file may be located in the DocumentRoot for a Route. All ESP requests using that Route will\n\
    source this header.\n\
 */\n\
#ifndef _h_ESP_APP\n\
#define _h_ESP_APP 1\n\
\n\
#include    \"esp.h\"\n\
\n\
#ifdef __cplusplus\n\
extern \"C\" {\n\
#endif\n\
\n\
/*\n\
    Put your definitions here\n\
\n\
    If you need to remap an abbreviated API name that clashes with an API name of yours, you can rename the\n\
    ESP APIs here. Change the \"xx_\" to any unique prefix you require. Then use that name in ESP pages and controllers.\n\
\n\
    #define script xx_script\n\
*/\n\
\n\
#ifdef __cplusplus\n\
} /* extern C */\n\
#endif\n\
#endif /* _h_ESP_APP */\n\
";


static cchar *ControllerTemplateHeader = "\
/*\n\
    ${NAME} controller\n\
 */\n\
#include \"esp-app.h\"\n\
\n\
";


static cchar *ControllerTemplateFooter = "\
ESP_EXPORT int esp_controller_${NAME}(EspRoute *eroute, MprModule *module) \n\
{\n\
${DEFINE_ACTIONS}    return 0;\n\
}\n";


static cchar *ScaffoldTemplateHeader = "\
/*\n\
    ${NAME} controller\n\
 */\n\
#include \"esp-app.h\"\n\
\n\
static void create() { \n\
    if (updateRec(createRec(\"${NAME}\", params()))) {\n\
        inform(\"New ${NAME} created\");\n\
        renderView(\"${NAME}-list\");\n\
    } else {\n\
        renderView(\"${NAME}-edit\");\n\
    }\n\
}\n\
\n\
static void destroy() { \n\
    if (removeRec(\"${NAME}\", param(\"id\"))) {\n\
        inform(\"${TITLE} removed\");\n\
    }\n\
    renderView(\"${NAME}-list\");\n\
}\n\
\n\
static void edit() { \n\
    readRec(\"${NAME}\");\n\
}\n\
\n\
static void list() { }\n\
\n\
static void init() { \n\
    createRec(\"${NAME}\", 0);\n\
    renderView(\"${NAME}-edit\");\n\
}\n\
\n\
static void show() { \n\
    readRec(\"${NAME}\");\n\
    renderView(\"${NAME}-edit\");\n\
}\n\
\n\
static void update() { \n\
    if (updateFields(\"${NAME}\", params())) {\n\
        inform(\"${TITLE} updated successfully.\");\n\
        renderView(\"${NAME}-list\");\n\
    } else {\n\
        renderView(\"${NAME}-edit\");\n\
    }\n\
}\n\
\n\
";

static cchar *ScaffoldTemplateFooter = "\
ESP_EXPORT int esp_controller_${NAME}(EspRoute *eroute, MprModule *module) \n\
{\n\
    espDefineAction(eroute, \"${NAME}-create\", create);\n\
    espDefineAction(eroute, \"${NAME}-destroy\", destroy);\n\
    espDefineAction(eroute, \"${NAME}-edit\", edit);\n\
    espDefineAction(eroute, \"${NAME}-init\", init);\n\
    espDefineAction(eroute, \"${NAME}-list\", list);\n\
    espDefineAction(eroute, \"${NAME}-show\", show);\n\
    espDefineAction(eroute, \"${NAME}-update\", update);\n\
${DEFINE_ACTIONS}    return 0;\n\
}\n";


static cchar *ScaffoldListView = "\
<h1>${TITLE} List</h1>\n\
\n\
<% table(readTable(\"${NAME}\"), \"{data-click: '@edit'}\"); %>\n\
<% buttonLink(\"New ${TITLE}\", \"@init\", 0); %>\n\
";


static cchar *ScaffoldEditView =  "\
<h1><%= hasRec() ? \"Edit\" : \"Create\" %> ${TITLE}</h1>\n\
\n\
<% form(0, 0); %>\n\
    <table border=\"0\">\n\
    <% {\n\
        char    *name, *uname;\n\
        int     next;\n\
        MprList *cols = getColumns(NULL);\n\
        for (ITERATE_ITEMS(cols, name, next)) {\n\
            if (smatch(name, \"id\")) continue;\n\
            uname = spascal(name);\n\
    %>\n\
            <tr><td><% render(uname); %></td><td><% input(name, 0); %></td></tr>\n\
        <% } %>\n\
    <% } %>\n\
    </table>\n\
    <% button(\"commit\", \"OK\", 0); %>\n\
    <% buttonLink(\"Cancel\", \"@\", 0); %>\n\
    <% if (hasRec()) buttonLink(\"Delete\", \"@destroy\", \"{data-method: 'DELETE'}\"); %>\n\
<% endform(); %>\n\
";



#if UNUSED && KEEP
static cchar *MigrationTemplate = "\
/*\n\
    ${COMMENT}\n\
 */\n\
#include \"esp-app.h\"\n\
\n\
static int forward(Edi *db) {\n\
${FORWARD}}\n\
\n\
static int backward(Edi *db) {\n\
${BACKWARD}}\n\
\n\
ESP_EXPORT int esp_migration_post(Esp *esp, MprModule *module)\n\
{\n\
    espDefineMigration(\"${NAME}\", forward, backward);\n\
    return 0;\n\
}\n\
";
#endif

/***************************** Forward Declarations ***************************/

static void clean(int argc, char **argv);
static void compile(int argc, char **argv);
static void copyDir(cchar *fromDir, cchar *toDir);
static void fail(cchar *fmt, ...);
static void findConfigFile();
static void generate(int argc, char **argv);
static void generateAppDb();
static void generateAppDirs();
static void generateAppFiles();
static void generateAppConfigFile();
static void generateAppHeader();
static void initialize();
static void makeDir(cchar *dir);
static void makeFile(cchar *path, cchar *data, cchar *msg);
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
    app->database = sclone("mdb");

    for (argind = 1; argind < argc && !app->error; argind++) {
        argp = argv[argind];
        if (*argp != '-') {
            break;
        }
        if (smatch(argp, "--chdir")) {
            if (argind >= argc) {
                usageError();
            } else {
                argp = argv[++argind];
                if (chdir((char*) argp) < 0) {
                    fail("Can't change directory to %s", argp);
                }
            }

        } else if (smatch(argp, "--config")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->configFile = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "--database")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->database = sclone(argv[++argind]);
                if (!smatch(app->database, "mdb") && !smatch(app->database, "sdb")) {
                    fail("Unknown database \"%s\"", app->database);
                    usageError();
                }
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
                mprStartLogging(argv[++argind], 0);
                mprSetCmdlineLogging(1);
            }

        } else if (smatch(argp, "--min")) {
            app->minified = 1;

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
            mprStartLogging("stderr:2", 0);
            mprSetCmdlineLogging(1);

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            mprPrintf("%s %s-%s\n", mprGetAppTitle(), BLD_VERSION, BLD_NUMBER);
            exit(0);

        } else {
            fail("Unknown switch \"%s\"", argp);
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
        mprMark(app->route);
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
        mprMark(app->database);
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
    if ((app->route = httpCreateRoute(NULL)) == 0) {
        return;
    }
    if ((app->eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return;
    }
    route = app->route;
    eroute = app->eroute;
    httpSetRouteDir(route, path);
    eroute->cacheDir = mprJoinPath(path, "cache");
    eroute->controllersDir = mprJoinPath(path, "controllers");
    eroute->dbDir = mprJoinPath(path, "db");
    eroute->layoutsDir = mprJoinPath(path, "layouts");
    eroute->staticDir = mprJoinPath(path, "static");
    eroute->viewsDir = mprJoinPath(path, "views");
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
        fail("Can't find default host");
        return;
    }
    routeName = app->routeName;
    routePrefix = app->routePrefix ? app->routePrefix : "/";

    /*
        Must find a route with ESP configuration
     */
    for (next = 0; (rp = mprGetNextItem(host->routes, &next)) != 0; ) {
        if (routeName) {
            if (strcmp(routeName, rp->name) != 0) {
                continue;
            }
        }
        if (rp->startWith && strcmp(routePrefix, rp->startWith) != 0) {
            continue;
        }
        if ((app->eroute = rp->eroute) != 0 && app->eroute->compile) {
            break;
        }
    }
    if (rp == 0) {
        fail("Can't find ESP configuration in config file");
        return;
    }
    route = rp;
    eroute = app->eroute;
    app->appName = mprGetPathBase(route->dir);
    app->isMvc = route->sourceName != 0;
}


static void readConfig()
{
    HttpStage   *stage;

    if ((app->appweb = maCreateAppweb()) == 0) {
        fail("Can't create HTTP service for %s", mprGetAppName());
        return;
    }
    appweb = app->appweb;
    http = app->appweb->http;

    findConfigFile();
    if (app->error) {
        return;
    }
    if ((app->server = maCreateServer(appweb, "default")) == 0) {
        fail("Can't create HTTP server for %s", mprGetAppName());
        return;
    }
    if (maParseConfig(app->server, app->configFile, MA_PARSE_NON_SERVER) < 0) {
        fail("Can't configure the server, exiting.");
        return;
    }
    if ((stage = httpLookupStage(http, "espHandler")) == 0) {
        fail("Can't find ESP handler in %s", app->configFile);
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
            fail("Unknown command %s", cmd);
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

    if (app->error) {
        return;
    }
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

    if (app->error) {
        return;
    }
    cmd = mprCreateCmd(0);
    trace("RUN", "appweb -v");
    if (mprRunCmd(cmd, "appweb -v", NULL, NULL, -1, MPR_CMD_DETACH) != 0) {
        fail("Can't run command: \n%s", app->command);
        return;
    }
    mprWaitForCmd(cmd, -1);
}


static int runCommand(cchar *command, cchar *csource, cchar *module)
{
    MprCmd      *cmd;
    char        *err, *out;

    cmd = mprCreateCmd(0);
    if ((app->command = espExpandCommand(command, csource, module)) == 0) {
        fail("Missing EspCompile directive for %s", csource);
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
	if (mprRunCmd(cmd, app->command, &out, &err, -1, 0) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
        fail("Can't run command: \n%s\nError %s", app->command, err);
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
    app->module = mprNormalizePath(sfmt("%s/%s%s", eroute->cacheDir, app->cacheName, BLD_SHOBJ));

    if (kind == ESP_CONTROLLER) {
        app->csource = source;
        if (app->flatFile) {
            if ((data = mprReadPathContents(source, &len)) == 0) {
                fail("Can't read %s", source);
                return;
            }
            if (mprWriteFile(app->flatFile, data, slen(data)) < 0) {
                fail("Can't write compiled script file %s", app->flatFile->path);
                return;
            }
            mprWriteFileFmt(app->flatFile, "\n\n");
            mprAddItem(app->flatItems, sfmt("esp_controller_%s", mprTrimPathExt(mprGetPathBase(source))));
        }
    }
    if (kind & (ESP_PAGE | ESP_VIEW)) {
#if UNUSED
        layout = mprJoinPath(mprJoinPath(route->dir, eroute->layoutsDir), "default.esp");
#else
        layout = app->isMvc ? mprJoinPath(eroute->layoutsDir, "default.esp") : 0;
#endif
        if ((page = mprReadPathContents(source, &len)) == 0) {
            fail("Can't read %s", source);
            return;
        }
        /* No yield here */
        if ((script = espBuildScript(route, page, source, app->cacheName, layout, &err)) == 0) {
            fail("Can't build %s, error %s", source, err);
            return;
        }
        len = slen(script);
        if (app->flatFile) {
            if (mprWriteFile(app->flatFile, script, len) < 0) {
                fail("Can't write compiled script file %s", app->flatFile->path);
                return;
            }
            mprWriteFileFmt(app->flatFile, "\n\n");
            mprAddItem(app->flatItems, sfmt("esp_%s", app->cacheName));

        } else {
            trace("BUILD", "%s", source);
            app->csource = mprJoinPathExt(mprTrimPathExt(app->module), ".c");
            if (mprWritePathContents(app->csource, script, len, 0664) < 0) {
                fail("Can't write compiled script file %s", app->csource);
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
            fail("Missing EspCompile directive for %s", app->csource);
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
    char        *path, *kind, *line, *name;
    int         i, next;

    if (app->error) {
        return;
    }
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
    if (smatch(kind, "controller") || smatch(kind, "controllers")) {
        if (argc == 1) {
        } else {
            for (i = 1; i < argc; i++) {
                name = mprTrimPathExt(mprGetPathBase(argv[i]));
                path = mprJoinPathExt(mprJoinPath(eroute->controllersDir, name), "c");
                compileFile(path, ESP_CONTROLLER);
            }
        }

    } else if (smatch(kind, "*")) {
        /*
            Build all items separately
         */
        if (app->isMvc) {
            app->files = files = mprGetPathTree(eroute->controllersDir, 0);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "c")) {
                    compileFile(path, ESP_CONTROLLER);
                }
            }
            app->files = files = mprGetPathTree(eroute->viewsDir, 0);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "esp")) {
                    compileFile(path, ESP_VIEW);
                }
            }
            app->files = files = mprGetPathTree(eroute->staticDir, MPR_PATH_ENUM_DIRS);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "esp")) {
                    compileFile(path, ESP_PAGE);
                }
            }
        } else {
            app->files = files = mprGetPathTree(route->dir, MPR_PATH_ENUM_DIRS);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "esp")) {
                    compileFile(path, ESP_PAGE);
                }
            }
        }

    } else if (smatch(kind, "flat")) {
        /*
            Catenate all source
         */
        app->flatItems = mprCreateList(-1, 0);
        app->flatPath = path = mprJoinPath(eroute->cacheDir, "app.c");
        if ((app->flatFile = mprOpenFile(path, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
            fail("Can't open %s", path);
            return;
        }
        mprWriteFileFmt(app->flatFile, "/*\n    Flat compilation of %s\n */\n\n", app->appName);

        if (app->isMvc) {
            app->files = files = mprGetPathTree(eroute->controllersDir, 0);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "c")) {
                    compileFile(path, ESP_CONTROLLER);
                }
            }
            app->files = files = mprGetPathTree(eroute->viewsDir, 0);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                compileFile(path, ESP_VIEW);
            }
            app->files = files = mprGetPathTree(eroute->staticDir, 0);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "esp")) {
                    compileFile(path, ESP_PAGE);
                }
            }
        } else {
            app->files = files = mprGetPathTree(route->dir, 0);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
                if (smatch(mprGetPathExt(path), "esp")) {
                    compileFile(path, ESP_PAGE);
                }
            }
        }
        mprWriteFileFmt(app->flatFile, "\nESP_EXPORT int esp_app_%s(EspRoute *el, MprModule *module) {\n", app->appName);
        for (next = 0; (line = mprGetNextItem(app->flatItems, &next)) != 0; ) {
            mprWriteFileFmt(app->flatFile, "    %s(el, module);\n", line);
        }
        mprWriteFileFmt(app->flatFile, "    return 0;\n}\n");
        mprCloseFile(app->flatFile);

        app->module = mprNormalizePath(sfmt("%s/app%s", eroute->cacheDir, BLD_SHOBJ));
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
                fail("Command arguments must be files with a \".esp\" extension");
                return;
            }
            path = mprJoinPath(route->dir, argv[i]);
            compileFile(path, ESP_PAGE);
        }
    }
    if (!app->error) {
        trace("TASK", "Complete");
    }
}


static void generateApp(cchar *name)
{
    app->appName = sclone(name);
    setDirs(name);
    generateAppDirs();
    generateAppFiles();
    generateAppConfigFile();
    generateAppHeader();
    generateAppDb();
}


/*
    esp generate controller name [action [, action] ...]
 */
static void generateController(int argc, char **argv)
{
    MprHash     *tokens;
    cchar       *defines, *name, *path, *data, *title, *action;
    int         i;

    if (argc < 1) {
        usageError();
        return;
    }
    name = sclone(argv[0]);
    title = spascal(name);
    path = mprJoinPathExt(mprJoinPath(eroute->controllersDir, name), ".c");
    defines = sclone("");
    for (i = 1; i < argc; i++) {
        action = argv[i];
        defines = sjoin(defines, sfmt("    espDefineAction(eroute, \"%s-%s\", %s);\n", name, action, action), NULL);
    }
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, DEFINE_ACTIONS: '%s' }", name, title, defines));

    data = stemplate(ControllerTemplateHeader, tokens);
    for (i = 1; i < argc; i++) {
        action = argv[i];
        data = sjoin(data, sfmt("static void %s() {\n}\n\n", action), NULL);
    }
    data = sjoin(data, stemplate(ControllerTemplateFooter, tokens), NULL);
    makeFile(path, data, "Controller");
}


static void generateScaffoldController(int argc, char **argv)
{
    MprHash     *tokens, *actions;
    cchar       *defines, *name, *path, *data, *title, *action;
    int         i;

    name = sclone(argv[0]);
    title = spascal(name);
    actions = mprCreateHashFromWords("create destroy edit init list show update");

    /*
        Create controller
     */
    path = mprJoinPathExt(mprJoinPath(eroute->controllersDir, name), ".c");
    defines = sclone("");
    for (i = 1; i < argc; i++) {
        action = argv[i];
        if (mprLookupKey(actions, action)) {
            continue;
        }
        defines = sjoin(defines, sfmt("    espDefineAction(eroute, \"%s-%s\", %s);\n", name, action, action), NULL);
    }
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, DEFINE_ACTIONS: '%s' }", name, title, defines));

    data = stemplate(ScaffoldTemplateHeader, tokens);
    for (i = 1; i < argc; i++) {
        action = argv[i];
        if (mprLookupKey(actions, action)) {
            continue;
        }
        data = sjoin(data, sfmt("static void %s() {\n}\n\n", action), NULL);
    }
    data = sjoin(data, stemplate(ScaffoldTemplateFooter, tokens), NULL);
    makeFile(path, data, "Scaffold");
}



#if UNUSED && KEEP
/*
    esp generate migration description model [field:type [, field:type] ...]
 */
static void generateScaffoldMigration(int argc, char **argv)
{
    MprHash     *tokens;
    cchar       *comment, *name, *title, *seq, *forward, *backward, *data, *path, *def, *field, *fileComment;
    char        *typeString;
    int         i, type, flags;

    name = sclone(argv[0]);
    title = spascal(name);
    comment = sfmt("Create Scaffold %s", title);
    seq = mprGetDate("%Y%m%d%H%M%S");

    forward = sfmt("    ediAddTable(db, \"%s\");\n", name);
    backward = sfmt("    ediRemoveTable(db, \"%s\");\n", name);

    for (i = 1; i < argc; i++) {
        field = stok(sclone(argv[i]), ":", &typeString);
        if ((type = ediParseTypeString(typeString)) < 0) {
            fail("Unknown type '%s' for field '%s'", typeString, field);
            return;
        }
        //  MOB -- should support flags
        flags = 0;
        def = sfmt("    ediAddColumn(db, \"%s\", \"%s\", %d, %d);\n", name, field, type, flags);
        forward = sjoin(forward, def, NULL);
    }
    dir = mprJoinPath(eroute->dbDir, "migrations");
    makeDir(dir);

    fileComment = sreplace(comment, " ", "_");
    path = sfmt("%s/%s_%s.c", eroute->migrationsDir, seq, fileComment, ".c");

    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, COMMENT: '%s', FORWARD: '%s', BACKWARD: '%s' }", 
        name, title, comment, forward, backward));
    data = stemplate(MigrationTemplate, tokens);
    makeFile(path, data, "Migration");
}
#endif


/*
    esp generate table name [field:type [, field:type] ...]
 */
static void generateTable(int argc, char **argv)
{
    Edi     *edi;
    cchar   *tableName, *field;
    char    *typeString;
    int     rc, i, type;

    tableName = sclone(argv[0]);
//  MOB - implement title
#if UNUSED
    title = spascal(tableName);
#endif
    edi = eroute->edi;

    if (edi == 0) {
        fail("Database not open. Check appweb.conf");
        return;
    }
    edi->flags |= EDI_SUPPRESS_SAVE;
    if ((rc = ediAddTable(edi, tableName)) < 0) {
        if (rc != MPR_ERR_ALREADY_EXISTS) {
            fail("Can't add table '%s'", tableName);
        }
    } else {
        if ((rc = ediAddColumn(edi, tableName, "id", EDI_TYPE_INT, EDI_AUTO_INC | EDI_INDEX | EDI_KEY)) != 0) {
            fail("Can't add column 'id'");
        }
    }
    for (i = 1; i < argc && !app->error; i++) {
        field = stok(sclone(argv[i]), ":", &typeString);
        if ((type = ediParseTypeString(typeString)) < 0) {
            fail("Unknown type '%s' for field '%s'", typeString, field);
            break;
        }
        if ((rc = ediAddColumn(edi, tableName, field, type, 0)) != 0) {
            if (rc != MPR_ERR_ALREADY_EXISTS) {
                fail("Can't add column '%s'", field);
                break;
            } else {
                ediChangeColumn(edi, tableName, field, type, 0);
            }
        }
    }
    edi->flags &= ~EDI_SUPPRESS_SAVE;
    ediSave(edi);
    trace("UPDATE", "Database schema");
}


static void generateScaffoldViews(int argc, char **argv)
{
    MprHash     *tokens;
    cchar       *title, *name, *path, *data;

    name = sclone(argv[0]);
    title = spascal(name);

    path = sfmt("%s/%s-list.esp", eroute->viewsDir, name);
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, }", name, title));
    data = stemplate(ScaffoldListView, tokens);
    makeFile(path, data, "Scaffold Index View");

    path = sfmt("%s/%s-edit.esp", eroute->viewsDir, name);
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, }", name, title));
    data = stemplate(ScaffoldEditView, tokens);
    makeFile(path, data, "Scaffold Index View");
}

/*
    esp generate scaffold NAME [field:type [, field:type] ...]
 */
static void generateScaffold(int argc, char **argv)
{
    if (argc < 1) {
        usageError();
        return;
    }
    generateScaffoldController(1, argv);
    generateScaffoldViews(argc, argv);
    generateTable(argc, argv);
}


static void generate(int argc, char **argv)
{
    char    *kind;

    if (argc < 2) {
        usageError();
        return;
    }
    kind = argv[0];

    if (smatch(kind, "app")) {
        generateApp(argv[1]);

    } else if (smatch(kind, "controller")) {
        readConfig();
        generateController(argc - 1, &argv[1]);

    } else if (smatch(kind, "scaffold")) {
        readConfig();
        generateScaffold(argc - 1, &argv[1]);

#if UNUSED && KEEP
    } else if (smatch(kind, "migration")) {
        readConfig();
        generateMigration(argc - 1, argv[1]);

#endif
    } else if (smatch(kind, "table")) {
        readConfig();
        generateTable(argc - 1, &argv[1]);

    } else {
        mprError("Unknown generation kind %s", kind);
        usageError();
        return;
    }
    if (!app->error) {
        trace("TASK", "Complete");
    }
}


static void generateAppDirs()
{
    makeDir("");
    makeDir("cache");
    makeDir("controllers");
    makeDir("db");
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

    path = mprJoinPath(route->dir, path);
    if ((data = mprReadPathContents(path, &len)) == 0) {
        fail("Can't read %s", path);
        return;
    }
    data = sreplace(data, "${NAME}", app->appName);
    data = sreplace(data, "${TITLE}", spascal(app->appName));
    data = sreplace(data, "${DATABASE}", app->database);
    data = sreplace(data, "${DIR}", route->dir);
    data = sreplace(data, "${LISTEN}", app->listen);
    data = sreplace(data, "${LIBDIR}", app->libDir);

    tmp = mprGetTempPath(0);
    if (mprWritePathContents(tmp, data, slen(data), 0644) < 0) {
        fail("Can't write %s", path);
        return;
    }
    if (rename(tmp, path) < 0) {
        fail("Can't rename %s to %s", tmp, path);
    }
}


static void generateAppFiles()
{
    copyDir(mprJoinPath(app->wwwDir, "files"), route->dir);
    fixupFile("layouts/default.esp");
}


static bool checkPath(cchar *path)
{
    if (scontains(path, "jquery.", -1)) {
        if (app->minified) {
            if (!scontains(path, ".min.js", -1)) {
                return 0;
            }
        } else {
            if (scontains(path, ".min.js", -1)) {
                return 0;
            }
        }
    } else if (scontains(path, "treeview", -1)) {
        return 0;
    }
    return 1;
}



static void copyDir(cchar *fromDir, cchar *toDir)
{
    MprList     *files;
    MprDirEntry *dp;
    char        *from, *to;
    int         next;

    files = mprGetPathFiles(fromDir, 1);
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
        if (!checkPath(dp->name)) {
            continue;
        }
        from = mprJoinPath(fromDir, dp->name);
        to = mprJoinPath(toDir, dp->name);
        if (dp->isDir) {
            if (!mprPathExists(to, R_OK)) {
                if (mprMakeDir(to, 0755, -1, -1, 1) < 0) {
                    fail("Can't make directory %s", to);
                    return;
                }
                trace("CREATE",  "Directory: %s", mprGetRelPath(to));
            }
            copyDir(from, to);
        
        } else {
            if (mprMakeDir(mprGetPathDir(to), 0755, -1, -1, 1) < 0) {
                fail("Can't make directory %s", mprGetPathDir(to));
                return;
            }
            if (mprPathExists(to, R_OK) && !app->overwrite) {
                trace("EXISTS",  "File: %s", mprGetRelPath(to));
            } else {
                trace("CREATE",  "File: %s", mprGetRelPath(to));
            }
            if (mprCopyPath(from, to, 0644) < 0) {
                fail("Can't copy file %s to %s", from, mprGetRelPath(to));
                return;
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
    if ((conf = mprReadPathContents(from)) == 0) {
        fail("Can't read %s", from);
        return;
    }
    to = sfmt("%s/conf/%s.conf", app->serverRoot, mprGetPathBase(app->appName));
    if (mprPathExists(to, R_OK) && !app->overwrite) {
        trace("EXISTS",  "Config file: %s", to);
        return;

    } else if (mprWritePathContents(to, conf, slen(conf), 0644) < 0) {
        fail("Can't write %s", to);
        return;
    }
    fixupFile(to);
    trace("CREATE",  "Config file: %s", to);
}
#endif


static void copyFile(cchar *from, cchar *to)
{
    char    *data;
    ssize   len;

    if ((data = mprReadPathContents(from, &len)) == 0) {
        fail("Can't read %s", from);
        return;
    }
    if (mprPathExists(to, R_OK) && !app->overwrite) {
        trace("EXISTS",  "Config file: %s", to);
        return;

    } else if (mprWritePathContents(to, data, slen(data), 0644) < 0) {
        fail("Can't write %s", to);
        return;
    }
    fixupFile(mprGetAbsPath(to));
    trace("CREATE",  "File: %s", mprGetRelPath(to));
}


static void generateAppConfigFile()
{
    char    *from, *to;

    from = mprJoinPath(app->wwwDir, "appweb.conf");
    to = mprJoinPath(route->dir, "appweb.conf");
    copyFile(from, to);

    from = mprJoinPath(app->wwwDir, "app.conf");
    to = mprJoinPath(route->dir, "app.conf");
    copyFile(from, to);
}


static void generateAppHeader()
{
    MprHash *tokens;
    char    *path, *data;

    path = mprJoinPath(route->dir, mprJoinPathExt("esp-app", "h"));
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s }", app->appName, spascal(app->appName)));
    data = stemplate(AppHeader, tokens);
    makeFile(path, data, "Header");
}


static void generateAppDb()
{
    char    *ext, *dbpath, buf[1];

    ext = smatch(app->database, "mdb") ? "mdb" : "sdb";
    dbpath = sfmt("%s/%s.%s", eroute->dbDir, app->appName, ext);
    if (mprWritePathContents(dbpath, buf, 0, 0664) < 0) {
        return;
    }
    trace("CREATE", "Database: %s", mprGetRelPath(dbpath));
}


/*
    Search strategy is:

    [--config dir] : ./appweb.conf : /usr/lib/appweb/lib/appweb.conf
 */
static void findConfigFile()
{
    cchar   *userPath;

    userPath = app->configFile;
    if (app->configFile == 0) {
        app->configFile = mprJoinPathExt(BLD_PRODUCT, ".conf");
    }
    if (!mprPathExists(app->configFile, R_OK)) {
        if (userPath) {
            fail("Can't open config file %s", app->configFile);
            return;
        }
        app->configFile = mprJoinPath(mprGetAppDir(), sfmt("../%s/%s.conf", BLD_LIB_NAME, BLD_PRODUCT));
        if (!mprPathExists(app->configFile, R_OK)) {
            fail("Can't open config file %s", app->configFile);
            return;
        }
    }
    app->serverRoot = mprGetAbsPath(mprGetPathDir(app->configFile));
}


static void makeDir(cchar *dir)
{
    char    *path;

    path = mprJoinPath(route->dir, dir);
    if (mprPathExists(path, X_OK)) {
        trace("EXISTS",  "Directory: %s", path);
    } else {
        if (mprMakeDir(path, 0755, -1, -1, 1) < 0) {
            app->error++;
        } else {
            trace("CREATE",  "Directory: %s", mprGetRelPath(path));
        }
    }
}


static void makeFile(cchar *path, cchar *data, cchar *msg)
{
    bool    exists;

    exists = mprPathExists(path, R_OK);
    if (exists && !app->overwrite) {
        trace("EXISTS", path);
        return;
    }
    if (mprWritePathContents(path, data, slen(data), 0644) < 0) {
        fail("Can't write %s", path);
        return;
    }
    msg = sfmt("%s: %s", msg, path);
    if (!exists) {
        trace("CREATE", mprGetRelPath(path));
    } else {
        trace("OVERWRITE", path);
    }
}


static void usageError(Mpr *mpr)
{
    cchar   *name;

    name = mprGetAppName();

    mprPrintfError("\nESP Usage:\n\n"
    "  %s [options] [commands]\n\n"
    "  Options:\n"
    "    --chdir dir            # Change to the named directory first\n"
    "    --config configFile    # Use named config file instead appweb.conf\n"
    "    --database name        # Database provider 'mdb|sqlite' \n"
    "    --flat                 # Compile into a single module\n"
    "    --listen [ip:]port     # Listen on specified address \n"
    "    --log logFile:level    # Log to file file at verbosity level\n"
    "    --overwrite            # Overwrite existing files \n"
    "    --quiet                # Don't emit trace \n"
    "    --routeName name       # Route name in appweb.conf to use \n"
    "    --routePrefix prefix   # Route prefix in appweb.conf to use \n"
    "    --verbose              # Emit verbose trace \n"
    "\n"
    "  Commands:\n"
    "    esp clean\n"
    "    esp compile\n"
    "    esp compile controller name ...\n"
    "    esp compile path/*.esp\n"
    "    esp generate app name\n"
    "    esp generate controller name [action [, action] ...\n"
    "    esp generate scaffold model [field:type [, field:type] ...]\n"
    "    esp generate table name [field:type [, field:type] ...]\n"
    "    esp run\n"
    "", name);
#if UNUSED && KEEP
    "    --static               # Compile static content into C code\n"
    "    esp generate migration description model [field:type [, field:type] ...]\n"
    "    esp migrate [forward|backward|NNN]\n"
#endif
    app->error++;
}


static void fail(cchar *fmt, ...)
{
    va_list     args;
    char        *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
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
        mprRawLog(0, "%14s %s\n", tag, msg);
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
