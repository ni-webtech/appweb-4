/*
    simpleHandler.cpp - Create a simple AppWeb request handler
  
    This sample demonstrates creating a request handler to process requests.
  
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
 
/******************************* Includes *****************************/

#include    "appweb.h"

/********************************* Code *******************************/
/*
    Run the handler. This is called when all input data has been received.
 */
static void processSimple(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;

    conn = q->conn;
    rx = conn->rx;
    
    httpSetHeader(conn, 0, "Last-Modified", conn->http->currentDate);

    /*
        Create the empty header packet. This will be filled in by the downstream network connector stage.
     */
    httpPutForService(q, httpCreateHeaderPacket(), 0);

    /*
        Generate some dynamic data. If you generate a lot, this will buffer up to a configured maximum. 
        If that limit is exceeded, the packet will be sent downstream and the response headers will be created.
     */
    httpWrite(q, "Hello World");

    /*
        Send an end of data packet
     */
    httpPutForService(q, httpCreateEndPacket(), 1);
}



static void incomingSimpleData(HttpQueue *q, HttpPacket *packet)
{
    /*
        Do something with the incoming data in packet and then free the packet.
     */
    mprLog(0, "Data in packet is %s", mprGetBufStart(packet->content));
}


/*
    Module load initialization. This is called when the module is first loaded. The module name is "Simple".
 */
int maSimpleHandlerInit(Http *http, MprModule *module)
{
    HttpStage   *stage;

    if ((stage = httpCreateHandler(http, "simpleHandler", 0, module)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->process = processSimple;
    stage->incomingData = incomingSimpleData;
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
