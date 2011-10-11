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
    if (req->autoFinalize && !req->finalized) {
        req->finalized = 1;
        if (req->cacheBuffer) {
            httpSetResponded(conn);
        } else  {
            httpFinalize(conn);
        }
    }
}


static void manageAction(EspAction *ap, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ap->cacheUri);
    }
}


void espCacheControl(EspRoute *eroute, cchar *target, int lifesecs, cchar *uri)
{
    EspAction  *action;
    Esp         *esp;

    esp = MPR->espService;
    if ((action = mprLookupKey(esp->actions, mprJoinPath(eroute->controllersDir, target))) == 0) {
        if ((action = mprAllocObj(EspAction, manageAction)) == 0) {
            return;
        }
    }
    if (uri) {
        action->cacheUri = sclone(uri);
    }
    if (lifesecs <= 0) {
        action->lifespan = eroute->lifespan;
    } else {
        action->lifespan = lifesecs * MPR_TICKS_PER_SEC;
    }
}


bool espCheckSecurityToken(HttpConn *conn) 
{
    HttpRx  *rx;
    EspReq  *req;
    cchar   *securityToken, *sessionToken;

    rx = conn->rx;
    req = conn->data;

    if (!(rx->flags & HTTP_POST)) {
        return 1;
    }
    if (rx->securityToken == 0) {
        rx->securityToken = (char*) espGetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, "");
        sessionToken = rx->securityToken;
        securityToken = espGetParam(conn, ESP_SECURITY_TOKEN_NAME, "");
        if (!smatch(sessionToken, securityToken)) {
            httpError(conn, HTTP_CODE_NOT_ACCEPTABLE, 
                "Security token does not match. Potential CSRF attack. Denying request");
            return 0;
        }
    }
    return 1;
}


void espDefineAction(EspRoute *eroute, cchar *target, void *actionProc)
{
    EspAction   *action;
    Esp         *esp;

    mprAssert(eroute);
    mprAssert(target && *target);
    mprAssert(actionProc);

    esp = MPR->espService;
    if ((action = mprAllocObj(EspAction, manageAction)) == 0) {
        return;
    }
    action->actionProc = actionProc;
    mprAddKey(esp->actions, mprJoinPath(eroute->controllersDir, target), action);
}


void espDefineBase(EspRoute *eroute, EspProc baseProc)
{
    eroute->controllerBase = baseProc;
}


/*
    Path should be an app-relative path to the view file (relative-path.esp)
 */
void espDefineView(EspRoute *eroute, cchar *path, void *view)
{
    Esp         *esp;

    mprAssert(eroute);
    mprAssert(path && *path);
    mprAssert(view);

    esp = MPR->espService;
	path = mprGetPortablePath(mprJoinPath(eroute->dir, path));
    mprAddKey(esp->views, path, view);
}


void espFinalize(HttpConn *conn) 
{
    EspReq     *req;
    
    req = conn->data;
    if (req->cacheBuffer) {
        httpSetResponded(conn);
    } else {
        httpFinalize(conn);
    }
    req->finalized = 1;
}


void espFlush(HttpConn *conn) 
{
    httpFlush(conn);
}


MprOff espGetContentLength(HttpConn *conn)
{
    return httpGetContentLength(conn);
}


cchar *espGetCookies(HttpConn *conn) 
{
    return httpGetCookies(conn);
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


MprHash *espGetParams(HttpConn *conn)
{
    return httpGetParams(conn);
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


char *espGetHome(HttpConn *conn)
{
    HttpRx      *rx;
    cchar       *path, *end, *sp;
    char        *home, *cp;
    int         levels;

    //  MOB - should this just use the route.home.  ie. httpLink(conn, "~", NULL);
    if (conn == NULL) {
        return sclone("/");
    }
    rx = conn->rx;
    mprAssert(rx->pathInfo);

    path = rx->pathInfo;
    end = &path[strlen(path)];
    for (levels = 1, sp = &path[1]; sp < end; sp++) {
        if (*sp == '/' && sp[-1] != '/') {
            levels++;
        }
    }
    home = mprAlloc(levels * 3 + 2);
    if (levels) {
        for (cp = home; levels > 0; levels--) {
            strcpy(cp, "../");
            cp += 3;
        }
        *cp = '\0';
    } else {
        strcpy(home, "./");
    }
    return home;
}


cchar *espGetParam(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    return httpGetParam(conn, var, defaultValue);
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
    return espGetHome(conn);
}


/*
    Get a security token. This will use and existing token or create if not present. It will store in the session store.
 */
cchar *espGetSecurityToken(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;

    if (rx->securityToken == 0) {
        rx->securityToken = (char*) espGetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, 0);
        if (rx->securityToken == 0) {
            rx->securityToken = mprGetMD5(sfmt("%d-%p", mprGetTicks(), conn->rx));
            espSetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, rx->securityToken);
        }
    }
    return rx->securityToken;
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


bool espIsFinalized(HttpConn *conn) 
{
    EspReq      *req;

    req = conn->data;
    return req->finalized;
}


void espRedirect(HttpConn *conn, int status, cchar *target)
{
    EspReq  *req;
    
    req = conn->data;
    //  MOB - should this httpLink be pushed into httpRedirect?
    httpRedirect(conn, status, httpLink(conn, target, NULL));
}


void espRedirectBack(HttpConn *conn)
{
    if (conn->rx->referrer) {
        espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, conn->rx->referrer); 
    }
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


//  MOB - sort
int espGetIntParam(HttpConn *conn, cchar *var, int defaultValue)
{
    return httpGetIntParam(conn, var, defaultValue);
}


void espSetIntParam(HttpConn *conn, cchar *var, int value) 
{
    httpSetIntParam(conn, var, value);
}


void espSetParam(HttpConn *conn, cchar *var, cchar *value) 
{
    httpSetParam(conn, var, value);
}


bool espMatchParam(HttpConn *conn, cchar *var, cchar *value)
{
    return httpMatchParam(conn, var, value);
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


void espSetStatus(HttpConn *conn, int status)
{
    EspReq      *req;

    req = conn->data;
    httpSetStatus(conn, status);
}


ssize espWrite(HttpConn *conn, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;

    va_start(vargs, fmt);
    buf = sfmtv(fmt, vargs);
    va_end(vargs);
    return espWriteString(conn, buf);
}
    

ssize espWriteBlock(HttpConn *conn, cchar *buf, ssize size)
{
    EspReq      *req;
    
    req = conn->data;
    if (req->cacheBuffer) {
        httpSetResponded(conn);
        return mprPutBlockToBuf(req->cacheBuffer, buf, size);
    } else {
        return httpWriteBlock(conn->writeq, buf, size);
    }
}


ssize espWriteFile(HttpConn *conn, cchar *path)
{
    MprFile     *from;
    ssize       count, written, nbytes;
    char        buf[MPR_BUFSIZE];

    if ((from = mprOpenFile(path, O_RDONLY | O_BINARY, 0)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    written = 0;
    while ((count = mprReadFile(from, buf, sizeof(buf))) > 0) {
        if ((nbytes = espWriteBlock(conn, buf, count)) < 0) {
            return nbytes;
        }
        written += nbytes;
    }
    mprCloseFile(from);
    return 0;
}


ssize espWriteParam(HttpConn *conn, cchar *name)
{
    cchar   *value;

    if ((value = espGetParam(conn, name, 0)) == 0) {
        value = espGetSessionVar(conn, name, "");
    }
    return espWriteSafeString(conn, value);
}


ssize espWriteSafeString(HttpConn *conn, cchar *s)
{
    s = mprEscapeHtml(s);
    return espWriteBlock(conn, s, slen(s));
}


ssize espWriteString(HttpConn *conn, cchar *s)
{
    return espWriteBlock(conn, s, slen(s));
}


//  MOB - missing WriteView

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
    espWrite(conn, "\r\n");

    /*
        Query
     */
    if ((query = espGetQueryString(conn)) != 0) {
        scopy(qbuf, sizeof(qbuf), query);
        if ((numKeys = getParams(&keys, qbuf, (int) strlen(qbuf))) > 0) {
            for (i = 0; i < (numKeys * 2); i += 2) {
                value = keys[i+1];
                espWrite(conn, "QUERY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espWrite(conn, "\r\n");
    }

    /*
        Http Headers
     */
    env = espGetHeaderHash(conn);
    for (kp = 0; (kp = mprGetNextKey(env, kp)) != 0; ) {
        espWrite(conn, "HEADER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espWrite(conn, "\r\n");

    /*
        Form vars
     */
    if ((env = espGetParams(conn)) != 0) {
        for (kp = 0; (kp = mprGetNextKey(env, kp)) != 0; ) {
            espWrite(conn, "FORM %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
        }
        espWrite(conn, "\r\n");
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
                espWrite(conn, "BODY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espWrite(conn, "\r\n");
    }
}


void espNoticev(HttpConn *conn, cchar *kind, cchar *fmt, va_list args)
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


void espNotice(HttpConn *conn, cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(conn, kind, fmt, args);
    va_end(args);
}


void espWarn(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(conn, "warn", fmt, args);
    va_end(args);
}


void espError(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(conn, "error", fmt, args);
    va_end(args);
}


void espInform(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(conn, "inform", fmt, args);
    va_end(args);
}


#if FUTURE
#if MACOSX
    static MprThreadLocal *tls;

    HttpConn *espGetCurrentConn() {
        return mprGetThreadData(tls);
    }
#elif __GCC__ && (LINUX || VXWORKS)
    static __thread currentConn;
    HttpConn *espGetCurrentConn() {
        return currentConn;
    }
#endif
//  MOB - move to espSession.c

char *session(cchar *key)
{
    HttpConn    *conn;

    conn = espGetCurrentConn();
}
#endif


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
