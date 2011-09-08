/*
    testHttp.c - Test URL validation and encoding routines

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "testAppweb.h"

/********************************** Forwards **********************************/

static bool normalize(MprTestGroup *gp, char *uri, char *expectedUri);
static bool okEscapeUri(MprTestGroup *gp, char *uri, char *expectedUri, int map);
static bool okEscapeCmd(MprTestGroup *gp, char *cmd, char *validCmd);
static bool okEscapeHtml(MprTestGroup *gp, char *html, char *expectedHtml);

/*********************************** Code *************************************/

static void setup(MprTestGroup *gp)
{
    if (!simpleGet(gp, "/index.html", 0)) {
        mprError("Can't access web server at http://%s:%d/index.html", getDefaultHost(gp), getDefaultPort(gp));
        exit(5);
    }
}


static void validateUri(MprTestGroup *gp)
{
    assert(normalize(gp, "", ""));
    assert(normalize(gp, "/", "/"));
    assert(normalize(gp, "/index.html", "/index.html"));
    assert(normalize(gp, "/a/index.html", "/a/index.html"));

    assert(normalize(gp, "..", ""));
    assert(normalize(gp, "../", ""));
    assert(normalize(gp, "/..", "/"));
    assert(normalize(gp, "/a/..", "/"));
    assert(normalize(gp, "/a/../", "/"));
    assert(normalize(gp, "/a/../b/..", "/"));
    assert(normalize(gp, "../a/b/..", "a/"));

    assert(normalize(gp, "./", ""));
    assert(normalize(gp, "./.", ""));
    assert(normalize(gp, "././", ""));
    assert(normalize(gp, "/a/./", "/a/"));
    assert(normalize(gp, "/a/./.", "/a/"));
    assert(normalize(gp, "/a/././", "/a/"));
    assert(normalize(gp, "/a/.", "/a/"));

    assert(normalize(gp, "/*a////b/", "/*a/b/"));
    assert(normalize(gp, "/*a/////b/", "/*a/b/"));

    assert(normalize(gp, "\\a\\b\\", "\\a\\b\\"));
}


static void escape(MprTestGroup *gp)
{
    /*  
        URI unsafe chars are:
            0x00-0x1F, 0x7F, 0x80-0xFF, <>'"#%{}|\^~[]
            Space, \t, \r, \n
            Reserved chars with special meaning are:
                ;/?:@=& 
            FUTURE -- should we not allow "'~"
            FUTURE -- shold we not allow ";?" but allow "/?:@=&"
     */
    assert(okEscapeUri(gp, " \t\r\n\x01\x7f\xff?<>\"#%{}|\\^[]?;", 
        "+%09%0D%0A%01%7F%FF%3F%3C%3E%22%23%25%7B%7D%7C%5C%5E%5B%5D%3F%3B", MPR_ENCODE_URI_COMPONENT));
    assert(okEscapeUri(gp, " \t\r\n\x01\x7f\xff?<>\"#%{}|\\^[]?;", 
        "%20%09%0D%0A%01%7F%FF?%3C%3E%22#%25%7B%7D%7C%5C%5E[]?;", MPR_ENCODE_URI));
   
    assert(okEscapeCmd(gp, "&;`'\"|*?~<>^()[]{}$\\\n", 
        "\\&\\;\\`\\\'\\\"\\|\\*\\\?\\~\\<\\>\\^\\(\\)\\[\\]\\{\\}\\$\\\\\\\n"));
    assert(okEscapeHtml(gp, "<>&", "&lt;&gt;&amp;"));
    assert(okEscapeHtml(gp, "#()", "#()"));
}


static void descape(MprTestGroup *gp)
{
    char    *uri, *escaped, *descaped;
    bool    match;
    int     i;

    uri = " \t\r\n\x01\x7f\xff?<>\"#%{}|\\^[]?;";
    escaped = mprEncode64(uri);
    descaped = mprDecode64(escaped);
    match = (strcmp(descaped, uri) == 0);
    if (!match) {
        mprLog(0, "Uri \"%s\" descaped to \"%s\" from \"%s\"\n", uri, descaped, escaped);
        mprLog(0, "Lengths %d %d\n", strlen(descaped), strlen(uri));
        for (i = 0; i < (int) strlen(descaped); i++) {
            mprLog(0, "Chr[%d] descaped %x, uri %x\n", i, descaped[i], uri[i]);
        }
    }
    assert(match);
}


static bool normalize(MprTestGroup *gp, char *uri, char *expectedUri)
{
    char    *validated;

    validated = httpNormalizeUriPath(uri);
    if (strcmp(expectedUri, validated) == 0) {
        return 1;
    } else {
        mprLog(0, "Uri \"%s\" validated to \"%s\" instead of \"%s\"\n", uri, validated, expectedUri);
        return 0;
    }
}


static bool okEscapeUri(MprTestGroup *gp, char *uri, char *expectedUri, int map)
{
    char    *escaped;

    escaped = mprUriEncode(uri, map);
    if (strcmp(expectedUri, escaped) == 0) {
        return 1;
    }
    mprLog(0, "Uri \"%s\" is escaped to be \n" "\"%s\" instead of \n\"%s\"\n", uri, escaped, expectedUri);
    return 0;
}


static bool okEscapeCmd(MprTestGroup *gp, char *cmd, char *validCmd)
{
    char    *escaped;

    escaped = mprEscapeCmd(cmd, '\\');
    if (strcmp(validCmd, escaped) == 0) {
        return 1;
    }
    mprLog(0, "Cmd \"%s\" is escaped to be \n"
        "\"%s\" instead of \n"
        "\"%s\"\n", cmd, escaped, validCmd);
    return 0;
}


static bool okEscapeHtml(MprTestGroup *gp, char *html, char *expectedHtml)
{
    char    *escaped;

    escaped = mprEscapeHtml(html);
    if (strcmp(expectedHtml, escaped) == 0) {
        return 1;
    }
    mprLog(0, "HTML \"%s\" is escaped to be \n"
        "\"%s\" instead of \n"
        "\"%s\"\n", html, escaped, expectedHtml);
    return 0;
}


MprTestDef testHttp = {
    "http", 0, 0, 0,
    {
        MPR_TEST(0, setup),
        MPR_TEST(0, validateUri),
        MPR_TEST(0, escape),
        MPR_TEST(0, descape),
        MPR_TEST(0, 0),
    },
};


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
