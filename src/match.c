/*
    match.c -- HTTP pipeline matching.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

static char *addIndexToUrl(HttpConn *conn, cchar *index);
static HttpStage *checkStage(HttpConn *conn, HttpStage *stage);
static char *getExtension(HttpConn *conn);
static HttpStage *findHandlerByExtension(HttpConn *conn, bool *rescan);
static HttpStage *findLocationHandler(HttpConn *conn);
static char *makeFilename(HttpConn *conn, MaAlias *alias, cchar *url, bool skipAliasPrefix);
static bool mapToFile(HttpConn *conn, HttpStage *handler, bool *rescan);
static HttpStage *matchHandler(HttpConn *conn);
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
    HttpReceiver    *rec;

    mprAssert(conn);

    switch (state) {
    case HTTP_STATE_FIRST:
        server = httpGetMetaServer(conn->server);
        listenSock = conn->sock->listenSock;
        address = (MaHostAddress*) maLookupHostAddress(server, listenSock->ip, listenSock->port);
        if (address == 0 || (host = mprGetFirstItem(address->vhosts)) == 0) {
            mprError(server, "No host configured for request %s:%d", listenSock->ip, listenSock->port);
            //  MOB TODO - should cancel request
            return;
        }
        if (maIsNamedVirtualHostAddress(address)) {
            rec = conn->receiver;
            if ((host = maLookupVirtualHost(address, rec->hostName)) == 0) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "No host to serve request. Searching for %s", rec->hostName);
            }
        }
        httpSetConnHost(conn, host);
        conn->documentRoot = host->documentRoot;
        conn->server->serverRoot = server->serverRoot;
        if (conn->receiver) {
            conn->receiver->location = host->location;
        }
        conn->transmitter->handler = matchHandler(conn);
		conn->traceLevel = host->traceLevel;
		conn->traceMaxLength = host->traceMaxLength;
		conn->traceMask = host->traceMask;
		conn->traceInclude = host->traceInclude;
		conn->traceExclude = host->traceExclude;
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
    Http            *http;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation    *location;
    HttpStage       *handler;
    MaHost          *host;
    MaAlias         *alias;
    bool            rescan;
    int             loopCount;

    http = conn->http;
    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    /*
        Find the alias that applies for this url. There is always a catch-all alias for the document root.
     */
    alias = rec->alias = maGetAlias(host, rec->pathInfo);
    mprAssert(alias);
    if (alias->redirectCode) {
        httpRedirect(conn, alias->redirectCode, alias->uri);
        return 0;
    }
    if (conn->error || (conn->receiver->flags & (HTTP_OPTIONS | HTTP_TRACE))) {
        location = rec->location = maLookupBestLocation(host, rec->pathInfo);
        mprAssert(location);
        rec->auth = location->auth;        
        return http->passHandler;
    }

    /*
        Get the best (innermost) location block and see if a handler is explicitly set for that location block.
        Possibly rewrite the url and retry.
     */
    loopCount = MA_MAX_REWRITE;
    do {
        rescan = 0;
        if ((handler = findLocationHandler(conn)) == 0) {
            handler = findHandlerByExtension(conn, &rescan);
        }
    } while (handler && rescan && loopCount-- > 0);

    if (conn->receiver->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        trans->traceMethods = handler->flags;
        handler = http->passHandler;
    }
    if (handler == 0) {
        httpError(conn, HTTP_CODE_BAD_METHOD, "Requested method %s not supported for URL: %s", rec->method, rec->uri);
    }
    if (conn->error || ((trans->flags & HTTP_TRANS_NO_BODY) && !(rec->flags & HTTP_HEAD))) {
        handler = http->passHandler;
    }
    location = rec->location;
    if (location && location->connector) {
        trans->connector = location->connector;
    } else if (handler == http->fileHandler && !rec->ranges && !conn->secure && trans->chunkSize <= 0 && !conn->traceMask) {
        trans->connector = http->sendConnector;
    } else {
        trans->connector = http->netConnector;
    }
    prepRequest(conn, handler);
    mprLog(trans, 4, "Select handler: \"%s\" for \"%s\"", handler->name, rec->uri);
    return handler;
}


static HttpStage *findLocationHandler(HttpConn *conn)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation    *location;
    HttpStage       *handler;
    MaHost          *host;
    int             loopCount;

    rec = conn->receiver;
    trans = conn->transmitter;
    loopCount = MA_MAX_REWRITE;
    host = httpGetConnHost(conn);
    handler = 0;
    do {
        location = rec->location = maLookupBestLocation(host, rec->pathInfo);
        mprAssert(location);
        rec->auth = location->auth;
        handler = checkStage(conn, location->handler);
    } while (maRewriteUri(conn) && --loopCount > 0);
    return handler;
}


/*
    Get an extension used for mime type matching. NOTE: this does not permit any kind of platform specific filename.
    Rather only those suitable as mime type extensions (simple alpha numeric extensions)
 */
static char *getExtension(HttpConn *conn)
{
    HttpReceiver    *rec;
    char            *cp;
    char            *ep, *ext;

    rec = conn->receiver;
    if (rec && rec->pathInfo) {
        if ((cp = strrchr(&rec->pathInfo[rec->alias->prefixLen], '.')) != 0) {
            ext = mprStrdup(rec, ++cp);
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
static HttpStage *findHandlerByExtension(HttpConn *conn, bool *rescan)
{
    Http            *http;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpStage       *handler;
    HttpLocation    *location;
    int             next;

    http = conn->http;
    rec = conn->receiver;
    trans = conn->transmitter;
    location = rec->location;
    handler = 0;
    *rescan = 0;
    
    if (rec->pathInfo == 0) {
        handler = http->passHandler;
    } else {
        trans->extension = getExtension(conn);
        if (*trans->extension) {
            handler = httpGetHandlerByExtension(location, trans->extension);
            if (!checkStage(conn, handler)) {
                handler = 0;
            }
        }
        if (handler == 0) {
            /*
                Failed to match by extension, so perform custom handler matching on all defined handlers
             */
            trans->filename = makeFilename(conn, rec->alias, rec->pathInfo, 1);
            for (next = 0; (handler = mprGetNextItem(location->handlers, &next)) != 0; ) {
                if (handler->match && handler->match(conn, handler)) {
                    if (checkStage(conn, handler)) {
                        break;
                    }
                }
            }
        }
    }
    if (handler == 0 && (handler = httpGetHandlerByExtension(location, "")) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing handler to match request %s", rec->pathInfo);
        handler = http->passHandler;
    }
    if (mapToFile(conn, handler, rescan)) {
        if (trans->fileInfo.isDir) {
            processDirectory(conn, rescan);
        }
    } else {
        handler = http->passHandler;
    }
    return handler;
}


static char *makeFilename(HttpConn *conn, MaAlias *alias, cchar *url, bool skipAliasPrefix)
{
    char        *cleanPath, *path;
    int         len;

    mprAssert(alias);
    mprAssert(url);

    if (skipAliasPrefix) {
        url += alias->prefixLen;
    }
    while (*url == '/') {
        url++;
    }
    len = (int) strlen(alias->filename);
    if ((path = mprAlloc(conn->receiver, len + (int) strlen(url) + 2)) == 0) {
        return 0;
    }
    strcpy(path, alias->filename);
    if (*url) {
        path[len++] = mprGetPathSeparator(path, path);
        strcpy(&path[len], url);
    }
    cleanPath = mprGetNativePath(conn, path);
    mprFree(path);
    return cleanPath;
}


//  MOB -- if the fileHandler is moved to http, then this code needs to go also
//  MOB -- perhaps this should be in Http regardless

static bool mapToFile(HttpConn *conn, HttpStage *handler, bool *rescan)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MaHost          *host;

    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    if (handler->flags & HTTP_STAGE_VIRTUAL) {
        return 1;
    }
    if (trans->filename == 0) {
        trans->filename = makeFilename(conn, rec->alias, rec->pathInfo, 1);
    }
    rec->dir = maLookupBestDir(host, trans->filename);
    if (rec->dir == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Missing directory block for %s", trans->filename);
        return 0;
    }
    if (rec->location->auth->type == 0) {
        rec->auth = rec->dir->auth;
    }
    if (!trans->fileInfo.checked && mprGetPathInfo(conn, trans->filename, &trans->fileInfo) < 0) {
        mprAssert(handler);
        if (!(rec->flags & HTTP_PUT) && handler->flags & HTTP_STAGE_VERIFY_ENTITY) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", trans->filename);
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
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprPath         *info;
    MaHost          *host;
    char            *path, *index;
    int             len, sep;

    rec = conn->receiver;
    trans = conn->transmitter;
    info = &trans->fileInfo;
    host = httpGetConnHost(conn);

    mprAssert(info->isDir);
    index = rec->dir->indexName;
    if (rec->pathInfo[strlen(rec->pathInfo) - 1] == '/') {
        /*  
            Internal directory redirections
         */
        //  TODO - is this really required?
        len = (int) strlen(trans->filename);
        sep = mprGetPathSeparator(trans, trans->filename);
        if (trans->filename[len - 1] == sep) {
            trans->filename[len - 1] = '\0';
        }
        path = mprJoinPath(trans, trans->filename, index);
        if (mprPathExists(trans, path, R_OK)) {
            /*  
                Index file exists, so do an internal redirect to it. Client will not be aware of this happening.
                Must rematch the handler on return.
             */
            httpSetUri(conn, addIndexToUrl(conn, index));
            trans->filename = path;
            mprGetPathInfo(conn, trans->filename, &trans->fileInfo);
            trans->extension = getExtension(conn);
#if UNUSED
            if ((trans->mimeType = (char*) maLookupMimeType(host, trans->extension)) == 0) {
                trans->mimeType = (char*) "text/html";
            }
#endif
            *rescan = 1;
        } else {
            mprFree(path);
        }
        return;
    }

    /*  
        External redirect. Ask the client to re-issue a request for a new location. See if an index exists and if so, 
        construct a new location for the index. If the index can't be accessed, just append a "/" to the URI and redirect.
     */
    if (rec->parsedUri->query && rec->parsedUri->query[0]) {
        path = mprAsprintf(trans, -1, "%s/%s?%s", rec->pathInfo, index, rec->parsedUri->query);
    } else {
        path = mprJoinPath(trans, rec->pathInfo, index);
    }
    if (!mprPathExists(trans, path, R_OK)) {
        path = mprStrcat(trans, -1, rec->pathInfo, "/", NULL);
    }
    httpRedirect(conn, HTTP_CODE_MOVED_PERMANENTLY, path);
}


static bool fileExists(MprCtx ctx, cchar *path) {
    if (mprPathExists(ctx, path, R_OK)) {
        return 1;
    }
#if BLD_WIN_LIKE
{
    char    *file;
    file = mprStrcat(ctx, -1, path, ".exe", NULL);
    if (mprPathExists(ctx, file, R_OK)) {
        return 1;
    }
    file = mprStrcat(ctx, -1, path, ".bat", NULL);
    if (mprPathExists(ctx, file, R_OK)) {
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
    MaAlias         *alias;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    char            *last, *start, *cp, *pathInfo, *scriptName;
    int             found, sep;

    rec = conn->receiver;
    trans = conn->transmitter;
    alias = rec->alias;
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
    scriptName = rec->scriptName = rec->pathInfo;
    trans->filename = makeFilename(conn, alias, rec->pathInfo, 1);
    sep = mprGetPathSeparator(trans, trans->filename);
    for (cp = start = &trans->filename[strlen(alias->filename)]; cp; ) {
        if ((cp = strchr(cp, sep)) != 0) {
            *cp = '\0';
        }
        found = fileExists(conn, trans->filename);
        if (cp) {
            *cp = sep;
        }
        if (found) {
            if (cp) {
                last = cp++;
            } else {
                last = &trans->filename[strlen(trans->filename)];
                break;
            }
        } else {
            break;
        }
    }
    if (last) {
        pathInfo = &scriptName[alias->prefixLen + last - start];
        rec->pathInfo = mprStrdup(rec, pathInfo);
        *last = '\0';
        pathInfo[0] = '\0';
        if (rec->pathInfo[0]) {
            rec->pathTranslated = makeFilename(conn, alias, rec->pathInfo, 0);
        }
    }
}


//  MOB -- perhaps this should be in Http regardless

/*
    Prepare to run the request. Set various transmitter fields and create env vars from the query
 */
static void prepRequest(HttpConn *conn, HttpStage *handler)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprPath         *info;
    MaHost          *host;

    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    setPathInfo(conn, handler);

    if (trans->extension == 0) {
        trans->extension = getExtension(conn);
    }
#if UNUSED
    if ((trans->mimeType = (char*) maLookupMimeType(host, trans->extension)) == 0) {
        //  TODO - set a header
        trans->mimeType = (char*) "text/html";
    }
#endif
    if (trans->filename) {
#if UNUSED
        if (trans->filename == 0) {
            trans->filename = makeFilename(conn, rec->alias, rec->pathInfo, 1);
        }
#endif
        /*
            Define an Etag for physical entities. Redo the file info if not valid now that extra path has been removed.
         */
        info = &trans->fileInfo;
        if (!info->checked) {
            mprGetPathInfo(conn, trans->filename, info);
        }
        if (info->valid) {
            //  TODO - set a header here
            //  MOB -- should be moved back to http/src
            trans->etag = mprAsprintf(trans, -1, "\"%x-%Lx-%Lx\"", info->inode, info->size, info->mtime);
        }
    }
}


static char *addIndexToUrl(HttpConn *conn, cchar *index)
{
    HttpReceiver    *rec;
    char            *path;

    rec = conn->receiver;
    path = mprJoinPath(rec, rec->pathInfo, index);
    if (rec->parsedUri->query && rec->parsedUri->query[0]) {
        return mprReallocStrcat(rec, -1, path, "?", rec->parsedUri->query, NULL);
    }
    return path;
}


static HttpStage *checkStage(HttpConn *conn, HttpStage *stage)
{
    HttpReceiver   *rec;

    rec = conn->receiver;
    if (stage == 0) {
        return 0;
    }
    if ((stage->flags & HTTP_STAGE_ALL & rec->flags) == 0) {
        return 0;
    }
    if (stage->match && !stage->match(conn, stage)) {
        return 0;
    }
    return stage;
}


//  MOB
int maRewriteUri(HttpConn *conn) { return 0; }

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
