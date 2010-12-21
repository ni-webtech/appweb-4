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

static int loadStartupScript(Http *http, HttpLoc *loc);

/************************************* Code ***********************************/

static bool matchEjs(HttpConn *conn, HttpStage *handler)
{
    HttpRx      *rx;
    MaAlias     *alias;
    char        *uri;

    rx = conn->rx;
    alias = rx->alias;
    uri = rx->pathInfo;

    /*
        Set the scriptName to the alias prefix and remove from pathInfo
     */
    if (alias->prefixLen > 0) {
        uri = &uri[alias->prefixLen];
        if (*uri != '/' && uri[-1] == '/') {
            uri--;
        }
        rx->scriptName = alias->prefix;
        rx->pathInfo = (char*) uri;
    }
    return 1;
}


static void openEjs(HttpQueue *q)
{
    HttpConn    *conn;
    HttpLoc     *loc;
    
    conn = q->conn;
    loc = conn->rx->loc;
    if (loc == 0 || loc->context == 0) {
        /*
            On-demand loading of the startup script
         */
        if (loadStartupScript(conn->http, loc) < 0) {
            //  MOB -- should this set an error somewhere?
            return;
        }
        if (loc == 0 || loc->context == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Undefined location context. Check EjsStartup script.");
            //  MOB -- should this set an error somewhere?
            return;
        }
    }
}


static int parseEjs(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLoc     *loc;
    char        *path;
    
    if (scasecmp(key, "EjsStartup") == 0) {
        path = strim(value, "\"", MPR_TRIM_BOTH);
        loc = state->loc;
        loc->script = sclone(path);
        return 1;
    }
    return 0;
}


/*
    Find a start script. Default to /usr/lib/PRODUCT/lib/PRODUCT.es.
 */
static char *findScript(char *script)
{
    char    *base, *result;

    if (script == 0 || *script == '\0') {
        script = mprAsprintf("%s/../%s/%s", mprGetAppDir(), BLD_LIB_NAME, MA_EJS_START);
    } else {
        if (!mprPathExists(script, R_OK)) {
            base = mprGetPathBase(script);
            script = mprAsprintf("%s/../%s/%s", mprGetAppDir(), BLD_LIB_NAME, base);
        }
    }
    if (mprPathExists(script, R_OK)) {
        result = mprGetNativePath(script);
        return result;
    }
    return 0;
}


static int loadStartupScript(Http *http, HttpLoc *loc)
{
    Ejs     *ejs;
    char    *script;
    int     ver;

    ejs = ejsCreateVm(NULL, NULL, 0, NULL, 0);
    if (ejs == 0) {
        return 0;
    }
    ejs->loc = loc;
    ver = 0;
    if (ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, "ejs.web"), ver, ver, 0) < 0) {
        mprError("Can't load ejs.web.mod: %s", ejsGetErrorMsg(ejs, 1));
        return MPR_ERR_CANT_INITIALIZE;
    }
    if ((script = findScript(loc->script)) == 0) {
        mprError("Can't find script file %s", loc->script);
        return MPR_ERR_CANT_OPEN;
    }
    LOG(2, "Loading Ejscript Server script: \"%s\"", script);
    if (ejsLoadScriptFile(ejs, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
        mprError("Can't load \"%s\"", script);
    }
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
        Most of the Ejs handler is in ejsLib.c. Augment the handler with match, open and parse callbacks.
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
