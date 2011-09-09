/*
    mprSsl.h - Header for SSL services. Currently supporting OpenSSL and MatrixSSL.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_MPR_SSL
#define _h_MPR_SSL 1
/***********************************Includes **********************************/

#include "mpr.h"

#if BLD_FEATURE_OPENSSL
    /* Clashes with WinCrypt.h */
    #undef OCSP_RESPONSE
    #include    <openssl/ssl.h>
    #include    <openssl/evp.h>
    #include    <openssl/rand.h>
    #include    <openssl/err.h>
#endif

#if BLD_FEATURE_MATRIXSSL
    #include    "matrixSsl.h"
#endif 

#ifdef __cplusplus
extern "C" {
#endif

/************************************* Defines ********************************/

#define MPR_DEFAULT_SERVER_CERT_FILE    "server.crt"
#define MPR_DEFAULT_SERVER_KEY_FILE     "server.key.pem"
#define MPR_DEFAULT_CLIENT_CERT_FILE    "client.crt"
#define MPR_DEFAULT_CLIENT_CERT_PATH    "certs"

typedef struct MprSsl {
    /*
        Server key and certificate configuration
     */
    char            *key;               /* Key string */
    char            *cert;              /* Cert string */
    char            *keyFile;           /* Alternatively, locate the key in a file */
    char            *certFile;          /* Alternatively, locate the cert in a file */
    char            *caFile;            /* Client verification cert file or bundle */
    char            *caPath;            /* Client verification cert directory */
    char            *ciphers;
    int             configured;

    /*
        Client configuration
     */
    int             verifyClient;
    int             verifyDepth;
    int             protocols;

    /*
        Per-SSL provider context information
     */
#if BLD_FEATURE_MATRIXSSL
    sslKeys_t       *keys;
#endif

#if BLD_FEATURE_OPENSSL
    SSL_CTX         *context;
    RSA             *rsaKey512;
    RSA             *rsaKey1024;
    DH              *dhKey512;
    DH              *dhKey1024;
#endif
} MprSsl;


typedef struct MprSslSocket
{
    MprSocket       *sock;
    MprSsl          *ssl;
#if BLD_FEATURE_OPENSSL
    SSL             *osslStruct;
    BIO             *bio;
#endif
#if BLD_FEATURE_MATRIXSSL
    ssl_t           *mssl;
    sslBuf_t        insock;             /* Cached ciphertext from socket */
    sslBuf_t        inbuf;              /* Cached (decoded) plaintext */
    sslBuf_t        outsock;            /* Cached ciphertext to socket */
    int             outBufferCount;     /* Count of outgoing data we've buffered */
#endif
} MprSslSocket;


#if BLD_FEATURE_OPENSSL
extern int mprCreateOpenSslModule(bool lazy);
#endif

#if BLD_FEATURE_MATRIXSSL
extern int mprCreateMatrixSslModule(bool lazy);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _h_MPR_SSL */

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
