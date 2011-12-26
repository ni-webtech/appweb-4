/*
    convenience.c -- High level convenience API
    This module provides simple, high-level APIs for creating servers.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/************************************ Code ************************************/
/*  
    Create a web server described by a config file. 
 */
int maRunWebServer(cchar *configFile)
{
    Mpr         *mpr;
    MaAppweb    *appweb;
    MaServer    *server;
    int         rc;

    rc = MPR_ERR_CANT_CREATE;
    if ((mpr = mprCreate(0, NULL, 0)) == 0) {
        mprError("Can't create the web server runtime");
    } else {
        if (mprStart() < 0) {
            mprError("Can't start the web server runtime");
        } else {
            if ((appweb = maCreateAppweb(mpr)) == 0) {
                mprError("Can't create appweb object");
            } else {
                mprAddRoot(appweb);
                if ((server = maCreateServer(appweb, 0)) == 0) {
                    mprError("Can't create the web server");
                } else {
                    if (maParseConfig(server, configFile, 0) < 0) {
                        mprError("Can't parse the config file %s", configFile);
                    } else {
                        if (maStartServer(server) < 0) {
                            mprError("Can't start the web server");
                        } else {
                            mprServiceEvents(-1, 0);
                            rc = 0;
                        }
                        maStopServer(server);
                    }
                }
                mprRemoveRoot(appweb);
            }
        }
    }
    mprDestroy(MPR_EXIT_DEFAULT);
    return rc;
}


/*
    Run a web server not based on a config file.
 */
int maRunSimpleWebServer(cchar *ip, int port, cchar *home, cchar *documents)
{
    Mpr         *mpr;
    MaServer    *server;
    MaAppweb    *appweb;
    int         rc;

    /*  
        Initialize and start the portable runtime services.
     */
    rc = MPR_ERR_CANT_CREATE;
    if ((mpr = mprCreate(0, NULL, 0)) == 0) {
        mprError("Can't create the web server runtime");
    } else {
        if (mprStart(mpr) < 0) {
            mprError("Can't start the web server runtime");
        } else {
            if ((appweb = maCreateAppweb(mpr)) == 0) {
                mprError("Can't create the web server http services");
            } else {
                mprAddRoot(appweb);
                if ((server = maCreateServer(appweb, 0)) == 0) {
                    mprError("Can't create the web server");
                } else {
                    if (maConfigureServer(server, 0, home, documents, ip, port) < 0) {
                        mprError("Can't create the web server");
                    } else {
                        if (maStartServer(server) < 0) {
                            mprError("Can't start the web server");
                        } else {
                            mprServiceEvents(-1, 0);
                            rc = 0;
                        }
                        maStopServer(server);
                    }
                }
                mprRemoveRoot(appweb);
            }
        }
        mprDestroy(MPR_EXIT_DEFAULT);
    }
    return rc;
}


//  BLOG
/*
    This will restart the default server on a new IP:PORT. It will stop listening on the default endpoint on the default
    server, optionally modify the IP:PORT and resume listening. NOTE: running requests will be unaffected.
    WARNING: this is demonstration code and has no error checking.
 */
void maRestartServer(cchar *ip, int port)
{
    MaAppweb        *appweb;
    MaServer        *server;
    HttpEndpoint    *endpoint;

    appweb = MPR->appwebService;
    server = mprGetFirstItem(appweb->servers);
    lock(appweb->servers);
    endpoint = mprGetFirstItem(server->endpoints);
    httpStopEndpoint(endpoint);

    /* 
        Alternatively, iterate over all endpoints by

        Http *http = MPR->httpService;
        int  next;
        for (ITERATE_ITEMS(http->endpoints, endpoint, next)) {
            ...
        }
     */

    if (port) {
        endpoint->port = port;
    }
    if (ip) {
        endpoint->ip = sclone(ip);
    }
    httpStartEndpoint(endpoint);
    unlock(appweb->servers);
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
