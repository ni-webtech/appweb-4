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
    ssize           outlen;             /* Length of outbuf */
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

static void     closeMss(MprSocket *sp, bool gracefully);
static MprMatrixSsl *createMatrixSslConfig(MprSsl *ssl, int server);
static MprSocketProvider *createMatrixSslProvider();
static void     disconnectMss(MprSocket *sp);
static int      doHandshake(MprSocket *sp, short cipherSuite);
static ssize    flushMss(MprSocket *sp);
static ssize    innerRead(MprSocket *sp, char *userBuf, ssize len);
static int      listenMss(MprSocket *sp, cchar *host, int port, int flags);
static void     manageMatrixSocket(MprMatrixSocket *msp, int flags);
static void     manageMatrixSsl(MprMatrixSsl *mssl, int flags);
static ssize    processMssData(MprSocket *sp, char *buf, ssize size, ssize nbytes, int *readMore);
static ssize    readMss(MprSocket *sp, void *buf, ssize len);
static int      upgradeMss(MprSocket *sp, MprSsl *ssl, int server);
static int      verifyServer(ssl_t *ssl, psX509Cert_t *cert, int32 alert);
static ssize    writeMss(MprSocket *sp, cvoid *buf, ssize len);

/************************************ Code ************************************/

int mprCreateMatrixSslModule()
{
    MprSocketProvider   *provider;
    MprSocketService    *ss;

    ss = MPR->socketService;

    /*
        Install this module as the SSL provider (can only have one)
     */
    if ((provider = createMatrixSslProvider()) == 0) {
        return 0;
    }
    if (matrixSslOpen() < 0) {
        return 0;
    }
    return 0;
}


/*
    Initialize the SSL configuration. An application can have multiple different SSL
    configurations for different routes. There is default SSL configuration that is used
    when a route does not define a configuration and also for clients.
 */
static MprMatrixSsl *createMatrixSslConfig(MprSsl *ssl, int server)
{
    MprMatrixSsl    *mssl;
    char            *password;

    mprAssert(ssl);

    if ((ssl->pconfig = mprAllocObj(MprMatrixSsl, manageMatrixSsl)) == 0) {
        return 0;
    }
    mssl = ssl->pconfig;

    //  OPT - does this need to be done for each MprSsl or just once?
    if (matrixSslNewKeys(&mssl->keys) < 0) {
        mprError("MatrixSSL: Can't create new MatrixSSL keys");
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
        return 0;
    }
    if (ssl->verifyPeer < 0) {
        ssl->verifyPeer = !server;
    }
    return mssl;
}


static MprSocketProvider *createMatrixSslProvider()
{
    MprSocketProvider   *provider;

    if ((provider = mprAllocObj(MprSocketProvider, 0)) == NULL) {
        return 0;
    }
    provider->closeSocket = closeMss;
    provider->disconnectSocket = disconnectMss;
    provider->flushSocket = flushMss;
    provider->listenSocket = listenMss;
    provider->readSocket = readMss;
    provider->writeSocket = writeMss;
    provider->upgradeSocket = upgradeMss;
    mprAddSocketProvider("matrixssl", provider);
    return provider;
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


static int upgradeMss(MprSocket *sp, MprSsl *ssl, int server)
{
    MprSocketService    *ss;
    MprMatrixSocket     *msp;
    MprMatrixSsl        *mssl;
    uint32              cipherSuite;

    ss = sp->service;
    mprAssert(ss);
    mprAssert(sp);

    if ((msp = (MprMatrixSocket*) mprAllocObj(MprMatrixSocket, manageMatrixSocket)) == 0) {
        return MPR_ERR_MEMORY;
    }
    lock(sp);
    msp->sock = sp;
    sp->sslSocket = msp;
    sp->ssl = ssl;

    mprAddItem(ss->secureSockets, sp);

    if (!ssl->pconfig && (ssl->pconfig = createMatrixSslConfig(ssl, server)) == 0) {
        unlock(sp);
        return MPR_ERR_CANT_INITIALIZE;
    }
    mssl = ssl->pconfig;

    /* 
        Associate a new ssl session with this socket. The session represents the state of the ssl protocol 
        over this socket. Session caching is handled automatically by this api.
     */
    if (server) {
        if (matrixSslNewServerSession(&msp->handle, mssl->keys, NULL) < 0) {
            unlock(sp);
            return MPR_ERR_CANT_CREATE;
        }
    } else {
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
    }
    unlock(sp);
    return 0;
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
    sp = 0;
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
        if (!sp->ssl->verifyPeer) {
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
            *readMore = 0;
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
            }
            nbytes = min(msp->outlen, SSL_MAX_PLAINTEXT_LEN);
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
#if UNUSED
    MprMutex        **locks;
#endif
} MprOpenSsl;

typedef struct MprOpenSocket {
    MprSocket       *sock;
    SSL             *handle;
    BIO             *bio;
} MprOpenSocket;

typedef struct RandBuf {
    MprTime     now;
    int         pid;
} RandBuf;

static int      numLocks;
static MprMutex **olocks;
static MprSocketProvider *openSslProvider;
static MprOpenSsl *defaultOpenSsl;

struct CRYPTO_dynlock_value {
    MprMutex    *mutex;
};
typedef struct CRYPTO_dynlock_value DynLock;

/***************************** Forward Declarations ***************************/

static void     closeOss(MprSocket *sp, bool gracefully);
static int      configureCertificateFiles(MprSsl *ssl, SSL_CTX *ctx, char *key, char *cert);
static MprOpenSsl *createOpenSslConfig(MprSsl *ssl, int server);
static MprSocketProvider *createOpenSslProvider();
static DH       *dhCallback(SSL *ssl, int isExport, int keyLength);
static void     disconnectOss(MprSocket *sp);
static ssize    flushOss(MprSocket *sp);
static int      listenOss(MprSocket *sp, cchar *host, int port, int flags);
static void     manageOpenSsl(MprOpenSsl *ossl, int flags);
static void     manageOpenSocket(MprOpenSocket *ssp, int flags);
static ssize    readOss(MprSocket *sp, void *buf, ssize len);
static RSA      *rsaCallback(SSL *ssl, int isExport, int keyLength);
static int      upgradeOss(MprSocket *sp, MprSsl *ssl, int server);
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
int mprCreateOpenSslModule()
{
    MprSocketService    *ss;
    RandBuf             randBuf;
    int                 i;

    ss = MPR->socketService;
    mprAssert(ss);

    /*
        Get some random data
     */
    randBuf.now = mprGetTime();
    randBuf.pid = getpid();
    RAND_seed((void*) &randBuf, sizeof(randBuf));
#if BIT_UNIX_LIKE
    mprLog(6, "OpenSsl: Before calling RAND_load_file");
    RAND_load_file("/dev/urandom", 256);
    mprLog(6, "OpenSsl: After calling RAND_load_file");
#endif

    if ((openSslProvider = createOpenSslProvider()) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddSocketProvider("openssl", openSslProvider);

    /*
        Pre-create expensive keys
     */
    if ((defaultOpenSsl = mprAllocObj(MprOpenSsl, manageOpenSsl)) == 0) {
        return MPR_ERR_MEMORY;
    }
    defaultOpenSsl->rsaKey512 = RSA_generate_key(512, RSA_F4, 0, 0);
    defaultOpenSsl->rsaKey1024 = RSA_generate_key(1024, RSA_F4, 0, 0);
    defaultOpenSsl->dhKey512 = get_dh512();
    defaultOpenSsl->dhKey1024 = get_dh1024();

    /*
        Configure the SSL library. Use the crypto ID as a one-time test. This allows
        users to configure the library and have their configuration used instead.
     */
    if (CRYPTO_get_id_callback() == 0) {
        numLocks = CRYPTO_num_locks();
        if ((olocks = mprAlloc(numLocks * sizeof(MprMutex*))) == 0) {
            return MPR_ERR_MEMORY;
        }
        for (i = 0; i < numLocks; i++) {
            olocks[i] = mprCreateLock();
        }
        CRYPTO_set_id_callback(sslThreadId);
        CRYPTO_set_locking_callback(sslStaticLock);

        CRYPTO_set_dynlock_create_callback(sslCreateDynLock);
        CRYPTO_set_dynlock_destroy_callback(sslDestroyDynLock);
        CRYPTO_set_dynlock_lock_callback(sslDynLock);
#if !BIT_WIN_LIKE
        /* OPT - Should be a configure option to specify desired ciphers */
        OpenSSL_add_all_algorithms();
#endif
        /*
            WARNING: SSL_library_init() is not reentrant. Caller must ensure safety.
         */
        SSL_library_init();
        SSL_load_error_strings();
    }
    return 0;
}


static void manageOpenSsl(MprOpenSsl *ossl, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;
    } else if (flags & MPR_MANAGE_FREE) {
        if (ossl->context != 0) {
            SSL_CTX_free(ossl->context);
            ossl->context = 0;
        }
        if (ossl == defaultOpenSsl) {
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


static void manageOpenSslProvider(MprSocketProvider *provider, int flags)
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        /*
            Mark global locks
         */
        if (olocks) {
            mprMark(olocks);
            for (i = 0; i < numLocks; i++) {
                mprMark(olocks[i]);
            }
        }
        mprMark(defaultOpenSsl);

    } else if (flags & MPR_MANAGE_FREE) {
        olocks = 0;
    }
}


/*
    Initialize a provider structure for OpenSSL
 */
static MprSocketProvider *createOpenSslProvider()
{
    MprSocketProvider   *provider;

    if ((provider = mprAllocObj(MprSocketProvider, manageOpenSslProvider)) == NULL) {
        return 0;
    }
    provider->upgradeSocket = upgradeOss;
    provider->closeSocket = closeOss;
    provider->disconnectSocket = disconnectOss;
    provider->flushSocket = flushOss;
    provider->listenSocket = listenOss;
    provider->readSocket = readOss;
    provider->writeSocket = writeOss;
    return provider;
}


/*
    Create an SSL configuration for a route. An application can have multiple different SSL 
    configurations for different routes. There is default SSL configuration that is used
    when a route does not define a configuration and also for clients.
 */
static MprOpenSsl *createOpenSslConfig(MprSsl *ssl, int server)
{
    MprSocketService    *ss;
    MprOpenSsl          *ossl;
    SSL_CTX             *context;
    uchar               resume[16];

    mprAssert(ssl);
    ss = MPR->socketService;

    if ((ssl->pconfig = mprAllocObj(MprOpenSsl, manageOpenSsl)) == 0) {
        return 0;
    }
    ossl = ssl->pconfig;
    ossl->rsaKey512 = defaultOpenSsl->rsaKey512;
    ossl->rsaKey1024 = defaultOpenSsl->rsaKey1024;
    ossl->dhKey512 = defaultOpenSsl->dhKey512;
    ossl->dhKey1024 = defaultOpenSsl->dhKey1024;

    ossl = ssl->pconfig;
    mprAssert(ossl);

    if ((context = SSL_CTX_new(SSLv23_method())) == 0) {
        mprError("OpenSSL: Unable to create SSL context"); 
        return 0;
    }
    ossl->context = context;

    SSL_CTX_set_app_data(context, (void*) ssl);
    SSL_CTX_sess_set_cache_size(context, 512);
    RAND_bytes(resume, sizeof(resume));
    SSL_CTX_set_session_id_context(context, resume, sizeof(resume));

    /*
        Configure the certificates
     */
    if (ssl->keyFile || ssl->certFile) {
        if (configureCertificateFiles(ssl, context, ssl->keyFile, ssl->certFile) != 0) {
            SSL_CTX_free(context);
            return 0;
        }
    }
    SSL_CTX_set_cipher_list(context, ssl->ciphers);

    if (ssl->caFile || ssl->caPath) {
        if ((!SSL_CTX_load_verify_locations(context, ssl->caFile, ssl->caPath)) ||
                (!SSL_CTX_set_default_verify_paths(context))) {
            mprError("OpenSSL: Unable to set certificate locations"); 
            SSL_CTX_free(context);
            return 0;
        }
        if (ssl->caFile) {
            STACK_OF(X509_NAME) *certNames;
            certNames = SSL_load_client_CA_file(ssl->caFile);
            if (certNames) {
                /*
                    Define the list of CA certificates to send to the client
                    before they send their client certificate for validation
                 */
                SSL_CTX_set_client_CA_list(context, certNames);
            }
        }
    }
    if (ssl->verifyPeer < 0) {
        ssl->verifyPeer = !server;
    }
    if (server) {
        if (ssl->verifyPeer) {
            if (!ssl->caFile == 0 && !ssl->caPath) {
                mprError("OpenSSL: Must define CA certificates if using client verification");
                SSL_CTX_free(context);
                return 0;
            }
            SSL_CTX_set_verify(context, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verifyX509Certificate);
            SSL_CTX_set_verify_depth(context, ssl->verifyDepth);
        } else {
            /* With this, the server will not request a client certificate */
            SSL_CTX_set_verify(context, SSL_VERIFY_NONE, verifyX509Certificate);
        }
    } else {
        SSL_CTX_set_verify(context, SSL_VERIFY_PEER, verifyX509Certificate);
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
    }
    if (!(ssl->protocols & MPR_PROTO_TLSV1)) {
        SSL_CTX_set_options(context, SSL_OP_NO_TLSv1);
    }
    /* 
        Ensure we generate a new private key for each connection
     */
    SSL_CTX_set_options(context, SSL_OP_SINGLE_DH_USE);
    return ossl;
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
    Initialize a new server-side connection
 */
static int listenOss(MprSocket *sp, cchar *host, int port, int flags)
{
    mprAssert(sp);
    mprAssert(port);
    return sp->service->standardProvider->listenSocket(sp, host, port, flags);
}


/*
    Upgrade a standard socket to use SSL/TLS
 */
static int upgradeOss(MprSocket *sp, MprSsl *ssl, int server)
{
    MprSocketService    *ss;
    MprOpenSocket       *osp;
    MprOpenSsl          *ossl;
    char                ebuf[MPR_MAX_STRING];
    ulong               error;
    int                 rc;

    ss = sp->service;
    mprAssert(ss);
    mprAssert(sp);

    if (ssl == 0) {
        ssl = mprCreateSsl();
    }
    lock(sp);
    if ((osp = (MprOpenSocket*) mprAllocObj(MprOpenSocket, manageOpenSocket)) == 0) {
        unlock(sp);
        return MPR_ERR_MEMORY;
    }
    osp->sock = sp;
    sp->sslSocket = osp;
    sp->ssl = ssl;

    if (!ssl->pconfig && (ssl->pconfig = createOpenSslConfig(ssl, server)) == 0) {
        unlock(sp);
        return MPR_ERR_CANT_INITIALIZE;
    }
    /*
        Create and configure the SSL struct
     */
    ossl = sp->ssl->pconfig;
    if ((osp->handle = (SSL*) SSL_new(ossl->context)) == 0) {
        unlock(sp);
        return MPR_ERR_BAD_STATE;
    }
    SSL_set_app_data(osp->handle, (void*) osp);

    /*
        Create a socket bio
     */
    osp->bio = BIO_new_socket(sp->fd, BIO_NOCLOSE);
    SSL_set_bio(osp->handle, osp->bio, osp->bio);
    if (server) {
        SSL_set_accept_state(osp->handle);
    } else {
        /* Block while connecting */
        mprSetSocketBlockingMode(sp, 1);
        if ((rc = SSL_connect(osp->handle)) < 1) {
            error = ERR_get_error();
            ERR_error_string_n(error, ebuf, sizeof(ebuf) - 1);
            sp->errorMsg = sclone(ebuf);
            mprLog(4, "SSL_read error %s", ebuf);
            unlock(sp);
            return MPR_ERR_CANT_CONNECT;
        }
        mprSetSocketBlockingMode(sp, 0);
    }
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
    MprOpenSocket   *osp;
    MprSsl          *ssl;
    X509_NAME       *xSubject;
    X509            *cert;
    char            subject[260], issuer[260], peer[260], ebuf[MPR_MAX_STRING];
    ulong           serror;
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
            error = SSL_get_error(osp->handle, rc);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_CONNECT || error == SSL_ERROR_WANT_ACCEPT) {
                continue;
            }
            serror = ERR_get_error();
            ERR_error_string_n(error, ebuf, sizeof(ebuf) - 1);
            mprLog(5, "SSL_read %s", ebuf);
        }
        break;
    }
    if (rc > 0 && !(sp->flags & MPR_SOCKET_TRACED)) {
        ssl = sp->ssl;
        mprLog(4, "OpenSSL connected using cipher: \"%s\" from set %s", SSL_get_cipher(osp->handle), ssl->ciphers);
        if (ssl->caFile) {
            mprLog(4, "OpenSSL: Using certificates from %s", ssl->caFile);
        } else if (ssl->caPath) {
            mprLog(4, "OpenSSL: Using certificates from directory %s", ssl->caPath);
        }
        cert = SSL_get_peer_certificate(osp->handle);
        if (cert == 0) {
            mprLog(4, "OpenSSL: client supplied no certificate");
        } else {
            xSubject = X509_get_subject_name(cert);
            X509_NAME_oneline(xSubject, subject, sizeof(subject) -1);
            X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) -1);
            X509_NAME_get_text_by_NID(xSubject, NID_commonName, peer, sizeof(peer) - 1);
            mprLog(4, "OpenSSL Subject %s", subject);
            mprLog(4, "OpenSSL Issuer: %s", issuer);
            mprLog(4, "OpenSSL Peer: %s", peer);
            X509_free(cert);
        }
        sp->flags |= MPR_SOCKET_TRACED;
    }
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
            serror = ERR_get_error();
            ERR_error_string_n(error, ebuf, sizeof(ebuf) - 1);
            mprLog(4, "OpenSSL: connection with protocol error: %s", ebuf);
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

    cert = X509_STORE_CTX_get_current_cert(xContext);
    depth = X509_STORE_CTX_get_error_depth(xContext);
    error = X509_STORE_CTX_get_error(xContext);

    ok = 1;
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
    if (ok && ssl->verifyDepth < depth) {
        if (error == 0) {
            error = X509_V_ERR_CERT_CHAIN_TOO_LONG;
        }
    }
    switch (error) {
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        /* Normal self signed certificate */
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    case X509_V_ERR_CERT_UNTRUSTED:
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        if (ssl->verifyIssuer) {
            /* Issuer can't be verified */
            ok = 0;
        }
        break;

    case X509_V_ERR_CERT_CHAIN_TOO_LONG:
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_CERT_REJECTED:
    case X509_V_ERR_CERT_SIGNATURE_FAILURE:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
    case X509_V_ERR_INVALID_CA:
    default:
        ok = 0;
        break;
    }
    if (ok) {
        mprLog(3, "OpenSSL: Certificate verified: subject %s", subject);
        mprLog(4, "OpenSSL: Issuer: %s", issuer);
        mprLog(4, "OpenSSL: Peer: %s", peer);
    } else {
        mprLog(1, "OpenSSL: Certification failed: subject %s (more trace at level 4)", subject);
        mprLog(4, "OpenSSL: Issuer: %s", issuer);
        mprLog(4, "OpenSSL: Peer: %s", peer);
        mprLog(4, "OpenSSL: Error: %d: %s", error, X509_verify_cert_error_string(error));
    }
    return ok;
}


static ssize flushOss(MprSocket *sp)
{
#if NOT_REQUIRED && KEEP
    MprOpenSocket    *osp;
    osp = (MprOpenSocket*) sp->sslSocket;
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

    if (olocks) {
        if (mode & CRYPTO_LOCK) {
            mprLock(olocks[n]);
        } else {
            mprUnlock(olocks[n]);
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
    ossl = sp->ssl->pconfig;

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
    ossl = sp->ssl->pconfig;

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

    if ((dh = DH_new()) == NULL) {
        return(NULL);
    }
    dh->p=BN_bin2bn(dh512_p,sizeof(dh512_p),NULL);
    dh->g=BN_bin2bn(dh512_g,sizeof(dh512_g),NULL);

    if ((dh->p == NULL) || (dh->g == NULL)) { 
        DH_free(dh); 
        return(NULL); 
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

    if ((dh = DH_new()) == NULL) {
        return(NULL);
    }
    dh->p = BN_bin2bn(dh1024_p,sizeof(dh1024_p),NULL);
    dh->g = BN_bin2bn(dh1024_g,sizeof(dh1024_g),NULL);
    if ((dh->p == NULL) || (dh->g == NULL)) {
        DH_free(dh); 
        return(NULL); 
    }
    return dh;
}

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
    mprSsl.c -- Initialization for libmprssl. Load the SSL provider.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"

#if BIT_FEATURE_SSL
/************************************ Code ************************************/
/*
    Module initialization entry point
 */
int mprSslInit(void *unused, MprModule *module)
{
    mprAssert(module);

#if BIT_FEATURE_MATRIXSSL
    if (mprCreateMatrixSslModule() < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    MPR->socketService->defaultProvider = sclone("matrixssl");
#endif
#if BIT_FEATURE_OPENSSL
    if (mprCreateOpenSslModule() < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    MPR->socketService->defaultProvider = sclone("openssl");
#endif
    return 0;
}

#endif /* BLD_FEATURE_SSL */

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
