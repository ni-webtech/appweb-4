/**
    esp.c -- Embedded Server Pages (ESP) utility program

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "esp.h"

#if BIT_FEATURE_ESP
/********************************** Locals ************************************/
/*
    Global application object. Provides the top level roots of all data objects for the GC.
 */
typedef struct App {
    Mpr         *mpr;
    MaAppweb    *appweb;
    MaServer    *server;

    cchar       *appName;               /* Application name */
    cchar       *serverRoot;            /* Root directory for server config */
    cchar       *configFile;            /* Arg to --config */
    cchar       *currentDir;            /* Initial starting current directory */
    cchar       *database;              /* Database provider "mdb" | "sqlite" */
    cchar       *flatPath;              /* Output filename for flat compilations */
    cchar       *libDir;                /* Appweb lib directory */
    cchar       *listen;                /* Listen endpoint for "esp run" */
    cchar       *platform;              /* Target platform os-arch-profile (lower) */
    cchar       *wwwDir;                /* Appweb esp-www default files directory */
    MprFile     *flatFile;              /* Output file for flat compilations */
    MprList     *flatItems;             /* Items to invoke from Init */

    /*
        GC retention
     */
    MprList     *routes;
    MprList     *files;                 /* List of files to process */
    MprHash     *targets;               /* Command line targets */

    cchar       *command;               /* Compilation or link command */
    cchar       *cacheName;             /* MD5 name of cached component */
    cchar       *csource;               /* Name of "C" source for controller or view */
    cchar       *routeName;             /* Name of route to use for ESP configuration */
    cchar       *routePrefix;           /* Route prefix to use for ESP configuration */
    cchar       *module;                /* Compiled module name */

    int         error;                  /* Any processing error */
    int         flat;                   /* Combine all inputs into one, flat output */ 
    int         minified;               /* Use minified JS files */
    int         overwrite;              /* Overwrite existing files if required */
    int         quiet;                  /* Don't trace progress */
} App;

static App       *app;                  /* Top level application object */
static Esp       *esp;                  /* ESP control object */
static Http      *http;                 /* HTTP service object */
static MaAppweb  *appweb;               /* Appweb service object */

/*
    CompileFile flags
 */
#define ESP_CONTROLLER  0x1
#define ESP_VIEW        0x2
#define ESP_PAGE        0x4

#define ESP_FOUND_TARGET 1

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

static void clean(MprList *routes);
static void compile(MprList *routes);
static void compileItems(HttpRoute *route);
static void compileFlat(HttpRoute *route);
static void copyEspDir(cchar *fromDir, cchar *toDir);
static HttpRoute *createRoute(cchar *dir);
static void fail(cchar *fmt, ...);
static void findConfigFile();
static MprHash *getTargets(int argc, char **argv);
static HttpRoute *getMvcRoute();
static MprList *getRoutes();
static void generate(int argc, char **argv);
static void generateAppDb(HttpRoute *route);
static void generateAppDirs(HttpRoute *route);
static void generateAppFiles(HttpRoute *route);
static void generateAppConfigFile(HttpRoute *route);
static void generateAppHeader(HttpRoute *route);
static void initialize();
static void makeEspDir(HttpRoute *route, cchar *dir);
static void makeEspFile(cchar *path, cchar *data, cchar *msg);
static void manageApp(App *app, int flags);
static void process(int argc, char **argv);
static void readConfig();
static void run(int argc, char **argv);
static void trace(cchar *tag, cchar *fmt, ...);
static void usageError();
static bool validTarget(cchar *target);

/*********************************** Code *************************************/

int main(int argc, char **argv)
{
    Mpr     *mpr;
    cchar   *argp, *logSpec, *path;
    int     argind, rc;

    if ((mpr = mprCreate(argc, argv, 0)) == NULL) {
        exit(1);
    }
    if ((app = mprAllocObj(App, manageApp)) == NULL) {
        exit(2);
    }
    mprAddRoot(app);
    mprAddStandardSignals();
    
    logSpec = "stderr:0";
    argc = mpr->argc;
    argv = (char**) mpr->argv;
    app->mpr = mpr;
    app->configFile = 0;
    app->listen = sclone(ESP_LISTEN);
    app->database = sclone("mdb");

    for (argind = 1; argind < argc && !app->error; argind++) {
        argp = argv[argind];
        if (*argp++ != '-') {
            break;
        }
        if (*argp == '-') {
            argp++;
        }
        if (smatch(argp, "chdir")) {
            if (argind >= argc) {
                usageError();
            } else {
                argp = argv[++argind];
                if (chdir((char*) argp) < 0) {
                    fail("Can't change directory to %s", argp);
                }
            }

        } else if (smatch(argp, "config")) {
            if (argind >= argc) {
                usageError();
            } else {
                path = argv[++argind];
                if (chdir((char*) mprGetPathDir(path)) < 0) {
                    mprError("Can't change directory to %s", mprGetPathDir(path));
                }
                app->configFile = mprGetPathBase(path);
            }

        } else if (smatch(argp, "database")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->database = sclone(argv[++argind]);
                if (!smatch(app->database, "mdb") && !smatch(app->database, "sdb")) {
                    fail("Unknown database \"%s\"", app->database);
                    usageError();
                }
            }

        } else if (smatch(argp, "flat") || smatch(argp, "f")) {
            app->flat = 1;

        } else if (smatch(argp, "listen") || smatch(argp, "l")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->listen = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "log") || smatch(argp, "l")) {
            if (argind >= argc) {
                usageError();
            } else {
                logSpec = argv[++argind];
            }

        } else if (smatch(argp, "min")) {
            app->minified = 1;

        } else if (smatch(argp, "overwrite")) {
            app->overwrite = 1;

        } else if (smatch(argp, "platform")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->platform = slower(argv[++argind]);
            }

        } else if (smatch(argp, "quiet") || smatch(argp, "q")) {
            app->quiet = 1;

        } else if (smatch(argp, "routeName")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->routeName = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "routePrefix")) {
            if (argind >= argc) {
                usageError();
            } else {
                app->routePrefix = sclone(argv[++argind]);
            }

        } else if (smatch(argp, "verbose") || smatch(argp, "v")) {
            logSpec = "stderr:2";

        } else if (smatch(argp, "version") || smatch(argp, "V")) {
            mprPrintf("%s %s-%s\n", mprGetAppTitle(), BIT_VERSION, BIT_NUMBER);
            exit(0);

        } else {
            fail("Unknown switch \"%s\"", argp);
            usageError();
        }
    }
    mprStartLogging(logSpec, 0);
    mprSetCmdlineLogging(1);

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
        mprMark(app->database);
        mprMark(app->files);
        mprMark(app->flatFile);
        mprMark(app->flatItems);
        mprMark(app->flatPath);
        mprMark(app->libDir);
        mprMark(app->listen);
        mprMark(app->module);
        mprMark(app->mpr);
        mprMark(app->platform);
        mprMark(app->routes);
        mprMark(app->routeName);
        mprMark(app->routePrefix);
        mprMark(app->server);
        mprMark(app->serverRoot);
        mprMark(app->targets);
        mprMark(app->wwwDir);
    }
}


static HttpRoute *createRoute(cchar *dir)
{
    HttpRoute   *route;
    EspRoute    *eroute;
    
    if ((route = httpCreateRoute(NULL)) == 0) {
        return 0;
    }
    if ((eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return 0;
    }
    route->eroute = eroute;
    httpSetRouteDir(route, dir);
    eroute->cacheDir = mprJoinPath(dir, "cache");
    eroute->controllersDir = mprJoinPath(dir, "controllers");
    eroute->dbDir = mprJoinPath(dir, "db");
    eroute->layoutsDir = mprJoinPath(dir, "layouts");
    eroute->staticDir = mprJoinPath(dir, "static");
    eroute->viewsDir = mprJoinPath(dir, "views");
    return route;
}


static void initialize()
{
    readConfig();
    app->currentDir = mprGetCurrentPath();
    app->libDir = mprGetAppDir();
    app->wwwDir = mprJoinPath(app->libDir, "esp-www");
}


static MprHash *getTargets(int argc, char **argv)
{
    MprHash     *targets;
    int         i;

    targets = mprCreateHash(0, 0);
    for (i = 0; i < argc; i++) {
        mprAddKey(targets, mprGetAbsPath(argv[i]), NULL);
    }
    return targets;
}


static MprList *getRoutes()
{
    HttpHost    *host;
    HttpRoute   *route, *rp, *parent;
    EspRoute    *eroute;
    MprList     *routes;
    MprKey      *kp;
    cchar       *routeName, *routePrefix;
    int         prev, nextRoute;

    if ((host = mprGetFirstItem(http->hosts)) == 0) {
        fail("Can't find default host");
        return 0;
    }
    routeName = app->routeName;
    routePrefix = app->routePrefix ? app->routePrefix : 0;
    routes = mprCreateList(0, 0);

    /*
        Filter ESP routes. Go in reverse order to locate outermost routes first.
     */
    for (prev = -1; (route = mprGetPrevItem(host->routes, &prev)) != 0; ) {
        mprLog(3, "Check route name %s, prefix %s", route->name, route->startWith);

        if ((eroute = route->eroute) == 0 || !eroute->compile) {
            /* No ESP configuration for compiling */
            continue;
        }
        if (routeName) {
            if (!smatch(routeName, route->name)) {
                continue;
            }
        } else if (routePrefix) {
            if (route->startWith == 0 || !smatch(routePrefix, route->startWith)) {
                continue;
            }
        } 
        parent = route->parent;
        if (parent && ((EspRoute*) parent->eroute)->compile && smatch(route->dir, parent->dir) && parent->startWith) {
            /* 
                Use the parent instead if it has the same directory and is not the default route 
                This is for MVC apps with a prefix of "/" and a directory the same as the default route.
             */
            continue;
        }
        if (!validTarget(route->dir)) {
            continue;
        }
        /*
            Check for routes of the same directory or of a direct parent directory
         */
        rp = 0;
        for (ITERATE_ITEMS(routes, rp, nextRoute)) {
            if (sstarts(route->dir, rp->dir)) {
                if (!rp->startWith && route->sourceName) {
                    /* Replace the default route with this route. This is for MVC Apps with prefix of "/" */
                    mprRemoveItem(routes, rp);
                    rp = 0;
                }
                break;
            }
        }
        if (rp) {
            continue;
        }
        if (mprLookupItem(routes, route) < 0) {
            // mprLog(1, "Compiling route dir: %-40s name: %-16s prefix: %-16s", route->dir, route->name, route->startWith);
            mprLog(2, "Compiling route dir: %s name: %s prefix: %s", route->dir, route->name, route->startWith);
            mprAddItem(routes, route);
        }
    }
    if (mprGetListLength(routes) == 0) {
        if (routeName) {
            fail("Can't find usable ESP configuration in %s for route %s", app->configFile, routeName);
        } else if (routePrefix) {
            fail("Can't find usable ESP configuration in %s for route prefix %s", app->configFile, routePrefix);
        } else {
            fail("Can't find usable ESP configuration in %s", app->configFile);
        }
        return 0;
    }
    /*
        Check we have a route for all targets
     */
    for (ITERATE_KEYS(app->targets, kp)) {
        if (!kp->type) {
            fail("Can't find a usable route for %s", kp->key);
        }
    }
    return routes;
}


static HttpRoute *getMvcRoute()
{
    HttpHost    *host;
    HttpRoute   *route, *parent;
    EspRoute    *eroute;
    cchar       *routeName, *routePrefix;
    int         prev;

    if ((host = mprGetFirstItem(http->hosts)) == 0) {
        fail("Can't find default host");
        return 0;
    }
    routeName = app->routeName;
    routePrefix = app->routePrefix ? app->routePrefix : 0;

    /*
        Filter ESP routes and find the ...
        Go in reverse order to locate outermost routes first.
     */
    for (prev = -1; (route = mprGetPrevItem(host->routes, &prev)) != 0; ) {
        mprLog(3, "Check route name %s, prefix %s", route->name, route->startWith);
        if ((eroute = route->eroute) == 0 || !eroute->compile) {
            /* No ESP configuration for compiling */
            continue;
        }
        if (!route->startWith) {
            continue;
        }
        if (routeName) {
            if (!smatch(routeName, route->name)) {
                continue;
            }
        } else if (routePrefix) {
            if (route->startWith == 0 || !smatch(routePrefix, route->startWith)) {
                continue;
            }
        } 
        parent = route->parent;
        if (parent && ((EspRoute*) parent->eroute)->compile && smatch(route->dir, parent->dir) && parent->startWith) {
            /* 
                Use the parent instead if it has the same directory and is not the default route 
                This is for MVC apps with a prefix of "/" and a directory the same as the default route.
             */
            continue;
        }
        if (!validTarget(route->dir)) {
            continue;
        }
        break;
    }
    if (route == 0) {
        if (routeName) {
            fail("Can't find ESP configuration in %s for route %s", app->configFile, routeName);
        } else {
            fail("Can't find ESP configuration in %s for route prefix %s", app->configFile, routePrefix);
        }
    } else {
        mprLog(1, "Using route dir: %s, name: %s, prefix: %s", route->dir, route->name, route->startWith);
    }
    return route;
}


static void readConfig()
{
    HttpStage   *stage;

    if ((app->appweb = maCreateAppweb()) == 0) {
        fail("Can't create HTTP service for %s", mprGetAppName());
        return;
    }
    appweb = MPR->appwebService = app->appweb;
    appweb->skipModules = 1;
    http = app->appweb->http;
    if (app->platform) {
        appweb->platform = app->platform;
    }
    findConfigFile();
    if (app->error) {
        return;
    }
    if ((app->server = maCreateServer(appweb, "default")) == 0) {
        fail("Can't create HTTP server for %s", mprGetAppName());
        return;
    }
    if (app->platform) {
        if (maSetPlatform(app->platform) < 0) {
            fail("Can't find platform %s", app->platform);
            return;
        }
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
        app->targets = getTargets(argc - 1, &argv[1]);
        app->routes = getRoutes();
        clean(app->routes);

    } else if (smatch(cmd, "compile")) {
        app->targets = getTargets(argc - 1, &argv[1]);
        app->routes = getRoutes();
        compile(app->routes);

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


static void clean(MprList *routes)
{
    MprList         *files;
    MprDirEntry     *dp;
    HttpRoute       *route;
    EspRoute        *eroute;
    char            *path;
    int             next, nextFile;

    if (app->error) {
        return;
    }
    for (ITERATE_ITEMS(routes, route, next)) {
        eroute = route->eroute;
        if (eroute->cacheDir) {
            trace("CLEAN", "Route \"%s\" at %s", route->name, route->dir);
            files = mprGetPathFiles(eroute->cacheDir, 0);
            for (nextFile = 0; (dp = mprGetNextItem(files, &nextFile)) != 0; ) {
                path = mprJoinPath(eroute->cacheDir, dp->name);
                trace("CLEAN", "%s", path);
                mprDeletePath(path);
            }
        }
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
    if (mprRunCmd(cmd, "appweb -v", NULL, NULL, NULL, -1, MPR_CMD_DETACH) != 0) {
        fail("Can't run command: \n%s", app->command);
        return;
    }
    mprWaitForCmd(cmd, -1);
}


static int runEspCommand(HttpRoute *route, cchar *command, cchar *csource, cchar *module)
{
    EspRoute    *eroute;
    MprCmd      *cmd;
    MprList     *elist;
    MprKey      *var;
    cchar       **env;
    char        *err, *out;

    eroute = route->eroute;
    cmd = mprCreateCmd(0);
    if ((app->command = espExpandCommand(eroute, command, csource, module)) == 0) {
        fail("Missing EspCompile directive for %s", csource);
        return MPR_ERR_CANT_READ;
    }
    mprLog(4, "ESP command: %s\n", app->command);
    if (eroute->env) {
        elist = mprCreateList(0, 0);
        for (ITERATE_KEYS(eroute->env, var)) {
            mprAddItem(elist, sfmt("%s=%s", var->key, var->data));
        }
        mprAddNullItem(elist);
        env = (cchar**) &elist->items[0];
    } else {
        env = 0;
    }
    if (eroute->searchPath) {
        mprSetCmdSearchPath(cmd, eroute->searchPath);
    }
    mprLog(1, "Run: %s", app->command);

    //  WARNING: GC will run here
	if (mprRunCmd(cmd, app->command, env, &out, &err, -1, 0) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
        fail("Can't run command: \n%s\nError: %s", app->command, err);
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}


static void compileFile(HttpRoute *route, cchar *source, int kind)
{
    EspRoute    *eroute;
    cchar       *script, *page, *layout, *data;
    char        *err;
    ssize       len;

    if (app->error) {
        return;
    }
    eroute = route->eroute;
    layout = 0;
    app->cacheName = mprGetMD5WithPrefix(source, slen(source), (kind == ESP_CONTROLLER) ? "controller_" : "view_");
    app->module = mprNormalizePath(sfmt("%s/%s%s", eroute->cacheDir, app->cacheName, BIT_SHOBJ));

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
        layout = (eroute->layoutsDir) ? mprJoinPath(eroute->layoutsDir, "default.esp") : 0;
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
            app->csource = mprJoinPathExt(mprTrimPathExt(app->module), ".c");
            trace("PARSE", "%s", source);
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
        if (runEspCommand(route, eroute->compile, app->csource, app->module) < 0) {
            return;
        }
        if (eroute->link) {
            trace("LINK", "%s", app->module);
            if (runEspCommand(route, eroute->link, app->csource, app->module) < 0) {
                return;
            }
#if !(BIT_DEBUG && MACOSX)
            /*
                MAC needs the object for debug information
             */
            mprDeletePath(mprJoinPathExt(mprTrimPathExt(app->module), BIT_OBJ));
#endif
        }
        if (!eroute->keepSource && (kind & (ESP_VIEW | ESP_PAGE))) {
            mprDeletePath(app->csource);
        }
    }
}


/*
    esp compile [flat | controller_names | view_names]
        [] - compile controllers and views separately into cache
        [controller_names/view_names] - compile single file
        [app] - compile all files into a single source file with one init that calls all sub-init files

    esp compile path/name.ejs ...
        [controller_names/view_names] - compile single file

    esp compile static
        use makerom code and compile static into a file in cache

 */
static void compile(MprList *routes)
{
    HttpRoute   *route;
    MprKey      *kp;
    int         next;
 
    if (app->error) {
        return;
    }
    for (ITERATE_KEYS(app->targets, kp)) {
        kp->type = 0;
    }
    for (ITERATE_ITEMS(routes, route, next)) {
        mprLog(2, "Build with route \"%s\" at %s", route->name, route->dir);
        if (app->flat) {
            compileFlat(route);
        } else {
            compileItems(route);
        }
    }
    /*
        Check we have compiled all targets
     */
    for (ITERATE_KEYS(app->targets, kp)) {
        if (!kp->type) {
            fail("Can't find target %s to compile", kp->key);
        }
    }
    if (!app->error) {
        trace("TASK", "Complete");
    }
}


/* 
    Allow a target that is a parent directory of a route directory or
    Allow a target that is contained within a route directory 
 */
static bool validTarget(cchar *target)
{
    MprKey  *kp;
    cchar   *tp;

    if (app->targets == 0 || mprGetHashLength(app->targets) == 0) {
        return 1;
    }
    for (ITERATE_KEYS(app->targets, kp)) {
        if (sstarts(target, kp->key) || sstarts(kp->key, target)) {
            kp->type = ESP_FOUND_TARGET;
            return 1;
        }
    }
    return 0;
}


static void compileItems(HttpRoute *route) 
{
    EspRoute    *eroute;
    MprDirEntry *dp;
    cchar       *path;
    int         next;

    eroute = route->eroute;

    if (eroute->controllersDir) {
        mprAssert(eroute);
        app->files = mprGetPathFiles(eroute->controllersDir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (!validTarget(path)) {
                continue;
            }
            if (smatch(mprGetPathExt(path), "c")) {
                compileFile(route, path, ESP_CONTROLLER);
            }
        }
    }
    if (eroute->viewsDir) {
        app->files = mprGetPathFiles(eroute->viewsDir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (!validTarget(path)) {
                continue;
            }
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(route, path, ESP_VIEW);
            }
        }
    }
    if (eroute->staticDir) {
        app->files = mprGetPathFiles(eroute->staticDir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (!validTarget(path)) {
                continue;
            }
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(route, path, ESP_PAGE);
            }
        }

    } else {
        /* Non-MVC */
        app->files = mprGetPathFiles(route->dir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (!validTarget(path)) {
                continue;
            }
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(route, path, ESP_PAGE);
            }
        }
    }
}


static void compileFlat(HttpRoute *route)
{
    EspRoute    *eroute;
    MprDirEntry *dp;
    char        *path, *line;
    int         next;
    
    eroute = route->eroute;
    
    /*
        Flat ... Catenate all source
     */
    app->flatItems = mprCreateList(-1, 0);
    app->flatPath = path = mprJoinPath(eroute->cacheDir, "app.c");
    if ((app->flatFile = mprOpenFile(path, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
        fail("Can't open %s", path);
        return;
    }
    mprWriteFileFmt(app->flatFile, "/*\n    Flat compilation of %s\n */\n\n", app->appName);

    if (route->sourceName) {
        /* MVC */
        app->files = mprGetPathFiles(eroute->controllersDir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (smatch(mprGetPathExt(path), "c")) {
                compileFile(route, path, ESP_CONTROLLER);
            }
        }
        app->files = mprGetPathFiles(eroute->viewsDir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            compileFile(route, path, ESP_VIEW);
        }
        app->files = mprGetPathFiles(eroute->staticDir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(route, path, ESP_PAGE);
            }
        }
    } else {
        app->files = mprGetPathFiles(route->dir, MPR_PATH_DESCEND);
        for (next = 0; (dp = mprGetNextItem(app->files, &next)) != 0 && !app->error; ) {
            path = dp->name;
            if (smatch(mprGetPathExt(path), "esp")) {
                compileFile(route, path, ESP_PAGE);
            }
        }
    }
    mprWriteFileFmt(app->flatFile, "\nESP_EXPORT int esp_app_%s(EspRoute *el, MprModule *module) {\n", app->appName);
    for (next = 0; (line = mprGetNextItem(app->flatItems, &next)) != 0; ) {
        mprWriteFileFmt(app->flatFile, "    %s(el, module);\n", line);
    }
    mprWriteFileFmt(app->flatFile, "    return 0;\n}\n");
    mprCloseFile(app->flatFile);

    app->module = mprNormalizePath(sfmt("%s/app%s", eroute->cacheDir, BIT_SHOBJ));
    trace("COMPILE", "%s", app->csource);
    if (runEspCommand(route, eroute->compile, app->flatPath, app->module) < 0) {
        return;
    }
    if (eroute->link) {
        trace("LINK", "%s", app->module);
        if (runEspCommand(route, eroute->link, app->flatPath, app->module) < 0) {
            return;
        }
    }
    app->flatItems = 0;
    app->flatFile = 0;
    app->flatPath = 0;
}

    
static void generateApp(HttpRoute *route, cchar *name)
{
    app->appName = sclone(name);
    generateAppDirs(route);
    generateAppFiles(route);
    generateAppConfigFile(route);
    generateAppHeader(route);
    generateAppDb(route);
}


/*
    esp generate controller name [action [, action] ...]
 */
static void generateController(HttpRoute *route, int argc, char **argv)
{
    EspRoute    *eroute;
    MprHash     *tokens;
    cchar       *defines, *name, *path, *data, *title, *action;
    int         i;

    if (argc < 1) {
        usageError();
        return;
    }
    if (app->error) {
        return;
    }
    eroute = route->eroute;
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
    makeEspFile(path, data, "Controller");
}


static void generateScaffoldController(HttpRoute *route, int argc, char **argv)
{
    EspRoute    *eroute;
    MprHash     *tokens, *actions;
    cchar       *defines, *name, *path, *data, *title, *action;
    int         i;

    if (app->error) {
        return;
    }
    eroute = route->eroute;
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
    makeEspFile(path, data, "Scaffold");
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

    if (app->error) {
        return;
    }
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
        flags = 0;
        def = sfmt("    ediAddColumn(db, \"%s\", \"%s\", %d, %d);\n", name, field, type, flags);
        forward = sjoin(forward, def, NULL);
    }
    dir = mprJoinPath(eroute->dbDir, "migrations");
    makeEspDir(route, dir);

    fileComment = sreplace(comment, " ", "_");
    path = sfmt("%s/%s_%s.c", eroute->migrationsDir, seq, fileComment, ".c");

    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, COMMENT: '%s', FORWARD: '%s', BACKWARD: '%s' }", 
        name, title, comment, forward, backward));
    data = stemplate(MigrationTemplate, tokens);
    makeEspFile(path, data, "Migration");
}
#endif


/*
    esp generate table name [field:type [, field:type] ...]
 */
static void generateTable(HttpRoute *route, int argc, char **argv)
{
    EspRoute    *eroute;
    Edi         *edi;
    cchar       *tableName, *field;
    char        *typeString;
    int         rc, i, type;

    if (app->error) {
        return;
    }
    eroute = route->eroute;
    tableName = sclone(argv[0]);
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


static void generateScaffoldViews(HttpRoute *route, int argc, char **argv)
{
    EspRoute    *eroute;
    MprHash     *tokens;
    cchar       *title, *name, *path, *data;

    if (app->error) {
        return;
    }
    eroute = route->eroute;
    name = sclone(argv[0]);
    title = spascal(name);

    path = sfmt("%s/%s-list.esp", eroute->viewsDir, name);
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, }", name, title));
    data = stemplate(ScaffoldListView, tokens);
    makeEspFile(path, data, "Scaffold Index View");

    path = sfmt("%s/%s-edit.esp", eroute->viewsDir, name);
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s, }", name, title));
    data = stemplate(ScaffoldEditView, tokens);
    makeEspFile(path, data, "Scaffold Index View");
}

/*
    esp generate scaffold NAME [field:type [, field:type] ...]
 */
static void generateScaffold(HttpRoute *route, int argc, char **argv)
{
    if (argc < 1) {
        usageError();
        return;
    }
    if (app->error) {
        return;
    }
    generateScaffoldController(route, 1, argv);
    generateScaffoldViews(route, argc, argv);
    generateTable(route, argc, argv);
}


static void generate(int argc, char **argv)
{
    HttpRoute   *route;
    char        *name, *kind;

    if (argc < 2) {
        usageError();
        return;
    }
    kind = argv[0];

    if (smatch(kind, "app")) {
        name = argv[1];
        generateApp(createRoute(name), name);

    } else if (smatch(kind, "controller")) {
        route = getMvcRoute();
        generateController(route, argc - 1, &argv[1]);

    } else if (smatch(kind, "scaffold")) {
        route = getMvcRoute();
        generateScaffold(route, argc - 1, &argv[1]);

#if UNUSED && KEEP
    } else if (smatch(kind, "migration")) {
        readConfig();
        route = getMvcRoute();
        generateMigration(route, argc - 1, argv[1]);

#endif
    } else if (smatch(kind, "table")) {
        route = getMvcRoute();
        generateTable(route, argc - 1, &argv[1]);

    } else {
        mprError("Unknown generation kind %s", kind);
        usageError();
        return;
    }
    if (!app->error) {
        trace("TASK", "Complete");
    }
}


static void generateAppDirs(HttpRoute *route)
{
    makeEspDir(route, "");
    makeEspDir(route, "cache");
    makeEspDir(route, "controllers");
    makeEspDir(route, "db");
    makeEspDir(route, "layouts");
    makeEspDir(route, "static");
    makeEspDir(route, "static/images");
    makeEspDir(route, "static/js");
    makeEspDir(route, "static/themes");
    makeEspDir(route, "views");
}


static void fixupFile(HttpRoute *route, cchar *path)
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


static void generateAppFiles(HttpRoute *route)
{
    copyEspDir(mprJoinPath(app->wwwDir, "files"), route->dir);
    fixupFile(route, "layouts/default.esp");
}


static bool checkEspPath(cchar *path)
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



static void copyEspDir(cchar *fromDir, cchar *toDir)
{
    MprList     *files;
    MprDirEntry *dp;
    char        *from, *to;
    int         next;

    files = mprGetPathFiles(fromDir, MPR_PATH_DESCEND | MPR_PATH_RELATIVE | MPR_PATH_NODIRS);
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0 && !app->error; ) {
        if (!checkEspPath(dp->name)) {
            continue;
        }
        from = mprJoinPath(fromDir, dp->name);
        to = mprJoinPath(toDir, dp->name);
#if UNUSED
        if (dp->isDir) {
            if (!mprPathExists(to, R_OK)) {
                if (mprMakeDir(to, 0755, -1, -1, 1) < 0) {
                    fail("Can't make directory %s", to);
                    return;
                }
                trace("CREATE",  "Directory: %s", mprGetRelPath(to, 0));
            }
            copyEspDir(from, to);
        } else {
#endif
        if (mprMakeDir(mprGetPathDir(to), 0755, -1, -1, 1) < 0) {
            fail("Can't make directory %s", mprGetPathDir(to));
            return;
        }
        if (mprPathExists(to, R_OK) && !app->overwrite) {
            trace("EXISTS",  "File: %s", mprGetRelPath(to, 0));
        } else {
            trace("CREATE",  "File: %s", mprGetRelPath(to, 0));
            if (mprCopyPath(from, to, 0644) < 0) {
                fail("Can't copy file %s to %s", from, mprGetRelPath(to, 0));
                return;
            }
        }
    }
}


#if UNUSED && KEEP
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
    fixupFile(route, to);
    trace("CREATE",  "Config file: %s", to);
}
#endif


static void copyFile(HttpRoute *route, cchar *from, cchar *to)
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
    fixupFile(route, mprGetAbsPath(to));
    trace("CREATE",  "File: %s", mprGetRelPath(to, 0));
}


static void generateAppConfigFile(HttpRoute *route)
{
    char    *from, *to;

    from = mprJoinPath(app->wwwDir, "appweb.conf");
    to = mprJoinPath(route->dir, "appweb.conf");
    copyFile(route, from, to);

    from = mprJoinPath(app->wwwDir, "app.conf");
    to = mprJoinPath(route->dir, "app.conf");
    copyFile(route, from, to);
}


static void generateAppHeader(HttpRoute *route)
{
    MprHash *tokens;
    char    *path, *data;

    path = mprJoinPath(route->dir, mprJoinPathExt("esp-app", "h"));
    tokens = mprDeserialize(sfmt("{ NAME: %s, TITLE: %s }", app->appName, spascal(app->appName)));
    data = stemplate(AppHeader, tokens);
    makeEspFile(path, data, "Header");
}


static void generateAppDb(HttpRoute *route)
{
    EspRoute    *eroute;
    char        *ext, *dbpath, buf[1];

    eroute = route->eroute;
    ext = smatch(app->database, "mdb") ? "mdb" : "sdb";
    dbpath = sfmt("%s/%s.%s", eroute->dbDir, app->appName, ext);
    if (mprWritePathContents(dbpath, buf, 0, 0664) < 0) {
        return;
    }
    trace("CREATE", "Database: %s", mprGetRelPath(dbpath, 0));
}


/*
    Search strategy is:

    [--config dir] : ./appweb.conf : /usr/lib/appweb/VER/bin/esp-appweb.conf
 */
static void findConfigFile()
{
    cchar   *userPath;

    userPath = app->configFile;
    if (app->configFile == 0) {
        app->configFile = mprJoinPathExt(BIT_PRODUCT, ".conf");
    }
    if (!mprPathExists(app->configFile, R_OK)) {
        if (userPath) {
            fail("Can't open config file %s", app->configFile);
            return;
        }
        app->configFile = mprJoinPath(mprGetAppDir(), sfmt("esp-%s.conf", BIT_PRODUCT));
        if (!mprPathExists(app->configFile, R_OK)) {
            fail("Can't open config file %s", app->configFile);
            return;
        }
    }
    app->serverRoot = mprGetAbsPath(mprGetPathDir(app->configFile));
}


static void makeEspDir(HttpRoute *route, cchar *dir)
{
    char    *path;

    path = mprJoinPath(route->dir, dir);
    if (mprPathExists(path, X_OK)) {
        trace("EXISTS",  "Directory: %s", path);
    } else {
        if (mprMakeDir(path, 0755, -1, -1, 1) < 0) {
            app->error++;
        } else {
            trace("CREATE",  "Directory: %s", mprGetRelPath(path, 0));
        }
    }
}


static void makeEspFile(cchar *path, cchar *data, cchar *msg)
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
        trace("CREATE", mprGetRelPath(path, 0));
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
    "    --chdir dir                # Change to the named directory first\n"
    "    --config configFile        # Use named config file instead appweb.conf\n"
    "    --database name            # Database provider 'mdb|sqlite' \n"
    "    --flat                     # Compile into a single module\n"
    "    --listen [ip:]port         # Listen on specified address \n"
    "    --log logFile:level        # Log to file file at verbosity level\n"
    "    --overwrite                # Overwrite existing files \n"
    "    --quiet                    # Don't emit trace \n"
    "    --platform os-arch-profile # Target platform\n"
    "    --routeName name           # Route name in appweb.conf to use \n"
    "    --routePrefix prefix       # Route prefix in appweb.conf to use \n"
    "    --verbose                  # Emit verbose trace \n"
    "\n"
    "  Commands:\n"
    "    esp clean\n"
    "    esp compile\n"
    "    esp compile [pathFilters ...]\n"
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
    app->error = 1;
}


static void fail(cchar *fmt, ...)
{
    va_list     args;
    char        *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    mprError("%s", msg);
    va_end(args);
    app->error = 1;
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

#endif /* BIT_FEATURE_ESP */

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
