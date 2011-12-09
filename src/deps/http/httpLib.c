#include "http.h"

/******************************************************************************/
/* 
    This file is an amalgamation of all the individual source code files for the
    Http Library Library Source.
  
    Catenating all the source into a single file makes embedding simpler and
    the resulting application faster, as many compilers can do whole file
    optimization.
  
    If you want to modify the product, you can still get the whole source as 
    individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "./src/auth.c"
 */
/************************************************************************/

/*
    auth.c - Generic authorization code
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void manageAuth(HttpAuth *auth, int flags);


HttpAuth *httpCreateAuth()
{
    HttpAuth      *auth;

    auth = mprAllocObj(HttpAuth, manageAuth);
#if BLD_FEATURE_AUTH_PAM
    auth->backend = HTTP_AUTH_METHOD_PAM;
#elif BLD_FEATURE_AUTH_FILE
    auth->backend = HTTP_AUTH_METHOD_FILE;
#endif
    return auth;
}


HttpAuth *httpCreateInheritedAuth(HttpAuth *parent)
{
    HttpAuth      *auth;

    auth = mprAllocObj(HttpAuth, manageAuth);
    if (parent) {
        if (parent->allow) {
            auth->allow = mprCloneHash(parent->allow);
        }
        if (parent->deny) {
            auth->deny = mprCloneHash(parent->deny);
        }
        auth->anyValidUser = parent->anyValidUser;
        auth->type = parent->type;
        auth->backend = parent->backend;
        auth->flags = parent->flags;
        auth->order = parent->order;
        auth->qop = parent->qop;
        auth->requiredRealm = parent->requiredRealm;
        auth->requiredUsers = parent->requiredUsers;
        auth->requiredGroups = parent->requiredGroups;

        auth->userFile = parent->userFile;
        auth->groupFile = parent->groupFile;
        auth->users = parent->users;
        auth->groups = parent->groups;

    } else{
#if BLD_FEATURE_AUTH_PAM
        auth->backend = HTTP_AUTH_METHOD_PAM;
#elif BLD_FEATURE_AUTH_FILE
        auth->backend = HTTP_AUTH_METHOD_FILE;
#endif
    }
    return auth;
}


static void manageAuth(HttpAuth *auth, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(auth->allow);
        mprMark(auth->deny);
        mprMark(auth->requiredRealm);
        mprMark(auth->requiredGroups);
        mprMark(auth->requiredUsers);
        mprMark(auth->qop);
        mprMark(auth->userFile);
        mprMark(auth->groupFile);
        mprMark(auth->users);
        mprMark(auth->groups);
    }
}


void httpSetAuthAllow(HttpAuth *auth, cchar *allow)
{
    if (auth->allow == 0) {
        auth->allow = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    }
    mprAddKey(auth->allow, sclone(allow), "");
}


void httpSetAuthAnyValidUser(HttpAuth *auth)
{
    auth->anyValidUser = 1;
    auth->flags |= HTTP_AUTH_REQUIRED;
}


void httpSetAuthDeny(HttpAuth *auth, cchar *client)
{
    if (auth->deny == 0) {
        auth->deny = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    }
    mprAddKey(auth->deny, sclone(client), "");
}


void httpSetAuthOrder(HttpAuth *auth, int order)
{
    auth->order = order;
}


void httpSetAuthQop(HttpAuth *auth, cchar *qop)
{
    if (strcmp(qop, "auth") == 0 || strcmp(qop, "auth-int") == 0) {
        auth->qop = sclone(qop);
    } else {
        auth->qop = mprEmptyString();
    }
}


void httpSetAuthRealm(HttpAuth *auth, cchar *realm)
{
    auth->requiredRealm = sclone(realm);
}


void httpSetAuthRequiredGroups(HttpAuth *auth, cchar *groups)
{
    auth->requiredGroups = sclone(groups);
    auth->flags |= HTTP_AUTH_REQUIRED;
    auth->anyValidUser = 0;
}


void httpSetAuthRequiredUsers(HttpAuth *auth, cchar *users)
{
    auth->requiredUsers = sclone(users);
    auth->flags |= HTTP_AUTH_REQUIRED;
    auth->anyValidUser = 0;
}


void httpSetAuthUser(HttpConn *conn, cchar *user)
{
    conn->authUser = sclone(user);
}


/*  
    Validate the user credentials with the designated authorization backend method.
 */
static bool validateCred(HttpAuth *auth, cchar *realm, char *user, cchar *password, cchar *requiredPass, char **msg)
{
    /*  
        Use this funny code construct incase no backend method is configured. Still want the code to compile.
     */
    if (0) {
#if BLD_FEATURE_AUTH_FILE
    } else if (auth->backend == HTTP_AUTH_METHOD_FILE) {
        return httpValidateFileCredentials(auth, realm, user, password, requiredPass, msg);
#endif
#if BLD_FEATURE_AUTH_PAM
    } else if (auth->backend == HTTP_AUTH_METHOD_PAM) {
        return httpValidatePamCredentials(auth, realm, user, password, NULL, msg);
#endif
    } else {
        *msg = "Required authorization backend method is not enabled or configured";
    }
    return 0;
}


/*  
    Get the password (if the designated authorization backend method will give it to us)
 */
static cchar *getPassword(HttpAuth *auth, cchar *realm, cchar *user)
{
    /*  
        Use this funny code construct incase no backend method is configured. Still want the code to compile.
     */
    if (0) {
#if BLD_FEATURE_AUTH_FILE
    } else if (auth->backend == HTTP_AUTH_METHOD_FILE) {
        return httpGetFilePassword(auth, realm, user);
#endif
#if BLD_FEATURE_AUTH_PAM
    } else if (auth->backend == HTTP_AUTH_METHOD_PAM) {
        return httpGetPamPassword(auth, realm, user);
#endif
    }
    return 0;
}


void httpInitAuth(Http *http)
{
    http->validateCred = validateCred;
    http->getPassword = getPassword;
}


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
/************************************************************************/
/*
 *  End of file "./src/auth.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/authCheck.c"
 */
/************************************************************************/

/*
    authCheck.c - Authorization checking for basic and digest authentication.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



/*
    Per-request authorization data
 */
typedef struct AuthData 
{
    char            *password;          /* User password or digest */
    char            *userName;
    char            *cnonce;
    char            *nc;
    char            *nonce;
    char            *opaque;
    char            *qop;
    char            *realm;
    char            *uri;
} AuthData;


static int calcDigest(char **digest, cchar *userName, cchar *password, cchar *realm, cchar *uri, 
    cchar *nonce, cchar *qop, cchar *nc, cchar *cnonce, cchar *method);
static char *createDigestNonce(HttpConn *conn, cchar *secret, cchar *realm);
static void decodeBasicAuth(HttpConn *conn, AuthData *ad);
static int  decodeDigestDetails(HttpConn *conn, AuthData *ad);
static void formatAuthResponse(HttpConn *conn, HttpAuth *auth, int code, char *msg, char *logMsg);
static int parseDigestNonce(char *nonce, cchar **secret, cchar **realm, MprTime *when);


int httpCheckAuth(HttpConn *conn)
{
    Http        *http;
    HttpRx      *rx;
    HttpAuth    *auth;
    HttpRoute   *route;
    AuthData    *ad;
    MprTime     when;
    cchar       *requiredPassword;
    char        *msg, *requiredDigest;
    cchar       *secret, *realm;
    int         actualAuthType;

    rx = conn->rx;
    http = conn->http;
    route = rx->route;
    auth = route->auth;

    if (!conn->endpoint || auth == 0 || auth->type == 0) {
        return 0;
    }
    if ((ad = mprAllocStruct(AuthData)) == 0) {
        return 0;
    }
    if (rx->authDetails == 0) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Missing authorization details.", 0);
        return HTTP_ROUTE_OK;
    }
    if (scasecmp(rx->authType, "basic") == 0) {
        decodeBasicAuth(conn, ad);
        actualAuthType = HTTP_AUTH_BASIC;

    } else if (scasecmp(rx->authType, "digest") == 0) {
        if (decodeDigestDetails(conn, ad) < 0) {
            httpError(conn, 400, "Bad authorization header");
            return HTTP_ROUTE_OK;
        }
        actualAuthType = HTTP_AUTH_DIGEST;
    } else {
        actualAuthType = HTTP_AUTH_UNKNOWN;
    }
    mprLog(4, "matchAuth: type %d, url %s\nDetails %s\n", auth->type, rx->pathInfo, rx->authDetails);

    if (ad->userName == 0) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Missing user name", 0);
        return HTTP_ROUTE_OK;
    }
    if (auth->type != actualAuthType) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Wrong authentication protocol", 0);
        return HTTP_ROUTE_OK;
    }
    /*  
        Some backend methods can't return the password and will simply do everything in validateCred. 
        In this case, they and will return "". That is okay.
     */
    if ((requiredPassword = (http->getPassword)(auth, auth->requiredRealm, ad->userName)) == 0) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, authentication error", "User not defined");
        return 1;
    }
    if (auth->type == HTTP_AUTH_DIGEST) {
        if (scmp(ad->qop, auth->qop) != 0) {
            formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied. Protection quality does not match", 0);
            return HTTP_ROUTE_OK;
        }
        calcDigest(&requiredDigest, 0, requiredPassword, ad->realm, ad->uri, ad->nonce, ad->qop, ad->nc, 
            ad->cnonce, rx->method);
        requiredPassword = requiredDigest;

        /*
            Validate the nonce value - prevents replay attacks
         */
        when = 0; secret = 0; realm = 0;
        parseDigestNonce(ad->nonce, &secret, &realm, &when);
        if (scmp(secret, http->secret) != 0 || scmp(realm, auth->requiredRealm) != 0) {
            formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access denied, authentication error", "Nonce mismatch");
        } else if ((when + (5 * 60 * MPR_TICKS_PER_SEC)) < http->now) {
            formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access denied, authentication error", "Nonce is stale");
        }
    }
    if (!(http->validateCred)(auth, auth->requiredRealm, ad->userName, ad->password, requiredPassword, &msg)) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access denied, incorrect username/password", msg);
    }
    rx->authenticated = 1;
    return HTTP_ROUTE_OK;
}


/*  
    Decode basic authorization details
 */
static void decodeBasicAuth(HttpConn *conn, AuthData *ad)
{
    HttpRx  *rx;
    char    *decoded, *cp;

    rx = conn->rx;
    if ((decoded = mprDecode64(rx->authDetails)) == 0) {
        return;
    }
    if ((cp = strchr(decoded, ':')) != 0) {
        *cp++ = '\0';
    }
    if (cp) {
        ad->userName = sclone(decoded);
        ad->password = sclone(cp);

    } else {
        ad->userName = mprEmptyString();
        ad->password = mprEmptyString();
    }
    httpSetAuthUser(conn, ad->userName);
}


/*  
    Decode the digest authentication details.
 */
static int decodeDigestDetails(HttpConn *conn, AuthData *ad)
{
    HttpRx      *rx;
    char        *value, *tok, *key, *dp, *sp;
    int         seenComma;

    rx = conn->rx;
    key = sclone(rx->authDetails);

    while (*key) {
        while (*key && isspace((int) *key)) {
            key++;
        }
        tok = key;
        while (*tok && !isspace((int) *tok) && *tok != ',' && *tok != '=') {
            tok++;
        }
        *tok++ = '\0';

        while (isspace((int) *tok)) {
            tok++;
        }
        seenComma = 0;
        if (*tok == '\"') {
            value = ++tok;
            while (*tok != '\"' && *tok != '\0') {
                tok++;
            }
        } else {
            value = tok;
            while (*tok != ',' && *tok != '\0') {
                tok++;
            }
            seenComma++;
        }
        *tok++ = '\0';

        /*
            Handle back-quoting
         */
        if (strchr(value, '\\')) {
            for (dp = sp = value; *sp; sp++) {
                if (*sp == '\\') {
                    sp++;
                }
                *dp++ = *sp++;
            }
            *dp = '\0';
        }

        /*
            user, response, oqaque, uri, realm, nonce, nc, cnonce, qop
         */
        switch (tolower((int) *key)) {
        case 'a':
            if (scasecmp(key, "algorithm") == 0) {
                break;
            } else if (scasecmp(key, "auth-param") == 0) {
                break;
            }
            break;

        case 'c':
            if (scasecmp(key, "cnonce") == 0) {
                ad->cnonce = sclone(value);
            }
            break;

        case 'd':
            if (scasecmp(key, "domain") == 0) {
                break;
            }
            break;

        case 'n':
            if (scasecmp(key, "nc") == 0) {
                ad->nc = sclone(value);
            } else if (scasecmp(key, "nonce") == 0) {
                ad->nonce = sclone(value);
            }
            break;

        case 'o':
            if (scasecmp(key, "opaque") == 0) {
                ad->opaque = sclone(value);
            }
            break;

        case 'q':
            if (scasecmp(key, "qop") == 0) {
                ad->qop = sclone(value);
            }
            break;

        case 'r':
            if (scasecmp(key, "realm") == 0) {
                ad->realm = sclone(value);
            } else if (scasecmp(key, "response") == 0) {
                /* Store the response digest in the password field */
                ad->password = sclone(value);
            }
            break;

        case 's':
            if (scasecmp(key, "stale") == 0) {
                break;
            }
        
        case 'u':
            if (scasecmp(key, "uri") == 0) {
                ad->uri = sclone(value);
            } else if (scasecmp(key, "username") == 0 || scasecmp(key, "user") == 0) {
                ad->userName = sclone(value);
            }
            break;

        default:
            /*  Just ignore keywords we don't understand */
            ;
        }
        key = tok;
        if (!seenComma) {
            while (*key && *key != ',') {
                key++;
            }
            if (*key) {
                key++;
            }
        }
    }
    if (ad->userName == 0 || ad->realm == 0 || ad->nonce == 0 || ad->uri == 0 || ad->password == 0) {
        return MPR_ERR_BAD_ARGS;
    }
    if (ad->qop && (ad->cnonce == 0 || ad->nc == 0)) {
        return MPR_ERR_BAD_ARGS;
    }
    if (ad->qop == 0) {
        ad->qop = mprEmptyString();
    }
    httpSetAuthUser(conn, ad->userName);
    return 0;
}


/*  
    Format an authentication response. This is typically a 401 response code.
 */
static void formatAuthResponse(HttpConn *conn, HttpAuth *auth, int code, char *msg, char *logMsg)
{
    char    *qopClass, *nonce;

    if (logMsg == 0) {
        logMsg = msg;
    }
    mprLog(3, "Auth response: code %d, %s", code, logMsg);

    if (auth->type == HTTP_AUTH_BASIC) {
        httpSetHeader(conn, "WWW-Authenticate", "Basic realm=\"%s\"", auth->requiredRealm);

    } else if (auth->type == HTTP_AUTH_DIGEST) {
        qopClass = auth->qop;
        nonce = createDigestNonce(conn, conn->http->secret, auth->requiredRealm);
        mprAssert(conn->host);

        if (scmp(qopClass, "auth") == 0) {
            httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", domain=\"%s\", "
                "qop=\"auth\", nonce=\"%s\", opaque=\"%s\", algorithm=\"MD5\", stale=\"FALSE\"", 
                auth->requiredRealm, conn->host->name, nonce, "");

        } else if (scmp(qopClass, "auth-int") == 0) {
            httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", domain=\"%s\", "
                "qop=\"auth\", nonce=\"%s\", opaque=\"%s\", algorithm=\"MD5\", stale=\"FALSE\"", 
                auth->requiredRealm, conn->host->name, nonce, "");

        } else {
            httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", nonce=\"%s\"", auth->requiredRealm, nonce);
        }
    }
    httpSetContentType(conn, "text/plain");
    httpError(conn, code, "Authentication Error: %s", msg);
    httpSetPipelineHandler(conn, conn->http->passHandler);
}


/*
    Create a nonce value for digest authentication (RFC 2617)
 */ 
static char *createDigestNonce(HttpConn *conn, cchar *secret, cchar *realm)
{
    MprTime      now;
    char         nonce[256];
    static int64 next = 0;

    mprAssert(realm && *realm);

    now = conn->http->now;
    mprSprintf(nonce, sizeof(nonce), "%s:%s:%Lx:%Lx", secret, realm, now, next++);
    return mprEncode64(nonce);
}


static int parseDigestNonce(char *nonce, cchar **secret, cchar **realm, MprTime *when)
{
    char    *tok, *decoded, *whenStr;

    if ((decoded = mprDecode64(nonce)) == 0) {
        return MPR_ERR_CANT_READ;
    }
    *secret = stok(decoded, ":", &tok);
    *realm = stok(NULL, ":", &tok);
    whenStr = stok(NULL, ":", &tok);
    *when = (MprTime) stoiradix(whenStr, 16, NULL); 
    return 0;
}


/*
    Get a Digest value using the MD5 algorithm -- See RFC 2617 to understand this code.
 */ 
static int calcDigest(char **digest, cchar *userName, cchar *password, cchar *realm, cchar *uri, 
    cchar *nonce, cchar *qop, cchar *nc, cchar *cnonce, cchar *method)
{
    char    a1Buf[256], a2Buf[256], digestBuf[256];
    char    *ha1, *ha2;

    mprAssert(qop);

    /*
        Compute HA1. If userName == 0, then the password is already expected to be in the HA1 format 
        (MD5(userName:realm:password).
     */
    if (userName == 0) {
        ha1 = sclone(password);
    } else {
        mprSprintf(a1Buf, sizeof(a1Buf), "%s:%s:%s", userName, realm, password);
        ha1 = mprGetMD5(a1Buf);
    }

    /*
        HA2
     */ 
    mprSprintf(a2Buf, sizeof(a2Buf), "%s:%s", method, uri);
    ha2 = mprGetMD5(a2Buf);

    /*
        H(HA1:nonce:HA2)
     */
    if (scmp(qop, "auth") == 0) {
        mprSprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s:%s:%s:%s", ha1, nonce, nc, cnonce, qop, ha2);

    } else if (scmp(qop, "auth-int") == 0) {
        mprSprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s:%s:%s:%s", ha1, nonce, nc, cnonce, qop, ha2);

    } else {
        mprSprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s", ha1, nonce, ha2);
    }
    *digest = mprGetMD5(digestBuf);
    return 0;
}



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
/************************************************************************/
/*
 *  End of file "./src/authCheck.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/authFile.c"
 */
/************************************************************************/

/*
    authFile.c - File based authorization using password files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if BLD_FEATURE_AUTH_FILE

static bool isUserValid(HttpAuth *auth, cchar *realm, cchar *user);
static void manageGroup(HttpGroup *group, int flags);
static void manageUser(HttpUser *user, int flags);


cchar *httpGetFilePassword(HttpAuth *auth, cchar *realm, cchar *user)
{
    HttpUser    *up;
    char        *key;

    up = 0;
    key = sjoin(realm, ":", user, NULL);
    if (auth->users) {
        up = (HttpUser*) mprLookupKey(auth->users, key);
    }
    if (up == 0) {
        return 0;
    }
    return up->password;
}


bool httpValidateFileCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, cchar *requiredPassword, 
    char **msg)
{
    char    passbuf[HTTP_MAX_PASS * 2], *hashedPassword;

    hashedPassword = 0;
    
    if (auth->type == HTTP_AUTH_BASIC) {
        mprSprintf(passbuf, sizeof(passbuf), "%s:%s:%s", user, realm, password);
        hashedPassword = mprGetMD5(passbuf);
        password = hashedPassword;
    }
    if (!isUserValid(auth, realm, user)) {
        *msg = "Access Denied, Unknown User.";
        return 0;
    }
    if (strcmp(password, requiredPassword)) {
        *msg = "Access Denied, Wrong Password.";
        return 0;
    }
    return 1;
}


/*
    Determine if this user is specified as being eligible for this realm. We examine the requiredUsers and requiredGroups.
 */
static bool isUserValid(HttpAuth *auth, cchar *realm, cchar *user)
{
    HttpGroup       *gp;
    HttpUser        *up;
    char            *key, *requiredUser, *requiredUsers, *requiredGroups, *group, *name, *tok, *gtok;
    int             next;

    if (auth->anyValidUser) {
        key = sjoin(realm, ":", user, NULL);
        if (auth->users == 0) {
            return 0;
        }
        return mprLookupKey(auth->users, key) != 0;
    }

    if (auth->requiredUsers) {
        requiredUsers = sclone(auth->requiredUsers);
        tok = NULL;
        requiredUser = stok(requiredUsers, " \t", &tok);
        while (requiredUser) {
            if (strcmp(user, requiredUser) == 0) {
                return 1;
            }
            requiredUser = stok(NULL, " \t", &tok);
        }
    }

    if (auth->requiredGroups) {
        gtok = NULL;
        requiredGroups = sclone(auth->requiredGroups);
        /*
            For each group, check all the users in the group.
         */
        group = stok(requiredGroups, " \t", &gtok);
        while (group) {
            if (auth->groups == 0) {
                gp = 0;
            } else {
                gp = (HttpGroup*) mprLookupKey(auth->groups, group);
            }
            if (gp == 0) {
                mprError("Can't find group %s", group);
                group = stok(NULL, " \t", &gtok);
                continue;
            }
            for (next = 0; (name = mprGetNextItem(gp->users, &next)) != 0; ) {
                if (strcmp(user, name) == 0) {
                    return 1;
                }
            }
            group = stok(NULL, " \t", &gtok);
        }
    }
    if (auth->requiredAcl != 0) {
        key = sjoin(realm, ":", user, NULL);
        up = (HttpUser*) mprLookupKey(auth->users, key);
        if (up) {
            mprLog(6, "UserRealm \"%s\" has ACL %lx, Required ACL %lx", key, up->acl, auth->requiredAcl);
            if (up->acl & auth->requiredAcl) {
                return 1;
            }
        }
    }
    return 0;
}


HttpGroup *httpCreateGroup(HttpAuth *auth, cchar *name, HttpAcl acl, bool enabled)
{
    HttpGroup     *gp;

    if ((gp = mprAllocObj(HttpGroup, manageGroup)) == 0) {
        return 0;
    }
    gp->acl = acl;
    gp->name = sclone(name);
    gp->enabled = enabled;
    gp->users = mprCreateList(0, 0);
    return gp;
}


static void manageGroup(HttpGroup *group, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(group->name);
        mprMark(group->users);
    }
}


int httpAddGroup(HttpAuth *auth, cchar *group, HttpAcl acl, bool enabled)
{
    HttpGroup     *gp;

    mprAssert(auth);
    mprAssert(group && *group);

    gp = httpCreateGroup(auth, group, acl, enabled);
    if (gp == 0) {
        return MPR_ERR_MEMORY;
    }
    /*
        Create the index on demand
     */
    if (auth->groups == 0) {
        auth->groups = mprCreateHash(-1, 0);
    }
    if (mprLookupKey(auth->groups, group)) {
        return MPR_ERR_ALREADY_EXISTS;
    }
    if (mprAddKey(auth->groups, group, gp) == 0) {
        return MPR_ERR_MEMORY;
    }
    return 0;
}


HttpUser *httpCreateUser(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, bool enabled)
{
    HttpUser      *up;

    if ((up = mprAllocObj(HttpUser, manageUser)) == 0) {
        return 0;
    }
    up->name = sclone(user);
    up->realm = sclone(realm);
    up->password = sclone(password);
    up->enabled = enabled;
    return up;
}


static void manageUser(HttpUser *user, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(user->password);
        mprMark(user->realm);
        mprMark(user->name);
    }
}


int httpAddUser(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, bool enabled)
{
    HttpUser  *up;

    char    *key;

    up = httpCreateUser(auth, realm, user, password, enabled);
    if (up == 0) {
        return MPR_ERR_MEMORY;
    }
    if (auth->users == 0) {
        auth->users = mprCreateHash(-1, 0);
    }
    key = sjoin(realm, ":", user, NULL);
    if (mprLookupKey(auth->users, key)) {
        return MPR_ERR_ALREADY_EXISTS;
    }
    if (mprAddKey(auth->users, key, up) == 0) {
        return MPR_ERR_MEMORY;
    }
    return 0;
}


int httpAddUserToGroup(HttpAuth *auth, HttpGroup *gp, cchar *user)
{
    char        *name;
    int         next;

    for (next = 0; (name = mprGetNextItem(gp->users, &next)) != 0; ) {
        if (strcmp(name, user) == 0) {
            return MPR_ERR_ALREADY_EXISTS;
        }
    }
    mprAddItem(gp->users, sclone(user));
    return 0;
}


int httpAddUsersToGroup(HttpAuth *auth, cchar *group, cchar *userList)
{
    HttpGroup   *gp;
    char        *user, *users, *tok;

    gp = 0;

    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    tok = NULL;
    users = sclone(userList);
    user = stok(users, " ,\t", &tok);
    while (user) {
        /* Ignore already exists errors */
        httpAddUserToGroup(auth, gp, user);
        user = stok(NULL, " \t", &tok);
    }
    return 0;
}


int httpDisableGroup(HttpAuth *auth, cchar *group)
{
    HttpGroup     *gp;

    gp = 0;

    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    gp->enabled = 0;
    return 0;
}


int httpDisableUser(HttpAuth *auth, cchar *realm, cchar *user)
{
    HttpUser      *up;
    char        *key;

    up = 0;
    key = sjoin(realm, ":", user, NULL);
    if (auth->users == 0 || (up = (HttpUser*) mprLookupKey(auth->users, key)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    up->enabled = 0;
    return 0;
}


int httpEnableGroup(HttpAuth *auth, cchar *group)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    gp->enabled = 1;
    return 0;
}


int httpEnableUser(HttpAuth *auth, cchar *realm, cchar *user)
{
    HttpUser      *up;
    char        *key;

    up = 0;
    key = sjoin(realm, ":", user, NULL);    
    if (auth->users == 0 || (up = (HttpUser*) mprLookupKey(auth->users, key)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    up->enabled = 1;
    return 0;
}


HttpAcl httpGetGroupAcl(HttpAuth *auth, char *group)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return gp->acl;
}


bool httpIsGroupEnabled(HttpAuth *auth, cchar *group)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return 0;
    }
    return gp->enabled;
}


bool httpIsUserEnabled(HttpAuth *auth, cchar *realm, cchar *user)
{
    HttpUser  *up;
    char    *key;

    up = 0;
    key = sjoin(realm, ":", user, NULL);
    if (auth->users == 0 || (up = (HttpUser*) mprLookupKey(auth->users, key)) == 0) {
        return 0;
    }
    return up->enabled;
}


/*
    ACLs are simple hex numbers
 */
//  TODO - better to convert to user, role, capability
HttpAcl httpParseAcl(HttpAuth *auth, cchar *aclStr)
{
    HttpAcl acl = 0;
    int     c;

    if (aclStr) {
        if (aclStr[0] == '0' && aclStr[1] == 'x') {
            aclStr += 2;
        }
        for (; isxdigit((int) *aclStr); aclStr++) {
            c = (int) tolower((int) *aclStr);
            if ('0' <= c && c <= '9') {
                acl = (acl * 16) + c - '0';
            } else {
                acl = (acl * 16) + c - 'a' + 10;
            }
        }
    }
    return acl;
}


/*
    Update the total ACL for each user. We do this by oring all the ACLs for each group the user is a member of. 
    For fast access, this union ACL is stored in the HttpUser object
 */
void httpUpdateUserAcls(HttpAuth *auth)
{
    MprKey      *groupHash, *userHash;
    HttpUser    *user;
    HttpGroup   *gp;
    
    /*
        Reset the ACL for each user
     */
    if (auth->users != 0) {
        for (userHash = 0; (userHash = mprGetNextKey(auth->users, userHash)) != 0; ) {
            ((HttpUser*) userHash->data)->acl = 0;
        }
    }

    /*
        Get the union of all ACLs defined for a user over all groups that the user is a member of.
     */
    if (auth->groups != 0 && auth->users != 0) {
        for (groupHash = 0; (groupHash = mprGetNextKey(auth->groups, groupHash)) != 0; ) {
            gp = ((HttpGroup*) groupHash->data);
            for (userHash = 0; (userHash = mprGetNextKey(auth->users, userHash)) != 0; ) {
                user = ((HttpUser*) userHash->data);
                if (strcmp(user->name, user->name) == 0) {
                    user->acl = user->acl | gp->acl;
                    break;
                }
            }
        }
    }
}


int httpRemoveGroup(HttpAuth *auth, cchar *group)
{
    if (auth->groups == 0 || !mprLookupKey(auth->groups, group)) {
        return MPR_ERR_CANT_ACCESS;
    }
    mprRemoveKey(auth->groups, group);
    return 0;
}


int httpRemoveUser(HttpAuth *auth, cchar *realm, cchar *user)
{
    char    *key;

    key = sjoin(realm, ":", user, NULL);
    if (auth->users == 0 || !mprLookupKey(auth->users, key)) {
        return MPR_ERR_CANT_ACCESS;
    }
    mprRemoveKey(auth->users, key);
    return 0;
}


//  MOB - inconsistent. This takes a "group" string. httpRemoveUserFromGroup takes a "gp"
int httpRemoveUsersFromGroup(HttpAuth *auth, cchar *group, cchar *userList)
{
    HttpGroup   *gp;
    char        *user, *users, *tok;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    tok = NULL;
    users = sclone(userList);
    user = stok(users, " \t", &tok);
    while (user) {
        httpRemoveUserFromGroup(gp, user);
        user = stok(NULL, " \t", &tok);
    }
    return 0;
}


int httpSetGroupAcl(HttpAuth *auth, cchar *group, HttpAcl acl)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupKey(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    gp->acl = acl;
    return 0;
}


void httpSetRequiredAcl(HttpAuth *auth, HttpAcl acl)
{
    auth->requiredAcl = acl;
}


int httpRemoveUserFromGroup(HttpGroup *gp, cchar *user)
{
    char    *name;
    int     next;

    for (next = 0; (name = mprGetNextItem(gp->users, &next)) != 0; ) {
        if (strcmp(name, user) == 0) {
            mprRemoveItem(gp->users, name);
            return 0;
        }
    }
    return MPR_ERR_CANT_ACCESS;
}


int httpReadGroupFile(HttpAuth *auth, char *path)
{
    MprFile     *file;
    HttpAcl     acl;
    char        *buf;
    char        *users, *group, *enabled, *aclSpec, *tok, *cp;

    auth->groupFile = sclone(path);

    if ((file = mprOpenFile(path, O_RDONLY | O_TEXT, 0444)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    while ((buf = mprReadLine(file, MPR_BUFSIZE, NULL)) != NULL) {
        enabled = stok(buf, " :\t", &tok);
        for (cp = enabled; isspace((int) *cp); cp++) {
            ;
        }
        if (*cp == '\0' || *cp == '#') {
            continue;
        }
        aclSpec = stok(NULL, " :\t", &tok);
        group = stok(NULL, " :\t", &tok);
        users = stok(NULL, "\r\n", &tok);

        acl = httpParseAcl(auth, aclSpec);
        httpAddGroup(auth, group, acl, (*enabled == '0') ? 0 : 1);
        httpAddUsersToGroup(auth, group, users);
    }
    httpUpdateUserAcls(auth);
    mprCloseFile(file);
    return 0;
}


int httpReadUserFile(HttpAuth *auth, char *path)
{
    MprFile     *file;
    char        *buf;
    char        *enabled, *user, *password, *realm, *tok, *cp;

    auth->userFile = sclone(path);

    if ((file = mprOpenFile(path, O_RDONLY | O_TEXT, 0444)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    while ((buf = mprReadLine(file, MPR_BUFSIZE, NULL)) != NULL) {
        enabled = stok(buf, " :\t", &tok);
        for (cp = enabled; isspace((int) *cp); cp++) {
            ;
        }
        if (*cp == '\0' || *cp == '#') {
            continue;
        }
        user = stok(NULL, ":", &tok);
        realm = stok(NULL, ":", &tok);
        password = stok(NULL, " \t\r\n", &tok);

        user = strim(user, " \t", MPR_TRIM_BOTH);
        realm = strim(realm, " \t", MPR_TRIM_BOTH);
        password = strim(password, " \t", MPR_TRIM_BOTH);

        httpAddUser(auth, realm, user, password, (*enabled == '0' ? 0 : 1));
    }
    httpUpdateUserAcls(auth);
    mprCloseFile(file);
    return 0;
}


int httpWriteUserFile(HttpAuth *auth, char *path)
{
    MprFile         *file;
    MprKey          *kp;
    HttpUser        *up;
    char            buf[HTTP_MAX_PASS * 2];
    char            *tempFile;

    tempFile = mprGetTempPath(NULL);
    if ((file = mprOpenFile(tempFile, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0444)) == 0) {
        mprError("Can't open %s", tempFile);
        return MPR_ERR_CANT_OPEN;
    }
    kp = mprGetNextKey(auth->users, 0);
    while (kp) {
        up = (HttpUser*) kp->data;
        mprSprintf(buf, sizeof(buf), "%d: %s: %s: %s\n", up->enabled, up->name, up->realm, up->password);
        mprWriteFile(file, buf, (int) strlen(buf));
        kp = mprGetNextKey(auth->users, kp);
    }
    mprCloseFile(file);
    unlink(path);
    if (rename(tempFile, path) < 0) {
        mprError("Can't create new %s", path);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


int httpWriteGroupFile(HttpAuth *auth, char *path)
{
    MprKey          *kp;
    MprFile         *file;
    HttpGroup       *gp;
    char            buf[MPR_MAX_STRING], *tempFile, *name;
    int             next;

    tempFile = mprGetTempPath(NULL);
    if ((file = mprOpenFile(tempFile, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0444)) == 0) {
        mprError("Can't open %s", tempFile);
        return MPR_ERR_CANT_OPEN;
    }
    kp = mprGetNextKey(auth->groups, 0);
    while (kp) {
        gp = (HttpGroup*) kp->data;
        mprSprintf(buf, sizeof(buf), "%d: %x: %s: ", gp->enabled, gp->acl, gp->name);
        mprWriteFile(file, buf, strlen(buf));
        for (next = 0; (name = mprGetNextItem(gp->users, &next)) != 0; ) {
            mprWriteFile(file, name, strlen(name));
        }
        mprWriteFile(file, "\n", 1);
        kp = mprGetNextKey(auth->groups, kp);
    }
    mprCloseFile(file);
    unlink(path);
    if (rename(tempFile, path) < 0) {
        mprError("Can't create new %s", path);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}

#else
void __nativeAuthFile() {}
#endif /* BLD_FEATURE_AUTH_FILE */

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You httpy use the GPL open source license described below or you may acquire 
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
/************************************************************************/
/*
 *  End of file "./src/authFile.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/authPam.c"
 */
/************************************************************************/

/*
    authPam.c - Authorization using PAM (Pluggable Authorization Module)

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if BLD_FEATURE_AUTH_PAM && BLD_UNIX_LIKE

#if MACOSX
    #include    <pam/pam_appl.h>
#else
    #include    <security/pam_appl.h>
#endif


typedef struct {
    char    *name;
    char    *password;
} UserInfo;


static int pamChat(int msgCount, const struct pam_message **msg, struct pam_response **resp, void *data);


cchar *httpGetPamPassword(HttpAuth *auth, cchar *realm, cchar *user)
{
    /*  Can't return the password.
     */
    return "";
}


bool httpValidatePamCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, cchar *requiredPass, char **msg)
{
    pam_handle_t        *pamh;
    UserInfo            info;
    struct pam_conv     conv = { pamChat, &info };
    int                 res;
   
    info.name = (char*) user;
    info.password = (char*) password;
    pamh = NULL;
        
    if ((res = pam_start("login", user, &conv, &pamh)) != PAM_SUCCESS) {
        return 0;
    }
    if ((res = pam_authenticate(pamh, 0)) != PAM_SUCCESS) {
        return 0;
    }
    pam_end(pamh, PAM_SUCCESS);
    return 1;
}


/*  
    Callback invoked by the pam_authenticate function
 */
static int pamChat(int msgCount, const struct pam_message **msg, struct pam_response **resp, void *data) 
{
    UserInfo                *info;
    struct pam_response     *reply;
    int                     i;
    
    i = 0;
    reply = 0;
    info = (UserInfo*) data;

    if (resp == 0 || msg == 0 || info == 0) {
        return PAM_CONV_ERR;
    }
    if ((reply = malloc(msgCount * sizeof(struct pam_response))) == 0) {
        return PAM_CONV_ERR;
    }
    for (i = 0; i < msgCount; i++) {
        reply[i].resp_retcode = 0;
        reply[i].resp = 0;
        
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            reply[i].resp = strdup(info->name);
            break;

        case PAM_PROMPT_ECHO_OFF:
            reply[i].resp = strdup(info->password);
            break;

        default:
            return PAM_CONV_ERR;
        }
    }
    *resp = reply;
    return PAM_SUCCESS;
}


#else
void __pamAuth() {}
#endif /* BLD_FEATURE_AUTH_PAM */

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
/************************************************************************/
/*
 *  End of file "./src/authPam.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/cache.c"
 */
/************************************************************************/

/*
    cache.c -- Http request route caching 

    Caching operates as both a handler and an output filter. If acceptable cached content is found, the 
    cacheHandler will serve it instead of the normal handler. If no content is acceptable and caching is enabled
    for the request, the cacheFilter will capture and save the response.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void cacheAtClient(HttpConn *conn);
static bool fetchCachedResponse(HttpConn *conn);
static HttpCache *lookupCacheControl(HttpConn *conn);
static char *makeCacheKey(HttpConn *conn);
static void manageHttpCache(HttpCache *cache, int flags);
static int matchCacheFilter(HttpConn *conn, HttpRoute *route, int dir);
static int matchCacheHandler(HttpConn *conn, HttpRoute *route, int dir);
static void outgoingCacheFilterService(HttpQueue *q);
static void processCacheHandler(HttpQueue *q);
static void saveCachedResponse(HttpConn *conn);
static cchar *setHeadersFromCache(HttpConn *conn, cchar *content);


int httpOpenCacheHandler(Http *http)
{
    HttpStage     *handler, *filter;

    /*
        Create the cache handler to serve cached content 
     */
    if ((handler = httpCreateHandler(http, "cacheHandler", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->cacheHandler = handler;
    handler->match = matchCacheHandler;
    handler->process = processCacheHandler;

    /*
        Create the cache filter to capture and cache response content
     */
    if ((filter = httpCreateFilter(http, "cacheFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->cacheFilter = filter;
    filter->match = matchCacheFilter;
    filter->outgoingService = outgoingCacheFilterService;
    return 0;
}


/*
    See if there is acceptable cached content to serve
 */
static int matchCacheHandler(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpCache   *cache;

    mprAssert(route->caching);

    if ((cache = conn->tx->cache = lookupCacheControl(conn)) == 0) {
        /* Caching not configured for this route */
        return HTTP_ROUTE_REJECT;
    }
    if (cache->flags & HTTP_CACHE_CLIENT) {
        cacheAtClient(conn);
    }
    if (cache->flags & HTTP_CACHE_SERVER) {
        if (!(cache->flags & HTTP_CACHE_MANUAL) && fetchCachedResponse(conn)) {
            /* Found cached content */
            return HTTP_ROUTE_OK;
        }
        /*
            Caching is configured but no acceptable cached content. Create a capture buffer for the cacheFilter.
         */
        conn->tx->cacheBuffer = mprCreateBuf(-1, -1);
    }
    return HTTP_ROUTE_REJECT;
}


static void processCacheHandler(HttpQueue *q) 
{
    HttpConn    *conn;
    HttpTx      *tx;
    cchar       *data;

    conn = q->conn;
    tx = conn->tx;

    if (tx->cachedContent) {
        mprLog(3, "cacheHandler: write cached content for '%s'", conn->rx->uri);
        if ((data = setHeadersFromCache(conn, tx->cachedContent)) != 0) {
            tx->length = slen(data);
            httpWriteString(q, data);
        }
    }
    httpFinalize(conn);
}


static int matchCacheFilter(HttpConn *conn, HttpRoute *route, int dir)
{
    if ((dir & HTTP_STAGE_TX) && conn->tx->cacheBuffer) {
        mprLog(3, "cacheFilter: Cache response content for '%s'", conn->rx->uri);
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*
    This will be enabled when caching is enabled for the route and there is no acceptable cache data to use.
    OR - manual caching has been enabled.
 */
static void outgoingCacheFilterService(HttpQueue *q)
{
    HttpPacket  *packet, *data;
    HttpConn    *conn;
    HttpTx      *tx;
    MprKey      *kp;
    cchar       *cachedData;
    ssize       size;
    int         foundDataPacket;

    conn = q->conn;
    tx = conn->tx;
    foundDataPacket = 0;
    cachedData = 0;

    if (tx->status < 200 || tx->status > 299) {
        tx->cacheBuffer = 0;
    }

    /*
        This routine will save cached responses to tx->cacheBuffer.
        It will also send cached data if the X-SendCache header is present. Normal caching is done by cacheHandler
     */
    if (mprLookupKey(conn->tx->headers, "X-SendCache") != 0) {
        if (fetchCachedResponse(conn)) {
            mprLog(3, "cacheFilter: write cached content for '%s'", conn->rx->uri);
            cachedData = setHeadersFromCache(conn, tx->cachedContent);
            tx->length = slen(cachedData);
        }
    }
    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!httpWillNextQueueAcceptPacket(q, packet)) {
            httpPutBackPacket(q, packet);
            return;
        }
        if (packet->flags & HTTP_PACKET_HEADER) {
            if (!cachedData && tx->cacheBuffer) {
                /*
                    Add defined headers to the start of the cache buffer. Separate with a double newline.
                 */
                mprPutFmtToBuf(tx->cacheBuffer, "X-Status: %d\n", tx->status);
                for (kp = 0; (kp = mprGetNextKey(tx->headers, kp)) != 0; ) {
                    mprPutFmtToBuf(tx->cacheBuffer, "%s: %s\n", kp->key, kp->data);
                }
                mprPutCharToBuf(tx->cacheBuffer, '\n');
            }

        } else if (packet->flags & HTTP_PACKET_DATA) {
            if (cachedData) {
                /*
                    Using X-SendCache. Replace the data with the cached response.
                 */
                mprFlushBuf(packet->content);
                mprPutBlockToBuf(packet->content, cachedData, (ssize) tx->length);

            } else if (tx->cacheBuffer) {
                /*
                    Save the response packet to the cache buffer. Will write below in saveCachedResponse.
                 */
                size = mprGetBufLength(packet->content);
                if ((tx->cacheBufferLength + size) < conn->limits->cacheItemSize) {
                    mprPutBlockToBuf(tx->cacheBuffer, mprGetBufStart(packet->content), mprGetBufLength(packet->content));
                    tx->cacheBufferLength += size;
                } else {
                    tx->cacheBuffer = 0;
                    mprLog(3, "cacheFilter: Item too big to cache %d bytes, limit %d", tx->cacheBufferLength + size,
                        conn->limits->cacheItemSize);
                }
            }
            foundDataPacket = 1;

        } else if (packet->flags & HTTP_PACKET_END) {
            if (cachedData && !foundDataPacket) {
                /*
                    Using X-SendCache but there was no data packet to replace. So do the write here
                 */
                data = httpCreateDataPacket((ssize) tx->length);
                mprPutBlockToBuf(data->content, cachedData, (ssize) tx->length);
                httpPutPacketToNext(q, data);

            } else if (tx->cacheBuffer) {
                /*
                    Save the cache buffer to the cache store
                 */
                saveCachedResponse(conn);
            }
        }
        httpPutPacketToNext(q, packet);
    }
}


/*
    Find a qualifying cache control entry. Any configured uri,method,extension,type must match.
 */
static HttpCache *lookupCacheControl(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpCache   *cache;
    cchar       *mimeType, *ukey;
    int         next;

    rx = conn->rx;
    tx = conn->tx;

    /*
        Find first qualifying cache control entry. Any configured uri,method,extension,type must match.
     */
    for (next = 0; (cache = mprGetNextItem(rx->route->caching, &next)) != 0; ) {
        if (cache->uris) {
            if (cache->flags & HTTP_CACHE_ONLY) {
                ukey = sfmt("%s?%s", rx->pathInfo, httpGetParamsString(conn));
            } else {
                ukey = rx->pathInfo;
            }
            if (!mprLookupKey(cache->uris, ukey)) {
                continue;
            }
        }
        if (cache->methods && !mprLookupKey(cache->methods, rx->method)) {
            continue;
        }
        if (cache->extensions && !mprLookupKey(cache->extensions, tx->ext)) {
            continue;
        }
        if (cache->types) {
            if ((mimeType = (char*) mprLookupMime(rx->route->mimeTypes, tx->ext)) != 0) {
                if (!mprLookupKey(cache->types, mimeType)) {
                    continue;
                }
            }
        }
        /* All match */
        break;
    }
    return cache;
}


static void cacheAtClient(HttpConn *conn)
{
    HttpTx      *tx;
    HttpCache   *cache;
    cchar       *value;

    tx = conn->tx;
    cache = conn->tx->cache;

    if (!mprLookupKey(tx->headers, "Cache-Control")) {
        if ((value = mprLookupKey(conn->tx->headers, "Cache-Control")) != 0) {
            if (strstr(value, "max-age") == 0) {
                httpAppendHeader(conn, "Cache-Control", "max-age=%d", cache->clientLifespan / MPR_TICKS_PER_SEC);
            }
        } else {
            httpAddHeader(conn, "Cache-Control", "max-age=%d", cache->clientLifespan / MPR_TICKS_PER_SEC);
        }
#if UNUSED && KEEP
        {
            /* Old HTTP/1.0 clients don't understand Cache-Control */
            struct tm   tm;
            mprDecodeUniversalTime(&tm, conn->http->now + (expires * MPR_TICKS_PER_SEC));
            httpAddHeader(conn, "Expires", "%s", mprFormatTime(MPR_HTTP_DATE, &tm));
        }
#endif
    }
}


/*
    See if there is acceptable cached content for this request. If so, return true.
    Will setup tx->cacheBuffer as a side-effect if the output should be captured and cached.
 */
static bool fetchCachedResponse(HttpConn *conn)
{
    HttpTx      *tx;
    MprTime     modified, when;
    cchar       *value, *key, *tag;
    int         status, cacheOk, canUseClientCache;

    tx = conn->tx;

    /*
        Transparent caching. Manual caching must manually call httpWriteCached()
     */
    key = makeCacheKey(conn);
    if ((value = httpGetHeader(conn, "Cache-Control")) != 0 && 
            (scontains(value, "max-age=0", -1) == 0 || scontains(value, "no-cache", -1) == 0)) {
        mprLog(3, "Client reload. Cache-control header '%s' rejects use of cached content.", value);

    } else if ((tx->cachedContent = mprReadCache(conn->host->responseCache, key, &modified, 0)) != 0) {
        /*
            See if a NotModified response can be served. This is much faster than sending the response.
            Observe headers:
                If-None-Match: "ec18d-54-4d706a63"
                If-Modified-Since: Fri, 04 Mar 2011 04:28:19 GMT
            Set status to OK when content must be transmitted.
         */
        cacheOk = 1;
        canUseClientCache = 0;
        tag = mprGetMD5(key);
        if ((value = httpGetHeader(conn, "If-None-Match")) != 0) {
            canUseClientCache = 1;
            if (scmp(value, tag) != 0) {
                cacheOk = 0;
            }
        }
        if (cacheOk && (value = httpGetHeader(conn, "If-Modified-Since")) != 0) {
            canUseClientCache = 1;
            mprParseTime(&when, value, 0, 0);
            if (modified > when) {
                cacheOk = 0;
            }
        }
        status = (canUseClientCache && cacheOk) ? HTTP_CODE_NOT_MODIFIED : HTTP_CODE_OK;
        mprLog(3, "cacheHandler: Use cached content for %s, status %d", key, status);
        httpSetStatus(conn, status);
        httpSetHeader(conn, "Etag", mprGetMD5(key));
        httpSetHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
        return 1;
    }
    mprLog(3, "cacheHandler: No cached content for %s", key);
    return 0;
}


static void saveCachedResponse(HttpConn *conn)
{
    HttpTx      *tx;
    MprBuf      *buf;
    MprTime     modified;

    tx = conn->tx;

    mprAssert(conn->finalized && tx->cacheBuffer);
    buf = tx->cacheBuffer;
    mprAddNullToBuf(buf);
    tx->cacheBuffer = 0;
    /* 
        Truncate modified time to get a 1 sec resolution. This is the resolution for If-Modified headers.  
     */
    modified = mprGetTime() / MPR_TICKS_PER_SEC * MPR_TICKS_PER_SEC;
    mprWriteCache(conn->host->responseCache, makeCacheKey(conn), mprGetBufStart(buf), modified, 
        tx->cache->serverLifespan, 0, 0);
}


ssize httpWriteCached(HttpConn *conn)
{
    MprTime     modified;
    cchar       *cacheKey, *data, *content;

    if (!conn->tx->cache) {
        return MPR_ERR_CANT_FIND;
    }
    cacheKey = makeCacheKey(conn);
    if ((content = mprReadCache(conn->host->responseCache, cacheKey, &modified, 0)) == 0) {
        mprLog(3, "No cached data for ", cacheKey);
        return 0;
    }
    mprLog(5, "Used cached ", cacheKey);
    data = setHeadersFromCache(conn, content);
    httpSetHeader(conn, "Etag", mprGetMD5(cacheKey));
    httpSetHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
    conn->tx->cacheBuffer = 0;
    httpWriteString(conn->writeq, data);
    httpFinalize(conn);
    return slen(data);
}


ssize httpUpdateCache(HttpConn *conn, cchar *uri, cchar *data, MprTime lifespan)
{
    cchar   *key;
    ssize   len;

    len = slen(data);
    if (len > conn->limits->cacheItemSize) {
        return MPR_ERR_WONT_FIT;
    }
    if (lifespan <= 0) {
        lifespan = conn->rx->route->lifespan;
    }
    key = sfmt("http::response-%s", uri);
    if (data == 0 || lifespan <= 0) {
        mprRemoveCache(conn->host->responseCache, key);
        return 0;
    }
    return mprWriteCache(conn->host->responseCache, key, data, 0, lifespan, 0, 0);
}


/*
    Add cache configuration to the route. This can be called multiple times.
    Uris, extensions and methods may optionally provide a space or comma separated list of items.
    If URI is NULL or "*", cache all URIs for this route. Otherwise, cache only the given URIs. 
    The URIs may contain an ordered set of request parameters. For example: "/user/show?name=john&posts=true"
    Note: the URI should not include the route prefix (scriptName)
    The extensions should not contain ".". The methods may contain "*" for all methods.
 */
void httpAddCache(HttpRoute *route, cchar *methods, cchar *uris, cchar *extensions, cchar *types, MprTime clientLifespan, 
        MprTime serverLifespan, int flags)
{
    HttpCache   *cache;
    char        *item, *tok;

    cache = 0;
    if (!route->caching) {
        httpAddRouteHandler(route, "cacheHandler", "");
        httpAddRouteFilter(route, "cacheFilter", "", HTTP_STAGE_TX);
        route->caching = mprCreateList(0, 0);

    } else if (flags & HTTP_CACHE_RESET) {
        route->caching = mprCreateList(0, 0);

    } else if (route->parent && route->caching == route->parent->caching) {
        route->caching = mprCloneList(route->parent->caching);
    }
    if ((cache = mprAllocObj(HttpCache, manageHttpCache)) == 0) {
        return;
    }
    if (extensions) {
        cache->extensions = mprCreateHash(0, 0);
        for (item = stok(sclone(extensions), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (smatch(item, "*")) {
                extensions = 0;
            } else {
                mprAddKey(cache->extensions, item, cache);
            }
        }
    } else if (types) {
        cache->types = mprCreateHash(0, 0);
        for (item = stok(sclone(types), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (smatch(item, "*")) {
                extensions = 0;
            } else {
                mprAddKey(cache->types, item, cache);
            }
        }
    }
    if (methods) {
        cache->methods = mprCreateHash(0, MPR_HASH_CASELESS);
        for (item = stok(sclone(methods), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (smatch(item, "*")) {
                methods = 0;
            } else {
                mprAddKey(cache->methods, item, cache);
            }
        }
    }
    if (uris) {
        cache->uris = mprCreateHash(0, 0);
        for (item = stok(sclone(uris), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (flags & HTTP_CACHE_ONLY && route->prefix && !scontains(item, sfmt("prefix=%s", route->prefix), -1)) {
                /*
                    Auto-add ?prefix=ROUTE_NAME if there is no query
                 */
                if (!schr(item, '?')) {
                    item = sfmt("%s?prefix=%s", item, route->prefix); 
                }
            }
            mprAddKey(cache->uris, item, cache);
        }
    }
    if (clientLifespan <= 0) {
        clientLifespan = route->lifespan;
    }
    cache->clientLifespan = clientLifespan;
    if (serverLifespan <= 0) {
        serverLifespan = route->lifespan;
    }
    cache->serverLifespan = serverLifespan;
    cache->flags = flags;
    mprAddItem(route->caching, cache);

#if UNUSED && KEEP
    mprLog(3, "Caching route %s for methods %s, URIs %s, extensions %s, types %s, client lifespan %d, server lifespan %d", 
        route->name,
        (methods) ? methods: "*",
        (uris) ? uris: "*",
        (extensions) ? extensions: "*",
        (types) ? types: "*",
        cache->clientLifespan / MPR_TICKS_PER_SEC);
        cache->serverLifespan / MPR_TICKS_PER_SEC);
#endif
}


static void manageHttpCache(HttpCache *cache, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cache->extensions);
        mprMark(cache->methods);
        mprMark(cache->types);
        mprMark(cache->uris);
    }
}


static char *makeCacheKey(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (conn->tx->cache->flags & (HTTP_CACHE_ONLY | HTTP_CACHE_UNIQUE)) {
        return sfmt("http::response-%s?%s", rx->pathInfo, httpGetParamsString(conn));
    } else {
        return sfmt("http::response-%s", rx->pathInfo);
    }
}


/*
    Parse cached content of the form:  headers \n\n data
    Set headers in the current requeset and return a reference to the data portion
 */
static cchar *setHeadersFromCache(HttpConn *conn, cchar *content)
{
    cchar   *data;
    char    *header, *headers, *key, *value, *tok;

    if ((data = strstr(content, "\n\n")) == 0) {
        data = content;
    } else {
        headers = snclone(content, data - content);
        data += 2;
        for (header = stok(headers, "\n", &tok); header; header = stok(NULL, "\n", &tok)) {
            key = stok(header, ": ", &value);
            if (smatch(key, "X-Status")) {
                conn->tx->status = (int) stoi(value);
            } else {
                httpAddHeader(conn, key, value);
            }
        }
    }
    return data;
}


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
/************************************************************************/
/*
 *  End of file "./src/cache.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/chunkFilter.c"
 */
/************************************************************************/

/*
    chunkFilter.c - Transfer chunk endociding filter.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static int matchChunk(HttpConn *conn, HttpRoute *route, int dir);
static void openChunk(HttpQueue *q);
static void outgoingChunkService(HttpQueue *q);
static void setChunkPrefix(HttpQueue *q, HttpPacket *packet);

/* 
   Loadable module initialization
 */
int httpOpenChunkFilter(Http *http)
{
    HttpStage     *filter;

    mprLog(5, "Open chunk filter");
    if ((filter = httpCreateFilter(http, "chunkFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->chunkFilter = filter;
    filter->match = matchChunk; 
    filter->open = openChunk; 
    filter->outgoingService = outgoingChunkService; 
    return 0;
}


static int matchChunk(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpTx  *tx;

    if (dir & HTTP_STAGE_TX) {
        /*
            Don't match if chunking is explicitly turned off vi a the X_APPWEB_CHUNK_SIZE header which sets the chunk 
            size to zero. Also remove if the response length is already known.
         */
        tx = conn->tx;
        if (tx->length < 0 && tx->chunkSize != 0) {
            return HTTP_ROUTE_OK;
        }
        return HTTP_ROUTE_REJECT;
    }
    /* 
        Must always be ready to handle chunked response data. Clients create their incoming pipeline before it is
        know what the response data looks like (chunked or not).
     */
    return HTTP_ROUTE_OK;
}


static void openChunk(HttpQueue *q)
{
    HttpConn    *conn;

    conn = q->conn;
    q->packetSize = min(conn->limits->chunkSize, q->max);
}


/*  
    Filter chunk headers and leave behind pure data. This is called for chunked and unchunked data.
    Chunked data format is:
        Chunk spec <CRLF>
        Data <CRLF>
        Chunk spec (size == 0) <CRLF>
        <CRLF>
    Chunk spec is: "HEX_COUNT; chunk length DECIMAL_COUNT\r\n". The "; chunk length DECIMAL_COUNT is optional.
    As an optimization, use "\r\nSIZE ...\r\n" as the delimiter so that the CRLF after data does not special consideration.
    Achive this by parseHeaders reversing the input start by 2.
 */
ssize httpFilterChunkData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *buf;
    ssize       chunkSize;
    char        *start, *cp;
    int         bad;

    conn = q->conn;
    rx = conn->rx;
    mprAssert(packet);
    buf = packet->content;
    mprAssert(buf);

    switch (rx->chunkState) {
    case HTTP_CHUNK_UNCHUNKED:
        return (ssize) min(rx->remainingContent, mprGetBufLength(buf));

    case HTTP_CHUNK_DATA:
        mprLog(7, "chunkFilter: data %d bytes, rx->remainingContent %d", httpGetPacketLength(packet), rx->remainingContent);
        if (rx->remainingContent > 0) {
            return (ssize) min(rx->remainingContent, mprGetBufLength(buf));
        }
        /* End of chunk - prep for the next chunk */
        rx->remainingContent = HTTP_BUFSIZE;
        rx->chunkState = HTTP_CHUNK_START;
        /* Fall through */

    case HTTP_CHUNK_START:
        /*  
            Validate:  "\r\nSIZE.*\r\n"
         */
        if (mprGetBufLength(buf) < 5) {
            return MPR_ERR_NOT_READY;
        }
        start = mprGetBufStart(buf);
        bad = (start[0] != '\r' || start[1] != '\n');
        for (cp = &start[2]; cp < buf->end && *cp != '\n'; cp++) {}
        if (*cp != '\n' && (cp - start) < 80) {
            return MPR_ERR_NOT_READY;
        }
        bad += (cp[-1] != '\r' || cp[0] != '\n');
        if (bad) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return 0;
        }
        chunkSize = (int) stoiradix(&start[2], 16, NULL);
        if (!isxdigit((int) start[2]) || chunkSize < 0) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return 0;
        }
        mprAdjustBufStart(buf, (cp - start + 1));
        /* Remaining content is set to the next chunk size */
        rx->remainingContent = chunkSize;
        if (chunkSize == 0) {
            /*
                We are lenient if the request does not have a trailing "\r\n" after the last chunk
             */
            cp = mprGetBufStart(buf);
            if (mprGetBufLength(buf) == 2 && *cp == '\r' && cp[1] == '\n') {
                mprAdjustBufStart(buf, 2);
            }
            rx->chunkState = HTTP_CHUNK_EOF;
            rx->eof = 1;
        } else {
            rx->chunkState = HTTP_CHUNK_DATA;
        }
        mprLog(7, "chunkFilter: start incoming chunk of %d bytes", chunkSize);
        return min(chunkSize, mprGetBufLength(buf));

    default:
        mprError("chunkFilter: bad state %d", rx->chunkState);
    }
    return 0;
}


static void outgoingChunkService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;
    HttpTx      *tx;
    cchar       *value;

    conn = q->conn;
    tx = conn->tx;

    if (!(q->flags & HTTP_QUEUE_SERVICED)) {
        /*  
            If we don't know the content length (tx->length < 0) and if the last packet is the end packet. Then
            we have all the data. Thus we can determine the actual content length and can bypass the chunk handler.
         */
        if (tx->length < 0 && (value = mprLookupKey(tx->headers, "Content-Length")) != 0) {
            tx->length = stoi(value);
        }
        if (tx->length < 0) {
            if (q->last->flags & HTTP_PACKET_END) {
                if (tx->chunkSize < 0 && tx->length <= 0) {
                    /*  
                        Set the response content length and thus disable chunking -- not needed as we know the entity length.
                     */
                    tx->length = (int) q->count;
                }
            } else {
                if (tx->chunkSize < 0) {
                    tx->chunkSize = min(conn->limits->chunkSize, q->max);
                }
            }
        }
    }
    if (tx->chunkSize <= 0) {
        httpDefaultOutgoingServiceStage(q);
    } else {
        for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
            if (!(packet->flags & HTTP_PACKET_HEADER)) {
                httpPutBackPacket(q, packet);
                httpJoinPackets(q, tx->chunkSize);
                packet = httpGetPacket(q);
                if (httpGetPacketLength(packet) > tx->chunkSize) {
                    httpResizePacket(q, packet, tx->chunkSize);
                }
            }
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            if (!(packet->flags & HTTP_PACKET_HEADER)) {
                setChunkPrefix(q, packet);
            }
            httpPutPacketToNext(q, packet);
        }
    }
}


static void setChunkPrefix(HttpQueue *q, HttpPacket *packet)
{
    if (packet->prefix) {
        return;
    }
    packet->prefix = mprCreateBuf(32, 32);
    /*  
        NOTE: prefixes don't count in the queue length. No need to adjust q->count
     */
    if (httpGetPacketLength(packet)) {
        mprPutFmtToBuf(packet->prefix, "\r\n%x\r\n", httpGetPacketLength(packet));
    } else {
        mprPutStringToBuf(packet->prefix, "\r\n0\r\n\r\n");
    }
}


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
/************************************************************************/
/*
 *  End of file "./src/chunkFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/client.c"
 */
/************************************************************************/

/*
    client.c -- Client side specific support.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static HttpConn *openConnection(HttpConn *conn, cchar *url)
{
    Http        *http;
    HttpUri     *uri;
    MprSocket   *sp;
    char        *ip;
    int         port, rc, level;

    mprAssert(conn);

    http = conn->http;
    uri = httpCreateUri(url, 0);

    if (uri->secure) {
#if BLD_FEATURE_SSL
        if (!http->sslLoaded) {
            if (!mprLoadSsl(0)) {
                mprError("Can't load SSL provider");
                return 0;
            }
            http->sslLoaded = 1;
        }
#else
        httpError(conn, HTTP_CODE_COMMS_ERROR, "SSL communications support not included in build");
        return 0;
#endif
    }
    if (*url == '/') {
        ip = (http->proxyHost) ? http->proxyHost : http->defaultClientHost;
        port = (http->proxyHost) ? http->proxyPort : http->defaultClientPort;
    } else {
        ip = (http->proxyHost) ? http->proxyHost : uri->host;
        port = (http->proxyHost) ? http->proxyPort : uri->port;
    }
    if (port == 0) {
        port = (uri->secure) ? 443 : 80;
    }
    if (conn && conn->sock) {
        if (--conn->keepAliveCount < 0 || port != conn->port || strcmp(ip, conn->ip) != 0) {
            httpCloseConn(conn);
        } else {
            mprLog(4, "Http: reusing keep-alive socket on: %s:%d", ip, port);
        }
    }
    if (conn->sock) {
        return conn;
    }
    if ((sp = mprCreateSocket((uri->secure) ? MPR_SECURE_CLIENT: NULL)) == 0) {
        httpError(conn, HTTP_CODE_COMMS_ERROR, "Can't create socket for %s", url);
        return 0;
    }
    rc = mprConnectSocket(sp, ip, port, 0);
    if (rc < 0) {
        httpError(conn, HTTP_CODE_COMMS_ERROR, "Can't open socket on %s:%d", ip, port);
        return 0;
    }
    conn->sock = sp;
    conn->ip = sclone(ip);
    conn->port = port;
    conn->secure = uri->secure;
    conn->keepAliveCount = (conn->limits->keepAliveCount) ? conn->limits->keepAliveCount : -1;

    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_CONN, NULL)) >= 0) {
        mprLog(level, "### Outgoing connection from %s:%d to %s:%d", 
            conn->ip, conn->port, conn->sock->ip, conn->sock->port);
    }
    return conn;
}


/*  
    Define headers and create an empty header packet that will be filled later by the pipeline.
 */
static int setClientHeaders(HttpConn *conn)
{
    Http        *http;
    HttpTx      *tx;
    HttpUri     *parsedUri;
    char        *encoded;

    mprAssert(conn);

    http = conn->http;
    tx = conn->tx;
    parsedUri = tx->parsedUri;
    if (conn->authType && strcmp(conn->authType, "basic") == 0) {
        char    abuf[MPR_MAX_STRING];
        mprSprintf(abuf, sizeof(abuf), "%s:%s", conn->authUser, conn->authPassword);
        encoded = mprEncode64(abuf);
        httpAddHeader(conn, "Authorization", "basic %s", encoded);
        conn->sentCredentials = 1;

    } else if (conn->authType && strcmp(conn->authType, "digest") == 0) {
        char    a1Buf[256], a2Buf[256], digestBuf[256];
        char    *ha1, *ha2, *digest, *qop;
        if (http->secret == 0 && httpCreateSecret(http) < 0) {
            mprLog(MPR_ERROR, "Http: Can't create secret for digest authentication");
            return MPR_ERR_CANT_CREATE;
        }
        conn->authCnonce = sfmt("%s:%s:%x", http->secret, conn->authRealm, (int) http->now);

        mprSprintf(a1Buf, sizeof(a1Buf), "%s:%s:%s", conn->authUser, conn->authRealm, conn->authPassword);
        ha1 = mprGetMD5(a1Buf);
        mprSprintf(a2Buf, sizeof(a2Buf), "%s:%s", tx->method, parsedUri->path);
        ha2 = mprGetMD5(a2Buf);
        qop = (conn->authQop) ? conn->authQop : (char*) "";

        conn->authNc++;
        if (scasecmp(conn->authQop, "auth") == 0) {
            mprSprintf(digestBuf, sizeof(digestBuf), "%s:%s:%08x:%s:%s:%s",
                ha1, conn->authNonce, conn->authNc, conn->authCnonce, conn->authQop, ha2);
        } else if (scasecmp(conn->authQop, "auth-int") == 0) {
            mprSprintf(digestBuf, sizeof(digestBuf), "%s:%s:%08x:%s:%s:%s",
                ha1, conn->authNonce, conn->authNc, conn->authCnonce, conn->authQop, ha2);
        } else {
            qop = "";
            mprSprintf(digestBuf, sizeof(digestBuf), "%s:%s:%s", ha1, conn->authNonce, ha2);
        }
        digest = mprGetMD5(digestBuf);

        if (*qop == '\0') {
            httpAddHeader(conn, "Authorization", "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", "
                "uri=\"%s\", response=\"%s\"",
                conn->authUser, conn->authRealm, conn->authNonce, parsedUri->path, digest);

        } else if (strcmp(qop, "auth") == 0) {
            httpAddHeader(conn, "Authorization", "Digest username=\"%s\", realm=\"%s\", domain=\"%s\", "
                "algorithm=\"MD5\", qop=\"%s\", cnonce=\"%s\", nc=\"%08x\", nonce=\"%s\", opaque=\"%s\", "
                "stale=\"FALSE\", uri=\"%s\", response=\"%s\"",
                conn->authUser, conn->authRealm, conn->authDomain, conn->authQop, conn->authCnonce, conn->authNc,
                conn->authNonce, conn->authOpaque, parsedUri->path, digest);

        } else if (strcmp(qop, "auth-int") == 0) {
            ;
        }
        conn->sentCredentials = 1;
    }
    if (conn->port != 80) {
        httpSetHeader(conn, "Host", "%s:%d", conn->ip, conn->port);
    } else {
        httpSetHeaderString(conn, "Host", conn->ip);
    }

    if (strcmp(conn->protocol, "HTTP/1.1") == 0) {
        /* If zero, we ask the client to close one request early. This helps with client led closes */
        if (conn->keepAliveCount > 0) {
            httpSetHeaderString(conn, "Connection", "Keep-Alive");
        } else {
            httpSetHeaderString(conn, "Connection", "close");
        }

    } else {
        /* Set to zero to let the client initiate the close */
        conn->keepAliveCount = 0;
        httpSetHeaderString(conn, "Connection", "close");
    }
    return 0;
}


int httpConnect(HttpConn *conn, cchar *method, cchar *url)
{
    mprAssert(conn);
    mprAssert(method && *method);
    mprAssert(url && *url);

    if (conn->endpoint) {
        httpError(conn, HTTP_CODE_BAD_GATEWAY, "Can't call connect in a server");
        return MPR_ERR_BAD_STATE;
    }
    mprLog(4, "Http: client request: %s %s", method, url);

    if (conn->tx == 0 || conn->state != HTTP_STATE_BEGIN) {
        /* WARNING: this will erase headers */
        httpPrepClientConn(conn, 0);
    }
    mprAssert(conn->state == HTTP_STATE_BEGIN);
    httpSetState(conn, HTTP_STATE_CONNECTED);
    conn->sentCredentials = 0;

    conn->tx->method = supper(method);
    conn->tx->parsedUri = httpCreateUri(url, 0);

#if BLD_DEBUG
    conn->startTime = conn->http->now;
    conn->startTicks = mprGetTicks();
#endif
    if (openConnection(conn, url) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    httpCreateTxPipeline(conn, conn->http->clientRoute);

    if (setClientHeaders(conn) < 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    return 0;
}


/*  
    Check the response for authentication failures and redirections. Return true if a retry is requried.
 */
bool httpNeedRetry(HttpConn *conn, char **url)
{
    HttpRx      *rx;

    mprAssert(conn->rx);

    *url = 0;
    rx = conn->rx;

    if (conn->state < HTTP_STATE_FIRST) {
        return 0;
    }
    if (rx->status == HTTP_CODE_UNAUTHORIZED) {
        if (conn->authUser == 0) {
            httpFormatError(conn, rx->status, "Authentication required");
        } else if (conn->sentCredentials) {
            httpFormatError(conn, rx->status, "Authentication failed");
        } else {
            return 1;
        }
    } else if (HTTP_CODE_MOVED_PERMANENTLY <= rx->status && rx->status <= HTTP_CODE_MOVED_TEMPORARILY && 
            conn->followRedirects) {
        if (rx->redirect) {
            *url = rx->redirect;
            return 1;
        }
        httpFormatError(conn, rx->status, "Missing location header");
        return -1;
    }
    return 0;
}


/*  
    Set the request as being a multipart mime upload. This defines the content type and defines a multipart mime boundary
 */
void httpEnableUpload(HttpConn *conn)
{
    conn->boundary = sfmt("--BOUNDARY--%Ld", conn->http->now);
    httpSetHeader(conn, "Content-Type", "multipart/form-data; boundary=%s", &conn->boundary[2]);
}


static int blockingFileCopy(HttpConn *conn, cchar *path)
{
    MprFile     *file;
    char        buf[MPR_BUFSIZE];
    ssize       bytes, nbytes, offset;
    int         oldMode;

    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        mprError("Can't open %s", path);
        return MPR_ERR_CANT_OPEN;
    }
    mprAddRoot(file);
    oldMode = mprSetSocketBlockingMode(conn->sock, 1);
    while ((bytes = mprReadFile(file, buf, sizeof(buf))) > 0) {
        offset = 0;
        while (bytes > 0) {
            if ((nbytes = httpWriteBlock(conn->writeq, &buf[offset], bytes)) < 0) {
                mprCloseFile(file);
                mprRemoveRoot(file);
                return MPR_ERR_CANT_WRITE;
            }
            bytes -= nbytes;
            offset += nbytes;
            mprAssert(bytes >= 0);
        }
        mprYield(0);
    }
    httpFlushQueue(conn->writeq, 1);
    mprSetSocketBlockingMode(conn->sock, oldMode);
    mprCloseFile(file);
    mprRemoveRoot(file);
    return 0;
}


/*  
    Write upload data. This routine blocks. If you need non-blocking ... cut and paste.
 */
ssize httpWriteUploadData(HttpConn *conn, MprList *fileData, MprList *formData)
{
    char    *path, *pair, *key, *value, *name;
    ssize   rc;
    int     next;

    rc = 0;
    if (formData) {
        for (rc = next = 0; rc >= 0 && (pair = mprGetNextItem(formData, &next)) != 0; ) {
            key = stok(sclone(pair), "=", &value);
            rc += httpWrite(conn->writeq, "%s\r\nContent-Disposition: form-data; name=\"%s\";\r\n", conn->boundary, key);
            rc += httpWrite(conn->writeq, "Content-Type: application/x-www-form-urlencoded\r\n\r\n%s\r\n", value);
        }
    }
    if (fileData) {
        for (rc = next = 0; rc >= 0 && (path = mprGetNextItem(fileData, &next)) != 0; ) {
            name = mprGetPathBase(path);
            rc += httpWrite(conn->writeq, "%s\r\nContent-Disposition: form-data; name=\"file%d\"; filename=\"%s\"\r\n", 
                conn->boundary, next - 1, name);
            rc += httpWrite(conn->writeq, "Content-Type: %s\r\n\r\n", mprLookupMime(MPR->mimeTypes, path));
            rc += blockingFileCopy(conn, path);
            rc += httpWrite(conn->writeq, "\r\n");
        }
    }
    rc += httpWrite(conn->writeq, "%s--\r\n--", conn->boundary);
    httpFinalize(conn);
    return rc;
}


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
/************************************************************************/
/*
 *  End of file "./src/client.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/conn.c"
 */
/************************************************************************/

/*
    conn.c -- Connection module to handle individual HTTP connections.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageConn(HttpConn *conn, int flags);
static HttpPacket *getPacket(HttpConn *conn, ssize *bytesToRead);
static void readEvent(HttpConn *conn);
static void writeEvent(HttpConn *conn);

/*
    Create a new connection object.
 */
HttpConn *httpCreateConn(Http *http, HttpEndpoint *endpoint, MprDispatcher *dispatcher)
{
    HttpConn    *conn;
    HttpHost    *host;
    HttpRoute   *route;

    if ((conn = mprAllocObj(HttpConn, manageConn)) == 0) {
        return 0;
    }
    conn->http = http;
    conn->canProceed = 1;

    conn->protocol = http->protocol;
    conn->port = -1;
    conn->retries = HTTP_RETRIES;
    conn->endpoint = endpoint;
    conn->lastActivity = http->now;
    conn->ioCallback = httpEvent;

    if (endpoint) {
        conn->notifier = endpoint->notifier;
        host = mprGetFirstItem(endpoint->hosts);
        if (host && (route = host->defaultRoute) != 0) {
            conn->limits = route->limits;
            conn->trace[0] = route->trace[0];
            conn->trace[1] = route->trace[1];
        } else {
            conn->limits = http->serverLimits;
            httpInitTrace(conn->trace);
        }
    } else {
        conn->limits = http->clientLimits;
        httpInitTrace(conn->trace);
    }
    conn->keepAliveCount = conn->limits->keepAliveCount;
    conn->serviceq = httpCreateQueueHead(conn, "serviceq");

    if (dispatcher) {
        conn->dispatcher = dispatcher;
    } else if (endpoint) {
        conn->dispatcher = endpoint->dispatcher;
    } else {
        conn->dispatcher = mprGetDispatcher();
    }
    httpSetState(conn, HTTP_STATE_BEGIN);
    httpAddConn(http, conn);
    return conn;
}


/*
    Destroy a connection. This removes the connection from the list of connections. Should GC after that.
 */
void httpDestroyConn(HttpConn *conn)
{
    if (conn->http) {
        mprAssert(conn->http);
        httpRemoveConn(conn->http, conn);
        if (conn->endpoint) {
            if (conn->state >= HTTP_STATE_PARSED) {
                httpValidateLimits(conn->endpoint, HTTP_VALIDATE_CLOSE_REQUEST, conn);
            }
            httpValidateLimits(conn->endpoint, HTTP_VALIDATE_CLOSE_CONN, conn);
        }
        if (HTTP_STATE_PARSED <= conn->state && conn->state < HTTP_STATE_COMPLETE) {
            HTTP_NOTIFY(conn, HTTP_STATE_COMPLETE, 0);
        }
        HTTP_NOTIFY(conn, HTTP_EVENT_CLOSE, 0);
        conn->input = 0;
        if (conn->tx) {
            httpDestroyPipeline(conn);
            conn->tx->conn = 0;
            conn->tx = 0;
        }
        if (conn->rx) {
            conn->rx->conn = 0;
            conn->rx = 0;
        }
        httpCloseConn(conn);
        conn->http = 0;
    }
}


static void manageConn(HttpConn *conn, int flags)
{
    mprAssert(conn);

    if (flags & MPR_MANAGE_MARK) {
        mprMark(conn->rx);
        mprMark(conn->tx);
        mprMark(conn->endpoint);
        mprMark(conn->host);
        mprMark(conn->limits);
        mprMark(conn->http);
        mprMark(conn->stages);
        mprMark(conn->dispatcher);
        mprMark(conn->newDispatcher);
        mprMark(conn->oldDispatcher);
        mprMark(conn->waitHandler);
        mprMark(conn->sock);
        mprMark(conn->serviceq);
        mprMark(conn->currentq);
        mprMark(conn->input);
        mprMark(conn->readq);
        mprMark(conn->writeq);
        mprMark(conn->connectorq);
        mprMark(conn->timeoutEvent);
        mprMark(conn->workerEvent);
        mprMark(conn->context);
        mprMark(conn->ejs);
        mprMark(conn->pool);
        mprMark(conn->mark);
        mprMark(conn->data);
        mprMark(conn->grid);
        mprMark(conn->record);
        mprMark(conn->boundary);
        mprMark(conn->errorMsg);
        mprMark(conn->ip);
        mprMark(conn->protocol);
        httpManageTrace(&conn->trace[0], flags);
        httpManageTrace(&conn->trace[1], flags);

        mprMark(conn->authCnonce);
        mprMark(conn->authDomain);
        mprMark(conn->authNonce);
        mprMark(conn->authOpaque);
        mprMark(conn->authRealm);
        mprMark(conn->authQop);
        mprMark(conn->authType);
        mprMark(conn->authUser);
        mprMark(conn->authPassword);
        mprMark(conn->headersCallbackArg);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyConn(conn);
    }
}


/*  
    Close the connection but don't destroy the conn object.
 */
void httpCloseConn(HttpConn *conn)
{
    mprAssert(conn);

    if (conn->sock) {
        mprLog(6, "Closing connection");
        if (conn->waitHandler) {
            mprRemoveWaitHandler(conn->waitHandler);
            conn->waitHandler = 0;
        }
        mprCloseSocket(conn->sock, 0);
        conn->sock = 0;
    }
}


void httpConnTimeout(HttpConn *conn)
{
    HttpLimits  *limits;
    MprTime     now;

    if (!conn->http) {
        return;
    }
    now = conn->http->now;
    limits = conn->limits;
    mprAssert(limits);

    mprLog(6, "Inactive connection timed out");
    if (conn->state >= HTTP_STATE_PARSED) {
        if ((conn->lastActivity + limits->inactivityTimeout) < now) {
            httpError(conn, HTTP_CODE_REQUEST_TIMEOUT,
                "Exceeded inactivity timeout of %Ld sec", limits->inactivityTimeout / 1000);

        } else if ((conn->started + limits->requestTimeout) < now) {
            httpError(conn, HTTP_CODE_REQUEST_TIMEOUT, "Exceeded timeout %d sec", limits->requestTimeout / 1000);
        }
        httpFinalize(conn);
    }
    httpDisconnect(conn);
    httpDiscardData(conn->writeq, 1);
    httpEnableConnEvents(conn);
    conn->timeoutEvent = 0;
}


static void commonPrep(HttpConn *conn)
{
    Http    *http;

    http = conn->http;
    lock(http);

    if (conn->timeoutEvent) {
        mprRemoveEvent(conn->timeoutEvent);
    }
    conn->canProceed = 1;
    conn->error = 0;
    conn->connError = 0;
    conn->errorMsg = 0;
    conn->state = 0;
    conn->responded = 0;
    conn->finalized = 0;
    conn->refinalize = 0;
    conn->connectorComplete = 0;
    conn->lastActivity = conn->http->now;
    httpSetState(conn, HTTP_STATE_BEGIN);
    httpInitSchedulerQueue(conn->serviceq);
    unlock(http);
}


/*  
    Prepare a connection for a new request after completing a prior request.
 */
void httpPrepServerConn(HttpConn *conn)
{
    mprAssert(conn);
    mprAssert(conn->rx == 0);
    mprAssert(conn->tx == 0);
    mprAssert(conn->endpoint);

    conn->readq = 0;
    conn->writeq = 0;
    commonPrep(conn);
}


void httpPrepClientConn(HttpConn *conn, bool keepHeaders)
{
    MprHash     *headers;

    mprAssert(conn);

    if (conn->keepAliveCount >= 0 && conn->sock) {
        /* Eat remaining input incase last request did not consume all data */
        httpConsumeLastRequest(conn);
    } else {
        conn->input = 0;
    }
    if (conn->tx) {
        conn->tx->conn = 0;
    }
    headers = (keepHeaders && conn->tx) ? conn->tx->headers: NULL;
    conn->tx = httpCreateTx(conn, headers);
    if (conn->rx) {
        conn->rx->conn = 0;
    }
    conn->rx = httpCreateRx(conn);
    commonPrep(conn);
}


void httpConsumeLastRequest(HttpConn *conn)
{
    MprTime     mark;
    char        junk[4096];

    if (!conn->sock) {
        return;
    }
    if (conn->state >= HTTP_STATE_FIRST) {
        mark = conn->http->now;
        while (!httpIsEof(conn) && mprGetRemainingTime(mark, conn->limits->requestTimeout) > 0) {
            if (httpRead(conn, junk, sizeof(junk)) <= 0) {
                break;
            }
        }
    }
    if (HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE) {
        conn->keepAliveCount = -1;
    }
}


void httpCallEvent(HttpConn *conn, int mask)
{
    MprEvent    e;

    if (conn->http) {
        e.mask = mask;
        e.timestamp = conn->http->now;
        httpEvent(conn, &e);
    }
}


/*  
    IO event handler. This is invoked by the wait subsystem in response to I/O events. It is also invoked via 
    relay when an accept event is received by the server. Initially the conn->dispatcher will be set to the
    server->dispatcher and the first I/O event will be handled on the server thread (or main thread). A request handler
    may create a new conn->dispatcher and transfer execution to a worker thread if required.
 */
void httpEvent(HttpConn *conn, MprEvent *event)
{
    LOG(7, "httpEvent for fd %d, mask %d\n", conn->sock->fd, event->mask);
    conn->lastActivity = conn->http->now;

    if (event->mask & MPR_WRITABLE) {
        writeEvent(conn);
    }
    if (event->mask & MPR_READABLE) {
        readEvent(conn);
    }
    mprAssert(conn->sock);

    if (conn->endpoint) {
        if (conn->error) {
            httpDestroyConn(conn);

        } else if (conn->keepAliveCount < 0 && conn->state <= HTTP_STATE_CONNECTED) {
            /*  
                Idle connection.
                NOTE: compare keepAliveCount with "< 0" so that the client can have one more keep alive request. 
                It should respond to the "Connection: close" and thus initiate a client-led close. This reduces 
                TIME_WAIT states on the server. NOTE: after httpDestroyConn, conn structure and memory is still 
                intact but conn->sock is zero.
             */
            httpDestroyConn(conn);

        } else if (mprIsSocketEof(conn->sock)) {
            httpDestroyConn(conn);

        } else {
            mprAssert(conn->state < HTTP_STATE_COMPLETE);
            httpEnableConnEvents(conn);
        }
    } else if (conn->state < HTTP_STATE_COMPLETE) {
        httpEnableConnEvents(conn);
    }
    mprYield(0);
}


/*
    Process a socket readable event
 */
static void readEvent(HttpConn *conn)
{
    HttpPacket  *packet;
    ssize       nbytes, size;

    while (!conn->connError && (packet = getPacket(conn, &size)) != 0) {
        nbytes = mprReadSocket(conn->sock, mprGetBufEnd(packet->content), size);
        LOG(8, "http: read event. Got %d", nbytes);
       
        if (nbytes > 0) {
            mprAdjustBufEnd(packet->content, nbytes);
            httpProcess(conn, packet);

        } else if (nbytes < 0) {
            if (conn->state <= HTTP_STATE_CONNECTED) {
                if (mprIsSocketEof(conn->sock)) {
                    conn->keepAliveCount = -1;
                }
                break;
            } else if (conn->state < HTTP_STATE_COMPLETE) {
                httpProcess(conn, packet);
                if (!conn->error && conn->state < HTTP_STATE_COMPLETE && mprIsSocketEof(conn->sock)) {
                    httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "Connection lost");
                    break;
                }
            }
            break;
        }
        //  MOB - refactor these tests
        if (nbytes == 0 || conn->state >= HTTP_STATE_RUNNING || !conn->canProceed) {
            break;
        }
        if (conn->readq && conn->readq->count > conn->readq->max) {
            break;
        }
        if (mprDispatcherHasEvents(conn->dispatcher)) {
            break;
        }
    }
}


static void writeEvent(HttpConn *conn)
{
    LOG(6, "httpProcessWriteEvent, state %d", conn->state);

    conn->writeBlocked = 0;
    if (conn->tx) {
        httpEnableQueue(conn->connectorq);
        httpServiceQueues(conn);
        httpProcess(conn, NULL);
    }
}


void httpUseWorker(HttpConn *conn, MprDispatcher *dispatcher, MprEvent *event)
{
    //  MOB -- locking should not be needed
    lock(conn->http);
    conn->oldDispatcher = conn->dispatcher;
    conn->dispatcher = dispatcher;
    conn->worker = 1;

    mprAssert(!conn->workerEvent);
    conn->workerEvent = event;
    unlock(conn->http);
}


void httpUsePrimary(HttpConn *conn)
{
    //  MOB -- locking should not be needed
    lock(conn->http);
    mprAssert(conn->worker);
    mprAssert(conn->state == HTTP_STATE_BEGIN);
    mprAssert(conn->oldDispatcher && conn->dispatcher != conn->oldDispatcher);

    conn->dispatcher = conn->oldDispatcher;
    conn->oldDispatcher = 0;
    conn->worker = 0;
    unlock(conn->http);
}


void httpEnableConnEvents(HttpConn *conn)
{
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q;
    MprEvent    *event;
    int         eventMask;

    mprLog(7, "EnableConnEvents");

    if (!conn->async || !conn->sock || mprIsSocketEof(conn->sock)) {
        return;
    }
    tx = conn->tx;
    rx = conn->rx;
    eventMask = 0;
    conn->lastActivity = conn->http->now;

    if (conn->workerEvent) {
        event = conn->workerEvent;
        conn->workerEvent = 0;
        mprQueueEvent(conn->dispatcher, event);

    } else {
        //  MOB - why locking here?
        lock(conn->http);
        if (tx) {
            /*
                Can be writeBlocked with data in the iovec and none in the queue
             */
            if (conn->writeBlocked || (conn->connectorq && conn->connectorq->count > 0)) {
                eventMask |= MPR_WRITABLE;
            }
            /*
                Enable read events if the read queue is not full. 
             */
            q = tx->queue[HTTP_QUEUE_RX]->nextQ;
            if (q->count < q->max || rx->form) {
                eventMask |= MPR_READABLE;
            }
        } else {
            eventMask |= MPR_READABLE;
        }
        if (eventMask) {
            if (conn->waitHandler == 0) {
                conn->waitHandler = mprCreateWaitHandler(conn->sock->fd, eventMask, conn->dispatcher, conn->ioCallback, 
                    conn, 0);
            } else {
                //  MOB API for this
                conn->waitHandler->dispatcher = conn->dispatcher;
                mprWaitOn(conn->waitHandler, eventMask);
            }
        } else if (conn->waitHandler) {
            mprWaitOn(conn->waitHandler, eventMask);
        }
        mprAssert(conn->dispatcher->enabled);
        unlock(conn->http);
    }
    if (tx && tx->handler && tx->handler->module) {
        tx->handler->module->lastActivity = conn->lastActivity;
    }
}


void httpFollowRedirects(HttpConn *conn, bool follow)
{
    conn->followRedirects = follow;
}


/*  
    Get the packet into which to read data. Return in *size the length of data to attempt to read.
 */
static HttpPacket *getPacket(HttpConn *conn, ssize *size)
{
    HttpPacket  *packet;
    MprBuf      *content;

    if ((packet = conn->input) == NULL) {
        conn->input = packet = httpCreatePacket(HTTP_BUFSIZE);
    } else {
        content = packet->content;
        mprResetBufIfEmpty(content);
        mprAddNullToBuf(content);
        if (mprGetBufSpace(content) < HTTP_BUFSIZE) {
            mprGrowBuf(content, HTTP_BUFSIZE);
        }
    }
    *size = mprGetBufSpace(packet->content);
    mprAssert(*size > 0);
    return packet;
}


int httpGetAsync(HttpConn *conn)
{
    return conn->async;
}


ssize httpGetChunkSize(HttpConn *conn)
{
    if (conn->tx) {
        return conn->tx->chunkSize;
    }
    return 0;
}


void *httpGetConnContext(HttpConn *conn)
{
    return conn->context;
}


void *httpGetConnHost(HttpConn *conn)
{
    return conn->host;
}


void httpResetCredentials(HttpConn *conn)
{
    conn->authType = 0;
    conn->authDomain = 0;
    conn->authCnonce = 0;
    conn->authNonce = 0;
    conn->authOpaque = 0;
    conn->authPassword = 0;
    conn->authRealm = 0;
    conn->authQop = 0;
    conn->authType = 0;
    conn->authUser = 0;
    
    httpRemoveHeader(conn, "Authorization");
}


void httpSetAsync(HttpConn *conn, int enable)
{
    conn->async = (enable) ? 1 : 0;
}


void httpSetConnNotifier(HttpConn *conn, HttpNotifier notifier)
{
    conn->notifier = notifier;
}


void httpSetCredentials(HttpConn *conn, cchar *user, cchar *password)
{
    httpResetCredentials(conn);
    conn->authUser = sclone(user);
    if (password == NULL && strchr(user, ':') != 0) {
        conn->authUser = stok(conn->authUser, ":", &conn->authPassword);
        conn->authPassword = sclone(conn->authPassword);
    } else {
        conn->authPassword = sclone(password);
    }
}


void httpSetKeepAliveCount(HttpConn *conn, int count)
{
    conn->keepAliveCount = count;
}


void httpSetChunkSize(HttpConn *conn, ssize size)
{
    if (conn->tx) {
        conn->tx->chunkSize = size;
    }
}


//  MOB - why not define this on the host or endpoint?
void httpSetHeadersCallback(HttpConn *conn, HttpHeadersCallback fn, void *arg)
{
    conn->headersCallback = fn;
    conn->headersCallbackArg = arg;
}


void httpSetIOCallback(HttpConn *conn, HttpIOCallback fn)
{
    conn->ioCallback = fn;
}


void httpSetConnContext(HttpConn *conn, void *context)
{
    conn->context = context;
}


void httpSetConnHost(HttpConn *conn, void *host)
{
    conn->host = host;
}


/*  
    Set the protocol to use for outbound requests
 */
void httpSetProtocol(HttpConn *conn, cchar *protocol)
{
    if (conn->state < HTTP_STATE_CONNECTED) {
        conn->protocol = sclone(protocol);
        if (strcmp(protocol, "HTTP/1.0") == 0) {
            conn->keepAliveCount = -1;
        }
    }
}


void httpSetRetries(HttpConn *conn, int count)
{
    conn->retries = count;
}


static char *notifyState[] = {
    "IO_EVENT", "BEGIN", "STARTED", "FIRST", "PARSED", "CONTENT", "RUNNING", "COMPLETE",
};


void httpSetState(HttpConn *conn, int state)
{
    if (state == conn->state) {
        return;
    }
    if (state < conn->state) {
        /* Prevent regressions */
        return;
    }
    conn->state = state;
    LOG(6, "Connection state change to %s", notifyState[state]);
    HTTP_NOTIFY(conn, state, 0);
}


/*
    Set each timeout arg to -1 to skip. Set to zero for no timeout. Otherwise set to number of msecs
 */
void httpSetTimeout(HttpConn *conn, int requestTimeout, int inactivityTimeout)
{
    if (requestTimeout >= 0) {
        if (requestTimeout == 0) {
            conn->limits->requestTimeout = MAXINT;
        } else {
            conn->limits->requestTimeout = requestTimeout;
        }
    }
    if (inactivityTimeout >= 0) {
        if (inactivityTimeout == 0) {
            conn->limits->inactivityTimeout = MAXINT;
        } else {
            conn->limits->inactivityTimeout = inactivityTimeout;
        }
    }
}


HttpLimits *httpSetUniqueConnLimits(HttpConn *conn)
{
    HttpLimits      *limits;

    if ((limits = mprAllocStruct(HttpLimits)) != 0) {
        *limits = *conn->limits;
        conn->limits = limits;
    }
    return limits;
}


void httpWritable(HttpConn *conn)
{
    HTTP_NOTIFY(conn, HTTP_EVENT_IO, HTTP_NOTIFY_WRITABLE);
}

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
/************************************************************************/
/*
 *  End of file "./src/conn.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/endpoint.c"
 */
/************************************************************************/

/*
    endpoint.c -- Create and manage listening endpoints.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static int manageEndpoint(HttpEndpoint *endpoint, int flags);
static int destroyEndpointConnections(HttpEndpoint *endpoint);

/*
    Create a listening endpoint on ip:port. NOTE: ip may be empty which means bind to all addresses.
 */
HttpEndpoint *httpCreateEndpoint(cchar *ip, int port, MprDispatcher *dispatcher)
{
    HttpEndpoint    *endpoint;
    Http            *http;

    if ((endpoint = mprAllocObj(HttpEndpoint, manageEndpoint)) == 0) {
        return 0;
    }
    http = MPR->httpService;
    endpoint->http = http;
    endpoint->clientLoad = mprCreateHash(HTTP_CLIENTS_HASH, MPR_HASH_STATIC_VALUES);
    endpoint->async = 1;
    endpoint->http = MPR->httpService;
    endpoint->port = port;
    endpoint->ip = sclone(ip);
    endpoint->dispatcher = dispatcher;
    endpoint->hosts = mprCreateList(-1, 0);
    httpAddEndpoint(http, endpoint);
    return endpoint;
}


void httpDestroyEndpoint(HttpEndpoint *endpoint)
{
    if (endpoint->waitHandler) {
        mprRemoveWaitHandler(endpoint->waitHandler);
        endpoint->waitHandler = 0;
    }
    destroyEndpointConnections(endpoint);
    if (endpoint->sock) {
        mprCloseSocket(endpoint->sock, 0);
        endpoint->sock = 0;
    }
    httpRemoveEndpoint(MPR->httpService, endpoint);
}


static int manageEndpoint(HttpEndpoint *endpoint, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(endpoint->http);
        mprMark(endpoint->hosts);
        mprMark(endpoint->limits);
        mprMark(endpoint->waitHandler);
        mprMark(endpoint->clientLoad);
        mprMark(endpoint->ip);
        mprMark(endpoint->context);
        mprMark(endpoint->sock);
        mprMark(endpoint->dispatcher);
        mprMark(endpoint->ssl);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyEndpoint(endpoint);
    }
    return 0;
}


/*  
    Convenience function to create and configure a new endpoint without using a config file.
 */
HttpEndpoint *httpCreateConfiguredEndpoint(cchar *home, cchar *documents, cchar *ip, int port)
{
    Http            *http;
    HttpHost        *host;
    HttpEndpoint    *endpoint;
    HttpRoute       *route;

    http = MPR->httpService;

    if (ip == 0) {
        /*  
            If no IP:PORT specified, find the first endpoint
         */
        if ((endpoint = mprGetFirstItem(http->endpoints)) != 0) {
            ip = endpoint->ip;
            port = endpoint->port;
        } else {
            ip = "localhost";
            if (port <= 0) {
                port = HTTP_DEFAULT_PORT;
            }
            if ((endpoint = httpCreateEndpoint(ip, port, NULL)) == 0) {
                return 0;
            }
        }
    } else {
        if ((endpoint = httpCreateEndpoint(ip, port, NULL)) == 0) {
            return 0;
        }
    }
    if ((host = httpCreateHost()) == 0) {
        return 0;
    }
    if ((route = httpCreateRoute(host)) == 0) {
        return 0;
    }
    httpSetHostDefaultRoute(host, route);
    httpSetHostIpAddr(host, ip, port);
    httpAddHostToEndpoint(endpoint, host);
    httpSetHostHome(host, home);
    httpSetRouteDir(route, documents);
    httpFinalizeRoute(route);
    return endpoint;
}


static int destroyEndpointConnections(HttpEndpoint *endpoint)
{
    HttpConn    *conn;
    Http        *http;
    int         next;

    http = endpoint->http;
    lock(http);

    for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
        if (conn->endpoint == endpoint) {
            conn->endpoint = 0;
            httpDestroyConn(conn);
            next--;
        }
    }
    unlock(http);
    return 0;
}


static bool validateEndpoint(HttpEndpoint *endpoint)
{
    HttpHost    *host;

    if ((host = mprGetFirstItem(endpoint->hosts)) == 0) {
        mprError("Missing host object on endpoint");
        return 0;
    }
    return 1;
}


int httpStartEndpoint(HttpEndpoint *endpoint)
{
    HttpHost    *host;
    cchar       *proto, *ip;
    int         next;

    if (!validateEndpoint(endpoint)) {
        return MPR_ERR_BAD_ARGS;
    }
    for (ITERATE_ITEMS(endpoint->hosts, host, next)) {
        httpStartHost(host);
    }
    if ((endpoint->sock = mprCreateSocket(endpoint->ssl)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (mprListenOnSocket(endpoint->sock, endpoint->ip, endpoint->port, MPR_SOCKET_NODELAY | MPR_SOCKET_THREAD) < 0) {
        mprError("Can't open a socket on %s:%d", *endpoint->ip ? endpoint->ip : "*", endpoint->port);
        return MPR_ERR_CANT_OPEN;
    }
    if (endpoint->http->listenCallback && (endpoint->http->listenCallback)(endpoint) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    if (endpoint->async && endpoint->waitHandler ==  0) {
        //  MOB TODO -- this really should be in endpoint->listen->handler
        endpoint->waitHandler = mprCreateWaitHandler(endpoint->sock->fd, MPR_SOCKET_READABLE, endpoint->dispatcher,
            httpAcceptConn, endpoint, (endpoint->dispatcher) ? 0 : MPR_WAIT_NEW_DISPATCHER);
    } else {
        mprSetSocketBlockingMode(endpoint->sock, 1);
    }
    proto = mprIsSocketSecure(endpoint->sock) ? "HTTPS" : "HTTP ";
    ip = *endpoint->ip ? endpoint->ip : "*";
    mprLog(1, "Started %s service on \"%s:%d\"", proto, ip, endpoint->port);
    return 0;
}


void httpStopEndpoint(HttpEndpoint *endpoint)
{
    HttpHost    *host;
    int         next;

    for (ITERATE_ITEMS(endpoint->hosts, host, next)) {
        httpStopHost(host);
    }
    if (endpoint->waitHandler) {
        mprRemoveWaitHandler(endpoint->waitHandler);
        endpoint->waitHandler = 0;
    }
    if (endpoint->sock) {
        mprRemoveSocketHandler(endpoint->sock);
        mprCloseSocket(endpoint->sock, 0);
        endpoint->sock = 0;
    }
}


bool httpValidateLimits(HttpEndpoint *endpoint, int event, HttpConn *conn)
{
    HttpLimits      *limits;
    cchar           *action;
    int             count, level, dir;

    limits = conn->limits;
    dir = HTTP_TRACE_RX;
    action = "unknown";
    mprAssert(conn->endpoint == endpoint);
    lock(endpoint->http);

    switch (event) {
    case HTTP_VALIDATE_OPEN_CONN:
        /*
            This active client systems with unique IP addresses.
         */
        if (endpoint->clientCount >= limits->clientCount) {
            unlock(endpoint->http);
            httpError(conn, HTTP_ABORT | HTTP_CODE_SERVICE_UNAVAILABLE, 
                "Too many concurrent clients %d/%d", endpoint->clientCount, limits->clientCount);
            return 0;
        }
        count = (int) PTOL(mprLookupKey(endpoint->clientLoad, conn->ip));
        mprAddKey(endpoint->clientLoad, conn->ip, ITOP(count + 1));
        endpoint->clientCount = (int) mprGetHashLength(endpoint->clientLoad);
        action = "open conn";
        dir = HTTP_TRACE_RX;
        break;

    case HTTP_VALIDATE_CLOSE_CONN:
        count = (int) PTOL(mprLookupKey(endpoint->clientLoad, conn->ip));
        if (count > 1) {
            mprAddKey(endpoint->clientLoad, conn->ip, ITOP(count - 1));
        } else {
            mprRemoveKey(endpoint->clientLoad, conn->ip);
        }
        endpoint->clientCount = (int) mprGetHashLength(endpoint->clientLoad);
        action = "close conn";
        dir = HTTP_TRACE_TX;
        break;
    
    case HTTP_VALIDATE_OPEN_REQUEST:
        if (endpoint->requestCount >= limits->requestCount) {
            unlock(endpoint->http);
            httpError(conn, HTTP_ABORT | HTTP_CODE_SERVICE_UNAVAILABLE, 
                "Too many concurrent requests %d/%d", endpoint->requestCount, limits->requestCount);
            return 0;
        }
        endpoint->requestCount++;
        action = "open request";
        dir = HTTP_TRACE_RX;
        break;

    case HTTP_VALIDATE_CLOSE_REQUEST:
        endpoint->requestCount--;
        mprAssert(endpoint->requestCount >= 0);
        action = "close request";
        dir = HTTP_TRACE_TX;
        break;
    }
    if (event == HTTP_VALIDATE_CLOSE_CONN || event == HTTP_VALIDATE_CLOSE_REQUEST) {
        if ((level = httpShouldTrace(conn, dir, HTTP_TRACE_LIMITS, NULL)) >= 0) {
            LOG(4, "Validate request for %s. Active connections %d, active requests: %d/%d, active client IP %d/%d", 
                action, mprGetListLength(endpoint->http->connections), endpoint->requestCount, limits->requestCount, 
                endpoint->clientCount, limits->clientCount);
        }
    }
    unlock(endpoint->http);
    return 1;
}


/*  
    Accept a new client connection on a new socket. If multithreaded, this will come in on a worker thread 
    dedicated to this connection. This is called from the listen wait handler.
 */
HttpConn *httpAcceptConn(HttpEndpoint *endpoint, MprEvent *event)
{
    HttpConn        *conn;
    MprSocket       *sock;
    MprDispatcher   *dispatcher;
    MprEvent        e;
    int             level;

    mprAssert(endpoint);
    mprAssert(event);

    /*
        This will block in sync mode until a connection arrives
     */
    if ((sock = mprAcceptSocket(endpoint->sock)) == 0) {
        if (endpoint->waitHandler) {
            mprWaitOn(endpoint->waitHandler, MPR_READABLE);
        }
        return 0;
    }
    if (endpoint->waitHandler) {
        /* Re-enable events on the listen socket */
        mprWaitOn(endpoint->waitHandler, MPR_READABLE);
    }
    dispatcher = event->dispatcher;

    if (mprShouldDenyNewRequests()) {
        mprCloseSocket(sock, 0);
        return 0;
    }
    if ((conn = httpCreateConn(endpoint->http, endpoint, dispatcher)) == 0) {
        mprCloseSocket(sock, 0);
        return 0;
    }
    conn->notifier = endpoint->notifier;
    conn->async = endpoint->async;
    conn->endpoint = endpoint;
    conn->sock = sock;
    conn->port = sock->port;
    conn->ip = sclone(sock->ip);
    conn->secure = mprIsSocketSecure(sock);

    if (!httpValidateLimits(endpoint, HTTP_VALIDATE_OPEN_CONN, conn)) {
        /* Prevent validate limits from */
        conn->endpoint = 0;
        httpDestroyConn(conn);
        return 0;
    }
    mprAssert(conn->state == HTTP_STATE_BEGIN);
    httpSetState(conn, HTTP_STATE_CONNECTED);

    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_CONN, NULL)) >= 0) {
        mprLog(level, "### Incoming connection from %s:%d to %s:%d %s", 
            conn->ip, conn->port, sock->acceptIp, sock->acceptPort, conn->secure ? "(secure)" : "");
    }
    e.mask = MPR_READABLE;
    e.timestamp = conn->http->now;
    (conn->ioCallback)(conn, &e);
    return conn;
}


void httpMatchHost(HttpConn *conn)
{ 
    MprSocket       *listenSock;
    HttpEndpoint    *endpoint;
    HttpHost        *host;
    Http            *http;

    http = conn->http;
    listenSock = conn->sock->listenSock;

    if ((endpoint = httpLookupEndpoint(http, listenSock->ip, listenSock->port)) == 0) {
        mprError("No listening endpoint for request from %s:%d", listenSock->ip, listenSock->port);
        mprCloseSocket(conn->sock, 0);
        return;
    }
    if (httpHasNamedVirtualHosts(endpoint)) {
        host = httpLookupHostOnEndpoint(endpoint, conn->rx->hostHeader);
    } else {
        host = mprGetFirstItem(endpoint->hosts);
    }
    if (host == 0) {
        httpSetConnHost(conn, 0);
        httpError(conn, HTTP_CODE_NOT_FOUND, "No host to serve request. Searching for %s", conn->rx->hostHeader);
        conn->host = mprGetFirstItem(endpoint->hosts);
        return;
    }
    if (conn->rx->traceLevel >= 0) {
        mprLog(conn->rx->traceLevel, "Select host: \"%s\"", host->name);
    }
    conn->host = host;
}


void *httpGetEndpointContext(HttpEndpoint *endpoint)
{
    return endpoint->context;
}


int httpIsEndpointAsync(HttpEndpoint *endpoint) 
{
    return endpoint->async;
}


//  MOB - rename. This could be a "restart"
void httpSetEndpointAddress(HttpEndpoint *endpoint, cchar *ip, int port)
{
    if (ip) {
        endpoint->ip = sclone(ip);
    }
    if (port >= 0) {
        endpoint->port = port;
    }
    if (endpoint->sock) {
        httpStopEndpoint(endpoint);
        httpStartEndpoint(endpoint);
    }
}


void httpSetEndpointAsync(HttpEndpoint *endpoint, int async)
{
    if (endpoint->sock) {
        if (endpoint->async && !async) {
            mprSetSocketBlockingMode(endpoint->sock, 1);
        }
        if (!endpoint->async && async) {
            mprSetSocketBlockingMode(endpoint->sock, 0);
        }
    }
    endpoint->async = async;
}


void httpSetEndpointContext(HttpEndpoint *endpoint, void *context)
{
    mprAssert(endpoint);
    endpoint->context = context;
}


void httpSetEndpointNotifier(HttpEndpoint *endpoint, HttpNotifier notifier)
{
    mprAssert(endpoint);
    endpoint->notifier = notifier;
}


int httpSecureEndpoint(HttpEndpoint *endpoint, struct MprSsl *ssl)
{
#if BLD_FEATURE_SSL
    endpoint->ssl = ssl;
    return 0;
#else
    return MPR_ERR_BAD_STATE;
#endif
}


int httpSecureEndpointByName(cchar *name, struct MprSsl *ssl)
{
    HttpEndpoint    *endpoint;
    Http            *http;
    char            *ip;
    int             port, next, count;

    http = MPR->httpService;
    mprParseSocketAddress(name, &ip, &port, -1);
    if (ip == 0) {
        ip = "";
    }
    for (count = 0, next = 0; (endpoint = mprGetNextItem(http->endpoints, &next)) != 0; ) {
        if (endpoint->port <= 0 || port <= 0 || endpoint->port == port) {
            mprAssert(endpoint->ip);
            if (*endpoint->ip == '\0' || *ip == '\0' || scmp(endpoint->ip, ip) == 0) {
                httpSecureEndpoint(endpoint, ssl);
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
}


void httpAddHostToEndpoint(HttpEndpoint *endpoint, HttpHost *host)
{
    mprAddItem(endpoint->hosts, host);
    if (endpoint->limits == 0) {
        endpoint->limits = host->defaultRoute->limits;
    }
}


bool httpHasNamedVirtualHosts(HttpEndpoint *endpoint)
{
    return endpoint->flags & HTTP_NAMED_VHOST;
}


void httpSetHasNamedVirtualHosts(HttpEndpoint *endpoint, bool on)
{
    if (on) {
        endpoint->flags |= HTTP_NAMED_VHOST;
    } else {
        endpoint->flags &= ~HTTP_NAMED_VHOST;
    }
}


HttpHost *httpLookupHostOnEndpoint(HttpEndpoint *endpoint, cchar *name)
{
    HttpHost    *host;
    int         next;

    if (name == 0 || *name == '\0') {
        return mprGetFirstItem(endpoint->hosts);
    }
    for (next = 0; (host = mprGetNextItem(endpoint->hosts, &next)) != 0; ) {
        if (smatch(host->name, name)) {
            return host;
        }
        if (*host->name == '*') {
            if (host->name[1] == '\0') {
                return host;
            }
            if (scontains(name, &host->name[1], -1)) {
                return host;
            }
        }
    }
    return 0;
}


int httpConfigureNamedVirtualEndpoints(Http *http, cchar *ip, int port)
{
    HttpEndpoint    *endpoint;
    int             next, count;

    if (ip == 0) {
        ip = "";
    }
    for (count = 0, next = 0; (endpoint = mprGetNextItem(http->endpoints, &next)) != 0; ) {
        if (endpoint->port <= 0 || port <= 0 || endpoint->port == port) {
            mprAssert(endpoint->ip);
            if (*endpoint->ip == '\0' || *ip == '\0' || scmp(endpoint->ip, ip) == 0) {
                httpSetHasNamedVirtualHosts(endpoint, 1);
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
}


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
/************************************************************************/
/*
 *  End of file "./src/endpoint.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/error.c"
 */
/************************************************************************/

/*
    error.c -- Http error handling
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void errorv(HttpConn *conn, int flags, cchar *fmt, va_list args);
static void formatErrorv(HttpConn *conn, int status, cchar *fmt, va_list args);


void httpDisconnect(HttpConn *conn)
{
    if (conn->sock) {
        mprDisconnectSocket(conn->sock);
    }
    conn->connError = 1;
    conn->error = 1;
    conn->keepAliveCount = -1;
    if (conn->rx) {
        conn->rx->eof = 1;
    }
}


void httpError(HttpConn *conn, int flags, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    errorv(conn, flags, fmt, args);
    va_end(args);
}


/*
    The current request has an error and cannot complete as normal. This call sets the Http response status and 
    overrides the normal output with an alternate error message. If the output has alread started (headers sent), then
    the connection MUST be closed so the client can get some indication the request failed.
 */
static void errorv(HttpConn *conn, int flags, cchar *fmt, va_list args)
{
    HttpRx      *rx;
    HttpTx      *tx;
    cchar       *uri;
    int         status;

    mprAssert(fmt);
    rx = conn->rx;
    tx = conn->tx;

    if (conn == 0) {
        return;
    }
    if (flags & HTTP_ABORT) {
        conn->connError = 1;
    }
    if (flags & (HTTP_ABORT | HTTP_CLOSE)) {
        conn->keepAliveCount = -1;
    }
    if (flags & HTTP_ABORT || (tx && tx->flags & HTTP_TX_HEADERS_CREATED)) {
        /* 
            If headers have been sent, must let the other side of the failure - abort is the only way.
            Disconnect will cause a readable (EOF) event
         */
        httpDisconnect(conn);
    }
    if (conn->error) {
        return;
    }
    if (rx) {
        rx->eof = 1;
    }
    conn->error = 1;
    status = flags & HTTP_CODE_MASK;
    if (status == 0) {
        status = HTTP_CODE_INTERNAL_SERVER_ERROR;
    }
    formatErrorv(conn, status, fmt, args);

    if (conn->endpoint && tx && rx) {
        if (!(tx->flags & HTTP_TX_HEADERS_CREATED)) {
            if (rx->route && (uri = httpLookupRouteErrorDocument(rx->route, tx->status))) {
                httpRedirect(conn, HTTP_CODE_MOVED_PERMANENTLY, uri);
            } else {
                httpFormatResponseError(conn, status, "%s", conn->errorMsg);
            }
        }
    }
    conn->responded = 1;
    httpFinalize(conn);
}


/*
    Just format conn->errorMsg and set status - nothing more
    NOTE: this is an internal API. Users should use httpError()
 */
static void formatErrorv(HttpConn *conn, int status, cchar *fmt, va_list args)
{
    if (conn->errorMsg == 0) {
        conn->errorMsg = sfmtv(fmt, args);
        if (status) {
            if (status < 0) {
                status = HTTP_CODE_INTERNAL_SERVER_ERROR;
            }
            if (conn->endpoint && conn->tx) {
                conn->tx->status = status;
            } else if (conn->rx) {
                conn->rx->status = status;
            }
        }
        if (conn->rx == 0 || conn->rx->uri == 0) {
            mprLog(2, "\"%s\", status %d: %s.", httpLookupStatus(conn->http, status), status, conn->errorMsg);
        } else {
            mprLog(2, "Error: \"%s\", status %d for URI \"%s\": %s.",
                httpLookupStatus(conn->http, status), status, conn->rx->uri ? conn->rx->uri : "", conn->errorMsg);
        }
    }
}


/*
    Just format conn->errorMsg and set status - nothing more
    NOTE: this is an internal API. Users should use httpError()
 */
void httpFormatError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt); 
    formatErrorv(conn, status, fmt, args);
    va_end(args); 
}


cchar *httpGetError(HttpConn *conn)
{
    if (conn->errorMsg) {
        return conn->errorMsg;
    } else if (conn->state >= HTTP_STATE_FIRST) {
        return httpLookupStatus(conn->http, conn->rx->status);
    } else {
        return "";
    }
}


void httpMemoryError(HttpConn *conn)
{
    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Memory allocation error");
}


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
/************************************************************************/
/*
 *  End of file "./src/error.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/host.c"
 */
/************************************************************************/

/*
    host.c -- Host class for all HTTP hosts

    The Host class is used for the default HTTP server and for all virtual hosts (including SSL hosts).
    Many objects are controlled at the host level. Eg. URL handlers.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageHost(HttpHost *host, int flags);


HttpHost *httpCreateHost()
{
    HttpHost    *host;
    Http        *http;

    http = MPR->httpService;
    if ((host = mprAllocObj(HttpHost, manageHost)) == 0) {
        return 0;
    }
    if ((host->responseCache = mprCreateCache(MPR_CACHE_SHARED)) == 0) {
        return 0;
    }
    mprSetCacheLimits(host->responseCache, 0, HTTP_CACHE_LIFESPAN, 0, 0);

    host->mutex = mprCreateLock();
    host->routes = mprCreateList(-1, 0);
    host->flags = HTTP_HOST_NO_TRACE;
    host->protocol = sclone("HTTP/1.1");
    host->home = sclone(".");
    httpAddHost(http, host);
    return host;
}


HttpHost *httpCloneHost(HttpHost *parent)
{
    HttpHost    *host;
    Http        *http;

    http = MPR->httpService;

    if ((host = mprAllocObj(HttpHost, manageHost)) == 0) {
        return 0;
    }
    host->mutex = mprCreateLock();

    /*  
        The dirs and routes are all copy-on-write.
        Don't clone ip, port and name
     */
    host->parent = parent;
    host->responseCache = parent->responseCache;
    host->home = parent->home;
    host->routes = parent->routes;
    host->flags = parent->flags | HTTP_HOST_VHOST;
    host->protocol = parent->protocol;
    httpAddHost(http, host);
    return host;
}


static void manageHost(HttpHost *host, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(host->name);
        mprMark(host->ip);
        mprMark(host->parent);
        mprMark(host->responseCache);
        mprMark(host->routes);
        mprMark(host->defaultRoute);
        mprMark(host->protocol);
        mprMark(host->mutex);
        mprMark(host->home);

    } else if (flags & MPR_MANAGE_FREE) {
        /* The http->hosts list is static. ie. The hosts won't be marked via http->hosts */
        httpRemoveHost(MPR->httpService, host);
    }
}


int httpStartHost(HttpHost *host)
{
    HttpRoute   *route;
    int         next;

    for (ITERATE_ITEMS(host->routes, route, next)) {
        httpStartRoute(route);
    }
    for (ITERATE_ITEMS(host->routes, route, next)) {
        if (!route->log && route->parent && route->parent->log) {
            route->log = route->parent->log;
        }
    }
    return 0;
}


void httpStopHost(HttpHost *host)
{
    HttpRoute   *route;
    int         next;

    for (ITERATE_ITEMS(host->routes, route, next)) {
        httpStopRoute(route);
    }
}


HttpRoute *httpGetHostDefaultRoute(HttpHost *host)
{
    return host->defaultRoute;
}


static void printRoute(HttpRoute *route, int next, bool full)
{
    cchar   *methods, *pattern, *target, *index;
    int     nextIndex;

    methods = httpGetRouteMethods(route);
    methods = methods ? methods : "*";
    pattern = (route->pattern && *route->pattern) ? route->pattern : "^/";
    target = (route->target && *route->target) ? route->target : "$&";
    if (full) {
        mprRawLog(0, "\n%d. %s\n", next, route->name);
        mprRawLog(0, "    Pattern:      %s\n", pattern);
        mprRawLog(0, "    StartSegment: %s\n", route->startSegment);
        mprRawLog(0, "    StartsWith:   %s\n", route->startWith);
        mprRawLog(0, "    RegExp:       %s\n", route->optimizedPattern);
        mprRawLog(0, "    Methods:      %s\n", methods);
        mprRawLog(0, "    Prefix:       %s\n", route->prefix);
        mprRawLog(0, "    Target:       %s\n", target);
        mprRawLog(0, "    Directory:    %s\n", route->dir);
        if (route->indicies) {
            mprRawLog(0, "    Indicies      ");
            for (ITERATE_ITEMS(route->indicies, index, nextIndex)) {
                mprRawLog(0, "%s ", index);
            }
        }
        mprRawLog(0, "\n    Next Group    %d\n", route->nextGroup);
        if (route->handler) {
            mprRawLog(0, "    Handler:      %s\n", route->handler->name);
        }
        mprRawLog(0, "\n");
    } else {
        mprRawLog(0, "%-20s %-12s %-40s %-14s\n", route->name, methods ? methods : "*", pattern, target);
    }
}


void httpLogRoutes(HttpHost *host, bool full)
{
    HttpRoute   *route;
    int         next, foundDefault;

    if (!full) {
        mprRawLog(0, "%-20s %-12s %-40s %-14s\n", "Name", "Methods", "Pattern", "Target");
    }
    for (foundDefault = next = 0; (route = mprGetNextItem(host->routes, &next)) != 0; ) {
        printRoute(route, next - 1, full);
        if (route == host->defaultRoute) {
            foundDefault++;
        }
    }
    /*
        Add the default so LogRoutes can print the default route which has yet been added to host->routes
     */
    if (!foundDefault && host->defaultRoute) {
        printRoute(host->defaultRoute, next - 1, full);
    }
    mprRawLog(0, "\n");
}


void httpSetHostHome(HttpHost *host, cchar *home)
{
    host->home = mprGetAbsPath(home);
}


/*
    IP may be null in which case the host is listening on all interfaces. Port may be set to -1 and ip may contain a port
    specifier, ie. "address:port".
 */
void httpSetHostIpAddr(HttpHost *host, cchar *ip, int port)
{
    char    *pip;

    if (port < 0 && schr(ip, ':')) {
        mprParseSocketAddress(ip, &pip, &port, -1);
        ip = pip;
    }
    host->ip = sclone(ip);
    host->port = port;

    if (!host->name) {
        if (ip) {
            if (port > 0) {
                host->name = sfmt("%s:%d", ip, port);
            } else {
                host->name = sclone(ip);
            }
        } else {
            mprAssert(port > 0);
            host->name = sfmt("*:%d", port);
        }
    }
}


void httpSetHostName(HttpHost *host, cchar *name)
{
    host->name = sclone(name);
}


void httpSetHostProtocol(HttpHost *host, cchar *protocol)
{
    host->protocol = sclone(protocol);
}


int httpAddRoute(HttpHost *host, HttpRoute *route)
{
    HttpRoute   *prev, *item;
    int         i, thisRoute;

    mprAssert(route);
    
    if (host->parent && host->routes == host->parent->routes) {
        host->routes = mprCloneList(host->parent->routes);
    }
    if (mprLookupItem(host->routes, route) < 0) {
        thisRoute = mprAddItem(host->routes, route);
        if (thisRoute > 0) {
            prev = mprGetItem(host->routes, thisRoute - 1);
            if (!smatch(prev->startSegment, route->startSegment)) {
                prev->nextGroup = thisRoute;
                for (i = thisRoute - 2; i >= 0; i--) {
                    item = mprGetItem(host->routes, i);
                    if (smatch(item->startSegment, prev->startSegment)) {
                        item->nextGroup = thisRoute;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    httpSetRouteHost(route, host);
    return 0;
}


HttpRoute *httpLookupRoute(HttpHost *host, cchar *name)
{
    HttpRoute   *route;
    int         next;

    if (name == 0 || *name == '\0') {
        name = "default";
    }
    for (next = 0; (route = mprGetNextItem(host->routes, &next)) != 0; ) {
        mprAssert(route->name);
        if (smatch(route->name, name)) {
            return route;
        }
    }
    return 0;
}


void httpResetRoutes(HttpHost *host)
{
    host->routes = mprCreateList(-1, 0);
}


void httpSetHostDefaultRoute(HttpHost *host, HttpRoute *route)
{
    host->defaultRoute = route;
}

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
/************************************************************************/
/*
 *  End of file "./src/host.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/httpService.c"
 */
/************************************************************************/

/*
    httpService.c -- Http service. Includes timer for expired requests.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/**
    Standard HTTP error code table
 */
typedef struct HttpStatusCode {
    int     code;                           /**< Http error code */
    char    *codeString;                    /**< Code as a string (for hashing) */
    char    *msg;                           /**< Error message */
} HttpStatusCode;


HttpStatusCode HttpStatusCodes[] = {
    { 100, "100", "Continue" },
    { 200, "200", "OK" },
    { 201, "201", "Created" },
    { 202, "202", "Accepted" },
    { 204, "204", "No Content" },
    { 205, "205", "Reset Content" },
    { 206, "206", "Partial Content" },
    { 301, "301", "Moved Permanently" },
    { 302, "302", "Moved Temporarily" },
    { 304, "304", "Not Modified" },
    { 305, "305", "Use Proxy" },
    { 307, "307", "Temporary Redirect" },
    { 400, "400", "Bad Request" },
    { 401, "401", "Unauthorized" },
    { 402, "402", "Payment Required" },
    { 403, "403", "Forbidden" },
    { 404, "404", "Not Found" },
    { 405, "405", "Method Not Allowed" },
    { 406, "406", "Not Acceptable" },
    { 408, "408", "Request Timeout" },
    { 409, "409", "Conflict" },
    { 410, "410", "Length Required" },
    { 411, "411", "Length Required" },
    { 412, "412", "Precondition Failed" },
    { 413, "413", "Request Entity Too Large" },
    { 414, "414", "Request-URI Too Large" },
    { 415, "415", "Unsupported Media Type" },
    { 416, "416", "Requested Range Not Satisfiable" },
    { 417, "417", "Expectation Failed" },
    { 500, "500", "Internal Server Error" },
    { 501, "501", "Not Implemented" },
    { 502, "502", "Bad Gateway" },
    { 503, "503", "Service Unavailable" },
    { 504, "504", "Gateway Timeout" },
    { 505, "505", "Http Version Not Supported" },
    { 507, "507", "Insufficient Storage" },

    /*
        Proprietary codes (used internally) when connection to client is severed
     */
    { 550, "550", "Comms Error" },
    { 551, "551", "General Client Error" },
    { 0,   0 }
};


static void httpTimer(Http *http, MprEvent *event);
static bool isIdle();
static void manageHttp(Http *http, int flags);
static void terminateHttp(int how, int status);
static void updateCurrentDate(Http *http);


Http *httpCreate()
{
    Http            *http;
    HttpStatusCode  *code;

    if (MPR->httpService) {
        return MPR->httpService;
    }
    if ((http = mprAllocObj(Http, manageHttp)) == 0) {
        return 0;
    }
    MPR->httpService = http;
    http->software = sclone(HTTP_NAME);
    http->protocol = sclone("HTTP/1.1");
    http->mutex = mprCreateLock(http);
    http->stages = mprCreateHash(-1, 0);
    http->routeTargets = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    http->routeConditions = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    http->routeUpdates = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    http->hosts = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->endpoints = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->connections = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->defaultClientHost = sclone("127.0.0.1");
    http->defaultClientPort = 80;
    http->booted = mprGetTime();

    updateCurrentDate(http);
    http->statusCodes = mprCreateHash(41, MPR_HASH_STATIC_VALUES | MPR_HASH_STATIC_KEYS);
    for (code = HttpStatusCodes; code->code; code++) {
        mprAddKey(http->statusCodes, code->codeString, code);
    }
    httpCreateSecret(http);
    httpInitAuth(http);
    httpOpenNetConnector(http);
    httpOpenSendConnector(http);
    httpOpenRangeFilter(http);
    httpOpenChunkFilter(http);
    httpOpenUploadFilter(http);
    httpOpenCacheHandler(http);
    httpOpenPassHandler(http);

    http->clientLimits = httpCreateLimits(0);
    http->serverLimits = httpCreateLimits(1);
    http->clientRoute = httpCreateConfiguredRoute(0, 0);

    mprSetIdleCallback(isIdle);
    mprAddTerminator(terminateHttp);
    httpDefineRouteBuiltins();
    return http;
}


static void manageHttp(Http *http, int flags)
{
    HttpConn    *conn;
    int         next;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(http->endpoints);
        mprMark(http->hosts);
        mprMark(http->connections);
        mprMark(http->stages);
        mprMark(http->statusCodes);
        mprMark(http->routeTargets);
        mprMark(http->routeConditions);
        mprMark(http->routeUpdates);
        /* Don't mark convenience stage references as they will be in http->stages */
        
        mprMark(http->clientLimits);
        mprMark(http->serverLimits);
        mprMark(http->clientRoute);
        mprMark(http->timer);
        mprMark(http->mutex);
        mprMark(http->software);
        mprMark(http->forkData);
        mprMark(http->context);
        mprMark(http->currentDate);
        mprMark(http->expiresDate);
        mprMark(http->secret);
        mprMark(http->defaultClientHost);
        mprMark(http->protocol);
        mprMark(http->proxyHost);

        /*
            Endpoints keep connections alive until a timeout. Keep marking even if no other references.
         */
        lock(http);
        for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
            if (conn->endpoint) {
                mprMark(conn);
            }
        }
        unlock(http);
    }
}


void httpDestroy(Http *http)
{
    if (http->timer) {
        mprRemoveEvent(http->timer);
    }
    MPR->httpService = NULL;
}


void httpAddEndpoint(Http *http, HttpEndpoint *endpoint)
{
    mprAddItem(http->endpoints, endpoint);
}


void httpRemoveEndpoint(Http *http, HttpEndpoint *endpoint)
{
    mprRemoveItem(http->endpoints, endpoint);
}


/*  
    Lookup a host address. If ipAddr is null or port is -1, then those elements are wild.
 */
HttpEndpoint *httpLookupEndpoint(Http *http, cchar *ip, int port)
{
    HttpEndpoint    *endpoint;
    int             next;

    if (ip == 0) {
        ip = "";
    }
    for (next = 0; (endpoint = mprGetNextItem(http->endpoints, &next)) != 0; ) {
        if (endpoint->port <= 0 || port <= 0 || endpoint->port == port) {
            mprAssert(endpoint->ip);
            if (*endpoint->ip == '\0' || *ip == '\0' || scmp(endpoint->ip, ip) == 0) {
                return endpoint;
            }
        }
    }
    return 0;
}


HttpEndpoint *httpGetFirstEndpoint(Http *http)
{
    return mprGetFirstItem(http->endpoints);
}


/*
    WARNING: this should not be called by users as httpCreateHost will automatically call this.
 */
void httpAddHost(Http *http, HttpHost *host)
{
    mprAddItem(http->hosts, host);
}


void httpRemoveHost(Http *http, HttpHost *host)
{
    mprRemoveItem(http->hosts, host);
}


HttpHost *httpLookupHost(Http *http, cchar *name)
{
    HttpHost    *host;
    int         next;

    for (next = 0; (host = mprGetNextItem(http->hosts, &next)) != 0; ) {
        if (smatch(name, host->name)) {
            return host;
        }
    }
    return 0;
}


void httpInitLimits(HttpLimits *limits, bool serverSide)
{
    memset(limits, 0, sizeof(HttpLimits));
    limits->cacheItemSize = HTTP_MAX_CACHE_ITEM;
    limits->chunkSize = HTTP_MAX_CHUNK;
    limits->headerCount = HTTP_MAX_NUM_HEADERS;
    limits->headerSize = HTTP_MAX_HEADERS;
    limits->receiveFormSize = HTTP_MAX_RECEIVE_FORM;
    limits->receiveBodySize = HTTP_MAX_RECEIVE_BODY;
    limits->requestCount = HTTP_MAX_REQUESTS;
    limits->stageBufferSize = HTTP_MAX_STAGE_BUFFER;
    limits->transmissionBodySize = HTTP_MAX_TX_BODY;
    limits->uploadSize = HTTP_MAX_UPLOAD;
    limits->uriSize = MPR_MAX_URL;

    limits->inactivityTimeout = HTTP_INACTIVITY_TIMEOUT;
    limits->requestTimeout = MAXINT;
    limits->sessionTimeout = HTTP_SESSION_TIMEOUT;

    limits->clientCount = HTTP_MAX_CLIENTS;
    limits->keepAliveCount = HTTP_MAX_KEEP_ALIVE;
    limits->requestCount = HTTP_MAX_REQUESTS;

#if FUTURE
    mprSetMaxSocketClients(endpoint, atoi(value));

    if (scasecmp(key, "LimitClients") == 0) {
        mprSetMaxSocketClients(endpoint, atoi(value));
        return 1;
    }
    if (scasecmp(key, "LimitMemoryMax") == 0) {
        mprSetAllocLimits(endpoint, -1, atoi(value));
        return 1;
    }
    if (scasecmp(key, "LimitMemoryRedline") == 0) {
        mprSetAllocLimits(endpoint, atoi(value), -1);
        return 1;
    }
#endif
}


HttpLimits *httpCreateLimits(int serverSide)
{
    HttpLimits  *limits;

    if ((limits = mprAllocStruct(HttpLimits)) != 0) {
        httpInitLimits(limits, serverSide);
    }
    return limits;
}


void httpEaseLimits(HttpLimits *limits)
{
    limits->receiveFormSize = MAXOFF;
    limits->receiveBodySize = MAXOFF;
    limits->transmissionBodySize = MAXOFF;
    limits->uploadSize = MAXOFF;
}


void httpAddStage(Http *http, HttpStage *stage)
{
    mprAddKey(http->stages, stage->name, stage);
}


HttpStage *httpLookupStage(Http *http, cchar *name)
{
    return mprLookupKey(http->stages, name);
}


void *httpLookupStageData(Http *http, cchar *name)
{
    HttpStage   *stage;
    if ((stage = mprLookupKey(http->stages, name)) != 0) {
        return stage->stageData;
    }
    return 0;
}


cchar *httpLookupStatus(Http *http, int status)
{
    HttpStatusCode  *ep;
    char            *key;
    
    key = itos(status);
    ep = (HttpStatusCode*) mprLookupKey(http->statusCodes, key);
    if (ep == 0) {
        return "Custom error";
    }
    return ep->msg;
}


void httpSetForkCallback(Http *http, MprForkCallback callback, void *data)
{
    http->forkCallback = callback;
    http->forkData = data;
}


void httpSetListenCallback(Http *http, HttpListenCallback fn)
{
    http->listenCallback = fn;
}


/*  
    Start the http timer. This may create multiple timers -- no worry. httpAddConn does its best to only schedule one.
 */
static void startTimer(Http *http)
{
    updateCurrentDate(http);
    http->timer = mprCreateTimerEvent(NULL, "httpTimer", HTTP_TIMER_PERIOD, httpTimer, http, 
        MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK);
}


/*  
    The http timer does maintenance activities and will fire per second while there are active requests.
    NOTE: Because we lock the http here, connections cannot be deleted while we are modifying the list.
 */
static void httpTimer(Http *http, MprEvent *event)
{
    HttpConn    *conn;
    HttpStage   *stage;
    HttpRx      *rx;
    HttpLimits  *limits;
    MprModule   *module;
    int         next, active;

    mprAssert(event);
    
    updateCurrentDate(http);
    if (mprGetDebugMode()) {
        return;
    }
    /* 
       Check for any inactive connections or expired requests (inactivityTimeout and requestTimeout)
     */
    lock(http);
    mprLog(6, "httpTimer: %d active connections", mprGetListLength(http->connections));
    for (active = 0, next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; active++) {
        rx = conn->rx;
        limits = conn->limits;
        if (!conn->timeoutEvent && (
            (conn->lastActivity + limits->inactivityTimeout) < http->now || 
            (conn->started + limits->requestTimeout) < http->now)) {
            if (rx) {
                /*
                    Don't call APIs on the conn directly (thread-race). Schedule a timer on the connection's dispatcher
                 */
                if (!conn->timeoutEvent) {
                    conn->timeoutEvent = mprCreateEvent(conn->dispatcher, "connTimeout", 0, httpConnTimeout, conn, 0);
                }
            } else {
                mprLog(6, "Idle connection timed out");
                httpDisconnect(conn);
                httpDiscardData(conn->writeq, 1);
                httpEnableConnEvents(conn);
                conn->lastActivity = conn->started = http->now;
            }
        }
    }

    /*
        Check for unloadable modules
        MOB - move down into MPR and set stage->flags in an unload callback
     */
    if (mprGetListLength(http->connections) == 0) {
        for (next = 0; (module = mprGetNextItem(MPR->moduleService->modules, &next)) != 0; ) {
            if (module->timeout) {
                if (module->lastActivity + module->timeout < http->now) {
                    mprLog(2, "Unloading inactive module %s", module->name);
                    if ((stage = httpLookupStage(http, module->name)) != 0) {
                        if (mprUnloadModule(module) < 0)  {
                            active++;
                        } else {
                            stage->flags |= HTTP_STAGE_UNLOADED;
                        }
                    } else {
                        mprUnloadModule(module);
                    }
                } else {
                    active++;
                }
            }
        }
    }
    if (active == 0) {
        mprRemoveEvent(event);
        http->timer = 0;
    }
    unlock(http);
}


static void terminateHttp(int how, int status)
{
    Http            *http;
    HttpEndpoint    *endpoint;
    int             next;

    /*
        Stop listening for new requests
     */
    http = (Http*) mprGetMpr()->httpService;
    if (http) {
        for (ITERATE_ITEMS(http->endpoints, endpoint, next)) {
            httpStopEndpoint(endpoint);
        }
    }
}


static bool isIdle()
{
    HttpConn        *conn;
    Http            *http;
    MprTime         now;
    int             next;
    static MprTime  lastTrace = 0;

    http = (Http*) mprGetMpr()->httpService;
    now = http->now;

    lock(http);
    for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
        if (conn->state != HTTP_STATE_BEGIN) {
            if (lastTrace < now) {
                mprLog(1, "Waiting for request %s to complete", conn->rx->uri ? conn->rx->uri : conn->rx->pathInfo);
                lastTrace = now;
            }
            unlock(http);
            return 0;
        }
    }
    unlock(http);
    if (!mprServicesAreIdle()) {
        if (lastTrace < now) {
            mprLog(4, "Waiting for MPR services complete");
            lastTrace = now;
        }
        return 0;
    }
    return 1;
}


void httpAddConn(Http *http, HttpConn *conn)
{
    lock(http);
    mprAddItem(http->connections, conn);
    conn->started = http->now;
    conn->seqno = http->connCount++;
    if (http->now < (conn->started - MPR_TICKS_PER_SEC)) {
        updateCurrentDate(http);
    }
    if (http->timer == 0) {
        startTimer(http);
    }
    unlock(http);
}


void httpRemoveConn(Http *http, HttpConn *conn)
{
    lock(http);
    mprRemoveItem(http->connections, conn);
    unlock(http);
}


/*  
    Create a random secret for use in authentication. Create once for the entire http service. Created on demand.
    Users can recall as required to update.
 */
int httpCreateSecret(Http *http)
{
    MprTime     now;
    char        *hex = "0123456789abcdef";
    char        bytes[HTTP_MAX_SECRET], ascii[HTTP_MAX_SECRET * 2 + 1], *ap, *cp, *bp;
    int         i, pid;

    if (mprGetRandomBytes(bytes, sizeof(bytes), 0) < 0) {
        mprError("Can't get sufficient random data for secure SSL operation. If SSL is used, it may not be secure.");
        now = http->now;
        pid = (int) getpid();
        cp = (char*) &now;
        bp = bytes;
        for (i = 0; i < sizeof(now) && bp < &bytes[HTTP_MAX_SECRET]; i++) {
            *bp++= *cp++;
        }
        cp = (char*) &now;
        for (i = 0; i < sizeof(pid) && bp < &bytes[HTTP_MAX_SECRET]; i++) {
            *bp++ = *cp++;
        }
        mprAssert(0);
        return MPR_ERR_CANT_INITIALIZE;
    }

    ap = ascii;
    for (i = 0; i < (int) sizeof(bytes); i++) {
        *ap++ = hex[((uchar) bytes[i]) >> 4];
        *ap++ = hex[((uchar) bytes[i]) & 0xf];
    }
    *ap = '\0';
    http->secret = sclone(ascii);
    return 0;
}


void httpEnableTraceMethod(HttpLimits *limits, bool on)
{
    limits->enableTraceMethod = on;
}


char *httpGetDateString(MprPath *sbuf)
{
    MprTime     when;

    if (sbuf == 0) {
        when = ((Http*) MPR->httpService)->now;
    } else {
        when = (MprTime) sbuf->mtime * MPR_TICKS_PER_SEC;
    }
    return mprFormatUniversalTime(HTTP_DATE_FORMAT, when);
}


void *httpGetContext(Http *http)
{
    return http->context;
}


void httpSetContext(Http *http, void *context)
{
    http->context = context;
}


int httpGetDefaultClientPort(Http *http)
{
    return http->defaultClientPort;
}


cchar *httpGetDefaultClientHost(Http *http)
{
    return http->defaultClientHost;
}


int httpLoadSsl(Http *http)
{
#if BLD_FEATURE_SSL
    if (!http->sslLoaded) {
        if (!mprLoadSsl(0)) {
            mprError("Can't load SSL provider");
            return MPR_ERR_CANT_LOAD;
        }
        http->sslLoaded = 1;
    }
#else
    mprError("SSL communications support not included in build");
#endif
    return 0;
}


void httpSetDefaultClientPort(Http *http, int port)
{
    http->defaultClientPort = port;
}


void httpSetDefaultClientHost(Http *http, cchar *host)
{
    http->defaultClientHost = sclone(host);
}


void httpSetSoftware(Http *http, cchar *software)
{
    http->software = sclone(software);
}


void httpSetProxy(Http *http, cchar *host, int port)
{
    http->proxyHost = sclone(host);
    http->proxyPort = port;
}


static void updateCurrentDate(Http *http)
{
    static MprTime  recalcExpires = 0;

    lock(http);
    http->now = mprGetTime();
    http->currentDate = httpGetDateString(NULL);

    if (http->expiresDate == 0 || recalcExpires < (http->now / (60 * 1000))) {
        http->expiresDate = mprFormatUniversalTime(HTTP_DATE_FORMAT, http->now + (86400 * 1000));
        recalcExpires = http->now / (60 * 1000);
    }
    unlock(http);
}


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
/************************************************************************/
/*
 *  End of file "./src/httpService.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/log.c"
 */
/************************************************************************/

/*
    log.c -- Http request access logging

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




void httpSetRouteLog(HttpRoute *route, cchar *path, ssize size, int backup, cchar *format, int flags)
{
    char    *src, *dest;

    mprAssert(route);
    mprAssert(path && *path);
    mprAssert(format);
    
    if (format == NULL || *format == '\0') {
        format = HTTP_LOG_FORMAT;
    }
    route->logFlags = flags;
    route->logSize = size;
    route->logBackup = backup;
    route->logPath = sclone(path);
    route->logFormat = sclone(format);

    for (src = dest = route->logFormat; *src; src++) {
        if (*src == '\\' && src[1] != '\\') {
            continue;
        }
        *dest++ = *src;
    }
    *dest = '\0';
}


void httpBackupRouteLog(HttpRoute *route)
{
    MprPath     info;

    mprAssert(route->logBackup);
    mprAssert(route->logSize > 100);
    mprGetPathInfo(route->logPath, &info);
    if (info.valid && ((route->logFlags & MPR_LOG_ANEW) || info.size > route->logSize || route->logSize <= 0)) {
        if (route->log) {
            mprCloseFile(route->log);
            route->log = 0;
        }
        mprBackupLog(route->logPath, route->logBackup);
    }
}


MprFile *httpOpenRouteLog(HttpRoute *route)
{
    MprFile     *file;
    int         mode;

    mprAssert(route->log == 0);
    mode = O_CREAT | O_WRONLY | O_TEXT;
    if ((file = mprOpenFile(route->logPath, mode, 0664)) == 0) {
        mprError("Can't open log file %s", route->logPath);
        unlock(MPR);
        return 0;
    }
    route->log = file;
    return file;
}


void httpWriteRouteLog(HttpRoute *route, cchar *buf, ssize len)
{
    lock(MPR);
    if (route->logBackup > 0) {
        httpBackupRouteLog(route);
        if (!route->log && !httpOpenRouteLog(route)) {
            unlock(MPR);
            return;
        }
    }
    if (mprWriteFile(route->log, (char*) buf, len) != len) {
        mprError("Can't write to access log %s", route->logPath);
        mprCloseFile(route->log);
        route->log = 0;
    }
    unlock(MPR);
}


void httpLogRequest(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    MprBuf      *buf;
    char        keyBuf[80], *timeText, *fmt, *cp, *qualifier, *value, c;
    int         len;

    rx = conn->rx;
    tx = conn->tx;
    route = rx->route;
    if (!route->log) {
        return;
    }
    fmt = route->logFormat;
    if (fmt == 0) {
        fmt = HTTP_LOG_FORMAT;
    }
    len = MPR_MAX_URL + 256;
    buf = mprCreateBuf(len, len);

    while ((c = *fmt++) != '\0') {
        if (c != '%' || (c = *fmt++) == '%') {
            mprPutCharToBuf(buf, c);
            continue;
        }
        switch (c) {
        case 'a':                           /* Remote IP */
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'A':                           /* Local IP */
            mprPutStringToBuf(buf, conn->sock->listenSock->ip);
            break;

        case 'b':
            if (tx->bytesWritten == 0) {
                mprPutCharToBuf(buf, '-');
            } else {
                mprPutIntToBuf(buf, tx->bytesWritten);
            } 
            break;

        case 'B':                           /* Bytes written (minus headers) */
            mprPutIntToBuf(buf, (tx->bytesWritten - tx->headerSize));
            break;

        case 'h':                           /* Remote host */
            //  TODO - Should this trigger a reverse DNS?
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'n':                           /* Local host */
            mprPutStringToBuf(buf, rx->parsedUri->host);
            break;

        case 'O':                           /* Bytes written (including headers) */
            mprPutIntToBuf(buf, tx->bytesWritten);
            break;

        case 'r':                           /* First line of request */
            mprPutFmtToBuf(buf, "%s %s %s", rx->method, rx->uri, conn->protocol);
            break;

        case 's':                           /* Response code */
            mprPutIntToBuf(buf, tx->status);
            break;

        case 't':                           /* Time */
            mprPutCharToBuf(buf, '[');
            timeText = mprFormatLocalTime(MPR_DEFAULT_DATE, mprGetTime());
            mprPutStringToBuf(buf, timeText);
            mprPutCharToBuf(buf, ']');
            break;

        case 'u':                           /* Remote username */
            mprPutStringToBuf(buf, conn->authUser ? conn->authUser : "-");
            break;

        case '{':                           /* Header line */
            qualifier = fmt;
            if ((cp = strchr(qualifier, '}')) != 0) {
                fmt = &cp[1];
                *cp = '\0';
                c = *fmt++;
                scopy(keyBuf, sizeof(keyBuf), "HTTP_");
                scopy(&keyBuf[5], sizeof(keyBuf) - 5, qualifier);
                switch (c) {
                case 'i':
                    value = (char*) mprLookupKey(rx->headers, supper(keyBuf));
                    mprPutStringToBuf(buf, value ? value : "-");
                    break;
                default:
                    mprPutStringToBuf(buf, qualifier);
                }
                *cp = '}';

            } else {
                mprPutCharToBuf(buf, c);
            }
            break;

        case '>':
            if (*fmt == 's') {
                fmt++;
                mprPutIntToBuf(buf, tx->status);
            }
            break;

        default:
            mprPutCharToBuf(buf, c);
            break;
        }
    }
    mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);
    httpWriteRouteLog(route, mprGetBufStart(buf), mprGetBufLength(buf));
}



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
/************************************************************************/
/*
 *  End of file "./src/log.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/netConnector.c"
 */
/************************************************************************/

/*
    netConnector.c -- General network connector. 

    The Network connector handles output data (only) from upstream handlers and filters. It uses vectored writes to
    aggregate output packets into fewer actual I/O requests to the O/S. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void addPacketForNet(HttpQueue *q, HttpPacket *packet);
static void adjustNetVec(HttpQueue *q, ssize written);
static MprOff buildNetVec(HttpQueue *q);
static void freeNetPackets(HttpQueue *q, ssize written);
static void netClose(HttpQueue *q);
static void netOutgoingService(HttpQueue *q);

/*  
    Initialize the net connector
 */
int httpOpenNetConnector(Http *http)
{
    HttpStage     *stage;

    mprLog(5, "Open net connector");
    if ((stage = httpCreateConnector(http, "netConnector", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->close = netClose;
    stage->outgoingService = netOutgoingService;
    http->netConnector = stage;
    return 0;
}


static void netClose(HttpQueue *q)
{
    HttpTx      *tx;

    tx = q->conn->tx;
    if (tx->file) {
        mprCloseFile(tx->file);
        tx->file = 0;
    }
}


static void netOutgoingService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       written;
    int         errCode;

    conn = q->conn;
    tx = conn->tx;
    conn->lastActivity = conn->http->now;
    mprAssert(conn->sock);
    mprAssert(!conn->connectorComplete);
    
    if (!conn->sock || conn->connectorComplete) {
        return;
    }
    if (tx->flags & HTTP_TX_NO_BODY) {
        httpDiscardData(q, 1);
    }
    if ((tx->bytesWritten + q->count) > conn->limits->transmissionBodySize) {
        httpError(conn, HTTP_CODE_REQUEST_TOO_LARGE | ((tx->bytesWritten) ? HTTP_ABORT : 0),
            "Http transmission aborted. Exceeded transmission max body of %,Ld bytes", conn->limits->transmissionBodySize);
        if (tx->bytesWritten) {
            httpConnectorComplete(conn);
            return;
        }
    }
    if (tx->flags & HTTP_TX_SENDFILE) {
        /* Relay via the send connector */
        if (tx->file == 0) {
            if (tx->flags & HTTP_TX_HEADERS_CREATED) {
                tx->flags &= ~HTTP_TX_SENDFILE;
            } else {
                tx->connector = conn->http->sendConnector;
                httpSendOpen(q);
            }
        }
        if (tx->file) {
            httpSendOutgoingService(q);
            return;
        }
    }
    while (q->first || q->ioIndex) {
        if (q->ioIndex == 0 && buildNetVec(q) <= 0) {
            break;
        }
        /*  
            Issue a single I/O request to write all the blocks in the I/O vector
         */
        mprAssert(q->ioIndex > 0);
        written = mprWriteSocketVector(conn->sock, q->iovec, q->ioIndex);
        LOG(5, "Net connector wrote %d, written so far %Ld, q->count %d/%d", written, tx->bytesWritten, q->count, q->max);
        if (written < 0) {
            errCode = mprGetError(q);
            if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
                /*  Socket full, wait for an I/O event */
                httpSetWriteBlocked(conn);
                break;
            }
            if (errCode != EPIPE && errCode != ECONNRESET) {
                LOG(5, "netOutgoingService write failed, error %d", errCode);
            }
            httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "Write error %d", errCode);
            httpConnectorComplete(conn);
            break;

        } else if (written == 0) {
            /*  Socket full, wait for an I/O event */
            httpSetWriteBlocked(conn);
            break;

        } else if (written > 0) {
            tx->bytesWritten += written;
            freeNetPackets(q, written);
            adjustNetVec(q, written);
        }
    }
    if (q->ioCount == 0) {
        if ((q->flags & HTTP_QUEUE_EOF)) {
            httpConnectorComplete(conn);
        } else {
            httpWritable(conn);
        }
    }
}


/*
    Build the IO vector. Return the count of bytes to be written. Return -1 for EOF.
 */
static MprOff buildNetVec(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpPacket  *packet;

    conn = q->conn;
    tx = conn->tx;

    /*
        Examine each packet and accumulate as many packets into the I/O vector as possible. Leave the packets on the queue 
        for now, they are removed after the IO is complete for the entire packet.
     */
    for (packet = q->first; packet; packet = packet->next) {
        if (packet->flags & HTTP_PACKET_HEADER) {
            if (tx->chunkSize <= 0 && q->count > 0 && tx->length < 0) {
                /* Incase no chunking filter and we've not seen all the data yet */
                conn->keepAliveCount = 0;
            }
            httpWriteHeaders(conn, packet);
            q->count += httpGetPacketLength(packet);

        } else if (httpGetPacketLength(packet) == 0) {
            q->flags |= HTTP_QUEUE_EOF;
            if (packet->prefix == NULL) {
                break;
            }
        }
        if (q->ioIndex >= (HTTP_MAX_IOVEC - 2)) {
            break;
        }
        addPacketForNet(q, packet);
    }
    return q->ioCount;
}


/*
    Add one entry to the io vector
 */
static void addToNetVector(HttpQueue *q, char *ptr, ssize bytes)
{
    mprAssert(bytes > 0);

    q->iovec[q->ioIndex].start = ptr;
    q->iovec[q->ioIndex].len = bytes;
    q->ioCount += bytes;
    q->ioIndex++;
}


/*
    Add a packet to the io vector. Return the number of bytes added to the vector.
 */
static void addPacketForNet(HttpQueue *q, HttpPacket *packet)
{
    HttpTx      *tx;
    HttpConn    *conn;
    int         item;

    conn = q->conn;
    tx = conn->tx;

    mprAssert(q->count >= 0);
    mprAssert(q->ioIndex < (HTTP_MAX_IOVEC - 2));

    if (packet->prefix) {
        addToNetVector(q, mprGetBufStart(packet->prefix), mprGetBufLength(packet->prefix));
    }
    if (httpGetPacketLength(packet) > 0) {
        addToNetVector(q, mprGetBufStart(packet->content), mprGetBufLength(packet->content));
    }
    item = (packet->flags & HTTP_PACKET_HEADER) ? HTTP_TRACE_HEADER : HTTP_TRACE_BODY;
    if (httpShouldTrace(conn, HTTP_TRACE_TX, item, tx->ext) >= 0) {
        httpTraceContent(conn, HTTP_TRACE_TX, item, packet, 0, (ssize) tx->bytesWritten);
    }
}


static void freeNetPackets(HttpQueue *q, ssize bytes)
{
    HttpPacket    *packet;
    ssize         len;

    mprAssert(q->count >= 0);
    mprAssert(bytes >= 0);

    while (bytes > 0 && (packet = q->first) != 0) {
        if (packet->prefix) {
            len = mprGetBufLength(packet->prefix);
            len = min(len, bytes);
            mprAdjustBufStart(packet->prefix, len);
            bytes -= len;
            /* Prefixes don't count in the q->count. No need to adjust */
            if (mprGetBufLength(packet->prefix) == 0) {
                packet->prefix = 0;
            }
        }
        if (packet->content) {
            len = mprGetBufLength(packet->content);
            len = min(len, bytes);
            mprAdjustBufStart(packet->content, len);
            bytes -= len;
            q->count -= len;
            mprAssert(q->count >= 0);
        }
        if (packet->content == 0 || mprGetBufLength(packet->content) == 0) {
            /*
                This will remove the packet from the queue and will re-enable upstream disabled queues.
             */
            httpGetPacket(q);
        }
#if UNUSED
        mprAssert(bytes >= 0);
        if (bytes == 0 && (q->first == NULL || !(q->first->flags & HTTP_PACKET_END))) {
            break;
        }
#endif
    }
}


/*
    Clear entries from the IO vector that have actually been transmitted. Support partial writes.
 */
static void adjustNetVec(HttpQueue *q, ssize written)
{
    MprIOVec    *iovec;
    ssize       len;
    int         i, j;

    /*
        Cleanup the IO vector
     */
    if (written == q->ioCount) {
        /*
            Entire vector written. Just reset.
         */
        q->ioIndex = 0;
        q->ioCount = 0;

    } else {
        /*
            Partial write of an vector entry. Need to copy down the unwritten vector entries.
         */
        q->ioCount -= written;
        mprAssert(q->ioCount >= 0);
        iovec = q->iovec;
        for (i = 0; i < q->ioIndex; i++) {
            len = iovec[i].len;
            if (written < len) {
                iovec[i].start += written;
                iovec[i].len -= written;
                break;
            } else {
                written -= len;
            }
        }
        /*
            Compact
         */
        for (j = 0; i < q->ioIndex; ) {
            iovec[j++] = iovec[i++];
        }
        q->ioIndex = j;
    }
}


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
/************************************************************************/
/*
 *  End of file "./src/netConnector.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/packet.c"
 */
/************************************************************************/

/*
    packet.c -- Queue support routines. Queues are the bi-directional data flow channels for the pipeline.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void managePacket(HttpPacket *packet, int flags);

/*  
    Create a new packet. If size is -1, then also create a default growable buffer -- 
    used for incoming body content. If size > 0, then create a non-growable buffer 
    of the requested size.
 */
HttpPacket *httpCreatePacket(ssize size)
{
    HttpPacket  *packet;

    if ((packet = mprAllocObj(HttpPacket, managePacket)) == 0) {
        return 0;
    }
    if (size != 0) {
        if ((packet->content = mprCreateBuf(size < 0 ? HTTP_BUFSIZE: (ssize) size, -1)) == 0) {
            return 0;
        }
    }
    return packet;
}


static void managePacket(HttpPacket *packet, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(packet->prefix);
        mprMark(packet->content);
        /* Don't mark next packet. List owner will mark */
    }
}


HttpPacket *httpCreateDataPacket(ssize size)
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(size)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_DATA;
    return packet;
}


HttpPacket *httpCreateEntityPacket(MprOff pos, MprOff size, HttpFillProc fill)
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(0)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_DATA;
    packet->epos = pos;
    packet->esize = size;
    packet->fill = fill;
    return packet;
}


HttpPacket *httpCreateEndPacket()
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(0)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_END;
    return packet;
}


HttpPacket *httpCreateHeaderPacket()
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(HTTP_BUFSIZE)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_HEADER;
    return packet;
}


HttpPacket *httpClonePacket(HttpPacket *orig)
{
    HttpPacket  *packet;

    if ((packet = httpCreatePacket(0)) == 0) {
        return 0;
    }
    if (orig->content) {
        packet->content = mprCloneBuf(orig->content);
    }
    if (orig->prefix) {
        packet->prefix = mprCloneBuf(orig->prefix);
    }
    packet->flags = orig->flags;
    packet->esize = orig->esize;
    packet->epos = orig->epos;
    packet->fill = orig->fill;
    return packet;
}


void httpAdjustPacketStart(HttpPacket *packet, MprOff size)
{
    if (packet->esize) {
        packet->epos += size;
        packet->esize -= size;
    } else if (packet->content) {
        mprAdjustBufStart(packet->content, (ssize) size);
    }
}


void httpAdjustPacketEnd(HttpPacket *packet, MprOff size)
{
    if (packet->esize) {
        packet->esize += size;
    } else if (packet->content) {
        mprAdjustBufEnd(packet->content, (ssize) size);
    }
}


HttpPacket *httpGetPacket(HttpQueue *q)
{
    HttpQueue     *prev;
    HttpPacket    *packet;

    while (q->first) {
        if ((packet = q->first) != 0) {
            q->first = packet->next;
            packet->next = 0;
            q->count -= httpGetPacketLength(packet);
            mprAssert(q->count >= 0);
            if (packet == q->last) {
                q->last = 0;
                mprAssert(q->first == 0);
            }
        }
        if (q->flags & HTTP_QUEUE_FULL && q->count < q->low) {
            /*
                This queue was full and now is below the low water mark. Back-enable the previous queue.
             */
            q->flags &= ~HTTP_QUEUE_FULL;
            prev = httpFindPreviousQueue(q);
            if (prev && prev->flags & HTTP_QUEUE_DISABLED) {
                httpEnableQueue(prev);
            }
        }
        return packet;
    }
    return 0;
}


/*  
    Test if the packet is too too large to be accepted by the downstream queue.
 */
bool httpIsPacketTooBig(HttpQueue *q, HttpPacket *packet)
{
    ssize   size;
    
    size = mprGetBufLength(packet->content);
    return size > q->max || size > q->packetSize;
}


/*  
    Join a packet onto the service queue. This joins packet content data.
 */
void httpJoinPacketForService(HttpQueue *q, HttpPacket *packet, bool serviceQ)
{
    if (q->first == 0) {
        /*  Just use the service queue as a holding queue while we aggregate the post data.  */
        httpPutForService(q, packet, HTTP_DELAY_SERVICE);

    } else {
        /* Skip over the header packet */
        if (q->first && q->first->flags & HTTP_PACKET_HEADER) {
            packet = q->first->next;
            q->first = packet;
        } else {
            /* Aggregate all data into one packet and free the packet.  */
            httpJoinPacket(q->first, packet);
        }
        q->count += httpGetPacketLength(packet);
    }
    mprAssert(httpVerifyQueue(q));
    if (serviceQ && !(q->flags & HTTP_QUEUE_DISABLED))  {
        httpScheduleQueue(q);
    }
}


/*  
    Join two packets by pulling the content from the second into the first.
    WARNING: this will not update the queue count. Assumes the either both are on the queue or neither. 
 */
int httpJoinPacket(HttpPacket *packet, HttpPacket *p)
{
    ssize   len;

    mprAssert(packet->esize == 0);
    mprAssert(p->esize == 0);

    len = httpGetPacketLength(p);
    if (mprPutBlockToBuf(packet->content, mprGetBufStart(p->content), (ssize) len) != len) {
        mprAssert(0);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


/*
    Join queue packets up to the maximum of the given size and the downstream queue packet size.
    WARNING: this will not update the queue count.
 */
void httpJoinPackets(HttpQueue *q, ssize size)
{
    HttpPacket  *packet, *first;
    ssize       len;

    if (size < 0) {
        size = MAXINT;
    }
    if ((first = q->first) != 0 && first->next) {
        if (first->flags & HTTP_PACKET_HEADER) {
            /* Step over a header packet */
            first = first->next;
        }
        for (packet = first->next; packet; packet = packet->next) {
            if (packet->content == 0 || (len = httpGetPacketLength(packet)) == 0) {
                break;
            }
            mprAssert(!(packet->flags & HTTP_PACKET_END));
            httpJoinPacket(first, packet);
            /* Unlink the packet */
            first->next = packet->next;
        }
    }
}


void httpPutPacket(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(packet);
    mprAssert(q->put);

    q->put(q, packet);
}


/*  
    Pass to the next queue
 */
void httpPutPacketToNext(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(packet);
    mprAssert(q->nextQ->put);

    q->nextQ->put(q->nextQ, packet);
}


void httpPutPackets(HttpQueue *q)
{
    HttpPacket    *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        httpPutPacketToNext(q, packet);
    }
}


/*  
    Put the packet back at the front of the queue
 */
void httpPutBackPacket(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(packet);
    mprAssert(packet->next == 0);
    mprAssert(q->count >= 0);
    
    if (packet) {
        packet->next = q->first;
        if (q->first == 0) {
            q->last = packet;
        }
        q->first = packet;
        q->count += httpGetPacketLength(packet);
    }
}


/*  
    Put a packet on the service queue.
 */
void httpPutForService(HttpQueue *q, HttpPacket *packet, bool serviceQ)
{
    mprAssert(packet);
   
    q->count += httpGetPacketLength(packet);
    packet->next = 0;
    
    if (q->first) {
        q->last->next = packet;
        q->last = packet;
    } else {
        q->first = packet;
        q->last = packet;
    }
    if (serviceQ && !(q->flags & HTTP_QUEUE_DISABLED))  {
        httpScheduleQueue(q);
    }
}


/*  
    Resize and possibly split a packet so it fits in the downstream queue. Put back the 2nd portion of the split packet 
    on the queue. Ensure that the packet is not larger than "size" if it is greater than zero. If size < 0, then
    use the default packet size. 
 */
int httpResizePacket(HttpQueue *q, HttpPacket *packet, ssize size)
{
    HttpPacket  *tail;
    ssize       len;
    
    if (size <= 0) {
        size = MAXINT;
    }
    if (packet->esize > size) {
        if ((tail = httpSplitPacket(packet, size)) == 0) {
            return MPR_ERR_MEMORY;
        }
    } else {
        /*  
            Calculate the size that will fit downstream
         */
        len = packet->content ? httpGetPacketLength(packet) : 0;
        size = min(size, len);
        size = min(size, q->nextQ->max);
        size = min(size, q->nextQ->packetSize);
        if (size == 0 || size == len) {
            return 0;
        }
        if ((tail = httpSplitPacket(packet, size)) == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    httpPutBackPacket(q, tail);
    return 0;
}


/*  
    Split a packet at a given offset and return a new packet containing the data after the offset.
    The prefix data remains with the original packet. 
 */
HttpPacket *httpSplitPacket(HttpPacket *orig, ssize offset)
{
    HttpPacket  *packet;
    ssize       count, size;

    if (orig->esize) {
        if ((packet = httpCreateEntityPacket(orig->epos + offset, orig->esize - offset, orig->fill)) == 0) {
            return 0;
        }
        orig->esize = offset;

    } else {
        if (offset >= httpGetPacketLength(orig)) {
            mprAssert(offset < httpGetPacketLength(orig));
            return 0;
        }
        count = httpGetPacketLength(orig) - offset;
        size = max(count, HTTP_BUFSIZE);
        size = HTTP_PACKET_ALIGN(size);
        if ((packet = httpCreateDataPacket(size)) == 0) {
            return 0;
        }
        httpAdjustPacketEnd(orig, (ssize) -count);
        if (mprPutBlockToBuf(packet->content, mprGetBufEnd(orig->content), (ssize) count) != count) {
            return 0;
        }
#if BLD_DEBUG
        mprAddNullToBuf(orig->content);
#endif
    }
    packet->flags = orig->flags;
    return packet;
}


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
/************************************************************************/
/*
 *  End of file "./src/packet.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/passHandler.c"
 */
/************************************************************************/

/*
    passHandler.c -- Pass through handler

    This handler simply relays all content onto a connector. It is used to when there is no handler defined 
    and to convey errors when the actual handler fails.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




void httpHandleOptionsTrace(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    int         flags;

    tx = conn->tx;
    rx = conn->rx;

    if (rx->flags & HTTP_TRACE) {
        if (!conn->limits->enableTraceMethod) {
            tx->status = HTTP_CODE_NOT_ACCEPTABLE;
            httpFormatResponseBody(conn, "Trace Request Denied", "The TRACE method is disabled on this server.");
        } else {
            httpFormatResponse(conn, "%s %s %s\r\n", rx->method, rx->uri, conn->protocol);
        }
        httpFinalize(conn);

    } else if (rx->flags & HTTP_OPTIONS) {
        flags = tx->traceMethods;
        httpSetHeader(conn, "Allow", "OPTIONS%s%s%s%s%s%s",
            (conn->limits->enableTraceMethod) ? ",TRACE" : "",
            (flags & HTTP_STAGE_GET) ? ",GET" : "",
            (flags & HTTP_STAGE_HEAD) ? ",HEAD" : "",
            (flags & HTTP_STAGE_POST) ? ",POST" : "",
            (flags & HTTP_STAGE_PUT) ? ",PUT" : "",
            (flags & HTTP_STAGE_DELETE) ? ",DELETE" : "");
        httpOmitBody(conn);
        httpFinalize(conn);
    }
}


static void openPass(HttpQueue *q)
{
    mprLog(5, "Open passHandler");
    if (q->conn->rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        httpHandleOptionsTrace(q->conn);
    }
}


static void processPass(HttpQueue *q)
{
    HttpConn    *conn;

    conn = q->conn;
    if (!conn->finalized) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't serve request: %s", conn->rx->uri);
    }
    httpFinalize(conn);
}


int httpOpenPassHandler(Http *http)
{
    HttpStage     *stage;

    if ((stage = httpCreateHandler(http, "passHandler", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->passHandler = stage;
    stage->open = openPass;
    stage->process = processPass;

    /*
        PassHandler has an alias as the ErrorHandler too
     */
    if ((stage = httpCreateHandler(http, "errorHandler", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->open = openPass;
    stage->process = processPass;
    return 0;
}


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
/************************************************************************/
/*
 *  End of file "./src/passHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/pipeline.c"
 */
/************************************************************************/

/*
    pipeline.c -- HTTP pipeline processing. 
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static bool matchFilter(HttpConn *conn, HttpStage *filter, HttpRoute *route, int dir);
static void openQueues(HttpConn *conn);
static void pairQueues(HttpConn *conn);


void httpCreateTxPipeline(HttpConn *conn, HttpRoute *route)
{
    Http        *http;
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q;
    HttpStage   *stage, *filter;
    int         next, hasOutputFilters;

    mprAssert(conn);
    mprAssert(route);

    http = conn->http;
    rx = conn->rx;
    tx = conn->tx;

    tx->outputPipeline = mprCreateList(-1, 0);
    if (tx->handler == 0) {
        tx->handler = http->passHandler;
    }
    mprAddItem(tx->outputPipeline, tx->handler);

    hasOutputFilters = 0;
    if (route->outputStages) {
        for (next = 0; (filter = mprGetNextItem(route->outputStages, &next)) != 0; ) {
            if (matchFilter(conn, filter, route, HTTP_STAGE_TX) == HTTP_ROUTE_OK) {
                mprAddItem(tx->outputPipeline, filter);
                hasOutputFilters = 1;
            }
        }
    }
    if (tx->connector == 0) {
        if (tx->handler == http->fileHandler && (rx->flags & HTTP_GET) && !hasOutputFilters && 
                !conn->secure && httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_BODY, tx->ext) < 0) {
            tx->connector = http->sendConnector;
        } else if (route && route->connector) {
            tx->connector = route->connector;
        } else {
            tx->connector = http->netConnector;
        }
    }
    mprAddItem(tx->outputPipeline, tx->connector);

    /*  Create the outgoing queue heads and open the queues */
    q = tx->queue[HTTP_QUEUE_TX];
    for (next = 0; (stage = mprGetNextItem(tx->outputPipeline, &next)) != 0; ) {
        q = httpCreateQueue(conn, stage, HTTP_QUEUE_TX, q);
    }
    conn->writeq = tx->queue[HTTP_QUEUE_TX]->nextQ;
    conn->connectorq = tx->queue[HTTP_QUEUE_TX]->prevQ;

    pairQueues(conn);
    /*
        Put the header before opening the queues incase an open routine actually services and completes the request
        httpHandleOptionsTrace does this when called from openFile() in fileHandler.
     */
    httpPutForService(conn->writeq, httpCreateHeaderPacket(), HTTP_DELAY_SERVICE);
    openQueues(conn);

    /*
        Refinalize if httpFinalize was called before the Tx pipeline was created
     */
    if (conn->refinalize) {
        conn->finalized = 0;
        httpFinalize(conn);
    }
}


void httpCreateRxPipeline(HttpConn *conn, HttpRoute *route)
{
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q;
    HttpStage   *stage, *filter;
    int         next;

    mprAssert(conn);
    mprAssert(route);

    rx = conn->rx;
    tx = conn->tx;
    rx->inputPipeline = mprCreateList(-1, 0);
    if (route) {
        for (next = 0; (filter = mprGetNextItem(route->inputStages, &next)) != 0; ) {
            if (matchFilter(conn, filter, route, HTTP_STAGE_RX) == HTTP_ROUTE_OK) {
                mprAddItem(rx->inputPipeline, filter);
            }
        }
    }
    mprAddItem(rx->inputPipeline, tx->handler);

    /*  Create the incoming queue heads and open the queues.  */
    q = tx->queue[HTTP_QUEUE_RX];
    for (next = 0; (stage = mprGetNextItem(rx->inputPipeline, &next)) != 0; ) {
        q = httpCreateQueue(conn, stage, HTTP_QUEUE_RX, q);
    }
    conn->readq = tx->queue[HTTP_QUEUE_RX]->prevQ;

    if (!conn->endpoint) {
        pairQueues(conn);
        openQueues(conn);
    }
}


static void pairQueues(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead, *rq, *rqhead;

    tx = conn->tx;
    qhead = tx->queue[HTTP_QUEUE_TX];
    rqhead = tx->queue[HTTP_QUEUE_RX];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        if (q->pair == 0) {
            for (rq = rqhead->nextQ; rq != rqhead; rq = rq->nextQ) {
                if (q->stage == rq->stage) {
                    q->pair = rq;
                    rq->pair = q;
                }
            }
        }
    }
}


static void openQueues(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;
    int         i;

    tx = conn->tx;
    for (i = 0; i < HTTP_MAX_QUEUE; i++) {
        qhead = tx->queue[i];
        for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
            if (q->open && !(q->flags & (HTTP_QUEUE_OPEN))) {
                if (q->pair == 0 || !(q->pair->flags & HTTP_QUEUE_OPEN)) {
                    q->flags |= HTTP_QUEUE_OPEN;
                    httpOpenQueue(q, conn->tx->chunkSize);
                }
            }
        }
    }
}


void httpSetPipelineHandler(HttpConn *conn, HttpStage *handler)
{
    conn->tx->handler = (handler) ? handler : conn->http->passHandler;
}


void httpSetSendConnector(HttpConn *conn, cchar *path)
{
#if !BLD_FEATURE_ROMFS
    HttpTx      *tx;

    tx = conn->tx;
    tx->flags |= HTTP_TX_SENDFILE;
    tx->filename = sclone(path);
#else
    mprError("Send connector not available if ROMFS enabled");
#endif
}


void httpDestroyPipeline(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;
    int         i;

    tx = conn->tx;
    if (tx) {
        for (i = 0; i < HTTP_MAX_QUEUE; i++) {
            qhead = tx->queue[i];
            for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
                if (q->close && q->flags & HTTP_QUEUE_OPEN) {
                    q->flags &= ~HTTP_QUEUE_OPEN;
                    q->stage->close(q);
                }
            }
        }
    }
}


void httpStartPipeline(HttpConn *conn)
{
    HttpQueue   *qhead, *q, *prevQ, *nextQ;
    HttpTx      *tx;
    HttpRx      *rx;
    
    tx = conn->tx;
    if (tx->started) {
        return;
    }
    tx->started = 1;
    rx = conn->rx;
    if (rx->needInputPipeline) {
        qhead = tx->queue[HTTP_QUEUE_RX];
        for (q = qhead->nextQ; q->nextQ != qhead; q = nextQ) {
            nextQ = q->nextQ;
            if (q->start && !(q->flags & HTTP_QUEUE_STARTED)) {
                if (q->pair == 0 || !(q->pair->flags & HTTP_QUEUE_STARTED)) {
                    q->flags |= HTTP_QUEUE_STARTED;
                    q->stage->start(q);
                }
            }
        }
    }
    qhead = tx->queue[HTTP_QUEUE_TX];
    for (q = qhead->prevQ; q->prevQ != qhead; q = prevQ) {
        prevQ = q->prevQ;
        if (q->start && !(q->flags & HTTP_QUEUE_STARTED)) {
            q->flags |= HTTP_QUEUE_STARTED;
            q->stage->start(q);
        }
    }
    /* Start the handler last */
    q = qhead->nextQ;
    if (q->start) {
        mprAssert(!(q->flags & HTTP_QUEUE_STARTED));
        q->flags |= HTTP_QUEUE_STARTED;
        q->stage->start(q);
    }
    if (!conn->error && !conn->connectorComplete && rx->remainingContent > 0) {
        /* If no remaining content, wait till the processing stage to avoid duplicate writable events */
        httpWritable(conn);
    }
}


/*
    Note: this may be called multiple times
 */
void httpProcessPipeline(HttpConn *conn)
{
    HttpQueue   *q;
    
    if (conn->error) {
        httpFinalize(conn);
    }
    q = conn->tx->queue[HTTP_QUEUE_TX]->nextQ;
    if (q->stage->process) {
        q->stage->process(q);
    }
}


/*  
    Run the queue service routines until there is no more work to be done. NOTE: all I/O is non-blocking.
 */
bool httpServiceQueues(HttpConn *conn)
{
    HttpQueue   *q;
    int         workDone;

    workDone = 0;
    while (conn->state < HTTP_STATE_COMPLETE && (q = httpGetNextQueueForService(conn->serviceq)) != NULL) {
        if (q->servicing) {
            q->flags |= HTTP_QUEUE_RESERVICE;
        } else {
            mprAssert(q->schedulePrev == q->scheduleNext);
            httpServiceQueue(q);
            workDone = 1;
        }
    }
    return workDone;
}


void httpDiscardTransmitData(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;

    tx = conn->tx;
    if (tx == 0) {
        return;
    }
    qhead = tx->queue[HTTP_QUEUE_TX];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        httpDiscardData(q, 1);
    }
}


static bool matchFilter(HttpConn *conn, HttpStage *filter, HttpRoute *route, int dir)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (filter->match) {
        return filter->match(conn, route, dir);
    }
    if (filter->extensions && tx->ext) {
        return mprLookupKey(filter->extensions, tx->ext) != 0;
    }
    return 1;
}


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
/************************************************************************/
/*
 *  End of file "./src/pipeline.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/queue.c"
 */
/************************************************************************/

/*
    queue.c -- Queue support routines. Queues are the bi-directional data flow channels for the pipeline.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageQueue(HttpQueue *q, int flags);


HttpQueue *httpCreateQueueHead(HttpConn *conn, cchar *name)
{
    HttpQueue   *q;

    if ((q = mprAllocObj(HttpQueue, manageQueue)) == 0) {
        return 0;
    }
    httpInitQueue(conn, q, name);
    httpInitSchedulerQueue(q);
    return q;
}


HttpQueue *httpCreateQueue(HttpConn *conn, HttpStage *stage, int dir, HttpQueue *prev)
{
    HttpQueue   *q;

    if ((q = mprAllocObj(HttpQueue, manageQueue)) == 0) {
        return 0;
    }
    q->conn = conn;
    httpInitQueue(conn, q, stage->name);
    httpInitSchedulerQueue(q);
    httpAssignQueue(q, stage, dir);
    if (prev) {
        httpInsertQueue(prev, q);
    }
    return q;
}


static void manageQueue(HttpQueue *q, int flags)
{
    HttpPacket      *packet;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(q->owner);
        for (packet = q->first; packet; packet = packet->next) {
            mprMark(packet);
        }
        mprMark(q->last);
        mprMark(q->nextQ);
        mprMark(q->prevQ);
        mprMark(q->stage);
        mprMark(q->conn);
        mprMark(q->scheduleNext);
        mprMark(q->schedulePrev);
        mprMark(q->pair);
        mprMark(q->queueData);
        mprMark(q->queueData);
        if (q->nextQ && q->nextQ->stage) {
            /* Not a queue head */
            mprMark(q->nextQ);
        }
    }
}


void httpAssignQueue(HttpQueue *q, HttpStage *stage, int dir)
{
    q->stage = stage;
    q->close = stage->close;
    q->open = stage->open;
    q->start = stage->start;
    if (dir == HTTP_QUEUE_TX) {
        q->put = stage->outgoingData;
        q->service = stage->outgoingService;
    } else {
        q->put = stage->incomingData;
        q->service = stage->incomingService;
    }
    q->owner = stage->name;
}


void httpInitQueue(HttpConn *conn, HttpQueue *q, cchar *name)
{
    q->conn = conn;
    q->nextQ = q;
    q->prevQ = q;
    q->owner = sclone(name);
    q->packetSize = conn->limits->stageBufferSize;
    q->max = conn->limits->stageBufferSize;
    q->low = q->max / 100 *  5;    
}


/*  
    Insert a queue after the previous element
 */
void httpAppendQueue(HttpQueue *head, HttpQueue *q)
{
    q->nextQ = head;
    q->prevQ = head->prevQ;
    head->prevQ->nextQ = q;
    head->prevQ = q;
}


void httpDisableQueue(HttpQueue *q)
{
    mprLog(7, "Disable q %s", q->owner);
    q->flags |= HTTP_QUEUE_DISABLED;
}


/*  
    Remove all data in the queue. If removePackets is true, actually remove the packet too.
    This preserves the header and EOT packets.
 */
void httpDiscardData(HttpQueue *q, bool removePackets)
{
    HttpPacket  *packet, *prev, *next;
    ssize       len;

    if (q == 0) {
        return;
    }
    for (prev = 0, packet = q->first; packet; packet = next) {
        next = packet->next;
        if (packet->flags & (HTTP_PACKET_RANGE | HTTP_PACKET_DATA)) {
            if (removePackets) {
                if (prev) {
                    prev->next = next;
                } else {
                    q->first = next;
                }
                if (packet == q->last) {
                    q->last = prev;
                }
                q->count -= httpGetPacketLength(packet);
                mprAssert(q->count >= 0);
                continue;
            } else {
                len = httpGetPacketLength(packet);
                //  MOB - should this be done? Why not in the removePackets case?
                q->conn->tx->length -= len;
                q->count -= len;
                mprAssert(q->count >= 0);
                if (packet->content) {
                    mprFlushBuf(packet->content);
                }
            }
        }
        prev = packet;
    }
}


/*  
    Flush queue data by scheduling the queue and servicing all scheduled queues. Return true if there is room for more data.
    If blocking is requested, the call will block until the queue count falls below the queue max.
    WARNING: Be very careful when using blocking == true. Should only be used by end applications and not by middleware.
 */
bool httpFlushQueue(HttpQueue *q, bool blocking)
{
    HttpConn    *conn;
    HttpQueue   *next;

    conn = q->conn;
    mprAssert(conn->sock);
    do {
        httpScheduleQueue(q);
        next = q->nextQ;
        if (next->count >= next->max) {
            httpScheduleQueue(next);
        }
        httpServiceQueues(conn);
        if (conn->sock == 0) {
            break;
        }
    } while (blocking && q->count >= q->max);
    return (q->count < q->max) ? 1 : 0;
}


void httpEnableQueue(HttpQueue *q)
{
    mprLog(7, "Enable q %s", q->owner);
    q->flags &= ~HTTP_QUEUE_DISABLED;
    httpScheduleQueue(q);
}


HttpQueue *httpFindPreviousQueue(HttpQueue *q)
{
    while (q->prevQ) {
        q = q->prevQ;
        if (q->service) {
            return q;
        }
    }
    return 0;
}


HttpQueue *httpGetNextQueueForService(HttpQueue *q)
{
    HttpQueue     *next;
    
    if (q->scheduleNext != q) {
        next = q->scheduleNext;
        next->schedulePrev->scheduleNext = next->scheduleNext;
        next->scheduleNext->schedulePrev = next->schedulePrev;
        next->schedulePrev = next->scheduleNext = next;
        return next;
    }
    return 0;
}


/*  
    Return the number of bytes the queue will accept. Always positive.
 */
ssize httpGetQueueRoom(HttpQueue *q)
{
    mprAssert(q->max > 0);
    mprAssert(q->count >= 0);
    
    if (q->count >= q->max) {
        return 0;
    }
    return q->max - q->count;
}


void httpInitSchedulerQueue(HttpQueue *q)
{
    q->scheduleNext = q;
    q->schedulePrev = q;
}


/*  
    Insert a queue after the previous element
    TODO - rename append
 */
void httpInsertQueue(HttpQueue *prev, HttpQueue *q)
{
    q->nextQ = prev->nextQ;
    q->prevQ = prev;
    prev->nextQ->prevQ = q;
    prev->nextQ = q;
}


bool httpIsQueueEmpty(HttpQueue *q)
{
    return q->first == 0;
}


int httpOpenQueue(HttpQueue *q, ssize chunkSize)
{
    Http        *http;
    HttpConn    *conn;
    HttpStage   *stage;
    MprModule   *module;

    stage = q->stage;
    conn = q->conn;
    http = q->conn->http;

    if (chunkSize > 0) {
        q->packetSize = min(q->packetSize, chunkSize);
    }
    if (stage->flags & HTTP_STAGE_UNLOADED && stage->module) {
        module = stage->module;
        module = mprCreateModule(module->name, module->path, module->entry, http);
        if (mprLoadModule(module) < 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't load module %s", module->name);
            return MPR_ERR_CANT_READ;
        }
        stage->module = module;
    }
    if (stage->module) {
        stage->module->lastActivity = http->now;
    }
    q->flags |= HTTP_QUEUE_OPEN;
    if (q->open) {
        q->stage->open(q);
    }
    return 0;
}


/*  
    Read data. If sync mode, this will block. If async, will never block.
    Will return what data is available up to the requested size. Returns a byte count.
 */
ssize httpRead(HttpConn *conn, char *buf, ssize size)
{
    HttpPacket  *packet;
    HttpQueue   *q;
    MprBuf      *content;
    ssize       nbytes, len;

    q = conn->readq;
    mprAssert(q->count >= 0);
    mprAssert(size >= 0);
    mprAssert(httpVerifyQueue(q));

    while (q->count <= 0 && !conn->async && !conn->error && conn->sock && (conn->state <= HTTP_STATE_CONTENT)) {
        httpServiceQueues(conn);
        if (conn->sock) {
            httpWait(conn, 0, MPR_TIMEOUT_SOCKETS);
        }
    }
    //  TODO - better place for this?
    conn->lastActivity = conn->http->now;
    mprAssert(httpVerifyQueue(q));

    for (nbytes = 0; size > 0 && q->count > 0; ) {
        if ((packet = q->first) == 0) {
            break;
        }
        content = packet->content;
        len = mprGetBufLength(content);
        len = min(len, size);
        mprAssert(len <= q->count);
        if (len > 0) {
            len = mprGetBlockFromBuf(content, buf, len);
            mprAssert(len <= q->count);
        }
        buf += len;
        size -= len;
        q->count -= len;
        mprAssert(q->count >= 0);
        nbytes += len;
        if (mprGetBufLength(content) == 0) {
            httpGetPacket(q);
        }
    }
    mprAssert(q->count >= 0);
    mprAssert(httpVerifyQueue(q));
    return nbytes;
}


bool httpIsEof(HttpConn *conn) 
{
    return conn->rx == 0 || conn->rx->eof;
}


/*
    Read all the content buffered so far
 */
char *httpReadString(HttpConn *conn)
{
    HttpRx      *rx;
    ssize       sofar, nbytes, remaining;
    char        *content;

    rx = conn->rx;
    remaining = (ssize) min(MAXSSIZE, rx->length);

    if (remaining > 0) {
        content = mprAlloc(remaining + 1);
        sofar = 0;
        while (remaining > 0) {
            nbytes = httpRead(conn, &content[sofar], remaining);
            if (nbytes < 0) {
                return 0;
            }
            sofar += nbytes;
            remaining -= nbytes;
        }
    } else {
        content = mprAlloc(HTTP_BUFSIZE);
        sofar = 0;
        while (1) {
            nbytes = httpRead(conn, &content[sofar], HTTP_BUFSIZE);
            if (nbytes < 0) {
                return 0;
            } else if (nbytes == 0) {
                break;
            }
            sofar += nbytes;
            content = mprRealloc(content, sofar + HTTP_BUFSIZE);
        }
    }
    content[sofar] = '\0';
    return content;
}


void httpRemoveQueue(HttpQueue *q)
{
    q->prevQ->nextQ = q->nextQ;
    q->nextQ->prevQ = q->prevQ;
    q->prevQ = q->nextQ = q;
}


void httpScheduleQueue(HttpQueue *q)
{
    HttpQueue     *head;
    
    mprAssert(q->conn);
    head = q->conn->serviceq;
    
    if (q->scheduleNext == q && !(q->flags & HTTP_QUEUE_DISABLED)) {
        q->scheduleNext = head;
        q->schedulePrev = head->schedulePrev;
        head->schedulePrev->scheduleNext = q;
        head->schedulePrev = q;
    }
}


void httpServiceQueue(HttpQueue *q)
{
    q->conn->currentq = q;

    if (q->servicing) {
        q->flags |= HTTP_QUEUE_RESERVICE;
    } else {
        /*  
            Since we are servicing this "q" now, we can remove from the schedule queue if it is already queued.
         */
        if (q->conn->serviceq->scheduleNext == q) {
            httpGetNextQueueForService(q->conn->serviceq);
        }
        if (!(q->flags & HTTP_QUEUE_DISABLED)) {
            q->servicing = 1;
            q->service(q);
            if (q->flags & HTTP_QUEUE_RESERVICE) {
                q->flags &= ~HTTP_QUEUE_RESERVICE;
                httpScheduleQueue(q);
            }
            q->flags |= HTTP_QUEUE_SERVICED;
            q->servicing = 0;
        }
    }
}


/*  
    Return true if the next queue will accept this packet. If not, then disable the queue's service procedure.
    This may split the packet if it exceeds the downstreams maximum packet size.
 */
bool httpWillNextQueueAcceptPacket(HttpQueue *q, HttpPacket *packet)
{
    HttpQueue   *nextQ;
    ssize       size;

    nextQ = q->nextQ;
    size = httpGetPacketLength(packet);
    if (size <= nextQ->packetSize && (size + nextQ->count) <= nextQ->max) {
        return 1;
    }
    if (httpResizePacket(q, packet, 0) < 0) {
        return 0;
    }
    size = httpGetPacketLength(packet);
    mprAssert(size <= nextQ->packetSize);
    if ((size + nextQ->count) <= nextQ->max) {
        return 1;
    }
    /*  
        The downstream queue is full, so disable the queue and mark the downstream queue as full and service 
     */
    httpDisableQueue(q);
    nextQ->flags |= HTTP_QUEUE_FULL;
    if (!(nextQ->flags & HTTP_QUEUE_DISABLED)) {
        httpScheduleQueue(nextQ);
    }
    return 0;
}


/*  
    Return true if the next queue will accept a certain amount of data.
 */
bool httpWillNextQueueAcceptSize(HttpQueue *q, ssize size)
{
    HttpQueue   *nextQ;

    nextQ = q->nextQ;
    if (size <= nextQ->packetSize && (size + nextQ->count) <= nextQ->max) {
        return 1;
    }
    httpDisableQueue(q);
    nextQ->flags |= HTTP_QUEUE_FULL;
    if (!(nextQ->flags & HTTP_QUEUE_DISABLED)) {
        httpScheduleQueue(nextQ);
    }
    return 0;
}


/*  
    Write a block of data. This is the lowest level write routine for data. This will buffer the data and flush if
    the queue buffer is full. This will always accept the data.
 */
ssize httpWriteBlock(HttpQueue *q, cchar *buf, ssize size)
{
    HttpPacket  *packet;
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       bytes, written, packetSize;

    mprAssert(q == q->conn->writeq);
               
    conn = q->conn;
    tx = conn->tx;
    if (conn->finalized || tx == 0) {
        return MPR_ERR_CANT_WRITE;
    }
    conn->responded = 1;

    for (written = 0; size > 0; ) {
        LOG(7, "httpWriteBlock q_count %d, q_max %d", q->count, q->max);
        if (conn->state >= HTTP_STATE_COMPLETE) {
            return MPR_ERR_CANT_WRITE;
        }
        if (q->last != q->first && q->last->flags & HTTP_PACKET_DATA) {
            packet = q->last;
            mprAssert(packet->content);
        } else {
            packet = 0;
        }
        if (packet == 0 || mprGetBufSpace(packet->content) == 0) {
            packetSize = (tx->chunkSize > 0) ? tx->chunkSize : q->packetSize;
            if ((packet = httpCreateDataPacket(packetSize)) == 0) {
                return MPR_ERR_MEMORY;
            }
            httpPutForService(q, packet, HTTP_DELAY_SERVICE);
        }
        if ((bytes = mprPutBlockToBuf(packet->content, buf, size)) == 0) {
            return MPR_ERR_MEMORY;
        }
        buf += bytes;
        size -= bytes;
        q->count += bytes;
        written += bytes;
    }
    if (q->count >= q->max) {
        httpFlushQueue(q, 0);
    }
    if (conn->error) {
        return MPR_ERR_CANT_WRITE;
    }
    return written;
}


ssize httpWriteString(HttpQueue *q, cchar *s)
{
    return httpWriteBlock(q, s, strlen(s));
}


ssize httpWriteSafeString(HttpQueue *q, cchar *s)
{
    s = mprEscapeHtml(s);
    return httpWriteString(q, s);
}


ssize httpWrite(HttpQueue *q, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;
    
    va_start(vargs, fmt);
    buf = sfmtv(fmt, vargs);
    va_end(vargs);
    return httpWriteString(q, buf);
}


bool httpVerifyQueue(HttpQueue *q)
{
    HttpPacket  *packet;
    ssize       count;

    count = 0;
    for (packet = q->first; packet; packet = packet->next) {
        count += httpGetPacketLength(packet);
    }
    mprAssert(count <= q->count);
    return count <= q->count;
}

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
/************************************************************************/
/*
 *  End of file "./src/queue.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/rangeFilter.c"
 */
/************************************************************************/

/*
    rangeFilter.c - Ranged request filter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static bool applyRange(HttpQueue *q, HttpPacket *packet);
static void createRangeBoundary(HttpConn *conn);
static HttpPacket *createRangePacket(HttpConn *conn, HttpRange *range);
static HttpPacket *createFinalRangePacket(HttpConn *conn);
static void outgoingRangeService(HttpQueue *q);
static bool fixRangeLength(HttpConn *conn);
static int matchRange(HttpConn *conn, HttpRoute *route, int dir);
static void startRange(HttpQueue *q);


int httpOpenRangeFilter(Http *http)
{
    HttpStage     *filter;

    mprLog(5, "Open range filter");
    if ((filter = httpCreateFilter(http, "rangeFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->rangeFilter = filter;
    filter->match = matchRange; 
    filter->start = startRange; 
    filter->outgoingService = outgoingRangeService; 
    return 0;
}


static int matchRange(HttpConn *conn, HttpRoute *route, int dir)
{
    mprAssert(conn->rx);

    if ((dir & HTTP_STAGE_TX) && conn->tx->outputRanges) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


static void startRange(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;
    /*
        The httpContentNotModified routine can set outputRanges to zero if returning not-modified.
     */
    if (tx->outputRanges == 0 || tx->status != HTTP_CODE_OK || !fixRangeLength(conn)) {
        httpRemoveQueue(q);
    } else {
        tx->status = HTTP_CODE_PARTIAL;
        if (tx->outputRanges->next) {
            createRangeBoundary(conn);
        }
    }
}


static void outgoingRangeService(HttpQueue *q)
{
    HttpPacket  *packet;
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (packet->flags & HTTP_PACKET_DATA) {
            if (!applyRange(q, packet)) {
                return;
            }
        } else {
            /*
                Send headers and end packet downstream
             */
            if (packet->flags & HTTP_PACKET_END && tx->rangeBoundary) {
                httpPutPacketToNext(q, createFinalRangePacket(conn));
            }
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            httpPutPacketToNext(q, packet);
        }
    }
}


static bool applyRange(HttpQueue *q, HttpPacket *packet)
{
    HttpRange   *range;
    HttpConn    *conn;
    HttpTx      *tx;
    MprOff      endPacket, length, gap, span;
    ssize       count;

    conn = q->conn;
    tx = conn->tx;
    range = tx->currentRange;

    /*  
        Process the data packet over multiple ranges ranges until all the data is processed or discarded.
        A packet may contain data or it may be empty with an associated entityLength. If empty, range packets
        are filled with entity data as required.
     */
    while (range && packet) {
        length = httpGetPacketEntityLength(packet);
        if (length <= 0) {
            break;
        }
        endPacket = tx->rangePos + length;
        if (endPacket < range->start) {
            /* Packet is before the next range, so discard the entire packet and seek forwards */
            tx->rangePos += length;
            break;

        } else if (tx->rangePos < range->start) {
            /*  Packet starts before range so skip some data, but some packet data is in range */
            gap = (range->start - tx->rangePos);
            tx->rangePos += gap;
            if (gap < length) {
                httpAdjustPacketStart(packet, (ssize) gap);
            }
            /* Keep going and examine next range */

        } else {
            /* In range */
            mprAssert(range->start <= tx->rangePos && tx->rangePos < range->end);
            span = min(length, (range->end - tx->rangePos));
            count = (ssize) min(span, q->nextQ->packetSize);
            mprAssert(count > 0);
            if (!httpWillNextQueueAcceptSize(q, count)) {
                httpPutBackPacket(q, packet);
                return 0;
            }
            if (length > count) {
                /* Split packet if packet extends past range */
                httpPutBackPacket(q, httpSplitPacket(packet, count));
            }
            if (packet->fill && (*packet->fill)(q, packet, tx->rangePos, count) < 0) {
                return 0;
            }
            if (tx->rangeBoundary) {
                httpPutPacketToNext(q, createRangePacket(conn, range));
            }
            httpPutPacketToNext(q, packet);
            packet = 0;
            tx->rangePos += count;
        }
        if (tx->rangePos >= range->end) {
            tx->currentRange = range = range->next;
        }
    }
    return 1;
}


/*  
    Create a range boundary packet
 */
static HttpPacket *createRangePacket(HttpConn *conn, HttpRange *range)
{
    HttpPacket  *packet;
    HttpTx      *tx;
    char        *length;

    tx = conn->tx;

    length = (tx->entityLength >= 0) ? itos(tx->entityLength) : "*";
    packet = httpCreatePacket(HTTP_RANGE_BUFSIZE);
    packet->flags |= HTTP_PACKET_RANGE;
    mprPutFmtToBuf(packet->content, 
        "\r\n--%s\r\n"
        "Content-Range: bytes %Ld-%Ld/%s\r\n\r\n",
        tx->rangeBoundary, range->start, range->end - 1, length);
    return packet;
}


/*  
    Create a final range packet that follows all the data
 */
static HttpPacket *createFinalRangePacket(HttpConn *conn)
{
    HttpPacket  *packet;
    HttpTx      *tx;

    tx = conn->tx;

    packet = httpCreatePacket(HTTP_RANGE_BUFSIZE);
    packet->flags |= HTTP_PACKET_RANGE;
    mprPutFmtToBuf(packet->content, "\r\n--%s--\r\n", tx->rangeBoundary);
    return packet;
}


/*  
    Create a range boundary. This is required if more than one range is requested.
 */
static void createRangeBoundary(HttpConn *conn)
{
    HttpTx      *tx;
    int         when;

    tx = conn->tx;
    mprAssert(tx->rangeBoundary == 0);
    when = (int) conn->http->now;
    tx->rangeBoundary = sfmt("%08X%08X", PTOI(tx) + PTOI(conn) * when, when);
}


/*  
    Ensure all the range limits are within the entity size limits. Fixup negative ranges.
 */
static bool fixRangeLength(HttpConn *conn)
{
    HttpTx      *tx;
    HttpRange   *range;
    MprOff      length;

    tx = conn->tx;
    length = tx->entityLength ? tx->entityLength : tx->length;

    for (range = tx->outputRanges; range; range = range->next) {
        /*
                Range: 0-49             first 50 bytes
                Range: 50-99,200-249    Two 50 byte ranges from 50 and 200
                Range: -50              Last 50 bytes
                Range: 1-               Skip first byte then emit the rest
         */
        if (length) {
            if (range->end > length) {
                range->end = length;
            }
            if (range->start > length) {
                range->start = length;
            }
        }
        if (range->start < 0) {
            if (length <= 0) {
                /*
                    Can't compute an offset from the end as we don't know the entity length and it is not always possible
                    or wise to buffer all the output.
                 */
                httpError(conn, HTTP_CODE_RANGE_NOT_SATISFIABLE, "Can't compute end range with unknown content length"); 
                return 0;
            }
            /* select last -range-end bytes */
            range->start = length - range->end + 1;
            range->end = length;
        }
        if (range->end < 0) {
            if (length <= 0) {
                return 0;
            }
            range->end = length - range->end - 1;
        }
        range->len = (int) (range->end - range->start);
    }
    return 1;
}


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
/************************************************************************/
/*
 *  End of file "./src/rangeFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/route.c"
 */
/************************************************************************/

/*
    route.c -- Http request routing 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */


#include    "pcre.h"


#define GRADUATE_LIST(route, field) \
    if (route->field == 0) { \
        route->field = mprCreateList(-1, 0); \
    } else if (route->parent && route->field == route->parent->field) { \
        route->field = mprCloneList(route->parent->field); \
    }

#define GRADUATE_HASH(route, field) \
    mprAssert(route->field) ; \
    if (route->parent && route->field == route->parent->field) { \
        route->field = mprCloneHash(route->parent->field); \
    }


static void addUniqueItem(MprList *list, HttpRouteOp *op);
static HttpLang *createLangDef(cchar *path, cchar *suffix, int flags);
static HttpRouteOp *createRouteOp(cchar *name, int flags);
static void definePathVars(HttpRoute *route);
static void defineHostVars(HttpRoute *route);
static char *expandTokens(HttpConn *conn, cchar *path);
static char *expandPatternTokens(cchar *str, cchar *replacement, int *matches, int matchCount);
static char *expandRequestTokens(HttpConn *conn, char *targetKey);
static cchar *expandRouteName(HttpConn *conn, cchar *routeName);
static void finalizeMethods(HttpRoute *route);
static void finalizePattern(HttpRoute *route);
static char *finalizeReplacement(HttpRoute *route, cchar *str);
static char *finalizeTemplate(HttpRoute *route);
static bool opPresent(MprList *list, HttpRouteOp *op);
static void manageRoute(HttpRoute *route, int flags);
static void manageLang(HttpLang *lang, int flags);
static void manageRouteOp(HttpRouteOp *op, int flags);
static int matchRequestUri(HttpConn *conn, HttpRoute *route);
static int testRoute(HttpConn *conn, HttpRoute *route);
static char *qualifyName(HttpRoute *route, cchar *controller, cchar *name);
static int selectHandler(HttpConn *conn, HttpRoute *route);
static int testCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *condition);
static char *trimQuotes(char *str);
static int updateRequest(HttpConn *conn, HttpRoute *route, HttpRouteOp *update);

/*
    Host may be null
 */
HttpRoute *httpCreateRoute(HttpHost *host)
{
    HttpRoute  *route;

    if ((route = mprAllocObj(HttpRoute, manageRoute)) == 0) {
        return 0;
    }
    route->auth = httpCreateAuth();
    route->defaultLanguage = sclone("en");
    route->dir = mprGetCurrentPath(".");
    route->errorDocuments = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
    route->extensions = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    route->flags = HTTP_ROUTE_GZIP;
    route->handlers = mprCreateList(-1, 0);
    route->handlersWithMatch = mprCreateList(-1, 0);
    route->host = host;
    route->http = MPR->httpService;
    route->indicies = mprCreateList(-1, 0);
    route->inputStages = mprCreateList(-1, 0);
    route->lifespan = HTTP_CACHE_LIFESPAN;
    route->outputStages = mprCreateList(-1, 0);
    route->pathTokens = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    route->pattern = MPR->emptyString;
    route->targetRule = sclone("run");
    route->autoDelete = 1;
    route->workers = -1;
    route->limits = mprMemdup(((Http*) MPR->httpService)->serverLimits, sizeof(HttpLimits));
    route->mimeTypes = MPR->mimeTypes;
    httpInitTrace(route->trace);

    if ((route->mimeTypes = mprCreateMimeTypes("mime.types")) == 0) {
        route->mimeTypes = MPR->mimeTypes;                                                                  
    }  
    definePathVars(route);
    return route;
}


/*  
    Create a new location block. Inherit from the parent. We use a copy-on-write scheme if these are modified later.
 */
HttpRoute *httpCreateInheritedRoute(HttpRoute *parent)
{
    HttpRoute  *route;

    mprAssert(parent);
    if ((route = mprAllocObj(HttpRoute, manageRoute)) == 0) {
        return 0;
    }
    route->parent = parent;
    route->auth = httpCreateInheritedAuth(parent->auth);
    route->autoDelete = parent->autoDelete;
    route->caching = parent->caching;
    route->conditions = parent->conditions;
    route->connector = parent->connector;
    route->defaultLanguage = parent->defaultLanguage;
    route->dir = parent->dir;
    route->data = parent->data;
    route->eroute = parent->eroute;
    route->errorDocuments = parent->errorDocuments;
    route->extensions = parent->extensions;
    route->handler = parent->handler;
    route->handlers = parent->handlers;
    route->handlersWithMatch = parent->handlersWithMatch;
    route->headers = parent->headers;
    route->http = MPR->httpService;
    route->host = parent->host;
    route->inputStages = parent->inputStages;
    route->indicies = parent->indicies;
    route->languages = parent->languages;
    route->lifespan = parent->lifespan;
    route->methods = parent->methods;
    route->methodSpec = parent->methodSpec;
    route->outputStages = parent->outputStages;
    route->params = parent->params;
    route->parent = parent;
    route->pathTokens = parent->pathTokens;
    route->pattern = parent->pattern;
    route->patternCompiled = parent->patternCompiled;
    route->optimizedPattern = parent->optimizedPattern;
    route->responseStatus = parent->responseStatus;
    route->script = parent->script;
    route->prefix = parent->prefix;
    route->prefixLen = parent->prefixLen;
    route->scriptPath = parent->scriptPath;
    route->sourceName = parent->sourceName;
    route->sourcePath = parent->sourcePath;
    route->ssl = parent->ssl;
    route->target = parent->target;
    route->targetRule = parent->targetRule;
    route->tokens = parent->tokens;
    route->updates = parent->updates;
    route->uploadDir = parent->uploadDir;
    route->workers = parent->workers;
    route->limits = parent->limits;
    route->mimeTypes = parent->mimeTypes;
    route->trace[0] = parent->trace[0];
    route->trace[1] = parent->trace[1];
    route->log = parent->log;
    route->logFormat = parent->logFormat;
    route->logPath = parent->logPath;
    route->logSize = parent->logSize;
    route->logBackup = parent->logBackup;
    route->logFlags = parent->logFlags;
    route->flags = parent->flags & ~(HTTP_ROUTE_FREE_PATTERN);
    return route;
}


static void manageRoute(HttpRoute *route, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(route->name);
        mprMark(route->pattern);
        mprMark(route->startSegment);
        mprMark(route->startWith);
        mprMark(route->optimizedPattern);
        mprMark(route->prefix);
        mprMark(route->tplate);
        mprMark(route->targetRule);
        mprMark(route->target);
        mprMark(route->dir);
        mprMark(route->indicies);
        mprMark(route->methodSpec);
        mprMark(route->handler);
        mprMark(route->caching);
        mprMark(route->auth);
        mprMark(route->http);
        mprMark(route->host);
        mprMark(route->parent);
        mprMark(route->defaultLanguage);
        mprMark(route->extensions);
        mprMark(route->handlers);
        mprMark(route->handlersWithMatch);
        mprMark(route->connector);
        mprMark(route->data);
        mprMark(route->eroute);
        mprMark(route->pathTokens);
        mprMark(route->languages);
        mprMark(route->inputStages);
        mprMark(route->outputStages);
        mprMark(route->errorDocuments);
        mprMark(route->context);
        mprMark(route->uploadDir);
        mprMark(route->script);
        mprMark(route->scriptPath);
        mprMark(route->methods);
        mprMark(route->params);
        mprMark(route->headers);
        mprMark(route->conditions);
        mprMark(route->updates);
        mprMark(route->sourceName);
        mprMark(route->sourcePath);
        mprMark(route->tokens);
        mprMark(route->ssl);
        mprMark(route->limits);
        mprMark(route->mimeTypes);
        httpManageTrace(&route->trace[0], flags);
        httpManageTrace(&route->trace[1], flags);
        mprMark(route->log);
        mprMark(route->logFormat);
        mprMark(route->logPath);

    } else if (flags & MPR_MANAGE_FREE) {
        if (route->patternCompiled && (route->flags & HTTP_ROUTE_FREE_PATTERN)) {
            free(route->patternCompiled);
            route->patternCompiled = 0;
        }
    }
}


HttpRoute *httpCreateDefaultRoute(HttpHost *host)
{
    HttpRoute   *route;

    mprAssert(host);
    if ((route = httpCreateRoute(host)) == 0) {
        return 0;
    }
    httpSetRouteName(route, "default");
    httpFinalizeRoute(route);
    return route;
}


/*
    Create and configure a basic route. This is mainly used for client side piplines.
    Host may be null.
 */
HttpRoute *httpCreateConfiguredRoute(HttpHost *host, int serverSide)
{
    HttpRoute   *route;
    Http        *http;

    /*
        Create default incoming and outgoing pipelines. Order matters.
     */
    route = httpCreateRoute(host);
    http = route->http;
    httpAddRouteFilter(route, http->rangeFilter->name, NULL, HTTP_STAGE_TX);
    httpAddRouteFilter(route, http->chunkFilter->name, NULL, HTTP_STAGE_RX | HTTP_STAGE_TX);
    if (serverSide) {
        httpAddRouteFilter(route, http->uploadFilter->name, NULL, HTTP_STAGE_RX);
    }
    return route;
}


HttpRoute *httpCreateAliasRoute(HttpRoute *parent, cchar *pattern, cchar *path, int status)
{
    HttpRoute   *route;

    mprAssert(parent);
    mprAssert(pattern && *pattern);

    if ((route = httpCreateInheritedRoute(parent)) == 0) {
        return 0;
    }
    httpSetRoutePattern(route, pattern, 0);
    if (path) {
        httpSetRouteDir(route, path);
    }
    route->responseStatus = status;
    return route;
}


int httpStartRoute(HttpRoute *route)
{
#if !BLD_FEATURE_ROMFS
    if (route->logPath && (!route->parent || route->logPath != route->parent->logPath)) {
        if (route->logBackup > 0) {
            httpBackupRouteLog(route);
        }
        mprAssert(!route->log);
        route->log = mprOpenFile(route->logPath, O_CREAT | O_APPEND | O_WRONLY | O_TEXT, 0664);
        if (route->log == 0) {
            mprError("Can't open log file %s", route->logPath);
            return MPR_ERR_CANT_OPEN;
        }
    }
#endif
    return 0;
}


void httpStopRoute(HttpRoute *route)
{
    route->log = 0;
}


/*
    Find the matching route and handler for a request. If any errors occur, the pass handler is used to 
    pass errors via the net/sendfile connectors onto the client. This process may rewrite the request 
    URI and may redirect the request.
 */
void httpRouteRequest(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    int         next, rewrites, match;

    rx = conn->rx;
    tx = conn->tx;

    for (next = rewrites = 0; rewrites < HTTP_MAX_REWRITE; ) {
        if ((route = mprGetNextItem(conn->host->routes, &next)) == 0) {
            break;
        }
        if (route->startSegment && strncmp(rx->pathInfo, route->startSegment, route->startSegmentLen) != 0) {
            /* Failed to match the first URI segment, skip to the next group */
            mprAssert(next <= route->nextGroup);
            next = route->nextGroup;

        } else if (route->startWith && strncmp(rx->pathInfo, route->startWith, route->startWithLen) != 0) {
            /* Failed to match starting literal segment of the route pattern, advance to test the next route */
            continue;

        } else if ((match = httpMatchRoute(conn, route)) == HTTP_ROUTE_REROUTE) {
            next = 0;
            route = 0;
            rewrites++;

        } else if (match == HTTP_ROUTE_OK) {
            break;
        }
    }
    if (route == 0 || tx->handler == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't find suitable route for request");
        return;
    }
    rx->route = route;
    conn->limits = route->limits;

    conn->trace[0] = route->trace[0];
    conn->trace[1] = route->trace[1];

    if (conn->finalized) {
        tx->handler = conn->http->passHandler;
        if (rewrites >= HTTP_MAX_REWRITE) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Too many request rewrites");
        }
    }
    if (rx->traceLevel >= 0) {
        mprLog(rx->traceLevel, "Select handler: \"%s\" for \"%s\"", tx->handler->name, rx->uri);
    }
}



int httpMatchRoute(HttpConn *conn, HttpRoute *route)
{
    HttpRx      *rx;
    char        *savePathInfo, *pathInfo;
    int         rc;

    mprAssert(conn);
    mprAssert(route);

    rx = conn->rx;
    savePathInfo = 0;

    /*
        Remove the route prefix. Restore after matching.
     */
    if (route->prefix) {
        savePathInfo = rx->pathInfo;
        pathInfo = &rx->pathInfo[route->prefixLen];
        if (*pathInfo == '\0') {
            pathInfo = "/";
        }
        rx->pathInfo = sclone(pathInfo);
        rx->scriptName = route->prefix;
    }
    if ((rc = matchRequestUri(conn, route)) == HTTP_ROUTE_OK) {
        rc = testRoute(conn, route);
    }
    if (rc == HTTP_ROUTE_REJECT && route->prefix) {
        /* Keep the modified pathInfo if OK or REWRITE */
        rx->pathInfo = savePathInfo;
        rx->scriptName = 0;
    }
    return rc;
}


static int matchRequestUri(HttpConn *conn, HttpRoute *route)
{
    HttpRx      *rx;

    mprAssert(conn);
    mprAssert(route);
    rx = conn->rx;

    if (route->patternCompiled) {
        rx->matchCount = pcre_exec(route->patternCompiled, NULL, rx->pathInfo, (int) slen(rx->pathInfo), 0, 0, 
                rx->matches, sizeof(rx->matches) / sizeof(int));
        mprLog(6, "Test route pattern \"%s\", regexp %s, pathInfo %s", route->name, route->optimizedPattern, rx->pathInfo);

        if (route->flags & HTTP_ROUTE_NOT) {
            if (rx->matchCount > 0) {
                return HTTP_ROUTE_REJECT;
            }
            rx->matchCount = 1;
            rx->matches[0] = 0;
            rx->matches[1] = (int) slen(rx->pathInfo);

        } else if (rx->matchCount <= 0) {
            return HTTP_ROUTE_REJECT;
        }
    }
    mprLog(6, "Test route methods \"%s\"", route->name);
    if (route->methods && !mprLookupKey(route->methods, rx->method)) {
        return HTTP_ROUTE_REJECT;
    }
    rx->route = route;
    return HTTP_ROUTE_OK;
}


static int testRoute(HttpConn *conn, HttpRoute *route)
{
    HttpRouteOp     *op, *condition, *update;
    HttpRouteProc   *proc;
    HttpRx          *rx;
    HttpTx          *tx;
    cchar           *token, *value, *header, *field;
    int             next, rc, matched[HTTP_MAX_ROUTE_MATCHES * 2], count, result;

    mprAssert(conn);
    mprAssert(route);
    rx = conn->rx;
    tx = conn->tx;

    rx->target = route->target ? expandTokens(conn, route->target) : sclone(&conn->rx->pathInfo[1]);

    if (route->headers) {
        for (next = 0; (op = mprGetNextItem(route->headers, &next)) != 0; ) {
            mprLog(6, "Test route \"%s\" header \"%s\"", route->name, op->name);
            if ((header = httpGetHeader(conn, op->name)) != 0) {
                count = pcre_exec(op->mdata, NULL, header, (int) slen(header), 0, 0, 
                    matched, sizeof(matched) / sizeof(int)); 
                result = count > 0;
                if (op->flags & HTTP_ROUTE_NOT) {
                    result = !result;
                }
                if (!result) {
                    return HTTP_ROUTE_REJECT;
                }
            }
        }
    }
    if (route->params) {
        for (next = 0; (op = mprGetNextItem(route->params, &next)) != 0; ) {
            mprLog(6, "Test route \"%s\" field \"%s\"", route->name, op->name);
            if ((field = httpGetParam(conn, op->name, "")) != 0) {
                count = pcre_exec(op->mdata, NULL, field, (int) slen(field), 0, 0, 
                    matched, sizeof(matched) / sizeof(int)); 
                result = count > 0;
                if (op->flags & HTTP_ROUTE_NOT) {
                    result = !result;
                }
                if (!result) {
                    return HTTP_ROUTE_REJECT;
                }
            }
        }
    }
    if (route->conditions) {
        for (next = 0; (condition = mprGetNextItem(route->conditions, &next)) != 0; ) {
            mprLog(6, "Test route \"%s\" condition \"%s\"", route->name, condition->name);
            rc = testCondition(conn, route, condition);
            if (rc == HTTP_ROUTE_REROUTE) {
                return rc;
            }
            if (condition->flags & HTTP_ROUTE_NOT) {
                rc = !rc;
            }
            if (rc == HTTP_ROUTE_REJECT) {
                return rc;
            }
        }
    }
    if (route->updates) {
        for (next = 0; (update = mprGetNextItem(route->updates, &next)) != 0; ) {
            mprLog(6, "Run route \"%s\" update \"%s\"", route->name, update->name);
            if ((rc = updateRequest(conn, route, update)) == HTTP_ROUTE_REROUTE) {
                return rc;
            }
        }
    }
    if (route->prefix) {
        /* This is needed by some handler match routines */
        httpSetParam(conn, "prefix", route->prefix);
    }
    if ((rc = selectHandler(conn, route)) != HTTP_ROUTE_OK) {
        return rc;
    }
    if (route->tokens) {
        for (next = 0; (token = mprGetNextItem(route->tokens, &next)) != 0; ) {
            value = snclone(&rx->pathInfo[rx->matches[next * 2]], rx->matches[(next * 2) + 1] - rx->matches[(next * 2)]);
            httpSetParam(conn, token, value);
        }
    }
    if ((proc = mprLookupKey(conn->http->routeTargets, route->targetRule)) == 0) {
        httpError(conn, -1, "Can't find route target rule \"%s\"", route->targetRule);
        return HTTP_ROUTE_REJECT;
    }
    if (rx->traceLevel >= 0) {
        mprLog(4, "Select route \"%s\" target \"%s\"", route->name, route->targetRule);
    }
    if ((rc = (*proc)(conn, route, 0)) != HTTP_ROUTE_OK) {
        return rc;
    }
    if (!conn->finalized && tx->handler->check) {
        rc = tx->handler->check(conn, route);
    }
    return rc;
}


static int selectHandler(HttpConn *conn, HttpRoute *route)
{
    HttpTx      *tx;
    int         next, rc;

    mprAssert(conn);
    mprAssert(route);

    tx = conn->tx;
    if (route->handler) {
        tx->handler = route->handler;
        return HTTP_ROUTE_OK;
    }
    /*
        Handlers with match routines are examined first (in-order)
     */
    for (next = 0; (tx->handler = mprGetNextItem(route->handlersWithMatch, &next)) != 0; ) {
        rc = tx->handler->match(conn, route, 0);
        if (rc == HTTP_ROUTE_OK || rc == HTTP_ROUTE_REROUTE) {
            return rc;
        }
    }
    if (!tx->handler) {
        /*
            Now match by extensions
         */
        if (!tx->ext || (tx->handler = mprLookupKey(route->extensions, tx->ext)) == 0) {
            tx->handler = mprLookupKey(route->extensions, "");
        }
    }
    return tx->handler ? HTTP_ROUTE_OK : HTTP_ROUTE_REJECT;
}


/*
    Map the target to physical storage. Sets tx->filename and tx->ext.
 */
void httpMapFile(HttpConn *conn, HttpRoute *route)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLang    *lang;
    MprPath     *info;

    mprAssert(conn);
    mprAssert(route);

    rx = conn->rx;
    tx = conn->tx;
    lang = rx->lang;

    mprAssert(rx->target);
    tx->filename = rx->target;
    if (lang && lang->path) {
        tx->filename = mprJoinPath(lang->path, tx->filename);
    }
    tx->filename = mprJoinPath(route->dir, tx->filename);
    tx->ext = httpGetExt(conn);
    info = &tx->fileInfo;
    mprGetPathInfo(tx->filename, info);
    if (info->valid) {
        tx->etag = sfmt("\"%x-%Lx-%Lx\"", info->inode, info->size, info->mtime);
    }
    LOG(7, "mapFile uri \"%s\", filename: \"%s\", extension: \"%s\"", rx->uri, tx->filename, tx->ext);
}



int httpAddRouteCondition(HttpRoute *route, cchar *name, cchar *details, int flags)
{
    HttpRouteOp *op;
    cchar       *errMsg;
    char        *pattern, *value;
    int         column;

    mprAssert(route);

    GRADUATE_LIST(route, conditions);
    if ((op = createRouteOp(name, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (scasematch(name, "auth")) {
        /* Nothing to do. Route->auth has it all */

    } else if (scasematch(name, "missing")) {
        op->details = finalizeReplacement(route, "${request:filename}");

    } else if (scasematch(name, "directory")) {
        op->details = finalizeReplacement(route, details);

    } else if (scasematch(name, "exists")) {
        op->details = finalizeReplacement(route, details);

    } else if (scasematch(name, "match")) {
        /* 
            Condition match string pattern
            String can contain matching ${tokens} from the route->pattern and can contain request ${tokens}
         */
        if (!httpTokenize(route, details, "%S %S", &value, &pattern)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if ((op->mdata = pcre_compile2(pattern, 0, 0, &errMsg, &column, NULL)) == 0) {
            mprError("Can't compile condition match pattern. Error %s at column %d", errMsg, column); 
            return MPR_ERR_BAD_SYNTAX;
        }
        op->details = finalizeReplacement(route, value);
        op->flags |= HTTP_ROUTE_FREE;
    }
    addUniqueItem(route->conditions, op);
    return 0;
}


int httpAddRouteFilter(HttpRoute *route, cchar *name, cchar *extensions, int direction)
{
    HttpStage   *stage;
    HttpStage   *filter;
    char        *extlist, *word, *tok;
    int         pos;

    mprAssert(route);
    
    stage = httpLookupStage(route->http, name);
    if (stage == 0) {
        mprError("Can't find filter %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    /*
        Clone an existing stage because each filter stores its own set of extensions to match against
     */
    filter = httpCloneStage(route->http, stage);

    if (extensions && *extensions) {
        filter->extensions = mprCreateHash(0, MPR_HASH_CASELESS);
        extlist = sclone(extensions);
        word = stok(extlist, " \t\r\n", &tok);
        while (word) {
            if (*word == '*' && word[1] == '.') {
                word += 2;
            } else if (*word == '.') {
                word++;
            } else if (*word == '\"' && word[1] == '\"') {
                word = "";
            }
            mprAddKey(filter->extensions, word, filter);
            word = stok(0, " \t\r\n", &tok);
        }
    }
    if (direction & HTTP_STAGE_RX && filter->incomingData) {
        GRADUATE_LIST(route, inputStages);
        mprAddItem(route->inputStages, filter);
    }
    if (direction & HTTP_STAGE_TX && filter->outgoingData) {
        GRADUATE_LIST(route, outputStages);
        if (smatch(name, "cacheFilter") && 
                (pos = mprGetListLength(route->outputStages) - 1) >= 0 &&
                smatch(((HttpStage*) mprGetLastItem(route->outputStages))->name, "chunkFilter")) {
            mprInsertItemAtPos(route->outputStages, pos, filter);
        } else {
            mprAddItem(route->outputStages, filter);
        }
    }
    return 0;
}


int httpAddRouteHandler(HttpRoute *route, cchar *name, cchar *extensions)
{
    Http            *http;
    HttpStage       *handler;
    char            *extlist, *word, *tok;

    mprAssert(route);

    http = route->http;
    if ((handler = httpLookupStage(http, name)) == 0) {
        mprError("Can't find stage %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    GRADUATE_HASH(route, extensions);

    if (extensions && *extensions) {
        /*
            Add to the handler extension hash. Skip over "*." and "."
         */ 
        extlist = sclone(extensions);
        word = stok(extlist, " \t\r\n", &tok);
        while (word) {
            if (*word == '*' && word[1] == '.') {
                word += 2;
            } else if (*word == '.') {
                word++;
            } else if (*word == '\"' && word[1] == '\"') {
                word = "";
            }
            mprAddKey(route->extensions, word, handler);
            word = stok(0, " \t\r\n", &tok);
        }

    } else {
        if (mprLookupItem(route->handlers, handler) < 0) {
            GRADUATE_LIST(route, handlers);
            mprAddItem(route->handlers, handler);
        }
        if (handler->match) {
            if (mprLookupItem(route->handlersWithMatch, handler) < 0) {
                GRADUATE_LIST(route, handlersWithMatch);
                mprAddItem(route->handlersWithMatch, handler);
            }
        } else {
            /*
                Only match by extensions if no-match routine provided.
             */
            mprAddKey(route->extensions, "", handler);
        }
    }
    return 0;
}


/*
    Header field valuePattern
 */
void httpAddRouteHeader(HttpRoute *route, cchar *header, cchar *value, int flags)
{
    HttpRouteOp     *op;
    cchar           *errMsg;
    int             column;

    mprAssert(route);
    mprAssert(header && *header);
    mprAssert(value && *value);

    GRADUATE_LIST(route, headers);
    if ((op = createRouteOp(header, flags | HTTP_ROUTE_FREE)) == 0) {
        return;
    }
    if ((op->mdata = pcre_compile2(value, 0, 0, &errMsg, &column, NULL)) == 0) {
        mprError("Can't compile header pattern. Error %s at column %d", errMsg, column); 
    } else {
        mprAddItem(route->headers, op);
    }
}


#if FUTURE && KEEP
void httpAddRouteLoad(HttpRoute *route, cchar *module, cchar *path)
{
    HttpRouteOp     *op;

    GRADUATE_LIST(route, updates);
    if ((op = createRouteOp("--load--", 0)) == 0) {
        return;
    }
    op->var = sclone(module);
    op->value = sclone(path);
    mprAddItem(route->updates, op);
}
#endif


/*
    Param field valuePattern
 */
void httpAddRouteParam(HttpRoute *route, cchar *field, cchar *value, int flags)
{
    HttpRouteOp     *op;
    cchar           *errMsg;
    int             column;

    mprAssert(route);
    mprAssert(field && *field);
    mprAssert(value && *value);

    GRADUATE_LIST(route, params);
    if ((op = createRouteOp(field, flags | HTTP_ROUTE_FREE)) == 0) {
        return;
    }
    if ((op->mdata = pcre_compile2(value, 0, 0, &errMsg, &column, NULL)) == 0) {
        mprError("Can't compile field pattern. Error %s at column %d", errMsg, column); 
    } else {
        mprAddItem(route->params, op);
    }
}


/*
    Add a route update record. These run to modify a request.
        Update rule var value
        rule == "cmd|param"
        details == "var value"
    Value can contain pattern and request tokens.
 */
int httpAddRouteUpdate(HttpRoute *route, cchar *rule, cchar *details, int flags)
{
    HttpRouteOp *op;
    char        *value;

    mprAssert(route);
    mprAssert(rule && *rule);

    GRADUATE_LIST(route, updates);
    if ((op = createRouteOp(rule, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (scasematch(rule, "cmd")) {
        op->details = sclone(details);

    } else if (scasematch(rule, "lang")) {
        /* Nothing to do */;

    } else if (scasematch(rule, "param")) {
        if (!httpTokenize(route, details, "%S %S", &op->var, &value)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        op->value = finalizeReplacement(route, value);
    
    } else {
        return MPR_ERR_BAD_SYNTAX;
    }
    addUniqueItem(route->updates, op);
    return 0;
}


void httpClearRouteStages(HttpRoute *route, int direction)
{
    mprAssert(route);

    if (direction & HTTP_STAGE_RX) {
        route->inputStages = mprCreateList(-1, 0);
    }
    if (direction & HTTP_STAGE_TX) {
        route->outputStages = mprCreateList(-1, 0);
    }
}


void httpDefineRouteTarget(cchar *key, HttpRouteProc *proc)
{
    mprAssert(key && *key);
    mprAssert(proc);

    mprAddKey(((Http*) MPR->httpService)->routeTargets, key, proc);
}


void httpDefineRouteCondition(cchar *key, HttpRouteProc *proc)
{
    mprAssert(key && *key);
    mprAssert(proc);

    mprAddKey(((Http*) MPR->httpService)->routeConditions, key, proc);
}


void httpDefineRouteUpdate(cchar *key, HttpRouteProc *proc)
{
    mprAssert(key && *key);
    mprAssert(proc);

    mprAddKey(((Http*) MPR->httpService)->routeUpdates, key, proc);
}


void *httpGetRouteData(HttpRoute *route, cchar *key)
{
    mprAssert(route);
    mprAssert(key && *key);

    if (!route->data) {
        return 0;
    }
    return mprLookupKey(route->data, key);
}


cchar *httpGetRouteDir(HttpRoute *route)
{
    mprAssert(route);
    return route->dir;
}


cchar *httpGetRouteMethods(HttpRoute *route)
{
    mprAssert(route);
    return route->methodSpec;
}


void httpResetRoutePipeline(HttpRoute *route)
{
    mprAssert(route);

    if (route->parent == 0) {
        route->errorDocuments = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
        route->caching = 0;
        route->extensions = 0;
        route->handlers = mprCreateList(-1, 0);
        route->handlersWithMatch = mprCreateList(-1, 0);
        route->inputStages = mprCreateList(-1, 0);
        route->indicies = mprCreateList(-1, 0);
    }
    route->outputStages = mprCreateList(-1, 0);
}


void httpResetHandlers(HttpRoute *route)
{
    mprAssert(route);
    route->handlers = mprCreateList(-1, 0);
    route->handlersWithMatch = mprCreateList(-1, 0);
}


void httpSetRouteAuth(HttpRoute *route, HttpAuth *auth)
{
    mprAssert(route);
    route->auth = auth;
}


void httpSetRouteAutoDelete(HttpRoute *route, bool enable)
{
    mprAssert(route);
    route->autoDelete = enable;
}


void httpSetRouteCompression(HttpRoute *route, int flags)
{
    mprAssert(route);
    route->flags &= (HTTP_ROUTE_GZIP);
    route->flags |= (HTTP_ROUTE_GZIP & flags);
}


int httpSetRouteConnector(HttpRoute *route, cchar *name)
{
    HttpStage     *stage;

    mprAssert(route);
    
    stage = httpLookupStage(route->http, name);
    if (stage == 0) {
        mprError("Can't find connector %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    route->connector = stage;
    return 0;
}


void httpSetRouteData(HttpRoute *route, cchar *key, void *data)
{
    mprAssert(route);
    mprAssert(key && *key);
    mprAssert(data);

    if (route->data == 0) {
        route->data = mprCreateHash(-1, 0);
    } else {
        GRADUATE_HASH(route, data);
    }
    mprAddKey(route->data, key, data);
}


void httpSetRouteFlags(HttpRoute *route, int flags)
{
    mprAssert(route);
    route->flags = flags;
}


int httpSetRouteHandler(HttpRoute *route, cchar *name)
{
    HttpStage     *handler;

    mprAssert(route);
    mprAssert(name && *name);
    
    if ((handler = httpLookupStage(route->http, name)) == 0) {
        mprError("Can't find handler %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    route->handler = handler;
    return 0;
}


void httpSetRouteDir(HttpRoute *route, cchar *path)
{
    mprAssert(route);
    mprAssert(path && *path);
    
    route->dir = httpMakePath(route, path);
    httpSetRoutePathVar(route, "DOCUMENT_ROOT", route->dir);
}


/*
    WARNING: internal API only. 
 */
void httpSetRouteHost(HttpRoute *route, HttpHost *host)
{
    mprAssert(route);
    mprAssert(host);
    
    route->host = host;
    defineHostVars(route);
}


void httpAddRouteIndex(HttpRoute *route, cchar *index)
{
    cchar   *item;
    int     next;

    mprAssert(route);
    mprAssert(index && *index);
    
    GRADUATE_LIST(route, indicies);
    for (ITERATE_ITEMS(route->indicies, item, next)) {
        if (smatch(index, item)) {
            return;
        }
    }
    mprAddItem(route->indicies, sclone(index));
}


void httpSetRouteMethods(HttpRoute *route, cchar *methods)
{
    mprAssert(route);
    mprAssert(methods && methods);

    route->methodSpec = sclone(methods);
    finalizeMethods(route);
}


void httpSetRouteName(HttpRoute *route, cchar *name)
{
    mprAssert(route);
    mprAssert(name && *name);
    
    route->name = sclone(name);
}


void httpSetRoutePattern(HttpRoute *route, cchar *pattern, int flags)
{
    mprAssert(route);
    mprAssert(pattern && *pattern);
    
    route->flags |= (flags & HTTP_ROUTE_NOT);
    route->pattern = sclone(pattern);
    finalizePattern(route);
}


void httpSetRoutePrefix(HttpRoute *route, cchar *prefix)
{
    mprAssert(route);
    mprAssert(prefix && *prefix);
    
    route->prefix = sclone(prefix);
    route->prefixLen = slen(prefix);
    if (route->pattern) {
        finalizePattern(route);
    }
}


void httpSetRouteSource(HttpRoute *route, cchar *source)
{
    mprAssert(route);
    mprAssert(source);

    /* Source can be empty */
    route->sourceName = sclone(source);
}


void httpSetRouteScript(HttpRoute *route, cchar *script, cchar *scriptPath)
{
    mprAssert(route);
    
    if (script) {
        mprAssert(*script);
        route->script = sclone(script);
    }
    if (scriptPath) {
        mprAssert(*scriptPath);
        route->scriptPath = sclone(scriptPath);
    }
}


/*
    Target names are extensible and hashed in http->routeTargets. 

        Target close
        Target redirect status [URI]
        Target run ${DOCUMENT_ROOT}/${request:uri}.gz
        Target run ${controller}-${name} 
        Target write [-r] status "Hello World\r\n"
 */
int httpSetRouteTarget(HttpRoute *route, cchar *rule, cchar *details)
{
    char    *redirect, *msg;

    mprAssert(route);
    mprAssert(rule && *rule);

    route->targetRule = sclone(rule);
    route->target = sclone(details);

    if (scasematch(rule, "close")) {
        route->target = sclone(details);

    } else if (scasematch(rule, "redirect")) {
        if (!httpTokenize(route, details, "%N ?S", &route->responseStatus, &redirect)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        route->target = finalizeReplacement(route, redirect);
        return 0;

    } else if (scasematch(rule, "run")) {
        route->target = finalizeReplacement(route, details);

    } else if (scasematch(rule, "write")) {
        /*
            Write [-r] status Message
         */
        if (sncmp(details, "-r", 2) == 0) {
            route->flags |= HTTP_ROUTE_RAW;
            details = &details[2];
        }
        if (!httpTokenize(route, details, "%N %S", &route->responseStatus, &msg)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        route->target = finalizeReplacement(route, msg);

    } else {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


void httpSetRouteTemplate(HttpRoute *route, cchar *tplate)
{
    mprAssert(route);
    mprAssert(tplate && *tplate);
    
    route->tplate = sclone(tplate);
}


void httpSetRouteWorkers(HttpRoute *route, int workers)
{
    mprAssert(route);
    route->workers = workers;
}


void httpAddRouteErrorDocument(HttpRoute *route, int status, cchar *url)
{
    char    *code;

    mprAssert(route);
    GRADUATE_HASH(route, errorDocuments);
    code = itos(status);
    mprAddKey(route->errorDocuments, code, sclone(url));
}


cchar *httpLookupRouteErrorDocument(HttpRoute *route, int code)
{
    char   *num;

    mprAssert(route);
    if (route->errorDocuments == 0) {
        return 0;
    }
    num = itos(code);
    return (cchar*) mprLookupKey(route->errorDocuments, num);
}


static void finalizeMethods(HttpRoute *route)
{
    char    *method, *methods, *tok;

    mprAssert(route);
    methods = route->methodSpec;
    if (methods && *methods && !scasematch(methods, "ALL") && !smatch(methods, "*")) {
        if ((route->methods = mprCreateHash(-1, 0)) == 0) {
            return;
        }
        methods = sclone(methods);
        while ((method = stok(methods, ", \t\n\r", &tok)) != 0) {
            mprAddKey(route->methods, method, route);
            methods = 0;
        }
    } else {
        route->methodSpec = sclone("*");
    }
}


/*
    Finalize the pattern. 
        - Change "\{n[:m]}" to "{n[:m]}"
        - Change "\~" to "~"
        - Change "(~ PAT ~)" to "(?: PAT )?"
        - Extract the tokens and change tokens: "{word}" to "([^/]*)"
 */
static void finalizePattern(HttpRoute *route)
{
    MprBuf      *pattern;
    cchar       *errMsg;
    char        *startPattern, *cp, *ep, *token, *field;
    ssize       len;
    int         column;

    mprAssert(route);
    route->tokens = mprCreateList(-1, 0);
    pattern = mprCreateBuf(-1, -1);
    startPattern = route->pattern[0] == '^' ? &route->pattern[1] : route->pattern;

    if (route->name == 0) {
        route->name = sclone(startPattern);
    }
    if (route->tplate == 0) {
        /* Do this while the prefix is still in the route pattern */
        route->tplate = finalizeTemplate(route);
    }
    /*
        Create an simple literal startWith string to optimize route rejection.
     */
    len = strcspn(startPattern, "^$*+?.(|{[\\");
    if (len) {
        route->startWith = snclone(startPattern, len);
        route->startWithLen = len;
        if ((cp = strchr(&route->startWith[1], '/')) != 0) {
            route->startSegment = snclone(route->startWith, cp - route->startWith);
        } else {
            route->startSegment = route->startWith;
        }
        route->startSegmentLen = slen(route->startSegment);
    } else {
        /* Pattern has special characters */
        route->startWith = 0;
        route->startWithLen = 0;
        route->startSegmentLen = 0;
        route->startSegment = 0;
    }

    /*
        Remove the route prefix from the start of the compiled pattern.
     */
    if (route->prefix && sstarts(startPattern, route->prefix)) {
        mprAssert(route->prefixLen <= route->startWithLen);
        startPattern = sfmt("^%s", &startPattern[route->prefixLen]);
    } else {
        startPattern = sjoin("^", startPattern, NULL);
    }
    for (cp = startPattern; *cp; cp++) {
        /* Alias for optional, non-capturing pattern:  "(?: PAT )?" */
        //  MOB - change ~ is confusing with ~ for top of app
        if (*cp == '(' && cp[1] == '~') {
            mprPutStringToBuf(pattern, "(?:");
            cp++;

        } else if (*cp == '(') {
            mprPutCharToBuf(pattern, *cp);
        } else if (*cp == '~' && cp[1] == ')') {
            mprPutStringToBuf(pattern, ")?");
            cp++;

        } else if (*cp == ')') {
            mprPutCharToBuf(pattern, *cp);

        } else if (*cp == '{') {
            if (cp > route->pattern && cp[-1] == '\\') {
                mprAdjustBufEnd(pattern, -1);
                mprPutCharToBuf(pattern, *cp);
            } else {
                if ((ep = schr(cp, '}')) != 0) {
                    /* Trim {} off the token and replace in pattern with "([^/]*)"  */
                    token = snclone(&cp[1], ep - cp - 1);
                    if ((field = schr(token, '=')) != 0) {
                        *field++ = '\0';
                        field = sfmt("(%s)", field);
                    } else {
                        field = "([^/]*)";
                    }
                    mprPutStringToBuf(pattern, field);
                    mprAddItem(route->tokens, token);
                    /* Params ends up looking like "$1:$2:$3:$4" */
                    cp = ep;
                }
            }
        } else if (*cp == '\\' && *cp == '~') {
            mprPutCharToBuf(pattern, *++cp);

        } else {
            mprPutCharToBuf(pattern, *cp);
        }
    }
    mprAddNullToBuf(pattern);
    route->optimizedPattern = sclone(mprGetBufStart(pattern));
    if (mprGetListLength(route->tokens) == 0) {
        route->tokens = 0;
    }
    if (route->patternCompiled && (route->flags & HTTP_ROUTE_FREE_PATTERN)) {
        free(route->patternCompiled);
    }
    if ((route->patternCompiled = pcre_compile2(route->optimizedPattern, 0, 0, &errMsg, &column, NULL)) == 0) {
        mprError("Can't compile route. Error %s at column %d", errMsg, column); 
    }
    route->flags |= HTTP_ROUTE_FREE_PATTERN;
}


static char *finalizeReplacement(HttpRoute *route, cchar *str)
{
    MprBuf      *buf;
    cchar       *item;
    cchar       *tok, *cp, *ep, *token;
    int         next, braced;

    mprAssert(route);

    /*
        Prepare a replacement string. Change $token to $N
     */
    buf = mprCreateBuf(-1, -1);
    if (str && *str) {
        for (cp = str; *cp; cp++) {
            if ((tok = schr(cp, '$')) != 0 && (tok == str || tok[-1] != '\\')) {
                if (tok > cp) {
                    mprPutBlockToBuf(buf, cp, tok - cp);
                }
                if ((braced = (*++tok == '{')) != 0) {
                    tok++;
                }
                if (*tok == '&' || *tok == '\'' || *tok == '`' || *tok == '$') {
                    mprPutCharToBuf(buf, '$');
                    mprPutCharToBuf(buf, *tok);
                    ep = tok + 1;
                } else {
                    if (braced) {
                        for (ep = tok; *ep && *ep != '}'; ep++) ;
                    } else {
                        for (ep = tok; *ep && isdigit((int) *ep); ep++) ;
                    }
                    token = snclone(tok, ep - tok);
                    if (schr(token, ':')) {
                        /* Double quote to get through two levels of expansion in writeTarget */
                        mprPutStringToBuf(buf, "$${");
                        mprPutStringToBuf(buf, token);
                        mprPutCharToBuf(buf, '}');
                    } else {
                        for (next = 0; (item = mprGetNextItem(route->tokens, &next)) != 0; ) {
                            if (scmp(item, token) == 0) {
                                break;
                            }
                        }
                        /*  Insert "$" in front of "{token}" */
                        if (item) {
                            mprPutCharToBuf(buf, '$');
                            mprPutIntToBuf(buf, next);
                        } else if (snumber(token)) {
                            mprPutCharToBuf(buf, '$');
                            mprPutStringToBuf(buf, token);
                        } else {
                            mprError("Can't find token \"%s\" in template \"%s\"", token, route->pattern);
                        }
                    }
                }
                if (braced) {
                    ep++;
                }
                cp = ep - 1;

            } else {
                if (*cp == '\\') {
                    if (cp[1] == 'r') {
                        mprPutCharToBuf(buf, '\r');
                        cp++;
                    } else if (cp[1] == 'n') {
                        mprPutCharToBuf(buf, '\n');
                        cp++;
                    } else {
                        mprPutCharToBuf(buf, *cp);
                    }
                } else {
                    mprPutCharToBuf(buf, *cp);
                }
            }
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


/*
    Convert a route pattern into a usable template to construct URI links
    NOTE: this is heuristic and not perfect. Users can define the template via the httpSetTemplate API or in appweb via the
    EspURITemplate configuration directive.
 */
static char *finalizeTemplate(HttpRoute *route)
{
    MprBuf  *buf;
    char    *sp, *tplate;

    if ((buf = mprCreateBuf(0, 0)) == 0) {
        return 0;
    }
    /*
        Note: the route->pattern includes the prefix
     */
    for (sp = route->pattern; *sp; sp++) {
        switch (*sp) {
        default:
            mprPutCharToBuf(buf, *sp);
            break;
        case '$':
            if (sp[1] == '\0') {
                sp++;
            } else {
                mprPutCharToBuf(buf, *sp);
            }
            break;
        case '^':
            if (sp > route->pattern) {
                mprPutCharToBuf(buf, *sp);
            }
            break;
        case '+':
        case '?':
        case '|':
        case '[':
        case ']':
        case '*':
        case '.':
            break;
        case '(':
            if (sp[1] == '~') {
                sp++;
            }
            break;
        case '~':
            if (sp[1] == ')') {
                sp++;
            } else {
                mprPutCharToBuf(buf, *sp);
            }
            break;
        case ')':
            break;
        case '\\':
            if (sp[1] == '\\') {
                mprPutCharToBuf(buf, *sp++);
            } else {
                mprPutCharToBuf(buf, *++sp);
            }
            break;
        case '{':
            mprPutCharToBuf(buf, '$');
            while (sp[1] && *sp != '}') {
                if (*sp == '=') {
                    while (sp[1] && *sp != '}') sp++;
                } else {
                    mprPutCharToBuf(buf, *sp++);
                }
            }
            mprPutCharToBuf(buf, '}');
            break;
        }
    }
    if (mprLookAtLastCharInBuf(buf) == '/') {
        mprAdjustBufEnd(buf, -1);
    }
    mprAddNullToBuf(buf);
    if (mprGetBufLength(buf) > 0) {
        tplate = sclone(mprGetBufStart(buf));
    } else {
        tplate = sclone("/");
    }
    return tplate;
}


void httpFinalizeRoute(HttpRoute *route)
{
    /*
        Add the route to the owning host. When using an Appweb configuration file, the order of route finalization 
        will be from the inside out. This ensures that nested routes are defined BEFORE outer/enclosing routes.
        This is important as requests process routes in-order.
     */
    mprAssert(route);
    if (mprGetListLength(route->indicies) == 0) {
        mprAddItem(route->indicies,  sclone("index.html"));
    }
    httpAddRoute(route->host, route);
#if BLD_FEATURE_SSL
    mprConfigureSsl(route->ssl);
#endif
}


/*
    MOB - some description here
    what does this return. Does it return an absolute URI?
    MOB - rename httpUri() and move to uri.c
 */
char *httpLink(HttpConn *conn, cchar *target, MprHash *options)
{
    HttpRoute       *route, *lroute;
    HttpRx          *rx;
    HttpUri         *uri;
    cchar           *routeName, *action, *controller, *originalAction, *tplate;
    char            *rest;

    rx = conn->rx;
    route = rx->route;
    controller = 0;

    if (target == 0) {
        target = "";
    }
    if (*target == '@') {
        target = sjoin("{action: '", target, "'}", NULL);
    } 
    if (*target != '{') {
        target = httpTemplate(conn, target, 0);
    } else  {
        if (options) {
            options = mprBlendHash(httpGetOptions(target), options);
        } else {
            options = httpGetOptions(target);
        }
        /*
            Prep the action. Forms are:
                . @action               # Use the current controller
                . @controller/          # Use "list" as the action
                . @controller/action
         */
        if ((action = httpGetOption(options, "action", 0)) != 0) {
            originalAction = action;
            if (*action == '@') {
                action = &action[1];
            }
            if (strchr(action, '/')) {
                controller = stok((char*) action, "/", (char**) &action);
                action = stok((char*) action, "/", &rest);
            }
            if (controller) {
                httpSetOption(options, "controller", controller);
            } else {
                controller = httpGetParam(conn, "controller", 0);
            }
            if (action == 0 || *action == '\0') {
                action = "list";
            }
            if (action != originalAction) {
                httpSetOption(options, "action", action);
            }
        }
        /*
            Find the template to use. Strategy is this order:
                . options.template
                . options.route.template
                . options.action mapped to a route.template, via:
                . /app/STAR/action
                . /app/controller/action
                . /app/STAR/default
                . /app/controller/default
         */
        if ((tplate = httpGetOption(options, "template", 0)) == 0) {
            if ((routeName = httpGetOption(options, "route", 0)) != 0) {
                routeName = expandRouteName(conn, routeName);
                lroute = httpLookupRoute(conn->host, routeName);
            } else {
                lroute = 0;
            }
            if (lroute == 0) {
                if ((lroute = httpLookupRoute(conn->host, qualifyName(route, "{controller}", action))) == 0) {
                    if ((lroute = httpLookupRoute(conn->host, qualifyName(route, controller, action))) == 0) {
                        if ((lroute = httpLookupRoute(conn->host, qualifyName(route, "{controller}", "default"))) == 0) {
                            lroute = httpLookupRoute(conn->host, qualifyName(route, controller, "default"));
                        }
                    }
                }
            }
            if (lroute) {
                tplate = lroute->tplate;
            }
        }
        if (tplate) {
            target = httpTemplate(conn, tplate, options);
        } else {
            mprError("Can't find template for URI %s", target);
            target = "/";
        }
    }
    //  MOB OPT
    uri = httpCreateUri(target, 0);
    uri = httpResolveUri(httpCreateUri(rx->uri, 0), 1, &uri, 0);
    httpNormalizeUri(uri);
    return httpUriToString(uri, 0);
}


/*
    Limited expansion of route names. Support ~/ and ${app} at the start of the route name
 */
static cchar *expandRouteName(HttpConn *conn, cchar *routeName)
{
    HttpRoute   *route;

    route = conn->rx->route;
    if (routeName[0] == '~') {
        return sjoin(route->prefix, &routeName[1], NULL);
    }
    if (sstarts(routeName, "${app}")) {
        return sjoin(route->prefix, &routeName[6], NULL);
    }
    return routeName;
}


/*
    Expect a route->tplate with embedded tokens of the form: "/${controller}/${action}/${other}"
    The options is a hash of token values.
 */
char *httpTemplate(HttpConn *conn, cchar *tplate, MprHash *options)
{
    MprBuf      *buf;
    HttpRoute   *route;
    cchar       *cp, *ep, *value;
    char        key[MPR_MAX_STRING];

    route = conn->rx->route;
    if (tplate == 0 || *tplate == '\0') {
        return MPR->emptyString;
    }
    buf = mprCreateBuf(-1, -1);
    for (cp = tplate; *cp; cp++) {
        if (*cp == '~' && (cp == tplate || cp[-1] != '\\')) {
            if (route->prefix) {
                mprPutStringToBuf(buf, route->prefix);
            }

        } else if (*cp == '$' && cp[1] == '{' && (cp == tplate || cp[-1] != '\\')) {
            cp += 2;
            if ((ep = strchr(cp, '}')) != 0) {
                sncopy(key, sizeof(key), cp, ep - cp);
                if (options && (value = httpGetOption(options, key, 0)) != 0) {
                    mprPutStringToBuf(buf, value);
                } else if ((value = mprLookupKey(conn->rx->params, key)) != 0) {
                    mprPutStringToBuf(buf, value);
                }
                if (value == 0) {
                    /* Just emit the token name if the token can't be found */
                    mprPutStringToBuf(buf, key);
                }
                cp = ep;
            }
        } else {
            mprPutCharToBuf(buf, *cp);
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


void httpSetRoutePathVar(HttpRoute *route, cchar *key, cchar *value)
{
    mprAssert(route);
    mprAssert(key);
    mprAssert(value);

    GRADUATE_HASH(route, pathTokens);
    if (schr(value, '$')) {
        value = stemplate(value, route->pathTokens);
    }
    mprAddKey(route->pathTokens, key, sclone(value));
}


/*
    Make a path name. This replaces $references, converts to an absolute path name, cleans the path and maps delimiters.
    Paths are resolved relative to host->home (ServerRoot).
 */
char *httpMakePath(HttpRoute *route, cchar *file)
{
    char    *path;

    mprAssert(route);
    mprAssert(file);

    if ((path = stemplate(file, route->pathTokens)) == 0) {
        return 0;
    }
    if (mprIsPathRel(path) && route->host) {
        path = mprJoinPath(route->host->home, path);
    }
    return mprGetAbsPath(path);
}

/*
    Language can be an empty string
 */
int httpAddRouteLanguageSuffix(HttpRoute *route, cchar *language, cchar *suffix, int flags)
{
    HttpLang    *lp;

    mprAssert(route);
    mprAssert(language);
    mprAssert(suffix && *suffix);

    if (route->languages == 0) {
        route->languages = mprCreateHash(-1, 0);
    } else {
        GRADUATE_HASH(route, languages);
    }
    if ((lp = mprLookupKey(route->languages, language)) != 0) {
        lp->suffix = sclone(suffix);
        lp->flags = flags;
    } else {
        mprAddKey(route->languages, language, createLangDef(0, suffix, flags));
    }
    return httpAddRouteUpdate(route, "lang", 0, 0);
}


int httpAddRouteLanguageDir(HttpRoute *route, cchar *language, cchar *path)
{
    HttpLang    *lp;

    mprAssert(route);
    mprAssert(language && *language);
    mprAssert(path && *path);

    if (route->languages == 0) {
        route->languages = mprCreateHash(-1, 0);
    } else {
        GRADUATE_HASH(route, languages);
    }
    if ((lp = mprLookupKey(route->languages, language)) != 0) {
        lp->path = sclone(path);
    } else {
        mprAddKey(route->languages, language, createLangDef(path, 0, 0));
    }
    return httpAddRouteUpdate(route, "lang", 0, 0);
}


void httpSetRouteDefaultLanguage(HttpRoute *route, cchar *language)
{
    mprAssert(route);
    mprAssert(language && *language);

    route->defaultLanguage = sclone(language);
}



static int testCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *condition)
{
    HttpRouteProc   *proc;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(condition);

    if ((proc = mprLookupKey(conn->http->routeConditions, condition->name)) == 0) {
        httpError(conn, -1, "Can't find route condition rule %s", condition->name);
        return 0;
    }
    mprLog(6, "run condition on route %s condition %s", route->name, condition->name);
    return (*proc)(conn, route, condition);
}


/*
    Allow/Deny authorization
 */
static int allowDenyCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpRx      *rx;
    HttpAuth    *auth;
    int         allow, deny;

    mprAssert(conn);
    mprAssert(route);

    rx = conn->rx;
    auth = rx->route->auth;
    if (auth == 0) {
        return HTTP_ROUTE_OK;
    }
    allow = 0;
    deny = 0;
    if (auth->order == HTTP_ALLOW_DENY) {
        if (auth->allow && mprLookupKey(auth->allow, conn->ip)) {
            allow++;
        } else {
            allow++;
        }
        if (auth->deny && mprLookupKey(auth->deny, conn->ip)) {
            deny++;
        }
        if (!allow || deny) {
            httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access denied for this server %s", conn->ip);
            return HTTP_ROUTE_OK;
        }
    } else {
        if (auth->deny && mprLookupKey(auth->deny, conn->ip)) {
            deny++;
        }
        if (auth->allow && !mprLookupKey(auth->allow, conn->ip)) {
            deny = 0;
            allow++;
        } else {
            allow++;
        }
        if (deny || !allow) {
            httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access denied for this server %s", conn->ip);
            return HTTP_ROUTE_OK;
        }
    }
    return HTTP_ROUTE_OK;
}


static int authCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    mprAssert(conn);
    mprAssert(route);

    if (route->auth == 0 || route->auth->type == 0) {
        return HTTP_ROUTE_OK;
    }
    return httpCheckAuth(conn);
}


static int directoryCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpTx      *tx;
    MprPath     info;
    char        *path;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(op);

    /* 
        Must have tx->filename set when expanding op->details, so map target now 
     */
    tx = conn->tx;
    httpMapFile(conn, route);
    path = mprJoinPath(route->dir, expandTokens(conn, op->details));
    tx->ext = tx->filename = 0;
    mprGetPathInfo(path, &info);
    if (info.isDir) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*
    Test if a file exists
 */
static int existsCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpTx  *tx;
    char    *path;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(op);

    tx = conn->tx;

    /* 
        Must have tx->filename set when expanding op->details, so map target now 
     */
    httpMapFile(conn, route);
    path = mprJoinPath(route->dir, expandTokens(conn, op->details));
    tx->ext = tx->filename = 0;
    if (mprPathExists(path, R_OK)) {
        return HTTP_ROUTE_OK;
    }
    return 0;
}


static int matchCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    char    *str;
    int     matched[HTTP_MAX_ROUTE_MATCHES * 2], count;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(op);

    str = expandTokens(conn, op->details);
    count = pcre_exec(op->mdata, NULL, str, (int) slen(str), 0, 0, matched, sizeof(matched) / sizeof(int)); 
    if (count > 0) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}



static int updateRequest(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpRouteProc   *proc;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(op);

    if ((proc = mprLookupKey(conn->http->routeUpdates, op->name)) == 0) {
        httpError(conn, -1, "Can't find route update rule %s", op->name);
        return HTTP_ROUTE_OK;
    }
    mprLog(6, "run update on route %s update %s", route->name, op->name);
    return (*proc)(conn, route, op);
}


static int cmdUpdate(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    MprCmd  *cmd;
    char    *command, *out, *err;
    int     status;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(op);

    command = expandTokens(conn, op->details);
    cmd = mprCreateCmd(conn->dispatcher);
    if ((status = mprRunCmd(cmd, command, &out, &err, -1, 0)) != 0) {
        /* Don't call httpError, just set errorMsg which can be retrieved via: ${request:error} */
        conn->errorMsg = sfmt("Command failed: %s\nStatus: %d\n%s\n%s", command, status, out, err);
        mprError("%s", conn->errorMsg);
        /* Continue */
    }
    return HTTP_ROUTE_OK;
}


static int paramUpdate(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    mprAssert(conn);
    mprAssert(route);
    mprAssert(op);

    httpSetParam(conn, op->var, expandTokens(conn, op->value));
    return HTTP_ROUTE_OK;
}


static int langUpdate(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpUri     *prior;
    HttpRx      *rx;
    HttpLang    *lang;
    char        *ext, *pathInfo, *uri;

    mprAssert(conn);
    mprAssert(route);

    rx = conn->rx;
    prior = rx->parsedUri;
    mprAssert(route->languages);

    if ((lang = httpGetLanguage(conn, route->languages, 0)) != 0) {
        rx->lang = lang;
        if (lang->suffix) {
            pathInfo = 0;
            if (lang->flags & HTTP_LANG_AFTER) {
                pathInfo = sjoin(rx->pathInfo, ".", lang->suffix, NULL);
            } else if (lang->flags & HTTP_LANG_BEFORE) {
                ext = httpGetExt(conn);
                if (ext && *ext) {
                    pathInfo = sjoin(mprJoinPathExt(mprTrimPathExt(rx->pathInfo), lang->suffix), ".", ext, NULL);
                } else {
                    pathInfo = mprJoinPathExt(mprTrimPathExt(rx->pathInfo), lang->suffix);
                }
            }
            if (pathInfo) {
                uri = httpFormatUri(prior->scheme, prior->host, prior->port, pathInfo, prior->reference, prior->query, 0);
                httpSetUri(conn, uri, 0);
            }
        }
    }
    return HTTP_ROUTE_OK;
}



static int closeTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    mprAssert(conn);
    mprAssert(route);

    httpError(conn, HTTP_CODE_RESET, "Route target \"close\" is closing request");
    httpDisconnect(conn);
    return HTTP_ROUTE_OK;
}


static int redirectTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpRx      *rx;
    HttpUri     *dest, *prior;
    cchar       *scheme, *host, *query, *reference, *uri, *target;
    int         port;

    mprAssert(conn);
    mprAssert(route);
    mprAssert(route->target);

    rx = conn->rx;

    /* Note: target may be empty */
    target = expandTokens(conn, route->target);
    if (route->responseStatus) {
        httpRedirect(conn, route->responseStatus, target);
        return HTTP_ROUTE_OK;
    }
    prior = rx->parsedUri;
    dest = httpCreateUri(route->target, 0);
    scheme = dest->scheme ? dest->scheme : prior->scheme;
    host = dest->host ? dest->host : prior->host;
    port = dest->port ? dest->port : prior->port;
    query = dest->query ? dest->query : prior->query;
    reference = dest->reference ? dest->reference : prior->reference;
    uri = httpFormatUri(scheme, host, port, target, reference, query, 0);
    httpSetUri(conn, uri, 0);
    return HTTP_ROUTE_REROUTE;
}


static int runTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    /*
        Need to re-compute output string as updates may have run to define params which affect the route->target tokens
     */
    conn->rx->target = route->target ? expandTokens(conn, route->target) : sclone(&conn->rx->pathInfo[1]);
    return HTTP_ROUTE_OK;
}


static int writeTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    char    *str;

    mprAssert(conn);
    mprAssert(route);

    /*
        Need to re-compute output string as updates may have run to define params which affect the route->target tokens
     */
    str = route->target ? expandTokens(conn, route->target) : sclone(&conn->rx->pathInfo[1]);
    if (!(route->flags & HTTP_ROUTE_RAW)) {
        str = mprEscapeHtml(str);
    }
    httpSetStatus(conn, route->responseStatus);
    httpFormatResponse(conn, "%s", str);
    httpFinalize(conn);
    return HTTP_ROUTE_OK;
}



HttpRoute *httpDefineRoute(HttpRoute *parent, cchar *name, cchar *methods, cchar *pattern, cchar *target, 
        cchar *source)
{
    HttpRoute   *route;

    if ((route = httpCreateInheritedRoute(parent)) == 0) {
        return 0;
    }
    httpSetRouteName(route, name);
    httpSetRoutePattern(route, pattern, 0);
    if (methods) {
        httpSetRouteMethods(route, methods);
    }
    if (source) {
        httpSetRouteSource(route, source);
    }
    httpSetRouteTarget(route, "run", target);
    httpFinalizeRoute(route);
    return route;
}


/*
    Calculate a qualified route name. The form is: /{app}/{controller}/action
 */
static char *qualifyName(HttpRoute *route, cchar *controller, cchar *action)
{
    cchar   *prefix, *controllerPrefix;

    prefix = route->prefix ? route->prefix : "";
    if (action == 0 || *action == '\0') {
        action = "default";
    }
    if (controller) {
        controllerPrefix = (controller && smatch(controller, "{controller}")) ? "*" : controller;
        return sjoin(prefix, "/", controllerPrefix, "/", action, NULL);
    } else {
        return sjoin(prefix, "/", action, NULL);
    }
}


/*
    Add a restful route. The parent route may supply a route prefix. If defined, the route name will prepend the prefix.
 */
static void addRestful(HttpRoute *parent, cchar *action, cchar *methods, cchar *pattern, cchar *target, cchar *resource)
{
    cchar   *name, *nameResource, *source;

    nameResource = smatch(resource, "{controller}") ? "*" : resource;
    if (parent->prefix) {
        name = sfmt("%s/%s/%s", parent->prefix, nameResource, action);
        pattern = sfmt("^%s/%s%s", parent->prefix, resource, pattern);
    } else {
        name = sfmt("/%s/%s", nameResource, action);
        pattern = sfmt("^/%s%s", resource, pattern);
    }
    if (*resource == '{') {
        target = sfmt("$%s-%s", resource, target);
        source = sfmt("$%s.c", resource);
    } else {
        target = sfmt("%s-%s", resource, target);
        source = sfmt("%s.c", resource);
    }
    httpDefineRoute(parent, name, methods, pattern, target, source);
}


/*
    httpAddResourceGroup(parent, "{controller}")
 */
void httpAddResourceGroup(HttpRoute *parent, cchar *resource)
{
    addRestful(parent, "list",      "GET",    "(/)*$",                   "list",          resource);
    addRestful(parent, "init",      "GET",    "/init$",                  "init",          resource);
    addRestful(parent, "create",    "POST",   "(/)*$",                   "create",        resource);
    addRestful(parent, "edit",      "GET",    "/{id=[0-9]+}/edit$",      "edit",          resource);
    addRestful(parent, "show",      "GET",    "/{id=[0-9]+}$",           "show",          resource);
    addRestful(parent, "update",    "PUT",    "/{id=[0-9]+}$",           "update",        resource);
    addRestful(parent, "destroy",   "DELETE", "/{id=[0-9]+}$",           "destroy",       resource);
    //  MOB - rethink "custom" name - should action be after id? Should this be custom-${action}?
    addRestful(parent, "custom",    "POST",   "/{action}/{id=[0-9]+}$",  "${action}",     resource);
    addRestful(parent, "default",   "*",      "/{action}$",              "cmd-${action}", resource);
}


/*
    httpAddResource(parent, "{controller}")
 */
void httpAddResource(HttpRoute *parent, cchar *resource)
{
    addRestful(parent, "init",      "GET",    "/init$",       "init",          resource);
    addRestful(parent, "create",    "POST",   "(/)*$",        "create",        resource);
    addRestful(parent, "edit",      "GET",    "/edit$",       "edit",          resource);
    addRestful(parent, "show",      "GET",    "$",            "show",          resource);
    addRestful(parent, "update",    "PUT",    "$",            "update",        resource);
    addRestful(parent, "destroy",   "DELETE", "$",            "destroy",       resource);
    addRestful(parent, "default",   "*",      "/{action}$",   "cmd-${action}", resource);
}


void httpAddStaticRoute(HttpRoute *parent)
{
    cchar   *source, *name, *path, *pattern, *prefix;

    prefix = parent->prefix ? parent->prefix : "";
    source = parent->sourceName;
    name = qualifyName(parent, NULL, "home");
    path = stemplate("${STATIC_DIR}/index.esp", parent->pathTokens);
    pattern = sfmt("^%s%s", prefix, "(/)*$");
    httpDefineRoute(parent, name, "GET,POST,PUT", pattern, path, source);
}


void httpAddHomeRoute(HttpRoute *parent)
{
    cchar   *source, *name, *path, *pattern, *prefix;

    prefix = parent->prefix ? parent->prefix : "";
    source = parent->sourceName;

    name = qualifyName(parent, NULL, "static");
    path = stemplate("${STATIC_DIR}/$1", parent->pathTokens);
    pattern = sfmt("^%s%s", prefix, "/static/(.*)");
    httpDefineRoute(parent, name, "GET", pattern, path, source);
}


void httpAddRouteSet(HttpRoute *parent, cchar *set)
{
    if (scasematch(set, "simple")) {
        httpAddHomeRoute(parent);

    } else if (scasematch(set, "mvc")) {
        httpAddHomeRoute(parent);
        httpAddStaticRoute(parent);
        httpDefineRoute(parent, "default", NULL, "^/{controller}(~/{action}~)", "${controller}-${action}", 
            "${controller}.c");

    } else if (scasematch(set, "restful")) {
        httpAddHomeRoute(parent);
        httpAddStaticRoute(parent);
        httpAddResourceGroup(parent, "{controller}");

    } else if (!scasematch(set, "none")) {
        mprError("Unknown route set %s", set);
    }
}


/*
    Route operations are used per-route for headers and fields
 */
static HttpRouteOp *createRouteOp(cchar *name, int flags)
{
    HttpRouteOp   *op;

    mprAssert(name && *name);

    if ((op = mprAllocObj(HttpRouteOp, manageRouteOp)) == 0) {
        return 0;
    }
    op->name = sclone(name);
    op->flags = flags;
    return op;
}


static void manageRouteOp(HttpRouteOp *op, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(op->name);
        mprMark(op->details);
        mprMark(op->var);
        mprMark(op->value);

    } else if (flags & MPR_MANAGE_FREE) {
        if (op->flags & HTTP_ROUTE_FREE) {
            free(op->mdata);
            op->mdata = 0;
        }
    }
}


static bool opPresent(MprList *list, HttpRouteOp *op)
{
    HttpRouteOp   *last;

    if ((last = mprGetLastItem(list)) == 0) {
        return 0;
    }
    if (smatch(last->name, op->name) && 
        smatch(last->details, op->details) && 
        smatch(last->var, op->var) && 
        smatch(last->value, op->value) && 
        last->mdata == op->mdata && 
        last->flags == op->flags) {
        return 1;
    }
    return 0;
}


static void addUniqueItem(MprList *list, HttpRouteOp *op)
{
    mprAssert(list);
    mprAssert(op);

    if (!opPresent(list, op)) {
        mprAddItem(list, op);
    }
}


static HttpLang *createLangDef(cchar *path, cchar *suffix, int flags)
{
    HttpLang    *lang;

    if ((lang = mprAllocObj(HttpLang, manageLang)) == 0) {
        return 0;
    }
    if (path) {
        lang->path = sclone(path);
    }
    if (suffix) {
        lang->suffix = sclone(suffix);
    }
    lang->flags = flags;
    return lang;
}


static void manageLang(HttpLang *lang, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(lang->path);
        mprMark(lang->suffix);
    }
}


static void definePathVars(HttpRoute *route)
{
    mprAssert(route);

    mprAddKey(route->pathTokens, "PRODUCT", sclone(BLD_PRODUCT));
    mprAddKey(route->pathTokens, "OS", sclone(BLD_OS));
    mprAddKey(route->pathTokens, "VERSION", sclone(BLD_VERSION));
    mprAddKey(route->pathTokens, "LIBDIR", mprNormalizePath(sfmt("%s/../%s", mprGetAppDir(), BLD_LIB_NAME))); 
    if (route->host) {
        defineHostVars(route);
    }
}


static void defineHostVars(HttpRoute *route) 
{
    mprAssert(route);
    mprAddKey(route->pathTokens, "DOCUMENT_ROOT", route->dir);
    mprAddKey(route->pathTokens, "SERVER_ROOT", route->host->home);
}


static char *expandTokens(HttpConn *conn, cchar *str)
{
    HttpRx      *rx;

    mprAssert(conn);
    mprAssert(str);

    rx = conn->rx;
    return expandRequestTokens(conn, expandPatternTokens(rx->pathInfo, str, rx->matches, rx->matchCount));
}


/*
    WARNING: str is modified. Result is allocated string.
 */
static char *expandRequestTokens(HttpConn *conn, char *str)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    MprBuf      *buf;
    HttpLang    *lang;
    char        *tok, *cp, *key, *value, *field, *header, *defaultValue;

    mprAssert(conn);
    mprAssert(str);

    rx = conn->rx;
    route = rx->route;
    tx = conn->tx;
    buf = mprCreateBuf(-1, -1);
    tok = 0;

    for (cp = str; cp && *cp; ) {
        if ((tok = strstr(cp, "${")) == 0) {
            break;
        }
        if (tok > cp) {
            mprPutBlockToBuf(buf, cp, tok - cp);
        }
        if ((key = stok(&tok[2], ":", &value)) == 0) {
            continue;
        }
        stok(value, "}", &cp);

        if (smatch(key, "header")) {
            header = stok(value, "=", &defaultValue);
            if ((value = (char*) httpGetHeader(conn, header)) == 0) {
                value = defaultValue ? defaultValue : "";
            }
            mprPutStringToBuf(buf, value);

        } else if (smatch(key, "param")) {
            field = stok(value, "=", &defaultValue);
            if (defaultValue == 0) {
                defaultValue = "";
            }
            mprPutStringToBuf(buf, httpGetParam(conn, field, defaultValue));

        } else if (smatch(key, "request")) {
            value = stok(value, "=", &defaultValue);
            //  MOB - implement default value below for those that can be null
            //  MOB - OPT with switch on first char
            if (smatch(value, "clientAddress")) {
                mprPutStringToBuf(buf, conn->ip);

            } else if (smatch(value, "clientPort")) {
                mprPutIntToBuf(buf, conn->port);

            } else if (smatch(value, "error")) {
                mprPutStringToBuf(buf, conn->errorMsg);

            } else if (smatch(value, "ext")) {
                mprPutStringToBuf(buf, rx->parsedUri->ext);

            } else if (smatch(value, "extraPath")) {
                mprPutStringToBuf(buf, rx->extraPath);

            } else if (smatch(value, "filename")) {
                mprPutStringToBuf(buf, tx->filename);

            } else if (scasematch(value, "language")) {
                if (!defaultValue) {
                    defaultValue = route->defaultLanguage;
                }
                if ((lang = httpGetLanguage(conn, route->languages, defaultValue)) != 0) {
                    mprPutStringToBuf(buf, lang->suffix);
                } else {
                    mprPutStringToBuf(buf, defaultValue);
                }

            } else if (scasematch(value, "languageDir")) {
                lang = httpGetLanguage(conn, route->languages, 0);
                if (!defaultValue) {
                    defaultValue = ".";
                }
                mprPutStringToBuf(buf, lang ? lang->path : defaultValue);

            } else if (smatch(value, "host")) {
                mprPutStringToBuf(buf, rx->parsedUri->host);

            } else if (smatch(value, "method")) {
                mprPutStringToBuf(buf, rx->method);

            } else if (smatch(value, "originalUri")) {
                mprPutStringToBuf(buf, rx->originalUri);

            } else if (smatch(value, "pathInfo")) {
                mprPutStringToBuf(buf, rx->pathInfo);

            } else if (smatch(value, "prefix")) {
                mprPutStringToBuf(buf, route->prefix);

            } else if (smatch(value, "query")) {
                mprPutStringToBuf(buf, rx->parsedUri->query);

            } else if (smatch(value, "reference")) {
                mprPutStringToBuf(buf, rx->parsedUri->reference);

            } else if (smatch(value, "scheme")) {
                if (rx->parsedUri->scheme) {
                    mprPutStringToBuf(buf, rx->parsedUri->scheme);
                }  else {
                    mprPutStringToBuf(buf, (conn->secure) ? "https" : "http");
                }

            } else if (smatch(value, "scriptName")) {
                mprPutStringToBuf(buf, rx->scriptName);

            } else if (smatch(value, "serverAddress")) {
                mprPutStringToBuf(buf, conn->sock->acceptIp);

            } else if (smatch(value, "serverPort")) {
                mprPutIntToBuf(buf, conn->sock->acceptPort);

            } else if (smatch(value, "uri")) {
                mprPutStringToBuf(buf, rx->uri);
            }
        }
    }
    if (tok) {
        if (tok > cp) {
            mprPutBlockToBuf(buf, tok, tok - cp);
        }
    } else {
        mprPutStringToBuf(buf, cp);
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


/*
    Replace text using pcre regular expression match indicies
 */
static char *expandPatternTokens(cchar *str, cchar *replacement, int *matches, int matchCount)
{
    MprBuf  *result;
    cchar   *end, *cp, *lastReplace;
    int     submatch;

    mprAssert(str);
    mprAssert(replacement);
    mprAssert(matches);

    if (matchCount <= 0) {
        return MPR->emptyString;
    }
    result = mprCreateBuf(-1, -1);
    lastReplace = replacement;
    end = &replacement[slen(replacement)];

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
                mprPutSubStringToBuf(result, &str[matches[1]], slen(str) - matches[1]);
                break;
            default:
                /* Insert the nth submatch */
                if (isdigit((int) *cp)) {
                    submatch = (int) wtoi(cp);
                    while (isdigit((int) *++cp))
                        ;
                    cp--;
                    if (submatch < matchCount) {
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
    mprAddNullToBuf(result);
    return sclone(mprGetBufStart(result));
}


void httpDefineRouteBuiltins()
{
    /*
        These are the conditions that can be selected. Use httpAddRouteCondition to add to a route.
        The allow and auth conditions are internal and are configured via various Auth APIs.
     */
    httpDefineRouteCondition("allowDeny", allowDenyCondition);
    httpDefineRouteCondition("auth", authCondition);
    httpDefineRouteCondition("match", matchCondition);
    httpDefineRouteCondition("exists", existsCondition);
    httpDefineRouteCondition("directory", directoryCondition);

    httpDefineRouteUpdate("param", paramUpdate);
    httpDefineRouteUpdate("cmd", cmdUpdate);
    httpDefineRouteUpdate("lang", langUpdate);

    httpDefineRouteTarget("close", closeTarget);
    httpDefineRouteTarget("redirect", redirectTarget);
    httpDefineRouteTarget("run", runTarget);
    httpDefineRouteTarget("write", writeTarget);
}


/*
    Tokenizes a line using %formats. Mandatory tokens can be specified with %. Optional tokens are specified with ?. 
    Supported tokens:
        %B - Boolean. Parses: on/off, true/false, yes/no.
        %N - Number. Parses numbers in base 10.
        %S - String. Removes quotes.
        %T - Template String. Removes quotes and expand ${PathVars}.
        %P - Path string. Removes quotes and expands ${PathVars}. Resolved relative to host->dir (ServerRoot).
        %W - Parse words into a list
        %! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.
    Values wrapped in quotes will have the outermost quotes trimmed.
 */
bool httpTokenize(HttpRoute *route, cchar *line, cchar *fmt, ...)
{
    va_list     args;
    bool        rc;

    mprAssert(route);
    mprAssert(line);
    mprAssert(fmt);

    va_start(args, fmt);
    rc =  httpTokenizev(route, line, fmt, args);
    va_end(args);
    return rc;
}


bool httpTokenizev(HttpRoute *route, cchar *line, cchar *fmt, va_list args)
{
    MprList     *list;
    cchar       *f;
    char        *tok, *etok, *value, *word, *end;
    int         quote;

    mprAssert(route);
    mprAssert(fmt);

    if (line == 0) {
        line = "";
    }
    tok = sclone(line);
    end = &tok[slen(line)];

    for (f = fmt; *f && tok < end; f++) {
        for (; isspace((int) *tok); tok++) ;
        if (*tok == '\0' || *tok == '#') {
            break;
        }
        if (isspace((int) *f)) {
            continue;
        }
        if (*f == '%' || *f == '?') {
            f++;
            quote = 0;
            if (*f != '*' && (*tok == '"' || *tok == '\'')) {
                quote = *tok++;
            }
            if (*f == '!') {
                etok = &tok[1];
            } else {
                if (quote) {
                    for (etok = tok; *etok && !(*etok == quote && etok[-1] != '\\'); etok++) ; 
                    *etok++ = '\0';
                } else if (*f == '*') {
                    for (etok = tok; *etok; etok++) {
                        if (*etok == '#') {
                            *etok = '\0';
                        }
                    }
                } else {
                    for (etok = tok; *etok && !isspace((int) *etok); etok++) ;
                }
                *etok++ = '\0';
            }
            if (*f == '*') {
                f++;
                tok = trimQuotes(tok);
                * va_arg(args, char**) = tok;
                tok = etok;
                break;
            }

            switch (*f) {
            case '!':
                if (*tok == '!') {
                    *va_arg(args, int*) = HTTP_ROUTE_NOT;
                } else {
                    *va_arg(args, int*) = 0;
                    continue;
                }
                break;
            case 'B':
                if (scasecmp(tok, "on") == 0 || scasecmp(tok, "true") == 0 || scasecmp(tok, "yes") == 0) {
                    *va_arg(args, bool*) = 1;
                } else {
                    *va_arg(args, bool*) = 0;
                }
                break;
            case 'N':
                *va_arg(args, int*) = (int) stoi(tok);
                break;
            case 'P':
                *va_arg(args, char**) = httpMakePath(route, strim(tok, "\"", MPR_TRIM_BOTH));
                break;
            case 'S':
                *va_arg(args, char**) = strim(tok, "\"", MPR_TRIM_BOTH);
                break;
            case 'T':
                value = strim(tok, "\"", MPR_TRIM_BOTH);
                *va_arg(args, char**) = stemplate(value, route->pathTokens);
                break;
            case 'W':
                list = va_arg(args, MprList*);
                word = stok(tok, " \t\r\n", &tok);
                while (word) {
                    mprAddItem(list, word);
                    word = stok(0, " \t\r\n", &tok);
                }
                break;
            default:
                mprError("Unknown token pattern %%\"%c\"", *f);
                break;
            }
            tok = etok;
        }
    }
    if (tok < end) {
        /*
            Extra unparsed text
         */
        for (; tok < end && isspace((int) *tok); tok++) ;
        if (*tok) {
            mprError("Extra unparsed text: \"%s\"", tok);
            return 0;
        }
    }
    if (*f) {
        /*
            Extra unparsed format tokens
         */
        for (; *f; f++) {
            if (*f == '%') {
                break;
            } else if (*f == '?') {
                switch (*++f) {
                case '!':
                case 'N':
                    *va_arg(args, int*) = 0;
                    break;
                case 'B':
                    *va_arg(args, bool*) = 0;
                    break;
                case 'D':
                case 'P':
                case 'S':
                case 'T':
                case '*':
                    *va_arg(args, char**) = 0;
                    break;
                case 'W':
                    break;
                default:
                    mprError("Unknown token pattern %%\"%c\"", *f);
                    break;
                }
            }
        }
        if (*f) {
            mprError("Missing directive parameters");
            return 0;
        }
    }
    va_end(args);
    return 1;
}


static char *trimQuotes(char *str)
{
    ssize   len;

    mprAssert(str);
    len = slen(str);
    if (*str == '\"' && str[len - 1] == '\"' && len > 2 && str[1] != '\"') {
        return snclone(&str[1], len - 2);
    }
    return sclone(str);
}


MprHash *httpGetOptions(cchar *options)
{
    if (options == 0) {
        return mprCreateHash(-1, 0);
    }
    if (*options == '@') {
        /* Allow embedded URIs as options */
        options = sfmt("{ data-click: '%s'}", options);
    }
    mprAssert(*options == '{');
    if (*options != '{') {
        options = sfmt("{%s}", options);
    }
    return mprDeserialize(options);
}


cchar *httpGetOption(MprHash *options, cchar *field, cchar *defaultValue)
{
    MprKey      *kp;
    cchar       *value;

    if (options == 0) {
        value = defaultValue;
    } else if ((kp = mprLookupKeyEntry(options, field)) == 0) {
        value = defaultValue;
    } else {
        value = kp->data;
    }
    return value;
}


MprHash *httpGetOptionHash(MprHash *options, cchar *field)
{
    MprKey      *kp;

    if (options == 0) {
        return 0;
    }
    if ((kp = mprLookupKeyEntry(options, field)) == 0) {
        return 0;
    }
    if (kp->type != MPR_JSON_ARRAY && kp->type != MPR_JSON_OBJ) {
        return 0;
    }
    return (MprHash*) kp->data;
}


void httpAddOption(MprHash *options, cchar *field, cchar *value)
{
    MprKey      *kp;

    if (options == 0) {
        mprAssert(options);
        return;
    }
    if ((kp = mprLookupKeyEntry(options, field)) != 0) {
        kp = mprAddKey(options, field, sjoin(kp->data, " ", value, NULL));
    } else {
        kp = mprAddKey(options, field, value);
    }
    kp->type = MPR_JSON_STRING;
}


void httpRemoveOption(MprHash *options, cchar *field)
{
    if (options == 0) {
        mprAssert(options);
        return;
    }
    mprRemoveKey(options, field);
}


void httpSetOption(MprHash *options, cchar *field, cchar *value)
{
    MprKey  *kp;

    if (value == 0) {
        return;
    }
    if (options == 0) {
        mprAssert(options);
        return;
    }
    if ((kp = mprAddKey(options, field, value)) != 0) {
        kp->type = MPR_JSON_STRING;
    }
}


HttpLimits *httpGraduateLimits(HttpRoute *route)
{
    if (route->parent && route->limits == route->parent->limits) {
        route->limits = mprMemdup(((Http*) MPR->httpService)->serverLimits, sizeof(HttpLimits));
    }
    return route->limits;
}

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
/************************************************************************/
/*
 *  End of file "./src/route.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/rx.c"
 */
/************************************************************************/

/*
    rx.c -- Http receiver. Parses http requests and client responses.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void addMatchEtag(HttpConn *conn, char *etag);
static char *getToken(HttpConn *conn, cchar *delim);
static void manageRange(HttpRange *range, int flags);
static void manageRx(HttpRx *rx, int flags);
static bool parseAuthenticate(HttpConn *conn, char *authDetails);
static void parseHeaders(HttpConn *conn, HttpPacket *packet);
static bool parseIncoming(HttpConn *conn, HttpPacket *packet);
static bool parseRange(HttpConn *conn, char *value);
static void parseRequestLine(HttpConn *conn, HttpPacket *packet);
static void parseResponseLine(HttpConn *conn, HttpPacket *packet);
static bool processCompletion(HttpConn *conn);
static bool processContent(HttpConn *conn, HttpPacket *packet);
static void parseMethod(HttpConn *conn);
static bool processParsed(HttpConn *conn);
static bool processRunning(HttpConn *conn);
static void routeRequest(HttpConn *conn);


HttpRx *httpCreateRx(HttpConn *conn)
{
    HttpRx      *rx;

    if ((rx = mprAllocObj(HttpRx, manageRx)) == 0) {
        return 0;
    }
    rx->conn = conn;
    rx->length = -1;
    rx->ifMatch = 1;
    rx->ifModified = 1;
    rx->pathInfo = sclone("/");
    rx->scriptName = mprEmptyString();
    rx->needInputPipeline = !conn->endpoint;
    rx->headers = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    rx->chunkState = HTTP_CHUNK_UNCHUNKED;
    rx->traceLevel = -1;
    return rx;
}


static void manageRx(HttpRx *rx, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(rx->method);
        mprMark(rx->uri);
        mprMark(rx->pathInfo);
        mprMark(rx->scriptName);
        mprMark(rx->extraPath);
        mprMark(rx->conn);
        mprMark(rx->route);
        mprMark(rx->etags);
        mprMark(rx->headerPacket);
        mprMark(rx->headers);
        mprMark(rx->inputPipeline);
        mprMark(rx->parsedUri);
        mprMark(rx->requestData);
        mprMark(rx->statusMessage);
        mprMark(rx->accept);
        mprMark(rx->acceptCharset);
        mprMark(rx->acceptEncoding);
        mprMark(rx->acceptLanguage);
        mprMark(rx->cookie);
        mprMark(rx->connection);
        mprMark(rx->contentLength);
        mprMark(rx->hostHeader);
        mprMark(rx->pragma);
        mprMark(rx->mimeType);
        mprMark(rx->originalMethod);
        mprMark(rx->originalUri);
        mprMark(rx->redirect);
        mprMark(rx->referrer);
        mprMark(rx->securityToken);
        mprMark(rx->userAgent);
        mprMark(rx->params);
        mprMark(rx->svars);
        mprMark(rx->inputRange);
        mprMark(rx->authAlgorithm);
        mprMark(rx->authDetails);
        mprMark(rx->authStale);
        mprMark(rx->authType);
        mprMark(rx->files);
        mprMark(rx->uploadDir);
        mprMark(rx->paramString);
        mprMark(rx->lang);
        mprMark(rx->target);

    } else if (flags & MPR_MANAGE_FREE) {
        if (rx->conn) {
            rx->conn->rx = 0;
        }
    }
}


void httpDestroyRx(HttpRx *rx)
{
    if (rx->conn) {
        rx->conn->rx = 0;
        rx->conn = 0;
    }
}


/*  
    Process incoming requests and drive the state machine. This will process as many requests as possible before returning. 
    All socket I/O is non-blocking, and this routine must not block. Note: packet may be null.
 */
void httpProcess(HttpConn *conn, HttpPacket *packet)
{
    mprAssert(conn);

    conn->canProceed = 1;
    conn->advancing = 1;

    while (conn->canProceed) {
        LOG(7, "httpProcess %s, state %d, error %d", conn->dispatcher->name, conn->state, conn->error);

        switch (conn->state) {
        case HTTP_STATE_BEGIN:
        case HTTP_STATE_CONNECTED:
            conn->canProceed = parseIncoming(conn, packet);
            break;

        case HTTP_STATE_PARSED:
            conn->canProceed = processParsed(conn);
            break;

        case HTTP_STATE_CONTENT:
            conn->canProceed = processContent(conn, packet);
            break;

        case HTTP_STATE_RUNNING:
            conn->canProceed = processRunning(conn);
            break;

        case HTTP_STATE_COMPLETE:
            conn->canProceed = processCompletion(conn);
            break;
        }
        packet = conn->input;
    }
    conn->advancing = 0;
}


/*  
    Parse the incoming http message. Return true to keep going with this or subsequent request, zero means
    insufficient data to proceed.
 */
static bool parseIncoming(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    ssize       len;
    char        *start, *end;

    if (packet == NULL) {
        return 0;
    }
    if (mprShouldDenyNewRequests()) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_NOT_ACCEPTABLE, "Server terminating");
        return 0;
    }
    if (conn->rx == NULL) {
        conn->rx = httpCreateRx(conn);
        conn->tx = httpCreateTx(conn, NULL);
    }
    rx = conn->rx;
    if ((len = httpGetPacketLength(packet)) == 0) {
        return 0;
    }
    start = mprGetBufStart(packet->content);
    if ((end = scontains(start, "\r\n\r\n", len)) == 0) {
        if (len >= conn->limits->headerSize) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE, "Header too big");
        }
        return 0;
    }
    len = (int) (end - start);
    mprAddNullToBuf(packet->content);

    if (len >= conn->limits->headerSize) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE, "Header too big");
        return 0;
    }
    if (conn->endpoint) {
        parseRequestLine(conn, packet);
    } else {
        parseResponseLine(conn, packet);
    }
    if (!conn->error) {
        parseHeaders(conn, packet);
    }
    if (conn->endpoint) {
        httpMatchHost(conn);
        if (httpSetUri(conn, rx->uri, "") < 0) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad URL format");
        }
        if (conn->secure) {
            rx->parsedUri->scheme = sclone("https");
        }
        rx->parsedUri->port = conn->sock->listenSock->port;
        rx->parsedUri->host = rx->hostHeader ? rx->hostHeader : conn->host->name;

    } else if (!(100 <= rx->status && rx->status < 200)) {
        /* Clients have already created their Tx pipeline */
        httpCreateRxPipeline(conn, conn->http->clientRoute);
    }
    httpSetState(conn, HTTP_STATE_PARSED);
    if (conn->endpoint && !httpValidateLimits(conn->endpoint, HTTP_VALIDATE_OPEN_REQUEST, conn)) {
        return 0;
    }
    return 1;
}


static void mapMethod(HttpConn *conn)
{
    HttpRx      *rx;
    cchar       *method;

    rx = conn->rx;
    if (rx->flags & HTTP_POST) {
        if ((method = httpGetParam(conn, "-http-method-", 0)) != 0) {
            if (!scasematch(method, rx->method)) {
                mprLog(3, "Change method from %s to %s for %s", rx->method, method, rx->uri);
                httpSetMethod(conn, method);
            }
        }
    }
}


static void routeRequest(HttpConn *conn)
{
    HttpRx  *rx;

    mprAssert(conn->endpoint);

    rx = conn->rx;
    httpAddParams(conn);
    mapMethod(conn);
    httpRouteRequest(conn);  
    httpCreateRxPipeline(conn, rx->route);
    httpCreateTxPipeline(conn, rx->route);
}


/*
    Only called by parseRequestLine
 */
static void traceRequest(HttpConn *conn, HttpPacket *packet)
{
    MprBuf  *content;
    cchar   *endp, *ext, *cp;
    int     len, level;

    content = packet->content;
    ext = 0;

    /*
        Find the Uri extension:   "GET /path.ext HTTP/1.1"
     */
    if ((cp = schr(content->start, ' ')) != 0) {
        if ((cp = schr(++cp, ' ')) != 0) {
            for (ext = --cp; ext > content->start && *ext != '.'; ext--) ;
            ext = (*ext == '.') ? snclone(&ext[1], cp - ext) : 0;
            conn->tx->ext = ext;
        }
    }

    /*
        If tracing header, do entire header including first line
     */
    if ((conn->rx->traceLevel = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, ext)) >= 0) {
        mprLog(4, "New request from %s:%d to %s:%d", conn->ip, conn->port, conn->sock->acceptIp, conn->sock->acceptPort);
        endp = strstr((char*) content->start, "\r\n\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 4) : 0;
        httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, packet, len, 0);

    } else if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_FIRST, ext)) >= 0) {
        endp = strstr((char*) content->start, "\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 2) : 0;
        if (len > 0) {
            content->start[len - 2] = '\0';
            mprLog(level, "%s", content->start);
            content->start[len - 2] = '\r';
        }
    }
}


static void parseMethod(HttpConn *conn)
{
    HttpRx      *rx;
    cchar       *method;
    int         methodFlags;

    rx = conn->rx;
    method = rx->method;
    methodFlags = 0;

    rx->flags &= (HTTP_DELETE | HTTP_GET | HTTP_HEAD | HTTP_POST | HTTP_PUT | HTTP_TRACE | HTTP_UNKNOWN);

    switch (method[0]) {
    case 'D':
        if (strcmp(method, "DELETE") == 0) {
            methodFlags = HTTP_DELETE;
        }
        break;

    case 'G':
        if (strcmp(method, "GET") == 0) {
            methodFlags = HTTP_GET;
        }
        break;

    case 'H':
        if (strcmp(method, "HEAD") == 0) {
            methodFlags = HTTP_HEAD;
            httpOmitBody(conn);
        }
        break;

    case 'O':
        if (strcmp(method, "OPTIONS") == 0) {
            methodFlags = HTTP_OPTIONS;
        }
        break;

    case 'P':
        if (strcmp(method, "POST") == 0) {
            methodFlags = HTTP_POST;
            rx->needInputPipeline = 1;

        } else if (strcmp(method, "PUT") == 0) {
            methodFlags = HTTP_PUT;
            rx->needInputPipeline = 1;
        }
        break;

    case 'T':
        if (strcmp(method, "TRACE") == 0) {
            methodFlags = HTTP_TRACE;
        }
        break;
    }
    if (methodFlags == 0) {
        methodFlags = HTTP_UNKNOWN;
    }
    rx->flags |= methodFlags;
}


/*  
    Parse the first line of a http request. Return true if the first line parsed. This is only called once all the headers
    have been read and buffered. Requests look like: METHOD URL HTTP/1.X.
 */
static void parseRequestLine(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    char        *method, *uri, *protocol;

    rx = conn->rx;
#if BLD_DEBUG
    conn->startTime = conn->http->now;
    conn->startTicks = mprGetTicks();
#endif
    traceRequest(conn, packet);

    method = getToken(conn, " ");
    rx->originalMethod = rx->method = method = supper(method);
    parseMethod(conn);

    uri = getToken(conn, " ");
    if (*uri == '\0') {
        httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad HTTP request. Empty URI");
    } else if ((int) strlen(uri) >= conn->limits->uriSize) {
        httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_URL_TOO_LARGE, "Bad request. URI too long");
    }
    protocol = conn->protocol = supper(getToken(conn, "\r\n"));
    if (strcmp(protocol, "HTTP/1.0") == 0) {
        if (rx->flags & (HTTP_POST|HTTP_PUT)) {
            rx->remainingContent = MAXINT;
            rx->needInputPipeline = 1;
        }
        conn->http10 = 1;
        conn->protocol = protocol;
    } else if (strcmp(protocol, "HTTP/1.1") == 0) {
        conn->protocol = protocol;
    } else {
        conn->protocol = sclone("HTTP/1.1");
        httpError(conn, HTTP_CLOSE | HTTP_CODE_NOT_ACCEPTABLE, "Unsupported HTTP protocol");
    }
    rx->originalUri = rx->uri = sclone(uri);
    httpSetState(conn, HTTP_STATE_FIRST);
}


/*  
    Parse the first line of a http response. Return true if the first line parsed. This is only called once all the headers
    have been read and buffered. Response status lines look like: HTTP/1.X CODE Message
 */
static void parseResponseLine(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprBuf      *content;
    cchar       *endp;
    char        *protocol, *status;
    int         len, level, traced;

    rx = conn->rx;
    tx = conn->tx;
    traced = 0;

    if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, tx->ext) >= 0) {
        content = packet->content;
        endp = strstr((char*) content->start, "\r\n\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 4) : 0;
        httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, packet, len, 0);
        traced = 1;
    }
    protocol = conn->protocol = supper(getToken(conn, " "));
    if (strcmp(protocol, "HTTP/1.0") == 0) {
        conn->http10 = 1;
    } else if (strcmp(protocol, "HTTP/1.1") != 0) {
        httpError(conn, HTTP_CLOSE | HTTP_CODE_NOT_ACCEPTABLE, "Unsupported HTTP protocol");
    }
    status = getToken(conn, " ");
    if (*status == '\0') {
        httpError(conn, HTTP_CLOSE | HTTP_CODE_NOT_ACCEPTABLE, "Bad response status code");
    }
    rx->status = atoi(status);
    rx->statusMessage = sclone(getToken(conn, "\r\n"));

    if (slen(rx->statusMessage) >= conn->limits->uriSize) {
        httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_URL_TOO_LARGE, "Bad response. Status message too long");
    }
    if (!traced && (level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_FIRST, tx->ext)) >= 0) {
        mprLog(level, "%s %d %s", protocol, rx->status, rx->statusMessage);
    }
}


/*  
    Parse the request headers. Return true if the header parsed.
 */
static void parseHeaders(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLimits  *limits;
    MprBuf      *content;
    char        *key, *value, *tok, *hvalue;
    cchar       *oldValue;
    int         count, keepAlive;

    rx = conn->rx;
    tx = conn->tx;
    content = packet->content;
    conn->rx->headerPacket = packet;
    limits = conn->limits;
    keepAlive = (conn->http10) ? 0 : 1;

    for (count = 0; content->start[0] != '\r' && !conn->connError; count++) {
        if (count >= limits->headerCount) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Too many headers");
            break;
        }
        if ((key = getToken(conn, ":")) == 0 || *key == '\0') {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad header format");
            break;
        }
        value = getToken(conn, "\r\n");
        while (isspace((int) *value)) {
            value++;
        }
        LOG(8, "Key %s, value %s", key, value);
        if (strspn(key, "%<>/\\") > 0) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad header key value");
        }
        if ((oldValue = mprLookupKey(rx->headers, key)) != 0) {
            hvalue = sfmt("%s, %s", oldValue, value);
        } else {
            hvalue = sclone(value);
        }
        mprAddKey(rx->headers, key, hvalue);

        switch (tolower((int) key[0])) {
        case 'a':
            if (strcasecmp(key, "authorization") == 0) {
                value = sclone(value);
                rx->authType = stok(value, " \t", &tok);
                rx->authDetails = sclone(tok);

            } else if (strcasecmp(key, "accept-charset") == 0) {
                rx->acceptCharset = sclone(value);

            } else if (strcasecmp(key, "accept") == 0) {
                rx->accept = sclone(value);

            } else if (strcasecmp(key, "accept-encoding") == 0) {
                rx->acceptEncoding = sclone(value);

            } else if (strcasecmp(key, "accept-language") == 0) {
                rx->acceptLanguage = sclone(value);
            }
            break;

        case 'c':
            if (strcasecmp(key, "connection") == 0) {
                rx->connection = sclone(value);
                if (scasecmp(value, "KEEP-ALIVE") == 0) {
                    keepAlive = 1;
                } else if (scasecmp(value, "CLOSE") == 0) {
                    /*  Not really required, but set to 0 to be sure */
                    conn->keepAliveCount = 0;
                }

            } else if (strcasecmp(key, "content-length") == 0) {
                if (rx->length >= 0) {
                    httpError(conn, HTTP_CLOSE | HTTP_CODE_BAD_REQUEST, "Mulitple content length headers");
                    break;
                }
                rx->length = stoi(value);
                if (rx->length < 0) {
                    httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad content length");
                    break;
                }
                if (rx->length >= conn->limits->receiveBodySize) {
                    httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE,
                        "Request content length %,Ld bytes is too big. Limit %,Ld", 
                        rx->length, conn->limits->receiveBodySize);
                    break;
                }
                rx->contentLength = sclone(value);
                mprAssert(rx->length >= 0);
                if (conn->endpoint || strcasecmp(tx->method, "HEAD") != 0) {
                    rx->remainingContent = rx->length;
                    rx->needInputPipeline = 1;
                }

            } else if (strcasecmp(key, "content-range") == 0) {
                /*
                    This headers specifies the range of any posted body data
                    Format is:  Content-Range: bytes n1-n2/length
                    Where n1 is first byte pos and n2 is last byte pos
                 */
                char    *sp;
                MprOff  start, end, size;

                start = end = size = -1;
                sp = value;
                while (*sp && !isdigit((int) *sp)) {
                    sp++;
                }
                if (*sp) {
                    start = stoi(sp);
                    if ((sp = strchr(sp, '-')) != 0) {
                        end = stoi(++sp);
                    }
                    if ((sp = strchr(sp, '/')) != 0) {
                        /*
                            Note this is not the content length transmitted, but the original size of the input of which
                            the client is transmitting only a portion.
                         */
                        size = stoi(++sp);
                    }
                }
                if (start < 0 || end < 0 || size < 0 || end <= start) {
                    httpError(conn, HTTP_CLOSE | HTTP_CODE_RANGE_NOT_SATISFIABLE, "Bad content range");
                    break;
                }
                rx->inputRange = httpCreateRange(conn, start, end);

            } else if (strcasecmp(key, "content-type") == 0) {
                rx->mimeType = sclone(value);
                rx->form = (rx->flags & HTTP_POST) && scontains(rx->mimeType, "application/x-www-form-urlencoded", -1);
                rx->upload = (rx->flags & HTTP_POST) && scontains(rx->mimeType, "multipart/form-data", -1);

            } else if (strcasecmp(key, "cookie") == 0) {
                if (rx->cookie && *rx->cookie) {
                    rx->cookie = sjoin(rx->cookie, "; ", value, NULL);
                } else {
                    rx->cookie = sclone(value);
                }
            }
            break;

        case 'h':
            if (strcasecmp(key, "host") == 0) {
                rx->hostHeader = sclone(value);
            }
            break;

        case 'i':
            if ((strcasecmp(key, "if-modified-since") == 0) || (strcasecmp(key, "if-unmodified-since") == 0)) {
                MprTime     newDate = 0;
                char        *cp;
                bool        ifModified = (key[3] == 'm');

                if ((cp = strchr(value, ';')) != 0) {
                    *cp = '\0';
                }
                if (mprParseTime(&newDate, value, MPR_UTC_TIMEZONE, NULL) < 0) {
                    mprAssert(0);
                    break;
                }
                if (newDate) {
                    rx->since = newDate;
                    rx->ifModified = ifModified;
                    rx->flags |= HTTP_IF_MODIFIED;
                }

            } else if ((strcasecmp(key, "if-match") == 0) || (strcasecmp(key, "if-none-match") == 0)) {
                char    *word, *tok;
                bool    ifMatch = key[3] == 'm';

                if ((tok = strchr(value, ';')) != 0) {
                    *tok = '\0';
                }
                rx->ifMatch = ifMatch;
                rx->flags |= HTTP_IF_MODIFIED;
                value = sclone(value);
                word = stok(value, " ,", &tok);
                while (word) {
                    addMatchEtag(conn, word);
                    word = stok(0, " ,", &tok);
                }

            } else if (strcasecmp(key, "if-range") == 0) {
                char    *word, *tok;

                if ((tok = strchr(value, ';')) != 0) {
                    *tok = '\0';
                }
                rx->ifMatch = 1;
                rx->flags |= HTTP_IF_MODIFIED;
                value = sclone(value);
                word = stok(value, " ,", &tok);
                while (word) {
                    addMatchEtag(conn, word);
                    word = stok(0, " ,", &tok);
                }
            }
            break;

        case 'k':
            /* Keep-Alive: timeout=N, max=1 */
            if (strcasecmp(key, "keep-alive") == 0) {
                keepAlive = 1;
                if ((tok = scontains(value, "max=", -1)) != 0) {
                    conn->keepAliveCount = atoi(&tok[4]);
                    /*  
                        IMPORTANT: Deliberately close the connection one request early. This ensures a client-led 
                        termination and helps relieve server-side TIME_WAIT conditions.
                     */
                    if (conn->keepAliveCount == 1) {
                        conn->keepAliveCount = 0;
                    }
                }
            }
            break;                
                
        case 'l':
            if (strcasecmp(key, "location") == 0) {
                rx->redirect = sclone(value);
            }
            break;

        case 'p':
            if (strcasecmp(key, "pragma") == 0) {
                rx->pragma = sclone(value);
            }
            break;

        case 'r':
            if (strcasecmp(key, "range") == 0) {
                if (!parseRange(conn, value)) {
                    httpError(conn, HTTP_CLOSE | HTTP_CODE_RANGE_NOT_SATISFIABLE, "Bad range");
                }
            } else if (strcasecmp(key, "referer") == 0) {
                /* NOTE: yes the header is misspelt in the spec */
                rx->referrer = sclone(value);
            }
            break;

        case 't':
            if (strcasecmp(key, "transfer-encoding") == 0) {
                if (scasecmp(value, "chunked") == 0) {
                    /*  
                        remainingContent will be revised by the chunk filter as chunks are processed and will 
                        be set to zero when the last chunk has been received.
                     */
                    rx->flags |= HTTP_CHUNKED;
                    rx->chunkState = HTTP_CHUNK_START;
                    rx->remainingContent = MAXINT;
                    rx->needInputPipeline = 1;
                }
            }
            break;

        case 'x':
            if (strcasecmp(key, "x-http-method-override") == 0) {
                httpSetMethod(conn, value);
#if BLD_DEBUG
            } else if (strcasecmp(key, "x-chunk-size") == 0) {
                tx->chunkSize = atoi(value);
                if (tx->chunkSize <= 0) {
                    tx->chunkSize = 0;
                } else if (tx->chunkSize > conn->limits->chunkSize) {
                    tx->chunkSize = conn->limits->chunkSize;
                }
#endif
            }
            break;

        case 'u':
            if (strcasecmp(key, "user-agent") == 0) {
                rx->userAgent = sclone(value);
            }
            break;

        case 'w':
            if (strcasecmp(key, "www-authenticate") == 0) {
                conn->authType = value;
                while (*value && !isspace((int) *value)) {
                    value++;
                }
                *value++ = '\0';
                conn->authType = slower(conn->authType);
                // MOB - move this into the auth filter
                if (!parseAuthenticate(conn, value)) {
                    httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad Authentication header");
                    break;
                }
            }
            break;
        }
    }
    /*
        Don't stream input if a form or upload. NOTE: Upload needs the Files[] collection.
     */
    rx->streamInput = !(rx->form || rx->upload);
    if (!keepAlive) {
        conn->keepAliveCount = 0;
    }
    if (!(rx->flags & HTTP_CHUNKED)) {
        /*  
            Step over "\r\n" after headers. 
            Don't do this if chunked so chunking can parse a single chunk delimiter of "\r\nSIZE ...\r\n"
         */
        if (httpGetPacketLength(packet) >= 2) {
            mprAdjustBufStart(content, 2);
        }
    }
}


/*  
    Parse an authentication response (client side only)
    MOB - move this into the auth filter
 */
static bool parseAuthenticate(HttpConn *conn, char *authDetails)
{
    HttpRx  *rx;
    char    *value, *tok, *key, *dp, *sp;
    int     seenComma;

    rx = conn->rx;
    key = (char*) authDetails;

    while (*key) {
        while (*key && isspace((int) *key)) {
            key++;
        }
        tok = key;
        while (*tok && !isspace((int) *tok) && *tok != ',' && *tok != '=') {
            tok++;
        }
        *tok++ = '\0';

        while (isspace((int) *tok)) {
            tok++;
        }
        seenComma = 0;
        if (*tok == '\"') {
            value = ++tok;
            while (*tok != '\"' && *tok != '\0') {
                tok++;
            }
        } else {
            value = tok;
            while (*tok != ',' && *tok != '\0') {
                tok++;
            }
            seenComma++;
        }
        *tok++ = '\0';

        /*
            Handle back-quoting
         */
        if (strchr(value, '\\')) {
            for (dp = sp = value; *sp; sp++) {
                if (*sp == '\\') {
                    sp++;
                }
                *dp++ = *sp++;
            }
            *dp = '\0';
        }

        /*
            algorithm, domain, nonce, oqaque, realm, qop, stale
            We don't strdup any of the values as the headers are persistently saved.
         */
        switch (tolower((int) *key)) {
        case 'a':
            if (scasecmp(key, "algorithm") == 0) {
                rx->authAlgorithm = sclone(value);
                break;
            }
            break;

        case 'd':
            if (scasecmp(key, "domain") == 0) {
                conn->authDomain = sclone(value);
                break;
            }
            break;

        case 'n':
            if (scasecmp(key, "nonce") == 0) {
                conn->authNonce = sclone(value);
                conn->authNc = 0;
            }
            break;

        case 'o':
            if (scasecmp(key, "opaque") == 0) {
                conn->authOpaque = sclone(value);
            }
            break;

        case 'q':
            if (scasecmp(key, "qop") == 0) {
                conn->authQop = sclone(value);
            }
            break;

        case 'r':
            if (scasecmp(key, "realm") == 0) {
                conn->authRealm = sclone(value);
            }
            break;

        case 's':
            if (scasecmp(key, "stale") == 0) {
                rx->authStale = sclone(value);
                break;
            }

        default:
            /*  For upward compatibility --  ignore keywords we don't understand */
            ;
        }
        key = tok;
        if (!seenComma) {
            while (*key && *key != ',') {
                key++;
            }
            if (*key) {
                key++;
            }
        }
    }
    if (strcmp(rx->conn->authType, "basic") == 0) {
        if (conn->authRealm == 0) {
            return 0;
        }
        return 1;
    }
    /* Digest */
    if (conn->authRealm == 0 || conn->authNonce == 0) {
        return 0;
    }
    if (conn->authQop) {
        if (conn->authDomain == 0 || conn->authOpaque == 0 || rx->authAlgorithm == 0 || rx->authStale == 0) {
            return 0;
        }
    }
    return 1;
}


/*
    Called once the HTTP request/response headers have been parsed
 */
static bool processParsed(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (!rx->form && conn->endpoint) {
        /*
            Routes need to be able to access form data, so forms route later after all input is received.
         */
        routeRequest(conn);
    }
    if (rx->streamInput) {
        httpStartPipeline(conn);
    }
    httpSetState(conn, HTTP_STATE_CONTENT);
    if (conn->workerEvent && conn->tx->started && rx->eof) {
        httpSetState(conn, HTTP_STATE_RUNNING);
        return 0;
    }
    return 1;
}


static bool analyseContent(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpQueue   *q;
    MprBuf      *content;
    ssize       nbytes;

    rx = conn->rx;
    tx = conn->tx;
    content = packet->content;
    q = tx->queue[HTTP_QUEUE_RX];
    mprAssert(httpVerifyQueue(q));

    LOG(7, "processContent: packet of %d bytes, remaining %d", mprGetBufLength(content), rx->remainingContent);
    if ((nbytes = httpFilterChunkData(q, packet)) < 0) {
        return 0;
    }
    mprAssert(nbytes >= 0);

    if (nbytes > 0) {
        rx->remainingContent -= nbytes;
        rx->bytesRead += nbytes;
        if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_BODY, tx->ext) >= 0) {
            httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_BODY, packet, nbytes, rx->bytesRead);
        }
        if (rx->bytesRead >= conn->limits->receiveBodySize) {
            httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_TOO_LARGE, 
                "Request body of %,Ld bytes is too big. Limit %,Ld", rx->bytesRead, conn->limits->receiveBodySize);
            return 1;
        }
        if (rx->form && rx->length >= conn->limits->receiveFormSize) {
            httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_TOO_LARGE, 
                "Request form of %,Ld bytes is too big. Limit %,Ld", rx->bytesRead, conn->limits->receiveFormSize);
            return 1;
        }
        if (packet == rx->headerPacket && nbytes > 0) {
            packet = httpSplitPacket(packet, 0);
        }
        if (httpGetPacketLength(packet) > nbytes) {
            /*  Split excess data belonging to the next chunk or pipelined request */
            LOG(7, "processContent: Split packet of %d at %d", httpGetPacketLength(packet), nbytes);
            conn->input = httpSplitPacket(packet, nbytes);
        } else {
            conn->input = 0;
        }
        if (!(conn->finalized && conn->endpoint)) {
            if (rx->form) {
                httpPutForService(q, packet, HTTP_DELAY_SERVICE);
            } else {
                httpPutPacketToNext(q, packet);
            }
            mprAssert(httpVerifyQueue(q));
        }
    }
    if (rx->remainingContent == 0 && !(rx->flags & HTTP_CHUNKED)) {
        rx->eof = 1;
    }
    return 1;
}


/*  
    Process request body data (typically post or put content)
 */
static bool processContent(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpQueue   *q;

    rx = conn->rx;

    if (packet == 0 || !analyseContent(conn, packet)) {
        return 0;
    }
    if (rx->eof) {
        q = conn->tx->queue[HTTP_QUEUE_RX];
        if (rx->form && conn->endpoint) {
            routeRequest(conn);
            while ((packet = httpGetPacket(q)) != 0) {
                httpPutPacketToNext(q, packet);
            }
        }
        httpPutPacketToNext(q, httpCreateEndPacket());
        if (!rx->streamInput) {
            httpStartPipeline(conn);
        }
        httpSetState(conn, HTTP_STATE_RUNNING);
        return conn->workerEvent ? 0 : 1;
    }
    httpServiceQueues(conn);
    if (conn->connError) {
        httpSetState(conn, HTTP_STATE_RUNNING);
    }
    return conn->error || (conn->input ? httpGetPacketLength(conn->input) : 0);
}


/*
    In the running state after all content has been received
    Note: may be called multiple times
 */
static bool processRunning(HttpConn *conn)
{
    int     canProceed;

    //  MOB - refactor
    canProceed = 1;
    if (conn->connError) {
        httpSetState(conn, HTTP_STATE_COMPLETE);
    } else {
        if (conn->endpoint) {
            httpProcessPipeline(conn);
            if (conn->connError || conn->connectorComplete) {
                httpSetState(conn, HTTP_STATE_COMPLETE);
            } else {
                httpWritable(conn);
                canProceed = httpServiceQueues(conn);
            }
        } else {
            httpServiceQueues(conn);
            httpFinalize(conn);
            httpSetState(conn, HTTP_STATE_COMPLETE);
        }
    }
    return canProceed;
}


#if BLD_DEBUG
static void measure(HttpConn *conn)
{
    MprTime     elapsed;
    HttpTx      *tx;
    cchar       *uri;
    int         level;

    tx = conn->tx;
    if (conn->rx == 0 || tx == 0) {
        return;
    }
    uri = (conn->endpoint) ? conn->rx->uri : tx->parsedUri->path;
   
    if ((level = httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_TIME, tx->ext)) >= 0) {
        elapsed = mprGetTime() - conn->startTime;
#if MPR_HIGH_RES_TIMER
        if (elapsed < 1000) {
            mprLog(level, "TIME: Request %s took %,d msec %,d ticks", uri, elapsed, mprGetTicks() - conn->startTicks);
        } else
#endif
            mprLog(level, "TIME: Request %s took %,d msec", uri, elapsed);
    }
}
#else
#define measure(conn)
#endif


static bool processCompletion(HttpConn *conn)
{
    HttpPacket  *packet;
    HttpRx      *rx;
    bool        more;

    rx = conn->rx;
    mprAssert(conn->state == HTTP_STATE_COMPLETE);

    httpDestroyPipeline(conn);
    measure(conn);
    if (conn->endpoint && rx) {
        if (rx->route->log) {
            httpLogRequest(conn);
        }
        rx->conn = 0;
        conn->tx->conn = 0;
        conn->rx = 0;
        conn->tx = 0;
        packet = conn->input;
        more = packet && !conn->connError && (httpGetPacketLength(packet) > 0);
        httpValidateLimits(conn->endpoint, HTTP_VALIDATE_CLOSE_REQUEST, conn);
        httpPrepServerConn(conn);
        return more;
    }
    return 0;
}


void httpCloseRx(HttpConn *conn)
{
    if (!conn->rx->eof) {
        /* May not have consumed all read data, so can't be assured the next request will be okay */
        conn->keepAliveCount = -1;
    }
    if (conn->state < HTTP_STATE_COMPLETE && !conn->advancing) {
        httpProcess(conn, NULL);
    }
}


bool httpContentNotModified(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprTime     modified;
    bool        same;

    rx = conn->rx;
    tx = conn->tx;

    if (rx->flags & HTTP_IF_MODIFIED) {
        /*  
            If both checks, the last modification time and etag, claim that the request doesn't need to be
            performed, skip the transfer.
         */
        mprAssert(tx->fileInfo.valid);
        modified = (MprTime) tx->fileInfo.mtime * MPR_TICKS_PER_SEC;
        same = httpMatchModified(conn, modified) && httpMatchEtag(conn, tx->etag);
        if (tx->outputRanges && !same) {
            tx->outputRanges = 0;
        }
        return same;
    }
    return 0;
}


HttpRange *httpCreateRange(HttpConn *conn, MprOff start, MprOff end)
{
    HttpRange     *range;

    if ((range = mprAllocObj(HttpRange, manageRange)) == 0) {
        return 0;
    }
    range->start = start;
    range->end = end;
    range->len = end - start;
    return range;
}


static void manageRange(HttpRange *range, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(range->next);
    }
}


MprOff httpGetContentLength(HttpConn *conn)
{
    if (conn->rx == 0) {
        mprAssert(conn->rx);
        return 0;
    }
    return conn->rx->length;
}


cchar *httpGetCookies(HttpConn *conn)
{
    if (conn->rx == 0) {
        mprAssert(conn->rx);
        return 0;
    }
    return conn->rx->cookie;
}


cchar *httpGetHeader(HttpConn *conn, cchar *key)
{
    if (conn->rx == 0) {
        mprAssert(conn->rx);
        return 0;
    }
    return mprLookupKey(conn->rx->headers, slower(key));
}


char *httpGetHeadersFromHash(MprHash *hash)
{
    MprKey      *kp;
    char        *headers, *cp;
    ssize       len;

    for (len = 0, kp = 0; (kp = mprGetNextKey(hash, kp)) != 0; ) {
        len += strlen(kp->key) + 2 + strlen(kp->data) + 1;
    }
    if ((headers = mprAlloc(len + 1)) == 0) {
        return 0;
    }
    for (kp = 0, cp = headers; (kp = mprGetNextKey(hash, kp)) != 0; ) {
        strcpy(cp, kp->key);
        cp += strlen(cp);
        *cp++ = ':';
        *cp++ = ' ';
        strcpy(cp, kp->data);
        cp += strlen(cp);
        *cp++ = '\n';
    }
    *cp = '\0';
    return headers;
}


char *httpGetHeaders(HttpConn *conn)
{
    return httpGetHeadersFromHash(conn->rx->headers);
}


MprHash *httpGetHeaderHash(HttpConn *conn)
{
    if (conn->rx == 0) {
        mprAssert(conn->rx);
        return 0;
    }
    return conn->rx->headers;
}


cchar *httpGetQueryString(HttpConn *conn)
{
    return (conn->rx && conn->rx->parsedUri) ? conn->rx->parsedUri->query : 0;
}


int httpGetStatus(HttpConn *conn)
{
    return (conn->rx) ? conn->rx->status : 0;
}


char *httpGetStatusMessage(HttpConn *conn)
{
    return (conn->rx) ? conn->rx->statusMessage : 0;
}


void httpSetMethod(HttpConn *conn, cchar *method)
{
    conn->rx->method = sclone(method);
    parseMethod(conn);
}


int httpSetUri(HttpConn *conn, cchar *uri, cchar *query)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpUri     *prior;

    rx = conn->rx;
    tx = conn->tx;
    prior = rx->parsedUri;

    if ((rx->parsedUri = httpCreateUri(uri, 0)) == 0) {
        return MPR_ERR_BAD_ARGS;
    }
    if (prior) {
        if (rx->parsedUri->scheme == 0) {
            rx->parsedUri->scheme = prior->scheme;
        }
        if (rx->parsedUri->port == 0) {
            rx->parsedUri->port = prior->port;
        }
    }
    if (query == 0 && prior) {
        rx->parsedUri->query = prior->query;
        rx->parsedUri->host = prior->host;
    } else if (*query) {
        rx->parsedUri->query = sclone(query);
    }
    /*
        Start out with no scriptName and the entire URI in the pathInfo. Stages may rewrite.
     */
    rx->uri = rx->parsedUri->path;
    rx->pathInfo = httpNormalizeUriPath(mprUriDecode(rx->parsedUri->path));
    rx->scriptName = mprEmptyString();
    tx->ext = httpGetExt(conn);
    return 0;
}


static void waitHandler(HttpConn *conn, struct MprEvent *event)
{
    httpCallEvent(conn, event->mask);
    mprSignalDispatcher(conn->dispatcher);
}


/*
    Wait for an I/O event. There are two modes:
      - Wait for just one event (set state to zero)
      - Wait until the given state is achieved
    If timeout is zero, then wait forever. If set to < 0, then use default inactivity and duration timeouts. 
    This routine will process buffered input.
 */
int httpWait(HttpConn *conn, int state, MprTime timeout)
{
    MprTime     mark, remaining, inactivityTimeout;
    int         eventMask, saveAsync, justOne, workDone;

    if (state == 0) {
        state = HTTP_STATE_COMPLETE;
        justOne = 1;
    } else {
        justOne = 0;
    }
    if (conn->state <= HTTP_STATE_BEGIN) {
        mprAssert(conn->state >= HTTP_STATE_BEGIN);
        return MPR_ERR_BAD_STATE;
    } 
    if (conn->input && httpGetPacketLength(conn->input) > 0) {
        httpProcess(conn, conn->input);
    }
    if (conn->error) {
        return MPR_ERR_BAD_STATE;
    }
    mark = mprGetTime();
    if (mprGetDebugMode()) {
        inactivityTimeout = timeout = MPR_MAX_TIMEOUT;
    } else {
        inactivityTimeout = timeout < 0 ? conn->limits->inactivityTimeout : MPR_MAX_TIMEOUT;
        if (timeout < 0) {
            timeout = conn->limits->requestTimeout;
        }
    }
    saveAsync = conn->async;
    conn->async = 1;

    eventMask = MPR_READABLE;
    if (!conn->connectorComplete) {
        eventMask |= MPR_WRITABLE;
    }
    if (conn->waitHandler == 0) {
        conn->waitHandler = mprCreateWaitHandler(conn->sock->fd, eventMask, conn->dispatcher, waitHandler, conn, 0);
    } else {
        mprWaitOn(conn->waitHandler, eventMask);
    }
    remaining = timeout;
    do {
        workDone = httpServiceQueues(conn);
        if (conn->state < state) {
            mprWaitForEvent(conn->dispatcher, min(inactivityTimeout, remaining));
        }
        if (conn->sock && mprIsSocketEof(conn->sock) && !workDone) {
            break;
        }
        remaining = mprGetRemainingTime(mark, timeout);
    } while (!justOne && !conn->error && conn->state < state && remaining > 0);

    conn->async = saveAsync;
    if (conn->sock == 0 || conn->error) {
        return MPR_ERR_CANT_CONNECT;
    }
    if (!justOne && conn->state < state) {
        return (remaining <= 0) ? MPR_ERR_TIMEOUT : MPR_ERR_CANT_READ;
    }
    return 0;
}


/*  
    Set the connector as write blocked and can't proceed.
 */
void httpSetWriteBlocked(HttpConn *conn)
{
    mprLog(6, "Write Blocked");
    conn->canProceed = 0;
    conn->writeBlocked = 1;
}


static void addMatchEtag(HttpConn *conn, char *etag)
{
    HttpRx   *rx;

    rx = conn->rx;
    if (rx->etags == 0) {
        rx->etags = mprCreateList(-1, 0);
    }
    mprAddItem(rx->etags, etag);
}


/*  
    Get the next input token. The content buffer is advanced to the next token. This routine always returns a
    non-zero token. The empty string means the delimiter was not found. The delimiter is a string to match and not
    a set of characters. HTTP header header parsing does not work as well using classical strtok parsing as you must
    know when the "/r/n/r/n" body delimiter has been encountered. Strtok will eat such delimiters.
 */
static char *getToken(HttpConn *conn, cchar *delim)
{
    MprBuf  *buf;
    char    *token, *nextToken;
    int     len;

    buf = conn->input->content;
    token = mprGetBufStart(buf);
    nextToken = scontains(mprGetBufStart(buf), delim, mprGetBufLength(buf));
    if (nextToken) {
        *nextToken = '\0';
        len = (int) strlen(delim);
        nextToken += len;
        buf->start = nextToken;
    } else {
        buf->start = mprGetBufEnd(buf);
    }
    return token;
}


/*  
    Match the entity's etag with the client's provided etag.
 */
bool httpMatchEtag(HttpConn *conn, char *requestedEtag)
{
    HttpRx  *rx;
    char    *tag;
    int     next;

    rx = conn->rx;
    if (rx->etags == 0) {
        return 1;
    }
    if (requestedEtag == 0) {
        return 0;
    }
    for (next = 0; (tag = mprGetNextItem(rx->etags, &next)) != 0; ) {
        if (strcmp(tag, requestedEtag) == 0) {
            return (rx->ifMatch) ? 0 : 1;
        }
    }
    return (rx->ifMatch) ? 1 : 0;
}


/*  
    If an IF-MODIFIED-SINCE was specified, then return true if the resource has not been modified. If using
    IF-UNMODIFIED, then return true if the resource was modified.
 */
bool httpMatchModified(HttpConn *conn, MprTime time)
{
    HttpRx   *rx;

    rx = conn->rx;

    if (rx->since == 0) {
        /*  If-Modified or UnModified not supplied. */
        return 1;
    }
    if (rx->ifModified) {
        /*  Return true if the file has not been modified.  */
        return !(time > rx->since);
    } else {
        /*  Return true if the file has been modified.  */
        return (time > rx->since);
    }
}


/*  
    Format is:  Range: bytes=n1-n2,n3-n4,...
    Where n1 is first byte pos and n2 is last byte pos

    Examples:
        Range: 0-49             first 50 bytes
        Range: 50-99,200-249    Two 50 byte ranges from 50 and 200
        Range: -50              Last 50 bytes
        Range: 1-               Skip first byte then emit the rest

    Return 1 if more ranges, 0 if end of ranges, -1 if bad range.
 */
static bool parseRange(HttpConn *conn, char *value)
{
    HttpTx      *tx;
    HttpRange   *range, *last, *next;
    char        *tok, *ep;

    tx = conn->tx;
    value = sclone(value);
    if (value == 0) {
        return 0;
    }
    /*  
        Step over the "bytes="
     */
    stok(value, "=", &value);

    for (last = 0; value && *value; ) {
        if ((range = mprAllocObj(HttpRange, manageRange)) == 0) {
            return 0;
        }
        /*  
            A range "-7" will set the start to -1 and end to 8
         */
        tok = stok(value, ",", &value);
        if (*tok != '-') {
            range->start = (ssize) stoi(tok);
        } else {
            range->start = -1;
        }
        range->end = -1;

        if ((ep = strchr(tok, '-')) != 0) {
            if (*++ep != '\0') {
                /*
                    End is one beyond the range. Makes the math easier.
                 */
                range->end = (ssize) stoi(ep) + 1;
            }
        }
        if (range->start >= 0 && range->end >= 0) {
            range->len = (int) (range->end - range->start);
        }
        if (last == 0) {
            tx->outputRanges = range;
        } else {
            last->next = range;
        }
        last = range;
    }

    /*  
        Validate ranges
     */
    for (range = tx->outputRanges; range; range = range->next) {
        if (range->end != -1 && range->start >= range->end) {
            return 0;
        }
        if (range->start < 0 && range->end < 0) {
            return 0;
        }
        next = range->next;
        if (range->start < 0 && next) {
            /* This range goes to the end, so can't have another range afterwards */
            return 0;
        }
        if (next) {
            if (range->end < 0) {
                return 0;
            }
            if (next->start >= 0 && range->end > next->start) {
                return 0;
            }
        }
    }
    conn->tx->currentRange = tx->outputRanges;
    return (last) ? 1: 0;
}


void httpSetStageData(HttpConn *conn, cchar *key, cvoid *data)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->requestData == 0) {
        rx->requestData = mprCreateHash(-1, 0);
    }
    mprAddKey(rx->requestData, key, data);
}


cvoid *httpGetStageData(HttpConn *conn, cchar *key)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->requestData == 0) {
        return NULL;
    }
    return mprLookupKey(rx->requestData, key);
}


char *httpGetPathExt(cchar *path)
{
    char    *ep, *ext;

    if ((ext = strrchr(path, '.')) != 0) {
        ext = sclone(++ext);
        for (ep = ext; *ep && isalnum((int)*ep); ep++) {
            ;
        }
        *ep = '\0';
    }
    return ext;
}


/*
    Get the request extension. Look first at the URI pathInfo. If no extension, look at the filename if defined.
    Return NULL if no extension.
 */
char *httpGetExt(HttpConn *conn)
{
    HttpRx  *rx;
    char    *ext;

    rx = conn->rx;
    if ((ext = httpGetPathExt(rx->pathInfo)) == 0) {
        if (conn->tx->filename) {
            ext = httpGetPathExt(conn->tx->filename);
        }
    }
    return ext;
}


static int compareLang(char **s1, char **s2)
{
    return scmp(*s1, *s2);
}


HttpLang *httpGetLanguage(HttpConn *conn, MprHash *spoken, cchar *defaultLang)
{
    HttpRx      *rx;
    HttpLang    *lang;
    MprList     *list;
    cchar       *accept;
    char        *nextTok, *tok, *quality, *language;
    int         next;

    rx = conn->rx;
    if (rx->lang) {
        return rx->lang;
    }
    if (spoken == 0) {
        return 0;
    }
    list = mprCreateList(-1, 0);
    if ((accept = httpGetHeader(conn, "Accept-Language")) != 0) {
        for (tok = stok(sclone(accept), ",", &nextTok); tok; tok = stok(nextTok, ",", &nextTok)) {
            language = stok(tok, ";", &quality);
            if (quality == 0) {
                quality = "1";
            }
            mprAddItem(list, sfmt("%03d %s", (int) (atof(quality) * 100), language));
        }
        mprSortList(list, compareLang);
        for (next = 0; (language = mprGetNextItem(list, &next)) != 0; ) {
            if ((lang = mprLookupKey(rx->route->languages, &language[4])) != 0) {
                rx->lang = lang;
                return lang;
            }
        }
    }
    if (defaultLang && (lang = mprLookupKey(rx->route->languages, defaultLang)) != 0) {
        rx->lang = lang;
        return lang;
    }
    return 0;
}


/*
    Trim extra path information after the uri extension. This is used by CGI and PHP only. The strategy is to 
    heuristically find the script name in the uri. This is assumed to be the original uri up to and including 
    first path component containing a "." Any path information after that is regarded as extra path.
    WARNING: Extra path is an old, unreliable, CGI specific technique. Do not use directories with embedded periods.
 */
void httpTrimExtraPath(HttpConn *conn)
{
    HttpRx      *rx;
    char        *cp, *extra;
    ssize       len;

    rx = conn->rx;
    if (!(rx->flags & (HTTP_OPTIONS | HTTP_TRACE))) { 
        if ((cp = strchr(rx->pathInfo, '.')) != 0 && (extra = strchr(cp, '/')) != 0) {
            len = extra - rx->pathInfo;
            if (0 < len && len < slen(rx->pathInfo)) {
                rx->extraPath = sclone(&rx->pathInfo[len]);
                rx->pathInfo[len] = '\0';
            }
        }
        if ((cp = strchr(rx->target, '.')) != 0 && (extra = strchr(cp, '/')) != 0) {
            len = extra - rx->target;
            if (0 < len && len < slen(rx->target)) {
                rx->target[len] = '\0';
            }
        }
    }
}


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
/************************************************************************/
/*
 *  End of file "./src/rx.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/sendConnector.c"
 */
/************************************************************************/

/*
    sendConnector.c -- Send file connector. 

    The Sendfile connector supports the optimized transmission of whole static files. It uses operating system 
    sendfile APIs to eliminate reading the document into user space and multiple socket writes. The send connector 
    is not a general purpose connector. It cannot handle dynamic data or ranged requests. It does support chunked requests.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if !BLD_FEATURE_ROMFS

static void addPacketForSend(HttpQueue *q, HttpPacket *packet);
static void adjustSendVec(HttpQueue *q, MprOff written);
static MprOff buildSendVec(HttpQueue *q);
static void adjustPacketData(HttpQueue *q, MprOff written);


int httpOpenSendConnector(Http *http)
{
    HttpStage     *stage;

    mprLog(5, "Open send connector");
    if ((stage = httpCreateConnector(http, "sendConnector", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->open = httpSendOpen;
    stage->close = httpSendClose;
    stage->outgoingService = httpSendOutgoingService; 
    http->sendConnector = stage;
    return 0;
}


/*  
    Initialize the send connector for a request
 */
void httpSendOpen(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;

    if (tx->connector != conn->http->sendConnector) {
        httpAssignQueue(q, tx->connector, HTTP_QUEUE_TX);
        tx->connector->open(q);
        return;
    }
    if (!(tx->flags & HTTP_TX_NO_BODY)) {
        mprAssert(tx->fileInfo.valid);
        if (tx->fileInfo.size > conn->limits->transmissionBodySize) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE,
                "Http transmission aborted. File size exceeds max body of %,Ld bytes", conn->limits->transmissionBodySize);
            return;
        }
        tx->file = mprOpenFile(tx->filename, O_RDONLY | O_BINARY, 0);
        if (tx->file == 0) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s, err %d", tx->filename, mprGetError());
        }
    }
}


void httpSendClose(HttpQueue *q)
{
    HttpTx  *tx;

    tx = q->conn->tx;
    if (tx->file) {
        mprCloseFile(tx->file);
        tx->file = 0;
    }
}


void httpSendOutgoingService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    MprFile     *file;
    MprOff      written;
    int         errCode;

    conn = q->conn;
    tx = conn->tx;
    conn->lastActivity = conn->http->now;
    mprAssert(conn->sock);

    if (!conn->sock || conn->connectorComplete) {
        return;
    }
    if (tx->flags & HTTP_TX_NO_BODY) {
        httpDiscardData(q, 1);
    }
    if ((tx->bytesWritten + q->ioCount) > conn->limits->transmissionBodySize) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE | ((tx->bytesWritten) ? HTTP_ABORT : 0),
            "Http transmission aborted. Exceeded max body of %,Ld bytes", conn->limits->transmissionBodySize);
        if (tx->bytesWritten) {
            httpConnectorComplete(conn);
            return;
        }
    }
    /*
        Loop doing non-blocking I/O until blocked or all the packets received are written.
     */
    while (1) {
        /*
            Rebuild the iovector only when the past vector has been completely written. Simplifies the logic quite a bit.
         */
        if (q->ioIndex == 0 && buildSendVec(q) <= 0) {
            break;
        }
        file = q->ioFile ? tx->file : 0;
        written = mprSendFileToSocket(conn->sock, file, q->ioPos, q->ioCount, q->iovec, q->ioIndex, NULL, 0);

        mprLog(8, "Send connector ioCount %d, wrote %Ld, written so far %Ld, sending file %d, q->count %d/%d", 
                q->ioCount, written, tx->bytesWritten, q->ioFile, q->count, q->max);
        if (written < 0) {
            errCode = mprGetError(q);
            if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
                /* Socket is full. Wait for an I/O event */
                httpSetWriteBlocked(conn);
                break;
            }
            if (errCode != EPIPE && errCode != ECONNRESET) {
                mprLog(7, "SendFileToSocket failed, errCode %d", errCode);
            }
            httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "SendFileToSocket failed, errCode %d", errCode);
            httpConnectorComplete(conn);
            break;

        } else if (written == 0) {
            /* Socket is full. Wait for an I/O event */
            httpSetWriteBlocked(conn);
            break;

        } else if (written > 0) {
            tx->bytesWritten += written;
            adjustPacketData(q, written);
            adjustSendVec(q, written);
        }
    }
    if (q->ioCount == 0) {
        if ((q->flags & HTTP_QUEUE_EOF)) {
            httpConnectorComplete(conn);
        } else {
            httpWritable(conn);
        }
    }
}


/*  
    Build the IO vector. This connector uses the send file API which permits multiple IO blocks to be written with 
    file data. This is used to write transfer the headers and chunk encoding boundaries. Return the count of bytes to 
    be written. Return -1 for EOF.
 */
static MprOff buildSendVec(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;

    mprAssert(q->ioIndex == 0);

    conn = q->conn;
    q->ioCount = 0;
    q->ioFile = 0;

    /*  
        Examine each packet and accumulate as many packets into the I/O vector as possible. Can only have one data packet at
        a time due to the limitations of the sendfile API (on Linux). And the data packet must be after all the 
        vector entries. Leave the packets on the queue for now, they are removed after the IO is complete for the 
        entire packet.
     */
    for (packet = q->first; packet; packet = packet->next) {
        if (packet->flags & HTTP_PACKET_HEADER) {
            httpWriteHeaders(conn, packet);
            q->count += httpGetPacketLength(packet);
            
        } else if (httpGetPacketLength(packet) == 0 && packet->esize == 0) {
            q->flags |= HTTP_QUEUE_EOF;
            if (packet->prefix == NULL) {
                break;
            }
        }
        if (q->ioFile || q->ioIndex >= (HTTP_MAX_IOVEC - 2)) {
            break;
        }
        addPacketForSend(q, packet);
    }
    return q->ioCount;
}


/*  
    Add one entry to the io vector
 */
static void addToSendVector(HttpQueue *q, char *ptr, ssize bytes)
{
    mprAssert(ptr > 0);
    mprAssert(bytes > 0);

    q->iovec[q->ioIndex].start = ptr;
    q->iovec[q->ioIndex].len = bytes;
    q->ioCount += bytes;
    q->ioIndex++;
}


/*  
    Add a packet to the io vector. Return the number of bytes added to the vector.
 */
static void addPacketForSend(HttpQueue *q, HttpPacket *packet)
{
    HttpTx      *tx;
    HttpConn    *conn;
    int         item;

    conn = q->conn;
    tx = conn->tx;
    
    mprAssert(q->count >= 0);
    mprAssert(q->ioIndex < (HTTP_MAX_IOVEC - 2));

    if (packet->prefix) {
        addToSendVector(q, mprGetBufStart(packet->prefix), mprGetBufLength(packet->prefix));
    }
    if (packet->esize > 0) {
        mprAssert(q->ioFile == 0);
        q->ioFile = 1;
        q->ioCount += packet->esize;

    } else if (httpGetPacketLength(packet) > 0) {
        /*
            Header packets have actual content. File data packets are virtual and only have a count.
         */
        addToSendVector(q, mprGetBufStart(packet->content), httpGetPacketLength(packet));
        item = (packet->flags & HTTP_PACKET_HEADER) ? HTTP_TRACE_HEADER : HTTP_TRACE_BODY;
        if (httpShouldTrace(conn, HTTP_TRACE_TX, item, tx->ext) >= 0) {
            httpTraceContent(conn, HTTP_TRACE_TX, item, packet, 0, tx->bytesWritten);
        }
    }
}


/*  
    Clear entries from the IO vector that have actually been transmitted. This supports partial writes due to the socket
    being full. Don't come here if we've seen all the packets and all the data has been completely written. ie. small files
    don't come here.
 */
static void adjustPacketData(HttpQueue *q, MprOff bytes)
{
    HttpPacket  *packet;
    ssize       len;

    mprAssert(q->first);
    mprAssert(q->count >= 0);
    mprAssert(bytes >= 0);

    while ((packet = q->first) != 0) {
        if (packet->prefix) {
            len = mprGetBufLength(packet->prefix);
            len = (ssize) min(len, bytes);
            mprAdjustBufStart(packet->prefix, len);
            bytes -= len;
            /* Prefixes don't count in the q->count. No need to adjust */
            if (mprGetBufLength(packet->prefix) == 0) {
                packet->prefix = 0;
            }
        }
        if (packet->esize) {
            len = (ssize) min(packet->esize, bytes);
            packet->esize -= len;
            packet->epos += len;
            bytes -= len;
            mprAssert(packet->esize >= 0);
            mprAssert(bytes == 0);
            if (packet->esize > 0) {
                break;
            }
        } else if ((len = httpGetPacketLength(packet)) > 0) {
            len = (ssize) min(len, bytes);
            mprAdjustBufStart(packet->content, len);
            bytes -= len;
            q->count -= len;
            mprAssert(q->count >= 0);
        }
        if (httpGetPacketLength(packet) == 0) {
            httpGetPacket(q);
        }
        mprAssert(bytes >= 0);
        if (bytes == 0 && (q->first == NULL || !(q->first->flags & HTTP_PACKET_END))) {
            break;
        }
    }
}


/*  
    Clear entries from the IO vector that have actually been transmitted. This supports partial writes due to the socket
    being full. Don't come here if we've seen all the packets and all the data has been completely written. ie. small files
    don't come here.
 */
static void adjustSendVec(HttpQueue *q, MprOff written)
{
    MprIOVec    *iovec;
    ssize       len;
    int         i, j;

    iovec = q->iovec;
    for (i = 0; i < q->ioIndex; i++) {
        len = iovec[i].len;
        if (written < len) {
            iovec[i].start += (ssize) written;
            iovec[i].len -= (ssize) written;
            return;
        }
        written -= len;
        q->ioCount -= len;
        for (j = i + 1; i < q->ioIndex; ) {
            iovec[j++] = iovec[i++];
        }
        q->ioIndex--;
        i--;
    }
    if (written > 0 && q->ioFile) {
        /* All remaining data came from the file */
        q->ioPos += written;
    }
    q->ioIndex = 0;
    q->ioCount = 0;
    q->ioFile = 0;
}


#else
int httpOpenSendConnector(Http *http) { return 0; }
void httpSendOpen(HttpQueue *q) {}
void httpSendOutgoingService(HttpQueue *q) {}

#endif /* !BLD_FEATURE_ROMFS */
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
/************************************************************************/
/*
 *  End of file "./src/sendConnector.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/stage.c"
 */
/************************************************************************/

/*
    stage.c -- Stages are the building blocks of the Http request pipeline.

    Stages support the extensible and modular processing of HTTP requests. Handlers are a kind of stage that are the 
    first line processing of a request. Connectors are the last stage in a chain to send/receive data over a network.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageStage(HttpStage *stage, int flags);


static void defaultOpen(HttpQueue *q)
{
    HttpTx      *tx;

    tx = q->conn->tx;
    q->packetSize = (tx->chunkSize > 0) ? min(q->max, tx->chunkSize): q->max;
}


static void defaultClose(HttpQueue *q)
{
}


/*  
    The default put will put the packet on the service queue.
 */
static void outgoingData(HttpQueue *q, HttpPacket *packet)
{
    int     enableService;

    /*  
        Handlers service routines must only be auto-enabled if in the running state.
     */
    mprAssert(httpVerifyQueue(q));
    enableService = !(q->stage->flags & HTTP_STAGE_HANDLER) || (q->conn->state == HTTP_STATE_RUNNING) ? 1 : 0;
    httpPutForService(q, packet, enableService);
    mprAssert(httpVerifyQueue(q));
}


/*  
    Default incoming data routine.  Simply transfer the data upstream to the next filter or handler.
 */
static void incomingData(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(q);
    mprAssert(packet);
    mprAssert(httpVerifyQueue(q));
    
    if (q->nextQ->put) {
        httpPutPacketToNext(q, packet);
    } else {
        /* This queue is the last queue in the pipeline */
        //  TODO - should this call WillAccept?
        if (httpGetPacketLength(packet) > 0) {
            httpJoinPacketForService(q, packet, 0);
            HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
        } else {
            /* Zero length packet means eof */
            httpPutForService(q, packet, HTTP_DELAY_SERVICE);
            HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
        }
    }
    mprAssert(httpVerifyQueue(q));
}


void httpDefaultOutgoingServiceStage(HttpQueue *q)
{
    HttpPacket    *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!httpWillNextQueueAcceptPacket(q, packet)) {
            httpPutBackPacket(q, packet);
            return;
        }
        httpPutPacketToNext(q, packet);
    }
}


static void incomingService(HttpQueue *q)
{
}


HttpStage *httpCreateStage(Http *http, cchar *name, int flags, MprModule *module)
{
    HttpStage     *stage;

    mprAssert(http);
    mprAssert(name && *name);

    if ((stage = httpLookupStage(http, name)) != 0) {
        if (!(stage->flags & HTTP_STAGE_UNLOADED)) {
            mprError("Stage %s already exists", name);
            return 0;
        }
    } else if ((stage = mprAllocObj(HttpStage, manageStage)) == 0) {
        return 0;
    }
    if ((flags & HTTP_METHOD_MASK) == 0) {
        flags |= HTTP_STAGE_METHODS;
    }
    stage->flags = flags;
    stage->name = sclone(name);
    stage->open = defaultOpen;
    stage->close = defaultClose;
    stage->incomingData = incomingData;
    stage->incomingService = incomingService;
    stage->outgoingData = outgoingData;
    stage->outgoingService = httpDefaultOutgoingServiceStage;
    stage->module = module;
    httpAddStage(http, stage);
    return stage;
}


static void manageStage(HttpStage *stage, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(stage->name);
        mprMark(stage->path);
        mprMark(stage->stageData);
        mprMark(stage->module);
        mprMark(stage->extensions);
    }
}


HttpStage *httpCloneStage(Http *http, HttpStage *stage)
{
    HttpStage   *clone;

    if ((clone = mprAllocObj(HttpStage, manageStage)) == 0) {
        return 0;
    }
    *clone = *stage;
    return clone;
}


HttpStage *httpCreateHandler(Http *http, cchar *name, int flags, MprModule *module)
{
    return httpCreateStage(http, name, flags | HTTP_STAGE_HANDLER, module);
}


HttpStage *httpCreateFilter(Http *http, cchar *name, int flags, MprModule *module)
{
    return httpCreateStage(http, name, flags | HTTP_STAGE_FILTER, module);
}


HttpStage *httpCreateConnector(Http *http, cchar *name, int flags, MprModule *module)
{
    return httpCreateStage(http, name, flags | HTTP_STAGE_CONNECTOR, module);
}


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
/************************************************************************/
/*
 *  End of file "./src/stage.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/trace.c"
 */
/************************************************************************/

/*
    trace.c -- Trace data
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




void httpSetRouteTraceFilter(HttpRoute *route, int dir, int levels[HTTP_TRACE_MAX_ITEM], ssize len, 
    cchar *include, cchar *exclude)
{
    HttpTrace   *trace;
    char        *word, *tok, *line;
    int         i;

    trace = &route->trace[dir];
    trace->size = len;
    for (i = 0; i < HTTP_TRACE_MAX_ITEM; i++) {
        trace->levels[i] = levels[i];
    }
    if (include && strcmp(include, "*") != 0) {
        trace->include = mprCreateHash(0, 0);
        line = sclone(include);
        word = stok(line, ", \t\r\n", &tok);
        while (word) {
            if (word[0] == '*' && word[1] == '.') {
                word += 2;
            }
            mprAddKey(trace->include, word, route);
            word = stok(NULL, ", \t\r\n", &tok);
        }
    }
    if (exclude) {
        trace->exclude = mprCreateHash(0, 0);
        line = sclone(exclude);
        word = stok(line, ", \t\r\n", &tok);
        while (word) {
            if (word[0] == '*' && word[1] == '.') {
                word += 2;
            }
            mprAddKey(trace->exclude, word, route);
            word = stok(NULL, ", \t\r\n", &tok);
        }
    }
}


void httpManageTrace(HttpTrace *trace, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(trace->include);
        mprMark(trace->exclude);
    }
}


void httpInitTrace(HttpTrace *trace)
{
    int     dir;

    mprAssert(trace);

    for (dir = 0; dir < HTTP_TRACE_MAX_DIR; dir++) {
        trace[dir].levels[HTTP_TRACE_CONN] = 3;
        trace[dir].levels[HTTP_TRACE_FIRST] = 2;
        trace[dir].levels[HTTP_TRACE_HEADER] = 3;
        trace[dir].levels[HTTP_TRACE_BODY] = 4;
        trace[dir].levels[HTTP_TRACE_LIMITS] = 5;
        trace[dir].levels[HTTP_TRACE_TIME] = 6;
        trace[dir].size = -1;
    }
}


/*
    If tracing should occur, return the level
 */
int httpShouldTrace(HttpConn *conn, int dir, int item, cchar *ext)
{
    HttpTrace   *trace;

    mprAssert(0 <= dir && dir < HTTP_TRACE_MAX_DIR);
    mprAssert(0 <= item && item < HTTP_TRACE_MAX_ITEM);

    trace = &conn->trace[dir];
    if (trace->disable || trace->levels[item] > MPR->logLevel) {
        return -1;
    }
    if (ext) {
        if (trace->include && !mprLookupKey(trace->include, ext)) {
            trace->disable = 1;
            return -1;
        }
        if (trace->exclude && mprLookupKey(trace->exclude, ext)) {
            trace->disable = 1;
            return -1;
        }
    }
    return trace->levels[item];
}


//  MOB OPT
static void traceBuf(HttpConn *conn, int dir, int level, cchar *msg, cchar *buf, ssize len)
{
    cchar       *start, *cp, *tag, *digits;
    char        *data, *dp;
    static int  txSeq = 0;
    static int  rxSeq = 0;
    int         seqno, i, printable;

    start = buf;
    if (len > 3 && start[0] == (char) 0xef && start[1] == (char) 0xbb && start[2] == (char) 0xbf) {
        start += 3;
    }
    for (printable = 1, i = 0; i < len; i++) {
        if (!isascii(start[i])) {
            printable = 0;
        }
    }
    if (dir == HTTP_TRACE_TX) {
        tag = "Transmit";
        seqno = txSeq++;
    } else {
        tag = "Receive";
        seqno = rxSeq++;
    }
    if (printable) {
        data = mprAlloc(len + 1);
        memcpy(data, start, len);
        data[len] = '\0';
        mprRawLog(level, "\n>>>>>>>>>> %s %s packet %d, len %d (conn %d) >>>>>>>>>>\n%s", tag, msg, seqno, 
            len, conn->seqno, data);
    } else {
        mprRawLog(level, "\n>>>>>>>>>> %s %s packet %d, len %d (conn %d) >>>>>>>>>> (binary)\n", tag, msg, seqno, 
            len, conn->seqno);
        data = mprAlloc(len * 3 + ((len / 16) + 1) + 1);
        digits = "0123456789ABCDEF";
        for (i = 0, cp = start, dp = data; cp < &start[len]; cp++) {
            *dp++ = digits[(*cp >> 4) & 0x0f];
            *dp++ = digits[*cp++ & 0x0f];
            *dp++ = ' ';
            if ((++i % 16) == 0) {
                *dp++ = '\n';
            }
        }
        *dp++ = '\n';
        *dp = '\0';
        mprRawLog(level, "%s", data);
    }
    mprRawLog(level, "<<<<<<<<<< End %s packet, conn %d\n\n", tag, conn->seqno);
}


void httpTraceContent(HttpConn *conn, int dir, int item, HttpPacket *packet, ssize len, MprOff total)
{
    HttpTrace   *trace;
    ssize       size;
    int         level;

    trace = &conn->trace[dir];
    level = trace->levels[item];

    if (trace->size >= 0 && total >= trace->size) {
        mprLog(level, "Abbreviating response trace for conn %d", conn->seqno);
        trace->disable = 1;
        return;
    }
    if (len <= 0) {
        len = MAXINT;
    }
    if (packet->prefix) {
        size = mprGetBufLength(packet->prefix);
        size = min(size, len);
        traceBuf(conn, dir, level, "prefix", mprGetBufStart(packet->prefix), size);
    }
    if (packet->content) {
        size = httpGetPacketLength(packet);
        size = min(size, len);
        traceBuf(conn, dir, level, "content", mprGetBufStart(packet->content), size);
    }
}


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
/************************************************************************/
/*
 *  End of file "./src/trace.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/tx.c"
 */
/************************************************************************/

/*
    tx.c - Http transmitter for server responses and client requests.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageTx(HttpTx *tx, int flags);


HttpTx *httpCreateTx(HttpConn *conn, MprHash *headers)
{
    HttpTx      *tx;

    if ((tx = mprAllocObj(HttpTx, manageTx)) == 0) {
        return 0;
    }
    conn->tx = tx;
    tx->conn = conn;
    tx->status = HTTP_CODE_OK;
    tx->length = -1;
    tx->entityLength = -1;
    tx->traceMethods = HTTP_STAGE_ALL;
    tx->chunkSize = -1;

    tx->queue[HTTP_QUEUE_TX] = httpCreateQueueHead(conn, "TxHead");
    tx->queue[HTTP_QUEUE_RX] = httpCreateQueueHead(conn, "RxHead");

    if (headers) {
        tx->headers = headers;
    } else if ((tx->headers = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS)) != 0) {
        if (conn->endpoint) {
            httpAddHeaderString(conn, "Server", conn->http->software);
        } else {
            httpAddHeaderString(conn, "User-Agent", sclone(HTTP_NAME));
        }
    }
    return tx;
}


void httpDestroyTx(HttpTx *tx)
{
    if (tx->file) {
        mprCloseFile(tx->file);
        tx->file = 0;
    }
    if (tx->conn) {
        tx->conn->tx = 0;
        tx->conn = 0;
    }
}


static void manageTx(HttpTx *tx, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(tx->ext);
        mprMark(tx->etag);
        mprMark(tx->filename);
        mprMark(tx->handler);
        mprMark(tx->parsedUri);
        mprMark(tx->method);
        mprMark(tx->conn);
        mprMark(tx->outputPipeline);
        mprMark(tx->connector);
        mprMark(tx->queue[0]);
        mprMark(tx->queue[1]);
        mprMark(tx->headers);
        mprMark(tx->cache);
        mprMark(tx->cacheBuffer);
        mprMark(tx->cachedContent);
        mprMark(tx->outputRanges);
        mprMark(tx->currentRange);
        mprMark(tx->rangeBoundary);
        mprMark(tx->altBody);
        mprMark(tx->file);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyTx(tx);
    }
}


/*
    Add key/value to the header hash. If already present, update the value
*/
static void addHeader(HttpConn *conn, cchar *key, cchar *value)
{
    mprAssert(key && *key);
    mprAssert(value);

    mprAddKey(conn->tx->headers, key, value);
}


int httpRemoveHeader(HttpConn *conn, cchar *key)
{
    mprAssert(key && *key);
    if (conn->tx == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return mprRemoveKey(conn->tx->headers, key);
}


/*  
    Add a http header if not already defined
 */
void httpAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    char        *value;
    va_list     vargs;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    value = sfmtv(fmt, vargs);
    va_end(vargs);

    if (!mprLookupKey(conn->tx->headers, key)) {
        addHeader(conn, key, value);
    }
}


/*
    Add a header string if not already defined
 */
void httpAddHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    mprAssert(key && *key);
    mprAssert(value);

    if (!mprLookupKey(conn->tx->headers, key)) {
        addHeader(conn, key, sclone(value));
    }
}


/* 
   Append a header. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
void httpAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;
    char        *value;
    cchar       *oldValue;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    value = sfmtv(fmt, vargs);
    va_end(vargs);

    oldValue = mprLookupKey(conn->tx->headers, key);
    if (oldValue) {
        /*
            Set-Cookie has legacy behavior and some browsers require separate headers
         */
        if (scasematch(key, "Set-Cookie")) {
            mprAddDuplicateKey(conn->tx->headers, key, value);
        } else {
            addHeader(conn, key, sfmt("%s, %s", oldValue, value));
        }
    } else {
        addHeader(conn, key, value);
    }
}


/* 
   Append a header string. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
void httpAppendHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    cchar   *oldValue;

    mprAssert(key && *key);
    mprAssert(value && *value);

    oldValue = mprLookupKey(conn->tx->headers, key);
    if (oldValue) {
        if (scasematch(key, "Set-Cookie")) {
            mprAddDuplicateKey(conn->tx->headers, key, sclone(value));
        } else {
            addHeader(conn, key, sfmt("%s, %s", oldValue, value));
        }
    } else {
        addHeader(conn, key, sclone(value));
    }
}


/*  
    Set a http header. Overwrite if present.
 */
void httpSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    char        *value;
    va_list     vargs;

    mprAssert(key && *key);
    mprAssert(fmt && *fmt);

    va_start(vargs, fmt);
    value = sfmtv(fmt, vargs);
    va_end(vargs);
    addHeader(conn, key, value);
}


//  MOB - sort file

void httpSetHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    mprAssert(key && *key);
    mprAssert(value);

    addHeader(conn, key, sclone(value));
}


/*
    Called by connectors (ONLY) when writing the transmission is complete
 */
void httpConnectorComplete(HttpConn *conn)
{
    conn->connectorComplete = 1;
    conn->finalized = 1;
}


void httpFinalize(HttpConn *conn)
{
    if (conn->finalized) {
        return;
    }
    conn->responded = 1;
    conn->finalized = 1;
    if (conn->state >= HTTP_STATE_CONNECTED && conn->writeq && conn->sock) {
        httpPutForService(conn->writeq, httpCreateEndPacket(), HTTP_SERVICE_NOW);
        httpServiceQueues(conn);
        if (conn->state == HTTP_STATE_RUNNING && conn->connectorComplete && !conn->advancing) {
            httpProcess(conn, NULL);
        }
        conn->refinalize = 0;
    } else {
        /* Pipeline has not been setup yet */
        conn->refinalize = 1;
    }
}


int httpIsFinalized(HttpConn *conn)
{
    return conn->finalized;
}


/*
    Flush the write queue
 */
void httpFlush(HttpConn *conn)
{
    httpFlushQueue(conn->writeq, !conn->async);
}


/*
    This formats a response and sets the altBody. The response is not HTML escaped.
    This is the lowest level for formatResponse.
 */
ssize httpFormatResponsev(HttpConn *conn, cchar *fmt, va_list args)
{
    HttpTx      *tx;
    char        *body;

    tx = conn->tx;
    conn->responded = 1;
    body = sfmtv(fmt, args);
    httpOmitBody(conn);
    tx->altBody = body;
    tx->length = slen(tx->altBody);
    return (ssize) tx->length;
}


/*
    This formats a response and sets the altBody. The response is not HTML escaped.
 */
ssize httpFormatResponse(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;
    ssize       rc;

    va_start(args, fmt);
    rc = httpFormatResponsev(conn, fmt, args);
    va_end(args);
    return rc;
}


/*
    This formats a complete response. Depending on the Accept header, the response will be either HTML or plain text.
    The response is not HTML escaped.  This calls httpFormatResponse.
 */
ssize httpFormatResponseBody(HttpConn *conn, cchar *title, cchar *fmt, ...)
{
    va_list     args;
    char        *msg, *body;

    va_start(args, fmt);
    body = sfmtv(fmt, args);
    if (scmp(conn->rx->accept, "text/plain") == 0) {
        msg = body;
    } else {
        msg = sfmt(
            "<!DOCTYPE html>\r\n"
            "<html><head><title>%s</title></head>\r\n"
            "<body>\r\n%s\r\n</body>\r\n</html>\r\n",
            title, body);
    }
    va_end(args);
    return httpFormatResponse(conn, "%s", msg);
}


/*
    Create an alternate body response. Typically used for error responses. The message is HTML escaped.
    NOTE: this is an internal API. Users should use httpFormatError
 */
void httpFormatResponseError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;
    cchar       *statusMsg;
    char        *msg;

    mprAssert(fmt && fmt);

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    statusMsg = httpLookupStatus(conn->http, status);
    if (scmp(conn->rx->accept, "text/plain") == 0) {
        msg = sfmt("Access Error: %d -- %s\r\n%s\r\n", status, statusMsg, msg);
    } else {
        msg = sfmt("<h2>Access Error: %d -- %s</h2>\r\n<pre>%s</pre>\r\n", status, statusMsg, mprEscapeHtml(msg));
    }
    httpAddHeaderString(conn, "Cache-Control", "no-cache");
    httpFormatResponseBody(conn, statusMsg, "%s", msg);
    va_end(args);
}


void *httpGetQueueData(HttpConn *conn)
{
    HttpQueue     *q;

    q = conn->tx->queue[HTTP_QUEUE_TX];
    return q->nextQ->queueData;
}


void httpOmitBody(HttpConn *conn)
{
    if (conn->tx) {
        conn->tx->flags |= HTTP_TX_NO_BODY;
    }
    if (conn->tx->flags & HTTP_TX_HEADERS_CREATED) {
        mprError("Can't set response body if headers have already been created");
        /* Connectors will detect this also and disconnect */
    } else {
        httpDiscardTransmitData(conn);
    }
}


/*  
    Redirect the user to another web page. The targetUri may or may not have a scheme.
 */
void httpRedirect(HttpConn *conn, int status, cchar *targetUri)
{
    HttpTx      *tx;
    HttpRx      *rx;
    HttpUri     *target, *prev;
    cchar       *msg;
    char        *path, *uri, *dir, *cp;
    int         port;

    mprAssert(targetUri);

    mprLog(3, "redirect %d %s", status, targetUri);

    rx = conn->rx;
    tx = conn->tx;
    tx->status = status;
    msg = httpLookupStatus(conn->http, status);

    if (300 <= status && status <= 399) {
        if (targetUri == 0) {
            targetUri = "/";
        }
        target = httpCreateUri(targetUri, 0);

        if (conn->http->redirectCallback) {
            targetUri = (conn->http->redirectCallback)(conn, &status, target);
        }
        if (strstr(targetUri, "://") == 0) {
            prev = rx->parsedUri;
            port = strchr(targetUri, ':') ? prev->port : conn->endpoint->port;
            uri = 0;
            if (target->path[0] == '/') {
                /*
                    Absolute URL. If hostName has a port specifier, it overrides prev->port.
                 */
                uri = httpFormatUri(prev->scheme, rx->hostHeader, port, target->path, target->reference, target->query, 1);
            } else {
                /*
                    Relative file redirection to a file in the same directory as the previous request.
                 */
                dir = sclone(rx->pathInfo);
                if ((cp = strrchr(dir, '/')) != 0) {
                    /* Remove basename */
                    *cp = '\0';
                }
                path = sjoin(dir, "/", target->path, NULL);
                uri = httpFormatUri(prev->scheme, rx->hostHeader, port, path, target->reference, target->query, 1);
            }
            targetUri = uri;
        }
        httpSetHeader(conn, "Location", "%s", targetUri);
        httpFormatResponse(conn, 
            "<!DOCTYPE html>\r\n"
            "<html><head><title>%s</title></head>\r\n"
            "<body><h1>%s</h1>\r\n<p>The document has moved <a href=\"%s\">here</a>.</p></body></html>\r\n",
            msg, msg, targetUri);
    } else {
        httpFormatResponse(conn, 
            "<!DOCTYPE html>\r\n"
            "<html><head><title>%s</title></head>\r\n"
            "<body><h1>%s</h1>\r\n</body></html>\r\n",
            msg, msg);
    }
    httpFinalize(conn);
}


void httpSetContentLength(HttpConn *conn, MprOff length)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        return;
    }
    tx->length = length;
    httpSetHeader(conn, "Content-Length", "%Ld", tx->length);
}


void httpSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, 
        MprTime lifespan, int flags)
{
    HttpRx      *rx;
    char        *cp, *expiresAtt, *expires, *domainAtt, *domain, *extra;
    int         webkitVersion;

    rx = conn->rx;
    if (path == 0) {
        path = "/";
    }
    /* 
        Fix for Safari >= 3.2.1 with Bonjour addresses with a trailing ".". Safari discards cookies without a domain 
        specifier or with a domain that includes a trailing ".". Solution: include an explicit domain and trim the 
        trailing ".".
      
        User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_6; en-us) 
             AppleWebKit/530.0+ (KHTML, like Gecko) Version/3.1.2 Safari/525.20.1
    */
    webkitVersion = 0;
    if (cookieDomain == 0 && rx->userAgent && strstr(rx->userAgent, "AppleWebKit") != 0) {
        if ((cp = strstr(rx->userAgent, "Version/")) != NULL && strlen(cp) >= 13) {
            cp = &cp[8];
            webkitVersion = (cp[0] - '0') * 100 + (cp[2] - '0') * 10 + (cp[4] - '0');
        }
    }
    domain = (char*) cookieDomain;
    if (webkitVersion >= 312) {
        domain = sclone(rx->hostHeader);
        if ((cp = strchr(domain, ':')) != 0) {
            *cp = '\0';
        }
        if (*domain && domain[strlen(domain) - 1] == '.') {
            domain[strlen(domain) - 1] = '\0';
        } else {
            domain = 0;
        }
    }
    if (domain) {
        if (*domain != '.') {
            domain = sjoin(".", domain, NULL);
        }
        domainAtt = "; domain=";
    } else {
        domainAtt = "";
        domain = "";
    }
    if (lifespan > 0) {
        expiresAtt = "; expires=";
        expires = mprFormatUniversalTime(MPR_HTTP_DATE, conn->http->now + lifespan);

    } else {
        expires = expiresAtt = "";
    }
    if (flags & HTTP_COOKIE_SECURE) {
        extra = "; secure";
    } else if (flags & HTTP_COOKIE_HTTP) {
        extra = "; secure";
    } else {
        extra = ";";
    }
    /* 
       Allow multiple cookie headers. Even if the same name. Later definitions take precedence
     */
    httpAppendHeader(conn, "Set-Cookie", 
        sjoin(name, "=", value, "; path=", path, domainAtt, domain, expiresAtt, expires, extra, NULL));
    httpAppendHeader(conn, "Cache-control", "no-cache=\"set-cookie\"");
}


/*  
    Set headers for httpWriteHeaders. This defines standard headers.
 */
static void setHeaders(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    HttpRange   *range;
    cchar       *mimeType;

    mprAssert(packet->flags == HTTP_PACKET_HEADER);

    rx = conn->rx;
    tx = conn->tx;
    route = rx->route;

    /*
        Mandatory headers that must be defined here use httpSetHeader which overwrites existing values. 
     */
    httpAddHeaderString(conn, "Date", conn->http->currentDate);

    if (tx->ext) {
        if ((mimeType = (char*) mprLookupMime(route->mimeTypes, tx->ext)) != 0) {
            if (conn->error) {
                httpAddHeaderString(conn, "Content-Type", "text/html");
            } else {
                httpAddHeaderString(conn, "Content-Type", mimeType);
            }
        }
    }
    if (tx->etag) {
        httpAddHeader(conn, "ETag", "%s", tx->etag);
    }
    if (tx->chunkSize > 0) {
        if (!(rx->flags & HTTP_HEAD)) {
            httpSetHeaderString(conn, "Transfer-Encoding", "chunked");
        }
    } else if (tx->length > 0 || conn->endpoint) {
        httpAddHeader(conn, "Content-Length", "%Ld", tx->length);
    }
    if (tx->outputRanges) {
        if (tx->outputRanges->next == 0) {
            range = tx->outputRanges;
            if (tx->entityLength > 0) {
                httpSetHeader(conn, "Content-Range", "bytes %Ld-%Ld/%Ld", range->start, range->end, tx->entityLength);
            } else {
                httpSetHeader(conn, "Content-Range", "bytes %Ld-%Ld/*", range->start, range->end);
            }
        } else {
            httpSetHeader(conn, "Content-Type", "multipart/byteranges; boundary=%s", tx->rangeBoundary);
        }
        httpSetHeader(conn, "Accept-Ranges", "bytes");
    }
    if (conn->endpoint) {
        if (--conn->keepAliveCount > 0) {
            httpAddHeaderString(conn, "Connection", "keep-alive");
            httpAddHeader(conn, "Keep-Alive", "timeout=%Ld, max=%d", conn->limits->inactivityTimeout / 1000,
                conn->keepAliveCount);
        } else {
            httpAddHeaderString(conn, "Connection", "close");
        }
    }
}


void httpSetEntityLength(HttpConn *conn, int64 len)
{
    HttpTx      *tx;

    tx = conn->tx;
    tx->entityLength = len;
    if (tx->outputRanges == 0) {
        tx->length = len;
    }
}


void httpSetResponded(HttpConn *conn)
{
    conn->responded = 1;
}


void httpSetStatus(HttpConn *conn, int status)
{
    conn->tx->status = status;
    conn->responded = 1;
}


void httpSetContentType(HttpConn *conn, cchar *mimeType)
{
    httpSetHeaderString(conn, "Content-Type", sclone(mimeType));
}


void httpWriteHeaders(HttpConn *conn, HttpPacket *packet)
{
    Http        *http;
    HttpTx      *tx;
    HttpUri     *parsedUri;
    MprKey      *kp;
    MprBuf      *buf;
    int         level;

    mprAssert(packet->flags == HTTP_PACKET_HEADER);

    http = conn->http;
    tx = conn->tx;
    buf = packet->content;

    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        return;
    }    
    conn->responded = 1;
    if (conn->headersCallback) {
        /* Must be before headers below */
        (conn->headersCallback)(conn->headersCallbackArg);
    }
    setHeaders(conn, packet);

    if (conn->endpoint) {
        mprPutStringToBuf(buf, conn->protocol);
        mprPutCharToBuf(buf, ' ');
        mprPutIntToBuf(buf, tx->status);
        mprPutCharToBuf(buf, ' ');
        mprPutStringToBuf(buf, httpLookupStatus(http, tx->status));
    } else {
        mprPutStringToBuf(buf, tx->method);
        mprPutCharToBuf(buf, ' ');
        parsedUri = tx->parsedUri;
        if (http->proxyHost && *http->proxyHost) {
            if (parsedUri->query && *parsedUri->query) {
                mprPutFmtToBuf(buf, "http://%s:%d%s?%s %s", http->proxyHost, http->proxyPort, 
                    parsedUri->path, parsedUri->query, conn->protocol);
            } else {
                mprPutFmtToBuf(buf, "http://%s:%d%s %s", http->proxyHost, http->proxyPort, parsedUri->path,
                    conn->protocol);
            }
        } else {
            if (parsedUri->query && *parsedUri->query) {
                mprPutFmtToBuf(buf, "%s?%s %s", parsedUri->path, parsedUri->query, conn->protocol);
            } else {
                mprPutStringToBuf(buf, parsedUri->path);
                mprPutCharToBuf(buf, ' ');
                mprPutStringToBuf(buf, conn->protocol);
            }
        }
    }
    if ((level = httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_FIRST, tx->ext)) >= mprGetLogLevel(tx)) {
        mprAddNullToBuf(buf);
        mprLog(level, "  %s", mprGetBufStart(buf));
    }
    mprPutStringToBuf(buf, "\r\n");

    /* 
       Output headers
     */
    kp = mprGetFirstKey(conn->tx->headers);
    while (kp) {
        mprPutStringToBuf(packet->content, kp->key);
        mprPutStringToBuf(packet->content, ": ");
        if (kp->data) {
            mprPutStringToBuf(packet->content, kp->data);
        }
        mprPutStringToBuf(packet->content, "\r\n");
        kp = mprGetNextKey(conn->tx->headers, kp);
    }

    /* 
       By omitting the "\r\n" delimiter after the headers, chunks can emit "\r\nSize\r\n" as a single chunk delimiter
     */
    if (tx->chunkSize <= 0) {
        mprPutStringToBuf(buf, "\r\n");
    }
    if (tx->altBody) {
        mprPutStringToBuf(buf, tx->altBody);
        httpDiscardData(tx->queue[HTTP_QUEUE_TX]->nextQ, 0);
    }
    tx->headerSize = mprGetBufLength(buf);
    tx->flags |= HTTP_TX_HEADERS_CREATED;
}


bool httpFileExists(HttpConn *conn)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (!tx->fileInfo.checked) {
        mprGetPathInfo(tx->filename, &tx->fileInfo);
    }
    return tx->fileInfo.valid;
}


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
/************************************************************************/
/*
 *  End of file "./src/tx.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/uploadFilter.c"
 */
/************************************************************************/

/*
    uploadFilter.c - Upload file filter.
    The upload filter processes post data according to RFC-1867 ("multipart/form-data" post data). 
    It saves the uploaded files in a configured upload directory.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/*
    Upload state machine states
 */
#define HTTP_UPLOAD_REQUEST_HEADER        1   /* Request header */
#define HTTP_UPLOAD_BOUNDARY              2   /* Boundary divider */
#define HTTP_UPLOAD_CONTENT_HEADER        3   /* Content part header */
#define HTTP_UPLOAD_CONTENT_DATA          4   /* Content encoded data */
#define HTTP_UPLOAD_CONTENT_END           5   /* End of multipart message */

/*
    Per upload context
 */
typedef struct Upload {
    HttpUploadFile  *currentFile;       /* Current file context */
    MprFile         *file;              /* Current file I/O object */
    char            *boundary;          /* Boundary signature */
    ssize           boundaryLen;        /* Length of boundary */
    int             contentState;       /* Input states */
    char            *clientFilename;    /* Current file filename */
    char            *tmpPath;           /* Current temp filename for upload data */
    char            *id;                /* Current name keyword value */
} Upload;



static void closeUpload(HttpQueue *q);
static char *getBoundary(void *buf, ssize bufLen, void *boundary, ssize boundaryLen);
static void incomingUploadData(HttpQueue *q, HttpPacket *packet);
static void manageHttpUploadFile(HttpUploadFile *file, int flags);
static void manageUpload(Upload *up, int flags);
static int matchUpload(HttpConn *conn, HttpRoute *route, int dir);
static void openUpload(HttpQueue *q);
static int  processContentBoundary(HttpQueue *q, char *line);
static int  processContentHeader(HttpQueue *q, char *line);
static int  processContentData(HttpQueue *q);


int httpOpenUploadFilter(Http *http)
{
    HttpStage     *filter;

    if ((filter = httpCreateFilter(http, "uploadFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->uploadFilter = filter;
    filter->match = matchUpload; 
    filter->open = openUpload; 
    filter->close = closeUpload; 
    filter->incomingData = incomingUploadData; 
    return 0;
}


/*  
    Match if this request needs the upload filter. Return true if needed.
 */
static int matchUpload(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpRx  *rx;
    char    *pat;
    ssize   len;
    
    if (!(dir & HTTP_STAGE_RX)) {
        return HTTP_ROUTE_REJECT;
    }
    rx = conn->rx;
    if (!(rx->flags & HTTP_POST) || rx->remainingContent <= 0) {
        return HTTP_ROUTE_REJECT;
    }
    pat = "multipart/form-data";
    len = strlen(pat);
    if (sncasecmp(rx->mimeType, pat, len) == 0) {
        rx->upload = 1;
        mprLog(5, "matchUpload for %s", rx->uri);
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*  
    Initialize the upload filter for a new request
 */
static void openUpload(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    Upload      *up;
    char        *boundary;

    conn = q->conn;
    rx = conn->rx;

    mprLog(5, "Open upload filter");
    if ((up = mprAllocObj(Upload, manageUpload)) == 0) {
        return;
    }
    q->queueData = up;
    up->contentState = HTTP_UPLOAD_BOUNDARY;

    if (rx->uploadDir == 0) {
#if BLD_WIN_LIKE
        rx->uploadDir = mprNormalizePath(getenv("TEMP"));
#else
        rx->uploadDir = sclone("/tmp");
#endif
    }
    mprLog(5, "Upload directory is %s", rx->uploadDir);

    if ((boundary = strstr(rx->mimeType, "boundary=")) != 0) {
        boundary += 9;
        up->boundary = sjoin("--", boundary, NULL);
        up->boundaryLen = strlen(up->boundary);
    }
    if (up->boundaryLen == 0 || *up->boundary == '\0') {
        httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad boundary");
        return;
    }
    httpSetParam(conn, "UPLOAD_DIR", rx->uploadDir);
}


static void manageUpload(Upload *up, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(up->currentFile);
        mprMark(up->file);
        mprMark(up->boundary);
        mprMark(up->clientFilename);
        mprMark(up->tmpPath);
        mprMark(up->id);
    }
}


/*  
    Cleanup when the entire request has complete
 */
static void closeUpload(HttpQueue *q)
{
    HttpUploadFile  *file;
    HttpRx          *rx;
    Upload          *up;

    rx = q->conn->rx;
    up = q->queueData;
    
    if (up->currentFile) {
        file = up->currentFile;
        file->filename = 0;
    }
    if (rx->autoDelete) {
        httpRemoveAllUploadedFiles(q->conn);
    }
}


/*  
    Incoming data acceptance routine. The service queue is used, but not a service routine as the data is processed
    immediately. Partial data is buffered on the service queue until a correct mime boundary is seen.
 */
static void incomingUploadData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *content;
    Upload      *up;
    char        *line, *nextTok;
    ssize       count;
    int         done, rc;
    
    mprAssert(packet);
    
    conn = q->conn;
    rx = conn->rx;
    up = q->queueData;
    
    if (httpGetPacketLength(packet) == 0) {
        if (up->contentState != HTTP_UPLOAD_CONTENT_END) {
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Client supplied insufficient upload data");
        }
        httpPutPacketToNext(q, packet);
        return;
    }
    mprLog(7, "uploadIncomingData: %d bytes", httpGetPacketLength(packet));
    
    /*  
        Put the packet data onto the service queue for buffering. This aggregates input data incase we don't have
        a complete mime record yet.
     */
    httpJoinPacketForService(q, packet, 0);
    
    packet = q->first;
    content = packet->content;
    mprAddNullToBuf(content);
    count = httpGetPacketLength(packet);

    for (done = 0, line = 0; !done; ) {
        if  (up->contentState == HTTP_UPLOAD_BOUNDARY || up->contentState == HTTP_UPLOAD_CONTENT_HEADER) {
            /*
                Parse the next input line
             */
            line = mprGetBufStart(content);
            stok(line, "\n", &nextTok);
            if (nextTok == 0) {
                /* Incomplete line */
                /* done++; */
                break; 
            }
            mprAdjustBufStart(content, (int) (nextTok - line));
            line = strim(line, "\r", MPR_TRIM_END);
        }
        switch (up->contentState) {
        case HTTP_UPLOAD_BOUNDARY:
            if (processContentBoundary(q, line) < 0) {
                done++;
            }
            break;

        case HTTP_UPLOAD_CONTENT_HEADER:
            if (processContentHeader(q, line) < 0) {
                done++;
            }
            break;

        case HTTP_UPLOAD_CONTENT_DATA:
            rc = processContentData(q);
            if (rc < 0) {
                done++;
            }
            if (httpGetPacketLength(packet) < up->boundaryLen) {
                /*  Incomplete boundary - return to get more data */
                done++;
            }
            break;

        case HTTP_UPLOAD_CONTENT_END:
            done++;
            break;
        }
    }
    /*  
        Compact the buffer to prevent memory growth. There is often residual data after the boundary for the next block.
     */
    if (packet != rx->headerPacket) {
        mprCompactBuf(content);
    }
    q->count -= (count - httpGetPacketLength(packet));
    mprAssert(q->count >= 0);

    if (httpGetPacketLength(packet) == 0) {
        /* 
           Quicker to remove the buffer so the packets don't have to be joined the next time 
         */
        httpGetPacket(q);
        mprAssert(q->count >= 0);
    }
}


/*  
    Process the mime boundary division
    Returns  < 0 on a request or state error
            == 0 if successful
 */
static int processContentBoundary(HttpQueue *q, char *line)
{
    HttpConn    *conn;
    Upload      *up;

    conn = q->conn;
    up = q->queueData;

    /*
        Expecting a multipart boundary string
     */
    if (strncmp(up->boundary, line, up->boundaryLen) != 0) {
        httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad upload state. Incomplete boundary");
        return MPR_ERR_BAD_STATE;
    }
    if (line[up->boundaryLen] && strcmp(&line[up->boundaryLen], "--") == 0) {
        up->contentState = HTTP_UPLOAD_CONTENT_END;
    } else {
        up->contentState = HTTP_UPLOAD_CONTENT_HEADER;
    }
    return 0;
}


/*  
    Expecting content headers. A blank line indicates the start of the data.
    Returns  < 0  Request or state error
    Returns == 0  Successfully parsed the input line.
 */
static int processContentHeader(HttpQueue *q, char *line)
{
    HttpConn        *conn;
    HttpRx          *rx;
    HttpUploadFile  *file;
    Upload          *up;
    char            *key, *headerTok, *rest, *nextPair, *value;

    conn = q->conn;
    rx = conn->rx;
    up = q->queueData;
    
    if (line[0] == '\0') {
        up->contentState = HTTP_UPLOAD_CONTENT_DATA;
        return 0;
    }
    mprLog(7, "Header line: %s", line);

    headerTok = line;
    stok(line, ": ", &rest);

    if (scasecmp(headerTok, "Content-Disposition") == 0) {

        /*  
            The content disposition header describes either a form
            variable or an uploaded file.
        
            Content-Disposition: form-data; name="field1"
            >>blank line
            Field Data
            ---boundary
     
            Content-Disposition: form-data; name="field1" ->
                filename="user.file"
            >>blank line
            File data
            ---boundary
         */
        key = rest;
        up->id = up->clientFilename = 0;
        while (key && stok(key, ";\r\n", &nextPair)) {

            key = strim(key, " ", MPR_TRIM_BOTH);
            stok(key, "= ", &value);
            value = strim(value, "\"", MPR_TRIM_BOTH);

            if (scasecmp(key, "form-data") == 0) {
                /* Nothing to do */

            } else if (scasecmp(key, "name") == 0) {
                up->id = sclone(value);

            } else if (scasecmp(key, "filename") == 0) {
                if (up->id == 0) {
                    httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad upload state. Missing name field");
                    return MPR_ERR_BAD_STATE;
                }
                up->clientFilename = sclone(value);
                /*  
                    Create the file to hold the uploaded data
                 */
                up->tmpPath = mprGetTempPath(rx->uploadDir);
                if (up->tmpPath == 0) {
                    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                        "Can't create upload temp file %s. Check upload temp dir %s", up->tmpPath, rx->uploadDir);
                    return MPR_ERR_CANT_OPEN;
                }
                mprLog(5, "File upload of: %s stored as %s", up->clientFilename, up->tmpPath);

                up->file = mprOpenFile(up->tmpPath, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0600);
                if (up->file == 0) {
                    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't open upload temp file %s", up->tmpPath);
                    return MPR_ERR_BAD_STATE;
                }
                /*  
                    Create the files[id]
                 */
                file = up->currentFile = mprAllocObj(HttpUploadFile, manageHttpUploadFile);
                file->clientFilename = sclone(up->clientFilename);
                file->filename = sclone(up->tmpPath);
            }
            key = nextPair;
        }

    } else if (scasecmp(headerTok, "Content-Type") == 0) {
        if (up->clientFilename) {
            mprLog(5, "Set files[%s][CONTENT_TYPE] = %s", up->id, rest);
            up->currentFile->contentType = sclone(rest);
        }
    }
    return 1;
}


static void manageHttpUploadFile(HttpUploadFile *file, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(file->filename);
        mprMark(file->clientFilename);
        mprMark(file->contentType);
    }
}


static void defineFileFields(HttpQueue *q, Upload *up)
{
    HttpConn        *conn;
    HttpUploadFile  *file;
    char            *key;

    conn = q->conn;
    if (conn->tx->handler == conn->http->ejsHandler) {
        /*  
            Ejscript manages this for itself
         */
        return;
    }
    up = q->queueData;
    file = up->currentFile;
    key = sjoin("FILE_CLIENT_FILENAME_", up->id, NULL);
    httpSetParam(conn, key, file->clientFilename);

    key = sjoin("FILE_CONTENT_TYPE_", up->id, NULL);
    httpSetParam(conn, key, file->contentType);

    key = sjoin("FILE_FILENAME_", up->id, NULL);
    httpSetParam(conn, key, file->filename);

    key = sjoin("FILE_SIZE_", up->id, NULL);
    httpSetIntParam(conn, key, (int) file->size);
}


static int writeToFile(HttpQueue *q, char *data, ssize len)
{
    HttpConn        *conn;
    HttpUploadFile  *file;
    HttpLimits      *limits;
    Upload          *up;
    ssize           rc;

    conn = q->conn;
    limits = conn->limits;
    up = q->queueData;
    file = up->currentFile;

    if ((file->size + len) > limits->uploadSize) {
        httpError(conn, HTTP_CODE_REQUEST_TOO_LARGE, "Uploaded file exceeds maximum %,Ld", limits->uploadSize);
        return MPR_ERR_CANT_WRITE;
    }
    if (len > 0) {
        /*  
            File upload. Write the file data.
         */
        rc = mprWriteFile(up->file, data, len);
        if (rc != len) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                "Can't write to upload temp file %s, rc %d, errno %d", up->tmpPath, rc, mprGetOsError(up));
            return MPR_ERR_CANT_WRITE;
        }
        file->size += len;
        mprLog(7, "uploadFilter: Wrote %d bytes to %s", len, up->tmpPath);
    }
    return 0;
}


/*  
    Process the content data.
    Returns < 0 on error
            == 0 when more data is needed
            == 1 when data successfully written
 */
static int processContentData(HttpQueue *q)
{
    HttpConn        *conn;
    HttpUploadFile  *file;
    HttpPacket      *packet;
    MprBuf          *content;
    Upload          *up;
    ssize           size, dataLen;
    char            *data, *bp, *key;

    conn = q->conn;
    up = q->queueData;
    content = q->first->content;
    file = up->currentFile;
    packet = 0;

    size = mprGetBufLength(content);
    if (size < up->boundaryLen) {
        /*  Incomplete boundary. Return and get more data */
        return 0;
    }
    bp = getBoundary(mprGetBufStart(content), size, up->boundary, up->boundaryLen);
    if (bp == 0) {
        mprLog(7, "uploadFilter: Got boundary filename %x", up->clientFilename);
        if (up->clientFilename) {
            /*  
                No signature found yet. probably more data to come. Must handle split boundaries.
             */
            data = mprGetBufStart(content);
            dataLen = ((int) (mprGetBufEnd(content) - data)) - (up->boundaryLen - 1);
            if (dataLen > 0 && writeToFile(q, mprGetBufStart(content), dataLen) < 0) {
                return MPR_ERR_CANT_WRITE;
            }
            mprAdjustBufStart(content, dataLen);
            return 0;       /* Get more data */
        }
    }
    data = mprGetBufStart(content);
    dataLen = (bp) ? (bp - data) : mprGetBufLength(content);

    if (dataLen > 0) {
        mprAdjustBufStart(content, dataLen);
        /*  
            This is the CRLF before the boundary
         */
        if (dataLen >= 2 && data[dataLen - 2] == '\r' && data[dataLen - 1] == '\n') {
            dataLen -= 2;
        }
        if (up->clientFilename) {
            /*  
                Write the last bit of file data and add to the list of files and define environment variables
             */
            if (writeToFile(q, data, dataLen) < 0) {
                return MPR_ERR_CANT_WRITE;
            }
            httpAddUploadFile(conn, up->id, file);
            defineFileFields(q, up);

        } else {
            /*  
                Normal string form data variables
             */
            data[dataLen] = '\0'; 
            mprLog(5, "uploadFilter: form[%s] = %s", up->id, data);
            key = mprUriDecode(up->id);
            data = mprUriDecode(data);
            httpSetParam(conn, key, data);

            if (packet == 0) {
                packet = httpCreatePacket(HTTP_BUFSIZE);
            }
            if (httpGetPacketLength(packet) > 0) {
                /*
                    Need to add www-form-urlencoding separators
                 */
                mprPutCharToBuf(packet->content, '&');
            } else {
                conn->rx->mimeType = sclone("application/x-www-form-urlencoded");

            }
            mprPutFmtToBuf(packet->content, "%s=%s", up->id, data);
        }
    }
    if (up->clientFilename) {
        /*  
            Now have all the data (we've seen the boundary)
         */
        mprCloseFile(up->file);
        up->file = 0;
        up->clientFilename = 0;
    }
    if (packet) {
        httpPutPacketToNext(q, packet);
    }
    up->contentState = HTTP_UPLOAD_BOUNDARY;
    return 1;
}


/*  
    Find the boundary signature in memory. Returns pointer to the first match.
 */ 
static char *getBoundary(void *buf, ssize bufLen, void *boundary, ssize boundaryLen)
{
    char    *cp, *endp;
    char    first;

    mprAssert(buf);
    mprAssert(boundary);
    mprAssert(boundaryLen > 0);

    first = *((char*) boundary);
    cp = (char*) buf;

    if (bufLen < boundaryLen) {
        return 0;
    }
    endp = cp + (bufLen - boundaryLen) + 1;
    while (cp < endp) {
        cp = (char *) memchr(cp, first, endp - cp);
        if (!cp) {
            return 0;
        }
        if (memcmp(cp, boundary, boundaryLen) == 0) {
            return cp;
        }
        cp++;
    }
    return 0;
}

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
/************************************************************************/
/*
 *  End of file "./src/uploadFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/uri.c"
 */
/************************************************************************/

/*
    uri.c - URI manipulation routines
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static int getPort(HttpUri *uri);
static int getDefaultPort(cchar *scheme);
static void manageUri(HttpUri *uri, int flags);
static void trimPathToDirname(HttpUri *uri);

/*  
    Create and initialize a URI. This accepts full URIs with schemes (http:) and partial URLs
 */
HttpUri *httpCreateUri(cchar *uri, int complete)
{
    HttpUri     *up;
    char        *tok, *cp;

    mprAssert(uri);

    if ((up = mprAllocObj(HttpUri, manageUri)) == 0) {
        return 0;
    }
    up->uri = sclone(uri);

    tok = up->uri;
    if (schr(tok, ':')) {
        if (sncmp(up->uri, "https://", 8) == 0) {
            up->scheme = sclone("https");
            up->secure = 1;
            if (complete) {
                up->port = 443;
            }
            tok = &up->uri[8];

        } else if (sncmp(up->uri, "http://", 7) == 0) {
            up->scheme = sclone("http");
            tok = &up->uri[7];

        } else {
            up->scheme = 0;
            tok = up->uri;
        }
        if ((cp = spbrk(tok, ":/")) == NULL) {
            up->host = sclone(tok);

        } else if (*cp == ':') {
            up->host = snclone(tok, cp - tok);
            up->port = atoi(++cp);

        } else if (*cp == '/') {
            up->host = snclone(tok, cp - tok);
        }
        if (complete && up->port == 0) {
            up->port = 80;
        }
        tok = schr(cp, '/');

    } else {
        if (complete) {
            up->scheme = "http";
            up->host = "localhost";
            up->port = 80;
        }
        tok = up->uri;
    }

    if ((cp = spbrk(tok, "#?")) == NULL) {
        up->path = sclone(tok);

    } else {
        up->path = snclone(tok, cp - tok);
        tok = cp + 1;
        if (*cp == '#') {
            if ((cp = schr(tok, '?')) != NULL) {
                up->reference = snclone(tok, cp - tok);
                up->query = sclone(++cp);
            } else {
                up->reference = sclone(tok);
            }
        } else {
            up->query = sclone(tok);
        }
    }

    if (up->path && (tok = srchr(up->path, '.')) != NULL) {
        if ((cp = srchr(up->path, '/')) != NULL) {
            if (cp <= tok) {
                up->ext = sclone(++tok);
            }
        } else {
            up->ext = sclone(++tok);
        }
    }
    if (up->path == 0) {
        up->path = sclone("/");
    }
    return up;
}


static void manageUri(HttpUri *uri, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(uri->scheme);
        mprMark(uri->host);
        mprMark(uri->path);
        mprMark(uri->ext);
        mprMark(uri->reference);
        mprMark(uri->query);
        mprMark(uri->uri);
    }
}


/*  
    Create and initialize a URI. This accepts full URIs with schemes (http:) and partial URLs
 */
HttpUri *httpCreateUriFromParts(cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, cchar *query, 
        int complete)
{
    HttpUri     *up;
    char        *cp, *tok;

    if ((up = mprAllocObj(HttpUri, manageUri)) == 0) {
        return 0;
    }
    if (scheme) {
        up->scheme = sclone(scheme);
    } else if (complete) {
        up->scheme = "http";
    }
    if (host) {
        up->host = sclone(host);
        if ((cp = schr(host, ':')) && port == 0) {
            port = (int) stoi(++cp);
        }
    } else if (complete) {
        up->host = sclone("localhost");
    }
    if (port) {
        up->port = port;
    }
    if (path) {
        while (path[0] == '/' && path[1] == '/') {
            path++;
        }
        up->path = sclone(path);
    }
    if (up->path == 0) {
        up->path = sclone("/");
    }
    if (reference) {
        up->reference = sclone(reference);
    }
    if (query) {
        up->query = sclone(query);
    }
    if ((tok = srchr(up->path, '.')) != NULL) {
        if ((cp = srchr(up->path, '/')) != NULL) {
            if (cp <= tok) {
                up->ext = sclone(&tok[1]);
            }
        } else {
            up->ext = sclone(&tok[1]);
        }
    }
    return up;
}


HttpUri *httpCloneUri(HttpUri *base, int complete)
{
    HttpUri     *up;
    char        *path, *cp, *tok;
    int         port;

    if ((up = mprAllocObj(HttpUri, manageUri)) == 0) {
        return 0;
    }
    port = base->port;
    path = base->path;

    if (base->scheme) {
        up->scheme = sclone(base->scheme);
    } else if (complete) {
        up->scheme = sclone("http");
    }
    if (base->host) {
        up->host = sclone(base->host);
        if ((cp = schr(base->host, ':')) && port == 0) {
            port = (int) stoi(++cp);
        }
    } else if (complete) {
        base->host = sclone("localhost");
    }
    if (port) {
        up->port = port;
    }
    if (path) {
        while (path[0] == '/' && path[1] == '/') {
            path++;
        }
        up->path = sclone(path);
    }
    if (up->path == 0) {
        up->path = sclone("/");
    }
    if (base->reference) {
        up->reference = sclone(base->reference);
    }
    if (base->query) {
        up->query = sclone(base->query);
    }
    if ((tok = srchr(up->path, '.')) != NULL) {
        if ((cp = srchr(up->path, '/')) != NULL) {
            if (cp <= tok) {
                up->ext = sclone(&tok[1]);
            }
        } else {
            up->ext = sclone(&tok[1]);
        }
    }
    return up;
}


HttpUri *httpCompleteUri(HttpUri *uri, HttpUri *missing)
{
    char        *scheme, *host;
    int         port;

    scheme = (missing) ? missing->scheme : "http";
    host = (missing) ? missing->host : "localhost";
    port = (missing) ? missing->port : 0;

    if (uri->scheme == 0) {
        uri->scheme = sclone(scheme);
    }
    if (uri->port == 0 && port) {
        /* Don't complete port if there is a host */
        if (uri->host == 0) {
            uri->port = port;
        }
    }
    if (uri->host == 0) {
        uri->host = sclone(host);
    }
    return uri;
}


/*  
    Format a fully qualified URI
    If complete is true, missing elements are completed
 */
char *httpFormatUri(cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, cchar *query, int complete)
{
    char    *uri;
    cchar   *portStr, *hostDelim, *portDelim, *pathDelim, *queryDelim, *referenceDelim;

    if (complete || host || scheme) {
        if (scheme == 0 || *scheme == '\0') {
            scheme = "http";
        }
        if (host == 0 || *host == '\0') {
            host = "localhost";
        }
        hostDelim = "://";
    } else {
        host = hostDelim = "";
    }
    /*  
        Hosts with integral port specifiers override
     */
    if (host && schr(host, ':')) {
        portDelim = "";
        portStr = "";
    } else {
        if (port != 0 && port != getDefaultPort(scheme)) {
            portStr = itos(port);
            portDelim = ":";
        } else {
            portStr = "";
            portDelim = "";
        }
    }
    if (scheme == 0) {
        scheme = "";
    }
    if (path && *path) {
        if (*hostDelim) {
            pathDelim = (*path == '/') ? "" :  "/";
        } else {
            pathDelim = "";
        }
    } else {
        pathDelim = path = "";
    }
    if (reference && *reference) {
        referenceDelim = "#";
    } else {
        referenceDelim = reference = "";
    }
    if (query && *query) {
        queryDelim = "?";
    } else {
        queryDelim = query = "";
    }
    if (portDelim) {
        uri = sjoin(scheme, hostDelim, host, portDelim, portStr, pathDelim, path, referenceDelim, reference, 
            queryDelim, query, NULL);
    } else {
        uri = sjoin(scheme, hostDelim, host, pathDelim, path, referenceDelim, reference, queryDelim, query, NULL);
    }
    return uri;
}


/*
    This returns a URI relative to the base for the given target

    uri = target.relative(base)
 */
HttpUri *httpGetRelativeUri(HttpUri *base, HttpUri *target, int clone)
{
    HttpUri     *uri;
    char        *basePath, *bp, *cp, *tp, *startDiff;
    int         i, baseSegments, commonSegments;

    if (target == 0) {
        return (clone) ? httpCloneUri(base, 0) : base;
    }
    if (!(target->path && target->path[0] == '/') || !((base->path && base->path[0] == '/'))) {
        /* If target is relative, just use it. If base is relative, can't use it because we don't know where it is */
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    if (base->scheme && target->scheme && scmp(base->scheme, target->scheme) != 0) {
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    if (base->host && target->host && (base->host && scmp(base->host, target->host) != 0)) {
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    if (getPort(base) != getPort(target)) {
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    basePath = httpNormalizeUriPath(base->path);

    /* Count trailing "/" */
    for (baseSegments = 0, bp = basePath; *bp; bp++) {
        if (*bp == '/') {
            baseSegments++;
        }
    }

    /*
        Find portion of path that matches the base, if any.
     */
    commonSegments = 0;
    for (bp = base->path, tp = startDiff = target->path; *bp && *tp; bp++, tp++) {
        if (*bp == '/') {
            if (*tp == '/') {
                commonSegments++;
                startDiff = tp;
            }
        } else {
            if (*bp != *tp) {
                break;
            }
        }
    }

    /*
        Add one more segment if the last segment matches. Handle trailing separators.
     */
#if OLD
    if ((*bp == '/' || *bp == '\0') && (*tp == '/' || *tp == '\0')) {
        commonSegments++;
    }
#endif
    if (*startDiff == '/') {
        startDiff++;
    }
    
    if ((uri = httpCloneUri(target, 0)) == 0) {
        return 0;
    }
    uri->host = 0;
    uri->scheme = 0;
    uri->port = 0;

    uri->path = cp = mprAlloc(baseSegments * 3 + (int) slen(target->path) + 2);
    for (i = commonSegments; i < baseSegments; i++) {
        *cp++ = '.';
        *cp++ = '.';
        *cp++ = '/';
    }
    if (*startDiff) {
        strcpy(cp, startDiff);
    } else if (cp > uri->path) {
        /*
            Cleanup trailing separators ("../" is the end of the new path)
         */
        cp[-1] = '\0';
    } else {
        strcpy(uri->path, ".");
    }
    return uri;
}


/*
    result = base.join(other)
 */
HttpUri *httpJoinUriPath(HttpUri *result, HttpUri *base, HttpUri *other)
{
    char    *sep;

    if (other->path[0] == '/') {
        result->path = sclone(other->path);
    } else {
        sep = ((base->path[0] == '\0' || base->path[slen(base->path) - 1] == '/') || 
               (other->path[0] == '\0' || other->path[0] == '/'))  ? "" : "/";
        result->path = sjoin(base->path, sep, other->path, NULL);
    }
    return result;
}


HttpUri *httpJoinUri(HttpUri *uri, int argc, HttpUri **others)
{
    HttpUri     *other;
    int         i;

    uri = httpCloneUri(uri, 0);

    for (i = 0; i < argc; i++) {
        other = others[i];
        if (other->scheme) {
            uri->scheme = sclone(other->scheme);
        }
        if (other->host) {
            uri->host = sclone(other->host);
        }
        if (other->port) {
            uri->port = other->port;
        }
        if (other->path) {
            httpJoinUriPath(uri, uri, other);
        }
        if (other->reference) {
            uri->reference = sclone(other->reference);
        }
        if (other->query) {
            uri->query = sclone(other->query);
        }
    }
    uri->ext = mprGetPathExt(uri->path);
    return uri;
}


HttpUri *httpMakeUriLocal(HttpUri *uri)
{
    if (uri) {
        uri->host = 0;
        uri->scheme = 0;
        uri->port = 0;
    }
    return uri;
}


void httpNormalizeUri(HttpUri *uri)
{
    uri->path = httpNormalizeUriPath(uri->path);
}


/*
    Normalize a URI path to remove redundant "./" and cleanup "../" and make separator uniform. Does not make an abs path.
    It does not map separators nor change case. 
 */
char *httpNormalizeUriPath(cchar *pathArg)
{
    char    *dupPath, *path, *sp, *dp, *mark, **segments;
    int     firstc, j, i, nseg, len;

    if (pathArg == 0 || *pathArg == '\0') {
        return mprEmptyString();
    }
    len = (int) slen(pathArg);
    if ((dupPath = mprAlloc(len + 2)) == 0) {
        return NULL;
    }
    strcpy(dupPath, pathArg);

    if ((segments = mprAlloc(sizeof(char*) * (len + 1))) == 0) {
        return NULL;
    }
    nseg = len = 0;
    firstc = *dupPath;
    for (mark = sp = dupPath; *sp; sp++) {
        if (*sp == '/') {
            *sp = '\0';
            while (sp[1] == '/') {
                sp++;
            }
            segments[nseg++] = mark;
            len += (int) (sp - mark);
            mark = sp + 1;
        }
    }
    segments[nseg++] = mark;
    len += (int) (sp - mark);
    for (j = i = 0; i < nseg; i++, j++) {
        sp = segments[i];
        if (sp[0] == '.') {
            if (sp[1] == '\0')  {
                if ((i+1) == nseg) {
                    segments[j] = "";
                } else {
                    j--;
                }
            } else if (sp[1] == '.' && sp[2] == '\0')  {
                if (i == 1 && *segments[0] == '\0') {
                    j = 0;
                } else if ((i+1) == nseg) {
                    if (--j >= 0) {
                        segments[j] = "";
                    }
                } else {
                    j = max(j - 2, -1);
                }
            }
        } else {
            segments[j] = segments[i];
        }
    }
    nseg = j;
    mprAssert(nseg >= 0);
    if ((path = mprAlloc(len + nseg + 1)) != 0) {
        for (i = 0, dp = path; i < nseg; ) {
            strcpy(dp, segments[i]);
            len = (int) slen(segments[i]);
            dp += len;
            if (++i < nseg || (nseg == 1 && *segments[0] == '\0' && firstc == '/')) {
                *dp++ = '/';
            }
        }
        *dp = '\0';
    }
    return path;
}


HttpUri *httpResolveUri(HttpUri *base, int argc, HttpUri **others, bool local)
{
    HttpUri     *current, *other;
    int         i;

    if ((current = httpCloneUri(base, 0)) == 0) {
        return 0;
    }
    if (local) {
        current->host = 0;
        current->scheme = 0;
        current->port = 0;
    }
    /*
        Must not inherit the query or reference
     */
    current->query = NULL;
    current->reference = NULL;

    for (i = 0; i < argc; i++) {
        other = others[i];
        if (other->scheme) {
            current->scheme = sclone(other->scheme);
        }
        if (other->host) {
            current->host = sclone(other->host);
        }
        if (other->port) {
            current->port = other->port;
        }
        if (other->path) {
            trimPathToDirname(current);
            httpJoinUriPath(current, current, other);
        }
        if (other->reference) {
            current->reference = sclone(other->reference);
        }
        if (other->query) {
            current->query = sclone(other->query);
        }
    }
    current->ext = mprGetPathExt(current->path);
    return current;
}


char *httpUriToString(HttpUri *uri, int complete)
{
    return httpFormatUri(uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, complete);
}


static int getPort(HttpUri *uri)
{
    if (uri->port) {
        return uri->port;
    }
    return (uri->scheme && scmp(uri->scheme, "https") == 0) ? 443 : 80;
}


static int getDefaultPort(cchar *scheme)
{
    return (scheme && scmp(scheme, "https") == 0) ? 443 : 80;
}


static void trimPathToDirname(HttpUri *uri) 
{
    char        *path, *cp;
    int         len;

    path = uri->path;
    len = (int) slen(path);
    if (path[len - 1] == '/') {
        if (len > 1) {
            path[len - 1] = '\0';
        }
    } else {
        if ((cp = srchr(path, '/')) != 0) {
            if (cp > path) {
                *cp = '\0';
            } else {
                cp[1] = '\0';
            }
        } else if (*path) {
            path[0] = '\0';
        }
    }
}


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
/************************************************************************/
/*
 *  End of file "./src/uri.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "./src/var.c"
 */
/************************************************************************/

/*
    var.c -- Manage the request variables
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/*
    Define standard CGI variables
 */
void httpCreateCGIParams(HttpConn *conn)
{
    HttpRx          *rx;
    HttpTx          *tx;
    HttpHost        *host;
    HttpUploadFile  *up;
    MprSocket       *sock;
    MprHash         *vars, *svars;
    MprKey          *kp;
    int             index;

    rx = conn->rx;
    if ((svars = rx->svars) != 0) {
        /* Do only once */
        return;
    }
    svars = rx->svars = mprCreateHash(HTTP_MED_HASH_SIZE, 0);
    tx = conn->tx;
    host = conn->host;
    sock = conn->sock;

    mprAddKey(svars, "AUTH_TYPE", rx->authType);
    mprAddKey(svars, "AUTH_USER", conn->authUser);
    mprAddKey(svars, "AUTH_ACL", MPR->emptyString);
    mprAddKey(svars, "CONTENT_LENGTH", rx->contentLength);
    mprAddKey(svars, "CONTENT_TYPE", rx->mimeType);
    mprAddKey(svars, "DOCUMENT_ROOT", rx->route->dir);
    mprAddKey(svars, "GATEWAY_INTERFACE", sclone("CGI/1.1"));
    mprAddKey(svars, "QUERY_STRING", rx->parsedUri->query);
    mprAddKey(svars, "REMOTE_ADDR", conn->ip);
    mprAddKeyFmt(svars, "REMOTE_PORT", "%d", conn->port);
    mprAddKey(svars, "REMOTE_USER", conn->authUser);
    mprAddKey(svars, "REQUEST_METHOD", rx->method);
    mprAddKey(svars, "REQUEST_TRANSPORT", sclone((char*) ((conn->secure) ? "https" : "http")));
    mprAddKey(svars, "SERVER_ADDR", sock->acceptIp);
    mprAddKey(svars, "SERVER_NAME", host->name);
    mprAddKeyFmt(svars, "SERVER_PORT", "%d", sock->acceptPort);
    mprAddKey(svars, "SERVER_PROTOCOL", conn->protocol);
    mprAddKey(svars, "SERVER_ROOT", host->home);
    mprAddKey(svars, "SERVER_SOFTWARE", conn->http->software);
    /*
        For PHP, REQUEST_URI must be the original URI. The SCRIPT_NAME will refer to the new pathInfo
     */
    mprAddKey(svars, "REQUEST_URI", rx->originalUri);
    /*  
        URIs are broken into the following: http://{SERVER_NAME}:{SERVER_PORT}{SCRIPT_NAME}{PATH_INFO} 
        NOTE: Appweb refers to pathInfo as the app relative URI and scriptName as the app address before the pathInfo.
        In CGI|PHP terms, the scriptName is the appweb rx->pathInfo and the PATH_INFO is the extraPath. 
     */
    mprAddKey(svars, "PATH_INFO", rx->extraPath);
    mprAddKeyFmt(svars, "SCRIPT_NAME", "%s%s", rx->scriptName, rx->pathInfo);
    mprAddKey(svars, "SCRIPT_FILENAME", tx->filename);
    if (rx->extraPath) {
        /*  
            Only set PATH_TRANSLATED if extraPath is set (CGI spec) 
         */
        mprAssert(rx->extraPath[0] == '/');
        mprAddKey(svars, "PATH_TRANSLATED", mprNormalizePath(sfmt("%s%s", rx->route->dir, rx->extraPath)));
    }

    if (rx->files) {
        vars = httpGetParams(conn);
        mprAssert(vars);
        for (index = 0, kp = 0; (kp = mprGetNextKey(conn->rx->files, kp)) != 0; index++) {
            up = (HttpUploadFile*) kp->data;
            mprAddKey(vars, sfmt("FILE_%d_FILENAME", index), up->filename);
            mprAddKey(vars, sfmt("FILE_%d_CLIENT_FILENAME", index), up->clientFilename);
            mprAddKey(vars, sfmt("FILE_%d_CONTENT_TYPE", index), up->contentType);
            mprAddKey(vars, sfmt("FILE_%d_NAME", index), kp->key);
            mprAddKeyFmt(vars, sfmt("FILE_%d_SIZE", index), "%d", up->size);
        }
    }
    if (conn->http->envCallback) {
        conn->http->envCallback(conn);
    }
}


/*  
    Add variables to the vars environment store. This comes from the query string and urlencoded post data.
    Make variables for each keyword in a query string. The buffer must be url encoded (ie. key=value&key2=value2..., 
    spaces converted to '+' and all else should be %HEX encoded).
 */
static void addParamsFromBuf(HttpConn *conn, cchar *buf, ssize len)
{
    MprHash     *vars;
    cchar       *oldValue;
    char        *newValue, *decoded, *keyword, *value, *tok;

    mprAssert(conn);
    vars = httpGetParams(conn);
    decoded = mprAlloc(len + 1);
    decoded[len] = '\0';
    memcpy(decoded, buf, len);

    keyword = stok(decoded, "&", &tok);
    while (keyword != 0) {
        if ((value = strchr(keyword, '=')) != 0) {
            *value++ = '\0';
            value = mprUriDecode(value);
        } else {
            value = "";
        }
        keyword = mprUriDecode(keyword);

        if (*keyword) {
            /*  
                Append to existing keywords
             */
            oldValue = mprLookupKey(vars, keyword);
            if (oldValue != 0 && *oldValue) {
                if (*value) {
                    newValue = sjoin(oldValue, " ", value, NULL);
                    mprAddKey(vars, keyword, newValue);
                }
            } else {
                mprAddKey(vars, keyword, sclone(value));
            }
        }
        keyword = stok(0, "&", &tok);
    }
}


static void addParamsFromQueue(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *content;

    mprAssert(q);
    
    conn = q->conn;
    rx = conn->rx;

    if ((rx->form || rx->upload) && q->first && q->first->content) {
        httpJoinPackets(q, -1);
        content = q->first->content;
        mprAddNullToBuf(content);
        mprLog(6, "Form body data: length %d, \"%s\"", mprGetBufLength(content), mprGetBufStart(content));
        addParamsFromBuf(conn, mprGetBufStart(content), mprGetBufLength(content));
    }
}


static void addQueryParams(HttpConn *conn) 
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->parsedUri->query && !(rx->flags & HTTP_ADDED_QUERY_PARAMS)) {
        addParamsFromBuf(conn, rx->parsedUri->query, slen(rx->parsedUri->query));
        rx->flags |= HTTP_ADDED_QUERY_PARAMS;
    }
}


static void addBodyParams(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->form) {
        if (!(rx->flags & HTTP_ADDED_FORM_PARAMS)) {
            conn->readq = conn->tx->queue[HTTP_QUEUE_RX]->prevQ;
            addParamsFromQueue(conn->readq);
            rx->flags |= HTTP_ADDED_FORM_PARAMS;
        }
    }
}


void httpAddParams(HttpConn *conn)
{
    addQueryParams(conn);
    addBodyParams(conn);
}


MprHash *httpGetParams(HttpConn *conn)
{ 
    if (conn->rx->params == 0) {
        conn->rx->params = mprCreateHash(HTTP_MED_HASH_SIZE, 0);
    }
    return conn->rx->params;
}


//  MOB - sort file
int httpTestParam(HttpConn *conn, cchar *var)
{
    MprHash    *vars;
    
    vars = httpGetParams(conn);
    return vars && mprLookupKey(vars, var) != 0;
}


cchar *httpGetParam(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    MprHash     *vars;
    cchar       *value;
    
    vars = httpGetParams(conn);
    value = mprLookupKey(vars, var);
    return (value) ? value : defaultValue;
}


int httpGetIntParam(HttpConn *conn, cchar *var, int defaultValue)
{
    MprHash     *vars;
    cchar       *value;
    
    vars = httpGetParams(conn);
    value = mprLookupKey(vars, var);
    return (value) ? (int) stoi(value) : defaultValue;
}


static int sortParam(MprKey **h1, MprKey **h2)
{
    return scmp((*h1)->key, (*h2)->key);
}


/*
    Return the request parameters as a string. 
    This will return the exact same string regardless of the order of form parameters.
 */
char *httpGetParamsString(HttpConn *conn)
{
    HttpRx      *rx;
    MprHash     *params;
    MprKey      *kp;
    MprList     *list;
    char        *buf, *cp;
    ssize       len;
    int         next;

    mprAssert(conn);

    rx = conn->rx;

    if (rx->paramString == 0) {
        if ((params = conn->rx->params) != 0) {
            if ((list = mprCreateList(mprGetHashLength(params), 0)) != 0) {
                len = 0;
                for (kp = 0; (kp = mprGetNextKey(params, kp)) != NULL; ) {
                    mprAddItem(list, kp);
                    len += slen(kp->key) + slen(kp->data) + 2;
                }
                if ((buf = mprAlloc(len + 1)) != 0) {
                    mprSortList(list, sortParam);
                    cp = buf;
                    for (next = 0; (kp = mprGetNextItem(list, &next)) != 0; ) {
                        strcpy(cp, kp->key); cp += slen(kp->key);
                        *cp++ = '=';
                        strcpy(cp, kp->data); cp += slen(kp->data);
                        *cp++ = '&';
                    }
                    cp[-1] = '\0';
                    rx->paramString = buf;
                }
            }
        }
    }
    return rx->paramString;
}


void httpSetParam(HttpConn *conn, cchar *var, cchar *value) 
{
    MprHash     *vars;

    vars = httpGetParams(conn);
    mprAddKey(vars, var, sclone(value));
}


void httpSetIntParam(HttpConn *conn, cchar *var, int value) 
{
    MprHash     *vars;
    
    vars = httpGetParams(conn);
    mprAddKey(vars, var, sfmt("%d", value));
}


bool httpMatchParam(HttpConn *conn, cchar *var, cchar *value)
{
    if (strcmp(value, httpGetParam(conn, var, " __UNDEF__ ")) == 0) {
        return 1;
    }
    return 0;
}


void httpAddUploadFile(HttpConn *conn, cchar *id, HttpUploadFile *upfile)
{
    HttpRx   *rx;

    rx = conn->rx;
    if (rx->files == 0) {
        rx->files = mprCreateHash(-1, 0);
    }
    mprAddKey(rx->files, id, upfile);
}


void httpRemoveUploadFile(HttpConn *conn, cchar *id)
{
    HttpRx    *rx;
    HttpUploadFile  *upfile;

    rx = conn->rx;

    upfile = (HttpUploadFile*) mprLookupKey(rx->files, id);
    if (upfile) {
        mprDeletePath(upfile->filename);
        upfile->filename = 0;
    }
}


void httpRemoveAllUploadedFiles(HttpConn *conn)
{
    HttpRx          *rx;
    HttpUploadFile  *upfile;
    MprKey          *kp;

    rx = conn->rx;

    for (kp = 0; rx->files && (kp = mprGetNextKey(rx->files, kp)) != 0; ) {
        upfile = (HttpUploadFile*) kp->data;
        if (upfile->filename) {
            mprDeletePath(upfile->filename);
            upfile->filename = 0;
        }
    }
}

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
/************************************************************************/
/*
 *  End of file "./src/var.c"
 */
/************************************************************************/

