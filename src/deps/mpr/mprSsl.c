/*
    mprSsl.c -- Multithreaded Portable Runtime SSL Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.
 */


/************************************************************************/
/*
    Start of file "src/ssl/mprMatrixssl.c"
 */
/************************************************************************/

/*
    matrixSslModule.c -- Support for secure sockets via MatrixSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "buildConfig.h"

#if BLD_FEATURE_MATRIXSSL
 #include    "matrixsslApi.h"

#define     HAS_INT32 1
#define     HAS_UINT32 1
#define     HAS_INT64 1
#define     HAS_UINT64 1

#include    "mpr.h"

/************************************* Defines ********************************/

#define MPR_DEFAULT_SERVER_CERT_FILE    "server.crt"
#define MPR_DEFAULT_SERVER_KEY_FILE     "server.key.pem"
#define MPR_DEFAULT_CLIENT_CERT_FILE    "client.crt"
#define MPR_DEFAULT_CLIENT_CERT_PATH    "certs"

typedef struct MprMatrixSsl {
    sslKeys_t       *keys;
    sslSessionId_t  *session;
} MprMatrixSsl;


typedef struct MprMatrixSocket {
    MprSocket       *sock;
    MprMatrixSsl    *mssl;
    ssl_t           *handle;            /* MatrixSSL ssl_t structure */
    char            *outbuf;
    ssize           outlen;
    int             more;               /* MatrixSSL stack has buffered data */
} MprMatrixSocket;

/***************************** Forward Declarations ***************************/

static MprSocket *acceptMss(MprSocket *sp);
static void     closeMss(MprSocket *sp, bool gracefully);
static int      configureMss(MprSsl *ssl);
static int      connectMss(MprSocket *sp, cchar *host, int port, int flags);
static MprSocketProvider *createMatrixSslProvider();
static MprSocket *createMss(MprSsl *ssl);
static void     disconnectMss(MprSocket *sp);
static int      doHandshake(MprSocket *sp, short cipherSuite);
static ssize    flushMss(MprSocket *sp);
static MprSsl   *getDefaultMatrixSsl();
static ssize    innerRead(MprSocket *sp, char *userBuf, ssize len);
static int      listenMss(MprSocket *sp, cchar *host, int port, int flags);
static void     manageMatrixSocket(MprMatrixSocket *msp, int flags);
static void     manageMatrixProvider(MprSocketProvider *provider, int flags);
static void     manageMatrixSsl(MprMatrixSsl *mssl, int flags);
static ssize    readMss(MprSocket *sp, void *buf, ssize len);
static ssize    writeMss(MprSocket *sp, cvoid *buf, ssize len);

/************************************ Code ************************************/

int mprCreateMatrixSslModule(bool lazy)
{
    MprSocketProvider   *provider;

    /*
        Install this module as the SSL provider (can only have 1)
     */
    if ((provider = createMatrixSslProvider()) == 0) {
        return 0;
    }
    mprSetSecureProvider(provider);

    if (matrixSslOpen() < 0) {
        return 0;
    }
    if (!lazy) {
        getDefaultMatrixSsl();
    }
    return 0;
}


static MprSsl *getDefaultMatrixSsl()
{
    MprSocketService    *ss;
    MprSsl              *ssl;

    ss = MPR->socketService;
    if (ss->secureProvider->defaultSsl) {
        return ss->secureProvider->defaultSsl;
    }
    if ((ssl = mprCreateSsl()) == 0) {
        return 0;
    }
    ss->secureProvider->defaultSsl = ssl;

    if ((ssl->extendedSsl = mprAllocObj(MprMatrixSsl, manageMatrixSsl)) == 0) {
        return 0;
    }
    return ssl;
}


static MprSocketProvider *createMatrixSslProvider()
{
    MprSocketProvider   *provider;

    if ((provider = mprAllocObj(MprSocketProvider, manageMatrixProvider)) == NULL) {
        return 0;
    }
    provider->name = sclone("MatrixSsl");
    provider->acceptSocket = acceptMss;
    provider->closeSocket = closeMss;
    provider->configureSsl = configureMss;
    provider->connectSocket = connectMss;
    provider->createSocket = createMss;
    provider->disconnectSocket = disconnectMss;
    provider->flushSocket = flushMss;
    provider->listenSocket = listenMss;
    provider->readSocket = readMss;
    provider->writeSocket = writeMss;
    return provider;
}


static void manageMatrixProvider(MprSocketProvider *provider, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(provider->defaultSsl);
        mprMark(provider->name);
        mprMark(provider->data);
    }
}


static int configureMss(MprSsl *ssl)
{
    MprMatrixSsl    *ms;
    char            *password;

    ms = (MprMatrixSsl*) ssl->extendedSsl;

    /*
        Read the certificate and the key file for this server. FUTURE - If using encrypted private keys, 
        we should prompt through a dialog box or on the console, for the user to enter the password
        rather than using NULL as the password here.
     */
    password = NULL;
    mprAssert(ms->keys == NULL);

    if (matrixSslNewKeys(&ms->keys) < 0) {
        mprError("MatrixSSL: Could not read or decode certificate or key file."); 
        return MPR_ERR_CANT_INITIALIZE;
    }
    if (matrixSslLoadRsaKeys(ms->keys, ssl->certFile, ssl->keyFile, password, NULL) < 0) {
        mprError("MatrixSSL: Could not read or decode certificate or key file."); 
        return MPR_ERR_CANT_INITIALIZE;
    }

    /*
        Select the required protocols. MatrixSSL supports only SSLv3.
     */
    if (ssl->protocols & MPR_PROTO_SSLV2) {
        mprError("MatrixSSL: SSLv2 unsupported"); 
        return MPR_ERR_CANT_INITIALIZE;
    }
#if TODO
    if (!(ssl->protocols & MPR_PROTO_SSLV3)) {
        mprError("MatrixSSL: SSLv3 not enabled, unable to continue"); 
        return MPR_ERR_CANT_INITIALIZE;
    }
    //  AND 11
    if (ssl->protocols & MPR_PROTO_TLSV1) {
        mprLog(2, "MatrixSSL: Warning, TLSv1 not supported. Using SSLv3 only.");
    }
#endif
#if UNUSED
    if ((msp->session = mprAllocObj(sslSessionId_t, NULL)) == 0) {
        return 0;
    }
    mprMark(msp->session);
#endif
    return 0;
}


static void manageMatrixSsl(MprMatrixSsl *mssl, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;

    } else if (flags & MPR_MANAGE_FREE) {
        if (mssl->keys) {
            matrixSslDeleteKeys(mssl->keys);
            mssl->keys = 0;
        }
    }
}


/*
    Create a new Matrix socket
 */
static MprSocket *createMss(MprSsl *ssl)
{
    MprSocketService    *ss;
    MprSocket           *sp;
    MprMatrixSocket     *msp;
    
    //  MOB - what is this
    if (ssl == MPR_SECURE_CLIENT) {
        ssl = 0;
    }

    /*
        First get a standard socket
     */
    ss = MPR->socketService;
    sp = ss->standardProvider->createSocket(ssl);
    if (sp == 0) {
        return 0;
    }
    lock(sp);
    sp->provider = ss->secureProvider;

    if ((msp = (MprMatrixSocket*) mprAllocObj(MprMatrixSocket, manageMatrixSocket)) == 0) {
        return 0;
    }
    sp->sslSocket = msp;
    sp->ssl = ssl;
    msp->sock = sp;
    msp->mssl = ssl->extendedSsl;
#if UNUSED
    msp->ssl = ssl;
#endif
    unlock(sp);
    return sp;
}


static void manageMatrixSocket(MprMatrixSocket *msp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(msp->sock);
        mprMark(msp->mssl);
#if UNUSED
        mprMark(msp->ssl);
#endif

    } else if (flags & MPR_MANAGE_FREE) {
        if (msp->handle) {
            matrixSslDeleteSession(msp->handle);
        }
    }
}


/*
    Close a slock
 */
static void closeMss(MprSocket *sp, bool gracefully)
{
    MprMatrixSocket    *msp;
    uchar           *obuf;
    int             nbytes;

    mprAssert(sp);

    lock(sp);
    msp = sp->sslSocket;
    mprAssert(msp);

    if (!(sp->flags & MPR_SOCKET_EOF) && msp->handle) {
        /*
            Flush data. Append a closure alert to any buffered output data, and try to send it.
            Don't bother retrying or blocking, we're just closing anyway.
         */
        matrixSslEncodeClosureAlert(msp->handle);
        //  MOB RC
        nbytes = matrixSslGetOutdata(msp->handle, &obuf);
        if (nbytes > 0) {
            sp->service->standardProvider->writeSocket(sp, obuf, nbytes);
        }
    }
    sp->service->standardProvider->closeSocket(sp, gracefully);
    unlock(sp);
}


static int listenMss(MprSocket *sp, cchar *host, int port, int flags)
{
    return sp->service->standardProvider->listenSocket(sp, host, port, flags);
}


/*
    Called to accept an incoming connection request
 */
static MprSocket *acceptMss(MprSocket *listen)
{
    MprSocket           *sp;
    MprMatrixSocket     *msp;

    /*
        Do the standard accept stuff
     */
    sp = listen->service->standardProvider->acceptSocket(listen);
    if (sp == 0) {
        return 0;
    }

    lock(sp);
    msp = sp->sslSocket;
    mprAssert(msp);

    /* 
        Associate a new ssl session with this socket.  The session represents the state of the ssl protocol 
        over this socket.  Session caching is handled automatically by this api.
     */
    //  MOB - cert callback is NULL
    if (matrixSslNewServerSession(&msp->handle, msp->mssl->keys, NULL) < 0) {
        unlock(sp);
        return 0;
    }
#if UNUSED
    /* 
        MatrixSSL doesn't provide buffers for data internally. Define them here to support buffered reading and writing 
        for non-blocking sockets. 
     */
    msp->insock.size = MPR_SSL_BUFSIZE;
    msp->insock.start = mprAlloc(msp->insock.size);
    msp->insock.end = msp->insock.buf = msp->insock.start;

    msp->outsock.size = MPR_SSL_BUFSIZE;
    msp->outsock.start = mprAlloc(msp->outsock.size);
    msp->outsock.end = msp->outsock.buf = msp->outsock.start;

    msp->inbuf.size = 0;
    msp->inbuf.start = msp->inbuf.end = msp->inbuf.buf = 0;
    msp->outBufferCount = 0;
#endif
    unlock(sp);
    return sp;
}


/*
    Validate the certificate
 */
static int certValidator(ssl_t *ssl, psX509Cert_t *cert, int32 alert)
{
    //  See client.c - example
    return alert;

#if UNUSED
    psX509Cert_t   *next;
    
    /*
          Make sure we are checking the last cert in the chain
     */
    next = cert;
    while (next->next != NULL) {
        next = next->next;
    }
    
    /*
        Flag a non-authenticated server correctly. Call matrixSslGetAnonStatus later to 
        see the status of this connection.
     */
    if (next->verified != 1) {
        return SSL_ALLOW_ANON_CONNECTION;
    }
    return next->verified;
#endif
}


/*
    Connect as a client
 */
static int connectMss(MprSocket *sp, cchar *host, int port, int flags)
{
    MprSocketService    *ss;
    MprMatrixSocket     *msp;
    MprSsl              *ssl;
    int32               cipherSuite;
    
    lock(sp);
    ss = sp->service;

    if (sp->service->standardProvider->connectSocket(sp, host, port, flags) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_CONNECT;
    }

    msp = sp->sslSocket;
    mprAssert(msp);

    if (ss->secureProvider->defaultSsl == 0) {
        if ((ssl = getDefaultMatrixSsl()) == 0) {
            unlock(sp);
            return MPR_ERR_CANT_INITIALIZE;
        }
    } else {
        ssl = ss->secureProvider->defaultSsl;
    }
    msp->mssl = ssl->extendedSsl;
#if UNUSED
    msp->ssl = ssl;
#endif

    cipherSuite = 0;
    if (matrixSslNewClientSession(&msp->handle, msp->mssl->keys, NULL, cipherSuite, certValidator, NULL, NULL) < 0) {
        unlock(sp);
        return -1;
    }
    
#if UNUSED
    /*
        Configure the certificate validator and do the SSL handshake
     */
    matrixSslSetCertValidator(msp->handle, certValidator, NULL);
#endif
    
    if (doHandshake(sp, 0) < 0) {
        unlock(sp);
        return -1;
    }
    unlock(sp);
    return 0;
}


static void disconnectMss(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}


#if UNUSED
static int blockingWrite(MprSocket *sp, sslBuf_t *out)
{
    MprSocketProvider   *standard;
    uchar               *s;
    ssize               bytes;

    standard = sp->service->standardProvider;
    
    s = out->start;
    while (out->start < out->end) {
        bytes = standard->writeSocket(sp, out->start, (int) (out->end - out->start));
        if (bytes < 0) {
            return -1;
            
        } else if (bytes == 0) {
            mprNap(10);
        }
        out->start += bytes;
    }
    return (int) (out->start - s);
}
#endif


/*
    Construct the initial HELLO message to send to the server and initiate
    the SSL handshake. Can be used in the re-handshake scenario as well.
 */
static int doHandshake(MprSocket *sp, short cipherSuite)
{
    MprMatrixSocket        *msp;
    MprSocketProvider   *standard;
    char                buf[MPR_SSL_BUFSIZE];
    ssize               rc;
    uchar               *obuf;
    int                 toWrite, written;

    msp = sp->sslSocket;
    standard = sp->service->standardProvider;

#if UNUSED
    /*
        MatrixSSL doesn't provide buffers for data internally.  Define them here to support buffered reading 
        and writing for non-blocking sockets. Although it causes quite a bit more work, we support dynamically growing
        the buffers as needed.
     */
    msp->insock.size = MPR_SSL_BUFSIZE;
    msp->insock.start = msp->insock.end = msp->insock.buf = mprAlloc(msp->insock.size);
    msp->outsock.size = MPR_SSL_BUFSIZE;
    msp->outsock.start = msp->outsock.end = msp->outsock.buf = mprAlloc(msp->outsock.size);
    msp->inbuf.size = 0;
    msp->inbuf.start = msp->inbuf.end = msp->inbuf.buf = NULL;

    bytes = matrixSslEncodeClientHello(msp->handle, &msp->outsock, cipherSuite);
    if (bytes < 0) {
        mprAssert(bytes < 0);
        goto error;
    }
    /*
        Send the hello with a blocking write
     */
    if (blockingWrite(sp, &msp->outsock) < 0) {
        mprError("MatrixSSL: Error in socketWrite");
        goto error;
    }
    msp->outsock.start = msp->outsock.end = msp->outsock.buf;
#endif

    toWrite = matrixSslGetOutdata(msp->handle, &obuf);

    //  MOB - save & restore
    mprSetSocketBlockingMode(sp, 1);
    written = (int) standard->writeSocket(sp, obuf, toWrite);
    mprSetSocketBlockingMode(sp, 0);
    matrixSslSentData(msp->handle, written);

    /*
        Call sslRead to work through the handshake. Not actually expecting data back, so the finished case 
        is simply when the handshake is complete.
     */
readMore:
    rc = innerRead(sp, buf, sizeof(buf));
    
    /*
        Reading handshake records should always return 0 bytes, we aren't expecting any data yet.
     */
    if (rc == 0) {
        if (mprIsSocketEof(sp)) {
            goto error;
        }
#if KEEP
        if (status == SSLSOCKET_EOF || status == SSLSOCKET_CLOSE_NOTIFY) {
            goto error;
        }
#endif
        if (matrixSslHandshakeIsComplete(msp->handle) == 0) {
            goto readMore;
        }

    } else if (rc > 0) {
        mprError("MatrixSSL: sslRead got %d data in sslDoHandshake %s", rc, buf);
        goto readMore;

    } else {
        mprError("MatrixSSL: sslRead error in sslDoHandhake");
        goto error;
    }
    return 0;

error:
    return MPR_ERR_CANT_INITIALIZE;
}


/*
    Determine if there is buffered data available
 */
static bool isBufferedData(MprMatrixSocket *msp)
{
#if UNUSED
    if (msp->mssl == NULL) {
        return 0;
    }
    if (msp->inbuf.buf && msp->inbuf.start < msp->inbuf.end) {
        return 1;
    }
    if (msp->insock.start < msp->insock.end) {
        return 1;
    }
    return 0;
#else
    return msp->more;
#endif
}


static ssize process(MprSocket *sp, char *buf, ssize size, ssize nbytes, int *readMore)
{
    MprMatrixSocket        *msp;
    MprSocketProvider   *standard;
    uchar               *data, *obuf;
    ssize               toWrite, written, copied, sofar;
    uint32              dlen;
    int                 rc;

    msp = (MprMatrixSocket*) sp->sslSocket;
    standard = sp->service->standardProvider;
    sofar = 0;

    rc = matrixSslReceivedData(msp->handle, (int) nbytes, &data, &dlen);

    while (1) {
        switch (rc) {
        case MATRIXSSL_REQUEST_SEND:
            toWrite = matrixSslGetOutdata(msp->handle, &obuf);
            //  MOB - save & restore
            mprSetSocketBlockingMode(sp, 1);
            written = standard->writeSocket(sp, obuf, toWrite);
            mprSetSocketBlockingMode(sp, 0);
            if (written != toWrite) {
                return MPR_ERR_CANT_WRITE;
            }
            matrixSslSentData(msp->handle, (int) written);
            *readMore = 1;
            return 0;

        case MATRIXSSL_REQUEST_RECV:
            //  Partial 
            *readMore = 1;
            msp->more = 1;
            return 0;

        case MATRIXSSL_HANDSHAKE_COMPLETE:
            *readMore = 1;
            return 0;

        case MATRIXSSL_RECEIVED_ALERT:
            mprAssert(dlen == 2);
            if (data[0] == SSL_ALERT_LEVEL_FATAL) {
                return MPR_ERR;
            } else if (data[1] == SSL_ALERT_CLOSE_NOTIFY) {
                //  ignore - graceful close
                return 0;
            } else {
                //  ignore
            }
            rc = matrixSslProcessedData(msp->handle, &data, &dlen);
            break;

        case MATRIXSSL_APP_DATA:
            copied = min(dlen, size);
            msp->more = (dlen > size) ? 1 : 0;
            memcpy(buf, data, copied);
            buf += copied;
            size -= copied;
            data += copied;
            dlen = dlen - (int) copied;
            sofar += copied;
            if (!msp->more) {
                rc = matrixSslProcessedData(msp->handle, &data, &dlen);
                break;
            }
            return sofar;

        default:
            return MPR_ERR;
        }
    }
}


/*
    Return number of bytes read. Return -1 on errors and EOF
 */
static ssize innerRead(MprSocket *sp, char *buf, ssize size)
{
    MprMatrixSocket        *msp;
    MprSocketProvider   *standard;
    uchar               *mbuf;
    ssize               nbytes;
    int                 msize, readMore;

    msp = (MprMatrixSocket*) sp->sslSocket;
    standard = sp->service->standardProvider;
    readMore = 0;

    do {
        if ((msize = matrixSslGetReadbuf(msp->handle, &mbuf)) < 0) {
            return MPR_ERR_BAD_STATE;
        }
        if ((nbytes = standard->readSocket(sp, mbuf, msize)) < 0) {
            return nbytes;
        } else if (nbytes == 0) {
            if (isBufferedData(msp)) {
                //BRYAN - what to do here?
            } else {
                return nbytes;
            }
        } else {
            if ((nbytes = process(sp, buf, size, nbytes, &readMore)) > 0) {
                return nbytes;
            }
        }
    } while (readMore);
    return 0;

#if UNUSED
    MprMatrixSocket        *msp;
    MprSocketProvider   *standard;
    sslBuf_t            *inbuf;     /* Cached decoded plain text */
    sslBuf_t            *insock;    /* Cached ciphertext to socket */
    uchar               *buf, error, alertLevel, alertDescription;
    ssize               bytes, space, remaining;
    int                 rc, performRead;

    msp = (MprMatrixSocket*) sp->sslSocket;
    buf = (uchar*) userBuf;

    inbuf = &msp->inbuf;
    insock = &msp->insock;
    standard = sp->service->standardProvider;

    /*
        If inbuf is valid, then we have previously decoded data that must be returned, return as much as possible.  
        Once all buffered data is returned, free the inbuf.
     */
    if (inbuf->buf) {
        if (inbuf->start < inbuf->end) {
            remaining = (int) (inbuf->end - inbuf->start);
            bytes = min(len, remaining);
            memcpy(buf, inbuf->start, bytes);
            inbuf->start += bytes;
            return bytes;
        }
        inbuf->buf = NULL;
    }

    /*
        Pack the buffered socket data (if any) so that start is at zero.
     */
    if (insock->buf < insock->start) {
        if (insock->start == insock->end) {
            insock->start = insock->end = insock->buf;
        } else {
            memmove(insock->buf, insock->start, insock->end - insock->start);
            insock->end -= (insock->start - insock->buf);
            insock->start = insock->buf;
        }
    }
    performRead = 0;

    /*
        If we have data still in the buffer, we must process if we can without another read (incase there 
        is no more data to read and we block).
     */
    if (insock->end == insock->start) {
readMore:
        //
        //  Read up to as many bytes as there are remaining in the buffer.
        //
        if ((insock->end - insock->start) < insock->size) {
            performRead = 1;
            bytes = standard->readSocket(sp, insock->end, (insock->buf + insock->size) - insock->end);
            if (bytes <= 0 && (insock->end == insock->start)) {
                return bytes;
            }
            insock->end += bytes;
        }
    }

    /*
        Define a temporary sslBuf
     */
    inbuf->start = inbuf->end = inbuf->buf = mprAlloc(len);
    inbuf->size = (int) len;
decodeMore:
    /*
        Decode the data we just read from the socket
     */
    error = 0;
    alertLevel = 0;
    alertDescription = 0;

    rc = matrixSslDecode(msp->handle, insock, inbuf, &error, &alertLevel, &alertDescription);
    switch (rc) {

    /*
        Successfully decoded a record that did not return data or require a response.
     */
    case SSL_SUCCESS:
        if (insock->end > insock->start) {
            goto decodeMore;
        }
        return 0;

    /*
        Successfully decoded an application data record, and placed in tmp buf
     */
    case SSL_PROCESS_DATA:
        //
        //  Copy as much as we can from the temp buffer into the caller's buffer
        //  and leave the remainder in inbuf until the next call to read
        //
        space = (inbuf->end - inbuf->start);
        len = min(space, len);
        memcpy(buf, inbuf->start, len);
        inbuf->start += len;
        return len;

    /*
        We've decoded a record that requires a response into tmp If there is no data to be flushed in the out 
        buffer, we can write out the contents of the tmp buffer.  Otherwise, we need to append the data 
        to the outgoing data buffer and flush it out.
     */
    case SSL_SEND_RESPONSE:
        bytes = standard->writeSocket(sp, inbuf->start, inbuf->end - inbuf->start);
        inbuf->start += bytes;
        if (inbuf->start < inbuf->end) {
            mprSetSocketBlockingMode(sp, 1);
            while (inbuf->start < inbuf->end) {
                bytes = standard->writeSocket(sp, inbuf->start, inbuf->end - inbuf->start);
                if (bytes < 0) {
                    goto readError;
                }
                inbuf->start += bytes;
            }
            mprSetSocketBlockingMode(sp, 0);
        }
        inbuf->start = inbuf->end = inbuf->buf;
        if (insock->end > insock->start) {
            goto decodeMore;
        }
        return 0;

    /*
        There was an error decoding the data, or encoding the out buffer. There may be a response data in the out 
        buffer, so try to send. We try a single hail-mary send of the data, and then close the socket. Since we're 
        closing on error, we don't worry too much about a clean flush.
     */
    case SSL_ERROR:
        mprLog(4, "MatrixSSL: Closing on protocol error %d", error);
        if (inbuf->start < inbuf->end) {
            mprSetSocketBlockingMode(sp, 0);
            bytes = standard->writeSocket(sp, inbuf->start, inbuf->end - inbuf->start);
        }
        goto readError;

    /*
        We've decoded an alert.  The level and description passed into matrixSslDecode are filled in with the specifics.
     */
    case SSL_ALERT:
        if (alertDescription == SSL_ALERT_CLOSE_NOTIFY) {
            goto readZero;
        }
        mprLog(4, "MatrixSSL: Closing on client alert %d: %d", alertLevel, alertDescription);
        goto readError;

    /*
        We have a partial record, we need to read more data off the socket. If we have a completely full insock buffer,
        we'll need to grow it here so that we CAN read more data when called the next time.
     */
    case SSL_PARTIAL:
        if (insock->start == insock->buf && insock->end == (insock->buf + insock->size)) {
            if (insock->size > SSL_MAX_BUF_SIZE) {
                goto readError;
            }
            insock->size *= 2;
            insock->start = insock->buf = mprRealloc(insock->buf, insock->size);
            insock->end = insock->buf + (insock->size / 2);
        }
        if (!performRead) {
            performRead = 1;
            inbuf->buf = 0;
            goto readMore;
        }
        goto readZero;

    /*
        The out buffer is too small to fit the decoded or response data. Increase the size of the buffer and 
        call decode again.
     */
    case SSL_FULL:
        mprAssert(inbuf->start == inbuf->end);
        inbuf->size *= 2;
        if (inbuf->buf != buf) {
            inbuf->buf = 0;
        }
        inbuf->start = mprAlloc(inbuf->size);
        inbuf->end = inbuf->buf = inbuf->start;
        goto decodeMore;
    }

readZero:
    return 0;

readError:
    sp->flags |= MPR_SOCKET_EOF;
    return -1;
#endif
}


/*
    Return number of bytes read. Return -1 on errors and EOF.
 */
static ssize readMss(MprSocket *sp, void *buf, ssize len)
{
    MprMatrixSocket *msp;
    ssize           bytes;

    lock(sp);
    msp = (MprMatrixSocket*) sp->sslSocket;

    if (len <= 0) {
        unlock(sp);
        return -1;
    }
    bytes = innerRead(sp, buf, len);

    /*
        If there is more data buffered locally here, then ensure the select handler will recall us again even 
        if there is no more IO events
     */
    if (isBufferedData(msp)) {
        sp->flags |= MPR_SOCKET_PENDING;
        mprRecallWaitHandlerByFd(sp->fd);
    }
    unlock(sp);
    return bytes;
}


/*
    Write data. Return the number of bytes written or -1 on errors.

    Encode caller's data buffer into an SSL record and write to socket. The encoded data will always be 
    bigger than the incoming data because of the record header (5 bytes) and MAC (16 bytes MD5 / 20 bytes SHA1)
    This would be fine if we were using blocking sockets, but non-blocking presents an interesting problem.  Example:

        A 100 byte input record is encoded to a 125 byte SSL record
        We can send 124 bytes without blocking, leaving one buffered byte
        We can't return 124 to the caller because it's more than they requested
        We can't return 100 to the caller because they would assume all data
        has been written, and we wouldn't get re-called to send the last byte

    We handle the above case by returning 0 to the caller if the entire encoded record could not be sent. Returning 
    0 will prompt us to select this socket for write events, and we'll be called again when the socket is writable.  
    We'll use this mechanism to flush the remaining encoded data, ignoring the bytes sent in, as they have already 
    been encoded.  When it is completely flushed, we return the originally requested length, and resume normal 
    processing.
 */
static ssize writeMss(MprSocket *sp, cvoid *buf, ssize len)
{
    MprMatrixSocket     *msp;
    uchar               *obuf;
    ssize               nbytes, written, total;

    msp = (MprMatrixSocket*) sp->sslSocket;
    total = 0;

    while (len > 0 || msp->outlen > 0) {
        if ((nbytes = matrixSslGetOutdata(msp->handle, &obuf)) <= 0) {
            if (msp->outlen <= 0) {
                msp->outbuf = (char*) buf;
                msp->outlen = len;
                len = 0;
                nbytes = min(msp->outlen, SSL_MAX_PLAINTEXT_LEN);
                msp->outbuf += nbytes;
                msp->outlen -= nbytes;
            }
            if ((nbytes = matrixSslEncodeToOutdata(msp->handle, (uchar*) buf, (int) nbytes)) < 0) {
                return nbytes;
            }
        }
        if ((written = sp->service->standardProvider->writeSocket(sp, obuf, nbytes)) <= 0) {
            return written;
        } else {
            matrixSslSentData(msp->handle, (int) written);
            total += written;
        }
    }
    return total;
    
#if UNUSED
    MprMatrixSocket    *msp;
    sslBuf_t        *outsock;
    ssize           rc;

    lock(sp);

    msp = (MprMatrixSocket*) sp->sslSocket;
    outsock = &msp->outsock;

    if (len > SSL_MAX_PLAINTEXT_LEN) {
        len = SSL_MAX_PLAINTEXT_LEN;
    }

    /*
        Pack the buffered socket data (if any) so that start is at zero.
     */
    if (outsock->buf < outsock->start) {
        if (outsock->start == outsock->end) {
            outsock->start = outsock->end = outsock->buf;
        } else {
            memmove(outsock->buf, outsock->start, outsock->end - outsock->start);
            outsock->end -= (outsock->start - outsock->buf);
            outsock->start = outsock->buf;
        }
    }

    /*
        If there is buffered output data, the caller must be trying to send the same amount of data as last time.  
        We don't support sending additional data until the original buffered request has been completely sent.
     */
    if (msp->outBufferCount > 0 && len != msp->outBufferCount) {
        mprAssert(len != msp->outBufferCount);
        unlock(sp);
        return -1;
    }
    
    /*
        If we don't have buffered data, encode the caller's data
     */
    if (msp->outBufferCount == 0) {
retryEncode:
        rc = matrixSslEncode(msp->handle, (uchar*) buf, (int) len, outsock);
        switch (rc) {
        case SSL_ERROR:
            unlock(sp);
            return -1;

        case SSL_FULL:
            if (outsock->size > SSL_MAX_BUF_SIZE) {
                unlock(sp);
                return -1;
            }
            outsock->size *= 2;
            outsock->buf = mprRealloc(outsock->buf, outsock->size);
            outsock->end = outsock->buf + (outsock->end - outsock->start);
            outsock->start = outsock->buf;
            goto retryEncode;
        }
    }

    /*
        We've got data to send.  Try to write it all out.
     */
    rc = sp->service->standardProvider->writeSocket(sp, outsock->start, outsock->end - outsock->start);
    if (rc <= 0) {
        unlock(sp);
        return rc;
    }
    outsock->start += rc;

    /*
        If we wrote it all return the length, otherwise remember the number of bytes passed in, and return 0 
        to be called again later.
     */
    if (outsock->start == outsock->end) {
        msp->outBufferCount = 0;
        unlock(sp);
        return len;
    }
    msp->outBufferCount = (int) len;
    unlock(sp);
    return 0;
#endif
}


//  MOB - should this block?

static ssize flushMss(MprSocket *sp)
{
    MprMatrixSocket  *msp;

    msp = (MprMatrixSocket*) sp->sslSocket;

    if (msp->outlen > 0) {
        return writeMss(sp, NULL, 0);
    }
#if UNUSED
    if (msp->outsock.start < msp->outsock.end) {
        return sp->service->standardProvider->writeSocket(sp, msp->outsock.start, msp->outsock.end - msp->outsock.start);
    }
#endif
    return 0;
}

#else

int mprCreateMatrixSslModule() { return -1; }
#endif /* BLD_FEATURE_MATRIXSSL */

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

/************************************************************************/
/*
    Start of file "src/ssl/mprOpenssl.c"
 */
/************************************************************************/

/*
    mprOpenSsl.c - Support for secure sockets via OpenSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "mpr.h"

#if BLD_FEATURE_OPENSSL

/* Clashes with WinCrypt.h */
#undef OCSP_RESPONSE
#include    <openssl/ssl.h>
#include    <openssl/evp.h>
#include    <openssl/rand.h>
#include    <openssl/err.h>
#include    <openssl/dh.h>

/************************************* Defines ********************************/

#define MPR_DEFAULT_SERVER_CERT_FILE    "server.crt"
#define MPR_DEFAULT_SERVER_KEY_FILE     "server.key.pem"
#define MPR_DEFAULT_CLIENT_CERT_FILE    "client.crt"
#define MPR_DEFAULT_CLIENT_CERT_PATH    "certs"

typedef struct MprOpenSsl {
    SSL_CTX         *context;
    RSA             *rsaKey512;
    RSA             *rsaKey1024;
    DH              *dhKey512;
    DH              *dhKey1024;
} MprOpenSsl;


typedef struct MprOpenSocket {
    MprSocket       *sock;
#if UNUSED
    MprSsl          *ssl;
    MprOpenSsl      *ossl;
#endif
    SSL             *handle;
    BIO             *bio;
} MprOpenSocket;

typedef struct OpenSslLocks {
    MprMutex    **locks;
} OpenSslLocks;

static OpenSslLocks *olocks;

typedef struct RandBuf {
    MprTime     now;
    int         pid;
} RandBuf;

static int      numLocks;

struct CRYPTO_dynlock_value {
    MprMutex    *mutex;
};
typedef struct CRYPTO_dynlock_value DynLock;

/***************************** Forward Declarations ***************************/

static MprSocket *acceptOss(MprSocket *sp);
static void     closeOss(MprSocket *sp, bool gracefully);
static MprSsl   *getDefaultSslSettings();
static int      configureCertificateFiles(MprSsl *ssl, SSL_CTX *ctx, char *key, char *cert);
static int      configureOss(MprSsl *ssl);
static int      connectOss(MprSocket *sp, cchar *host, int port, int flags);
static MprSocketProvider *createOpenSslProvider();
static MprSocket *createOss(MprSsl *ssl);
static DH       *dhCallback(SSL *ssl, int isExport, int keyLength);
static void     disconnectOss(MprSocket *sp);
static ssize    flushOss(MprSocket *sp);
static int      listenOss(MprSocket *sp, cchar *host, int port, int flags);
static void     manageOpenSsl(MprOpenSsl *ossl, int flags);
static void     manageOpenSslLocks(OpenSslLocks *olocks, int flags);
static void     manageOpenProvider(MprSocketProvider *provider, int flags);
static void     manageOpenSocket(MprOpenSocket *ssp, int flags);
static ssize    readOss(MprSocket *sp, void *buf, ssize len);
static RSA      *rsaCallback(SSL *ssl, int isExport, int keyLength);
static int      verifyX509Certificate(int ok, X509_STORE_CTX *ctx);
static ssize    writeOss(MprSocket *sp, cvoid *buf, ssize len);

static DynLock  *sslCreateDynLock(const char *file, int line);
static void     sslDynLock(int mode, DynLock *dl, const char *file, int line);
static void     sslDestroyDynLock(DynLock *dl, const char *file, int line);
static void     sslStaticLock(int mode, int n, const char *file, int line);
static ulong    sslThreadId(void);

static DH       *get_dh512();
static DH       *get_dh1024();

/************************************* Code ***********************************/
/*
    Create the Openssl module. This is called only once
 */
int mprCreateOpenSslModule(bool lazy)
{
    MprSocketProvider   *provider;
    RandBuf             randBuf;
    int                 i;

    if ((olocks = mprAllocObj(OpenSslLocks, manageOpenSslLocks)) == 0) {
        return MPR_ERR_MEMORY;
    }

    /*
        Get some random bytes
     */
    randBuf.now = mprGetTime();
    randBuf.pid = getpid();
    RAND_seed((void*) &randBuf, sizeof(randBuf));

#if BLD_UNIX_LIKE
    mprLog(6, "OpenSsl: Before calling RAND_load_file");
    RAND_load_file("/dev/urandom", 256);
    mprLog(6, "OpenSsl: After calling RAND_load_file");
#endif

    /*
        Configure the global locks
     */
    numLocks = CRYPTO_num_locks();
    olocks->locks = mprAlloc(numLocks * sizeof(MprMutex*));
    for (i = 0; i < numLocks; i++) {
        olocks->locks[i] = mprCreateLock();
    }
    CRYPTO_set_id_callback(sslThreadId);
    CRYPTO_set_locking_callback(sslStaticLock);

    CRYPTO_set_dynlock_create_callback(sslCreateDynLock);
    CRYPTO_set_dynlock_destroy_callback(sslDestroyDynLock);
    CRYPTO_set_dynlock_lock_callback(sslDynLock);

#if !BLD_WIN_LIKE
    OpenSSL_add_all_algorithms();
#endif

    SSL_library_init();

    if ((provider = createOpenSslProvider()) == 0) {
        return MPR_ERR_MEMORY;
    }
    provider->data = olocks;
    mprSetSecureProvider(provider);
    if (!lazy) {
        getDefaultSslSettings();
    }
    return 0;
}


static void manageOpenSslLocks(OpenSslLocks *olocks, int flags)
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(olocks->locks);
        for (i = 0; i < numLocks; i++) {
            mprMark(olocks->locks[i]);
        }
    } else if (flags & MPR_MANAGE_FREE) {
        olocks->locks = 0;
    }
}


static MprSsl *getDefaultSslSettings()
{
    MprSocketService    *ss;
    MprSsl              *ssl;
    MprOpenSsl          *ossl;

    ss = MPR->socketService;

    if (ss->secureProvider->defaultSsl) {
        return ss->secureProvider->defaultSsl;
    }
    if ((ssl = mprCreateSsl()) == 0) {
        return 0;
    }
    ss->secureProvider->defaultSsl = ssl;

    /*
        Pre-generate keys that are slow to compute.
     */
    if ((ssl->extendedSsl = mprAllocObj(MprOpenSsl, manageOpenSsl)) == 0) {
        return 0;
    }
    ossl = ssl->extendedSsl;
    ossl->rsaKey512 = RSA_generate_key(512, RSA_F4, 0, 0);
    ossl->rsaKey1024 = RSA_generate_key(1024, RSA_F4, 0, 0);
    ossl->dhKey512 = get_dh512();
    ossl->dhKey1024 = get_dh1024();
    return ssl;
}


/*
    Initialize a provider structure for OpenSSL
 */
static MprSocketProvider *createOpenSslProvider()
{
    MprSocketProvider   *provider;

    if ((provider = mprAllocObj(MprSocketProvider, manageOpenProvider)) == NULL) {
        return 0;
    }
    provider->name = sclone("OpenSsl");
    provider->acceptSocket = acceptOss;
    provider->closeSocket = closeOss;
    provider->configureSsl = configureOss;
    provider->connectSocket = connectOss;
    provider->createSocket = createOss;
    provider->disconnectSocket = disconnectOss;
    provider->flushSocket = flushOss;
    provider->listenSocket = listenOss;
    provider->readSocket = readOss;
    provider->writeSocket = writeOss;
    return provider;
}


static void manageOpenProvider(MprSocketProvider *provider, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(provider->name);
        mprMark(provider->defaultSsl);
        mprMark(provider->data);
    }
}


/*
    Configure the SSL configuration. Called from connect or explicitly in server code
    to setup various SSL contexts. Appweb uses this from location.c.
 */
static int configureOss(MprSsl *ssl)
{
    MprSsl              *defaultSsl;
    MprOpenSsl          *ossl, *src;
    SSL_CTX             *context;
    uchar               resume[16];

    if ((context = SSL_CTX_new(SSLv23_method())) == 0) {
        mprError("OpenSSL: Unable to create SSL context"); 
        return MPR_ERR_CANT_CREATE;
    }

    SSL_CTX_set_app_data(context, (void*) ssl);
#if UNUSED
    SSL_CTX_set_quiet_shutdown(context, 1);
#endif
    SSL_CTX_sess_set_cache_size(context, 512);

    RAND_bytes(resume, sizeof(resume));
    SSL_CTX_set_session_id_context(context, resume, sizeof(resume));

    /*
        Configure the certificates
     */
#if FUTURE
    if (ssl->key || ssl->cert) {
        if (configureCertificates(ssl, context, ssl->key, ssl->cert) != 0) {
            SSL_CTX_free(context);
            return MPR_ERR_CANT_INITIALIZE;
        }
    } else 
#endif
    if (ssl->keyFile || ssl->certFile) {
        if (configureCertificateFiles(ssl, context, ssl->keyFile, ssl->certFile) != 0) {
            SSL_CTX_free(context);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }

    mprLog(4, "OpenSSL: Using ciphers %s", ssl->ciphers);
    SSL_CTX_set_cipher_list(context, ssl->ciphers);

    /*
        Configure the client verification certificate locations
     */
    if (ssl->verifyClient) {
        if (ssl->caFile == 0 && ssl->caPath == 0) {
            mprError("OpenSSL: Must define CA certificates if using client verification");
            SSL_CTX_free(context);
            return MPR_ERR_BAD_STATE;
        }
        if (ssl->caFile || ssl->caPath) {
            if ((!SSL_CTX_load_verify_locations(context, ssl->caFile, ssl->caPath)) ||
                    (!SSL_CTX_set_default_verify_paths(context))) {
                mprError("OpenSSL: Unable to set certificate locations"); 
                SSL_CTX_free(context);
                return MPR_ERR_CANT_ACCESS;
            }
            if (ssl->caFile) {
                STACK_OF(X509_NAME) *certNames;
                certNames = SSL_load_client_CA_file(ssl->caFile);
                if (certNames == 0) {
                    ;
                } else {
                    /*
                        Define the list of CA certificates to send to the client
                        before they send their client certificate for validation
                     */
                    SSL_CTX_set_client_CA_list(context, certNames);
                }
            }
        }

        mprLog(4, "OpenSSL: enable verification of client connections");

        if (ssl->caFile) {
            mprLog(4, "OpenSSL: Using certificates from %s", ssl->caFile);

        } else if (ssl->caPath) {
            mprLog(4, "OpenSSL: Using certificates from directory %s", ssl->caPath);
        }
        SSL_CTX_set_verify(context, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyX509Certificate);
        SSL_CTX_set_verify_depth(context, ssl->verifyDepth);

    } else {
        SSL_CTX_set_verify(context, SSL_VERIFY_NONE, verifyX509Certificate);
    }

    /*
        Define callbacks
     */
    SSL_CTX_set_tmp_rsa_callback(context, rsaCallback);
    SSL_CTX_set_tmp_dh_callback(context, dhCallback);

    SSL_CTX_set_options(context, SSL_OP_ALL);
#ifdef SSL_OP_NO_TICKET
    SSL_CTX_set_options(context, SSL_OP_NO_TICKET);
#endif
#ifdef SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
    SSL_CTX_set_options(context, SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
#endif
    SSL_CTX_set_mode(context, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_AUTO_RETRY);

    /*
        Select the required protocols
        Disable SSLv2 by default -- it is insecure.
     */
    SSL_CTX_set_options(context, SSL_OP_NO_SSLv2);
    if (!(ssl->protocols & MPR_PROTO_SSLV3)) {
        SSL_CTX_set_options(context, SSL_OP_NO_SSLv3);
        mprLog(4, "OpenSSL: Disabling SSLv3");
    }
    if (!(ssl->protocols & MPR_PROTO_TLSV1)) {
        SSL_CTX_set_options(context, SSL_OP_NO_TLSv1);
        mprLog(4, "OpenSSL: Disabling TLSv1");
    }

    /* 
        Ensure we generate a new private key for each connection
     */
    SSL_CTX_set_options(context, SSL_OP_SINGLE_DH_USE);

    if ((defaultSsl = getDefaultSslSettings()) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (ssl != defaultSsl) {
        if (!ssl->extendedSsl && (ssl->extendedSsl = mprAllocObj(MprOpenSsl, manageOpenSsl)) == 0) {
            return 0;
        }
        ossl = ssl->extendedSsl;
        src = defaultSsl->extendedSsl;
        ossl->rsaKey512 = src->rsaKey512;
        ossl->rsaKey1024 = src->rsaKey1024;
        ossl->dhKey512 = src->dhKey512;
        ossl->dhKey1024 = src->dhKey1024;
    } else {
        ossl = ssl->extendedSsl;
    }
    mprAssert(ossl);
    ossl->context = context;
    return 0;
}


/*
    Update the destructor for the MprSsl object. 
 */
static void manageOpenSsl(MprOpenSsl *ossl, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;

    } else if (flags & MPR_MANAGE_FREE) {
        if (ossl->context != 0) {
            SSL_CTX_free(ossl->context);
            ossl->context = 0;
        }
        if (ossl == MPR->socketService->secureProvider->defaultSsl->extendedSsl) {
            if (ossl->rsaKey512) {
                RSA_free(ossl->rsaKey512);
                ossl->rsaKey512 = 0;
            }
            if (ossl->rsaKey1024) {
                RSA_free(ossl->rsaKey1024);
                ossl->rsaKey1024 = 0;
            }
            if (ossl->dhKey512) {
                DH_free(ossl->dhKey512);
                ossl->dhKey512 = 0;
            }
            if (ossl->dhKey1024) {
                DH_free(ossl->dhKey1024);
                ossl->dhKey1024 = 0;
            }
        }
    }
}


/*
    Configure the SSL certificate information using key and cert files
 */
static int configureCertificateFiles(MprSsl *ssl, SSL_CTX *ctx, char *key, char *cert)
{
    mprAssert(ctx);

    if (cert == 0) {
        return 0;
    }

    if (cert && SSL_CTX_use_certificate_chain_file(ctx, cert) <= 0) {
        if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_ASN1) <= 0) {
            mprError("OpenSSL: Can't open certificate file: %s", cert); 
            return -1;
        }
    }
    key = (key == 0) ? cert : key;
    if (key) {
        if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
            /* attempt ASN1 for self-signed format */
            if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_ASN1) <= 0) {
                mprError("OpenSSL: Can't define private key file: %s", key); 
                return -1;
            }
        }
        if (!SSL_CTX_check_private_key(ctx)) {
            mprError("OpenSSL: Check of private key file failed: %s", key);
            return -1;
        }
    }
    return 0;
}


/*
    Create a new socket. If listen is set, this is a socket for an accepting connection.
 */
static MprSocket *createOss(MprSsl *ssl)
{
    MprSocketService    *ss;
    MprSocket           *sp;
    MprOpenSocket       *osp;
    
    if (ssl == MPR_SECURE_CLIENT) {
        ssl = 0;
    }
    /*
        First get a standard socket
     */
    ss = MPR->socketService;
    if ((sp = ss->standardProvider->createSocket(ssl)) == 0) {
        return 0;
    }
    lock(sp);
    sp->provider = ss->secureProvider;

    /*
        Create a SslSocket object for ssl state. This logically extends MprSocket.
     */
    if ((osp = (MprOpenSocket*) mprAllocObj(MprOpenSocket, manageOpenSocket)) == 0) {
        return 0;
    }
    osp->sock = sp;
    sp->sslSocket = osp;
    sp->ssl = ssl;
#if UNUSED
    osp->ossl = ssl->extendedSsl;
#endif
    unlock(sp);
    return sp;
}


/*
    Destructor for an MprOpenSocket object
 */
static void manageOpenSocket(MprOpenSocket *osp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(osp->sock);
#if MOB
        mprMark(osp->ossl);
#endif

    } else if (flags & MPR_MANAGE_FREE) {
        if (osp->handle) {
            SSL_set_shutdown(osp->handle, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
            SSL_free(osp->handle);
            osp->handle = 0;
        }
    }
}


static void closeOss(MprSocket *sp, bool gracefully)
{
    MprOpenSocket    *osp;

    osp = sp->sslSocket;
    SSL_free(osp->handle);
    osp->handle = 0;
    sp->service->standardProvider->closeSocket(sp, gracefully);
}


/*
    Initialize a new server-side connection. Called by listenOss and by acceptOss.
 */
static int listenOss(MprSocket *sp, cchar *host, int port, int flags)
{
    return sp->service->standardProvider->listenSocket(sp, host, port, flags);
}


/*
    Initialize a new server-side connection
 */
static MprSocket *acceptOss(MprSocket *listen)
{
    MprSocket       *sp;
    MprOpenSocket   *osp;
    MprOpenSsl      *ossl;
    BIO             *bioSock;
    SSL             *handle;

    if ((sp = listen->service->standardProvider->acceptSocket(listen)) == 0) {
        return 0;
    }
    lock(sp);
    osp = sp->sslSocket;
    mprAssert(osp);

    /*
        Create and configure the SSL struct
     */
    ossl = sp->ssl->extendedSsl;
    if ((handle = (SSL*) SSL_new(ossl->context)) == 0) {
        unlock(sp);
        return 0;
    }
    osp->handle = handle;
    SSL_set_app_data(handle, (void*) osp);

    /*
        Create a socket bio
     */
    bioSock = BIO_new_socket(sp->fd, BIO_NOCLOSE);
    mprAssert(bioSock);
    SSL_set_bio(handle, bioSock, bioSock);
    SSL_set_accept_state(handle);
    osp->bio = bioSock;
    unlock(sp);
    return sp;
}


/*
    Initialize a new client connection
 */
static int connectOss(MprSocket *sp, cchar *host, int port, int flags)
{
    MprSocketService    *ss;
    MprOpenSocket       *osp;
    MprOpenSsl          *ossl;
    MprSsl              *ssl;
    BIO                 *bioSock;
    int                 rc;
    
    lock(sp);
    ss = sp->service;
    if (ss->standardProvider->connectSocket(sp, host, port, flags) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_CONNECT;
    }
    osp = sp->sslSocket;
    mprAssert(osp);

    if (ss->secureProvider->defaultSsl == 0) {
        if ((ssl = getDefaultSslSettings()) == 0) {
            unlock(sp);
            return MPR_ERR_CANT_INITIALIZE;
        }
    } else {
        ssl = ss->secureProvider->defaultSsl;
    }
    sp->ssl = ssl;
#if UNUSED
    osp->ossl = ssl->extendedSsl;
#endif
    ossl = ssl->extendedSsl;

    if (ossl->context == 0 && configureOss(ssl) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_INITIALIZE;
    }

    /*
        Create and configure the SSL struct
     */
    osp->handle = (SSL*) SSL_new(ossl->context);
    mprAssert(osp->handle);
    if (osp->handle == 0) {
        unlock(sp);
        return MPR_ERR_CANT_INITIALIZE;
    }
    SSL_set_app_data(osp->handle, (void*) osp);

    /*
        Create a socket bio
     */
    bioSock = BIO_new_socket(sp->fd, BIO_NOCLOSE);
    mprAssert(bioSock);
    SSL_set_bio(osp->handle, bioSock, bioSock);

    //  TODO - should be calling SSL_set_connect_state(osp->handle);
    osp->bio = bioSock;

    /*
        Make the socket blocking while we connect
     */
    mprSetSocketBlockingMode(sp, 1);
    
    rc = SSL_connect(osp->handle);
    if (rc < 1) {
#if KEEP
        rc = SSL_get_error(osp->handle, rc);
        if (rc == SSL_ERROR_WANT_READ) {
            rc = SSL_connect(osp->handle);
        }
#endif
        unlock(sp);
        return MPR_ERR_CANT_CONNECT;
    }
    mprSetSocketBlockingMode(sp, 0);
    unlock(sp);
    return 0;
}


static void disconnectOss(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}



/*
    Return the number of bytes read. Return -1 on errors and EOF
 */
static ssize readOss(MprSocket *sp, void *buf, ssize len)
{
    MprOpenSocket    *osp;
    int             rc, error, retries, i;

    lock(sp);
    osp = (MprOpenSocket*) sp->sslSocket;
    mprAssert(osp);

    if (osp->handle == 0) {
        mprAssert(osp->handle);
        unlock(sp);
        return -1;
    }

    /*  
        Limit retries on WANT_READ. If non-blocking and no data, then this can spin forever.
     */
    retries = 5;
    for (i = 0; i < retries; i++) {
        rc = SSL_read(osp->handle, buf, (int) len);
        if (rc < 0) {
            char    ebuf[MPR_MAX_STRING];
            error = SSL_get_error(osp->handle, rc);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_CONNECT || error == SSL_ERROR_WANT_ACCEPT) {
                continue;
            }
            ERR_error_string_n(error, ebuf, sizeof(ebuf) - 1);
            mprLog(4, "SSL_read error %d, %s", error, ebuf);
        }
        break;
    }

#if DEBUG
    if (rc > 0 && !connTraced) {
        X509_NAME   *xSubject;
        X509        *cert;
        char        subject[260], issuer[260], peer[260];

        mprLog(4, "%d: OpenSSL Connected using: \"%s\"", sock, SSL_get_cipher(ssl));

        cert = SSL_get_peer_certificate(ssl);
        if (cert == 0) {
            mprLog(4, "%d: OpenSSL Details: client supplied no certificate", sock);

        } else {
            xSubject = X509_get_subject_name(cert);
            X509_NAME_oneline(xSubject, subject, sizeof(subject) -1);
            X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) -1);
            X509_NAME_get_text_by_NID(xSubject, NID_commonName, peer, sizeof(peer) - 1);
            mprLog(4, "%d: OpenSSL Subject %s", sock, subject);
            mprLog(4, "%d: OpenSSL Issuer: %s", sock, issuer);
            mprLog(4, "%d: OpenSSL Peer: %s", sock, peer);
            X509_free(cert);
        }
        connTraced = 1;
    }
#endif

    if (rc <= 0) {
        error = SSL_get_error(osp->handle, rc);
        if (error == SSL_ERROR_WANT_READ) {
            rc = 0;
        } else if (error == SSL_ERROR_WANT_WRITE) {
            mprNap(10);
            rc = 0;
        } else if (error == SSL_ERROR_ZERO_RETURN) {
            sp->flags |= MPR_SOCKET_EOF;
            rc = -1;
        } else if (error == SSL_ERROR_SYSCALL) {
            sp->flags |= MPR_SOCKET_EOF;
            rc = -1;
        } else if (error != SSL_ERROR_ZERO_RETURN) {
            /* SSL_ERROR_SSL */
            rc = -1;
            sp->flags |= MPR_SOCKET_EOF;
        }
    } else if (SSL_pending(osp->handle) > 0) {
        sp->flags |= MPR_SOCKET_PENDING;
        mprRecallWaitHandlerByFd(sp->fd);
    }
    unlock(sp);
    return rc;
}


/*
    Write data. Return the number of bytes written or -1 on errors.
 */
static ssize writeOss(MprSocket *sp, cvoid *buf, ssize len)
{
    MprOpenSocket    *osp;
    ssize           totalWritten;
    int             rc;

    lock(sp);
    osp = (MprOpenSocket*) sp->sslSocket;

    if (osp->bio == 0 || osp->handle == 0 || len <= 0) {
        mprAssert(0);
        unlock(sp);
        return -1;
    }
    totalWritten = 0;
    ERR_clear_error();

    do {
        rc = SSL_write(osp->handle, buf, (int) len);
        mprLog(7, "OpenSSL: written %d, requested len %d", rc, len);
        if (rc <= 0) {
            rc = SSL_get_error(osp->handle, rc);
            if (rc == SSL_ERROR_WANT_WRITE) {
                mprNap(10);
                continue;
                
            } else if (rc == SSL_ERROR_WANT_READ) {
                //  AUTO-RETRY should stop this
                mprAssert(0);
                unlock(sp);
                return -1;
            } else {
                unlock(sp);
                return -1;
            }
        }
        totalWritten += rc;
        buf = (void*) ((char*) buf + rc);
        len -= rc;
        mprLog(7, "OpenSSL: write: len %d, written %d, total %d, error %d", len, rc, totalWritten, 
            SSL_get_error(osp->handle, rc));

    } while (len > 0);

    unlock(sp);
    return totalWritten;
}


/*
    Called to verify X509 client certificates
 */
static int verifyX509Certificate(int ok, X509_STORE_CTX *xContext)
{
    X509            *cert;
    SSL             *handle;
    MprOpenSocket   *osp;
    MprSsl          *ssl;
    char            subject[260], issuer[260], peer[260];
    int             error, depth;
    
    subject[0] = issuer[0] = '\0';

    handle = (SSL*) X509_STORE_CTX_get_app_data(xContext);
    osp = (MprOpenSocket*) SSL_get_app_data(handle);
    ssl = osp->sock->ssl;

    if (!ssl->verifyClient) {
        return ok;
    }
    cert = X509_STORE_CTX_get_current_cert(xContext);
    depth = X509_STORE_CTX_get_error_depth(xContext);
    error = X509_STORE_CTX_get_error(xContext);

    if (X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject) - 1) < 0) {
        ok = 0;
    }

    /*
        TODO -- should compare subject name and host name. Need smart compare.
     */
    if (X509_NAME_oneline(X509_get_issuer_name(xContext->current_cert), issuer, sizeof(issuer) - 1) < 0) {
        ok = 0;
    }
    if (X509_NAME_get_text_by_NID(X509_get_subject_name(xContext->current_cert), NID_commonName, peer, 
                sizeof(peer) - 1) < 0) {
        ok = 0;
    }

    /*
        Customizers: add your own code here to validate client certificates
     */
    if (ok && ssl->verifyDepth < depth) {
        if (error == 0) {
            error = X509_V_ERR_CERT_CHAIN_TOO_LONG;
        }
        ok = 0;
    }
    if (error != 0) {
        mprAssert(!ok);
        /* X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY */
    }

#if KEEP
    switch (error) {
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_CERT_REJECTED:
    case X509_V_ERR_CERT_SIGNATURE_FAILURE:
    case X509_V_ERR_CERT_UNTRUSTED:
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    case X509_V_ERR_INVALID_CA:
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    default:
        ok = 0;
        break;
    }
#endif

    if (!ok) {
        mprLog(0, "OpenSSL: Certification failed: subject %s", subject);
        mprLog(4, "OpenSSL: Issuer: %s", issuer);
        mprLog(4, "OpenSSL: Peer: %s", peer);
        mprLog(4, "OpenSSL: Error: %d: %s", error, X509_verify_cert_error_string(error));
    } else {
        mprLog(0, "OpenSSL: Certificate verified: subject %s", subject);
        mprLog(4, "OpenSSL: Issuer: %s", issuer);
        mprLog(4, "OpenSSL: Peer: %s", peer);
    }
    return ok;
}


static ssize flushOss(MprSocket *sp)
{
#if KEEP
    MprOpenSocket    *osp;

    osp = (MprOpenSocket*) sp->sslSocket;

    mprAssert(0);
    return BIO_flush(osp->bio);
#endif
    return 0;
}

 
static ulong sslThreadId()
{
    return (long) mprGetCurrentOsThread();
}


static void sslStaticLock(int mode, int n, const char *file, int line)
{
    mprAssert(0 <= n && n < numLocks);

    if (olocks->locks) {
        if (mode & CRYPTO_LOCK) {
            mprLock(olocks->locks[n]);
        } else {
            mprUnlock(olocks->locks[n]);
        }
    }
}


static DynLock *sslCreateDynLock(const char *file, int line)
{
    DynLock     *dl;

    dl = mprAllocZeroed(sizeof(DynLock));
    dl->mutex = mprCreateLock(dl);
    mprHold(dl->mutex);
    return dl;
}


static void sslDestroyDynLock(DynLock *dl, const char *file, int line)
{
    if (dl->mutex) {
        mprRelease(dl->mutex);
        dl->mutex = 0;
    }
}


static void sslDynLock(int mode, DynLock *dl, const char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        mprLock(dl->mutex);
    } else {
        mprUnlock(dl->mutex);
    }
}


/*
    Used for ephemeral RSA keys
 */
static RSA *rsaCallback(SSL *handle, int isExport, int keyLength)
{
    MprSocket       *sp;
    MprOpenSocket   *osp;
    MprOpenSsl      *ossl;
    RSA             *key;

    osp = (MprOpenSocket*) SSL_get_app_data(handle);
    sp = osp->sock;
    mprAssert(sp);

    ossl = sp->ssl->extendedSsl;

    key = 0;
    switch (keyLength) {
    case 512:
        key = ossl->rsaKey512;
        break;

    case 1024:
    default:
        key = ossl->rsaKey1024;
    }
    return key;
}


/*
    Used for ephemeral DH keys
 */
static DH *dhCallback(SSL *handle, int isExport, int keyLength)
{
    MprSocket       *sp;
    MprOpenSocket   *osp;
    MprOpenSsl      *ossl;
    DH              *key;

    osp = (MprOpenSocket*) SSL_get_app_data(handle);
    sp = osp->sock;
    ossl = sp->ssl->extendedSsl;

    key = 0;
    switch (keyLength) {
    case 512:
        key = ossl->dhKey512;
        break;

    case 1024:
    default:
        key = ossl->dhKey1024;
    }
    return key;
}


/*
    openSslDh.c - OpenSSL DH get routines. Generated by openssl.
    Use bit gendh to generate new content.
 */
static DH *get_dh512()
{
    static unsigned char dh512_p[] = {
        0x8E,0xFD,0xBE,0xD3,0x92,0x1D,0x0C,0x0A,0x58,0xBF,0xFF,0xE4,
        0x51,0x54,0x36,0x39,0x13,0xEA,0xD8,0xD2,0x70,0xBB,0xE3,0x8C,
        0x86,0xA6,0x31,0xA1,0x04,0x2A,0x09,0xE4,0xD0,0x33,0x88,0x5F,
        0xEF,0xB1,0x70,0xEA,0x42,0xB6,0x0E,0x58,0x60,0xD5,0xC1,0x0C,
        0xD1,0x12,0x16,0x99,0xBC,0x7E,0x55,0x7C,0xE4,0xC1,0x5D,0x15,
        0xF6,0x45,0xBC,0x73,
    };

    static unsigned char dh512_g[] = {
        0x02,
    };

    DH *dh;

    if ((dh=DH_new()) == NULL) {
        return(NULL);
    }

    dh->p=BN_bin2bn(dh512_p,sizeof(dh512_p),NULL);
    dh->g=BN_bin2bn(dh512_g,sizeof(dh512_g),NULL);

    if ((dh->p == NULL) || (dh->g == NULL)) { 
        DH_free(dh); return(NULL); 
    }
    return dh;
}


static DH *get_dh1024()
{
    static unsigned char dh1024_p[] = {
        0xCD,0x02,0x2C,0x11,0x43,0xCD,0xAD,0xF5,0x54,0x5F,0xED,0xB1,
        0x28,0x56,0xDF,0x99,0xFA,0x80,0x2C,0x70,0xB5,0xC8,0xA8,0x12,
        0xC3,0xCD,0x38,0x0D,0x3B,0xE1,0xE3,0xA3,0xE4,0xE9,0xCB,0x58,
        0x78,0x7E,0xA6,0x80,0x7E,0xFC,0xC9,0x93,0x3A,0x86,0x1C,0x8E,
        0x0B,0xA2,0x1C,0xD0,0x09,0x99,0x29,0x9B,0xC1,0x53,0xB8,0xF3,
        0x98,0xA7,0xD8,0x46,0xBE,0x5B,0xB9,0x64,0x31,0xCF,0x02,0x63,
        0x0F,0x5D,0xF2,0xBE,0xEF,0xF6,0x55,0x8B,0xFB,0xF0,0xB8,0xF7,
        0xA5,0x2E,0xD2,0x6F,0x58,0x1E,0x46,0x3F,0x74,0x3C,0x02,0x41,
        0x2F,0x65,0x53,0x7F,0x1C,0x7B,0x8A,0x72,0x22,0x1D,0x2B,0xE9,
        0xA3,0x0F,0x50,0xC3,0x13,0x12,0x6C,0xD2,0x17,0xA9,0xA5,0x82,
        0xFC,0x91,0xE3,0x3E,0x28,0x8A,0x97,0x73,
    };

    static unsigned char dh1024_g[] = {
        0x02,
    };

    DH *dh;

    if ((dh=DH_new()) == NULL) return(NULL);
    dh->p=BN_bin2bn(dh1024_p,sizeof(dh1024_p),NULL);
    dh->g=BN_bin2bn(dh1024_g,sizeof(dh1024_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL)) {
        DH_free(dh); 
        return(NULL); 
    }
    return dh;
}

#else
int mprCreateOpenSslModule(bool lazy) { return -1; }
#endif /* BLD_FEATURE_OPENSSL */

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

/************************************************************************/
/*
    Start of file "src/ssl/mprSsl.c"
 */
/************************************************************************/

/**
    mprSsl.c -- Load and manage the SSL providers.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"

#if BLD_FEATURE_SSL
/************************************ Code ************************************/
/*
    Load the ssl provider
 */
static MprModule *loadSsl(bool lazy)
{
    MprModule   *mp;

    if (MPR->flags & MPR_SSL_PROVIDER_LOADED) {
        return mprLookupModule("sslModule");
    }
#if BLD_FEATURE_OPENSSL
    /*
        NOTE: preference given to open ssl if both are enabled
     */
    mprLog(4, "Loading OpenSSL module");
    if (mprCreateOpenSslModule(lazy) < 0) {
        return 0;
    }
#elif BLD_FEATURE_MATRIXSSL
    mprLog(4, "Loading MatrixSSL module");
    if (mprCreateMatrixSslModule(lazy) < 0) {
        return 0;
    }
#endif
    if ((mp = mprCreateModule("sslModule", NULL, NULL, NULL)) == 0) {
        return 0;
    }
    MPR->flags |= MPR_SSL_PROVIDER_LOADED;
    return mp;
}


MprModule *mprLoadSsl(bool lazy)
{
    return loadSsl(lazy);
}


/*
    Loadable module interface. 
 */
MprModule *mprSslInit(cchar *path)
{
    return loadSsl(1);
}


static void manageSsl(MprSsl *ssl, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ssl->key);
        mprMark(ssl->cert);
        mprMark(ssl->keyFile);
        mprMark(ssl->certFile);
        mprMark(ssl->caFile);
        mprMark(ssl->caPath);
        mprMark(ssl->ciphers);
        mprMark(ssl->extendedSsl);
    }
}


/*
    Create a new Ssl context object
 */
MprSsl *mprCreateSsl()
{
    MprSsl      *ssl;

    if ((ssl = mprAllocObj(MprSsl, manageSsl)) == 0) {
        return 0;
    }
    ssl->ciphers = sclone(MPR_DEFAULT_CIPHER_SUITE);
    ssl->protocols = MPR_PROTO_SSLV3 | MPR_PROTO_TLSV1;
    ssl->verifyDepth = 6;
    return ssl;
}


void mprConfigureSsl(MprSsl *ssl)
{
    MprSocketProvider   *provider;

    if (ssl == 0 || ssl->configured) {
        return;
    }
    provider = MPR->socketService->secureProvider;
    if (provider) {
        provider->configureSsl(ssl);
        ssl->configured = 1;
    } else {
        mprError("Secure socket provider not loaded");
    }
}


void mprSetSslCiphers(MprSsl *ssl, cchar *ciphers)
{
    mprAssert(ssl);
    
    ssl->ciphers = sclone(ciphers);
}


void mprSetSslKeyFile(MprSsl *ssl, cchar *keyFile)
{
    mprAssert(ssl);
    
    ssl->keyFile = sclone(keyFile);
}


void mprSetSslCertFile(MprSsl *ssl, cchar *certFile)
{
    mprAssert(ssl);
    
    ssl->certFile = sclone(certFile);
}


void mprSetSslCaFile(MprSsl *ssl, cchar *caFile)
{
    mprAssert(ssl);
    
    ssl->caFile = sclone(caFile);
}


void mprSetSslCaPath(MprSsl *ssl, cchar *caPath)
{
    mprAssert(ssl);
    
    ssl->caPath = sclone(caPath);
}


void mprSetSslProtocols(MprSsl *ssl, int protocols)
{
    ssl->protocols = protocols;
}


#if UNUSED
void mprSetSocketSslConfig(MprSocket *sp, MprSsl *ssl)
{
    if (sp->sslSocket) {
        sp->sslSocket->ssl = ssl;
    }
}
#endif


void mprVerifySslClients(MprSsl *ssl, bool on)
{
    ssl->verifyClient = on;
}


#else /* SSL */

/*
    Stubs
 */
MprModule *mprLoadSsl(bool lazy)
{
    return 0;
}

MprModule *mprSslInit(cchar *path)
{
    return 0;
}


MprSsl *mprCreateSsl()
{
    return 0;
}


void mprConfigureSsl(MprSsl *ssl)
{
}


void mprSetSslCiphers(MprSsl *ssl, cchar *ciphers)
{
}


void mprSetSslKeyFile(MprSsl *ssl, cchar *keyFile)
{
}


void mprSetSslCertFile(MprSsl *ssl, cchar *certFile)
{
}


void mprSetSslCaFile(MprSsl *ssl, cchar *caFile)
{
}


void mprSetSslCaPath(MprSsl *ssl, cchar *caPath)
{
}


void mprSetSslProtocols(MprSsl *ssl, int protocols)
{
}


void mprSetSocketSslConfig(MprSocket *sp, MprSsl *ssl)
{
}


void mprVerifySslClients(MprSsl *ssl, bool on)
{
}


#endif /* SSL */


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
