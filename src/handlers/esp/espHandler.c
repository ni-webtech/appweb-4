/*
    espHandler.c -- Embedded Gateway Interface (ESP) handler. Fast in-process replacement for CGI.

    The ESP handler implements a very fast in-process CGI scheme.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
    #include    "esp.h"
//  MOB - does esp require pcre?
    #include    "pcre.h"

/************************************* Local **********************************/

static bool moduleIsCurrent(HttpConn *conn, cchar *name, cchar *stem, cchar *source, cchar *module);
static int fetchCachedResponse(HttpConn *conn);
static void manageReq(EspReq *req, int flags);
static void manageRoute(EspRoute *route, int flags);
static char *matchRoute(HttpConn *conn, EspRoute *route);
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
    conn->data = req;
    req->autoFinalize = 1;
    req->esp = q->stage->stageData;

    /*
        Set the scriptName to the alias prefix and remove from pathInfo
     */
    rx = conn->rx;
    uri = rx->pathInfo;
    alias = rx->alias;
    if (alias->prefixLen > 0) {
        uri = &uri[alias->prefixLen];
        if (*uri != '/' && uri[-1] == '/') {
            uri--;
        }
        rx->scriptName = alias->prefix;
        rx->pathInfo = sclone(uri);
        mprLog(5, "ejs: set script name: \"%s\", pathInfo: \"%s\"", rx->scriptName, rx->pathInfo);
    }
}


static void startEsp(HttpQueue *q)
{
    HttpConn        *conn;
    Esp             *esp;
    EspReq          *req;
    EspRoute        *route;
    EspAction       *action;
    MprModule       *mp;
    cchar           *actionKey, *entry;
    int             next;

    conn = q->conn;
    req = conn->data;
    esp = req->esp;

    if (conn->error) {
        //  MOB - is this required?
        return;
    }
    for (next = 0; (route = mprGetNextItem(esp->routes, &next)) != 0; ) {
        if ((actionKey = matchRoute(conn, route)) != 0) {
            break;
        }
    }
    if (route == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "No route to serve request");
        return;
    }
    req->route = route;

    if (route->module) {
        /*
            Load module containing controller/action
            MOB - should have same code as for views and build from source
         */
        if (mprLookupModule(route->module) == 0) {
            entry = sfmt("esp_%s", supper(mprGetPathBase(route->module)));
            if ((mp = mprCreateModule(route->module, route->module, entry, esp)) == 0) {
                httpMemoryError(conn);
                return;
            }
            if (mprLoadModule(mp) < 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load module %s", route->module);
                return;
            }
        }
    }
    if (esp->lifespan && fetchCachedResponse(conn)) {
        return;
    }
    if ((action = mprLookupKey(esp->actions, actionKey)) != 0) {
        (*action)(conn);
    }
    if (!req->responded && req->autoFinalize) {
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


static void runView(HttpConn *conn, cchar *actionKey)
{
    MprModule   *mp;
    Esp         *esp;
    EspReq      *req;
    EspView     view;

    req = conn->data;
    esp = req->esp;
    req->path = mprJoinPath(conn->host->documentRoot, actionKey);
    req->source = mprJoinPathExt(req->path, ".esp");
    req->baseName = mprGetMD5Hash(req->source, slen(req->source), "espView_");
    req->module = mprGetNormalizedPath(mprAsprintf("%s/%s%s", req->esp->modDir, req->baseName, BLD_SHOBJ));

    if (esp->reload) {
        if (!mprPathExists(req->source, R_OK)) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find view %s", req->source);
            return;
        }
        if (!moduleIsCurrent(conn, req->baseName, req->path, req->source, req->module)) {
            /* Modules are named by source to aid debugging */
            if ((mp = mprLookupModule(req->source)) != 0) {
                //  What if some modules cant be unloaded?
                //  MOB - must complete all other running requests first
                mprUnloadModule(mp);
            }
            //  WARNING: this will allow GC
            if (!espCompile(conn, req->baseName, req->source, req->module)) {
                return;
            }
            req->entry = sfmt("espInit_%s", req->baseName);
            //  MOB - who keeps reference to module?
            if ((mp = mprCreateModule(req->source, req->module, req->entry, esp)) == 0) {
                httpMemoryError(conn);
                return;
            }
            //  MOB - this should return an error msg
            if (mprLoadModule(mp) < 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load compiled module for %s", req->source);
                return;
            }
        }
    }
    if ((view = mprLookupKey(esp->views, mprGetPortablePath(req->path))) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find defined view for %s", req->path);
        return;
    }
    (view)(conn);
}


static bool moduleIsCurrent(HttpConn *conn, cchar *name, cchar *stem, cchar *source, cchar *module)
{
    EspReq      *req;
    MprPath     sinfo, cinfo;

    req = conn->data;
    if (mprPathExists(module, R_OK)) {
        mprGetPathInfo(source, &sinfo);
        mprGetPathInfo(module, &cinfo);
        //  MOB - also want a universal touch to rebuild all. Touch appweb.conf
        /* The loaded module is named by source to aid debugging */
        if (sinfo.mtime < cinfo.mtime && mprLookupModule(source) != 0) {
            return 1;
        }
    }
    return 0;
}


void espDefineAction(Esp *esp, cchar *name, EspAction action)
{
    mprAssert(esp);
    mprAssert(name && *name);
    mprAssert(action);

    mprAddKey(esp->actions, name, action);
}


void espDefineView(Esp *esp, cchar *path, void *view)
{
    mprAssert(esp);
    mprAssert(path && *path);
    mprAssert(view);

    mprAddKey(esp->views, path, view);
}


/*
    Compile the route into fast forms. This compiles the route->methods into a hash table, route->pattern into 
    a regular expression in route->compiledPattern.
    MOB - refactor into separate functions
 */
static int prepRoute(Esp *esp, EspRoute *route, char *methods)
{
    MprBuf  *pattern, *params, *buf;
    cchar   *errMsg, *item;
    char    *method, *tok, *cp, *ep, *token;
    int     column, errCode, index, next;

    /*
        Convert the methods into a method hash
     */
    if (route->methods == 0) {
        if ((route->methods = mprCreateHash(-1, 0)) == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    //  MOB - does this work having methods always defined?
    while ((method = stok(methods, ", \t\n\r", &tok)) != 0) {
        mprAddKey(route->methods, method, route);
        methods = 0;
    }

    /*
        Prepend the module directory to the module name
     */
    if (route->module) {
        route->module = mprJoinPath(esp->modDir, route->module);
    }

    /*
        Prepare the pattern. 
            - Extract the tokens and change tokens: "{word}" to "(word)"
            - Change optional sections: "(portion)" to "(?:portion)?"
            - Change "/" to "\/"
            - Create a params RE replacement string of the form "$1:$2:$3" for the {tokens}
            - Wrap the pattern in "^" and "$"
     */
    route->tokens = mprCreateList(-1, 0);
    pattern = mprCreateBuf(-1, -1);
    params = mprCreateBuf(-1, -1);

    if (route->pattern[0] == '%') {
        route->patternExpression = sclone(&route->pattern[1]);
        if (route->action) {
            route->compiledAction = route->action;
        } else {
            /* Replace with everything */
            route->compiledAction = sclone("$&");
        }
        /* Can't have a link template if using regular expressions */

    } else {
        for (cp = route->pattern; *cp; cp++) {
            if (*cp == '(') {
                mprPutStringToBuf(pattern, "(?:");

            } else if (*cp == ')') {
                mprPutStringToBuf(pattern, ")?");

            } else if (*cp == '/') {
                mprPutStringToBuf(pattern, "\\/");

            } else if (*cp == '{' && (cp == route->pattern || cp[-1] != '\\')) {
                if ((ep = schr(cp, '}')) != 0) {
                    /* Trim {} off the token and replace in pattern with "([^/]*)"  */
                    mprPutStringToBuf(pattern, "([^/]*)");

                    /* Params ends up looking like "$1:$2:$3:$4" */
                    index = mprAddItem(route->tokens, snclone(&cp[1], ep - cp -1));
                    mprPutCharToBuf(params, '$');
                    mprPutIntToBuf(params, index + 1);
                    mprPutCharToBuf(params, ':');
                    cp = ep;
                }
            } else {
                mprPutCharToBuf(pattern, *cp);
            }
        }
        mprAddNullToBuf(pattern);
        mprAddNullToBuf(params);
        route->patternExpression = sclone(mprGetBufStart(pattern));

        /* Trim last ":" from params */
        route->params = sclone(mprGetBufStart(params));
        route->params[slen(route->params) - 1] = '\0';

        /*
            Prepare the action. Change $token to $N
         */
        buf = mprCreateBuf(-1, -1);
        for (cp = route->action; *cp; cp++) {
            if ((tok = schr(cp, '$')) != 0 && (tok == route->action || tok[-1] != '\\')) {
                for (ep = &tok[1]; isalnum(*ep); ep++) ;
                token = snclone(&tok[1], ep - tok - 1);
                for (next = 0; (item = mprGetNextItem(route->tokens, &next)) != 0; ) {
                    if (scmp(item, token) == 0) {
                        break;
                    }
                }
                if (item) {
                    mprPutCharToBuf(buf, '$');
                    mprPutIntToBuf(buf, next);
                    mprPutCharToBuf(buf, ':');
                } else {
                    mprError("Can't find token \"%s\" in template \"%s\"", token, route->pattern);
                }
                cp = ep - 1;
            } else {
                mprPutCharToBuf(buf, *cp);
            }
        }
        mprAddNullToBuf(buf);
        /* Trim last ":" from params */
        route->compiledAction = sclone(mprGetBufStart(buf));
        route->compiledAction[slen(route->compiledAction) - 1] = '\0';

        /*
            Create a template for links. Strip "()" and "/.*" from the pattern.
         */
        route->template = sreplace(route->pattern, "(", "");
        route->template = sreplace(route->template, ")", "");
        route->template = sreplace(route->template, "/.*", "");
    }
    if ((route->compiledPattern = pcre_compile2(route->patternExpression, 0, &errCode, &errMsg, &column, NULL)) == 0) {
        mprError("Can't compile route. Error %s at column %d", errMsg, column); 
    }
    return 0;
}


char *pcre_replace(cchar *str, void *pattern, cchar *replacement, MprList **parts, int flags)
{
    MprBuf  *result;
    cchar   *end, *cp, *lastReplace;
    ssize   len, count;
    int     matches[ESP_MAX_ROUTE_MATCHES * 3];
    int     i, endLastMatch, startNextMatch, submatch;

    len = slen(str);
    startNextMatch = endLastMatch = 0;
    result = mprCreateBuf(-1, -1);
    do {
        if (startNextMatch > len) {
            break;
        }
        count = pcre_exec(pattern, NULL, str, (int) len, startNextMatch, 0, matches, sizeof(matches) / sizeof(int));
        if (count < 0) {
            break;
        }
        if (endLastMatch < matches[0]) {
            /* Append prior string text */
            mprPutSubStringToBuf(result, &str[endLastMatch], matches[0] - endLastMatch);
        }
        end = &replacement[slen(replacement)];
        lastReplace = replacement;
        for (cp = replacement; cp < end; ) {
            if (*cp == '$') {
                if (lastReplace < cp) {
                    mprPutSubStringToBuf(result, lastReplace, (int) (cp - lastReplace));
                }
                switch (*++cp) {
                case '$':
                    mprPutCharToBuf(result, '$');
                    break;
                case '&':
                    /* Replace the matched string */
                    mprPutSubStringToBuf(result, &str[matches[0]], matches[1] - matches[0]);
                    break;
                case '`':
                    /* Insert the portion that preceeds the matched string */
                    mprPutSubStringToBuf(result, str, matches[0]);
                    break;
                case '\'':
                    /* Insert the portion that follows the matched string */
                    mprPutSubStringToBuf(result, &str[matches[1]], len - matches[1]);
                    break;
                default:
                    /* Insert the nth submatch */
                    if (isdigit((int) *cp)) {
                        submatch = (int) wtoi(cp, 10, NULL);
                        while (isdigit((int) *++cp))
                            ;
                        cp--;
                        if (submatch < count) {
                            submatch *= 2;
                            mprPutSubStringToBuf(result, &str[matches[submatch]], matches[submatch + 1] - matches[submatch]);
                        }
                    } else {
                        mprError("Bad replacement $ specification in page");
                        return 0;
                    }
                }
                lastReplace = cp + 1;
            }
            cp++;
        }
        if (lastReplace < cp && lastReplace < end) {
            mprPutSubStringToBuf(result, lastReplace, (int) (cp - lastReplace));
        }
        endLastMatch = matches[1];
        startNextMatch = (startNextMatch == endLastMatch) ? startNextMatch + 1 : endLastMatch;
    } while (flags & PCRE_GLOBAL);

    if (endLastMatch < len) {
        /* Append remaining string text */
        mprPutSubStringToBuf(result, &str[endLastMatch], len - endLastMatch);
    }
    mprAddNullToBuf(result);
    if (parts) {
        *parts = mprCreateList(-1, 0);
        for (i = 0; i < count; i++) {
            char *token = snclone(&str[matches[i]], matches[i + 1] - matches[i]);
            mprAddItem(*parts, token);
        }
    }
    return sclone(mprGetBufStart(result));
}


static char *matchRoute(HttpConn *conn, EspRoute *route)
{
    HttpRx      *rx;
    MprList     *parts;
    cchar       *token, *actionKey;
    int         matches[ESP_MAX_ROUTE_MATCHES * 3];
    int         next, start, count;

    rx = conn->rx;
    if (!mprLookupKey(route->methods, conn->rx->method)) {
        return 0;
    }
    start = 0;
    count = pcre_exec(route->compiledPattern, NULL, rx->pathInfo, (int) slen(rx->pathInfo), start, 0, 
        matches, sizeof(matches) / sizeof(int));
    if (count < 0) {
        return 0;
    }
    if (route->params) {
        if (!pcre_replace(rx->pathInfo, route->compiledPattern, route->params, &parts, 0)) {
            return 0;
        }
        //  MOB - must trim "/" from parts
        for (next = 0; (token = mprGetNextItem(route->tokens, &next)) != 0; ) {
            //  MOB - is next -1 right?
            httpSetFormVar(conn, token, mprGetItem(parts, next - 1));
        }
    }
    actionKey = pcre_replace(rx->pathInfo, route->compiledPattern, route->compiledAction, NULL, 0);
    return strim(actionKey, "/", MPR_TRIM_START);
}


static int parseEsp(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLoc     *loc;
    HttpHost    *host;
    HttpAlias   *alias;
    HttpDir     *dir, *parent;
    Esp         *esp;
    EspRoute    *route;
    char        *ekey, *evalue, *prefix, *path, *next, *methods, *prior;
    
    host = state->host;
    loc = state->loc;

    //  MOB - should not need to supply http on all these APIs
    esp = httpLookupStageData(MPR->httpService, "espHandler");
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
        prefix = httpReplaceReferences(host, prefix);
        path = httpMakePath(host, path);
        if (httpLookupDir(host, path) == 0) {
            parent = mprGetFirstItem(host->dirs);
            dir = httpCreateDir(path, parent);
            httpAddDir(host, dir);
        }
        alias = httpCreateAlias(prefix, path, 0);
        mprLog(4, "EspAlias \"%s\" for \"%s\"", prefix, path);
        httpAddAlias(host, alias);

        if (httpLookupLocation(host, prefix)) {
            mprError("Location already exists for \"%s\"", value);
            return MPR_ERR_BAD_SYNTAX;
        }
        loc = httpCreateInheritedLocation(state->loc);
        httpSetLocationPrefix(loc, prefix);
        httpSetLocationAlias(loc, alias);
        httpSetLocationAuth(loc, state->dir->auth);
        httpAddLocation(host, loc);
        httpSetHandler(loc, "espHandler");
        return 1;

    } else if (scasecmp(key, "EspCache") == 0) {
        esp->lifespan = ((MprTime) atoi(value)) * MPR_TICKS_PER_SEC;
        return 1;

    } else if (scasecmp(key, "EspCompile") == 0) {
        esp->compile = strim(value, "\"", MPR_TRIM_BOTH);
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

    } else if (scasecmp(key, "EspModules") == 0) {
        esp->modDir = sclone(value);
        return 1;

    } else if (scasecmp(key, "EspReload") == 0) {
        esp->reload = scasecmp(value, "on") == 0;
        return 1;

    } else if (scasecmp(key, "EspRoute") == 0) {
        if ((route = mprAllocObj(EspRoute, manageRoute)) == 0) {
            return MPR_ERR_MEMORY;
        }
        if (maGetConfigValue(&route->name, value, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        //  MOB - should maGetConfigValue do a clone?
        route->name = sclone(route->name);

        if (maGetConfigValue(&methods, next, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (maGetConfigValue(&route->pattern, next, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (route->pattern) {
            route->pattern = sclone(route->pattern);
        }
        maGetConfigValue(&route->action, next, &next, 1);
        if (route->action) {
            route->action = sclone(route->action);
        }
        maGetConfigValue(&route->module, next, &next, 1);
        if (route->module) {
            route->module = sclone(route->module);
        }
        if (scasecmp(methods, "ALL") == 0) {
            methods = "DELETE, GET, HEAD, OPTIONS, POST, PUT, TRACE";
        }
        if (prepRoute(esp, route, methods) < 0) {
            return MPR_ERR_MEMORY;
        }
        mprAddItem(esp->routes, route);
        return 1;
        
#if UNUSED
    } else if (scasecmp(key, "EspWorkers") == 0) {
        if ((stage = httpLookupStage(http, "espHandler")) == 0) {
            mprError("EspHandler is not configured");
            return MPR_ERR_BAD_SYNTAX;
        }
        loc->workers = atoi(value);
        return 1;
#endif
    }
    return 0;
}


static void manageEsp(Esp *esp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(esp->actions);
        mprMark(esp->routes);
        mprMark(esp->views);
        mprMark(esp->compile);
        mprMark(esp->modDir);
        mprMark(esp->env);
    }
}


static void manageReq(EspReq *req, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(req->entry);
        mprMark(req->path);
        mprMark(req->source);
        mprMark(req->module);
        mprMark(req->baseName);
        mprMark(req->cacheBuffer);
        mprMark(req->route);
        mprMark(req->esp);
    }
}


static void manageRoute(EspRoute *route, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(route->name);
        mprMark(route->methods);
        mprMark(route->pattern);
        mprMark(route->action);
        mprMark(route->module);
        mprMark(route->patternExpression);
        mprMark(route->compiledAction);
        mprMark(route->params);
        mprMark(route->tokens);

    } else if (flags & MPR_MANAGE_FREE) {
        free(route->compiledPattern);
    }
}


int maEspHandlerInit(Http *http)
{
    HttpStage       *handler;
    Esp             *esp;

    handler = httpCreateHandler(http, "espHandler", 
        HTTP_STAGE_QUERY_VARS | HTTP_STAGE_CGI_VARS | HTTP_STAGE_EXTRA_PATH | HTTP_STAGE_VIRTUAL, NULL);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if ((esp = mprAllocObj(Esp, manageEsp)) == 0) {
        return MPR_ERR_MEMORY;
    }
    handler->stageData = esp;
    handler->open = openEsp; 
    handler->start = startEsp; 
    handler->parse = (HttpParse) parseEsp; 

    if ((esp->actions = mprCreateHash(-1, MPR_HASH_STATIC_VALUES)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if ((esp->routes = mprCreateList(-1, 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if ((esp->views = mprCreateHash(-1, MPR_HASH_STATIC_VALUES)) == 0) {
        return MPR_ERR_MEMORY;
    }
#if DEBUG_IDE
    esp->modDir = mprGetAppDir();
#else
    esp->modDir = mprJoinPath(mprGetAppDir(), "../" BLD_LIB_NAME);
#endif
    if (!mprPathExists(esp->modDir, X_OK)) {
        esp->modDir = sclone(".");
    }
    esp->lifespan = ESP_LIFESPAN;
    esp->keepSource = 0;
    esp->reload = BLD_DEBUG;
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
