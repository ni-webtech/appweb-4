/**
    config.c - Parse the configuration file.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"
#include    "pcre.h"

/***************************** Forward Declarations ****************************/

static int addCondition(MaState *state, cchar *condition, int flags);
static int addUpdate(MaState *state, cchar *kind, cchar *details, int flags);
static bool conditionalDefinition(cchar *key);
static int configError(MaState *state, cchar *key);
static MaState *createState(MaServer *server, HttpHost *host, HttpRoute *route);
static void manageState(MaState *state, int flags);
static int parseFile(MaState *state, cchar *path);
static int parseFileInner(MaState *state, cchar *path);
static int setTarget(MaState *state, cchar *kind, cchar *details);

/******************************************************************************/

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


int maParseConfig(MaServer *server, cchar *path)
{
    MaState     *state;
    HttpHost    *host;
    HttpRoute   *route;

    mprLog(2, "Config File %s", path);

    /*
        Create top level host and route
     */
    host = httpCreateHost();
    route = httpCreateRoute(host);

    state = createState(server, host, route);
    if (parseFile(state, path) < 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (!maValidateConfiguration(server)) {
        return MPR_ERR_BAD_ARGS;
    }
    httpFinalizeRoute(state->route);
    if (mprHasMemError()) {
        mprError("Memory allocation error when initializing");
        return MPR_ERR_MEMORY;
    }
    return 0;
}


/*
    Get the directive and value details. Return key and *valuep.
 */
static char *getDirective(char *line, char **valuep)
{
    char    *key, *value;
    ssize   len;
    
    *valuep = 0;
    key = stok(line, " \t", &value);
    key = strim(key, ">", MPR_TRIM_END);
    if (value) {
        value = strim(value, " \t\r\n>", MPR_TRIM_END);
        /*
            Trim quotes if wrapping the entire value. Preserve embedded quotes and leading/trailing "" etc.
         */
        len = slen(value);
        if (*value == '\"' && value[len - 1] == '\"' && len > 2 && value[1] != '\"') {
            value = snclone(&value[1], len - 2);
        }
        *valuep = value;
    }
    return key;
}


static int parseFile(MaState *state, cchar *path)
{
    int     rc;

    if ((state = maPushState(state)) == 0) {
        return 0;
    }
    mprAssert(state == state->top->current);
    
    rc = parseFileInner(state, path);
    state->lineNumber = state->prev->lineNumber;
    state = maPopState(state);
    mprAssert(state->top->current == state);
    return rc;
}


static int parseFileInner(MaState *state, cchar *path)
{
    MaDirective *directive;
    char        *tok, *key, *line, *value;
    
    if (maOpenConfig(state, path) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    for (state->lineNumber = 1; (line = mprGetFileString(state->file, 0, NULL)) != 0; state->lineNumber++) {
        for (tok = line; isspace((int) *tok); tok++) ;
        if (*tok == '\0' || *tok == '#') {
            continue;
        }
        state->key = 0;
        key = getDirective(line, &value);
        if (!state->enabled && key[0] != '<') {
            //MOB 8
            mprLog(8, "Skip: %s %s", key, value);
            continue;
        }
        if ((directive = mprLookupKey(state->appweb->directives, key)) == 0) {
            mprError("Unknown directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
            return MPR_ERR_BAD_SYNTAX;
        }
        state->key = key;
        mprLog(8, "Line %d, Parse %s %s", state->lineNumber, key, value ? value : "");
        if ((*directive)(state, key, value) < 0) {
            mprError("Bad directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
            return MPR_ERR_BAD_SYNTAX;
        }
        state = state->top->current;
    }
    /* EOF */
    if (state->prev && state->file == state->prev->file) {
        mprError("Unclosed directives in %s", state->filename);
        while (state->prev && state->file == state->prev->file) {
            state = state->prev;
        }
    }
    mprCloseFile(state->file);
    return 0;
}


/*
    AccessLog path
    TODO MOB: AccessLog conf/log.conf append,rotate,limit=10K
 */
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


/*
    AddFilter filter [ext ext ext ...]
 */
static int addFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *filter, *extensions;

    if (!maTokenize(state, value, "%S ?*", &filter, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteFilter(state->route, filter, extensions, HTTP_STAGE_RX | HTTP_STAGE_TX) < 0) {
        mprError("Can't add filter %s", filter);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    AddInputFilter filter [ext ext ext ...]
 */
static int addInputFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *filter, *extensions;

    if (!maTokenize(state, value, "%S ?*", &filter, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteFilter(state->route, filter, extensions, HTTP_STAGE_RX) < 0) {
        mprError("Can't add filter %s", filter);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    AddLanguage lang ext [position]
    AddLanguage en .en before
 */
static int addLanguageDirective(MaState *state, cchar *key, cchar *value)
{
    char    *lang, *ext, *position;
    int     flags;

    if (!maTokenize(state, value, "%S %S ?S", &lang, &ext, &position)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    flags = 0;
    if (scasematch(position, "after")) {
        flags |= HTTP_LANG_AFTER;
    } else if (scasematch(position, "before")) {
        flags |= HTTP_LANG_BEFORE;
    }
    httpAddRouteLanguage(state->route, lang, ext, flags);
    return 0;
}


/*
    AddLanguageRoot lang path
 */
static int addLanguageRootDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *route;
    char        *lang, *path;

    route = state->route;
    if (!maTokenize(state, value, "%S %S", &lang, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if ((path = stemplate(path, route->pathVars)) == 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (mprIsRelPath(path)) {
        path = mprJoinPath(route->dir, path);
    }
    httpAddRouteLanguageRoot(route, lang, mprGetAbsPath(path));
    return 0;
}


/*
    AddOutputFilter filter [ext ext ...]
 */
static int addOutputFilterDirective(MaState *state, cchar *key, cchar *value)
{
    char    *filter, *extensions;

    if (!maTokenize(state, value, "%S ?*", &filter, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteFilter(state->route, filter, extensions, HTTP_STAGE_TX) < 0) {
        mprError("Can't add filter %s", filter);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    AddHandler handler [ext ext ...]
 */
static int addHandlerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *handler, *extensions;

    if (!maTokenize(state, value, "%S ?*", &handler, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (httpAddRouteHandler(state->route, handler, extensions) < 0) {
        mprError("Can't add handler %s", handler);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    AddType mimeType ext
 */
static int addTypeDirective(MaState *state, cchar *key, cchar *value)
{
    char    *ext, *mimeType;

    if (!maTokenize(state, value, "%S %S", &mimeType, &ext)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    mprAddMime(state->host->mimeTypes, mimeType, ext);
    return 0;
}


/*
    Alias /uriPrefix /path
 */
static int aliasDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *alias;
    MprPath     info;
    char        *prefix, *path;

    if (!maTokenize(state, value, "%S %P", &prefix, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    mprGetPathInfo(path, &info);
    if (info.isDir) {
        alias = httpCreateAliasRoute(state->route, prefix, path, 0);
        httpSetRoutePattern(alias, sfmt("^%s(.*)$", prefix), 0);
        httpSetRouteTarget(alias, "file", "$1");
    } else {
        alias = httpCreateAliasRoute(state->route, prefix, 0, 0);
        httpSetRouteTarget(alias, "file", path);
    }
    httpFinalizeRoute(alias);
    return 0;
}


/*
    Allow
 */
static int allowDirective(MaState *state, cchar *key, cchar *value)
{
    char    *from, *spec;

    if (!maTokenize(state, value, "%S %S", &from, &spec)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetAuthAllow(state->auth, spec);
    return addCondition(state, "allowDeny", 0);
}


/*
    AuthGroupFile path
 */
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


/*
    AuthMethod pam|file
 */
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


/*
    AuthName realm
 */
static int authNameDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetAuthRealm(state->auth, value);
    return 0;
}


/*
    AuthType basic|digest|none
 */
static int authTypeDirective(MaState *state, cchar *key, cchar *value)
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
    return addCondition(state, "auth", 0);
}


/*
    AuthUserFile path
 */
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


/*
    AuthDigestQop none|auth|auth-int
 */
static int authDigestQopDirective(MaState *state, cchar *key, cchar *value)
{
    if (!scasematch(value, "none") && !scasematch(value, "auth") && !scasematch(value, "auth-int")) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetAuthQop(state->auth, value);
    return 0;
}


/*
    ClientCache timeout ext ...
 */
static int clientCacheDirective(MaState *state, cchar *key, cchar *value)
{
    char    *when, *extensions;

    if (!maTokenize(state, value, "%S %*", &when, &extensions)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteExpiry(state->route, (MprTime) stoi(when, 10, NULL), extensions);
    return 0;
}


/*
    ClientCacheByType timeout mimeTypes ...
 */
static int clientCacheByTypeDirective(MaState *state, cchar *key, cchar *value)
{
    char    *when, *mimeTypes;

    if (!maTokenize(state, value, "%S %S", &when, &mimeTypes)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteExpiryByType(state->route, (MprTime) stoi(when, 10, NULL), mimeTypes);
    return 0;
}


/*
    Chroot path
 */
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


/*
    </Route>, </Directory>, </VirtualHost>, </If>
 */
static int closeDirective(MaState *state, cchar *key, cchar *value)
{
    /*
        The order of route finalization will be from the inside. Route finalization causes the route to be added
        to the enclosing host. This ensures that nested routes are defined BEFORE outer/enclosing routes.
     */
    if (state->route != state->prev->route) {
        httpFinalizeRoute(state->route);
    }
    maPopState(state);
    return 0;
}


/*
    Compress [gzip|none]
 */
static int compressDirective(MaState *state, cchar *key, cchar *value)
{
    char    *format;

    if (!maTokenize(state, value, "%S", &format)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasematch(format, "gzip")) {
        httpSetRouteCompression(state->route, HTTP_ROUTE_GZIP);

    } else if (scasematch(format, "none") || scasematch(format, "off")) {
        httpSetRouteCompression(state->route, 0);
    }
    return 0;
}


/*
    Condition [!] condition

    Condition [!] exists string
    Condition [!] directory string
    Condition [!] match pattern string

    Strings can contain route->pattern and request ${tokens}
 */
static int conditionDirective(MaState *state, cchar *key, cchar *value)
{
    char    *condition;
    int     not;

    if (!maTokenize(state, value, "%! %*", &not, &condition)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return addCondition(state, condition, not | HTTP_ROUTE_STATIC_VALUES);
}


/*
    CustomLog path format
 */
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


static int defaultLanguageDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteDefaultLanguage(state->route, value);
    return 0;
}


/*
    Deny "from" address
 */
static int denyDirective(MaState *state, cchar *key, cchar *value)
{
    char *from, *spec;

    if (!maTokenize(state, value, "%S %S", &from, &spec)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetAuthDeny(state->auth, spec);
    return addCondition(state, "allowDeny", 0);
}


/*
    <Directory path>
 */
static int directoryDirective(MaState *state, cchar *key, cchar *value)
{
    mprError("The <Directory> directive is deprecated. Use <Route> with a DocumentRoot instead.");
    return MPR_ERR_BAD_SYNTAX;
}


/*
    DirectoryIndex path
 */
static int directoryIndexDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteIndex(state->route, value);
    return 0;
}


/*
    DocumentRoot path
 */
static int documentRootDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteDir(state->route, value);
    return 0;
}


/*
    ErrorDocument status URI
 */
static int errorDocumentDirective(MaState *state, cchar *key, cchar *value)
{
    char    *status, *uri;

    if (!maTokenize(state, value, "%S %S", &status, &uri)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteErrorDocument(state->route, status, uri);
    return 0;
}


/*
    ErrorLog logSpec
 */
static int errorLogDirective(MaState *state, cchar *key, cchar *value)
{
    if (state->server->alreadyLogging) {
        mprLog(4, "Already logging. Ignoring ErrorLog directive");
        return 0;
    }
    maStopLogging(state->server);
    if (sncmp(value, "stdout", 6) != 0 && sncmp(value, "stderr", 6) != 0) {
        value = httpMakePath(state->route, value);
    }
    if (maStartLogging(state->host, value) < 0) {
        mprError("Can't write to ErrorLog: %s", value);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


/*
    Query [!] name value
 */
static int queryDirective(MaState *state, cchar *key, cchar *value)
{
    char    *field;
    int     not;

    if (!maTokenize(state, value, "?! %S %*", &not, &field, &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteQuery(state->route, field, value, not);
    return 0;
}


/*
    Group groupName
 */
static int groupDirective(MaState *state, cchar *key, cchar *value)
{
    maSetHttpGroup(state->appweb, value);
    return 0;
}


/*
    Header [!] name value
 */
static int headerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *header;
    int     not;

    if (!maTokenize(state, value, "?! %S %*", &not, &header, &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteHeader(state->route, header, value, not);
    return 0;
}


/*
    <Include pattern>
 */
static int includeDirective(MaState *state, cchar *key, cchar *value)
{
    MprList         *includes;
    MprDirEntry     *dp;
    void            *compiled;
    cchar           *errMsg;
    char            *path, *pattern;
    int             matches[4 * 3], next, count, column;

    if (!maTokenize(state, value, "%P", &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (strpbrk(value, "^$*+?([|{") == 0) {
        if (parseFile(state, value) < 0) {
            return MPR_ERR_CANT_OPEN;
        }
    } else {
        /*
            Convert glob style to regexp
         */
        path = mprGetPathDir(mprJoinPath(state->server->home, value));
        pattern = mprGetPathBase(value);
        pattern = sreplace(pattern, ".", "\\.");
        pattern = sreplace(pattern, "*", ".*");
        pattern = sjoin("^", pattern, "$", 0);
        if ((includes = mprGetPathFiles(path, 0)) != 0) {
            if ((compiled = pcre_compile2(pattern, 0, 0, &errMsg, &column, NULL)) == 0) {
                mprError("Can't compile include pattern. Error %s at column %d", errMsg, column);
                return 0;
            }
            for (next = 0; (dp = mprGetNextItem(includes, &next)) != 0; ) {
                count = pcre_exec(compiled, NULL, dp->name, (int) slen(dp->name), 0, 0, matches, 
                    sizeof(matches) / sizeof(int));
                if (count > 0) {
                    if (parseFile(state, mprJoinPath(path, dp->name)) < 0) {
                        return MPR_ERR_CANT_OPEN;
                    }
                }
            }
        }
    }
    return 0;
}


/*
    <If DEFINITION>
 */
static int ifDirective(MaState *state, cchar *key, cchar *value)
{
    state = maPushState(state);
    if (state->enabled) {
        state->enabled = conditionalDefinition(value);
        if (!state->enabled) {
            mprLog(7, "If \"%s\" conditional is false at %s:%d", value, state->filename, state->lineNumber);
        }
    }
    return 0;
}


/*
    KeepAlive on|off
 */
static int keepAliveDirective(MaState *state, cchar *key, cchar *value)
{
    bool    on;

    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    state->host->limits->keepAliveCount = on;
    return 0;
}


/*
    KeepAliveTimeout secs
 */
static int keepAliveTimeoutDirective(MaState *state, cchar *key, cchar *value)
{
    if (! mprGetDebugMode()) {
        state->host->limits->inactivityTimeout = atoi(value) * MPR_TICKS_PER_SEC;
    }
    return 0;
}


/*
    LimitChunkSize bytes
 */
static int limitChunkSizeDirective(MaState *state, cchar *key, cchar *value)
{
    //  MOB - API for this
    state->host->limits->chunkSize = atoi(value);
    return 0;
}


/*
    LimitClients count
 */
static int limitClientsDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMaxSocketClients(atoi(value));
    return 0;
}


/*
    LimitMemoryMax bytes
 */
static int limitMemoryMaxDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMemLimits(-1, (ssize) stoi(value, 10, 0));
    return 0;
}


/*
    LimitMemoryRedline bytes
 */
static int limitMemoryRedlineDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMemLimits((ssize) stoi(value, 10, 0), -1);
    return 0;
}


/*
    LimitRequestBody bytes
 */
static int limitRequestBodyDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->receiveBodySize = stoi(value, 10, 0);
    return 0;
}


/*
    LimitRequestHeaderCount count
 */
static int limitRequestHeaderCountDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->headerCount = atoi(value);
    return 0;
}


/*
    LimitRequestHeaderSize bytes
 */
static int limitRequestHeaderSizeDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->headerSize = atoi(value);
    return 0;
}


/*
    LimitResponsebody bytes
 */
static int limitResponseBodyDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->transmissionBodySize = stoi(value, 10, 0);
    return 0;
}


/*
    LimitStageBuffer bytes
 */
static int limitStageBufferDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->stageBufferSize = atoi(value);
    return 0;
}


/*
    LimitUri bytes
 */
static int limitUriDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->uriSize = atoi(value);
    return 0;
}


/*
    LimitUploadSize bytes
 */
static int limitUploadSizeDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->uploadSize = stoi(value, 10, 0);
    return 0;
}


/*
    Listen ip:port
    Listen ip
    Listen port

    Where ip may be "::::::" for ipv6 addresses or may be enclosed in "[]" if appending a port.
 */
static int listenDirective(MaState *state, cchar *key, cchar *value)
{
    HttpEndpoint    *endpoint;
    char            *ip, *cp, *vp;
    ssize           len;
    int             port, colonCount;

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
    endpoint = httpCreateEndpoint(ip, port, NULL);
    endpoint->limits = state->limits;
    mprAddItem(state->server->endpoints, endpoint);
    return 0;
}


/*
    Load name path
 */
//  MOB - complete
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


/*
    LogLevel 0-9
 */
static int logLevelDirective(MaState *state, cchar *key, cchar *value)
{
    if (state->server->alreadyLogging) {
        mprLog(4, "Already logging. Ignoring LogLevel directive");
    } else {
        mprSetLogLevel(atoi(value));
    }
    return 0;
}


/*
    LogRotation count size
 */
//  MOB - redesign
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


/*
    LogRoutes 
 */
static int logRoutesDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *rp;
    int         next;

    mprLog(0, "HTTP Routes for URI %s", state->route->pattern);
    for (next = 0; (rp = mprGetNextItem(state->host->routes, &next)) != 0; ) {
        if (rp->target) {
            mprLog(0, "  %-12s %-16s %-30s %-14s", rp->name, rp->methods ? rp->methods : "", rp->pattern, rp->target);
        } else {
            mprLog(0, "  %-12s %-16s %-30s", rp->name, rp->methods ? rp->methods : "", rp->pattern);
        }
    }
    return 0;
}


/*
    LogTrace level items
 */
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


/*
    LogTraceFilter size include exclude
 */
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


/*
    LoadModulePath searchPath
 */
static int loadModulePathDirective(MaState *state, cchar *key, cchar *value)
{
    char    *sep, *lib, *path;

    sep = MPR_SEARCH_SEP;
    lib = mprJoinPath(mprGetPathParent(mprGetAppDir()), BLD_LIB_NAME);
    path = sjoin(value, sep, mprGetAppDir(), sep, lib, sep, BLD_LIB_PREFIX, NULL);
    mprSetModuleSearchPath(path);
    return 0;
}


/*
    LoadModule name path
 */
static int loadModuleDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *path;

    if (!maTokenize(state, value, "%S %S", &name, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (maLoadModule(state->appweb, name, path) < 0) {
        /*  Error messages already done */
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    MaxKeepAliveRequests count
 */
static int maxKeepAliveRequestsDirective(MaState *state, cchar *key, cchar *value)
{
    state->host->limits->keepAliveCount = atoi(value);
    return 0;
}


/*
    MemoryDepletionPolicy exit|restart
 */
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


/*
    Methods method method ...
 */
static int methodsDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteMethods(state->route, value);
    return 0;
}


/*
   Update field var value
   Update cmd commandLine
 */
static int updateDirective(MaState *state, cchar *key, cchar *value)
{
    char    *kind, *rest;

    if (!maTokenize(state, value, "%S %*", &kind, &rest)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return addUpdate(state, kind, rest, 0);
}


/*
    NameVirtualHost ip[:port]
 */
static int nameVirtualHostDirective(MaState *state, cchar *key, cchar *value)
{
    char    *ip;
    int     port;

    mprLog(4, "NameVirtual Host: %s ", value);
    mprParseIp(value, &ip, &port, -1);
    httpSetNamedVirtualEndpoints(state->http, ip, port);
    return 0;
}


/*
    Order Allow,Deny
    Order Deny,Allow
 */
static int orderDirective(MaState *state, cchar *key, cchar *value)
{
    if (scasecmp(value, "Allow,Deny") == 0) {
        httpSetAuthOrder(state->auth, HTTP_ALLOW_DENY);
    } else if (scasecmp(value, "Deny,Allow") == 0) {
        httpSetAuthOrder(state->auth, HTTP_DENY_ALLOW);
    } else {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


/*
    Protocol HTTP/1.0
    Protocol HTTP/1.1
 */
static int protocolDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetHostProtocol(state->host, value);
    if (!scasematch(value, "HTTP/1.0") && !scasematch(value, "HTTP/1.1")) {
        mprError("Unknown http protocol %s. Should be HTTP/1.0 or HTTP/1.1", value);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


/*
    PutMethod on|off
 */
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


/*
    Redirect [status|permanent|temp|seeother|gone] prefix path
 */
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
        if (scasematch(code, "permanent")) {
            status = 301;
        } else if (scasematch(code, "temp")) {
            status = 302;
        } else if (scasematch(code, "seeother")) {
            status = 303;
        } else if (scasematch(code, "gone")) {
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
    return 0;
}


/*
    Require acl|valid-user|user|group details
 */
static int requireDirective(MaState *state, cchar *key, cchar *value)
{
    char    *type, *rest;

    if (!maTokenize(state, value, "%S ?*", &type, &rest)) {
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


/*
    Reset routes
    Reset pipeline
 */
static int resetDirective(MaState *state, cchar *key, cchar *value)
{
    char    *kind;

    if (!maTokenize(state, value, "%S", &kind)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasematch(key, "routes")) {
        httpResetRoutes(state->host);

    } else if (scasematch(key, "pipeline")) {
        httpResetRoutePipeline(state->route);

    } else {
        return configError(state, key);
    }
    return 0;
}


/*
    ResetPipeline (alias for Reset routes)
 */
static int resetPipelineDirective(MaState *state, cchar *key, cchar *value)
{
    httpResetRoutePipeline(state->route);
    return 0;
}


/*
    <Route pattern>
 */
static int routeDirective(MaState *state, cchar *key, cchar *value)
{
    char    *pattern;
    int     not;

    state = maPushState(state);
    if (state->enabled) {
        if (!maTokenize(state, value, "%!%S", &not, &pattern)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        state->route = httpCreateInheritedRoute(state->route);
        httpSetRoutePattern(state->route, pattern, not);
        httpSetRouteHost(state->route, state->host);
        /* Routes are added when the route block is closed (see closeDirective) */
        state->auth = state->route->auth;
    }
    return 0;
}


/*
    RouteName name
 */
static int routeNameDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteName(state->route, value);
    return 0;
}


/*
    Prefix /URI-PREFIX
 */
static int prefixDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRoutePrefix(state->route, value);
    return 0;
}


/*
    ServerName URI
 */
static int serverNameDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetHostName(state->host, strim(value, "http://", MPR_TRIM_START));
    return 0;
}


/*
    ServerRoot path
 */
static int serverRootDirective(MaState *state, cchar *key, cchar *value)
{
    char    *path;

    if (!maTokenize(state, value, "%T", &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    maSetServerHome(state->server, path);
    httpSetHostHome(state->host, path);
    httpSetRoutePathVar(state->route, "SERVER_ROOT", path);
    mprLog(MPR_CONFIG, "Server Root \"%s\"", path);
    return 0;
}


/*
    SetConnector connector
 */
static int setConnectorDirective(MaState *state, cchar *key, cchar *value)
{
    if (httpSetRouteConnector(state->route, value) < 0) {
        mprError("Can't add handler %s", value);
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    SetHandler handler
 */
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


/*
    Source path
 */
static int sourceDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteSource(state->route, value);
    return 0;
}


/*
    StartHandler before|after|smart
 */
static int startHandlerDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *when;

    if (!maTokenize(state, value, "%S %*", &name, &when)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasematch(value, "before")) {
        state->route->flags |= HTTP_ROUTE_HANDLER_BEFORE;
    } else if (scasematch(value, "after")) {
        state->route->flags |= HTTP_ROUTE_HANDLER_AFTER;
    } else if (scasematch(value, "smart")) {
        state->route->flags |= HTTP_ROUTE_HANDLER_SMART;
    } else {
        return configError(state, key);
    }
    return 0;
}


/*
    StartThreads count
 */
static int startThreadsDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMinWorkers(atoi(value));
    return 0;
}


/*
    Target close [immediate]
    Target file ${DOCUMENT_ROOT}/${request:uri}
    Target redirect [status] URI            # Redirect to a new URI and re-route
    Target virt ${controller}-${name} 
 */
static int targetDirective(MaState *state, cchar *key, cchar *value)
{
    char    *kind, *details;

    if (!maTokenize(state, value, "%S ?*", &kind, &details)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return setTarget(state, kind, details);
}


/*
    ThreadLimit count
 */
static int threadLimitDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMaxWorkers(atoi(value));
    return 0;
}


/*
    ThreadStackSize bytes
 */
static int threadStackSizeDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetThreadStackSize(atoi(value));
    return 0;
}


/*
    Timeout secs
 */
static int timeoutDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->requestTimeout = atoi(value) * MPR_TICKS_PER_SEC;
    return 0;
}


/*
    TraceMethod on|off
 */
static int traceMethodDirective(MaState *state, cchar *key, cchar *value)
{
    bool    on;

    if (!maTokenize(state, value, "%B", &on)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    state->limits->enableTraceMethod = on;
    return 0;
}


/*  
    TypesConfig path
 */
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


/*
    UnloadModule name [timeout]
 */
static int unloadModuleDirective(MaState *state, cchar *key, cchar *value)
{
    MprModule   *module;
    char        *name;
    int         timeout;

    timeout = MA_UNLOAD_TIMEOUT;
    if (!maTokenize(state, value, "%S ?N", &name, &timeout)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if ((module = mprLookupModule(name)) == 0) {
        mprError("Can't find module stage %s", name);
        return MPR_ERR_BAD_SYNTAX;
    }
    module->timeout = (int) stoi(value, 10, NULL) * MPR_TICKS_PER_SEC;
    return 0;
}


/*
    UploadDir path
 */
static int uploadDirDirective(MaState *state, cchar *key, cchar *value)
{
    state->route->uploadDir = httpMakePath(state->route, value);
    mprLog(MPR_CONFIG, "Upload directory: %s", state->route->uploadDir);
    return 0;
}


/*
    UploadAutoDelete on|off
 */
static int uploadAutoDeleteDirective(MaState *state, cchar *key, cchar *value)
{
    if (!maTokenize(state, value, "%B", &state->route->autoDelete)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


/*
    User username
 */
static int userDirective(MaState *state, cchar *key, cchar *value)
{
    maSetHttpUser(state->appweb, value);
    return 0;
}


/*
    <VirtualHost ip[:port]>
 */
static int virtualHostDirective(MaState *state, cchar *key, cchar *value)
{
    HttpEndpoint    *endpoint;
    char            *ip;
    int             port;

    state = maPushState(state);
    if (state->enabled) {
        mprParseIp(value, &ip, &port, -1);
        state->host = httpCloneHost(state->host);
        httpSetHostIpAddr(state->host, ip, port);
        state->route = httpCreateInheritedRoute(state->route);
        httpSetRouteHost(state->route, state->host);
        httpSetRouteName(state->route, sfmt("host: %s", state->host->name));
        if ((endpoint = httpLookupEndpoint(state->http, ip, port)) == 0) {
            mprError("Can't find listen directive for virtual host %s", value);
            return MPR_ERR_BAD_SYNTAX;
        } else {
            httpAddHostToEndpoint(endpoint, state->host);
        }
    }
    return 0;
}


int maValidateConfiguration(MaServer *server)
{
    MaAppweb        *appweb;
    Http            *http;
    HttpHost        *host, *defaultHost;
    HttpEndpoint    *endpoint;
    HttpRoute       *route;
    int             nextHost, nextEndpoint, nextRoute;

    appweb = server->appweb;
    http = appweb->http;
    defaultHost = mprGetFirstItem(http->hosts);
    mprAssert(defaultHost);

    if (mprGetListLength(server->endpoints) == 0) {
        mprError("Missing listening HttpEndpoint. Must have a Listen directive.");
        return 0;
    }
    /*
        Add hosts to relevant listen endpoints
     */
    for (nextEndpoint = 0; (endpoint = mprGetNextItem(http->endpoints, &nextEndpoint)) != 0; ) {
        if (mprGetListLength(endpoint->hosts) == 0) {
            httpAddHostToEndpoint(endpoint, defaultHost);
            if (!defaultHost->ip) {
                httpSetHostIpAddr(defaultHost, endpoint->ip, endpoint->port);
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
        for (nextRoute = 0; (route = mprGetNextItem(host->routes, &nextRoute)) != 0; ) {
            if (!mprLookupKey(route->extensions, "")) {
                mprError("Route %s in host %s is missing a catch-all handler\n"
                        "Adding: AddHandler fileHandler \"\"", route->name, host->name);
                httpAddRouteHandler(route, "fileHandler", "");
            }
        }
        mprLog(MPR_CONFIG, "Configured host \"%s\" at \"%s\"", host->name, host->home);
    }
    return 1;
}


static bool conditionalDefinition(cchar *key)
{
    //  TODO - should always create a conditional directive when a module is loaded. Even for user modules.
    if (scasematch(key, "BLD_COMMERCIAL")) {
        return strcmp(BLD_COMMERCIAL, "0");

#ifdef BLD_DEBUG
    } else if (scasematch(key, "BLD_DEBUG")) {
        return BLD_DEBUG;
#endif

    } else if (scasematch(key, "CGI_MODULE")) {
        return BLD_FEATURE_CGI;

    } else if (scasematch(key, "DIR_MODULE")) {
        return BLD_FEATURE_DIR;

    } else if (scasematch(key, "EJS_MODULE")) {
        return BLD_FEATURE_EJSCRIPT;

    } else if (scasematch(key, "ESP_MODULE")) {
        return BLD_FEATURE_ESP;

    } else if (scasematch(key, "PHP_MODULE")) {
        return BLD_FEATURE_PHP;

    } else if (scasematch(key, "SSL_MODULE")) {
        return BLD_FEATURE_SSL;

    } else if (scasematch(key, BLD_OS)) {
        return 1;

    } else if (scasematch(key, BLD_CPU)) {
        return 1;
    }
    return 0;
}


/*
    Tokenizes a line using %formats. Mandatory tokens can be specified with %. Optional tokens are specified with ?. 
    Supported tokens:
        %B - Boolean. Parses: on/off, true/false, yes/no.
        %N - Number. Parses numbers in base 10.
        %S - String. Removes quotes.
        %P - Path string. Removes quotes and expands ${PathVars}. Resolved relative to host->dir (ServerRoot).
        %W - Parse words into a list
        %! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.
 */
bool maTokenize(MaState *state, cchar *line, cchar *fmt, ...)
{
    va_list     ap;

    va_start(ap, fmt);
    if (!httpTokenizev(state->route, line, fmt, ap)) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, line);
    }
    va_end(ap);
    return 1;
}


static int addCondition(MaState *state, cchar *condition, int flags)
{
    if (httpAddRouteCondition(state->route, condition, flags) < 0) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, condition);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int addUpdate(MaState *state, cchar *kind, cchar *details, int flags)
{
    if (httpAddRouteUpdate(state->route, kind, details, flags) < 0) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, kind, details);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int setTarget(MaState *state, cchar *kind, cchar *details)
{
    if (httpSetRouteTarget(state->route, kind, details) < 0) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, kind, details);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


/*
    This is used to create the outermost state only
 */
static MaState *createState(MaServer *server, HttpHost *host, HttpRoute *route)
{
    MaState     *state;

    if ((state = mprAllocObj(MaState, manageState)) == 0) {
        return 0;
    }
    state->top = state;
    state->current = state;
    state->server = server;
    state->appweb = server->appweb;
    state->http = server->http;
    state->host = host;
    state->limits = state->host->limits;
    state->route = route;
    state->enabled = 1;
    state->lineNumber = 0;
    state->auth = state->route->auth;
    httpSetRouteName(state->route, "global");
    return state;
}


MaState *maPushState(MaState *prev)
{
    MaState   *state;

    if ((state = mprAllocObj(MaState, manageState)) == 0) {
        return 0;
    }
    state->top = prev->top;
    state->prev = prev;
    state->appweb = prev->appweb;
    state->http = prev->http;
    state->server = prev->server;
    state->host = prev->host;
    state->route = prev->route;
    state->lineNumber = prev->lineNumber;
    state->enabled = prev->enabled;
    state->filename = prev->filename;
    state->file = prev->file;
    state->limits = prev->limits;
    state->auth = state->route->auth;
    state->top->current = state;
    return state;
}


MaState *maPopState(MaState *state)
{
    if (state->prev == 0) {
        mprError("Too many closing blocks.\nAt line %d in %s\n\n", state->lineNumber, state->filename);
    }
    state->prev->lineNumber = state->lineNumber;
    state = state->prev;
    state->top->current = state;
    return state;
}


static void manageState(MaState *state, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
		mprMark(state->appweb);
		mprMark(state->auth);
		mprMark(state->configDir);
		mprMark(state->current);
		mprMark(state->file);
		mprMark(state->filename);
		mprMark(state->host);
		mprMark(state->http);
		mprMark(state->key);
		mprMark(state->limits);
		mprMark(state->prev);
		mprMark(state->route);
		mprMark(state->server);
		mprMark(state->top);
    }
}


static int configError(MaState *state, cchar *key)
{
    mprError("Bad directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
    return MPR_ERR_BAD_SYNTAX;
}



void maAddDirective(MaAppweb *appweb, cchar *directive, MaDirective proc)
{
    mprAddKey(appweb->directives, directive, proc);
}


int maParseInit(MaAppweb *appweb)
{
    if ((appweb->directives = mprCreateHash(-1, MPR_HASH_STATIC_VALUES | MPR_HASH_CASELESS)) == 0) {
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
    maAddDirective(appweb, "AuthType", authTypeDirective);
    maAddDirective(appweb, "AuthUserFile", authUserFileDirective);
    maAddDirective(appweb, "AuthDigestQop", authDigestQopDirective);
    maAddDirective(appweb, "ClientCache", clientCacheDirective);
    maAddDirective(appweb, "ClientCacheByType", clientCacheByTypeDirective);
    maAddDirective(appweb, "Chroot", chrootDirective);
    maAddDirective(appweb, "Compress", compressDirective);
    maAddDirective(appweb, "Condition", conditionDirective);
    maAddDirective(appweb, "CustomLog", customLogDirective);
    maAddDirective(appweb, "DefaultLanguage", defaultLanguageDirective);
    maAddDirective(appweb, "Deny", denyDirective);
    maAddDirective(appweb, "DirectoryIndex", directoryIndexDirective);
    maAddDirective(appweb, "DocumentRoot", documentRootDirective);
    maAddDirective(appweb, "<Directory", directoryDirective);
    maAddDirective(appweb, "</Directory", closeDirective);
    maAddDirective(appweb, "ErrorDocument", errorDocumentDirective);
    maAddDirective(appweb, "ErrorLog", errorLogDirective);
    maAddDirective(appweb, "Group", groupDirective);
    maAddDirective(appweb, "Header", headerDirective);
    maAddDirective(appweb, "<If", ifDirective);
    maAddDirective(appweb, "</If", closeDirective);
    maAddDirective(appweb, "Include", includeDirective);
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

    /* Deprecated */
    maAddDirective(appweb, "LimitRequestFields", limitRequestHeaderCountDirective);
    maAddDirective(appweb, "LimitRequestFieldSize", limitRequestHeaderSizeDirective);

    maAddDirective(appweb, "LimitRequestHeaderCount", limitRequestHeaderCountDirective);
    maAddDirective(appweb, "LimitRequestHeaderSize", limitRequestHeaderSizeDirective);


    /* Deprecated */
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
    maAddDirective(appweb, "Prefix", prefixDirective);
    maAddDirective(appweb, "Protocol", protocolDirective);
    maAddDirective(appweb, "PutMethod", putMethodDirective);
    maAddDirective(appweb, "Query", queryDirective);
    maAddDirective(appweb, "Redirect", redirectDirective);
    maAddDirective(appweb, "Require", requireDirective);
    maAddDirective(appweb, "Reset", resetDirective);
    maAddDirective(appweb, "ResetPipeline", resetPipelineDirective);
    maAddDirective(appweb, "<Route", routeDirective);
    maAddDirective(appweb, "</Route", closeDirective);
    maAddDirective(appweb, "RouteName", routeNameDirective);
    maAddDirective(appweb, "ServerName", serverNameDirective);
    maAddDirective(appweb, "ServerRoot", serverRootDirective);
    maAddDirective(appweb, "SetConnector", setConnectorDirective);
    maAddDirective(appweb, "SetHandler", setHandlerDirective);
    maAddDirective(appweb, "Source", sourceDirective);
    maAddDirective(appweb, "StartHandler", startHandlerDirective);
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
