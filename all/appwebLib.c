#include "appweb.h"

/******************************************************************************/
/* 
 *  This file is an amalgamation of all the individual source code files for
 *  Embedthis Appweb 3.3.0.
 *
 *  Catenating all the source into a single file makes embedding simpler and
 *  the resulting application faster, as many compilers can do whole file
 *  optimization.
 *
 *  If you want to modify appweb, you can still get the whole source
 *  as individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "../src/alias.c"
 */
/************************************************************************/

/*
    alias.c -- Alias service for aliasing URLs to file storage.

    This module supports the alias directives and mapping URLs to physical locations. 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/*
    Create an alias for a URI prefix. During processing, a request URI prefix is substituted to the target which
    may be either a physical path or a URI if a non-zero redirect code is supplied.
 */

MaAlias *maCreateAlias(MprCtx ctx, cchar *prefix, cchar *target, int code)
{
    MaAlias     *ap;
    int         len, sep;

    mprAssert(ctx);
    mprAssert(prefix);
    mprAssert(target && *target);

    ap = mprAllocObjZeroed(ctx, MaAlias);
    if (ap == 0) {
        return 0;
    }
    ap->prefix = mprStrdup(ctx, prefix);
    ap->prefixLen = (int) strlen(prefix);

    /*  
        Always strip trailing "/"
     */
    if (ap->prefixLen > 0 && ap->prefix[ap->prefixLen - 1] == '/') {
        ap->prefix[--ap->prefixLen] = '\0';
    }
    if (code) {
        ap->redirectCode = code;
        ap->uri = mprStrdup(ctx, target);
    } else {
        /*  
            Trim trailing "/" from filename always
         */
        sep = mprGetPathSeparator(ctx, target);
        ap->filename = mprStrdup(ctx, target);
        len = strlen(ap->filename) - 1;
        if (len >= 0 && ap->filename[len] == sep) {
            ap->filename[len] = '\0';
        }
    }
    return ap;
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
/************************************************************************/
/*
 *  End of file "../src/alias.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/config.c"
 */
/************************************************************************/

/**
    config.c - Parse the configuration file.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static bool featureSupported(MprCtx ctx, char *key);
static MaConfigState *pushState(MprCtx ctx, MaConfigState *state, int *top);
static int processSetting(MaServer *server, char *key, char *value, MaConfigState *state);

#if BLD_FEATURE_CONFIG_SAVE
static void tabs(int fd, int indent);
static void printAuth(int fd, MaHost *host, HttpAuth *auth, int indent);
#endif


/*  Configure the server
 */
int maConfigureServer(MaServer *server, cchar *configFile, cchar *ip, int port, cchar *docRoot)
{
    MaAppweb        *appweb;
    MaHost          *host;
    MaAlias         *ap;
    HttpLocation    *location, *loc;
    Http            *http;
    char            *path, *searchPath;

    appweb = server->appweb;
    http = appweb->http;

    if (ip && docRoot) {
        mprLog(server, 2, "DocumentRoot %s", docRoot);
        if ((host = maCreateDefaultHost(server, docRoot, ip, port)) == 0) {
            mprUserError(server, "Can't open server on %s", ip);
            return MPR_ERR_CANT_OPEN;
        }
        location = host->location;
#if WIN
        searchPath = mprAsprintf(server, -1, "%s" MPR_SEARCH_SEP ".", mprGetAppDir(server));
#else
        searchPath = mprAsprintf(server, -1, "%s" MPR_SEARCH_SEP "%s" MPR_SEARCH_SEP ".", mprGetAppDir(server),
            mprSamePath(server, BLD_BIN_PREFIX, mprGetAppDir(server)) ? BLD_MOD_PREFIX: BLD_ABS_MOD_DIR);
#endif
        mprSetModuleSearchPath(server, searchPath);
        mprFree(searchPath);
#if UNUSED
        httpSetConnector(location, "netConnector");
#endif
        /*  
            Auth must be added first to authorize all requests. File is last as a catch all.
         */
        if (httpLookupStage(http, "authFilter")) {
            httpAddHandler(location, "authFilter", "");
        }
        maLoadModule(appweb, "cgiHandler", "mod_cgi");
        if (httpLookupStage(http, "cgiHandler")) {
            httpAddHandler(location, "cgiHandler", ".cgi .cgi-nph .bat .cmd .pl .py");
            /*
                Add cgi-bin with a location block for the /cgi-bin URL prefix.
             */
            path = "cgi-bin";
            if (mprPathExists(host, path, X_OK)) {
                ap = maCreateAlias(host, "/cgi-bin/", path, 0);
                mprLog(host, 4, "ScriptAlias \"/cgi-bin/\":\"%s\"", path);
                maInsertAlias(host, ap);
                loc = httpCreateInheritedLocation(http, host->location);
                httpSetLocationPrefix(loc, "/cgi-bin/");
                httpSetHandler(loc, "cgiHandler");
                maAddLocation(host, loc);
            }
        }
        maLoadModule(appweb, "ejsHandler", "mod_ejs");
        if (httpLookupStage(http, "ejsHandler")) {
            httpAddHandler(location, "ejsHandler", ".ejs");
        }
        maLoadModule(appweb, "phpHandler", "mod_php");
        if (httpLookupStage(http, "phpHandler")) {
            httpAddHandler(location, "phpHandler", ".php");
        }
        if (httpLookupStage(http, "fileHandler")) {
            httpAddHandler(location, "fileHandler", "");
        }

    } else {
        /*
            Configure the http service and hosts specified in the config file.
         */
        path = mprGetAbsPath(server, configFile);
        if (maParseConfig(server, path) < 0) {
            /* mprUserError(server, "Can't configure server using %s", path); */
            return MPR_ERR_CANT_INITIALIZE;
        }
        mprFree(path);
    }
    return 0;
}


int maParseConfig(MaServer *server, cchar *configFile)
{
    Mpr             *mpr;
    MprList         *includes;
    MaAppweb        *appweb;
    MaHost          *defaultHost;
    MaConfigState   stack[MA_MAX_CONFIG_DEPTH], *state;
    MaHostAddress   *address;
    Http            *http;
    HttpServer      *httpServer;
    MaDir           *dir, *bestDir;
    MaHost          *host, *hp;
    MaAlias         *alias;
    MprDirEntry     *dp;
    HttpLimits      *limits;
    char            buf[MPR_MAX_STRING];
    char            *cp, *tok, *key, *value, *path;
    int             i, rc, top, next, nextAlias, len;

    mpr = mprGetMpr();
    mprLog(mpr, 2, "Config File %s", configFile);

    appweb = server->appweb;
    http = appweb->http;
    memset(stack, 0, sizeof(stack));
    server->alreadyLogging = mprGetLogHandler(server) ? 1 : 0;

    /*
        Create the default host and directory
     */
    defaultHost = host = maCreateHost(server, NULL, NULL);
    server->defaultHost = defaultHost;
    mprAddItem(server->hosts, host);

    top = 0;
    state = &stack[top];
    state->server = server;
    state->host = host;
    state->dir = maCreateBareDir(host, ".");
    state->location = defaultHost->location;
    state->location->connector = http->netConnector;
    state->enabled = 1;
    state->lineNumber = 0;

    state->filename = (char*) configFile;

    state->file = mprOpen(server, configFile, O_RDONLY | O_TEXT, 0444);
    if (state->file == 0) {
        mprError(server, "Can't open %s for config directives", configFile);
        return MPR_ERR_CANT_OPEN;
    }

    /*
        Set the default location authorization definition to match the default directory auth
     */
    state->location->auth = state->dir->auth;
    state->auth = state->dir->auth;

    maInsertDir(host, state->dir);
    maSetHostName(host, "Main Server");

    /*
        Parse each line in the config file
     */
    for (state->lineNumber = 1; top >= 0; state->lineNumber++) {

        state = &stack[top];
        mprAssert(state->file);
        if (mprGets(state->file, buf, sizeof(buf) - 1) == 0) {
            if (--top > 0 && strcmp(state->filename, stack[top].filename) == 0) {
                mprError(server, "Unclosed directives in %s", state->filename);
                while (top >= 0 && strcmp(state->filename, state[top].filename) == 0) {
                    top--;
                }
            }
            if (top >= 0 && state->file == stack[top].file) {
                stack[top].lineNumber = state->lineNumber + 1;
            } else {
                mprFree(state->file);
                state->file = 0;
                if (top >= 0) {
                    state = &stack[top];
                }
            }
            continue;
        }

        buf[sizeof(buf) - 1] = '\0';
        cp = buf;
        while (isspace((int) *cp)) {
            cp++;
        }
        if (*cp == '\0' || *cp == '#') {
            continue;
        }

        cp = mprStrTrim(cp, "\r\n\t ");
        key = mprStrTok(cp, " \t\n", &tok);
        value = mprStrTok(0, "\n", &tok);
        if (key == 0 || *key == '\0') {
            goto syntaxErr;
        }
        if (value) {
            while (isspace((int) *value)) {
                value++;
            }
            if (*value) {
                cp = &value[strlen(value) - 1];
                while (cp > value && isspace((int) *cp)) {
                    cp--;
                }
                *++cp = '\0';
            }

        } else {
            value = "";
        }

        if (mprStrcmpAnyCase(key, "Include") == 0) {
            state->lineNumber++;
            value = mprStrTrim(value, "\"");
            if ((cp = strchr(value, '*')) == 0) {
                state = pushState(server, state, &top);
                state->lineNumber = 0;
                state->filename = mprJoinPath(server, server->serverRoot, value);
                state->file = mprOpen(server, state->filename, O_RDONLY | O_TEXT, 0444);
                if (state->file == 0) {
                    mprError(server, "Can't open include file %s for config directives", state->filename);
                    return MPR_ERR_CANT_OPEN;
                }
                mprLog(server, 5, "Parsing config file: %s", state->filename);

            } else {
                /*
                    Process wild cards. This is very simple - only "*" is supported.
                 */
                *cp = '\0';
                len = (int) strlen(value);
                if (value[len - 1] == '/') {
                    value[len - 1] = '\0';
                }
                cp = mprJoinPath(server, server->serverRoot, value);
                includes = mprGetPathFiles(server, cp, 0);
                if (includes == 0) {
                    continue;
                }
                value = mprJoinPath(server, server->serverRoot, value);
                for (next = 0; (dp = mprGetNextItem(includes, &next)) != 0; ) {
                    state = pushState(server, state, &top);
                    state->filename = mprJoinPath(server, value, dp->name);
                    state->file = mprOpen(server, state->filename, O_RDONLY | O_TEXT, 0444);
                    if (state->file == 0) {
                        mprError(server, "Can't open include file %s for config directives", state->filename);
                        return MPR_ERR_CANT_OPEN;
                    }
                    mprLog(server, 5, "Parsing config file: %s", state->filename);
                }
                mprFree(includes);
            }
            continue;

        } else if (*key != '<') {

            if (!state->enabled) {
                mprLog(server, 8, "Skipping key %s at %s:%d", key, state->filename, state->lineNumber);
                continue;
            }

            /*
                Keywords outside of a virtual host or directory section
             */
            rc = processSetting(server, key, value, state);
            if (rc == 0) {
                char    *extraMsg;
                if (strcmp(key, "SSLEngine") == 0) {
                    extraMsg =
                        "\n\nFor SSL, you may have one SSL provider loaded.\n"
                        "Make sure that either OpenSSL or MatrixSSL is loaded.";
                } else {
                    extraMsg = "";
                }
                mprError(server,
                    "Ignoring unknown directive \"%s\"\nAt line %d in %s\n\n"
                    "Make sure the required modules are loaded. %s\n",
                    key, state->lineNumber, state->filename, extraMsg);
                continue;

            } else if (rc < 0) {
                mprError(server, "Ignoring bad directive \"%s\" at %s:%d in %s", key, state->filename, state->lineNumber, 
                    configFile);
            }
            continue;
        }

        mprLog(server, 9, "AT %d, key %s", state->lineNumber, key);

        /*
            Directory, Location and virtual host sections
         */
        key++;
        i = (int) strlen(key) - 1;
        if (key[i] == '>') {
            key[i] = '\0';
        }
        if (*key != '/') {
            if (!state->enabled) {
                state = pushState(server, state, &top);
                mprLog(server, 8, "Skipping key %s at %s:%d", key, state->filename, state->lineNumber);
                continue;
            }

            i = (int) strlen(value) - 1;
            if (value[i] == '>') {
                value[i] = '\0';
            }

            /*
                Opening tags
             */
            if (mprStrcmpAnyCase(key, "If") == 0) {
                value = mprStrTrim(value, "\"");

                /*
                    Want to be able to nest <if> directives.
                 */
                state = pushState(server, state, &top);
                state->enabled = featureSupported(server, value);
                if (!state->enabled) {
                    mprLog(server, 7, "If \"%s\" conditional is false at %s:%d", value, state->filename, state->lineNumber);
                }

            } else if (mprStrcmpAnyCase(key, "VirtualHost") == 0) {
                value = mprStrTrim(value, "\"");
                state = pushState(server, state, &top);
                state->host = host = maCreateVirtualHost(server, value, host);
                state->location = host->location;
                state->auth = host->location->auth;

                maAddHost(server, host);
                maSetVirtualHost(host);

                state->dir = maCreateDir(host, stack[top - 1].dir->path, stack[top - 1].dir);
                state->auth = state->dir->auth;
                maInsertDir(host, state->dir);

                mprLog(server, 2, "Virtual Host: %s ", value);
                if (maCreateHostAddresses(server, host, value) < 0) {
                    mprFree(host);
                    goto err;
                }

            } else if (mprStrcmpAnyCase(key, "Directory") == 0) {
                path = maMakePath(host, mprStrTrim(value, "\""));
                state = pushState(server, state, &top);

                if ((dir = maLookupDir(host, path)) != 0) {
                    /*
                        Allow multiple occurences of the same directory. Append directives.
                     */  
                    state->dir = dir;

                } else {
                    /*
                        Create a new directory inherit and parent directory settings. This means inherit authorization from
                        the enclosing host.
                     */
                    state->dir = maCreateDir(host, path, stack[top - 1].dir);
                    maInsertDir(host, state->dir);
                }
                state->auth = state->dir->auth;

            } else if (mprStrcmpAnyCase(key, "Location") == 0) {
                value = mprStrTrim(value, "\"");
                if (maLookupLocation(host, value)) {
                    mprError(server, "Location block already exists for \"%s\"", value);
                    goto err;
                }
                state = pushState(server, state, &top);
                state->location = httpCreateInheritedLocation(http, state->location);
                state->auth = state->location->auth;

                httpSetLocationPrefix(state->location, value);

                if (maAddLocation(host, state->location) < 0) {
                    mprError(server, "Can't add location %s", value);
                    goto err;
                }
                mprAssert(host->location->prefix);
            }

        } else {

            stack[top - 1].lineNumber = state->lineNumber + 1;
            key++;

            /*
                Closing tags
             */
            if (state->enabled && state->location != stack[top-1].location) {
                httpFinalizeLocation(state->location);
            }
            if (mprStrcmpAnyCase(key, "If") == 0) {
                top--;
                host = stack[top].host;

            } else if (mprStrcmpAnyCase(key, "VirtualHost") == 0) {
                top--;
                host = stack[top].host;

            } else if (mprStrcmpAnyCase(key, "Directory") == 0) {
                top--;

            } else if (mprStrcmpAnyCase(key, "Location") == 0) {
                top--;
            }
            if (top < 0) {
                goto syntaxErr;
            }
        }
    }

    /*
        Validate configuration
     */
    if (mprGetListCount(server->httpServers) == 0) {
        mprError(server, "Must have a Listen directive");
        return MPR_ERR_BAD_SYNTAX;
    }
    if (defaultHost->documentRoot == 0) {
        maSetDocumentRoot(defaultHost, ".");
    }

    /*
        TODO -- should test here that all location handlers are defined
     */
    limits = http->limits;
    if (limits->maxThreads > 0) {
        mprSetMaxWorkers(appweb, limits->maxThreads);
        mprSetMinWorkers(appweb, limits->minThreads);
    }

    /*
        Add default server listening addresses to the HostAddress hash. We pretend it is a vhost. Insert at the
        end of the vhost list so we become the default if no other vhost matches. Ie. vhosts take precedence
     */
    for (next = 0; (httpServer = mprGetNextItem(server->httpServers, &next)) != 0; ) {
        address = (MaHostAddress*) maLookupHostAddress(server, httpServer->ip, httpServer->port);
        if (address == 0) {
            address = maCreateHostAddress(server, httpServer->ip, httpServer->port);
            mprAddItem(server->hostAddresses, address);
        }
        maInsertVirtualHost(address, defaultHost);
    }

    if (strcmp(defaultHost->name, "Main Server") == 0) {
        defaultHost->name = 0;
    }

#if KEEP
    natServerName = 0;
    needServerName = (defaultHost->name == 0);

    for (next = 0; (httpServer = mprGetNextItem(server->httpServer, &next)) != 0; ) {
        ip = httpServer->ip;
        if (needServerName && *ip != '\0') {
            /*
                Try to get the most accessible server name possible.
             */
            if (strncmp(ip, "127.", 4) == 0 || strncmp(ip, "localhost:", 10) == 0) {
                if (! natServerName) {
                    maSetHostName(defaultHost, ip);
                    needServerName = 0;
                }
            } else {
                if (strncmp(ip, "10.", 3) == 0 || strncmp(ip, "192.168.", 8) == 0 || 
                        strncmp(ip, "172.16.", 7) == 0) {
                    natServerName = 1;
                } else {
                    maSetHostName(defaultHost, ip);
                    needServerName = 0;
                }
            }
        }
    }

    /*
        Last try to setup the server name if we don't have a non-local name.
     */
    if (needServerName && !natServerName) {
        /*
            This code is undesirable as it makes us dependent on DNS -- bad
         */
        if (natServerName) {
            /*
                TODO - but this code is not doing a DNS server lookup anymore
             */
            cchar *hostName = mprGetServerName(server);
            mprLog(server, 0, "WARNING: Missing ServerName directive, doing DNS lookup.");
            httpServer = mprGetFirstItem(server->httpServers);
            mprSprintf(ipAddrPort, sizeof(ipAddrPort), "%s:%d", hostName, httpServer->port);
            maSetHostName(defaultHost, hostName);

        } else {
            maSetHostName(defaultHost, defaultHost->ipAddrPort);
        }
        mprLog(server, 2, "Missing ServerName directive, ServerName set to: \"%s\"", defaultHost->name);
    }
#endif

    /*
        Validate all the hosts
     */
    for (next = 0; (hp = mprGetNextItem(server->hosts, &next)) != 0; ) {

        if (hp->documentRoot == 0) {
            maSetDocumentRoot(hp, defaultHost->documentRoot);
        }

        /*
            Ensure all hosts have mime types.
         */
        if (hp->mimeTypes == 0 || mprGetHashCount(hp->mimeTypes) == 0) {
            if (hp == defaultHost && defaultHost->mimeTypes) {
                hp->mimeTypes = defaultHost->mimeTypes;

            } else if (maOpenMimeTypes(hp, "mime.types") < 0) {
                static int once = 0;
                /*
                    Do minimal mime types configuration
                 */
                maAddStandardMimeTypes(hp);
                if (once++ == 0) {
                    mprLog(server, 2, "No mime.types file, using minimal mime configuration");
                }
            }
        }

        /*
            Check aliases have directory blocks. We must be careful to inherit authorization from the best 
            matching directory.
         */
        for (nextAlias = 0; (alias = mprGetNextItem(hp->aliases, &nextAlias)) != 0; ) {
            // mprLog(hp, 0, "Alias \"%s\" %s", alias->prefix, alias->filename);
            path = maMakePath(hp, alias->filename);
            bestDir = maLookupBestDir(hp, path);
            if (bestDir == 0) {
                bestDir = maCreateDir(hp, alias->filename, stack[0].dir);
                maInsertDir(hp, bestDir);
            }
            mprFree(path);
        }

        /*
            Define a ServerName if one has not been defined. Just use the IP:Port if defined.
         */
        if (host->name == 0) {
            if (host->ipAddrPort) {
                maSetHostName(hp, host->ipAddrPort);
            } else {
                maSetHostName(hp, mprGetHostName(host));
            }
        }
    }

    if (mprHasAllocError(mpr)) {
        mprError(server, "Memory allocation error when initializing");
        return MPR_ERR_NO_MEMORY;
    }
    return 0;

syntaxErr:
    mprError(server, "Syntax error in %s at %s:%d", configFile, state->filename, state->lineNumber);
    return MPR_ERR_BAD_SYNTAX;

err:
    mprError(server, "Error in %s at %s:%d", configFile, state->filename, state->lineNumber);
    return MPR_ERR_BAD_SYNTAX;
}


/*  Process the configuration settings. Permissible to modify key and value.
    Return < 0 for errors, zero if directive not found, otherwise 1 is success.
    TODO -- this function is quite big. Could be subject to a FEATURE.
 */
static int processSetting(MaServer *server, char *key, char *value, MaConfigState *state)
{
    MaAppweb        *appweb;
    MaAlias         *alias;
    HttpLocation      *location;
    MaHost          *host;
    MaDir           *dir;
    Http            *http;
    HttpAuth        *auth;
    HttpLimits      *limits;
    HttpStage       *stage;
    HttpServer      *httpServer;
    MprHash         *hp;
    char            ipAddrPort[MPR_MAX_IP_ADDR_PORT];
    char            *name, *path, *prefix, *cp, *tok, *ext, *mimeType, *url, *newUrl, *extensions, *codeStr, *hostName;
    char            *names, *type;
    int             len, port, rc, code, processed, num, flags, colonCount;

    mprAssert(state);
    mprAssert(key);

    appweb = server->appweb;
    http = appweb->http;
    httpServer = 0;
    host = state->host;
    dir = state->dir;
    location = state->location;
    mprAssert(state->location->prefix);

    mprAssert(host);
    mprAssert(dir);
    auth = state->auth;
    processed = 0;
    limits = http->limits;
    flags = 0;

    //  TODO - crashes with missing value
    //  TODO - need a real parser
    switch (toupper((int) key[0])) {
    case 'A':
        if (mprStrcmpAnyCase(key, "Alias") == 0) {
            /* Scope: server, host */
            if (maSplitConfigValue(server, &prefix, &path, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            prefix = maReplaceReferences(host, prefix);
            path = maMakePath(host, path);
            if (maLookupAlias(host, prefix)) {
                mprError(server, "Alias \"%s\" already exists", prefix);
                return MPR_ERR_BAD_SYNTAX;
            }
            alias = maCreateAlias(host, prefix, path, 0);
            mprLog(server, 4, "Alias: \"%s for \"%s\"", prefix, path);
            if (maInsertAlias(host, alias) < 0) {
                mprError(server, "Can't insert alias: %s", prefix);
                return MPR_ERR_BAD_SYNTAX;
            }
            mprFree(path);
            mprFree(prefix);
            return 1;

        } else if (mprStrcmpAnyCase(key, "AddFilter") == 0) {
            /* Scope: server, host, location */
            name = mprStrTok(value, " \t", &extensions);
            if (httpAddFilter(location, name, extensions, HTTP_STAGE_INCOMING | HTTP_STAGE_OUTGOING) < 0) {
                mprError(server, "Can't add filter %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "AddInputFilter") == 0) {
            /* Scope: server, host, location */
            name = mprStrTok(value, " \t", &extensions);
            if (httpAddFilter(location, name, extensions, HTTP_STAGE_INCOMING) < 0) {
                mprError(server, "Can't add filter %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "AddOutputFilter") == 0) {
            /* Scope: server, host, location */
            name = mprStrTok(value, " \t", &extensions);
            if (httpAddFilter(location, name, extensions, HTTP_STAGE_OUTGOING) < 0) {
                mprError(server, "Can't add filter %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "AddHandler") == 0) {
            /* Scope: server, host, location */
            name = mprStrTok(value, " \t", &extensions);
            if (httpAddHandler(state->location, name, extensions) < 0) {
                mprError(server, "Can't add handler %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "AddType") == 0) {
            if (maSplitConfigValue(server, &mimeType, &ext, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            maAddMimeType(host, ext, mimeType);
            return 1;

        } else if (mprStrcmpAnyCase(key, "Allow") == 0) {
            char *from, *spec;
            if (maSplitConfigValue(server, &from, &spec, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            /* spec can be: all, host, ipAddr */
            httpSetAuthAllow(auth, spec);
            return 1;

        } else if (mprStrcmpAnyCase(key, "AuthGroupFile") == 0) {
#if BLD_FEATURE_AUTH_FILE
            //  TODO - this belongs elsewhere
            path = maMakePath(host, mprStrTrim(value, "\""));
            if (httpReadGroupFile(http, auth, path) < 0) {
                mprError(http, "Can't open AuthGroupFile %s", path);
                return MPR_ERR_BAD_SYNTAX;
            }
            mprFree(path);
#else
            mprError(server, "AuthGroupFile directive not supported when --auth=pam");
#endif
            return 1;

        } else if (mprStrcmpAnyCase(key, "AuthMethod") == 0) {
            value = mprStrTrim(value, "\"");
            if (mprStrcmpAnyCase(value, "pam") == 0) {
                auth->backend = HTTP_AUTH_METHOD_PAM;
                return 1;

            } else if (mprStrcmpAnyCase(value, "file") == 0) {
                auth->backend = HTTP_AUTH_METHOD_FILE;
                return 1;

            } else {
                return MPR_ERR_BAD_SYNTAX;
            }

        } else if (mprStrcmpAnyCase(key, "AuthName") == 0) {
            //  TODO - should be httpSetAuthRealm
            httpSetAuthRealm(auth, mprStrTrim(value, "\""));
            return 1;
            
        } else if (mprStrcmpAnyCase(key, "AuthType") == 0) {
            //  TODO - config parser should trim there for us
            value = mprStrTrim(value, "\"");
            if (mprStrcmpAnyCase(value, "Basic") == 0) {
                auth->type = HTTP_AUTH_BASIC;

            } else if (mprStrcmpAnyCase(value, "Digest") == 0) {
                auth->type = HTTP_AUTH_DIGEST;

            } else {
                mprError(http, "Unsupported authorization protocol");
                return MPR_ERR_BAD_SYNTAX;
            }
            return 1;
            
        } else if (mprStrcmpAnyCase(key, "AuthUserFile") == 0) {
#if BLD_FEATURE_AUTH_FILE
            //  TODO - this belons elsewhere
            path = maMakePath(host, mprStrTrim(value, "\""));
            if (httpReadUserFile(http, auth, path) < 0) {
                mprError(http, "Can't open AuthUserFile %s", path);
                return MPR_ERR_BAD_SYNTAX;
            }
            mprFree(path);
#else
            mprError(server, "AuthUserFile directive not supported when --auth=pam");
#endif
            return 1;

        } else if (mprStrcmpAnyCase(key, "AuthDigestQop") == 0) {
            value = mprStrTrim(value, "\"");
            mprStrLower(value);
            if (strcmp(value, "none") != 0 && strcmp(value, "auth") != 0 && strcmp(value, "auth-int") != 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            httpSetAuthQop(auth, value);
            return 1;

        } else if (mprStrcmpAnyCase(key, "AuthDigestAlgorithm") == 0) {
            return 1;

        } else if (mprStrcmpAnyCase(key, "AuthDigestDomain") == 0) {
            return 1;

        } else if (mprStrcmpAnyCase(key, "AuthDigestNonceLifetime") == 0) {
            return 1;
        }
        break;

    case 'B':
        if (mprStrcmpAnyCase(key, "BrowserMatch") == 0) {
            return 1;
        }
        break;

    case 'C':
        if (mprStrcmpAnyCase(key, "CustomLog") == 0) {
#if !BLD_FEATURE_ROMFS
            char *format, *end;
            if (*value == '\"') {
                end = strchr(++value, '\"');
                if (end == 0) {
                    mprError(server, "Missing closing quote");
                    return MPR_ERR_BAD_SYNTAX;
                }
                *end++ = '\0';
                path = value;
                format = end;
                while ((int) isspace((int) *format)) {
                    format++;
                }

            } else {
                path = mprStrTok(value, " \t", &format);
            }
            if (path == 0 || format == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            path = maMakePath(host, path);
            maSetAccessLog(host, path, mprStrTrim(format, "\""));
            mprFree(path);
            maSetLogHost(host, host);
#endif
            return 1;
        }
        break;

    case 'D':
        if (mprStrcmpAnyCase(key, "Deny") == 0) {
            char *from, *spec;
            if (maSplitConfigValue(server, &from, &spec, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            httpSetAuthDeny(auth, spec);
            return 1;

        } else if (mprStrcmpAnyCase(key, "DirectoryIndex") == 0) {
            value = mprStrTrim(value, "\"");
            if (dir == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            maSetDirIndex(dir, value);
            return 1;

        } else if (mprStrcmpAnyCase(key, "DocumentRoot") == 0) {
            path = maMakePath(host, mprStrTrim(value, "\""));
#if !VXWORKS
            /*
                VxWorks stat() is broken if using a network FTP server.
             */
            if (!mprPathExists(server, path, X_OK)) {
                mprError(server, "Can't access DocumentRoot directory %s", path);
                return MPR_ERR_BAD_SYNTAX;
            }
#endif
            maSetDocumentRoot(host, path);
            maSetDirPath(dir, path);
            mprLog(server, MPR_CONFIG, "DocRoot (%s): \"%s\"", host->name, path);
            mprFree(path);
            return 1;
        }
        break;

    case 'E':
        if (mprStrcmpAnyCase(key, "ErrorDocument") == 0) {
            codeStr = mprStrTok(value, " \t", &url);
            if (codeStr == 0 || url == 0) {
                mprError(server, "Bad ErrorDocument directive");
                return MPR_ERR_BAD_SYNTAX;
            }
            httpAddErrorDocument(location, codeStr, url);
            return 1;

        } else if (mprStrcmpAnyCase(key, "ErrorLog") == 0) {
            path = mprStrTrim(value, "\"");
            if (path && *path) {
                if (server->alreadyLogging) {
                    mprLog(server, 4, "Already logging. Ignoring ErrorLog directive");
                } else {
                    maStopLogging(server);
                    if (strncmp(path, "stdout", 6) != 0) {
                        path = maMakePath(host, path);
                        rc = maStartLogging(server, path);
                        mprFree(path);
                    } else {
                        rc = maStartLogging(server, path);
                    }
                    if (rc < 0) {
                        mprError(server, "Can't write to ErrorLog");
                        return MPR_ERR_BAD_SYNTAX;
                    }
                }
            }
            return 1;
        }
        break;

    case 'G':
        if (mprStrcmpAnyCase(key, "Group") == 0) {
            value = mprStrTrim(value, "\"");
            maSetHttpGroup(appweb, value);
            return 1;
        }
        break;

    case 'H':
        break;

    case 'K':
        if (mprStrcmpAnyCase(key, "KeepAlive") == 0) {
            if (mprStrcmpAnyCase(value, "on") != 0) {
                maSetMaxKeepAlive(appweb, 0);
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "KeepAliveTimeout") == 0) {
            if (! mprGetDebugMode(server)) {
                maSetKeepAliveTimeout(appweb, atoi(value) * 1000);
            }
            return 1;
        }
        break;

    case 'L':
        if (mprStrcmpAnyCase(key, "LimitChunkSize") == 0) {
            num = atoi(value);
            if (num < MA_BOT_CHUNK_SIZE || num > MA_TOP_CHUNK_SIZE) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxChunkSize = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitClients") == 0) {
            mprSetMaxSocketClients(server, atoi(value));
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitMemoryMax") == 0) {
            mprSetAllocLimits(server, -1, atoi(value));
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitMemoryRedline") == 0) {
            mprSetAllocLimits(server, atoi(value), -1);
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitRequestBody") == 0) {
            num = atoi(value);
            if (num < MA_BOT_BODY || num > MA_TOP_BODY) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxReceiveBody = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitRequestFields") == 0) {
            num = atoi(value);
            if (num < MA_BOT_NUM_HEADERS || num > MA_TOP_NUM_HEADERS) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxNumHeaders = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitRequestFieldSize") == 0) {
            num = atoi(value);
            if (num < MA_BOT_HEADER || num > MA_TOP_HEADER) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxHeader = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitResponseBody") == 0) {
            num = atoi(value);
            if (num < MA_BOT_RESPONSE_BODY || num > MA_TOP_RESPONSE_BODY) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxTransmissionBody = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitStageBuffer") == 0) {
            num = atoi(value);
            if (num < MA_BOT_STAGE_BUFFER || num > MA_TOP_STAGE_BUFFER) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxStageBuffer = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitUrl") == 0) {
            num = atoi(value);
            if (num < MA_BOT_URL || num > MA_TOP_URL){
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxUri = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "LimitUploadSize") == 0) {
            num = atoi(value);
            if (num != -1 && (num < MA_BOT_UPLOAD_SIZE || num > MA_TOP_UPLOAD_SIZE)){
                //  TODO - should emit a message for all these cases
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxUploadSize = num;
            return 1;

#if DEPRECATED
        } else if (mprStrcmpAnyCase(key, "ListenIF") == 0) {
            MprList         *ipList;
            MprInterface    *ip;

            /*
                Options:
                    interface:port
                    interface   (default port HTTP_DEFAULT_PORT)
             */
            if ((cp = strchr(value, ':')) != 0) {           /* interface:port */
                do {                                        /* find last colon */
                    tok = cp;
                    cp = strchr(cp + 1, ':');
                } while (cp != 0);
                cp = tok;
                *cp++ ='\0';

                port = atoi(cp);
                if (port <= 0 || port > 65535) {
                    mprError(server, "Bad listen port number %d", port);
                    return MPR_ERR_BAD_SYNTAX;
                }

            } else {            /* interface */
                port = HTTP_DEFAULT_PORT;
            }

            ipList = mprGetMpr()->socketService->getInterfaceList();
            ip = (MprInterface*) ipList->getFirst();
            if (ip == 0) {
                mprError(server, "Can't find interfaces, use Listen-directive with IP address.");
                return MPR_ERR_BAD_SYNTAX;
            }
            while (ip) {
                if (mprStrcmpAnyCase(ip->name, value) != 0) {
                    ip = (MprInterface*) ipList->getNext(ip);
                    continue;
                }
                server.insert(new HttpServer(ip->ip, port));
                if (host->ipAddrPort == 0) {
                    mprSprintf(ipAddrPort, sizeof(ipAddrPort), "%s:%d", ip->ip, port);
                    maSetIpAddrPort(host, ipAddrPort);
                }
                break;
            }
            return 1;
#endif

        } else if (mprStrcmpAnyCase(key, "Listen") == 0) {

            /*
                Options:
                    ipAddr:port
                    ipAddr          default port MA_SERVER_DEFAULT_PORT_NUM
                    port            All ip interfaces on this port
             *
                Where ipAddr may be "::::::" for ipv6 addresses or may be enclosed in "[]" if appending a port.
             */

            value = mprStrTrim(value, "\"");

            if (isdigit((int) *value) && strchr(value, '.') == 0 && strchr(value, ':') == 0) {
                /*
                    Port only, listen on all interfaces (ipv4 + ipv6)
                 */
                port = atoi(value);
                if (port <= 0 || port > 65535) {
                    mprError(server, "Bad listen port number %d", port);
                    return MPR_ERR_BAD_SYNTAX;
                }
                hostName = "";
                flags = MA_LISTEN_WILD_IP;

            } else {
                colonCount = 0;
                for (cp = (char*) value; ((*cp != '\0') && (colonCount < 2)) ; cp++) {
                    if (*cp == ':') {
                        colonCount++;
                    }
                }
                /*
                    Hostname with possible port number. Port number parsed if address enclosed in "[]"
                 */
                if (colonCount > 1) {
                    /* ipv6 */
                    hostName = value;
                    if (*value == '[' && (cp = strrchr(cp, ':')) != 0) {
                        *cp++ = '\0';
                        port = atoi(cp);
                    } else {
                        port = HTTP_DEFAULT_PORT;
                        flags = MA_LISTEN_DEFAULT_PORT;
                    }

                } else {
                    /* ipv4 */
                    hostName = value;
                    if ((cp = strrchr(value, ':')) != 0) {
                        *cp++ = '\0';
                        port = atoi(cp);

                    } else {
                        port = HTTP_DEFAULT_PORT;
                        flags = MA_LISTEN_DEFAULT_PORT;
                    }
                    if (*hostName == '[') {
                        hostName++;
                    }
                    len = strlen(hostName);
                    if (hostName[len - 1] == ']') {
                        hostName[len - 1] = '\0';
                    }
                }
            }
            //  TODO - need api insead of mprAddItem
            mprAddItem(server->httpServers, httpCreateServer(http, hostName, port, NULL));

            /*
                Set the host ip spec if not already set
             */
            if (host->ipAddrPort == 0) {
                mprSprintf(ipAddrPort, sizeof(ipAddrPort), "%s:%d", hostName, port);
                maSetHostIpAddrPort(host, ipAddrPort);
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "LogFormat") == 0) {
            return 1;

        } else if (mprStrcmpAnyCase(key, "LogLevel") == 0) {
            if (server->alreadyLogging) {
                mprLog(server, 4, "Already logging. Ignoring LogLevel directive");

            } else {
                int level;
                value = mprStrTrim(value, "\"");
                level = atoi(value);
                mprSetLogLevel(server, level);
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "LoadModulePath") == 0) {
            value = mprStrTrim(value, "\"");
            path = mprStrcat(server, -1, value, MPR_SEARCH_SEP, mprGetAppDir(server), NULL);
            mprSetModuleSearchPath(server, path);
            mprFree(path);
            return 1;

        } else if (mprStrcmpAnyCase(key, "LoadModule") == 0) {
            name = mprStrTok(value, " \t", &tok);
            if (name == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            path = mprStrTok(0, "\n", &tok);
            if (path == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            if (maLoadModule(appweb, name, path) < 0) {
                /*  Error messages already done */
                return MPR_ERR_CANT_CREATE;
            }
            return 1;
        }
        break;

    case 'M':
        if (mprStrcmpAnyCase(key, "MaxKeepAliveRequests") == 0) {
            maSetMaxKeepAlive(appweb, atoi(value));
            return 1;

        } else if (mprStrcmpAnyCase(key, "MemoryDepletionPolicy") == 0) {
            mprStrLower(value);
            if (strcmp(value, "exit") == 0) {
                mprSetAllocPolicy(server, MPR_ALLOC_POLICY_EXIT);
            } else if (strcmp(value, "restart") == 0) {
                mprSetAllocPolicy(server, MPR_ALLOC_POLICY_RESTART);
            }
            mprSetAllocLimits(server, atoi(value), -1);
            return 1;
        }
        break;

    case 'N':
        if (mprStrcmpAnyCase(key, "NameVirtualHost") == 0) {
            mprLog(server, 2, "NameVirtual Host: %s ", value);
            if (maCreateHostAddresses(server, 0, value) < 0) {
                return -1;
            }
            return 1;
        }
        break;

    case 'O':
        if (mprStrcmpAnyCase(key, "Order") == 0) {
            if (mprStrcmpAnyCase(mprStrTrim(value, "\""), "Allow,Deny") == 0) {
                httpSetAuthOrder(auth, HTTP_ALLOW_DENY);
            } else {
                httpSetAuthOrder(auth, HTTP_DENY_ALLOW);
            }
            return 1;
        }
        break;

    case 'P':
        if (mprStrcmpAnyCase(key, "Protocol") == 0) {
            if (strcmp(value, "HTTP/1.0") == 0) {
                maSetHttpVersion(host, 0);
            } else if (strcmp(value, "HTTP/1.1") == 0) {
                maSetHttpVersion(host, 1);
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "PutMethod") == 0) {
            if (mprStrcmpAnyCase(value, "on") == 0) {
                location->flags |= HTTP_LOC_PUT_DELETE;
            } else {
                location->flags &= ~HTTP_LOC_PUT_DELETE;
            }
            return 1;
        }
        break;

    case 'R':
        if (mprStrcmpAnyCase(key, "Redirect") == 0) {
            if (value[0] == '/' || value[0] == 'h') {
                code = 302;
                url = mprStrTok(value, " \t", &tok);

            } else if (isdigit((int) value[0])) {
                cp = mprStrTok(value, " \t", &tok);
                code = atoi(cp);
                url = mprStrTok(0, " \t\n", &tok);

            } else {
                cp = mprStrTok(value, " \t", &tok);
                if (strcmp(value, "permanent") == 0) {
                    code = 301;
                } else if (strcmp(value, "temp") == 0) {
                    code = 302;
                } else if (strcmp(value, "seeother") == 0) {
                    code = 303;
                } else if (strcmp(value, "gone") == 0) {
                    code = 410;
                } else {
                    return MPR_ERR_BAD_SYNTAX;
                }
                url = mprStrTok(0, " \t\n", &tok);
            }
            if (code >= 300 && code <= 399) {
                newUrl = mprStrTok(0, "\n", &tok);
            } else {
                newUrl = "";
            }
            if (code <= 0 || url == 0 || newUrl == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            url = mprStrTrim(url, "\"");
            newUrl = mprStrTrim(newUrl, "\"");
            mprLog(server, 4, "insertAlias: Redirect %d from \"%s\" to \"%s\"", code, url, newUrl);
            alias = maCreateAlias(host, url, newUrl, code);
            maInsertAlias(host, alias);
            return 1;

        } else if (mprStrcmpAnyCase(key, "Require") == 0) {
            if (maGetConfigValue(http, &type, value, &tok, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            if (mprStrcmpAnyCase(type, "acl") == 0) {
#if BLD_FEATURE_AUTH_FILE
                HttpAcl   acl;
                char    *aclSpec;
                aclSpec = mprStrTrim(tok, "\"");
                acl = httpParseAcl(auth, aclSpec);
                httpSetRequiredAcl(auth, acl);
#else
            mprError(server, "Acl directive not supported when --auth=pam");
#endif

            } else if (mprStrcmpAnyCase(type, "valid-user") == 0) {
                httpSetAuthAnyValidUser(auth);

            } else {
                names = mprStrTrim(tok, "\"");
                if (mprStrcmpAnyCase(type, "user") == 0) {
                    httpSetAuthRequiredUsers(auth, names);

                } else if (mprStrcmpAnyCase(type, "group") == 0) {
                    httpSetAuthRequiredGroups(auth, names);

                } else {
                    mprError(http, "Bad Require syntax: %s", type);
                    return MPR_ERR_BAD_SYNTAX;
                }
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "ResetPipeline") == 0) {
            httpResetPipeline(location);
            return 1;
        }
        break;

    case 'S':
        if (mprStrcmpAnyCase(key, "ServerName") == 0) {
            value = mprStrTrim(value, "\"");
            if (strncmp(value, "http://", 7) == 0) {
                maSetHostName(host, &value[7]);
            } else {
                maSetHostName(host, value);
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "ServerRoot") == 0) {
            path = maMakePath(host, mprStrTrim(value, "\""));
            maSetServerRoot(server, path);
#if BLD_FEATURE_ROMFS
            mprLog(server, MPR_CONFIG, "Server Root \"%s\" in ROM", path);
#else
            mprLog(server, MPR_CONFIG, "Server Root \"%s\"", path);
#endif
            mprFree(path);
            return 1;

        } else if (mprStrcmpAnyCase(key, "SetConnector") == 0) {
            /* Scope: server, host, location */
            value = mprStrTrim(value, "\"");
            if (httpSetConnector(location, value) < 0) {
                mprError(server, "Can't add handler %s", value);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "SetHandler") == 0) {
            /* Scope: server, host, location */
            name = mprStrTok(value, " \t", &extensions);
            if (httpSetHandler(state->location, name) < 0) {
                mprError(server, "Can't add handler %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (mprStrcmpAnyCase(key, "StartThreads") == 0) {
            num = atoi(value);
            if (num < 0 || num > MA_TOP_THREADS) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->minThreads = num;
            return 1;
        }
        break;

    case 'T':
        if (mprStrcmpAnyCase(key, "ThreadLimit") == 0) {
            num = atoi(value);
            if (num < 0 || num > MA_TOP_THREADS) {
                return MPR_ERR_BAD_SYNTAX;
            }
            limits->maxThreads = num;
            return 1;

        } else if (mprStrcmpAnyCase(key, "ThreadStackSize") == 0) {
            num = atoi(value);
            if (num < MA_BOT_STACK || num > MA_TOP_STACK) {
                return MPR_ERR_BAD_SYNTAX;
            }
            mprSetThreadStackSize(server, num);
            return 1;

        } else if (mprStrcmpAnyCase(key, "TimeOut") == 0) {
            maSetTimeout(appweb, atoi(value) * 1000);
            return 1;

        } else if (mprStrcmpAnyCase(key, "TraceMethod") == 0) {
            httpEnableTraceMethod(http, mprStrcmpAnyCase(value, "on") == 0);
            return 1;

        } else if (mprStrcmpAnyCase(key, "TypesConfig") == 0) {
            path = maMakePath(host, mprStrTrim(value, "\""));
            if (maOpenMimeTypes(host, path) < 0) {
                mprError(server, "Can't open TypesConfig mime file %s", path);
                maAddStandardMimeTypes(host);
                mprFree(path);
                return MPR_ERR_BAD_SYNTAX;
            }
            mprFree(path);
            return 1;
        }
        break;

    case 'U':
        if (mprStrcmpAnyCase(key, "UploadDir") == 0 || mprStrcmpAnyCase(key, "FileUploadDir") == 0) {
            path = maMakePath(host, mprStrTrim(value, "\""));
            mprFree(location->uploadDir);
            location->uploadDir = mprStrdup(location, path);
            mprLog(http, MPR_CONFIG, "Upload directory: %s", path);
            mprFree(path);
            return 1;

        } else if (mprStrcmpAnyCase(key, "UploadAutoDelete") == 0) {
            value = mprStrTrim(value, "\"");
            location->autoDelete = (mprStrcmpAnyCase(value, "on") == 0);
            return 1;

        } else if (mprStrcmpAnyCase(key, "User") == 0) {
            maSetHttpUser(appweb, mprStrTrim(value, "\""));
            return 1;
        }
        break;
    }

    rc = 0;

    /*
        Allow all stages to parse the request
     */
    hp = mprGetFirstHash(http->stages);
    while (hp) {
        stage = (HttpStage*) hp->data;
        if (stage->parse) {
            rc = stage->parse(http, key, value, state);
        }
        if (rc < 0) {
            return rc;
        } else if (rc > 0) {
            break;
        }
        hp = mprGetNextHash(http->stages, hp);
    }
    return rc;
}


/*
    Create a location block for a handler and an alias. Convenience routine for ScriptAlias, EjsAppAlias, EjsAppDirAlias.
 */
HttpLocation *maCreateLocationAlias(Http *http, MaConfigState *state, cchar *prefixArg, cchar *pathArg, cchar *handlerName, 
        int flags)
{
    MaHost          *host;
    MaAlias         *alias;
    HttpLocation    *location;
    char            *path, *prefix;

    host = state->host;

    prefix = maReplaceReferences(host, prefixArg);
    path = maMakePath(host, pathArg);

    /*
        Create an ejs application location block and alias
     */
    alias = maCreateAlias(host, prefix, path, 0);
    maInsertAlias(host, alias);
    mprLog(http, 4, "Alias \"%s\" for \"%s\"", prefix, path);
    mprFree(path);

    if (maLookupLocation(host, prefix)) {
        mprError(http, "Location block already exists for \"%s\"", prefix);
        return 0;
    }
    location = httpCreateInheritedLocation(http, state->location);
    httpSetLocationAuth(location, state->dir->auth);
    httpSetLocationPrefix(location, prefix);
    maAddLocation(host, location);
    httpSetLocationFlags(location, flags);
    httpSetHandler(location, handlerName);
    mprFree(prefix);
    return location;
}


/*
    Make a path name. This replaces $references, converts to an absolute path name, cleans the path and maps delimiters.
 */
char *maMakePath(MaHost *host, cchar *file)
{
    MaServer      *server;
    char        *result, *path;

    mprAssert(file);

    server = host->server;

    if ((path = maReplaceReferences(host, file)) == 0) {
        /*  Overflow */
        return 0;
    }
    if (*path == '\0' || strcmp(path, ".") == 0) {
        result = mprStrdup(host, server->serverRoot);

#if BLD_WIN_LIKE
    } else if (*path != '/' && path[1] != ':' && path[2] != '/') {
        result = mprJoinPath(host, server->serverRoot, path);
#elif VXWORKS
    } else if (strchr((path), ':') == 0 && *path != '/') {
        result = mprJoinPath(host, server->serverRoot, path);
#else
    } else if (*path != '/') {
        result = mprJoinPath(host, server->serverRoot, path);
#endif
    } else {
        result = mprGetAbsPath(server, path);
    }
    mprFree(path);
    return result;
}


static int matchRef(cchar *key, char **src)
{
    int     len;

    mprAssert(src);
    mprAssert(key && *key);

    len = strlen(key);
    if (strncmp(*src, key, len) == 0) {
        *src += len;
        return 1;
    }
    return 0;
}


/*
    Replace a limited set of $VAR references. Currently support DOCUMENT_ROOT, SERVER_ROOT and PRODUCT
    TODO - Expand and formalize this. Should support many more variables.
 */
char *maReplaceReferences(MaHost *host, cchar *str)
{
    MprBuf  *buf;
    char    *src;
    char    *result;

    buf = mprCreateBuf(host, 0, 0);

    for (src = (char*) str; *src; ) {
        if (*src == '$') {
            ++src;
            if (matchRef("DOCUMENT_ROOT", &src)) {
                mprPutStringToBuf(buf, host->documentRoot);
                continue;

            } else if (matchRef("SERVER_ROOT", &src)) {
                mprPutStringToBuf(buf, host->server->serverRoot);
                continue;

            } else if (matchRef("PRODUCT", &src)) {
                mprPutStringToBuf(buf, BLD_PRODUCT);
                continue;
            }
        }
        mprPutCharToBuf(buf, *src++);
    }
    mprAddNullToBuf(buf);
    result = mprStealBuf(host, buf);
    mprFree(buf);
    return result;
}


/*
    Max stack depth is:
        Default Server          Level 1
            <VirtualHost>       Level 2
                <Directory>     Level 3
                    <Location>  Level 4
 *
 */
static MaConfigState *pushState(MprCtx ctx, MaConfigState *state, int *top)
{
    MaConfigState   *next;

    (*top)++;
    if (*top >= MA_MAX_CONFIG_DEPTH) {
        mprError(ctx, "Too many nested directives in config file at %s:%d", state->filename, state->lineNumber);
        return 0;
    }
    next = state + 1;
    next->server = state->server;
    next->host = state->host;
    next->location = state->location;
    next->dir = state->dir;
    next->auth = state->auth;
    next->lineNumber = state->lineNumber;
    next->enabled = state->enabled;
    next->filename = state->filename;
    next->file = state->file;

    return next;
}


static bool featureSupported(MprCtx ctx, char *key)
{
    //  TODO - should always create a conditional directive when a module is loaded. Even for user modules.
    if (mprStrcmpAnyCase(key, "BLD_COMMERCIAL") == 0) {
        return strcmp(BLD_COMMERCIAL, "0") == 0;

#ifdef BLD_DEBUG
    } else if (mprStrcmpAnyCase(key, "BLD_DEBUG") == 0) {
        return BLD_DEBUG;
#endif

#ifdef BLD_FEATURE_CGI
    } else if (mprStrcmpAnyCase(key, "CGI_MODULE") == 0) {
        return BLD_FEATURE_CGI;
#endif

    } else if (mprStrcmpAnyCase(key, "DIR_MODULE") == 0) {
        return 1;

    } else if (mprStrcmpAnyCase(key, "EGI_MODULE") == 0) {
        return 1;

#ifdef BLD_FEATURE_EJS
    } else if (mprStrcmpAnyCase(key, "EJS_MODULE") == 0) {
        return BLD_FEATURE_EJS;
#endif

#ifdef BLD_FEATURE_PHP
    } else if (mprStrcmpAnyCase(key, "PHP_MODULE") == 0) {
        return BLD_FEATURE_PHP;
#endif

#ifdef BLD_FEATURE_SSL
    } else if (mprStrcmpAnyCase(key, "SSL_MODULE") == 0) {
        return BLD_FEATURE_SSL;
#endif
    }
    mprError(ctx, "Unknown conditional \"%s\"", key);
    return 0;
}


int maSplitConfigValue(MprCtx ctx, char **s1, char **s2, char *buf, int quotes)
{
    char    *next;

    if (maGetConfigValue(ctx, s1, buf, &next, quotes) < 0 || maGetConfigValue(ctx, s2, next, &next, quotes) < 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (*s1 == 0 || *s2 == 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


int maGetConfigValue(MprCtx ctx, char **arg, char *buf, char **nextToken, int quotes)
{
    char    *endp;

    if (buf == 0) {
        return -1;
    }
    while (isspace((int) *buf)) {
        buf++;
    }

    if (quotes && *buf == '\"') {
        *arg = ++buf;
        if ((endp = strchr(buf, '\"')) != 0) {
            *endp++ = '\0';
        } else {
            return MPR_ERR_BAD_SYNTAX;
        }
        while ((int) isspace((int) *endp)) {
            endp++;
        }
        *nextToken = endp;
    } else {
        *arg = mprStrTok(buf, " \t\n", nextToken);
    }
    return 0;
}


#if BLD_FEATURE_CONFIG_SAVE
/*
    Save the configuration to the named config file
 */
int HttpServer::saveConfig(char *configFile)
{
    MaAlias         *alias;
    MaDir           *dp, *defaultDp;
    MprFile         out;
    HttpStage     *hanp;
    MaHost          *host, *defaultHost;
    HttpLimits        *limits;
    HttpServer        *listen;
    HttpLocation      *loc;
    MaMimeHashEntry *mt;
    MprHashTable    *mimeTypes;
    MprList         *aliases;
    MaModule        *mp;
    MprList         *modules;
    MprLogService   *logService;
    char            *ext, *path, *cp, *mimeFile;
    char            *hostName, *actionProgram;
    int             fd, indent, flags, first, code;
    char            *logSpec;

    indent = 0;
    host = 0;
    defaultHost = (MaHost*) hosts.getFirst();

    fd = open(configFile, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0666);
    if (fd < 0) {
        mprLog(server, 0, "saveConfig: Can't open %s", configFile);
        return MPR_ERR_CANT_OPEN;
    }

    mprFprintf(fd, \
    "#\n"
    "#  Configuration for %s\n"
    "#\n"
    "#  This file is dynamically generated. If you edit this file, your\n"
    "#  changes may not be preserved by the manager. PLEASE keep a backup of\n"
    "#  the file before and after all manual changes.\n"
    "#\n"
    "#  The order of configuration directives matters as this file is parsed\n"
    "#  only once. You must put the server root and error log definitions\n"
    "#  first ensure configuration errors are logged.\n"
    "#\n\n", BLD_NAME);

    mprFprintf(fd, "ServerRoot \"%s\"\n", serverRoot);

    logService = mprGetMpr()->logService;
    logSpec = mprStrdup(logService->getLogSpec());
    if ((cp = strchr(logSpec, ':')) != 0) {
        *cp = '\0';
    }
    mprFprintf(fd, "ErrorLog \"%s\"\n", logSpec);
    mprFprintf(fd, "LogLevel %d\n", logService->getDefaultLevel());

    /*
        Listen directives
     */
    listen = (HttpServer*) servers.getFirst();
    while (listen) {
        flags = listen->getFlags();
        if (flags & MA_LISTEN_DEFAULT_PORT) {
            mprFprintf(fd, "Listen %s # %d\n", listen->getIpAddr(), listen->getPort());
        } else if (flags & MA_LISTEN_WILD_IP) {
            mprFprintf(fd, "Listen %d\n", listen->getPort());
        } else if (flags & MA_LISTEN_WILD_IP2) {
            /*  Ignore */
        } else {
            if (strchr(listen->getIpAddr(), '.') != 0) {
                mprFprintf(fd, "Listen %s:%d\n", listen->getIpAddr(),
                    listen->getPort());
            } else {
                mprFprintf(fd, "Listen [%s]:%d\n", listen->getIpAddr(),
                    listen->getPort());
            }
        }
        listen = (HttpServer*) servers.getNext(listen);
    }

    /*
        Global directives
     */
    mprFprintf(fd, "User %s\n", http->getUser());
    mprFprintf(fd, "Group %s\n", http->getGroup());

    mprFprintf(fd, "\n#\n#  Loadable Modules\n#\n");
    mprFprintf(fd, "LoadModulePath %s\n", defaultHost->getModuleDirs());
    modules = http->getModules();
    mp = (MaModule*) modules->getFirst();
    while (mp) {
        mprFprintf(fd, "LoadModule %s lib%sModule\n", mp->name, mp->name);
        mp = (MaModule*) modules->getNext(mp);
    }

    /*
        For clarity, always print the ThreadLimit even if default.
     */
    limits = http->getLimits();
    mprFprintf(fd, "ThreadLimit %d\n", limits->maxThreads);
    if (limits->threadStackSize != 0) {
        mprFprintf(fd, "ThreadStackSize %d\n", limits->threadStackSize);
    }
    if (limits->minThreads != 0) {
        mprFprintf(fd, "\nStartThreads %d\n", limits->minThreads);
    }
    if (limits->maxReceiveBody != MA_MAX_BODY) {
        mprFprintf(fd, "LimitRequestBody %d\n", limits->maxReceiveBody);
    }
    if (limits->maxTransmissionBody != MA_MAX_RESPONSE_BODY) {
        mprFprintf(fd, "LimitResponseBody %d\n", limits->maxTransmissionBody);
    }
    if (limits->maxNumHeaders != MA_MAX_NUM_HEADERS) {
        mprFprintf(fd, "LimitRequestFields %d\n", limits->maxNumHeaders);
    }
    if (limits->maxHeader != MA_MAX_HEADERS) {
        mprFprintf(fd, "LimitRequestFieldSize %d\n", limits->maxHeader);
    }
    if (limits->maxUri != MPR_MAX_URL) {
        mprFprintf(fd, "LimitUrl %d\n", limits->maxUri);
    }
    if (limits->maxUploadSize != MA_MAX_UPLOAD_SIZE) {
        mprFprintf(fd, "LimitUploadSize %d\n", limits->maxUploadSize);
    }

    /*
        Virtual hosts. The first host is the default server
     */
    host = (MaHost*) hosts.getFirst();
    while (host) {
        mprFprintf(fd, "\n");
        if (host->isVhost()) {
            if (host->isNamedVhost()) {
                mprFprintf(fd, "NameVirtualHost %s\n", host->getIpSpec());
            }
            mprFprintf(fd, "<VirtualHost %s>\n", host->getIpSpec());
            indent++;
        }

        hostName = host->getName();
        if (strcmp(hostName, "default") != 0) {
            tabs(fd, indent);
            mprFprintf(fd, "ServerName http://%s\n", hostName);
        }

        tabs(fd, indent);
        mprFprintf(fd, "DocumentRoot %s\n", host->getDocumentRoot());

        /*
            Handlers
         */
        flags = host->getFlags();
        if (flags & MA_ADD_HANDLER) {
            mprFprintf(fd, "\n");
            if (flags & MA_RESET_HANDLERS) {
                tabs(fd, indent);
                mprFprintf(fd, "ResetPipeline\n");
            }
            hanp = (HttpStage*) host->getHandlers()->getFirst();
            while (hanp) {
                ext = (char*) (hanp->getExtensions() ?
                    hanp->getExtensions() : "");
                tabs(fd, indent);
                mprFprintf(fd, "AddHandler %s %s\n", hanp->getName(), ext);
                hanp = (HttpStage*) host->getHandlers()->getNext(hanp);
            }
        }

#if BLD_FEATURE_SSL
        /*
            SSL configuration
         */
        if (host->isSecure()) {
            MaSslConfig *sslConfig;
            MaSslModule *sslModule;

            mprFprintf(fd, "\n");
            tabs(fd, indent);
            mprFprintf(fd, "SSLEngine on\n");
            sslModule = (MaSslModule*) http->findModule("ssl");
            if (sslModule != 0) {
                sslConfig = sslModule->getSslConfig(host->getName());
                if (sslConfig != 0) {

                    tabs(fd, indent);
                    mprFprintf(fd, "SSLCipherSuite %s\n",
                        sslConfig->getCipherSuite());

                    tabs(fd, indent);
                    mprFprintf(fd, "SSLProtocol ");
                    int protoMask = sslConfig->getSslProto();
                    if (protoMask == MA_PROTO_ALL) {
                        mprFprintf(fd, "ALL");
                    } else if (protoMask ==
                        (MA_PROTO_ALL & ~MA_PROTO_SSLV2)) {
                        mprFprintf(fd, "ALL -SSLV2");
                    } else {
                        if (protoMask & MA_PROTO_SSLV2) {
                            mprFprintf(fd, "SSLv2 ");
                        }
                        if (protoMask & MA_PROTO_SSLV3) {
                            mprFprintf(fd, "SSLv3 ");
                        }
                        if (protoMask & MA_PROTO_TLSV1) {
                            mprFprintf(fd, "TLSv1 ");
                        }
                    }
                    mprFprintf(fd, "\n");

                    if ((path = sslConfig->getCertFile()) != 0) {
                        tabs(fd, indent);
                        mprFprintf(fd, "SSLCertificateFile %s\n", path);
                    }
                    if ((path = sslConfig->getKeyFile()) != 0) {
                        tabs(fd, indent);
                        mprFprintf(fd, "SSLCertificateKeyFile %s\n", path);
                    }
                    if (sslConfig->getVerifyClient()) {
                        tabs(fd, indent);
                        mprFprintf(fd, "SSLVerifyClient require\n");
                        if ((path = sslConfig->getCaFile()) != 0) {
                            tabs(fd, indent);
                            mprFprintf(fd, "SSLCaCertificateFile %s\n", path);
                        }
                        if ((path = sslConfig->getCaPath()) != 0) {
                            tabs(fd, indent);
                            mprFprintf(fd, "SSLCertificatePath %s\n", path);
                        }
                    }
                }
            }
        }
#endif
        /*
            General per-host directives
         */
        if (! host->getKeepAlive()) {
            tabs(fd, indent);
            mprFprintf(fd, "KeepAlive off\n");
        } else {
            if (host->getMaxKeepAlive() != defaultHost->getMaxKeepAlive()) {
                tabs(fd, indent);
                mprFprintf(fd, "MaxKeepAliveRequests %d\n",
                    host->getMaxKeepAlive());
            }
            if (host->getKeepAliveTimeout() !=
                    defaultHost->getKeepAliveTimeout()) {
                tabs(fd, indent);
                mprFprintf(fd, "KeepAliveTimeout %d\n",
                    host->getKeepAliveTimeout() / 1000);
            }
        }
        mimeFile = host->getMimeFile();
        if (mimeFile && *mimeFile) {
            mprFprintf(fd, "TypesConfig %s\n", mimeFile);
        }
        if (host->getTimeout() != defaultHost->getTimeout()) {
            tabs(fd, indent);
            mprFprintf(fd, "Timeout %d\n", host->getTimeout() / 1000);
        }

        if (host->getSessionTimeout() != defaultHost->getSessionTimeout()) {
            tabs(fd, indent);
            mprFprintf(fd, "SessionTimeout %d\n", host->getSessionTimeout());
        }
#if !BLD_FEATURE_ROMFS
        if (host->getLogHost() == host) {
            char    format[MPR_MAX_FNAME * 2];
            char    *fp;
            fp = format;
            format[0] = '\0';
            for (cp = host->getLogFormat();
                    cp && *cp && fp < &format[sizeof(format) - 4]; cp++) {
                if (*cp == '\"') {
                    *fp++ = '\\';
                    *fp++ = *cp;
                } else {
                    *fp++ = *cp;
                }
            }
            *fp++ = '\0';
            tabs(fd, indent);
            mprFprintf(fd, "CustomLog %s \"%s\"\n", host->getLogPath(), format);
        }
#endif

        /*
            ActionPrograms. One mimeTypes table is shared among all hosts.
         */
        if (host == defaultHost) {
            mimeTypes = host->getMimeTypes();
            mt = (MaMimeHashEntry*) mimeTypes->getFirst();
            first = 1;
            while (mt) {
                actionProgram = mt->getActionProgram();
                if (actionProgram && *actionProgram) {
                    if (first) {
                        mprFprintf(fd, "\n");
                        first = 0;
                    }
                    tabs(fd, indent);
                    mprFprintf(fd, "Action %s %s\n", mt->getMimeType(),
                        mt->getActionProgram());
                }
                mt = (MaMimeHashEntry*) mimeTypes->getNext(mt);
            }
        }

        /*
            Aliases
         */
        aliases = host->getAliases();
        alias = (MaAlias*) aliases->getFirst();
        first = 1;
        while (alias) {
            /*
                Must skip the catchall alias which has an empty prefix
             */
            if (alias->getPrefix()[0] != '\0' && !alias->isInherited()) {
                if (first) {
                    mprFprintf(fd, "\n");
                    first = 0;
                }
                tabs(fd, indent);
                code = alias->getRedirectCode();
                if (code != 0) {
                    mprFprintf(fd, "Redirect %d %s \"%s\"\n", code, alias->getPrefix(), alias->getName());
                } else {
                    mprFprintf(fd, "Alias %s \"%s\"\n", alias->getPrefix(), alias->getName());
                }
            }
            alias = (MaAlias*) aliases->getNext(alias);
        }

        /*
            Directories -- Do in reverse order
         */
        defaultDp = dp = (MaDir*) host->getDirs()->getLast();
        first = 1;
        while (dp) {
            if (dp->isInherited()) {
                dp = (MaDir*) host->getDirs()->getPrev(dp);
                continue;
            }
            path = dp->getPath();
            if (*path) {
                if (!first) {
                    mprFprintf(fd, "\n");
                    tabs(fd, indent++);
                    mprFprintf(fd, "<Directory %s>\n", dp->getPath());
                }
            }
            if (strcmp(dp->getIndex(), defaultDp->getIndex()) != 0) {
                tabs(fd, indent);
                mprFprintf(fd, "DirectoryIndex %s\n", dp->getIndex());
            }

            printAuth(fd, host, dp, indent);

            if (*path && !first) {
                tabs(fd, --indent);
                mprFprintf(fd, "</Directory>\n");
            }
            first = 0;
            dp = (MaDir*) host->getDirs()->getPrev(dp);
        }

        /*
            Locations
         */
        loc = (HttpLocation*) host->getLocations()->getLast();
        while (loc) {
            if (loc->isInherited()) {
                loc = (HttpLocation*) host->getLocations()->getPrev(loc);
                continue;
            }
            mprFprintf(fd, "\n");
            tabs(fd, indent++);
            mprFprintf(fd, "<Location %s>\n", loc->getPrefix());

            if (loc->getHandlerName()) {
                tabs(fd, indent);
                //  TODO - not supported
                mprFprintf(fd, "SetHandler %s\n", loc->getHandlerName());
            }

            printAuth(fd, host, loc, indent);

            tabs(fd, --indent);
            mprFprintf(fd, "</Location>\n");

            loc = (HttpLocation*) host->getLocations()->getPrev(loc);
        }

        /*
            Close out the VirtualHosts
         */
        if (host->isVhost()) {
            tabs(fd, --indent);
            mprFprintf(fd, "</VirtualHost>\n");
        }
        host = (MaHost*) hosts.getNext(host);
    }
    close(fd);
    return 0;
}


/*
    Print Authorization configuration
 */
static void printAuth(int fd, MaHost *host, HttpAuth *auth, int indent)
{
    HttpAuthType        authType;
    HttpAuthHandler     *handler;
    HttpAcl             acl;
    char                *users, *groups, *realm;

    if (auth->isAuthInherited()) {
        return;
    }

    handler = (HttpAuthHandler*) host->lookupHandler("auth");
    if (handler) {
        char    *path;
        path = handler->getGroupFile();
        if (path) {
            tabs(fd, indent);
            mprFprintf(fd, "AuthGroupFile %s\n", path);
        }
        path = handler->getUserFile();
        if (path) {
            tabs(fd, indent);
            mprFprintf(fd, "AuthUserFile %s\n", path);
        }
    }

    authType = auth->getType();
    if (authType == HTTP_AUTH_BASIC) {
        tabs(fd, indent);
        mprFprintf(fd, "AuthType basic\n");
    } else if (authType == HTTP_AUTH_DIGEST) {
        char *qop = auth->getQop();

        tabs(fd, indent);
        mprFprintf(fd, "AuthType digest\n");
        tabs(fd, indent);
        if (qop && *qop) {
            mprFprintf(fd, "AuthDigestQop %s\n", qop);
        }
    }

    realm = auth->getRealm();
    groups = auth->getRequiredGroups();
    users = auth->getRequiredUsers();
    acl = auth->getRequiredAcl();

    if (realm && *realm) {
        tabs(fd, indent);
        mprFprintf(fd, "AuthName \"%s\"\n", realm);
    }
    if (auth->getAnyValidUser()) {
        tabs(fd, indent);
        mprFprintf(fd, "Require valid-user\n");
    } else if (groups && *groups) {
        tabs(fd, indent);
        mprFprintf(fd, "Require group %s\n", groups);
    } else if (users && *users) {
        tabs(fd, indent);
        mprFprintf(fd, "Require user %s\n", users);
    } else if (acl) {
        tabs(fd, indent);
        mprFprintf(fd, "Require acl 0x%x\n", acl);
    }
}


static void tabs(int fd, int indent)
{
    for (int i = 0; i < indent; i++) {
        write(fd, "\t", 1);
    }
}

#endif /* BLD_FEATURE_CONFIG_SAVE */

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
/************************************************************************/
/*
 *  End of file "../src/config.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/connectors/sendConnector.c"
 */
/************************************************************************/

/*
    sendConnector.c -- Send file connector. 

    The Sendfile connector supports the optimized transmission of whole static files. It uses operating system 
    sendfile APIs to eliminate reading the document into user space and multiple socket writes. The send connector 
    is not a general purpose connector. It cannot handle dynamic data or ranged requests. It does support chunked requests.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void addPacketForSend(HttpQueue *q, HttpPacket *packet);
static void adjustSendVec(HttpQueue *q, int written);
static int  buildSendVec(HttpQueue *q);
static void freeSentPackets(HttpQueue *q, int written);

/*  
    Invoked to initialize the send connector for a request
 */
static void sendOpen(HttpQueue *q)
{
    HttpConn            *conn;
    HttpTransmitter     *trans;

    conn = q->conn;
    trans = conn->transmitter;

    /*  
        To write an entire file, reset the maximum and packet size to the maximum response body size (LimitResponseBody)
     */
    q->max = conn->limits->maxTransmissionBody;
    q->packetSize = conn->limits->maxTransmissionBody;

    if (!(trans->flags & HTTP_TRANS_NO_BODY)) {
        trans->file = mprOpen(q, trans->filename, O_RDONLY | O_BINARY, 0);
        if (trans->file == 0) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", trans->filename);
        }
    }
}


static void sendIncomingService(HttpQueue *q)
{
    httpEnableConnEvents(q->conn);
}


/*  
    Outgoing data service routine. May be called multiple times.
 */
static void sendOutgoingService(HttpQueue *q)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    int             written, ioCount, errCode;

    conn = q->conn;
    trans = conn->transmitter;

    if (trans->flags & HTTP_TRANS_NO_BODY) {
        httpDiscardData(q, 1);
    }
    /*
        Loop doing non-blocking I/O until blocked or all the packets received are written.
     */
    while (1) {
        /*
            Rebuild the iovector only when the past vector has been completely written. Simplifies the logic quite a bit.
         */
        written = 0;
        if (q->ioIndex == 0 && buildSendVec(q) <= 0) {
            break;
        }
        /*
            Write the vector and file data. Exclude the file entry in the io vector.
         */
        ioCount = q->ioIndex - q->ioFileEntry;
        mprAssert(ioCount >= 0);
        written = (int) mprSendFileToSocket(conn->sock, trans->file, trans->pos, q->ioCount, q->iovec, ioCount, NULL, 0);
        mprLog(q, 5, "Send connector written %d", written);
        if (written < 0) {
            errCode = mprGetError(q);
            if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
                break;
            }
            if (errCode != EPIPE && errCode != ECONNRESET) {
                mprLog(conn, 7, "mprSendFileToSocket failed, errCode %d", errCode);
            }
            conn->error = 1;
            /* This forces a read event so the socket can be closed */
            mprDisconnectSocket(conn->sock);
            freeSentPackets(q, MAXINT);
            break;

        } else if (written == 0) {
            /* Socket is full. Wait for an I/O event */
            httpWriteBlocked(conn);
            break;

        } else if (written > 0) {
            trans->bytesWritten += written;
            freeSentPackets(q, written);
            adjustSendVec(q, written);
        }
    }
    if (q->ioCount == 0 && q->flags & HTTP_QUEUE_EOF) {
        if (conn->server) {
            httpSetState(conn, HTTP_STATE_COMPLETE);
        } else {
            httpSetState(conn, HTTP_STATE_WAIT);
        }
    }
}


/*  
    Build the IO vector. This connector uses the send file API which permits multiple IO blocks to be written with 
    file data. This is used to write transfer the headers and chunk encoding boundaries. Return the count of bytes to 
    be written. Return -1 for EOF.
 */
static int buildSendVec(HttpQueue *q)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpPacket      *packet;

    conn = q->conn;
    trans = conn->transmitter;

    mprAssert(q->ioIndex == 0);
    q->ioCount = 0;
    q->ioFileEntry = 0;

    /*  
        Examine each packet and accumulate as many packets into the I/O vector as possible. Can only have one data packet at
        a time due to the limitations of the sendfile API (on Linux). And the data packet must be after all the 
        vector entries. Leave the packets on the queue for now, they are removed after the IO is complete for the 
        entire packet.
     */
    for (packet = q->first; packet && !q->ioFileEntry; packet = packet->next) {
        if (packet->flags & HTTP_PACKET_HEADER) {
            httpWriteHeaders(conn, packet);
            q->count += httpGetPacketLength(packet);

        } else if (httpGetPacketLength(packet) == 0) {
            q->flags |= HTTP_QUEUE_EOF;
            if (packet->prefix == NULL) {
                break;
            }
        }
        if (q->ioIndex >= (HTTP_MAX_IOVEC - 2)) {
            break;
        }
        addPacketForSend(q, packet);
    }
    return q->ioCount;
}


/*  
    Add one entry to the io vector
 */
static void addToSendVector(HttpQueue *q, char *ptr, int bytes)
{
    mprAssert(bytes > 0);

    q->iovec[q->ioIndex].start = ptr;
    q->iovec[q->ioIndex].len = bytes;
    q->ioCount += bytes;
    q->ioIndex++;
}


/*  
    Add a packet to the io vector. Return the number of bytes added to the vector.
 */
static void addPacketForSend(HttpQueue *q, HttpPacket *packet)
{
    HttpTransmitter *trans;
    HttpConn        *conn;
    MprIOVec        *iovec;

    conn = q->conn;
    trans = conn->transmitter;
    iovec = q->iovec;
    
    mprAssert(q->count >= 0);
    mprAssert(q->ioIndex < (HTTP_MAX_IOVEC - 2));

    if (packet->prefix) {
        addToSendVector(q, mprGetBufStart(packet->prefix), mprGetBufLength(packet->prefix));
    }
    if (httpGetPacketLength(packet) > 0) {
        /*
            Header packets have actual content. File data packets are virtual and only have a count.
         */
        if (packet->content) {
            addToSendVector(q, mprGetBufStart(packet->content), mprGetBufLength(packet->content));

        } else {
            addToSendVector(q, 0, httpGetPacketLength(packet));
            mprAssert(q->ioFileEntry == 0);
            q->ioFileEntry = 1;
            q->ioFileOffset += httpGetPacketLength(packet);
        }
    }
}


/*  
    Clear entries from the IO vector that have actually been transmitted. This supports partial writes due to the socket
    being full. Don't come here if we've seen all the packets and all the data has been completely written. ie. small files
    don't come here.
 */
static void freeSentPackets(HttpQueue *q, int bytes)
{
    HttpPacket  *packet;
    int         len;

    mprAssert(q->first);
    mprAssert(q->count >= 0);
    mprAssert(bytes >= 0);

    while ((packet = q->first) != 0) {
        if (packet->prefix) {
            len = mprGetBufLength(packet->prefix);
            len = min(len, bytes);
            mprAdjustBufStart(packet->prefix, len);
            bytes -= len;
            /* Prefixes dont' count in the q->count. No need to adjust */
            if (mprGetBufLength(packet->prefix) == 0) {
                packet->prefix = 0;
            }
        }
        if ((len = httpGetPacketLength(packet)) > 0) {
            len = min(len, bytes);
            if (packet->content) {
                mprAdjustBufStart(packet->content, len);
            } else {
                packet->entityLength -= len;
            }
            bytes -= len;
            q->count -= len;
            mprAssert(q->count >= 0);
        }
        if (httpGetPacketLength(packet) == 0) {
            if ((packet = httpGetPacket(q)) != 0) {
                httpFreePacket(q, packet);
            }
        }
        mprAssert(bytes >= 0);
        if (bytes == 0 && (q->first == NULL || !(q->first->flags & HTTP_PACKET_END))) {
            break;
        }
    }
}


/*  
    Clear entries from the IO vector that have actually been transmitted. This supports partial writes due to the socket
    being full. Don't come here if we've seen all the packets and all the data has been completely written. ie. small files
    don't come here.
 */
static void adjustSendVec(HttpQueue *q, int written)
{
    HttpTransmitter *trans;
    MprIOVec        *iovec;
    int             i, j, len;

    trans = q->conn->transmitter;

    /*  
        Cleanup the IO vector
     */
    if (written == q->ioCount) {
        /*  
            Entire vector written. Just reset.
         */
        q->ioIndex = 0;
        q->ioCount = 0;
        trans->pos = q->ioFileOffset;

    } else {
        /*  
            Partial write of an vector entry. Need to copy down the unwritten vector entries.
         */
        q->ioCount -= written;
        mprAssert(q->ioCount >= 0);
        iovec = q->iovec;
        for (i = 0; i < q->ioIndex; i++) {
            len = (int) iovec[i].len;
            if (iovec[i].start) {
                if (written < len) {
                    iovec[i].start += written;
                    iovec[i].len -= written;
                    break;
                } else {
                    written -= len;
                }
            } else {
                /*
                    File data has a null start ptr
                 */
                trans->pos += written;
                q->ioIndex = 0;
                q->ioCount = 0;
                return;
            }
        }

        /*  Compact */
        for (j = 0; i < q->ioIndex; ) {
            iovec[j++] = iovec[i++];
        }
        q->ioIndex = j;
    }
}


int maOpenSendConnector(Http *http)
{
    HttpStage     *stage;

    stage = httpCreateConnector(http, "sendConnector", HTTP_STAGE_ALL);
    if (stage == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->open = sendOpen;
    stage->outgoingService = sendOutgoingService; 
    stage->incomingService = sendIncomingService; 
    http->sendConnector = stage;
    return 0;
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
/************************************************************************/
/*
 *  End of file "../src/connectors/sendConnector.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/convenience.c"
 */
/************************************************************************/

/*
    convenience.c -- High level convenience API
    This module provides simple, high-level APIs for creating servers.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




/*  
    Create a web server described by a config file. 
 */
MaServer *maCreateWebServer(cchar *configFile)
{
    Mpr         *mpr;
    MaAppweb    *appweb;
    MaServer    *server;

    /*  
        Initialize and start the portable runtime services.
     */
    if ((mpr = mprCreate(0, NULL, NULL)) == 0) {
        mprError(mpr, "Can't create the web server runtime");
        return 0;
    }
    if (mprStart(mpr) < 0) {
        mprError(mpr, "Can't start the web server runtime");
        return 0;
    }
    if ((appweb = maCreateAppweb(mpr)) == 0) {
        mprError(mpr, "Can't create appweb object");
        return 0;
    }
    if ((server = maCreateServer(appweb, "default", ".", NULL, 0)) == 0) {
        mprError(mpr, "Can't create the web server");
        return 0;
    }
    if (maParseConfig(server, configFile) < 0) {
        mprError(mpr, "Can't parse the config file %s", configFile);
        return 0;
    }
    return server;
}


/*  
    Service requests for a web server.
 */
int maServiceWebServer(MaServer *server)
{
    if (maStartServer(server) < 0) {
        mprError(server, "Can't start the web server");
        return MPR_ERR_CANT_CREATE;
    }
    mprServiceEvents(mprGetDispatcher(server), -1, 0);
    maStopServer(server);
    return 0;
}


/*  
    Run a web server using a config file. 
 */
int maRunWebServer(cchar *configFile)
{
    MaServer  *server;

    if ((server = maCreateWebServer(configFile)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    return maServiceWebServer(server);
}


int maRunSimpleWebServer(cchar *ip, int port, cchar *docRoot)
{
    Mpr         *mpr;
    MaServer    *server;
    MaAppweb    *appweb;

    /*  
        Initialize and start the portable runtime services.
     */
    if ((mpr = mprCreate(0, NULL, NULL)) == 0) {
        mprError(mpr, "Can't create the web server runtime");
        return MPR_ERR_CANT_CREATE;
    }
    if (mprStart(mpr) < 0) {
        mprError(mpr, "Can't start the web server runtime");
        return MPR_ERR_CANT_INITIALIZE;
    }
    //  TODO MOB - do we need a meta server when running via an API
    if ((appweb = maCreateAppweb(mpr)) == 0) {
        mprError(mpr, "Can't create the web server http services");
        return MPR_ERR_CANT_INITIALIZE;
    }

    /*  
        Create and start the HTTP server. Give the server a name of "default" and define "." as the default serverRoot, 
        ie. the directory with the server configuration files.
     */
    server = maCreateServer(appweb, ip, ".", ip, port);
    if (server == 0) {
        mprError(mpr, "Can't create the web server");
        return MPR_ERR_CANT_CREATE;
    }
    maSetDocumentRoot(server->defaultHost, docRoot);
    
    if (maStartServer(server) < 0) {
        mprError(mpr, "Can't start the web server");
        return MPR_ERR_CANT_CREATE;
    }
    mprServiceEvents(mprGetDispatcher(mpr), -1, 0);
    maStopServer(server);
    mprFree(mpr);
    return 0;
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
/************************************************************************/
/*
 *  End of file "../src/convenience.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/dir.c"
 */
/************************************************************************/

/*
    dir.c -- Support authorization on a per-directory basis.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




MaDir *maCreateBareDir(MaHost *host, cchar *path)
{
    MaDir   *dir;

    mprAssert(host);
    mprAssert(path);

    dir = mprAllocObjZeroed(host, MaDir);
    if (dir == 0) {
        return 0;
    }
    dir->indexName = mprStrdup(dir, "index.html");
    dir->host = host;
    dir->auth = httpCreateAuth(dir, 0);
    if (path) {
        dir->path = mprStrdup(dir, path);
        dir->pathLen = strlen(path);
    }
    return dir;
}


MaDir *maCreateDir(MaHost *host, cchar *path, MaDir *parent)
{
    MaDir   *dir;

    mprAssert(host);
    mprAssert(path);
    mprAssert(parent);

    dir = mprAllocObjZeroed(host, MaDir);
    if (dir == 0) {
        return 0;
    }
    dir->host = host;
    dir->indexName = mprStrdup(dir, parent->indexName);

    if (path == 0) {
        path = parent->path;
    }
    maSetDirPath(dir, path);
    dir->auth = httpCreateAuth(dir, parent->auth);
    return dir;
}


void maSetDirPath(MaDir *dir, cchar *fileName)
{
    mprAssert(dir);
    mprAssert(fileName);

    mprFree(dir->path);
    dir->path = mprGetAbsPath(dir, fileName);
    dir->pathLen = (int) strlen(dir->path);

    /*  
        Always strip trailing "/"
     */
    if (dir->pathLen > 0 && dir->path[dir->pathLen - 1] == '/') {
        dir->path[--dir->pathLen] = '\0';
    }
}


void maSetDirIndex(MaDir *dir, cchar *name) 
{ 
    mprAssert(dir);
    mprAssert(name && *name);

    mprFree(dir->indexName);
    dir->indexName = mprStrdup(dir, name); 
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
/************************************************************************/
/*
 *  End of file "../src/dir.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/handlers/cgiHandler.c"
 */
/************************************************************************/

/* 
    cgiHandler.c -- Common Gateway Interface Handler

    Support the CGI/1.1 standard for external gateway programs to respond to HTTP requests.
    This CGI handler uses async-pipes and non-blocking I/O for all communications.
 
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



#if BLD_FEATURE_CGI

#define MA_CGI_SEEN_HEADER          0x1
#define MA_CGI_FLOW_CONTROL         0x2     /* Output to client is flow controlled */


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

/*
    Open this handler instance for a new request
 */
static void openCgi(HttpQueue *q)
{
    HttpReceiver    *rec;
    HttpConn        *conn;

    conn = q->conn;
    rec = conn->receiver;

    /*  
        If there is body content, it may arrive on another thread. Uclibc can't wait accross thread groups,
        so we must ensure that further callback events come on the same thread that creates the CGI process.
     */
#if __UCLIBC__
    if (rec->remainingContent > 0) {
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
    HttpReceiver        *rec;
    HttpTransmitter     *trans;
    HttpConn            *conn;
    MprCmd              *cmd;
    MprHashTable        *vars;
    cchar               *baseName;
    char                **argv, **envv, *fileName;
    int                 argc, varCount, count;

    argv = 0;
    vars = 0;
    argc = 0;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    cmd = q->queueData = mprCreateCmd(rec, conn->dispatcher);

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
    vars = rec->formVars;
    varCount = mprGetHashCount(vars) + mprGetHashCount(rec->headers);
    if ((envv = (char**) mprAlloc(cmd, (varCount + 1) * sizeof(char*))) != 0) {
        count = copyVars(cmd, envv, 0, rec->formVars, "");
        count = copyVars(cmd, envv, count, rec->headers, "HTTP_");
        mprAssert(count <= varCount);
    }
    cmd->stdoutBuf = mprCreateBuf(cmd, HTTP_BUFSIZE, -1);
    cmd->stderrBuf = mprCreateBuf(cmd, HTTP_BUFSIZE, -1);

    mprSetCmdDir(cmd, mprGetPathDir(q, fileName));
    mprSetCmdCallback(cmd, cgiCallback, conn);

    if (mprStartCmd(cmd, argc, argv, envv, MPR_CMD_IN | MPR_CMD_OUT | MPR_CMD_ERR) < 0) {
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't run CGI process: %s, URI %s", fileName, rec->uri);
    }
}


#if __UCLIBC__
/*  
    WARNING: Block and wait for the CGI program to complete. Required for UCLIBC because waitpid won't work across threads.
    Wait for the CGI process to complete and collect status. Must run on the same thread as that initiated the CGI process.
 */
static void waitForCgiCompletion(HttpQueue *q)
{
    HttpTransmitter *trans;
    HttpConn        *conn;
    MprCmd          *cmd;
    int             rc;

    conn = q->conn;
    trans = conn->transmitter;
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
            trans->filename, cmd->status);
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
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;
    MprCmd          *cmd;

    mprAssert(q);
    mprAssert(packet);
    
    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;

    cmd = (MprCmd*) q->pair->queueData;
    mprAssert(cmd);
    cmd->lastActivity = mprGetTime(cmd);

    if (httpGetPacketLength(packet) == 0) {
        /* End of input */
        if (rec->remainingContent > 0) {
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
    HttpQueue       *q;
    HttpConn        *conn;
    HttpTransmitter *trans;

    conn = (HttpConn*) data;
    mprAssert(conn);
    mprAssert(conn->transmitter);

    mprLog(cmd, 6, "CGI callback channel %d", channel);
    
    trans = conn->transmitter;
    cmd->lastActivity = mprGetTime(cmd);
    q = conn->transmitter->queue[HTTP_QUEUE_TRANS].nextQ;

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
    HttpConn        *conn;
    HttpTransmitter *trans;
    int             space, nbytes, err;

    conn = q->conn;
    trans = conn->transmitter;
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
    HttpTransmitter *trans;
    HttpQueue       *q;
    MprBuf          *buf;
    char            *endHeaders, *headers, *key, *value, *location;
    int             len;

    trans = conn->transmitter;
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
        httpRedirect(conn, trans->status, location);
        q = trans->queue[HTTP_QUEUE_TRANS].nextQ;
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
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MaHost          *host;
    char            *fileName, **argv, *program, *cmdScript, status[8], *indexQuery, *cp, *tok;
    cchar           *actionProgram;
    int             argc, argind, len;

    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    fileName = trans->filename;
    mprAssert(fileName);

    program = cmdScript = 0;
    actionProgram = 0;
    argind = 0;
    argc = *argcp;

    if (rec->mimeType) {
        actionProgram = maGetMimeActionProgram(host, rec->mimeType);
        if (actionProgram != 0) {
            argc++;
            /*
                This is an Apache compatible hack
             */
            mprItoa(status, sizeof(status), HTTP_CODE_MOVED_TEMPORARILY, 10);
            mprAddHash(rec->headers, "REDIRECT_STATUS", status);
        }
    }

    /*
        Count the args for ISINDEX queries. Only valid if there is not a "=" in the query. 
        If this is so, then we must not have these args in the query env also?
     */
    indexQuery = rec->parsedUri->query;
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
        mprSprintf(cmdBuf, len, "\"%s\"", cmdScript);
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
            argv[argind++] = mprEscapeCmd(trans, mprUriDecode(trans, cp), 0);
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
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation      *location;
    MprHash         *hp;
    MprFile         *file;
    cchar           *actionProgram, *ext, *cmdShell;
    char            *tok, buf[MPR_MAX_FNAME + 1], *path;

    rec = conn->receiver;
    trans = conn->transmitter;
    location = rec->location;

    *bangScript = 0;
    *script = 0;
    *program = 0;
    path = 0;

    actionProgram = maGetMimeActionProgram(conn->host, rec->mimeType);
    ext = trans->extension;

    /*
        If not found, go looking for the fileName with the extensions defined in appweb.conf. 
        NOTE: we don't use PATH deliberately!!!
     */
    if (access(fileName, X_OK) < 0 && *ext == '\0') {
        for (hp = 0; (hp = mprGetNextHash(location->extensions, hp)) != 0; ) {
            path = mprStrcat(trans, -1, fileName, ".", hp->key, NULL);
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
        *script = mprStrdup(trans, path);
        *program = mprStrdup(trans, cmdShell);
        return;
    }
#endif

    if ((file = mprOpen(trans, path, O_RDONLY, 0)) != 0) {
        if (mprRead(file, buf, MPR_MAX_FNAME) > 0) {
            mprFree(file);
            buf[MPR_MAX_FNAME] = '\0';
            if (buf[0] == '#' && buf[1] == '!') {
                cmdShell = mprStrTok(&buf[2], " \t\r\n", &tok);
                if (mprIsAbsPath(trans, cmdShell)) {
                    /*
                        If we can't access the command shell and the command is not an absolute path, 
                        look in the same directory as the script.
                     */
                    if (mprPathExists(trans, cmdShell, X_OK)) {
                        cmdShell = mprJoinPath(trans, mprGetPathDir(trans, path), cmdShell);
                    }
                }
                if (actionProgram) {
                    *program = mprStrdup(trans, actionProgram);
                } else {
                    *program = mprStrdup(trans, cmdShell);
                }
                *bangScript = mprStrdup(trans, path);
                return;
            }
        } else {
            mprFree(file);
        }
    }

    if (actionProgram) {
        *program = mprStrdup(trans, actionProgram);
        *bangScript = mprStrdup(trans, path);
    } else {
        *program = mprStrdup(trans, path);
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
    HttpLocation    *location;
    MaServer        *server;
    MaHost          *host;
    MaAlias         *alias;
    MaDir           *dir, *parent;
    char            *program, *mimeType, *prefix, *path;

    host = state->host;
    server = state->server;
    location = state->location;

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

        if ((location = maLookupLocation(host, prefix)) == 0) {
            location = httpCreateInheritedLocation(server->http, state->location);
            httpSetLocationAuth(location, state->dir->auth);
            httpSetLocationPrefix(location, prefix);
            maAddLocation(host, location);
        } else {
            httpSetLocationPrefix(location, prefix);
        }
        httpSetHandler(location, "cgiHandler");
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
/************************************************************************/
/*
 *  End of file "../src/handlers/cgiHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/handlers/dirHandler.c"
 */
/************************************************************************/

/*
    dirHandler.c - Directory listing handler.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/*
    Handler configuration
 */
typedef struct Dir {
    cchar           *defaultIcon;
    MprList         *dirList;
    bool            enabled;
    MprList         *extList;
    int             fancyIndexing;
    bool            foldersFirst;
    MprList         *ignoreList;
    cchar           *pattern;
    char            *sortField;
    int             sortOrder;              /* 1 == ascending, -1 descending */
} Dir;


static void filterDirList(HttpConn *conn, MprList *list);
static int  match(cchar *pattern, cchar *file);
static void outputFooter(HttpQueue *q);
static void outputHeader(HttpQueue *q, cchar *dir, int nameSize);
static void outputLine(HttpQueue *q, MprDirEntry *ep, cchar *dir, int nameSize);
static void parseQuery(HttpConn *conn);
static void parseWords(MprList *list, cchar *str);
static void sortList(HttpConn *conn, MprList *list);

/*
    Match if the url maps to a directory.
 */
static bool matchDir(HttpConn *conn, HttpStage *handler)
{
    HttpTransmitter *trans;
    MprPath         *info;
    Dir             *dir;

    trans = conn->transmitter;
    info = &trans->fileInfo;
    dir = handler->stageData;
    
    if (!info->valid && mprGetPathInfo(conn, trans->filename, info) < 0) {
        return 0;
    }
    return dir->enabled && info->isDir;
}


static void processDir(HttpQueue *q)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;
    MprList         *list;
    MprDirEntry     *dp;
    Dir             *dir;
    cchar           *filename;
    uint            nameSize;
    int             next;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    dir = q->stage->stageData;

    filename = trans->filename;
    mprAssert(filename);

    httpDontCache(conn);
    httpSetHeader(conn, "Last-Modified", conn->http->currentDate);
    parseQuery(conn);

    list = mprGetPathFiles(conn, filename, 1);
    if (list == 0) {
        httpWrite(q, "<h2>Can't get file list</h2>\r\n");
        outputFooter(q);
        return;
    }
    if (dir->pattern) {
        filterDirList(conn, list);
    }
    sortList(conn, list);

    /*
        Get max filename
     */
    nameSize = 0;
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        nameSize = max((int) strlen(dp->name), nameSize);
    }
    nameSize = max(nameSize, 22);

    outputHeader(q, rec->pathInfo, nameSize);
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        outputLine(q, dp, filename, nameSize);
    }
    outputFooter(q);
    httpFinalize(conn);
    mprFree(list);
}
 

static void parseQuery(HttpConn *conn)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    Dir             *dir;
    char            *value, *query, *next, *tok;

    rec = conn->receiver;
    trans = conn->transmitter;
    dir = trans->handler->stageData;
    
    query = mprStrdup(rec, rec->parsedUri->query);
    if (query == 0) {
        return;
    }

    tok = mprStrTok(query, ";&", &next);
    while (tok) {
        if ((value = strchr(tok, '=')) != 0) {
            *value++ = '\0';
            if (*tok == 'C') {                  /* Sort column */
                mprFree(dir->sortField);
                if (*value == 'N') {
                    dir->sortField = "Name";
                } else if (*value == 'M') {
                    dir->sortField = "Date";
                } else if (*value == 'S') {
                    dir->sortField = "Size";
                }
                dir->sortField = mprStrdup(dir, dir->sortField);

            } else if (*tok == 'O') {           /* Sort order */
                if (*value == 'A') {
                    dir->sortOrder = 1;
                } else if (*value == 'D') {
                    dir->sortOrder = -1;
                }

            } else if (*tok == 'F') {           /* Format */ 
                if (*value == '0') {
                    dir->fancyIndexing = 0;
                } else if (*value == '1') {
                    dir->fancyIndexing = 1;
                } else if (*value == '2') {
                    dir->fancyIndexing = 2;
                }

            } else if (*tok == 'P') {           /* Pattern */ 
                dir->pattern = mprStrdup(dir, value);
            }
        }
        tok = mprStrTok(next, ";&", &next);
    }
    
    mprFree(query);
}


static void sortList(HttpConn *conn, MprList *list)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprDirEntry     *tmp, **items;
    Dir             *dir;
    int             count, i, j, rc;

    rec = conn->receiver;
    trans = conn->transmitter;
    dir = trans->handler->stageData;
    
    if (dir->sortField == 0) {
        return;
    }

    count = mprGetListCount(list);
    items = (MprDirEntry**) list->items;
    if (mprStrcmpAnyCase(dir->sortField, "Name") == 0) {
        for (i = 1; i < count; i++) {
            for (j = 0; j < i; j++) {
                rc = strcmp(items[i]->name, items[j]->name);
                if (dir->foldersFirst) {
                    if (items[i]->isDir && !items[j]->isDir) {
                        rc = -dir->sortOrder;
                    } else if (items[j]->isDir && !items[i]->isDir) {
                        rc = dir->sortOrder;
                    } 
                }
                rc *= dir->sortOrder;
                if (rc < 0) {
                    tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }

    } else if (mprStrcmpAnyCase(dir->sortField, "Size") == 0) {
        for (i = 1; i < count; i++) {
            for (j = 0; j < i; j++) {
                rc = (items[i]->size < items[j]->size) ? -1 : 1;
                if (dir->foldersFirst) {
                    if (items[i]->isDir && !items[j]->isDir) {
                        rc = -dir->sortOrder;
                    } else if (items[j]->isDir && !items[i]->isDir) {
                        rc = dir->sortOrder;
                    }
                }
                rc *= dir->sortOrder;
                if (rc < 0) {
                    tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }

    } else if (mprStrcmpAnyCase(dir->sortField, "Date") == 0) {
        for (i = 1; i < count; i++) {
            for (j = 0; j < i; j++) {
                rc = (items[i]->lastModified < items[j]->lastModified) ? -1: 1;
                if (dir->foldersFirst) {
                    if (items[i]->isDir && !items[j]->isDir) {
                        rc = -dir->sortOrder;
                    } else if (items[j]->isDir && !items[i]->isDir) {
                        rc = dir->sortOrder;
                    }
                }
                rc *= dir->sortOrder;
                if (rc < 0) {
                    tmp = items[i];
                    items[i] = items[j];
                    items[j] = tmp;
                }
            }
        }
    }
}


static void outputHeader(HttpQueue *q, cchar *path, int nameSize)
{
    Dir     *dir;
    char    *parent, *parentSuffix;
    int     order, reverseOrder, fancy, isRootDir;

    dir = q->stage->stageData;
    
    fancy = 1;

    httpWrite(q, "<!DOCTYPE HTML PUBLIC \"-/*W3C//DTD HTML 3.2 Final//EN\">\r\n");
    httpWrite(q, "<html>\r\n <head>\r\n  <title>Index of %s</title>\r\n", path);
    httpWrite(q, " </head>\r\n");
    httpWrite(q, "<body>\r\n");

    httpWrite(q, "<h1>Index of %s</h1>\r\n", path);

    if (dir->sortOrder > 0) {
        order = 'A';
        reverseOrder = 'D';
    } else {
        order = 'D';
        reverseOrder = 'A';
    }

    if (dir->fancyIndexing == 0) {
        fancy = '0';
    } else if (dir->fancyIndexing == 1) {
        fancy = '1';
    } else if (dir->fancyIndexing == 2) {
        fancy = '2';
    }

    parent = mprGetPathDir(q, path);
    if (parent[strlen(parent) - 1] != '/') {
        parentSuffix = "/";
    } else {
        parentSuffix = "";
    }

    isRootDir = (strcmp(path, "/") == 0);

    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<table><tr><th><img src=\"/icons/blank.gif\" alt=\"[ICO]\" /></th>");

        httpWrite(q, "<th><a href=\"?C=N;O=%c;F=%c\">Name</a></th>", reverseOrder, fancy);
        httpWrite(q, "<th><a href=\"?C=M;O=%c;F=%c\">Last modified</a></th>", reverseOrder, fancy);
        httpWrite(q, "<th><a href=\"?C=S;O=%c;F=%c\">Size</a></th>", reverseOrder, fancy);
        httpWrite(q, "<th><a href=\"?C=D;O=%c;F=%c\">Description</a></th>\r\n", reverseOrder, fancy);

        httpWrite(q, "</tr><tr><th colspan=\"5\"><hr /></th></tr>\r\n");

        if (! isRootDir) {
            httpWrite(q, "<tr><td valign=\"top\"><img src=\"/icons/back.gif\"");
            httpWrite(q, "alt=\"[DIR]\" /></td><td><a href=\"%s%s\">", parent, parentSuffix);
            httpWrite(q, "Parent Directory</a></td>");
            httpWrite(q, "<td align=\"right\">  - </td></tr>\r\n");
        }

    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<pre><img src=\"/icons/space.gif\" alt=\"Icon\" /> ");

        httpWrite(q, "<a href=\"?C=N;O=%c;F=%c\">Name</a>%*s", reverseOrder, fancy, nameSize - 3, " ");
        httpWrite(q, "<a href=\"?C=M;O=%c;F=%c\">Last modified</a>       ", reverseOrder, fancy);
        httpWrite(q, "<a href=\"?C=S;O=%c;F=%c\">Size</a>               ", reverseOrder, fancy);
        httpWrite(q, "<a href=\"?C=D;O=%c;F=%c\">Description</a>\r\n", reverseOrder, fancy);

        httpWrite(q, "<hr />");

        if (! isRootDir) {
            httpWrite(q, "<img src=\"/icons/parent.gif\" alt=\"[DIR]\" />");
            httpWrite(q, " <a href=\"%s%s\">Parent Directory</a>\r\n", parent, parentSuffix);
        }

    } else {
        httpWrite(q, "<ul>\n");
        if (! isRootDir) {
            httpWrite(q, "<li><a href=\"%s%s\"> Parent Directory</a></li>\r\n", parent, parentSuffix);
        }
    }
    mprFree(parent);
}


static void fmtNum(char *buf, int bufsize, int num, int divisor, char *suffix)
{
    int     whole, point;

    whole = num / divisor;
    point = (num % divisor) / (divisor / 10);

    if (point == 0) {
        mprSprintf(buf, bufsize, "%6d%s", whole, suffix);
    } else {
        mprSprintf(buf, bufsize, "%4d.%d%s", whole, point, suffix);
    }
}


static void outputLine(HttpQueue *q, MprDirEntry *ep, cchar *path, int nameSize)
{
    MprPath     info;
    Dir         *dir;
    MprTime     when;
    MaHost      *host;
    char        *newPath, sizeBuf[16], timeBuf[48], *icon;
    struct tm   tm;
    bool        isDir;
    int         len;
    cchar       *ext, *mimeType;
    char        *dirSuffix;
    char        *months[] = { 
                    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
                };

    dir = q->stage->stageData;
    if (ep->size >= (1024*1024*1024)) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024 * 1024 * 1024, "G");

    } else if (ep->size >= (1024*1024)) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024 * 1024, "M");

    } else if (ep->size >= 1024) {
        fmtNum(sizeBuf, sizeof(sizeBuf), (int) ep->size, 1024, "K");

    } else {
        mprSprintf(sizeBuf, sizeof(sizeBuf), "%6d", (int) ep->size);
    }
    newPath = mprJoinPath(q, path, ep->name);

    if (mprGetPathInfo(q, newPath, &info) < 0) {
        when = mprGetTime(q);
        isDir = 0;

    } else {
        isDir = info.isDir ? 1 : 0;
        when = (MprTime) info.mtime * MPR_TICKS_PER_SEC;
    }
    mprFree(newPath);

    if (isDir) {
        icon = "folder";
        dirSuffix = "/";
    } else {
        host = httpGetConnHost(q->conn);
        ext = mprGetPathExtension(q, ep->name);
        if ((mimeType = maLookupMimeType(host, ext)) != 0) {
            if (strcmp(ext, "es") == 0 || strcmp(ext, "ejs") == 0 || strcmp(ext, "php") == 0) {
                icon = "text";
            } else if (strstr(mimeType, "text") != 0) {
                icon = "text";
            } else {
                icon = "compressed";
            }
        } else {
            icon = "compressed";
        }
        dirSuffix = "";
    }
    mprDecodeLocalTime(q, &tm, when);

    mprSprintf(timeBuf, sizeof(timeBuf), "%02d-%3s-%4d %02d:%02d",
        tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900, tm.tm_hour,  tm.tm_min);
    len = (int) strlen(ep->name) + (int) strlen(dirSuffix);

    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<tr><td valign=\"top\">");
        httpWrite(q, "<img src=\"/icons/%s.gif\" alt=\"[   ]\", /></td>", icon);
        httpWrite(q, "<td><a href=\"%s%s\">%s%s</a></td>", ep->name, dirSuffix, ep->name, dirSuffix);
        httpWrite(q, "<td>%s</td><td>%s</td></tr>\r\n", timeBuf, sizeBuf);

    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<img src=\"/icons/%s.gif\" alt=\"[   ]\", /> ", icon);
        httpWrite(q, "<a href=\"%s%s\">%s%s</a>%-*s %17s %4s\r\n", ep->name, dirSuffix, ep->name, dirSuffix, 
            nameSize - len, "", timeBuf, sizeBuf);

    } else {
        httpWrite(q, "<li><a href=\"%s%s\"> %s%s</a></li>\r\n", ep->name, dirSuffix, ep->name, dirSuffix);
    }
}


static void outputFooter(HttpQueue *q)
{
    HttpReceiver   *rec;
    HttpConn      *conn;
    MprSocket   *sock;
    Dir         *dir;
    
    conn = q->conn;
    rec = conn->receiver;
    dir = q->stage->stageData;
    
    if (dir->fancyIndexing == 2) {
        httpWrite(q, "<tr><th colspan=\"5\"><hr /></th></tr>\r\n</table>\r\n");
        
    } else if (dir->fancyIndexing == 1) {
        httpWrite(q, "<hr /></pre>\r\n");
    } else {
        httpWrite(q, "</ul>\r\n");
    }
    sock = conn->sock->listenSock;
    httpWrite(q, "<address>%s %s at %s Port %d</address>\r\n", BLD_NAME, BLD_VERSION, sock->ip, sock->port);
    httpWrite(q, "</body></html>\r\n");
}


static void filterDirList(HttpConn *conn, MprList *list)
{
    Dir             *dir;
    MprDirEntry     *dp;
    int             next;

    dir = conn->transmitter->handler->stageData;
    
    /*
        Do pattern matching. Entries that don't match, free the name to mark
     */
    for (next = 0; (dp = mprGetNextItem(list, &next)) != 0; ) {
        if (! match(dir->pattern, dp->name)) {
            mprRemoveItem(list, dp);
            mprFree(dp);
            next--;
        }
    }
}


/*
    Return true if the file matches the pattern. Supports '?' and '*'
 */
static int match(cchar *pattern, cchar *file)
{
    cchar   *pp, *fp;

    if (pattern == 0 || *pattern == '\0') {
        return 1;
    }
    if (file == 0 || *file == '\0') {
        return 0;
    }
    for (pp = pattern, fp = file; *pp; ) {
        if (*fp == '\0') {
            if (*pp == '*' && pp[1] == '\0') {
                /* Trailing wild card */
                return 1;
            }
            return 0;
        }

        if (*pp == '*') {
            if (match(&pp[1], &fp[0])) {
                return 1;
            }
            fp++;
            continue;

        } else if (*pp == '?' || *pp == *fp) {
            fp++;

        } else {
            return 0;
        }
        pp++;
    }
    if (*fp == '\0') {
        /* Match */
        return 1;
    }
    return 0;
}


static int parseDir(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpStage   *handler;
    Dir         *dir;
    char        *name, *extensions, *option, *nextTok, *junk;

    handler = httpLookupStage(http, "dirHandler");
    dir = handler->stageData;
    mprAssert(dir);
    
    if (mprStrcmpAnyCase(key, "AddIcon") == 0) {
        /*  AddIcon file ext ext ext */
        /*  Not yet supported */
        name = mprStrTok(value, " \t", &extensions);
        parseWords(dir->extList, extensions);
        return 1;

    } else if (mprStrcmpAnyCase(key, "DefaultIcon") == 0) {
        /*  DefaultIcon file */
        /*  Not yet supported */
        dir->defaultIcon = mprStrTok(value, " \t", &junk);
        return 1;

    } else if (mprStrcmpAnyCase(key, "IndexOrder") == 0) {
        /*  IndexOrder ascending|descending name|date|size */
        mprFree(dir->sortField);
        dir->sortField = 0;
        option = mprStrTok(value, " \t", &dir->sortField);
        if (mprStrcmpAnyCase(option, "ascending") == 0) {
            dir->sortOrder = 1;
        } else {
            dir->sortOrder = -1;
        }
        if (dir->sortField) {
            dir->sortField = mprStrdup(dir, dir->sortField);
        }
        return 1;

    } else if (mprStrcmpAnyCase(key, "IndexIgnore") == 0) {
        /*  IndexIgnore pat ... */
        /*  Not yet supported */
        parseWords(dir->ignoreList, value);
        return 1;

    } else if (mprStrcmpAnyCase(key, "IndexOptions") == 0) {
        /*  IndexOptions FancyIndexing|FoldersFirst ... (set of options) */
        option = mprStrTok(value, " \t", &nextTok);
        while (option) {
            if (mprStrcmpAnyCase(option, "FancyIndexing") == 0) {
                dir->fancyIndexing = 1;
            } else if (mprStrcmpAnyCase(option, "HTMLTable") == 0) {
                dir->fancyIndexing = 2;
            } else if (mprStrcmpAnyCase(option, "FoldersFirst") == 0) {
                dir->foldersFirst = 1;
            }
            option = mprStrTok(nextTok, " \t", &nextTok);
        }
        return 1;

    } else if (mprStrcmpAnyCase(key, "Options") == 0) {
        /*  Options Indexes */
        option = mprStrTok(value, " \t", &nextTok);
        while (option) {
            if (mprStrcmpAnyCase(option, "Indexes") == 0) {
                dir->enabled = 1;
            }
            option = mprStrTok(nextTok, " \t", &nextTok);
        }
        return 1;
    }
    return 0;
}


static void parseWords(MprList *list, cchar *str)
{
    char    *word, *tok, *strTok;

    mprAssert(str);
    if (str == 0 || *str == '\0') {
        return;
    }

    strTok = mprStrdup(list, str);
    word = mprStrTok(strTok, " \t\r\n", &tok);
    while (word) {
        mprAddItem(list, word);
        word = mprStrTok(0, " \t\r\n", &tok);
    }
}


/*
    Dynamic module initialization
 */
int maOpenDirHandler(Http *http)
{
    HttpStage     *handler;
    Dir         *dir;

    handler = httpCreateHandler(http, "dirHandler", HTTP_STAGE_GET | HTTP_STAGE_HEAD);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->match = matchDir; 
    handler->process = processDir; 
    handler->parse = (HttpParse) parseDir; 
    handler->stageData = dir = mprAllocObjZeroed(handler, Dir);
    http->dirHandler = handler;
    dir->sortOrder = 1;
    return 0;
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
/************************************************************************/
/*
 *  End of file "../src/handlers/dirHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/handlers/egiHandler.c"
 */
/************************************************************************/

/*
    egiHandler.c -- Embedded Gateway Interface (EGI) handler. Fast in-process replacement for CGI.

    The EGI handler implements a very fast in-process CGI scheme.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




typedef struct MaEgi {
    MprHashTable        *forms;
} MaEgi;


static bool matchEgi(HttpConn *conn, HttpStage *handler)
{
    HttpReceiver    *rec;

    /*  
        Rewrite the entire URL as the script name 
     */
    rec = conn->receiver;
    rec->scriptName = rec->pathInfo;
    rec->pathInfo = "";
    return 1;
}


/*
    The process method will run after receiving upload or form data. Otherwise it runs before receiving all the data.
 */
static void runEgi(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    MaEgiForm       *form;
    MaEgi           *egi;

    conn = q->conn;
    rec = conn->receiver;
    egi = (MaEgi*) q->stage->stageData;
    
    form = (MaEgiForm*) mprLookupHash(egi->forms, rec->scriptName);
    if (form == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Egi Form: \"%s\" is not defined", rec->scriptName);
    } else {
        (*form)(q);
    }
}


static void startEgi(HttpQueue *q)
{
    if (!q->conn->receiver->form) {
        runEgi(q);
    }
}


static void processEgi(HttpQueue *q)
{
    if (q->conn->receiver->form) {
        runEgi(q);
    }
}


/*
    Define an EGI form
 */
int maDefineEgiForm(Http *http, cchar *name, MaEgiForm *form)
{
    HttpStage   *handler;
    MaEgi       *egi;

    handler = httpLookupStage(http, "egiHandler");
    if (handler) {
        egi = (MaEgi*) handler->stageData;
        mprAddHash(egi->forms, name, form);
    }
    return 0;
}


int maOpenEgiHandler(Http *http)
{
    HttpStage       *handler;
    MaEgi           *egi;

    handler = httpCreateHandler(http, "egiHandler", 
        HTTP_STAGE_GET | HTTP_STAGE_HEAD | HTTP_STAGE_POST | HTTP_STAGE_PUT | HTTP_STAGE_VARS | HTTP_STAGE_ENV_VARS | 
        HTTP_STAGE_VIRTUAL | HTTP_STAGE_THREAD);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->match = matchEgi; 
    handler->start = startEgi; 
    handler->process = processEgi; 
    handler->stageData = egi = mprAllocObjZeroed(handler, MaEgi);
    egi->forms = mprCreateHash(egi, HTTP_LARGE_HASH_SIZE);
    return 0;
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
/************************************************************************/
/*
 *  End of file "../src/handlers/egiHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/handlers/ejsHandler.c"
 */
/************************************************************************/

/*
    ejsHandler.c -- Ejscript language request handler for the Ejscript Web Framework.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#include    "ejs.h"

#if BLD_FEATURE_EJS

static EjsService   *ejsService;


static int startEjsApp(Http *http, HttpLocation *location);

/*  
    Confirm the request match to the ejs handler.
 */
static bool matchEjs(HttpConn *conn, HttpStage *handler)
{
    HttpReceiver    *rec;
    MaAlias         *alias;
    char            *uri;

    rec = conn->receiver;
    alias = rec->alias;
    uri = rec->pathInfo;

    /*
        Set the scriptName to the alias prefix and remove from pathInfo
     */
    if (alias->prefixLen > 0) {
        uri = &uri[alias->prefixLen];
        if (*uri != '/' && uri[-1] == '/') {
            uri--;
        }
        rec->scriptName = alias->prefix;
        rec->pathInfo = (char*) uri;
    }
    return 1;
}


static void openEjs(HttpQueue *q)
{
    HttpConn        *conn;
    HttpLocation    *location;
    
    conn = q->conn;
    location = conn->receiver->location;
    if (location == 0 || location->context == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Undefined location context. Missing HttpServer.attach call.");
        return;
    }
}


static int parseEjs(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLocation    *location, *newloc;
    MaServer        *server;
    MaHost          *host;
    char            *prefix, *path;
    
    host = state->host;
    server = state->server;
    location = state->location;
    
    if (mprStrcmpAnyCase(key, "EjsLocation") == 0) {
        if (maSplitConfigValue(http, &prefix, &path, value, 1) < 0 || path == 0 || prefix == 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        prefix = mprStrTrim(value, "\"");
        path = mprStrTrim(value, "\"");
        location->script = mprStrdup(location, path);
        newloc = httpCreateInheritedLocation(http, location);
        if (newloc == 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        httpSetLocationPrefix(newloc, prefix);
        newloc->autoDelete = 1;
        location->script = mprStrdup(location, path);
        startEjsApp(http, location);
        return 1;

    } else if (mprStrcmpAnyCase(key, "EjsStartup") == 0) {
        path = mprStrTrim(value, "\"");
        location->script = mprStrdup(location, path);
        startEjsApp(http, location);
        return 1;
    }
    return 0;
}


static int startEjsApp(Http *http, HttpLocation *location)
{
    Ejs         *ejs;
    int         ver;

    ejs = ejsCreateVm(ejsService, NULL, NULL, NULL, EJS_FLAG_MASTER);
    if (ejs == 0) {
        return 0;
    }
    ejs->location = location;

    ver = 0;
    if (ejsLoadModule(ejs, "ejs.web", ver, ver, 0) < 0) {
        mprError(ejs, "Can't load ejs.web.mod: %s", ejsGetErrorMsg(ejs, 1));
        return MPR_ERR_CANT_INITIALIZE;
    }
    if ((ejs->service->loadScriptFile)(ejs, location->script, NULL) == 0) {
        ejsReportError(ejs, "Can't load \"%s\"", location->script);
    } else {
        LOG(http, 2, "Loading Ejscript Server \"%s\"", location->script);
    }
    return 0;
}


#if VXWORKS
/*
    Create a routine to pull in the GCC support routines for double and int64 manipulations for some platforms. Do this
    incase modules reference these routines. Without this, the modules have to reference them. Which leads to multiple 
    defines if two modules include them. (Code to pull in moddi3, udivdi3, umoddi3)
 */
double  __ejsAppweb_floating_point_resolution(double a, double b, int64 c, int64 d, uint64 e, uint64 f) {
    a = a / b; a = a * b; c = c / d; c = c % d; e = e / f; e = e % f;
    c = (int64) a; d = (uint64) a; a = (double) c; a = (double) e;
    return (a == b) ? a : b;
}
#endif


/*  
    Ejs handler initialization
 */
int maEjsHandlerInit(Http *http, MprModule *mp)
{
    HttpStage     *handler;

    ejsService = ejsCreateService(http);
    ejsService->http = http;
    ejsInitCompiler(ejsService);

    /*
        Most of the Ejs handler is in ejsLib.c. Search for ejsAddWebHandler. Augment the handler with match, open
        and parse callbacks.
     */
    if ((handler = ejsAddWebHandler(http)) != 0) {
        handler->match = matchEjs;
        handler->open = openEjs;
        handler->parse = (HttpParse) parseEjs;
    }
    return 0;
}

#else
int maEjsHandlerInit(Http *http, MprModule *mp)
{
    return 0;
}

#endif /* BLD_FEATURE_EJS */

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
/************************************************************************/
/*
 *  End of file "../src/handlers/ejsHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/handlers/fileHandler.c"
 */
/************************************************************************/

/*
    fileHandler.c -- Static file content handler

    This handler manages static file based content such as HTML, GIF /or JPEG pages. It supports all methods including:
    GET, PUT, DELETE, OPTIONS and TRACE. It is event based and does not use worker threads.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static void handleDeleteRequest(HttpQueue *q);
static int  readFileData(HttpQueue *q, HttpPacket *packet);
static void handlePutRequest(HttpQueue *q);

/*
    Initialize a handler instance for the file handler.
 */
static void openFile(HttpQueue *q)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation    *location;
    HttpConn        *conn;
    char            *date;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;
    location = rec->location;

    if (rec->flags & (HTTP_GET | HTTP_HEAD | HTTP_POST)) {
        if (trans->fileInfo.valid && trans->fileInfo.mtime) {
            //  TODO - OPT could cache this
            date = httpGetDateString(rec, &trans->fileInfo);
            httpSetHeader(conn, "Last-Modified", date);
            mprFree(date);
        }
        if (httpContentNotModified(conn)) {
            httpSetStatus(conn, HTTP_CODE_NOT_MODIFIED);
            httpOmitBody(conn);
        } else {
            httpSetEntityLength(conn, (int) trans->fileInfo.size);
        }
        
        if (!trans->fileInfo.isReg && !trans->fileInfo.isLink) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't locate document: %s", rec->uri);
            
        } else if (!(trans->connector == conn->http->sendConnector)) {
            /*
                Open the file if a body must be sent with the response. The file will be automatically closed when 
                the response is freed. Cool eh?
             */
            trans->file = mprOpen(trans, trans->filename, O_RDONLY | O_BINARY, 0);
            if (trans->file == 0) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", trans->filename);
            }
        }

    } else if (rec->flags & (HTTP_PUT | HTTP_DELETE)) {
        if (location->flags & HTTP_LOC_PUT_DELETE) {
            httpOmitBody(conn);
        } else {
            httpError(q->conn, HTTP_CODE_BAD_METHOD, 
                "Method %s not supported by file handler at this location %s", rec->method, location->prefix);
        }
    } else {
        httpError(q->conn, HTTP_CODE_BAD_METHOD, 
            "Method %s not supported by file handler at this location %s", rec->method, location->prefix);
    }
}


static void startFile(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpPacket      *packet;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    
    if (trans->flags & HTTP_TRANS_NO_BODY) {
        if (rec->flags & HTTP_PUT) {
            handlePutRequest(q);
        } else if (rec->flags & HTTP_DELETE) {
            handleDeleteRequest(q);
        }
    } else {
        /* Create a single data packet based on the entity length.  */
        packet = httpCreateDataPacket(q, 0);
        packet->entityLength = trans->entityLength;
        if (!rec->ranges) {
            trans->length = trans->entityLength;
        }
        httpPutForService(q, packet, 0);
    }
}


static void processFile(HttpQueue *q)
{
    httpFinalize(q->conn);
}


/*  
    The service routine will be called when all incoming data has been received. This routine may flow control if the 
    downstream stage cannot accept all the file data. It will then be re-called as required to send more data.
 */
static void outgoingFileService(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpPacket      *packet;
    bool            usingSend;
    int             len;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;

    mprLog(q, 7, "OutgoingFileService");

    usingSend = trans->connector == conn->http->sendConnector;

    if (rec->ranges) {
        mprAssert(conn->http->rangeService);
        (*conn->http->rangeService)(q, (usingSend) ? NULL : readFileData);
    
    } else {
        for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
            if (!usingSend && packet->flags & HTTP_PACKET_DATA) {
                if (!httpWillNextQueueAcceptPacket(q, packet)) {
                    mprLog(q, 7, "OutgoingFileService downstream full, putback");
                    httpPutBackPacket(q, packet);
                    return;
                }
                if ((len = readFileData(q, packet)) < 0) {
                    return;
                }
                mprLog(q, 7, "OutgoingFileService readData %d", len);
                trans->pos += len;
            }
            httpSendPacketToNext(q, packet);
        }
    }
    mprLog(q, 7, "OutgoingFileService complete");
}


static void incomingFileData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;
    HttpRange       *range;
    MprBuf          *buf;
    MprFile         *file;
    int             len;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;
    file = (MprFile*) q->queueData;
    
    if (httpGetPacketLength(packet) == 0) {
        /*
            End of input
         */
        mprFree(file);
        q->queueData = 0;
        httpFreePacket(q, packet);
        return;
    }
    if (file == 0) {
        /*  
            Not a PUT so just ignore the incoming data.
         */
        httpFreePacket(q, packet);
        return;
    }
    buf = packet->content;
    len = mprGetBufLength(buf);
    mprAssert(len > 0);

    range = rec->inputRange;
    if (range && ((MprOffset) mprSeek(file, SEEK_SET, (long) range->start)) != range->start) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't seek to range start to %d", range->start);
    } else {
        if (mprWrite(file, mprGetBufStart(buf), len) != len) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't PUT to %s", trans->filename);
        }
    }
    httpFreePacket(q, packet);
}


/*  
    Populate a packet with file data
 */
static int readFileData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;
    int             len, rc;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;
    mprAssert(len >= 0);

    if (packet->content == 0) {
        len = packet->entityLength;
        if ((packet->content = mprCreateBuf(packet, len, len)) == 0) {
            return MPR_ERR_NO_MEMORY;
        }
    } else {
        len = mprGetBufSpace(packet->content);
    }
    mprAssert(len <= mprGetBufSpace(packet->content));    
    mprLog(q, 7, "readFileData len %d, pos %d", len, trans->pos);
    
    if (rec->ranges) {
        /*  
            rangeService will have set trans->pos to the next read position already
         */
        mprSeek(trans->file, SEEK_SET, trans->pos);
    }

    if ((rc = mprRead(trans->file, mprGetBufStart(packet->content), len)) != len) {
        /*  
            As we may have sent some data already to the client, the only thing we can do is abort and hope the client 
            notices the short data.
         */
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Can't read file %s", trans->filename);
        return MPR_ERR_CANT_READ;
    }

    mprAdjustBufEnd(packet->content, len);
    return len;
}


/*  
    This is called to setup for a HTTP PUT request. It is called before receiving the post data via incomingFileData
 */
static void handlePutRequest(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprFile         *file;
    char            *path;

    mprAssert(q->pair->queueData == 0);

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    path = trans->filename;
    if (path == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't map URI to file storage");
        return;
    }
    if (rec->ranges) {
        /*  
            Open an existing file with fall-back to create
         */
        file = mprOpen(q, path, O_BINARY | O_WRONLY, 0644);
        if (file == 0) {
            file = mprOpen(q, path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644);
            if (file == 0) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
                return;
            }
        } else {
            mprSeek(file, SEEK_SET, 0);
        }
    } else {
        file = mprOpen(q, path, O_CREAT | O_TRUNC | O_BINARY | O_WRONLY, 0644);
        if (file == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't create the put URI");
            return;
        }
    }
    httpSetStatus(conn, trans->fileInfo.isReg ? HTTP_CODE_NO_CONTENT : HTTP_CODE_CREATED);
    q->pair->queueData = (void*) file;
}


/*  
    Immediate processing of delete requests
 */
static void handleDeleteRequest(HttpQueue *q)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    char            *path;

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    path = trans->filename;
    if (path == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't map URI to file storage");
        return;
    }
    if (!conn->transmitter->fileInfo.isReg) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "URI not found");
        return;
    }
    if (mprDeletePath(q, path) < 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Can't remove URI");
        return;
    }
    httpSetStatus(conn, HTTP_CODE_NO_CONTENT);
}


/*  
    Dynamic module initialization
 */
int maOpenFileHandler(Http *http)
{
    HttpStage     *handler;

    /* 
        This handler serves requests without using thread workers.
     */
    handler = httpCreateHandler(http, "fileHandler", 
        HTTP_STAGE_GET | HTTP_STAGE_HEAD | HTTP_STAGE_POST | HTTP_STAGE_PUT | HTTP_STAGE_DELETE | HTTP_STAGE_VERIFY_ENTITY);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openFile;
    handler->start = startFile;
    handler->process = processFile;
    handler->outgoingService = outgoingFileService;
    handler->incomingData = incomingFileData;
    http->fileHandler = handler;
    return 0;
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
/************************************************************************/
/*
 *  End of file "../src/handlers/fileHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/handlers/phpHandler.c"
 */
/************************************************************************/

/*
    phpHandler.c - Appweb PHP handler

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if BLD_FEATURE_PHP

#if BLD_WIN_LIKE
    /*
        Workaround for VS 2005 and PHP headers. Need to include before PHP headers include it.
     */
    #if _MSC_VER >= 1400
        #include    <sys/utime.h>
    #endif
    #undef  WIN32
    #define WIN32 1
    #define WINNT 1
    #define TIME_H
    #undef _WIN32_WINNT
    #undef chdir
    #undef popen
    #undef pclose
    #define PHP_WIN32 1
    #define ZEND_WIN32 1
    #define ZTS 1
#endif
    #undef ulong
    #undef HAVE_SOCKLEN_T

    /*
        Indent headers to side-step make depend if PHP is not enabled
     */
    #include <main/php.h>
    #include <main/php_globals.h>
    #include <main/php_variables.h>
    #include <Zend/zend_modules.h>
    #include <main/SAPI.h>
#ifdef PHP_WIN32
    #include <win32/time.h>
    #include <win32/signal.h>
    #include <process.h>
#else
    #include <main/build-defs.h>
#endif
    #include <Zend/zend.h>
    #include <Zend/zend_extensions.h>
    #include <main/php_ini.h>
    #include <main/php_globals.h>
    #include <main/php_main.h>
#if ZTS
    #include <TSRM/TSRM.h>
#endif


typedef struct MaPhp {
    zval    *var_array;             /* Track var array */
} MaPhp;


static void flushOutput(void *context);
static void logMessage(char *message);
static char *readCookies(TSRMLS_D);
static int  readBodyData(char *buffer, uint len TSRMLS_DC);
static void registerServerVars(zval *varArray TSRMLS_DC);
static int  startup(sapi_module_struct *sapiModule);
static int  sendHeaders(sapi_headers_struct *sapiHeaders TSRMLS_DC);
static int  writeBlock(cchar *str, uint len TSRMLS_DC);

#if PHP_MAJOR_VERSION >=5 && PHP_MINOR_VERSION >= 3
static int  writeHeader(sapi_header_struct *sapiHeader, sapi_header_op_enum op, sapi_headers_struct *sapiHeaders TSRMLS_DC);
#else
static int  writeHeader(sapi_header_struct *sapiHeader, sapi_headers_struct *sapiHeaders TSRMLS_DC);
#endif

/*
    PHP Module Interface
 */
static sapi_module_struct phpSapiBlock = {
    BLD_PRODUCT,                    /* Sapi name */
    BLD_NAME,                       /* Full name */
    startup,                        /* Start routine */
    php_module_shutdown_wrapper,    /* Stop routine  */
    0,                              /* Activate */
    0,                              /* Deactivate */
    writeBlock,                     /* Write */
    flushOutput,                    /* Flush */
    0,                              /* Getuid */
    0,                              /* Getenv */
    php_error,                      /* Errors */
    writeHeader,                    /* Write headers */
    sendHeaders,                    /* Send headers */
    0,                              /* Send single header */
    readBodyData,                   /* Read body data */
    readCookies,                    /* Read session cookies */
    registerServerVars,             /* Define request variables */
    logMessage,                     /* Emit a log message */
    NULL,                           /* Override for the php.ini */
    0,                              /* Block */
    0,                              /* Unblock */
    STANDARD_SAPI_MODULE_PROPERTIES
};

/*
    Open the queue for a new request
 */
static void openPhp(HttpQueue *q)
{
    HttpReceiver      *rec;

    rec = q->conn->receiver;

    if (rec->flags & (HTTP_GET | HTTP_HEAD | HTTP_POST | HTTP_PUT)) {
        q->queueData = mprAllocObjZeroed(rec, MaPhp);
    } else {
        httpError(q->conn, HTTP_CODE_BAD_METHOD, "Method not supported by file handler: %s", rec->method);
    }
}


/*
    Process the request. Run once all the input data has been received
 */
static void processPhp(HttpQueue *q)
{
    HttpConn            *conn;
    HttpReceiver        *rec;
    HttpTransmitter     *trans;
    MprHash             *hp;
    MaPhp               *php;
    zend_file_handle    file_handle;

    TSRMLS_FETCH();

    conn = q->conn;
    rec = conn->receiver;
    trans = conn->transmitter;
    php = q->queueData;

    /*
        Set the request context
     */
    zend_first_try {
        php->var_array = 0;
        SG(server_context) = conn;
        SG(request_info).auth_user = conn->authUser;
        SG(request_info).auth_password = conn->authPassword;
        SG(request_info).content_type = rec->mimeType;
        SG(request_info).content_length = rec->length;
        SG(sapi_headers).http_response_code = HTTP_CODE_OK;
        SG(request_info).path_translated = trans->filename;
        SG(request_info).query_string = rec->parsedUri->query;
        SG(request_info).request_method = rec->method;
        SG(request_info).request_uri = rec->uri;

        /*
            Workaround on MAC OS X where the SIGPROF is given to the wrong thread
            TODO - need to implement a local timeout here via the host timeout. Then invoke zend_bailout.
         */
        PG(max_input_time) = -1;
        EG(timeout_seconds) = 0;

        php_request_startup(TSRMLS_C);
        CG(zend_lineno) = 0;

    } zend_catch {
        mprError(q, "Can't start PHP request");
        zend_try {
            php_request_shutdown(0);
        } zend_end_try();
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "PHP initialization failed");
        return;
    } zend_end_try();

    /*
        Define the header variable
     */
    zend_try {
        hp = mprGetFirstHash(rec->headers);
        while (hp) {
            if (hp->data) {
                char key[MPR_MAX_STRING];
                mprStrcpy(key, sizeof(key), hp->key);
                mprStrUpper(key);
                php_register_variable(key, (char*) hp->data, php->var_array TSRMLS_CC);
            }
            hp = mprGetNextHash(rec->headers, hp);
        }
        hp = mprGetFirstHash(rec->formVars);
        while (hp) {
            if (hp->data) {
                char key[MPR_MAX_STRING];
                mprStrcpy(key, sizeof(key), hp->key);
                mprStrUpper(key);
                php_register_variable(key, (char*) hp->data, php->var_array TSRMLS_CC);
            }
            hp = mprGetNextHash(rec->formVars, hp);
        }
    } zend_end_try();

    /*
        Execute the script file
     */
    file_handle.filename = trans->filename;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = 0;

    zend_try {
        php_execute_script(&file_handle TSRMLS_CC);
        if (!SG(headers_sent)) {
            sapi_send_headers(TSRMLS_C);
        }
    } zend_catch {
        php_request_shutdown(0);
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR,  "PHP script execution failed");
        return;
    } zend_end_try();

    zend_try {
        php_request_shutdown(0);
    } zend_end_try();

    httpFinalize(conn);
}

 /*************************** PHP Support Functions ***************************/
/*
    Flush write data back to the client
 */
static void flushOutput(void *server_context)
{
    HttpConn      *conn;

    conn = (HttpConn*) server_context;
    if (conn) {
        httpServiceQueues(conn);
    }
}


static int writeBlock(cchar *str, uint len TSRMLS_DC)
{
    HttpConn    *conn;
    int         written;

    conn = (HttpConn*) SG(server_context);
    if (conn == 0) {
        return -1;
    }
    written = httpWriteBlock(conn->transmitter->queue[HTTP_QUEUE_TRANS].nextQ, str, len, 0);
    if (written <= 0) {
        php_handle_aborted_connection();
    }
    return written;
}


static void registerServerVars(zval *track_vars_array TSRMLS_DC)
{
    HttpConn    *conn;
    MaPhp       *php;

    conn = (HttpConn*) SG(server_context);
    if (conn == 0) {
        return;
    }
    php_import_environment_variables(track_vars_array TSRMLS_CC);

    if (SG(request_info).request_uri) {
        php_register_variable("PHP_SELF", SG(request_info).request_uri,  track_vars_array TSRMLS_CC);
    }
    php = httpGetQueueData(conn);
    mprAssert(php);
    php->var_array = track_vars_array;
}


static void logMessage(char *message)
{
    mprLog(mprGetMpr(), 0, "phpModule: %s", message);
}


static char *readCookies(TSRMLS_D)
{
    HttpConn      *conn;

    conn = (HttpConn*) SG(server_context);
    return conn->receiver->cookie;
}


static int sendHeaders(sapi_headers_struct *phpHeaders TSRMLS_DC)
{
    HttpConn      *conn;

    conn = (HttpConn*) SG(server_context);
    httpSetStatus(conn, phpHeaders->http_response_code);
    httpSetMimeType(conn, phpHeaders->mimetype);
    return SAPI_HEADER_SENT_SUCCESSFULLY;
}


#if PHP_MAJOR_VERSION >=5 && PHP_MINOR_VERSION >= 3
static int writeHeader(sapi_header_struct *sapiHeader, sapi_header_op_enum op, sapi_headers_struct *sapiHeaders TSRMLS_DC)
#else
static int writeHeader(sapi_header_struct *sapiHeader, sapi_headers_struct *sapi_headers TSRMLS_DC)
#endif
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    bool            allowMultiple;
    char            *key, *value;

    conn = (HttpConn*) SG(server_context);
    trans = conn->transmitter;
    allowMultiple = 1;

    key = mprStrdup(trans, sapiHeader->header);
    if ((value = strchr(key, ':')) == 0) {
        return -1;
    }
    *value++ = '\0';
    while (!isalnum(*value) && *value) {
        value++;
    }
#if PHP_MAJOR_VERSION >=5 && PHP_MINOR_VERSION >= 3
    switch(op) {
        case SAPI_HEADER_DELETE_ALL:
            //  TODO - not supported
            return 0;

        case SAPI_HEADER_DELETE:
            //  TODO - not supported
            return 0;

        case SAPI_HEADER_REPLACE:
            httpSetHeader(conn, key, value);
            return SAPI_HEADER_ADD;

        case SAPI_HEADER_ADD:
            httpAppendHeader(conn, key, value);
            return SAPI_HEADER_ADD;

        default:
            return 0;
    }
#else
    allowMultiple = !sapiHeader->replace;
    if (allowMultiple) {
        httpAppendHeader(conn, key, value);
    } else {
        httpSetHeader(conn, key, value);
    }
    return SAPI_HEADER_ADD;
#endif
}


static int readBodyData(char *buffer, uint bufsize TSRMLS_DC)
{
    HttpConn    *conn;
    HttpQueue   *q;
    MprBuf      *content;
    int         len;

    conn = (HttpConn*) SG(server_context);
    q = conn->transmitter->queue[HTTP_QUEUE_RECEIVE].prevQ;
    if (q->first == 0) {
        return 0;
    }
    content = q->first->content;
    len = min(mprGetBufLength(content), (int) bufsize);
    if (len > 0) {
        mprMemcpy(buffer, len, mprGetBufStart(content), len);
        mprAdjustBufStart(content, len);
    }
    return len;
}


static int startup(sapi_module_struct *sapi_module)
{
    return php_module_startup(sapi_module, 0, 0);
}


/*
    Initialze the php module
 */
static int initializePhp(Http *http)
{
    MaAppweb                *appweb;
#if ZTS
    void                    ***tsrm_ls;
    php_core_globals        *core_globals;
    sapi_globals_struct     *sapi_globals;
    zend_llist              global_vars;
    zend_compiler_globals   *compiler_globals;
    zend_executor_globals   *executor_globals;

    tsrm_startup(128, 1, 0, 0);
    compiler_globals = (zend_compiler_globals*)  ts_resource(compiler_globals_id);
    executor_globals = (zend_executor_globals*)  ts_resource(executor_globals_id);
    core_globals = (php_core_globals*) ts_resource(core_globals_id);
    sapi_globals = (sapi_globals_struct*) ts_resource(sapi_globals_id);
    tsrm_ls = (void***) ts_resource(0);
#endif

    appweb = httpGetContext(http);
    phpSapiBlock.php_ini_path_override = appweb->defaultServer->serverRoot;
    sapi_startup(&phpSapiBlock);
    if (php_module_startup(&phpSapiBlock, 0, 0) == FAILURE) {
        mprError(http, "PHP did not initialize");
        return MPR_ERR_CANT_INITIALIZE;
    }

#if ZTS
    zend_llist_init(&global_vars, sizeof(char *), 0, 0);
#endif

    SG(options) |= SAPI_OPTION_NO_CHDIR;
    zend_alter_ini_entry("register_argc_argv", 19, "0", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
    zend_alter_ini_entry("html_errors", 12, "0", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
    zend_alter_ini_entry("implicit_flush", 15, "0", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
    return 0;
}


static int finalizePhp(MprModule *mp)
{
    TSRMLS_FETCH();
    php_module_shutdown(TSRMLS_C);
    sapi_shutdown();
#if ZTS
    tsrm_shutdown();
#endif
    return 0;
}


/*
    Dynamic module initialization
 */
int maPhpHandlerInit(Http *http, MprModule *mp)
{
    HttpStage     *handler;

    handler = httpCreateHandler(http, "phpHandler", 
        HTTP_STAGE_GET | HTTP_STAGE_HEAD | HTTP_STAGE_PUT | HTTP_STAGE_DELETE | HTTP_STAGE_POST | HTTP_STAGE_ENV_VARS | 
        HTTP_STAGE_PATH_INFO | HTTP_STAGE_VERIFY_ENTITY | HTTP_STAGE_THREAD);
    if (handler == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    handler->open = openPhp;
    handler->process = processPhp;
    mp->stop = finalizePhp;
    initializePhp(http);
    return 0;
}


#else
int maPhpHandlerInit(Http *http, MprModule *mp)
{
    return 0;
}

#endif /* BLD_FEATURE_PHP */

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
/************************************************************************/
/*
 *  End of file "../src/handlers/phpHandler.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/host.c"
 */
/************************************************************************/

/*
    host.c -- Host class for all HTTP hosts

    The Host class is used for the default HTTP server and for all virtual hosts (including SSL hosts).
    Many objects are controlled at the host level. Eg. URL handlers.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




/*  Create a host from scratch
 */
MaHost *maCreateHost(MaServer *server, cchar *ipAddrPort, HttpLocation *location)
{
    MaHost      *host;

    host = mprAllocObjZeroed(server, MaHost);
    if (host == 0) {
        return 0;
    }
    host->aliases = mprCreateList(host);
    host->dirs = mprCreateList(host);
#if UNUSED
    host->connections = mprCreateList(host);
#endif
    host->locations = mprCreateList(host);

    if (ipAddrPort) {
        maSetHostIpAddrPort(host, ipAddrPort);
    }
    host->server = server;
    host->flags = MA_HOST_NO_TRACE;
    host->httpVersion = 1;

#if UNUSED
    //  MOB -- is this used anymore 
    host->timeout = HTTP_SERVER_TIMEOUT;

    //  MOB -- is this necessary as limits are pushed down into http
    host->limits = server->appweb->http->limits;

    host->keepAliveTimeout = HTTP_KEEP_TIMEOUT;
    host->maxKeepAlive = HTTP_MAX_KEEP_ALIVE;
    host->keepAlive = 1;
#endif

    host->location = (location) ? location : httpCreateLocation(server->http);
    maAddLocation(host, host->location);
    host->location->auth = httpCreateAuth(host->location, host->location->auth);
#if UNUSED
    host->mutex = mprCreateLock(host);
#endif
    return host;
}


/*  
    Create a new virtual host and inherit settings from another host
 */
MaHost *maCreateVirtualHost(MaServer *server, cchar *ipAddrPort, MaHost *parent)
{
    MaHost      *host;

    host = mprAllocObjZeroed(server, MaHost);
    if (host == 0) {
        return 0;
    }
    host->parent = parent;
#if UNUSED
    host->connections = mprCreateList(host);
#endif
    if (ipAddrPort) {
        maSetHostIpAddrPort(host, ipAddrPort);
    }
    /*  
        The aliases, dirs and locations are all copy-on-write
     */
    host->aliases = parent->aliases;
    host->dirs = parent->dirs;
    host->locations = parent->locations;
    host->server = parent->server;
    host->flags = parent->flags;
    host->httpVersion = parent->httpVersion;
#if UNUSED
    host->timeout = parent->timeout;
    host->limits = parent->limits;
    host->keepAliveTimeout = parent->keepAliveTimeout;
    host->maxKeepAlive = parent->maxKeepAlive;
    host->keepAlive = parent->keepAlive;
#endif
    host->accessLog = parent->accessLog;
    host->location = httpCreateInheritedLocation(server->http, parent->location);
#if UNUSED
    host->mutex = mprCreateLock(host);
#endif
    maAddLocation(host, host->location);
    return host;
}


/*  
    Convenience function to create a new default host
 */
MaHost *maCreateDefaultHost(MaServer *server, cchar *docRoot, cchar *ip, int port)
{
    MaHost          *host;
    HttpServer      *httpServer;
    MaHostAddress   *address;

    if (ip == 0) {
        /*  
            If no IP:PORT specified, find the first listening endpoint. In this case, we expect the caller to
            have setup the lisenting endponts and to have added them to the host address hash.
         */
        httpServer = mprGetFirstItem(server->httpServers);
        if (httpServer) {
            ip = httpServer->ip;
            port = httpServer->port;

        } else {
            ip = "localhost";
            httpServer = httpCreateServer(server->appweb->http, ip, HTTP_DEFAULT_PORT, NULL);
            maAddHttpServer(server, httpServer);
            port = HTTP_DEFAULT_PORT;
        }
        host = maCreateHost(server, ip, NULL);

    } else {
        /*  
            Create a new listening endpoint
         */
        httpServer = httpCreateServer(server->appweb->http, ip, port, NULL);
        maAddHttpServer(server, httpServer);
        host = maCreateHost(server, ip, NULL);
    }

    if (maOpenMimeTypes(host, "mime.types") < 0) {
        maAddStandardMimeTypes(host);
    }

    /*  
        Insert the host and create a directory object for the docRoot
     */
    maAddHost(server, host);
    maInsertDir(host, maCreateBareDir(host, docRoot));
    maSetDocumentRoot(host, docRoot);

    /* 
        Ensure we are in the hash lookup of all the addresses to listen to acceptWrapper uses this hash to find
        the host to serve the request.
     */
    address = maLookupHostAddress(server, ip, port);
    if (address == 0) {
        address = maCreateHostAddress(server, ip, port);
        mprAddItem(server->hostAddresses, address);
    }
    maInsertVirtualHost(address, host);

    if (server->defaultHost == 0) {
        server->defaultHost = host;
    }
    return host;
}


int maStartHost(MaHost *host)
{
    return maStartAccessLogging(host);
}


int maStopHost(MaHost *host)
{
    return maStopAccessLogging(host);
}


void maSetDocumentRoot(MaHost *host, cchar *dir)
{
    MaAlias     *alias;
    char        *doc;
    int         len;

    doc = host->documentRoot = maMakePath(host, dir);
    len = (int) strlen(doc);
    if (doc[len - 1] == '/') {
        doc[len - 1] = '\0';
    }
    /*  Create a catch-all alias
     */
    alias = maCreateAlias(host, "", doc, 0);
    maInsertAlias(host, alias);
}


/*  
    Set the host name. Comes from the ServerName directive. Name should not contain "http://"
 */
void maSetHostName(MaHost *host, cchar *name)
{
    if (host->name) {
        mprFree(host->name);
    }
    host->name = mprStrdup(host, name);
}


void maSetHostIpAddrPort(MaHost *host, cchar *ipAddrPort)
{
    char    *cp;

    mprAssert(ipAddrPort);
    mprFree(host->ipAddrPort);

    if (*ipAddrPort == ':') {
        ++ipAddrPort;
    }
    if (isdigit((int) *ipAddrPort) && strchr(ipAddrPort, '.') == 0) {
        host->ipAddrPort = mprStrcat(host, -1, "127.0.0.1", ":", ipAddrPort, NULL);
    } else {
        host->ipAddrPort = mprStrdup(host, ipAddrPort);
    }
    if ((cp = strchr(host->ipAddrPort, ':')) != 0) {
        *cp++ = '\0';
        host->ip = mprStrdup(host, host->ipAddrPort);
        host->port = (int) mprAtoi(cp, 10);
        cp[-1] = ':';
    }
    if (host->name == 0) {
        host->name = mprStrdup(host, host->ipAddrPort);
    }
}


void maSetHttpVersion(MaHost *host, int version)
{
    host->httpVersion = version;
}


#if UNUSED
void maSetKeepAlive(MaHost *host, bool on)
{
    host->keepAlive = on;
}


void maSetKeepAliveTimeout(MaHost *host, int timeout)
{
    //  MOB -- is this used
    host->keepAliveTimeout = timeout;
}


void maSetMaxKeepAlive(MaHost *host, int timeout)
{
    //  MOB -- is this used
    host->maxKeepAlive = timeout;
}
#endif


void maSetNamedVirtualHost(MaHost *host)
{
    host->flags |= MA_HOST_NAMED_VHOST;
}


void maSecureHost(MaHost *host, struct MprSsl *ssl)
{
    HttpServer  *httpServer;
    cchar       *hostIp;
    char        *ip;
    int         port, next;

#if UNUSED
    host->secure = 1;
#endif
    hostIp = host->ipAddrPort;
    if (mprStrcmpAnyCase(hostIp, "_default_") == 0) {
        hostIp = (char*) "*:*";
    }
    mprParseIp(host, hostIp, &ip, &port, -1);
   
    for (next = 0; (httpServer = mprGetNextItem(host->server->httpServers, &next)) != 0; ) {
        if (port > 0 && port != httpServer->port) {
            continue;
        }
        if (*httpServer->ip && ip && ip[0] != '*' && strcmp(ip, httpServer->ip) != 0) {
            continue;
        }
#if BLD_FEATURE_SSL
        if (host->flags & MA_HOST_NAMED_VHOST) {
            mprError(host, "SSL does not support named virtual hosts");
            return;
        }
        httpServer->ssl = ssl;
#endif
    }
}


void maSetVirtualHost(MaHost *host)
{
    host->flags |= MA_HOST_VHOST;
}


int maInsertAlias(MaHost *host, MaAlias *newAlias)
{
    MaAlias     *alias, *old;
    int         rc, next, index;

    if (mprGetParent(host->aliases) == host->parent) {
        host->aliases = mprDupList(host, host->parent->aliases);
    }

    /*  Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc. But we sort redirects with
        status codes first.
     */
    for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
        rc = strcmp(newAlias->prefix, alias->prefix);
        if (rc == 0) {
            index = mprLookupItem(host->aliases, alias);
            old = (MaAlias*) mprGetItem(host->aliases, index);
            mprRemoveItem(host->aliases, alias);
            mprInsertItemAtPos(host->aliases, next - 1, newAlias);
            return 0;
            
        } else if (rc > 0) {
            if (newAlias->redirectCode >= alias->redirectCode) {
                mprInsertItemAtPos(host->aliases, next - 1, newAlias);
                return 0;
            }
        }
    }
    mprAddItem(host->aliases, newAlias);
    return 0;
}


int maInsertDir(MaHost *host, MaDir *newDir)
{
    MaDir       *dir;
    int         rc, next;

    mprAssert(newDir);
    mprAssert(newDir->path);
    
    if (mprGetParent(host->dirs) == host->parent) {
        host->dirs = mprDupList(host, host->parent->dirs);
    }

    /*
        Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc
     */
    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        mprAssert(dir->path);
        rc = strcmp(newDir->path, dir->path);
        if (rc == 0) {
            mprRemoveItem(host->dirs, dir);
            mprInsertItemAtPos(host->dirs, next - 1, newDir);
            return 0;

        } else if (rc > 0) {
            mprInsertItemAtPos(host->dirs, next - 1, newDir);
            return 0;
        }
    }

    mprAddItem(host->dirs, newDir);
    return 0;
}


int maAddLocation(MaHost *host, HttpLocation *newLocation)
{
    HttpLocation  *location;
    int         next, rc;

    mprAssert(newLocation);
    mprAssert(newLocation->prefix);
    
    if (mprGetParent(host->locations) == host->parent) {
        host->locations = mprDupList(host, host->parent->locations);
    }

    /*
        Sort in reverse collating sequence. Must make sure that /abc/def sorts before /abc
     */
    for (next = 0; (location = mprGetNextItem(host->locations, &next)) != 0; ) {
        rc = strcmp(newLocation->prefix, location->prefix);
        if (rc == 0) {
            mprRemoveItem(host->locations, location);
            mprInsertItemAtPos(host->locations, next - 1, newLocation);
            return 0;
        }
        if (strcmp(newLocation->prefix, location->prefix) > 0) {
            mprInsertItemAtPos(host->locations, next - 1, newLocation);
            return 0;
        }
    }
    mprAddItem(host->locations, newLocation);

    return 0;
}


MaAlias *maGetAlias(MaHost *host, cchar *uri)
{
    MaAlias     *alias;
    int         next;

    if (uri) {
        for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
            if (strncmp(alias->prefix, uri, alias->prefixLen) == 0) {
                if (uri[alias->prefixLen] == '\0' || uri[alias->prefixLen] == '/') {
                    return alias;
                }
            }
        }
    }
    /*
        Must always return an alias. The last is the catch-all.
     */
    return mprGetLastItem(host->aliases);
}


MaAlias *maLookupAlias(MaHost *host, cchar *prefix)
{
    MaAlias     *alias;
    int         next;

    for (next = 0; (alias = mprGetNextItem(host->aliases, &next)) != 0; ) {
        if (strcmp(alias->prefix, prefix) == 0) {
            return alias;
        }
    }
    return 0;
}


/*  
    Find an exact dir match
 */
MaDir *maLookupDir(MaHost *host, cchar *pathArg)
{
    MaDir       *dir;
    char        *path, *tmpPath;
    int         next, len;

    if (!mprIsAbsPath(host, pathArg)) {
        path = tmpPath = mprGetAbsPath(host, pathArg);
    } else {
        path = (char*) pathArg;
        tmpPath = 0;
    }
    len = (int) strlen(path);

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        mprAssert(strlen(dir->path) == 0 || dir->path[strlen(dir->path) - 1] != '/');
        if (dir->path != 0) {
            if (mprSamePath(host, dir->path, path)) {
                mprFree(tmpPath);
                return dir;
            }
        }
    }
    mprFree(tmpPath);
    return 0;
}


/*  
    Find the directory entry that this file (path) resides in. path is a physical file path. We find the most specific
    (longest) directory that matches. The directory must match or be a parent of path. Not called with raw files names.
    They will be lower case and only have forward slashes. For windows, the will be in cannonical format with drive
    specifiers.
 */
MaDir *maLookupBestDir(MaHost *host, cchar *path)
{
    MaDir   *dir;
    int     next, len, dlen;

    len = (int) strlen(path);

    for (next = 0; (dir = mprGetNextItem(host->dirs, &next)) != 0; ) {
        dlen = dir->pathLen;
        mprAssert(dlen == 0 || dir->path[dlen - 1] != '/');
        if (mprSamePathCount(host, dir->path, path, dlen)) {
            if (dlen >= 0) {
                return dir;
            }
        }
    }
    return 0;
}


HttpLocation *maLookupLocation(MaHost *host, cchar *prefix)
{
    HttpLocation      *location;
    int             next;

    for (next = 0; (location = mprGetNextItem(host->locations, &next)) != 0; ) {
        if (strcmp(prefix, location->prefix) == 0) {
            return location;
        }
    }
    return 0;
}


HttpLocation *maLookupBestLocation(MaHost *host, cchar *uri)
{
    HttpLocation  *location;
    int         next, rc;

    if (uri) {
        for (next = 0; (location = mprGetNextItem(host->locations, &next)) != 0; ) {
            rc = strncmp(location->prefix, uri, location->prefixLen);
            if (rc == 0 && uri[location->prefixLen] == '/') {
                return location;
            }
        }
    }
    return mprGetLastItem(host->locations);
}


MaHostAddress *maCreateHostAddress(MprCtx ctx, cchar *ip, int port)
{
    MaHostAddress   *hostAddress;

    mprAssert(ip && ip);
    mprAssert(port >= 0);

    hostAddress = mprAllocObjZeroed(ctx, MaHostAddress);
    if (hostAddress == 0) {
        return 0;
    }
    hostAddress->flags = 0;
    hostAddress->ip = mprStrdup(hostAddress, ip);
    hostAddress->port = port;
    hostAddress->vhosts = mprCreateList(hostAddress);
    return hostAddress;
}


void maInsertVirtualHost(MaHostAddress *hostAddress, MaHost *vhost)
{
    mprAddItem(hostAddress->vhosts, vhost);
}


bool maIsNamedVirtualHostAddress(MaHostAddress *hostAddress)
{
    return hostAddress->flags & MA_IPADDR_VHOST;
}


void maSetNamedVirtualHostAddress(MaHostAddress *hostAddress)
{
    hostAddress->flags |= MA_IPADDR_VHOST;
}


/*
    Look for a host with the right host name (ServerName)
 */
MaHost *maLookupVirtualHost(MaHostAddress *hostAddress, cchar *hostStr)
{
    MaHost      *host;
    int         next;

    for (next = 0; (host = mprGetNextItem(hostAddress->vhosts, &next)) != 0; ) {
        /*  TODO  -- need to support aliases */
        if (hostStr == 0 || strcmp(hostStr, host->name) == 0) {
            return host;
        }
    }
    return 0;
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
/************************************************************************/
/*
 *  End of file "../src/host.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/link.c"
 */
/************************************************************************/

/*
    link.c -- Link in static modules and initialize.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if BLD_FEATURE_STATIC || (!BLD_APPWEB_PRODUCT && BLD_APPWEB_BUILTIN)

static MprList *staticModules;


static int loadStaticModule(Http *http, cchar *name, int (*callback)(Http *http, MprModule *mp))
{
    MprModule   *mp;
    int         rc;

    mprAssert(staticModules);
    mprAssert(name && *name);

    mp = mprCreateModule(http, name, NULL);
    rc = (callback)(http, mp);
    if (rc == 0) {
        mprAddItem(staticModules, mp);
    }
    return rc;
}


void maLoadStaticModules(MaAppweb *appweb)
{
    Http    *http;

    http = appweb->http;
    staticModules = mprCreateList(http);

#if BLD_FEATURE_CGI
    loadStaticModule(http, "mod_cgi", maCgiHandlerInit);
#endif
#if BLD_FEATURE_EJS
    loadStaticModule(http, "mod_ejs", maEjsHandlerInit);
#endif
#if BLD_FEATURE_SSL
    loadStaticModule(http, "mod_ssl", maSslModuleInit);
#endif
#if BLD_FEATURE_PHP
    loadStaticModule(http, "mod_php", maPhpHandlerInit);
#endif
}


void maUnloadStaticModules(MaAppweb *appweb)
{
    MprModule   *mp;
    int         next;

    for (next = 0; (mp = mprGetNextItem(staticModules, &next)) != 0; ) {
        mprUnloadModule(mp);
    }
}

#else
void maLoadStaticModules(MaAppweb *appweb) {}
void maUnloadStaticModules(MaAppweb *appweb) {}

#endif /* BLD_FEATURE_STATIC */

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
/************************************************************************/
/*
 *  End of file "../src/link.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/log.c"
 */
/************************************************************************/

/*
    log.c -- Logging

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */



/*
    Turn on logging. If no logSpec is specified, default to stdout:2. If the user specifies --log "none" then 
    the log is disabled. This is useful when specifying the log via the appweb.conf.
 */
static void logHandler(MprCtx ctx, int flags, int level, cchar *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *prefix, buf[MPR_MAX_STRING];

    mpr = mprGetMpr();
    if ((file = (MprFile*) mpr->logHandlerData) == 0) {
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
        mprSprintf(buf, sizeof(buf), "%s: Fatal: %s\n", prefix, msg);
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
    mpr = mprGetMpr();

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

    mpr = mprGetMpr();

    file = mpr->logHandlerData;
    if (file) {
        mprFree(file);
        mpr->logHandlerData = 0;
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
void maRotateAccessLog(MaHost *host)
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

        mprSprintf(bak, sizeof(bak), "%s-%02d-%02d-%02d-%02d:%02d:%02d", host->logPath, 
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
/************************************************************************/
/*
 *  End of file "../src/log.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/match.c"
 */
/************************************************************************/

/*
    match.c -- HTTP pipeline matching.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static char *addIndexToUrl(HttpConn *conn, cchar *index);
static HttpStage *checkStage(HttpConn *conn, HttpStage *stage);
static char *getExtension(HttpConn *conn);
static HttpStage *findHandlerByExtension(HttpConn *conn, bool *rescan);
static HttpStage *findLocationHandler(HttpConn *conn);
static char *makeFilename(HttpConn *conn, MaAlias *alias, cchar *url, bool skipAliasPrefix);
static bool mapToFile(HttpConn *conn, HttpStage *handler, bool *rescan);
static HttpStage *matchHandler(HttpConn *conn);
static void processDirectory(HttpConn *conn, bool *rescan);
static void prepRequest(HttpConn *conn, HttpStage *handler);
static void setPathInfo(HttpConn *conn, HttpStage *handler);

//  MOB
int maRewriteUri(HttpConn *conn);

/*
    Notification callback from Http. Invoked for each state change.
 */
void maNotifyServerStateChange(HttpConn *conn, int state, int notifyFlags)
{
    MprSocket       *listenSock;
    MaHostAddress   *address;
    MaHost          *host;
    MaServer        *server;
    HttpReceiver    *rec;

    mprAssert(conn);

    switch (state) {
    case HTTP_STATE_PARSED:
        server = httpGetMetaServer(conn->server);
        listenSock = conn->sock->listenSock;
        address = (MaHostAddress*) maLookupHostAddress(server, listenSock->ip, listenSock->port);
        if (address == 0 || (host = mprGetFirstItem(address->vhosts)) == 0) {
            mprError(server, "No host configured for request %s:%d", listenSock->ip, listenSock->port);
            //  MOB TODO - should cancel request
            return;
        }
        if (maIsNamedVirtualHostAddress(address)) {
            rec = conn->receiver;
            if ((host = maLookupVirtualHost(address, rec->hostName)) == 0) {
                httpError(conn, HTTP_CODE_NOT_FOUND, "No host to serve request. Searching for %s", rec->hostName);
            }
        }

        httpSetConnHost(conn, host);
        conn->documentRoot = host->documentRoot;
        conn->server->serverRoot = server->serverRoot;
        if (conn->receiver) {
            conn->receiver->location = host->location;
        }
        conn->transmitter->handler = matchHandler(conn);
        break;
    }
}


//  MOB -- this routine can be in Http

/*
    Find the matching handler for a request. If any errors occur, the pass handler is used to pass errors onto the 
    net/sendfile connectors to send to the client. This routine may rewrite the request URI and may redirect the request.
 */
static HttpStage *matchHandler(HttpConn *conn)
{
    Http            *http;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation    *location;
    HttpStage       *handler;
    MaHost          *host;
    MaAlias         *alias;
    bool            rescan;
    int             loopCount;

    http = conn->http;
    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    /*
        Find the alias that applies for this url. There is always a catch-all alias for the document root.
     */
    alias = rec->alias = maGetAlias(host, rec->pathInfo);
    mprAssert(alias);
    if (alias->redirectCode) {
        httpRedirect(conn, alias->redirectCode, alias->uri);
        return 0;
    }
    if (conn->error || (conn->receiver->flags & (HTTP_OPTIONS | HTTP_TRACE))) {
        location = rec->location = maLookupBestLocation(host, rec->pathInfo);
        mprAssert(location);
        rec->auth = location->auth;        
        return http->passHandler;
    }

    /*
        Get the best (innermost) location block and see if a handler is explicitly set for that location block.
        Possibly rewrite the url and retry.
     */
    loopCount = MA_MAX_REWRITE;
    do {
        rescan = 0;
        if ((handler = findLocationHandler(conn)) == 0) {
            handler = findHandlerByExtension(conn, &rescan);
        }
    } while (handler && rescan && loopCount-- > 0);

    if (conn->receiver->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        trans->traceMethods = handler->flags;
        handler = http->passHandler;
    }
    if (handler == 0) {
        httpError(conn, HTTP_CODE_BAD_METHOD, "Requested method %s not supported for URL: %s", rec->method, rec->uri);
    }
    if (conn->error || ((trans->flags & HTTP_TRANS_NO_BODY) && !(rec->flags & HTTP_HEAD))) {
        handler = http->passHandler;
    }
    location = rec->location;
    if (location && location->connector) {
        trans->connector = location->connector;
    } else if (handler == http->fileHandler && !rec->ranges && !conn->secure && trans->chunkSize <= 0) {
        trans->connector = http->sendConnector;
    } else {
        trans->connector = http->netConnector;
    }
    prepRequest(conn, handler);
    mprLog(trans, 4, "Select handler: \"%s\" for \"%s\"", handler->name, rec->uri);
    return handler;
}


static HttpStage *findLocationHandler(HttpConn *conn)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpLocation    *location;
    HttpStage       *handler;
    MaHost          *host;
    int             loopCount;

    rec = conn->receiver;
    trans = conn->transmitter;
    loopCount = MA_MAX_REWRITE;
    host = httpGetConnHost(conn);
    handler = 0;
    do {
        location = rec->location = maLookupBestLocation(host, rec->pathInfo);
        mprAssert(location);
        rec->auth = location->auth;
        handler = checkStage(conn, location->handler);
    } while (maRewriteUri(conn) && --loopCount > 0);
    return handler;
}


/*
    Get an extension used for mime type matching. NOTE: this does not permit any kind of platform specific filename.
    Rather only those suitable as mime type extensions (simple alpha numeric extensions)
 */
static char *getExtension(HttpConn *conn)
{
    HttpReceiver    *rec;
    char            *cp;
    char            *ep, *ext;

    rec = conn->receiver;
    if (rec && rec->pathInfo) {
        if ((cp = strrchr(&rec->pathInfo[rec->alias->prefixLen], '.')) != 0) {
            ext = mprStrdup(rec, ++cp);
            for (ep = ext; *ep && isalnum((int)*ep); ep++) {
                ;
            }
            *ep = '\0';
            return ext;
        }
    }
    return "";
}


/*
    Search for a handler by request extension. If that fails, use handler custom matching.
 */
static HttpStage *findHandlerByExtension(HttpConn *conn, bool *rescan)
{
    Http            *http;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    HttpStage       *handler;
    HttpLocation    *location;
    int             next;

    http = conn->http;
    rec = conn->receiver;
    trans = conn->transmitter;
    location = rec->location;
    handler = 0;
    *rescan = 0;
    
    if (rec->pathInfo == 0) {
        handler = http->passHandler;
    } else {
        trans->extension = getExtension(conn);
        if (*trans->extension) {
            handler = httpGetHandlerByExtension(location, trans->extension);
            if (!checkStage(conn, handler)) {
                handler = 0;
            }
        }
        if (handler == 0) {
            /*
                Failed to match by extension, so perform custom handler matching on all defined handlers
             */
            trans->filename = makeFilename(conn, rec->alias, rec->pathInfo, 1);
            for (next = 0; (handler = mprGetNextItem(location->handlers, &next)) != 0; ) {
                if (handler->match && handler->match(conn, handler)) {
                    if (checkStage(conn, handler)) {
                        break;
                    }
                }
            }
        }
    }
    if (handler == 0 && (handler = httpGetHandlerByExtension(location, "")) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing handler to match request %s", rec->pathInfo);
        handler = http->passHandler;
    }
    if (mapToFile(conn, handler, rescan)) {
        if (trans->fileInfo.isDir) {
            processDirectory(conn, rescan);
        }
    } else {
        handler = http->passHandler;
    }
    return handler;
}


static char *makeFilename(HttpConn *conn, MaAlias *alias, cchar *url, bool skipAliasPrefix)
{
    char        *cleanPath, *path;
    int         len;

    mprAssert(alias);
    mprAssert(url);

    if (skipAliasPrefix) {
        url += alias->prefixLen;
    }
    while (*url == '/') {
        url++;
    }
    len = (int) strlen(alias->filename);
    if ((path = mprAlloc(conn->receiver, len + (int) strlen(url) + 2)) == 0) {
        return 0;
    }
    strcpy(path, alias->filename);
    if (*url) {
        path[len++] = mprGetPathSeparator(path, path);
        strcpy(&path[len], url);
    }
    cleanPath = mprGetNativePath(conn, path);
    mprFree(path);
    return cleanPath;
}


//  MOB -- if the fileHandler is moved to http, then this code needs to go also
//  MOB -- perhaps this should be in Http regardless

static bool mapToFile(HttpConn *conn, HttpStage *handler, bool *rescan)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MaHost          *host;

    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    if (handler->flags & HTTP_STAGE_VIRTUAL) {
        return 1;
    }
    if (trans->filename == 0) {
        trans->filename = makeFilename(conn, rec->alias, rec->pathInfo, 1);
    }
    rec->dir = maLookupBestDir(host, trans->filename);
    if (rec->dir == 0) {
        httpError(conn, HTTP_CODE_NOT_FOUND, "Missing directory block for %s", trans->filename);
        return 0;
    }
    if (rec->location->auth->type == 0) {
        rec->auth = rec->dir->auth;
    }
    if (!trans->fileInfo.checked && mprGetPathInfo(conn, trans->filename, &trans->fileInfo) < 0) {
        mprAssert(handler);
        if (!(rec->flags & HTTP_PUT) && handler->flags & HTTP_STAGE_VERIFY_ENTITY) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Can't open document: %s", trans->filename);
            return 0;
        }
    }
    return 1;
}


//  MOB -- perhaps this should be in Http regardless

/*
    Manage requests to directories. This will either do an external redirect back to the browser or do an internal 
    (transparent) redirection and serve different content back to the browser. This routine may modify the requested 
    URI and/or the request handler.
 */
static void processDirectory(HttpConn *conn, bool *rescan)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprPath         *info;
    MaHost          *host;
    char            *path, *index;
    int             len, sep;

    rec = conn->receiver;
    trans = conn->transmitter;
    info = &trans->fileInfo;
    host = httpGetConnHost(conn);

    mprAssert(info->isDir);
    index = rec->dir->indexName;
    if (rec->pathInfo[strlen(rec->pathInfo) - 1] == '/') {
        /*  
            Internal directory redirections
         */
        //  TODO - is this really required?
        len = (int) strlen(trans->filename);
        sep = mprGetPathSeparator(trans, trans->filename);
        if (trans->filename[len - 1] == sep) {
            trans->filename[len - 1] = '\0';
        }
        path = mprJoinPath(trans, trans->filename, index);
        if (mprPathExists(trans, path, R_OK)) {
            /*  
                Index file exists, so do an internal redirect to it. Client will not be aware of this happening.
                Must rematch the handler on return.
             */
            httpSetUri(conn, addIndexToUrl(conn, index));
            trans->filename = path;
            mprGetPathInfo(conn, trans->filename, &trans->fileInfo);
            trans->extension = getExtension(conn);
#if UNUSED
            if ((trans->mimeType = (char*) maLookupMimeType(host, trans->extension)) == 0) {
                trans->mimeType = (char*) "text/html";
            }
#endif
            *rescan = 1;
        } else {
            mprFree(path);
        }
        return;
    }

    /*  
        External redirect. Ask the client to re-issue a request for a new location. See if an index exists and if so, 
        construct a new location for the index. If the index can't be accessed, just append a "/" to the URI and redirect.
     */
    if (rec->parsedUri->query && rec->parsedUri->query[0]) {
        path = mprAsprintf(trans, -1, "%s/%s?%s", rec->pathInfo, index, rec->parsedUri->query);
    } else {
        path = mprJoinPath(trans, rec->pathInfo, index);
    }
    if (!mprPathExists(trans, path, R_OK)) {
        path = mprStrcat(trans, -1, rec->pathInfo, "/", NULL);
    }
    httpRedirect(conn, HTTP_CODE_MOVED_PERMANENTLY, path);
}


static bool fileExists(MprCtx ctx, cchar *path) {
    if (mprPathExists(ctx, path, R_OK)) {
        return 1;
    }
#if BLD_WIN_LIKE
{
    char    *file;
    file = mprStrcat(ctx, -1, path, ".exe", NULL);
    if (mprPathExists(ctx, file, R_OK)) {
        return 1;
    }
    file = mprStrcat(ctx, -1, path, ".bat", NULL);
    if (mprPathExists(ctx, file, R_OK)) {
        return 1;
    }
}
#endif
    return 0;
}


//  MOB -- perhaps this should be in Http regardless

/*  
    Set the pathInfo (PATH_INFO) and update the request uri. This may set the response filename if convenient.
 */
static void setPathInfo(HttpConn *conn, HttpStage *handler)
{
    MaAlias         *alias;
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    char            *last, *start, *cp, *pathInfo, *scriptName;
    int             found, sep;

    rec = conn->receiver;
    trans = conn->transmitter;
    alias = rec->alias;
    mprAssert(handler);

    if (!(handler && handler->flags & HTTP_STAGE_PATH_INFO)) {
        return;
    }
    /*  
        Find the longest subset of the filename that matches a real file. Test each segment to see if 
        it corresponds to a real physical file. This also defines a new response filename after trimming the 
        extra path info.
     */
    last = 0;
    scriptName = rec->scriptName = rec->pathInfo;
    trans->filename = makeFilename(conn, alias, rec->pathInfo, 1);
    sep = mprGetPathSeparator(trans, trans->filename);
    for (cp = start = &trans->filename[strlen(alias->filename)]; cp; ) {
        if ((cp = strchr(cp, sep)) != 0) {
            *cp = '\0';
        }
        found = fileExists(conn, trans->filename);
        if (cp) {
            *cp = sep;
        }
        if (found) {
            if (cp) {
                last = cp++;
            } else {
                last = &trans->filename[strlen(trans->filename)];
                break;
            }
        } else {
            break;
        }
    }
    if (last) {
        pathInfo = &scriptName[alias->prefixLen + last - start];
        rec->pathInfo = mprStrdup(rec, pathInfo);
        *last = '\0';
        pathInfo[0] = '\0';
        if (rec->pathInfo[0]) {
            rec->pathTranslated = makeFilename(conn, alias, rec->pathInfo, 0);
        }
    }
}


//  MOB -- perhaps this should be in Http regardless

/*
    Prepare to run the request. Set various transmitter fields and create env vars from the query
 */
static void prepRequest(HttpConn *conn, HttpStage *handler)
{
    HttpReceiver    *rec;
    HttpTransmitter *trans;
    MprPath         *info;
    MaHost          *host;

    rec = conn->receiver;
    trans = conn->transmitter;
    host = httpGetConnHost(conn);

    setPathInfo(conn, handler);

    if (trans->extension == 0) {
        trans->extension = getExtension(conn);
    }
#if UNUSED
    if ((trans->mimeType = (char*) maLookupMimeType(host, trans->extension)) == 0) {
        //  TODO - set a header
        trans->mimeType = (char*) "text/html";
    }
#endif
    if (trans->filename) {
#if UNUSED
        if (trans->filename == 0) {
            trans->filename = makeFilename(conn, rec->alias, rec->pathInfo, 1);
        }
#endif
        /*
            Define an Etag for physical entities. Redo the file info if not valid now that extra path has been removed.
         */
        info = &trans->fileInfo;
        if (!info->checked) {
            mprGetPathInfo(conn, trans->filename, info);
        }
        if (info->valid) {
            //  TODO - set a header here
            trans->etag = mprAsprintf(trans, -1, "%x-%Lx-%Lx", info->inode, info->size, info->mtime);
        }
    }
}


static char *addIndexToUrl(HttpConn *conn, cchar *index)
{
    HttpReceiver    *rec;
    char            *path;

    rec = conn->receiver;
    path = mprJoinPath(rec, rec->pathInfo, index);
    if (rec->parsedUri->query && rec->parsedUri->query[0]) {
        return mprReallocStrcat(rec, -1, path, "?", rec->parsedUri->query, NULL);
    }
    return path;
}


static HttpStage *checkStage(HttpConn *conn, HttpStage *stage)
{
    HttpReceiver   *rec;

    rec = conn->receiver;
    if (stage == 0) {
        return 0;
    }
    if ((stage->flags & HTTP_STAGE_ALL & rec->flags) == 0) {
        return 0;
    }
    if (stage->match && !stage->match(conn, stage)) {
        return 0;
    }
    return stage;
}


//  MOB
int maRewriteUri(HttpConn *conn) { return 0; }

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
/************************************************************************/
/*
 *  End of file "../src/match.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/mime.c"
 */
/************************************************************************/

/* 
    mime.c - Mime type handling

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




int maOpenMimeTypes(MaHost *host, cchar *path)
{
    MprFile     *file;
    char        buf[80], *tok, *ext, *type;
    int         line;

    host->mimeFile = mprStrdup(host, path);

    if (host->mimeTypes == 0) {
        host->mimeTypes = mprCreateHash(host, HTTP_LARGE_HASH_SIZE);
    }
    file = mprOpen(host, path, O_RDONLY | O_TEXT, 0);
    if (file == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    line = 0;
    while (mprGets(file, buf, sizeof(buf)) != 0) {
        line++;
        if (buf[0] == '#' || isspace((int) buf[0])) {
            continue;
        }
        type = mprStrTok(buf, " \t\n\r", &tok);
        ext = mprStrTok(0, " \t\n\r", &tok);
        if (type == 0 || ext == 0) {
            mprError(host, "Bad mime spec in %s at line %d", path, line);
            continue;
        }
        while (ext) {
            maAddMimeType(host, ext, type);
            ext = mprStrTok(0, " \t\n\r", &tok);
        }
    }
    mprFree(file);
    return 0;
}


/*
    Add a mime type to the mime lookup table. Action Programs are added separately.
 */
MaMimeType *maAddMimeType(MaHost *host, cchar *ext, cchar *mimeType)
{
    MaMimeType  *mime;

    mime = mprAllocObjZeroed(host->mimeTypes, MaMimeType);
    if (mime == 0) {
        return 0;
    }
    mime->type = mprStrdup(host, mimeType);
    if (host->mimeTypes == 0) {
        host->mimeTypes = mprCreateHash(host, HTTP_LARGE_HASH_SIZE);
    }
    if (*ext == '.') {
        ext++;
    }
    mprAddHash(host->mimeTypes, ext, mime);
    return mime;
}


int maSetMimeActionProgram(MaHost *host, cchar *mimeType, cchar *actionProgram)
{
    MaMimeType      *mime;
    MprHash         *hp;
    
    if (host->mimeTypes == 0) {
        host->mimeTypes = mprCreateHash(host, HTTP_LARGE_HASH_SIZE);
        maAddStandardMimeTypes(host);
    }
    hp = 0;
    mime = 0;
    while ((hp = mprGetNextHash(host->mimeTypes, hp)) != 0) {
        mime = (MaMimeType*) hp->data;
        if (mime->type[0] == mimeType[0] && strcmp(mime->type, mimeType) == 0) {
            break;
        }
    }
    if (mime == 0) {
        mprError(host, "Can't find mime type %s for action program %s", mimeType, actionProgram);
        return MPR_ERR_NOT_FOUND;
    }
    mprFree(mime->actionProgram);
    mime->actionProgram = mprStrdup(host, actionProgram);
    return 0;
}


cchar *maGetMimeActionProgram(MaHost *host, cchar *mimeType)
{
    MaMimeType      *mime;

    if (mimeType == 0 || *mimeType == '\0') {
        return 0;
    }
    mime = (MaMimeType*) mprLookupHash(host->mimeTypes, mimeType);
    if (mime == 0) {
        return 0;
    }
    return mime->actionProgram;
}


cchar *maLookupMimeType(MaHost *host, cchar *ext)
{
    MaMimeType      *mime;

    if (ext == 0 || *ext == '\0') {
        return 0;
    }
    mime = (MaMimeType*) mprLookupHash(host->mimeTypes, ext);
    if (mime == 0) {
        return 0;
    }
    return mime->type;
}


void maAddStandardMimeTypes(MaHost *host)
{
    maAddMimeType(host, "ai", "application/postscript");
    maAddMimeType(host, "asc", "text/plain");
    maAddMimeType(host, "au", "audio/basic");
    maAddMimeType(host, "avi", "video/x-msvideo");
    maAddMimeType(host, "bin", "application/octet-stream");
    maAddMimeType(host, "bmp", "image/bmp");
    maAddMimeType(host, "class", "application/octet-stream");
    maAddMimeType(host, "css", "text/css");
    maAddMimeType(host, "dll", "application/octet-stream");
    maAddMimeType(host, "doc", "application/msword");
    maAddMimeType(host, "ejs", "text/html");
    maAddMimeType(host, "eps", "application/postscript");
    maAddMimeType(host, "es", "application/x-javascript");
    maAddMimeType(host, "exe", "application/octet-stream");
    maAddMimeType(host, "gif", "image/gif");
    maAddMimeType(host, "gz", "application/x-gzip");
    maAddMimeType(host, "htm", "text/html");
    maAddMimeType(host, "html", "text/html");
    maAddMimeType(host, "ico", "image/x-icon");
    maAddMimeType(host, "jar", "application/octet-stream");
    maAddMimeType(host, "jpeg", "image/jpeg");
    maAddMimeType(host, "jpg", "image/jpeg");
    maAddMimeType(host, "js", "application/javascript");
    maAddMimeType(host, "mp3", "audio/mpeg");
    maAddMimeType(host, "pdf", "application/pdf");
    maAddMimeType(host, "png", "image/png");
    maAddMimeType(host, "ppt", "application/vnd.ms-powerpoint");
    maAddMimeType(host, "ps", "application/postscript");
    maAddMimeType(host, "ra", "audio/x-realaudio");
    maAddMimeType(host, "ram", "audio/x-pn-realaudio");
    maAddMimeType(host, "rmm", "audio/x-pn-realaudio");
    maAddMimeType(host, "rtf", "text/rtf");
    maAddMimeType(host, "rv", "video/vnd.rn-realvideo");
    maAddMimeType(host, "so", "application/octet-stream");
    maAddMimeType(host, "swf", "application/x-shockwave-flash");
    maAddMimeType(host, "tar", "application/x-tar");
    maAddMimeType(host, "tgz", "application/x-gzip");
    maAddMimeType(host, "tiff", "image/tiff");
    maAddMimeType(host, "txt", "text/plain");
    maAddMimeType(host, "wav", "audio/x-wav");
    maAddMimeType(host, "xls", "application/vnd.ms-excel");
    maAddMimeType(host, "zip", "application/zip");

    maAddMimeType(host, "php", "application/x-appweb-php");
    maAddMimeType(host, "pl", "application/x-appweb-perl");
    maAddMimeType(host, "py", "application/x-appweb-python");
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
/************************************************************************/
/*
 *  End of file "../src/mime.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/misc.c"
 */
/************************************************************************/

/*
    misc.c -- Bits and pieces.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




#if BLD_UNIX_LIKE
static int allDigits(cchar *s);
#endif


void maGetUserGroup(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;
    struct group    *gp;

    appweb->uid = getuid();
    if ((pp = getpwuid(appweb->uid)) == 0) {
        mprError(appweb, "Can't read user credentials: %d. Check your /etc/passwd file.", appweb->uid);
    } else {
        appweb->user = mprStrdup(appweb, pp->pw_name);
    }
    appweb->gid = getgid();
    if ((gp = getgrgid(appweb->gid)) == 0) {
        mprError(appweb, "Can't read group credentials: %d. Check your /etc/group file", appweb->gid);
    } else {
        appweb->group = mprStrdup(appweb, gp->gr_name);
    }
#else
    appweb->uid = appweb->gid = -1;
#endif
}


int maSetHttpUser(MaAppweb *appweb, cchar *newUser)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;

    if (allDigits(newUser)) {
        appweb->uid = atoi(newUser);
        if ((pp = getpwuid(appweb->uid)) == 0) {
            mprError(appweb, "Bad user id: %d", appweb->uid);
            return MPR_ERR_CANT_ACCESS;
        }
        newUser = pp->pw_name;

    } else {
        if ((pp = getpwnam(newUser)) == 0) {
            mprError(appweb, "Bad user name: %s", newUser);
            return MPR_ERR_CANT_ACCESS;
        }
        appweb->uid = pp->pw_uid;
    }
#endif
    mprFree(appweb->user);
    appweb->user = mprStrdup(appweb, newUser);
    return 0;
}


int maSetHttpGroup(MaAppweb *appweb, cchar *newGroup)
{
#if BLD_UNIX_LIKE
    struct group    *gp;

    if (allDigits(newGroup)) {
        appweb->gid = atoi(newGroup);
        if ((gp = getgrgid(appweb->gid)) == 0) {
            mprError(appweb, "Bad group id: %d", appweb->gid);
            return MPR_ERR_CANT_ACCESS;
        }
        newGroup = gp->gr_name;

    } else {
        if ((gp = getgrnam(newGroup)) == 0) {
            mprError(appweb, "Bad group name: %s", newGroup);
            return MPR_ERR_CANT_ACCESS;
        }
        appweb->gid = gp->gr_gid;
    }
#endif
    mprFree(appweb->group);
    appweb->group = mprStrdup(appweb, newGroup);
    return 0;
}


int maApplyChangedUser(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    if (appweb->uid >= 0) {
        if ((setuid(appweb->uid)) != 0) {
            mprError(appweb, "Can't change user to: %s: %d\n"
                "WARNING: This is a major security exposure", appweb->user, appweb->uid);
#if LINUX && PR_SET_DUMPABLE
        } else {
            prctl(PR_SET_DUMPABLE, 1);
#endif
        }
    }
#endif
    return 0;
}


int maApplyChangedGroup(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    if (appweb->gid >= 0) {
        if (setgid(appweb->gid) != 0) {
            mprError(appweb, "Can't change group to %s: %d\n"
                "WARNING: This is a major security exposure", appweb->group, appweb->gid);
#if LINUX && PR_SET_DUMPABLE
        } else {
            prctl(PR_SET_DUMPABLE, 1);
#endif
        }
    }
#endif
    return 0;
}


/*
    Load a module. Returns 0 if the modules is successfully loaded either statically or dynamically.
 */
int maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname)
{
    MprModule   *module;
    char        entryPoint[MPR_MAX_FNAME];
    char        *path;

    if (strcmp(name, "authFilter") == 0 || strcmp(name, "rangeFilter") == 0 || strcmp(name, "uploadFilter") == 0 ||
            strcmp(name, "fileHandler") == 0 || strcmp(name, "egiHandler") == 0 || strcmp(name, "dirHandler") == 0) {
        mprError(appweb, "The %s module is now builtin. No need to use LoadModule", name);
        return 0;
    }
    if (libname == 0) {
        path = mprStrcat(appweb, -1, "mod_", name, BLD_SHOBJ, NULL);
    } else {
        path = mprStrdup(appweb, libname);
    }
    module = mprLookupModule(appweb, path);
    if (module) {
        mprLog(appweb, MPR_CONFIG, "Activating module (Builtin) %s", name);
        mprFree(path);
        return 0;
    }
    mprSprintf(entryPoint, sizeof(entryPoint), "ma%sInit", name);
    entryPoint[2] = toupper((int) entryPoint[2]);

    if (mprLoadModule(appweb->http, path, entryPoint, NULL) == 0) {
        mprFree(path);
        return MPR_ERR_CANT_CREATE;
    }
    mprFree(path);
    mprLog(appweb, MPR_CONFIG, "Activating module (Loadable) %s", name);
    return 0;
}



#if BLD_UNIX_LIKE
static int allDigits(cchar *s)
{
    return strspn(s, "1234567890") == strlen(s);
}
#endif

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
/************************************************************************/
/*
 *  End of file "../src/misc.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/modules/sslModule.c"
 */
/************************************************************************/

/*
    sslModule.c - Module for SSL support
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#include    "mprSsl.h"

#if BLD_FEATURE_SSL

static int parseSsl(Http *http, cchar *key, char *value, MaConfigState *state)
{
    HttpLocation  *location;
    MaServer    *server;
    MaHost      *host;
    char        *path, prefix[MPR_MAX_FNAME];
    char        *tok, *word, *enable, *provider;
    int         protoMask, mask;
    static int  hasBeenWarned = 0;

    host = state->host;
    server = state->server;
    location = state->location;

    mprStrcpy(prefix, sizeof(prefix), key);
    prefix[3] = '\0';

    if (mprStrcmpAnyCase(prefix, "SSL") != 0) {
        return 0;
    }
    if (!mprHasSecureSockets(http)) {
        if (!hasBeenWarned++) {
            mprError(http, "Missing an SSL Provider");
        }
        return 0;
        /* return MPR_ERR_BAD_SYNTAX; */
    }
    if (location->ssl == 0) {
        location->ssl = mprCreateSsl(location);
    }
    if (mprStrcmpAnyCase(key, "SSLEngine") == 0) {
        enable = mprStrTok(value, " \t", &tok);
        provider = mprStrTok(0, " \t", &tok);
        if (mprStrcmpAnyCase(value, "on") == 0) {
            maSecureHost(host, location->ssl);
        }
        return 1;
    }

    path = maMakePath(host, mprStrTrim(value, "\""));

    if (mprStrcmpAnyCase(key, "SSLCACertificatePath") == 0) {
        mprSetSslCaPath(location->ssl, path);
        mprFree(path);
        return 1;

    } else if (mprStrcmpAnyCase(key, "SSLCACertificateFile") == 0) {
        mprSetSslCaFile(location->ssl, path);
        mprFree(path);
        return 1;

    } else if (mprStrcmpAnyCase(key, "SSLCertificateFile") == 0) {
        mprSetSslCertFile(location->ssl, path);
        mprFree(path);
        return 1;

    } else if (mprStrcmpAnyCase(key, "SSLCertificateKeyFile") == 0) {
        mprSetSslKeyFile(location->ssl, path);
        mprFree(path);
        return 1;

    } else if (mprStrcmpAnyCase(key, "SSLCipherSuite") == 0) {
        mprSetSslCiphers(location->ssl, value);
        mprFree(path);
        return 1;

    } else if (mprStrcmpAnyCase(key, "SSLVerifyClient") == 0) {
        mprFree(path);
        if (mprStrcmpAnyCase(value, "require") == 0) {
            mprVerifySslClients(location->ssl, 1);

        } else if (mprStrcmpAnyCase(value, "none") == 0) {
            mprVerifySslClients(location->ssl, 0);

        } else {
            return -1;
        }
        return 1;

    } else if (mprStrcmpAnyCase(key, "SSLProtocol") == 0) {
        mprFree(path);
        protoMask = 0;
        word = mprStrTok(value, " \t", &tok);
        while (word) {
            mask = -1;
            if (*word == '-') {
                word++;
                mask = 0;
            } else if (*word == '+') {
                word++;
            }
            if (mprStrcmpAnyCase(word, "SSLv2") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV2 & ~mask);
                protoMask |= (MPR_PROTO_SSLV2 & mask);

            } else if (mprStrcmpAnyCase(word, "SSLv3") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV3 & ~mask);
                protoMask |= (MPR_PROTO_SSLV3 & mask);

            } else if (mprStrcmpAnyCase(word, "TLSv1") == 0) {
                protoMask &= ~(MPR_PROTO_TLSV1 & ~mask);
                protoMask |= (MPR_PROTO_TLSV1 & mask);

            } else if (mprStrcmpAnyCase(word, "ALL") == 0) {
                protoMask &= ~(MPR_PROTO_ALL & ~mask);
                protoMask |= (MPR_PROTO_ALL & mask);
            }
            word = mprStrTok(0, " \t", &tok);
        }
        mprSetSslProtocols(location->ssl, protoMask);
        return 1;
    }
    mprFree(path);
    return 0;
}


/*
    Loadable module initialization. 
 */
int maSslModuleInit(Http *http, MprModule *mp)
{
    HttpStage     *stage;

    if (mprLoadSsl(http, 1) == 0) {
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
/************************************************************************/
/*
 *  End of file "../src/modules/sslModule.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../src/server.c"
 */
/************************************************************************/

/*
    server.c -- Manage a meta-server with one or more virtual hosts.

    A meta-server supports multiple endpoints and one or more (virtual) hosts.
    Meta Servers may be configured manually or via an "appweb.conf" configuration  file.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */




static int appwebDestructor(MaAppweb *appweb);

/*
    Create the top level appweb control object. This is typically a singleton.
 */
MaAppweb *maCreateAppweb(MprCtx ctx)
{
    MaAppweb    *appweb;
    Http        *http;

    appweb = mprAllocObjWithDestructorZeroed(ctx, MaAppweb, appwebDestructor);
    if (appweb == 0) {
        return 0;
    }
    appweb->servers = mprCreateList(appweb);
    appweb->http = http = httpCreate(appweb);
    httpSetContext(http, appweb);
    maGetUserGroup(appweb);
    maOpenDirHandler(http);
    maOpenEgiHandler(http);
    maOpenFileHandler(http);
    maOpenSendConnector(http);
    return appweb;
}


static int appwebDestructor(MaAppweb *appweb)
{
    /* TODO - should this call stop? */
    maUnloadStaticModules(appweb);
    return 0;
}


void maAddServer(MaAppweb *appweb, MaServer *server)
{
    mprAddItem(appweb->servers, server);
}


void maSetDefaultServer(MaAppweb *appweb, MaServer *server)
{
    appweb->defaultServer = server;
}


//  TODO - standardize on Find, Lookup, Get
MaServer *maLookupServer(MaAppweb *appweb, cchar *name)
{
    MaServer    *server;
    int         next;

    for (next = 0; (server = mprGetNextItem(appweb->servers, &next)) != 0; ) {
        if (strcmp(server->name, name) == 0) {
            return server;
        }
    }
    return 0;
}


int maStartAppweb(MaAppweb *appweb)
{
    MaServer    *server;
    int         next;

    for (next = 0; (server = mprGetNextItem(appweb->servers, &next)) != 0; ) {
        if (maStartServer(server) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    return 0;
}


int maStopAppweb(MaAppweb *appweb)
{
    MaServer    *server;
    int         next;

    for (next = 0; (server = mprGetNextItem(appweb->servers, &next)) != 0; ) {
        maStopServer(server);
    }
    return 0;
}


/*  
    Create a new meta-server. This may manage may virtual hosts. If ip and port are supplied,
    create a HttpServer now on that endpoint. Otherwise, call maConfigureServer later.
 */
MaServer *maCreateServer(MaAppweb *appweb, cchar *name, cchar *root, cchar *ip, int port)
{
    MaServer    *server;
    static int  staticModulesLoaded = 0;

    mprAssert(appweb);
    mprAssert(name && *name);

    server = mprAllocObjZeroed(appweb, MaServer);
    if (server == 0) {
        return 0;
    }
    server->hosts = mprCreateList(server);
    server->httpServers = mprCreateList(server);
    server->hostAddresses = mprCreateList(server);
    server->name = mprStrdup(server, name);
    server->appweb = appweb;
    server->http = appweb->http;

    maAddServer(appweb, server);
    maSetServerRoot(server, root);

    if (ip && port > 0) {
        maAddHttpServer(server, httpCreateServer(appweb->http, ip, port, NULL));
        mprAddItem(server->hostAddresses, maCreateHostAddress(server, ip, port));
    }
    maSetDefaultServer(appweb, server);

    if (!staticModulesLoaded) {
        staticModulesLoaded = 1;
        maLoadStaticModules(appweb);
    }
    return server;
}


int maStartServer(MaServer *server)
{
    MaHost      *host;
    HttpServer  *httpServer;
    int         next, count, warned;

    /*  
        Start the hosts
     */
    for (next = 0; (host = mprGetNextItem(server->hosts, &next)) != 0; ) {
        mprLog(server, 1, "Starting host named: \"%s\"", host->name);
        if (maStartHost(host) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
    }

    /*  
        Start the Http servers and being listening for requests
     */
    warned = 0;
    count = 0;
    for (next = 0; (httpServer = mprGetNextItem(server->httpServers, &next)) != 0; ) {
        httpSetMetaServer(httpServer, server);
        /*
            Define a server notifier. This will be inherited by all accepted connections on the server
         */
        httpSetServerNotifier(httpServer, (HttpNotifier) maNotifyServerStateChange);
        if (httpStartServer(httpServer) < 0) {
            warned++;
            break;
        } else {
            count++;
        }
    }
    if (count == 0) {
        if (!warned) {
            mprError(server, "Server is not listening on any addresses");
        }
        return MPR_ERR_CANT_OPEN;
    }
    if (maApplyChangedGroup(server->appweb) < 0 || maApplyChangedUser(server->appweb) < 0) {
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}


int maStopServer(MaServer *server)
{
    MaHost      *host;
    HttpServer  *httpServer;
    int         next;

    for (next = 0; (host = mprGetNextItem(server->hosts, &next)) != 0; ) {
        maStopHost(host);
    }
    for (next = 0; (httpServer = mprGetNextItem(server->httpServers, &next)) != 0; ) {
        httpStopServer(httpServer);
    }
    return 0;
}


void maAddHost(MaServer *server, MaHost *host)
{
    mprAddItem(server->hosts, host);
}


void maAddHttpServer(MaServer *server, HttpServer *httpServer)
{
    mprAddItem(server->httpServers, httpServer);
}


MaHost *maLookupHost(MaServer *server, cchar *name)
{
    MaHost  *host;
    int     next;

    for (next = 0; (host = mprGetNextItem(server->hosts, &next)) != 0; ) {
        if (strcmp(host->name, name) == 0) {
            return host;
        }
    }
    return 0;
}


/*  
    Lookup a host address. If ipAddr is null or port is -1, then those elements are wild.
 */
MaHostAddress *maLookupHostAddress(MaServer *server, cchar *ip, int port)
{
    MaHostAddress   *address;
    int             next;

    for (next = 0; (address = mprGetNextItem(server->hostAddresses, &next)) != 0; ) {
        if (address->port < 0 || port < 0 || address->port == port) {
            if (ip == 0 || address->ip == 0 || strcmp(address->ip, ip) == 0) {
                return address;
            }
        }
    }
    return 0;
}


/*  
    Create the host addresses for a host. Called for hosts or for NameVirtualHost directives (host == 0).
 */
int maCreateHostAddresses(MaServer *server, MaHost *host, cchar *configValue)
{
    HttpServer      *httpServer;
    MaHostAddress   *address;
    char            *ipAddrPort, *ip, *value, *tok;
    char            addrBuf[MPR_MAX_IP_ADDR_PORT];
    int             next, port;

    address = 0;
    value = mprStrdup(server, configValue);
    ipAddrPort = mprStrTok(value, " \t", &tok);

    while (ipAddrPort) {
        if (mprStrcmpAnyCase(ipAddrPort, "_default_") == 0) {
            //  TODO is this used?
            mprAssert(0);
            ipAddrPort = "*:*";
        }

        if (mprParseIp(server, ipAddrPort, &ip, &port, -1) < 0) {
            mprError(server, "Can't parse ipAddr %s", ipAddrPort);
            continue;
        }
        mprAssert(ip && *ip);
        if (ip[0] == '*') {
            ip = mprStrdup(server, "");
        }

        /*  
            For each listening endpiont,
         */
        for (next = 0; (httpServer = mprGetNextItem(server->httpServers, &next)) != 0; ) {
            if (port > 0 && port != httpServer->port) {
                continue;
            }
            if (httpServer->ip[0] != '\0' && ip[0] != '\0' && strcmp(ip, httpServer->ip) != 0) {
                continue;
            }

            /*
                Find the matching host address or create a new one
             */
            if ((address = maLookupHostAddress(server, httpServer->ip, httpServer->port)) == 0) {
                address = maCreateHostAddress(server, httpServer->ip, httpServer->port);
                mprAddItem(server->hostAddresses, address);
            }

            /*
                If a host is specified
             */
            if (host == 0) {
                maSetNamedVirtualHostAddress(address);

            } else {
                maInsertVirtualHost(address, host);
                if (httpServer->ip[0] != '\0') {
                    mprSprintf(addrBuf, sizeof(addrBuf), "%s:%d", httpServer->ip, httpServer->port);
                } else {
                    mprSprintf(addrBuf, sizeof(addrBuf), "%s:%d", ip, httpServer->port);
                }
                maSetHostName(host, addrBuf);
            }
        }
        mprFree(ip);
        ipAddrPort = mprStrTok(0, " \t", &tok);
    }

    if (host) {
        if (address == 0) {
            mprError(server, "No valid IP address for host %s", host->name);
            mprFree(value);
            return MPR_ERR_CANT_INITIALIZE;
        }
        if (maIsNamedVirtualHostAddress(address)) {
            maSetNamedVirtualHost(host);
        }
    }
    mprFree(value);
    return 0;
}


/*  
    Set the Server Root directory. We convert path into an absolute path.
 */
void maSetServerRoot(MaServer *server, cchar *path)
{
    if (path == 0 || BLD_FEATURE_ROMFS) {
        path = ".";
    }
#if !VXWORKS
    /*
        VxWorks stat() is broken if using a network FTP server.
     */
    if (! mprPathExists(server, path, R_OK)) {
        mprError(server, "Can't access ServerRoot directory %s", path);
        return;
    }
#endif
    mprFree(server->serverRoot);
    server->serverRoot = mprGetAbsPath(server, path);
}


void maSetDefaultHost(MaServer *server, MaHost *host)
{
    server->defaultHost = host;
}


void maSetKeepAliveTimeout(MaAppweb *appweb, int timeout)
{
    appweb->http->keepAliveTimeout = timeout;
}


void maSetMaxKeepAlive(MaAppweb *appweb, int timeout)
{
    appweb->http->maxKeepAlive = timeout;
}


/*  
    Set the default request timeout. This is the maximum time a request can run.
    Not to be confused with the session timeout or the keep alive timeout.
 */
void maSetTimeout(MaAppweb *appweb, int timeout)
{
    appweb->http->timeout = timeout;
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
/************************************************************************/
/*
 *  End of file "../src/server.c"
 */
/************************************************************************/

