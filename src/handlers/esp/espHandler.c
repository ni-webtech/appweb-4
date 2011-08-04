/*
    espHandler.c -- Embedded Server Pages (ESP) handler. Fast in-process replacement for CGI.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
#include    "esp.h"
//  MOB - does esp require pcre?
#include    "pcre.h"

/************************************* Local **********************************/

#define ESP_NAME "espHandler"

static EspLoc *allocEspLoc(HttpLoc *loc);
static bool moduleIsCurrent(HttpConn *conn, cchar *source, cchar *module);
static int fetchCachedResponse(HttpConn *conn);
static void manageEspLoc(EspLoc *esp, int flags);
static void manageReq(EspReq *req, int flags);
static int  runAction(HttpConn *conn, cchar *actionKey);
static void runView(HttpConn *conn, cchar *name);
static int saveCachedResponse(HttpConn *conn);

/************************************* Code ***********************************/

static void openEsp(HttpQueue *q)
{
    HttpConn    *conn;
    HttpAlias   *alias;
    HttpRx      *rx;
    EspReq      *req;
    char        *uri;

    conn = q->conn;
    rx = conn->rx;

    if ((req = mprAllocObj(EspReq, manageReq)) == 0) {
        httpMemoryError(conn);
        return;
    }
    req->loc = rx->loc;
    conn->data = req;
    req->autoFinalize = 1;
    if ((req->esp = httpGetLocationData(rx->loc, ESP_NAME)) == 0) {
        req->esp = allocEspLoc(rx->loc);
        httpSetLocationData(rx->loc, ESP_NAME, req->esp);
    }
    mprAssert(req->esp);

    /*
        Set the scriptName to the alias prefix and remove from pathInfo
     */
    rx = conn->rx;
    uri = rx->pathInfo;
    alias = rx->alias;
    if (alias->prefixLen > 0) {
        uri = &uri[alias->prefixLen];
#if UNUSED
        if (uri > rx->pathInfo && *uri != '/' && uri[-1] == '/') {
            uri--;
        }
#endif
        if (*uri == '\0') {
            uri = "/";
        }
        rx->scriptName = alias->prefix;
        rx->pathInfo = sclone(uri);
        mprLog(5, "ejs: set script name: \"%s\", pathInfo: \"%s\"", rx->scriptName, rx->pathInfo);
    }
}


static void startEsp(HttpQueue *q)
{
    HttpConn        *conn;
    EspLoc          *esp;
    EspReq          *req;
    EspRoute        *route;
    cchar           *actionKey;
    int             next;

    conn = q->conn;
    req = conn->data;
    esp = req->esp;

    if (conn->error) {
        //  MOB - is this required?
        return;
    }
    for (next = 0; (route = mprGetNextItem(esp->routes, &next)) != 0; ) {
        if ((actionKey = espMatchRoute(conn, route)) != 0) {
            break;
        }
    }
    if (route == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "No route to serve request");
        return;
    }
    req->route = route;
    mprLog(4, "Using route: %s for %s, actionKey %s", route->name, conn->rx->pathInfo, actionKey);

    httpAddFormVars(conn);
#if FUTURE
    espSetConn(conn);
        mprSetThreadData(esp->tls, conn);
        //  MOB - better to use
#endif

    if (esp->lifespan && fetchCachedResponse(conn)) {
        return;
    }
    if (route->controllerName && !runAction(conn, actionKey)) {
        return;
    }
    if (!conn->tx->responded && req->autoFinalize) {
        runView(conn, actionKey);
    }
    if (req->autoFinalize && !conn->tx->finalized) {
        httpFinalize(conn);
    }
    if (req->cacheBuffer) {
        saveCachedResponse(conn);
    }
}


static int fetchCachedResponse(HttpConn *conn)
{
    //  MOB complete
    //  MOB - allocate cacheBuffer
    return 0;
}


static int saveCachedResponse(HttpConn *conn)
{
    //  MOB - or does the write buffer prevent this being set?
    //  MOB complete
    if (conn->tx->finalized) {
    }
    return 0;
}


static char *getControllerEntry(cchar *controllerName)
{
    char    *cp, *entry;

    entry = sfmt("espController_%s", mprGetPathBase(controllerName));
    for (cp = entry; *cp; cp++) {
        if (!isalnum((int) *cp) && *cp != '_') {
            *cp = '_';
        }
    }
    return entry;
}


static int runAction(HttpConn *conn, cchar *actionKey)
{
    MprModule   *mp;
    EspLoc      *esp;
    EspReq      *req;
    EspRoute    *route;
    EspAction   action;

    req = conn->data;
    esp = req->esp;
    route = req->route;

    /*
        Expand any form var $tokens. This permits ${controller} and user form data to be used in the controller name
     */
    if (schr(route->controllerName, '$')) {
        route->controllerName = stemplate(route->controllerName, conn->rx->formVars);
    }
    route->controllerPath = mprJoinPath(esp->controllersDir, route->controllerName);
    req->cacheName = mprGetMD5Hash(route->controllerPath, slen(route->controllerPath), "controller_");
    req->module = mprGetNormalizedPath(sfmt("%s/%s%s", esp->cacheDir, req->cacheName, BLD_SHOBJ));

    if (esp->reload) {
        if (!mprPathExists(route->controllerPath, R_OK)) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find controller %s", route->controllerPath);
            return 0;
        }
        if (!moduleIsCurrent(conn, route->controllerPath, req->module)) {
            /* Modules are named by source to aid debugging */
            if ((mp = mprLookupModule(route->controllerPath)) != 0) {
                //  What if some modules cant be unloaded?
                //  MOB - must complete all other running requests first
                mprUnloadModule(mp);
            }
            //  WARNING: this will allow GC
            if (!espCompile(conn, route->controllerPath, req->module, req->cacheName, 0)) {
                return 0;
            }
        }
        if (mprLookupModule(route->controllerPath) == 0) {
            req->entry = getControllerEntry(route->controllerName);
            //  MOB - who keeps reference to module?
            if ((mp = mprCreateModule(route->controllerPath, req->module, req->entry, esp)) == 0) {
                httpMemoryError(conn);
                return 0;
            }
            //  MOB - this should return an error msg
            if (mprLoadModule(mp) < 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                    "Can't load compiled esp module for %s", route->controllerPath);
                return 0;
            }
        }
    }
    if ((action = mprLookupKey(esp->actions, actionKey)) == 0) {
        actionKey = (cchar*) sfmt("%s-missing", mprTrimPathExtension(route->controllerName));
        if ((action = mprLookupKey(esp->actions, actionKey)) == 0) {
            if ((action = mprLookupKey(esp->actions, "missing")) == 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing action for %s", actionKey);
                return 0;
            }
        }
    }
    (action)(conn);
    return 1;
}


static void runView(HttpConn *conn, cchar *actionKey)
{
    MprModule   *mp;
    EspLoc      *esp;
    EspReq      *req;
    EspRoute    *route;
    EspView     view;
    
    req = conn->data;
    esp = req->esp;
    route = req->route;
    
    if (route->controllerName) {
        req->view = mprJoinPath(esp->viewsDir, actionKey);
    } else {
        req->view = mprJoinPath(conn->host->documentRoot, actionKey);
    }
    req->source = mprJoinPathExt(req->view, ".esp");
    req->cacheName = mprGetMD5Hash(req->source, slen(req->source), "view_");
    req->module = mprGetNormalizedPath(sfmt("%s/%s%s", req->esp->cacheDir, req->cacheName, BLD_SHOBJ));

    if (esp->reload) {
        if (!mprPathExists(req->source, R_OK)) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find view %s", req->source);
            return;
        }
        if (!moduleIsCurrent(conn, req->source, req->module)) {
            /* Modules are named by source to aid debugging */
            if ((mp = mprLookupModule(req->source)) != 0) {
                //  What if some modules cant be unloaded?
                //  MOB - must complete all other running requests first
                mprUnloadModule(mp);
            }
            //  WARNING: this will allow GC
            if (!espCompile(conn, req->source, req->module, req->cacheName, 1)) {
                return;
            }
        }
        if (mprLookupModule(req->source) == 0) {
            req->entry = sfmt("espInit_%s", req->cacheName);
            //  MOB - who keeps reference to module?
            if ((mp = mprCreateModule(req->source, req->module, req->entry, esp)) == 0) {
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


static bool moduleIsCurrent(HttpConn *conn, cchar *source, cchar *module)
{
    EspReq      *req;
    MprPath     sinfo, minfo;

    req = conn->data;
    if (mprPathExists(module, R_OK)) {
        mprGetPathInfo(source, &sinfo);
        mprGetPathInfo(module, &minfo);
        //  MOB - also want a universal touch to rebuild all. Touch appweb.conf
        /* The loaded module is named by source to aid debugging */
        if (sinfo.mtime < minfo.mtime) {
            return 1;
        }
    }
    return 0;
}


void espDefineAction(EspLoc *esp, cchar *name, void *action)
{
    mprAssert(esp);
    mprAssert(name && *name);
    mprAssert(action);

    mprAddKey(esp->actions, name, action);
}


void espDefineView(EspLoc *esp, cchar *path, void *view)
{
    mprAssert(esp);
    mprAssert(path && *path);
    mprAssert(view);

    mprAddKey(esp->views, path, view);
}


static EspLoc *allocEspLoc(HttpLoc *loc)
{
    EspLoc  *esp;

    if ((esp = mprAllocObj(EspLoc, manageEspLoc)) == 0) {
        return 0;
    }
    if ((esp->actions = mprCreateHash(-1, MPR_HASH_STATIC_VALUES)) == 0) {
        return 0;
    }
    if ((esp->routes = mprCreateList(-1, 0)) == 0) {
        return 0;
    }
    if ((esp->views = mprCreateHash(-1, MPR_HASH_STATIC_VALUES)) == 0) {
        return 0;
    }
#if FUTURE
    //  MOB - only do this where required
    if ((esp->tls = mprCreateThreadLocal()) == 0) {
        return 0;
    }
#endif
    esp->dir = (loc->alias) ? loc->alias->filename : loc->host->serverRoot;
    esp->controllersDir = esp->dir;
    esp->databasesDir = esp->dir;
    esp->layoutsDir = esp->dir;
    esp->modelsDir = esp->dir;
    esp->viewsDir = esp->dir;
    esp->webDir = esp->dir;

    /*
        Setup default parameters for $expansion of Http location paths
     */
    httpAddLocationToken(loc, "CONTROLLERS_DIR", esp->controllersDir);
    httpAddLocationToken(loc, "DATABASES_DIR", esp->databasesDir);
    httpAddLocationToken(loc, "LAYOUTS_DIR", esp->layoutsDir);
    httpAddLocationToken(loc, "MODELS_DIR", esp->modelsDir);
    httpAddLocationToken(loc, "STATIC_DIR", esp->webDir);
    httpAddLocationToken(loc, "VIEWS_DIR", esp->viewsDir);

#if DEBUG_IDE
    esp->cacheDir = mprGetAppDir();
#else
    esp->cacheDir = mprJoinPath(mprGetAppDir(), "../" BLD_LIB_NAME);
#endif

    esp->lifespan = ESP_LIFESPAN;
    esp->keepSource = 0;
#if BLD_DEBUG
	esp->reload = 1;
	esp->showErrors = 1;
#endif
    esp->loc = loc;
    return esp;
}


static EspLoc *cloneEspLoc(HttpLoc *loc)
{
    EspLoc      *esp, *outer;
    HttpLoc     *parent;

    parent = loc->parent;

    if ((outer = httpGetLocationData(parent, ESP_NAME)) == 0) {
        return allocEspLoc(loc);
    }
    if ((esp = mprAllocObj(EspLoc, manageEspLoc)) == 0) {
        return 0;
    }
    esp->reload = outer->reload;
    esp->keepSource = outer->keepSource;
    esp->showErrors = outer->showErrors;
    esp->lifespan = outer->lifespan;
    if (outer->compile) {
        esp->compile = sclone(outer->compile);
    }
    if (outer->link) {
        esp->link = sclone(outer->link);
    }
    if (outer->env) {
        esp->env = mprCloneList(outer->env);
    }
    if (outer->routes) {
        esp->routes = mprCloneList(outer->routes);
    } else {
        esp->routes = mprCreateList(-1, 0);
    }
    if (outer->actions) {
        esp->actions = mprCloneHash(outer->actions);
    } else {
        esp->actions = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    }
    if (outer->views) {
        esp->views = mprCloneHash(outer->views);
    } else {
        esp->views = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);   
    }
    esp->dir = outer->dir;
    esp->cacheDir = outer->cacheDir;
    esp->controllersDir = outer->controllersDir;
    esp->databasesDir = outer->databasesDir;
    esp->layoutsDir = outer->layoutsDir;
    esp->modelsDir = outer->modelsDir;
    esp->viewsDir = outer->viewsDir;
    esp->webDir = outer->webDir;

    if (mprHasMemError()) {
        return 0;
    }
    return esp;
}


static int parseEsp(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLoc     *loc;
    HttpHost    *host;
    HttpAlias   *alias;
    HttpDir     *dir, *parent;
    EspLoc      *esp;
    EspRoute    *route;
    char        *name, *ekey, *evalue, *prefix, *path, *next, *methods, *prior, *pattern, *action, *controller;
    
    host = state->host;
    loc = state->loc;

    if (!sstarts(key, "Esp")) {
        return 0;
    }
    if ((esp = httpGetLocationData(loc, ESP_NAME)) == 0) {
        if (loc->parent) {
            esp = cloneEspLoc(loc);
        } else {
            esp = allocEspLoc(loc);
        }
        if (esp == 0) {
            return MPR_ERR_MEMORY;
        }
        httpSetLocationData(loc, ESP_NAME, esp);
    }
    mprAssert(esp);

    if (scasecmp(key, "EspAlias") == 0) {
        /*
            EspAlias prefix [path]
         */
        if (maGetConfigValue(&prefix, value, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (maGetConfigValue(&path, next, &next, 1) < 0) {
            path = ".";
        }
        prefix = stemplate(prefix, loc->tokens);
        esp->dir = path = httpMakePath(loc, path);
        if (httpLookupDir(host, path) == 0) {
            parent = mprGetFirstItem(host->dirs);
            dir = httpCreateDir(path, parent);
            httpAddDir(host, dir);
        }
        if ((loc = httpLookupLocation(host, prefix)) == 0) {
            loc = httpCreateInheritedLocation(state->loc);
            httpSetLocationHost(loc, host);
            httpSetLocationPrefix(loc, prefix);
            httpSetLocationAuth(loc, state->dir->auth);
            httpAddLocation(host, loc);
            httpSetHandler(loc, "espHandler");
        }
        if (loc->alias == 0) {
            alias = httpCreateAlias(prefix, path, 0);
            mprLog(4, "EspAlias \"%s\" for \"%s\"", prefix, path);
            httpSetLocationAlias(loc, alias);
            httpAddAlias(host, alias);
        }
        return 1;

    } else if (scasecmp(key, "EspCache") == 0) {
        esp->lifespan = ((MprTime) atoi(value)) * MPR_TICKS_PER_SEC;
        return 1;

    } else if (scasecmp(key, "EspCompile") == 0) {
        esp->compile = sclone(value);
        return 1;

    } else if (scasecmp(key, "EspDir") == 0) {
        /*
            EspDir name dir
         */
        if (maGetConfigValue(&name, value, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (scmp(name, "mvc") == 0) {
            esp->cacheDir = mprJoinPath(esp->dir, "cache");
            httpAddLocationToken(loc, "CACHE_DIR", esp->cacheDir);

            esp->controllersDir = mprJoinPath(esp->dir, "controllers");
            httpAddLocationToken(loc, "CONTROLLERS_DIR", esp->controllersDir);

            esp->databasesDir = mprJoinPath(esp->dir, "databases");
            httpAddLocationToken(loc, "DATABASES_DIR", esp->databasesDir);

            esp->layoutsDir  = mprJoinPath(esp->dir, "layouts");
            httpAddLocationToken(loc, "LAYOUTS_DIR", esp->layoutsDir);

            esp->modelsDir  = mprJoinPath(esp->dir, "models");
            httpAddLocationToken(loc, "MODELS_DIR", esp->modelsDir);

            esp->webDir = mprJoinPath(esp->dir, "static");
            httpAddLocationToken(loc, "STATIC_DIR", esp->webDir);

            esp->viewsDir = mprJoinPath(esp->dir, "views");
            httpAddLocationToken(loc, "VIEWS_DIR", esp->viewsDir);

        } else {
            path = stemplate(mprJoinPath(esp->dir, next), loc->tokens);
            if (scmp(name, "cache") == 0) {
                esp->cacheDir = path;
            } if (scmp(name, "controllers") == 0) {
                esp->controllersDir = path;
            } else if (scmp(name, "databases") == 0) {
                esp->databasesDir = path;
            } else if (scmp(name, "layouts") == 0) {
                esp->layoutsDir = path;
            } else if (scmp(name, "models") == 0) {
                esp->modelsDir = path;
            } else if (scmp(name, "static") == 0) {
                esp->webDir = path;
            } else if (scmp(name, "views") == 0) {
                esp->viewsDir = path;
            }
            httpAddLocationToken(loc, name, path);
        }
        return 1;

    } else if (scasecmp(key, "EspEnv") == 0) {
        if (maGetConfigValue(&ekey, value, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (maGetConfigValue(&evalue, next, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (esp->env == 0) {
            esp->env = mprCreateList(-1, 0);
        }
        if ((prior = getenv(ekey)) != 0) {
            mprAddItem(esp->env, sfmt("%s=%s;%s", ekey, evalue, prior));
        } else {
            mprAddItem(esp->env, sfmt("%s=%s", ekey, evalue));
        }
        return 1;

    } else if (scasecmp(key, "EspKeepSource") == 0) {
        esp->keepSource = (scasecmp(value, "on") == 0 || scasecmp(value, "yes") == 0);
        return 1;

    } else if (scasecmp(key, "EspLink") == 0) {
        esp->link = sclone(value);
        return 1;

    } else if (scasecmp(key, "EspReload") == 0) {
        esp->reload = scasecmp(value, "on") == 0;
        return 1;

    } else if (scasecmp(key, "EspReset") == 0) {
        if (scasecmp(value, "all") == 0) {
            esp = allocEspLoc(loc);
            httpSetLocationData(loc, ESP_NAME, esp);
        } else if (scasecmp(value, "routes") == 0) {
            esp->routes = mprCreateList(-1, 0);
        }
        return 1;

    } else if (scasecmp(key, "EspRoute") == 0) {
        /*
            EspRoute name methods pattern action [controller]
         */
        if (maGetConfigValue(&name, value, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (maGetConfigValue(&methods, next, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (maGetConfigValue(&pattern, next, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        maGetConfigValue(&action, next, &next, 1);
        if (action) {
            action = stemplate(action, loc->tokens);
        }
        maGetConfigValue(&controller, next, &next, 1);
        if (controller) {
            controller = stemplate(controller, loc->tokens);
        }
        if ((route = espCreateRoute(name, methods, pattern, action, controller)) == 0) {
            return MPR_ERR_MEMORY;
        }
        mprAddItem(esp->routes, route);
        return 1;
        
    } else if (scasecmp(key, "EspShowErrors") == 0) {
		esp->showErrors = (scasecmp(value, "on") == 0 || scasecmp(value, "yes") == 0);
        return 1;
    }
    return 0;
}


static void manageEspLoc(EspLoc *esp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(esp->actions);
        mprMark(esp->routes);
        mprMark(esp->views);
        mprMark(esp->compile);
        mprMark(esp->link);
        mprMark(esp->env);
        mprMark(esp->dir);
        mprMark(esp->cacheDir);
        mprMark(esp->controllersDir);
        mprMark(esp->databasesDir);
        mprMark(esp->layoutsDir);
        mprMark(esp->modelsDir);
        mprMark(esp->viewsDir);
        mprMark(esp->webDir);
    }
}


static void manageReq(EspReq *req, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(req->entry);
        mprMark(req->view);
        mprMark(req->controller);
        mprMark(req->source);
        mprMark(req->module);
        mprMark(req->cacheName);
        mprMark(req->cacheBuffer);
        mprMark(req->route);
        mprMark(req->esp);
    }
}


int maEspHandlerInit(Http *http)
{
    HttpStage       *handler;

    if ((handler = httpCreateHandler(http, "espHandler", HTTP_STAGE_QUERY_VARS | HTTP_STAGE_VIRTUAL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openEsp; 
    handler->start = startEsp; 
    handler->parse = (HttpParse) parseEsp; 
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
