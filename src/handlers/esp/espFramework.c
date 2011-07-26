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
    if (req->autoFinalize && !conn->tx->finalized) {
        httpFinalize(conn);
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


cchar *espGetHeader(HttpConn *conn, cchar *key)
{
    return httpGetHeader(conn, key);
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
    httpFinalize(conn);
}


bool espFinalized(HttpConn *conn) 
{
    return conn->tx->finalized;
}


void espFlush(HttpConn *conn) 
{
    httpFlush(conn);
}


void espRedirect(HttpConn *conn, int status, cchar *target)
{
    httpRedirect(conn, status, target);
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
    req->responded = 1;
    httpSetStatus(conn, status);
}


ssize espWrite(HttpConn *conn, cchar *buf, ssize len)
{
    EspReq      *req;
    ssize       written;
    
    req = conn->data;
    if (len < 0) {
        len = slen(buf);
    }
    written = httpWriteBlock(conn->writeq, buf, len);
    //  MOB - should responded be pushed down into Conn
    //  MOB - capture output for caching here
    req->responded = 1;
    return written;
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
