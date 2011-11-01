/**
    espSession.c - Session data storage

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"

#if BLD_FEATURE_ESP
/********************************** Forwards  *********************************/

static char *makeKey(EspSession *sp, cchar *key);
static char *makeSessionID(HttpConn *conn);
static void manageSession(EspSession *sp, int flags);

/************************************* Code ***********************************/

EspSession *espAllocSession(HttpConn *conn, cchar *id, MprTime lifespan)
{
    EspReq      *req;
    EspSession  *sp;

    mprAssert(conn);
    req = conn->data;
    mprAssert(req);

    if ((sp = mprAllocObj(EspSession, manageSession)) == 0) {
        return 0;
    }
    mprSetName(sp, "session");
    sp->lifespan = lifespan;
    if (id == 0) {
        id = makeSessionID(conn);
    }
    sp->id = sclone(id);
    sp->cache = req->esp->sessionCache;
    return sp;
}


//  MOB - need to be able to hook session creation / destruction
void espDestroySession(EspSession *sp)
{
    mprAssert(sp);
    sp->id = 0;
}


static void manageSession(EspSession *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->id);
        mprMark(sp->cache);
    }
}


EspSession *espCreateSession(HttpConn *conn)
{
    return espGetSession(conn, 1);
}


EspSession *espGetSession(HttpConn *conn, int create)
{
    EspReq      *req;
    char        *id;

    mprAssert(conn);
    req = conn->data;
    mprAssert(req);
    if (req->session || !conn) {
        return req->session;
    }
    id = espGetSessionID(conn);
    if (id || create) {
        req->session = espAllocSession(conn, id, conn->limits->sessionTimeout);
        if (req->session && !id) {
            httpSetCookie(conn, ESP_SESSION, req->session->id, "/", NULL, 0, conn->secure);
        }
    }
    return req->session;
}



MprHash *espGetSessionObj(HttpConn *conn, cchar *key)
{
    cchar   *str;

    if ((str = espGetSessionVar(conn, key, 0)) != 0 && *str) {
        mprAssert(*str == '{');
        return mprDeserialize(str);
    }
    return 0;
}


cchar *espGetSessionVar(HttpConn *conn, cchar *key, cchar *defaultValue)
{
    EspSession  *sp;
    cchar       *result;

    mprAssert(conn);
    mprAssert(key && *key);

    result = 0;
    if ((sp = espGetSession(conn, 0)) != 0) {
        result = mprReadCache(sp->cache, makeKey(sp, key), 0, 0);
    }
    return result ? result : defaultValue;
}


int espSetSessionObj(HttpConn *conn, cchar *key, MprHash *obj)
{
    espSetSessionVar(conn, key, mprSerialize(obj, 0));
    return 0;
}


int espSetSessionVar(HttpConn *conn, cchar *key, cchar *value)
{
    EspSession  *sp;

    mprAssert(conn);
    mprAssert(key && *key);
    mprAssert(value);

    if ((sp = espGetSession(conn, 1)) == 0) {
        return 0;
    }
//  MOB - should not the session expire all at once?
    if (mprWriteCache(sp->cache, makeKey(sp, key), value, 0, sp->lifespan, 0, MPR_CACHE_SET) == 0) {
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


#if UNUSED
//  MOB - sessions should expire in their entirity
void espExpireSessionVar(HttpConn *conn, cchar *key, MprTime lifespan)
{
    EspReq      *req;
    EspSession  *sp;

    mprAssert(conn);
    mprAssert(key && *key);

    req = conn->data;
    sp = req->session;
    mprExpireCache(sp->cache, makeKey(sp, key), mprGetTime() + lifespan);
}
#endif


char *espGetSessionID(HttpConn *conn)
{
    EspReq  *req;
    cchar   *cookies, *cookie;
    char    *cp, *value;
    int     quoted;

    mprAssert(conn);
    req = conn->data;
    mprAssert(req);

    if (req->session) {
        return req->session->id;
    }
    if (req->sessionProbed) {
        return 0;
    }
    req->sessionProbed = 1;
    cookies = httpGetCookies(conn);
    for (cookie = cookies; cookie && (value = strstr(cookie, ESP_SESSION)) != 0; cookie = value) {
        value += strlen(ESP_SESSION);
        while (isspace((int) *value) || *value == '=') {
            value++;
        }
        quoted = 0;
        if (*value == '"') {
            value++;
            quoted++;
        }
        for (cp = value; *cp; cp++) {
            if (quoted) {
                if (*cp == '"' && cp[-1] != '\\') {
                    break;
                }
            } else {
                if ((*cp == ',' || *cp == ';') && cp[-1] != '\\') {
                    break;
                }
            }
        }
        return snclone(value, cp - value);
    }
    return 0;
}


static char *makeSessionID(HttpConn *conn)
{
    char        idBuf[64];
    static int  nextSession = 0;

    mprAssert(conn);

    /* Thread race here on nextSession++ not critical */
    mprSprintf(idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(conn->data) + PTOI(conn), (int) mprGetTime(), nextSession++);
    return mprGetMD5WithPrefix(idBuf, sizeof(idBuf), "::esp.session::");
}


static char *makeKey(EspSession *sp, cchar *key)
{
    return sfmt("session-%s-%s", sp->id, key);
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
