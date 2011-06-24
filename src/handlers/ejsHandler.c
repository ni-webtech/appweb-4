/*
    ejsHandler.c -- Ejscript language request handler for the Ejscript Web Framework.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "http.h"
#include    "appweb.h"

#if BLD_FEATURE_EJS
#include    "ejs.h"

/************************************* Data ***********************************/

static char startup[] = "\
    require ejs.web; \
    let server: HttpServer = new HttpServer; \
    var router = Router(Router.Top); \
    server.on('readable', function (event, request) { \
        server.serve(request, router) \
    }); \
    server.listen(); \
";

/************************************* Code ***********************************/

static void openEjs(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpAlias   *alias;
    HttpLoc     *loc;
    Ejs         *ejs;
    EjsPool     *pool;
    char        *poolScript, *uri;
    
    conn = q->conn;
    rx = conn->rx;
    alias = rx->alias;
    uri = rx->pathInfo;
    loc = rx->loc;

    if (!conn->ejs) {
        if (!loc->context) {
            if (loc->script == 0) {
                loc->script = sclone(startup);
            }
            if (loc->workers < 0) {
                loc->workers = mprGetMaxWorkers();
            }
            poolScript = sfmt("require ejs.web; global.ejs::HttpServerHome = '%s'; global.ejs::HttpServerDocuments = '%s';",
                alias->filename, alias->filename);
            loc->context = ejsCreatePool(loc->workers, poolScript, loc->script, loc->scriptPath);
            mprLog(5, "ejs: Demand load Ejscript web framework");
        }
        //  MOB - remove conn->pool and store in ejs->pool
        pool = conn->pool = loc->context;
        if ((ejs = ejsAllocPoolVM(pool, EJS_FLAG_HOSTED)) == 0) {
            httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't create Ejs interpreter");
            return;
        }
        conn->ejs = ejs;
        ejs->hosted = 1;
    }
    /*
        Set the scriptName to the alias prefix and remove from pathInfo
     */
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


static int parseEjs(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpStage   *stage;
    HttpLoc     *loc;
    HttpHost    *host;
    HttpAlias   *alias;
    HttpDir     *dir, *parent;
    char        *prefix, *path, *script, *next, *workers;
    
    loc = state->loc;
    host = state->host;

    if (scasecmp(key, "EjsAlias") == 0) {
        /*
            EjsAlias prefix [path] [script]
         */
        if (maGetConfigValue(&prefix, value, &next, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (maGetConfigValue(&path, next, &next, 1) < 0) {
            path = ".";
        }
        maGetConfigValue(&script, next, &next, 1);
        maGetConfigValue(&workers, next, &next, 1);

        prefix = httpReplaceReferences(host, prefix);
        path = httpMakePath(host, path);
        dir = httpLookupDir(host, path);
        if (script) {
            script = strim(script, "\"", MPR_TRIM_BOTH);
        }
        if (httpLookupDir(host, path) == 0) {
            parent = mprGetFirstItem(host->dirs);
            dir = httpCreateDir(path, parent);
        }
        alias = httpCreateAlias(prefix, path, 0);
        mprLog(4, "EjsAlias \"%s\" for \"%s\"", prefix, path);
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
        httpSetLocationScript(loc, 0, script);
        if (workers) {
            httpSetLocationWorkers(loc, atoi(workers));
        }
        httpSetHandler(loc, "ejsHandler");
        return 1;

    } else if (scasecmp(key, "EjsStartup") == 0) {
        loc->scriptPath = strim(value, "\"", MPR_TRIM_BOTH);
        return 1;

    } else if (scasecmp(key, "EjsWorkers") == 0) {
        if ((stage = httpLookupStage(http, "ejsHandler")) == 0) {
            mprError("EjsHandler is not configured");
            return MPR_ERR_BAD_SYNTAX;
        }
        loc->workers = atoi(value);
        return 1;
    }
    return 0;
}


int maEjsHandlerInit(Http *http, MprModule *module)
{
    HttpStage   *stage;

    /*
        Most of the Ejs handler is in ejsLib.c. Augment the handler with match, open and parse callbacks.
     */
    if ((stage = ejsAddWebHandler(http, module)) != 0) {
        stage->open = openEjs;
        stage->parse = (HttpParse) parseEjs;
    }
    return 0;
}
#endif /* BLD_FEATURE_EJS */

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
