/*
    dir.c -- Support authorization on a per-directory basis.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/********************************* Forwards ************************************/

static void manageDir(MaDir *dir, int flags);

/************************************ Code *************************************/

static void manageDir(MaDir *dir, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(dir->auth);
        mprMark(dir->host);
        mprMark(dir->indexName);
        mprMark(dir->path);
    }
}


MaDir *maCreateBareDir(MaHost *host, cchar *path)
{
    MaDir   *dir;

    mprAssert(host);
    mprAssert(path);

    if ((dir = mprAllocObj(MaDir, manageDir)) == 0) {
        return 0;
    }
    dir->indexName = sclone("index.html");
    dir->host = host;
    dir->auth = httpCreateAuth(0);
    if (path) {
        dir->path = sclone(path);
        dir->pathLen = strlen(path);
    }
    return dir;
}


MaDir *maCreateDir(MaHost *host, cchar *path, MaDir *parent)
{
    MaDir   *dir;

    mprAssert(host);
    mprAssert(path);
    mprAssert(parent);

    if ((dir = mprAllocObj(MaDir, manageDir)) == 0) {
        return 0;
    }
    dir->host = host;
    dir->indexName = sclone(parent->indexName);

    if (path == 0) {
        path = parent->path;
    }
    maSetDirPath(dir, path);
    dir->auth = httpCreateAuth(parent->auth);
    return dir;
}


void maSetDirPath(MaDir *dir, cchar *fileName)
{
    mprAssert(dir);
    mprAssert(fileName);

    dir->path = mprGetAbsPath(fileName);
    dir->pathLen = strlen(dir->path);

    /*  
        Always strip trailing "/"
     */
    if (dir->pathLen > 0 && dir->path[dir->pathLen - 1] == '/') {
        dir->path[--dir->pathLen] = '\0';
    }
}


void maSetDirIndex(MaDir *dir, cchar *name) 
{ 
    mprAssert(dir);
    mprAssert(name && *name);

    dir->indexName = sclone(name); 
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
