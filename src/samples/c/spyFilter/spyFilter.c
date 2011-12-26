/*
    spyFilter.c - Eavesdrop on input data

    This sample filter examines form data for name/password fields. If the name and password match
    an AUTH variable is defined. Form data is passed onto the handler.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "http.h"

#if BLD_FEATURE_SAMPLES || 1
/*********************************** Code *************************************/

static bool matchSpy(MaConn *conn, MaStage *handler, cchar *url)
{
    return (conn->request->form && strncmp(url, "/", 1) == 0);
}


static void incomingSpyData(MaQueue *q, MaPacket *packet)
{
    MprHashTable    *table;
    cchar       *name, *password;
    
    if (packet->content == 0) {
        /*
            Create form vars for all the input data
         */
        table = mprCreateHash(q, -1);
        maAddVarsFromQueue(table, q);
        name = mprLookupHash(table, "name");
        password = mprLookupHash(table, "password");
        if (name && password && strcmp(name, "admin") == 0 && strcmp(password, "secret") == 0) {
            maSetHeader(q->conn, 0, "AUTH", "authorized");
        }
        mprFree(table);
        if (q->first) {
            maPutNext(q, q->first);
        }
        maPutNext(q, packet);
    } else {
        maJoinForService(q, packet, 0);
    }
}


MprModule *maSpyFilterInit(MaHttp *http, cchar *path)
{
    MprModule   *module;
    MaStage     *filter;

    if ((module = mprCreateModule(http, "spyFilter", BLD_VERSION, NULL, NULL, NULL)) == 0) {
        return 0;
    }
    if ((filter = maCreateFilter(http, "spyFilter", MA_STAGE_ALL)) == 0) {
        mprFree(module);
        return 0;
    }
    filter->match = matchSpy; 
    filter->incomingData = incomingSpyData; 
    return module;
}


#else

MprModule *maSpyFilterInit(MaHttp *http, cchar *path)
{
    return 0;
}
#endif /* BLD_FEATURE_SAMPLES */

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
