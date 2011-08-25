/*
    espHandler.c -- Embedded Server Pages (ESP) handler. Fast in-process replacement for CGI.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
#include    "esp.h"

/************************************* Local **********************************/
/*
    Singleton ESP control structure
 */
static Esp *esp;

/************************************ Forward *********************************/

static void addExtRoute(HttpRoute *parent);
static void addDefaultRoutes(HttpRoute *parent);
static void addRoutePack(HttpRoute *parent, cchar *pack, cchar *prefix, cchar *controller);
static EspRoute *allocEspRoute(HttpRoute *loc);
static bool fetchCachedResponse(HttpConn *conn);
static EspRoute *getEspRoute(HttpRoute *route);
static void manageAction(EspAction *cp, int flags);
static void manageEsp(Esp *esp, int flags);
static void manageReq(EspReq *req, int flags);
static int  runAction(HttpConn *conn);
static void runView(HttpConn *conn);
static void saveCachedResponse(HttpConn *conn);
static bool moduleIsStale(HttpConn *conn, cchar *source, cchar *module, int *recompile);
static void setRouteDirs(MaState *state, cchar *kind);
static bool unloadModule(cchar *module);

/************************************* Code ***********************************/

static void openEsp(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    EspReq      *req;

    conn = q->conn;
    rx = conn->rx;

    if (rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        httpHandleOptionsTrace(q);

    } else {
        if ((req = mprAllocObj(EspReq, manageReq)) == 0) {
            httpMemoryError(conn);
            return;
        }
        for (route = rx->route; route; route = route->parent) {
            if ((eroute = httpGetRouteData(route, ESP_NAME)) != 0) {
                break;
            }
        }
        if (eroute == 0) {
            eroute = allocEspRoute(route);
            return;
        }
        conn->data = req;
        req->esp = esp;
        req->eroute = eroute;
        req->autoFinalize = 1;
    }
    lock(esp);
    esp->inUse++;
    unlock(esp);
}


static void closeEsp(HttpQueue *q)
{
    lock(esp);
    esp->inUse--;
    mprAssert(esp->inUse >= 0);
    unlock(esp);
}


static void startEsp(HttpQueue *q)
{
    HttpConn    *conn;
    EspReq      *req;

    conn = q->conn;
    req = conn->data;

    if (req) {
        httpAddFormVars(conn);
        
        if (!runAction(conn)) {
            return;
        }
        if (req->autoFinalize) {
            if (!conn->tx->responded) {
                runView(conn);
            }
            espFinalize(conn);
        }
        saveCachedResponse(conn);
    }
}


static char *makeCacheKey(HttpConn *conn, cchar *targetKey, cchar *uri)
{
    EspReq      *req;
    EspRoute    *eroute;
    HttpQueue   *q;
    char        *path, *form, *key;

    req = conn->data;
    eroute = req->eroute;
    q = conn->readq;

    path = mprJoinPath(eroute->controllersDir, targetKey);
    if (uri) {
        form = httpGetFormData(conn);
        key = sfmt("content-%s:%s?%s", eroute->controllersDir, uri, form);
        
    } else {
        key = sfmt("content-%s", mprJoinPath(eroute->controllersDir, targetKey));
    }
    return key;
}


static bool fetchCachedResponse(HttpConn *conn)
{
    HttpRx      *rx;
    EspReq      *req;
    MprTime     modified, when;
    cchar       *value, *extraUri, *content, *key, *tag;
    int         status, cacheOk, canUseClientCache;

    req = conn->data;
    rx = conn->rx;

    if (req->action && req->action->lifespan) {
        extraUri = req->action ? req->action->uri : 0;
        if (extraUri && scmp(extraUri, "*") == 0) {
            extraUri = conn->rx->pathInfo;
        }
        key = makeCacheKey(conn, rx->targetKey, extraUri);
        tag = mprGetMD5Hash(key, slen(key), 0);

        if ((value = httpGetHeader(conn, "Cache-Control")) != 0 && 
                (scontains(value, "max-age=0", -1) == 0 || scontains(value, "no-cache", -1) == 0)) {
            mprLog(5, "Cache-control header rejects use of cached content");

        } else if ((content = mprReadCache(esp->cache, key, &modified, 0)) != 0) {
            /*
                Observe headers:
                    If-None-Match: "ec18d-54-4d706a63"
                    If-Modified-Since: Fri, 04 Mar 2011 04:28:19 GMT
                Set status to OK when content must be transmitted.
             */
            cacheOk = 1;
            canUseClientCache = 0;
            if ((value = httpGetHeader(conn, "If-None-Match")) != 0) {
                canUseClientCache = 1;
                if (scmp(value, tag) != 0) {
                    cacheOk = 0;
                }
            }
            if (cacheOk && (value = httpGetHeader(conn, "If-Modified-Since")) != 0) {
                canUseClientCache = 1;
                mprParseTime(&when, value, 0, 0);
                if (modified > when) {
                    cacheOk = 0;
                }
            }
            status = (canUseClientCache && cacheOk) ? HTTP_CODE_NOT_MODIFIED : HTTP_CODE_OK;
            mprLog(5, "Use cached content for %s, status %d", key, status);
            httpSetStatus(conn, status);
            httpSetHeader(conn, "Etag", mprGetMD5Hash(key, slen(key), 0));
            httpSetHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
            if (status == HTTP_CODE_OK) {
                espWriteString(conn, content);
            }
            return 1;
        } else {
            mprLog(5, "No cached content for %s", key);
        }
    }
    return 0;
}


static void saveCachedResponse(HttpConn *conn)
{
    HttpRx      *rx;
    EspReq      *req;
    EspAction   *action;
    MprBuf      *buf;
    MprTime     modified;
    char        *key, *extraUri;

    req = conn->data;
    rx = conn->rx;

    if (req->finalized && req->cacheBuffer) {
        buf = req->cacheBuffer;
        req->cacheBuffer = 0;
        mprAddNullToBuf(buf);
        action = req->action;
        extraUri = req->action->uri;
        if (action->uri && scmp(action->uri, "*") == 0) {
            extraUri = conn->rx->pathInfo;
        }
        key = makeCacheKey(conn, rx->targetKey, extraUri);
        /*
            Truncate modified to get a 1 sec resolution. This is the resolution for If-Modified headers
         */
        modified = mprGetTime() / MPR_TICKS_PER_SEC * MPR_TICKS_PER_SEC;
        mprWriteCache(esp->cache, key, mprGetBufStart(buf), modified, action->lifespan, 0, 0);
        httpAddHeader(conn, "Etag", mprGetMD5Hash(key, slen(key), 0));
        httpAddHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
        espWriteBlock(conn, mprGetBufStart(buf), mprGetBufLength(buf));
        espFinalize(conn);
    }
}


void espUpdateCache(HttpConn *conn, cchar *targetKey, cchar *data, int lifesecs, cchar *uri)
{
    mprWriteCache(esp->cache, makeCacheKey(conn, targetKey, uri), data, 0, lifesecs * MPR_TICKS_PER_SEC, 0, 0);
}


bool espWriteCached(HttpConn *conn, cchar *targetKey, cchar *uri)
{
    EspReq      *req;
    MprTime     modified;
    cchar       *key, *content;

    req = conn->data;
    key = makeCacheKey(conn, targetKey, uri);
    if ((content = mprReadCache(esp->cache, key, &modified, 0)) == 0) {
        mprLog(5, "No cached data for ", key);
        return 0;
    }
    mprLog(5, "Used cached ", key);
    req->cacheBuffer = 0;
    espSetHeader(conn, "Etag", mprGetMD5Hash(key, slen(key), 0));
    espSetHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
    espWriteString(conn, content);
    espFinalize(conn);
    return 1;
}


static char *getControllerEntry(cchar *controllerName)
{
    char    *cp, *entry;

    entry = sfmt("espInit_controller_%s", mprTrimPathExt(mprGetPathBase(controllerName)));
    for (cp = entry; *cp; cp++) {
        if (!isalnum((int) *cp) && *cp != '_') {
            *cp = '_';
        }
    }
    return entry;
}


/*
    Load the (flat) app module. If modified, unload and reload if required.
 */
static int loadApp(HttpConn *conn, int *updated)
{
    MprModule   *mp;
    EspRoute    *eroute;
    EspReq      *req;
    MprPath     minfo;
    char        *entry;

    req = conn->data;
    if (req->appLoaded) {
        return 1;
    }
    *updated = 0;
    eroute = req->eroute;
    if ((mp = mprLookupModule(eroute->appModuleName)) != 0) {
        if (eroute->update) {
            mprGetPathInfo(mp->path, &minfo);
            if (minfo.valid && mp->modified < minfo.mtime) {
                unloadModule(eroute->appModuleName);
                mp = 0;
            }
        }
    }
    if (mp == 0) {
        entry = sfmt("espInit_app_%s", mprGetPathBase(eroute->dir));
        if ((mp = mprCreateModule(eroute->appModuleName, eroute->appModulePath, entry, eroute)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find module %s", eroute->appModulePath);
            return 0;
        }
        if (mprLoadModule(mp) < 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load compiled esp module for %s", eroute->appModuleName);
            return 0;
        }
        *updated = 1;
    }
    req->appLoaded = 1;
    return 1;
}


static int runAction(HttpConn *conn)
{
    MprModule   *mp;
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    EspReq      *req;
    EspAction   *action;
    char        *key;
    int         updated, recompile;

    rx = conn->rx;
    req = conn->data;
    route = rx->route;
    eroute = req->eroute;
    mprAssert(eroute);
    updated = 0;
    recompile = 0;

    if (rx->route->sourceName == 0) {
        return 1;
    }
    /*
        Expand any form var $tokens. This permits ${controller} and user form data to be used in the controller name
     */
    if (schr(route->sourceName, '$')) {
        req->controllerName = stemplate(route->sourceName, conn->rx->formVars);
    } else {
        req->controllerName = route->sourceName;
    }
    req->controllerPath = mprJoinPath(eroute->controllersDir, req->controllerName);

    if (eroute->appModuleName) {
        if (!loadApp(conn, &updated)) {
            return 0;
        }
    } else if (eroute->update || !mprLookupModule(req->controllerPath)) {
        req->cacheName = mprGetMD5Hash(req->controllerPath, slen(req->controllerPath), "controller_");
        req->module = mprGetNormalizedPath(sfmt("%s/%s%s", eroute->cacheDir, req->cacheName, BLD_SHOBJ));

        if (!mprPathExists(req->controllerPath, R_OK)) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find controller %s", req->controllerPath);
            return 0;
        }
        if (moduleIsStale(conn, req->controllerPath, req->module, &recompile)) {
            /*  WARNING: GC yield here */
            if (recompile && !espCompile(conn, req->controllerPath, req->module, req->cacheName, 0)) {
                return 0;
            }
        }
        if (mprLookupModule(req->controllerPath) == 0) {
            req->entry = getControllerEntry(req->controllerName);
            if ((mp = mprCreateModule(req->controllerPath, req->module, req->entry, eroute)) == 0) {
                httpMemoryError(conn);
                return 0;
            }
            if (mprLoadModule(mp) < 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                    "Can't load compiled esp module for %s", req->controllerPath);
                return 0;
            }
            updated = 1;
        }
    }
    key = mprJoinPath(eroute->controllersDir, conn->rx->targetKey);
    if ((action = mprLookupKey(esp->actions, key)) == 0) {
        req->controllerPath = mprJoinPath(eroute->controllersDir, req->controllerName);
        key = sfmt("%s/%s-missing", req->controllerPath, mprTrimPathExt(req->controllerName));
        if ((action = mprLookupKey(esp->actions, key)) == 0) {
            if ((action = mprLookupKey(esp->actions, "missing")) == 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing action for %s", conn->rx->targetKey);
                return 0;
            }
        }
    }
    req->action = action;
    
    if (eroute->lifespan) {
        /* Must stabilize form data prior to controllers injecting variables */
        httpGetFormData(conn);
        if (!updated && fetchCachedResponse(conn)) {
            return 1;
        }
        req->cacheBuffer = mprCreateBuf(-1, -1);
    }
    if (action->actionFn) {
        (action->actionFn)(conn);
        return 1;
    }
    return 0;
}


static void runView(HttpConn *conn)
{
    MprModule   *mp;
    HttpRx      *rx;
    EspRoute    *eroute;
    EspReq      *req;
    EspViewFn   view;
    int         recompile, updated;
    
    rx = conn->rx;
    req = conn->data;
    eroute = req->eroute;
    recompile = updated = 0;
    
    if (req->controllerName) {
        req->view = mprJoinPath(eroute->viewsDir, rx->targetKey);
        req->source = mprJoinPathExt(req->view, ".esp");
    } else {
        req->view = conn->tx->filename;
        req->source = req->view;
    }
    if (eroute->appModuleName) {
        if (!loadApp(conn, &updated)) {
            return;
        }
    } else if (eroute->update || !mprLookupModule(req->source)) {
        req->cacheName = mprGetMD5Hash(req->source, slen(req->source), "view_");
        req->module = mprGetNormalizedPath(sfmt("%s/%s%s", eroute->cacheDir, req->cacheName, BLD_SHOBJ));

        if (!mprPathExists(req->source, R_OK)) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find view %s", req->source);
            return;
        }
        if (moduleIsStale(conn, req->source, req->module, &recompile)) {
            /* WARNING: this will allow GC */
            if (recompile && !espCompile(conn, req->source, req->module, req->cacheName, 1)) {
                return;
            }
        }
        if (mprLookupModule(req->source) == 0) {
            req->entry = sfmt("espInit_%s", req->cacheName);
            //  MOB - who keeps reference to module?
            if ((mp = mprCreateModule(req->source, req->module, req->entry, eroute)) == 0) {
                httpMemoryError(conn);
                return;
            }
            //  MOB - this should return an error msg
            if (mprLoadModule(mp) < 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load compiled esp module for %s", req->source);
                return;
            }
        }
    }
    if ((view = mprLookupKey(esp->views, mprGetPortablePath(req->source))) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find defined view for %s", req->view);
        return;
    }
	httpAddHeaderString(conn, "Content-Type", "text/html");
    (view)(conn);
}


static bool moduleIsStale(HttpConn *conn, cchar *source, cchar *module, int *recompile)
{
    MprModule   *mp;
    MprPath     sinfo, minfo;

    *recompile = 0;
    mprGetPathInfo(module, &minfo);
    if (!minfo.valid) {
        *recompile = 1;
        return 1;
    }
    mprGetPathInfo(source, &sinfo);
    /*
        Use >= to ensure we reload. This may cause redundant reloads as mtime has a 1 sec granularity.
     */
    if (sinfo.valid && sinfo.mtime >= minfo.mtime) {
        *recompile = 1;
        if ((mp = mprLookupModule(source)) != 0) {
            unloadModule(source);
        }
        return 1;
    }
    if ((mp = mprLookupModule(source)) != 0) {
        if (minfo.mtime > mp->modified) {
            /* Module file has been updated */
            unloadModule(source);
            return 1;
        }
    }
    /* Loaded module is current */
    return 0;
}


static bool unloadModule(cchar *module)
{
    MprModule   *mp;
    MprTime     mark;

    if ((mp = mprLookupModule(module)) != 0) {
        mark = mprGetTime();
        do {
            lock(esp);
            /* Own request will count as 1 */
            if (esp->inUse <= 1) {
                mprUnloadModule(mp);
                unlock(esp);
                return 1;
            }
            unlock(esp);
            mprSleep(20);
        } while (mprGetRemainingTime(mark, ESP_UNLOAD_TIMEOUT));
    }
    return 0;
}


void espCacheControl(EspRoute *eroute, cchar *targetKey, int lifesecs, cchar *uri)
{
    EspAction  *action;
    
    if ((action = mprLookupKey(esp->actions, mprJoinPath(eroute->controllersDir, targetKey))) == 0) {
        if ((action = mprAllocObj(EspAction, manageAction)) == 0) {
            return;
        }
    }
    if (uri) {
        action->uri = sclone(uri);
    }
    if (lifesecs == 0) {
        action->lifespan = eroute->lifespan;
    } else {
        action->lifespan = lifesecs * MPR_TICKS_PER_SEC;
    }
}


void espDefineAction(EspRoute *eroute, cchar *targetKey, void *actionFn)
{
    EspAction   *action;

    mprAssert(eroute);
    mprAssert(targetKey && *targetKey);
    mprAssert(actionFn);

    if ((action = mprAllocObj(EspAction, manageAction)) == 0) {
        return;
    }
    action->actionFn = actionFn;
    mprAddKey(esp->actions, mprJoinPath(eroute->controllersDir, targetKey), action);
}


/*
    Path should be an app-relative path to the view file (relative-path.esp)
 */
void espDefineView(EspRoute *eroute, cchar *path, void *view)
{
    mprAssert(eroute);
    mprAssert(path && *path);
    mprAssert(view);

	path = mprGetPortablePath(mprJoinPath(eroute->dir, path));
    mprAddKey(esp->views, path, view);
}


static EspRoute *allocEspRoute(HttpRoute *route)
{
    EspRoute  *eroute;

    if ((eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return 0;
    }
    httpSetRouteData(route, ESP_NAME, eroute);
    if ((esp->actions = mprCreateHash(-1, 0)) == 0) {
        return 0;
    }
    if ((esp->views = mprCreateHash(-1, MPR_HASH_STATIC_VALUES)) == 0) {
        return 0;
    }
#if FUTURE
    //  MOB - only do this where required
    if ((eroute->tls = mprCreateThreadLocal()) == 0) {
        return 0;
    }
#endif
#if DEBUG_IDE
    eroute->cacheDir = mprGetAppDir();
#else
    eroute->cacheDir = mprJoinPath(mprGetAppDir(), "../" BLD_LIB_NAME);
#endif
    eroute->dir = route->dir;
    eroute->controllersDir = eroute->dir;
    eroute->databasesDir = eroute->dir;
    eroute->layoutsDir = eroute->dir;
    eroute->modelsDir = eroute->dir;
    eroute->viewsDir = eroute->dir;
    eroute->staticDir = eroute->dir;

    /*
        Setup default parameters for $expansion of Http route paths
     */
    httpSetRoutePathVar(route, "CONTROLLERS_DIR", eroute->controllersDir);
    httpSetRoutePathVar(route, "DATABASES_DIR", eroute->databasesDir);
    httpSetRoutePathVar(route, "LAYOUTS_DIR", eroute->layoutsDir);
    httpSetRoutePathVar(route, "MODELS_DIR", eroute->modelsDir);
    httpSetRoutePathVar(route, "STATIC_DIR", eroute->staticDir);
    httpSetRoutePathVar(route, "VIEWS_DIR", eroute->viewsDir);

    eroute->lifespan = ESP_LIFESPAN;
    eroute->keepSource = BLD_DEBUG;
#if BLD_DEBUG
	eroute->update = 1;
	eroute->showErrors = 1;
#endif
    return eroute;
}


static EspRoute *cloneEspRoute(EspRoute *parent, HttpRoute *route)
{
    EspRoute      *eroute;
    
    mprAssert(parent);
    mprAssert(route);

    if ((eroute = mprAllocObj(EspRoute, espManageEspRoute)) == 0) {
        return 0;
    }
    httpSetRouteData(route, ESP_NAME, eroute);
    eroute->searchPath = parent->searchPath;
    eroute->appModuleName = parent->appModuleName;
    eroute->appModulePath = parent->appModulePath;
    eroute->update = parent->update;
    eroute->keepSource = parent->keepSource;
    eroute->showErrors = parent->showErrors;
    eroute->lifespan = parent->lifespan;
    if (parent->compile) {
        eroute->compile = sclone(parent->compile);
    }
    if (parent->link) {
        eroute->link = sclone(parent->link);
    }
    if (parent->env) {
        eroute->env = mprCloneList(parent->env);
    }
    eroute->dir = parent->dir;
    eroute->cacheDir = parent->cacheDir;
    eroute->controllersDir = parent->controllersDir;
    eroute->databasesDir = parent->databasesDir;
    eroute->layoutsDir = parent->layoutsDir;
    eroute->modelsDir = parent->modelsDir;
    eroute->viewsDir = parent->viewsDir;
    eroute->staticDir = parent->staticDir;
    return eroute;
}


static void setSimpleDirs(EspRoute *eroute)
{
    char    *dir;

    /* Don't set cache dir here - keep inherited value */
    dir = eroute->dir;
    eroute->controllersDir = dir;
    eroute->databasesDir = dir;
    eroute->layoutsDir = dir;
    eroute->modelsDir = dir;
    eroute->viewsDir = dir;
    eroute->staticDir = dir;
}


static void setMvcDirs(EspRoute *eroute, HttpRoute *route)
{
    char    *dir;

    dir = route->dir;
    eroute->cacheDir = mprJoinPath(dir, "cache");
    httpSetRoutePathVar(route, "CACHE_DIR", eroute->cacheDir);

    eroute->controllersDir = mprJoinPath(dir, "controllers");
    httpSetRoutePathVar(route, "CONTROLLERS_DIR", eroute->controllersDir);

    eroute->databasesDir = mprJoinPath(dir, "databases");
    httpSetRoutePathVar(route, "DATABASES_DIR", eroute->databasesDir);

    eroute->layoutsDir  = mprJoinPath(dir, "layouts");
    httpSetRoutePathVar(route, "LAYOUTS_DIR", eroute->layoutsDir);

    eroute->modelsDir  = mprJoinPath(dir, "models");
    httpSetRoutePathVar(route, "MODELS_DIR", eroute->modelsDir);

    eroute->staticDir = mprJoinPath(dir, "static");
    httpSetRoutePathVar(route, "STATIC_DIR", eroute->staticDir);

    eroute->viewsDir = mprJoinPath(dir, "views");
    httpSetRoutePathVar(route, "VIEWS_DIR", eroute->viewsDir);
}


static HttpRoute *addRoute(HttpRoute *parent, cchar *name, cchar *methods, cchar *pattern, cchar *targetKey, 
        cchar *controller)
{
    HttpRoute   *route;

    if ((route = httpCreateInheritedRoute(parent)) == 0) {
        return 0;
    }
    httpSetRouteName(route, name);
    httpSetRoutePattern(route, pattern, 0);
    httpSetRouteTarget(route, "virtual", targetKey);
    if (methods) {
        httpSetRouteMethods(route, methods);
    }
    if (controller) {
        httpSetRouteSource(route, controller);
    }
    httpFinalizeRoute(route);
    return route;
}


static void addRestfulRoute(HttpRoute *parent, cchar *name, cchar *methods, cchar *pattern, cchar *targetKey, 
    cchar *controller, cchar *prefix, cchar *controllerPattern)
{
    pattern = sfmt(pattern, prefix);
    targetKey = sfmt(targetKey, controllerPattern);
    controller = sfmt(controller, controllerPattern);
    addRoute(parent, name, methods, pattern, targetKey, controller);
}


static void addRestfulRoutes(HttpRoute *parent, cchar *prefix, cchar *controller)
{
    addRestfulRoute(parent, "init",    "GET",    "^/%s/init",             "%s-init",      "%s.c", prefix, controller);
    addRestfulRoute(parent, "index",   "GET",    "^/%s(/)$",              "%s-index",     "%s.c", prefix, controller);
    addRestfulRoute(parent, "create",  "POST",   "^/%s(/)",               "%s-create",    "%s.c", prefix, controller);
    addRestfulRoute(parent, "edit",    "GET",    "^/%s/{id=[0-9]+}/edit", "%s-edit",      "%s.c", prefix, controller);
    addRestfulRoute(parent, "show",    "GET",    "^/%s/{id=[0-9]+}",      "%s-show",      "%s.c", prefix, controller);
    addRestfulRoute(parent, "update",  "PUT",    "^/%s/{id=[0-9]+}",      "%s-update",    "%s.c", prefix, controller);
    addRestfulRoute(parent, "destroy", "DELETE", "^/%s/{id=[0-9]+}",      "%s-destroy",   "%s.c", prefix, controller);
    addRestfulRoute(parent, "default", "ALL",    "^/%s(~/{action}~)",     "%s-${action}", "%s.c", prefix, controller);
}


static void addDefaultRoutes(HttpRoute *parent)
{
    addRoute(parent, "home", "GET,POST,PUT", "^/$", stemplate("${STATIC_DIR}/index.esp", parent->pathVars), NULL);
    addRoute(parent, "static", "GET", "%^/static/(.*)", stemplate("${STATIC_DIR}/$1", parent->pathVars), NULL);
}


static void addExtRoute(HttpRoute *parent)
{
#if UNUSED
    HttpRoute   *route;

    route = addRoute(parent, "esp", NULL, "\\.[eE][sS][pP]$", NULL, NULL);
    httpSetRouteTarget(route, "file", 0);
#endif
}

 
static void addRoutePack(HttpRoute *parent, cchar *pack, cchar *prefix, cchar *controller)
{
    if (scasematch(pack, "simple")) {
        addExtRoute(parent);
        addDefaultRoutes(parent);

    } else if (scasematch(pack, "mvc")) {
        addDefaultRoutes(parent);
        addRoute(parent, "default", NULL, "^/{controller}(~/{action}~)", "${controller}-${action}", "${controller}.c");
        addExtRoute(parent);

    } else if (scasematch(pack, "restful")) {
        if (prefix == 0) {
            prefix = "{controller}";
        }
        if (controller == 0) {
            controller = "${controller}";
        }
        addExtRoute(parent);
        addRestfulRoutes(parent, prefix, controller);

    } else if (!scasematch(pack, "none")) {
        mprError("Unknown route pack %s", pack);
    }
}


#if UNUSED
//  MOB - trim slashes not needed
static char *trimSlashes(cchar *str)
{
    ssize   len;

    if (str == 0) {
        return MPR->emptyString;
    }
    if (*str == '/') {
        str++;
    }
    len = slen(str);
    if (str[len - 1] == '/') {
        return snclone(str, len - 1);
    } else {
        return sclone(str);
    }
}
#endif


#if UNUSED
EspPair *espCreatePair(cchar *key, void *data, int flags)
{
    EspPair     *pair;

    if ((pair = mprAllocObj(EspPair, manageEspPair)) == 0) {
        return 0;
    }
    pair->key = sclone(key);
    pair->data = data;
    pair->flags = flags;
    return pair;
}


void manageEspPair(EspPair *pair, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(pair->key);
        if (!(pair->flags & ESP_PAIR_STATIC_VALUES)) {
            mprMark(pair->data);
        }
    } else if (flags & MPR_MANAGE_FREE) {
        if (pair->flags & ESP_PAIR_FREE) {
            free(pair->data);
            pair->data = 0;
        }
    }
}
#endif


void espManageEspRoute(EspRoute *eroute, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(eroute->appModuleName);
        mprMark(eroute->appModulePath);
        mprMark(eroute->cacheDir);
        mprMark(eroute->compile);
        mprMark(eroute->controllersDir);
        mprMark(eroute->databasesDir);
        mprMark(eroute->dir);
        mprMark(eroute->env);
        mprMark(eroute->layoutsDir);
        mprMark(eroute->link);
        mprMark(eroute->modelsDir);
        mprMark(eroute->searchPath);
        mprMark(eroute->staticDir);
        mprMark(eroute->viewsDir);
    }
}


static void manageReq(EspReq *req, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(req->action);
        mprMark(req->cacheBuffer);
        mprMark(req->cacheName);
        mprMark(req->commandLine);
        mprMark(req->controllerName);
        mprMark(req->controllerPath);
        mprMark(req->entry);
        mprMark(req->eroute);
        mprMark(req->module);
        mprMark(req->route);
        mprMark(req->session);
        mprMark(req->source);
        mprMark(req->view);
    }
}


static void manageAction(EspAction *ap, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ap->uri);
    }
}


static void manageEsp(Esp *esp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(esp->mutex);
        mprMark(esp->actions);
        mprMark(esp->views);
        mprMark(esp->cache);
    }
}


static EspRoute *getEspRoute(HttpRoute *route)
{
    EspRoute    *eroute, *parent;

    eroute = httpGetRouteData(route, ESP_NAME);
    if (route->parent) {
        /*
            If the parent route has the same route data, then force a clone so the parent route does not get modified
         */ 
        parent = httpGetRouteData(route->parent, ESP_NAME);
        if (eroute == parent) {
            eroute = 0;
        }
    }
    if (eroute == 0) {
        if (route->parent && (parent = httpGetRouteData(route->parent, ESP_NAME)) != 0) {
            eroute = cloneEspRoute(parent, route);
        } else {
            eroute = allocEspRoute(route);
        }
    }
    mprAssert(eroute);
    return eroute;
}


static void setRouteDirs(MaState *state, cchar *kind)
{
    EspRoute    *eroute;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return;
    }
    if (scmp(kind, "none") == 0) {
        setSimpleDirs(eroute);

    } else if (scmp(kind, "simple") == 0) {
        setSimpleDirs(eroute);

    } else if (scmp(kind, "mvc") == 0) {
        setMvcDirs(eroute, state->route);

    } else if (scmp(kind, "restful") == 0) {
        setMvcDirs(eroute, state->route);
    }
}


/*
    EspApp scriptName [path [routePack]]

    This should only be used inside dedicated ESP routes. This directive is equivalent to:

        ScriptName   scriptName
        DocumentRoot path
        SetHandler   espHandler
        EspRoutePack routePack
 */
static int espAppDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *route;
    EspRoute    *eroute;
    char        *scriptName, *path, *routePack;

    route = state->route;
    if ((eroute = getEspRoute(route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%S %S ?S", &scriptName, &path, &routePack)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    scriptName = stemplate(scriptName, route->pathVars);
    if (scmp(scriptName, "/") == 0) {
        scriptName = MPR->emptyString;
    }
    httpSetRouteScriptName(route, scriptName);
    if (route->pattern == 0) {
        httpSetRoutePattern(route, sjoin("/", scriptName, 0), 0);
    }
    httpSetRouteDir(route, path);
    eroute->dir = route->dir;
    httpAddRouteHandler(route, "espHandler", "");
    addRoutePack(state->route, routePack, 0, 0);
    setRouteDirs(state, routePack);
    //  MOB - is this needed or desired
    httpSetRouteTarget(route, "file", 0);
    return 0;
}


/*
    EspAppAlias scriptName [path [routePackage]]
 */
static int espAppAliasDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *route;
    int         rc;

    if ((route = httpCreateInheritedRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    state = maPushState(state);
    state->route = route;
    rc = espAppDirective(state, NULL, value);
#if UNUSED
    /* MOB - route is not added because it is not really used for routing */
    httpFinalizeRoute(route);
#endif
    maPopState(state);
    return rc;
}


/*
    EspCompile template
 */
static int espCompileDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    eroute->compile = sclone(value);
    return 0;
}


/*
    EspDir key path
 */
static int espDirDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    char        *name, *path;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%S ?S", &name, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scmp(name, "mvc") == 0) {
        setMvcDirs(eroute, state->route);
    } else {
        path = stemplate(mprJoinPath(state->host->home, path), state->route->pathVars);
        if (scmp(name, "cache") == 0) {
            eroute->cacheDir = path;
        } if (scmp(name, "controllers") == 0) {
            eroute->controllersDir = path;
        } else if (scmp(name, "databases") == 0) {
            eroute->databasesDir = path;
        } else if (scmp(name, "layouts") == 0) {
            eroute->layoutsDir = path;
        } else if (scmp(name, "models") == 0) {
            eroute->modelsDir = path;
        } else if (scmp(name, "static") == 0) {
            eroute->staticDir = path;
        } else if (scmp(name, "views") == 0) {
            eroute->viewsDir = path;
        }
        httpSetRoutePathVar(state->route, name, path);
    }
    return 0;
}


/*
    EspEnv var string
 */
static int espEnvDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    char        *ekey, *evalue, *prior;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%S %S", &ekey, &evalue)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (eroute->env == 0) {
        eroute->env = mprCreateList(-1, 0);
    }
    evalue = espExpandCommand(evalue, "", "");
    if ((prior = getenv(ekey)) != 0) {
        mprAddItem(eroute->env, sfmt("%s=%s;%s", ekey, evalue, prior));
    } else {
        mprAddItem(eroute->env, sfmt("%s=%s", ekey, evalue));
    }
    if (scasematch(ekey, "PATH")) {
        if (eroute->searchPath) {
            eroute->searchPath = sclone(evalue);
        } else {
            eroute->searchPath = sjoin(eroute->searchPath, MPR_SEARCH_SEP, evalue, 0);
        }
    }
    return 0;
}


/*
    EspKeepSource on|off
 */
static int espKeepSourceDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    bool        on;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    eroute->keepSource = on;
    return 0;
}


/*
    EspLifespan secs
 */
static int espLifespanDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    eroute->lifespan = ((MprTime) atoi(value)) * MPR_TICKS_PER_SEC;
    return 0;
}


/*
    EspLink template
 */
static int espLinkDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    eroute->link = sclone(value);
    return 0;
}


/*
    EspLoad name path
 */
static int espLoadDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    char        *name, *path;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%S %P", &name, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    eroute->appModuleName = sclone(name);
    eroute->appModulePath = sclone(path);
    return 0;
}


/*
    EspRoutePack kind [/prefix controller]
 */
static int espRoutePackDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    char        *kind, *prefix, *controller;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%S ?S ?P", &kind, &prefix, &controller)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (prefix == 0) {
        prefix = "{controller}";
        controller = "${controller}";
    } else {
        prefix = strim(prefix, "/", MPR_TRIM_START);
    }
    addRoutePack(state->route, kind, prefix, controller);
    return 0;
}


/*
    EspRoute name methods pattern targetKey source
 */
static int espRouteDirective(MaState *state, cchar *key, cchar *value)
{
    char        *name, *methods, *pattern, *targetKey, *source;

    if (!maTokenize(state, value, "%S %S %S %S %S", &name, &methods, &pattern, &targetKey, &source)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    addRoute(state->route, name, methods, pattern, targetKey, source);
    return 0;
}


/*
    EspShowErrors on|off
 */
static int espShowErrorsDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    bool        on;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    eroute->showErrors = on;
    return 0;
}


/*
    EspUpdate on|off
 */
static int espUpdateDirective(MaState *state, cchar *key, cchar *value)
{
    EspRoute    *eroute;
    bool        on;

    if ((eroute = getEspRoute(state->route)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    eroute->update = on;
    return 0;

}


int maEspHandlerInit(Http *http)
{
    HttpStage   *handler;
    MaAppweb    *appweb;

    if ((handler = httpCreateHandler(http, "espHandler", HTTP_STAGE_QUERY_VARS, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openEsp; 
    handler->close = closeEsp; 
    handler->start = startEsp; 
    if ((esp = handler->stageData = mprAllocObj(Esp, manageEsp)) == 0) {
        return MPR_ERR_MEMORY;
    }
    esp->mutex = mprCreateLock();
    if ((esp->views = mprCreateHash(-1, MPR_HASH_STATIC_VALUES)) == 0) {
        return 0;
    }
    if ((esp->actions = mprCreateHash(-1, 0)) == 0) {
        return 0;
    }
    if ((esp->cache = mprCreateCache(MPR_CACHE_SHARED)) == 0) {
        return MPR_ERR_MEMORY;
    }

    appweb = httpGetContext(http);
    maAddDirective(appweb, "EspApp", espAppDirective);
    maAddDirective(appweb, "EspAppAlias", espAppAliasDirective);
    maAddDirective(appweb, "EspCompile", espCompileDirective);
    maAddDirective(appweb, "EspDir", espDirDirective);
    maAddDirective(appweb, "EspEnv", espEnvDirective);
    maAddDirective(appweb, "EspKeepSource", espKeepSourceDirective);
    maAddDirective(appweb, "EspLifespan", espLifespanDirective);
    maAddDirective(appweb, "EspLink", espLinkDirective);
    maAddDirective(appweb, "EspLoad", espLoadDirective);
    maAddDirective(appweb, "EspRoute", espRouteDirective);
    maAddDirective(appweb, "EspRoutePack", espRoutePackDirective);
    maAddDirective(appweb, "EspShowErrors", espShowErrorsDirective);
    maAddDirective(appweb, "EspUpdate", espUpdateDirective);
    return 0;
}


#else /* BLD_FEATURE_ESP */

int maEspHandlerInit(Http *http)
{
    mprNop(0);
    return 0;
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
