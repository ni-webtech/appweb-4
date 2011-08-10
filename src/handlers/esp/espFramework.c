/*
    espFramework.c -- 

    The ESP handler implements a very fast in-process CGI scheme.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
#include    "esp.h"

/************************************* Local **********************************/

/************************************* Code ***********************************/
#if TODO
    - Request.flash
    - Request.cookies
    - Request.contentType
    - Request.dir
    - Request.contentLength
    - Request.files
    - Request.home (home URI)
    - Request.isSecure
    - Request.method
    - Request.originalMethod        (Need code to modify method based on header)
    - Request.extension

    - Request.checkSecurityToken()
    - Request.clearFlash
    - Request.createSession()
    - Request.dontAutoFinalize()
    - Request.destroySession()
    - Request.error(), inform(), notify(), warn()
    - Request.finalizeFlash
    - Request.link
    - Request.matchContent
    - Request.read()                What about incoming content?
    - Request.securityToken() 
    - Request.setCookie() 
    - Request.setHeader() 
    - Request.setHeaders() 
    - Request.setupFlash() 
    - Request.trace() 
    - Request.writeContent() 
    - Request.writeError() 
    - Request.writeFile() 
    - Request.writeResponse() 
    - Request.writeSafe() 
    - Request.written() 

    - Controller.before(), after()
    - Controller.writeView()
    - Controller.writePartialTemplate()
    - Controller.writeTemplate()
    - Controller.writeTemplateLiteral()
    - Controller.runCheckers()
    - Controller.viewExists()

    - Html.alert
    - Html.anchor
    - Html.button
    - Html.buttonLink
    - Html.chart
    - Html.checkbox
    - Html.div
    - Html.endform
    - Html.flash
    - Html.form
    - Html.icon
    - Html.image
    - Html.label
    - Html.list
    - Html.mail
    - Html.progress
    - Html.radio
    - Html.refresh
    - Html.script
    - Html.securityToken
    - Html.stylesheet
    - Html.table
    - Html.tabs
    - Html.text
    - Html.tree
    - Html.emitFormErros
#endif

/*  
    Add a http header if not already defined
 */
void espAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAddHeaderString(conn, key, mprAsprintfv(fmt, vargs));
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
    httpAppendHeaderString(conn, key, mprAsprintfv(fmt, vargs));
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


void espDontCache(HttpConn *conn)
{
    httpDontCache(conn);
}


MprOff espGetContentLength(HttpConn *conn)
{
    return httpGetContentLength(conn);
}


cchar *espGetCookies(HttpConn *conn) 
{
    return httpGetCookies(conn);
}


MprHashTable *espGetFormVars(HttpConn *conn)
{
    return httpGetFormVars(conn);
}


cchar *espGetHeader(HttpConn *conn, cchar *key)
{
    return httpGetHeader(conn, key);
}


MprHashTable *espGetHeaderHash(HttpConn *conn)
{
    return httpGetHeaderHash(conn);
}


char *espGetHeaders(HttpConn *conn)
{
    return httpGetHeaders(conn);
}


cchar *espGetQueryString(HttpConn *conn)
{
    return httpGetQueryString(conn);
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


bool espFinalized(HttpConn *conn) 
{
    EspReq      *req;

    req = conn->data;
    return req->finalized;
}


void espFlush(HttpConn *conn) 
{
    httpFlush(conn);
}


void espRedirect(HttpConn *conn, int status, cchar *target)
{
    EspReq  *req;
    
    req = conn->data;
    httpRedirect(conn, status, target);
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


bool espSetAutoFinalizing(HttpConn *conn, int on) 
{
    EspReq  *req;
    int     old;

    req = conn->data;
    old = req->autoFinalize;
    req->autoFinalize = on;
    return old;
}


void espSetContentLength(HttpConn *conn, MprOff length)
{
    httpSetContentLength(conn, length);
}


void espSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, int lifetime, bool isSecure)
{
    httpSetCookie(conn, name, value, path, cookieDomain, lifetime, isSecure);
}


void espSetContentType(HttpConn *conn, cchar *mimeType)
{
    httpSetContentType(conn, mimeType);
}


int espGetIntVar(HttpConn *conn, cchar *var, int defaultValue)
{
    return httpGetIntFormVar(conn, var, defaultValue);
}


cchar *espGetVar(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    return httpGetFormVar(conn, var, defaultValue);
}


void espSetIntVar(HttpConn *conn, cchar *var, int value) 
{
    httpSetIntFormVar(conn, var, value);
}


void espSetVar(HttpConn *conn, cchar *var, cchar *value) 
{
    httpSetFormVar(conn, var, value);
}


bool espMatchVar(HttpConn *conn, cchar *var, cchar *value)
{
    return httpMatchFormVar(conn, var, value);
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
    httpSetHeader(conn, key, mprAsprintfv(fmt, vargs));
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
    buf = mprAsprintfv(fmt, vargs);
    va_end(vargs);
    return espWriteString(conn, buf);
}
    

ssize espWriteBlock(HttpConn *conn, cchar *buf, ssize size)
{
    EspReq      *req;
    
    req = conn->data;
#if BLD_DEBUG
{
    /* Verify length */
    ssize len = slen(buf);
    mprAssert(len == size);
}
#endif
    if (req->cacheBuffer) {
        httpSetResponded(conn);
        return mprPutBlockToBuf(req->cacheBuffer, buf, size);
    } else {
        return httpWriteBlock(conn->writeq, buf, size);
    }
}


ssize espWriteFile(HttpConn *conn, cchar *path)
{
    //  MOB - TODO (must read in small chunks)
    //  finalized = 1;
    return 0;
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


ssize espWriteVar(HttpConn *conn, cchar *name)
{
    cchar   *value;

    if ((value = espGetVar(conn, name, 0)) == 0) {
        value = espGetSessionVar(conn, name, "");
    }
    return espWriteString(conn, mprEscapeHtml(value));
}


/*
    Convert queue data in key / value pairs
 */
static int getVars(char ***keys, char *buf, int len)
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
    MprHashTable    *env;
    MprHash         *hp;
    MprBuf          *buf;
    HttpRx          *rx;
    HttpQueue       *q;
    cchar           *query;
    char            qbuf[MPR_MAX_STRING], **keys, *value;
    int             i, numKeys;

    rx = conn->rx;
    espDontCache(conn);
    espWrite(conn, "\r\n");

    /*
        Query
     */
    if ((query = espGetQueryString(conn)) != 0) {
        scopy(qbuf, sizeof(qbuf), query);
        if ((numKeys = getVars(&keys, qbuf, (int) strlen(qbuf))) > 0) {
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
    for (hp = 0; (hp = mprGetNextKey(env, hp)) != 0; ) {
        espWrite(conn, "HEADER %s=%s\r\n", hp->key, hp->data ? hp->data: "null");
    }
    espWrite(conn, "\r\n");

    /*
        Form vars
     */
    if ((env = espGetFormVars(conn)) != 0) {
        for (hp = 0; (hp = mprGetNextKey(env, hp)) != 0; ) {
            espWrite(conn, "FORM %s=%s\r\n", hp->key, hp->data ? hp->data: "null");
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
        if ((numKeys = getVars(&keys, mprGetBufStart(buf), (int) mprGetBufLength(buf))) > 0) {
            for (i = 0; i < (numKeys * 2); i += 2) {
                value = keys[i+1];
                espWrite(conn, "BODY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espWrite(conn, "\r\n");
    }
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
