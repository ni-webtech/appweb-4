/* 
    cgiHandler.c -- Common Gateway Interface Handler

    Support the CGI/1.1 standard for external gateway programs to respond to HTTP requests.
    This CGI handler uses async-pipes and non-blocking I/O for all communications.
 
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/*********************************** Includes *********************************/

#include    "appweb.h"

#if BLD_FEATURE_CGI
/************************************ Locals ***********************************/

#define MA_CGI_SEEN_HEADER          0x1
#define MA_CGI_FLOW_CONTROL         0x2     /* Output to client is flow controlled */

/*********************************** Forwards *********************************/

static void buildArgs(HttpConn *conn, MprCmd *cmd, int *argcp, char ***argvp);
static void cgiCallback(MprCmd *cmd, int channel, void *data);
static int copyVars(MprCtx ctx, char **envv, int index, MprHashTable *vars, cchar *prefix);
static void enableCgiEvents(HttpQueue *q, MprCmd *cmd, int channel);
static char *getCgiToken(MprBuf *buf, cchar *delim);
static bool parseFirstCgiResponse(HttpConn *conn, MprCmd *cmd);
static bool parseHeader(HttpConn *conn, MprCmd *cmd);
static int processCgiData(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf);
static void pushDataToCgi(HttpQueue *q);
static void readCgiResponseData(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf);
static void startCgi(HttpQueue *q);

#if BLD_DEBUG
static void traceCGIData(MprCmd *cmd, char *src, int size);
#define traceData(cmd, src, size) traceCGIData(cmd, src, size)
#else
#define traceData(cmd, src, size)
#endif

#if BLD_WIN_LIKE || VXWORKS
static void findExecutable(HttpConn *conn, char **program, char **script, char **bangScript, char *fileName);
#endif
#if BLD_WIN_LIKE
static void checkCompletion(HttpQueue *q, MprEvent *event);
#endif

/************************************* Code ***********************************/
/*
    Open this handler instance for a new request
 */
static void openCgi(HttpQueue *q)
{
    HttpRx      *rx;
    HttpConn    *conn;

    conn = q->conn;
    rx = conn->rx;

    /*  
        If there is body content, it may arrive on another thread. Uclibc can't wait accross thread groups,
        so we must ensure that further callback events come on the same thread that creates the CGI process.
     */
#if __UCLIBC__
    if (rx->remainingContent > 0) {
        mprDedicateWorkerToDispatcher(conn->dispatcher, mprGetCurrentWorker(conn));
        mprAssert(conn->dispatcher->requiredWorker == mprGetCurrentWorker(conn));
    }
#endif
}


static void closeCgi(HttpQueue *q)
{
    mprDisconnectCmd((MprCmd*) q->queueData);
}


/*  
    Start the CGI command program. This commences the CGI gateway program. Body data from the client may flow 
    to the command and response data may be received back.
 */
static void startCgi(HttpQueue *q)
{
    HttpRx          *rx;
    HttpTx          *tx;
    HttpConn        *conn;
    MprCmd          *cmd;
    MprHashTable    *vars;
    cchar           *baseName;
    char            **argv, **envv, *fileName;
    int             argc, varCount, count;

    argv = 0;
    vars = 0;
    argc = 0;

    conn = q->conn;
    rx = conn->rx;
    tx = conn->tx;
    cmd = q->queueData = mprCreateCmd(rx, conn->dispatcher);

    argc = 1;                                   /* argv[0] == programName */
    buildArgs(conn, cmd, &argc, &argv);
    fileName = argv[0];

    baseName = mprGetPathBase(q, fileName);
    if (strncmp(baseName, "nph-", 4) == 0 || 
            (strlen(baseName) > 4 && strcmp(&baseName[strlen(baseName) - 4], "-nph") == 0)) {
        /* Pretend we've seen the header for Non-parsed Header CGI programs */
        cmd->userFlags |= MA_CGI_SEEN_HEADER;
    }
    /*  
        Build environment variables
     */
    vars = rx->formVars;
    varCount = mprGetHashCount(vars) + mprGetHashCount(rx->headers);
    if ((envv = (char**) mprAlloc(cmd, (varCount + 1) * sizeof(char*))) != 0) {
        count = copyVars(cmd, envv, 0, rx->formVars, "");
        count = copyVars(cmd, envv, count, rx->headers, "HTTP_");
        mprAssert(count <= varCount);
    }
    cmd->stdoutBuf = mprCreateBuf(cmd, HTTP_BUFSIZE, -1);
    cmd->stderrBuf = mprCreateBuf(cmd, HTTP_BUFSIZE, -1);

    mprSetCmdDir(cmd, mprGetPathDir(q, fileName));
    mprSetCmdCallback(cmd, cgiCallback, conn);

    if (mprStartCmd(cmd, argc, argv, envv, MPR_CMD_IN | MPR_CMD_OUT | MPR_CMD_ERR) < 0) {
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't run CGI process: %s, URI %s", fileName, rx->uri);
    }
}


#if __UCLIBC__
/*  
    WARNING: Block and wait for the CGI program to complete. Required for UCLIBC because waitpid won't work across threads.
    Wait for the CGI process to complete and collect status. Must run on the same thread as that initiated the CGI process.
 */
static void waitForCgiCompletion(HttpQueue *q)
{
    HttpTx      *tx;
    HttpConn    *conn;
    MprCmd      *cmd;
    int         rc;

    conn = q->conn;
    tx = conn->tx;
    cmd = (MprCmd*) q->queueData;

    /*  
        Because some platforms can't wait accross thread groups (uclibc) on the spawned child, the parent thread 
        must block here and wait for the child and collect exit status.
     */
    do {
        rc = mprWaitForCmd(cmd, 5 * 1000);
    } while (rc < 0 && (mprGetElapsedTime(cmd, cmd->lastActivity) <= conn->timeout || mprGetDebugMode(q)));

    if (cmd->pid) {
        mprStopCmd(cmd);
        cmd->status = 250;
    }

    if (cmd->status != 0) {
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE,
            "CGI process %s: exited abnormally with exit status: %d.\nSee the server log for more information.\n", 
            tx->filename, cmd->status);
    }
    httpFinalize(conn);
    if (conn->state == HTTP_STATE_COMPLETE) {
        httpAdvanceReceiver(conn, NULL);
    }
}
#endif


/*
    This routine runs after all incoming data has been received
    Must run on the same thread as that which initiated the CGI process. And must be called with the connection locked.
 */
static void processCgi(HttpQueue *q)
{
    HttpConn        *conn;
    MprCmd          *cmd;

    conn = q->conn;
    cmd = (MprCmd*) q->queueData;

    if (q->queueData) {
        /*  Close the CGI program's stdin. This will allow it to exit if it was expecting input data.  */
        mprCloseCmdFd(cmd, MPR_CMD_STDIN);
    }
#if __UCLIBC__
    if (conn->dispatcher->requiredWorker) {
        waitForCgiCompletion(q);
        mprAssert(cmd->pid == 0);
        mprAssert(conn->dispatcher->requiredWorker == mprGetCurrentWorker(conn));
        mprReleaseWorkerFromDispatcher(conn->dispatcher, mprGetCurrentWorker(conn));
    }
#endif
}


/*
    Service outgoing data destined for the browser. This is response data from the CGI program. See writeToClient.
    This may be called by {maRunPipeline|cgiCallback} => httpServiceQueues
 */ 
static void outgoingCgiService(HttpQueue *q)
{
    MprCmd      *cmd;

    cmd = (MprCmd*) q->queueData;

    /*
        This will copy outgoing packets downstream toward the network connector and on to the browser. 
        This may disable this queue if the downstream net connector queue overflows because the socket 
        is full. In that case, httpEnableConnEvents will setup to listen for writable events. When the 
        socket is writable again, the connector will drain its queue which will re-enable this queue 
        and schedule it for service again.
     */ 
    httpDefaultOutgoingServiceStage(q);
    if (cmd->userFlags & MA_CGI_FLOW_CONTROL && q->count < q->low) {
        cmd->userFlags &= ~MA_CGI_FLOW_CONTROL;
        mprEnableCmdEvents(cmd, MPR_CMD_STDOUT);
    }
}


/*
    Accept incoming body data from the client destined for the CGI gateway. This is typically POST or PUT data.
 */
static void incomingCgiData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpRx      *rx;
    MprCmd      *cmd;

    mprAssert(q);
    mprAssert(packet);
    
    conn = q->conn;
    tx = conn->tx;
    rx = conn->rx;

    cmd = (MprCmd*) q->pair->queueData;
    mprAssert(cmd);
    cmd->lastActivity = mprGetTime(cmd);

    if (httpGetPacketLength(packet) == 0) {
        /* End of input */
        if (rx->remainingContent > 0) {
            /* Short incoming body data. Just kill the CGI process.  */
            mprFree(cmd);
            q->queueData = 0;
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Client supplied insufficient body data");
        }
        httpAddVarsFromQueue(q);
    } else {
        /* No service routine, we just need it to be queued for pushDataToCgi */
        httpPutForService(q, packet, 0);
    }
    pushDataToCgi(q);
}


/*
    Write data to the CGI program. (may block). This is called from incomingCgiData and from the cgiCallback when the pipe
    to the CGI program becomes writable. Must be locked when called.
 */
static void pushDataToCgi(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;
    MprCmd      *cmd;
    MprBuf      *buf;
    int         len, rc;

    cmd = (MprCmd*) q->pair->queueData;
    mprAssert(cmd);
    conn = q->conn;

    for (packet = httpGetPacket(q); packet && conn->state < HTTP_STATE_ERROR; packet = httpGetPacket(q)) {
        buf = packet->content;
        len = mprGetBufLength(buf);
        mprAssert(len > 0);
        rc = mprWriteCmdPipe(cmd, MPR_CMD_STDIN, mprGetBufStart(buf), len);
        if (rc < 0) {
            mprLog(q, 2, "CGI: write to gateway failed for %d bytes, rc %d, errno %d\n", len, rc, mprGetOsError());
            mprCloseCmdFd(cmd, MPR_CMD_STDIN);
            httpError(conn, HTTP_CODE_BAD_GATEWAY, "Can't write body data to CGI gateway");
            break;

        } else {
            mprLog(q, 5, "CGI: write to gateway %d bytes asked to write %d\n", rc, len);
            mprAdjustBufStart(buf, rc);
            if (mprGetBufLength(buf) > 0) {
                httpPutBackPacket(q, packet);
            } else {
                httpFreePacket(q, packet);
            }
            if (rc < len) {
                /*
                    CGI gateway didn't accept all the data. Enable CGI write events to be notified when the gateway
                    can read more data.
                 */
                mprEnableCmdEvents(cmd, MPR_CMD_STDIN);
            }
        }
    }
}


/*
    Write data back to the client (browser). Must be locked when called.
 */
static int writeToClient(HttpQueue *q, MprCmd *cmd, MprBuf *buf, int channel)
{
    HttpConn    *conn;
    int         servicedQueues, rc, len;

    conn = q->conn;

    /*
        Write to the browser. We write as much as we can. Service queues to get the filters and connectors pumping.
     */
    for (servicedQueues = 0; (len = mprGetBufLength(buf)) > 0 ; ) {
        if (conn->state < HTTP_STATE_ERROR) {
            rc = httpWriteBlock(q, mprGetBufStart(buf), len, 0);
            mprLog(cmd, 5, "Write to browser ask %d, actual %d", len, rc);
        } else {
            rc = len;
        }
        if (rc > 0) {
            mprAdjustBufStart(buf, rc);
            mprResetBufIfEmpty(buf);

        } else if (rc == 0) {
            if (servicedQueues) {
                /*
                    Can't write anymore data. Block the CGI gateway. outgoingCgiService will enable.
                 */
                mprAssert(q->count >= q->max);
                mprAssert(q->flags & HTTP_QUEUE_DISABLED);
                cmd->userFlags |= MA_CGI_FLOW_CONTROL;
                mprDisableCmdEvents(cmd, channel);
                return MPR_ERR_CANT_WRITE;

            } else {
                httpServiceQueues(conn);
                servicedQueues++;
            }
        }
    }
    return 0;
}


/*
    Read the output data from the CGI script and return it to the client. This is called for stdout/stderr data from
    the CGI script and for EOF from the CGI's stdin.
    This event runs on the connections dispatcher. (ie. single threaded and safe)
 */
static void cgiCallback(MprCmd *cmd, int channel, void *data)
{
    HttpQueue   *q;
    HttpConn    *conn;
    HttpTx      *tx;

    conn = (HttpConn*) data;
    mprAssert(conn);
    mprAssert(conn->tx);

    mprLog(cmd, 6, "CGI callback channel %d", channel);
    
    tx = conn->tx;
    cmd->lastActivity = mprGetTime(cmd);
    q = conn->tx->queue[HTTP_QUEUE_TRANS].nextQ;

    switch (channel) {
    case MPR_CMD_STDIN:
        /* CGI's stdin is now accepting more data */
        //  MOB -- check this
        mprDisableCmdEvents(cmd, MPR_CMD_STDIN);
        pushDataToCgi(q->pair);
        enableCgiEvents(q, cmd, channel);
        break;

    case MPR_CMD_STDOUT:
        readCgiResponseData(q, cmd, channel, cmd->stdoutBuf);
        break;

    case MPR_CMD_STDERR:
        readCgiResponseData(q, cmd, channel, cmd->stderrBuf);
        break;
    }
    if (conn->state == HTTP_STATE_COMPLETE) {
        httpAdvanceReceiver(conn, NULL);
    }
}


/*
    Come here for CGI stdout, stderr events. ie. reading data from the CGI program.
 */
static void readCgiResponseData(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf)
{
    HttpConn    *conn;
    HttpTx      *tx;
    int         space, nbytes, err;

    conn = q->conn;
    tx = conn->tx;
    mprResetBufIfEmpty(buf);

    /*
        Read as much data from the CGI as possible
     */
    while (mprGetCmdFd(cmd, channel) >= 0 && conn->sock) {
        while ((space = mprGetBufSpace(buf)) > 0) {
            nbytes = mprReadCmdPipe(cmd, channel, mprGetBufEnd(buf), space);
            if (nbytes < 0) {
                err = mprGetError();
                if (err == EINTR) {
                    continue;
                } else if (err == EAGAIN || err == EWOULDBLOCK) {
                    break;
                }
                mprLog(cmd, 5, "CGI read error %d for %", mprGetError(), (channel == MPR_CMD_STDOUT) ? "stdout" : "stderr");
                mprCloseCmdFd(cmd, channel);
                break;
                
            } else if (nbytes == 0) {
                /*
                    This may reap the terminated child and thus clear cmd->process if both stderr and stdout are closed.
                 */
                mprLog(cmd, 5, "CGI EOF for %s", (channel == MPR_CMD_STDOUT) ? "stdout" : "stderr");
                mprCloseCmdFd(cmd, channel);
                if (cmd->pid == 0) {
                    httpFinalize(conn);
                }
                break;

            } else {
                mprLog(cmd, 5, "CGI read %d bytes from %s", nbytes, (channel == MPR_CMD_STDOUT) ? "stdout" : "stderr");
                mprAdjustBufEnd(buf, nbytes);
                traceData(cmd, mprGetBufStart(buf), nbytes);
                mprAddNullToBuf(buf);
            }
        }
        if (mprGetBufLength(buf) == 0 || processCgiData(q, cmd, channel, buf) < 0) {
            break;
        }
    }
    enableCgiEvents(q, cmd, channel);
}


static int processCgiData(HttpQueue *q, MprCmd *cmd, int channel, MprBuf *buf)
{
    HttpConn    *conn;

    conn = q->conn;

    if (channel == MPR_CMD_STDERR) {
        mprLog(conn, 4, mprGetBufStart(buf));
        if (writeToClient(q, cmd, buf, channel) < 0) {
            mprDisconnectSocket(conn->sock);
            return -1;
        }
        httpSetStatus(conn, HTTP_CODE_SERVICE_UNAVAILABLE);
        cmd->userFlags |= MA_CGI_SEEN_HEADER;
        cmd->status = 0;
    } else {
        if (!(cmd->userFlags & MA_CGI_SEEN_HEADER) && !parseHeader(conn, cmd)) {
            return -1;
        } 
        if (cmd->userFlags & MA_CGI_SEEN_HEADER) {
            if (writeToClient(q, cmd, buf, channel) < 0) {
                mprDisconnectSocket(conn->sock);
                return -1;
            }
        }
    }
    return 0;
}


static void enableCgiEvents(HttpQueue *q, MprCmd *cmd, int channel)
{
    if (cmd->pid == 0) {
        return;
    }
    if (channel == MPR_CMD_STDOUT && mprGetCmdFd(cmd, channel) < 0) {
        /*
            Now that stdout is complete, enable stderr to receive an EOF or any error output. This is 
            serialized to eliminate both stdin and stdout events on different threads at the same time.
         */
        mprLog(cmd, 8, "CGI enable stderr");
        mprEnableCmdEvents(cmd, MPR_CMD_STDERR);
        
    } else if (cmd->pid) {
        mprEnableCmdEvents(cmd, channel);
    }
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
    mprLog(conn, 4, "CGI status line: %s %s %s", protocol, status, message);
    return 1;
}


/*
    Parse the CGI output headers. 
    Sample CGI program:
 *
    Content-type: text/html
   
    <html.....
 */
static bool parseHeader(HttpConn *conn, MprCmd *cmd)
{
    HttpTx      *tx;
    HttpQueue   *q;
    MprBuf      *buf;
    char        *endHeaders, *headers, *key, *value, *location;
    int         len;

    tx = conn->tx;
    location = 0;
    value = 0;

    buf = mprGetCmdBuf(cmd, MPR_CMD_STDOUT);
    headers = mprGetBufStart(buf);

    /*
        Split the headers from the body.
     */
    len = 0;
    if ((endHeaders = strstr(headers, "\r\n\r\n")) == NULL) {
        if ((endHeaders = strstr(headers, "\n\n")) == NULL) {
            return 1;
        } 
        len = 2;
    } else {
        len = 4;
    }

    endHeaders[len - 1] = '\0';
    endHeaders += len;

    /*
        Want to be tolerant of CGI programs that omit the status line.
     */
    if (strncmp((char*) buf->start, "HTTP/1.", 7) == 0) {
        if (!parseFirstCgiResponse(conn, cmd)) {
            /* httpConnError already called */
            return 0;
        }
    }
    
    if (strchr(mprGetBufStart(buf), ':')) {
        mprLog(conn, 4, "CGI: parseHeader: header\n%s\n", headers);

        while (mprGetBufLength(buf) > 0 && buf->start[0] && (buf->start[0] != '\r' && buf->start[0] != '\n')) {

            if ((key = getCgiToken(buf, ":")) == 0) {
                httpConnError(conn, HTTP_CODE_BAD_REQUEST, "Bad header format");
                return 0;
            }
            value = getCgiToken(buf, "\n");
            while (isspace((int) *value)) {
                value++;
            }
            len = (int) strlen(value);
            while (len > 0 && (value[len - 1] == '\r' || value[len - 1] == '\n')) {
                value[len - 1] = '\0';
                len--;
            }
            mprStrLower(key);

            if (strcmp(key, "location") == 0) {
                location = value;

            } else if (strcmp(key, "status") == 0) {
                httpSetStatus(conn, atoi(value));

            } else if (strcmp(key, "content-type") == 0) {
                httpSetSimpleHeader(conn, "Content-Type", value);

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
        q = tx->queue[HTTP_QUEUE_TRANS].nextQ;
        httpFinalize(conn);
        if (conn->state == HTTP_STATE_COMPLETE) {
            httpAdvanceReceiver(conn, NULL);
        }
    }
    return 1;
}


/*
    Build the command arguments. NOTE: argv is untrusted input.
 */
static void buildArgs(HttpConn *conn, MprCmd *cmd, int *argcp, char ***argvp)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MaHost      *host;
    char        *fileName, **argv, *program, *cmdScript, status[8], *indexQuery, *cp, *tok;
    cchar       *actionProgram;
    int         argc, argind, len;

    rx = conn->rx;
    tx = conn->tx;
    host = httpGetConnHost(conn);

    fileName = tx->filename;
    mprAssert(fileName);

    program = cmdScript = 0;
    actionProgram = 0;
    argind = 0;
    argc = *argcp;

    if (rx->mimeType) {
        actionProgram = maGetMimeActionProgram(host, rx->mimeType);
        if (actionProgram != 0) {
            argc++;
        }
        /*
            This is an Apache compatible hack for PHP 5.3
         */
        mprItoa(status, sizeof(status), HTTP_CODE_MOVED_TEMPORARILY, 10);
        mprAddHash(rx->headers, "REDIRECT_STATUS", mprStrdup(rx, status));
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

#if BLD_WIN_LIKE || VXWORKS
{
    char    *bangScript, *cmdBuf;

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
         *
                argv[0] = cmd.exe
                argv[1] = /Q
                argv[2] = /C
                argv[3] = ""script" args ..."
         */
        argc = 4;

        len = (argc + 1) * sizeof(char*);
        argv = (char**) mprAlloc(cmd, len);
        memset(argv, 0, len);

        argv[argind++] = program;               /* Duped in findExecutable */
        argv[argind++] = mprStrdup(cmd, "/Q");
        argv[argind++] = mprStrdup(cmd, "/C");

        len = (int) strlen(cmdScript) + 2 + 1;
        cmdBuf = (char*) mprAlloc(cmd, len);
        mprSprintf(conn, cmdBuf, len, "\"%s\"", cmdScript);
        argv[argind++] = cmdBuf;

        mprSetCmdDir(cmd, cmdScript);
        mprFree(cmdScript);
        /*  program will get freed when argv[] gets freed */
        
    } else if (bangScript) {
        /*
            Script used "#!/program". NOTE: this may be overridden by a mime
            Action directive.
         */
        argc++;     /* Adding bangScript arg */

        len = (argc + 1) * sizeof(char*);
        argv = (char**) mprAlloc(cmd, len);
        memset(argv, 0, len);

        argv[argind++] = program;       /* Will get freed when argv[] is freed */
        argv[argind++] = bangScript;    /* Will get freed when argv[] is freed */
        mprSetCmdDir(cmd, bangScript);

    } else {
        /*
            Either unknown extension or .exe (.out) program.
         */
        len = (argc + 1) * sizeof(char*);
        argv = (char**) mprAlloc(cmd, len);
        memset(argv, 0, len);

        if (actionProgram) {
            argv[argind++] = mprStrdup(cmd, actionProgram);
        }
        argv[argind++] = program;
    }
}
#else
    len = (argc + 1) * sizeof(char*);
    argv = (char**) mprAlloc(cmd, len);
    memset(argv, 0, len);

    if (actionProgram) {
        argv[argind++] = mprStrdup(cmd, actionProgram);
    }
    argv[argind++] = mprStrdup(cmd, fileName);

#endif

    /*
        ISINDEX queries. Only valid if there is not a "=" in the query. If this is so, then we must not
        have these args in the query env also?
        TODO - should query vars be set in the env?
     */
    if (indexQuery) {
        indexQuery = mprStrdup(cmd, indexQuery);

        cp = mprStrTok(indexQuery, "+", &tok);
        while (cp) {
            argv[argind++] = mprEscapeCmd(tx, mprUriDecode(tx, cp), 0);
            cp = mprStrTok(NULL, "+", &tok);
        }
    }
    
    mprAssert(argind <= argc);
    argv[argind] = 0;
    *argcp = argc;
    *argvp = argv;
}


#if BLD_WIN_LIKE || VXWORKS
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
    HttpLoc     *loc;
    MprHash     *hp;
    MprFile     *file;
    cchar       *actionProgram, *ext, *cmdShell;
    char        *tok, buf[MPR_MAX_FNAME + 1], *path;

    rx = conn->rx;
    tx = conn->tx;
    loc = rx->loc;

    *bangScript = 0;
    *script = 0;
    *program = 0;
    path = 0;

    actionProgram = maGetMimeActionProgram(conn->host, rx->mimeType);
    ext = tx->extension;

    /*
        If not found, go looking for the fileName with the extensions defined in appweb.conf. 
        NOTE: we don't use PATH deliberately!!!
     */
    if (access(fileName, X_OK) < 0 && *ext == '\0') {
        for (hp = 0; (hp = mprGetNextHash(loc->extensions, hp)) != 0; ) {
            path = mprStrcat(tx, -1, fileName, ".", hp->key, NULL);
            if (access(path, X_OK) == 0) {
                break;
            }
            mprFree(path);
            path = 0;
        }
        if (hp) {
            ext = hp->key;
        } else {
            path = fileName;
        }

    } else {
        path = fileName;
    }
    mprAssert(path && *path);

#if BLD_WIN_LIKE
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
        *script = mprStrdup(tx, path);
        *program = mprStrdup(tx, cmdShell);
        return;
    }
#endif

    if ((file = mprOpen(tx, path, O_RDONLY, 0)) != 0) {
        if (mprRead(file, buf, MPR_MAX_FNAME) > 0) {
            mprFree(file);
            buf[MPR_MAX_FNAME] = '\0';
            if (buf[0] == '#' && buf[1] == '!') {
                cmdShell = mprStrTok(&buf[2], " \t\r\n", &tok);
                if (mprIsAbsPath(tx, cmdShell)) {
                    /*
                        If we can't access the command shell and the command is not an absolute path, 
                        look in the same directory as the script.
                     */
                    if (mprPathExists(tx, cmdShell, X_OK)) {
                        cmdShell = mprJoinPath(tx, mprGetPathDir(tx, path), cmdShell);
                    }
                }
                if (actionProgram) {
                    *program = mprStrdup(tx, actionProgram);
                } else {
                    *program = mprStrdup(tx, cmdShell);
                }
                *bangScript = mprStrdup(tx, path);
                return;
            }
        } else {
            mprFree(file);
        }
    }

    if (actionProgram) {
        *program = mprStrdup(tx, actionProgram);
        *bangScript = mprStrdup(tx, path);
    } else {
        *program = mprStrdup(tx, path);
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
    int     len;

    len = mprGetBufLength(buf);
    if (len == 0) {
        return "";
    }

    token = mprGetBufStart(buf);
    nextToken = mprStrnstr(mprGetBufStart(buf), delim, len);
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


#if BLD_DEBUG
/*
    Trace output received from the cgi process
 */
static void traceCGIData(MprCmd *cmd, char *src, int size)
{
    char    dest[512];
    int     index, i;

    mprRawLog(cmd, 5, "@@@ CGI process wrote => \n");

    for (index = 0; index < size; ) { 
        for (i = 0; i < (sizeof(dest) - 1) && index < size; i++) {
            dest[i] = src[index];
            index++;
        }
        dest[i] = '\0';
        mprRawLog(cmd, 5, "%s", dest);
    }
    mprRawLog(cmd, 5, "\n");
}
#endif


static int copyVars(MprCtx ctx, char **envv, int index, MprHashTable *vars, cchar *prefix)
{
    MprHash     *hp;
    char        *cp;

    for (hp = 0; (hp = mprGetNextHash(vars, hp)) != 0; ) {
        if (hp->data) {
            cp = envv[index] = mprStrcat(ctx, -1, prefix, hp->key, "=", (char*) hp->data, NULL);
            for (; *cp != '='; cp++) {
                if (*cp == '-') {
                    *cp = '_';
                } else {
                    *cp = toupper((int) *cp);
                }
            }
            index++;
        }
    }
    envv[index] = 0;
    return index;
}


static int parseCgi(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLoc     *loc;
    MaServer    *server;
    MaHost      *host;
    MaAlias     *alias;
    MaDir       *dir, *parent;
    char        *program, *mimeType, *prefix, *path;

    host = state->host;
    server = state->server;
    loc = state->loc;

    if (mprStrcmpAnyCase(key, "Action") == 0) {
        if (maSplitConfigValue(http, &mimeType, &program, value, 1) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        maSetMimeActionProgram(host, mimeType, program);
        return 1;

    } else if (mprStrcmpAnyCase(key, "ScriptAlias") == 0) {
        if (maSplitConfigValue(server, &prefix, &path, value, 1) < 0 || path == 0 || prefix == 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        /*
            Create an alias and location with a cgiHandler and pathInfo processing
         */
        path = maMakePath(host, path);
        dir = maLookupDir(host, path);
        if (maLookupDir(host, path) == 0) {
            parent = mprGetFirstItem(host->dirs);
            dir = maCreateDir(host, path, parent);
        }
        alias = maCreateAlias(host, prefix, path, 0);
        mprLog(server, 4, "ScriptAlias \"%s\" for \"%s\"", prefix, path);
        mprFree(path);

        maInsertAlias(host, alias);

        if ((loc = maLookupLocation(host, prefix)) == 0) {
            loc = httpCreateInheritedLocation(server->http, state->loc);
            httpSetLocationAuth(loc, state->dir->auth);
            httpSetLocationPrefix(loc, prefix);
            maAddLocation(host, loc);
        } else {
            httpSetLocationPrefix(loc, prefix);
        }
        httpSetHandler(loc, "cgiHandler");
        return 1;
    }
    return 0;
}


/*  Dynamic module initialization
 */
int maCgiHandlerInit(Http *http, MprModule *mp)
{
    HttpStage     *handler;

    handler = httpCreateHandler(http, "cgiHandler", 
        HTTP_STAGE_ALL | HTTP_STAGE_VARS | HTTP_STAGE_ENV_VARS | HTTP_STAGE_PATH_INFO | HTTP_STAGE_THREAD);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openCgi; 
    handler->close = closeCgi; 
    handler->outgoingService = outgoingCgiService;
    handler->incomingData = incomingCgiData; 
    handler->start = startCgi; 
    handler->process = processCgi; 
    handler->parse = (HttpParse) parseCgi; 
    return 0;
}
#else

int maCgiHandlerInit(Http *http, MprModule *mp)
{
    return 0;
}

#endif /* BLD_FEATURE_CGI */

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
