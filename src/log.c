/*
    log.c -- Error and access Logging

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/************************************ Code *************************************/
#if UNUSED
/*
    Start error and information logging. Note: this is not per-request access logging.
 */
int maStartLogging(HttpHost *host, cchar *logSpec)
{
    Mpr         *mpr;
    MprFile     *file;
    MprPath     info;
    char        *levelSpec, *spec;
    int         level, mode;
    static int  once = 0;

    level = -1;
    mpr = mprGetMpr();

    if (logSpec == 0) {
        logSpec = "stderr:0";
    }
    if (*logSpec && strcmp(logSpec, "none") != 0) {
        spec = sclone(logSpec);
        if ((levelSpec = strrchr(spec, ':')) != 0 && isdigit((int) levelSpec[1])) {
            *levelSpec++ = '\0';
            level = atoi(levelSpec);
        }
        if (strcmp(spec, "stdout") == 0) {
            file = mpr->fileSystem->stdOutput;
        } else if (strcmp(spec, "stderr") == 0) {
            file = mpr->fileSystem->stdError;
        } else {
            mode = O_CREAT | O_WRONLY | O_TEXT;
            if (host && host->logCount) {
                mode |= O_APPEND;
                mprGetPathInfo(spec, &info);
                if (host->logSize <= 0 || (info.valid && info.size > host->logSize)) {
                    mprArchiveLog(spec, host->logCount);
                }
            } else {
                mode |= O_TRUNC;
            }
            if ((file = mprOpenFile(spec, mode, 0664)) == 0) {
                mprError("Can't open log file %s", spec);
                return -1;
            }
            once = 0;
        }
        if (level >= 0) {
            mprSetLogLevel(level);
        }
#if UNUSED
        mprSetLogHandler(logHandler);
#endif
        mprSetLogFile(file);

        if (once++ == 0) {
            mprLog(MPR_CONFIG, "Configuration for %s", mprGetAppTitle(mpr));
            mprLog(MPR_CONFIG, "---------------------------------------------");
            mprLog(MPR_CONFIG, "Version:            %s-%s", BLD_VERSION, BLD_NUMBER);
            mprLog(MPR_CONFIG, "BuildType:          %s", BLD_TYPE);
            mprLog(MPR_CONFIG, "CPU:                %s", BLD_CPU);
            mprLog(MPR_CONFIG, "OS:                 %s", BLD_OS);
            if (strcmp(BLD_DIST, "Unknown") != 0) {
                mprLog(MPR_CONFIG, "Distribution:       %s %s", BLD_DIST, BLD_DIST_VER);
            }
            mprLog(MPR_CONFIG, "Host:               %s", mprGetHostName(mpr));
            mprLog(MPR_CONFIG, "Configure:          %s", BLD_CONFIG_CMD);
            mprLog(MPR_CONFIG, "---------------------------------------------");
        }
    }
    return 0;
}


/*
    Stop the error and information logging. Note: this is not per-request access logging
 */
void maStopLogging()
{
    MprFile     *file;
    Mpr         *mpr;

    mpr = mprGetMpr();
    file = mpr->logFile;
    if (file) {
        mprCloseFile(file);
        mprSetLogHandler(0);
        mprSetLogFile(0);
    }
}


int maStartAccessLogging(HttpRoute *route)
{
#if !BLD_FEATURE_ROMFS
    if (route->logPath) {
        route->log = mprOpenFile(route->logPath, O_CREAT | O_APPEND | O_WRONLY | O_TEXT, 0664);
        if (route->log == 0) {
            mprError("Can't open log file %s", route->logPath);
            return MPR_ERR_CANT_OPEN;
        }
    }
#endif
    return 0;
}


void maStopAccessLogging(HttpRoute *route)
{
    route->log = 0;
}


void maSetAccessLog(HttpRoute *route, cchar *path, cchar *format)
{
    char    *src, *dest;

    mprAssert(route);
    mprAssert(path && *path);
    mprAssert(format);
    
    if (format == NULL || *format == '\0') {
        format = "%h %l %u %t \"%r\" %>s %b";
    }
    route->logPath = sclone(path);
    route->logFormat = sclone(format);

    for (src = dest = route->logFormat; *src; src++) {
        if (*src == '\\' && src[1] != '\\') {
            continue;
        }
        *dest++ = *src;
    }
    *dest = '\0';
}
#endif


void maWriteAccessLogEntry(HttpRoute *route, cchar *buf, int len)
{
    static int once = 0;

    if (mprWriteFile(route->log, (char*) buf, len) != len && once++ == 0) {
        mprError("Can't write to access log %s", route->logPath);
    }
}


void maLogRequest(HttpConn *conn)
{
    HttpHost    *host;
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    MprBuf      *buf;
    char        keyBuf[80], *timeText, *fmt, *cp, *qualifier, *value, c;
    int         len;

    rx = conn->rx;
    tx = conn->tx;
    route = rx->route;
    host = httpGetConnContext(conn);
    if (host == 0) {
        return;
    }
    fmt = route->logFormat;
    if (fmt == 0) {
        return;
    }
    if (rx->method == 0) {
        return;
    }
    len = MPR_MAX_URL + 256;
    buf = mprCreateBuf(len, len);

    while ((c = *fmt++) != '\0') {
        if (c != '%' || (c = *fmt++) == '%') {
            mprPutCharToBuf(buf, c);
            continue;
        }
        switch (c) {
        case 'a':                           /* Remote IP */
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'A':                           /* Local IP */
            mprPutStringToBuf(buf, conn->sock->listenSock->ip);
            break;

        case 'b':
            if (tx->bytesWritten == 0) {
                mprPutCharToBuf(buf, '-');
            } else {
                mprPutIntToBuf(buf, tx->bytesWritten);
            } 
            break;

        case 'B':                           /* Bytes written (minus headers) */
            mprPutIntToBuf(buf, (tx->bytesWritten - tx->headerSize));
            break;

        case 'h':                           /* Remote host */
            //  TODO - Should this trigger a reverse DNS?
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'n':                           /* Local host */
            mprPutStringToBuf(buf, rx->parsedUri->host);
            break;

#if UNUSED
        case 'l':                           /* Supplied in authorization */
            mprPutStringToBuf(buf, conn->authUser ? conn->authUser : "-");
            break;
#endif

        case 'O':                           /* Bytes written (including headers) */
            mprPutIntToBuf(buf, tx->bytesWritten);
            break;

        case 'r':                           /* First line of request */
            mprPutFmtToBuf(buf, "%s %s %s", rx->method, rx->uri, conn->protocol);
            break;

        case 's':                           /* Response code */
            mprPutIntToBuf(buf, tx->status);
            break;

        case 't':                           /* Time */
            mprPutCharToBuf(buf, '[');
            timeText = mprFormatLocalTime(MPR_DEFAULT_DATE, mprGetTime());
            mprPutStringToBuf(buf, timeText);
            mprPutCharToBuf(buf, ']');
            break;

        case 'u':                           /* Remote username */
            mprPutStringToBuf(buf, conn->authUser ? conn->authUser : "-");
            break;

        case '{':                           /* Header line */
            qualifier = fmt;
            if ((cp = strchr(qualifier, '}')) != 0) {
                fmt = &cp[1];
                *cp = '\0';
                c = *fmt++;
                scopy(keyBuf, sizeof(keyBuf), "HTTP_");
                scopy(&keyBuf[5], sizeof(keyBuf) - 5, qualifier);
                switch (c) {
                case 'i':
                    value = (char*) mprLookupKey(rx->headers, supper(keyBuf));
                    mprPutStringToBuf(buf, value ? value : "-");
                    break;
                default:
                    mprPutStringToBuf(buf, qualifier);
                }
                *cp = '}';

            } else {
                mprPutCharToBuf(buf, c);
            }
            break;

        case '>':
            if (*fmt == 's') {
                fmt++;
                mprPutIntToBuf(buf, tx->status);
            }
            break;

        default:
            mprPutCharToBuf(buf, c);
            break;
        }
    }
    mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);
    mprWriteFile(route->log, mprGetBufStart(buf), mprGetBufLength(buf));
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
