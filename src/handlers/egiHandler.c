/*
    egiHandler.c -- Embedded Gateway Interface (EGI) handler. Fast in-process replacement for CGI.

    The EGI handler implements a very fast in-process CGI scheme.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/************************************* Local **********************************/

typedef struct MaEgi {
    MprHashTable    *forms;
} MaEgi;

/************************************* Code ***********************************/

static bool matchEgi(HttpConn *conn, HttpStage *handler)
{
    HttpRx    *rx;

    /*  
        Rewrite the entire URL as the script name 
     */
    rx = conn->rx;
    rx->scriptName = rx->pathInfo;
    rx->pathInfo = "";
    return 1;
}


/*
    The process method will run after receiving upload or form data. Otherwise it runs before receiving all the data.
 */
static void runEgi(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MaEgiForm   *form;
    MaEgi       *egi;

    conn = q->conn;
    rx = conn->rx;
    egi = (MaEgi*) q->stage->stageData;
    
    form = (MaEgiForm*) mprLookupHash(egi->forms, rx->scriptName);
    if (form == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Egi Form: \"%s\" is not defined", rx->scriptName);
    } else {
        (*form)(q);
    }
}


static void startEgi(HttpQueue *q)
{
    if (!q->conn->rx->form) {
        runEgi(q);
    }
}


static void processEgi(HttpQueue *q)
{
    if (q->conn->rx->form) {
        runEgi(q);
    }
}


/*
    Define an EGI form
 */
int maDefineEgiForm(Http *http, cchar *name, MaEgiForm *form)
{
    HttpStage   *handler;
    MaEgi       *egi;

    handler = httpLookupStage(http, "egiHandler");
    if (handler) {
        egi = (MaEgi*) handler->stageData;
        mprAddKey(egi->forms, name, form);
    }
    return 0;
}


static void manageEgi(MaEgi *egi, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(egi->forms);
    } else if (flags & MPR_MANAGE_FREE) {
    }
}


int maOpenEgiHandler(Http *http)
{
    HttpStage       *handler;
    MaEgi           *egi;

    handler = httpCreateHandler(http, "egiHandler", 
        HTTP_STAGE_GET | HTTP_STAGE_HEAD | HTTP_STAGE_POST | HTTP_STAGE_PUT | HTTP_STAGE_VARS | HTTP_STAGE_ENV_VARS | 
        HTTP_STAGE_VIRTUAL | HTTP_STAGE_THREAD);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if ((handler->stageData = mprAllocObj(MaEgi, manageEgi)) == 0) {
        return MPR_ERR_MEMORY;
    }
    egi = handler->stageData;
    handler->match = matchEgi; 
    handler->start = startEgi; 
    handler->process = processEgi; 
    egi->forms = mprCreateHash(HTTP_LARGE_HASH_SIZE, MPR_HASH_STATIC_VALUES);
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
