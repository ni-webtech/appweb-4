/**
    config.c - Parse the configuration file.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ****************************/

static bool conditionalDefinition(char *key);
static MaConfigState *pushState(MaConfigState *state, int *top);
static int processSetting(MaMeta *server, char *key, char *value, MaConfigState *state);

#if BLD_FEATURE_CONFIG_SAVE
static void tabs(int fd, int indent);
static void printAuth(int fd, HttpHost *host, HttpAuth *auth, int indent);
#endif

/******************************************************************************/
/*  
    Configure the meta-server. If the configFile is defined, use it. If not, then consider serverRoot, docRoot, ip and port.
 */
int maConfigureMeta(MaMeta *meta, cchar *configFile, cchar *serverRoot, cchar *docRoot, cchar *ip, int port)
{
    MaAppweb        *appweb;
    Http            *http;
    HttpServer      *server;
    HttpHost        *host;
    HttpAlias       *alias;
    HttpLoc         *loc, *cloc;
    char            *path, *searchPath, *dir;

    appweb = meta->appweb;
    http = appweb->http;
    dir = mprGetAppDir();

    if (configFile) {
        path = mprGetAbsPath(configFile);
        if (maParseConfig(meta, path) < 0) {
            /* mprUserError("Can't configure meta-server using %s", path); */
            return MPR_ERR_CANT_INITIALIZE;
        }
        return 0;

    } else {
        mprLog(2, "DocumentRoot %s", docRoot);
        if ((server = httpCreateConfiguredServer(docRoot, ip, port)) == 0) {
            return MPR_ERR_CANT_OPEN;
        }
        maAddServer(meta, server);
        host = meta->defaultHost = mprGetFirstItem(server->hosts);
        mprAssert(host);

        loc = host->loc;
#if WIN
        searchPath = mprAsprintf("%s" MPR_SEARCH_SEP ".", dir);
#else
{
        char *libDir = mprJoinPath(mprGetPathParent(dir), BLD_LIB_NAME);
        searchPath = mprAsprintf("%s" MPR_SEARCH_SEP "%s" MPR_SEARCH_SEP ".", dir,
            mprSamePath(BLD_BIN_PREFIX, dir) ? BLD_LIB_PREFIX: libDir);
}
#endif
        mprSetModuleSearchPath(searchPath);

#if UNUSED
        httpSetConnector(loc, "netConnector");

        /*
            Auth must be added first to authorize all requests. File is last as a catch all.
         */
        if (httpLookupStage(http, "authFilter")) {
            httpAddHandler(loc, "authFilter", "");
            httpAddFilter(loc, http->rangeFilter->name, NULL, HTTP_STAGE_TX);
            httpAddFilter(loc, http->chunkFilter->name, NULL, HTTP_STAGE_RX | HTTP_STAGE_TX);
            httpAddFilter(loc, "uploadFilter", NULL, HTTP_STAGE_RX);
            httpAddFilter(loc, "authFilter", NULL, HTTP_STAGE_RX);
        }
#endif
        maLoadModule(appweb, "cgiHandler", "mod_cgi");
        if (httpLookupStage(http, "cgiHandler")) {
            httpAddHandler(loc, "cgiHandler", ".cgi .cgi-nph .bat .cmd .pl .py");
            /*
                Add cgi-bin with a loc block for the /cgi-bin URL prefix.
             */
            path = "cgi-bin";
            if (mprPathExists(path, X_OK)) {
                alias = httpCreateAlias("/cgi-bin/", path, 0);
                mprLog(4, "ScriptAlias \"/cgi-bin/\":\"%s\"", path);
                httpAddAlias(host, alias);
                cloc = httpCreateInheritedLocation(host->loc);
                httpSetLocationHost(cloc, host);
                httpSetLocationPrefix(cloc, "/cgi-bin/");
                httpSetHandler(cloc, "cgiHandler");
                httpAddLocation(host, cloc);
            }
        }
        maLoadModule(appweb, "ejsHandler", "mod_ejs");
        if (httpLookupStage(http, "ejsHandler")) {
            httpAddHandler(loc, "ejsHandler", ".ejs");
#if UNUSED
            httpSetLocationScript(loc, "start.es");
#endif
        }
        maLoadModule(appweb, "phpHandler", "mod_php");
        if (httpLookupStage(http, "phpHandler")) {
            httpAddHandler(loc, "phpHandler", ".php");
        }
        if (httpLookupStage(http, "fileHandler")) {
            httpAddHandler(loc, "fileHandler", "");
        }
    }
    if (serverRoot) {
        maSetMetaRoot(meta, serverRoot);
    }
    if (ip || port > 0) {
        maSetMetaAddress(meta, ip, port);
    }
    return 0;
}


int maParseConfig(MaMeta *meta, cchar *configFile)
{
    MaAppweb        *appweb;
    HttpHost        *defaultHost;
    Http            *http;
    HttpDir         *dir;
    HttpHost        *host;
    HttpServer      *server;
    MprDirEntry     *dp;
    MprList         *includes;
    MaConfigState   stack[MA_MAX_CONFIG_DEPTH], *state;
    char            *buf, *cp, *tok, *key, *value, *path, *ip;
    int             i, rc, top, next, len, port;

    mprLog(2, "Config File %s", configFile);

    appweb = meta->appweb;
    http = appweb->http;
    memset(stack, 0, sizeof(stack));
    meta->alreadyLogging = mprGetLogHandler() ? 1 : 0;

    defaultHost = host = httpCreateHost(0);
    httpSetHostName(host, "default", -1);
    httpSetHostServerRoot(host, meta->serverRoot);
    meta->defaultHost = defaultHost;

    top = 0;
    state = &stack[top];
    state->meta = meta;
    state->host = host;
    state->dir = httpCreateBareDir(".");
    state->loc = defaultHost->loc;
    state->loc->connector = http->netConnector;
    state->enabled = 1;
    state->lineNumber = 0;

    state->filename = (char*) configFile;
    state->configDir = mprGetAbsPath(mprGetPathDir(state->filename));

    state->file = mprOpenFile(configFile, O_RDONLY | O_TEXT, 0444);
    if (state->file == 0) {
        mprError("Can't open %s for config directives", configFile);
        return MPR_ERR_CANT_OPEN;
    }

    /*
        Set the default location authorization definition to match the default directory auth
     */
    state->loc->auth = state->dir->auth;
    state->auth = state->dir->auth;
    httpAddDir(host, state->dir);

    /*
        Parse each line in the config file
     */
    for (state->lineNumber = 1; top >= 0; state->lineNumber++) {

        state = &stack[top];
        mprAssert(state->file);
        if ((buf = mprGetFileString(state->file, 0, NULL)) == 0) {
            if (--top > 0 && strcmp(state->filename, stack[top].filename) == 0) {
                mprError("Unclosed directives in %s", state->filename);
                while (top >= 0 && strcmp(state->filename, state[top].filename) == 0) {
                    top--;
                }
            }
            if (top >= 0 && state->file == stack[top].file) {
                stack[top].lineNumber = state->lineNumber + 1;
            } else {
                mprCloseFile(state->file);
                state->file = 0;
                if (top >= 0) {
                    state = &stack[top];
                }
            }
            continue;
        }
        cp = buf;
        while (isspace((int) *cp)) {
            cp++;
        }
        if (*cp == '\0' || *cp == '#') {
            continue;
        }
        cp = strim(cp, "\r\n\t ", MPR_TRIM_BOTH);
        key = stok(cp, " \t\n", &tok);
        value = stok(0, "\n", &tok);
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

        if (scasecmp(key, "Include") == 0) {
            state->lineNumber++;
            value = strim(value, "\"", MPR_TRIM_BOTH);
            value = stemplate(value, state->loc->tokens);
            if ((cp = strchr(value, '*')) == 0) {
                state = pushState(state, &top);
                state->lineNumber = 0;
                state->filename = mprJoinPath(meta->serverRoot, value);
                state->file = mprOpenFile(state->filename, O_RDONLY | O_TEXT, 0444);
                if (state->file == 0) {
                    mprError("Can't open include file %s for config directives", state->filename);
                    return MPR_ERR_CANT_OPEN;
                }
                mprLog(5, "Parsing config file: %s", state->filename);

            } else {
                /*
                    Process wild cards. This is very simple - only "*" is supported.
                 */
                *cp = '\0';
                len = (int) strlen(value);
                if (value[len - 1] == '/') {
                    value[len - 1] = '\0';
                }
                cp = mprJoinPath(meta->serverRoot, value);
                includes = mprGetPathFiles(cp, 0);
                if (includes == 0) {
                    continue;
                }
                value = mprJoinPath(meta->serverRoot, value);
                for (next = 0; (dp = mprGetNextItem(includes, &next)) != 0; ) {
                    state = pushState(state, &top);
                    state->filename = mprJoinPath(value, dp->name);
                    state->file = mprOpenFile(state->filename, O_RDONLY | O_TEXT, 0444);
                    if (state->file == 0) {
                        mprError("Can't open include file %s for config directives", state->filename);
                        return MPR_ERR_CANT_OPEN;
                    }
                    mprLog(5, "Parsing config file: %s", state->filename);
                }
            }
            continue;

        } else if (*key != '<') {

            if (!state->enabled) {
                mprLog(8, "Skipping key %s at %s:%d", key, state->filename, state->lineNumber);
                continue;
            }
            /*
                Keywords outside of a virtual host or directory section
             */
            rc = processSetting(meta, key, value, state);
            if (rc == 0) {
                char    *extraMsg;
                if (strcmp(key, "SSLEngine") == 0) {
                    extraMsg =
                        "\n\nFor SSL, you may have one SSL provider loaded.\n"
                        "Make sure that either OpenSSL or MatrixSSL is loaded.";
                } else {
                    extraMsg = "";
                }
                mprError("Ignoring unknown directive \"%s\"\nAt line %d in %s\n\n"
                    "Make sure the required modules are loaded. %s\n", key, state->lineNumber, state->filename, extraMsg);
                continue;

            } else if (rc < 0) {
                mprError("Ignoring bad directive \"%s\" at %s:%d in %s\n", key, state->filename, state->lineNumber, 
                    configFile);
            }
            continue;
        }

        mprLog(9, "AT %d, key %s", state->lineNumber, key);

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
                state = pushState(state, &top);
                mprLog(8, "Skipping key %s at %s:%d", key, state->filename, state->lineNumber);
                continue;
            }
            i = (int) strlen(value) - 1;
            if (value[i] == '>') {
                value[i] = '\0';
            }
            /*
                Opening tags
             */
            if (scasecmp(key, "If") == 0) {
                value = strim(value, "\"", MPR_TRIM_BOTH);

                /*
                    Want to be able to nest <if> directives.
                 */
                state = pushState(state, &top);
                state->enabled = conditionalDefinition(value);
                if (!state->enabled) {
                    mprLog(7, "If \"%s\" conditional is false at %s:%d", value, state->filename, state->lineNumber);
                }

            } else if (scasecmp(key, "VirtualHost") == 0) {
                value = strim(value, "\"", MPR_TRIM_BOTH);
                state = pushState(state, &top);
                mprParseIp(value, &ip, &port, -1);
                host = state->host = httpCloneHost(host);
                httpSetHostName(host, ip, port);
                state->loc = host->loc;
                state->auth = host->loc->auth;
                state->dir = httpCreateDir(stack[top - 1].dir->path, stack[top - 1].dir);
                state->auth = state->dir->auth;
                httpAddDir(host, state->dir);
                if ((server = httpLookupServer(http, ip, port)) == 0) {
                    mprError("Can't find listen directive for virtual host %s", value); 
                } else {
                    httpAddHostToServer(server, host);
                }

            } else if (scasecmp(key, "Directory") == 0) {
                path = httpMakePath(state->loc, strim(value, "\"", MPR_TRIM_BOTH));
                state = pushState(state, &top);

                if ((dir = httpLookupDir(host, path)) != 0) {
                    /*
                        Allow multiple occurences of the same directory. Append directives.
                     */  
                    state->dir = dir;

                } else {
                    /*
                        Create a new directory inherit and parent directory settings. This means inherit authorization from
                        the enclosing host.
                     */
                    state->dir = httpCreateDir(path, stack[top - 1].dir);
                    httpAddDir(host, state->dir);
                }
                state->auth = state->dir->auth;

            } else if (scasecmp(key, "Location") == 0) {
                value = strim(value, "\"", MPR_TRIM_BOTH);
                if (httpLookupLocation(host, value)) {
                    mprError("Location block already exists for \"%s\"", value);
                    goto err;
                }
                state = pushState(state, &top);
                state->loc = httpCreateInheritedLocation(state->loc);
                httpSetLocationHost(state->loc, host);
                state->auth = state->loc->auth;
                httpSetLocationPrefix(state->loc, value);
                if (httpAddLocation(host, state->loc) < 0) {
                    mprError("Can't add location %s", value);
                    goto err;
                }
                mprAssert(host->loc->prefix);
            }

        } else {

            stack[top - 1].lineNumber = state->lineNumber + 1;
            key++;

            /*
                Closing tags
             */
            if (state->enabled && state->loc != stack[top-1].loc) {
                httpFinalizeLocation(state->loc);
            }
            if (scasecmp(key, "If") == 0) {
                top--;
                host = stack[top].host;

            } else if (scasecmp(key, "VirtualHost") == 0) {
                top--;
                host = stack[top].host;

            } else if (scasecmp(key, "Directory") == 0) {
                top--;

            } else if (scasecmp(key, "Location") == 0) {
                top--;
            }
            if (top < 0) {
                goto syntaxErr;
            }
        }
    }
    if (!maValidateConfiguration(meta)) {
        return MPR_ERR_BAD_ARGS;
    }
    if (mprHasMemError()) {
        mprError("Memory allocation error when initializing");
        return MPR_ERR_MEMORY;
    }
    return 0;

syntaxErr:
    mprError("Syntax error in %s at %s:%d", configFile, state->filename, state->lineNumber);
    return MPR_ERR_BAD_SYNTAX;

err:
    mprError("Error in %s at %s:%d", configFile, state->filename, state->lineNumber);
    return MPR_ERR_BAD_SYNTAX;
}


int maValidateConfiguration(MaMeta *meta)
{
    MaAppweb        *appweb;
    Http            *http;
    HttpAlias       *alias;
    HttpDir         *bestDir;
    HttpHost        *host, *defaultHost;
    HttpServer      *server;
    char            *path;
    int             nextAlias, nextHost, nextServer;

    appweb = meta->appweb;
    http = appweb->http;
    defaultHost = meta->defaultHost;
    mprAssert(defaultHost);

    if (mprGetListLength(meta->servers) == 0) {
        mprError("Missing listening HttpServer. Must have a Listen directive.");
        return 0;
    }

    for (nextServer = 0; (server = mprGetNextItem(http->servers, &nextServer)) != 0; ) {
        if (mprGetListLength(server->hosts) == 0) {
            httpAddHostToServer(server, defaultHost);
            if (defaultHost->ip == 0) {
                httpSetHostName(defaultHost, server->ip, server->port);
            }
        }
    }

    for (nextHost = 0; (host = mprGetNextItem(http->hosts, &nextHost)) != 0; ) {
        mprAssert(host->name && host->name);
        if (host->documentRoot == 0) {
            httpSetHostDocumentRoot(host, defaultHost->documentRoot);
        }
        if (host->serverRoot == 0) {
            httpSetHostServerRoot(host, defaultHost->serverRoot);
        }
        if (host->mimeTypes == 0) {
            host->mimeTypes = defaultHost->mimeTypes;
        }
        /*
            Check aliases have directory blocks. Inherit authorization from the best matching directory
         */
        for (nextAlias = 0; (alias = mprGetNextItem(host->aliases, &nextAlias)) != 0; ) {
            path = httpMakePath(host->loc, alias->filename);
            if ((bestDir = httpLookupBestDir(host, path)) == 0) {
                bestDir = httpCreateBareDir(alias->filename);
                httpAddDir(host, bestDir);
            }
        }
        mprLog(MPR_CONFIG, "Host \"%s\"", host->name);
        mprLog(MPR_CONFIG, "    ServerRoot \"%s\"", host->serverRoot);
        mprLog(MPR_CONFIG, "    DocumentRoot: \"%s\"", host->documentRoot);
    }
    return 1;
}


/*  
    Process the configuration settings. Permissible to modify key and value.
    Return < 0 for errors, zero if directive not found, otherwise 1 is success.
    TODO -- this function is quite big. Could be subject to a FEATURE.
 */
static int processSetting(MaMeta *meta, char *key, char *value, MaConfigState *state)
{
    MaAppweb    *appweb;
    HttpAlias   *alias;
    HttpLoc     *loc;
    HttpHost    *host;
    HttpDir     *dir;
    Http        *http;
    HttpAuth    *auth;
    HttpLimits  *limits;
    HttpStage   *stage;
    HttpServer  *server;
    MprHash     *hp;
    MprModule   *module;
    MprOff      onum;
    char        *name, *path, *prefix, *cp, *tok, *ext, *mimeType, *url, *newUrl, *extensions, *codeStr, *ip;
    char        *names, *type, *items, *include, *exclude, *when, *mimeTypes, *lib, *sep;
    ssize       len;
    int         port, rc, code, num, colonCount, mask, level;

    mprAssert(state);
    mprAssert(key);

    appweb = meta->appweb;
    http = appweb->http;
    server = 0;
    host = state->host;
    limits = host->limits;
    dir = state->dir;
    loc = state->loc;
    mprAssert(state->loc->prefix);

    mprAssert(host);
    mprAssert(dir);
    auth = state->auth;

    switch (toupper((int) key[0])) {
    case 'A':
        if (scasecmp(key, "Alias") == 0) {
            /* Scope: server, host */
            if (maSplitConfigValue(&prefix, &path, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            prefix = stemplate(prefix, loc->tokens);
            path = httpMakePath(loc, path);
            if (httpLookupAlias(host, prefix)) {
                mprError("Alias \"%s\" already exists", prefix);
                return MPR_ERR_BAD_SYNTAX;
            }
            alias = httpCreateAlias(prefix, path, 0);
            mprLog(4, "Alias: \"%s for \"%s\"", prefix, path);
            if (httpAddAlias(host, alias) < 0) {
                mprError("Can't insert alias: %s", prefix);
                return MPR_ERR_BAD_SYNTAX;
            }
            if (scmp(loc->prefix, alias->prefix) == 0) {
                httpSetLocationAlias(loc, alias);
            }
            return 1;

        } else if (scasecmp(key, "AddFilter") == 0) {
            /* Scope: server, host, location */
            name = stok(value, " \t", &extensions);
            if (httpAddFilter(loc, name, extensions, HTTP_STAGE_RX | HTTP_STAGE_TX) < 0) {
                mprError("Can't add filter %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (scasecmp(key, "AddInputFilter") == 0) {
            /* Scope: server, host, location */
            name = stok(value, " \t", &extensions);
            if (httpAddFilter(loc, name, extensions, HTTP_STAGE_RX) < 0) {
                mprError("Can't add filter %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (scasecmp(key, "AddOutputFilter") == 0) {
            /* Scope: server, host, location */
            name = stok(value, " \t", &extensions);
            if (httpAddFilter(loc, name, extensions, HTTP_STAGE_TX) < 0) {
                mprError("Can't add filter %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (scasecmp(key, "AddHandler") == 0) {
            /* Scope: server, host, location */
            name = stok(value, " \t", &extensions);
            if (httpAddHandler(state->loc, name, extensions) < 0) {
                mprError("Can't add handler %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (scasecmp(key, "AddType") == 0) {
            if (maSplitConfigValue(&mimeType, &ext, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            mprAddMime(host->mimeTypes, ext, mimeType);
            return 1;

        } else if (scasecmp(key, "Allow") == 0) {
            char *from, *spec;
            if (maSplitConfigValue(&from, &spec, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            /* spec can be: all, host, ipAddr */
            httpSetAuthAllow(auth, spec);
            return 1;

        } else if (scasecmp(key, "AuthGroupFile") == 0) {
#if BLD_FEATURE_AUTH_FILE
            //  TODO - this belongs elsewhere
            path = mprJoinPath(state->configDir, strim(value, "\"", MPR_TRIM_BOTH));
            path = httpMakePath(loc, path);
            if (httpReadGroupFile(http, auth, path) < 0) {
                mprError("Can't open AuthGroupFile %s", path);
                return MPR_ERR_BAD_SYNTAX;
            }
#else
            mprError("AuthGroupFile directive not supported when --auth=pam");
#endif
            return 1;

        } else if (scasecmp(key, "AuthMethod") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            if (scasecmp(value, "pam") == 0) {
                auth->backend = HTTP_AUTH_METHOD_PAM;
                return 1;

            } else if (scasecmp(value, "file") == 0) {
                auth->backend = HTTP_AUTH_METHOD_FILE;
                return 1;

            } else {
                return MPR_ERR_BAD_SYNTAX;
            }

        } else if (scasecmp(key, "AuthName") == 0) {
            //  TODO - should be httpSetAuthRealm
            httpSetAuthRealm(auth, strim(value, "\"", MPR_TRIM_BOTH));
            return 1;
            
        } else if (scasecmp(key, "AuthType") == 0) {
            //  TODO - config parser should trim there for us
            value = strim(value, "\"", MPR_TRIM_BOTH);
            if (scasecmp(value, "Basic") == 0) {
                auth->type = HTTP_AUTH_BASIC;

            } else if (scasecmp(value, "Digest") == 0) {
                auth->type = HTTP_AUTH_DIGEST;

            } else if (scasecmp(value, "None") == 0) {
                auth->type = 0;

            } else {
                mprError("Unsupported authorization protocol");
                return MPR_ERR_BAD_SYNTAX;
            }
            return 1;
            
        } else if (scasecmp(key, "AuthUserFile") == 0) {
#if BLD_FEATURE_AUTH_FILE
            //  TODO - this belons elsewhere
            path = mprJoinPath(state->configDir, strim(value, "\"", MPR_TRIM_BOTH));
            path = httpMakePath(loc, path);
            if (httpReadUserFile(http, auth, path) < 0) {
                mprError("Can't open AuthUserFile %s", path);
                return MPR_ERR_BAD_SYNTAX;
            }
#else
            mprError("AuthUserFile directive not supported when --auth=pam");
#endif
            return 1;

        } else if (scasecmp(key, "AuthDigestQop") == 0) {
            value = slower(strim(value, "\"", MPR_TRIM_BOTH));
            if (strcmp(value, "none") != 0 && strcmp(value, "auth") != 0 && strcmp(value, "auth-int") != 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            httpSetAuthQop(auth, value);
            return 1;

        } else if (scasecmp(key, "AuthDigestAlgorithm") == 0) {
            return 1;

        } else if (scasecmp(key, "AuthDigestDomain") == 0) {
            return 1;

        } else if (scasecmp(key, "AuthDigestNonceLifetime") == 0) {
            return 1;
        }
        break;

    case 'B':
        if (scasecmp(key, "BrowserMatch") == 0) {
            return 1;
        }
        break;

    case 'C':
        if (scasecmp(key, "Cache") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            when = stok(value, " \t", &extensions);
            httpAddLocationExpiry(loc, (MprTime) stoi(when, 10, NULL), extensions);
            return 1;

        } else if (scasecmp(key, "CacheByType") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            when = stok(value, " \t", &mimeTypes);
            httpAddLocationExpiryByType(loc, (MprTime) stoi(when, 10, NULL), mimeTypes);
            return 1;

        } else if (scasecmp(key, "Chroot") == 0) {
#if BLD_UNIX_LIKE
            path = httpMakePath(loc, strim(value, "\"", MPR_TRIM_BOTH));
            if (chdir(path) < 0) {
                mprError("Can't change working directory to %s", path);
                return MPR_ERR_CANT_OPEN;
            }
            if (chroot(path) < 0) {
                if (errno == EPERM) {
                    mprError("Must be super user to use the --chroot option\n");
                } else {
                    mprError("Can't change change root directory to %s, errno %d\n", path, errno);
                }
                return MPR_ERR_BAD_SYNTAX;
            }
            mprLog(MPR_CONFIG, "Chroot to: \"%s\"", path);
            return 1;
#else
            mprError("Chroot directive not supported on this operating system\n");
            return MPR_ERR_BAD_SYNTAX;
#endif

        //  FUTURE - support simple AccessLog with standard NCSA format
        } else if (scasecmp(key, "CustomLog") == 0) {
#if !BLD_FEATURE_ROMFS
            char *format, *end;
            if (*value == '\"') {
                end = strchr(++value, '\"');
                if (end == 0) {
                    mprError("Missing closing quote");
                    return MPR_ERR_BAD_SYNTAX;
                }
                *end++ = '\0';
                path = value;
                format = end;
                while ((int) isspace((int) *format)) {
                    format++;
                }

            } else {
                path = stok(value, " \t", &format);
            }
            if (path == 0 || format == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            path = httpMakePath(loc, path);
            maSetAccessLog(host, path, strim(format, "\"", MPR_TRIM_BOTH));
#endif
            return 1;
        }
        break;

    case 'D':
        if (scasecmp(key, "Deny") == 0) {
            char *from, *spec;
            if (maSplitConfigValue(&from, &spec, value, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            httpSetAuthDeny(auth, spec);
            return 1;

        } else if (scasecmp(key, "DirectoryIndex") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            if (dir == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            httpSetDirIndex(dir, value);
            return 1;

        } else if (scasecmp(key, "DocumentRoot") == 0) {
            path = httpMakePath(loc, strim(value, "\"", MPR_TRIM_BOTH));
            httpSetHostDocumentRoot(host, path);
            httpSetDirPath(dir, path);
            return 1;
        }
        break;

    case 'E':
        if (scasecmp(key, "ErrorDocument") == 0) {
            codeStr = stok(value, " \t", &url);
            if (codeStr == 0 || url == 0) {
                mprError("Bad ErrorDocument directive");
                return MPR_ERR_BAD_SYNTAX;
            }
            httpAddErrorDocument(loc, codeStr, url);
            return 1;

        } else if (scasecmp(key, "ErrorLog") == 0) {
            path = strim(value, "\"", MPR_TRIM_BOTH);
            if (path && *path) {
                if (meta->alreadyLogging) {
                    mprLog(4, "Already logging. Ignoring ErrorLog directive");
                } else {
                    maStopLogging(meta);
                    if (strncmp(path, "stdout", 6) != 0 && strncmp(path, "stderr", 6) != 0) {
                        path = httpMakePath(loc, path);
                    }
                    if (maStartLogging(host, path) < 0) {
                        mprError("Can't write to ErrorLog: %s", path);
                        return MPR_ERR_BAD_SYNTAX;
                    }
                }
            }
            return 1;
        }
        break;

    case 'G':
        if (scasecmp(key, "Group") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            maSetHttpGroup(appweb, value);
            return 1;
        }
        break;

    case 'H':
        break;

    case 'K':
        if (scasecmp(key, "KeepAlive") == 0) {
            if (scasecmp(value, "on") != 0) {
                limits->keepAliveCount = 0;
            }
            return 1;

        } else if (scasecmp(key, "KeepAliveTimeout") == 0) {
            if (! mprGetDebugMode()) {
                limits->inactivityTimeout = atoi(value) * MPR_TICKS_PER_SEC;
            }
            return 1;
        }
        break;

    case 'L':
        if (scasecmp(key, "LimitChunkSize") == 0) {
            limits->chunkSize = atoi(value);
            return 1;

        } else if (scasecmp(key, "LimitClients") == 0) {
            mprSetMaxSocketClients(atoi(value));
            return 1;

        } else if (scasecmp(key, "LimitMemoryMax") == 0) {
            mprSetMemLimits(-1, (ssize) stoi(value, 10, 0));
            return 1;

        } else if (scasecmp(key, "LimitMemoryRedline") == 0) {
            mprSetMemLimits((ssize) stoi(value, 10, 0), -1);
            return 1;

        } else if (scasecmp(key, "LimitRequestBody") == 0) {
            onum = stoi(value, 10, 0);
            limits->receiveBodySize = onum;
            return 1;

        } else if (scasecmp(key, "LimitRequestFields") == 0 || scasecmp(key, "LimitRequestHeaderCount") == 0) {
            num = atoi(value);
            limits->headerCount = num;
            return 1;

        } else if (scasecmp(key, "LimitRequestFieldSize") == 0 || scasecmp(key, "LimitRequestHeaderSize") == 0) {
            num = atoi(value);
            limits->headerSize = num;
            return 1;

        } else if (scasecmp(key, "LimitResponseBody") == 0) {
            onum = stoi(value, 10, 0);
            limits->transmissionBodySize = onum;
            return 1;

        } else if (scasecmp(key, "LimitStageBuffer") == 0) {
            num = atoi(value);
            limits->stageBufferSize = num;
            return 1;

        } else if (scasecmp(key, "LimitUrl") == 0 || scasecmp(key, "LimitUri") == 0) {
            num = atoi(value);
            limits->uriSize = num;
            return 1;

        } else if (scasecmp(key, "LimitUploadSize") == 0) {
            limits->uploadSize = stoi(value, 10, 0);
            return 1;

#if DEPRECATED
        } else if (scasecmp(key, "ListenIF") == 0) {
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
                    mprError("Bad listen port number %d", port);
                    return MPR_ERR_BAD_SYNTAX;
                }

            } else {            /* interface */
                port = HTTP_DEFAULT_PORT;
            }

            ipList = mprGetMpr()->socketService->getInterfaceList();
            ip = (MprInterface*) ipList->getFirst();
            if (ip == 0) {
                mprError("Can't find interfaces, use Listen-directive with IP address.");
                return MPR_ERR_BAD_SYNTAX;
            }
            while (ip) {
                if (scasecmp(ip->name, value) != 0) {
                    ip = (MprInterface*) ipList->getNext(ip);
                    continue;
                }
                meta.insert(new HttpServer(ip->ip, port));
                if (host->ipAddrPort == 0) {
                    httpSetHostName(host, ip->ip, port);
                }
                break;
            }
            return 1;
#endif

        } else if (scasecmp(key, "Listen") == 0) {
            /*
                Options:
                    ip:port
                    ip          default port MA_SERVER_DEFAULT_PORT_NUM
                    port        All ip interfaces on this port
            
                Where ipAddr may be "::::::" for ipv6 addresses or may be enclosed in "[]" if appending a port.
             */
            value = strim(value, "\"", MPR_TRIM_BOTH);
            if (isdigit((int) *value) && strchr(value, '.') == 0 && strchr(value, ':') == 0) {
                /*
                    Port only, listen on all interfaces (ipv4 + ipv6)
                 */
                port = atoi(value);
                if (port <= 0 || port > 65535) {
                    mprError("Bad listen port number %d", port);
                    return MPR_ERR_BAD_SYNTAX;
                }
                ip = 0;

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
                    ip = value;
                    if (*value == '[' && (cp = strrchr(cp, ':')) != 0) {
                        *cp++ = '\0';
                        port = atoi(cp);
                    } else {
                        port = HTTP_DEFAULT_PORT;
                    }

                } else {
                    /* ipv4 */
                    ip = value;
                    if ((cp = strrchr(value, ':')) != 0) {
                        *cp++ = '\0';
                        port = atoi(cp);

                    } else {
                        port = HTTP_DEFAULT_PORT;
                    }
                    if (*ip == '[') {
                        ip++;
                    }
                    len = slen(ip);
                    if (ip[len - 1] == ']') {
                        ip[len - 1] = '\0';
                    }
                }
                if (ip && *ip == '*') {
                    ip = 0;
                }
            }
            if (port == 0) {
                mprError("Bad or missing port %d in Listen directive", port);
                return -1;
            }
#if FUTURE
            if (host != defaultHost) {
                mprError("Can't have listen directive inside a VirtualHost block");
                return -1;
            }
#endif
            server = httpCreateServer(ip, port, NULL, 0);
            server->limits = limits;
            mprAddItem(meta->servers, server);
#if UNUSED
            httpAddHostToServer(server, host);
            httpSetHostName(host, ip, port);
#endif
            return 1;

        } else if (scasecmp(key, "LogLevel") == 0) {
            if (meta->alreadyLogging) {
                mprLog(4, "Already logging. Ignoring LogLevel directive");
            } else {
                value = strim(value, "\"", MPR_TRIM_BOTH);
                level = atoi(value);
                mprSetLogLevel(level);
            }
            return 1;

        } else if (scasecmp(key, "LogRotation") == 0) {
            value = stok(value, " \t", &tok);
            host->logCount = (int) stoi(value, 10, NULL);
            value = stok(0, "\n", &tok);
            host->logSize = (int) stoi(value, 10, NULL) * (1024 * 1024);
            return 1;

        } else if (scasecmp(key, "LogTrace") == 0) {
            cp = stok(value, " \t", &tok);
            level = atoi(cp);
            if (level < 0 || level > 9) {
                mprError("Bad LogTrace level %d, must be 0-9", level);
                return MPR_ERR_BAD_SYNTAX;
            }
            items = slower(stok(0, "\n", &tok));
            mask = 0;
            if (strstr(items, "conn")) {
                mask |= HTTP_TRACE_CONN;
            }
            if (strstr(items, "first")) {
                mask |= HTTP_TRACE_FIRST;
            }
            if (strstr(items, "headers")) {
                mask |= HTTP_TRACE_HEADER;
            }
            if (strstr(items, "body")) {
                mask |= HTTP_TRACE_BODY;
            }
            if (strstr(items, "request") || strstr(items, "transmit")) {
                mask |= HTTP_TRACE_TX;
            }
            if (strstr(items, "response") || strstr(items, "receive")) {
                mask |= HTTP_TRACE_RX;
            }
            if (strstr(items, "time")) {
                mask |= HTTP_TRACE_TIME;
            }
            httpSetHostTrace(host, level, mask);
            return 1;

        } else if (scasecmp(key, "LogTraceFilter") == 0) {
            cp = stok(value, " \t", &tok);
            len = atoi(cp);
            include = stok(0, " \t", &tok);
            exclude = stok(0, " \t", &tok);
            include = strim(include, "\"", MPR_TRIM_BOTH);
            exclude = strim(exclude, "\"", MPR_TRIM_BOTH);
            httpSetHostTraceFilter(host, len, include, exclude);
            return 1;

        } else if (scasecmp(key, "LoadModulePath") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            sep = MPR_SEARCH_SEP;
            lib = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
            path = sjoin(value, sep, mprGetAppDir(), sep, lib, sep, BLD_LIB_PREFIX, NULL);
            mprSetModuleSearchPath(path);
            return 1;

        } else if (scasecmp(key, "LoadModule") == 0) {
            name = stok(value, " \t", &tok);
            if (name == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            path = stok(0, "\n", &tok);
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
        if (scasecmp(key, "MaxKeepAliveRequests") == 0) {
            limits->keepAliveCount = atoi(value);
            return 1;

        } else if (scasecmp(key, "MemoryDepletionPolicy") == 0) {
            value = slower(value);
            if (strcmp(value, "exit") == 0) {
                mprSetMemPolicy(MPR_ALLOC_POLICY_EXIT);
            } else if (strcmp(value, "restart") == 0) {
                mprSetMemPolicy(MPR_ALLOC_POLICY_RESTART);
            }
            mprSetMemLimits(atoi(value), -1);
            return 1;
        }
        break;

    case 'N':
        if (scasecmp(key, "NameVirtualHost") == 0) {
            mprLog(4, "NameVirtual Host: %s ", value);
            mprParseIp(value, &ip, &port, -1);
            httpSetNamedVirtualServers(http, ip, port); 
            return 1;
        }
        break;

    case 'O':
        if (scasecmp(key, "Order") == 0) {
            if (scasecmp(strim(value, "\"", MPR_TRIM_BOTH), "Allow,Deny") == 0) {
                httpSetAuthOrder(auth, HTTP_ALLOW_DENY);
            } else {
                httpSetAuthOrder(auth, HTTP_DENY_ALLOW);
            }
            return 1;
        }
        break;

    case 'P':
        if (scasecmp(key, "Protocol") == 0) {
            httpSetHostProtocol(host, value);
            if (strcmp(value, "HTTP/1.0") != 0 && strcmp(value, "HTTP/1.1") != 0) {
                mprError("Unknown http protocol %s. Should be HTTP/1.0 or HTTP/1.1", value);
            }
            return 1;

        } else if (scasecmp(key, "PutMethod") == 0) {
            if (scasecmp(value, "on") == 0) {
                loc->flags |= HTTP_LOC_PUT_DELETE;
            } else {
                loc->flags &= ~HTTP_LOC_PUT_DELETE;
            }
            return 1;
        }
        break;

    case 'R':
        if (scasecmp(key, "Redirect") == 0) {
            if (value[0] == '/' || value[0] == 'h') {
                code = 302;
                url = stok(value, " \t", &tok);

            } else if (isdigit((int) value[0])) {
                cp = stok(value, " \t", &tok);
                code = atoi(cp);
                url = stok(0, " \t\n", &tok);

            } else {
                cp = stok(value, " \t", &tok);
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
                url = stok(0, " \t\n", &tok);
            }
            if (code >= 300 && code <= 399) {
                newUrl = stok(0, "\n", &tok);
            } else {
                newUrl = "";
            }
            if (code <= 0 || url == 0 || newUrl == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            url = strim(url, "\"", MPR_TRIM_BOTH);
            newUrl = strim(newUrl, "\"", MPR_TRIM_BOTH);
            mprLog(4, "insertAlias: Redirect %d from \"%s\" to \"%s\"", code, url, newUrl);
            alias = httpCreateAlias(url, newUrl, code);
            httpAddAlias(host, alias);
            return 1;

        } else if (scasecmp(key, "Require") == 0) {
            if (maGetConfigValue(&type, value, &tok, 1) < 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            if (scasecmp(type, "acl") == 0) {
#if BLD_FEATURE_AUTH_FILE
                HttpAcl   acl;
                char    *aclSpec;
                aclSpec = strim(tok, "\"", MPR_TRIM_BOTH);
                acl = httpParseAcl(auth, aclSpec);
                httpSetRequiredAcl(auth, acl);
#else
            mprError("Acl directive not supported when --auth=pam");
#endif

            } else if (scasecmp(type, "valid-user") == 0) {
                httpSetAuthAnyValidUser(auth);

            } else {
                names = strim(tok, "\"", MPR_TRIM_BOTH);
                if (scasecmp(type, "user") == 0) {
                    httpSetAuthRequiredUsers(auth, names);

                } else if (scasecmp(type, "group") == 0) {
                    httpSetAuthRequiredGroups(auth, names);

                } else {
                    mprError("Bad Require syntax: %s", type);
                    return MPR_ERR_BAD_SYNTAX;
                }
            }
            return 1;

        } else if (scasecmp(key, "ResetPipeline") == 0) {
            httpResetPipeline(loc);
            return 1;

        } else if (scasecmp(key, "RunHandler") == 0) {
            //  TODO - not finished "name" is not set
            name = stok(value, " \t", &value);
            value = slower(strim(value, "\"", MPR_TRIM_BOTH));
            if (scmp(value, "before") == 0) {
                state->loc->flags |= HTTP_LOC_BEFORE;
            } else if (scmp(value, "after") == 0) {
                state->loc->flags |= HTTP_LOC_AFTER;
            } else if (scmp(value, "smart") == 0) {
                state->loc->flags |= HTTP_LOC_SMART;
            } else {
                mprError("Unknown RunHandler argument %s, valid [before|after|smart]", value);
                return MPR_ERR_BAD_SYNTAX;
            }
        }
        break;

    case 'S':
        if (scasecmp(key, "ServerName") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            value = strim(value, "http://", MPR_TRIM_START);
            httpSetHostName(host, value, -1);
            return 1;

        } else if (scasecmp(key, "ServerRoot") == 0) {
            path = stemplate(strim(value, "\"", MPR_TRIM_BOTH), loc->tokens);
            maSetMetaRoot(meta, path);
            httpSetHostServerRoot(host, path);
            mprLog(MPR_CONFIG, "Server Root \"%s\"", path);
            return 1;

        } else if (scasecmp(key, "SetConnector") == 0) {
            /* Scope: meta, host, loc */
            value = strim(value, "\"", MPR_TRIM_BOTH);
            if (httpSetConnector(loc, value) < 0) {
                mprError("Can't add handler %s", value);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (scasecmp(key, "SetHandler") == 0) {
            /* Scope: meta, host, location */
            name = stok(value, " \t", &extensions);
            if (httpSetHandler(state->loc, name) < 0) {
                mprError("Can't add handler %s", name);
                return MPR_ERR_CANT_CREATE;
            }
            return 1;

        } else if (scasecmp(key, "StartThreads") == 0) {
            num = atoi(value);
            mprSetMinWorkers(num);
            return 1;
        }
        break;

    case 'T':
        if (scasecmp(key, "ThreadLimit") == 0) {
            num = atoi(value);
            mprSetMaxWorkers(num);
            return 1;

        } else if (scasecmp(key, "ThreadStackSize") == 0) {
            num = atoi(value);
            mprSetThreadStackSize(num);
            return 1;

        } else if (scasecmp(key, "Timeout") == 0) {
            limits->requestTimeout = atoi(value) * MPR_TICKS_PER_SEC;
            return 1;

        } else if (scasecmp(key, "TraceMethod") == 0) {
            limits->enableTraceMethod = (scasecmp(value, "on") == 0);
            return 1;

        } else if (scasecmp(key, "TypesConfig") == 0) {
            path = httpMakePath(loc, strim(value, "\"", MPR_TRIM_BOTH));
            if ((host->mimeTypes = mprCreateMimeTypes(path)) == 0) {
                mprError("Can't open TypesConfig mime file %s", path);
                host->mimeTypes = mprCreateMimeTypes(NULL);
                return MPR_ERR_BAD_SYNTAX;
            }
            return 1;
        }
        break;

    case 'U':
        if (scasecmp(key, "UnloadModule") == 0) {
            name = stok(value, " \t", &tok);
            if (name == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            if ((cp = stok(0, "\n", &tok)) == 0) {
                return MPR_ERR_BAD_SYNTAX;
            }
            if ((module = mprLookupModule(name)) == 0) {
                mprError("Can't find module stage %s", name);
                return MPR_ERR_BAD_SYNTAX;
            }
            module->timeout = (int) stoi(cp, 10, NULL) * MPR_TICKS_PER_SEC;
            return 1;

        } else if (scasecmp(key, "UploadDir") == 0 || scasecmp(key, "FileUploadDir") == 0) {
            path = httpMakePath(loc, strim(value, "\"", MPR_TRIM_BOTH));
            loc->uploadDir = sclone(path);
            mprLog(MPR_CONFIG, "Upload directory: %s", path);
            return 1;

        } else if (scasecmp(key, "UploadAutoDelete") == 0) {
            value = strim(value, "\"", MPR_TRIM_BOTH);
            loc->autoDelete = (scasecmp(value, "on") == 0);
            return 1;

        } else if (scasecmp(key, "User") == 0) {
            maSetHttpUser(appweb, strim(value, "\"", MPR_TRIM_BOTH));
            return 1;
        }
        break;
    }
    rc = 0;

    /*
        Allow all stages to parse the request
     */
    hp = mprGetFirstKey(http->stages);
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
        hp = mprGetNextKey(http->stages, hp);
    }
    return rc;
}


/*
    Create a location block for a handler and an alias. Convenience routine for ScriptAlias, EjsAppAlias, EjsAppDirAlias.
 */
HttpLoc *maCreateLocationAlias(Http *http, MaConfigState *state, cchar *prefixArg, cchar *pathArg, cchar *handlerName, 
        int flags)
{
    HttpHost    *host;
    HttpAlias   *alias;
    HttpLoc     *loc;
    char        *path, *prefix;

    host = state->host;
    prefix = stemplate(prefixArg, state->loc->tokens);
    path = httpMakePath(state->loc, pathArg);

    /*
        Create an ejs application location block and alias
     */
    alias = httpCreateAlias(prefix, path, 0);
    httpAddAlias(host, alias);
    mprLog(4, "Alias \"%s\" for \"%s\"", prefix, path);

    if (httpLookupLocation(host, prefix)) {
        mprError("Location block already exists for \"%s\"", prefix);
        return 0;
    }
    loc = httpCreateInheritedLocation(state->loc);
    httpSetLocationHost(loc, host);
    httpSetLocationAuth(loc, state->dir->auth);
    httpSetLocationPrefix(loc, prefix);
    httpAddLocation(host, loc);
    httpSetLocationFlags(loc, flags);
    httpSetHandler(loc, handlerName);
    return loc;
}


/*
    Max stack depth is:
        Default Server          Level 1
            <VirtualHost>       Level 2
                <Directory>     Level 3
                    <Location>  Level 4
 *
 */
static MaConfigState *pushState(MaConfigState *state, int *top)
{
    MaConfigState   *next;

    (*top)++;
    if (*top >= MA_MAX_CONFIG_DEPTH) {
        mprError("Too many nested directives in config file at %s:%d", state->filename, state->lineNumber);
        return 0;
    }
    next = state + 1;
    next->meta = state->meta;
    next->host = state->host;
    next->loc = state->loc;
    next->dir = state->dir;
    next->auth = state->auth;
    next->lineNumber = state->lineNumber;
    next->enabled = state->enabled;
    next->filename = state->filename;
    next->file = state->file;

    return next;
}


static bool conditionalDefinition(char *key)
{
    //  TODO - should always create a conditional directive when a module is loaded. Even for user modules.
    if (scasecmp(key, "BLD_COMMERCIAL") == 0) {
        return strcmp(BLD_COMMERCIAL, "0") == 0;

#ifdef BLD_DEBUG
    } else if (scasecmp(key, "BLD_DEBUG") == 0) {
        return BLD_DEBUG;
#endif

    } else if (scasecmp(key, "CGI_MODULE") == 0) {
        return BLD_FEATURE_CGI;

    } else if (scasecmp(key, "DIR_MODULE") == 0) {
        return 1;

    } else if (scasecmp(key, "EJS_MODULE") == 0) {
        return BLD_FEATURE_EJS;

    } else if (scasecmp(key, "ESP_MODULE") == 0) {
        return BLD_FEATURE_ESP;

    } else if (scasecmp(key, "PHP_MODULE") == 0) {
        return BLD_FEATURE_PHP;
        
    } else if (scasecmp(key, "SSL_MODULE") == 0) {
        return BLD_FEATURE_SSL;

    } else if (scasecmp(key, BLD_OS) == 0) {
        return 1;

    } else if (scasecmp(key, BLD_CPU) == 0) {
        return 1;
    }
    return 0;
}


//  MOB - should these allocate
int maSplitConfigValue(char **s1, char **s2, char *buf, int quotes)
{
    char    *next;

    if (maGetConfigValue(s1, buf, &next, quotes) < 0 || maGetConfigValue(s2, next, &next, quotes) < 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (*s1 == 0 || *s2 == 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


//  MOB - refactor this API. Should be like stok()
int maGetConfigValue(char **arg, char *buf, char **nextToken, int quotes)
{
    char    *endp;

    mprAssert(arg);
    *arg = 0;
    
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
        *arg = stok(buf, " \t\n", nextToken);
    }
    return 0;
}

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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
