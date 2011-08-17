/*
    meta.c -- Manage a meta-server with one or more virtual hosts.

    A meta-server supports multiple endpoints and one or more (virtual) hosts.
    Meta Servers may be configured manually or via an "appweb.conf" configuration  file.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

#if BLD_UNIX_LIKE
static int allDigits(cchar *s);
#endif
static void manageAppweb(MaAppweb *appweb, int flags);
static void openHandlers(Http *http);

/************************************ Code ************************************/
/*
    Create the top level appweb control object. This is typically a singleton.
 */
MaAppweb *maCreateAppweb()
{
    MaAppweb    *appweb;
    Http        *http;

    if ((appweb = mprAllocObj(MaAppweb, manageAppweb)) == NULL) {
        return 0;
    }
    appweb->http = http = httpCreate(appweb);
    httpSetContext(http, appweb);
    appweb->metas = mprCreateList(-1, 0);
    maGetUserGroup(appweb);
    maParseInit(appweb);
    openHandlers(http);
    return appweb; 
}


static void manageAppweb(MaAppweb *appweb, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(appweb->defaultMeta);
        mprMark(appweb->metas);
        mprMark(appweb->http);
        mprMark(appweb->user);
        mprMark(appweb->group);
        mprMark(appweb->directives);

    } else if (flags & MPR_MANAGE_FREE) {
        maStopAppweb(appweb);
    }
}


static void openHandlers(Http *http)
{
#if BLD_FEATURE_DIR
    maOpenDirHandler(http);
#endif
    maOpenFileHandler(http);
}


void maAddMeta(MaAppweb *appweb, MaMeta *meta)
{
    mprAddItem(appweb->metas, meta);
}


void maSetDefaultMeta(MaAppweb *appweb, MaMeta *meta)
{
    appweb->defaultMeta = meta;
}


MaMeta *maLookupMeta(MaAppweb *appweb, cchar *name)
{
    MaMeta  *meta;
    int     next;

    for (next = 0; (meta = mprGetNextItem(appweb->metas, &next)) != 0; ) {
        if (strcmp(meta->name, name) == 0) {
            return meta;
        }
    }
    return 0;
}


int maStartAppweb(MaAppweb *appweb)
{
    MaMeta  *meta;
    char    *timeText;
    int     next;

    for (next = 0; (meta = mprGetNextItem(appweb->metas, &next)) != 0; ) {
        if (maStartMeta(meta) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    timeText = mprGetDate(0);
    mprLog(1, "HTTP services Started at %s with max %d threads", timeText, mprGetMaxWorkers(appweb));
    return 0;
}


int maStopAppweb(MaAppweb *appweb)
{
    MaMeta  *meta;
    int     next;

    for (next = 0; (meta = mprGetNextItem(appweb->metas, &next)) != 0; ) {
        maStopMeta(meta);
    }
    return 0;
}


static void manageMeta(MaMeta *meta, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(meta->appweb);
        mprMark(meta->http);
        mprMark(meta->name);
        mprMark(meta->serverRoot);
        mprMark(meta->servers);

    } else if (flags & MPR_MANAGE_FREE) {
        maStopMeta(meta);
    }
}


/*  
    Create a new meta-server. This may manage may virtual hosts. If ip and port are supplied,
    create a HttpServer now on that endpoint. Otherwise, call maConfigureMeta later.
 */
MaMeta *maCreateMeta(MaAppweb *appweb, cchar *name, cchar *root, cchar *ip, int port)
{
    MaMeta      *meta;
    HttpServer  *server;

    mprAssert(appweb);
    mprAssert(name && *name);

    if ((meta = mprAllocObj(MaMeta, manageMeta)) == NULL) {
        return 0;
    }
    meta->name = sclone(name);
    meta->servers = mprCreateList(-1, 0);
    meta->limits = httpCreateLimits(1);
    meta->appweb = appweb;
    meta->http = appweb->http;
    meta->alreadyLogging = mprGetLogHandler() ? 1 : 0;

    maAddMeta(appweb, meta);
    maSetMetaRoot(meta, root);

    if (ip && port > 0) {
        /* Passing NULL for the dispatcher will cause a new dispatcher to be created for each accepted connection */
        server = httpCreateServer(ip, port, NULL, HTTP_CREATE_HOST);
        maAddServer(meta, server);
    }
    maSetDefaultMeta(appweb, meta);
    return meta;
}


void maCreateMetaDefaultHost(MaMeta *meta)
{
    HttpHost    *host;

    host = httpCreateHost(0);
    meta->defaultHost = host;
    httpSetHostName(host, "default", -1);
    httpSetHostServerRoot(host, meta->serverRoot);
}


int maStartMeta(MaMeta *meta)
{
    HttpServer  *server;
    HttpHost    *host;
    int         next, nextHost, count, warned;

    warned = 0;
    count = 0;
    for (next = 0; (server = mprGetNextItem(meta->servers, &next)) != 0; ) {
        if (httpStartServer(server) < 0) {
            warned++;
            break;
        } else {
            count++;
        }
        for (nextHost = 0; (host = mprGetNextItem(server->hosts, &nextHost)) != 0; ) {
            mprLog(3, "Serving %s at \"%s\"", host->name, host->documentRoot);
        }
    }
    if (count == 0) {
        if (!warned) {
            mprError("Server is not listening on any addresses");
        }
        return MPR_ERR_CANT_OPEN;
    }
    if (maApplyChangedGroup(meta->appweb) < 0 || maApplyChangedUser(meta->appweb) < 0) {
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}


int maStopMeta(MaMeta *meta)
{
    HttpServer  *server;
    int         next;

    for (next = 0; (server = mprGetNextItem(meta->servers, &next)) != 0; ) {
        httpStopServer(server);
    }
    return 0;
}


void maAddServer(MaMeta *meta, HttpServer *server)
{
    mprAddItem(meta->servers, server);
}


void maRemoveServer(MaMeta *meta, HttpServer *server)
{
    mprRemoveItem(meta->servers, server);
}


/*  
    Set the Server Root directory. We convert path into an absolute path.
 */
void maSetMetaRoot(MaMeta *meta, cchar *path)
{
    if (path == 0 || BLD_FEATURE_ROMFS) {
        path = ".";
    }
#if !VXWORKS
    /*
        VxWorks stat() is broken if using a network FTP server.
     */
    if (! mprPathExists(path, R_OK)) {
        mprError("Can't access ServerRoot directory %s", path);
        return;
    }
#endif
    meta->serverRoot = mprGetAbsPath(path);
    mprLog(MPR_CONFIG, "Set meta server root to: \"%s\"", meta->serverRoot);

#if UNUSED
    if (chdir((char*) path) < 0) {
        mprError("Can't set server root to %s\n", path);
    } else {
        meta->serverRoot = mprGetAbsPath(path);
        mprLog(MPR_CONFIG, "Set meta server root to: \"%s\"", meta->serverRoot);
    }
#endif
}


/*
    Set the document root for the default server (only)
 */
void maSetMetaAddress(MaMeta *meta, cchar *ip, int port)
{
    HttpServer  *server;
    int         next;

    for (next = 0; ((server = mprGetNextItem(meta->servers, &next)) != 0); ) {
        httpSetServerAddress(server, ip, port);
    }
}


void maSetMetaDefaultHost(MaMeta *meta, HttpHost *host)
{
    meta->defaultHost = host;
}


void maGetUserGroup(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;
    struct group    *gp;

    appweb->uid = getuid();
    if ((pp = getpwuid(appweb->uid)) == 0) {
        mprError("Can't read user credentials: %d. Check your /etc/passwd file.", appweb->uid);
    } else {
        appweb->user = sclone(pp->pw_name);
    }
    appweb->gid = getgid();
    if ((gp = getgrgid(appweb->gid)) == 0) {
        mprError("Can't read group credentials: %d. Check your /etc/group file", appweb->gid);
    } else {
        appweb->group = sclone(gp->gr_name);
    }
#else
    appweb->uid = appweb->gid = -1;
#endif
}


int maSetHttpUser(MaAppweb *appweb, cchar *newUser)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;

    if (allDigits(newUser)) {
        appweb->uid = atoi(newUser);
        if ((pp = getpwuid(appweb->uid)) == 0) {
            mprError("Bad user id: %d", appweb->uid);
            return MPR_ERR_CANT_ACCESS;
        }
        newUser = pp->pw_name;

    } else {
        if ((pp = getpwnam(newUser)) == 0) {
            mprError("Bad user name: %s", newUser);
            return MPR_ERR_CANT_ACCESS;
        }
        appweb->uid = pp->pw_uid;
    }
    appweb->userChanged = 1;
#endif
    appweb->user = sclone(newUser);
    return 0;
}


int maSetHttpGroup(MaAppweb *appweb, cchar *newGroup)
{
#if BLD_UNIX_LIKE
    struct group    *gp;

    if (allDigits(newGroup)) {
        appweb->gid = atoi(newGroup);
        if ((gp = getgrgid(appweb->gid)) == 0) {
            mprError("Bad group id: %d", appweb->gid);
            return MPR_ERR_CANT_ACCESS;
        }
        newGroup = gp->gr_name;

    } else {
        if ((gp = getgrnam(newGroup)) == 0) {
            mprError("Bad group name: %s", newGroup);
            return MPR_ERR_CANT_ACCESS;
        }
        appweb->gid = gp->gr_gid;
    }
    appweb->groupChanged = 1;
#endif
    appweb->group = sclone(newGroup);
    return 0;
}


int maApplyChangedUser(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    if (appweb->userChanged && appweb->uid >= 0) {
        if ((setuid(appweb->uid)) != 0) {
            mprError("Can't change user to: %s: %d\n"
                "WARNING: This is a major security exposure", appweb->user, appweb->uid);
#if LINUX && PR_SET_DUMPABLE
        } else {
            prctl(PR_SET_DUMPABLE, 1);
#endif
        }
        mprLog(MPR_CONFIG, "Changing user ID to %s: %d", appweb->user, appweb->uid);
    }
#endif
    return 0;
}


int maApplyChangedGroup(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    if (appweb->groupChanged && appweb->gid >= 0) {
        if (setgid(appweb->gid) != 0) {
            mprError("Can't change group to %s: %d\n"
                "WARNING: This is a major security exposure", appweb->group, appweb->gid);
#if LINUX && PR_SET_DUMPABLE
        } else {
            prctl(PR_SET_DUMPABLE, 1);
#endif
        }
        mprLog(MPR_CONFIG, "Changing group ID to %s: %d", appweb->group, appweb->gid);
    }
#endif
    return 0;
}


/*
    Load a module. Returns 0 if the modules is successfully loaded either statically or dynamically.
 */
int maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname)
{
    MprModule   *module;
    char        entryPoint[MPR_MAX_FNAME];
    char        *path;

    if (strcmp(name, "authFilter") == 0 || strcmp(name, "rangeFilter") == 0 || strcmp(name, "uploadFilter") == 0 ||
            strcmp(name, "fileHandler") == 0 || strcmp(name, "dirHandler") == 0) {
        mprLog(1, "The %s module is now builtin. No need to use LoadModule", name);
        return 0;
    }
    if (libname == 0) {
        path = sjoin("mod_", name, BLD_SHOBJ, NULL);
    } else {
        path = sclone(libname);
    }
    if ((module = mprLookupModule(path)) != 0) {
        mprLog(MPR_CONFIG, "Activating module (Builtin) %s", name);
        return 0;
    }
    mprSprintf(entryPoint, sizeof(entryPoint), "ma%sInit", name);
    entryPoint[2] = toupper((int) entryPoint[2]);
    if ((module = mprCreateModule(name, path, entryPoint, MPR->httpService)) == 0) {
        return 0;
    }
    if (mprLoadModule(module) < 0) {
        return MPR_ERR_CANT_CREATE;
    }
    mprLog(MPR_CONFIG, "Activating module (Loadable) %s", name);
    return 0;
}


#if BLD_UNIX_LIKE
static int allDigits(cchar *s)
{
    return strspn(s, "1234567890") == strlen(s);
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
