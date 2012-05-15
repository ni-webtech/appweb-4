/*
    testAppweb.c - Main program for the Appweb API unit tests

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "testAppweb.h"

/****************************** Test Definitions ******************************/

typedef struct App {
    char        *postData;
    char        *host;
    int         port;
} App;

static App *app;
extern MprTestDef testHttp;

static MprTestDef *groups[] = 
{
    &testHttp,
    0
};
 
static MprTestDef master = {
    "api",
    groups,
    0, 0, 
    { { 0 } },
};

static MprTestService   *ts;

/******************************* Forward Declarations *************************/

static int  parseArgs(int argc, char **argv);
static void manageApp(App *app, int flags);

/************************************* Code ***********************************/

int main(int argc, char *argv[]) 
{
    Mpr             *mpr;
    int             rc;

    mpr = mprCreate(argc, argv, MPR_USER_EVENTS_THREAD);
    mprAddStandardSignals();

    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);

    app->host = sclone("127.0.0.1");
    app->port = 4100;

    if ((ts = mprCreateTestService(mpr)) == 0) {
        mprError("Can't create test service");
        exit(2);
    }
    if (mprParseTestArgs(ts, argc, argv, parseArgs) < 0) {
        mprPrintfError("\n"
            "  Commands specifically for %s\n"
            "    --host ip:port      # Set the default host address for testing\n\n",
            mprGetAppName(mpr));
        exit(3);
    }
    if (mprAddTestGroup(ts, &master) == 0) {
        exit(4);
    }

#if BIT_FEATURE_SSL
    if (!mprLoadSsl(0)) {
        return 0;
    }
#endif
    /*
        Need a background event thread as we use the main thread to run the tests.
     */
    if (mprStart(mpr)) {
        mprError("Can't start mpr services");
        exit(5);
    }
    /*
        Run the tests and return zero if 100% success
     */
    rc = mprRunTests(ts);
    mprReportTestResults(ts);
    mprDestroy(MPR_EXIT_DEFAULT);
    return (rc == 0) ? 0 : -1;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->postData);
        mprMark(app->host);
    }
}


static int parseArgs(int argc, char **argv)
{
    char    *argp, *ip, *cp;
    int     nextArg;

    mprAssert(app);
    nextArg = 0;

    argp = argv[nextArg++];

    if (scmp(argp, "--host") == 0 || scmp(argp, "-h") == 0) {
        ip = argv[nextArg++];
        if (ip == 0) {
            return MPR_ERR_BAD_ARGS;
        }
        if (sncmp(ip, "http://", 7) == 0) {
            ip += 7;
        }
        if ((cp = strchr(ip, ':')) != 0) {
            *cp++ = '\0';
            app->port = atoi(cp);
        } else {
            app->port = 80;
        }
        app->host = sclone(ip);
    }
    return nextArg - 1;
}


int startRequest(MprTestGroup *gp, cchar *method, cchar *uri)
{
    Http        *http;
    HttpConn    *conn;

    gp->content = 0;
    http = getHttp(gp);

    if (*uri == '/') {
        httpSetDefaultClientPort(http, app->port);
        httpSetDefaultClientHost(http, app->host);
    }
    gp->conn = conn = httpCreateConn(http, NULL, gp->dispatcher);
    if (httpConnect(conn, method, uri, NULL) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    return 0;
}


bool simpleGet(MprTestGroup *gp, cchar *uri, int expectStatus)
{
    HttpConn    *conn;
    int         status;

    if (expectStatus <= 0) {
        expectStatus = 200;
    }
    if (startRequest(gp, "GET", uri) < 0) {
        return 0;
    }
    conn = getConn(gp);
    httpFinalize(conn);
    if (httpWait(conn, HTTP_STATE_COMPLETE, -1) < 0) {
        return MPR_ERR_CANT_READ;
    }
    status = httpGetStatus(gp->conn);

    assert(status == expectStatus);
    if (status != expectStatus) {
        mprLog(0, "simpleGet: HTTP response code %d, expected %d", status, expectStatus);
        return 0;
    }
    assert(httpGetError(gp->conn) != 0);
    gp->content = httpReadString(gp->conn);
    assert(gp->content != NULL);
    mprLog(4, "Response content %s", gp->content);
    httpDestroyConn(gp->conn);
    gp->conn = 0;
    return 1;
}


bool simpleForm(MprTestGroup *gp, char *uri, char *formData, int expectStatus)
{
    HttpConn    *conn;
    MprOff      contentLen;
    ssize       len;
    int         status;

    contentLen = 0;
    
    if (expectStatus <= 0) {
        expectStatus = 200;
    }
    if (startRequest(gp, "POST", uri) < 0) {
        return 0;
    }
    conn = getConn(gp);

    if (formData) {
        httpSetHeader(conn, "Content-Type", "application/x-www-form-urlencoded");
        len = slen(formData);
        if (httpWrite(conn->writeq, formData, len) != len) {
            return MPR_ERR_CANT_WRITE;
        }
    }
    httpFinalize(conn);
    if (httpWait(conn, HTTP_STATE_COMPLETE, -1) < 0) {
        return MPR_ERR_CANT_READ;
    }
    status = httpGetStatus(conn);
    if (status != expectStatus) {
        mprLog(0, "Client failed for %s, response code: %d, msg %s\n", uri, status, httpGetStatusMessage(conn));
        return 0;
    }
    gp->content = httpReadString(conn);
    contentLen = httpGetContentLength(conn);
    if (! assert(gp->content != 0 && contentLen > 0)) {
        return 0;
    }
    mprLog(4, "Response content %s", gp->content);
    return 1;
}


bool simplePost(MprTestGroup *gp, char *uri, char *bodyData, ssize len, int expectStatus)
{
    HttpConn    *conn;
    MprOff      contentLen;
    int         status;

    contentLen = 0;
    conn = getConn(gp);

    if (expectStatus <= 0) {
        expectStatus = 200;
    }
    if (startRequest(gp, "POST", uri) < 0) {
        return 0;
    }
    if (bodyData) {
        if (httpWrite(conn->writeq, bodyData, len) != len) {
            return MPR_ERR_CANT_WRITE;
        }
    }
    httpFinalize(conn);
    if (httpWait(conn, HTTP_STATE_COMPLETE, -1) < 0) {
        return MPR_ERR_CANT_READ;
    }

    status = httpGetStatus(conn);
    if (status != expectStatus) {
        mprLog(0, "Client failed for %s, response code: %d, msg %s\n", uri, status, httpGetStatusMessage(conn));
        return 0;
    }
    gp->content = httpReadString(conn);
    contentLen = httpGetContentLength(conn);
    if (! assert(gp->content != 0 && contentLen > 0)) {
        return 0;
    }
    mprLog(4, "Response content %s", gp->content);
    return 1;
}


bool bulkPost(MprTestGroup *gp, char *url, int size, int expectStatus)
{
    char    *post;
    int     i, j;
    bool    success;

    app->postData = post = (char*) mprAlloc(size + 1);
    assert(post != 0);

    for (i = 0; i < size; i++) {
        if (i > 0) {
            mprSprintf(&post[i], 10, "&%07d=", i / 64);
        } else {
            mprSprintf(&post[i], 10, "%08d=", i / 64);
        }
        for (j = i + 9; j < (i + 63); j++) {
            post[j] = 'a';
        }
        post[j] = '\n';
        i = j;
    }
    post[i] = '\0';

    success = simplePost(gp, url, post, slen(post), expectStatus);
    assert(success);
    app->postData = 0;
    return success;
}


/*  
    Return the shared http instance. Using this minimizes TIME_WAITS by using keep alive.
 */
Http *getHttp(MprTestGroup *gp)
{
    if (gp->http == 0) {
        gp->http = httpCreate(gp);
    }
    return gp->http;
}


HttpConn *getConn(MprTestGroup *gp)
{
    return gp->conn;
}


/*
    Match a keyword in the content returned from the last request
    Format is:
        KEYWORD = value
 */
bool match(MprTestGroup *gp, char *key, char *value)
{
    char    *vp, *trim;

    vp = lookupValue(gp, key);
    if (vp == 0 && value == 0) {
        return 1;
    }
    trim = strim(vp, "\"", MPR_TRIM_BOTH);
    if (vp == 0 || value == 0 || scmp(trim, value) != 0) {
        mprLog(1, "Match %s failed. Got \"%s\" expected \"%s\"", key, vp, value);
        mprLog(1, "Content %s", gp->content);
        return 0;
    }
    return 1;
}


/*
    Match a keyword an ignore case for windows
 */
bool matchAnyCase(MprTestGroup *gp, char *key, char *value)
{
    char    *vp, *trim;

    vp = lookupValue(gp, key);
    if (vp == 0 && value == 0) {
        return 1;
    }
    trim = strim(vp, "\"", MPR_TRIM_BOTH);
#if BIT_WIN_LIKE
    if (vp == 0 || scasecmp(trim, value) != 0)
#else
    if (vp == 0 || value == 0 || scmp(trim, value) != 0)
#endif
    {
        mprLog(1, "Match %s failed. Got %s expected %s", key, vp, value);
        return 0;
    }
    return 1;
}


/*  
    Caller must free
 */
char *getValue(MprTestGroup *gp, char *key)
{
    char    *value;

    value = lookupValue(gp, key);
    if (value == 0) {
        return sclone("");
    } else {
        return value;
    }
}


/*  
    Return the value of a keyword in the content returned from the last request
    Format either: 
        KEYWORD=value<
        KEYWORD: value,
        "KEYWORD": value,
        "KEYWORD": value,
    Return 0 on errors. Caller must free result.
 */
char *lookupValue(MprTestGroup *gp, char *key)
{
    char    *nextToken, *bp, *result;

    if (gp->content == NULL) {
        gp->content = httpReadString(getConn(gp));
    }
    if (gp->content == 0 || (nextToken = strstr(gp->content, key)) == 0) {
        return 0;
    }
    nextToken += slen(key);
    if (*nextToken != '=' && *nextToken != ':' && *nextToken != '"') {
        return 0;
    }
    if (*nextToken == '"') {
        nextToken++;
    }
    if (*nextToken == ':') {
        nextToken += 2;
    } else {
        nextToken += 1;
    }
    result = sclone(nextToken);
    for (bp = result; *bp && *bp != '<' && *bp != ','; bp++) {
        ;
    }
    *bp++ = '\0';
    if (scmp(result, "null") == 0) {
        return 0;
    }
    return result;
}


int getDefaultPort(MprTestGroup *gp)
{
    return app->port;
}


char *getDefaultHost(MprTestGroup *gp)
{
    return app->host;
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
