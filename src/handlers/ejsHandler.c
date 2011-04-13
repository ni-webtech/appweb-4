/*
    ejsHandler.c -- Ejscript language request handler for the Ejscript Web Framework.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "http.h"
#include    "appweb.h"

#if BLD_FEATURE_EJS

#include    "ejs.h"

/*********************************** Forwards *********************************/

static int loadStartupScript(Ejs *ejs, HttpConn *conn, cchar *script);

/************************************* Code ***********************************/

static void setScriptName(HttpConn *conn)
{
    HttpRx      *rx;
    HttpAlias   *alias;
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
        rx->pathInfo = sclone(uri);
        mprLog(5, "ejs: set script name: \"%s\", pathInfo: \"%s\"", rx->scriptName, rx->pathInfo);
    }
}


static void openEjs(HttpQueue *q)
{
    HttpConn    *conn;
    HttpLoc     *loc;
    Ejs         *ejs;
    
    conn = q->conn;
    setScriptName(conn);

    loc = conn->rx->loc;
    if (loc == 0 || loc->context == 0) {
        /*
            On-demand loading of the startup script
         */
        mprLog(5, "ejs: create ejs interpreter");
        if ((ejs = ejsCreate(NULL, NULL, NULL, 0, NULL, 0)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create Ejs interpreter.");
            return;
        }
        q->stage->stageData = ejs;
        ejs->loc = loc;
        if (loadStartupScript(ejs, conn, loc->script) < 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load startup script.");
            return;
        }
        if (loc == 0 || loc->context == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Undefined location context. Check EjsStartup script.");
            return;
        }
    }
}


static int parseEjs(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLoc     *loc;
    
    if (scasecmp(key, "EjsStartup") == 0) {
        loc = state->loc;
        loc->script = strim(value, "\"", MPR_TRIM_BOTH);
        return 1;
    }
    return 0;
}


/*
    Find a start script. Default to /usr/lib/PRODUCT/lib/PRODUCT.es.
 */
static char *findScript(cchar *script)
{
    char    *base;

    if (script == 0 || *script == '\0') {
        script = mprAsprintf("%s/../%s/%s", mprGetAppDir(), BLD_LIB_NAME, MA_EJS_START);
    } else {
        if (!mprPathExists(script, R_OK)) {
            base = mprGetPathBase(script);
            script = mprAsprintf("%s/../%s/%s", mprGetAppDir(), BLD_LIB_NAME, base);
        }
    }
    if (mprPathExists(script, R_OK)) {
        return mprGetNativePath(script);
    }
    return 0;
}


static int loadStartupScript(Ejs *ejs, HttpConn *conn, cchar *script)
{
    char    *path;
    int     ver;

    ver = 0;
    if (ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, "ejs.web"), ver, ver, 0) < 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load ejs.web.mod: %s", ejsGetErrorMsg(ejs, 1));
        return MPR_ERR_CANT_INITIALIZE;
    }
    if ((path = findScript(script)) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find ejs start script file \"%s\"", script);
        return MPR_ERR_CANT_OPEN;
    }
    LOG(2, "Loading Ejscript Server script: \"%s\"", path);
    if (ejsLoadScriptFile(ejs, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load \"%s\"\n%s", path, ejsGetErrorMsg(ejs, 1));
        return MPR_ERR_CANT_OPEN;
    }
    return 0;
}


#if VXWORKS
//  MOB - is this still needed?
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
    Ejs module and handler initialization
 */
int maEjsHandlerInit(Http *http, MprModule *module)
{
    HttpStage   *handler;

    /*
        Most of the Ejs handler is in ejsLib.c. Augment the handler with match, open and parse callbacks.
     */
    if ((handler = ejsAddWebHandler(http, module)) != 0) {
        handler->open = openEjs;
        handler->parse = (HttpParse) parseEjs;
    }
    return 0;
}

#else
int maEjsHandlerInit(Http *http, MprModule *module)
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
