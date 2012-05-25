/*
    dirHandler.c - Directory listing handler

    The dirHandler is unusual in that is is called (only) from the fileHandler.
    The fileHandler tests if the request is for a directory and then examines if redirection
    to an index, or rendering a directory listing is required. If a listing, the request is
    relayed here.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/********************************** Defines ***********************************/
/*
    Handler configuration
 */
typedef struct Dir {
#if FUTURE
    MprList         *dirList;
    cchar           *defaultIcon;
    MprList         *extList;
    MprList         *ignoreList;
#endif
    bool            enabled;
    int             fancyIndexing;
    bool            foldersFirst;
    cchar           *pattern;
    char            *sortField;
    int             sortOrder;              /* 1 == ascending, -1 descending */
} Dir;

#define DIR_NAME "dirHandler"

/****************************** Forward Declarations **************************/

static Dir *allocDir(HttpRoute *route);
static Dir *cloneDir(Dir *parent, HttpRoute *route);
static void filterDirList(HttpConn *conn, MprList *list);
static Dir* getDirObj(MaState *state);
static int  matchDirPattern(cchar *pattern, cchar *file);
static void outputFooter(HttpQueue *q);
static void outputHeader(HttpQueue *q, cchar *dir, int nameSize);
static void outputLine(HttpQueue *q, MprDirEntry *ep, cchar *dir, int nameSize);
static void parseQuery(HttpConn *conn);
static void sortList(HttpConn *conn, MprList *list);

/************************************* Code ***********************************/
/*
    Test if this request is for a directory listing. This routine is called directly by the fileHandler.
    Directory listings are enabled in a route via "Options Indexes".
 */
bool maRenderDirListing(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    Dir         *dir;

    tx = conn->tx;
    rx = conn->rx;
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);

    if ((dir = httpGetRouteData(rx->route, DIR_NAME)) == 0) {
        return 0;
    }
    if (dir->enabled && tx->fileInfo.isDir && sends(rx->pathInfo, "/")) {
        conn->data = dir;
        return 1;
    }
    return 0;
}


/*
    Start the request (and complete it)
 */
static void startDir(HttpQueue *q)
{
    HttpConn        *conn;
    HttpTx          *tx;
    HttpRx          *rx;
    MprList         *list;
    MprDirEntry     *dp;
    Dir             *dir;
    uint            nameSize;
    int             next;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    dir = conn->data;
    mprAssert(tx->filename);

    httpSetHeaderString(conn, "Cache-Control", "no-cache");
    httpSetHeaderString(conn, "Last-Modified", conn->http->currentDate);
    parseQuery(conn);

    if ((list = mprGetPathFiles(tx->filename, MPR_PATH_RELATIVE)) == 0) {
        httpWrite(q, "<h2>Can't get file list</h2>\r\n");
        outputFooter(q);
        return;
    }
    if (dir->pattern) {
        filterDirList(conn, list);
    }
    sortList(conn, list);
    /*
        Get max filename size
     */
    nameSize = 0;
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        nameSize = max((int) strlen(dp->name), nameSize);
    }
    nameSize = max(nameSize, 22);

    outputHeader(q, rx->pathInfo, nameSize);
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        outputLine(q, dp, tx->filename, nameSize);
    }
    outputFooter(q);
    httpFinalize(conn);
}
 

static void parseQuery(HttpConn *conn)
{
    HttpRx      *rx;
    Dir         *dir;
    char        *value, *query, *next, *tok;

    rx = conn->rx;
    dir = conn->data;
    
    query = sclone(rx->parsedUri->query);
    if (query == 0) {
        return;
    }
    tok = stok(query, ";&", &next);
    while (tok) {
        if ((value = strchr(tok, '=')) != 0) {
            *value++ = '\0';
            if (*tok == 'C') {                  /* Sort column */
                if (*value == 'N') {
                    dir->sortField = "Name";
                } else if (*value == 'M') {
                    dir->sortField = "Date";
                } else if (*value == 'S') {
                    dir->sortField = "Size";
                }
                dir->sortField = sclone(dir->sortField);

            } else if (*tok == 'O') {           /* Sort order */
                if (*value == 'A') {
                    dir->sortOrder = 1;
                } else if (*value == 'D') {
                    dir->sortOrder = -1;
                }

            } else if (*tok == 'F') {           /* Format */ 
                if (*value == '0') {
                    dir->fancyIndexing = 0;
                } else if (*value == '1') {
                    dir->fancyIndexing = 1;
                } else if (*value == '2') {
                    dir->fancyIndexing = 2;
                }

            } else if (*tok == 'P') {           /* Pattern */ 
                dir->pattern = sclone(value);
            }
        }
        tok = stok(next, ";&", &next);
    }
}


static void sortList(HttpConn *conn, MprList *list)
{
    MprDirEntry *tmp, **items;
    Dir         *dir;
    int         count, i, j, rc;

    dir = conn->data;
    
    if (dir->sortField == 0) {
        return;
    }
    count = mprGetListLength(list);
    items = (MprDirEntry**) list->items;
    if (scasematch(dir->sortField, "Name")) {
        for (i = 1; i < count; i++) {
            for (j = 0; j < i; j++) {
                rc = strcmp(items[i]->name, items[j]->name);
                if (dir->foldersFirst) {
                    if (items[i]->isDir && !items[j]->isDir) {
                        rc = -dir->sortOrder;
                    } else if (items[j]->isDir && !items[i]->isDir) {
                        rc = dir->sortOrder;
                    } 
                }
                rc *= dir->sortOrder;
                if (rc < 0) {
                    tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }

    } else if (scasematch(dir->sortField, "Size")) {
        for (i = 1; i < count; i++) {
            for (j = 0; j < i; j++) {
                rc = (items[i]->size < items[j]->size) ? -1 : 1;
                if (dir->foldersFirst) {
                    if (items[i]->isDir && !items[j]->isDir) {
                        rc = -dir->sortOrder;
                    } else if (items[j]->isDir && !items[i]->isDir) {
                        rc = dir->sortOrder;
                    }
                }
                rc *= dir->sortOrder;
                if (rc < 0) {
                    tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }

    } else if (scasematch(dir->sortField, "Date")) {
        for (i = 1; i < count; i++) {
            for (j = 0; j < i; j++) {
                rc = (items[i]->lastModified < items[j]->lastModified) ? -1: 1;
                if (dir->foldersFirst) {
                    if (items[i]->isDir && !items[j]->isDir) {
                        rc = -dir->sortOrder;
                    } else if (items[j]->isDir && !items[i]->isDir) {
                        rc = dir->sortOrder;
                    }
                }
                rc *= dir->sortOrder;
                if (rc < 0) {
                    tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }
    }
}


static void outputHeader(HttpQueue *q, cchar *path, int nameSize)
{
    Dir     *dir;
    char    *parent, *parentSuffix;
    int     reverseOrder, fancy, isRootDir;

    dir = q->conn->data;
    fancy = 1;

    httpWrite(q, "<!DOCTYPE HTML PUBLIC \"-/*W3C//DTD HTML 3.2 Final//EN\">\r\n");
    httpWrite(q, "<html>\r\n <head>\r\n  <title>Index of %s</title>\r\n", path);
    httpWrite(q, " </head>\r\n");
    httpWrite(q, "<body>\r\n");

    httpWrite(q, "<h1>Index of %s</h1>\r\n", path);

    if (dir->sortOrder > 0) {
        reverseOrder = 'D';
    } else {
        reverseOrder = 'A';
    }
    if (dir->fancyIndexing == 0) {
        fancy = '0';
    } else if (dir->fancyIndexing == 1) {
        fancy = '1';
    } else if (dir->fancyIndexing == 2) {
        fancy = '2';
    }
    parent = mprGetPathDir(path);
    if (parent[strlen(parent) - 1] != '/') {
        parentSuffix = "/";
    } else {
        parentSuffix = "";
    }
    isRootDir = (strcmp(path, "/") == 0);

    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<table><tr><th><img src=\"/icons/blank.gif\" alt=\"[ICO]\" /></th>");

        httpWrite(q, "<th><a href=\"?C=N;O=%c;F=%c\">Name</a></th>", reverseOrder, fancy);
        httpWrite(q, "<th><a href=\"?C=M;O=%c;F=%c\">Last modified</a></th>", reverseOrder, fancy);
        httpWrite(q, "<th><a href=\"?C=S;O=%c;F=%c\">Size</a></th>", reverseOrder, fancy);
        httpWrite(q, "<th><a href=\"?C=D;O=%c;F=%c\">Description</a></th>\r\n", reverseOrder, fancy);

        httpWrite(q, "</tr><tr><th colspan=\"5\"><hr /></th></tr>\r\n");

        if (! isRootDir) {
            httpWrite(q, "<tr><td valign=\"top\"><img src=\"/icons/back.gif\"");
            httpWrite(q, "alt=\"[DIR]\" /></td><td><a href=\"%s%s\">", parent, parentSuffix);
            httpWrite(q, "Parent Directory</a></td>");
            httpWrite(q, "<td align=\"right\">  - </td></tr>\r\n");
        }

    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<pre><img src=\"/icons/space.gif\" alt=\"Icon\" /> ");

        httpWrite(q, "<a href=\"?C=N;O=%c;F=%c\">Name</a>%*s", reverseOrder, fancy, nameSize - 3, " ");
        httpWrite(q, "<a href=\"?C=M;O=%c;F=%c\">Last modified</a>       ", reverseOrder, fancy);
        httpWrite(q, "<a href=\"?C=S;O=%c;F=%c\">Size</a>               ", reverseOrder, fancy);
        httpWrite(q, "<a href=\"?C=D;O=%c;F=%c\">Description</a>\r\n", reverseOrder, fancy);

        httpWrite(q, "<hr />");

        if (! isRootDir) {
            httpWrite(q, "<img src=\"/icons/parent.gif\" alt=\"[DIR]\" />");
            httpWrite(q, " <a href=\"%s%s\">Parent Directory</a>\r\n", parent, parentSuffix);
        }

    } else {
        httpWrite(q, "<ul>\n");
        if (! isRootDir) {
            httpWrite(q, "<li><a href=\"%s%s\"> Parent Directory</a></li>\r\n", parent, parentSuffix);
        }
    }
}


static void fmtNum(char *buf, int bufsize, int num, int divisor, char *suffix)
{
    int     whole, point;

    whole = num / divisor;
    point = (num % divisor) / (divisor / 10);

    if (point == 0) {
        mprSprintf(buf, bufsize, "%6d%s", whole, suffix);
    } else {
        mprSprintf(buf, bufsize, "%4d.%d%s", whole, point, suffix);
    }
}


static void outputLine(HttpQueue *q, MprDirEntry *ep, cchar *path, int nameSize)
{
    MprPath     info;
    MprTime     when;
    Dir         *dir;
    char        *newPath, sizeBuf[16], timeBuf[48], *icon;
    struct tm   tm;
    bool        isDir;
    int         len;
    cchar       *ext, *mimeType;
    char        *dirSuffix;
    char        *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    dir = q->conn->data;
    if (ep->size >= (1024 * 1024 * 1024)) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024 * 1024 * 1024, "G");

    } else if (ep->size >= (1024 * 1024)) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024 * 1024, "M");

    } else if (ep->size >= 1024) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024, "K");

    } else {
        mprSprintf(sizeBuf, sizeof(sizeBuf), "%6d", (int) ep->size);
    }
    newPath = mprJoinPath(path, ep->name);

    if (mprGetPathInfo(newPath, &info) < 0) {
        when = mprGetTime();
        isDir = 0;
    } else {
        isDir = info.isDir ? 1 : 0;
        when = (MprTime) info.mtime * MPR_TICKS_PER_SEC;
    }
    if (isDir) {
        icon = "folder";
        dirSuffix = "/";
    } else {
        ext = mprGetPathExt(ep->name);
        if (ext && (mimeType = mprLookupMime(q->conn->rx->route->mimeTypes, ext)) != 0) {
            if (strcmp(ext, "es") == 0 || strcmp(ext, "ejs") == 0 || strcmp(ext, "php") == 0) {
                icon = "text";
            } else if (strstr(mimeType, "text") != 0) {
                icon = "text";
            } else {
                icon = "compressed";
            }
        } else {
            icon = "compressed";
        }
        dirSuffix = "";
    }
    mprDecodeLocalTime(&tm, when);

    mprSprintf(timeBuf, sizeof(timeBuf), "%02d-%3s-%4d %02d:%02d",
        tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900, tm.tm_hour,  tm.tm_min);
    len = (int) strlen(ep->name) + (int) strlen(dirSuffix);

    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<tr><td valign=\"top\">");
        httpWrite(q, "<img src=\"/icons/%s.gif\" alt=\"[   ]\", /></td>", icon);
        httpWrite(q, "<td><a href=\"%s%s\">%s%s</a></td>", ep->name, dirSuffix, ep->name, dirSuffix);
        httpWrite(q, "<td>%s</td><td>%s</td></tr>\r\n", timeBuf, sizeBuf);

    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<img src=\"/icons/%s.gif\" alt=\"[   ]\", /> ", icon);
        httpWrite(q, "<a href=\"%s%s\">%s%s</a>%-*s %17s %4s\r\n", ep->name, dirSuffix, ep->name, dirSuffix, 
            nameSize - len, "", timeBuf, sizeBuf);

    } else {
        httpWrite(q, "<li><a href=\"%s%s\"> %s%s</a></li>\r\n", ep->name, dirSuffix, ep->name, dirSuffix);
    }
}


static void outputFooter(HttpQueue *q)
{
    HttpConn    *conn;
    MprSocket   *sock;
    Dir         *dir;
    
    conn = q->conn;
    dir = conn->data;
    
    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<tr><th colspan=\"5\"><hr /></th></tr>\r\n</table>\r\n");
        
    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<hr /></pre>\r\n");
    } else {
        httpWrite(q, "</ul>\r\n");
    }
    sock = conn->sock->listenSock;
    httpWrite(q, "<address>%s %s at %s Port %d</address>\r\n", BIT_NAME, BIT_VERSION, sock->ip, sock->port);
    httpWrite(q, "</body></html>\r\n");
}


static void filterDirList(HttpConn *conn, MprList *list)
{
    Dir             *dir;
    MprDirEntry     *dp;
    int             next;

    dir = conn->data;
    
    /*
        Do pattern matching. Entries that don't match, free the name to mark
     */
    for (ITERATE_ITEMS(list, dp, next)) {
        if (!matchDirPattern(dir->pattern, dp->name)) {
            mprRemoveItem(list, dp);
            next--;
        }
    }
}


/*
    Return true if the file matches the pattern. Supports '?' and '*'
 */
static int matchDirPattern(cchar *pattern, cchar *file)
{
    cchar   *pp, *fp;

    if (pattern == 0 || *pattern == '\0') {
        return 1;
    }
    if (file == 0 || *file == '\0') {
        return 0;
    }
    for (pp = pattern, fp = file; *pp; ) {
        if (*fp == '\0') {
            if (*pp == '*' && pp[1] == '\0') {
                /* Trailing wild card */
                return 1;
            }
            return 0;
        }
        if (*pp == '*') {
            if (matchDirPattern(&pp[1], &fp[0])) {
                return 1;
            }
            fp++;
            continue;

        } else if (*pp == '?' || *pp == *fp) {
            fp++;

        } else {
            return 0;
        }
        pp++;
    }
    if (*fp == '\0') {
        /* Match */
        return 1;
    }
    return 0;
}


#if FUTURE
static int addIconDirective(MaState *state, cchar *key, cchar *value)
{
    if (!maTokenize(state, value, "%S %W", &path, &dir->extList)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int defaultIconDirective(MaState *state, cchar *key, cchar *value)
{
    state->dir->defaultIcon = sclone(value);
    return 0;
}

/*  
    IndexIgnore pat ... 
 */
static int indexIgnoreDirective(MaState *state, cchar *key, cchar *value)
{
    if (!maTokenize(state, value, "%W", &dir->ignoreList)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}
#endif


/*  
    IndexOrder ascending|descending name|date|size 
 */
static int indexOrderDirective(MaState *state, cchar *key, cchar *value)
{
    Dir     *dir;
    char    *option;

    dir = getDirObj(state);

    if (!maTokenize(state, value, "%S %S", &option, &dir->sortField)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    dir->sortField = 0;
    if (scasematch(option, "ascending")) {
        dir->sortOrder = 1;
    } else {
        dir->sortOrder = -1;
    }
    if (dir->sortField) {
        dir->sortField = sclone(dir->sortField);
    }
    return 0;
}


/*  
    IndexOptions FancyIndexing|FoldersFirst ... (set of options) 
 */
static int indexOptionsDirective(MaState *state, cchar *key, cchar *value)
{
    Dir     *dir;
    char    *option, *tok;

    dir = getDirObj(state);
    option = stok(sclone(value), " \t", &tok);
    while (option) {
        if (scasematch(option, "FancyIndexing")) {
            dir->fancyIndexing = 1;
        } else if (scasematch(option, "HTMLTable")) {
            dir->fancyIndexing = 2;
        } else if (scasematch(option, "FoldersFirst")) {
            dir->foldersFirst = 1;
        }
        option = stok(tok, " \t", &tok);
    }
    return 0;
}


/*  
    Options Indexes 
 */
static int optionsDirective(MaState *state, cchar *key, cchar *value)
{
    Dir     *dir;
    char    *option, *tok;

    dir = getDirObj(state);
    option = stok(sclone(value), " \t", &tok);
    while (option) {
        if (scasematch(option, "Indexes")) {
            dir->enabled = 1;
        }
        option = stok(tok, " \t", &tok);
    }
    return 0;
}


static void manageDir(Dir *dir, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
#if FUTURE
        mprMark(dir->dirList);
        mprMark(dir->defaultIcon);
        mprMark(dir->extList);
        mprMark(dir->ignoreList);
#endif
        mprMark(dir->pattern);
        mprMark(dir->sortField);
    }
}


static Dir *getDirObj(MaState *state)
{
    HttpRoute   *route;
    Dir         *dir, *parent;

    route = state->route;
    dir = httpGetRouteData(route, DIR_NAME);
    if (route->parent) {
        /*
            If the parent route has the same route data, then force a clone so the parent route does not get modified
         */ 
        parent = httpGetRouteData(route->parent, DIR_NAME);
        if (dir == parent) {
            dir = 0;
        }
    }
    if (dir == 0) {
        if (route->parent && (parent = httpGetRouteData(route->parent, DIR_NAME)) != 0) {
            dir = cloneDir(parent, route);
        } else {
            dir = allocDir(route);
        }
    }
    mprAssert(dir);
    return dir;
}


static Dir *allocDir(HttpRoute *route)
{
    Dir     *dir;

    if ((dir = mprAllocObj(Dir, manageDir)) == 0) {
        return 0;
    }
    httpSetRouteData(route, DIR_NAME, dir);
    return dir;
}


static Dir *cloneDir(Dir *parent, HttpRoute *route)
{
    Dir     *dir;

    if ((dir = mprAllocObj(Dir, manageDir)) == 0) {
        return 0;
    }
    dir->enabled = parent->enabled;
    dir->fancyIndexing = parent->fancyIndexing;
    dir->foldersFirst = parent->foldersFirst;
    dir->pattern = parent->pattern;
    dir->sortField = parent->sortField;
    dir->sortOrder = parent->sortOrder;
    httpSetRouteData(route, DIR_NAME, dir);
    return dir;
}


/*
    Loadable module initialization
 */
int maOpenDirHandler(Http *http)
{
    HttpStage   *handler;
    MaAppweb    *appweb;
    Dir         *dir;

    if ((handler = httpCreateHandler(http, "dirHandler", HTTP_STAGE_GET | HTTP_STAGE_HEAD, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if ((handler->stageData = dir = mprAllocObj(Dir, manageDir)) == 0) {
        return MPR_ERR_MEMORY;
    }
    handler->start = startDir; 
    http->dirHandler = handler;
    dir->sortOrder = 1;

    /*
        Declare configuration file directives
     */
    appweb = httpGetContext(http);
    maAddDirective(appweb, "IndexOrder", indexOrderDirective);
    maAddDirective(appweb, "indexOptions", indexOptionsDirective);
    maAddDirective(appweb, "Options", optionsDirective);
    return 0;
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
