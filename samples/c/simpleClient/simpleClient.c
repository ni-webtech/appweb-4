/*
    simpleClient.c - Simple client using the GET method to retrieve a web page.
  
    This sample demonstrates retrieving content using the HTTP GET 
    method via the Client class. This is a multi-threaded application.
  
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/***************************** Includes *******************************/

#include    "appweb.h"

#if BLD_FEATURE_HTTP_CLIENT
/********************************* Code *******************************/

MAIN(simpleClient, int argc, char** argv)
{
    Mpr         *mpr;
    Http        *http;
    HttpConn    *conn;
    cchar       *content;
    int         code, contentLen;

    mpr = mprCreate(argc, argv, NULL);

    /* 
       Start the Multithreaded Portable Runtime
     */
    mprStart(mpr);
    mprStartEventsThread(mpr);

    /* 
       Create an Http service object
     */
    http = httpCreate(mpr);

    /* 
       Get a client http object to work with. We can issue multiple requests with this one object.
     */
    conn = httpCreateConn(http, NULL, NULL);

    /* 
       Get a URL
     */
    if (httpConnect(conn, "GET", "http://www.embedthis.com/index.html") < 0) {
        mprError(mpr, "Can't get URL");
        exit(2);
    }

    /* 
       Examine the HTTP response HTTP code. 200 is success.
     */
    code = httpGetStatus(conn);
    if (code != 200) {
        mprError(mpr, "Server responded with code %d\n", code);
        exit(1);
    } 

    /* 
       Get the actual response content
     */
    content = httpReadString(conn);
    contentLen = httpGetContentLength(conn);
    if (content) {
        mprPrintf(mpr, "Server responded with:\n");
        write(1, (char*) content, contentLen);
    }
    mprFree(mpr);
    return 0;
}
#endif /* BLD_FEATURE_HTTP_CLIENT */

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
    vim: sw=8 ts=8 expandtab

    @end
 */
