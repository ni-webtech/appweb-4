/*
    ejsHandler.c -- Ejscript language request handler for the Ejscript Web Framework.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "http.h"
#include    "appweb.h"
#include    "ejs.h"

#if BLD_FEATURE_EJS
/*********************************** Forwards *********************************/

static int loadStartupScript(Http *http, HttpLocation *location);

/************************************* Code ***********************************/
/*  
    Confirm the request match to the ejs handler.
 */
static bool matchEjs(HttpConn *conn, HttpStage *handler)
{
    HttpReceiver    *rec;
    MaAlias         *alias;
    char            *uri;

    rec = conn->receiver;
    alias = rec->alias;
    uri = rec->pathInfo;

    /*
        Set the scriptName to the alias prefix and remove from pathInfo
     */
    if (alias->prefixLen > 0) {
        uri = &uri[alias->prefixLen];
        if (*uri != '/' && uri[-1] == '/') {
            uri--;
        }
        rec->scriptName = alias->prefix;
        rec->pathInfo = (char*) uri;
    }
    return 1;
}


static void openEjs(HttpQueue *q)
{
    HttpConn        *conn;
    HttpLocation    *location;
    
    conn = q->conn;
    location = conn->receiver->location;
    if (location == 0 || location->context == 0) {
        if (loadStartupScript(conn->http, location) < 0) {
            //  MOB -- should this set an error somewhere?
            return;
        }
        if (location == 0 || location->context == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Undefined location context. Missing HttpServer.attach call.");
            //  MOB -- should this set an error somewhere?
            return;
        }
    }
}


static int parseEjs(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLocation    *location;
    char            *path;
    
    if (mprStrcmpAnyCase(key, "EjsStartup") == 0) {
        path = mprStrTrim(value, "\"");
        location = state->location;
        location->script = mprStrdup(location, path);
        return 1;
    }
    return 0;
}


/*
    Find a start script. Default to /usr/lib/PRODUCT/lib/PRODUCT.es.
 */
static char *findScript(MprCtx ctx, char *script)
{
    char        *base, *result;

    if (script == 0 || *script == '\0') {
        script = mprAsprintf(ctx, -1, "%s/../%s/%s", mprGetAppDir(ctx), BLD_LIB_NAME, MA_EJS_START);
    } else {
        if (!mprPathExists(ctx, script, R_OK)) {
            base = mprGetPathBase(ctx, script);
            script = mprAsprintf(ctx, -1, "%s/../%s/%s", mprGetAppDir(ctx), BLD_LIB_NAME, base);
            mprFree(base);
        }
    }
    if (mprPathExists(ctx, script, R_OK)) {
        result = mprGetNativePath(ctx, script);
        mprFree(script);
        return result;
    }
    return 0;
}


static int loadStartupScript(Http *http, HttpLocation *location)
{
    Ejs         *ejs;
    char        *script;
    int         ver;

    ejs = ejsCreateVm(http, NULL, NULL, NULL, EJS_FLAG_MASTER);
    if (ejs == 0) {
        return 0;
    }
    ejs->location = location;
    ver = 0;
    if (ejsLoadModule(ejs, "ejs.web", ver, ver, 0) < 0) {
        mprError(ejs, "Can't load ejs.web.mod: %s", ejsGetErrorMsg(ejs, 1));
        return MPR_ERR_CANT_INITIALIZE;
    }
    if ((script = findScript(http, location->script)) == 0) {
        mprError(http, "Can't find script file %s", location->script);
        return MPR_ERR_CANT_OPEN;
    }
    if (ejsLoadScriptFile(ejs, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
        mprError(ejs, "Can't load \"%s\"", script);
    } else {
        LOG(http, 2, "Loading Ejscript Server script: \"%s\"", script);
    }
    mprFree(script);
    return 0;
}


#if VXWORKS
/*
    Create a routine to pull in the GCC support routines for double and int64 manipulations for some platforms. Do this
    incase modules reference these routines. Without this, the modules have to reference them. Which leads to multiple 
    defines if two modules include them. (Code to pull in moddi3, udivdi3, umoddi3)
 */
double  __ejsAppweb_floating_point_resolution(double a, double b, int64 c, int64 d, uint64 e, uint64 f) {
    a = a / b; a = a * b; c = c / d; c = c % d; e = e / f; e = e % f;
    c = (int64) a; d = (uint64) a; a = (double) c; a = (double) e;
    return (a == b) ? a : b;
}
#endif


/*  
    Ejs handler initialization
 */
int maEjsHandlerInit(Http *http, MprModule *mp)
{
    HttpStage   *handler;
    EjsService  *sp;

    sp = ejsCreateService(http);
    sp->http = http;
    ejsInitCompiler(sp);

    /*
        Most of the Ejs handler is in ejsLib.c. Search for ejsAddWebHandler. Augment the handler with match, open
        and parse callbacks.
     */
    if ((handler = ejsAddWebHandler(http)) != 0) {
        handler->match = matchEjs;
        handler->open = openEjs;
        handler->parse = (HttpParse) parseEjs;
    }
    return 0;
}

#else
int maEjsHandlerInit(Http *http, MprModule *mp)
{
    return 0;
}

#endif /* BLD_FEATURE_EJS */

/*
    @copy   default
 
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

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

    @end
 */
