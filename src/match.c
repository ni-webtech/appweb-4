/*
    match.c -- HTTP pipeline matching.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

static char *addIndexToUrl(HttpConn *conn, cchar *index);
static char *getExtension(HttpConn *conn);
static HttpStage *checkHandler(HttpConn *conn, HttpStage *stage);
static HttpStage *findHandler(HttpConn *conn, bool *rescan);
static HttpStage *findLocationHandler(HttpConn *conn);
static char *makeFilename(HttpConn *conn, MaAlias *alias, cchar *url, bool skipAliasPrefix);
static bool mapToFile(HttpConn *conn, HttpStage *handler, bool *rescan);
static HttpStage *matchHandler(HttpConn *conn);
static bool modifyRequest(HttpConn *conn);
static void processDirectory(HttpConn *conn, bool *rescan);
static void prepRequest(HttpConn *conn, HttpStage *handler);
static void setPathInfo(HttpConn *conn, HttpStage *handler);

//  MOB
int maRewriteUri(HttpConn *conn);

/*********************************** Code *************************************/
/*
    Notification callback from Http. Invoked for each state change.
 */
void maNotifyServerStateChange(HttpConn *conn, int state, int notifyFlags)
{
    MprSocket       *listenSock;
    MaHostAddress   *address;
    MaHost          *host;
    MaServer        *server;
    HttpRx          *rx;

    mprAssert(conn);

    switch (state) {
    case HTTP_STATE_FIRST:
        break;

    case HTTP_STATE_PARSED:
        server = httpGetMetaServer(conn->server);
        listenSock = conn->sock->listenSock;
        address = (MaHostAddress*) maLookupHostAddress(server, listenSock->ip, listenSock->port);
        if (address == 0 || (host = mprGetFirstItem(address->vhosts)) == 0) {
            mprError("No host configured for request %s:%d", listenSock->ip, listenSock->port);
            httpSetConnHost(conn, server->defaultHost);
            return;
        }
        if (maIsNamedVirtualHostAddress(address)) {
            rx = conn->rx;
            if ((host = maLookupVirtualHost(address, rx->hostName)) == 0) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "No host to serve request. Searching for %s", rx->hostName);
                httpSetConnHost(conn, server->defaultHost);
                return;
            }
        }
        mprAssert(host);
        httpSetConnHost(conn, host);
        conn->documentRoot = host->documentRoot;
        conn->server->serverRoot = server->serverRoot;
        if (conn->rx) {
            conn->rx->loc = host->loc;
        }
#if MOB
        if (mprGetLogLevel(conn) >= host->traceLevel) {
            conn->traceLevel = host->traceLevel;
            conn->traceMaxLength = host->traceMaxLength;
            conn->traceMask = host->traceMask;
            conn->traceInclude = host->traceInclude;
            conn->traceExclude = host->traceExclude;
        }            
#endif
        conn->tx->handler = matchHandler(conn);  
        break;
    }
}


//  MOB -- this routine can be in Http

/*
    Find the matching handler for a request. If any errors occur, the pass handler is used to pass errors onto the 
    net/sendfile connectors to send to the client. This routine may rewrite the request URI and may redirect the request.
 */
static HttpStage *matchHandler(HttpConn *conn)
{
    Http        *http;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLoc     *loc;
    HttpStage   *handler;
    MaHost      *host;
    MaAlias     *alias;
    bool        rescan;
    int         loopCount;

    http = conn->http;
    rx = conn->rx;
    tx = conn->tx;
    host = conn->host;

    /*
        Find the alias that applies for this url. There is always a catch-all alias for the document root.
     */
    alias = rx->alias = maGetAlias(host, rx->pathInfo);
    mprAssert(alias);
    if (alias->redirectCode) {
        httpRedirect(conn, alias->redirectCode, alias->uri);
        return NULL;
    }
    loc = rx->loc = maLookupBestLocation(host, rx->pathInfo);
    mprAssert(loc);
    rx->auth = loc->auth;        

    if (modifyRequest(conn)) {
        return NULL;
    }
    /*
        Get the best (innermost) location block and see if a handler is explicitly set for that location block.
        Possibly rewrite the url and retry.
     */
    loopCount = MA_MAX_REWRITE;
    do {
        rescan = 0;
        if ((handler = findLocationHandler(conn)) == 0) {
            handler = findHandler(conn, &rescan);
        }
    } while (handler && rescan && loopCount-- > 0);

    if (handler == 0) {
        handler = http->passHandler;
        if (!(rx->flags & (HTTP_OPTIONS | HTTP_TRACE))) {
            httpError(conn, HTTP_CODE_BAD_METHOD, "Requested method %s not supported for URL: %s", rx->method, rx->uri);
        }

    } else if (rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        tx->traceMethods = handler->flags;
        /*
            If the handler does not support OPTIONS or TRACE, send to the passHandler
         */
        if ((rx->flags & HTTP_OPTIONS) != !(handler->flags & HTTP_STAGE_OPTIONS)) {
            handler = http->passHandler;

        } else if ((rx->flags & HTTP_TRACE) != !(handler->flags & HTTP_STAGE_TRACE)) {
            handler = http->passHandler;
        }
    }
    if (conn->error || ((tx->flags & HTTP_TX_NO_BODY) && !(rx->flags & HTTP_HEAD))) {
        handler = http->passHandler;
    }
    loc = rx->loc;
    if (loc && loc->connector) {
        tx->connector = loc->connector;
    } else if (handler == http->fileHandler && !rx->ranges && !conn->secure && tx->chunkSize <= 0 
            /* MOB && !conn->traceMask */) {
        tx->connector = http->sendConnector;
    } else {
        tx->connector = http->netConnector;
    }
    prepRequest(conn, handler);
    mprLog(4, "Select handler: \"%s\" for \"%s\"", handler->name, rx->uri);
    return handler;
}


static bool modifyRequest(HttpConn *conn)
{
    HttpStage       *handler;
    HttpLoc         *loc;
    HttpTx          *tx;
    HttpRx          *rx;
    MprPath         *info;
    MprHash         *he;
    MprList         *handlers;
    int             next;

    rx = conn->rx;
    tx = conn->tx;
    handlers = NULL;

    if (tx->filename == 0) {
        tx->filename = makeFilename(conn, rx->alias, rx->uri, 1);
    }
    info = &tx->fileInfo;
    if (!info->checked) {
        mprGetPathInfo(tx->filename, info);
    }
    loc = conn->rx->loc;
    if (loc) {
        for (next = 0; (handler = mprGetNextItem(loc->handlers, &next)) != 0; ) {
            if (handler->modify) {
                if (handlers == NULL) {
                    handlers = mprCreateList();
                }
                if (mprLookupItem(handlers, handler) < 0) {
                    mprAddItem(handlers, handler);
                    if (handler->modify(conn, handler)) {
                        return 1;
                    }
                }
            }
        }
        for (he = 0; (he = mprGetNextHash(loc->extensions, he)) != 0; ) {
            handler = (HttpStage*) he->data;
            if (handler->modify) {
                if (mprLookupItem(handlers, handler) < 0) {
                    mprAddItem(handlers, handler);
                    if (handler->modify(conn, handler)) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}


static HttpStage *findLocationHandler(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLoc     *loc;
    MaHost      *host;

    rx = conn->rx;
    tx = conn->tx;
    host = httpGetConnHost(conn);
    loc = rx->loc = maLookupBestLocation(host, rx->pathInfo);
    rx->auth = loc->auth;
    mprAssert(loc);
    return checkHandler(conn, loc->handler);
}


/*
    Get an extension used for mime type matching. NOTE: this does not permit any kind of platform specific filename.
    Rather only those suitable as mime type extensions (simple alpha numeric extensions)
 */
static char *getExtension(HttpConn *conn)
{
    HttpRx      *rx;
    char        *cp;
    char        *ep, *ext;

    rx = conn->rx;
    if (rx && rx->pathInfo) {
        if ((cp = strrchr(&rx->pathInfo[rx->alias->prefixLen], '.')) != 0) {
            ext = sclone(++cp);
            for (ep = ext; *ep && isalnum((int)*ep); ep++) {
                ;
            }
            *ep = '\0';
            return ext;
        }
    }
    return "";
}


/*
    Search for a handler by request extension. If that fails, use handler custom matching.
 */
static HttpStage *findHandler(HttpConn *conn, bool *rescan)
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
    *rescan = 0;
    
    if (rx->pathInfo == 0) {
        handler = http->passHandler;
    } else {
        tx->extension = getExtension(conn);
        if (*tx->extension) {
            handler = checkHandler(conn, httpGetHandlerByExtension(loc, tx->extension));
        }
        if (handler == 0) {
            /*
                URI has no extension, check if the addition of configured  extensions results in a valid filename.
             */
            for (path = 0, hp = 0; (hp = mprGetNextHash(loc->extensions, hp)) != 0; ) {
                handler = (HttpStage*) hp->data;
                if (*hp->key && (handler->flags & HTTP_STAGE_MISSING_EXT)) {
                    path = sjoin(tx->filename, ".", hp->key, NULL);
                    if (mprGetPathInfo(path, &tx->fileInfo) == 0) {
                        mprLog(5, "findHandler: Adding extension, new path %s\n", path);
                        tx->filename = path;
                        httpSetUri(conn, sjoin(rx->uri, ".", hp->key, NULL));
                        break;
                    }
                }
            }
        }
        if (handler == 0) {
            /*
                Failed to match by extension, so perform custom handler matching on all defined handlers
             */
            tx->filename = makeFilename(conn, rx->alias, rx->pathInfo, 1);
            for (next = 0; (handler = mprGetNextItem(loc->handlers, &next)) != 0; ) {
                if (checkHandler(conn, handler)) {
                    break;
                }
            }
        }
    }
    if (handler == 0 && (handler = httpGetHandlerByExtension(loc, "")) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing handler to match request %s", rx->pathInfo);
        handler = http->passHandler;
    }
    if (mapToFile(conn, handler, rescan)) {
        if (tx->fileInfo.isDir) {
            processDirectory(conn, rescan);
        }
    } else {
        handler = http->passHandler;
    }
    return handler;
}


static char *makeFilename(HttpConn *conn, MaAlias *alias, cchar *url, bool skipAliasPrefix)
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


//  MOB -- if the fileHandler is moved to http, then this code needs to go also
//  MOB -- perhaps this should be in Http regardless

static bool mapToFile(HttpConn *conn, HttpStage *handler, bool *rescan)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MaHost      *host;

    rx = conn->rx;
    tx = conn->tx;
    host = httpGetConnHost(conn);

    if (handler->flags & HTTP_STAGE_VIRTUAL) {
        return 1;
    }
    if (tx->filename == 0) {
        tx->filename = makeFilename(conn, rx->alias, rx->pathInfo, 1);
    }
    rx->dir = maLookupBestDir(host, tx->filename);
    if (rx->dir == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Missing directory block for %s", tx->filename);
        return 0;
    }
    if (rx->loc->auth->type == 0) {
        rx->auth = rx->dir->auth;
    }
    if (!tx->fileInfo.checked && mprGetPathInfo(tx->filename, &tx->fileInfo) < 0) {
        mprAssert(handler);
        if (!(rx->flags & HTTP_PUT) && handler->flags & HTTP_STAGE_VERIFY_ENTITY) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", tx->filename);
            return 0;
        }
    }
    return 1;
}


//  MOB -- perhaps this should be in Http regardless

/*
    Manage requests to directories. This will either do an external redirect back to the browser or do an internal 
    (transparent) redirection and serve different content back to the browser. This routine may modify the requested 
    URI and/or the request handler.
 */
static void processDirectory(HttpConn *conn, bool *rescan)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprPath     *info;
    MaHost      *host;
    cchar       *seps;
    char        *path, *index;
    size_t      len;

    rx = conn->rx;
    tx = conn->tx;
    info = &tx->fileInfo;
    host = httpGetConnHost(conn);

    mprAssert(info->isDir);
    index = rx->dir->indexName;
    if (rx->pathInfo[strlen(rx->pathInfo) - 1] == '/') {
        /*  
            Internal directory redirections
         */
        //  TODO - is this really required?
        len = strlen(tx->filename);
        seps = mprGetPathSeparators(tx->filename);
        if (tx->filename[len - 1] == seps[0]) {
            tx->filename[len - 1] = '\0';
        }
        path = mprJoinPath(tx->filename, index);
        if (mprPathExists(path, R_OK)) {
            /*  
                Index file exists, so do an internal redirect to it. Client will not be aware of this happening.
                Must rematch the handler on return.
             */
            httpSetUri(conn, addIndexToUrl(conn, index));
            tx->filename = path;
            mprGetPathInfo(tx->filename, &tx->fileInfo);
            tx->extension = getExtension(conn);
#if UNUSED
            if ((tx->mimeType = (char*) maLookupMimeType(host, tx->extension)) == 0) {
                tx->mimeType = (char*) "text/html";
            }
#endif
            *rescan = 1;
        }
        return;
    }

    /*  
        External redirect. Ask the client to re-issue a request for a new location. See if an index exists and if so, 
        construct a new location for the index. If the index can't be accessed, just append a "/" to the URI and redirect.
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
}


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


//  MOB -- perhaps this should be in Http regardless

/*  
    Set the pathInfo (PATH_INFO) and update the request uri. This may set the response filename if convenient.
 */
static void setPathInfo(HttpConn *conn, HttpStage *handler)
{
    MaAlias     *alias;
    HttpRx      *rx;
    HttpTx      *tx;
    cchar       *seps;
    char        *last, *start, *cp, *pathInfo, *scriptName;
    int         found;

    rx = conn->rx;
    tx = conn->tx;
    alias = rx->alias;
    mprAssert(handler);

    if (!(handler && handler->flags & HTTP_STAGE_PATH_INFO)) {
        return;
    }
    /*  
        Find the longest subset of the filename that matches a real file. Test each segment to see if 
        it corresponds to a real physical file. This also defines a new response filename after trimming the 
        extra path info.
     */
    last = 0;
    scriptName = rx->scriptName = rx->pathInfo;
    tx->filename = makeFilename(conn, alias, rx->pathInfo, 1);
    seps = mprGetPathSeparators(tx->filename);
    for (cp = start = &tx->filename[strlen(alias->filename)]; cp; ) {
        if ((cp = strchr(cp, seps[0])) != 0) {
            *cp = '\0';
        }
        found = fileExists(tx->filename);
        if (cp) {
            *cp = seps[0];
        }
        if (found) {
            if (cp) {
                last = cp++;
            } else {
                last = &tx->filename[strlen(tx->filename)];
                break;
            }
        } else {
            break;
        }
    }
    if (last) {
        pathInfo = &scriptName[alias->prefixLen + last - start];
        rx->pathInfo = sclone(pathInfo);
        *last = '\0';
        pathInfo[0] = '\0';
        if (rx->pathInfo[0]) {
            rx->pathTranslated = makeFilename(conn, alias, rx->pathInfo, 0);
        }
    }
}


//  MOB -- perhaps this should be in Http regardless

/*
    Prepare to run the request. Set various transmitter fields and create env vars from the query
 */
static void prepRequest(HttpConn *conn, HttpStage *handler)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprPath     *info;
    MaHost      *host;

    rx = conn->rx;
    tx = conn->tx;
    host = httpGetConnHost(conn);

    setPathInfo(conn, handler);

    if (tx->extension == 0) {
        tx->extension = getExtension(conn);
    }
#if UNUSED
    if ((tx->mimeType = (char*) maLookupMimeType(host, tx->extension)) == 0) {
        //  TODO - set a header
        tx->mimeType = (char*) "text/html";
    }
#endif
    if (tx->filename) {
#if UNUSED
        if (tx->filename == 0) {
            tx->filename = makeFilename(conn, rx->alias, rx->pathInfo, 1);
        }
#endif
        /*
            Define an Etag for physical entities. Redo the file info if not valid now that extra path has been removed.
         */
        info = &tx->fileInfo;
        if (!info->checked) {
            mprGetPathInfo(tx->filename, info);
        }
        if (info->valid) {
            //  TODO - set a header here
            //  MOB -- should be moved back to http/src
            tx->etag = mprAsprintf("\"%x-%Lx-%Lx\"", info->inode, info->size, info->mtime);
        }
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
    if (stage->match && !stage->match(conn, stage)) {
        return 0;
    }
    return stage;
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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
