/**
    config.c - Parse the configuration file.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/***************************** Forward Declarations ****************************/

static bool closingBlock(MaState *state, cchar *key);
static bool conditionalDefinition(cchar *key);
static int configError(MaState *state, cchar *key);
static MaState *createState(MaMeta *meta);
static char *getSearchPath(cchar *dir);
static void manageState(MaState *state, int flags);
static int parseFile(MaState *state, cchar *path);
static MaState *popState(MaState *state);
static MaState *pushState(MaState *state, cchar *block);

/******************************************************************************/
/*
    Configure the meta-server. If the configFile is defined, use it. If not, then consider home, documents, ip and port.
 */
int maConfigureMeta(MaMeta *meta, cchar *configFile, cchar *home, cchar *documents, cchar *ip, int port)
{
    MaAppweb        *appweb;
    Http            *http;
    HttpServer      *server;
    HttpHost        *host;
    HttpRoute       *route, *cgiRoute;
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
        mprLog(2, "DocumentRoot %s", documents);
        if ((server = httpCreateConfiguredServer(home, documents, ip, port)) == 0) {
            return MPR_ERR_CANT_OPEN;
        }
        maAddServer(meta, server);
        host = meta->defaultHost = mprGetFirstItem(server->hosts);
        mprAssert(host);

        route = host->route;
        searchPath = getSearchPath(dir);
        mprSetModuleSearchPath(searchPath);

        maLoadModule(appweb, "cgiHandler", "mod_cgi");
        if (httpLookupStage(http, "cgiHandler")) {
            httpAddRouteHandler(route, "cgiHandler", ".cgi .cgi-nph .bat .cmd .pl .py");
            /*
                Add cgi-bin with a route for the /cgi-bin URL prefix.
             */
            path = "cgi-bin";
            if (mprPathExists(path, X_OK)) {
                cgiRoute = httpCreateAliasRoute(route, "/cgi-bin/", path, 0);
                mprLog(4, "ScriptAlias \"/cgi-bin/\":\"%s\"", path);
                httpSetRouteHost(cgiRoute, host);
                httpSetRouteHandler(cgiRoute, "cgiHandler");
                httpFinalizeRoute(cgiRoute);
                httpAddRoute(host, cgiRoute);
            }
        }
        maLoadModule(appweb, "espHandler", "mod_esp");
        if (httpLookupStage(http, "espHandler")) {
            httpAddRouteHandler(route, "espHandler", ".esp");
        }
        maLoadModule(appweb, "ejsHandler", "mod_ejs");
        if (httpLookupStage(http, "ejsHandler")) {
            httpAddRouteHandler(route, "ejsHandler", ".ejs");
        }
        maLoadModule(appweb, "phpHandler", "mod_php");
        if (httpLookupStage(http, "phpHandler")) {
            httpAddRouteHandler(route, "phpHandler", ".php");
        }
        if (httpLookupStage(http, "fileHandler")) {
            httpAddRouteHandler(route, "fileHandler", "");
        }
    }
    if (home) {
        maSetMetaHome(meta, home);
    }
    if (ip || port > 0) {
        maSetMetaAddress(meta, ip, port);
    }
    return 0;
}


int maOpenConfig(MaState *state, cchar *path)
{
    state->filename = sclone(path);
    state->configDir = mprGetAbsPath(mprGetPathDir(state->filename));
    if ((state->file = mprOpenFile(path, O_RDONLY | O_TEXT, 0444)) == 0) {
        mprError("Can't open %s for config directives", path);
        return MPR_ERR_CANT_OPEN;
    }
    mprLog(5, "Parsing config file: %s", state->filename);
    return 0;
}


int maParseConfig(MaMeta *meta, cchar *path)
{
    MaState     *state;

    mprLog(2, "Config File %s", path);
    state = createState(meta);
    if (parseFile(state, path) < 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (!maValidateConfiguration(meta)) {
        return MPR_ERR_BAD_ARGS;
    }
    if (mprHasMemError()) {
        mprError("Memory allocation error when initializing");
        return MPR_ERR_MEMORY;
    }
    return 0;
}


static int parseFile(MaState *state, cchar *path)
{
    MaDirective     *directive;
    char            *line, *tok, *key, *value;

    if ((state = pushState(state, 0)) == 0) {
        return 0;
    }
    if (maOpenConfig(state, path) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    for (state->lineNumber = 1; (line = mprGetFileString(state->file, 0, NULL)) != 0; state->lineNumber++) {
        for (tok = line; isspace((int) *tok); tok++) ;
        if (*tok == '\0' || *tok == '#') {
            continue;
        }
        if (!maTokenize(state, line, "%S %*", &key, &value)) {
            break;
        }
        if (!state->enabled && !closingBlock(state, key)) {
            mprLog(8, "Skipping key %s at %s:%d", key, state->filename, state->lineNumber);
            continue;
        }
        if ((directive = mprLookupKey(state->appweb->directives, key)) == 0) {
            mprError("Unknown directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
            return MPR_ERR_BAD_SYNTAX;
        }
        if ((*directive)(state, key, value) < 0) {
            return MPR_ERR_BAD_SYNTAX;
        }
        state = state->currentState;
    }
    /* EOF */
    if (state->prev && state->file == state->prev->file) {
        mprError("Unclosed directives in %s", state->filename);
        while (state->prev && state->file == state->prev->file) {
            state = state->prev;
        }
    }
    mprCloseFile(state->file);
    state = state->prev;
    state->lineNumber++;
    return 0;
}


static int accessLogDirective(MaState *state, cchar *key, cchar *value)
{
#if !BLD_FEATURE_ROMFS
    char    *path;
    if (!maTokenize(state, value, "%S", &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    maSetAccessLog(state->host, httpMakePath(state->route, path), "%h %l %u %t \"%r\" %>s %b");
    return 0;
#else
    configError("AccessLog not supported when using ROM FS");
    return MPR_ERR_BAD_SYNTAX;
#endif
}


static int addFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *extensions;

    if (!maTokenize(state, value, "%S %*", &name, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteFilter(state->route, name, extensions, HTTP_STAGE_RX | HTTP_STAGE_TX) < 0) {
        mprError("Can't add filter %s", name);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int addInputFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *extensions;

    if (!maTokenize(state, value, "%S %*", &name, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteFilter(state->route, name, extensions, HTTP_STAGE_RX) < 0) {
        mprError("Can't add filter %s", name);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int addLanguageDirective(MaState *state, cchar *key, cchar *value)
{
    char    *lang, *path, *position;

    if (!maTokenize(state, value, "%S %P %S", &lang, &path, position)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteLanguage(state->route, lang, path, scasecmp(position, "after") == 0 ? HTTP_LANG_AFTER : HTTP_LANG_BEFORE);
    return 0;
}


static int addLanguageRootDirective(MaState *state, cchar *key, cchar *value)
{
    char    *lang, *path;

    if (!maTokenize(state, value, "%S %P", &lang, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteLanguageRoot(state->route, lang, path);
    return 0;
}


static int addOutputFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *extensions;

    if (!maTokenize(state, value, "%S %*", &name, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteFilter(state->route, name, extensions, HTTP_STAGE_TX) < 0) {
        mprError("Can't add filter %s", name);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int addHandlerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *extensions;

    if (!maTokenize(state, value, "%S %*", &name, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteHandler(state->route, name, extensions) < 0) {
        mprError("Can't add handler %s", name);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int addTypeDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *mimeType;

    if (!maTokenize(state, value, "%S %S", &name, &mimeType)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    mprAddMime(state->host->mimeTypes, name, mimeType);
    return 0;
}


static int aliasDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *alias;
    char        *prefix, *path;

    if (!maTokenize(state, value, "%S %P", &prefix, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    alias = httpCreateAliasRoute(state->route, prefix, path, 0);
    httpFinalizeRoute(alias);
    httpAddRoute(state->host, alias);
    return 0;
}


static int allowDirective(MaState *state, cchar *key, cchar *value)
{
    char    *from, *spec;

    if (!maTokenize(state, value, "%S %S", &from, &spec)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetAuthAllow(state->auth, spec);
    return 0;
}


static int authGroupFileDirective(MaState *state, cchar *key, cchar *value)
{
#if BLD_FEATURE_AUTH_FILE
    char    *path;

    path = mprJoinPath(state->configDir, value);
    path = httpMakePath(state->route, path);
    if (httpReadGroupFile(state->http, state->auth, path) < 0) {
        mprError("Can't open AuthGroupFile %s", path);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
#else
    mprError("AuthGroupFile directive not supported when --auth=pam");
    return MPR_ERR_BAD_SYNTAX;
#endif
}


static int authMethodDirective(MaState *state, cchar *key, cchar *value)
{
    if (scasecmp(value, "pam") == 0) {
        state->auth->backend = HTTP_AUTH_METHOD_PAM;
    } else if (scasecmp(value, "file") == 0) {
        state->auth->backend = HTTP_AUTH_METHOD_FILE;
    } else {
        return configError(state, key);
    }
    return 0;
}


static int authNameDirective(MaState *state, cchar *key, cchar *value)
{
    if (scasecmp(value, "Basic") == 0) {
        state->auth->type = HTTP_AUTH_BASIC;
    } else if (scasecmp(value, "Digest") == 0) {
        state->auth->type = HTTP_AUTH_DIGEST;
    } else if (scasecmp(value, "None") == 0) {
        state->auth->type = 0;
    } else {
        mprError("Unsupported authorization protocol");
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int authRealmDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetAuthRealm(state->auth, value);
    return 0;
}


static int authUserFileDirective(MaState *state, cchar *key, cchar *value)
{
#if BLD_FEATURE_AUTH_FILE
    char    *path;
    path = mprJoinPath(state->configDir, value);
    path = httpMakePath(state->route, path);
    if (httpReadUserFile(state->http, state->auth, path) < 0) {
        mprError("Can't open AuthUserFile %s", path);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
#else
    mprError("AuthUserFile directive not supported when --auth=pam");
    return MPR_ERR_BAD_SYNTAX;
#endif
}


static int authDigestQopDirective(MaState *state, cchar *key, cchar *value)
{
    if (!scasesame(value, "none") && !scasesame(value, "auth") && !scasesame(value, "auth-int")) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetAuthQop(state->auth, value);
    return 0;
}


static int cacheByTypeDirective(MaState *state, cchar *key, cchar *value)
{
    char    *when, *mimeTypes;

    if (!maTokenize(state, value, "%S %S", &when, &mimeTypes)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteExpiryByType(state->route, (MprTime) stoi(when, 10, NULL), mimeTypes);
    return 0;
}


static int clientCacheDirective(MaState *state, cchar *key, cchar *value)
{
    char    *when, *extensions;

    if (!maTokenize(state, value, "%S %S", &when, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteExpiry(state->route, (MprTime) stoi(when, 10, NULL), extensions);
    return 0;
}


static int chrootDirective(MaState *state, cchar *key, cchar *value)
{
#if BLD_UNIX_LIKE
    char    *path;
    path = httpMakePath(state->route, value);
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
    return 0;
#else
    mprError("Chroot directive not supported on this operating system\n");
    return MPR_ERR_BAD_SYNTAX;
#endif
}


static int closeRouteDirective(MaState *state, cchar *key, cchar *value)
{
    if (state->route != state->prev->route) {
        httpFinalizeRoute(state->route);
    }
    return 0;
}


static int closeDirective(MaState *state, cchar *key, cchar *value)
{
    popState(state);
    return 0;
}


static int conditionDirective(MaState *state, cchar *key, cchar *value)
{
    int     not;

    if (!maTokenize(state, value, "%! %*", &not, &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteCondition(state->route, value, not | HTTP_ROUTE_STATIC_VALUES);
    return 0;
}


static int customLogDirective(MaState *state, cchar *key, cchar *value)
{
#if !BLD_FEATURE_ROMFS
    char    *path, *format;
    if (!maTokenize(state, value, "%S %*", &path, &format)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    path = httpMakePath(state->route, path);
    maSetAccessLog(state->host, path, format);
    return 0;
#else
    configError("CustomLog not supported when using ROM FS");
    return MPR_ERR_BAD_SYNTAX;
#endif
}


static int denyDirective(MaState *state, cchar *key, cchar *value)
{
    char *from, *spec;

    if (!maTokenize(state, value, "%S %S", &from, &spec)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetAuthDeny(state->auth, spec);
    return 0;
}


static int directoryDirective(MaState *state, cchar *key, cchar *value)
{
    char        *path;

    state = pushState(state, key);
    state->route = httpCreateInheritedRoute(state->route);
    httpSetRouteDir(state->route, path);
    return 0;
}


static int directoryIndexDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteIndex(state->route, value);
    return 0;
}


static int documentRootDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteDir(state->route, value);
    return 0;
}


static int errorDocumentDirective(MaState *state, cchar *key, cchar *value)
{
    char    *status, *url;

    if (!maTokenize(state, value, "%S %S", &status, &url)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteErrorDocument(state->route, status, url);
    return 0;
}


static int errorLogDirective(MaState *state, cchar *key, cchar *value)
{
    if (state->meta->alreadyLogging) {
        mprLog(4, "Already logging. Ignoring ErrorLog directive");
        return 0;
    }
    maStopLogging(state->meta);
    if (sncmp(value, "stdout", 6) != 0 && sncmp(value, "stderr", 6) != 0) {
        value = httpMakePath(state->route, value);
    }
    if (maStartLogging(state->host, value) < 0) {
        mprError("Can't write to ErrorLog: %s", value);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int fieldDirective(MaState *state, cchar *key, cchar *value)
{
    char    *field;
    int     not;

    if (!maTokenize(state, value, "%S %! %*", &field, &not, &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteField(state->route, field, value, not | HTTP_ROUTE_STATIC_VALUES);
    return 0;
}


static int groupDirective(MaState *state, cchar *key, cchar *value)
{
    maSetHttpGroup(state->appweb, value);
    return 0;
}


static int headerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *header;
    int     not;

    if (!maTokenize(state, value, "%S %! %*", &header, &not, &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteHeader(state->route, header, value, not | HTTP_ROUTE_STATIC_VALUES);
    return 0;
}


static int includeDirective(MaState *state, cchar *key, cchar *value)
{
    MprList         *includes;
    MprDirEntry     *dp;
    char            *cp;
    int             next;

    if (!maTokenize(state, value, "%P", &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    //  MOB - change this to support proper regexp
    if ((cp = strchr(value, '*')) == 0) {
        if (parseFile(state, value) < 0) {
            return MPR_ERR_CANT_OPEN;
        }
    } else {
        /* Process wild cards. This is very simple - only "*" is supported.  */
        *cp = '\0';
        value = strim(value, "/", MPR_TRIM_END);
        cp = mprJoinPath(state->meta->home, value);
        if ((includes = mprGetPathFiles(cp, 0)) != 0) {
            value = mprJoinPath(state->meta->home, value);
            for (next = 0; (dp = mprGetNextItem(includes, &next)) != 0; ) {
                if (parseFile(state, dp->name) < 0) {
                    return MPR_ERR_CANT_OPEN;
                }
            }
        }
    }
    return 0;
}


static int ifDirective(MaState *state, cchar *key, cchar *value)
{
    state = pushState(state, key);
    state->enabled = conditionalDefinition(value);
    if (!state->enabled) {
        mprLog(7, "If \"%s\" conditional is false at %s:%d", value, state->filename, state->lineNumber);
    }
    return 0;
}


static int keepAliveDirective(MaState *state, cchar *key, cchar *value)
{
    bool    on;

    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    state->host->limits->keepAliveCount = on;
    return 0;
}


static int keepAliveTimeoutDirective(MaState *state, cchar *key, cchar *value)
{
    if (! mprGetDebugMode()) {
        state->host->limits->inactivityTimeout = atoi(value) * MPR_TICKS_PER_SEC;
    }
    return 0;
}


static int limitChunkSizeDirective(MaState *state, cchar *key, cchar *value)
{
    //  MOB - API for this
    state->host->limits->chunkSize = atoi(value);
    return 0;
}


static int limitClientsDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMaxSocketClients(atoi(value));
    return 0;
}


static int limitMemoryMaxDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMemLimits(-1, (ssize) stoi(value, 10, 0));
    return 0;
}


static int limitMemoryRedlineDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMemLimits((ssize) stoi(value, 10, 0), -1);
    return 0;
}


static int limitRequestBodyDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->receiveBodySize = stoi(value, 10, 0);
    return 0;
}


static int limitRequestFieldsDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->headerCount = atoi(value);
    return 0;
}


static int limitRequestFieldSizeDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->headerSize = atoi(value);
    return 0;
}


static int limitResponseBodyDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->transmissionBodySize = stoi(value, 10, 0);
    return 0;
}


static int limitStageBufferDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->stageBufferSize = atoi(value);
    return 0;
}


static int limitUriDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->uriSize = atoi(value);
    return 0;
}


static int limitUploadSizeDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->uploadSize = stoi(value, 10, 0);
    return 0;
}


/*
    Listen Options:
        ip:port
        ip          default port MA_SERVER_DEFAULT_PORT_NUM
        port        All ip interfaces on this port
    Where ipAddr may be "::::::" for ipv6 addresses or may be enclosed in "[]" if appending a port.
 */
static int listenDirective(MaState *state, cchar *key, cchar *value)
{
    HttpServer  *server;
    char        *ip, *cp, *vp;
    ssize       len;
    int         port, colonCount;

    vp = sclone(value);
    if (isdigit((int) *vp) && strchr(vp, '.') == 0 && strchr(vp, ':') == 0) {
        /*
            Port only, listen on all interfaces (ipv4 + ipv6)
         */
        port = atoi(vp);
        if (port <= 0 || port > 65535) {
            mprError("Bad listen port number %d", port);
            return MPR_ERR_BAD_SYNTAX;
        }
        ip = 0;

    } else {
        colonCount = 0;
        for (cp = vp; ((*cp != '\0') && (colonCount < 2)) ; cp++) {
            if (*cp == ':') {
                colonCount++;
            }
        }
        /*
            Hostname with possible port number. Port number parsed if address enclosed in "[]"
         */
        if (colonCount > 1) {
            /* ipv6 */
            ip = vp;
            if (*vp == '[' && (cp = strrchr(cp, ':')) != 0) {
                *cp++ = '\0';
                port = atoi(cp);
            } else {
                port = HTTP_DEFAULT_PORT;
            }

        } else {
            /* ipv4 */
            ip = vp;
            if ((cp = strrchr(vp, ':')) != 0) {
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
    server = httpCreateServer(ip, port, NULL, 0);
    server->limits = state->limits;
    mprAddItem(state->meta->servers, server);
    return 0;
}


static int loadDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *path;

    if (!maTokenize(state, value, "%S %P", &name, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    state->route->sourceName = sclone(name);
    state->route->sourcePath = sclone(path);
    return 0;
}


static int logLevelDirective(MaState *state, cchar *key, cchar *value)
{
    if (state->meta->alreadyLogging) {
        mprLog(4, "Already logging. Ignoring LogLevel directive");
    } else {
        mprSetLogLevel(atoi(value));
    }
    return 0;
}


static int logRotationDirective(MaState *state, cchar *key, cchar *value)
{
    int     count, size;

    if (!maTokenize(state, value, "%N %N", &count, &size)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    state->host->logCount = count;
    state->host->logSize = size * (1024 * 1024);
    return 0;
}


static int logRoutesDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *rp;
    int         next;

    mprLog(0, "HTTP Routes for URI %s", state->route->pattern);
    for (next = 0; (rp = mprGetNextItem(state->host->routes, &next)) != 0; ) {
        if (rp->targetDest) {
            mprLog(0, "  %-14s %-20s %-30s %-14s", rp->name, rp->methods ? rp->methods : "", rp->pattern,
                rp->targetDest);
        } else {
            mprLog(0, "  %-14s %-20s %-30s", rp->name, rp->methods ? rp->methods : "", rp->pattern);
        }
    }
    return 0;
}


static int logTraceDirective(MaState *state, cchar *key, cchar *value)
{
    char    *items;
    int     level, mask;

    if (!maTokenize(state, value, "%N %*", &level, &items)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (level < 0 || level > 9) {
        mprError("Bad LogTrace level %d, must be 0-9", level);
        return MPR_ERR_BAD_SYNTAX;
    }
    items = slower(items);
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
    httpSetHostTrace(state->host, level, mask);
    return 0;
}


static int logTraceFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *include, *exclude;
    int     size;

    if (!maTokenize(state, value, "%N %S %S", &size, &include, &exclude)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetHostTraceFilter(state->host, size, include, exclude);
    return 0;
}


static int loadModulePathDirective(MaState *state, cchar *key, cchar *value)
{
    char    *sep, *lib, *path;

    sep = MPR_SEARCH_SEP;
    lib = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
    path = sjoin(value, sep, mprGetAppDir(), sep, lib, sep, BLD_LIB_PREFIX, NULL);
    mprSetModuleSearchPath(path);
    return 0;
}


static int loadModuleDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *path;

    if (!maTokenize(state, value, "%N %S %S", &name, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (maLoadModule(state->appweb, name, path) < 0) {
        /*  Error messages already done */
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int maxKeepAliveRequestsDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->keepAliveCount = atoi(value);
    return 0;
}


static int memoryDepletionPolicyDirective(MaState *state, cchar *key, cchar *value)
{
    value = slower(value);
    if (scmp(value, "exit") == 0) {
        mprSetMemPolicy(MPR_ALLOC_POLICY_EXIT);
    } else if (scmp(value, "restart") == 0) {
        mprSetMemPolicy(MPR_ALLOC_POLICY_RESTART);
    }
    mprSetMemLimits(atoi(value), -1);
    return 0;
}


static int methodsDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteMethods(state->route, value);
    return 0;
}


static int updateDirective(MaState *state, cchar *key, cchar *value)
{
    char    *kind, *rest;
    int     not;

    if (!maTokenize(state, value, "%S %! %*", &kind, &not, &rest)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteUpdate(state->route, kind, rest, not | HTTP_ROUTE_STATIC_VALUES);
    return 0;
}


static int nameVirtualHostDirective(MaState *state, cchar *key, cchar *value)
{
    char    *ip;
    int     port;

    mprLog(4, "NameVirtual Host: %s ", value);
    mprParseIp(value, &ip, &port, -1);
    httpSetNamedVirtualServers(state->http, ip, port);
    return 0;
}


static int orderDirective(MaState *state, cchar *key, cchar *value)
{
    if (scasecmp(value, "Allow,Deny") == 0) {
        httpSetAuthOrder(state->auth, HTTP_ALLOW_DENY);
    } else {
        httpSetAuthOrder(state->auth, HTTP_DENY_ALLOW);
    }
    return 0;
}


static int protocolDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetHostProtocol(state->host, value);
    if (!scasesame(value, "HTTP/1.0") && !scasesame(value, "HTTP/1.1")) {
        mprError("Unknown http protocol %s. Should be HTTP/1.0 or HTTP/1.1", value);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int putMethodDirective(MaState *state, cchar *key, cchar *value)
{
    bool    on;

    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (on) {
        state->route->flags |= HTTP_ROUTE_PUT_DELETE;
    } else {
        state->route->flags &= ~HTTP_ROUTE_PUT_DELETE;
    }
    return 0;
}


static int redirectDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *alias;
    char        *code, *uri, *path;
    int         status;

    if (value[0] == '/' || sncmp(value, "http://", 6) == 0) {
        if (!maTokenize(state, value, "%S %S", &uri, &path)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        status = 302;
    } else if (isdigit((int) value[0])) {
        if (!maTokenize(state, value, "%N %S %S", &status, &uri, &path)) {
            return MPR_ERR_BAD_SYNTAX;
        }
    } else {
        if (!maTokenize(state, value, "%S %S %S", &code, &uri, &path)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (scasesame(code, "permanent")) {
            status = 301;
        } else if (scasesame(code, "temp")) {
            status = 302;
        } else if (scasesame(code, "seeother")) {
            status = 303;
        } else if (scasesame(code, "gone")) {
            status = 410;
        } else {
            return configError(state, key);
        }
    }
    if (status < 300 || status > 399) {
        path = "";
    }
    if (status <= 0 || uri == 0 || path == 0) {
        return configError(state, key);
    }
    alias = httpCreateAliasRoute(state->route, uri, path, status);
    httpFinalizeRoute(alias);
    httpAddRoute(state->host, alias);
    return 0;
}


static int requireDirective(MaState *state, cchar *key, cchar *value)
{
    char    *type, *rest;

    if (!maTokenize(state, value, "%S %*", &type, &rest)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasecmp(type, "acl") == 0) {
#if BLD_FEATURE_AUTH_FILE
        httpSetRequiredAcl(state->auth, httpParseAcl(state->auth, rest));
#else
        mprError("Acl directive not supported when --auth=pam");
#endif
    } else if (scasecmp(type, "valid-user") == 0) {
        httpSetAuthAnyValidUser(state->auth);
    } else if (scasecmp(type, "user") == 0) {
        httpSetAuthRequiredUsers(state->auth, rest);
    } else if (scasecmp(type, "group") == 0) {
        httpSetAuthRequiredGroups(state->auth, rest);
    } else {
        return configError(state, key);
    }
    return 0;
}


static int resetDirective(MaState *state, cchar *key, cchar *value)
{
    char    *kind;

    if (!maTokenize(state, value, "%S", &kind)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasesame(key, "routes")) {
        httpResetRoutes(state->host);
    } else if (scasesame(key, "pipeline")) {
        httpResetRoutePipeline(state->route);
    } else {
        return configError(state, key);
    }
    return 0;
}


static int routeDirective(MaState *state, cchar *key, cchar *value)
{
    state = pushState(state, key);
    state->route = httpCreateInheritedRoute(state->route);
    httpSetRouteHost(state->route, state->host);
    httpSetRoutePattern(state->route, value);
    httpAddRoute(state->host, state->route);
    state->auth = state->auth;
    return 0;
}


static int runHandlerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *when;

    if (!maTokenize(state, value, "%S %*", &name, &when)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasesame(value, "before")) {
        state->route->flags |= HTTP_ROUTE_BEFORE;
    } else if (scasesame(value, "after")) {
        state->route->flags |= HTTP_ROUTE_AFTER;
    } else if (scasesame(value, "smart")) {
        state->route->flags |= HTTP_ROUTE_SMART;
    } else {
        return configError(state, key);
    }
    return 0;
}


static int serverNameDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetHostName(state->host, strim(value, "http://", MPR_TRIM_START), -1);
    return 0;
}


static int serverRootDirective(MaState *state, cchar *key, cchar *value)
{
    char    *path;

    if (!maTokenize(state, value, "%T", &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    maSetMetaHome(state->meta, path);
    httpSetHostHome(state->host, path);
    httpSetRoutePathVar(state->route, "SERVER_ROOT", path);
    mprLog(MPR_CONFIG, "Server Root \"%s\"", path);
    return 0;
}


static int setConnectorDirective(MaState *state, cchar *key, cchar *value)
{
    if (httpSetRouteConnector(state->route, value) < 0) {
        mprError("Can't add handler %s", value);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int setHandlerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name;

    if (!maTokenize(state, value, "%S", &name)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpSetRouteHandler(state->route, name) < 0) {
        mprError("Can't add handler %s", name);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static int sourceDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteSource(state->route, value);
    return 0;
}


static int startThreadsDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMinWorkers(atoi(value));
    return 0;
}


/*
    Standard target values are
        "alias", "close", "dest", "file", "redirect", "virt"

    Target alias "${DOCUMENT_ROOT}"
    Target close [immediate]
    Target dest "${DOCUMENT_ROOT}/${request:rest}"
    Target redirect [status] URI            # Redirect to a new URI and re-route
    Target file "${DOCUMENT_ROOT}/${request:uri}"
    Target virt ${controller}-${name} 
 */
static int targetDirective(MaState *state, cchar *key, cchar *value)
{
    char    *kind, *details;

    if (!maTokenize(state, value, "%S %*", &kind, &details)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetRouteTarget(state->route, kind, details);
    return 0;
}


static int threadLimitDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMaxWorkers(atoi(value));
    return 0;
}


static int threadStackSizeDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetThreadStackSize(atoi(value));
    return 0;
}


static int timeoutDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->requestTimeout = atoi(value) * MPR_TICKS_PER_SEC;
    return 0;
}


static int traceMethodDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->enableTraceMethod = (scasecmp(value, "on") == 0);
    return 0;
}


static int typesConfigDirective(MaState *state, cchar *key, cchar *value)
{
    char    *path;

    path = httpMakePath(state->route, value);
    if ((state->host->mimeTypes = mprCreateMimeTypes(path)) == 0) {
        mprError("Can't open TypesConfig mime file %s", path);
        state->host->mimeTypes = mprCreateMimeTypes(NULL);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int unloadModuleDirective(MaState *state, cchar *key, cchar *value)
{
    MprModule   *module;
    char        *name;
    int         timeout;

    if (!maTokenize(state, value, "%S %N", &name, &timeout)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if ((module = mprLookupModule(name)) == 0) {
        mprError("Can't find module stage %s", name);
        return MPR_ERR_BAD_SYNTAX;
    }
    module->timeout = (int) stoi(value, 10, NULL) * MPR_TICKS_PER_SEC;
    return 0;
}


static int uploadDirDirective(MaState *state, cchar *key, cchar *value)
{
    state->route->uploadDir = httpMakePath(state->route, value);
    mprLog(MPR_CONFIG, "Upload directory: %s", state->route->uploadDir);
    return 0;
}


static int uploadAutoDeleteDirective(MaState *state, cchar *key, cchar *value)
{
    if (!maTokenize(state, value, "%B", &state->route->autoDelete)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int userDirective(MaState *state, cchar *key, cchar *value)
{
    maSetHttpUser(state->appweb, value);
    return 0;
}


static int virtualHostDirective(MaState *state, cchar *key, cchar *value)
{
    HttpServer  *server;
    HttpHost    *host;
    char        *ip;
    int         port;

    state = pushState(state, key);
    mprParseIp(value, &ip, &port, -1);
    host = state->host = httpCloneHost(host);
    httpSetHostName(host, ip, port);
    state->route = host->route;
    if ((server = httpLookupServer(state->http, ip, port)) == 0) {
        mprError("Can't find listen directive for virtual host %s", value);
        return MPR_ERR_BAD_SYNTAX;
    } else {
        httpAddHostToServer(server, host);
    }
    return 0;
}


int maValidateConfiguration(MaMeta *meta)
{
    MaAppweb        *appweb;
    Http            *http;
    HttpHost        *host, *defaultHost;
    HttpServer      *server;
    int             nextHost, nextServer;

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
        if (host->home == 0) {
            httpSetHostHome(host, defaultHost->home);
        }
        if (host->mimeTypes == 0) {
            host->mimeTypes = defaultHost->mimeTypes;
        }
        mprLog(MPR_CONFIG, "Host \"%s\"", host->name);
        mprLog(MPR_CONFIG, "    ServerRoot \"%s\"", host->home);
    }
    return 1;
}


static bool conditionalDefinition(cchar *key)
{
    //  TODO - should always create a conditional directive when a module is loaded. Even for user modules.
    if (scasesame(key, "BLD_COMMERCIAL")) {
        return strcmp(BLD_COMMERCIAL, "0");

#ifdef BLD_DEBUG
    } else if (scasesame(key, "BLD_DEBUG")) {
        return BLD_DEBUG;
#endif

    } else if (scasesame(key, "CGI_MODULE")) {
        return BLD_FEATURE_CGI;

    } else if (scasesame(key, "DIR_MODULE")) {
        return BLD_FEATURE_DIR;

    } else if (scasesame(key, "EJS_MODULE")) {
        return BLD_FEATURE_EJSCRIPT;

    } else if (scasesame(key, "ESP_MODULE")) {
        return BLD_FEATURE_ESP;

    } else if (scasesame(key, "PHP_MODULE")) {
        return BLD_FEATURE_PHP;

    } else if (scasesame(key, "SSL_MODULE")) {
        return BLD_FEATURE_SSL;

    } else if (scasesame(key, BLD_OS)) {
        return 1;

    } else if (scasesame(key, BLD_CPU)) {
        return 1;
    }
    return 0;
}


/*
    MOB - must support quoted args

    Tokenizes a line using the tokens:
        %B - Boolean. Parses: on/off, true/false, yes/no.
        %N - Number. Parses numbers in base 10.
        %S - String. Removes quotes.
        %P - Template path string. Removes quotes and expands ${PathVars}.
        %W - Parse words into a list
        %! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.
 */
bool maTokenize(MaState *state, cchar *line, cchar *fmt, ...)
{
    va_list     args;
    MprList     *list;
    cchar       *f;
    char        *tok, *etok, *value, *word, *end;
    int         quote;

    mprAssert(fmt && *fmt);
    va_start(args, fmt);

    quote = 0;
    tok = sclone(line);
    end = &tok[slen(line)];

    for (f = fmt; *f && tok < end; f++) {
        for (; isspace((int) *tok); tok++) ;
        if (*tok == '\0' || *tok == '#') {
            break;
        }
        if (isspace((int) *f)) {
            continue;
        }
        if (*f == '%') {
            f++;
            if (*tok == '"' || *tok == '\'') {
                quote = *tok;
            }
            if (quote) {
                for (etok = ++tok; *etok && (*etok != quote && etok[-1] != '\\'); etok++) ; 
                *etok++ = '\0';
                quote = 0;
            } else if (*f == '*') {
                for (etok = tok; *etok; etok++) {
                    if (*etok == '#') {
                        *etok = '\0';
                    }
                }
            } else {
                for (etok = tok; !isspace((int) *etok); etok++) ;
            }
            *etok++ = '\0';

            if (*f == '*') {
                * va_arg(args, char**) = sclone(tok);
                tok = etok;
                break;
            }

            switch (*f) {
            case '!':
                if (*tok == '!') {
                    *va_arg(args, int*) = HTTP_ROUTE_NOT;
                } else {
                    *va_arg(args, int*) = 0;
                    continue;
                }
                break;
            case 'B':
                if (scasecmp(tok, "on") == 0 || scasecmp(tok, "true") == 0 || scasecmp(tok, "yes") == 0) {
                    *va_arg(args, bool*) = 1;
                } else {
                    *va_arg(args, bool*) = 0;
                }
                break;
            case 'N':
                *va_arg(args, int*) = (int) stoi(tok, 10, 0);
                break;
            case 'S':
                *va_arg(args, char**) = strim(tok, "\"", MPR_TRIM_BOTH);
                break;
            case 'T':
                value = strim(tok, "\"", MPR_TRIM_BOTH);
                *va_arg(args, char**) = stemplate(value, state->route->pathVars);
                break;
            case 'W':
                list = va_arg(args, MprList*);
                word = stok(tok, " \t\r\n", &tok);
                while (word) {
                    mprAddItem(list, word);
                    word = stok(0, " \t\r\n", &tok);
                }
                break;
            default:
                mprError("Unknown token pattern %%\"%c\"", *f);
                break;
            }
            tok = etok;
        }
    }
    if (*tok) {
        mprError("Bad directive \"%s\"\nAt line %d in %s\n\n", line, state->lineNumber, state->filename);
        return 0;
    }
    va_end(args);
    return 1;
}


static MaState *createState(MaMeta *meta)
{
    MaState     *state;

    if ((state = mprAllocObj(MaState, manageState)) == 0) {
        return 0;
    }
    state->meta = meta;
    state->appweb = meta->appweb;
    state->http = meta->http;
    state->host = meta->defaultHost;
    state->limits = state->host->limits;
    state->route = meta->defaultHost->route;
    state->route->connector = meta->http->netConnector;
    state->enabled = 1;
    state->lineNumber = 0;
    state->auth = state->route->auth;
    state->currentState = state;
    return state;
}


static MaState *pushState(MaState *prev, cchar *block)
{
    MaState   *state;

    if ((state = mprAllocObj(MaState, manageState)) == 0) {
        return 0;
    }
    state->prev = prev;
    prev->currentState = state;
    state->currentState = state;
    state->appweb = prev->appweb;
    state->http = prev->http;
    state->meta = prev->meta;
    state->host = prev->host;
    state->route = prev->route;
    state->lineNumber = prev->lineNumber;
    state->enabled = prev->enabled;
    state->filename = prev->filename;
    state->file = prev->file;
    state->limits = prev->limits;
    if (block) {
        state->currentBlock = sclone(&block[1]);
    } else {
        state->currentBlock = prev->currentBlock;
    }
    state->auth = state->route->auth;
    return state;
}


static void manageState(MaState *state, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
		mprMark(state->appweb);
		mprMark(state->http);
		mprMark(state->meta);
		mprMark(state->host);
		mprMark(state->route);
		mprMark(state->auth);
		mprMark(state->file);
		mprMark(state->limits);
		mprMark(state->configDir);
		mprMark(state->filename);
		mprMark(state->currentBlock);
		mprMark(state->prev);
		mprMark(state->currentState);
    }
}


static int configError(MaState *state, cchar *key)
{
    mprError("Bad directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
    return MPR_ERR_BAD_SYNTAX;
}


static MaState *popState(MaState *state)
{
    if (state->prev == 0) {
        mprError("Too many closing blocks.\nAt line %d in %s\n\n", state->lineNumber, state->filename);
    }
    return state->currentState = state->prev;
}


static bool closingBlock(MaState *state, cchar *key)
{
    if (key == 0 || *key == '\0') {
        return 0;
    }
    return scasesame(key, state->currentBlock);
}


void maAddDirective(MaAppweb *appweb, cchar *directive, MaDirective proc)
{
    mprAddKey(appweb->directives, directive, proc);
}


int maParseInit(MaAppweb *appweb)
{
    if ((appweb->directives = mprCreateHash(-1, 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    maAddDirective(appweb, "AccessLog", accessLogDirective);
    maAddDirective(appweb, "AddLanguage", addLanguageDirective);
    maAddDirective(appweb, "AddLanguageRoot", addLanguageRootDirective);
    maAddDirective(appweb, "AddFilter", addFilterDirective);
    maAddDirective(appweb, "AddInputFilter", addInputFilterDirective);
    maAddDirective(appweb, "AddOutputFilter", addOutputFilterDirective);
    maAddDirective(appweb, "AddHandler", addHandlerDirective);
    maAddDirective(appweb, "AddType", addTypeDirective);
    maAddDirective(appweb, "Alias", aliasDirective);
    maAddDirective(appweb, "Allow", allowDirective);
    maAddDirective(appweb, "AuthGroupFile", authGroupFileDirective);
    maAddDirective(appweb, "AuthMethod", authMethodDirective);
    maAddDirective(appweb, "AuthName", authNameDirective);
    maAddDirective(appweb, "AuthRealm", authRealmDirective);
    maAddDirective(appweb, "AuthUserFile", authUserFileDirective);
    maAddDirective(appweb, "AuthDigestQop", authDigestQopDirective);
    maAddDirective(appweb, "ClientCache", clientCacheDirective);
    maAddDirective(appweb, "CacheByType", cacheByTypeDirective);
    maAddDirective(appweb, "Chroot", chrootDirective);
    maAddDirective(appweb, "CustomLog", customLogDirective);
    maAddDirective(appweb, "Condition", conditionDirective);
    maAddDirective(appweb, "Deny", denyDirective);
    maAddDirective(appweb, "DirectoryIndex", directoryIndexDirective);
    maAddDirective(appweb, "DocumentRoot", documentRootDirective);
    maAddDirective(appweb, "<Directory", directoryDirective);
    maAddDirective(appweb, "</Directory", closeDirective);
    maAddDirective(appweb, "ErrorDocument", errorDocumentDirective);
    maAddDirective(appweb, "ErrorLog", errorLogDirective);
    maAddDirective(appweb, "Field", fieldDirective);
    maAddDirective(appweb, "Group", groupDirective);
    maAddDirective(appweb, "Header", headerDirective);
    maAddDirective(appweb, "<If", ifDirective);
    maAddDirective(appweb, "</If", closeDirective);
    maAddDirective(appweb, "<Include", includeDirective);
    maAddDirective(appweb, "KeepAlive", keepAliveDirective);
    maAddDirective(appweb, "KeepAliveTimeout", keepAliveTimeoutDirective);

    /* TODO Deprecated */
    maAddDirective(appweb, "<Location", routeDirective);
    maAddDirective(appweb, "</Location", closeDirective);
    maAddDirective(appweb, "LimitChunkSize", limitChunkSizeDirective);
    maAddDirective(appweb, "LimitClients", limitClientsDirective);
    maAddDirective(appweb, "LimitMemoryMax", limitMemoryMaxDirective);
    maAddDirective(appweb, "LimitMemoryRedline", limitMemoryRedlineDirective);
    maAddDirective(appweb, "LimitRequestBody", limitRequestBodyDirective);
    maAddDirective(appweb, "LimitRequestFields", limitRequestFieldsDirective);

    /* Deprecated */
    maAddDirective(appweb, "LimitRequestHeaderCount", limitRequestFieldsDirective);
    maAddDirective(appweb, "LimitRequestFieldSize", limitRequestFieldSizeDirective);

    /* Deprecated */
    maAddDirective(appweb, "LimitRequestHeaderSize", limitRequestFieldSizeDirective);
    maAddDirective(appweb, "LimitResponseBody", limitResponseBodyDirective);
    maAddDirective(appweb, "LimitStageBuffer", limitStageBufferDirective);
    maAddDirective(appweb, "LimitUri", limitUriDirective);

    /* Deprecated */
    maAddDirective(appweb, "LimitUrl", limitUriDirective);
    maAddDirective(appweb, "LimitUploadSize", limitUploadSizeDirective);
    maAddDirective(appweb, "Listen", listenDirective);

//  MOB - not a great name "Load"
    maAddDirective(appweb, "Load", loadDirective);
    maAddDirective(appweb, "LogLevel", logLevelDirective);
    maAddDirective(appweb, "LogRotation", logRotationDirective);
    maAddDirective(appweb, "LogRoutes", logRoutesDirective);
    maAddDirective(appweb, "LogTrace", logTraceDirective);
    maAddDirective(appweb, "LogTraceFilter", logTraceFilterDirective);
    maAddDirective(appweb, "LoadModulePath", loadModulePathDirective);
    maAddDirective(appweb, "LoadModule", loadModuleDirective);
    maAddDirective(appweb, "MaxKeepAliveRequests", maxKeepAliveRequestsDirective);
    maAddDirective(appweb, "MemoryDepletionPolicy", memoryDepletionPolicyDirective);
    maAddDirective(appweb, "Methods", methodsDirective);
    maAddDirective(appweb, "NameVirtualHost", nameVirtualHostDirective);
    maAddDirective(appweb, "Order", orderDirective);
    maAddDirective(appweb, "Protocol", protocolDirective);
    maAddDirective(appweb, "PutMethod", putMethodDirective);
    maAddDirective(appweb, "Redirect", redirectDirective);
    maAddDirective(appweb, "Require", requireDirective);
    maAddDirective(appweb, "Reset", resetDirective);
    maAddDirective(appweb, "<Route", routeDirective);
    maAddDirective(appweb, "</Route", closeRouteDirective);
    maAddDirective(appweb, "RunHandler", runHandlerDirective);
    maAddDirective(appweb, "ServerName", serverNameDirective);
    maAddDirective(appweb, "ServerRoot", serverRootDirective);
    maAddDirective(appweb, "SetConnector", setConnectorDirective);
    maAddDirective(appweb, "SetHandler", setHandlerDirective);
    maAddDirective(appweb, "Source", sourceDirective);
    maAddDirective(appweb, "StartThreads", startThreadsDirective);
    maAddDirective(appweb, "Target", targetDirective);
    maAddDirective(appweb, "Timeout", timeoutDirective);
    maAddDirective(appweb, "ThreadLimit", threadLimitDirective);
    maAddDirective(appweb, "ThreadStackSize", threadStackSizeDirective);
    maAddDirective(appweb, "TraceMethod", traceMethodDirective);
    maAddDirective(appweb, "TypesConfig", typesConfigDirective);
    maAddDirective(appweb, "Update", updateDirective);
    maAddDirective(appweb, "UnloadModule", unloadModuleDirective);
    maAddDirective(appweb, "UploadAutoDelete", uploadAutoDeleteDirective);
    maAddDirective(appweb, "UploadDir", uploadDirDirective);
    maAddDirective(appweb, "User", userDirective);

    maAddDirective(appweb, "<VirtualHost", virtualHostDirective);
    maAddDirective(appweb, "</VirtualHost", closeDirective);
    return 0;
}


static char *getSearchPath(cchar *dir)
{
#if WIN
        return sfmt("%s" MPR_SEARCH_SEP ".", dir);
#else
        char *libDir = mprJoinPath(mprGetPathParent(dir), BLD_LIB_NAME);
        return sfmt("%s" MPR_SEARCH_SEP "%s" MPR_SEARCH_SEP ".", dir,
            mprSamePath(BLD_BIN_PREFIX, dir) ? BLD_LIB_PREFIX: libDir);
#endif
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
