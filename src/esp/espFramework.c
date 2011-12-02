/*
    espFramework.c -- ESP Web Framework API

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"

#if BLD_FEATURE_ESP

/************************************* Code ***********************************/
/*  
    Add a http header if not already defined
 */
void espAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAddHeaderString(conn, key, sfmt(fmt, vargs));
    va_end(vargs);
}


/*
    Add a header string if not already defined
 */
void espAddHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    httpAddHeaderString(conn, key, value);
}


/* 
   Append a header. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
void espAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAppendHeaderString(conn, key, sfmt(fmt, vargs));
    va_end(vargs);
}


/* 
   Append a header string. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
void espAppendHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    httpAppendHeaderString(conn, key, value);
}


void espAutoFinalize(HttpConn *conn) 
{
    EspReq  *req;

    req = conn->data;
    if (req->autoFinalize) {
        httpFinalize(conn);
    }
}


void espManageAction(EspAction *ap, int flags)
{
}


int espCache(HttpRoute *route, cchar *uri, int lifesecs, int flags)
{
    httpAddCache(route, NULL, uri, NULL, NULL, 0, lifesecs * MPR_TICKS_PER_SEC, flags);
    return 0;
}


bool espCheckSecurityToken(HttpConn *conn) 
{
    HttpRx  *rx;
    cchar   *securityTokenName, *sessionToken;

    rx = conn->rx;
    if (!(rx->flags & HTTP_POST)) {
        return 1;
    }
    if (rx->securityToken == 0) {
        sessionToken = rx->securityToken = sclone(espGetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, ""));
        securityTokenName = espGetParam(conn, ESP_SECURITY_TOKEN_NAME, "");
        if (!smatch(sessionToken, securityTokenName)) {
            httpError(conn, HTTP_CODE_NOT_ACCEPTABLE, 
                "Security token does not match. Potential CSRF attack. Denying request");
            return 0;
        }
    }
    return 1;
}


EdiRec *espCreateRec(HttpConn *conn, cchar *tableName, MprHash *params)
{
    Edi         *edi;
    EdiRec      *rec;

    edi = espGetDatabase(conn);
    if ((rec = ediCreateRec(edi, tableName)) == 0) {
        return 0;
    }
    ediSetFields(rec, params);
    return espSetRec(conn, rec);
}


void espDefineAction(HttpRoute *route, cchar *target, void *actionProc)
{
    EspAction   *action;
    EspRoute    *eroute;
    Esp         *esp;

    mprAssert(route);
    mprAssert(target && *target);
    mprAssert(actionProc);

    esp = MPR->espService;
    eroute = route->eroute;
    if ((action = mprAllocObj(EspAction, espManageAction)) == 0) {
        return;
    }
    action->actionProc = actionProc;
    if (target) {
        mprAddKey(esp->actions, mprJoinPath(eroute->controllersDir, target), action);
    }
}


void espDefineBase(HttpRoute *route, EspProc baseProc)
{
    ((EspRoute*) route->eroute)->controllerBase = baseProc;
}


/*
    Path should be an app-relative path to the view file (relative-path.esp)
 */
void espDefineView(HttpRoute *route, cchar *path, void *view)
{
    Esp         *esp;

    mprAssert(route);
    mprAssert(path && *path);
    mprAssert(view);

    esp = MPR->espService;
	path = mprGetPortablePath(mprJoinPath(route->dir, path));
    mprAddKey(esp->views, path, view);
}


void espFinalize(HttpConn *conn) 
{
    httpFinalize(conn);
}


void espFlush(HttpConn *conn) 
{
    httpFlush(conn);
}


MprList *espGetColumns(HttpConn *conn, EdiRec *rec)
{
    if (rec == 0) {
        rec = conn->record;
    }
    if (rec) {
        return ediGetColumns(espGetDatabase(conn), rec->tableName);
    }
    return mprCreateList(0, 0);
}


MprOff espGetContentLength(HttpConn *conn)
{
    return httpGetContentLength(conn);
}


cchar *espGetContentType(HttpConn *conn)
{
    return conn->rx->mimeType;
}


//  MOB - need an API to parse these into a hash
cchar *espGetCookies(HttpConn *conn) 
{
    return httpGetCookies(conn);
}


Edi *espGetDatabase(HttpConn *conn)
{
    EspRoute    *eroute;
    EspReq      *req;

    req = conn->data;
    if (req == 0) {
        return 0;
    }
    eroute = req->eroute;
    if (eroute == 0 || eroute->edi == 0) {
        httpError(conn, 0, "Can't get database instance");
        return 0;
    }
    return eroute->edi;
}


EspRoute *espGetEspRoute(HttpConn *conn)
{
    EspReq      *req;

    if ((req = conn->data) == 0) {
        return 0;
    }
    return req->eroute;
}


cchar *espGetDir(HttpConn *conn)
{   
    return conn->rx->route->dir;
}


cchar *espGetFlashMessage(HttpConn *conn, cchar *kind)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *msg;
   
    req = conn->data;
    if (kind == 0 || req->flash == 0 || mprGetHashLength(req->flash) == 0) {
        return 0;
    }
    for (kp = 0; (kp = mprGetNextKey(req->flash, kp)) != 0; ) {
        msg = kp->data;
        if (smatch(kind, kp->key) || smatch(kind, "all")) {
            return msg;
        }
    }
    return 0;
}


EdiGrid *espGetGrid(HttpConn *conn)
{           
    return conn->grid;
}

    
cchar *espGetHeader(HttpConn *conn, cchar *key)
{
    return httpGetHeader(conn, key);
}


MprHash *espGetHeaderHash(HttpConn *conn)
{
    return httpGetHeaderHash(conn);
}


char *espGetHeaders(HttpConn *conn)
{
    return httpGetHeaders(conn);
}

            
cchar *espGetMethod(HttpConn *conn)
{   
    return conn->rx->method;
} 


cchar *espGetParam(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    return httpGetParam(conn, var, defaultValue);
}


MprHash *espGetParams(HttpConn *conn)
{
    return httpGetParams(conn);
}


int espGetIntParam(HttpConn *conn, cchar *var, int defaultValue)
{
    return httpGetIntParam(conn, var, defaultValue);
}


cchar *espGetQueryString(HttpConn *conn)
{
    return httpGetQueryString(conn);
}


char *espGetReferrer(HttpConn *conn)
{
    if (conn->rx->referrer) {
        return conn->rx->referrer;
    }
    return espGetTop(conn);
}


Edi *espGetRouteDatabase(HttpRoute *route)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    if (eroute == 0 || eroute->edi == 0) {
        return 0;
    }
    return eroute->edi;
}


//  MOB - is this only for clients?
int espGetStatus(HttpConn *conn)
{
    return httpGetStatus(conn);
}


//  MOB - is this only for clients?
char *espGetStatusMessage(HttpConn *conn)
{
    return httpGetStatusMessage(conn);
}


char *espGetTop(HttpConn *conn)
{
    return httpLink(conn, "~", NULL);
}


MprHash *espGetUploads(HttpConn *conn)
{
    return conn->rx->files;
}


cchar *espGetUri(HttpConn *conn)
{
    return conn->rx->uri;
}


bool espHasGrid(HttpConn *conn)
{
    return conn->grid != 0;
}


bool espHasRec(HttpConn *conn)
{
    EdiRec  *rec;

    rec = conn->record;
    return (rec && rec->id) ? 1 : 0;
}


bool espIsEof(HttpConn *conn)
{
    return httpIsEof(conn);
}


bool espIsFinalized(HttpConn *conn) 
{
    return httpIsFinalized(conn);
}


bool espIsSecure(HttpConn *conn)
{
    return conn->secure;
}


/*
    grid = makeGrid("[ \
        { id: '1', country: 'Australia' }, \
        { id: '2', country: 'China' }, \
    ]");
 */
EdiGrid *espMakeGrid(cchar *contents)
{
    return ediMakeGrid(contents);
}


MprHash *espMakeHash(cchar *fmt, ...)
{
    va_list     args;
    cchar       *str;

    va_start(args, fmt);
    str = sfmtv(fmt, args);
    va_end(args);
    return mprDeserialize(str);
}


/*
    rec = makeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
 */
EdiRec *espMakeRec(cchar *contents)
{
    return ediMakeRec(contents);
}


bool espMatchParam(HttpConn *conn, cchar *var, cchar *value)
{
    return httpMatchParam(conn, var, value);
}


ssize espReceive(HttpConn *conn, char *buf, ssize len)
{
    return httpRead(conn, buf, len);
}


EdiRec *espReadRecWhere(HttpConn *conn, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return espSetRec(conn, ediReadOneWhere(espGetDatabase(conn), tableName, fieldName, operation, value));
}


EdiRec *espReadRec(HttpConn *conn, cchar *tableName)
{
    return espSetRec(conn, ediReadRec(espGetDatabase(conn), tableName, espGetParam(conn, "id", NULL)));
}


EdiRec *espReadRecByKey(HttpConn *conn, cchar *tableName, cchar *key)
{
    return espSetRec(conn, ediReadRec(espGetDatabase(conn), tableName, key));
}


EdiGrid *espReadRecsWhere(HttpConn *conn, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return espSetGrid(conn, ediReadWhere(espGetDatabase(conn), tableName, fieldName, operation, value));
}


EdiGrid *espReadTable(HttpConn *conn, cchar *tableName)
{
    EdiGrid *grid;
    
    grid = ediReadWhere(espGetDatabase(conn), tableName, 0, 0, 0);
    espSetGrid(conn, grid);
    return grid;
}


void espRedirect(HttpConn *conn, int status, cchar *target)
{
    //  MOB - should this httpLink be pushed into httpRedirect?
    httpRedirect(conn, status, httpLink(conn, target, NULL));
}


void espRedirectBack(HttpConn *conn)
{
    if (conn->rx->referrer) {
        espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, conn->rx->referrer); 
    }
}


bool espRemoveRec(HttpConn *conn, cchar *tableName, cchar *key)
{
    if (ediDeleteRow(espGetDatabase(conn), tableName, key) < 0) {
        return 0;
    }
    return 1;
}

ssize espRender(HttpConn *conn, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;

    va_start(vargs, fmt);
    buf = sfmtv(fmt, vargs);
    va_end(vargs);
    return espRenderString(conn, buf);
}
    

ssize espRenderBlock(HttpConn *conn, cchar *buf, ssize size)
{
    return httpWriteBlock(conn->writeq, buf, size);
}


//  MOB - need a renderCached(), updateCache()
ssize espRenderCached(HttpConn *conn)
{
    return httpWriteCached(conn);
}


ssize espRenderError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;
    HttpRx      *rx;
    EspReq      *req;
    EspRoute    *eroute;
    ssize       written;
    cchar       *msg, *title, *text;

    va_start(args, fmt);    

    rx = conn->rx;
    req = conn->data;
    eroute = req->eroute;
    written = 0;

    if (!httpIsFinalized(conn)) {
        if (status == 0) {
            status = HTTP_CODE_INTERNAL_SERVER_ERROR;
        }
        title = sfmt("Request Error for \"%s\"", rx->pathInfo);
        msg = mprEscapeHtml(sfmtv(fmt, args));
        if (eroute->showErrors) {
            text = sfmt(\
                "<!DOCTYPE html>\r\n<html>\r\n<head><title>%s</title></head>\r\n" \
                "<body>\r\n<h1>%s</h1>\r\n" \
                "    <pre>%s</pre>\r\n" \
                "    <p>To prevent errors being displayed in the browser, " \
                "       set <b>log.showErrors</b> to false in the ejsrc file.</p>\r\n", \
                "</body>\r\n</html>\r\n", title, title, msg);
            httpSetHeader(conn, "Content-Type", "text/html");
            written += espRenderString(conn, text);
            espFinalize(conn);
            mprLog(4, "Request error (%d) for: \"%s\"", status, rx->pathInfo);
        }
    }
    va_end(args);    
    return written;
}


ssize espRenderFile(HttpConn *conn, cchar *path)
{
    MprFile     *from;
    ssize       count, written, nbytes;
    char        buf[MPR_BUFSIZE];

    if ((from = mprOpenFile(path, O_RDONLY | O_BINARY, 0)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    written = 0;
    while ((count = mprReadFile(from, buf, sizeof(buf))) > 0) {
        if ((nbytes = espRenderBlock(conn, buf, count)) < 0) {
            return nbytes;
        }
        written += nbytes;
    }
    mprCloseFile(from);
    return written;
}


ssize espRenderParam(HttpConn *conn, cchar *name)
{
    cchar   *value;

    if ((value = espGetParam(conn, name, 0)) == 0) {
        value = espGetSessionVar(conn, name, "");
    }
    return espRenderSafeString(conn, value);
}


ssize espRenderSafeString(HttpConn *conn, cchar *s)
{
    s = mprEscapeHtml(s);
    return espRenderBlock(conn, s, slen(s));
}


ssize espRenderString(HttpConn *conn, cchar *s)
{
    return espRenderBlock(conn, s, slen(s));
}


int espRemoveHeader(HttpConn *conn, cchar *key)
{
    mprAssert(key && *key);
    if (conn->tx == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return mprRemoveKey(conn->tx->headers, key);
}


bool espSetAutoFinalizing(HttpConn *conn, bool on) 
{
    EspReq  *req;
    bool    old;

    req = conn->data;
    old = req->autoFinalize;
    req->autoFinalize = on;
    return old;
}


void espSetContentLength(HttpConn *conn, MprOff length)
{
    httpSetContentLength(conn, length);
}


void espSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, MprTime lifespan, 
        bool isSecure)
{
    httpSetCookie(conn, name, value, path, cookieDomain, lifespan, isSecure);
}


void espSetContentType(HttpConn *conn, cchar *mimeType)
{
    httpSetContentType(conn, mimeType);
}


EdiRec *espSetField(EdiRec *rec, cchar *fieldName, cchar *value)
{
    return ediSetField(rec, fieldName, value);
}


EdiRec *espSetFields(EdiRec *rec, MprHash *params)
{
    return ediSetFields(rec, params);
}


void espSetFlash(HttpConn *conn, cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFlashv(conn, kind, fmt, args);
    va_end(args);
}


void espSetFlashv(HttpConn *conn, cchar *kind, cchar *fmt, va_list args)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *prior, *msg;

    req = conn->data;
    msg = sfmtv(fmt, args);

    if (req->flash == 0) {
        req->flash = mprCreateHash(0, 0);
        espGetSession(conn, 1);
    }
    if ((prior = mprLookupKey(req->flash, kind)) != 0) {
        kp = mprAddKey(req->flash, kind, sjoin(prior, "\n", msg, NULL));
    } else {
        kp = mprAddKey(req->flash, kind, sclone(msg));
    }
    if (kp) {
        kp->type = MPR_JSON_STRING;
    }
}


EdiGrid *espSetGrid(HttpConn *conn, EdiGrid *grid)
{
    conn->grid = grid;
    return grid;
}


/*  
    Set a http header. Overwrite if present.
 */
void espSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    httpSetHeader(conn, key, sfmt(fmt, vargs));
    va_end(vargs);
}


void espSetHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    httpSetHeaderString(conn, key, value);
}


void espSetIntParam(HttpConn *conn, cchar *var, int value) 
{
    httpSetIntParam(conn, var, value);
}


void espSetParam(HttpConn *conn, cchar *var, cchar *value) 
{
    httpSetParam(conn, var, value);
}


EdiRec *espSetRec(HttpConn *conn, EdiRec *rec)
{
    return conn->record = rec;
}


void espSetStatus(HttpConn *conn, int status)
{
    httpSetStatus(conn, status);
}


/*
    Convert queue data in key / value pairs
    MOB - should be able to remove this and use standard form parsing
 */
static int getParams(char ***keys, char *buf, int len)
{
    char**  keyList;
    char    *eq, *cp, *pp, *tok;
    int     i, keyCount;

    *keys = 0;

    /*
        Change all plus signs back to spaces
     */
    keyCount = (len > 0) ? 1 : 0;
    for (cp = buf; cp < &buf[len]; cp++) {
        if (*cp == '+') {
            *cp = ' ';
        } else if (*cp == '&' && (cp > buf && cp < &buf[len - 1])) {
            keyCount++;
        }
    }
    if (keyCount == 0) {
        return 0;
    }

    /*
        Crack the input into name/value pairs 
     */
    keyList = mprAlloc((keyCount * 2) * sizeof(char**));
    i = 0;
    tok = 0;
    for (pp = stok(buf, "&", &tok); pp; pp = stok(0, "&", &tok)) {
        if ((eq = strchr(pp, '=')) != 0) {
            *eq++ = '\0';
            pp = mprUriDecode(pp);
            eq = mprUriDecode(eq);
        } else {
            pp = mprUriDecode(pp);
            eq = 0;
        }
        if (i < (keyCount * 2)) {
            keyList[i++] = pp;
            keyList[i++] = eq;
        }
    }
    *keys = keyList;
    return keyCount;
}


void espShowRequest(HttpConn *conn)
{
    MprHash     *env;
    MprKey      *kp;
    MprBuf      *buf;
    HttpRx      *rx;
    HttpQueue   *q;
    cchar       *query;
    char        qbuf[MPR_MAX_STRING], **keys, *value;
    int         i, numKeys;

    rx = conn->rx;
    httpSetHeaderString(conn, "Cache-Control", "no-cache");
    httpCreateCGIParams(conn);
    espRender(conn, "\r\n");

    /*
        Query
     */
    if ((query = espGetQueryString(conn)) != 0) {
        scopy(qbuf, sizeof(qbuf), query);
        if ((numKeys = getParams(&keys, qbuf, (int) strlen(qbuf))) > 0) {
            for (i = 0; i < (numKeys * 2); i += 2) {
                value = keys[i+1];
                espRender(conn, "QUERY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espRender(conn, "\r\n");
    }

    /*
        Http Headers
     */
    env = espGetHeaderHash(conn);
    for (ITERATE_KEYS(env, kp)) {
        espRender(conn, "HEADER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espRender(conn, "\r\n");

    /*
        Server vars
     */
    for (ITERATE_KEYS(conn->rx->svars, kp)) {
        espRender(conn, "SERVER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espRender(conn, "\r\n");

    /*
        Form vars
     */
    if ((env = espGetParams(conn)) != 0) {
        for (ITERATE_KEYS(env, kp)) {
            espRender(conn, "FORM %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
        }
        espRender(conn, "\r\n");
    }

    /*
        Body
     */
    q = conn->readq;
    if (q->first && rx->bytesRead > 0 && scmp(rx->mimeType, "application/x-www-form-urlencoded") == 0) {
        buf = q->first->content;
        mprAddNullToBuf(buf);
        if ((numKeys = getParams(&keys, mprGetBufStart(buf), (int) mprGetBufLength(buf))) > 0) {
            for (i = 0; i < (numKeys * 2); i += 2) {
                value = keys[i+1];
                espRender(conn, "BODY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espRender(conn, "\r\n");
    }
}


void espUpdateCache(HttpConn *conn, cchar *uri, cchar *data, int lifesecs)
{
    httpUpdateCache(conn, uri, data, lifesecs * MPR_TICKS_PER_SEC);
}


bool espUpdateField(HttpConn *conn, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    return ediUpdateField(espGetDatabase(conn), tableName, key, fieldName, value) == 0;
}


bool espUpdateFields(HttpConn *conn, cchar *tableName, MprHash *params)
{
    EdiRec  *rec;

    if ((rec = espSetFields(espReadRec(conn, tableName), params)) == 0) {
        return 0;
    }
    return ediUpdateRec(espGetDatabase(conn), rec) == 0;
}


bool espUpdateRec(HttpConn *conn, EdiRec *rec)
{
    return ediUpdateRec(rec->edi, rec) == 0;
}


cchar *espUri(HttpConn *conn, cchar *target)
{
    return httpLink(conn, target, 0);
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
