/* 
    cgiHandler.c -- Common Gateway Interface Handler

    Support the CGI/1.1 standard for external gateway programs to respond to HTTP requests.
    This CGI handler uses async-pipes and non-blocking I/O for all communications.
 
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/*********************************** Includes *********************************/

#include    "appweb.h"

#if BIT_FEATURE_CGI
/************************************ Locals ***********************************/

#define MA_CGI_SEEN_HEADER          0x1
#define MA_CGI_FLOW_CONTROL         0x2     /* Output to client is flow controlled */

/*********************************** Forwards *********************************/

static void buildArgs(HttpConn *conn, MprCmd *cmd, int *argcp, cchar ***argvp);
static ssize cgiCallback(MprCmd *cmd, int channel, void *data);
static int copyVars(cchar **envv, int index, MprHash *vars, cchar *prefix);
static char *getCgiToken(MprBuf *buf, cchar *delim);
static bool parseFirstCgiResponse(HttpConn *conn, MprCmd *cmd);
static bool parseHeader(HttpConn *conn, MprCmd *cmd);
static int relayCgiResponse(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf);
static void writeToCGI(HttpQueue *q);
static ssize readCgiResponseData(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf);

#if BIT_DEBUG
    static void traceCGIData(MprCmd *cmd, char *src, ssize size);
    #define traceData(cmd, src, size) traceCGIData(cmd, src, size)
#else
    #define traceData(cmd, src, size)
#endif

#if BIT_WIN_LIKE || VXWORKS
    static void findExecutable(HttpConn *conn, char **program, char **script, char **bangScript, char *fileName);
#endif
#if BIT_WIN_LIKE
    static void checkCompletion(HttpQueue *q, MprEvent *event);
#endif

/************************************* Code ***********************************/
/*
    Open the handler for a new request
 */
static void openCgi(HttpQueue *q)
{
    HttpRx      *rx;

    rx = q->conn->rx;

    mprLog(5, "Open CGI handler");
    if (!httpValidateLimits(q->conn->endpoint, HTTP_VALIDATE_OPEN_PROCESS, q->conn)) {
        /* Too many active CGI processes */
        return;
    }
    if (rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        httpHandleOptionsTrace(q->conn);
    } else {
        httpTrimExtraPath(q->conn);
        httpMapFile(q->conn, rx->route);
        httpCreateCGIParams(q->conn);
    }

}


static void closeCgi(HttpQueue *q)
{
    MprCmd  *cmd;

    mprLog(5, "CGI: close");
    cmd = (MprCmd*) q->queueData;
    if (cmd) {
        mprSetCmdCallback(cmd, NULL, NULL);
        if (cmd->pid) {
            /* Request has completed so can kill the command */
            mprStopCmd(cmd, -1);
        }
        mprDisconnectCmd(cmd);
    }
    httpValidateLimits(q->conn->endpoint, HTTP_VALIDATE_CLOSE_PROCESS, q->conn);
}


/*  
    Start the CGI command program. This commences the CGI gateway program. This will be called after content for
    form and upload requests (or if "RunHandler" before specified), otherwise it runs before receiving content data.
 */
static void startCgi(HttpQueue *q)
{
    HttpRx          *rx;
    HttpTx          *tx;
    HttpConn        *conn;
    MprCmd          *cmd;
    cchar           *baseName;
    cchar           **argv, *fileName;
    cchar           **envv;
    int             argc, varCount, count;

    argv = 0;
    argc = 0;
    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;

    mprLog(5, "CGI: Start");

    /*
        The command uses the conn dispatcher. This serializes all I/O for both the connection and the CGI gateway
     */
    cmd = q->queueData = mprCreateCmd(conn->dispatcher);

    if (conn->http->forkCallback) {
        cmd->forkCallback = conn->http->forkCallback;
        cmd->forkData = conn->http->forkData;
    }

    argc = 1;                                   /* argv[0] == programName */
    buildArgs(conn, cmd, &argc, &argv);
    fileName = argv[0];
    baseName = mprGetPathBase(fileName);
    
    /*
        nph prefix means non-parsed-header. Don't parse the CGI output for a CGI header
     */
    if (strncmp(baseName, "nph-", 4) == 0 || 
            (strlen(baseName) > 4 && strcmp(&baseName[strlen(baseName) - 4], "-nph") == 0)) {
        /* Pretend we've seen the header for Non-parsed Header CGI programs */
        cmd->userFlags |= MA_CGI_SEEN_HEADER;
    }
    /*  
        Build environment variables
     */
    varCount = mprGetHashLength(rx->headers) + mprGetHashLength(rx->svars);
    if (rx->params) {
        varCount += mprGetHashLength(rx->params);
    }
    if ((envv = mprAlloc((varCount + 1) * sizeof(char*))) != 0) {
        count = copyVars(envv, 0, rx->params, "");
        count = copyVars(envv, 0, rx->svars, "");
        count = copyVars(envv, count, rx->headers, "HTTP_");
        mprAssert(count <= varCount);
    }
    cmd->stdoutBuf = mprCreateBuf(HTTP_BUFSIZE, HTTP_BUFSIZE);
    cmd->stderrBuf = mprCreateBuf(HTTP_BUFSIZE, HTTP_BUFSIZE);

#if !VXWORKS
    /*
        This will be ignored on VxWorks because there is only one global current directory for all tasks
     */
    mprSetCmdDir(cmd, mprGetPathDir(fileName));
#endif
    mprSetCmdCallback(cmd, cgiCallback, tx);

    if (mprStartCmd(cmd, argc, argv, envv, MPR_CMD_IN | MPR_CMD_OUT | MPR_CMD_ERR) < 0) {
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't run CGI process: %s, URI %s", fileName, rx->uri);
    }
}


#if BIT_WIN_LIKE
/*
    This routine is called for when the outgoing queue is writable.  We don't actually do output here, just poll
    on windows (only) until the command is complete.
 */
static void writableCgi(HttpQueue *q)
{
    MprCmd      *cmd;

    cmd = (MprCmd*) q->queueData;

    if (q->pair) {
        writeToCGI(q->pair);
    }
    if (q->pair == 0 || q->pair->count == 0) {
        mprCloseCmdFd(cmd, MPR_CMD_STDIN);
    }
    /*
        Windows can't select on named pipes. So must poll here. This consumes a thread.
     */
    while (!cmd->complete) {
        mprWaitForCmd(cmd, 1000);
    }
}
#endif


/*
    Service outgoing data destined for the browser. This is response data from the CGI program. See writeToClient.
    This may be called by {maRunPipeline|cgiCallback} => httpServiceQueues
 */ 
static void outgoingCgiService(HttpQueue *q)
{
    MprCmd      *cmd;

    cmd = (MprCmd*) q->queueData;
    mprLog(7, "CGI: OutgoingCgiService pid %d, q->count %d, q->flags %x, writeBlocked %d", 
        (cmd) ? cmd->pid: -1, q->count, q->flags, q->conn->writeBlocked);
           
    /*
        This will copy outgoing packets downstream toward the network connector and on to the browser. 
        This may disable the CGI queue if the downstream net connector queue overflows because the socket 
        is full. In that case, httpEnableConnEvents will setup to listen for writable events. When the 
        socket is writable again, the connector will drain its queue which will re-enable this queue 
        and schedule it for service again.
     */ 
    httpDefaultOutgoingServiceStage(q);

    if (cmd && cmd->userFlags & MA_CGI_FLOW_CONTROL && q->count < q->low) {
        mprLog(7, "CGI: @@@ OutgoingCgiService - re-enable gateway output count %d (low %d)", q->count, q->low);
        cmd->userFlags &= ~MA_CGI_FLOW_CONTROL;
        mprEnableCmdEvents(cmd, MPR_CMD_STDOUT);
        httpResumeQueue(q);
    }
}


/*
    Accept incoming body data from the client destined for the CGI gateway. This is typically POST or PUT data.
 */
static void incomingCgi(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprCmd      *cmd;

    mprAssert(q);
    mprAssert(packet);
    
    conn = q->conn;
    rx = conn->rx;
    cmd = (MprCmd*) q->pair->queueData;
    conn->lastActivity = conn->http->now;

    if (httpGetPacketLength(packet) == 0) {
        /* End of input */
        if (rx->remainingContent > 0) {
            /* Short incoming body data. Just kill the CGI process.  */
            if (cmd) {
                mprDestroyCmd(cmd);
            }
            q->queueData = 0;
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Client supplied insufficient body data");
        } else {
            /*  
                Close the CGI program's stdin. This will allow the gateway to exit if it was 
                expecting input data and insufficient was sent by the client.
             */
            if (cmd && cmd->files[MPR_CMD_STDIN].fd >= 0) {
                mprCloseCmdFd(cmd, MPR_CMD_STDIN);
            }
        }
    } else {
        /* No service routine, we just need it to be queued for writeToCGI */
        httpPutForService(q, packet, 0);
    }
    if (cmd) {
        writeToCGI(q);
    }
}


/*
    Write data to the CGI program. This is called from incomingCgiData and from the cgiCallback when the pipe
    to the CGI program becomes writable. This routine will write all queued data and will block until it is sent. 
 */
static void writeToCGI(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;
    MprCmd      *cmd;
    MprBuf      *buf;
    ssize       rc, len;
    int         err;

    cmd = (MprCmd*) q->pair->queueData;
    mprAssert(cmd);
    conn = q->conn;

    for (packet = httpGetPacket(q); packet && conn->state < HTTP_STATE_COMPLETE; packet = httpGetPacket(q)) {
        conn->lastActivity = conn->http->now;
        buf = packet->content;
        len = mprGetBufLength(buf);
        rc = mprWriteCmd(cmd, MPR_CMD_STDIN, mprGetBufStart(buf), len);
        mprLog(5, "CGI: write %d bytes to gateway. Rc rc %d, errno %d", len, rc, mprGetOsError());
        if (rc < 0) {
            err = mprGetError();
            if (err == EINTR) {
                continue;
            } else if (err == EAGAIN || err == EWOULDBLOCK) {
                break;
            }
            mprLog(2, "CGI: write to gateway failed for %d bytes, rc %d, errno %d", len, rc, mprGetOsError());
            mprCloseCmdFd(cmd, MPR_CMD_STDIN);
            httpError(conn, HTTP_CODE_BAD_GATEWAY, "Can't write body data to CGI gateway");
            break;
        }
        mprAdjustBufStart(buf, rc);
        if (mprGetBufLength(buf) > 0) {
            httpPutBackPacket(q, packet);
        }
    }
}


/*
    Write data back to the client (browser). Must be locked when called.
 */
static int writeToClient(HttpQueue *q, MprCmd *cmd, MprBuf *buf, int channel)
{
    HttpConn    *conn;
    ssize       len, rc;

    conn = q->conn;
    mprAssert(conn->tx);

    /*
        Write to the browser. Write as much as we can. Service queues to get the filters and connectors pumping.
     */
    while (conn->tx && (len = mprGetBufLength(buf)) > 0) {
        if (conn->tx && !conn->finalized && conn->state < HTTP_STATE_COMPLETE) {
            if ((q->count + len) > q->max) {
                cmd->userFlags |= MA_CGI_FLOW_CONTROL;
                mprLog(7, "CGI: @@@ client write queue full. Suspend queue, enable conn events");
                httpSuspendQueue(q);
                return -1;
            }
            rc = httpWriteBlock(q, mprGetBufStart(buf), len);
            mprLog(7, "CGI: Write to client %d, absorbed %d, q->count %d, q->max %d, q->flags %x, writeBlocked %d", 
                len, rc, q->count, q->max, q->flags, conn->writeBlocked);
        } else {
            /* Command complete - just discard the data */
            rc = len;
        }
        if (rc > 0) {
            mprAdjustBufStart(buf, rc);
            mprResetBufIfEmpty(buf);
            httpFlushQueue(q, 0);
            mprLog(7, "CGI: After flush q->count %d, q->max %d, q->flags %x", q->count, q->max, q->flags);
        } else {
            break;
        }
    }
    return 0;
}


/*
    Read the output data from the CGI script and return it to the client. This is called by the MPR in response to
    I/O events from the CGI process for stdout/stderr data from the CGI script and for EOF from the CGI's stdin.
    This event runs on the connection's dispatcher. (ie. single threaded and safe)
 */
static ssize cgiCallback(MprCmd *cmd, int channel, void *data)
{
    HttpQueue   *q;
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       nbytes;

    tx = (HttpTx*) data;
    conn = tx->conn;
    nbytes = 0;
    if (conn == 0) {
        return 0;
    }
    mprAssert(conn->tx);
    mprAssert(conn->rx);

    tx = conn->tx;
    mprAssert(tx);
    conn->lastActivity = conn->http->now;
    q = conn->writeq;

    switch (channel) {
    case MPR_CMD_STDIN:
        /* CGI's stdin can now accept more data */
        writeToCGI(q->pair);
        break;

    case MPR_CMD_STDOUT:
        nbytes = readCgiResponseData(q, cmd, channel, cmd->stdoutBuf);
        break;

    case MPR_CMD_STDERR:
        nbytes = readCgiResponseData(q, cmd, channel, cmd->stderrBuf);
        break;
            
    default:
        /* Child death notification */
        if (cmd->pid == 0 && cmd->complete) {
            httpFinalize(conn);
        }
    }
    if (conn->state < HTTP_STATE_COMPLETE) {
        if (cmd->pid && !(cmd->userFlags & MA_CGI_FLOW_CONTROL)) {
            mprLog(7, "CGI: @@@ enable CGI events for channel %d", channel);
            mprEnableCmdEvents(cmd, channel);
        }
        if (conn->connectorq->count > 0) {
            httpEnableConnEvents(conn);
        }
    } else {
        httpPump(conn, NULL);
    }
    return nbytes;
}


/*
    Come here for CGI stdout, stderr events. ie. reading data from the CGI program.
 */
static ssize readCgiResponseData(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf)
{
    HttpConn    *conn;
    ssize       space, nbytes, total;
    int         err;

    conn = q->conn;
    mprAssert(conn->state > HTTP_STATE_BEGIN);
    mprResetBufIfEmpty(buf);
    total = 0;

    while (mprGetCmdFd(cmd, channel) >= 0 && conn->sock && conn->state > HTTP_STATE_BEGIN) {
        do {
            mprAssert(conn->state > HTTP_STATE_BEGIN);
            /*
                Read as much data from the CGI as possible
             */
            if ((space = mprGetBufSpace(buf)) == 0) {
                mprGrowBuf(buf, MPR_BUFSIZE);
                if ((space = mprGetBufSpace(buf)) == 0) {
                    break;
                }
            }
            nbytes = mprReadCmd(cmd, channel, mprGetBufEnd(buf), space);

            mprLog(7, "CGI: Read from gateway, channel %d, got %d bytes. errno %d", channel, nbytes, 
                nbytes >= 0 ? 0 : mprGetOsError());
            if (nbytes < 0) {
                err = mprGetError();
                if (err == EINTR) {
                    continue;
                } else if (err == EAGAIN || err == EWOULDBLOCK) {
                    break;
                }
                mprLog(5, "CGI: Gateway read error %d for %s", err, (channel == MPR_CMD_STDOUT) ? "stdout" : "stderr");
                /* 
                    WARNING: this may complete the request here 
                 */
                mprCloseCmdFd(cmd, channel);
                break;
                
            } else if (nbytes == 0) {
                /*
                    This may reap the terminated child and thus clear cmd->process if both stderr and stdout are closed.
                    WARNING: this may complete the request here 
                 */
                mprLog(5, "CGI: Gateway EOF for %s, pid %d", (channel == MPR_CMD_STDOUT) ? "stdout" : "stderr", cmd->pid);
                mprCloseCmdFd(cmd, channel);
                break;

            } else {
                mprLog(5, "CGI: Gateway read %d bytes from %s", nbytes, (channel == MPR_CMD_STDOUT) ? "stdout" : "stderr");
                mprAdjustBufEnd(buf, nbytes);
                traceData(cmd, mprGetBufStart(buf), nbytes);
                mprAddNullToBuf(buf);
                total += nbytes;
            }
            conn->lastActivity = conn->http->now;
        } while ((space = mprGetBufSpace(buf)) > 0 && conn->state > HTTP_STATE_BEGIN);

        if (conn->state == HTTP_STATE_BEGIN) {
            return total;
        }
        if (mprGetBufLength(buf) == 0) {
            break;
        }
        if (relayCgiResponse(q, cmd, channel, buf) < 0) {
            break;
        }
    }
    if (cmd->complete && conn->state > HTTP_STATE_BEGIN) {
        mprAssert(conn->tx);
        httpFinalize(conn);
    }
    return total;
}


/*
    Relay CGI response data to the client
 */
static int relayCgiResponse(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf)
{
    HttpConn    *conn;

    conn = q->conn;
    mprAssert(conn->state > HTTP_STATE_BEGIN);
    mprAssert(conn->tx);
    
    mprLog(7, "relayCgiResponse pid %d", cmd->pid);

    if (channel == MPR_CMD_STDERR) {
        /*
            Write stderr to the log AND write to the client
         */
        mprError("Error output from CGI command for \"%s\"\n\n%s", conn->rx->uri, mprGetBufStart(buf));
        if (writeToClient(q, cmd, buf, channel) < 0) {
            mprNop(0);
            return -1;
        }
        httpSetStatus(conn, HTTP_CODE_SERVICE_UNAVAILABLE);
        cmd->userFlags |= MA_CGI_SEEN_HEADER;
        cmd->status = 0;

    } else {
        if (!(cmd->userFlags & MA_CGI_SEEN_HEADER) && !parseHeader(conn, cmd)) {
            return -1;
        } 
        if (cmd->userFlags & MA_CGI_SEEN_HEADER && conn->state > HTTP_STATE_BEGIN) {
            if (writeToClient(q, cmd, buf, channel) < 0) {
                mprNop(0);
                return -1;
            }
        }
    }
    return 0;
}


/*
    Parse the CGI output first line
 */
static bool parseFirstCgiResponse(HttpConn *conn, MprCmd *cmd)
{
    MprBuf      *buf;
    char        *protocol, *status, *message;
    
    buf = mprGetCmdBuf(cmd, MPR_CMD_STDOUT);
    
    protocol = getCgiToken(buf, " ");
    if (protocol == 0 || protocol[0] == '\0') {
        httpError(conn, HTTP_CODE_BAD_GATEWAY, "Bad CGI HTTP protocol response");
        return 0;
    }
    if (strncmp(protocol, "HTTP/1.", 7) != 0) {
        httpError(conn, HTTP_CODE_BAD_GATEWAY, "Unsupported CGI protocol");
        return 0;
    }
    status = getCgiToken(buf, " ");
    if (status == 0 || *status == '\0') {
        httpError(conn, HTTP_CODE_BAD_GATEWAY, "Bad CGI header response");
        return 0;
    }
    message = getCgiToken(buf, "\n");
    mprLog(4, "CGI: Status line: %s %s %s", protocol, status, message);
    return 1;
}


/*
    Parse the CGI output headers. 
    Sample CGI program:

    Content-type: text/html
   
    <html.....
 */
static bool parseHeader(HttpConn *conn, MprCmd *cmd)
{
    HttpTx      *tx;
    MprBuf      *buf;
    char        *endHeaders, *headers, *key, *value, *location;
    int         fd, len;

    tx = conn->tx;
    location = 0;
    value = 0;

    buf = mprGetCmdBuf(cmd, MPR_CMD_STDOUT);
    headers = mprGetBufStart(buf);

    /*
        Split the headers from the body.
     */
    len = 0;
    fd = mprGetCmdFd(cmd, MPR_CMD_STDOUT);
    if ((endHeaders = strstr(headers, "\r\n\r\n")) == NULL) {
        if ((endHeaders = strstr(headers, "\n\n")) == NULL) {
            if (fd >= 0 && strlen(headers) < HTTP_MAX_HEADERS) {
                /* Not EOF and less than max headers and have not yet seen an end of headers delimiter */
                return 0;
            }
        } 
        len = 2;
    } else {
        len = 4;
    }
    if (endHeaders) {
        endHeaders[len - 1] = '\0';
        endHeaders += len;
    }

    /*
        Want to be tolerant of CGI programs that omit the status line.
     */
    if (strncmp((char*) buf->start, "HTTP/1.", 7) == 0) {
        if (!parseFirstCgiResponse(conn, cmd)) {
            /* httpError already called */
            return 0;
        }
    }
    if (endHeaders && strchr(mprGetBufStart(buf), ':')) {
        mprLog(4, "CGI: parseHeader: header\n%s", headers);

        while (mprGetBufLength(buf) > 0 && buf->start[0] && (buf->start[0] != '\r' && buf->start[0] != '\n')) {
            if ((key = getCgiToken(buf, ":")) == 0) {
                key = "Bad Header";
            }
            value = getCgiToken(buf, "\n");
            while (isspace((uchar) *value)) {
                value++;
            }
            len = (int) strlen(value);
            while (len > 0 && (value[len - 1] == '\r' || value[len - 1] == '\n')) {
                value[len - 1] = '\0';
                len--;
            }
            key = slower(key);

            if (strcmp(key, "location") == 0) {
                location = value;

            } else if (strcmp(key, "status") == 0) {
                httpSetStatus(conn, atoi(value));

            } else if (strcmp(key, "content-type") == 0) {
                httpSetHeaderString(conn, "Content-Type", value);

            } else if (strcmp(key, "content-length") == 0) {
                httpSetContentLength(conn, (MprOff) stoi(value));
                httpSetChunkSize(conn, 0);

            } else {
                /*
                    Now pass all other headers back to the client
                 */
                httpSetHeader(conn, key, "%s", value);
            }
        }
        buf->start = endHeaders;
    }
    cmd->userFlags |= MA_CGI_SEEN_HEADER;

    if (location) {
        httpRedirect(conn, tx->status, location);
        httpFinalize(conn);
        if (conn->state == HTTP_STATE_COMPLETE) {
            httpPump(conn, NULL);
        }
    }
    return 1;
}


/*
    Build the command arguments. NOTE: argv is untrusted input.
 */
static void buildArgs(HttpConn *conn, MprCmd *cmd, int *argcp, cchar ***argvp)
{
    HttpRx      *rx;
    HttpTx      *tx;
    char        **argv;
    char        *fileName, *indexQuery, *cp, *tok;
    cchar       *actionProgram;
    size_t      len;
    int         argc, argind, i;

    rx = conn->rx;
    tx = conn->tx;

    fileName = tx->filename;
    mprAssert(fileName);

    actionProgram = 0;
    argind = 0;
    argc = *argcp;

    if (tx->ext) {
        actionProgram = mprGetMimeProgram(rx->route->mimeTypes, tx->ext);
        if (actionProgram != 0) {
            argc++;
        }
        /*
            This is an Apache compatible hack for PHP 5.3
         */
        mprAddKey(rx->headers, "REDIRECT_STATUS", itos(HTTP_CODE_MOVED_TEMPORARILY));
    }

    /*
        Count the args for ISINDEX queries. Only valid if there is not a "=" in the query. 
        If this is so, then we must not have these args in the query env also?
     */
    indexQuery = rx->parsedUri->query;
    if (indexQuery && !strchr(indexQuery, '=')) {
        argc++;
        for (cp = indexQuery; *cp; cp++) {
            if (*cp == '+') {
                argc++;
            }
        }
    } else {
        indexQuery = 0;
    }

#if BIT_WIN_LIKE || VXWORKS
{
    char    *bangScript, *cmdBuf, *program, *cmdScript;

    /*
        On windows we attempt to find an executable matching the fileName.
        We look for *.exe, *.bat and also do unix style processing "#!/program"
     */
    findExecutable(conn, &program, &cmdScript, &bangScript, fileName);
    mprAssert(program);

    if (cmdScript) {
        /*
            Cmd/Batch script (.bat | .cmd)
            Convert the command to the form where there are 4 elements in argv
            that cmd.exe can interpret.

                argv[0] = cmd.exe
                argv[1] = /Q
                argv[2] = /C
                argv[3] = ""script" args ..."
         */
        argc = 4;

        len = (argc + 1) * sizeof(char*);
        argv = (char**) mprAlloc(len);
        memset(argv, 0, len);

        argv[argind++] = program;               /* Duped in findExecutable */
        argv[argind++] = "/Q";
        argv[argind++] = "/C";

        len = strlen(cmdScript) + 2 + 1;
        cmdBuf = mprAlloc(len);
        mprSprintf(cmdBuf, len, "\"%s\"", cmdScript);
        argv[argind++] = cmdBuf;

        mprSetCmdDir(cmd, cmdScript);
        /*  program will get freed when argv[] gets freed */
        
    } else if (bangScript) {
        /*
            Script used "#!/program". NOTE: this may be overridden by a mime
            Action directive.
         */
        argc++;     /* Adding bangScript arg */

        len = (argc + 1) * sizeof(char*);
        argv = (char**) mprAlloc(len);
        memset(argv, 0, len);

        argv[argind++] = program;       /* Will get freed when argv[] is freed */
        argv[argind++] = bangScript;    /* Will get freed when argv[] is freed */
        mprSetCmdDir(cmd, bangScript);

    } else {
        /*
            Either unknown extension or .exe (.out) program.
         */
        len = (argc + 1) * sizeof(char*);
        argv = (char**) mprAlloc(len);
        memset(argv, 0, len);
        if (actionProgram) {
            argv[argind++] = sclone(actionProgram);
        }
        argv[argind++] = program;
    }
}
#else
    len = (argc + 1) * sizeof(char*);
    argv = mprAlloc(len);
    memset(argv, 0, len);

    if (actionProgram) {
        argv[argind++] = sclone(actionProgram);
    }
    //  OPT - why clone all these string?
    argv[argind++] = sclone(fileName);
#endif
    /*
        ISINDEX queries. Only valid if there is not a "=" in the query. If this is so, then we must not
        have these args in the query env also?
        TODO - should query vars be set in the env?
     */
    if (indexQuery) {
        indexQuery = sclone(indexQuery);

        cp = stok(indexQuery, "+", &tok);
        while (cp) {
            argv[argind++] = mprEscapeCmd(mprUriDecode(cp), 0);
            cp = stok(NULL, "+", &tok);
        }
    }
    
    mprAssert(argind <= argc);
    argv[argind] = 0;
    *argcp = argc;
    *argvp = (cchar**) argv;

    mprLog(5, "CGI: command:");
    for (i = 0; i < argind; i++) {
        mprLog(5, "   argv[%d] = %s", i, argv[i]);
    }
}


#if BIT_WIN_LIKE || VXWORKS
/*
    If the program has a UNIX style "#!/program" string at the start of the file that program will be selected 
    and the original program will be passed as the first arg to that program with argv[] appended after that. If 
    the program is not found, this routine supports a safe intelligent search for the command. If all else fails, 
    we just return in program the fileName we were passed in. script will be set if we are modifying the program 
    to run and we have extracted the name of the file to run as a script.
 */
static void findExecutable(HttpConn *conn, char **program, char **script, char **bangScript, char *fileName)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    MprKey      *kp;
    MprFile     *file;
    cchar       *actionProgram, *ext, *cmdShell, *cp, *start;
    char        *tok, buf[MPR_MAX_FNAME + 1], *path;

    rx = conn->rx;
    tx = conn->tx;
    route = rx->route;

    *bangScript = 0;
    *script = 0;
    *program = 0;
    path = 0;

    actionProgram = mprGetMimeProgram(rx->route->mimeTypes, rx->mimeType);
    ext = tx->ext;

    /*
        If not found, go looking for the fileName with the extensions defined in appweb.conf. 
        NOTE: we don't use PATH deliberately!!!
     */
    if (access(fileName, X_OK) < 0) {
        for (kp = 0; (kp = mprGetNextKey(route->extensions, kp)) != 0; ) {
            path = sjoin(fileName, ".", kp->key, NULL);
            if (access(path, X_OK) == 0) {
                break;
            }
            path = 0;
        }
        if (kp) {
            ext = kp->key;
        } else {
            path = fileName;
        }

    } else {
        path = fileName;
    }
    mprAssert(path && *path);

#if BIT_WIN_LIKE
    if (ext && (strcmp(ext, ".bat") == 0 || strcmp(ext, ".cmd") == 0)) {
        /*
            Let a mime action override COMSPEC
         */
        if (actionProgram) {
            cmdShell = actionProgram;
        } else {
            cmdShell = getenv("COMSPEC");
        }
        if (cmdShell == 0) {
            cmdShell = "cmd.exe";
        }
        *script = sclone(path);
        *program = sclone(cmdShell);
        return;
    }
#endif

    if (actionProgram) {
        *program = sclone(actionProgram);

    } else if ((file = mprOpenFile(path, O_RDONLY, 0)) != 0) {
        if (mprReadFile(file, buf, MPR_MAX_FNAME) > 0) {
            mprCloseFile(file);
            buf[MPR_MAX_FNAME] = '\0';
            if (buf[0] == '#' && buf[1] == '!') {
                cp = start = &buf[2];
                cmdShell = stok(&buf[2], "\r\n", &tok);
                if (!mprIsPathAbs(cmdShell)) {
                    /*
                        If we can't access the command shell and the command is not an absolute path, 
                        look in the same directory as the script.
                     */
                    if (mprPathExists(cmdShell, X_OK)) {
                        cmdShell = mprJoinPath(mprGetPathDir(path), cmdShell);
                    }
                }
                *program = sclone(cmdShell);
                *bangScript = sclone(path);
                return;
            }
        } else {
            mprCloseFile(file);
        }
    }

    if (actionProgram) {
        *program = sclone(actionProgram);
        *bangScript = sclone(path);
    } else {
        *program = sclone(path);
    }
    return;
}
#endif
 

/*
    Get the next input token. The content buffer is advanced to the next token. This routine always returns a 
    non-zero token. The empty string means the delimiter was not found.
 */
static char *getCgiToken(MprBuf *buf, cchar *delim)
{
    char    *token, *nextToken;
    ssize   len;

    len = mprGetBufLength(buf);
    if (len == 0) {
        return "";
    }
    token = mprGetBufStart(buf);
    nextToken = scontains(mprGetBufStart(buf), delim, len);
    if (nextToken) {
        *nextToken = '\0';
        len = (int) strlen(delim);
        nextToken += len;
        buf->start = nextToken;

    } else {
        buf->start = mprGetBufEnd(buf);
    }
    return token;
}


#if BIT_DEBUG
/*
    Trace output received from the cgi process
 */
static void traceCGIData(MprCmd *cmd, char *src, ssize size)
{
    char    dest[512];
    int     index, i;

    mprRawLog(5, "@@@ CGI process wrote => \n");

    for (index = 0; index < size; ) { 
        for (i = 0; i < (sizeof(dest) - 1) && index < size; i++) {
            dest[i] = src[index];
            index++;
        }
        dest[i] = '\0';
        mprRawLog(5, "%s", dest);
    }
    mprRawLog(5, "\n");
}
#endif


static int copyVars(cchar **envv, int index, MprHash *vars, cchar *prefix)
{
    MprKey  *kp;
    char    *cp;

    for (ITERATE_KEYS(vars, kp)) {
        if (kp->data) {
            if (prefix) {
                cp = sjoin(prefix, kp->key, "=", (char*) kp->data, NULL);
            } else {
                cp = sjoin(kp->key, "=", (char*) kp->data, NULL);
            }
            envv[index] = cp;
            for (; *cp != '='; cp++) {
                if (*cp == '-') {
                    *cp = '_';
                } else {
                    *cp = toupper((uchar) *cp);
                }
            }
            index++;
        }
    }
    envv[index] = 0;
    return index;
}


static int actionDirective(MaState *state, cchar *key, cchar *value)
{
    char    *mimeType, *program;

    if (!maTokenize(state, value, "%S %S", &mimeType, &program)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    mprSetMimeProgram(state->route->mimeTypes, mimeType, program);
    return 0;
}


static int scriptAliasDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *route;
    char        *prefix, *path;

    if (!maTokenize(state, value, "%S %S", &prefix, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    route = httpCreateAliasRoute(state->route, prefix, path, 0);
    httpSetRouteHandler(route, "cgiHandler");
    httpSetRoutePattern(route, sfmt("^%s(.*)$", prefix), 0);
    httpSetRouteTarget(route, "run", "$1");
    httpFinalizeRoute(route);
    mprLog(4, "ScriptAlias \"%s\" for \"%s\"", prefix, path);
    return 0;
}


/*  
    Loadable module initialization
 */
int maCgiHandlerInit(Http *http, MprModule *module)
{
    HttpStage   *handler;
    MaAppweb    *appweb;

    if ((handler = httpCreateHandler(http, "cgiHandler", 0, module)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->cgiHandler = handler;
    handler->close = closeCgi; 
    handler->outgoingService = outgoingCgiService;
    handler->incoming = incomingCgi; 
    handler->open = openCgi; 
    handler->start = startCgi; 
#if BIT_WIN_LIKE
    handler->writable = writableCgi; 
#endif
    /*
        Add configuration file directives
     */
    appweb = httpGetContext(http);
    maAddDirective(appweb, "Action", actionDirective);
    maAddDirective(appweb, "ScriptAlias", scriptAliasDirective);
    return 0;
}

#else /* BIT_FEATURE_CGI */

int maCgiHandlerInit(Http *http, MprModule *module)
{
    mprNop(0);
    return 0;
}
#endif /* BIT_FEATURE_CGI */

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
