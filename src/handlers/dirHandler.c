/*
    dirHandler.c - Directory listing handler.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/********************************** Defines ***********************************/
/*
    Handler configuration
 */
typedef struct Dir {
    cchar           *defaultIcon;
    MprList         *dirList;
    bool            enabled;
    MprList         *extList;
    int             fancyIndexing;
    bool            foldersFirst;
    MprList         *ignoreList;
    cchar           *pattern;
    char            *sortField;
    int             sortOrder;              /* 1 == ascending, -1 descending */
} Dir;

/****************************** Forward Declarations **************************/

static void filterDirList(HttpConn *conn, MprList *list);
static int  match(cchar *pattern, cchar *file);
static void outputFooter(HttpQueue *q);
static void outputHeader(HttpQueue *q, cchar *dir, int nameSize);
static void outputLine(HttpQueue *q, MprDirEntry *ep, cchar *dir, int nameSize);
static void parseQuery(HttpConn *conn);
static void parseWords(MprList *list, cchar *str);
static void sortList(HttpConn *conn, MprList *list);

/************************************* Code ***********************************/
/*
    Match if the url maps to a directory.
 */
static bool matchDir(HttpConn *conn, HttpStage *handler)
{
    HttpTx      *tx;
    Dir         *dir;

    tx = conn->tx;
    mprAssert(tx->filename);
    mprAssert(tx->fileInfo.checked);
    dir = handler->stageData;
    return dir->enabled && tx->fileInfo.isDir;
}


static void processDir(HttpQueue *q)
{
    HttpConn        *conn;
    HttpTx          *tx;
    HttpRx          *rx;
    MprList         *list;
    MprDirEntry     *dp;
    Dir             *dir;
    cchar           *filename;
    uint            nameSize;
    int             next;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    dir = q->stage->stageData;

    filename = tx->filename;
    mprAssert(filename);

    httpDontCache(conn);
    httpSetHeader(conn, "Last-Modified", conn->http->currentDate);
    parseQuery(conn);

    list = mprGetPathFiles(filename, 1);
    if (list == 0) {
        httpWrite(q, "<h2>Can't get file list</h2>\r\n");
        outputFooter(q);
        return;
    }
    if (dir->pattern) {
        filterDirList(conn, list);
    }
    sortList(conn, list);

    /*
        Get max filename
     */
    nameSize = 0;
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        nameSize = max((int) strlen(dp->name), nameSize);
    }
    nameSize = max(nameSize, 22);

    outputHeader(q, rx->pathInfo, nameSize);
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        outputLine(q, dp, filename, nameSize);
    }
    outputFooter(q);
    httpFinalize(conn);
}
 

static void parseQuery(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    Dir         *dir;
    char        *value, *query, *next, *tok;

    rx = conn->rx;
    tx = conn->tx;
    dir = tx->handler->stageData;
    
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
    HttpRx      *rx;
    HttpTx      *tx;
    MprDirEntry *tmp, **items;
    Dir         *dir;
    int         count, i, j, rc;

    rx = conn->rx;
    tx = conn->tx;
    dir = tx->handler->stageData;
    
    if (dir->sortField == 0) {
        return;
    }

    count = mprGetListLength(list);
    items = (MprDirEntry**) list->items;
    if (scasecmp(dir->sortField, "Name") == 0) {
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

    } else if (scasecmp(dir->sortField, "Size") == 0) {
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

    } else if (scasecmp(dir->sortField, "Date") == 0) {
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
    int     order, reverseOrder, fancy, isRootDir;

    dir = q->stage->stageData;
    
    fancy = 1;

    httpWrite(q, "<!DOCTYPE HTML PUBLIC \"-/*W3C//DTD HTML 3.2 Final//EN\">\r\n");
    httpWrite(q, "<html>\r\n <head>\r\n  <title>Index of %s</title>\r\n", path);
    httpWrite(q, " </head>\r\n");
    httpWrite(q, "<body>\r\n");

    httpWrite(q, "<h1>Index of %s</h1>\r\n", path);

    if (dir->sortOrder > 0) {
        order = 'A';
        reverseOrder = 'D';
    } else {
        order = 'D';
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
    Dir         *dir;
    MprTime     when;
    MaHost      *host;
    char        *newPath, sizeBuf[16], timeBuf[48], *icon;
    struct tm   tm;
    bool        isDir;
    int         len;
    cchar       *ext, *mimeType;
    char        *dirSuffix;
    char        *months[] = { 
                    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
                };

    dir = q->stage->stageData;
    if (ep->size >= (1024*1024*1024)) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024 * 1024 * 1024, "G");

    } else if (ep->size >= (1024*1024)) {
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
        host = httpGetConnHost(q->conn);
        ext = mprGetPathExtension(ep->name);
        if ((mimeType = maLookupMimeType(host, ext)) != 0) {
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
    HttpRx      *rx;
    HttpConn    *conn;
    MprSocket   *sock;
    Dir         *dir;
    
    conn = q->conn;
    rx = conn->rx;
    dir = q->stage->stageData;
    
    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<tr><th colspan=\"5\"><hr /></th></tr>\r\n</table>\r\n");
        
    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<hr /></pre>\r\n");
    } else {
        httpWrite(q, "</ul>\r\n");
    }
    sock = conn->sock->listenSock;
    httpWrite(q, "<address>%s %s at %s Port %d</address>\r\n", BLD_NAME, BLD_VERSION, sock->ip, sock->port);
    httpWrite(q, "</body></html>\r\n");
}


static void filterDirList(HttpConn *conn, MprList *list)
{
    Dir             *dir;
    MprDirEntry     *dp;
    int             next;

    dir = conn->tx->handler->stageData;
    
    /*
        Do pattern matching. Entries that don't match, free the name to mark
     */
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        if (! match(dir->pattern, dp->name)) {
            mprRemoveItem(list, dp);
            next--;
        }
    }
}


/*
    Return true if the file matches the pattern. Supports '?' and '*'
 */
static int match(cchar *pattern, cchar *file)
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
            if (match(&pp[1], &fp[0])) {
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


static int parseDir(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpStage   *handler;
    Dir         *dir;
    char        *name, *extensions, *option, *nextTok, *junk;

    handler = httpLookupStage(http, "dirHandler");
    dir = handler->stageData;
    mprAssert(dir);
    
    if (scasecmp(key, "AddIcon") == 0) {
        /*  AddIcon file ext ext ext */
        /*  Not yet supported */
        name = stok(value, " \t", &extensions);
        parseWords(dir->extList, extensions);
        return 1;

    } else if (scasecmp(key, "DefaultIcon") == 0) {
        /*  DefaultIcon file */
        /*  Not yet supported */
        dir->defaultIcon = stok(value, " \t", &junk);
        return 1;

    } else if (scasecmp(key, "IndexOrder") == 0) {
        /*  IndexOrder ascending|descending name|date|size */
        dir->sortField = 0;
        option = stok(value, " \t", &dir->sortField);
        if (scasecmp(option, "ascending") == 0) {
            dir->sortOrder = 1;
        } else {
            dir->sortOrder = -1;
        }
        if (dir->sortField) {
            dir->sortField = sclone(dir->sortField);
        }
        return 1;

    } else if (scasecmp(key, "IndexIgnore") == 0) {
        /*  IndexIgnore pat ... */
        /*  Not yet supported */
        parseWords(dir->ignoreList, value);
        return 1;

    } else if (scasecmp(key, "IndexOptions") == 0) {
        /*  IndexOptions FancyIndexing|FoldersFirst ... (set of options) */
        option = stok(value, " \t", &nextTok);
        while (option) {
            if (scasecmp(option, "FancyIndexing") == 0) {
                dir->fancyIndexing = 1;
            } else if (scasecmp(option, "HTMLTable") == 0) {
                dir->fancyIndexing = 2;
            } else if (scasecmp(option, "FoldersFirst") == 0) {
                dir->foldersFirst = 1;
            }
            option = stok(nextTok, " \t", &nextTok);
        }
        return 1;

    } else if (scasecmp(key, "Options") == 0) {
        /*  Options Indexes */
        option = stok(value, " \t", &nextTok);
        while (option) {
            if (scasecmp(option, "Indexes") == 0) {
                dir->enabled = 1;
            }
            option = stok(nextTok, " \t", &nextTok);
        }
        return 1;
    }
    return 0;
}


static void parseWords(MprList *list, cchar *str)
{
    char    *word, *tok, *strTok;

    mprAssert(str);
    if (str == 0 || *str == '\0') {
        return;
    }
    strTok = sclone(str);
    word = stok(strTok, " \t\r\n", &tok);
    while (word) {
        mprAddItem(list, word);
        word = stok(0, " \t\r\n", &tok);
    }
}


static void manageDir(Dir *dir, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(dir->defaultIcon);
        mprMark(dir->dirList);
        mprMark(dir->extList);
        mprMark(dir->ignoreList);
        mprMark(dir->pattern);
        mprMark(dir->sortField);
        
    } else if (flags & MPR_MANAGE_FREE) {
    }
}


/*
    Dynamic module initialization
 */
int maOpenDirHandler(Http *http)
{
    HttpStage   *handler;
    Dir         *dir;

    if ((handler = httpCreateHandler(http, "dirHandler", HTTP_STAGE_GET | HTTP_STAGE_HEAD, NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if ((handler->stageData = dir = mprAllocObj(Dir, manageDir)) == 0) {
        return MPR_ERR_MEMORY;
    }
    handler->match = matchDir; 
    handler->process = processDir; 
    handler->parse = (HttpParse) parseDir; 
    http->dirHandler = handler;
    dir->sortOrder = 1;
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
