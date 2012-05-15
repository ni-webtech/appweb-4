/*
    mprSsl.c -- Multithreaded Portable Runtime SSL Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.
 */


/************************************************************************/
/*
    Start of file "src/mprMatrixssl.c"
 */
/************************************************************************/

/*
    mprMatrixssl.c -- Support for secure sockets via MatrixSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "bit.h"

#if BIT_FEATURE_MATRIXSSL
#if WIN32
 #include   <winsock2.h>
 #include   <windows.h>
#endif
 #include    "matrixsslApi.h"

/*
    Matrixssl defines int32, uint32, int64 and uint64. Disable these in the mpr
 */
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

/*
    Per SSL configuration structure
 */
typedef struct MprMatrixSsl {
    sslKeys_t       *keys;
    sslSessionId_t  *session;
} MprMatrixSsl;

/*
    Per socket extended state
 */
typedef struct MprMatrixSocket {
    MprSocket       *sock;
    ssl_t           *handle;            /* MatrixSSL ssl_t structure */
    char            *outbuf;            /* Pending output data */
    ssize           outlen;             /* Length of outlen */
    ssize           written;            /* Number of unencoded bytes written */
    int             more;               /* MatrixSSL stack has buffered data */
} MprMatrixSocket;

/*
    Empty CA cert.
 */
static uchar CAcertSrvBuf[] = { 
    48, 130, 2, 144, 48, 130, 1, 249, 160, 3, 2, 1, 2,
    2, 1, 31, 48, 13, 6, 9, 42, 134, 72, 134, 247, 13,
    1, 1, 5, 5, 0, 48, 129, 129, 49, 35, 48, 33, 6,
    3, 85, 4, 3, 12, 26, 77, 97, 116, 114, 105, 120, 83,
    83, 76, 32, 83, 97, 109, 112, 108, 101, 32, 83, 101, 114,
    118, 101, 114, 32, 67, 65, 49, 11, 48, 9, 6, 3, 85,
    4, 6, 12, 2, 85, 83, 49, 11, 48, 9, 6, 3, 85,
    4, 8, 12, 2, 87, 65, 49, 16, 48, 14, 6, 3, 85,
    4, 7, 12, 7, 83, 101, 97, 116, 116, 108, 101, 49, 31,
    48, 29, 6, 3, 85, 4, 10, 12, 22, 80, 101, 101, 114, 
    83, 101, 99, 32, 78, 101, 116, 119, 111, 114, 107, 115, 44,
    32, 73, 110, 99, 46, 49, 13, 48, 11, 6, 3, 85, 4,
    11, 12, 4, 84, 101, 115, 116, 48, 30, 23, 13, 49, 48,
    48, 51, 48, 50, 49, 55, 49, 57, 48, 55, 90, 23, 13, 
    49, 51, 48, 51, 48, 49, 49, 55, 49, 57, 48, 55, 90,
    48, 129, 129, 49, 35, 48, 33, 6, 3, 85, 4, 3, 12,
    26, 77, 97, 116, 114, 105, 120, 83, 83, 76, 32, 83, 97,
    109, 112, 108, 101, 32, 83, 101, 114, 118, 101, 114, 32, 67,
    65, 49, 11, 48, 9, 6, 3, 85, 4, 6, 12, 2, 85,
    83, 49, 11, 48, 9, 6, 3, 85, 4, 8, 12, 2, 87,
    65, 49, 16, 48, 14, 6, 3, 85, 4, 7, 12, 7, 83,
    101, 97, 116, 116, 108, 101, 49, 31, 48, 29, 6, 3, 85, 
    4, 10, 12, 22, 80, 101, 101, 114, 83, 101, 99, 32, 78,
    101, 116, 119, 111, 114, 107, 115, 44, 32, 73, 110, 99, 46, 
    49, 13, 48, 11, 6, 3, 85, 4, 11, 12, 4, 84, 101,
    115, 116, 48, 129, 159, 48, 13, 6, 9, 42, 134, 72, 134,
    247, 13, 1, 1, 1, 5, 0, 3, 129, 141, 0, 48, 129,
    137, 2, 129, 129, 0, 161, 37, 100, 65, 3, 153, 4, 51,
    190, 127, 211, 25, 110, 88, 126, 201, 223, 171, 11, 203, 100,
    10, 206, 214, 152, 173, 187, 96, 24, 202, 103, 225, 54, 18,
    236, 171, 140, 125, 209, 68, 184, 212, 195, 191, 210, 98, 123,
    237, 205, 213, 23, 209, 245, 108, 70, 236, 20, 25, 140, 68,
    255, 68, 53, 29, 23, 231, 200, 206, 199, 45, 74, 46, 86, 
    10, 229, 243, 126, 72, 33, 184, 21, 16, 76, 55, 28, 97, 
    3, 79, 192, 236, 241, 47, 12, 220, 59, 158, 29, 243, 133,
    125, 83, 30, 232, 243, 131, 16, 217, 8, 2, 185, 80, 197,
    139, 49, 11, 191, 233, 160, 55, 73, 239, 43, 151, 12, 31,
    151, 232, 145, 2, 3, 1, 0, 1, 163, 22, 48, 20, 48,
    18, 6, 3, 85, 29, 19, 1, 1, 1, 4, 8, 48, 6,
    1, 1, 1, 2, 1, 1, 48, 13, 6, 9, 42, 134, 72,
    134, 247, 13, 1, 1, 5, 5, 0, 3, 129, 129, 0, 11,
    36, 231, 0, 168, 211, 179, 52, 55, 21, 63, 125, 230, 86,
    116, 204, 101, 189, 20, 132, 242, 238, 119, 80, 44, 174, 74,
    125, 81, 98, 111, 223, 242, 96, 20, 192, 210, 7, 248, 91,
    101, 37, 85, 46, 0, 132, 72, 12, 21, 100, 132, 5, 122,
    170, 109, 207, 114, 159, 182, 95, 106, 103, 135, 80, 158, 99, 
    44, 126, 86, 191, 79, 126, 249, 99, 3, 18, 40, 128, 21,
    127, 185, 53, 51, 128, 20, 24, 249, 23, 90, 5, 171, 46,
    164, 3, 39, 94, 98, 103, 213, 169, 59, 10, 10, 74, 205,
    201, 16, 116, 204, 66, 111, 187, 36, 87, 144, 81, 194, 26,
    184, 76, 67, 209, 132, 6, 150, 183, 119, 59
};

/***************************** Forward Declarations ***************************/

static MprSocket *acceptMss(MprSocket *sp);
static void     closeMss(MprSocket *sp, bool gracefully);
static int      configureMss(MprSsl *ssl);
static int      connectMss(MprSocket *sp, cchar *host, int port, int flags);
static MprMatrixSsl *createMatrixSsl(MprSsl *ssl);
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
static ssize    processMssData(MprSocket *sp, char *buf, ssize size, ssize nbytes, int *readMore);
static ssize    readMss(MprSocket *sp, void *buf, ssize len);
static ssize    writeMss(MprSocket *sp, cvoid *buf, ssize len);

/************************************ Code ************************************/

int mprCreateMatrixSslModule(bool lazy)
{
    MprSocketProvider   *provider;

    /*
        Install this module as the SSL provider (can only have one)
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


/*
    Create the default MatrixSSL configuration state structure.
    This is used for client connections and for server connections in the absense of a per-route configuration.
 */
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
    if (!createMatrixSsl(ssl)) {
        return 0;
    }
    ss->secureProvider->defaultSsl = ssl;
    return ssl;
}


static MprMatrixSsl *createMatrixSsl(MprSsl *ssl)
{
    MprMatrixSsl    *mssl;

    if ((mssl = mprAllocObj(MprMatrixSsl, manageMatrixSsl)) == 0) {
        return 0;
    }
    ssl->extendedSsl = mssl;
    if (matrixSslNewKeys(&mssl->keys) < 0) {
        return 0;
    }
    return mssl;
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


/*
    Initialize a server-side SSL configuration. An application can have multiple different SSL configurations
    for different routes.
 */
static int configureMss(MprSsl *ssl)
{
    MprMatrixSsl    *mssl;
    char            *password;

    mprAssert(ssl);

    if ((mssl = createMatrixSsl(ssl)) == 0) {
        return 0;
    }
    /*
        Read the certificate and the key file for this server. FUTURE - If using encrypted private keys, 
        we could prompt through a dialog box or on the console, for the user to enter the password
        rather than using NULL as the password here.
     */
    password = NULL;
    if (matrixSslLoadRsaKeys(mssl->keys, ssl->certFile, ssl->keyFile, password, NULL) < 0) {
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
#if UNUSED
    //  MOB - what is this
    if ((mssl->session = mprAllocObj(sslSessionId_t, NULL)) == 0) {
        return 0;
    }
    mprMark(mssl->session);
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
    
    if (ssl == MPR_SECURE_CLIENT) {
        /* Use the default SSL provider and configuration */
        ssl = 0;
    }
    /*
        First get a standard socket
     */
    ss = MPR->socketService;
    if ((sp = ss->standardProvider->createSocket(ssl)) == 0) {
        return 0;
    }
    sp->provider = ss->secureProvider;
    if ((msp = (MprMatrixSocket*) mprAllocObj(MprMatrixSocket, manageMatrixSocket)) == 0) {
        return 0;
    }
    msp->sock = sp;
    sp->sslSocket = msp;
    sp->ssl = ssl;
    mprAddItem(ss->secureSockets, sp);
    return sp;
}


static void manageMatrixSocket(MprMatrixSocket *msp, int flags)
{
    MprSocketService    *ss;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(msp->sock);

    } else if (flags & MPR_MANAGE_FREE) {
        if (msp->handle) {
            matrixSslDeleteSession(msp->handle);
        }
        ss = MPR->socketService;
        mprRemoveItem(ss->secureSockets, msp->sock);
    }
}


/*
    Close the socket
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
        if ((nbytes = matrixSslGetOutdata(msp->handle, &obuf)) > 0) {
            /* Ignore return */
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
    MprMatrixSsl        *mssl;

    /*
        Do the standard accept stuff
     */
    if ((sp = listen->service->standardProvider->acceptSocket(listen)) == 0) {
        return 0;
    }
    /* 
        Associate a new ssl session with this socket. The session represents the state of the ssl protocol 
        over this socket. Session caching is handled automatically by this api.
     */
    lock(sp);
    msp = sp->sslSocket;
    mprAssert(msp);
    mssl = sp->ssl->extendedSsl;
    if (matrixSslNewServerSession(&msp->handle, mssl->keys, NULL) < 0) {
        unlock(sp);
        return 0;
    }
    unlock(sp);
    return sp;
}


/*
    Validate the server certificate
    UGLY: really need a MprMatrixSsl handle here
 */
static int verifyServer(ssl_t *ssl, psX509Cert_t *cert, int32 alert)
{
    MprSocketService    *ss;
    MprSocket           *sp;
    struct tm           t;
    char                *c;
    int                 next, y, m, d;

    ss = MPR->socketService;
    lock(ss);
    for (ITERATE_ITEMS(ss->secureSockets, sp, next)) {
        if (sp->ssl && ((MprMatrixSocket*) sp->sslSocket)->handle == ssl) {
            break;
        }
    }
    unlock(ss);
    if (!sp) {
        /* Should not get here */
        mprAssert(sp);
        return SSL_ALLOW_ANON_CONNECTION;
    }
    if (alert > 0) {
        if (!sp->ssl->verifyServer) {
            return SSL_ALLOW_ANON_CONNECTION;
        }
        return alert;
    }
    mprDecodeLocalTime(&t, mprGetTime());

	/* 
        Validate the 'not before' date 
     */
	if ((c = cert->notBefore) != NULL) {
		if (strlen(c) < 8) {
			return PS_FAILURE;
		}
		/* 
            UTCTIME, defined in 1982, has just a 2 digit year 
         */
		if (cert->timeType == ASN_UTCTIME) {
			y =  2000 + 10 * (c[0] - '0') + (c[1] - '0'); 
            c += 2;
		} else {
			y = 1000 * (c[0] - '0') + 100 * (c[1] - '0') + 10 * (c[2] - '0') + (c[3] - '0'); 
            c += 4;
		}
		m = 10 * (c[0] - '0') + (c[1] - '0'); 
        c += 2;
		d = 10 * (c[0] - '0') + (c[1] - '0'); 
        y -= 1900;
        m -= 1;
		if (t.tm_year < y) {
            return PS_FAILURE; 
        }
		if (t.tm_year == y) {
			if (t.tm_mon < m || (t.tm_mon == m && t.tm_mday < d)) {
                mprError("Certificate not yet valid");
                return PS_FAILURE;
            }
		}
	}
	/* 
        Validate the 'not after' date 
     */
	if ((c = cert->notAfter) != NULL) {
		if (strlen(c) < 8) {
			return PS_FAILURE;
		}
		if (cert->timeType == ASN_UTCTIME) {
			y =  2000 + 10 * (c[0] - '0') + (c[1] - '0'); 
            c += 2;
		} else {
			y = 1000 * (c[0] - '0') + 100 * (c[1] - '0') + 10 * (c[2] - '0') + (c[3] - '0'); 
            c += 4;
		}
		m = 10 * (c[0] - '0') + (c[1] - '0'); 
        c += 2;
		d = 10 * (c[0] - '0') + (c[1] - '0'); 
        y -= 1900;
        m -= 1;
		if (t.tm_year > y) {
            return PS_FAILURE; 
        } else if (t.tm_year == y) {
			if (t.tm_mon > m || (t.tm_mon == m && t.tm_mday > d)) {
                mprError("Certificate has expired");
                return PS_FAILURE;
            }
		}
	}
	return PS_SUCCESS;
}


/*
    Connect as a client
 */
static int connectMss(MprSocket *sp, cchar *host, int port, int flags)
{
    MprSocketService    *ss;
    MprMatrixSocket     *msp;
    MprMatrixSsl        *mssl;
    MprSsl              *ssl;
    uint32              cipherSuite;
    
    lock(sp);
    ss = sp->service;
    if (sp->service->standardProvider->connectSocket(sp, host, port, flags) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_CONNECT;
    }
    msp = sp->sslSocket;
    mprAssert(msp);

    if (!sp->ssl) {
        if ((ssl = ss->secureProvider->defaultSsl) == 0) {
            if ((ssl = getDefaultMatrixSsl()) == 0) {
                unlock(sp);
                return MPR_ERR_CANT_INITIALIZE;
            }
        }
    }
    sp->ssl = ssl;
    mssl = ssl->extendedSsl;

    if (matrixSslLoadRsaKeysMem(mssl->keys, NULL, 0, NULL, 0, CAcertSrvBuf, sizeof(CAcertSrvBuf)) < 0) {
        mprError("MatrixSSL: Could not read or decode certificate or key file."); 
        unlock(sp);
        return MPR_ERR_CANT_INITIALIZE;
    }
    cipherSuite = 0;
    if (matrixSslNewClientSession(&msp->handle, mssl->keys, NULL, cipherSuite, verifyServer, NULL, NULL) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_CONNECT;
    }
    if (doHandshake(sp, 0) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_CONNECT;
    }
    unlock(sp);
    return 0;
}


static void disconnectMss(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}


/*
    Low level blocking write
 */
static ssize blockingWrite(MprSocket *sp, cchar *buf, ssize len)
{
    MprSocketProvider   *standard;
    ssize               written, bytes;
    int                 mode;

    standard = sp->service->standardProvider;
    mode = mprSetSocketBlockingMode(sp, 1);
    for (written = 0; len > 0; ) {
        if ((bytes = standard->writeSocket(sp, buf, len)) < 0) {
            mprSetSocketBlockingMode(sp, mode);
            return bytes;
        }
        buf += bytes;
        len -= bytes;
        written += bytes;
    }
    mprSetSocketBlockingMode(sp, mode);
    return written;
}


/*
    Construct the initial HELLO message to send to the server and initiate
    the SSL handshake. Can be used in the re-handshake scenario as well.
    This is a blocking routine.
 */
static int doHandshake(MprSocket *sp, short cipherSuite)
{
    MprMatrixSocket     *msp;
    ssize               rc, written, toWrite;
    char                *obuf, buf[MPR_SSL_BUFSIZE];
    int                 mode;

    msp = sp->sslSocket;

    toWrite = matrixSslGetOutdata(msp->handle, (uchar**) &obuf);
    if ((written = blockingWrite(sp, obuf, toWrite)) < 0) {
        mprError("MatrixSSL: Error in socketWrite");
        return MPR_ERR_CANT_INITIALIZE;
    }
    matrixSslSentData(msp->handle, (int) written);
    mode = mprSetSocketBlockingMode(sp, 1);

    while (1) {
        /*
            Reading handshake records should always return 0 bytes, we aren't expecting any data yet.
         */
        if ((rc = innerRead(sp, buf, sizeof(buf))) == 0) {
            if (mprIsSocketEof(sp)) {
                mprSetSocketBlockingMode(sp, mode);
                return MPR_ERR_CANT_INITIALIZE;
            }
            if (matrixSslHandshakeIsComplete(msp->handle)) {
                break;
            }
        } else {
            mprError("MatrixSSL: sslRead error in sslDoHandhake, rc %d", rc);
            mprSetSocketBlockingMode(sp, mode);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    mprSetSocketBlockingMode(sp, mode);
    return 0;
}


/*
    Process incoming data. Some is app data, some is SSL control data.
 */
static ssize processMssData(MprSocket *sp, char *buf, ssize size, ssize nbytes, int *readMore)
{
    MprMatrixSocket     *msp;
    uchar               *data, *obuf;
    ssize               toWrite, written, copied, sofar;
    uint32              dlen;
    int                 rc;

    msp = (MprMatrixSocket*) sp->sslSocket;
    *readMore = 0;
    sofar = 0;

    /*
        Process the received data. If there is application data, it is returned in data/dlen
     */
    rc = matrixSslReceivedData(msp->handle, (int) nbytes, &data, &dlen);

    while (1) {
        switch (rc) {
        case PS_SUCCESS:
            return sofar;

        case MATRIXSSL_REQUEST_SEND:
            toWrite = matrixSslGetOutdata(msp->handle, &obuf);
            if ((written = blockingWrite(sp, (cchar*) obuf, (int) toWrite)) < 0) {
                mprError("MatrixSSL: Error in process");
                return MPR_ERR_CANT_INITIALIZE;
            }
            matrixSslSentData(msp->handle, (int) written);
            *readMore = 1;
            return 0;

        case MATRIXSSL_REQUEST_RECV:
            /* Partial read. More read data required */
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
            copied = min((ssize) dlen, size);
            memcpy(buf, data, copied);
            buf += copied;
            size -= copied;
            data += copied;
            dlen = dlen - (int) copied;
            sofar += copied;
            msp->more = ((ssize) dlen > size) ? 1 : 0;
            if (!msp->more) {
                /* The MatrixSSL buffer has been consumed, see if we can get more data */
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
    MprMatrixSocket     *msp;
    MprSocketProvider   *standard;
    uchar               *mbuf;
    ssize               nbytes;
    int                 msize, readMore;

    msp = (MprMatrixSocket*) sp->sslSocket;
    standard = sp->service->standardProvider;
    do {
        /*
            Get the MatrixSSL read buffer to read data into
         */
        if ((msize = matrixSslGetReadbuf(msp->handle, &mbuf)) < 0) {
            return MPR_ERR_BAD_STATE;
        }
        readMore = 0;
        if ((nbytes = standard->readSocket(sp, mbuf, msize)) > 0) {
            if ((nbytes = processMssData(sp, buf, size, nbytes, &readMore)) > 0) {
                return nbytes;
            }
        }
    } while (readMore);
    return 0;
}


/*
    Return number of bytes read. Return -1 on errors and EOF.
 */
static ssize readMss(MprSocket *sp, void *buf, ssize len)
{
    MprMatrixSocket *msp;
    ssize           bytes;

    if (len <= 0) {
        return -1;
    }
    lock(sp);
    /*
        If there is more data buffered by MatrixSSL, then ensure the select handler will recall us again even 
        if there is no more IO events
     */
    bytes = innerRead(sp, buf, len);
    msp = (MprMatrixSocket*) sp->sslSocket;
    if (msp->more) {
        sp->flags |= MPR_SOCKET_PENDING;
        mprRecallWaitHandlerByFd(sp->fd);
    }
    unlock(sp);
    return bytes;
}


/*
    Non-blocking write data. Return the number of bytes written or -1 on errors.
    Returns zero if part of the data was written.

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
    ssize               encoded, nbytes, written;

    msp = (MprMatrixSocket*) sp->sslSocket;

    while (len > 0 || msp->outlen > 0) {
        if ((encoded = matrixSslGetOutdata(msp->handle, &obuf)) <= 0) {
            if (msp->outlen <= 0) {
                msp->outbuf = (char*) buf;
                msp->outlen = len;
                msp->written = 0;
                len = 0;
                nbytes = min(msp->outlen, SSL_MAX_PLAINTEXT_LEN);
            }
            if ((encoded = matrixSslEncodeToOutdata(msp->handle, (uchar*) buf, (int) nbytes)) < 0) {
                return encoded;
            }
            msp->outbuf += nbytes;
            msp->outlen -= nbytes;
            msp->written += nbytes;
        }
        if ((written = sp->service->standardProvider->writeSocket(sp, obuf, encoded)) < 0) {
            return written;
        } else if (written == 0) {
            break;
        }
        matrixSslSentData(msp->handle, (int) written);
    }
    /*
        Only signify all the data has been written if MatrixSSL has absorbed all the data
     */
    return msp->outlen == 0 ? msp->written : 0;
}


/*
    Flush write data. This is blocking.
 */
static ssize flushMss(MprSocket *sp)
{
    MprMatrixSocket     *msp;
    ssize               written, bytes;
    int                 mode;

    msp = (MprMatrixSocket*) sp->sslSocket;
    written = 0;
    mode = mprSetSocketBlockingMode(sp, 1);
    while (msp->outlen > 0) {
        if ((bytes = writeMss(sp, NULL, 0)) < 0) {
            mprSetSocketBlockingMode(sp, mode);
            return bytes;
        }
        written += bytes;
    }
    mprSetSocketBlockingMode(sp, mode);
    return written;
}

#else

int mprCreateMatrixSslModule() { return -1; }
#endif /* BIT_FEATURE_MATRIXSSL */

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
    Start of file "src/mprOpenssl.c"
 */
/************************************************************************/

/*
    mprOpenssl.c - Support for secure sockets via OpenSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "mpr.h"

#if BIT_FEATURE_OPENSSL

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

#if BIT_UNIX_LIKE
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

#if !BIT_WIN_LIKE
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
    /*
        Pre-generate keys that are slow to compute.
     */
    if ((ssl->extendedSsl = mprAllocObj(MprOpenSsl, manageOpenSsl)) == 0) {
        return 0;
    }
    ss->secureProvider->defaultSsl = ssl;
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
    Initialize a server-side SSL configuration. An application can have multiple different SSL configurations
    for different routes.
 */
static int configureOss(MprSsl *ssl)
{
    MprSsl              *defaultSsl;
    MprOpenSsl          *ossl, *src;
    SSL_CTX             *context;
    uchar               resume[16];

    mprAssert(ssl);

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

    if (!sp->ssl) {
        if ((ssl = ss->secureProvider->defaultSsl) == 0) {
            if ((ssl = getDefaultSslSettings()) == 0) {
                unlock(sp);
                return MPR_ERR_CANT_INITIALIZE;
            }
        }
    }
    sp->ssl = ssl;
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

#if DEBUG && UNUSED
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
#endif /* BIT_FEATURE_OPENSSL */

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
    Start of file "src/mprSsl.c"
 */
/************************************************************************/

/**
    mprSsl.c -- Load and manage the SSL providers.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"

#if BIT_FEATURE_SSL
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
#if BIT_FEATURE_OPENSSL
    /*
        NOTE: preference given to open ssl if both are enabled
     */
    mprLog(4, "Loading OpenSSL module");
    if (mprCreateOpenSslModule(lazy) < 0) {
        return 0;
    }
#elif BIT_FEATURE_MATRIXSSL
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
    Create a new SSL context object
 */
MprSsl *mprCreateSsl()
{
    MprSsl      *ssl;

    if ((ssl = mprAllocObj(MprSsl, manageSsl)) == 0) {
        return 0;
    }
    ssl->ciphers = sclone(MPR_DEFAULT_CIPHER_SUITE);
    ssl->protocols = MPR_PROTO_TLSV1 | MPR_PROTO_TLSV11;
    ssl->verifyDepth = 6;
    ssl->verifyClient = 0;
    ssl->verifyServer = 1;
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


void mprSetSocketSslConfig(MprSocket *sp, MprSsl *ssl)
{
    sp->ssl = ssl;
}


void mprVerifySslClients(MprSsl *ssl, bool on)
{
    ssl->verifyClient = on;
}


void mprVerifySslServers(MprSsl *ssl, bool on)
{
    ssl->verifyServer = on;
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


void mprConfigureSsl(MprSsl *ssl) { }
void mprSetSslCiphers(MprSsl *ssl, cchar *ciphers) { }
void mprSetSslKeyFile(MprSsl *ssl, cchar *keyFile) { }
void mprSetSslCertFile(MprSsl *ssl, cchar *certFile) { }
void mprSetSslCaFile(MprSsl *ssl, cchar *caFile) { }
void mprSetSslCaPath(MprSsl *ssl, cchar *caPath) { }
void mprSetSslProtocols(MprSsl *ssl, int protocols) { }
void mprSetSocketSslConfig(MprSocket *sp, MprSsl *ssl) { }
void mprVerifySslClients(MprSsl *ssl, bool on) { }
void mprVerifySslServers(MprSsl *ssl, bool on) { }

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
