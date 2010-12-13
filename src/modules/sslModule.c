/*
    sslModule.c - Module for SSL support
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"
#include    "mprSsl.h"

#if BLD_FEATURE_SSL
/*********************************** Code *************************************/

static int parseSsl(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLoc     *loc;
    MaServer    *server;
    MaHost      *host;
    char        *path, prefix[MPR_MAX_FNAME];
    char        *tok, *word, *enable, *provider;
    int         protoMask, mask;
    static int  hasBeenWarned = 0;

    host = state->host;
    server = state->server;
    loc = state->loc;

    scopy(prefix, sizeof(prefix), key);
    prefix[3] = '\0';

    if (scasecmp(prefix, "SSL") != 0) {
        return 0;
    }
    if (!mprHasSecureSockets(http)) {
        if (!hasBeenWarned++) {
            mprError("Missing an SSL Provider");
        }
        return 0;
        /* return MPR_ERR_BAD_SYNTAX; */
    }
    if (loc->ssl == 0) {
        loc->ssl = mprCreateSsl(loc);
    }
    if (scasecmp(key, "SSLEngine") == 0) {
        enable = stok(value, " \t", &tok);
        provider = stok(0, " \t", &tok);
        if (scasecmp(value, "on") == 0) {
            maSecureHost(host, loc->ssl);
        }
        return 1;
    }
    path = maMakePath(host, strim(value, "\"", MPR_TRIM_BOTH));

    if (scasecmp(key, "SSLCACertificatePath") == 0) {
        mprSetSslCaPath(loc->ssl, path);
        return 1;

    } else if (scasecmp(key, "SSLCACertificateFile") == 0) {
        mprSetSslCaFile(loc->ssl, path);
        return 1;

    } else if (scasecmp(key, "SSLCertificateFile") == 0) {
        mprSetSslCertFile(loc->ssl, path);
        return 1;

    } else if (scasecmp(key, "SSLCertificateKeyFile") == 0) {
        mprSetSslKeyFile(loc->ssl, path);
        return 1;

    } else if (scasecmp(key, "SSLCipherSuite") == 0) {
        mprSetSslCiphers(loc->ssl, value);
        return 1;

    } else if (scasecmp(key, "SSLVerifyClient") == 0) {
        if (scasecmp(value, "require") == 0) {
            mprVerifySslClients(loc->ssl, 1);

        } else if (scasecmp(value, "none") == 0) {
            mprVerifySslClients(loc->ssl, 0);

        } else {
            return -1;
        }
        return 1;

    } else if (scasecmp(key, "SSLProtocol") == 0) {
        protoMask = 0;
        word = stok(value, " \t", &tok);
        while (word) {
            mask = -1;
            if (*word == '-') {
                word++;
                mask = 0;
            } else if (*word == '+') {
                word++;
            }
            if (scasecmp(word, "SSLv2") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV2 & ~mask);
                protoMask |= (MPR_PROTO_SSLV2 & mask);

            } else if (scasecmp(word, "SSLv3") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV3 & ~mask);
                protoMask |= (MPR_PROTO_SSLV3 & mask);

            } else if (scasecmp(word, "TLSv1") == 0) {
                protoMask &= ~(MPR_PROTO_TLSV1 & ~mask);
                protoMask |= (MPR_PROTO_TLSV1 & mask);

            } else if (scasecmp(word, "ALL") == 0) {
                protoMask &= ~(MPR_PROTO_ALL & ~mask);
                protoMask |= (MPR_PROTO_ALL & mask);
            }
            word = stok(0, " \t", &tok);
        }
        mprSetSslProtocols(loc->ssl, protoMask);
        return 1;
    }
    return 0;
}


/*
    Loadable module initialization. 
 */
int maSslModuleInit(Http *http, MprModule *mp)
{
    HttpStage     *stage;

    if (mprLoadSsl(1) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if ((stage = httpCreateStage(http, "sslModule", HTTP_STAGE_MODULE)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->parse = (HttpParse) parseSsl; 
    return 0;
}
#else

int maSslModuleInit(Http *http, MprModule *mp)
{
    return 0;
}
#endif /* BLD_FEATURE_SSL */

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
