/*
    sslModule.c - Module for SSL support
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

#if BIT_FEATURE_SSL
/*********************************** Code *************************************/

static bool checkSsl(MaState *state)
{
    static int  hasBeenWarned = 0;

    if (!mprHasSecureSockets()) {
        if (!hasBeenWarned++) {
            mprError("Missing an SSL Provider");
        }
        return 0;
    }
    if (state->route->ssl == 0) {
        state->route->ssl = mprCreateSsl(state->route);
    }
    return 1;
}


static int sslCaCertificatePathDirective(MaState *state, cchar *key, cchar *value)
{
    checkSsl(state);
    mprSetSslCaPath(state->route->ssl, mprJoinPath(state->host->home, value));
    return 0;
}


static int sslCaCertificateFileDirective(MaState *state, cchar *key, cchar *value)
{
    checkSsl(state);
    mprSetSslCaFile(state->route->ssl, mprJoinPath(state->host->home, value));
    return 0;
}


static int sslCertificateFileDirective(MaState *state, cchar *key, cchar *value)
{
    checkSsl(state);
    mprSetSslCertFile(state->route->ssl, mprJoinPath(state->host->home, value));
    return 0;
}


static int sslCertificateKeyFileDirective(MaState *state, cchar *key, cchar *value)
{
    checkSsl(state);
    mprSetSslKeyFile(state->route->ssl, mprJoinPath(state->host->home, value));
    return 0;
}


static int sslCipherSuiteDirective(MaState *state, cchar *key, cchar *value)
{
    checkSsl(state);
    mprSetSslCiphers(state->route->ssl, value);
    return 0;
}


static int sslDirective(MaState *state, cchar *key, cchar *value)
{
    char    *provider;
    bool    on;

    if (!maTokenize(state, value, "%B ?S", &on, &provider)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (on) {
        if (httpSecureEndpointByName(state->host->name, state->route->ssl) < 0) {
            mprError("No HttpEndpoint at %s to secure", state->host->name);
            return MPR_ERR_BAD_STATE;
        }
    }
    return 0;
}


static int sslVerifyClientDirective(MaState *state, cchar *key, cchar *value)
{
    checkSsl(state);
    if (scasecmp(value, "require") == 0) {
        mprVerifySslClients(state->route->ssl, 1);

    } else if (scasecmp(value, "none") == 0) {
        mprVerifySslClients(state->route->ssl, 0);

    } else {
        mprError("Unknown verify client option");
        return MPR_ERR_BAD_STATE;
    }
    return 0;
}

static int sslProtocolDirective(MaState *state, cchar *key, cchar *value)
{
    char    *word, *tok;
    int     mask, protoMask;

    checkSsl(state);
    protoMask = 0;
    word = stok(sclone(value), " \t", &tok);
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
    mprSetSslProtocols(state->route->ssl, protoMask);
    return 0;
}


/*
    Loadable module initialization. 
 */
int maSslModuleInit(Http *http, MprModule *module)
{
    HttpStage   *stage;
    MaAppweb    *appweb;

    if (mprLoadSsl(1) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if ((stage = httpCreateStage(http, "sslModule", HTTP_STAGE_MODULE, module)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    appweb = httpGetContext(http);
    maAddDirective(appweb, "SSL", sslDirective);
    maAddDirective(appweb, "SSLEngine", sslDirective);
    maAddDirective(appweb, "SSLCACertificateFile", sslCaCertificateFileDirective);
    maAddDirective(appweb, "SSLCACertificatePath", sslCaCertificatePathDirective);
    maAddDirective(appweb, "SSLCertificateFile", sslCertificateFileDirective);
    maAddDirective(appweb, "SSLCertificateKeyFile", sslCertificateKeyFileDirective);
    maAddDirective(appweb, "SSLCipherSuite", sslCipherSuiteDirective);
    maAddDirective(appweb, "SSLProtocol", sslProtocolDirective);
    maAddDirective(appweb, "SSLVerifyClient", sslVerifyClientDirective);
    return 0;
}
#else

int maSslModuleInit(Http *http, MprModule *mp)
{
    return 0;
}
#endif /* BIT_FEATURE_SSL */

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
