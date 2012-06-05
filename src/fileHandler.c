/*
    fileHandler.c -- Static file content handler

    This handler manages static file based content such as HTML, GIF /or JPEG pages. It supports all methods including:
    GET, PUT, DELETE, OPTIONS and TRACE. It is event based and does not use worker threads.

    The fileHandler also manages requests for directories that require redirection to an index or responding with
    a directory listing. 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

static int findFile(HttpConn *conn);
static void handleDeleteRequest(HttpQueue *q);
static void handlePutRequest(HttpQueue *q);
static ssize readFileData(HttpQueue *q, HttpPacket *packet, MprOff pos, ssize size);

/*********************************** Code *************************************/
/*
    Test if the request matches. This may delegate the request to the dirHandler if a directory listing is required.
 */
static int matchFileHandler(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpRx      *rx;
    
    rx = conn->rx;
    httpMapFile(conn, route);
    if (rx->flags & (HTTP_GET | HTTP_HEAD | HTTP_POST)) {
        return findFile(conn);
    }
    return HTTP_ROUTE_OK;
}


/*
    Map the request pathInfo to a physical file. This may respond by generating an error if the file can't be found.
 */
static int findFile(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpUri     *prior;
    HttpRoute   *route;
    MprPath     *info, zipInfo;
    cchar       *index;
    char        *path, *pathInfo, *uri, *zipfile;
    int         next;

    tx = conn->tx;
    rx = conn->rx;
    route = rx->route;
    prior = rx->parsedUri;
    info = &tx->fileInfo;

    mprAssert(info->checked);

    if (info->isDir) {
        /*
            Manage requests for directories
         */
        if (!sends(rx->pathInfo, "/")) {
            /* 
               Append "/" and do an external redirect 
             */
            pathInfo = sjoin(rx->pathInfo, "/", NULL);
            uri = httpFormatUri(prior->scheme, prior->host, prior->port, pathInfo, prior->reference, prior->query, 0);
            httpRedirect(conn, HTTP_CODE_MOVED_PERMANENTLY, uri);
            return HTTP_ROUTE_OK;
        } 
        if (route->indicies) {
            /*
                Ends with a "/" so do internal redirection to an index file
             */
            for (ITERATE_ITEMS(route->indicies, index, next)) {
                /*  
                    Internal directory redirections. Transparently append index. Test indicies in order.
                 */
                path = mprJoinPath(tx->filename, index);
                if (mprPathExists(path, R_OK)) {
                    pathInfo = sjoin(rx->scriptName, rx->pathInfo, index, NULL);
                    uri = httpFormatUri(prior->scheme, prior->host, prior->port, pathInfo, prior->reference, 
                        prior->query, 0);
                    httpSetUri(conn, uri, 0);
                    tx->filename = path;
                    tx->ext = httpGetExt(conn);
                    mprGetPathInfo(tx->filename, info);
                    return HTTP_ROUTE_REROUTE;
                }
            }
        }
        /*
            If a directory, test if a directory listing should be rendered. If so, delegate to the dirHandler.
            Can't use the sendFile handler and must use the netConnector.
         */
        if (info->isDir && maRenderDirListing(conn)) {
            tx->handler = conn->http->dirHandler;
            tx->connector = conn->http->netConnector;
            return HTTP_ROUTE_OK;
        }
    }
    if (!info->valid && (route->flags & HTTP_ROUTE_GZIP) && rx->acceptEncoding && strstr(rx->acceptEncoding, "gzip") != 0) {
        /*
            If the route accepts zipped data and a zipped file exists, then transparently respond with it.
         */
        zipfile = sfmt("%s.gz", tx->filename);
        if (mprGetPathInfo(zipfile, &zipInfo) == 0) {
            tx->filename = zipfile;
            tx->fileInfo = zipInfo;
            httpSetHeader(conn, "Content-Encoding", "gzip");
        }
    }
    if (!(info->valid || info->isDir) && !(conn->rx->flags & HTTP_PUT)) {
        if (rx->referrer) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s from %s", tx->filename, rx->referrer);
        } else {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", tx->filename);
        }
    } else if (info->valid) {
        /*
            The sendFile connector is optimized on some platforms to use the sendfile() system call.
            Set the entity length for the sendFile connector to utilize.
         */
        httpSetEntityLength(conn, tx->fileInfo.size);
        if (!tx->etag) {
            /* Set the etag for caching in the client */
            tx->etag = sfmt("\"%x-%Lx-%Lx\"", info->inode, info->size, info->mtime);
        }
    }
    return HTTP_ROUTE_OK;
}


/*
    Initialize a handler instance for the file handler for this request
 */
static void openFileHandler(HttpQueue *q)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    HttpConn    *conn;
    char        *date;

    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;
    route = rx->route;

    if (rx->flags & (HTTP_GET | HTTP_HEAD | HTTP_POST)) {
        if (tx->fileInfo.valid && tx->fileInfo.mtime) {
            //  TODO - OPT could cache this
            date = httpGetDateString(&tx->fileInfo);
            httpSetHeader(conn, "Last-Modified", date);
        }
        if (httpContentNotModified(conn)) {
            httpSetStatus(conn, HTTP_CODE_NOT_MODIFIED);
            httpOmitBody(conn);
        }
        if (!tx->fileInfo.isReg && !tx->fileInfo.isLink) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't locate document: %s", rx->uri);
            
        } else if (tx->fileInfo.size > conn->limits->transmissionBodySize) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE,
                "Http transmission aborted. File size exceeds max body of %,Ld bytes", conn->limits->transmissionBodySize);
            
        } else if (!(tx->connector == conn->http->sendConnector)) {
            /*
                If using the net connector, open the file if a body must be sent with the response. The file will be
                automatically closed when the request completes.
             */
            if (!(tx->flags & HTTP_TX_NO_BODY)) {
                tx->file = mprOpenFile(tx->filename, O_RDONLY | O_BINARY, 0);
                if (tx->file == 0) {
                    if (rx->referrer) {
                        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s from %s", tx->filename, rx->referrer);
                    } else {
                        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s from %s", tx->filename);
                    }
                }
            }
        }

    } else if (rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        httpHandleOptionsTrace(q->conn);

    } else if ((rx->flags & (HTTP_PUT | HTTP_DELETE)) && (route->flags & HTTP_ROUTE_PUT_DELETE)) {
        /* No response body is sent for PUT or DELETE */
        httpOmitBody(conn);

    } else {
        httpError(q->conn, HTTP_CODE_BAD_METHOD, "The \"%s\" method is not supported by file handler", rx->method);
    }
}


static void startFileHandler(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpPacket  *packet;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    
    if (tx->flags & HTTP_TX_NO_BODY) {
        if (rx->flags & HTTP_PUT) {
            handlePutRequest(q);
        } else if (rx->flags & HTTP_DELETE) {
            handleDeleteRequest(q);
        }
    } else {
        /* Create a single data packet based on the entity length */
        packet = httpCreateEntityPacket(0, tx->entityLength, readFileData);
        if (!tx->outputRanges) {
            /* Can set a content length */
            tx->length = tx->entityLength;
        }
        /* Add to the output service queue */
        httpPutForService(q, packet, 0);
    }
}


/*
    The ready callback is invoked when all body data has been received
 */
static void readyFileHandler(HttpQueue *q)
{
    /*
        The queue already contains a single data packet representing all the output data. So can be finalized now.
     */
    httpFinalize(q->conn);
}


/*  
    Populate a packet with file data. Return the number of bytes read or a negative error code. Will not return with
    a short read.
 */
static ssize readFileData(HttpQueue *q, HttpPacket *packet, MprOff pos, ssize size)
{
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       nbytes;

    conn = q->conn;
    tx = conn->tx;

    if (packet->content == 0 && (packet->content = mprCreateBuf(size, -1)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAssert(size <= mprGetBufSpace(packet->content));    
    mprLog(7, "readFileData size %d, pos %Ld", size, pos);
    
    if (pos >= 0) {
        mprSeekFile(tx->file, SEEK_SET, pos);
    }
    if ((nbytes = mprReadFile(tx->file, mprGetBufStart(packet->content), size)) != size) {
        /*  
            As we may have sent some data already to the client, the only thing we can do is abort and hope the client 
            notices the short data.
         */
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't read file %s", tx->filename);
        return MPR_ERR_CANT_READ;
    }
    mprAdjustBufEnd(packet->content, nbytes);
    packet->esize -= nbytes;
    mprAssert(packet->esize == 0);
    return nbytes;
}


/*  
    Prepare a data packet for sending downstream. This involves reading file data into a suitably sized packet. Return
    the 1 if the packet was sent entirely, return zero if the packet could not be completely sent. Return a negative
    error code for write errors. This may split the packet if it exceeds the downstreams maximum packet size.
 */
static int prepPacket(HttpQueue *q, HttpPacket *packet)
{
    HttpQueue   *nextQ;
    ssize       size, nbytes;

    nextQ = q->nextQ;
    if (packet->esize > nextQ->packetSize) {
        httpPutBackPacket(q, httpSplitPacket(packet, nextQ->packetSize));
        size = nextQ->packetSize;
    } else {
        size = (ssize) packet->esize;
    }
    if ((size + nextQ->count) > nextQ->max) {
        /*  
            The downstream queue is full, so disable the queue and service downstream queue.
            Will re-enable via a writable event on the connection.
         */
        httpSuspendQueue(q);
        if (!(nextQ->flags & HTTP_QUEUE_SUSPENDED)) {
            httpScheduleQueue(nextQ);
        }
        return 0;
    }
    if ((nbytes = readFileData(q, packet, q->ioPos, size)) != size) {
        return MPR_ERR_CANT_READ;
    }
    q->ioPos += nbytes;
    return 1;
}


/*  
    The service callback will be invoked to service outgoing packets on the service queue. It will only be called 
    once all incoming data has been received and then there after when the downstream queues drain sufficiently to
    absorb more data. This routine may flow control if the downstream stage cannot accept all the file data. It will
    then be re-called as required to send more data.
 */
static void outgoingFileService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpPacket  *packet;
    bool        usingSend;
    int         rc;

    conn = q->conn;
    tx = conn->tx;
    usingSend = (tx->connector == conn->http->sendConnector);

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!usingSend && !tx->outputRanges && packet->flags & HTTP_PACKET_DATA) {
            if ((rc = prepPacket(q, packet)) < 0) {
                return;
            } else if (rc == 0) {
                mprLog(7, "OutgoingFileService downstream full, putback");
                httpPutBackPacket(q, packet);
                return;
            }
            mprLog(7, "OutgoingFileService readData %d", rc);
        }
        httpPutPacketToNext(q, packet);
    }
    mprLog(7, "OutgoingFileService complete");
}


/*
    The incoming callback is invoked to receive body data 
 */
static void incomingFile(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpRx      *rx;
    HttpRange   *range;
    MprBuf      *buf;
    MprFile     *file;
    ssize       len;

    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;
    file = (MprFile*) q->queueData;
    
    if (file == 0) {
        /*  Not a PUT so just ignore the incoming data.  */
        return;
    }
    if (httpGetPacketLength(packet) == 0) {
        /* End of input */
        if (file) {
            mprCloseFile(file);
        }
        q->queueData = 0;
        return;
    }
    buf = packet->content;
    len = mprGetBufLength(buf);
    mprAssert(len > 0);

    range = rx->inputRange;
    if (range && mprSeekFile(file, SEEK_SET, range->start) != range->start) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't seek to range start to %d", range->start);

    } else if (mprWriteFile(file, mprGetBufStart(buf), len) != len) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't PUT to %s", tx->filename);
    }
}


/*  
    This is called to setup for a HTTP PUT request. It is called before receiving the post data via incomingFileData
 */
static void handlePutRequest(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    MprFile     *file;
    char        *path;

    mprAssert(q->pair->queueData == 0);

    conn = q->conn;
    tx = conn->tx;
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);

    path = tx->filename;
    if (tx->outputRanges) {
        /*  
            Open an existing file with fall-back to create
         */
        if ((file = mprOpenFile(path, O_BINARY | O_WRONLY, 0644)) == 0) {
            if ((file = mprOpenFile(path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644)) == 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
                return;
            }
        } else {
            mprSeekFile(file, SEEK_SET, 0);
        }
    } else {
        if ((file = mprOpenFile(path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
            return;
        }
    }
    httpSetStatus(conn, tx->fileInfo.isReg ? HTTP_CODE_NO_CONTENT : HTTP_CODE_CREATED);
    httpSetContentLength(conn, 0);
    q->pair->queueData = (void*) file;
}


static void handleDeleteRequest(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);

    if (!tx->fileInfo.isReg) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "URI not found");
        return;
    }
    if (mprDeletePath(tx->filename) < 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't remove URI");
        return;
    }
    httpSetStatus(conn, HTTP_CODE_NO_CONTENT);
    httpSetContentLength(conn, 0);
}


/*  
    Loadable module initialization
 */
int maOpenFileHandler(Http *http)
{
    HttpStage     *handler;

    /* 
        This handler serves requests without using thread workers.
     */
    if ((handler = httpCreateHandler(http, "fileHandler", 0, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->match = matchFileHandler;
    handler->open = openFileHandler;
    handler->start = startFileHandler;
    handler->ready = readyFileHandler;
    handler->outgoingService = outgoingFileService;
    handler->incoming = incomingFile;
    http->fileHandler = handler;
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

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
