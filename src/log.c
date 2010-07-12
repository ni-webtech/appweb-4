/*
    log.c -- Logging

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/************************************ Code *************************************/
/*
    Turn on logging. If no logSpec is specified, default to stdout:2. If the user specifies --log "none" then 
    the log is disabled. This is useful when specifying the log via the appweb.conf.
 */
static void logHandler(MprCtx ctx, int flags, int level, cchar *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *prefix, buf[MPR_MAX_STRING];

    mpr = mprGetMpr(ctx);
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
        mprSprintf(ctx, buf, sizeof(buf), "%s: Error: %s\n", prefix, msg);
        mprWriteToOsLog(ctx, buf, flags, level);

        /*
            Use static printing to avoid malloc when the messages are small.
            This is important for memory allocation errors.
         */
        if (strlen(msg) < (MPR_MAX_STRING - 32)) {
            mprWriteString(file, buf);
        } else {
            mprFprintf(file, "%s: Error: %s\n", prefix, msg);
        }

    } else if (flags & MPR_FATAL_SRC) {
        mprSprintf(ctx, buf, sizeof(buf), "%s: Fatal: %s\n", prefix, msg);
        mprWriteString(file, buf);
        mprWriteToOsLog(ctx, buf, flags, level);
        
    } else if (flags & MPR_RAW) {
        mprFprintf(file, "%s", msg);
    }
}


/*
    Start error and information logging. Note: this is not per-request access logging
 */
int maStartLogging(MprCtx ctx, cchar *logSpec)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *levelSpec, *spec;
    int         level;
    static int  once = 0;

    level = 0;
    mpr = mprGetMpr(ctx);

    if (logSpec == 0) {
        logSpec = "stdout:0";
    }
    if (*logSpec && strcmp(logSpec, "none") != 0) {
        spec = mprStrdup(mpr, logSpec);
        if ((levelSpec = strrchr(spec, ':')) != 0 && isdigit((int) levelSpec[1])) {
            *levelSpec++ = '\0';
            level = atoi(levelSpec);
        }

        if (strcmp(spec, "stdout") == 0) {
            file = mpr->fileSystem->stdOutput;
        } else {
            if ((file = mprOpen(mpr, spec, O_CREAT | O_WRONLY | O_TRUNC | O_TEXT, 0664)) == 0) {
                mprPrintfError(mpr, "Can't open log file %s\n", spec);
                return -1;
            }
            once = 0;
        }
        mprSetLogLevel(mpr, level);
        mprSetLogHandler(mpr, logHandler, (void*) file);
#if FUTURE && !BLD_WIN_LIKE
        /*
            TODO - The currently breaks MprCmd as it will close stderr.
         */
        dup2(file->fd, 1);
        dup2(file->fd, 2);
#endif
        if (once++ == 0) {
            mprLog(mpr, MPR_CONFIG, "Configuration for %s", mprGetAppTitle(mpr));
            mprLog(mpr, MPR_CONFIG, "---------------------------------------------");
            mprLog(mpr, MPR_CONFIG, "Host:               %s", mprGetHostName(mpr));
            mprLog(mpr, MPR_CONFIG, "CPU:                %s", BLD_CPU);
            mprLog(mpr, MPR_CONFIG, "OS:                 %s", BLD_OS);
            if (strcmp(BLD_DIST, "Unknown") != 0) {
                mprLog(mpr, MPR_CONFIG, "Distribution:       %s %s", BLD_DIST, BLD_DIST_VER);
            }
            mprLog(mpr, MPR_CONFIG, "Version:            %s-%s", BLD_VERSION, BLD_NUMBER);
            mprLog(mpr, MPR_CONFIG, "BuildType:          %s", BLD_TYPE);
            mprLog(mpr, MPR_CONFIG, "---------------------------------------------");
        }
    }
    return 0;
}


/*
    Stop the error and information logging. Note: this is not per-request access logging
 */
int maStopLogging(MprCtx ctx)
{
    MprFile     *file;
    Mpr         *mpr;

    mpr = mprGetMpr(ctx);

    file = mpr->logData;
    if (file) {
        mprFree(file);
        mpr->logData = 0;
        mprSetLogHandler(mpr, 0, 0);
    }
    return 0;
}


int maStartAccessLogging(MaHost *host)
{
#if !BLD_FEATURE_ROMFS
    if (host->logPath) {
        host->accessLog = mprOpen(host, host->logPath, O_CREAT | O_APPEND | O_WRONLY | O_TEXT, 0664);
        if (host->accessLog == 0) {
            mprError(host, "Can't open log file %s", host->logPath);
        }
    }
#endif
    return 0;
}


int maStopAccessLogging(MaHost *host)
{
    if (host->accessLog) {
        mprFree(host->accessLog);
        host->accessLog = 0;
    }
    return 0;
}


void maSetAccessLog(MaHost *host, cchar *path, cchar *format)
{
    char    *src, *dest;

    mprAssert(host);
    mprAssert(path && *path);
    mprAssert(format);
    
    if (format == NULL || *format == '\0') {
        format = "%h %l %u %t \"%r\" %>s %b";
    }

    mprFree(host->logPath);
    host->logPath = mprStrdup(host, path);

    mprFree(host->logFormat);
    host->logFormat = mprStrdup(host, format);

    for (src = dest = host->logFormat; *src; src++) {
        if (*src == '\\' && src[1] != '\\') {
            continue;
        }
        *dest++ = *src;
    }
    *dest = '\0';
}


void maSetLogHost(MaHost *host, MaHost *logHost)
{
    host->logHost = logHost;
}


void maWriteAccessLogEntry(MaHost *host, cchar *buf, int len)
{
    static int once = 0;

    if (mprWrite(host->accessLog, (char*) buf, len) != len && once++ == 0) {
        mprError(host, "Can't write to access log %s", host->logPath);
    }
}


/*
    Called to rotate the access log
 */
static void rotateAccessLog(MaHost *host)
{
    MprPath         info;
    struct tm       tm;
    MprTime         when;
    char            bak[MPR_MAX_FNAME];

    /*
        Rotate logs when full
     */
    if (mprGetPathInfo(host, host->logPath, &info) == 0 && info.size > MA_MAX_ACCESS_LOG) {

        when = mprGetTime(host);
        mprDecodeUniversalTime(host, &tm, when);

        mprSprintf(host, bak, sizeof(bak), "%s-%02d-%02d-%02d-%02d:%02d:%02d", host->logPath, 
            tm.tm_mon, tm.tm_mday, tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);

        mprFree(host->accessLog);
        rename(host->logPath, bak);
        unlink(host->logPath);

        host->accessLog = mprOpen(host, host->logPath, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0664);
    }
}


void maLogRequest(HttpConn *conn)
{
    MaHost          *host, *logHost;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprBuf          *buf;
    char            keyBuf[80], *timeText, *fmt, *cp, *qualifier, *value, c;
    int             len;

    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnContext(conn);

    logHost = host->logHost;
    if (logHost == 0) {
        return;
    }
    fmt = logHost->logFormat;
    if (fmt == 0) {
        return;
    }
    if (rec->method == 0) {
        return;
    }

    len = MPR_MAX_URL + 256;
    buf = mprCreateBuf(trans, len, len);

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
            if (trans->bytesWritten == 0) {
                mprPutCharToBuf(buf, '-');
            } else {
                mprPutIntToBuf(buf, trans->bytesWritten);
            } 
            break;

        case 'B':                           /* Bytes written (minus headers) */
            mprPutIntToBuf(buf, trans->bytesWritten - trans->headerSize);
            break;

        case 'h':                           /* Remote host */
            //  TODO - Should this trigger a reverse DNS?
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'n':                           /* Local host */
            mprPutStringToBuf(buf, rec->parsedUri->host);
            break;

        case 'l':                           /* Supplied in authorization */
            mprPutStringToBuf(buf, conn->authUser ? conn->authUser : "-");
            break;

        case 'O':                           /* Bytes written (including headers) */
            mprPutIntToBuf(buf, trans->bytesWritten);
            break;

        case 'r':                           /* First line of request */
            mprPutFmtToBuf(buf, "%s %s %s", rec->method, rec->uri, conn->protocol);
            break;

        case 's':                           /* Response code */
            mprPutIntToBuf(buf, trans->status);
            break;

        case 't':                           /* Time */
            mprPutCharToBuf(buf, '[');
            timeText = mprFormatLocalTime(conn, mprGetTime(conn));
            mprPutStringToBuf(buf, timeText);
            mprFree(timeText);
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
                mprStrcpy(keyBuf, sizeof(keyBuf), "HTTP_");
                mprStrcpy(&keyBuf[5], sizeof(keyBuf) - 5, qualifier);
                mprStrUpper(keyBuf);
                switch (c) {
                case 'i':
                    value = (char*) mprLookupHash(rec->headers, keyBuf);
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
                mprPutIntToBuf(buf, trans->status);
            }
            break;

        default:
            mprPutCharToBuf(buf, c);
            break;
        }
    }
    mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);
    mprWrite(logHost->accessLog, mprGetBufStart(buf), mprGetBufLength(buf));

    rotateAccessLog(host);
}


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
