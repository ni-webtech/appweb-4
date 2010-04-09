/*
    link.c -- Link in static modules and initialize.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

#if BLD_FEATURE_STATIC || (!BLD_APPWEB_PRODUCT && BLD_APPWEB_BUILTIN)
/*********************************** Locals ***********************************/

static MprList *staticModules;

/*********************************** Code *************************************/

static int loadStaticModule(Http *http, cchar *name, int (*callback)(Http *http, MprModule *mp))
{
    MprModule   *mp;
    int         rc;

    mprAssert(staticModules);
    mprAssert(name && *name);

    mp = mprCreateModule(http, name, NULL);
    rc = (callback)(http, mp);
    if (rc == 0) {
        mprAddItem(staticModules, mp);
    }
    return rc;
}


void maLoadStaticModules(MaAppweb *appweb)
{
    Http    *http;

    http = appweb->http;
    staticModules = mprCreateList(http);

#if BLD_FEATURE_CGI
    loadStaticModule(http, "mod_cgi", maCgiHandlerInit);
#endif
#if BLD_FEATURE_EJS
    loadStaticModule(http, "mod_ejs", maEjsHandlerInit);
#endif
#if BLD_FEATURE_SSL
    loadStaticModule(http, "mod_ssl", maSslModuleInit);
#endif
#if BLD_FEATURE_PHP
    loadStaticModule(http, "mod_php", maPhpHandlerInit);
#endif
}


void maUnloadStaticModules(MaAppweb *appweb)
{
    MprModule   *mp;
    int         next;

    for (next = 0; (mp = mprGetNextItem(staticModules, &next)) != 0; ) {
        mprUnloadModule(mp);
    }
}

#else
void maLoadStaticModules(MaAppweb *appweb) {}
void maUnloadStaticModules(MaAppweb *appweb) {}

#endif /* BLD_FEATURE_STATIC */

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
