/*
    simpleClient.c - Simple client using the GET method to retrieve a web page.
  
    This sample demonstrates retrieving content using the HTTP GET 
    method via the Client class. This is a multi-threaded application.
  
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/***************************** Includes *******************************/

#include    "appweb.h"

/****************************** Forwards ******************************/

typedef struct App {
    Mpr         *mpr;
    Http        *http;
    HttpConn    *conn;
} App;

static void manageApp(App *app, int flags);

/********************************* Code *******************************/

MAIN(simpleClient, int argc, char **argv, char **envp)
{
    Mpr     *mpr;
    App     *app;
    cchar   *content;
    int     code;

    /* 
       Create the Multithreaded Portable Runtime
     */
    mpr = mprCreate(argc, argv, MPR_USER_EVENTS_THREAD);
    if ((app = mprAllocObj(App, manageApp)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(app);
    mprAddStandardSignals(app);

    /* 
       Start the Multithreaded Portable Runtime
     */
    mprStart();

    /* 
       Create an Http service object
     */
    app->http = httpCreate(mpr);

    /* 
       Get a client http object to work with. We can issue multiple requests with this one object.
     */
    app->conn = httpCreateConn(app->http, NULL, NULL);

    /* 
       Get a URL
     */
    if (httpConnect(app->conn, "GET", "http://www.embedthis.com/index.html", NULL) < 0) {
        mprError("Can't get URL");
        exit(2);
    }

    /* 
       Examine the HTTP response HTTP code. 200 is success.
     */
    code = httpGetStatus(app->conn);
    if (code != 200) {
        mprError("Server responded with code %d\n", code);
        exit(1);
    } 

    /* 
       Get the actual response content
     */
    content = httpReadString(app->conn);
    if (content) {
        mprPrintf("Server responded with: %s\n", content);
    }
    mprDestroy(MPR_EXIT_DEFAULT);
    return 0;
}


/*
    Manage the app instance for the garbage collector
 */
static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app);
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.md distributed with 
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
