/*
    testEgi.c -- Unit test for EGI

    The EGI handler implements a very fast in-process CGI scheme.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/*********************************** Defines **********************************/
 
static int  getVars(HttpQueue *q, char ***keys, char *buf, int len);
static void printFormVars(HttpQueue *q);
static void printRequestHeaders(HttpQueue *q);
static void printQueryData(HttpQueue *q);
static void printBodyData(HttpQueue *q);

/************************************* Code ***********************************/
/*
    This code is never in release product.
 */

static void simpleTest(HttpQueue *q)
{
    httpWrite(q, "Hello %s\r\n", httpGetFormVar(q->conn, "name", "unknown"));
    httpCompleteRequest(q->conn);
}


static void bigTest(HttpQueue *q)
{
    int     i;

    for (i = 0; i < 200; i++) {
        httpWrite(q, "line %04d 012345678901234567890123456789012345678901234567890123456789\r\n", i);
    }
    httpCompleteRequest(q->conn);
}


static void exitApp(HttpQueue *q)
{
    httpCompleteRequest(q->conn);
    mprLog(0, "Instructed to exit ...");
    mprTerminate(1);
}


static void printVars(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpRx      *rx;
    char        *sw;
    char        *newLocation;
    int         responseStatus;

    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;
    newLocation = 0;
    responseStatus = 0;
    sw = 0;

    /*
        Parse the switches
     */
    if (rx->parsedUri->query) {
        sw = (char*) strstr(rx->parsedUri->query, "SWITCHES=");
        if (sw) {
            sw = mprUriDecode(&sw[9]);
            if (*sw == '-') {
                if (sw[1] == 'l') {
                    newLocation = sw + 3;
                } else if (sw[1] == 's') {
                    responseStatus = atoi(sw + 3);
                }
            }
        }
    }
    httpSetStatus(conn, 200);
    httpSetMimeType(conn, "text/html");
    httpDontCache(conn);

    /*
        Test writing headers. The Server header overwrote the "Server" header
      
        httpSetHeader(conn, "MyCustomHeader", "true");
        httpSetHeader(conn, "Server", "private");
     */

    if (httpGetCookies(conn) == 0) {
        httpSetCookie(conn, "appwebTest", "Testing can be fun", "/", NULL, 43200, 0);
    }

    if (newLocation) {
        httpRedirect(conn, 302, newLocation);

    } else if (responseStatus) {
        httpError(conn, responseStatus, "Custom Status");

    } else {
        httpWrite(q, "<HTML><TITLE>egiProgram: EGI Output</TITLE><BODY>\r\n");
        printRequestHeaders(q);
        printFormVars(q);
        printQueryData(q);
        printBodyData(q);
        httpWrite(q, "</BODY></HTML>\r\n");
    }
    httpCompleteRequest(q->conn);
}


static void printRequestHeaders(HttpQueue *q)
{
    MprHashTable    *env;
    MprHash         *hp;

    env = q->conn->rx->headers;
    httpWrite(q, "<H2>Request Headers</H2>\r\n");
    for (hp = 0; (hp = mprGetNextHash(env, hp)) != 0; ) {
        httpWrite(q, "<P>%s=%s</P>\r\n", hp->key, hp->data ? hp->data: "");
    }
    httpWrite(q, "\r\n");
    /* Don't finalize */
}


static void printFormVars(HttpQueue *q)
{
    MprHashTable    *env;
    MprHash         *hp;

    env = q->conn->rx->formVars;
    httpWrite(q, "<H2>Request Form Variables</H2>\r\n");
    for (hp = 0; (hp = mprGetNextHash(env, hp)) != 0; ) {
        httpWrite(q, "<P>%s=%s</P>\r\n", hp->key, hp->data ? hp->data: "");
    }
    httpWrite(q, "\r\n");
    /* Don't finalize */
}


static void printQueryData(HttpQueue *q)
{
    HttpRx  *rx;
    char    buf[MPR_MAX_STRING], **keys, *value;
    int     i, numKeys;

    rx = q->conn->rx;
    if (rx->parsedUri->query == 0) {
        return;
    }
    scopy(buf, sizeof(buf), rx->parsedUri->query);
    numKeys = getVars(q, &keys, buf, (int) strlen(buf));

    if (numKeys == 0) {
        httpWrite(q, "<H2>No Query Data Found</H2>\r\n");
    } else {
        httpWrite(q, "<H2>Decoded Query Data Variables</H2>\r\n");
        for (i = 0; i < (numKeys * 2); i += 2) {
            value = keys[i+1];
            httpWrite(q, "<p>QVAR %s=%s</p>\r\n", keys[i], value ? value: "");
        }
    }
    httpWrite(q, "\r\n");
    /* Don't finalize */
}


static void printBodyData(HttpQueue *q)
{
    MprBuf  *buf;
    char    **keys, *value;
    int     i, numKeys;
    
    if (q->pair == 0 || q->pair->first == 0) {
        return;
    }
    buf = q->pair->first->content;
    mprAddNullToBuf(buf);
    numKeys = getVars(q, &keys, mprGetBufStart(buf), (int) mprGetBufLength(buf));

    if (numKeys == 0) {
        httpWrite(q, "<H2>No Body Data Found</H2>\r\n");
    } else {
        httpWrite(q, "<H2>Decoded Body Data</H2>\r\n");
        for (i = 0; i < (numKeys * 2); i += 2) {
            value = keys[i+1];
            httpWrite(q, "<p>PVAR %s=%s</p>\r\n", keys[i], value ? value: "");
        }
    }
    httpWrite(q, "\r\n");
    /* Don't finalize */
}


static int getVars(HttpQueue *q, char ***keys, char *buf, int len)
{
    char**  keyList;
    char    *eq, *cp, *pp, *tok;
    int     i, keyCount;

    *keys = 0;

    /*
        Change all plus signs back to spaces
     */
    keyCount = (len > 0) ? 1 : 0;
    for (cp = buf; cp < &buf[len]; cp++) {
        if (*cp == '+') {
            *cp = ' ';
        } else if (*cp == '&' && (cp > buf && cp < &buf[len - 1])) {
            keyCount++;
        }
    }

    if (keyCount == 0) {
        return 0;
    }

    /*
        Crack the input into name/value pairs 
     */
    keyList = mprAlloc((keyCount * 2) * sizeof(char**));

    i = 0;
    tok = 0;
    for (pp = stok(buf, "&", &tok); pp; pp = stok(0, "&", &tok)) {
        if ((eq = strchr(pp, '=')) != 0) {
            *eq++ = '\0';
            pp = mprUriDecode(pp);
            eq = mprUriDecode(eq);
        } else {
            pp = mprUriDecode(pp);
            eq = 0;
        }
        if (i < (keyCount * 2)) {
            keyList[i++] = pp;
            keyList[i++] = eq;
        }
    }
    *keys = keyList;
    return keyCount;
}


static void upload(HttpQueue *q)
{
    HttpConn    *conn;
    char        *sw;
    char        *newLocation;
    int         responseStatus;

    conn = q->conn;
    newLocation = 0;
    responseStatus = 0;

    sw = (char*) strstr(httpGetFormVar(conn, "QUERY_STRING", ""), "SWITCHES=");
    if (sw) {
        sw = mprUriDecode(&sw[9]);
        if (*sw == '-') {
            if (sw[1] == 'l') {
                newLocation = sw + 3;
            } else if (sw[1] == 's') {
                responseStatus = atoi(sw + 3);
            }
        }
    }
    httpSetStatus(conn, 200);
    httpSetMimeType(conn, "text/html");
    httpDontCache(conn);

    if (httpGetCookies(conn) == 0) {
        httpSetCookie(conn, "appwebTest", "Testing can be fun", "/", NULL, 43200, 0);
    }
    if (newLocation) {
        httpRedirect(conn, 302, newLocation);
    } else if (responseStatus) {
        httpError(conn, responseStatus, "Custom Status");
    } else {
        httpWrite(q, "<HTML><TITLE>egiProgram: EGI Output</TITLE><BODY>\r\n");
        printRequestHeaders(q);
        printQueryData(q);
        printBodyData(q);
        httpWrite(q, "</BODY></HTML>\r\n");
    }
    httpCompleteRequest(q->conn);
}


int maEgiTestModuleInit(Http *http, MprModule *mp)
{
    /*  
        Five instances of the same program. Location blocks must be defined in appweb.conf to test these.
     */
    maDefineEgiForm(http, "/egi/exit", exitApp);
    maDefineEgiForm(http, "/egi/egiProgram", printVars);
    maDefineEgiForm(http, "/egi/egiProgram.egi", printVars);
    maDefineEgiForm(http, "/egi/egi Program.egi", printVars);
    maDefineEgiForm(http, "/egiProgram.egi", printVars);
    maDefineEgiForm(http, "/MyInProcScripts/egiProgram.egi", printVars);
    maDefineEgiForm(http, "/myEgi/egiProgram.egi", printVars);
    maDefineEgiForm(http, "/myEgi/egiProgram", printVars);
    maDefineEgiForm(http, "/upload/upload.egi", upload);
    maDefineEgiForm(http, "/egi/test", simpleTest);
    maDefineEgiForm(http, "/test.egi", simpleTest);
    maDefineEgiForm(http, "/big.egi", bigTest);
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
