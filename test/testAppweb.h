/**
    testAppweb.h - Header for the Appweb Unit Test Framework
    
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/******************************************************************************/

#ifndef _h_TEST_APPWEB
#define _h_TEST_APPWEB 1

/********************************** Includes **********************************/

#include "mpr.h"
#include "http.h"

/********************************** Constants *********************************/

extern bool bulkPost(MprTestGroup *gp, char *url, int size, int expectCode);
extern char *getValue(MprTestGroup *gp, char *key);
extern int  httpRequest(MprTestGroup *gp, cchar *method, cchar *uri);
extern char *lookupValue(MprTestGroup *gp, char *key);
extern bool match(MprTestGroup *gp, char *key, char *value);
extern bool matchAnyCase(MprTestGroup *gp, char *key, char *value);
extern bool simpleForm(MprTestGroup *gp, char *uri, char *formBody, int expectCode);
extern bool simpleGet(MprTestGroup *gp, cchar *uri, int expect);
extern bool simplePost(MprTestGroup *gp, char *uri, char *postBody, ssize len, int expectCode);

extern HttpConn *getConn(MprTestGroup *gp);
extern Http *getHttp(MprTestGroup *gp);
extern char *getDefaultHost(MprTestGroup *gp);
extern int getDefaultPort(MprTestGroup *gp);

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

#endif /* _h_TEST_APPWEB */
