/*
    server.c -- Manage a web server with one or more virtual hosts.

    A server supports multiple endpoints and one or more (virtual) hosts.
    Server Servers may be configured manually or via an "appweb.conf" configuration  file.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

static char *getSearchPath(cchar *dir);
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
    appweb->servers = mprCreateList(-1, 0);
    maGetUserGroup(appweb);
    maParseInit(appweb);
    openHandlers(http);
    return appweb; 
}


static void manageAppweb(MaAppweb *appweb, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(appweb->defaultServer);
        mprMark(appweb->servers);
        mprMark(appweb->directives);
        mprMark(appweb->http);
        mprMark(appweb->user);
        mprMark(appweb->group);

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


void maAddServer(MaAppweb *appweb, MaServer *server)
{
    mprAddItem(appweb->servers, server);
}


void maSetDefaultServer(MaAppweb *appweb, MaServer *server)
{
    appweb->defaultServer = server;
}

//  MOB - sort this file

MaServer *maLookupServer(MaAppweb *appweb, cchar *name)
{
    MaServer    *server;
    int         next;

    for (next = 0; (server = mprGetNextItem(appweb->servers, &next)) != 0; ) {
        if (strcmp(server->name, name) == 0) {
            return server;
        }
    }
    return 0;
}


int maStartAppweb(MaAppweb *appweb)
{
    MaServer    *server;
    char        *timeText;
    int         next;

    for (next = 0; (server = mprGetNextItem(appweb->servers, &next)) != 0; ) {
        if (maStartServer(server) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    timeText = mprGetDate(0);
    mprLog(1, "HTTP services Started at %s with max %d threads", timeText, mprGetMaxWorkers(appweb));
    return 0;
}


int maStopAppweb(MaAppweb *appweb)
{
    MaServer  *server;
    int     next;

    for (next = 0; (server = mprGetNextItem(appweb->servers, &next)) != 0; ) {
        maStopServer(server);
    }
    return 0;
}


static void manageServer(MaServer *server, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(server->name);
        mprMark(server->appweb);
        mprMark(server->http);
        mprMark(server->limits);
        mprMark(server->endpoints);
        mprMark(server->home);

    } else if (flags & MPR_MANAGE_FREE) {
        maStopServer(server);
    }
}


/*  
    Create a new server. A server may manage may multiple servers and virtual hosts. 
    If ip/port endpoint is supplied, this call will create a Server on that endpoint. Otherwise, 
    maConfigureServer should be called later. A default route is created with the document root set to "."
 */
MaServer *maCreateServer(MaAppweb *appweb, cchar *name)
{
    MaServer    *server;

    mprAssert(appweb);

    if ((server = mprAllocObj(MaServer, manageServer)) == NULL) {
        return 0;
    }
    if (name == 0 || *name == '\0') {
        name = "default";
    }
    server->name = sclone(name);
    server->endpoints = mprCreateList(-1, 0);
    server->limits = httpCreateLimits(1);
    server->appweb = appweb;
    server->http = appweb->http;
    maAddServer(appweb, server);
    if (appweb->defaultServer == 0) {
        maSetDefaultServer(appweb, server);
    }
    return server;
}


/*
    Configure the server. If the configFile is defined, use it. If not, then consider home, documents, ip and port.
 */
int maConfigureServer(MaServer *server, cchar *configFile, cchar *home, cchar *documents, cchar *ip, int port)
{
    MaAppweb        *appweb;
    Http            *http;
    HttpEndpoint    *endpoint;
    HttpHost        *host;
    HttpRoute       *route;
    char            *path, *searchPath, *dir;

    appweb = server->appweb;
    http = appweb->http;
    dir = mprGetAppDir();

    if (configFile) {
        path = mprGetAbsPath(configFile);
        if (maParseConfig(server, path, 0) < 0) {
            /* mprUserError("Can't configure server using %s", path); */
            return MPR_ERR_CANT_INITIALIZE;
        }
        return 0;

    } else {
        mprLog(2, "DocumentRoot %s", documents);
        if ((endpoint = httpCreateConfiguredEndpoint(home, documents, ip, port)) == 0) {
            return MPR_ERR_CANT_OPEN;
        }
        maAddEndpoint(server, endpoint);
        host = mprGetFirstItem(endpoint->hosts);
        mprAssert(host);
        route = mprGetFirstItem(host->routes);
        mprAssert(route);

        searchPath = getSearchPath(dir);
        mprSetModuleSearchPath(searchPath);

#if BLD_FEATURE_CGI
        maLoadModule(appweb, "cgiHandler", "mod_cgi");
        if (httpLookupStage(http, "cgiHandler")) {
            httpAddRouteHandler(route, "cgiHandler", "cgi cgi-nph bat cmd pl py");
            /*
                Add cgi-bin with a route for the /cgi-bin URL prefix.
             */
            path = "cgi-bin";
            if (mprPathExists(path, X_OK)) {
                HttpRoute *cgiRoute;
                cgiRoute = httpCreateAliasRoute(route, "/cgi-bin/", path, 0);
                mprLog(4, "ScriptAlias \"/cgi-bin/\":\"%s\"", path);
                httpSetRouteHandler(cgiRoute, "cgiHandler");
                httpFinalizeRoute(cgiRoute);
            }
        }
#endif
#if BLD_FEATURE_ESP
        maLoadModule(appweb, "espHandler", "mod_esp");
        if (httpLookupStage(http, "espHandler")) {
            httpAddRouteHandler(route, "espHandler", "esp");
        }
#endif
#if BLD_FEATURE_EJSCRIPT
        maLoadModule(appweb, "ejsHandler", "mod_ejs");
        if (httpLookupStage(http, "ejsHandler")) {
            httpAddRouteHandler(route, "ejsHandler", "ejs");
        }
#endif
#if BLD_FEATURE_PHP
        maLoadModule(appweb, "phpHandler", "mod_php");
        if (httpLookupStage(http, "phpHandler")) {
            httpAddRouteHandler(route, "phpHandler", "php");
        }
#endif
        httpAddRouteHandler(route, "fileHandler", "");
        httpFinalizeRoute(route);
    }
    if (home) {
        maSetServerHome(server, home);
    }
    if (ip || port > 0) {
        maSetServerAddress(server, ip, port);
    }
    return 0;
}


int maStartServer(MaServer *server)
{
    HttpEndpoint    *endpoint;
    int             next, count, warned;

    warned = 0;
    count = 0;
    for (next = 0; (endpoint = mprGetNextItem(server->endpoints, &next)) != 0; ) {
        if (httpStartEndpoint(endpoint) < 0) {
            warned++;
            break;
        } else {
            count++;
        }
    }
    if (count == 0) {
        if (!warned) {
            mprError("Server is not listening on any addresses");
        }
        return MPR_ERR_CANT_OPEN;
    }
#if BLD_UNIX_LIKE
    MaAppweb    *appweb = server->appweb;
    if (appweb->userChanged || appweb->groupChanged) {
        if (!smatch(MPR->logPath, "stdout") && !smatch(MPR->logPath, "stderr")) {
            if (chown(MPR->logPath, appweb->uid, appweb->gid) < 0) {
                mprError("Can't change ownership on %s", MPR->logPath);
            }
        }
    }
    if (maApplyChangedGroup(appweb) < 0 || maApplyChangedUser(appweb) < 0) {
        return MPR_ERR_CANT_COMPLETE;
    }
#endif
    return 0;
}


void maStopServer(MaServer *server)
{
    HttpEndpoint    *endpoint;
    int             next;

    for (next = 0; (endpoint = mprGetNextItem(server->endpoints, &next)) != 0; ) {
        httpStopEndpoint(endpoint);
    }
}


void maAddEndpoint(MaServer *server, HttpEndpoint *endpoint)
{
    mprAddItem(server->endpoints, endpoint);
}


void maRemoveEndpoint(MaServer *server, HttpEndpoint *endpoint)
{
    mprRemoveItem(server->endpoints, endpoint);
}


/*  
    Set the home directory (Server Root). We convert path into an absolute path.
 */
void maSetServerHome(MaServer *server, cchar *path)
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
    server->home = mprGetAbsPath(path);
    mprLog(MPR_CONFIG, "Set server root to: \"%s\"", server->home);
}


/*
    Set the document root for the default server
 */
void maSetServerAddress(MaServer *server, cchar *ip, int port)
{
    HttpEndpoint    *endpoint;
    int             next;

    for (next = 0; ((endpoint = mprGetNextItem(server->endpoints, &next)) != 0); ) {
        httpSetEndpointAddress(endpoint, ip, port);
    }
}


//  MOB - rename maSetGroup

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


//  MOB - rename maSetUser
int maSetHttpUser(MaAppweb *appweb, cchar *newUser)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;

    if (snumber(newUser)) {
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

    if (snumber(newGroup)) {
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
            return MPR_ERR_BAD_STATE;
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
            return MPR_ERR_BAD_STATE;
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
    Load a module. Returns 0 if the modules is successfully loaded (may have already been loaded).
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


static char *getSearchPath(cchar *dir)
{
#if WIN
        return sfmt("%s" MPR_SEARCH_SEP ".", dir);
#else
        char *libDir = mprJoinPath(mprGetPathParent(dir), BLD_LIB_NAME);
        return sfmt("%s" MPR_SEARCH_SEP "%s" MPR_SEARCH_SEP ".", dir,
            mprSamePath(BLD_BIN_PREFIX, dir) ? BLD_LIB_PREFIX: libDir);
#endif
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
