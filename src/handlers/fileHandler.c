/*
    fileHandler.c -- Static file content handler

    This handler manages static file based content such as HTML, GIF /or JPEG pages. It supports all methods including:
    GET, PUT, DELETE, OPTIONS and TRACE. It is event based and does not use worker threads.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

//  MOB -- more unique prefixes for combo dist
static void handleDeleteRequest(HttpQueue *q);
static void handlePutRequest(HttpQueue *q);
static ssize readFileData(HttpQueue *q, HttpPacket *packet, MprOff pos, ssize size);

/*********************************** Code *************************************/
/*
    Initialize a handler instance for the file handler.
 */
static void openFile(HttpQueue *q)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLoc     *loc;
    HttpConn    *conn;
    char        *date;

    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;
    loc = rx->loc;

    mprLog(5, "Open file handler");
    if (rx->flags & (HTTP_GET | HTTP_HEAD | HTTP_POST)) {
        if (tx->fileInfo.valid && tx->fileInfo.mtime) {
            //  TODO - OPT could cache this
            date = httpGetDateString(&tx->fileInfo);
            httpSetHeader(conn, "Last-Modified", date);
        }
        if (httpContentNotModified(conn)) {
            httpSetStatus(conn, HTTP_CODE_NOT_MODIFIED);
            httpOmitBody(conn);
        } else {
            httpSetEntityLength(conn, tx->fileInfo.size);
        }
        if (!tx->fileInfo.isReg && !tx->fileInfo.isLink) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't locate document: %s", rx->uri);
            
        } else if (tx->fileInfo.size > conn->limits->transmissionBodySize) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE,
                "Http transmission aborted. File size exceeds max body of %d bytes", conn->limits->transmissionBodySize);
            
        } else if (!(tx->connector == conn->http->sendConnector)) {
            /*
                Open the file if a body must be sent with the response. The file will be automatically closed when 
                the response is freed. Cool eh?
             */
            tx->file = mprOpenFile(tx->filename, O_RDONLY | O_BINARY, 0);
            if (tx->file == 0) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", tx->filename);
            }
        }

    } else if ((rx->flags & (HTTP_PUT | HTTP_DELETE)) && (loc->flags & HTTP_LOC_PUT_DELETE)) {
        httpOmitBody(conn);
    } else {
        httpError(q->conn, HTTP_CODE_BAD_METHOD, "Method \"%s\" is not supported by file handler", rx->method);
    }
}


static void startFile(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpPacket  *packet;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    
    mprLog(5, "startFile");
    if (tx->flags & HTTP_TX_NO_BODY) {
        if (rx->flags & HTTP_PUT) {
            handlePutRequest(q);
        } else if (rx->flags & HTTP_DELETE) {
            handleDeleteRequest(q);
        }
    } else {
        /* Create a single data packet based on the entity length.  */
        packet = httpCreateEntityPacket(0, tx->entityLength, readFileData);
        if (!rx->ranges) {
            /* Can set a content length */
            tx->length = tx->entityLength;
        }
        httpPutForService(q, packet, 0);
    }
}


static void processFile(HttpQueue *q)
{
    /*
        The queue already contains a single data packet representing all the output data.
        So can be finalized now.
     */
    httpFinalize(q->conn);
}


/*  
    Populate a packet with file data
 */
static ssize readFileData(HttpQueue *q, HttpPacket *packet, MprOff pos, ssize size)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpRx      *rx;
    ssize       nbytes;

    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;

    if (packet->content == 0 && (packet->content = mprCreateBuf(size, size)) == 0) {
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
    Prepare a data packet. This involves reading file data into a suitably sized packet. Return the number of bytes read.
    This may split the packet if it exceeds the downstreams maximum packet size.
 */
static ssize prepPacket(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpQueue   *nextQ;
    ssize       size, nbytes;

    conn = q->conn;
    tx = conn->tx;
    nextQ = q->nextQ;

    if (packet->esize > nextQ->packetSize) {
        httpPutBackPacket(q, httpSplitPacket(packet, nextQ->packetSize));
        size = nextQ->packetSize;
    } else {
        size = (ssize) packet->esize;
    }
    if ((size + nextQ->count) > nextQ->max) {
        /*  
            The downstream queue is full, so disable the queue and mark the downstream queue as full and service.
            Will re-enable via a writable event on the connection.
         */
        nextQ->flags |= HTTP_QUEUE_FULL;
        httpDisableQueue(q);
        if (!(nextQ->flags & HTTP_QUEUE_DISABLED)) {
            httpScheduleQueue(nextQ);
        }
        return 0;
    }
    nbytes = readFileData(q, packet, q->ioPos, size);
    if (nbytes > 0) {
        q->ioPos += nbytes;
    }
    return nbytes;
}


/*  
    The service routine will be called to service outgoing packets on the service queue. It will only be called 
    once all incoming data has been received. This routine may flow control if the downstream stage cannot accept 
    all the file data. It will then be re-called as required to send more data.
 */
static void outgoingFileService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpPacket  *packet;
    bool        usingSend;
    MprOff      len;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    usingSend = tx->connector == conn->http->sendConnector;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!usingSend && !rx->ranges && packet->flags & HTTP_PACKET_DATA) {
            if ((len = prepPacket(q, packet)) <= 0) {
                if (len < 0) {
                    return;
                }
                mprLog(7, "OutgoingFileService downstream full, putback");
                httpPutBackPacket(q, packet);
                return;
            }
            mprLog(7, "OutgoingFileService readData %d", len);
        }
        httpSendPacketToNext(q, packet);
    }
    mprLog(7, "OutgoingFileService complete");
}


static void incomingFileData(HttpQueue *q, HttpPacket *packet)
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
    } else {
        if (mprWriteFile(file, mprGetBufStart(buf), len) != len) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't PUT to %s", tx->filename);
        }
    }
}


/*  
    This is called to setup for a HTTP PUT request. It is called before receiving the post data via incomingFileData
 */
static void handlePutRequest(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpTx      *tx;
    MprFile     *file;
    char        *path;

    mprAssert(q->pair->queueData == 0);

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);

    path = tx->filename;
    if (rx->ranges) {
        /*  
            Open an existing file with fall-back to create
         */
        file = mprOpenFile(path, O_BINARY | O_WRONLY, 0644);
        if (file == 0) {
            file = mprOpenFile(path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644);
            if (file == 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
                return;
            }
        } else {
            mprSeekFile(file, SEEK_SET, 0);
        }
    } else {
        file = mprOpenFile(path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644);
        if (file == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
            return;
        }
    }
    httpSetStatus(conn, tx->fileInfo.isReg ? HTTP_CODE_NO_CONTENT : HTTP_CODE_CREATED);
    q->pair->queueData = (void*) file;
}


/*  
    Immediate processing of delete requests
 */
static void handleDeleteRequest(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    HttpTx      *tx;

    conn = q->conn;
    rx = conn->rx;
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
}


/*  
    Dynamic module initialization
 */
int maOpenFileHandler(Http *http)
{
    HttpStage     *handler;

    /* 
        This handler serves requests without using thread workers.
     */
    if ((handler = httpCreateHandler(http, "fileHandler", HTTP_STAGE_VERIFY_ENTITY, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openFile;
    handler->start = startFile;
    handler->process = processFile;
    handler->outgoingService = outgoingFileService;
    handler->incomingData = incomingFileData;
    http->fileHandler = handler;
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
