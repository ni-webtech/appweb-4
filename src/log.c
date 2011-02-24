/*
    log.c -- Error and access Logging

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/************************************ Code *************************************/
/*
    Turn on logging. If no logSpec is specified, default to stdout:2. If the user specifies --log "none" then 
    the log is disabled. This is useful when specifying the log via the appweb.conf.
 */
static void logHandler(int flags, int level, cchar *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *prefix, buf[MPR_MAX_STRING];

    mpr = mprGetMpr();
    if ((file = (MprFile*) mpr->logData) == 0) {
        return;
    }
    prefix = mpr->name;

    while (*msg == '\n') {
        mprFprintf(file, "\n");
        msg++;
    }
    if (flags & MPR_LOG_SRC) {
        mprFprintf(file, "%s: %d: %s\n", prefix, level, msg);

    } else if (flags & MPR_ERROR_SRC) {
        mprSprintf(buf, sizeof(buf), "%s: Error: %s\n", prefix, msg);
        mprWriteToOsLog(buf, flags, level);

        /*
            Use static printing to avoid malloc when the messages are small.
            This is important for memory allocation errors.
         */
        if (strlen(msg) < (MPR_MAX_STRING - 32)) {
            mprWriteFileString(file, buf);
        } else {
            mprFprintf(file, "%s: Error: %s\n", prefix, msg);
        }

    } else if (flags & MPR_FATAL_SRC) {
        mprSprintf(buf, sizeof(buf), "%s: Fatal: %s\n", prefix, msg);
        mprWriteFileString(file, buf);
        mprWriteToOsLog(buf, flags, level);
        
    } else if (flags & MPR_RAW) {
        mprFprintf(file, "%s", msg);
    }
}


/*
    Start error and information logging. Note: this is not per-request access logging
 */
int maStartLogging(HttpHost *host, cchar *logSpec)
{
    Mpr         *mpr;
    MprFile     *file;
    MprPath     info;
    char        *levelSpec, *spec;
    int         level, mode;
    static int  once = 0;

    level = 0;
    mpr = mprGetMpr();

    if (logSpec == 0) {
        logSpec = "stdout:0";
    }
    if (*logSpec && strcmp(logSpec, "none") != 0) {
        spec = sclone(logSpec);
        if ((levelSpec = strrchr(spec, ':')) != 0 && isdigit((int) levelSpec[1])) {
            *levelSpec++ = '\0';
            level = atoi(levelSpec);
        }
        if (strcmp(spec, "stdout") == 0) {
            file = mpr->fileSystem->stdOutput;
        } else {
            mode = O_CREAT | O_WRONLY | O_TEXT;
            if (host && host->logCount) {
                mode |= O_APPEND;
                mprGetPathInfo(spec, &info);
                if (host->logSize <= 0 || (info.valid && info.size > host->logSize)) {
                    maRotateAccessLog(spec, host->logCount, host->logSize);
                }
            } else {
                mode |= O_TRUNC;
            }
            if ((file = mprOpenFile(spec, mode, 0664)) == 0) {
                mprPrintfError("Can't open log file %s\n", spec);
                return -1;
            }
            once = 0;
        }
        mprSetLogLevel(level);
        mprSetLogHandler(logHandler, (void*) file);
#if FUTURE && !BLD_WIN_LIKE
        /*
            TODO - The currently breaks MprCmd as it will close stderr.
         */
        dup2(file->fd, 1);
        dup2(file->fd, 2);
#endif
        if (once++ == 0) {
            mprLog(MPR_CONFIG, "Configuration for %s", mprGetAppTitle(mpr));
            mprLog(MPR_CONFIG, "---------------------------------------------");
            mprLog(MPR_CONFIG, "Host:               %s", mprGetHostName(mpr));
            mprLog(MPR_CONFIG, "CPU:                %s", BLD_CPU);
            mprLog(MPR_CONFIG, "OS:                 %s", BLD_OS);
            if (strcmp(BLD_DIST, "Unknown") != 0) {
                mprLog(MPR_CONFIG, "Distribution:       %s %s", BLD_DIST, BLD_DIST_VER);
            }
            mprLog(MPR_CONFIG, "Version:            %s-%s", BLD_VERSION, BLD_NUMBER);
            mprLog(MPR_CONFIG, "BuildType:          %s", BLD_TYPE);
            mprLog(MPR_CONFIG, "---------------------------------------------");
        }
    }
    return 0;
}


/*
    Stop the error and information logging. Note: this is not per-request access logging
 */
int maStopLogging()
{
    MprFile     *file;
    Mpr         *mpr;

    mpr = mprGetMpr();

    file = mpr->logData;
    if (file) {
        mprCloseFile(file);
        mpr->logData = 0;
        mprSetLogHandler(0, 0);
    }
    return 0;
}


int maStartAccessLogging(HttpHost *host)
{
#if !BLD_FEATURE_ROMFS
    if (host->logPath) {
        host->log = mprOpenFile(host->logPath, O_CREAT | O_APPEND | O_WRONLY | O_TEXT, 0664);
        if (host->log == 0) {
            mprError("Can't open log file %s", host->logPath);
        }
    }
#endif
    return 0;
}


int maStopAccessLogging(HttpHost *host)
{
    host->log = 0;
    return 0;
}


void maSetAccessLog(HttpHost *host, cchar *path, cchar *format)
{
    char    *src, *dest;

    mprAssert(host);
    mprAssert(path && *path);
    mprAssert(format);
    
    if (format == NULL || *format == '\0') {
        format = "%h %l %u %t \"%r\" %>s %b";
    }

    host->logPath = sclone(path);
    host->logFormat = sclone(format);

    for (src = dest = host->logFormat; *src; src++) {
        if (*src == '\\' && src[1] != '\\') {
            continue;
        }
        *dest++ = *src;
    }
    *dest = '\0';
}


#if UNUSED
void maSetLogHost(HttpHost *host, HttpHost *logHost)
{
    host->logHost = logHost;
}
#endif


void maWriteAccessLogEntry(HttpHost *host, cchar *buf, int len)
{
    static int once = 0;

    if (mprWriteFile(host->log, (char*) buf, len) != len && once++ == 0) {
        mprError("Can't write to access log %s", host->logPath);
    }
}


void maRotateAccessLog(cchar *path, int count, int maxSize)
{
    char    *from, *to;
    int     i;

    for (i = count - 1; i > 0; i--) {
        from = mprAsprintf("%s.%d", path, i);
        to = mprAsprintf("%s.%d", path, i - 1);
        unlink(to);
        rename(from, to);
    }
}


void maLogRequest(HttpConn *conn)
{
    HttpHost    *host;
    HttpRx      *rx;
    HttpTx      *tx;
    MprBuf      *buf;
    char        keyBuf[80], *timeText, *fmt, *cp, *qualifier, *value, c;
    int         len;

    rx = conn->rx;
    tx = conn->tx;
    host = httpGetConnContext(conn);
    if (host == 0) {
        return;
    }
#if UNUSED
    host = host->logHost;
#endif
    fmt = host->logFormat;
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
            mprPutIntToBuf(buf, tx->bytesWritten - tx->headerSize);
            break;

        case 'h':                           /* Remote host */
            //  TODO - Should this trigger a reverse DNS?
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'n':                           /* Local host */
            mprPutStringToBuf(buf, rx->parsedUri->host);
            break;

        case 'l':                           /* Supplied in authorization */
            mprPutStringToBuf(buf, conn->authUser ? conn->authUser : "-");
            break;

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
            timeText = mprFormatLocalTime(mprGetTime());
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
                    value = (char*) mprLookupHash(rx->headers, supper(keyBuf));
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
    mprWriteFile(host->log, mprGetBufStart(buf), mprGetBufLength(buf));
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
