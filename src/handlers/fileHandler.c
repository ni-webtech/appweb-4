/*
    fileHandler.c -- Static file content handler

    This handler manages static file based content such as HTML, GIF /or JPEG pages. It supports all methods including:
    GET, PUT, DELETE, OPTIONS and TRACE. It is event based and does not use worker threads.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

static void handleDeleteRequest(HttpQueue *q);
static int  readFileData(HttpQueue *q, HttpPacket *packet);
static void handlePutRequest(HttpQueue *q);

/*********************************** Code *************************************/
/*
    Initialize a handler instance for the file handler.
 */
static void openFile(HttpQueue *q)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation    *location;
    HttpConn        *conn;
    char            *date;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;
    location = rec->location;

    if (rec->flags & (HTTP_GET | HTTP_HEAD | HTTP_POST)) {
        if (trans->fileInfo.valid && trans->fileInfo.mtime) {
            //  TODO - OPT could cache this
            date = httpGetDateString(rec, &trans->fileInfo);
            httpSetHeader(conn, "Last-Modified", date);
            mprFree(date);
        }
        if (httpContentNotModified(conn)) {
            httpSetStatus(conn, HTTP_CODE_NOT_MODIFIED);
            httpOmitBody(conn);
        } else {
            httpSetEntityLength(conn, (int) trans->fileInfo.size);
        }
        
        if (!trans->fileInfo.isReg && !trans->fileInfo.isLink) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't locate document: %s", rec->uri);
            
        } else if (!(trans->connector == conn->http->sendConnector)) {
            /*
                Open the file if a body must be sent with the response. The file will be automatically closed when 
                the response is freed. Cool eh?
             */
            trans->file = mprOpen(trans, trans->filename, O_RDONLY | O_BINARY, 0);
            if (trans->file == 0) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", trans->filename);
            }
        }

    } else if (rec->flags & (HTTP_PUT | HTTP_DELETE)) {
        if (location->flags & HTTP_LOC_PUT_DELETE) {
            httpOmitBody(conn);
        } else {
            httpError(q->conn, HTTP_CODE_BAD_METHOD, 
                "Method %s not supported by file handler at this location %s", rec->method, location->prefix);
        }
    } else {
        httpError(q->conn, HTTP_CODE_BAD_METHOD, 
            "Method %s not supported by file handler at this location %s", rec->method, location->prefix);
    }
}


static void startFile(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpPacket      *packet;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    
    if (trans->flags & HTTP_TRANS_NO_BODY) {
        if (rec->flags & HTTP_PUT) {
            handlePutRequest(q);
        } else if (rec->flags & HTTP_DELETE) {
            handleDeleteRequest(q);
        }
    } else {
        /* Create a single data packet based on the entity length.  */
        packet = httpCreateDataPacket(q, 0);
        packet->entityLength = trans->entityLength;
        if (!rec->ranges) {
            trans->length = trans->entityLength;
        }
        httpPutForService(q, packet, 0);
    }
}


static void processFile(HttpQueue *q)
{
    httpFinalize(q->conn);
}


/*  
    The service routine will be called when all incoming data has been received. This routine may flow control if the 
    downstream stage cannot accept all the file data. It will then be re-called as required to send more data.
 */
static void outgoingFileService(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpPacket      *packet;
    bool            usingSend;
    int             len;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;

    mprLog(q, 7, "OutgoingFileService");

    usingSend = trans->connector == conn->http->sendConnector;

    if (rec->ranges) {
        mprAssert(conn->http->rangeService);
        (*conn->http->rangeService)(q, (usingSend) ? NULL : readFileData);
    
    } else {
        for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
            if (!usingSend && packet->flags & HTTP_PACKET_DATA) {
                if (!httpWillNextQueueAcceptPacket(q, packet)) {
                    mprLog(q, 7, "OutgoingFileService downstream full, putback");
                    httpPutBackPacket(q, packet);
                    return;
                }
                if ((len = readFileData(q, packet)) < 0) {
                    return;
                }
                mprLog(q, 7, "OutgoingFileService readData %d", len);
                trans->pos += len;
            }
            httpSendPacketToNext(q, packet);
        }
    }
    mprLog(q, 7, "OutgoingFileService complete");
}


static void incomingFileData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;
    HttpRange       *range;
    MprBuf          *buf;
    MprFile         *file;
    int             len;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;
    file = (MprFile*) q->queueData;
    
    if (httpGetPacketLength(packet) == 0) {
        /*
            End of input
         */
        mprFree(file);
        q->queueData = 0;
        httpFreePacket(q, packet);
        return;
    }
    if (file == 0) {
        /*  
            Not a PUT so just ignore the incoming data.
         */
        httpFreePacket(q, packet);
        return;
    }
    buf = packet->content;
    len = mprGetBufLength(buf);
    mprAssert(len > 0);

    range = rec->inputRange;
    if (range && ((MprOffset) mprSeek(file, SEEK_SET, (long) range->start)) != range->start) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't seek to range start to %d", range->start);
    } else {
        if (mprWrite(file, mprGetBufStart(buf), len) != len) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't PUT to %s", trans->filename);
        }
    }
    httpFreePacket(q, packet);
}


/*  
    Populate a packet with file data
 */
static int readFileData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;
    int             len, rc;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;

    if (packet->content == 0) {
        len = packet->entityLength;
        if ((packet->content = mprCreateBuf(packet, len, len)) == 0) {
            return MPR_ERR_NO_MEMORY;
        }
    } else {
        len = mprGetBufSpace(packet->content);
    }
    mprAssert(len <= mprGetBufSpace(packet->content));    
    mprLog(q, 7, "readFileData len %d, pos %d", len, trans->pos);
    
    if (rec->ranges) {
        /*  
            rangeService will have set trans->pos to the next read position already
         */
        mprSeek(trans->file, SEEK_SET, trans->pos);
    }

    if ((rc = mprRead(trans->file, mprGetBufStart(packet->content), len)) != len) {
        /*  
            As we may have sent some data already to the client, the only thing we can do is abort and hope the client 
            notices the short data.
         */
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't read file %s", trans->filename);
        return MPR_ERR_CANT_READ;
    }

    mprAdjustBufEnd(packet->content, len);
    return len;
}


/*  
    This is called to setup for a HTTP PUT request. It is called before receiving the post data via incomingFileData
 */
static void handlePutRequest(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprFile         *file;
    char            *path;

    mprAssert(q->pair->queueData == 0);

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    path = trans->filename;
    if (path == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't map URI to file storage");
        return;
    }
    if (rec->ranges) {
        /*  
            Open an existing file with fall-back to create
         */
        file = mprOpen(q, path, O_BINARY | O_WRONLY, 0644);
        if (file == 0) {
            file = mprOpen(q, path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644);
            if (file == 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
                return;
            }
        } else {
            mprSeek(file, SEEK_SET, 0);
        }
    } else {
        file = mprOpen(q, path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644);
        if (file == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
            return;
        }
    }
    httpSetStatus(conn, trans->fileInfo.isReg ? HTTP_CODE_NO_CONTENT : HTTP_CODE_CREATED);
    q->pair->queueData = (void*) file;
}


/*  
    Immediate processing of delete requests
 */
static void handleDeleteRequest(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    char            *path;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    path = trans->filename;
    if (path == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't map URI to file storage");
        return;
    }
    if (!conn->transmitter->fileInfo.isReg) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "URI not found");
        return;
    }
    if (mprDeletePath(q, path) < 0) {
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
    handler = httpCreateHandler(http, "fileHandler", 
        HTTP_STAGE_GET | HTTP_STAGE_HEAD | HTTP_STAGE_POST | HTTP_STAGE_PUT | HTTP_STAGE_DELETE | HTTP_STAGE_VERIFY_ENTITY);
    if (handler == 0) {
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
