/*
    host.c -- Host class for all HTTP hosts

    The Host class is used for the default HTTP server and for all virtual hosts (including SSL hosts).
    Many objects are controlled at the host level. Eg. URL handlers.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/*********************************** Code *************************************/

/*  Create a host from scratch
 */
MaHost *maCreateHost(MaServer *server, cchar *ipAddrPort, HttpLocation *location)
{
    MaHost      *host;

    host = mprAllocObjZeroed(server, MaHost);
    if (host == 0) {
        return 0;
    }
    host->aliases = mprCreateList(host);
    host->dirs = mprCreateList(host);
#if UNUSED
    host->connections = mprCreateList(host);
#endif
    host->locations = mprCreateList(host);

    if (ipAddrPort) {
        maSetHostIpAddrPort(host, ipAddrPort);
    }
    host->server = server;
    host->flags = MA_HOST_NO_TRACE;
    host->httpVersion = 1;

#if UNUSED
    //  MOB -- is this used anymore 
    host->timeout = HTTP_SERVER_TIMEOUT;

    //  MOB -- is this necessary as limits are pushed down into http
    host->limits = server->appweb->http->limits;

    host->keepAliveTimeout = HTTP_KEEP_TIMEOUT;
    host->maxKeepAlive = HTTP_MAX_KEEP_ALIVE;
    host->keepAlive = 1;
#endif

    host->location = (location) ? location : httpCreateLocation(server->http);
    maAddLocation(host, host->location);
    host->location->auth = httpCreateAuth(host->location, host->location->auth);
#if UNUSED
    host->mutex = mprCreateLock(host);
#endif
    return host;
}


/*  
    Create a new virtual host and inherit settings from another host
 */
MaHost *maCreateVirtualHost(MaServer *server, cchar *ipAddrPort, MaHost *parent)
{
    MaHost      *host;

    host = mprAllocObjZeroed(server, MaHost);
    if (host == 0) {
        return 0;
    }
    host->parent = parent;
#if UNUSED
    host->connections = mprCreateList(host);
#endif
    if (ipAddrPort) {
        maSetHostIpAddrPort(host, ipAddrPort);
    }
    /*  
        The aliases, dirs and locations are all copy-on-write
     */
    host->aliases = parent->aliases;
    host->dirs = parent->dirs;
    host->locations = parent->locations;
    host->server = parent->server;
    host->flags = parent->flags;
    host->httpVersion = parent->httpVersion;
#if UNUSED
    host->timeout = parent->timeout;
    host->limits = parent->limits;
    host->keepAliveTimeout = parent->keepAliveTimeout;
    host->maxKeepAlive = parent->maxKeepAlive;
    host->keepAlive = parent->keepAlive;
#endif
    host->accessLog = parent->accessLog;
    host->location = httpCreateInheritedLocation(server->http, parent->location);
#if UNUSED
    host->mutex = mprCreateLock(host);
#endif
    maAddLocation(host, host->location);
    return host;
}


/*  
    Convenience function to create a new default host
 */
MaHost *maCreateDefaultHost(MaServer *server, cchar *docRoot, cchar *ip, int port)
{
    MaHost          *host;
    HttpServer      *httpServer;
    MaHostAddress   *address;

    if (ip == 0) {
        /*  
            If no IP:PORT specified, find the first listening endpoint. In this case, we expect the caller to
            have setup the lisenting endponts and to have added them to the host address hash.
         */
        httpServer = mprGetFirstItem(server->httpServers);
        if (httpServer) {
            ip = httpServer->ip;
            port = httpServer->port;

        } else {
            ip = "localhost";
            if (port <= 0) {
                port = HTTP_DEFAULT_PORT;
            }
            httpServer = httpCreateServer(server->appweb->http, ip, port, NULL);
            maAddHttpServer(server, httpServer);
        }
        host = maCreateHost(server, ip, NULL);

    } else {
        /*  
            Create a new listening endpoint
         */
        httpServer = httpCreateServer(server->appweb->http, ip, port, NULL);
        maAddHttpServer(server, httpServer);
        host = maCreateHost(server, ip, NULL);
    }
    if (maOpenMimeTypes(host, "mime.types") < 0) {
        maAddStandardMimeTypes(host);
    }
    /*  
        Insert the host and create a directory object for the docRoot
     */
    maAddHost(server, host);
    maInsertDir(host, maCreateBareDir(host, docRoot));
    maSetHostDocumentRoot(host, docRoot);

    /* 
        Ensure we are in the hash lookup of all the addresses to listen to acceptWrapper uses this hash to find
        the host to serve the request.
     */
    address = maLookupHostAddress(server, ip, port);
    if (address == 0) {
        address = maCreateHostAddress(server, ip, port);
        mprAddItem(server->hostAddresses, address);
    }
    maInsertVirtualHost(address, host);

    if (server->defaultHost == 0) {
        server->defaultHost = host;
    }
    return host;
}


int maStartHost(MaHost *host)
{
    return maStartAccessLogging(host);
}


int maStopHost(MaHost *host)
{
    return maStopAccessLogging(host);
}


void maSetHostDocumentRoot(MaHost *host, cchar *dir)
{
    MaAlias     *alias;
    char        *doc;
    int         len;

    doc = host->documentRoot = maMakePath(host, dir);
    len = (int) strlen(doc);
    if (doc[len - 1] == '/') {
        doc[len - 1] = '\0';
    }
    /*  
        Create a catch-all alias
     */
    alias = maCreateAlias(host, "", doc, 0);
    maInsertAlias(host, alias);
}


/*  
    Set the host name. Comes from the ServerName directive. Name should not contain "http://"
 */
void maSetHostName(MaHost *host, cchar *name)
{
    if (host->name) {
        mprFree(host->name);
    }
    host->name = mprStrdup(host, name);
}


void maSetHostIpAddrPort(MaHost *host, cchar *ipAddrPort)
{
    char    *cp;

    mprAssert(ipAddrPort);
    
#if UNUSED
    maRemoveHostFromHostAddress(host->server, host->ip, host->port, host);
#endif
    mprFree(host->ipAddrPort);

    if (*ipAddrPort == ':') {
        ++ipAddrPort;
    }
    if (isdigit((int) *ipAddrPort) && strchr(ipAddrPort, '.') == 0) {
        host->ipAddrPort = mprStrcat(host, -1, "127.0.0.1", ":", ipAddrPort, NULL);
    } else {
        host->ipAddrPort = mprStrdup(host, ipAddrPort);
    }
    if ((cp = strchr(host->ipAddrPort, ':')) != 0) {
        *cp++ = '\0';
        host->ip = mprStrdup(host, host->ipAddrPort);
        host->port = (int) mprAtoi(cp, 10);
        cp[-1] = ':';
    }
    if (host->name == 0 || strchr(host->name, ':')) {
        mprFree(host->name);
        host->name = mprStrdup(host, host->ipAddrPort);
    }
#if UNUSED
    maAddHostAddress(host->server, host->ip, host->port);
#endif
}


void maSetHttpVersion(MaHost *host, int version)
{
    host->httpVersion = version;
}


#if UNUSED
void maSetKeepAlive(MaHost *host, bool on)
{
    host->keepAlive = on;
}


void maSetKeepAliveTimeout(MaHost *host, int timeout)
{
    //  MOB -- is this used
    host->keepAliveTimeout = timeout;
}


void maSetMaxKeepAlive(MaHost *host, int timeout)
{
    //  MOB -- is this used
    host->maxKeepAlive = timeout;
}
#endif


void maSetNamedVirtualHost(MaHost *host)
{
    host->flags |= MA_HOST_NAMED_VHOST;
}


void maSecureHost(MaHost *host, struct MprSsl *ssl)
{
    HttpServer  *httpServer;
    cchar       *hostIp;
    char        *ip;
    int         port, next;

#if UNUSED
    host->secure = 1;
#endif
    hostIp = host->ipAddrPort;
    if (mprStrcmpAnyCase(hostIp, "_default_") == 0) {
        hostIp = (char*) "*:*";
    }
    mprParseIp(host, hostIp, &ip, &port, -1);
   
    for (next = 0; (httpServer = mprGetNextItem(host->server->httpServers, &next)) != 0; ) {
        if (port > 0 && port != httpServer->port) {
            continue;
        }
        if (*httpServer->ip && ip && ip[0] != '*' && strcmp(ip, httpServer->ip) != 0) {
            continue;
        }
#if BLD_FEATURE_SSL
        if (host->flags & MA_HOST_NAMED_VHOST) {
            mprError(host, "SSL does not support named virtual hosts");
            return;
        }
        httpServer->ssl = ssl;
#endif
    }
}


void maSetVirtualHost(MaHost *host)
{
    host->flags |= MA_HOST_VHOST;
}


int maInsertAlias(MaHost *host, MaAlias *newAlias)
{
    MaAlias     *alias, *old;
    int         rc, next, index;

    if (mprGetParent(host->aliases) == host->parent) {
        host->aliases = mprDupList(host, host->parent->aliases);
    }

    /*  Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc. But we sort redirects with
        status codes first.
     */
    for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
        rc = strcmp(newAlias->prefix, alias->prefix);
        if (rc == 0) {
            index = mprLookupItem(host->aliases, alias);
            old = (MaAlias*) mprGetItem(host->aliases, index);
            mprRemoveItem(host->aliases, alias);
            mprInsertItemAtPos(host->aliases, next - 1, newAlias);
            return 0;
            
        } else if (rc > 0) {
            if (newAlias->redirectCode >= alias->redirectCode) {
                mprInsertItemAtPos(host->aliases, next - 1, newAlias);
                return 0;
            }
        }
    }
    mprAddItem(host->aliases, newAlias);
    return 0;
}


int maInsertDir(MaHost *host, MaDir *newDir)
{
    MaDir       *dir;
    int         rc, next;

    mprAssert(newDir);
    mprAssert(newDir->path);
    
    if (mprGetParent(host->dirs) == host->parent) {
        host->dirs = mprDupList(host, host->parent->dirs);
    }

    /*
        Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc
     */
    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        mprAssert(dir->path);
        rc = strcmp(newDir->path, dir->path);
        if (rc == 0) {
            mprRemoveItem(host->dirs, dir);
            mprInsertItemAtPos(host->dirs, next - 1, newDir);
            return 0;

        } else if (rc > 0) {
            mprInsertItemAtPos(host->dirs, next - 1, newDir);
            return 0;
        }
    }

    mprAddItem(host->dirs, newDir);
    return 0;
}


int maAddLocation(MaHost *host, HttpLocation *newLocation)
{
    HttpLocation  *location;
    int         next, rc;

    mprAssert(newLocation);
    mprAssert(newLocation->prefix);
    
    if (mprGetParent(host->locations) == host->parent) {
        host->locations = mprDupList(host, host->parent->locations);
    }

    /*
        Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc
     */
    for (next = 0; (location = mprGetNextItem(host->locations, &next)) != 0; ) {
        rc = strcmp(newLocation->prefix, location->prefix);
        if (rc == 0) {
            mprRemoveItem(host->locations, location);
            mprInsertItemAtPos(host->locations, next - 1, newLocation);
            return 0;
        }
        if (strcmp(newLocation->prefix, location->prefix) > 0) {
            mprInsertItemAtPos(host->locations, next - 1, newLocation);
            return 0;
        }
    }
    mprAddItem(host->locations, newLocation);

    return 0;
}


MaAlias *maGetAlias(MaHost *host, cchar *uri)
{
    MaAlias     *alias;
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


MaAlias *maLookupAlias(MaHost *host, cchar *prefix)
{
    MaAlias     *alias;
    int         next;

    for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
        if (strcmp(alias->prefix, prefix) == 0) {
            return alias;
        }
    }
    return 0;
}


/*  
    Find an exact dir match
 */
MaDir *maLookupDir(MaHost *host, cchar *pathArg)
{
    MaDir       *dir;
    char        *path, *tmpPath;
    int         next, len;

    if (!mprIsAbsPath(host, pathArg)) {
        path = tmpPath = mprGetAbsPath(host, pathArg);
    } else {
        path = (char*) pathArg;
        tmpPath = 0;
    }
    len = (int) strlen(path);

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        mprAssert(strlen(dir->path) == 0 || dir->path[strlen(dir->path) - 1] != '/');
        if (dir->path != 0) {
            if (mprSamePath(host, dir->path, path)) {
                mprFree(tmpPath);
                return dir;
            }
        }
    }
    mprFree(tmpPath);
    return 0;
}


void maSetHostDirs(MaHost *host, cchar *path)
{
    MaDir       *dir;
    int         next;

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        maSetDirPath(dir, path);
    }
}


/*  
    Find the directory entry that this file (path) resides in. path is a physical file path. We find the most specific
    (longest) directory that matches. The directory must match or be a parent of path. Not called with raw files names.
    They will be lower case and only have forward slashes. For windows, the will be in cannonical format with drive
    specifiers.
 */
MaDir *maLookupBestDir(MaHost *host, cchar *path)
{
    MaDir   *dir;
    int     next, len, dlen;

    len = (int) strlen(path);

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        dlen = dir->pathLen;
        mprAssert(dlen == 0 || dir->path[dlen - 1] != '/');
        if (mprSamePathCount(host, dir->path, path, dlen)) {
            if (dlen >= 0) {
                return dir;
            }
        }
    }
    return 0;
}


HttpLocation *maLookupLocation(MaHost *host, cchar *prefix)
{
    HttpLocation      *location;
    int             next;

    for (next = 0; (location = mprGetNextItem(host->locations, &next)) != 0; ) {
        if (strcmp(prefix, location->prefix) == 0) {
            return location;
        }
    }
    return 0;
}


HttpLocation *maLookupBestLocation(MaHost *host, cchar *uri)
{
    HttpLocation  *location;
    int         next, rc;

    if (uri) {
        for (next = 0; (location = mprGetNextItem(host->locations, &next)) != 0; ) {
            rc = strncmp(location->prefix, uri, location->prefixLen);
            if (rc == 0 && uri[location->prefixLen] == '/') {
                return location;
            }
        }
    }
    return mprGetLastItem(host->locations);
}


MaHostAddress *maCreateHostAddress(MprCtx ctx, cchar *ip, int port)
{
    MaHostAddress   *hostAddress;

    mprAssert(ip && ip);
    mprAssert(port >= 0);

    hostAddress = mprAllocObjZeroed(ctx, MaHostAddress);
    if (hostAddress == 0) {
        return 0;
    }
    hostAddress->flags = 0;
    hostAddress->ip = mprStrdup(hostAddress, ip);
    hostAddress->port = port;
    hostAddress->vhosts = mprCreateList(hostAddress);
    return hostAddress;
}


void maInsertVirtualHost(MaHostAddress *hostAddress, MaHost *vhost)
{
    mprAddItem(hostAddress->vhosts, vhost);
}


bool maIsNamedVirtualHostAddress(MaHostAddress *hostAddress)
{
    return hostAddress->flags & MA_IPADDR_VHOST;
}


void maSetNamedVirtualHostAddress(MaHostAddress *hostAddress)
{
    hostAddress->flags |= MA_IPADDR_VHOST;
}


/*
    Look for a host with the right host name (ServerName)
 */
MaHost *maLookupVirtualHost(MaHostAddress *hostAddress, cchar *hostStr)
{
    MaHost      *host;
    int         next;

    for (next = 0; (host = mprGetNextItem(hostAddress->vhosts, &next)) != 0; ) {
        /*  TODO  -- need to support aliases */
        if (hostStr == 0 || strcmp(hostStr, host->name) == 0) {
            return host;
        }
    }
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

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

    @end
 */
