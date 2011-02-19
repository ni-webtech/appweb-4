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
 *  Start of file "../src/alias.c"
 */
/************************************************************************/

/*
    alias.c -- Alias service for aliasing URLs to file storage.

    This module supports the alias directives and mapping URLs to physical locations. 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageAlias(HttpAlias *alias, int flags);

/*
    Create an alias for a URI prefix. During processing, a request URI prefix is substituted to the target which
    may be either a physical path or a URI if a non-zero redirect code is supplied.
 */
HttpAlias *httpCreateAlias(cchar *prefix, cchar *target, int code)
{
    HttpAlias   *ap;
    cchar       *seps;
    int         len;

    mprAssert(prefix);
    mprAssert(target && *target);

    if ((ap = mprAllocObj(HttpAlias, manageAlias)) == NULL) {
        return 0;
    }
    ap->prefix = sclone(prefix);
    ap->prefixLen = (int) strlen(prefix);

    /*  
        Always strip trailing "/" from the prefix
     */
    if (ap->prefixLen > 0 && ap->prefix[ap->prefixLen - 1] == '/') {
        ap->prefix[--ap->prefixLen] = '\0';
    }
    if (code) {
        ap->redirectCode = code;
        ap->uri = sclone(target);
        ap->filename = sclone("");
    } else {
        /*  
            Trim trailing "/" from filename always
         */
        seps = mprGetPathSeparators(target);
        ap->filename = sclone(target);
        len = strlen(ap->filename) - 1;
        if (len >= 0 && ap->filename[len] == seps[0]) {
            ap->filename[len] = '\0';
        }
    }
    return ap;
}


static void manageAlias(HttpAlias *alias, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(alias->prefix);
        mprMark(alias->filename);
        mprMark(alias->uri);
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
/************************************************************************/
/*
 *  End of file "../src/alias.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/auth.c"
 */
/************************************************************************/

/*
    auth.c - Generic authorization code
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void manageAuth(HttpAuth *auth, int flags);


HttpAuth *httpCreateAuth(HttpAuth *parent)
{
    HttpAuth      *auth;

    auth = mprAllocObj(HttpAuth, manageAuth);
    if (parent) {
        auth->allow = parent->allow;
        auth->anyValidUser = parent->anyValidUser;
        auth->type = parent->type;
        auth->deny = parent->deny;
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
    auth->allow = sclone(allow);
}


void httpSetAuthAnyValidUser(HttpAuth *auth)
{
    auth->anyValidUser = 1;
    auth->flags |= HTTP_AUTH_REQUIRED;
}


void httpSetAuthDeny(HttpAuth *auth, cchar *deny)
{
    auth->deny = sclone(deny);
}


void httpSetAuthGroup(HttpConn *conn, cchar *group)
{
    conn->authGroup = sclone(group);
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
        return httpValidateNativeCredentials(auth, realm, user, password, requiredPass, msg);
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
        return httpGetNativePassword(auth, realm, user);
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
/************************************************************************/
/*
 *  End of file "../src/auth.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/authFile.c"
 */
/************************************************************************/

/*
    authFile.c - File based authorization using httpPassword files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if BLD_FEATURE_AUTH_FILE

static bool isUserValid(HttpAuth *auth, cchar *realm, cchar *user);
static void manageGroup(HttpGroup *group, int flags);
static void manageUser(HttpUser *user, int flags);


cchar *httpGetNativePassword(HttpAuth *auth, cchar *realm, cchar *user)
{
    HttpUser    *up;
    char        *key;

    up = 0;
    key = sjoin(realm, ":", user, NULL);
    if (auth->users) {
        up = (HttpUser*) mprLookupHash(auth->users, key);
    }
    if (up == 0) {
        return 0;
    }
    return up->password;
}


bool httpValidateNativeCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, cchar *requiredPassword, 
    char **msg)
{
    char    passbuf[HTTP_MAX_PASS * 2], *hashedPassword;
    ssize   len;

    hashedPassword = 0;
    
    if (auth->type == HTTP_AUTH_BASIC) {
        mprSprintf(passbuf, sizeof(passbuf), "%s:%s:%s", user, realm, password);
        len = strlen(passbuf);
        hashedPassword = mprGetMD5Hash(passbuf, len, NULL);
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
        return mprLookupHash(auth->users, key) != 0;
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
                gp = (HttpGroup*) mprLookupHash(auth->groups, group);
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
        up = (HttpUser*) mprLookupHash(auth->users, key);
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
    if (mprLookupHash(auth->groups, group)) {
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
    if (mprLookupHash(auth->users, key)) {
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

    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    tok = NULL;
    users = sclone(userList);
    user = stok(users, " \t", &tok);
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

    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
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
    if (auth->users == 0 || (up = (HttpUser*) mprLookupHash(auth->users, key)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    up->enabled = 0;
    return 0;
}


int httpEnableGroup(HttpAuth *auth, cchar *group)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
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
    if (auth->users == 0 || (up = (HttpUser*) mprLookupHash(auth->users, key)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    up->enabled = 1;
    return 0;
}


HttpAcl httpGetGroupAcl(HttpAuth *auth, char *group)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return gp->acl;
}


bool httpIsGroupEnabled(HttpAuth *auth, cchar *group)
{
    HttpGroup     *gp;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
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
    if (auth->users == 0 || (up = (HttpUser*) mprLookupHash(auth->users, key)) == 0) {
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
    MprHash     *groupHash, *userHash;
    HttpUser      *user;
    HttpGroup     *gp;
    
    /*
        Reset the ACL for each user
     */
    if (auth->users != 0) {
        for (userHash = 0; (userHash = mprGetNextHash(auth->users, userHash)) != 0; ) {
            ((HttpUser*) userHash->data)->acl = 0;
        }
    }

    /*
        Get the union of all ACLs defined for a user over all groups that the user is a member of.
     */
    if (auth->groups != 0 && auth->users != 0) {
        for (groupHash = 0; (groupHash = mprGetNextHash(auth->groups, groupHash)) != 0; ) {
            gp = ((HttpGroup*) groupHash->data);
            for (userHash = 0; (userHash = mprGetNextHash(auth->users, userHash)) != 0; ) {
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
    if (auth->groups == 0 || !mprLookupHash(auth->groups, group)) {
        return MPR_ERR_CANT_ACCESS;
    }
    mprRemoveHash(auth->groups, group);
    return 0;
}


int httpRemoveUser(HttpAuth *auth, cchar *realm, cchar *user)
{
    char    *key;

    key = sjoin(realm, ":", user, NULL);
    if (auth->users == 0 || !mprLookupHash(auth->users, key)) {
        return MPR_ERR_CANT_ACCESS;
    }
    mprRemoveHash(auth->users, key);
    return 0;
}


int httpRemoveUsersFromGroup(HttpAuth *auth, cchar *group, cchar *userList)
{
    HttpGroup   *gp;
    char        *user, *users, *tok;

    gp = 0;
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
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
    if (auth->groups == 0 || (gp = (HttpGroup*) mprLookupHash(auth->groups, group)) == 0) {
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


int httpReadGroupFile(Http *http, HttpAuth *auth, char *path)
{
    MprFile     *file;
    HttpAcl     acl;
    char        *buf;
    char        *users, *group, *enabled, *aclSpec, *tok, *cp;

    auth->groupFile = sclone(path);

    if ((file = mprOpenFile(path, O_RDONLY | O_TEXT, 0444)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    while ((buf = mprGetFileString(file, MPR_BUFSIZE, NULL)) != NULL) {
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


int httpReadUserFile(Http *http, HttpAuth *auth, char *path)
{
    MprFile     *file;
    char        *buf;
    char        *enabled, *user, *password, *realm, *tok, *cp;

    auth->userFile = sclone(path);

    if ((file = mprOpenFile(path, O_RDONLY | O_TEXT, 0444)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    while ((buf = mprGetFileString(file, MPR_BUFSIZE, NULL)) != NULL) {
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


int httpWriteUserFile(Http *http, HttpAuth *auth, char *path)
{
    MprFile         *file;
    MprHash         *hp;
    HttpUser        *up;
    char            buf[HTTP_MAX_PASS * 2];
    char            *tempFile;

    tempFile = mprGetTempPath(NULL);
    if ((file = mprOpenFile(tempFile, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0444)) == 0) {
        mprError("Can't open %s", tempFile);
        return MPR_ERR_CANT_OPEN;
    }
    hp = mprGetNextHash(auth->users, 0);
    while (hp) {
        up = (HttpUser*) hp->data;
        mprSprintf(buf, sizeof(buf), "%d: %s: %s: %s\n", up->enabled, up->name, up->realm, up->password);
        mprWriteFile(file, buf, (int) strlen(buf));
        hp = mprGetNextHash(auth->users, hp);
    }
    mprCloseFile(file);
    unlink(path);
    if (rename(tempFile, path) < 0) {
        mprError("Can't create new %s", path);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


int httpWriteGroupFile(Http *http, HttpAuth *auth, char *path)
{
    MprHash         *hp;
    MprFile         *file;
    HttpGroup       *gp;
    char            buf[MPR_MAX_STRING], *tempFile, *name;
    int             next;

    tempFile = mprGetTempPath(NULL);
    if ((file = mprOpenFile(tempFile, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0444)) == 0) {
        mprError("Can't open %s", tempFile);
        return MPR_ERR_CANT_OPEN;
    }

    hp = mprGetNextHash(auth->groups, 0);
    while (hp) {
        gp = (HttpGroup*) hp->data;
        mprSprintf(buf, sizeof(buf), "%d: %x: %s: ", gp->enabled, gp->acl, gp->name);
        mprWriteFile(file, buf, strlen(buf));
        for (next = 0; (name = mprGetNextItem(gp->users, &next)) != 0; ) {
            mprWriteFile(file, name, strlen(name));
        }
        mprWriteFile(file, "\n", 1);
        hp = mprGetNextHash(auth->groups, hp);
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
/************************************************************************/
/*
 *  End of file "../src/authFile.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/authFilter.c"
 */
/************************************************************************/

/*
    authFilter.c - Authorization filter for basic and digest authentication.
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
static char *createDigestNonce(cchar *secret, cchar *etag, cchar *realm);
static void decodeBasicAuth(HttpConn *conn, AuthData *ad);
static int  decodeDigestDetails(HttpConn *conn, AuthData *ad);
static void formatAuthResponse(HttpConn *conn, HttpAuth *auth, int code, char *msg, char *logMsg);
static bool matchAuth(HttpConn *conn, HttpStage *handler);
static int parseDigestNonce(char *nonce, cchar **secret, cchar **etag, cchar **realm, MprTime *when);


int httpOpenAuthFilter(Http *http)
{
    HttpStage     *filter;

    if ((filter = httpCreateFilter(http, "authFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->authFilter = filter;
    filter->match = matchAuth; 
    return 0;
}


static bool matchAuth(HttpConn *conn, HttpStage *handler)
{
    Http        *http;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpAuth    *auth;
    AuthData    *ad;
    MprTime     when;
    cchar       *requiredPassword;
    char        *msg, *requiredDigest;
    cchar       *secret, *etag, *realm;
    int         actualAuthType;

    rx = conn->rx;
    tx = conn->tx;
    http = conn->http;
    auth = rx->auth;

    if (!conn->server || auth == 0 || auth->type == 0) {
        return 0;
    }
    if ((ad = mprAllocObj(AuthData, NULL)) == 0) {
        return 1;
    }
    //  MOB -- fix
#if UNUSED
    if (auth == 0) {
        httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access Denied, Authorization not enabled");
        return 1;
    }
    if (auth->type == 0) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Authorization required.", 0);
        return 1;
    }
#endif
    if (rx->authDetails == 0) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Missing authorization details.", 0);
        return 1;
    }
    if (scasecmp(rx->authType, "basic") == 0) {
        decodeBasicAuth(conn, ad);
        actualAuthType = HTTP_AUTH_BASIC;

    } else if (scasecmp(rx->authType, "digest") == 0) {
        if (decodeDigestDetails(conn, ad) < 0) {
            httpError(conn, 400, "Bad authorization header");
            return 1;
        }
        actualAuthType = HTTP_AUTH_DIGEST;
    } else {
        actualAuthType = HTTP_AUTH_UNKNOWN;
    }
    mprLog(4, "run: type %d, url %s\nDetails %s\n", auth->type, rx->pathInfo, rx->authDetails);

    if (ad->userName == 0) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Missing user name", 0);
        return 1;
    }
    if (auth->type != actualAuthType) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access Denied, Wrong authentication protocol", 0);
        return 1;
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
            return 1;
        }
        calcDigest(&requiredDigest, 0, requiredPassword, ad->realm, rx->pathInfo, ad->nonce, ad->qop, ad->nc, 
            ad->cnonce, rx->method);
        requiredPassword = requiredDigest;

        /*
            Validate the nonce value - prevents replay attacks
         */
        when = 0; secret = 0; etag = 0; realm = 0;
        parseDigestNonce(ad->nonce, &secret, &etag, &realm, &when);
        if (scmp(secret, http->secret) != 0 || scmp(etag, tx->etag) != 0 || scmp(realm, auth->requiredRealm) != 0) {
            formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access denied, authentication error", "Nonce mismatch");
        } else if ((when + (5 * 60 * MPR_TICKS_PER_SEC)) < http->now) {
            formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access denied, authentication error", "Nonce is stale");
        }
    }
    if (!(http->validateCred)(auth, auth->requiredRealm, ad->userName, ad->password, requiredPassword, &msg)) {
        formatAuthResponse(conn, auth, HTTP_CODE_UNAUTHORIZED, "Access denied, authentication error", msg);
    }
    return 1;
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
            } else if (scasecmp(key, "user") == 0) {
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
    HttpTx  *tx;
    char    *qopClass, *nonce, *etag;

    tx = conn->tx;
    if (logMsg == 0) {
        logMsg = msg;
    }
    mprLog(3, "Auth response: code %d, %s", code, logMsg);

    if (auth->type == HTTP_AUTH_BASIC) {
        httpSetHeader(conn, "WWW-Authenticate", "Basic realm=\"%s\"", auth->requiredRealm);

    } else if (auth->type == HTTP_AUTH_DIGEST) {
        qopClass = auth->qop;
        etag = tx->etag ? tx->etag : "";
        nonce = createDigestNonce(conn->http->secret, etag, auth->requiredRealm);

        if (scmp(qopClass, "auth") == 0) {
            httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", domain=\"%s\", "
                "qop=\"auth\", nonce=\"%s\", opaque=\"%s\", algorithm=\"MD5\", stale=\"FALSE\"", 
                auth->requiredRealm, conn->server->name, nonce, etag);

        } else if (scmp(qopClass, "auth-int") == 0) {
            httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", domain=\"%s\", "
                "qop=\"auth\", nonce=\"%s\", opaque=\"%s\", algorithm=\"MD5\", stale=\"FALSE\"", 
                auth->requiredRealm, conn->server->name, nonce, etag);

        } else {
            httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", nonce=\"%s\"", auth->requiredRealm, nonce);
        }
    }
    httpError(conn, code, "Authentication Error: %s", msg);
    httpSetPipeHandler(conn, conn->http->passHandler);
}


/*
    Create a nonce value for digest authentication (RFC 2617)
 */ 
static char *createDigestNonce(cchar *secret, cchar *etag, cchar *realm)
{
    MprTime     now;
    char        nonce[256];

    mprAssert(realm && *realm);

    now = mprGetTime();
    mprSprintf(nonce, sizeof(nonce), "%s:%s:%s:%Lx", secret, etag, realm, now);
    return mprEncode64(nonce);
}


static int parseDigestNonce(char *nonce, cchar **secret, cchar **etag, cchar **realm, MprTime *when)
{
    char    *tok, *decoded, *whenStr;

    if ((decoded = mprDecode64(nonce)) == 0) {
        return MPR_ERR_CANT_READ;
    }
    *secret = stok(decoded, ":", &tok);
    *etag = stok(NULL, ":", &tok);
    *realm = stok(NULL, ":", &tok);
    whenStr = stok(NULL, ":", &tok);
    *when = (MprTime) stoi(whenStr, 16, NULL); 
    return 0;
}


static char *md5(cchar *string)
{
    return mprGetMD5Hash(string, slen(string), NULL);
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
        ha1 = md5(a1Buf);
    }

    /*
        HA2
     */ 
    mprSprintf(a2Buf, sizeof(a2Buf), "%s:%s", method, uri);
    ha2 = md5(a2Buf);

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
    *digest = md5(digestBuf);
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
/************************************************************************/
/*
 *  End of file "../src/authFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/authPam.c"
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
/************************************************************************/
/*
 *  End of file "../src/authPam.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/chunkFilter.c"
 */
/************************************************************************/

/*
    chunkFilter.c - Transfer chunk endociding filter.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void incomingChunkData(HttpQueue *q, HttpPacket *packet);
static bool matchChunk(HttpConn *conn, HttpStage *handler);
static void openChunk(HttpQueue *q);
static void outgoingChunkService(HttpQueue *q);
static void setChunkPrefix(HttpQueue *q, HttpPacket *packet);

/* 
   Loadable module initialization
 */
int httpOpenChunkFilter(Http *http)
{
    HttpStage     *filter;

    if ((filter = httpCreateFilter(http, "chunkFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->chunkFilter = filter;
    filter->open = openChunk; 
    filter->match = matchChunk; 
    filter->outgoingService = outgoingChunkService; 
    filter->incomingData = incomingChunkData; 
    return 0;
}


static bool matchChunk(HttpConn *conn, HttpStage *handler)
{
    return (conn->tx->length <= 0) ? 1 : 0;
}


static void openChunk(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;

    conn = q->conn;
    rx = conn->rx;

    q->packetSize = min(conn->limits->chunkSize, q->max);
    rx->chunkState = HTTP_CHUNK_START;
}


/*  
    Get the next chunk size. Chunked data format is:
        Chunk spec <CRLF>
        Data <CRLF>
        Chunk spec (size == 0) <CRLF>
        <CRLF>
    Chunk spec is: "HEX_COUNT; chunk length DECIMAL_COUNT\r\n". The "; chunk length DECIMAL_COUNT is optional.
    As an optimization, use "\r\nSIZE ...\r\n" as the delimiter so that the CRLF after data does not special consideration.
    Achive this by parseHeaders reversing the input start by 2.
 */
static void incomingChunkData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *buf;
    char        *start, *cp;
    int         bad;

    conn = q->conn;
    rx = conn->rx;

    if (!(rx->flags & HTTP_CHUNKED)) {
        httpSendPacketToNext(q, packet);
        return;
    }
    buf = packet->content;

    if (packet->content == 0) {
        if (rx->chunkState == HTTP_CHUNK_DATA) {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad chunk state");
            return;
        }
        rx->chunkState = HTTP_CHUNK_EOF;
    }
    
    /*  
        NOTE: the request head ensures that packets are correctly sized by packet inspection. The packet will never
        have more data than the chunk state expects.
     */
    switch (rx->chunkState) {
    case HTTP_CHUNK_START:
        /*  
            Validate:  "\r\nSIZE.*\r\n"
         */
        if (mprGetBufLength(buf) < 5) {
            break;
        }
        start = mprGetBufStart(buf);
        bad = (start[0] != '\r' || start[1] != '\n');
        for (cp = &start[2]; cp < buf->end && *cp != '\n'; cp++) {}
        if (*cp != '\n' && (cp - start) < 80) {
            break;
        }
        bad += (cp[-1] != '\r' || cp[0] != '\n');
        if (bad) {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return;
        }
        rx->chunkSize = (int) stoi(&start[2], 16, NULL);
        if (!isxdigit((int) start[2]) || rx->chunkSize < 0) {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return;
        }
        mprAdjustBufStart(buf, (cp - start + 1));
        rx->remainingContent = rx->chunkSize;
        if (rx->chunkSize == 0) {
            rx->chunkState = HTTP_CHUNK_EOF;
            /*
                We are lenient if the request does not have a trailing "\r\n" after the last chunk
             */
            cp = mprGetBufStart(buf);
            if (mprGetBufLength(buf) == 2 && *cp == '\r' && cp[1] == '\n') {
                mprAdjustBufStart(buf, 2);
            }
        } else {
            rx->chunkState = HTTP_CHUNK_DATA;
        }
        mprAssert(mprGetBufLength(buf) == 0);
        mprLog(5, "chunkFilter: start incoming chunk of %d bytes", rx->chunkSize);
        break;

    case HTTP_CHUNK_DATA:
        mprAssert(httpGetPacketLength(packet) <= rx->chunkSize);
        mprLog(5, "chunkFilter: data %d bytes, rx->remainingContent %d", httpGetPacketLength(packet), 
            rx->remainingContent);
        httpSendPacketToNext(q, packet);
        if (rx->remainingContent == 0) {
            rx->chunkState = HTTP_CHUNK_START;
            rx->remainingContent = HTTP_BUFSIZE;
        }
        break;

    case HTTP_CHUNK_EOF:
        mprAssert(httpGetPacketLength(packet) == 0);
        httpSendPacketToNext(q, packet);
        mprLog(5, "chunkFilter: last chunk");
        break;    

    default:
        mprAssert(0);
    }
}


/*  
    Apply chunks to dynamic outgoing data. 
 */
static void outgoingChunkService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;

    if (!(q->flags & HTTP_QUEUE_SERVICED)) {
        /*  
            If the last packet is the end packet, we have all the data. Thus we know the actual content length 
            and can bypass the chunk handler.
         */
        if (q->last->flags & HTTP_PACKET_END) {
            //  MOB -- but what if a content-length header has been defined but not set tx->length
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
    if (tx->chunkSize <= 0 || tx->altBody) {
        httpDefaultOutgoingServiceStage(q);
    } else {
        for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
            if (!(packet->flags & HTTP_PACKET_HEADER)) {
                httpJoinPackets(q, tx->chunkSize);
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
            httpSendPacketToNext(q, packet);
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
/************************************************************************/
/*
 *  End of file "../src/chunkFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/client.c"
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
    ssize       len;

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
        conn->authCnonce = mprAsprintf("%s:%s:%x", http->secret, conn->authRealm, (uint) mprGetTime()); 

        mprSprintf(a1Buf, sizeof(a1Buf), "%s:%s:%s", conn->authUser, conn->authRealm, conn->authPassword);
        len = strlen(a1Buf);
        ha1 = mprGetMD5Hash(a1Buf, len, NULL);
        mprSprintf(a2Buf, sizeof(a2Buf), "%s:%s", tx->method, parsedUri->path);
        len = strlen(a2Buf);
        ha2 = mprGetMD5Hash(a2Buf, len, NULL);
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
        digest = mprGetMD5Hash(digestBuf, strlen(digestBuf), NULL);

        if (*qop == '\0') {
            httpAddHeader(conn, "Authorization", "Digest user=\"%s\", realm=\"%s\", nonce=\"%s\", "
                "uri=\"%s\", response=\"%s\"",
                conn->authUser, conn->authRealm, conn->authNonce, parsedUri->path, digest);

        } else if (strcmp(qop, "auth") == 0) {
            httpAddHeader(conn, "Authorization", "Digest user=\"%s\", realm=\"%s\", domain=\"%s\", "
                "algorithm=\"MD5\", qop=\"%s\", cnonce=\"%s\", nc=\"%08x\", nonce=\"%s\", opaque=\"%s\", "
                "stale=\"FALSE\", uri=\"%s\", response=\"%s\"",
                conn->authUser, conn->authRealm, conn->authDomain, conn->authQop, conn->authCnonce, conn->authNc,
                conn->authNonce, conn->authOpaque, parsedUri->path, digest);

        } else if (strcmp(qop, "auth-int") == 0) {
            ;
        }
        conn->sentCredentials = 1;
    }
    httpSetSimpleHeader(conn, "Host", conn->ip);

    if (strcmp(conn->protocol, "HTTP/1.1") == 0) {
        /* If zero, we ask the client to close one request early. This helps with client led closes */
        if (conn->keepAliveCount > 0) {
            httpSetSimpleHeader(conn, "Connection", "Keep-Alive");
        } else {
            httpSetSimpleHeader(conn, "Connection", "close");
        }

    } else {
        /* Set to zero to let the client initiate the close */
        conn->keepAliveCount = 0;
        httpSetSimpleHeader(conn, "Connection", "close");
    }
    return 0;
}


int httpConnect(HttpConn *conn, cchar *method, cchar *url)
{
    mprAssert(conn);
    mprAssert(method && *method);
    mprAssert(url && *url);

    if (conn->server) {
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
    conn->startTime = mprGetTime();
    conn->startTicks = mprGetTicks();
#endif

    if (openConnection(conn, url) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
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
    HttpTx      *tx;

    mprAssert(conn->rx);

    *url = 0;
    rx = conn->rx;
    tx = conn->tx;

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
    conn->boundary = mprAsprintf("--BOUNDARY--%Ld", conn->http->now);
    httpSetHeader(conn, "Content-Type", "multipart/form-data; boundary=%s", &conn->boundary[2]);
}


static int blockingFileCopy(HttpConn *conn, cchar *path)
{
    MprFile     *file;
    char        buf[MPR_BUFSIZE];
    ssize       bytes;

    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        mprError("Can't open %s", path);
        return MPR_ERR_CANT_OPEN;
    }
    while ((bytes = mprReadFile(file, buf, sizeof(buf))) > 0) {
        if (httpWriteBlock(conn->writeq, buf, bytes) != bytes) {
            mprCloseFile(file);
            return MPR_ERR_CANT_WRITE;
        }
    }
    mprCloseFile(file);
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
            rc += httpWrite(conn->writeq, "Content-Type: %s\r\n\r\n", mprLookupMime(conn->host->mimeTypes, path));
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
/************************************************************************/
/*
 *  End of file "../src/client.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/conn.c"
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
HttpConn *httpCreateConn(Http *http, HttpServer *server, MprDispatcher *dispatcher)
{
    HttpConn    *conn;
    HttpHost    *host;

    if ((conn = mprAllocObj(HttpConn, manageConn)) == 0) {
        return 0;
    }
    conn->http = http;
    conn->canProceed = 1;

    conn->protocol = http->protocol;
    conn->port = -1;
    conn->retries = HTTP_RETRIES;
    conn->server = server;
    conn->lastActivity = http->now;
    conn->ioCallback = httpEvent;

    if (server) {
        conn->notifier = server->notifier;
        host = mprGetFirstItem(server->hosts);
        conn->limits = (host) ? host->limits : http->serverLimits;
    } else {
        conn->limits = http->clientLimits;
    }
    conn->serviceq = httpCreateQueueHead(conn, "serviceq");

    //  MOB -- this just sets to defaults. Who sets to what the config file has defined?
    httpInitTrace(conn->trace);
    if (dispatcher) {
        conn->dispatcher = dispatcher;
    } else if (server) {
        conn->dispatcher = server->dispatcher;
    } else {
        conn->dispatcher = mprGetDispatcher();
    }
    conn->keepAliveCount = (conn->limits->keepAliveCount) ? conn->limits->keepAliveCount : -1;
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
        if (conn->server) {
            httpValidateLimits(conn->server, HTTP_VALIDATE_CLOSE_CONN, conn);
        }
        if (HTTP_STATE_PARSED <= conn->state && conn->state < HTTP_STATE_COMPLETE) {
            HTTP_NOTIFY(conn, HTTP_STATE_COMPLETE, 0);
        }
        HTTP_NOTIFY(conn, -1, 0);
        httpRemoveConn(conn->http, conn);
        httpCloseConn(conn);
        conn->input = 0;
        if (conn->rx) {
            conn->rx->conn = 0;
            conn->rx = 0;
        }
        if (conn->tx) {
            conn->tx->conn = 0;
            conn->tx = 0;
        }
        conn->http = 0;
        // mprLog(0, "DEBUG: destroy/free conn %p", conn);
    }
}


static void manageConn(HttpConn *conn, int flags)
{
    mprAssert(conn);

    if (flags & MPR_MANAGE_MARK) {
        mprMark(conn->rx);
        mprMark(conn->tx);
        mprMark(conn->limits);
        mprMark(conn->http);
        mprMark(conn->stages);
        mprMark(conn->dispatcher);
        mprMark(conn->waitHandler);
        mprMark(conn->server);
        mprMark(conn->host);
        mprMark(conn->sock);
        mprMark(conn->serviceq);
        mprMark(conn->currentq);
        mprMark(conn->input);
        mprMark(conn->readq);
        mprMark(conn->writeq);
        mprMark(conn->context);
        mprMark(conn->boundary);
        mprMark(conn->errorMsg);
        mprMark(conn->host);
        mprMark(conn->ip);
        mprMark(conn->protocol);
        mprMark(conn->headersCallbackArg);

        httpManageTrace(&conn->trace[0], flags);
        httpManageTrace(&conn->trace[1], flags);

        mprMark(conn->authCnonce);
        mprMark(conn->authDomain);
        mprMark(conn->authNonce);
        mprMark(conn->authOpaque);
        mprMark(conn->authRealm);
        mprMark(conn->authQop);
        mprMark(conn->authType);
        mprMark(conn->authGroup);
        mprMark(conn->authUser);
        mprMark(conn->authPassword);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyConn(conn);
    }
}


/*  
    Close the connection but don't destroy the conn object.
    WARNING: Once this is called, you can't get wait handler events. So handlers must not call this. 
    Rather, handlers should call mprDisconnectSocket that will cause a readable event to come and readEvent can
    then do an orderly close and free the connection structure.
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


/*  
    Prepare a connection for a new request after completing a prior request.
 */
void httpPrepServerConn(HttpConn *conn)
{
    mprAssert(conn);

    if (conn->state != HTTP_STATE_BEGIN) {
        conn->abortPipeline = 0;
        conn->canProceed = 1;
        conn->complete = 0;
        conn->connError = 0;
        conn->error = 0;
        conn->errorMsg = 0;
        conn->flags = 0;
        conn->state = 0;
        conn->readq = 0;
        conn->writeq = 0;
        conn->writeComplete = 0;
        conn->dispatcher = (conn->server) ? conn->server->dispatcher : mprGetDispatcher();
        httpSetState(conn, HTTP_STATE_BEGIN);
        httpInitSchedulerQueue(conn->serviceq);
        mprAssert(conn->rx == 0);
        mprAssert(conn->tx == 0);
    }
}


void httpPrepClientConn(HttpConn *conn, int keepHeaders)
{
    MprHashTable    *headers;

    mprAssert(conn);

    if (conn->keepAliveCount >= 0 && conn->sock) {
        /* Eat remaining input incase last request did not consume all data */
        httpConsumeLastRequest(conn);
    } else {
        conn->input = 0;
    }
    conn->abortPipeline = 0;
    conn->canProceed = 1;
    conn->complete = 0;
    conn->connError = 0;
    conn->error = 0;
    conn->errorMsg = 0;
    conn->flags = 0;
    conn->state = 0;
    conn->writeComplete = 0;

    if (conn->tx) {
        conn->tx->conn = 0;
    }
    headers = (keepHeaders && conn->tx) ? conn->tx->headers: NULL;
    conn->tx = httpCreateTx(conn, headers);
    if (conn->rx) {
        conn->rx->conn = 0;
    }
    conn->rx = httpCreateRx(conn);

    httpSetState(conn, HTTP_STATE_BEGIN);
    httpInitSchedulerQueue(conn->serviceq);
    httpCreatePipeline(conn, NULL, NULL);
}


void httpConsumeLastRequest(HttpConn *conn)
{
    MprTime     mark;
    char        junk[4096];
    int         requestTimeout;

    if (!conn->sock || conn->state < HTTP_STATE_FIRST) {
        return;
    }
    mark = mprGetTime();
    requestTimeout = conn->limits->requestTimeout ? conn->limits->requestTimeout : INT_MAX;
    while (!httpIsEof(conn) && mprGetRemainingTime(mark, requestTimeout) > 0) {
        if (httpRead(conn, junk, sizeof(junk)) <= 0) {
            break;
        }
    }
    if (HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE) {
        conn->keepAliveCount = -1;
    }
}


void httpCallEvent(HttpConn *conn, int mask)
{
    MprEvent    e;

    e.mask = mask;
    e.timestamp = mprGetTime();
    httpEvent(conn, &e);
}


/*  
    IO event handler. This is invoked by the wait subsystem in response to I/O events. It is also invoked via relay
    when an accept event is received by the server. Initially the conn->dispatcher will be set to the server->dispatcher 
    and the first I/O event will be handled on the server thread (or main thread). A request handler may create a 
    new conn->dispatcher and transfer execution to a worker thread if required.
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
    if (conn->server) {
        if (conn->connError || mprIsSocketEof(conn->sock) || (!conn->rx && conn->keepAliveCount < 0)) {
            /*  
                NOTE: compare keepAliveCount with "< 0" so that the client can have one more keep alive request. 
                It should respond to the "Connection: close" and thus initiate a client-led close. 
                This reduces TIME_WAIT states on the server. 
             */
            httpDestroyConn(conn);
            /* NOTE: conn structure and memory is still intact but conn->sock is zero */
            return;
        }
    }
    httpEnableConnEvents(conn);
}


/*
    Process a socket readable event
 */
static void readEvent(HttpConn *conn)
{
    HttpPacket  *packet;
    ssize       nbytes, len;

    while ((packet = getPacket(conn, &len)) != 0) {
        nbytes = mprReadSocket(conn->sock, mprGetBufEnd(packet->content), len);
        LOG(8, "http: read event. Got %d", nbytes);
       
        if (nbytes > 0) {
            mprAdjustBufEnd(packet->content, nbytes);
            httpProcess(conn, packet);

        } else if (nbytes < 0) {
            if (conn->state <= HTTP_STATE_CONNECTED) {
                if (mprIsSocketEof(conn->sock)) {
                    conn->connError = conn->error = 1;
                }
                break;
            } else if (conn->state < HTTP_STATE_COMPLETE) {
                httpProcess(conn, packet);
                if (!conn->error && conn->state < HTTP_STATE_COMPLETE && mprIsSocketEof(conn->sock)) {
                    httpConnError(conn, HTTP_CODE_COMMS_ERROR, "Connection lost");
                    break;
                }
            }
            break;
        }
        if (nbytes == 0 || conn->connError || conn->state >= HTTP_STATE_RUNNING || conn->startingThread) {
            break;
        }
    }
}


static void writeEvent(HttpConn *conn)
{
    LOG(6, "httpProcessWriteEvent, state %d", conn->state);

    conn->writeBlocked = 0;
    if (conn->tx) {
        httpEnableQueue(conn->tx->queue[HTTP_QUEUE_TRANS]->prevQ);
        httpServiceQueues(conn);
        httpProcess(conn, NULL);
    }
}


void httpEnableConnEvents(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q;
    int         eventMask;

    mprLog(7, "EnableConnEvents");

    if (!conn->async) {
        return;
    }
    tx = conn->tx;
    eventMask = 0;
    conn->lastActivity = conn->http->now;

    if (conn->state < HTTP_STATE_COMPLETE && conn->sock && !mprIsSocketEof(conn->sock)) {
        if (tx) {
            if (tx->queue[HTTP_QUEUE_TRANS]->prevQ->count > 0) {
                eventMask |= MPR_WRITABLE;
            } else {
                mprAssert(!conn->writeBlocked);
            } 
            /*
                Allow read events even if the current request is not complete. The pipelined request will be buffered 
                and will be ready when the current request completes.
             */
            q = tx->queue[HTTP_QUEUE_RECEIVE]->nextQ;
            if (q->count < q->max) {
                eventMask |= MPR_READABLE;
            }
        } else {
            eventMask |= MPR_READABLE;
        }
        if (conn->startingThread) {
            conn->startingThread = 0;
            if (conn->waitHandler) {
                mprRemoveWaitHandler(conn->waitHandler);
                conn->waitHandler = 0;
            }
        }
        if (eventMask) {
            if (conn->waitHandler == 0) {
                conn->waitHandler = mprCreateWaitHandler(conn->sock->fd, eventMask, conn->dispatcher, 
                    (MprEventProc) conn->ioCallback, conn, 0);
            } else if (eventMask != conn->waitHandler->desiredMask) {
                mprEnableWaitEvents(conn->waitHandler, eventMask);
            }
        } else {
            if (conn->waitHandler && eventMask != conn->waitHandler->desiredMask) {
                mprEnableWaitEvents(conn->waitHandler, eventMask);
            }
        }
        mprEnableDispatcher(conn->dispatcher);
    }
}


void httpFollowRedirects(HttpConn *conn, bool follow)
{
    conn->followRedirects = follow;
}


/*  
    Get the packet into which to read data. This may be owned by the connection or if mid-request, may be owned by the
    request. Also return in *bytesToRead the length of data to attempt to read.
 */
static HttpPacket *getPacket(HttpConn *conn, ssize *bytesToRead)
{
    HttpPacket  *packet;
    MprBuf      *content;
    HttpRx      *req;
    ssize       len;

    req = conn->rx;
    len = HTTP_BUFSIZE;

    //  MOB -- simplify. Okay to lose some optimization for chunked data?
    /*  
        The input packet may have pipelined headers and data left from the prior request. It may also have incomplete
        chunk boundary data.
     */
    if ((packet = conn->input) == NULL) {
        conn->input = packet = httpCreatePacket(len);
    } else {
        content = packet->content;
        mprResetBufIfEmpty(content);
        if (req) {
            /*  
                Don't read more than the remainingContent unless chunked. We do this to minimize requests split 
                accross packets.
             */
            if (req->remainingContent) {
                len = req->remainingContent;
                if (req->flags & HTTP_CHUNKED) {
                    len = max(len, HTTP_BUFSIZE);
                }
            }
            len = min(len, HTTP_BUFSIZE);
            mprAssert(len > 0);
            if (mprGetBufSpace(content) < len) {
                mprGrowBuf(content, len);
            }

        } else {
            //  MOB -- but this logic is not in the oter "then" case above.
            /* Still reading the headers */
            if (mprGetBufLength(content) >= conn->limits->headerSize) {
                httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE, "Header too big");
                return 0;
            }
            if (mprGetBufSpace(content) < HTTP_BUFSIZE) {
                mprGrowBuf(content, HTTP_BUFSIZE);
            }
            len = mprGetBufSpace(content);
        }
    }
    mprAssert(packet == conn->input);
    mprAssert(len > 0);
    *bytesToRead = len;
    return packet;
}


void httpCompleteRequest(HttpConn *conn)
{
    httpFinalize(conn);
    conn->complete = 1;
}


/*
    Called by connectors when writing the transmission is complete
 */
void httpCompleteWriting(HttpConn *conn)
{
    conn->writeComplete = 1;
    if (conn->tx) {
        conn->tx->finalized = 1;
    }
    //  MOB -- id this necessary?
    httpDiscardTransmitData(conn);
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


void httpSetRequestNotifier(HttpConn *conn, HttpNotifier notifier)
{
    conn->requestNotifier = notifier;
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


void httpSetTimeout(HttpConn *conn, int requestTimeout, int inactivityTimeout)
{
    if (requestTimeout >= 0) {
        conn->limits->requestTimeout = requestTimeout * MPR_TICKS_PER_SEC;
    }
    if (inactivityTimeout >= 0) {
        conn->limits->inactivityTimeout = inactivityTimeout * MPR_TICKS_PER_SEC;
    }
}


HttpLimits *httpSetUniqueConnLimits(HttpConn *conn)
{
    HttpLimits      *limits;

    limits = mprAllocObj(HttpLimits, NULL);
    *limits = *conn->limits;
    conn->limits = limits;
    return limits;
}


void httpWritable(HttpConn *conn)
{
    HTTP_NOTIFY(conn, 0, HTTP_NOTIFY_WRITABLE);
}


void httpFormatErrorV(HttpConn *conn, int status, cchar *fmt, va_list args)
{
    /*
        Lock as this may be called by httpTimer
     */
    mprLock(conn->http->mutex);
    conn->errorMsg = mprAsprintfv(fmt, args);
    if (status) {
        if (conn->server && conn->tx) {
            conn->tx->status = status;
        } else if (conn->rx) {
            conn->rx->status = status;
        }
    }
    mprUnlock(conn->http->mutex);
}


/*
    Just format conn->errorMsg and set status - nothing more
 */
void httpFormatError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt); 
    httpFormatErrorV(conn, status, fmt, args);
    va_end(args); 
}


/*
    The current request has an error and cannot complete as normal. This call sets the Http response status and 
    overrides the normal output with an alternate error message. If the output has alread started (headers sent), then
    the connection MUST be closed so the client can get some indication the request failed.
 */
static void httpErrorV(HttpConn *conn, int status, cchar *fmt, va_list args)
{
    HttpRx      *rx;
    HttpTx      *tx;

    mprAssert(fmt);

    rx = conn->rx;

    if (!conn->error) {
        conn->error = 1;
        httpFormatErrorV(conn, status, fmt, args);
        if (rx == 0) {
            mprLog(2, "\"%s\", status %d: %s.", httpLookupStatus(conn->http, status), status, conn->errorMsg);
        } else {
            mprLog(2, "Error: \"%s\", status %d for URI \"%s\": %s.",
                httpLookupStatus(conn->http, status), status, rx->uri ? rx->uri : "", conn->errorMsg);
        }
        tx = conn->tx;
        if (tx) {
            if (conn->server) {
                if (tx->flags & HTTP_TX_HEADERS_CREATED) {
                    /* Headers and status have been sent, so must let the client know the request has failed */
                    mprDisconnectSocket(conn->sock);
                } else {
                    httpSetResponseBody(conn, status, conn->errorMsg);
                    httpFinalize(conn);
                }
            } else {
                if (tx->flags & HTTP_TX_HEADERS_CREATED) {
                    httpCloseConn(conn);
                }
            }
        }
    }
}


void httpError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    httpErrorV(conn, status, fmt, args);
    va_end(args);
}


/*
    A resource limit error has occurred. For servers: best to stop future requests on the current connection, but 
    try to send a meaningful response back to the client. For clients, just close the connection.
 */
void httpLimitError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    httpErrorV(conn, status, fmt, args);
    va_end(args);
    conn->complete = 1;
    conn->connError = 1;
    if (!conn->server) {
        /* Handlers must not call CloseConn as it disables wait events */
        httpCloseConn(conn);
    }
}


/*
    A HTTP protocol error has occurred. For servers: best to stop future requests on the current connection, but 
    try to send a meaningful response back to the client. For clients, just close the connection.
 */
void httpProtocolError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    if (!conn->connError) { 
        va_start(args, fmt);
        httpErrorV(conn, status, fmt, args);
        va_end(args);
        conn->complete = 1;
        conn->connError = 1;
        if (!conn->server) {
            /* Handlers must not call CloseConn as it disables wait events */
            httpCloseConn(conn);
        }
    }
}


/*
    The connection has been broken. Fail the current request and the processing pipeline. Close the connection.
 */
void httpConnError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    if (!conn->connError) { 
        va_start(args, fmt);
        httpErrorV(conn, status, fmt, args);
        va_end(args);
        conn->connError = 1;
        conn->complete = 1;
        conn->writeComplete = 1;
        conn->abortPipeline = 1;
        if (conn->server) {
            mprDisconnectSocket(conn->sock);
        } else {
            /* Handlers must not call CloseConn as it disables wait events */
            httpCloseConn(conn);
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
/************************************************************************/
/*
 *  End of file "../src/conn.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/dir.c"
 */
/************************************************************************/

/*
    dir.c -- Support authorization on a per-directory basis.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void manageDir(HttpDir *dir, int flags);


static void manageDir(HttpDir *dir, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(dir->auth);
        mprMark(dir->indexName);
        mprMark(dir->path);
    }
}


HttpDir *httpCreateBareDir(cchar *path)
{
    HttpDir   *dir;

    mprAssert(path);

    if ((dir = mprAllocObj(HttpDir, manageDir)) == 0) {
        return 0;
    }
    dir->indexName = sclone("index.html");
    dir->auth = httpCreateAuth(0);
    if (path) {
        dir->path = sclone(path);
        dir->pathLen = strlen(path);
    }
    return dir;
}


HttpDir *httpCreateDir(cchar *path, HttpDir *parent)
{
    HttpDir   *dir;

    mprAssert(path);
    mprAssert(parent);

    if ((dir = mprAllocObj(HttpDir, manageDir)) == 0) {
        return 0;
    }
    dir->indexName = sclone(parent->indexName);
    if (path == 0) {
        path = parent->path;
    }
    httpSetDirPath(dir, path);
    dir->auth = httpCreateAuth(parent->auth);
    return dir;
}


void httpSetDirPath(HttpDir *dir, cchar *fileName)
{
    mprAssert(dir);
    mprAssert(fileName);

    dir->path = mprGetAbsPath(fileName);
    dir->pathLen = strlen(dir->path);

    /*  Always strip trailing "/" */
    if (dir->pathLen > 0 && dir->path[dir->pathLen - 1] == '/') {
        dir->path[--dir->pathLen] = '\0';
    }
}


void httpSetDirIndex(HttpDir *dir, cchar *name) 
{ 
    mprAssert(dir);
    mprAssert(name && *name);

    dir->indexName = sclone(name); 
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
/************************************************************************/
/*
 *  End of file "../src/dir.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/env.c"
 */
/************************************************************************/

/*
    env.c -- Manage the request environment
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/*
    Define standard CGI environment variables
 */
void httpCreateEnvVars(HttpConn *conn)
{
    HttpRx          *rx;
    HttpTx          *tx;
    HttpHost        *host;
    HttpUploadFile  *up;
    HttpServer      *server;
    MprSocket       *sock;
    MprHashTable    *vars;
    MprHash         *hp;
    int             index;

    rx = conn->rx;
    tx = conn->tx;
    vars = rx->formVars;
    server = conn->server;
    host = conn->host;

    //  TODO - Vars for COOKIEs
    
    /*  Alias for REMOTE_USER. Define both for broader compatibility with CGI */
    mprAddKey(vars, "AUTH_TYPE", rx->authType);
    mprAddKey(vars, "AUTH_USER", conn->authUser);
    mprAddKey(vars, "AUTH_GROUP", conn->authGroup);
    mprAddKey(vars, "AUTH_ACL", MPR->emptyString);
    mprAddKey(vars, "CONTENT_LENGTH", rx->contentLength);
    mprAddKey(vars, "CONTENT_TYPE", rx->mimeType);
    mprAddKey(vars, "GATEWAY_INTERFACE", sclone("CGI/1.1"));
    mprAddKey(vars, "QUERY_STRING", rx->parsedUri->query);

    if (conn->sock) {
        mprAddKey(vars, "REMOTE_ADDR", conn->ip);
    }
    mprAddKeyFmt(vars, "REMOTE_PORT", "%d", conn->port);

    /*  Same as AUTH_USER (yes this is right) */
    mprAddKey(vars, "REMOTE_USER", conn->authUser);
    mprAddKey(vars, "REQUEST_METHOD", rx->method);
    mprAddKey(vars, "REQUEST_TRANSPORT", sclone((char*) ((conn->secure) ? "https" : "http")));
    
    sock = conn->sock;
    mprAddKey(vars, "SERVER_ADDR", sock->acceptIp);
    mprAddKey(vars, "SERVER_NAME", server->name);
    mprAddKeyFmt(vars, "SERVER_PORT", "%d", sock->acceptPort);

    /*  HTTP/1.0 or HTTP/1.1 */
    mprAddKey(vars, "SERVER_PROTOCOL", conn->protocol);
    mprAddKey(vars, "SERVER_SOFTWARE", server->http->software);

    /*  This is the complete URI before decoding */ 
    mprAddKey(vars, "REQUEST_URI", rx->uri);

    /*  URLs are broken into the following: http://{SERVER_NAME}:{SERVER_PORT}{SCRIPT_NAME}{PATH_INFO} */
    mprAddKey(vars, "PATH_INFO", rx->pathInfo);
    mprAddKey(vars, "SCRIPT_NAME", rx->scriptName);
    mprAddKey(vars, "SCRIPT_FILENAME", tx->filename);

    if (rx->pathTranslated) {
        /*  Only set PATH_TRANSLATED if PATH_INFO is set (CGI spec) */
        mprAddKey(vars, "PATH_TRANSLATED", rx->pathTranslated);
    }
    //  MOB -- how do these relate to MVC apps and non-mvc apps
    mprAddKey(vars, "DOCUMENT_ROOT", host->documentRoot);
    mprAddKey(vars, "SERVER_ROOT", host->serverRoot);

    if (rx->files) {
        for (index = 0, hp = 0; (hp = mprGetNextHash(conn->rx->files, hp)) != 0; index++) {
            up = (HttpUploadFile*) hp->data;
            mprAddKey(vars, mprAsprintf("FILE_%d_FILENAME", index), up->filename);
            mprAddKey(vars, mprAsprintf("FILE_%d_CLIENT_FILENAME", index), up->clientFilename);
            mprAddKey(vars, mprAsprintf("FILE_%d_CONTENT_TYPE", index), up->contentType);
            mprAddKey(vars, mprAsprintf("FILE_%d_NAME", index), hp->key);
            mprAddKeyFmt(vars, mprAsprintf("FILE_%d_SIZE", index), "%d", up->size);
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
void httpAddVars(HttpConn *conn, cchar *buf, ssize len)
{
    HttpRx          *rx;
    MprHashTable    *vars;
    cchar           *oldValue;
    char            *newValue, *decoded, *keyword, *value, *tok;

    rx = conn->rx;
    vars = rx->formVars;
    if (vars == 0) {
        return;
    }
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
                Append to existing keywords.
             */
            oldValue = mprLookupHash(vars, keyword);
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
    /*  Must not free "decoded". This will be freed when the response completes */
}


void httpAddVarsFromQueue(HttpQueue *q)
{
    HttpConn        *conn;
    MprBuf          *content;

    mprAssert(q);
    
    conn = q->conn;
    if (conn->rx->form && q->first && q->first->content) {
        content = q->first->content;
        mprAddNullToBuf(content);
        mprLog(3, "Form body data: length %d, \"%s\"", mprGetBufLength(content), mprGetBufStart(content));
        httpAddVars(conn, mprGetBufStart(content), mprGetBufLength(content));
    }
}


int httpTestFormVar(HttpConn *conn, cchar *var)
{
    MprHashTable    *vars;
    
    vars = conn->rx->formVars;
    if (vars == 0) {
        return 0;
    }
    return vars && mprLookupHash(vars, var) != 0;
}


cchar *httpGetFormVar(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    MprHashTable    *vars;
    cchar           *value;
    
    vars = conn->rx->formVars;
    if (vars) {
        value = mprLookupHash(vars, var);
        return (value) ? value : defaultValue;
    }
    return defaultValue;
}


int httpGetIntFormVar(HttpConn *conn, cchar *var, int defaultValue)
{
    MprHashTable    *vars;
    cchar           *value;
    
    vars = conn->rx->formVars;
    if (vars) {
        value = mprLookupHash(vars, var);
        return (value) ? (int) stoi(value, 10, NULL) : defaultValue;
    }
    return defaultValue;
}


void httpSetFormVar(HttpConn *conn, cchar *var, cchar *value) 
{
    MprHashTable    *vars;
    
    vars = conn->rx->formVars;
    if (vars == 0) {
        /* This is allowed. Upload filter uses this when uploading to the file handler */
        return;
    }
    mprAddKey(vars, var, sclone(value));
}


void httpSetIntFormVar(HttpConn *conn, cchar *var, int value) 
{
    MprHashTable    *vars;
    
    vars = conn->rx->formVars;
    if (vars == 0) {
        /* This is allowed. Upload filter uses this when uploading to the file handler */
        return;
    }
    mprAddKey(vars, var, mprAsprintf("%d", value));
}


int httpCompareFormVar(HttpConn *conn, cchar *var, cchar *value)
{
    MprHashTable    *vars;
    
    vars = conn->rx->formVars;
    
    if (vars == 0) {
        return 0;
    }
    if (strcmp(value, httpGetFormVar(conn, var, " __UNDEF__ ")) == 0) {
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

    upfile = (HttpUploadFile*) mprLookupHash(rx->files, id);
    if (upfile) {
        mprDeletePath(upfile->filename);
        upfile->filename = 0;
    }
}


void httpRemoveAllUploadedFiles(HttpConn *conn)
{
    HttpRx          *rx;
    HttpUploadFile  *upfile;
    MprHash         *hp;

    rx = conn->rx;

    for (hp = 0; rx->files && (hp = mprGetNextHash(rx->files, hp)) != 0; ) {
        upfile = (HttpUploadFile*) hp->data;
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
/************************************************************************/
/*
 *  End of file "../src/env.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/host.c"
 */
/************************************************************************/

/*
    host.c -- Host class for all HTTP hosts

    The Host class is used for the default HTTP server and for all virtual hosts (including SSL hosts).
    Many objects are controlled at the host level. Eg. URL handlers.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageHost(HttpHost *host, int flags);


static void manageHost(HttpHost *host, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(host->name);
        mprMark(host->parent);
        mprMark(host->aliases);
        mprMark(host->dirs);
        mprMark(host->locations);
        mprMark(host->loc);
        mprMark(host->mimeTypes);
        mprMark(host->documentRoot);
        mprMark(host->serverRoot);
        mprMark(host->ip);
        mprMark(host->traceInclude);
        mprMark(host->traceExclude);
        mprMark(host->protocol);
        mprMark(host->mutex);
        mprMark(host->log);
        mprMark(host->logFormat);
        mprMark(host->logPath);
        mprMark(host->limits);

    } else if (flags & MPR_MANAGE_FREE) {
        httpRemoveHost(MPR->httpService, host);
    }
}


HttpHost *httpCreateHost(cchar *ip, int port, HttpLoc *loc)
{
    HttpHost    *host;
    Http        *http;

    http = MPR->httpService;

    if ((host = mprAllocObj(HttpHost, manageHost)) == 0) {
        return 0;
    }
    host->name = mprAsprintf("%s:%d", ip ? ip : "*", port);
    host->mutex = mprCreateLock();
    host->aliases = mprCreateList(-1, 0);
    host->dirs = mprCreateList(-1, 0);
    host->locations = mprCreateList(-1, 0);
    host->limits = mprMemdup(http->serverLimits, sizeof(HttpLimits));
    host->ip = sclone(ip);
    host->port = port;
    host->flags = HTTP_HOST_NO_TRACE;
    host->protocol = sclone("HTTP/1.1");
    host->mimeTypes = MPR->mimeTypes;
    host->documentRoot = host->serverRoot = sclone(".");

    //  MOB -- not right
    host->traceMask = HTTP_TRACE_TX | HTTP_TRACE_RX | HTTP_TRACE_FIRST | HTTP_TRACE_HEADER;
    host->traceLevel = 3;
    host->traceMaxLength = INT_MAX;

    host->loc = (loc) ? loc : httpCreateLocation(http);
    httpAddLocation(host, host->loc);
    host->loc->auth = httpCreateAuth(host->loc->auth);
    httpAddHost(http, host);
    return host;
}


/*  
    Create a new virtual host and inherit settings from another host
 */
HttpHost *httpCreateVirtualHost(cchar *ip, int port, HttpHost *parent)
{
    HttpHost    *host;
    Http        *http;

    http = MPR->httpService;

    if ((host = mprAllocObj(HttpHost, manageHost)) == 0) {
        return 0;
    }
    host->mutex = mprCreateLock();
    host->parent = parent;

    /*  
        The aliases, dirs and locations are all copy-on-write
     */
    host->aliases = parent->aliases;
    host->dirs = parent->dirs;
    host->locations = parent->locations;
    host->flags = parent->flags | HTTP_HOST_VHOST;
    host->protocol = parent->protocol;
    host->mimeTypes = parent->mimeTypes;
    host->ip = parent->ip;
    host->port = parent->port;
    host->limits = mprMemdup(parent->limits, sizeof(HttpLimits));
    if (ip) {
        host->ip = sclone(ip);
    }
    if (port) {
        host->port = port;
    }
    host->loc = httpCreateInheritedLocation(parent->loc);
    host->traceMask = parent->traceMask;
    host->traceLevel = parent->traceLevel;
    host->traceMaxLength = parent->traceMaxLength;
    if (parent->traceInclude) {
        host->traceInclude = mprCloneHash(parent->traceInclude);
    }
    if (parent->traceExclude) {
        host->traceExclude = mprCloneHash(parent->traceExclude);
    }
    httpAddLocation(host, host->loc);
    httpAddHost(http, host);
    return host;
}


void httpSetHostDocumentRoot(HttpHost *host, cchar *dir)
{
    char    *doc;
    int     len;

    doc = host->documentRoot = httpMakePath(host, dir);
    len = (int) strlen(doc);
    if (doc[len - 1] == '/') {
        doc[len - 1] = '\0';
    }
    /*  Create a catch-all alias */
    httpAddAlias(host, httpCreateAlias("", doc, 0));
}


void httpSetHostServerRoot(HttpHost *host, cchar *serverRoot)
{
    host->serverRoot = mprGetAbsPath(serverRoot);
}


void httpSetHostName(HttpHost *host, cchar *name)
{
    host->name = sclone(name);
}


void httpSetHostAddress(HttpHost *host, cchar *ip, int port)
{
    host->ip = sclone(ip);
    if (port > 0) {
        host->port = port;
    }
}


void httpSetHostProtocol(HttpHost *host, cchar *protocol)
{
    host->protocol = sclone(protocol);
}


int httpAddAlias(HttpHost *host, HttpAlias *newAlias)
{
    HttpAlias     *alias;
    int         rc, next;

    if (host->parent && host->aliases == host->parent->aliases) {
        host->aliases = mprCloneList(host->parent->aliases);
    }

    /*  
        Sort in reverse order. Make sure that /abc/def sorts before /abc. But we sort redirects with status codes first.
     */
    for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
        rc = strcmp(newAlias->prefix, alias->prefix);
        if (rc == 0) {
            if (newAlias->redirectCode > alias->redirectCode) {
                mprInsertItemAtPos(host->aliases, next - 1, newAlias);

            } else if (newAlias->redirectCode == alias->redirectCode) {
                mprRemoveItem(host->aliases, alias);
                mprInsertItemAtPos(host->aliases, next - 1, newAlias);
            }
            return 0;
            
        } else if (rc > 0) {
            mprInsertItemAtPos(host->aliases, next - 1, newAlias);
            return 0;
        }
    }
    mprAddItem(host->aliases, newAlias);
    return 0;
}


int httpAddDir(HttpHost *host, HttpDir *dir)
{
    HttpDir     *dp;
    int         rc, next;

    mprAssert(dir);
    mprAssert(dir->path);
    
    if (host->parent && host->dirs == host->parent->dirs) {
        host->dirs = mprCloneList(host->parent->dirs);
    }

    /*
        Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc
     */
    for (next = 0; (dp = mprGetNextItem(host->dirs, &next)) != 0; ) {
        mprAssert(dp->path);
        rc = strcmp(dir->path, dp->path);
        if (rc == 0) {
            mprRemoveItem(host->dirs, dir);
            mprInsertItemAtPos(host->dirs, next - 1, dir);
            return 0;

        } else if (rc > 0) {
            mprInsertItemAtPos(host->dirs, next - 1, dir);
            return 0;
        }
    }
    mprAddItem(host->dirs, dir);
    return 0;
}


int httpAddLocation(HttpHost *host, HttpLoc *newLocation)
{
    HttpLoc     *loc;
    int         next, rc;

    mprAssert(newLocation);
    mprAssert(newLocation->prefix);
    
    if (host->parent && host->locations == host->parent->locations) {
        host->locations = mprCloneList(host->parent->locations);
    }

    /*
        Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc
     */
    for (next = 0; (loc = mprGetNextItem(host->locations, &next)) != 0; ) {
        rc = strcmp(newLocation->prefix, loc->prefix);
        if (rc == 0) {
            mprRemoveItem(host->locations, loc);
            mprInsertItemAtPos(host->locations, next - 1, newLocation);
            return 0;
        }
        if (strcmp(newLocation->prefix, loc->prefix) > 0) {
            mprInsertItemAtPos(host->locations, next - 1, newLocation);
            return 0;
        }
    }
    mprAddItem(host->locations, newLocation);
    return 0;
}


HttpAlias *httpGetAlias(HttpHost *host, cchar *uri)
{
    HttpAlias     *alias;
    int         next;

    if (uri) {
        for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
            if (strncmp(alias->prefix, uri, alias->prefixLen) == 0) {
                if (uri[alias->prefixLen] == '\0' || uri[alias->prefixLen] == '/') {
                    return alias;
                }
            }
        }
    }
    /*
        Must always return an alias. The last is the catch-all.
     */
    return mprGetLastItem(host->aliases);
}


HttpAlias *httpLookupAlias(HttpHost *host, cchar *prefix)
{
    HttpAlias   *alias;
    int         next;

    for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
        if (strcmp(alias->prefix, prefix) == 0) {
            return alias;
        }
    }
    return 0;
}


HttpDir *httpLookupDir(HttpHost *host, cchar *pathArg)
{
    HttpDir       *dir;
    char        *path, *tmpPath;
    int         next, len;

    if (!mprIsAbsPath(pathArg)) {
        path = tmpPath = mprGetAbsPath(pathArg);
    } else {
        path = (char*) pathArg;
        tmpPath = 0;
    }
    len = (int) strlen(path);

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        mprAssert(strlen(dir->path) == 0 || dir->path[strlen(dir->path) - 1] != '/');
        if (dir->path != 0) {
            if (mprSamePath(dir->path, path)) {
                return dir;
            }
        }
    }
    return 0;
}


/*  
    Find the directory entry that this file (path) resides in. path is a physical file path. We find the most specific
    (longest) directory that matches. The directory must match or be a parent of path. Not called with raw files names.
    They will be lower case and only have forward slashes. For windows, the will be in cannonical format with drive
    specifiers.
 */
HttpDir *httpLookupBestDir(HttpHost *host, cchar *path)
{
    HttpDir *dir;
    int     next, len, dlen;

    len = (int) strlen(path);

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        dlen = dir->pathLen;
        mprAssert(dlen == 0 || dir->path[dlen - 1] != '/');
        if (mprSamePathCount(dir->path, path, dlen)) {
            if (dlen >= 0) {
                return dir;
            }
        }
    }
    return 0;
}


HttpLoc *httpLookupLocation(HttpHost *host, cchar *prefix)
{
    HttpLoc     *loc;
    int         next;

    for (next = 0; (loc = mprGetNextItem(host->locations, &next)) != 0; ) {
        if (strcmp(prefix, loc->prefix) == 0) {
            return loc;
        }
    }
    return 0;
}


HttpLoc *httpLookupBestLocation(HttpHost *host, cchar *uri)
{
    HttpLoc     *loc;
    int         next, rc;

    if (uri) {
        for (next = 0; (loc = mprGetNextItem(host->locations, &next)) != 0; ) {
            rc = sncmp(loc->prefix, uri, loc->prefixLen);
            if (rc == 0 && uri[loc->prefixLen] == '/') {
                return loc;
            }
        }
    }
    return mprGetLastItem(host->locations);
}


//  MOB -- order this file

void httpSetHostTrace(HttpHost *host, int level, int mask)
{
    host->traceMask = mask;
    host->traceLevel = level;
}


void httpSetHostTraceFilter(HttpHost *host, int len, cchar *include, cchar *exclude)
{
    char    *word, *tok, *line;

    host->traceMaxLength = len;

    if (include && strcmp(include, "*") != 0) {
        host->traceInclude = mprCreateHash(0, 0);
        line = sclone(include);
        word = stok(line, ", \t\r\n", &tok);
        while (word) {
            if (word[0] == '*' && word[1] == '.') {
                word += 2;
            }
            mprAddKey(host->traceInclude, word, host);
            word = stok(NULL, ", \t\r\n", &tok);
        }
    }
    if (exclude) {
        host->traceExclude = mprCreateHash(0, 0);
        line = sclone(exclude);
        word = stok(line, ", \t\r\n", &tok);
        while (word) {
            if (word[0] == '*' && word[1] == '.') {
                word += 2;
            }
            mprAddKey(host->traceExclude, word, host);
            word = stok(NULL, ", \t\r\n", &tok);
        }
    }
}


int httpSetupTrace(HttpHost *host, cchar *ext)
{
    if (ext) {
        if (host->traceInclude && !mprLookupHash(host->traceInclude, ext)) {
            return 0;
        }
        if (host->traceExclude && mprLookupHash(host->traceExclude, ext)) {
            return 0;
        }
    }
    return host->traceMask;
}


/*
    Make a path name. This replaces $references, converts to an absolute path name, cleans the path and maps delimiters.
 */
char *httpMakePath(HttpHost *host, cchar *file)
{
    char    *result, *path;

    mprAssert(file);

    if ((path = httpReplaceReferences(host, file)) == 0) {
        /*  Overflow */
        return 0;
    }
    if (*path == '\0' || strcmp(path, ".") == 0) {
        result = sclone(host->serverRoot);

#if BLD_WIN_LIKE
    } else if (*path != '/' && path[1] != ':' && path[2] != '/') {
        result = mprJoinPath(host->serverRoot, path);
#elif VXWORKS
    } else if (strchr((path), ':') == 0 && *path != '/') {
        result = mprJoinPath(host->serverRoot, path);
#else
    } else if (*path != '/') {
        result = mprJoinPath(host->serverRoot, path);
#endif
    } else {
        result = mprGetAbsPath(path);
    }
    return result;
}


static int matchRef(cchar *key, char **src)
{
    int     len;

    mprAssert(src);
    mprAssert(key && *key);

    len = strlen(key);
    if (strncmp(*src, key, len) == 0) {
        *src += len;
        return 1;
    }
    return 0;
}


/*
    Replace a limited set of $VAR references. Currently support DOCUMENT_ROOT, SERVER_ROOT and PRODUCT
    TODO - Expand and formalize this. Should support many more variables.
 */
//  MOB - rename
char *httpReplaceReferences(HttpHost *host, cchar *str)
{
    MprBuf  *buf;
    char    *src;
    char    *result;

    buf = mprCreateBuf(0, 0);
    if (str) {
        for (src = (char*) str; *src; ) {
            if (*src == '$') {
                ++src;
                if (matchRef("DOCUMENT_ROOT", &src)) {
                    mprPutStringToBuf(buf, host->documentRoot);
                    continue;

                } else if (matchRef("SERVER_ROOT", &src)) {
                    mprPutStringToBuf(buf, host->serverRoot);
                    continue;

                } else if (matchRef("PRODUCT", &src)) {
                    mprPutStringToBuf(buf, BLD_PRODUCT);
                    continue;
                }
            }
            mprPutCharToBuf(buf, *src++);
        }
    }
    mprAddNullToBuf(buf);
    result = sclone(mprGetBufStart(buf));
    return result;
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
/************************************************************************/
/*
 *  End of file "../src/host.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/httpService.c"
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
    { 408, "408", "Request Time-out" },
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
    { 504, "504", "Gateway Time-out" },
    { 505, "505", "Http Version Not Supported" },
    { 507, "507", "Insufficient Storage" },

    /*
        Proprietary codes (used internally) when connection to client is severed
     */
    { 550, "550", "Comms Error" },
    { 551, "551", "General Client Error" },
    { 0,   0 }
};


static int httpTimer(Http *http, MprEvent *event);
static bool isIdle();
static void manageHttp(Http *http, int flags);
static void updateCurrentDate(Http *http);


Http *httpCreate()
{
    Http            *http;
    HttpStatusCode  *code;

    if ((http = mprAllocObj(Http, manageHttp)) == 0) {
        return 0;
    }
    mprGetMpr()->httpService = http;
    http->software = sclone(HTTP_NAME);
    http->protocol = sclone("HTTP/1.1");
    http->mutex = mprCreateLock(http);
    http->stages = mprCreateHash(-1, 0);
    http->hosts = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->servers = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->connections = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->defaultClientHost = sclone("127.0.0.1");
    http->defaultClientPort = 80;

    updateCurrentDate(http);
    http->statusCodes = mprCreateHash(41, MPR_HASH_STATIC_VALUES | MPR_HASH_STATIC_KEYS);
    for (code = HttpStatusCodes; code->code; code++) {
        mprAddKey(http->statusCodes, code->codeString, code);
    }
    httpCreateSecret(http);
    httpInitAuth(http);
    httpOpenNetConnector(http);
    httpOpenSendConnector(http);
    httpOpenAuthFilter(http);
    httpOpenRangeFilter(http);
    httpOpenChunkFilter(http);
    httpOpenUploadFilter(http);
    httpOpenPassHandler(http);

    http->clientLimits = httpCreateLimits(0);
    http->serverLimits = httpCreateLimits(1);
    http->clientLocation = httpInitLocation(http, 0);

    mprSetIdleCallback(isIdle);
    return http;
}


static void manageHttp(Http *http, int flags)
{
    HttpConn    *conn;
    int         next;

    if (flags & MPR_MANAGE_MARK) {
        /* Note servers and hosts are static values - contents are not marked so they can be collected */
        mprMark(http->servers);
        mprMark(http->hosts);
        mprMark(http->connections);
        mprMark(http->stages);
        mprMark(http->statusCodes);
        mprMark(http->clientLimits);
        mprMark(http->serverLimits);
        mprMark(http->clientLocation);
        mprMark(http->timer);
        mprMark(http->mutex);
        mprMark(http->software);
        mprMark(http->forkData);
        mprMark(http->context);
        mprMark(http->currentDate);
        mprMark(http->expiresDate);
        mprMark(http->secret);
        mprMark(http->protocol);
        mprMark(http->proxyHost);
        mprMark(http->servers);
        mprMark(http->defaultClientHost);

        /*
            Servers keep connections alive until a timeout. Keep marking even if no other references.
         */
        lock(http);
        for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
            if (conn->server) {
                mprMark(conn);
            }
        }
        unlock(http);
    }
}


void httpDestroy(Http *http)
{
    MPR->httpService = NULL;
}


void httpAddServer(Http *http, HttpServer *server)
{
    mprAddItem(http->servers, server);
}


void httpRemoveServer(Http *http, HttpServer *server)
{
    mprRemoveItem(http->servers, server);
}


/*  
    Lookup a host address. If ipAddr is null or port is -1, then those elements are wild.
    MOB - wild cards are not being used - see asserts
 */
HttpServer *httpLookupServer(Http *http, cchar *ip, int port)
{
    HttpServer  *server;
    int         next;

    if (ip == 0) {
        ip = "";
    }
    for (next = 0; (server = mprGetNextItem(http->servers, &next)) != 0; ) {
        if (server->port <= 0 || port <= 0 || server->port == port) {
            mprAssert(server->ip);
            if (*server->ip == '\0' || *ip == '\0' || scmp(server->ip, ip) == 0) {
                return server;
            }
        }
    }
    return 0;
}


int httpAddHostToServers(Http *http, struct HttpHost *host)
{
    HttpServer  *server;
    cchar       *ip;
    int         count, next;

    ip = host->ip ? host->ip : "";
    mprAssert(ip);

    for (count = next = 0; (server = mprGetNextItem(http->servers, &next)) != 0; ) {
        if (server->port <= 0 || host->port <= 0 || server->port == host->port) {
            mprAssert(server->ip);
            if (*server->ip == '\0' || *ip == '\0' || scmp(server->ip, ip) == 0) {
                httpAddHostToServer(server, host);
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
}


int httpSetNamedVirtualServers(Http *http, cchar *ip, int port)
{
    HttpServer  *server;
    int         count, next;

    if (ip == 0) {
        ip = "";
    }
    for (count = next = 0; (server = mprGetNextItem(http->servers, &next)) != 0; ) {
        if (server->port <= 0 || port <= 0 || server->port == port) {
            mprAssert(server->ip);
            if (*server->ip == '\0' || *ip == '\0' || scmp(server->ip, ip) == 0) {
                httpSetNamedVirtualServer(server);
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
}


void httpAddHost(Http *http, HttpHost *host)
{
    mprAddItem(http->hosts, host);
#if UNUSED
    if (http->defaultHost == 0) {
        http->defaultHost = host;
    }
#endif
}


void httpRemoveHost(Http *http, HttpHost *host)
{
    mprRemoveItem(http->hosts, host);
#if UNUSED
    if (host == http->defaultHost) {
        http->defaultHost = mprGetFirstItem(http->hosts);
    }
#endif
}


#if UNUSED
//  MOB is this used?
HttpHost *httpLookupHost(Http *http, cchar *name)
{
    HttpHost  *host;
    int         next;

    for (next = 0; (host = mprGetNextItem(http->hosts, &next)) != 0; ) {
        if (strcmp(host->name, name) == 0) {
            return host;
        }
    }
    return 0;
}
#endif


//  MOB - rename
HttpLoc *httpInitLocation(Http *http, int serverSide)
{
    HttpLoc     *loc;

    /*
        Create default incoming and outgoing pipelines. Order matters.
     */
    loc = httpCreateLocation(http);
    httpAddFilter(loc, http->authFilter->name, NULL, HTTP_STAGE_OUTGOING);
    httpAddFilter(loc, http->rangeFilter->name, NULL, HTTP_STAGE_OUTGOING);
    httpAddFilter(loc, http->chunkFilter->name, NULL, HTTP_STAGE_OUTGOING);

    httpAddFilter(loc, http->chunkFilter->name, NULL, HTTP_STAGE_INCOMING);
    httpAddFilter(loc, http->uploadFilter->name, NULL, HTTP_STAGE_INCOMING);
    loc->connector = http->netConnector;
    return loc;
}


void httpInitLimits(HttpLimits *limits, int serverSide)
{
    memset(limits, 0, sizeof(HttpLimits));
    limits->chunkSize = HTTP_MAX_CHUNK;
    limits->headerCount = HTTP_MAX_NUM_HEADERS;
    limits->headerSize = HTTP_MAX_HEADERS;
    limits->receiveBodySize = HTTP_MAX_RECEIVE_BODY;
    limits->requestCount = HTTP_MAX_REQUESTS;
    limits->stageBufferSize = HTTP_MAX_STAGE_BUFFER;
    limits->transmissionBodySize = HTTP_MAX_TRANSMISSION_BODY;
    limits->uploadSize = HTTP_MAX_UPLOAD;
    limits->uriSize = MPR_MAX_URL;

    limits->inactivityTimeout = HTTP_INACTIVITY_TIMEOUT;
    limits->requestTimeout = 0;
    limits->sessionTimeout = HTTP_SESSION_TIMEOUT;

    limits->clientCount = HTTP_MAX_CLIENTS;
    limits->keepAliveCount = HTTP_MAX_KEEP_ALIVE;
    limits->requestCount = HTTP_MAX_REQUESTS;
    limits->sessionCount = HTTP_MAX_SESSIONS;

#if FUTURE
    mprSetMaxSocketClients(server, atoi(value));

    if (scasecmp(key, "LimitClients") == 0) {
        mprSetMaxSocketClients(server, atoi(value));
        return 1;
    }
    if (scasecmp(key, "LimitMemoryMax") == 0) {
        mprSetAllocLimits(server, -1, atoi(value));
        return 1;
    }
    if (scasecmp(key, "LimitMemoryRedline") == 0) {
        mprSetAllocLimits(server, atoi(value), -1);
        return 1;
    }
#endif
}


HttpLimits *httpCreateLimits(int serverSide)
{
    HttpLimits  *limits;

    if ((limits = mprAllocObj(HttpLimits, NULL)) != 0) {
        httpInitLimits(limits, serverSide);
    }
    return limits;
}


void httpAddStage(Http *http, HttpStage *stage)
{
    mprAddKey(http->stages, stage->name, stage);
}


HttpStage *httpLookupStage(Http *http, cchar *name)
{
    return (HttpStage*) mprLookupHash(http->stages, name);
}


cchar *httpLookupStatus(Http *http, int status)
{
    HttpStatusCode  *ep;
    char            key[8];
    
    itos(key, sizeof(key), status, 10);
    ep = (HttpStatusCode*) mprLookupHash(http->statusCodes, key);
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


void httpSetMatchCallback(Http *http, HttpMatchCallback fn)
{
    http->matchCallback = fn;
}


/*  
    Start the http timer. This may create multiple timers -- no worry. httpAddConn does its best to only schedule one.
 */
static void startTimer(Http *http)
{
    updateCurrentDate(http);
    http->timer = mprCreateTimerEvent(NULL, "httpTimer", HTTP_TIMER_PERIOD, (MprEventProc) httpTimer, 
        http, MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK);
}


/*  
    The http timer does maintenance activities and will fire per second while there is active requests.
    When multi-threaded, the http timer runs as an event off the service thread. Because we lock the http here,
    connections cannot be deleted while we are modifying the list.
 */
static int httpTimer(Http *http, MprEvent *event)
{
    HttpConn    *conn;
    HttpStage   *stage;
    MprModule   *module;
    int64       diff;
    int         next, count, inactivity, requestTimeout, inactivityTimeout;

    mprAssert(event);
    
    updateCurrentDate(http);
    if (mprGetDebugMode(http)) {
        return 0;
    }

    /* 
       Check for any inactive or expired connections (inactivityTimeout and requestTimeout)
     */
    lock(http);
    mprLog(8, "httpTimer: %d active connections", mprGetListLength(http->connections));
    for (count = 0, next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; count++) {
        requestTimeout = conn->limits->requestTimeout ? conn->limits->requestTimeout : INT_MAX;
        inactivityTimeout = conn->limits->inactivityTimeout ? conn->limits->inactivityTimeout : INT_MAX;
        /* 
            Workaround for a GCC bug when comparing two 64bit numerics directly. Need a temporary.
         */
        diff = (conn->lastActivity + inactivityTimeout) - http->now;
        inactivity = 1;
        if (diff > 0 && conn->rx) {
            diff = (conn->lastActivity + requestTimeout) - http->now;
            inactivity = 0;
        }

        if (diff < 0 && !conn->complete) {
            if (conn->rx) {
                if (inactivity) {
                    httpConnError(conn, HTTP_CODE_REQUEST_TIMEOUT,
                        "Inactive request timed out. Exceeded inactivity timeout of %d sec. Uri: \"%s\"", 
                        inactivityTimeout / 1000, conn->rx->uri);
                } else {
                    httpConnError(conn, HTTP_CODE_REQUEST_TIMEOUT, 
                        "Request timed out, exceeded timeout %d sec. Url %s", requestTimeout / 1000, conn->rx->uri);
                }
            } else {
                mprLog(6, "Idle connection timed out");
                conn->complete = 1;
                mprDisconnectSocket(conn->sock);
            }
        }
    }
    /*
        Check for unloadable modules
     */
    if (mprGetListLength(http->connections) == 0) {
        for (next = 0; (module = mprGetNextItem(MPR->moduleService->modules, &next)) != 0; ) {
            if (module->timeout) {
                if (module->lastActivity + module->timeout < http->now) {
                    mprLog(2, "Unloading inactive module %s", module->name);
                    if ((stage = httpLookupStage(http, module->name)) != 0) {
                        if (stage->match) {
                            mprError("Can't unload modules with match routines");
                            module->timeout = 0;
                        } else {
                            mprUnloadModule(module);
                            stage->flags |= HTTP_STAGE_UNLOADED;
                        }
                    } else {
                        mprUnloadModule(module);
                    }
                } else {
                    count++;
                }
            }
        }
    }
    if (count == 0) {
        mprRemoveEvent(event);
        http->timer = 0;
    }
    unlock(http);
    return 0;
}


static bool isIdle()
{
    HttpConn        *conn;
    Http            *http;
    MprTime         now;
    int             next;
    static MprTime  lastTrace = 0;

    http = (Http*) mprGetMpr()->httpService;
    now = mprGetTime();

    lock(http);
    for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
        if (conn->state != HTTP_STATE_BEGIN) {
            if (lastTrace < now) {
                mprLog(0, "Waiting for request %s to complete", conn->rx->uri ? conn->rx->uri : conn->rx->pathInfo);
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
    //  MOB
    mprAssert(http->connections->length < 25);
    mprAddItem(http->connections, conn);
    conn->started = mprGetTime();
    conn->seqno = http->connCount++;
    if ((http->now + MPR_TICKS_PER_SEC) < conn->started) {
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
        now = mprGetTime(); 
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
    struct tm   tm;

    if (sbuf == 0) {
        when = mprGetTime();
    } else {
        when = (MprTime) sbuf->mtime * MPR_TICKS_PER_SEC;
    }
    mprDecodeUniversalTime(&tm, when);
    return mprFormatTime(HTTP_DATE_FORMAT, &tm);
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
    struct tm       tm;
    static MprTime  recalcExpires = 0;

    lock(http);
    http->now = mprGetTime();
    http->currentDate = httpGetDateString(NULL);

    if (http->expiresDate == 0 || recalcExpires < (http->now / (60 * 1000))) {
        mprDecodeUniversalTime(&tm, http->now + (86400 * 1000));
        http->expiresDate = mprFormatTime(HTTP_DATE_FORMAT, &tm);
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
/************************************************************************/
/*
 *  End of file "../src/httpService.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/location.c"
 */
/************************************************************************/

/*
    location.c -- Server configuration for portions of the server (Location blocks).

    Location directives provide authorization and handler matching based on URL prefixes.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageLoc(HttpLoc *loc, int flags);


HttpLoc *httpCreateLocation()
{
    HttpLoc  *loc;

    if ((loc = mprAllocObj(HttpLoc, manageLoc)) == 0) {
        return 0;
    }
    loc->http = MPR->httpService;
    loc->errorDocuments = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
    loc->handlers = mprCreateList(-1, 0);
    loc->extensions = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
    loc->expires = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_STATIC_VALUES);
    loc->inputStages = mprCreateList(-1, 0);
    loc->outputStages = mprCreateList(-1, 0);
    loc->prefix = mprEmptyString();
    loc->prefixLen = (int) strlen(loc->prefix);
    loc->auth = httpCreateAuth(0);
    return loc;
}


static void manageLoc(HttpLoc *loc, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(loc->auth);
        mprMark(loc->http);
        mprMark(loc->prefix);
        mprMark(loc->connector);
        mprMark(loc->handler);
        mprMark(loc->extensions);
        mprMark(loc->expires);
        mprMark(loc->handlers);
        mprMark(loc->inputStages);
        mprMark(loc->outputStages);
        mprMark(loc->errorDocuments);
        mprMark(loc->parent);
        mprMark(loc->context);
        mprMark(loc->uploadDir);
        mprMark(loc->searchPath);
        mprMark(loc->script);
        mprMark(loc->ssl);
    }
}

/*  
    Create a new location block. Inherit from the parent. We use a copy-on-write scheme if these are modified later.
 */
HttpLoc *httpCreateInheritedLocation(HttpLoc *parent)
{
    HttpLoc  *loc;

    if (parent == 0) {
        return httpCreateLocation();
    }
    if ((loc = mprAllocObj(HttpLoc, manageLoc)) == 0) {
        return 0;
    }
    loc->http = MPR->httpService;
    loc->prefix = sclone(parent->prefix);
    loc->parent = parent;
    loc->prefixLen = parent->prefixLen;
    loc->flags = parent->flags;
    loc->inputStages = parent->inputStages;
    loc->outputStages = parent->outputStages;
    loc->handlers = parent->handlers;
    loc->extensions = parent->extensions;
    loc->expires = parent->expires;
    loc->connector = parent->connector;
    loc->errorDocuments = parent->errorDocuments;
    loc->sessionTimeout = parent->sessionTimeout;
    loc->auth = httpCreateAuth(parent->auth);
    loc->uploadDir = parent->uploadDir;
    loc->autoDelete = parent->autoDelete;
    loc->script = parent->script;
    loc->searchPath = parent->searchPath;
    loc->ssl = parent->ssl;
    return loc;
}


/*
    Separate from parent. Clone own configuration
 */
static void graduate(HttpLoc *loc) 
{
    if (loc->parent) {
        loc->errorDocuments = mprCloneHash(loc->parent->errorDocuments);
        loc->expires = mprCloneHash(loc->parent->expires);
        loc->extensions = mprCloneHash(loc->parent->extensions);
        loc->handlers = mprCloneList(loc->parent->handlers);
        loc->inputStages = mprCloneList(loc->parent->inputStages);
        loc->outputStages = mprCloneList(loc->parent->outputStages);
        loc->parent = 0;
    }
}

void httpFinalizeLocation(HttpLoc *loc)
{
#if BLD_FEATURE_SSL
    if (loc->ssl) {
        mprConfigureSsl(loc->ssl);
    }
#endif
}


void httpSetLocationAuth(HttpLoc *loc, HttpAuth *auth)
{
    loc->auth = auth;
}


/*  
    Add a handler. This adds a handler to the set of possible handlers for a set of file extensions.
 */
int httpAddHandler(HttpLoc *loc, cchar *name, cchar *extensions)
{
    Http        *http;
    HttpStage   *handler;
    char        *extlist, *word, *tok;

    mprAssert(loc);

    http = loc->http;
    graduate(loc);
    handler = httpLookupStage(http, name);
    if (handler == 0) {
        mprError("Can't find stage %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    if (extensions && *extensions) {
        mprLog(MPR_CONFIG, "Add handler \"%s\" for \"%s\"", name, extensions);
    } else {
        mprLog(MPR_CONFIG, "Add handler \"%s\" for \"%s\"", name, loc->prefix);
    }
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
            mprAddKey(loc->extensions, word, handler);
            word = stok(0, " \t\r\n", &tok);
        }

    } else {
        if (handler->match == 0) {
            /*
                If a handler provides a custom match() routine, then don't match by extension.
             */
            mprAddKey(loc->extensions, "", handler);
        }
        mprAddItem(loc->handlers, handler);
    }
    return 0;
}


/*  
    Set a handler to universally apply to requests in this location block.
 */
int httpSetHandler(HttpLoc *loc, cchar *name)
{
    HttpStage     *handler;

    mprAssert(loc);
    
    graduate(loc);
    handler = httpLookupStage(loc->http, name);
    if (handler == 0) {
        mprError("Can't find handler %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    loc->handler = handler;
    return 0;
}


/*  
    Add a filter. Direction defines what direction the stage filter be defined.
 */
int httpAddFilter(HttpLoc *loc, cchar *name, cchar *extensions, int direction)
{
    HttpStage   *stage;
    HttpStage   *filter;
    char        *extlist, *word, *tok;

    mprAssert(loc);
    
    stage = httpLookupStage(loc->http, name);
    if (stage == 0) {
        mprError("Can't find filter %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    /*
        Clone an existing stage because each filter stores its own set of extensions to match against
     */
    filter = httpCloneStage(loc->http, stage);

    if (extensions && *extensions) {
        filter->extensions = mprCreateHash(0, 0);
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
    graduate(loc);
    if (direction & HTTP_STAGE_INCOMING) {
        mprAddItem(loc->inputStages, filter);
    }
    if (direction & HTTP_STAGE_OUTGOING) {
        mprAddItem(loc->outputStages, filter);
    }
    return 0;
}


void httpAddLocationExpiry(HttpLoc *loc, MprTime when, cchar *mimeTypes)
{
    char    *types, *mime, *tok;

    if (mimeTypes && *mimeTypes) {
        graduate(loc);
        types = sclone(mimeTypes);
        mime = stok(types, " ,\t\r\n", &tok);
        while (mime) {
            mprAddKey(loc->expires, mime, ITOP(when));
            mime = stok(0, " \t\r\n", &tok);
        }
    }
}


void httpClearStages(HttpLoc *loc, int direction)
{
    if (direction & HTTP_STAGE_INCOMING) {
        loc->inputStages = mprCreateList(-1, 0);
    }
    if (direction & HTTP_STAGE_OUTGOING) {
        loc->outputStages = mprCreateList(-1, 0);
    }
}


/* 
   Set the network connector
 */
int httpSetConnector(HttpLoc *loc, cchar *name)
{
    HttpStage     *stage;

    mprAssert(loc);
    
    stage = httpLookupStage(loc->http, name);
    if (stage == 0) {
        mprError("Can't find connector %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    loc->connector = stage;
    mprLog(MPR_CONFIG, "Set connector \"%s\"", name);
    return 0;
}


void httpResetPipeline(HttpLoc *loc)
{
    if (loc->parent == 0) {
        loc->errorDocuments = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
        loc->expires = mprCreateHash(0, MPR_HASH_STATIC_VALUES);
        loc->extensions = mprCreateHash(0, 0);
        loc->handlers = mprCreateList(-1, 0);
        loc->inputStages = mprCreateList(-1, 0);
        loc->inputStages = mprCreateList(-1, 0);
    }
    loc->outputStages = mprCreateList(-1, 0);
}


void httpResetHandlers(HttpLoc *loc)
{
    graduate(loc);
    loc->handlers = mprCreateList(-1, 0);
}


HttpStage *httpGetHandlerByExtension(HttpLoc *loc, cchar *ext)
{
    return (HttpStage*) mprLookupHash(loc->extensions, ext);
}


void httpSetLocationPrefix(HttpLoc *loc, cchar *uri)
{
    mprAssert(loc);

    loc->prefix = sclone(uri);
    loc->prefixLen = (int) strlen(loc->prefix);

    /*
        Always strip trailing "/". Note this is a Uri and not a path.
     */
    if (loc->prefixLen > 0 && loc->prefix[loc->prefixLen - 1] == '/') {
        loc->prefix[--loc->prefixLen] = '\0';
    }
}


void httpSetLocationFlags(HttpLoc *loc, int flags)
{
    loc->flags = flags;
}


void httpSetLocationAutoDelete(HttpLoc *loc, int enable)
{
    loc->autoDelete = enable;
}


void httpSetLocationScript(HttpLoc *loc, cchar *script)
{
    loc->script = sclone(script);
}


void httpAddErrorDocument(HttpLoc *loc, cchar *code, cchar *url)
{
    graduate(loc);
    mprAddKey(loc->errorDocuments, code, sclone(url));
}


cchar *httpLookupErrorDocument(HttpLoc *loc, int code)
{
    char        numBuf[16];

    if (loc->errorDocuments == 0) {
        return 0;
    }
    itos(numBuf, sizeof(numBuf), code, 10);
    return (cchar*) mprLookupHash(loc->errorDocuments, numBuf);
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
/************************************************************************/
/*
 *  End of file "../src/location.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/match.c"
 */
/************************************************************************/

/*
    match.c -- HTTP pipeline matching.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static char *addIndexToUrl(HttpConn *conn, cchar *index);
static char *getExtension(HttpConn *conn, cchar *path);
static HttpStage *checkHandler(HttpConn *conn, HttpStage *stage);
static HttpStage *findHandler(HttpConn *conn);
static HttpStage *findLocationHandler(HttpConn *conn);
static HttpStage *mapToFile(HttpConn *conn, HttpStage *handler);
static HttpStage *processDirectory(HttpConn *conn, HttpStage *handler);
static bool rewriteRequest(HttpConn *conn);
static void setScriptName(HttpConn *conn);

/*
    Match a request to a Host. This is an initial match which may be revised if the request includes a Host header.
    This is typically invoked from the server state change notifier on transition to the PARSED state.
 */
void httpMatchHost(HttpConn *conn)
{ 
    MprSocket       *listenSock;
    HttpServer      *server;
    HttpHost        *host;
    HttpRx          *rx;
    Http            *http;

    http = conn->http;
    listenSock = conn->sock->listenSock;

    server = httpLookupServer(http, listenSock->ip, listenSock->port);
    if (server == 0 || (host = mprGetFirstItem(server->hosts)) == 0) {
        httpProtocolError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
            "No host to serve request from %s:%d", listenSock->ip, listenSock->port);
        return;
    }
    mprAssert(host);

    if (httpIsNamedVirtualServer(server)) {
        rx = conn->rx;
        if ((host = httpLookupHostByName(server, rx->hostName)) == 0) {
            httpSetConnHost(conn, host);
            httpError(conn, HTTP_CODE_NOT_FOUND, "No host to serve request. Searching for %s", rx->hostName);
            return;
        }
    }
    mprAssert(host);
    mprLog(3, "Select host: \"%s\"", host->name);
    conn->host = host;
#if UNUSED
    //  MOB - who does this?
    if (mprGetLogLevel(conn) >= host->traceLevel) {
        conn->traceLevel = host->traceLevel;
        conn->traceMaxLength = host->traceMaxLength;
        conn->traceMask = host->traceMask;
        conn->traceInclude = host->traceInclude;
        conn->traceExclude = host->traceExclude;
    }            
#endif
}


/*
    Find the matching handler for a request. If any errors occur, the pass handler is used to pass errors onto the 
    net/sendfile connectors to send to the client. This routine may rewrite the request URI and may redirect the request.
 */
void httpMatchHandler(HttpConn *conn)
{
    Http        *http;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpStage   *handler;
    HttpHost    *host;

    http = conn->http;
    rx = conn->rx;
    tx = conn->tx;
    host = conn->host;
    handler = 0;

    mprAssert(rx->uri);
    mprAssert(rx->loc);
    mprAssert(rx->alias);
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);

    /*
        Get the best (innermost) location block and see if a handler is explicitly set for that location block.
        Possibly rewrite the url and retry.
     */
    while (!handler && !conn->error && rx->rewrites++ < HTTP_MAX_REWRITE) {
        /*
            Give stages a cance to rewrite the request, then match the location handler. If that doesn't match,
            try to match by extension and/or handler match() routines. This may invoke processDirectory which
            may redirect and thus require reprocessing -- hence the loop.
        */
        if (!rewriteRequest(conn)) {
            if ((handler = findLocationHandler(conn)) == 0) {
                handler = findHandler(conn);
            }
            handler = mapToFile(conn, handler);
        }
    }
    if (handler == 0 || conn->error || ((tx->flags & HTTP_TX_NO_BODY) && !(rx->flags & HTTP_HEAD))) {
        handler = http->passHandler;
        if (rx->rewrites >= HTTP_MAX_REWRITE) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Too many request rewrites");
        }
    }
    if (handler->flags & HTTP_STAGE_PATH_INFO) {
        setScriptName(conn);
    }
    tx->handler = handler;
}


static bool rewriteRequest(HttpConn *conn)
{
    HttpStage       *handler;
    HttpAlias       *alias;
    HttpLoc         *loc;
    HttpRx          *rx;
    HttpTx          *tx;
    MprHash         *he;
    int             next;

    rx = conn->rx;
    tx = conn->tx;
    loc = rx->loc;
    alias = rx->alias;

    mprAssert(rx->alias);
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);

    if (alias->redirectCode) {
        httpRedirect(conn, alias->redirectCode, alias->uri);
        return NULL;
    }
    for (next = 0; (handler = mprGetNextItem(loc->handlers, &next)) != 0; ) {
        if (handler->rewrite && handler->rewrite(conn, handler)) {
            return 1;
        }
    }
    for (he = 0; (he = mprGetNextHash(loc->extensions, he)) != 0; ) {
        handler = (HttpStage*) he->data;
        if (handler->rewrite && handler->rewrite(conn, handler)) {
            return 1;
        }
    }
    return 0;
}


static HttpStage *findLocationHandler(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLoc     *loc;
    HttpHost    *host;

    rx = conn->rx;
    tx = conn->tx;
    host = httpGetConnHost(conn);
    loc = rx->loc = httpLookupBestLocation(host, rx->pathInfo);
    rx->auth = loc->auth;
    mprAssert(loc);
    return checkHandler(conn, loc->handler);
}


static char *getExtension(HttpConn *conn, cchar *path)
{
    HttpRx      *rx;
    char        *cp, *ep, *ext;

    rx = conn->rx;

    if (rx && rx->pathInfo) {
        if ((cp = strrchr(path, '.')) != 0) {
            ext = sclone(++cp);
            for (ep = ext; *ep && isalnum((int)*ep); ep++) {
                ;
            }
            *ep = '\0';
            return ext;
        }
    }
    return 0;
}


/*
    Get the request extension. Look first at the URI (pathInfo). If no extension, look at the filename.
    Return NULL if no extension.
 */
char *httpGetExtension(HttpConn *conn)
{
    HttpRx  *rx;
    char    *ext;

    rx = conn->rx;
    if ((ext = getExtension(conn, &rx->pathInfo[rx->alias->prefixLen])) == 0) {
        ext = getExtension(conn, conn->tx->filename);
    }
    return ext;
}


/*
    Search for a handler by request extension. If that fails, use handler custom matching.
 */
static HttpStage *findHandler(HttpConn *conn)
{
    Http        *http;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpStage   *handler;
    HttpLoc     *loc;
    MprHash     *hp;
    char        *path;
    int         next;

    http = conn->http;
    rx = conn->rx;
    tx = conn->tx;
    loc = rx->loc;
    handler = 0;

    mprAssert(rx->uri);
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);
    mprAssert(rx->pathInfo);

    if (rx->pathInfo == 0) {
        handler = http->passHandler;

    } else if (tx->extension) {
        handler = checkHandler(conn, httpGetHandlerByExtension(loc, tx->extension));

    } else if (!tx->fileInfo.valid) {
        /*
            URI has no extension, check if the addition of configured  extensions results in a valid filename.
         */
        for (path = 0, hp = 0; (hp = mprGetNextHash(loc->extensions, hp)) != 0; ) {
            handler = (HttpStage*) hp->data;
            if (*hp->key && (handler->flags & HTTP_STAGE_MISSING_EXT)) {
                path = sjoin(tx->filename, ".", hp->key, NULL);
                if (mprGetPathInfo(path, &tx->fileInfo) == 0) {
                    mprLog(5, "findHandler: Adding extension, new path %s\n", path);
#if UNUSED
                    tx->filename = path;
#endif
                    httpSetUri(conn, sjoin(rx->uri, ".", hp->key, NULL), NULL);
                    break;
                }
            }
        }
        if (hp == 0) {
            handler = 0;
        }
    }
    if (handler == 0) {
        /* Failed to match by extension, so perform custom handler matching on all defined handlers */
        for (next = 0; (handler = mprGetNextItem(loc->handlers, &next)) != 0; ) {
            if (checkHandler(conn, handler)) {
                break;
            }
        }
    }
    if (handler == 0 && (handler = httpGetHandlerByExtension(loc, "")) == 0) {
        handler = http->passHandler;
        if (!(rx->flags & (HTTP_OPTIONS | HTTP_TRACE))) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing handler to match request %s", rx->pathInfo);
        }
    }
    return handler;
}


static HttpStage *mapToFile(HttpConn *conn, HttpStage *handler)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpHost    *host;
    MprPath     *info, ginfo;
    char        *gfile;

    rx = conn->rx;
    tx = conn->tx;
    host = conn->host;
    info = &tx->fileInfo;

    mprAssert(handler);
    mprAssert(tx->filename);
    mprAssert(info->checked);

    if (!handler || (handler->flags & HTTP_STAGE_VIRTUAL)) {
        return handler;
    }
    if ((rx->dir = httpLookupBestDir(host, tx->filename)) == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Missing directory block for %s", tx->filename);
    } else {
        rx->auth = rx->dir->auth;
        if (info->isDir) {
            handler = processDirectory(conn, handler);

        } else if (!info->valid) {
            /*
                File not found. See if a compressed variant exists.
             */
            if (rx->acceptEncoding && strstr(rx->acceptEncoding, "gzip") != 0) {
                gfile = mprAsprintf("%s.gz", tx->filename);
                if (mprGetPathInfo(gfile, &ginfo) == 0) {
                    tx->filename = gfile;
                    tx->fileInfo = ginfo;
                    tx->etag = mprAsprintf("\"%x-%Lx-%Lx\"", ginfo.inode, ginfo.size, ginfo.mtime);
                    httpSetHeader(conn, "Content-Encoding", "gzip");
                    return handler;
                }
            }
            if (!(rx->flags & HTTP_PUT) && (handler->flags & HTTP_STAGE_VERIFY_ENTITY) && 
                    (rx->auth == 0 || rx->auth->type == 0)) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", tx->filename);
            }
        }
    }
    return handler;
}


/*
    Manage requests to directories. This will either do an external redirect back to the browser or do an internal 
    (transparent) redirection and serve different content back to the browser. This routine may modify the requested 
    URI and/or the request handler.
 */
static HttpStage *processDirectory(HttpConn *conn, HttpStage *handler)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprPath     *info;
    HttpHost    *host;
    char        *path, *index;

    rx = conn->rx;
    tx = conn->tx;
    host = conn->host;
    info = &tx->fileInfo;

    mprAssert(rx->dir);
    mprAssert(rx->pathInfo);
    mprAssert(info->isDir);

    index = rx->dir->indexName;
    if (rx->pathInfo[slen(rx->pathInfo) - 1] == '/') {
        /*  
            Internal directory redirections
         */
        path = mprJoinPath(tx->filename, index);
        if (mprPathExists(path, R_OK)) {
            /*  
                Index file exists, so do an internal redirect to it. Client will not be aware of this happening.
                Return zero so the request will be rematched on return.
             */
            httpSetUri(conn, addIndexToUrl(conn, index), NULL);
            return 0;
        }
    } else {
        /*  
            External redirect. Ask the client to re-issue a request for a new location. See if an index exists and if so, 
            construct a new location for the index. If the index can't be accessed, append a "/" to the URI and redirect.
         */
        if (rx->parsedUri->query && rx->parsedUri->query[0]) {
            path = mprAsprintf("%s/%s?%s", rx->pathInfo, index, rx->parsedUri->query);
        } else {
            path = mprJoinPath(rx->pathInfo, index);
        }
        if (!mprPathExists(path, R_OK)) {
            path = sjoin(rx->pathInfo, "/", NULL);
        }
        httpRedirect(conn, HTTP_CODE_MOVED_PERMANENTLY, path);
        handler = conn->http->passHandler;
    }
    return handler;
}


#if UNUSED
static bool fileExists(cchar *path) {
    if (mprPathExists(path, R_OK)) {
        return 1;
    }
#if BLD_WIN_LIKE
{
    char    *file;
    file = sjoin(path, ".exe", NULL);
    if (mprPathExists(file, R_OK)) {
        return 1;
    }
    file = sjoin(path, ".bat", NULL);
    if (mprPathExists(file, R_OK)) {
        return 1;
    }
}
#endif
    return 0;
}
#endif


static void setScriptName(HttpConn *conn)
{
    HttpAlias   *alias;
    HttpRx      *rx;
    HttpTx      *tx;
    char        *cp, *extraPath, *start, *pathInfo;
    int         scriptLen;

    rx = conn->rx;
    tx = conn->tx;
    alias = rx->alias;

    /*
        Find the script name in the filename. This is assumed to be either:
        - The original filename up to and including first portion containing a "."
        - The entire original filename
        Once found, set the scriptName and trim the extraPath from both the filename and the pathInfo
     */
    start = &tx->filename[strlen(alias->filename)];
    if ((cp = strchr(start, '.')) != 0 && (extraPath = strchr(cp, '/')) != 0) {
        rx->scriptName = sclone(rx->pathInfo);
        scriptLen = alias->prefixLen + extraPath - start;
        rx->pathInfo = sclone(&rx->scriptName[scriptLen]);
        rx->scriptName[0] = '\0';
        *extraPath = '\0';
        httpMapToStorage(conn);
        if (rx->pathInfo[0]) {
            rx->pathTranslated = httpMakeFilename(conn, alias, rx->pathInfo, 0);
        }
    } else {
        pathInfo = rx->pathInfo;
        rx->scriptName = rx->pathInfo;
        rx->pathInfo = 0;
    }
}


static char *addIndexToUrl(HttpConn *conn, cchar *index)
{
    HttpRx      *rx;
    char        *path;

    rx = conn->rx;
    path = mprJoinPath(rx->pathInfo, index);
    if (rx->parsedUri->query && rx->parsedUri->query[0]) {
        return srejoin(path, "?", rx->parsedUri->query, NULL);
    }
    return path;
}


static HttpStage *checkHandler(HttpConn *conn, HttpStage *stage)
{
    HttpRx   *rx;

    rx = conn->rx;
    if (stage == 0) {
        return 0;
    }
    if ((stage->flags & HTTP_STAGE_ALL & rx->flags) == 0) {
        return 0;
    }
    if (stage->match && !(stage->flags & HTTP_STAGE_UNLOADED)) {
        /* Can't have match routines on unloaded modules */
        if (!stage->match(conn, stage)) {
            return 0;
        }
    }
    return stage;
}


char *httpMakeFilename(HttpConn *conn, HttpAlias *alias, cchar *url, bool skipAliasPrefix)
{
    cchar   *seps;
    char    *path;
    int     len;

    mprAssert(alias);
    mprAssert(url);

    if (skipAliasPrefix) {
        url += alias->prefixLen;
    }
    while (*url == '/') {
        url++;
    }
    len = (int) strlen(alias->filename);
    if ((path = mprAlloc(len + strlen(url) + 2)) == 0) {
        return 0;
    }
    strcpy(path, alias->filename);
    if (*url) {
        seps = mprGetPathSeparators(path);
        path[len++] = seps[0];
        strcpy(&path[len], url);
    }
    return mprGetNativePath(path);
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
/************************************************************************/
/*
 *  End of file "../src/match.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/netConnector.c"
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
static ssize buildNetVec(HttpQueue *q);
static void freeNetPackets(HttpQueue *q, ssize written);
static void netOutgoingService(HttpQueue *q);

/*  
    Initialize the net connector
 */
int httpOpenNetConnector(Http *http)
{
    HttpStage     *stage;

    if ((stage = httpCreateConnector(http, "netConnector", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->outgoingService = netOutgoingService;
    http->netConnector = stage;
    return 0;
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
    
    if (conn->sock == 0 || conn->writeComplete) {
        return;
    }
    if (tx->flags & HTTP_TX_NO_BODY) {
        httpDiscardData(q, 1);
    }
    if ((tx->bytesWritten + q->count) > conn->limits->transmissionBodySize) {
        httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE, 
            "Http transmission aborted. Exceeded transmission max body of %d bytes", conn->limits->transmissionBodySize);
        if (tx->flags & HTTP_TX_HEADERS_CREATED) {
            /* Must disconnect as the client must be notified somehow */
            mprDisconnectSocket(conn->sock);
            httpCompleteWriting(conn);
            return;
        }
    }
    if (tx->flags & HTTP_TX_SENDFILE) {
        /* Relay via the send connector */
        if (tx->file == 0) {
            if (tx->flags & HTTP_TX_HEADERS_CREATED) {
                tx->flags &= ~HTTP_TX_SENDFILE;
            } else {
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
        LOG(5, "Net connector written %d", written);
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
            httpConnError(conn, HTTP_CODE_COMMS_ERROR, "Write error %d", errCode);
            httpCompleteWriting(conn);
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
            httpCompleteWriting(conn);
        } else {
            httpWritable(conn);
        }
    }
}


/*
    Build the IO vector. Return the count of bytes to be written. Return -1 for EOF.
 */
static ssize buildNetVec(HttpQueue *q)
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
    if (httpShouldTrace(conn, HTTP_TRACE_TX, item, NULL) >= 0) {
        httpTraceContent(conn, HTTP_TRACE_TX, item, packet, 0, tx->bytesWritten);
    }
}


static void freeNetPackets(HttpQueue *q, ssize bytes)
{
    HttpPacket    *packet;
    ssize         len;

    mprAssert(q->count >= 0);
    mprAssert(bytes >= 0);

    while ((packet = q->first) != 0) {
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
        mprAssert(bytes >= 0);
        if (bytes == 0 && (q->first == NULL || !(q->first->flags & HTTP_PACKET_END))) {
            break;
        }
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
/************************************************************************/
/*
 *  End of file "../src/netConnector.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/packet.c"
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
        packet->content = mprCreateBuf(size < 0 ? HTTP_BUFSIZE: size, -1);
        if (packet->content == 0) {
            return 0;
        }
    }
    mprLog(5, "DEBUG: httpCreate new packet %d\n", packet->entityLength);
    return packet;
}


static void managePacket(HttpPacket *packet, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(packet->prefix);
        mprMark(packet->content);
        mprMark(packet->suffix);
        /*  
            Move to manageQueue to reduce stack depth
            mprMark(packet->next);
         */
    }
}


#if FUTURE
void httpFreePacket(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;

    conn = q->conn;
    rx = conn->rx;

    if (rx == 0 || packet->content == 0 || packet->content->buflen < HTTP_BUFSIZE || mprIsParent(conn, packet)) {
        /* 
            Don't bother recycling non-content, small packets or packets owned by the connection
            We only store packets owned by the request and not by the connection on the free list.
         */
        return;
    }
    /*  
        Add to the packet free list for recycling
        MOB -- need some thresholds to manage this incase it gets too big
     */
    mprAssert(packet->content);
    mprFlushBuf(packet->content);
    packet->prefix = 0;
    packet->suffix = 0;
    packet->entityLength = 0;
    packet->flags = 0;
    packet->next = rx->freePackets;
    rx->freePackets = packet;
} 
#endif


HttpPacket *httpCreateDataPacket(ssize size)
{
    HttpPacket    *packet;

    packet = httpCreatePacket(size);
    if (packet == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_DATA;
    return packet;
}


HttpPacket *httpCreateEndPacket()
{
    HttpPacket    *packet;

    packet = httpCreatePacket(0);
    if (packet == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_END;
    return packet;
}


HttpPacket *httpCreateHeaderPacket()
{
    HttpPacket    *packet;

    packet = httpCreatePacket(HTTP_BUFSIZE);
    if (packet == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_HEADER;
    return packet;
}


/* 
   Get the next packet from the queue
 */
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
    Join a packet onto the service queue
 */
void httpJoinPacketForService(HttpQueue *q, HttpPacket *packet, bool serviceQ)
{
    if (q->first == 0) {
        /*  
            Just use the service queue as a holding queue while we aggregate the post data.
         */
        httpPutForService(q, packet, 0);
    } else {
        q->count += httpGetPacketLength(packet);
        if (q->first && httpGetPacketLength(q->first) == 0) {
            packet = q->first->next;
            q->first = packet;
        } else {
            /*
                Aggregate all data into one packet and free the packet.
             */
            httpJoinPacket(q->first, packet);
        }
    }
    if (serviceQ && !(q->flags & HTTP_QUEUE_DISABLED))  {
        httpScheduleQueue(q);
    }
}


/*  
    Join two packets by pulling the content from the second into the first.
 */
int httpJoinPacket(HttpPacket *packet, HttpPacket *p)
{
    int     len;

    len = httpGetPacketLength(p);

    if (mprPutBlockToBuf(packet->content, mprGetBufStart(p->content), len) != len) {
        return MPR_ERR_MEMORY;
    }
    return 0;
}


/*
    Join queue packets up to the maximum of the given size and the downstream queue packet size.
 */
void httpJoinPackets(HttpQueue *q, ssize size)
{
    HttpPacket  *first, *next;
    ssize       maxPacketSize;

    if ((first = q->first) != 0 && first->next) {
        maxPacketSize = min(q->nextQ->packetSize, size);
        while ((next = first->next) != 0) {
            if (next->content && (httpGetPacketLength(first) + httpGetPacketLength(next)) < maxPacketSize) {
                httpJoinPacket(first, next);
                first->next = next->next;
            } else {
                break;
            }
        }
    }
}


/*  
    Put the packet back at the front of the queue
 */
void httpPutBackPacket(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(packet);
    mprAssert(packet->next == 0);
    
    packet->next = q->first;

    if (q->first == 0) {
        q->last = packet;
    }
    q->first = packet;
    mprAssert(httpGetPacketLength(packet) >= 0);
    q->count += httpGetPacketLength(packet);
    mprAssert(q->count >= 0);
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
    Split a packet if required so it fits in the downstream queue. Put back the 2nd portion of the split packet on the queue.
    Ensure that the packet is not larger than "size" if it is greater than zero.
 */
int httpResizePacket(HttpQueue *q, HttpPacket *packet, ssize size)
{
    HttpPacket  *tail;
    ssize       len;
    
    if (size <= 0) {
        size = MAXINT;
    }

    /*  
        Calculate the size that will fit
     */
    len = packet->content ? httpGetPacketLength(packet) : packet->entityLength;
    size = min(size, len);
    size = min(size, q->nextQ->max);
    size = min(size, q->nextQ->packetSize);

    if (size == 0) {
        /* Can't fit anything downstream, no point splitting yet */
        return 0;
    }
    if (size == len) {
        return 0;
    }
    tail = httpSplitPacket(packet, size);
    if (tail == 0) {
        return MPR_ERR_MEMORY;
    }
    httpPutBackPacket(q, tail);
    return 0;
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
    if (orig->suffix) {
        packet->suffix = mprCloneBuf(orig->suffix);
    }
    packet->flags = orig->flags;
    packet->entityLength = orig->entityLength;
    return packet;
}


/*  
    Pass to a queue
 */
void httpSendPacket(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(packet);
    
    mprAssert(q->put);
    q->put(q, packet);
}


/*  
    Pass to the next queue
 */
void httpSendPacketToNext(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(packet);
    
    mprAssert(q->nextQ->put);
    q->nextQ->put(q->nextQ, packet);
}


void httpSendPackets(HttpQueue *q)
{
    HttpPacket    *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        httpSendPacketToNext(q, packet);
    }
}


/*  
    Split a packet at a given offset and return a new packet containing the data after the offset.
    The suffix data migrates to the new packet. 
 */
HttpPacket *httpSplitPacket(HttpPacket *orig, ssize offset)
{
    HttpPacket  *packet;
    ssize       count, size;

    if (offset >= httpGetPacketLength(orig)) {
        mprAssert(0);
        return 0;
    }
    count = httpGetPacketLength(orig) - offset;
    size = max(count, HTTP_BUFSIZE);
    size = HTTP_PACKET_ALIGN(size);

    if ((packet = httpCreatePacket((orig->content == 0) ? 0 : size)) == 0) {
        return 0;
    }
    packet->flags = orig->flags;

    if (orig->entityLength) {
        orig->entityLength = offset;
        packet->entityLength = count;
    }

#if FUTURE
    /*
        Suffix migrates to the new packet (Not currently used)
     */
    if (packet->suffix) {
        packet->suffix = orig->suffix;
        orig->suffix = 0;
    }
#endif

    if (orig->content && httpGetPacketLength(orig) > 0) {
        mprAdjustBufEnd(orig->content, -count);
        if (mprPutBlockToBuf(packet->content, mprGetBufEnd(orig->content), count) != count) {
            return 0;
        }
#if BLD_DEBUG
        mprAddNullToBuf(orig->content);
#endif
    }
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
/************************************************************************/
/*
 *  End of file "../src/packet.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/passHandler.c"
 */
/************************************************************************/

/*
    passHandler.c -- Pass through handler

    This handler simply relays all content onto a connector. It is used to when there is no handler defined 
    and to convey errors when the actual handler fails.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




void httpHandleOptionsTrace(HttpQueue *q)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpConn    *conn;
    int         flags;

    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;

#if UNUSED
    /* Called only for the send queue */
    if (q->pair) {
        q->pair->max = q->max;
        q->pair->packetSize = q->packetSize;
    }
#endif

    if (rx->flags & HTTP_TRACE) {
        if (!conn->limits->enableTraceMethod) {
            tx->status = HTTP_CODE_NOT_ACCEPTABLE;
            httpFormatBody(conn, "Trace Request Denied", "<p>The TRACE method is disabled on this server.</p>");
        } else {
            tx->altBody = mprAsprintf("%s %s %s\r\n", rx->method, rx->uri, conn->protocol);
        }

    } else if (rx->flags & HTTP_OPTIONS) {
        flags = tx->traceMethods;
        httpSetHeader(conn, "Allow", "OPTIONS%s%s%s%s%s%s",
            (conn->limits->enableTraceMethod) ? ",TRACE" : "",
            (flags & HTTP_STAGE_GET) ? ",GET" : "",
            (flags & HTTP_STAGE_HEAD) ? ",HEAD" : "",
            (flags & HTTP_STAGE_POST) ? ",POST" : "",
            (flags & HTTP_STAGE_PUT) ? ",PUT" : "",
            (flags & HTTP_STAGE_DELETE) ? ",DELETE" : "");
        tx->length = 0;
    }
}


static void openPass(HttpQueue *q)
{
#if UNUSED
    /* Called only for the send queue */
    if (q->pair) {
        q->pair->max = q->max;
        q->pair->packetSize = q->packetSize;
    }
#endif

    if (q->conn->rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        httpHandleOptionsTrace(q);
    }
}


static void processPass(HttpQueue *q)
{
    httpFinalize(q->conn);
}


int httpOpenPassHandler(Http *http)
{
    HttpStage     *stage;

    if ((stage = httpCreateHandler(http, "passHandler", HTTP_STAGE_ALL | HTTP_STAGE_VIRTUAL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->passHandler = stage;
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
/************************************************************************/
/*
 *  End of file "../src/passHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/pipeline.c"
 */
/************************************************************************/

/*
    pipeline.c -- HTTP pipeline processing.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static bool matchFilter(HttpConn *conn, HttpStage *filter);
static void setEnvironment(HttpConn *conn);

/*  
    Create processing pipeline
 */
void httpCreatePipeline(HttpConn *conn, HttpLoc *loc, HttpStage *proposedHandler)
{
    Http        *http;
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q, *qhead, *rq, *rqhead;
    HttpStage   *stage, *filter;
    int         next;

    http = conn->http;
    rx = conn->rx;
    tx = conn->tx;

    mprAssert(!conn->server || tx->filename);
    loc = (loc) ? loc : http->clientLocation;

    tx->outputPipeline = mprCreateList(-1, 0);
    tx->handler = proposedHandler ? proposedHandler : http->passHandler;

    mprAddItem(tx->outputPipeline, tx->handler);

    if (loc->outputStages) {
        for (next = 0; (filter = mprGetNextItem(loc->outputStages, &next)) != 0; ) {
            if (matchFilter(conn, filter)) {
                mprAddItem(tx->outputPipeline, filter);
            }
        }
    }
    if (tx->connector == 0) {
        //  MOB loc->connector is set to never selecting sendConnector
        if (loc && loc->connector) {
            tx->connector = loc->connector;
        } else if (tx->handler == http->fileHandler && !rx->ranges && !conn->secure && tx->chunkSize <= 0) {
            //  MOB - should not use this if tracing content is requested
            tx->connector = http->sendConnector;
        } else {
            tx->connector = http->netConnector;
        }
    }
    mprAddItem(tx->outputPipeline, tx->connector);

    /*  
        Create the receive pipeline for this request
     */
    if (rx->needInputPipeline) {
        rx->inputPipeline = mprCreateList(-1, 0);
        mprAddItem(rx->inputPipeline, http->netConnector);
        if (loc) {
            for (next = 0; (filter = mprGetNextItem(loc->inputStages, &next)) != 0; ) {
                if (!matchFilter(conn, filter)) {
                    continue;
                }
                mprAddItem(rx->inputPipeline, filter);
            }
        }
        mprAddItem(rx->inputPipeline, tx->handler);
    }
    
    /* Incase a filter changed the handler */
    mprSetItem(tx->outputPipeline, 0, tx->handler);

#if UNUSED && FUTURE
    if (tx->handler->flags & HTTP_STAGE_THREAD && !conn->threaded) {
        /* Start with dispatcher disabled. Conn.c will enable */
        tx->dispatcher = mprCreateDispatcher(tx->handler->name, 0);
        conn->dispatcher = tx->dispatcher;
    }
#endif

    /*  Create the outgoing queue heads and open the queues */
    q = tx->queue[HTTP_QUEUE_TRANS];
    for (next = 0; (stage = mprGetNextItem(tx->outputPipeline, &next)) != 0; ) {
        q = httpCreateQueue(conn, stage, HTTP_QUEUE_TRANS, q);
    }

    /*  Create the incoming queue heads and open the queues.  */
    q = tx->queue[HTTP_QUEUE_RECEIVE];
    for (next = 0; (stage = mprGetNextItem(rx->inputPipeline, &next)) != 0; ) {
        q = httpCreateQueue(conn, stage, HTTP_QUEUE_RECEIVE, q);
    }
    setEnvironment(conn);

    conn->writeq = tx->queue[HTTP_QUEUE_TRANS]->nextQ;
    conn->readq = tx->queue[HTTP_QUEUE_RECEIVE]->prevQ;
    httpPutForService(conn->writeq, httpCreateHeaderPacket(), 0);

    /*  
        Pair up the send and receive queues
     */
    qhead = tx->queue[HTTP_QUEUE_TRANS];
    rqhead = tx->queue[HTTP_QUEUE_RECEIVE];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        for (rq = rqhead->nextQ; rq != rqhead; rq = rq->nextQ) {
            if (q->stage == rq->stage) {
                q->pair = rq;
                rq->pair = q;
            }
        }
    }

    /*  
        Open the queues (keep going on errors)
        Open in reverse order so the handler is opened last. This lets authFilter go early.
     */
    qhead = tx->queue[HTTP_QUEUE_TRANS];
    for (q = qhead->prevQ; q != qhead; q = q->prevQ) {
        if (q->open && !(q->flags & HTTP_QUEUE_OPEN)) {
            q->flags |= HTTP_QUEUE_OPEN;
            httpOpenQueue(q, conn->tx->chunkSize);
        }
    }

    if (rx->needInputPipeline) {
        qhead = tx->queue[HTTP_QUEUE_RECEIVE];
        for (q = qhead->prevQ; q != qhead; q = q->prevQ) {
            if (q->open && !(q->flags & HTTP_QUEUE_OPEN)) {
                if (q->pair == 0 || !(q->pair->flags & HTTP_QUEUE_OPEN)) {
                    q->flags |= HTTP_QUEUE_OPEN;
                    httpOpenQueue(q, conn->tx->chunkSize);
                }
            }
        }
    }
    conn->flags |= HTTP_CONN_PIPE_CREATED;
}


//  MOB - who calls
void httpSetPipeHandler(HttpConn *conn, HttpStage *handler)
{
    conn->tx->handler = (handler) ? handler : conn->http->passHandler;
}


void httpSetSendConnector(HttpConn *conn, cchar *path)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;
    ssize       maxBody;

    tx = conn->tx;
    tx->flags |= HTTP_TX_SENDFILE;
    tx->filename = sclone(path);
    maxBody = conn->limits->transmissionBodySize;

    qhead = tx->queue[HTTP_QUEUE_TRANS];
    for (q = conn->writeq; q != qhead; q = q->nextQ) {
        q->max = maxBody;
        q->packetSize = maxBody;
    }
}


void httpDestroyPipeline(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;
    int         i;

    if (conn->flags & HTTP_CONN_PIPE_CREATED && conn->tx) {
        tx = conn->tx;
        for (i = 0; i < HTTP_MAX_QUEUE; i++) {
            qhead = tx->queue[i];
            for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
                if (q->close && q->flags & HTTP_QUEUE_OPEN) {
                    q->flags &= ~HTTP_QUEUE_OPEN;
                    q->stage->close(q);
                }
            }
        }
        conn->flags &= ~HTTP_CONN_PIPE_CREATED;
    }
}


void httpStartPipeline(HttpConn *conn)
{
    HttpQueue   *qhead, *q;
    HttpTx      *tx;
    
    tx = conn->tx;
    qhead = tx->queue[HTTP_QUEUE_TRANS];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        if (q->start && !(q->flags & HTTP_QUEUE_STARTED)) {
            q->flags |= HTTP_QUEUE_STARTED;
            HTTP_TIME(conn, q->stage->name, "start", q->stage->start(q));
        }
    }

    if (conn->rx->needInputPipeline) {
        qhead = tx->queue[HTTP_QUEUE_RECEIVE];
        for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
            if (q->start && !(q->flags & HTTP_QUEUE_STARTED)) {
                if (q->pair == 0 || !(q->pair->flags & HTTP_QUEUE_STARTED)) {
                    q->flags |= HTTP_QUEUE_STARTED;
                    HTTP_TIME(conn, q->stage->name, "start", q->stage->start(q));
                }
            }
        }
    }
}


void httpProcessPipeline(HttpConn *conn)
{
    HttpQueue   *q;
    
    q = conn->tx->queue[HTTP_QUEUE_TRANS]->nextQ;
    if (q->stage->process) {
        HTTP_TIME(conn, q->stage->name, "process", q->stage->process(q));
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
    qhead = tx->queue[HTTP_QUEUE_TRANS];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        httpDiscardData(q, 1);
    }
}


/*
    Create the form variables based on the URI query. Also create formVars for CGI style programs (cgi | egi)
 */
static void setEnvironment(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;

    rx = conn->rx;
    tx = conn->tx;

    mprAssert(tx->handler);

    if (tx->handler->flags & (HTTP_STAGE_VARS | HTTP_STAGE_ENV_VARS)) {
        rx->formVars = mprCreateHash(HTTP_MED_HASH_SIZE, 0);
        if (rx->parsedUri->query && (tx->handler->flags & HTTP_STAGE_VARS)) {
            httpAddVars(conn, rx->parsedUri->query, slen(rx->parsedUri->query));
        }
    }
    if (tx->handler->flags & HTTP_STAGE_ENV_VARS) {
        httpCreateEnvVars(conn);
    }
}


/*
    Match a filter by extension
 */
static bool matchFilter(HttpConn *conn, HttpStage *filter)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (filter->match) {
        return filter->match(conn, filter);
    }
    if (filter->extensions && tx->extension) {
        return mprLookupHash(filter->extensions, tx->extension) != 0;
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
/************************************************************************/
/*
 *  End of file "../src/pipeline.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/queue.c"
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
    httpInitQueue(conn, q, stage->name);
    httpInitSchedulerQueue(q);

    q->conn = conn;
    q->stage = stage;
    q->close = stage->close;
    q->open = stage->open;
    q->start = stage->start;
    q->direction = dir;

    if (dir == HTTP_QUEUE_TRANS) {
        q->put = stage->outgoingData;
        q->service = stage->outgoingService;
        
    } else {
        q->put = stage->incomingData;
        q->service = stage->incomingService;
    }
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
        mprMark(q->stage);
        mprMark(q->conn);
        mprMark(q->nextQ);
        mprMark(q->prevQ);
        mprMark(q->scheduleNext);
        mprMark(q->schedulePrev);
        mprMark(q->pair);
        mprMark(q->last);
        mprMark(q->queueData);
        for (packet = q->first; packet; packet = packet->next) {
            mprMark(packet);
        }
        mprMark(q->queueData);
        if (q->nextQ && q->nextQ->stage) {
            /* Not a queue head */
            mprMark(q->nextQ);
        }
    }
}


#if UNUSED
void httpMarkQueueHead(HttpQueue *q)
{
    if (q->nextQ && q->nextQ->stage) {
        mprMark(q->nextQ);
    }
}
#endif


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
    Remove all data from non-header, non-eof packets in the queue. If removePackets is true, actually remove the packet too.
 */
void httpDiscardData(HttpQueue *q, bool removePackets)
{
    HttpPacket  *packet, *prev, *next;
    ssize       len;

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
    int         oldMode;

    conn = q->conn;
    LOG(6, "httpFlushQueue blocking %d", blocking);
    mprAssert(conn->sock);

    if (q->flags & HTTP_QUEUE_DISABLED || conn->sock == 0) {
        return 0;
    }
    do {
        oldMode = mprSetSocketBlockingMode(conn->sock, blocking);
        httpScheduleQueue(q);
        next = q->nextQ;
        if (next->count >= next->max) {
            httpScheduleQueue(next);
        }
        httpServiceQueues(conn);
        if (conn->sock == 0) {
            break;
        }
        mprSetSocketBlockingMode(conn->sock, oldMode);
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
    MOB - rename append
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
        mprLog(2, "Loading module for %s", stage->name);
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
        HTTP_TIME(q->conn, q->stage->name, "open", q->stage->open(q));
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
    HttpRx      *rx;
    MprBuf      *content;
    ssize       nbytes, len;

    q = conn->readq;
    rx = conn->rx;
    
    while (q->count == 0 && !conn->async && conn->sock && (conn->state <= HTTP_STATE_CONTENT)) {
        httpServiceQueues(conn);
#if UNUSED
        events = MPR_READABLE;
        if (conn->sock && !mprSocketHasPendingData(conn->sock)) {
            if (mprIsSocketEof(conn->sock)) {
                break;
            }
            inactivityTimeout = conn->limits->inactivityTimeout ? conn->limits->inactivityTimeout : INT_MAX;
            events = mprWaitForSingleIO(conn->sock->fd, MPR_READABLE, inactivityTimeout);
        }
        if (events) {
            httpCallEvent(conn, MPR_READABLE);
        }
#else
        if (conn->sock) {
            httpWait(conn, 0, MPR_TIMEOUT_SOCKETS);
        }
#endif
    }
    //  MOB - better place for this?
    conn->lastActivity = conn->http->now;

    for (nbytes = 0; size > 0 && q->count > 0; ) {
        if ((packet = q->first) == 0) {
            break;
        }
        content = packet->content;
        len = mprGetBufLength(content);
        len = min(len, size);
        if (len > 0) {
            len = mprGetBlockFromBuf(content, buf, len);
        }
        rx->readContent += len;
        buf += len;
        size -= len;
        q->count -= len;
        nbytes += len;
        if (mprGetBufLength(content) == 0) {
            httpGetPacket(q);
        }
    }
    return nbytes;
}


int httpIsEof(HttpConn *conn) 
{
    return conn->rx == 0 || conn->rx->eof;
}


char *httpReadString(HttpConn *conn)
{
    HttpRx      *rx;
    char        *content;
    ssize       remaining, sofar, nbytes;

    rx = conn->rx;

    if (rx->length > 0) {
        content = mprAlloc(rx->length + 1);
        remaining = rx->length;
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
    HttpQueue   *next;
    ssize       size;

    next = q->nextQ;

    size = packet->content ? mprGetBufLength(packet->content) : 0;
    if (size == 0 || (size <= next->packetSize && (size + next->count) <= next->max)) {
        return 1;
    }
    if (httpResizePacket(q, packet, 0) < 0) {
        return 0;
    }
    size = httpGetPacketLength(packet);
    if (size <= next->packetSize && (size + next->count) <= next->max) {
        return 1;
    }
    /*  
        The downstream queue is full, so disable the queue and mark the downstream queue as full and service 
        if immediately if not disabled.  
     */
    mprLog(7, "Disable queue %s", q->owner);
    httpDisableQueue(q);
    next->flags |= HTTP_QUEUE_FULL;
    if (!(next->flags & HTTP_QUEUE_DISABLED)) {
        httpScheduleQueue(next);
    }
    return 0;
}


/*  
    Write a block of data. This is the lowest level write routine for data. This will buffer the data and flush if
    the queue buffer is full.
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
    if (tx->finalized) {
        return MPR_ERR_CANT_WRITE;
    }
    for (written = 0; size > 0; ) {
        LOG(6, "httpWriteBlock q_count %d, q_max %d", q->count, q->max);
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
            httpPutForService(q, packet, 0);
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


ssize httpWrite(HttpQueue *q, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;
    
    va_start(vargs, fmt);
    buf = mprAsprintfv(fmt, vargs);
    va_end(vargs);
    return httpWriteString(q, buf);
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
/************************************************************************/
/*
 *  End of file "../src/queue.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/rangeFilter.c"
 */
/************************************************************************/

/*
    rangeFilter.c - Ranged request filter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void createRangeBoundary(HttpConn *conn);
static HttpPacket *createRangePacket(HttpConn *conn, HttpRange *range);
static HttpPacket *createFinalRangePacket(HttpConn *conn);
static void incomingRangeData(HttpQueue *q, HttpPacket *packet);
static void outgoingRangeService(HttpQueue *q);
static bool fixRangeLength(HttpConn *conn);
static bool matchRange(HttpConn *conn, HttpStage *handler);
static void rangeService(HttpQueue *q, HttpRangeProc fill);


int httpOpenRangeFilter(Http *http)
{
    HttpStage     *filter;

    if ((filter = httpCreateFilter(http, "rangeFilter", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->rangeFilter = filter;
    http->rangeService = rangeService;
    filter->match = matchRange; 
    filter->outgoingService = outgoingRangeService; 
    filter->incomingData = incomingRangeData; 
    return 0;
}


static bool matchRange(HttpConn *conn, HttpStage *handler)
{
    return (conn->rx->ranges) ? 1 : 0;;
}


/*
    The RangeFilter does nothing for incoming data. The rx understands range headers
 */
static void incomingRangeData(HttpQueue *q, HttpPacket *packet)
{
    httpSendPacketToNext(q, packet);
}


/*  
    Apply ranges to outgoing data. 
 */
static void rangeService(HttpQueue *q, HttpRangeProc fill)
{
    HttpPacket  *packet;
    HttpRange   *range;
    HttpConn    *conn;
    HttpRx      *rx;
    HttpTx      *tx;
    ssize       bytes, count, endpos;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    range = tx->currentRange;

    if (!(q->flags & HTTP_QUEUE_SERVICED)) {
        if (tx->entityLength < 0 && q->last->flags & HTTP_PACKET_END) {

           /*   Compute an entity length. This allows negative ranges computed from the end of the data.
            */
           tx->entityLength = q->count;
        }
        if (tx->status != HTTP_CODE_OK || !fixRangeLength(conn)) {
            httpSendPackets(q);
            httpRemoveQueue(q);
            return;
        }
        if (rx->ranges->next) {
            createRangeBoundary(conn);
        }
        tx->status = HTTP_CODE_PARTIAL;
    }

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!(packet->flags & HTTP_PACKET_DATA)) {
            if (packet->flags & HTTP_PACKET_END && tx->rangeBoundary) {
                httpSendPacketToNext(q, createFinalRangePacket(conn));
            }
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            httpSendPacketToNext(q, packet);
            continue;
        }

        /*  Process the current packet over multiple ranges ranges until all the data is processed or discarded.
         */
        bytes = packet->content ? mprGetBufLength(packet->content) : packet->entityLength;
        while (range && bytes > 0) {

            endpos = tx->pos + bytes;
            if (endpos < range->start) {
                /* Packet is before the next range, so discard the entire packet */
                tx->pos += bytes;
                break;

            } else if (tx->pos > range->end) {
                /* Missing some output - should not happen */
                mprAssert(0);

            } else if (tx->pos < range->start) {
                /*  Packets starts before range with some data in range so skip some data */
                count = (range->start - tx->pos);
                bytes -= count;
                tx->pos += count;
                if (packet->content == 0) {
                    packet->entityLength -= count;
                }
                if (packet->content) {
                    mprAdjustBufStart(packet->content, count);
                }
                continue;

            } else {
                /* In range */
                mprAssert(range->start <= tx->pos && tx->pos < range->end);
                count = min(bytes, (int) (range->end - tx->pos));
                count = min(count, q->nextQ->packetSize);
                mprAssert(count > 0);
                if (count < bytes) {
                    //  TODO OPT> Only need to resize if this completes all the range data.
                    httpResizePacket(q, packet, count);
                }
                if (!httpWillNextQueueAcceptPacket(q, packet)) {
                    httpPutBackPacket(q, packet);
                    return;
                }
                if (fill) {
                    if ((*fill)(q, packet) < 0) {
                        return;
                    }
                }
                tx->pos += count;
                if (tx->rangeBoundary) {
                    httpSendPacketToNext(q, createRangePacket(conn, range));
                }
                httpSendPacketToNext(q, packet);
                if (tx->pos >= range->end) {
                    range = range->next;
                }
                break;
            }
        }
    }
    tx->currentRange = range;
}


static void outgoingRangeService(HttpQueue *q)
{
    rangeService(q, NULL);
}


/*  Create a range boundary packet
 */
static HttpPacket *createRangePacket(HttpConn *conn, HttpRange *range)
{
    HttpPacket  *packet;
    HttpTx      *tx;
    char        lenBuf[16];

    tx = conn->tx;

    if (tx->entityLength >= 0) {
        itos(lenBuf, sizeof(lenBuf), tx->entityLength, 10);
    } else {
        lenBuf[0] = '*';
        lenBuf[1] = '\0';
    }
    packet = httpCreatePacket(HTTP_RANGE_BUFSIZE);
    packet->flags |= HTTP_PACKET_RANGE;
    mprPutFmtToBuf(packet->content, 
        "\r\n--%s\r\n"
        "Content-Range: bytes %d-%d/%s\r\n\r\n",
        tx->rangeBoundary, range->start, range->end - 1, lenBuf);
    return packet;
}


/*  Create a final range packet that follows all the data
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


/*  Create a range boundary. This is required if more than one range is requested.
 */
static void createRangeBoundary(HttpConn *conn)
{
    HttpTx      *tx;
    int         when;

    tx = conn->tx;
    mprAssert(tx->rangeBoundary == 0);
    when = (int) conn->http->now;
    tx->rangeBoundary = mprAsprintf("%08X%08X", PTOI(tx) + PTOI(conn) * when, when);
}


/*  Ensure all the range limits are within the entity size limits. Fixup negative ranges.
 */
static bool fixRangeLength(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRange   *range;
    ssize       length;

    rx = conn->rx;
    tx = conn->tx;
    length = tx->entityLength;

    for (range = rx->ranges; range; range = range->next) {
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
                    Can't compute an offset from the end as we don't know the entity length
                 */
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
/************************************************************************/
/*
 *  End of file "../src/rangeFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/rx.c"
 */
/************************************************************************/

/*
    rx.c -- Http receiver. Parses http requests and client responses.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void addMatchEtag(HttpConn *conn, char *etag);
static ssize getChunkPacketSize(HttpConn *conn, MprBuf *buf);
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
static bool processParsed(HttpConn *conn);
static bool processRunning(HttpConn *conn);


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
    rx->needInputPipeline = !conn->server;
    rx->headers = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    return rx;
}


static void manageRx(HttpRx *rx, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(rx->method);
        mprMark(rx->uri);
        mprMark(rx->scriptName);
        mprMark(rx->pathInfo);
        mprMark(rx->conn);
        mprMark(rx->etags);
        mprMark(rx->headerPacket);
        mprMark(rx->headers);
        mprMark(rx->inputPipeline);
        mprMark(rx->loc);
        mprMark(rx->parsedUri);
        mprMark(rx->requestData);
        mprMark(rx->statusMessage);
        mprMark(rx->accept);
        mprMark(rx->acceptCharset);
        mprMark(rx->acceptEncoding);
        mprMark(rx->cookie);
        mprMark(rx->connection);
        mprMark(rx->contentLength);
        mprMark(rx->hostName);
        mprMark(rx->pathTranslated);
        mprMark(rx->pragma);
        mprMark(rx->mimeType);
        mprMark(rx->redirect);
        mprMark(rx->referrer);
        mprMark(rx->userAgent);
        mprMark(rx->formVars);
        mprMark(rx->ranges);
        mprMark(rx->inputRange);
        mprMark(rx->auth);
        mprMark(rx->authAlgorithm);
        mprMark(rx->authDetails);
        mprMark(rx->authStale);
        mprMark(rx->authType);
        mprMark(rx->files);
        mprMark(rx->uploadDir);
        mprMark(rx->alias);
        mprMark(rx->dir);

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
        LOG(7, "httpProcess, state %d, error %d", conn->state, conn->error);

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
    HttpTx      *tx;
    ssize       len;
    char        *start, *end;

    if (packet == NULL) {
        return 0;
    }
    if (conn->rx == NULL) {
        conn->rx = httpCreateRx(conn);
        conn->tx = httpCreateTx(conn, NULL);
    }
    rx = conn->rx;
    tx = conn->tx;
    
    if ((len = mprGetBufLength(packet->content)) == 0) {
        return 0;
    }
    start = mprGetBufStart(packet->content);
    if ((end = scontains(start, "\r\n\r\n", len)) == 0) {
        return 0;
    }
    len = (int) (end - start);
    mprAddNullToBuf(packet->content);

    if (len >= conn->limits->headerSize) {
        httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE, "Header too big");
        return 0;
    }
    if (conn->server) {
        if (!httpValidateLimits(conn->server, HTTP_VALIDATE_OPEN_REQUEST, conn)) {
            return 0;
        }
        parseRequestLine(conn, packet);
    } else {
        parseResponseLine(conn, packet);
    }
    if (!conn->connError) {
        parseHeaders(conn, packet);
    }
    if (conn->server) {
        httpMatchHost(conn);
        if (httpSetUri(conn, rx->uri, "") < 0) {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad URL format");
            return 0;
        }
        if (!tx->handler) {
            httpMatchHandler(conn);  
        }
        mprLog(3, "Select handler: \"%s\" for \"%s\"", tx->handler->name, rx->uri);
        httpSetState(conn, HTTP_STATE_PARSED);        
#if UNUSED
        loc = (rx->loc) ? rx->loc : conn->server->loc;
#endif
        httpCreatePipeline(conn, rx->loc, tx->handler);

#if FUTURE
        //  MOB -- TODO
        if (0 && tx->handler->flags & HTTP_STAGE_THREAD && !conn->threaded) {
            threadRequest(conn);
            return 0;
        }
#endif
    } else if (!(100 <= rx->status && rx->status < 200)) {
        httpSetState(conn, HTTP_STATE_PARSED);        
    }
    return 1;
}


static int traceRequest(HttpConn *conn, HttpPacket *packet)
{
    MprBuf  *content;
    cchar   *endp;
    int     len;

    if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, conn->tx->extension) >= 0) {
        content = packet->content;
        endp = strstr((char*) content->start, "\r\n\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 4) : 0;
        httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, packet, len, 0);
        return 1;
    }
    return 0;
}


/*  
    Parse the first line of a http request. Return true if the first line parsed. This is only called once all the headers
    have been read and buffered. Requests look like: METHOD URL HTTP/1.X.
 */
static void parseRequestLine(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    char        *method, *uri, *protocol;
    int         methodFlags, level;

    mprLog(4, "New request from %s:%d to %s:%d", conn->ip, conn->port, conn->sock->ip, conn->sock->port);

    rx = conn->rx;
    uri = 0;
    methodFlags = 0;

#if BLD_DEBUG
    conn->startTime = mprGetTime();
    conn->startTicks = mprGetTicks();
#endif

    method = getToken(conn, " ");
    method = supper(method);

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

    case 'P':
        if (strcmp(method, "POST") == 0) {
            methodFlags = HTTP_POST;
            rx->needInputPipeline = 1;

        } else if (strcmp(method, "PUT") == 0) {
            methodFlags = HTTP_PUT;
            rx->needInputPipeline = 1;
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
            httpOmitBody(conn);
        }
        break;

    case 'T':
        if (strcmp(method, "TRACE") == 0) {
            methodFlags = HTTP_TRACE;
            httpOmitBody(conn);
        }
        break;
    }
    if (methodFlags == 0) {
        methodFlags = HTTP_UNKNOWN;
#if UNUSED
        httpProtocolError(conn, HTTP_CODE_BAD_METHOD, "Unknown method");
#endif
    }
    uri = getToken(conn, " ");
    if (*uri == '\0') {
        httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad HTTP request. Empty URI");
    } else if ((int) strlen(uri) >= conn->limits->uriSize) {
        httpError(conn, HTTP_CODE_REQUEST_URL_TOO_LARGE, "Bad request. URI too long");
    }
    protocol = getToken(conn, "\r\n");
    if (strcmp(protocol, "HTTP/1.0") == 0) {
        if (methodFlags & (HTTP_POST|HTTP_PUT)) {
            rx->remainingContent = MAXINT;
            rx->needInputPipeline = 1;
        }
        conn->http10 = 1;
        conn->protocol = "HTTP/1.0";
    } else if (strcmp(protocol, "HTTP/1.1") != 0) {
        httpProtocolError(conn, HTTP_CODE_NOT_ACCEPTABLE, "Unsupported HTTP protocol");
    }
    rx->flags |= methodFlags;
    rx->method = supper(method);
    rx->uri = uri;

    httpSetState(conn, HTTP_STATE_FIRST);
    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_FIRST, NULL)) >= 0) {
        mprLog(level, "%s %s %s", rx->method, uri, protocol);
    }
    traceRequest(conn, packet);
}


/*  
    Parse the first line of a http response. Return true if the first line parsed. This is only called once all the headers
    have been read and buffered. Response status lines look like: HTTP/1.X CODE Message
 */
static void parseResponseLine(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    MprBuf      *content;
    cchar       *endp;
    char        *protocol, *status;
    int         len, level;

    rx = conn->rx;

    protocol = getToken(conn, " ");
    if (strcmp(protocol, "HTTP/1.0") == 0) {
        conn->http10 = 1;
        conn->protocol = "HTTP/1.0";
    } else if (strcmp(protocol, "HTTP/1.1") != 0) {
        httpProtocolError(conn, HTTP_CODE_NOT_ACCEPTABLE, "Unsupported HTTP protocol");
    }

    status = getToken(conn, " ");
    if (*status == '\0') {
        httpProtocolError(conn, HTTP_CODE_NOT_ACCEPTABLE, "Bad response status code");
    }
    rx->status = atoi(status);
    rx->statusMessage = sclone(getToken(conn, "\r\n"));

    if (slen(rx->statusMessage) >= conn->limits->uriSize) {
        httpError(conn, HTTP_CODE_REQUEST_URL_TOO_LARGE, "Bad response. Status message too long");
    }
    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_FIRST, conn->tx->extension)) >= 0) {
        mprLog(level, "%s %d %s", protocol, rx->status, rx->statusMessage);
    }
    if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, conn->tx->extension) >= 0) {
        content = packet->content;
        endp = strstr((char*) content->start, "\r\n\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 4) : 0;
        httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, packet, len, 0);
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
    char        *key, *value, *tok;
    cchar       *oldValue;
    int         len, count, keepAlive;

    rx = conn->rx;
    tx = conn->tx;
    content = packet->content;
    conn->rx->headerPacket = packet;
    limits = conn->limits;
    keepAlive = 0;

    for (count = 0; content->start[0] != '\r' && !conn->error; count++) {
        if (count >= limits->headerCount) {
            httpLimitError(conn, HTTP_CODE_BAD_REQUEST, "Too many headers");
            break;
        }
        if ((key = getToken(conn, ":")) == 0 || *key == '\0') {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad header format");
            break;
        }
        value = getToken(conn, "\r\n");
        while (isspace((int) *value)) {
            value++;
        }
        key = slower(key);

        LOG(8, "Key %s, value %s", key, value);
        if (strspn(key, "%<>/\\") > 0) {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad header key value");
            break;
        }
        if ((oldValue = mprLookupHash(rx->headers, key)) != 0) {
            mprAddKey(rx->headers, key, mprAsprintf("%s, %s", oldValue, value));
        } else {
            mprAddKey(rx->headers, key, sclone(value));
        }
        switch (key[0]) {
        case 'a':
            if (strcmp(key, "authorization") == 0) {
                value = sclone(value);
                rx->authType = stok(value, " \t", &tok);
                rx->authDetails = tok;

            } else if (strcmp(key, "accept-charset") == 0) {
                rx->acceptCharset = sclone(value);

            } else if (strcmp(key, "accept") == 0) {
                rx->accept = sclone(value);

            } else if (strcmp(key, "accept-encoding") == 0) {
                rx->acceptEncoding = sclone(value);
            }
            break;

        case 'c':
            if (strcmp(key, "content-length") == 0) {
                if (rx->length >= 0) {
                    httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Mulitple content length headers");
                    break;
                }
                rx->length = atoi(value);
                if (rx->length < 0) {
                    httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad content length");
                    break;
                }
                if (rx->length >= conn->limits->receiveBodySize) {
                    httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE,
                        "Request content length %d bytes is too big. Limit %d.", rx->length, conn->limits->receiveBodySize);
                    break;
                }
                rx->contentLength = sclone(value);
                mprAssert(rx->length >= 0);
                if (conn->server || strcmp(tx->method, "HEAD") != 0) {
                    rx->remainingContent = rx->length;
                    rx->needInputPipeline = 1;
                }

            } else if (strcmp(key, "content-range") == 0) {
                /*
                    This headers specifies the range of any posted body data
                    Format is:  Content-Range: bytes n1-n2/length
                    Where n1 is first byte pos and n2 is last byte pos
                 */
                char    *sp;
                int     start, end, size;

                start = end = size = -1;
                sp = value;
                while (*sp && !isdigit((int) *sp)) {
                    sp++;
                }
                if (*sp) {
                    start = (int) stoi(sp, 10, NULL);
                    if ((sp = strchr(sp, '-')) != 0) {
                        end = (int) stoi(++sp, 10, NULL);
                    }
                    if ((sp = strchr(sp, '/')) != 0) {
                        /*
                            Note this is not the content length transmitted, but the original size of the input of which
                            the client is transmitting only a portion.
                         */
                        size = (int) stoi(++sp, 10, NULL);
                    }
                }
                if (start < 0 || end < 0 || size < 0 || end <= start) {
                    httpError(conn, HTTP_CODE_RANGE_NOT_SATISFIABLE, "Bad content range");
                    break;
                }
                rx->inputRange = httpCreateRange(conn, start, end);

            } else if (strcmp(key, "content-type") == 0) {
                rx->mimeType = sclone(value);
                rx->form = scontains(rx->mimeType, "application/x-www-form-urlencoded", -1) != 0;

            } else if (strcmp(key, "cookie") == 0) {
                if (rx->cookie && *rx->cookie) {
                    rx->cookie = sjoin(rx->cookie, "; ", value, NULL);
                } else {
                    rx->cookie = sclone(value);
                }

            } else if (strcmp(key, "connection") == 0) {
                rx->connection = sclone(value);
                if (scasecmp(value, "KEEP-ALIVE") == 0) {
                    keepAlive = 1;
                } else if (scasecmp(value, "CLOSE") == 0) {
                    /*  Not really required, but set to 0 to be sure */
                    conn->keepAliveCount = 0;
                }
            }
            break;

        case 'h':
            if (strcmp(key, "host") == 0) {
                rx->hostName = sclone(value);
            }
            break;

        case 'i':
            if ((strcmp(key, "if-modified-since") == 0) || (strcmp(key, "if-unmodified-since") == 0)) {
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

            } else if ((strcmp(key, "if-match") == 0) || (strcmp(key, "if-none-match") == 0)) {
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

            } else if (strcmp(key, "if-range") == 0) {
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
            if (strcmp(key, "keep-alive") == 0) {
                len = (int) strlen(value);
                if (len > 2 && value[len - 1] == '1' && value[len - 2] == '=' && tolower((int)(value[len - 3])) == 'x') {
                    /*  
                        On second-last request (Keep-Alive: timeout=N, max=1)
                        IMPORTANT: Deliberately close the connection one request early. This ensures a client-led 
                        termination and helps relieve server-side TIME_WAIT conditions.
                     */
                    conn->keepAliveCount = 0;
                } else {
                    keepAlive = 1;
                }
            }
            break;                
                
        case 'l':
            if (strcmp(key, "location") == 0) {
                rx->redirect = sclone(value);
            }
            break;

        case 'p':
            if (strcmp(key, "pragma") == 0) {
                rx->pragma = sclone(value);
            }
            break;

        case 'r':
            if (strcmp(key, "range") == 0) {
                if (!parseRange(conn, value)) {
                    httpError(conn, HTTP_CODE_RANGE_NOT_SATISFIABLE, "Bad range");
                }
            } else if (strcmp(key, "referer") == 0) {
                /* NOTE: yes the header is misspelt in the spec */
                rx->referrer = sclone(value);
            }
            break;

        case 't':
            if (strcmp(key, "transfer-encoding") == 0) {
                if (scasecmp(value, "chunked") == 0) {
                    rx->flags |= HTTP_CHUNKED;
                    /*  
                        This will be revised by the chunk filter as chunks are processed and will be set to zero when the
                        last chunk has been received.
                     */
                    rx->remainingContent = MAXINT;
                    rx->needInputPipeline = 1;
                }
            }
            break;

#if BLD_DEBUG
        case 'x':
            if (strcmp(key, "x-chunk-size") == 0) {
                tx->chunkSize = atoi(value);
                if (tx->chunkSize <= 0) {
                    tx->chunkSize = 0;
                } else if (tx->chunkSize > conn->limits->chunkSize) {
                    tx->chunkSize = conn->limits->chunkSize;
                }
            }
            break;
#endif

        case 'u':
            if (strcmp(key, "user-agent") == 0) {
                rx->userAgent = sclone(value);
            }
            break;

        case 'w':
            if (strcmp(key, "www-authenticate") == 0) {
                conn->authType = value;
                while (*value && !isspace((int) *value)) {
                    value++;
                }
                *value++ = '\0';
                conn->authType = slower(conn->authType);
                if (!parseAuthenticate(conn, value)) {
                    httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad Authentication header");
                    break;
                }
            }
            break;
        }
    }
    if (!keepAlive) {
        conn->keepAliveCount = 0;
    }
    if (!(rx->flags & HTTP_CHUNKED)) {
        /*  
            Step over "\r\n" after headers. As an optimization, don't do this if chunked so chunking can parse a single
            chunk delimiter of "\r\nSIZE ...\r\n"
         */
        if (mprGetBufLength(content) >= 2) {
            mprAdjustBufStart(content, 2);
        }
    }
    if (rx->remainingContent == 0) {
        rx->eof = 1;
    }
}


/*  
    Parse an authentication response (client side only)
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


#if FUTURE
static void httpThreadEvent(HttpConn *conn)
{
    httpCallEvent(conn, 0);
}


static void threadRequest(HttpConn *conn)
{
    mprAssert(!conn->dispatcher->enabled);
    mprAssert(conn->dispatcher != conn->server->dispatcher);

    conn->threaded = 1;
    conn->startingThread = 1;
    mprInitEvent(conn->dispatcher, &conn->runEvent, "runEvent", 0, (MprEventProc) httpThreadEvent, conn, 0);
    mprQueueEvent(conn->dispatcher, &conn->runEvent);
    mprAssert(!conn->dispatcher->enabled);
}
#endif


static bool processParsed(HttpConn *conn)
{
    if (!conn->abortPipeline) {
        httpStartPipeline(conn);
        if (!conn->error && !conn->writeComplete && conn->rx->remainingContent > 0) {
            /* If no remaining content, wait till the processing stage to avoid duplicate writable events */
            httpWritable(conn);
        }
    }
    httpSetState(conn, HTTP_STATE_CONTENT);
    return 1;
}


static bool analyseContent(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpQueue   *q;
    MprBuf      *content;
    ssize       nbytes, remaining;

    rx = conn->rx;
    tx = conn->tx;
    q = tx->queue[HTTP_QUEUE_RECEIVE];

    content = packet->content;
    if (rx->flags & HTTP_CHUNKED) {
        if ((remaining = getChunkPacketSize(conn, content)) == 0) {
            /* Need more data or bad chunk specification */
            if (mprGetBufLength(content) > 0) {
                conn->input = packet;
            }
            return 0;
        }
    } else {
        remaining = rx->remainingContent;
    }
    nbytes = min(remaining, mprGetBufLength(content));
    mprAssert(nbytes >= 0);

    if (nbytes > 0) {
        if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_BODY, NULL) >= 0) {
            httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_BODY, packet, nbytes, 0);
        }
    }
    LOG(7, "processContent: packet of %d bytes, remaining %d", mprGetBufLength(content), remaining);

    if (nbytes > 0) {
        mprAssert(httpGetPacketLength(packet) > 0);
        remaining -= nbytes;
        rx->remainingContent -= nbytes;
        rx->receivedContent += nbytes;

        if (rx->receivedContent >= conn->limits->receiveBodySize) {
            httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE, "Request content body of %d bytes is too big. Limit %d.",
                rx->receivedContent, conn->limits->receiveBodySize);
            return 0;
        }
        if (packet == rx->headerPacket) {
            /* Preserve headers if more data to come. Otherwise handlers may free the packet and destory the headers */
            packet = httpSplitPacket(packet, 0);
        }
        conn->input = 0;
        if (remaining == 0 && mprGetBufLength(packet->content) > nbytes) {
            /*  Split excess data belonging to the next pipelined request.  */
            LOG(7, "processContent: Split packet of %d at %d", httpGetPacketLength(packet), nbytes);
            conn->input = httpSplitPacket(packet, nbytes);
        }
        if ((q->count + httpGetPacketLength(packet)) > q->max) {
            /*  
                MOB -- should flow control instead
                httpLimitError(q->conn, HTTP_CODE_REQUEST_TOO_LARGE, "Too much body data");
            */
            return 0;
        }
        if (!conn->error) {
            httpSendPacketToNext(q, packet);
        }

    } else {
        conn->input = 0;
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
    q = conn->tx->queue[HTTP_QUEUE_RECEIVE];

    if (packet == NULL) {
        return 0;
    }
    if (conn->complete || conn->connError || rx->remainingContent <= 0) {
        //  MOB -- this needs checking - upload too much data
        httpSetState(conn, HTTP_STATE_RUNNING);
        return 1;
    }
    mprAssert(packet);
    if (!analyseContent(conn, packet)) {
        if (conn->connError) {
            /* Abort the content state if there is a connection oriented error */
            httpSetState(conn, HTTP_STATE_RUNNING);
        }
        return conn->error;
    }
    if (rx->remainingContent == 0) {
        if (!(rx->flags & HTTP_CHUNKED) || (rx->chunkState == HTTP_CHUNK_EOF)) {
            rx->eof = 1;
            httpSendPacketToNext(q, httpCreateEndPacket());
        }
        httpSetState(conn, HTTP_STATE_RUNNING);
        return 1;
    } else {
        mprYield(0);
    }
    httpServiceQueues(conn);
    return conn->connError || (conn->input ? mprGetBufLength(conn->input->content) : 0);
}


static bool processRunning(HttpConn *conn)
{
    int     canProceed;

    if (conn->abortPipeline) {
        httpSetState(conn, HTTP_STATE_COMPLETE);
        canProceed = 1;
    } else {
        if (conn->server) {
            httpProcessPipeline(conn);
        }
        if (conn->server) {
            if (conn->complete || conn->writeComplete || conn->error) {
                httpSetState(conn, HTTP_STATE_COMPLETE);
                canProceed = 1;
            } else {
                httpWritable(conn);
                canProceed = httpServiceQueues(conn);
            }
        } else {
            httpServiceQueues(conn);
            httpFinalize(conn);
            conn->complete = 1;
            httpSetState(conn, HTTP_STATE_COMPLETE);
            canProceed = 1;
        }
    }
    return canProceed;
}


#if BLD_DEBUG
static void measure(HttpConn *conn)
{
    MprTime     elapsed;
    cchar       *uri;

    if (conn->rx == 0 || conn->tx == 0) {
        return;
    }
    uri = (conn->server) ? conn->rx->uri : conn->tx->parsedUri->path;
   
    if (httpShouldTrace(conn, 0, HTTP_TRACE_TIME, NULL) >= 0) {
        elapsed = mprGetTime() - conn->startTime;
#if MPR_HIGH_RES_TIMER
        if (elapsed < 1000) {
            mprLog(4, "TIME: Request %s took %,d msec %,d ticks", uri, elapsed, mprGetTicks() - conn->startTicks);
        } else
#endif
            mprLog(4, "TIME: Request %s took %,d msec", uri, elapsed);
    }
}
#else
#define measure(conn)
#endif


static bool processCompletion(HttpConn *conn)
{
    HttpPacket  *packet;
    bool        more;

    mprAssert(conn->state == HTTP_STATE_COMPLETE);

    httpDestroyPipeline(conn);
    measure(conn);

    if (conn->server) {
        conn->rx->conn = 0;
        conn->tx->conn = 0;
        conn->rx = 0;
        conn->tx = 0;
        packet = conn->input;
        more = packet && !conn->connError && (mprGetBufLength(packet->content) > 0);
        httpValidateLimits(conn->server, HTTP_VALIDATE_CLOSE_REQUEST, conn);
        httpPrepServerConn(conn);
        return more;
    }
    return 0;
}


void httpCloseRx(HttpConn *conn)
{
    if (!conn->rx->eof) {
        conn->connError = 1;
    }
    httpFinalize(conn);
    if (conn->state < HTTP_STATE_COMPLETE && !conn->advancing) {
        httpProcess(conn, NULL);
    }
}


/*  
    Optimization to correctly size the packets to the chunk filter.
 */
static ssize getChunkPacketSize(HttpConn *conn, MprBuf *buf)
{
    HttpRx      *rx;
    char        *start, *cp;
    ssize       need, size;

    rx = conn->rx;
    need = 0;

    switch (rx->chunkState) {
    case HTTP_CHUNK_DATA:
        need = rx->remainingContent;
        if (need != 0) {
            break;
        }
        /* Fall through */

    case HTTP_CHUNK_START:
        start = mprGetBufStart(buf);
        if (mprGetBufLength(buf) < 3) {
            return 0;
        }
        if (start[0] != '\r' || start[1] != '\n') {
            httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return 0;
        }
        for (cp = &start[2]; cp < (char*) buf->end && *cp != '\n'; cp++) {}
        if ((cp - start) < 2 || (cp[-1] != '\r' || cp[0] != '\n')) {
            /* Insufficient data */
            if ((cp - start) > 80) {
                httpProtocolError(conn, HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
                return 0;
            }
            return 0;
        }
        need = (cp - start + 1);
        size = (ssize) stoi(&start[2], 16, NULL);
        if (size == 0 && &cp[2] < buf->end && cp[1] == '\r' && cp[2] == '\n') {
            /*
                This is the last chunk (size == 0). Now need to consume the trailing "\r\n".
                We are lenient if the request does not have the trailing "\r\n" as required by the spec.
             */
            need += 2;
        }
        break;

    default:
        mprAssert(0);
    }
    rx->remainingContent = need;
    return need;
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
            performed, skip the transfer. TODO - need to check if fileInfo is actually set.
         */
        modified = (MprTime) tx->fileInfo.mtime * MPR_TICKS_PER_SEC;
        same = httpMatchModified(conn, modified) && httpMatchEtag(conn, tx->etag);
        if (rx->ranges && !same) {
            rx->ranges = 0;
        }
        return same;
    }
    return 0;
}


HttpRange *httpCreateRange(HttpConn *conn, int start, int end)
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


ssize httpGetContentLength(HttpConn *conn)
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
    return mprLookupHash(conn->rx->headers, slower(key));
}


//  MOB -- why does this allocate?
char *httpGetHeaders(HttpConn *conn)
{
    HttpRx      *rx;
    MprHash     *hp;
    char        *headers, *key, *cp;
    int         len;

    if (conn->rx == 0) {
        mprAssert(conn->rx);
        return 0;
    }
    rx = conn->rx;
    headers = 0;
    for (len = 0, hp = mprGetFirstHash(rx->headers); hp; ) {
        headers = srejoin(headers, hp->key, NULL);
        key = &headers[len];
        for (cp = &key[1]; *cp; cp++) {
            *cp = tolower((int) *cp);
            if (*cp == '-') {
                cp++;
            }
        }
        headers = srejoin(headers, ": ", hp->data, "\n", NULL);
        len = (int) strlen(headers);
        hp = mprGetNextHash(rx->headers, hp);
    }
    return headers;
}


MprHashTable *httpGetHeaderHash(HttpConn *conn)
{
    if (conn->rx == 0) {
        mprAssert(conn->rx);
        return 0;
    }
    return conn->rx->headers;
}


cchar *httpGetQueryString(HttpConn *conn)
{
    return conn->rx->parsedUri->query;
}


int httpGetStatus(HttpConn *conn)
{
    return conn->rx->status;
}


char *httpGetStatusMessage(HttpConn *conn)
{
    return conn->rx->statusMessage;
}


int httpMapToStorage(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpHost    *host;
    MprPath     *info;

    rx = conn->rx;
    tx = conn->tx;
    host = conn->host;
    info = &tx->fileInfo;

    rx->loc = httpLookupBestLocation(host, rx->pathInfo);
    rx->auth = rx->loc->auth;
    rx->alias = httpGetAlias(host, rx->pathInfo);
    tx->filename = httpMakeFilename(conn, rx->alias, rx->pathInfo, 1);
    tx->extension = httpGetExtension(conn);
    mprGetPathInfo(tx->filename, info);
    if (info->valid) {
        tx->etag = mprAsprintf("\"%x-%Lx-%Lx\"", info->inode, info->size, info->mtime);
    }
    return 0;
}


int httpSetUri(HttpConn *conn, cchar *uri, cchar *query)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpHost    *host;
    char        *oldQuery;

    rx = conn->rx;
    tx = conn->tx;
    host = conn->host;
    oldQuery = rx->parsedUri ? rx->parsedUri->query : 0;

    if ((rx->parsedUri = httpCreateUri(uri, 0)) == 0) {
        return MPR_ERR_BAD_ARGS;
    }
    if (query == 0) {
        rx->parsedUri->query = oldQuery;
    } else if (*query) {
        rx->parsedUri->query = sclone(query);
    }
    /*
        Start out with no scriptName and the entire URI in the pathInfo. Stages may rewrite.
     */
    rx->uri = rx->parsedUri->uri;
    rx->pathInfo = httpNormalizeUriPath(mprUriDecode(rx->parsedUri->path));
    rx->scriptName = mprEmptyString();
    httpMapToStorage(conn);
    return 0;
}


static void waitHandler(HttpConn *conn, struct MprEvent *event)
{
    httpCallEvent(conn, event->mask);
#if UNUSED
    httpEnableConnEvents(conn);
#endif
    mprSignalDispatcher(conn->dispatcher);
}


/*  
    Wait for the Http object to achieve a given state. Timeout is total wait time in msec. If <= 0, then dont wait.
 */
int httpWait(HttpConn *conn, int state, MprTime timeout)
{
    Http        *http;
    MprTime     expire, remainingTime;
    int         eventMask, addedHandler, saveAsync, justOne;

    http = conn->http;
    if (timeout <= 0) {
        timeout = MAXINT64;
    }
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
    saveAsync = conn->async;
    if (conn->waitHandler == 0) {
        conn->async = 1;
        eventMask = MPR_READABLE;
        if (!conn->writeComplete) {
            eventMask |= MPR_WRITABLE;
        }
        conn->waitHandler = mprCreateWaitHandler(conn->sock->fd, eventMask, conn->dispatcher, (MprEventProc) waitHandler, 
            conn, 0);
        addedHandler = 1;
    } else {
        addedHandler = 0;
    }
    http->now = mprGetTime();
    expire = http->now + timeout;

    while (!conn->error && conn->state < state && conn->sock && !mprIsSocketEof(conn->sock)) {
        remainingTime = (expire - http->now);
        if (remainingTime <= 0) {
            break;
        }
        mprAssert(!mprSocketHasPendingData(conn->sock));
        mprWaitForEvent(conn->dispatcher, remainingTime);
        if (justOne) {
            break;
        }
    }
    if (addedHandler && conn->waitHandler) {
        mprRemoveWaitHandler(conn->waitHandler);
        conn->waitHandler = 0;
        conn->async = saveAsync;
    }
    if (conn->sock == 0 || conn->error) {
        return MPR_ERR_CANT_CONNECT;
    }
    if (!justOne && conn->state < state) {
        return MPR_ERR_TIMEOUT;
    }
    return 0;
}


/*  
    Set the connector as write blocked and can't proceed.
 */
void httpSetWriteBlocked(HttpConn *conn)
{
    mprLog(7, "Write Blocked");
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
    HttpRx      *rx;
    HttpRange   *range, *last, *next;
    char        *tok, *ep;

    rx = conn->rx;

    value = sclone(value);
    if (value == 0) {
        return 0;
    }

    /*  
        Step over the "bytes="
     */
    stok(value, "=", &value);

    for (last = 0; value && *value; ) {
        if ((range = mprAllocObj(HttpRange, NULL)) == 0) {
            return 0;
        }
        /*  
            A range "-7" will set the start to -1 and end to 8
         */
        tok = stok(value, ",", &value);
        if (*tok != '-') {
            range->start = (ssize) stoi(tok, 10, NULL);
        } else {
            range->start = -1;
        }
        range->end = -1;

        if ((ep = strchr(tok, '-')) != 0) {
            if (*++ep != '\0') {
                /*
                    End is one beyond the range. Makes the math easier.
                 */
                range->end = (ssize) stoi(ep, 10, NULL) + 1;
            }
        }
        if (range->start >= 0 && range->end >= 0) {
            range->len = (int) (range->end - range->start);
        }
        if (last == 0) {
            rx->ranges = range;
        } else {
            last->next = range;
        }
        last = range;
    }

    /*  
        Validate ranges
     */
    for (range = rx->ranges; range; range = range->next) {
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
            if (next->start >= 0 && range->end > next->start) {
                return 0;
            }
        }
    }
    conn->tx->currentRange = rx->ranges;
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
    return mprLookupHash(rx->requestData, key);
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
/************************************************************************/
/*
 *  End of file "../src/rx.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/sendConnector.c"
 */
/************************************************************************/

/*
    sendConnector.c -- Send file connector. 

    The Sendfile connector supports the optimized transmission of whole static files. It uses operating system 
    sendfile APIs to eliminate reading the document into user space and multiple socket writes. The send connector 
    is not a general purpose connector. It cannot handle dynamic data or ranged requests. It does support chunked requests.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void addPacketForSend(HttpQueue *q, HttpPacket *packet);
static void adjustSendVec(HttpQueue *q, ssize written);
static int  buildSendVec(HttpQueue *q);
static void freeSentPackets(HttpQueue *q, ssize written);
static void sendIncomingService(HttpQueue *q);


int httpOpenSendConnector(Http *http)
{
    HttpStage     *stage;

    if ((stage = httpCreateConnector(http, "sendConnector", HTTP_STAGE_ALL, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->open = httpSendOpen;
    stage->outgoingService = httpSendOutgoingService; 
    stage->incomingService = sendIncomingService; 
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

    /*  
        To write an entire file, reset the maximum and packet size to the maximum response body size (LimitResponseBody)
     */
    //  MOB -- are these enforced somewhere as the packet will be empty
    //  MOB -- should these not be set elsewhere?
    q->max = conn->limits->transmissionBodySize;
    q->packetSize = conn->limits->transmissionBodySize;

    if (!(tx->flags & HTTP_TX_NO_BODY)) {
        tx->file = mprOpenFile(tx->filename, O_RDONLY | O_BINARY, 0);
        if (tx->file == 0) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", tx->filename);
        }
    }
}


static void sendIncomingService(HttpQueue *q)
{
    httpEnableConnEvents(q->conn);
}


void httpSendOutgoingService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       written;
    int         errCode, count;

    conn = q->conn;
    tx = conn->tx;
    conn->lastActivity = conn->http->now;

    if (conn->sock == 0 || conn->writeComplete) {
        return;
    }
    if (tx->flags & HTTP_TX_NO_BODY) {
        httpDiscardData(q, 1);
    }
    if ((tx->bytesWritten + q->ioCount) > conn->limits->transmissionBodySize) {
        httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE,
            "Http transmission aborted. Exceeded max body of %d bytes", conn->limits->transmissionBodySize);
       if (tx->flags & HTTP_TX_HEADERS_CREATED) {
            httpCompleteWriting(conn);
            /* Must disconnect as the client must be notified somehow */
            mprDisconnectSocket(conn->sock);
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
        /*
            Write the vector and file data. Exclude the file entry in the io vector.
         */
        count = q->ioIndex - q->ioFileEntry;
        mprAssert(count >= 0);
        written = mprSendFileToSocket(conn->sock, tx->file, (MprOffset) tx->pos, q->ioCount, q->iovec, count, NULL, 0);
        mprLog(5, "Send connector written %d", written);
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
            httpConnError(conn, HTTP_CODE_COMMS_ERROR, "SendFileToSocket failed, errCode %d", errCode);
            httpCompleteWriting(conn);
            break;

        } else if (written == 0) {
            /* Socket is full. Wait for an I/O event */
            httpSetWriteBlocked(conn);
            break;

        } else if (written > 0) {
            tx->bytesWritten += written;
            freeSentPackets(q, written);
            adjustSendVec(q, written);
        }
    }
    if (q->ioCount == 0) {
        if ((q->flags & HTTP_QUEUE_EOF)) {
            httpCompleteWriting(conn);
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
static int buildSendVec(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;

    mprAssert(q->ioIndex == 0);

    conn = q->conn;
    q->ioCount = 0;
    q->ioFileEntry = 0;

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

        } else if (httpGetPacketLength(packet) == 0) {
            q->flags |= HTTP_QUEUE_EOF;
            if (packet->prefix == NULL) {
                break;
            }
        }
        if (q->ioFileEntry || q->ioIndex >= (HTTP_MAX_IOVEC - 2)) {
            break;
        }
        addPacketForSend(q, packet);
    }
    return (int) q->ioCount;
}


/*  
    Add one entry to the io vector
 */
static void addToSendVector(HttpQueue *q, char *ptr, ssize bytes)
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
    if (httpGetPacketLength(packet) > 0) {
        /*
            Header packets have actual content. File data packets are virtual and only have a count.
         */
        if (packet->content) {
            addToSendVector(q, mprGetBufStart(packet->content), mprGetBufLength(packet->content));

        } else {
            addToSendVector(q, 0, httpGetPacketLength(packet));
            mprAssert(q->ioFileEntry == 0);
            q->ioFileEntry = 1;
            q->ioFileOffset += (MprOffset) httpGetPacketLength(packet);
        }
    }
    item = (packet->flags & HTTP_PACKET_HEADER) ? HTTP_TRACE_HEADER : HTTP_TRACE_BODY;
    if (httpShouldTrace(conn, HTTP_TRACE_TX, item, NULL) >= 0) {
        httpTraceContent(conn, HTTP_TRACE_TX, item, packet, 0, tx->bytesWritten);
    }
}


/*  
    Clear entries from the IO vector that have actually been transmitted. This supports partial writes due to the socket
    being full. Don't come here if we've seen all the packets and all the data has been completely written. ie. small files
    don't come here.
 */
static void freeSentPackets(HttpQueue *q, ssize bytes)
{
    HttpPacket  *packet;
    ssize       len;

    mprAssert(q->first);
    mprAssert(q->count >= 0);
    mprAssert(bytes >= 0);

    while ((packet = q->first) != 0) {
        if (packet->prefix) {
            len = mprGetBufLength(packet->prefix);
            len = min(len, bytes);
            mprAdjustBufStart(packet->prefix, len);
            bytes -= len;
            /* Prefixes dont' count in the q->count. No need to adjust */
            if (mprGetBufLength(packet->prefix) == 0) {
                packet->prefix = 0;
            }
        }
        if ((len = httpGetPacketLength(packet)) > 0) {
            len = min(len, bytes);
            if (packet->content) {
                mprAdjustBufStart(packet->content, len);
            } else {
                packet->entityLength -= len;
            }
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
static void adjustSendVec(HttpQueue *q, ssize written)
{
    HttpTx      *tx;
    MprIOVec    *iovec;
    ssize       len;
    int         i, j;

    tx = q->conn->tx;

    /*  
        Cleanup the IO vector
     */
    if (written == q->ioCount) {
        /*  
            Entire vector written. Just reset.
         */
        q->ioIndex = 0;
        q->ioCount = 0;
        tx->pos = q->ioFileOffset;

    } else {
        /*  
            Partial write of an vector entry. Need to copy down the unwritten vector entries.
         */
        q->ioCount -= written;
        mprAssert(q->ioCount >= 0);
        iovec = q->iovec;
        for (i = 0; i < q->ioIndex; i++) {
            len = iovec[i].len;
            if (iovec[i].start) {
                if (written < len) {
                    iovec[i].start += written;
                    iovec[i].len -= written;
                    break;
                } else {
                    written -= len;
                }
            } else {
                /*
                    File data has a null start ptr
                 */
                tx->pos += written;
                q->ioIndex = 0;
                q->ioCount = 0;
                return;
            }
        }

        /*  Compact */
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
/************************************************************************/
/*
 *  End of file "../src/sendConnector.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/server.c"
 */
/************************************************************************/

/*
    server.c -- Create Http servers.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static int manageServer(HttpServer *server, int flags);
static int destroyServerConnections(HttpServer *server);

/*
    Create a server listening on ip:port. NOTE: ip may be empty which means bind to all addresses.
 */
HttpServer *httpCreateServer(cchar *ip, int port, MprDispatcher *dispatcher, int flags)
{
    HttpServer  *server;
    Http        *http;

    if ((server = mprAllocObj(HttpServer, manageServer)) == 0) {
        return 0;
    }
    http = MPR->httpService;
    server->http = http;
    server->clientLoad = mprCreateHash(HTTP_CLIENTS_HASH, MPR_HASH_STATIC_VALUES);
    server->async = 1;
    server->http = MPR->httpService;
    server->port = port;
    server->ip = sclone(ip);
    server->dispatcher = dispatcher;
    if (server->ip && *server->ip) {
        server->name = server->ip;
    }
    server->loc = httpInitLocation(http, 1);
    server->hosts = mprCreateList(-1, 0);
    httpAddServer(http, server);
    if (flags & HTTP_CREATE_HOST) {
        httpAddHostToServer(server, httpCreateHost(ip, port, server->loc));
    }
    return server;
}


void httpDestroyServer(HttpServer *server)
{
    mprLog(4, "Destroy server %s", server->name);
    if (server->waitHandler) {
        mprRemoveWaitHandler(server->waitHandler);
        server->waitHandler = 0;
    }
    destroyServerConnections(server);
    if (server->sock) {
        mprCloseSocket(server->sock, 0);
        server->sock = 0;
    }
    httpRemoveServer(MPR->httpService, server);
}


static int manageServer(HttpServer *server, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(server->http);
        mprMark(server->loc);
        mprMark(server->limits);
        mprMark(server->waitHandler);
        mprMark(server->clientLoad);
        mprMark(server->hosts);
        mprMark(server->name);
        mprMark(server->ip);
        mprMark(server->context);
        mprMark(server->meta);
        mprMark(server->sock);
        mprMark(server->dispatcher);
        mprMark(server->ssl);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyServer(server);
    }
    return 0;
}


/*  
    Convenience function to create and configure a new server without using a config file.
 */
HttpServer *httpCreateConfiguredServer(cchar *docRoot, cchar *ip, int port)
{
    Http            *http;
    HttpHost        *host;
    HttpServer      *server;

    http = MPR->httpService;

    if (ip == 0) {
        /*  
            If no IP:PORT specified, find the first server
         */
        if ((server = mprGetFirstItem(http->servers)) != 0) {
            ip = server->ip;
            port = server->port;
        } else {
            ip = "localhost";
            if (port <= 0) {
                port = HTTP_DEFAULT_PORT;
            }
            server = httpCreateServer(ip, port, NULL, HTTP_CREATE_HOST);
        }
    } else {
        server = httpCreateServer(ip, port, NULL, HTTP_CREATE_HOST);
    }
    if ((host->mimeTypes = mprCreateMimeTypes("mime.types")) == 0) {
        host->mimeTypes = MPR->mimeTypes;
    }
    httpAddDir(host, httpCreateBareDir(docRoot));
    httpSetHostDocumentRoot(host, docRoot);
    return server;
}


static int destroyServerConnections(HttpServer *server)
{
    HttpConn    *conn;
    Http        *http;
    int         next;

    http = server->http;
    lock(http);

    for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
        if (conn->server == server) {
            conn->server = 0;
            httpDestroyConn(conn);
            next--;
        }
    }
    unlock(http);
    return 0;
}


static bool validateServer(HttpServer *server)
{
    HttpHost    *host;

    if ((host = mprGetFirstItem(server->hosts)) == 0) {
        mprError("Missing host object on server");
        return 0;
    }
    if (mprGetListLength(host->aliases) == 0) {
        httpAddAlias(host, httpCreateAlias("", host->documentRoot, 0));
    }
#if UNUSED
    if (!host->documentRoot) {
        host->documentRoot = server->documentRoot;
    }
    if (!host->serverRoot) {
        host->serverRoot = server->serverRoot;
    }
#endif
    return 1;
}


int httpStartServer(HttpServer *server)
{
    cchar   *proto, *ip;

    if (!validateServer(server)) {
        return MPR_ERR_BAD_ARGS;
    }
    if ((server->sock = mprCreateSocket(server->ssl)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (mprListenOnSocket(server->sock, server->ip, server->port, MPR_SOCKET_NODELAY | MPR_SOCKET_THREAD) < 0) {
        mprError("Can't open a socket on %s, port %d", server->ip, server->port);
        return MPR_ERR_CANT_OPEN;
    }
    if (server->http->listenCallback && (server->http->listenCallback)(server) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    if (server->async && server->waitHandler ==  0) {
        server->waitHandler = mprCreateWaitHandler(server->sock->fd, MPR_SOCKET_READABLE, server->dispatcher,
            (MprEventProc) httpAcceptConn, server, (server->dispatcher) ? 0 : MPR_WAIT_NEW_DISPATCHER);
    } else {
        mprSetSocketBlockingMode(server->sock, 1);
    }
    proto = mprIsSocketSecure(server->sock) ? "HTTPS" : "HTTP ";
    ip = *server->ip ? server->ip : "*";
    mprLog(MPR_CONFIG, "Started %s server on \"%s:%d\"", proto, ip, server->port);
    return 0;
}


void httpStopServer(HttpServer *server)
{
    mprCloseSocket(server->sock, 0);
    server->sock = 0;
}


int httpValidateLimits(HttpServer *server, int event, HttpConn *conn)
{
    HttpLimits      *limits;
    int             count;

    limits = server->limits;
    mprAssert(conn->server == server);
    lock(server->http);

    switch (event) {
    case HTTP_VALIDATE_OPEN_CONN:
        if (server->clientCount >= limits->clientCount) {
            unlock(server->http);
            httpConnError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, 
                "Too many concurrent clients %d/%d", server->clientCount, limits->clientCount);
            return 0;
        }
        count = (int) PTOL(mprLookupHash(server->clientLoad, conn->ip));
        mprAddKey(server->clientLoad, conn->ip, ITOP(count + 1));
        server->clientCount = mprGetHashLength(server->clientLoad);
        break;

    case HTTP_VALIDATE_CLOSE_CONN:
        count = (int) PTOL(mprLookupHash(server->clientLoad, conn->ip));
        if (count > 1) {
            mprAddKey(server->clientLoad, conn->ip, ITOP(count - 1));
        } else {
            mprRemoveHash(server->clientLoad, conn->ip);
        }
        server->clientCount = mprGetHashLength(server->clientLoad);
        mprLog(4, "Close connection %d. Active requests %d, active clients %d", 
            conn->seqno, server->requestCount, server->clientCount);
        break;
    
    case HTTP_VALIDATE_OPEN_REQUEST:
        if (server->requestCount >= limits->requestCount) {
            //  MOB -- will CLOSE_REQUEST get called and thus set the limit to negative?
            unlock(server->http);
            httpConnError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, 
                "Too many concurrent requests %d/%d", server->requestCount, limits->requestCount);
            return 0;
        }
        server->requestCount++;
        break;

    case HTTP_VALIDATE_CLOSE_REQUEST:
        server->requestCount--;
        mprAssert(server->requestCount >= 0);
        mprLog(4, "Close request. Active requests %d, active clients %d", server->requestCount, server->clientCount);
        break;
    }
    mprLog(6, "Validate request. Counts: requests: %d/%d, clients %d/%d", 
        server->requestCount, limits->requestCount, server->clientCount, limits->clientCount);
    unlock(server->http);
    return 1;
}


/*  
    Accept a new client connection on a new socket. If multithreaded, this will come in on a worker thread 
    dedicated to this connection. This is called from the listen wait handler.
 */
HttpConn *httpAcceptConn(HttpServer *server, MprEvent *event)
{
    HttpConn        *conn;
    MprSocket       *sock;
    MprDispatcher   *dispatcher;
    MprEvent        e;
    int             level;

    mprAssert(server);

    /*
        This will block in sync mode until a connection arrives
     */
    sock = mprAcceptSocket(server->sock);
    if (server->waitHandler) {
        mprEnableWaitEvents(server->waitHandler, MPR_READABLE);
    }
    if (sock == 0) {
        return 0;
    }
    dispatcher = (event && server->dispatcher == 0) ? event->dispatcher: server->dispatcher;
    mprLog(4, "New connection from %s:%d to %s:%d %s",
        sock->ip, sock->port, sock->acceptIp, sock->acceptPort, server->sock->sslSocket ? "(secure)" : "");

    if ((conn = httpCreateConn(server->http, server, dispatcher)) == 0) {
        mprError("Can't create connect object. Insufficient memory.");
        mprCloseSocket(sock, 0);
        return 0;
    }
    conn->notifier = server->notifier;
    conn->async = server->async;
    conn->server = server;
    conn->sock = sock;
    conn->port = sock->port;
    conn->ip = sclone(sock->ip);
    conn->secure = mprIsSocketSecure(sock);

    if (!httpValidateLimits(server, HTTP_VALIDATE_OPEN_CONN, conn)) {
        /* Prevent validate limits from */
        conn->server = 0;
        httpDestroyConn(conn);
        return 0;
    }
    mprAssert(conn->state == HTTP_STATE_BEGIN);
    httpSetState(conn, HTTP_STATE_CONNECTED);

    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_CONN, NULL)) >= 0) {
        mprLog(level, "### Incoming connection from %s:%d to %s:%d", 
            conn->ip, conn->port, conn->sock->ip, conn->sock->port);
    }
    e.mask = MPR_READABLE;
    e.timestamp = mprGetTime();
    (conn->ioCallback)(conn, &e);
    return conn;
}


//  MOB Is this used / needed
void *httpGetMetaServer(HttpServer *server)
{
    return server->meta;
}


void *httpGetServerContext(HttpServer *server)
{
    return server->context;
}


int httpGetServerAsync(HttpServer *server) 
{
    return server->async;
}


void httpSetServerAddress(HttpServer *server, cchar *ip, int port)
{
    HttpHost    *host;
    int         next;

    if (ip) {
        server->ip = sclone(ip);
    }
    if (port >= 0) {
        server->port = port;
    }
    for (next = 0; (host = mprGetNextItem(server->hosts, &next)) != 0; ) {
        httpSetHostAddress(host, ip, port);
    }
    if (server->sock) {
        httpStopServer(server);
        httpStartServer(server);
    }
}

void httpSetMetaServer(HttpServer *server, void *meta)
{
    server->meta = meta;
}


void httpSetServerAsync(HttpServer *server, int async)
{
    if (server->sock) {
        if (server->async && !async) {
            mprSetSocketBlockingMode(server->sock, 1);
        }
        if (!server->async && async) {
            mprSetSocketBlockingMode(server->sock, 0);
        }
    }
    server->async = async;
}


void httpSetServerContext(HttpServer *server, void *context)
{
    mprAssert(server);
    server->context = context;
}


void httpSetServerLocation(HttpServer *server, HttpLoc *loc)
{
    mprAssert(server);
    mprAssert(loc);
    server->loc = loc;
}


void httpSetServerName(HttpServer *server, cchar *name)
{
    mprAssert(server);
    server->name = sclone(name);
}


void httpSetServerNotifier(HttpServer *server, HttpNotifier notifier)
{
    mprAssert(server);
    server->notifier = notifier;
}


int httpSecureServer(cchar *ip, int port, struct MprSsl *ssl)
{
#if BLD_FEATURE_SSL
    HttpServer  *server;
    Http        *http;
    int         next, count;

    http = MPR->httpService;
    if (ip == 0) {
        ip = "";
    }
    for (count = next = 0; (server = mprGetNextItem(http->servers, &next)) != 0; ) {
        if (server->port <= 0 || port <= 0 || server->port == port) {
            mprAssert(server->ip);
            if (*server->ip == '\0' || *ip == '\0' || scmp(server->ip, ip) == 0) {
                server->ssl = ssl;
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
#else
    return MPR_ERR_BAD_STATE;
#endif
}


void httpAddHostToServer(HttpServer *server, HttpHost *host)
{
    mprAddItem(server->hosts, host);
    if (server->limits == 0) {
        server->limits = host->limits;
    }
}


#if UNUSED
HttpServer *httpRemoveHostFromServer(Http *http, cchar *ip, int port, HttpHost *host)
{
    HttpServer  *server;
    int         next;

    for (next = 0; (server = mprGetNextItem(http->servers, &next)) != 0; ) {
        if (server->port < 0 || port < 0 || server->port == port) {
            if (ip == 0 || server->ip == 0 || strcmp(server->ip, ip) == 0) {
                mprRemoveItem(server->hosts, host);
            }
        }
    }
    return 0;
}
#endif


bool httpIsNamedVirtualServer(HttpServer *server)
{
    return server->flags & HTTP_IPADDR_VHOST;
}


void httpSetNamedVirtualServer(HttpServer *server)
{
    server->flags |= HTTP_IPADDR_VHOST;
}


HttpHost *httpLookupHostByName(HttpServer *server, cchar *name)
{
    HttpHost    *host;
    int         next;

    for (next = 0; (host = mprGetNextItem(server->hosts, &next)) != 0; ) {
        if (name == 0 || strcmp(name, host->name) == 0) {
            return host;
        }
    }
    return 0;
}


#if UNUSED
void httpSetServerLimits(HttpServer *server, HttpLimits *limits)
{
    server->limits = limits;
}


/*  
    Create the host addresses for a host. Called for hosts or for NameVirtualHost directives (host == 0).
 */
int httpCreateEndpoints(HttpHost *host, cchar *configValue)
{
    Http            *http;
    HttpServer      *server;
    HttpServer      *endpoint;
    char            *ipAddrPort, *ip, *value, *tok;
    int             next, port;

    http = MPR->httpService;
    endpoint = 0;
    value = sclone(configValue);
    ipAddrPort = stok(value, " \t", &tok);

    while (ipAddrPort) {
        if (scasecmp(ipAddrPort, "_default_") == 0) {
            //  TODO is this used?
            mprAssert(0);
            ipAddrPort = "*:*";
        }
        if (mprParseIp(ipAddrPort, &ip, &port, -1) < 0) {
            mprError("Can't parse ipAddr %s", ipAddrPort);
            continue;
        }
        mprAssert(ip && *ip);
        if (ip[0] == '*') {
            ip = sclone("");
        }

        /*  
            For each listening endpiont
         */
        for (next = 0; (server = mprGetNextItem(http->servers, &next)) != 0; ) {
            if (port > 0 && port != server->port) {
                continue;
            }
            if (server->ip[0] != '\0' && ip[0] != '\0' && strcmp(ip, server->ip) != 0) {
                continue;
            }

            /*
                Find the matching endpoint or create a new one
             */
            if ((endpoint = httpLookupEndpoint(http, server->ip, server->port)) == 0) {
                endpoint = httpCreateEndpoint(server->ip, server->port);
                mprAddItem(http->servers, endpoint);
            }
            if (host == 0) {
                httpSetNamedVirtualEndpoint(endpoint);

            } else {
                httpAddHostToEndpoint(endpoint, host);
                if (server->ip[0] != '\0') {
                    httpSetHostName(host, mprAsprintf("%s:%d", server->ip, server->port));
                } else {
                    httpSetHostName(host, mprAsprintf("%s:%d", ip, server->port));
                }
            }
        }
        ipAddrPort = stok(0, " \t", &tok);
    }
    if (host) {
        if (endpoint == 0) {
            mprError("No valid IP address for host %s", host->name);
            return MPR_ERR_CANT_INITIALIZE;
        }
        if (httpIsNamedVirtualEndpoint(endpoint)) {
            httpSetNamedVirtualHost(host);
        }
    }
    return 0;
}
#endif


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
/************************************************************************/
/*
 *  End of file "../src/server.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/stage.c"
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
    enableService = !(q->stage->flags & HTTP_STAGE_HANDLER) || (q->conn->state == HTTP_STATE_RUNNING) ? 1 : 0;
    httpPutForService(q, packet, enableService);
}


/*  
    Default incoming data routine.  Simply transfer the data upstream to the next filter or handler.
 */
static void incomingData(HttpQueue *q, HttpPacket *packet)
{
    mprAssert(q);
    mprAssert(packet);
    
    if (q->nextQ->put) {
        httpSendPacketToNext(q, packet);
    } else {
        /* This queue is the last queue in the pipeline */
        if (httpGetPacketLength(packet) > 0) {
            httpJoinPacketForService(q, packet, 0);
            HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
        } else {
            /* Zero length packet means eof */
            httpPutForService(q, packet, 0);
            HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
        }
    }
}


void httpDefaultOutgoingServiceStage(HttpQueue *q)
{
    HttpPacket    *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!httpWillNextQueueAcceptPacket(q, packet)) {
            httpPutBackPacket(q, packet);
            return;
        }
        httpSendPacketToNext(q, packet);
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
        mprMark(stage->module);
        mprMark(stage->stageData);
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
/************************************************************************/
/*
 *  End of file "../src/stage.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/trace.c"
 */
/************************************************************************/

/*
    trace.c -- Trace data
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




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
        trace[dir].size = -1;
    }
}


/*
    Return the level at which tracing should occur
 */
int httpShouldTrace(HttpConn *conn, int dir, int item, cchar *ext)
{
    HttpTrace   *trace;
    int         mprLevel;

    mprAssert(0 <= dir && dir < HTTP_TRACE_MAX_DIR);
    mprAssert(0 <= item && item < HTTP_TRACE_MAX_ITEM);

    trace = &conn->trace[dir];
    if (trace->disable) {
        return -1;
    }
    if (ext) {
        if (trace->include && !mprLookupHash(trace->include, ext)) {
            trace->disable = 1;
            return -1;
        }
        if (trace->exclude && mprLookupHash(trace->exclude, ext)) {
            trace->disable = 1;
            return -1;
        }
    }
    mprLevel = mprGetLogLevel(conn);
    if (trace->levels[item] <= mprLevel) {
        return trace->levels[item];
    }
    return -1;
}


static void traceBuf(HttpConn *conn, int dir, int level, cchar *msg, cchar *buf, ssize len)
{
    cchar       *cp, *tag, *digits;
    char        *data, *dp;
    static int  txSeq = 0;
    static int  rxSeq = 0;
    int         seqno, i, printable;

    for (printable = 1, i = 0; i < len; i++) {
        if (!isascii(buf[i])) {
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
        memcpy(data, buf, len);
        data[len] = '\0';
        mprRawLog(level, "\n>>>>>>>>>> %s %s packet %d, len %d (conn %d) >>>>>>>>>>\n%s", tag, msg, seqno, 
            len, conn->seqno, data);
    } else {
        mprRawLog(level, "\n>>>>>>>>>> %s %s packet %d, len %d (conn %d) >>>>>>>>>> (binary)\n", tag, msg, seqno, 
            len, conn->seqno);
        data = mprAlloc(len * 3 + ((len / 16) + 1) + 1);
        digits = "0123456789ABCDEF";
        for (i = 0, cp = buf, dp = data; cp < &buf[len]; cp++) {
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
    mprRawLog(level, "<<<<<<<<<< %s packet, conn %d\n\n", tag, conn->seqno);
}


void httpTraceContent(HttpConn *conn, int dir, int item, HttpPacket *packet, ssize len, ssize total)
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
        len = INT_MAX;
    }
    if (packet->prefix) {
        size = mprGetBufLength(packet->prefix);
        size = min(size, len);
        traceBuf(conn, dir, level, "prefix", mprGetBufStart(packet->prefix), size);
    }
    if (packet->content) {
        size = mprGetBufLength(packet->content);
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
/************************************************************************/
/*
 *  End of file "../src/trace.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/tx.c"
 */
/************************************************************************/

/*
    tx.c - Http transmitter for server responses and client requests.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void manageTx(HttpTx *tx, int flags);


HttpTx *httpCreateTx(HttpConn *conn, MprHashTable *headers)
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

    tx->queue[HTTP_QUEUE_TRANS] = httpCreateQueueHead(conn, "TxHead");
    tx->queue[HTTP_QUEUE_RECEIVE] = httpCreateQueueHead(conn, "RxHead");

    if (headers) {
        tx->headers = headers;
    } else if ((tx->headers = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS)) != 0) {
        if (conn->server) {
            httpAddSimpleHeader(conn, "Server", conn->http->software);
        } else {
            httpAddSimpleHeader(conn, "User-Agent", sclone(HTTP_NAME));
        }
    }
    return tx;
}


void httpDestroyTx(HttpTx *tx)
{
    mprCloseFile(tx->file);
    if (tx->conn) {
        tx->conn->tx = 0;
        tx->conn = 0;
    }
}


static void manageTx(HttpTx *tx, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(tx->conn);
        mprMark(tx->outputPipeline);
        mprMark(tx->handler);
        mprMark(tx->connector);
        mprMark(tx->queue[0]);
        mprMark(tx->queue[1]);
        mprMark(tx->parsedUri);
        mprMark(tx->currentRange);
        mprMark(tx->headers);
        mprMark(tx->rangeBoundary);
        mprMark(tx->etag);
        mprMark(tx->method);
        mprMark(tx->altBody);
        mprMark(tx->file);
        mprMark(tx->filename);
        mprMark(tx->extension);

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

#if UNUSED
    //  MOB - remove this test
    if (scasecmp(key, "content-length") == 0) {
        conn->tx->length = (ssize) stoi(value, 10, NULL);
    }
#endif
    mprAddKey(conn->tx->headers, key, value);
}


int httpRemoveHeader(HttpConn *conn, cchar *key)
{
    mprAssert(key && *key);
    if (conn->tx == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return mprRemoveHash(conn->tx->headers, key);
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
    value = mprAsprintfv(fmt, vargs);
    va_end(vargs);

    if (!mprLookupHash(conn->tx->headers, key)) {
        addHeader(conn, key, value);
    }
}


/*
    Add a simple (non-formatted) header if not already defined
 */
void httpAddSimpleHeader(HttpConn *conn, cchar *key, cchar *value)
{

    mprAssert(key && *key);
    mprAssert(value);

    if (!mprLookupHash(conn->tx->headers, key)) {
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
    value = mprAsprintfv(fmt, vargs);
    va_end(vargs);

    oldValue = mprLookupHash(conn->tx->headers, key);
    if (oldValue) {
        addHeader(conn, key, mprAsprintf("%s, %s", oldValue, value));
    } else {
        addHeader(conn, key, value);
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
    value = mprAsprintfv(fmt, vargs);
    va_end(vargs);
    addHeader(conn, key, value);
}


void httpSetSimpleHeader(HttpConn *conn, cchar *key, cchar *value)
{
    mprAssert(key && *key);
    mprAssert(value);

    addHeader(conn, key, sclone(value));
}


void httpDontCache(HttpConn *conn)
{
    conn->tx->flags |= HTTP_TX_DONT_CACHE;
}


void httpFinalize(HttpConn *conn)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (tx->finalized || conn->state < HTTP_STATE_CONNECTED || conn->writeq == 0 || conn->sock == 0) {
        return;
    }
    tx->finalized = 1;
    httpPutForService(conn->writeq, httpCreateEndPacket(), 1);
    httpServiceQueues(conn);
    if (conn->state == HTTP_STATE_RUNNING && conn->writeComplete && !conn->advancing) {
        httpProcess(conn, NULL);
    }
}


int httpIsFinalized(HttpConn *conn)
{
    return conn->tx && conn->tx->finalized;
}


/*
    Flush the write queue
 */
void httpFlush(HttpConn *conn)
{
    httpFlushQueue(conn->writeq, !conn->async);
}


/*
    Format alternative body content. The message is HTML escaped.
 */
int httpFormatBody(HttpConn *conn, cchar *title, cchar *fmt, ...)
{
    HttpTx      *tx;
    va_list     args;
    char        *body;

    tx = conn->tx;
    mprAssert(tx->altBody == 0);

    va_start(args, fmt);
    body = mprAsprintfv(fmt, args);
    tx->altBody = mprAsprintf(
        "<!DOCTYPE html>\r\n"
        "<html><head><title>%s</title></head>\r\n"
        "<body>\r\n%s\r\n</body>\r\n</html>\r\n",
        title, body);
    httpOmitBody(conn);
    va_end(args);
    return (int) strlen(tx->altBody);
}


/*
    Create an alternate body response. Typically used for error responses. The message is HTML escaped.
 */
void httpSetResponseBody(HttpConn *conn, int status, cchar *msg)
{
    HttpTx      *tx;
    cchar       *statusMsg;
    char        *emsg;

    mprAssert(msg && msg);
    tx = conn->tx;

    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        mprError("Can't set response body if headers have already been created");
        /* Connectors will detect this also and disconnect */
    } else {
        httpDiscardTransmitData(conn);
    }
    tx->status = status;
    if (tx->altBody == 0) {
        statusMsg = httpLookupStatus(conn->http, status);
        emsg = mprEscapeHtml(msg);
        httpFormatBody(conn, statusMsg, "<h2>Access Error: %d -- %s</h2>\r\n<p>%s</p>\r\n", status, statusMsg, emsg);
    }
}


void *httpGetQueueData(HttpConn *conn)
{
    HttpQueue     *q;

    q = conn->tx->queue[HTTP_QUEUE_TRANS];
    return q->nextQ->queueData;
}


void httpOmitBody(HttpConn *conn)
{
    if (conn->tx) {
        conn->tx->flags |= HTTP_TX_NO_BODY;
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
    uri = 0;
    tx->status = status;
    prev = rx->parsedUri;
    target = httpCreateUri(targetUri, 0);

    if (conn->http->redirectCallback) {
        targetUri = (conn->http->redirectCallback)(conn, &status, target);
    }
    if (strstr(targetUri, "://") == 0) {
        port = strchr(targetUri, ':') ? prev->port : conn->server->port;
        if (target->path[0] == '/') {
            /*
                Absolute URL. If hostName has a port specifier, it overrides prev->port.
             */
            uri = httpFormatUri(prev->scheme, rx->hostName, port, target->path, target->reference, target->query, 1);
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
            uri = httpFormatUri(prev->scheme, rx->hostName, port, path, target->reference, target->query, 1);
        }
        targetUri = uri;
    }
    httpSetHeader(conn, "Location", "%s", targetUri);
    mprAssert(tx->altBody == 0);
    msg = httpLookupStatus(conn->http, status);
    tx->altBody = mprAsprintf(
        "<!DOCTYPE html>\r\n"
        "<html><head><title>%s</title></head>\r\n"
        "<body><h1>%s</h1>\r\n<p>The document has moved <a href=\"%s\">here</a>.</p>\r\n"
        "<address>%s at %s Port %d</address></body>\r\n</html>\r\n",
        msg, msg, targetUri, HTTP_NAME, conn->server->name, prev->port);
    httpOmitBody(conn);
}


void httpSetContentLength(HttpConn *conn, ssize length)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        return;
    }
    tx->length = length;
    httpSetHeader(conn, "Content-Length", "%d", tx->length);
}


void httpSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, int lifetime, bool isSecure)
{
    HttpRx      *rx;
    struct tm   tm;
    char        *cp, *expiresAtt, *expires, *domainAtt, *domain, *secure;
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
    if (webkitVersion >= 312) {
        domain = sclone(rx->hostName);
        if ((cp = strchr(domain, ':')) != 0) {
            *cp = '\0';
        }
        if (*domain && domain[strlen(domain) - 1] == '.') {
            domain[strlen(domain) - 1] = '\0';
        } else {
            domain = 0;
        }
    } else {
        domain = 0;
    }
    if (domain) {
        domainAtt = "; domain=";
    } else {
        domainAtt = "";
    }
    if (lifetime > 0) {
        mprDecodeUniversalTime(&tm, conn->http->now + (lifetime * MPR_TICKS_PER_SEC));
        expiresAtt = "; expires=";
        expires = mprFormatTime(MPR_HTTP_DATE, &tm);

    } else {
        expires = expiresAtt = "";
    }
    if (isSecure) {
        secure = "; secure";
    } else {
        secure = ";";
    }
    /* 
       Allow multiple cookie headers. Even if the same name. Later definitions take precedence
     */
    httpAppendHeader(conn, "Set-Cookie", 
        sjoin(name, "=", value, "; path=", path, domainAtt, domain, expiresAtt, expires, secure, NULL));
    httpAppendHeader(conn, "Cache-control", "no-cache=\"set-cookie\"");
}


/*  
    Set headers for httpWriteHeaders. This defines standard headers.
 */
static void setHeaders(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRange   *range;
    MprTime     expires;
    MprPath     *info;
    cchar       *mimeType;
    char        *hdr;
    struct tm   tm;

    mprAssert(packet->flags == HTTP_PACKET_HEADER);

    rx = conn->rx;
    tx = conn->tx;
    info = &tx->fileInfo;

    httpAddSimpleHeader(conn, "Date", conn->http->currentDate);

    if (tx->flags & HTTP_TX_DONT_CACHE) {
        httpAddSimpleHeader(conn, "Cache-Control", "no-cache");

    } else if (rx->loc && rx->loc->expires) {
        mimeType = mprLookupHash(tx->headers, "Content-Type");
        expires = PTOL(mprLookupHash(rx->loc->expires, mimeType ? mimeType : ""));
        if (expires == 0) {
            expires = PTOL(mprLookupHash(rx->loc->expires, ""));
        }
        if (expires) {
            mprDecodeUniversalTime(&tm, mprGetTime() + (expires * MPR_TICKS_PER_SEC));
            hdr = mprFormatTime(MPR_HTTP_DATE, &tm);
            httpAddHeader(conn, "Cache-Control", "max-age=%d", expires);
            httpAddHeader(conn, "Expires", "%s", hdr);
        }
    }
#if UNUSED
    if (tx->etag == 0 && info->valid) {
        tx->etag = mprAsprintf("\"%x-%Lx-%Lx\"", info->inode, info->size, info->mtime);
    }
#endif
    if (tx->etag) {
        httpAddHeader(conn, "ETag", "%s", tx->etag);
    }
    if (tx->altBody) {
        tx->length = (int) strlen(tx->altBody);
    }
    if (tx->chunkSize > 0 && !tx->altBody) {
        if (!(rx->flags & HTTP_HEAD)) {
            httpSetSimpleHeader(conn, "Transfer-Encoding", "chunked");
        }
    } else if (tx->length > 0) {
        httpSetHeader(conn, "Content-Length", "%d", tx->length);
    }
    if (rx->ranges) {
        if (rx->ranges->next == 0) {
            range = rx->ranges;
            if (tx->entityLength > 0) {
                httpSetHeader(conn, "Content-Range", "bytes %d-%d/%d", range->start, range->end, tx->entityLength);
            } else {
                httpSetHeader(conn, "Content-Range", "bytes %d-%d/*", range->start, range->end);
            }
        } else {
            httpSetHeader(conn, "Content-Type", "multipart/byteranges; boundary=%s", tx->rangeBoundary);
        }
        httpAddHeader(conn, "Accept-Ranges", "bytes");
    }
    if (tx->extension) {
        if ((mimeType = (char*) mprLookupMime(conn->host->mimeTypes, tx->extension)) != 0) {
            httpAddSimpleHeader(conn, "Content-Type", mimeType);
        }
    }
    if (conn->server) {
        if (--conn->keepAliveCount > 0) {
            httpSetSimpleHeader(conn, "Connection", "keep-alive");
            httpSetHeader(conn, "Keep-Alive", "timeout=%d, max=%d", conn->limits->inactivityTimeout / 1000, 
                conn->keepAliveCount);
        } else {
            httpSetSimpleHeader(conn, "Connection", "close");
        }
    }
}


void httpSetEntityLength(HttpConn *conn, ssize len)
{
    HttpTx      *tx;

    tx = conn->tx;
    tx->entityLength = len;
    if (conn->rx->ranges == 0) {
        tx->length = len;
    }
}


/*
    Set the tx status
 */
void httpSetStatus(HttpConn *conn, int status)
{
    conn->tx->status = status;
}


void httpSetMimeType(HttpConn *conn, cchar *mimeType)
{
    httpSetSimpleHeader(conn, "Content-Type", sclone(mimeType));
}


void httpWriteHeaders(HttpConn *conn, HttpPacket *packet)
{
    Http        *http;
    HttpTx      *tx;
    HttpUri     *parsedUri;
    MprHash     *hp;
    MprBuf      *buf;
    int         level;

    mprAssert(packet->flags == HTTP_PACKET_HEADER);

    http = conn->http;
    tx = conn->tx;
    buf = packet->content;

    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        return;
    }    
    if (conn->headersCallback) {
        /* Must be before headers below */
        (conn->headersCallback)(conn->headersCallbackArg);
    }
    setHeaders(conn, packet);

    if (conn->server) {
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
    if ((level = httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_FIRST, NULL)) >= mprGetLogLevel(tx)) {
        mprAddNullToBuf(buf);
        mprLog(level, "%s", mprGetBufStart(buf));
    }
    mprPutStringToBuf(buf, "\r\n");

    /* 
       Output headers
     */
    hp = mprGetFirstHash(conn->tx->headers);
    while (hp) {
        mprPutStringToBuf(packet->content, hp->key);
        mprPutStringToBuf(packet->content, ": ");
        if (hp->data) {
            mprPutStringToBuf(packet->content, hp->data);
        }
        mprPutStringToBuf(packet->content, "\r\n");
        hp = mprGetNextHash(conn->tx->headers, hp);
    }

    /* 
       By omitting the "\r\n" delimiter after the headers, chunks can emit "\r\nSize\r\n" as a single chunk delimiter
     */
    if (tx->chunkSize <= 0 || tx->altBody) {
        mprPutStringToBuf(buf, "\r\n");
    }
    if (tx->altBody) {
        mprPutStringToBuf(buf, tx->altBody);
        httpDiscardData(tx->queue[HTTP_QUEUE_TRANS]->nextQ, 0);
    }
    tx->headerSize = mprGetBufLength(buf);
    tx->flags |= HTTP_TX_HEADERS_CREATED;
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
/************************************************************************/
/*
 *  End of file "../src/tx.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/uploadFilter.c"
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
static bool matchUpload(HttpConn *conn, HttpStage *filter);
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
static bool matchUpload(HttpConn *conn, HttpStage *filter)
{
    HttpRx  *rx;
    char    *pat;
    ssize   len;
    
    rx = conn->rx;
    if (!(rx->flags & HTTP_POST) || rx->remainingContent <= 0) {
        return 0;
    }
    pat = "multipart/form-data";
    len = strlen(pat);
    if (sncasecmp(rx->mimeType, pat, len) == 0) {
        rx->flags |= HTTP_UPLOAD;
        return 1;
    } else {
        return 0;
    }   
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

    if ((up = mprAllocObj(Upload, manageUpload)) == 0) {
        return;
    }
    q->queueData = up;
    up->contentState = HTTP_UPLOAD_BOUNDARY;

    if (rx->uploadDir == 0) {
#if BLD_WIN_LIKE
        rx->uploadDir = mprGetNormalizedPath(getenv("TEMP"));
#else
        rx->uploadDir = sclone("/tmp");
#endif
    }
    if ((boundary = strstr(rx->mimeType, "boundary=")) != 0) {
        boundary += 9;
        up->boundary = sjoin("--", boundary, NULL);
        up->boundaryLen = strlen(up->boundary);
    }
    if (up->boundaryLen == 0 || *up->boundary == '\0') {
        httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad boundary");
        return;
    }
    httpSetFormVar(conn, "UPLOAD_DIR", rx->uploadDir);
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
        mprDeletePath(file->filename);
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
            return;
        }
        httpSendPacketToNext(q, packet);
        return;
    }
    mprLog(5, "uploadIncomingData: %d bytes", httpGetPacketLength(packet));
    
    /*  
        Put the packet data onto the service queue for buffering. This aggregates input data incase we don't have
        a complete mime record yet.
     */
    httpJoinPacketForService(q, packet, 0);
    
    packet = q->first;
    content = packet->content;
    mprAddNullToBuf(content);
    count = mprGetBufLength(content);

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
            if (mprGetBufLength(content) < up->boundaryLen) {
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
    q->count -= (count - mprGetBufLength(content));
    mprAssert(q->count >= 0);

    if (mprGetBufLength(content) == 0) {
        /* 
           Quicker to free the buffer so the packets don't have to be joined the next time 
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
    mprLog(5, "Header line: %s", line);

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
    httpSetFormVar(conn, key, file->clientFilename);

    key = sjoin("FILE_CONTENT_TYPE_", up->id, NULL);
    httpSetFormVar(conn, key, file->contentType);

    key = sjoin("FILE_FILENAME_", up->id, NULL);
    httpSetFormVar(conn, key, file->filename);

    key = sjoin("FILE_SIZE_", up->id, NULL);
    httpSetIntFormVar(conn, key, (int) file->size);
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
        httpLimitError(conn, HTTP_CODE_REQUEST_TOO_LARGE, "Uploaded file %s exceeds maximum %d", 
            up->tmpPath, limits->uploadSize);
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
        mprLog(6, "uploadFilter: Wrote %d bytes to %s", len, up->tmpPath);
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
        mprLog(6, "uploadFilter: Got boundary filename %x", up->clientFilename);
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
    if (bp) {
        dataLen = (int) (bp - data);
    } else {
        dataLen = mprGetBufLength(content);
    }
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
            httpSetFormVar(conn, key, data);
            if (packet == 0) {
                packet = httpCreatePacket(HTTP_BUFSIZE);
            }
            if (mprGetBufLength(packet->content) > 0) {
                /*
                    Need to add www-form-urlencoding separators
                 */
                mprPutCharToBuf(packet->content, '&');
            }
            mprPutFmtToBuf(packet->content, "%s=%s", up->id, data);
        }
    }
    if (up->clientFilename) {
        /*  
            Now have all the data (we've seen the boundary)
         */
        up->file = 0;
        up->clientFilename = 0;
    }
    if (packet) {
        httpSendPacketToNext(q, packet);
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
/************************************************************************/
/*
 *  End of file "../src/uploadFilter.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/uri.c"
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
#if BLD_WIN_LIKE
                for (cp = up->ext; *cp; cp++) {
                    *cp = (char) tolower((int) *cp);
                }
#endif
            }
        } else {
            up->ext = sclone(++tok);
#if BLD_WIN_LIKE
            for (cp = up->ext; *cp; cp++) {
                *cp = (char) tolower((int) *cp);
            }
#endif
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
            port = (int) stoi(++cp, 10, NULL);
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
        up->path = "/";
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
#if BLD_WIN_LIKE
                for (cp = up->ext; *cp; cp++) {
                    *cp = (char) tolower((int) *cp);
                }
#endif
            }
        } else {
            up->ext = sclone(&tok[1]);
#if BLD_WIN_LIKE
            for (cp = up->ext; *cp; cp++) {
                *cp = (char) tolower((int) *cp);
            }
#endif
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
            port = (int) stoi(++cp, 10, NULL);
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
#if BLD_WIN_LIKE
                for (cp = up->ext; *cp; cp++) {
                    *cp = (char) tolower((int) *cp);
                }
#endif
            }
        } else {
            up->ext = sclone(&tok[1]);
#if BLD_WIN_LIKE
            for (cp = up->ext; *cp; cp++) {
                *cp = (char) tolower((int) *cp);
            }
#endif
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
    char    portBuf[16], *uri;
    cchar   *hostDelim, *portDelim, *pathDelim, *queryDelim, *referenceDelim;

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

    /*  Hosts with integral port specifiers override
     */
    if (host && schr(host, ':')) {
        portDelim = 0;
    } else {
        if (port != 0 && port != getDefaultPort(scheme)) {
            itos(portBuf, sizeof(portBuf), port, 10);
            portDelim = ":";
        } else {
            portBuf[0] = '\0';
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
        uri = sjoin(scheme, hostDelim, host, portDelim, portBuf, pathDelim, path, referenceDelim, reference, 
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
HttpUri *httpGetRelativeUri(HttpUri *base, HttpUri *target, int dup)
{
    HttpUri     *uri;
    char        *basePath, *bp, *cp, *tp, *startDiff;
    int         i, baseSegments, commonSegments;

    if (target == 0) {
        return (dup) ? httpCloneUri(base, 0) : base;
    }
    if (!(target->path && target->path[0] == '/') || !((base->path && base->path[0] == '/'))) {
        /* If target is relative, just use it. If base is relative, can't use it because we don't know where it is */
        return (dup) ? httpCloneUri(target, 0) : target;
    }
    if (base->scheme && target->scheme && scmp(base->scheme, target->scheme) != 0) {
        return (dup) ? httpCloneUri(target, 0) : target;
    }
    if (base->host && target->host && (base->host && scmp(base->host, target->host) != 0)) {
        return (dup) ? httpCloneUri(target, 0) : target;
    }
    if (getPort(base) != getPort(target)) {
        return (dup) ? httpCloneUri(target, 0) : target;
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
            MOB -- do we want to do this?
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
    uri->ext = mprGetPathExtension(uri->path);
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


HttpUri *httpResolveUri(HttpUri *base, int argc, HttpUri **others, int local)
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
    current->ext = mprGetPathExtension(current->path);
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
/************************************************************************/
/*
 *  End of file "../src/uri.c"
 */
/************************************************************************/

