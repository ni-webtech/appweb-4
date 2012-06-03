/**
    config.c - Parse the configuration file.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"
#include    "pcre.h"

/***************************** Forward Declarations ****************************/

static int addCondition(MaState *state, cchar *name, cchar *condition, int flags);
static int addUpdate(MaState *state, cchar *name, cchar *details, int flags);
static bool conditionalDefinition(MaState *state, cchar *key);
static int configError(MaState *state, cchar *key);
static MaState *createState(MaServer *server, HttpHost *host, HttpRoute *route);
static char *getDirective(char *line, char **valuep);
static int64 getnum(cchar *value);
static MprTime gettime(cchar *value);
static char *gettok(char *s, char **tok);
static void manageState(MaState *state, int flags);
static int parseFile(MaState *state, cchar *path);
static int parseFileInner(MaState *state, cchar *path);
static int setTarget(MaState *state, cchar *name, cchar *details);

/******************************************************************************/

int maOpenConfig(MaState *state, cchar *path)
{
    mprAssert(state);
    mprAssert(path && *path);

    state->filename = sclone(path);
    state->configDir = mprGetAbsPath(mprGetPathDir(state->filename));
    if ((state->file = mprOpenFile(path, O_RDONLY | O_TEXT, 0444)) == 0) {
        mprError("Can't open %s for config directives", path);
        return MPR_ERR_CANT_OPEN;
    }
    mprLog(5, "Parsing config file: %s", state->filename);
    return 0;
}


int maParseConfig(MaServer *server, cchar *path, int flags)
{
    MaState     *state;
    HttpHost    *host;
    HttpRoute   *route;

    mprAssert(server);
    mprAssert(path && *path);

    mprLog(2, "Config File %s", path);

    /*
        Create top level host and route
        NOTE: the route is not added to the host until the finalization below
     */
    host = httpCreateHost(mprGetPathDir(path));
    httpSetHostName(host, "default-server");
    route = httpCreateRoute(host);
    httpSetHostDefaultRoute(host, route);
    httpSetRoutePathVar(route, "LIBDIR", mprJoinPath(server->appweb->platformDir, "bin"));
    route->limits = server->limits;

    state = createState(server, host, route);
    state->flags = flags;
    if (parseFile(state, path) < 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (!maValidateServer(server)) {
        return MPR_ERR_BAD_ARGS;
    }
    httpFinalizeRoute(state->route);
    if (mprHasMemError()) {
        mprError("Memory allocation error when initializing");
        return MPR_ERR_MEMORY;
    }
    return 0;
}


static int parseFile(MaState *state, cchar *path)
{
    int     rc;

    mprAssert(state);
    mprAssert(path && *path);

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
    
    mprAssert(state);
    mprAssert(path && *path);

    if (maOpenConfig(state, path) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    for (state->lineNumber = 1; state->file && (line = mprReadLine(state->file, 0, NULL)) != 0; state->lineNumber++) {
        for (tok = line; isspace((uchar) *tok); tok++) ;
        if (*tok == '\0' || *tok == '#') {
            continue;
        }
        state->key = 0;
        key = getDirective(line, &value);
        if (!state->enabled) {
            if (key[0] != '<') {
                mprLog(8, "Skip: %s %s", key, value);
                continue;
            }
        }
        if ((directive = mprLookupKey(state->appweb->directives, key)) == 0) {
            mprError("Unknown directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
            return MPR_ERR_BAD_SYNTAX;
        }
        state->key = key;
        mprLog(8, "Line %d, Parse %s %s", state->lineNumber, key, value ? value : "");
        if ((*directive)(state, key, value) < 0) {
            mprError("Error with directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
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


#if !BIT_FEATURE_ROMFS
/*
    AccessLog path
    AccessLog conf/log.conf size=10K, backup=5, append, anew, format=""
 */
static int accessLogDirective(MaState *state, cchar *key, cchar *value)
{
    char        *option, *ovalue, *tok, *path;
    ssize       size;
    int         flags, backup;

    size = INT_MAX;
    backup = 0;
    flags = 0;
    path = 0;
    
    for (option = gettok(sclone(value), &tok); option; option = gettok(tok, &tok)) {
        if (!path) {
            path = sclone(option);
        } else {
            option = stok(option, " =\t,", &ovalue);
            ovalue = strim(ovalue, "\"'", MPR_TRIM_BOTH);
            if (smatch(option, "size")) {
                size = (ssize) getnum(ovalue);

            } else if (smatch(option, "backup")) {
                backup = atoi(ovalue);

            } else if (smatch(option, "append")) {
                flags |= MPR_LOG_APPEND;

            } else if (smatch(option, "anew")) {
                flags |= MPR_LOG_ANEW;

            } else {
                mprError("Unknown option %s", option);
            }
        }
    }
    if (size < (10 * 1024)) {
        mprError("Size is too small. Must be larger than 10K");
        return MPR_ERR_BAD_SYNTAX;
    }
    if (path == 0) {
        mprError("Missing filename");
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetRouteLog(state->route, httpMakePath(state->route, path), size, backup, HTTP_LOG_FORMAT, flags);
    return 0;
}
#endif


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
    AddLanguageSuffix lang ext [position]
    AddLanguageSuffix en .en before
 */
static int addLanguageSuffixDirective(MaState *state, cchar *key, cchar *value)
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
    httpAddRouteLanguageSuffix(state->route, lang, ext, flags);
    return 0;
}


/*
    AddLanguageDir lang path
 */
static int addLanguageDirDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *route;
    char        *lang, *path;

    route = state->route;
    if (!maTokenize(state, value, "%S %S", &lang, &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if ((path = stemplate(path, route->pathTokens)) == 0) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (mprIsPathRel(path)) {
        path = mprJoinPath(route->dir, path);
    }
    httpAddRouteLanguageDir(route, lang, mprGetAbsPath(path));
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
    mprAddMime(state->route->mimeTypes, mimeType, ext);
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
        if (sends(prefix, "/")) {
            httpSetRoutePattern(alias, sfmt("^%s(.*)$", prefix), 0);
        } else {
            /* Add a non-capturing optional trailing "/" */
            httpSetRoutePattern(alias, sfmt("^%s(?:/)*(.*)$", prefix), 0);
        }
        httpSetRouteTarget(alias, "run", "$1");
    } else {
        alias = httpCreateAliasRoute(state->route, sjoin("^", prefix, NULL), 0, 0);
        httpSetRouteTarget(alias, "run", path);
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
    return addCondition(state, "allowDeny", 0, 0);
}


/*
    AuthGroupFile path
 */
static int authGroupFileDirective(MaState *state, cchar *key, cchar *value)
{
    char    *path;

    path = mprJoinPath(state->configDir, value);
    path = httpMakePath(state->route, path);
    if (httpReadGroupFile(state->auth, path) < 0) {
        mprError("Can't open AuthGroupFile %s", path);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


/*
    AuthMethod pam|file
 */
static int authMethodDirective(MaState *state, cchar *key, cchar *value)
{
    if (scasecmp(value, "run") == 0) {
        state->auth->backend = HTTP_AUTH_FILE;
#if BLD_CC_PAM
    } else if (scasecmp(value, "pam") == 0) {
        state->auth->backend = HTTP_AUTH_PAM;
#endif
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
    value = strim(value, "\"'", MPR_TRIM_BOTH);
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
    return addCondition(state, "auth", 0, 0);
}


/*
    AuthUserFile path
 */
static int authUserFileDirective(MaState *state, cchar *key, cchar *value)
{
    char    *path;
    path = mprJoinPath(state->configDir, value);
    path = httpMakePath(state->route, path);
    if (httpReadUserFile(state->auth, path) < 0) {
        mprError("Can't open AuthUserFile %s", path);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
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
    Cache options
    Options:
        lifespan
        server=lifespan
        client=lifespan
        extensions="html,gif,..."
        methods="GET,PUT,*,..."
        types="mime-type,*,..."
        all | only | unique
 */
static int cacheDirective(MaState *state, cchar *key, cchar *value)
{
    MprTime     lifespan, clientLifespan, serverLifespan;
    char        *option, *ovalue, *tok;
    char        *methods, *extensions, *types, *uris;
    int         flags;

    flags = 0;
    lifespan = clientLifespan = serverLifespan = 0;
    methods = uris = extensions = types = 0;

    for (option = stok(sclone(value), " \t", &tok); option; option = stok(0, " \t", &tok)) {
        if (*option == '/') {
            uris = option;
            if (tok) {
                /* Join the rest of the options back into one list of URIs */
                tok[-1] = ',';
            }
            break;
        }
        option = stok(option, " =\t,", &ovalue);
        ovalue = strim(ovalue, "\"'", MPR_TRIM_BOTH);
        if ((int) isdigit((uchar) *option)) {
            lifespan = gettime(option);

        } else if (smatch(option, "client")) {
            flags |= HTTP_CACHE_CLIENT;
            if (snumber(ovalue)) {
                clientLifespan = gettime(ovalue);
            }

        } else if (smatch(option, "server")) {
            flags |= HTTP_CACHE_SERVER;
            if (snumber(ovalue)) {
                serverLifespan = gettime(ovalue);
            }

        } else if (smatch(option, "extensions")) {
            extensions = ovalue;

        } else if (smatch(option, "types")) {
            types = ovalue;

        } else if (smatch(option, "all")) {
            flags |= HTTP_CACHE_ALL;
            flags &= ~(HTTP_CACHE_ONLY | HTTP_CACHE_UNIQUE);

        } else if (smatch(option, "only")) {
            flags |= HTTP_CACHE_ONLY;
            flags &= ~(HTTP_CACHE_ALL | HTTP_CACHE_UNIQUE);

        } else if (smatch(option, "unique")) {
            flags |= HTTP_CACHE_UNIQUE;
            flags &= ~(HTTP_CACHE_ALL | HTTP_CACHE_ONLY);

        } else if (smatch(option, "manual")) {
            flags |= HTTP_CACHE_MANUAL;

        } else if (smatch(option, "methods")) {
            methods = ovalue;

        } else {
            mprError("Unknown Cache option '%s'", option);
            return MPR_ERR_BAD_SYNTAX;
        }
    }
    if (flags & !(HTTP_CACHE_ONLY | HTTP_CACHE_UNIQUE)) {
        flags |= HTTP_CACHE_ALL;
    }
    if (lifespan > 0 && !uris && !extensions && !types && !methods) {
        state->route->lifespan = lifespan;
    } else {
        httpAddCache(state->route, methods, uris, extensions, types, clientLifespan, serverLifespan, flags);
    }
    return 0;
}


/*
    Chroot path
 */
static int chrootDirective(MaState *state, cchar *key, cchar *value)
{
#if BIT_UNIX_LIKE
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
    </Route>, </Location>, </Directory>, </VirtualHost>, </If>
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
    Condition [!] match string valuePattern

    Strings can contain route->pattern and request ${tokens}
 */
static int conditionDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *details;
    int     not;

    if (!maTokenize(state, value, "%! %S %*", &not, &name, &details)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return addCondition(state, name, details, not);
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
    return addCondition(state, "allowDeny", 0, 0);
}


/*
    <Directory path>
 */
static int directoryDirective(MaState *state, cchar *key, cchar *value)
{
    /*
        Directory must be deprecated because Auth directives inside a directory block applied to physical filenames.
        The router and Route directives can't emulate this. The user needs to migrate such configurations to apply
        Auth directives to route URIs instead.
     */
    mprError("The <Directory> directive is deprecated. Use <Route> with a DocumentRoot instead.");
    return MPR_ERR_BAD_SYNTAX;
}


/*
    DirectoryIndex path
 */
static int directoryIndexDirective(MaState *state, cchar *key, cchar *value)
{
    httpAddRouteIndex(state->route, value);
    return 0;
}


/*
    DocumentRoot path
 */
static int documentRootDirective(MaState *state, cchar *key, cchar *value)
{
    cchar   *path;
    if (!maTokenize(state, value, "%T", &path)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpSetRouteDir(state->route, path);
    return 0;
}


/*
    <else>
 */
static int elseDirective(MaState *state, cchar *key, cchar *value)
{
    state->enabled = !state->enabled;
    return 0;
}


/*
    ErrorDocument status URI
 */
static int errorDocumentDirective(MaState *state, cchar *key, cchar *value)
{
    char    *uri;
    int     status;

    if (!maTokenize(state, value, "%N %S", &status, &uri)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteErrorDocument(state->route, status, uri);
    return 0;
}


/*
    ErrorLog path
        [size=bytes] 
        [level=0-9] 
        [backup=count] 
        [append] 
        [anew]
        [stamp=period]
 */
static int errorLogDirective(MaState *state, cchar *key, cchar *value)
{
    MprTime     stamp;
    char        *option, *ovalue, *tok, *path;
    ssize       size;
    int         level, flags, backup;

    if (mprGetCmdlineLogging()) {
        mprLog(4, "Already logging. Ignoring ErrorLog directive");
        return 0;
    }
    size = INT_MAX;
    stamp = 0;
    level = 0;
    backup = 0;
    path = 0;
    flags = 0;
    
    for (option = gettok(sclone(value), &tok); option; option = gettok(tok, &tok)) {
        if (!path) {
            path = sclone(option);
        } else {
            option = stok(option, " =\t,", &ovalue);
            ovalue = strim(ovalue, "\"'", MPR_TRIM_BOTH);
            if (smatch(option, "size")) {
                size = (ssize) getnum(ovalue);

            } else if (smatch(option, "level")) {
                level = atoi(ovalue);

            } else if (smatch(option, "backup")) {
                backup = atoi(ovalue);

            } else if (smatch(option, "append")) {
                flags |= MPR_LOG_APPEND;

            } else if (smatch(option, "anew")) {
                flags |= MPR_LOG_ANEW;

            } else if (smatch(option, "stamp")) {
                stamp = gettime(ovalue);

            } else {
                mprError("Unknown option %s", option);
            }
        }
    }
    if (size < (10 * 1000)) {
        mprError("Size is too small. Must be larger than 10K");
        return MPR_ERR_BAD_SYNTAX;
    }
    if (path == 0) {
        mprError("Missing filename");
        return MPR_ERR_BAD_SYNTAX;
    }
    mprSetLogBackup(size, backup, flags);

    if (sncmp(path, "stdout", 6) != 0 && sncmp(path, "stderr", 6) != 0) {
        path = httpMakePath(state->route, path);
    }
    if (mprStartLogging(path, 0) < 0) {
        mprError("Can't write to ErrorLog: %s", path);
        return MPR_ERR_BAD_SYNTAX;
    }
    mprSetLogLevel(level);
    mprLogHeader();
    if (stamp) {
        httpSetTimestamp(stamp);
    }
    return 0;
}


/*
    ExitTimeout timeout
 */
static int exitTimeoutDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetExitTimeout(gettime(value));
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
    Header [!] name valuePattern
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
        pattern = sjoin("^", pattern, "$", NULL);
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
            free(compiled);
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
        state->enabled = conditionalDefinition(state, value);
        if (!state->enabled) {
            mprLog(7, "If \"%s\" conditional is false at %s:%d", value, state->filename, state->lineNumber);
        }
    }
    return 0;
}


/*
    InactivityTimeout secs
 */
static int inactivityTimeoutDirective(MaState *state, cchar *key, cchar *value)
{
    if (! mprGetDebugMode()) {
        state->limits = httpGraduateLimits(state->route, state->server->limits);
        state->limits->inactivityTimeout = gettime(value);
    }
    return 0;
}


/*
    LimitCache bytes
 */
static int limitCacheDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetCacheLimits(state->host->responseCache, 0, 0, getnum(value), 0);
    return 0;
}


/*
    LimitCacheItem bytes
 */
static int limitCacheItemDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->cacheItemSize = (int) getnum(value);
    return 0;
}


/*
    LimitChunk bytes
 */
static int limitChunkDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->chunkSize = (int) getnum(value);
    return 0;
}


/*
    LimitClients count
 */
static int limitClientsDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->clientMax = atoi(value);
    return 0;
}


/*
    LimitMemory size

    Redline set to 85%
 */
static int limitMemoryDirective(MaState *state, cchar *key, cchar *value)
{
    ssize   maxMem;

    maxMem = (ssize) getnum(value);
    mprSetMemLimits(maxMem / 100 * 85, maxMem);
    return 0;
}


/*
    LimitProcesses count
 */
static int limitProcessesDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->processMax = atoi(value);
    return 0;
}


/*
    LimitRequests count
 */
static int limitRequestsDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->requestMax = atoi(value);
    return 0;
}


/*
    LimitRequestBody bytes
 */
static int limitRequestBodyDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->receiveBodySize = getnum(value);
    return 0;
}


/*
    LimitRequestForm bytes
 */
static int limitRequestFormDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->receiveFormSize = getnum(value);
    return 0;
}


/*
    LimitRequestHeaderLines count
 */
static int limitRequestHeaderLinesDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->headerMax = (int) getnum(value);
    return 0;
}


/*
    LimitRequestHeader bytes
 */
static int limitRequestHeaderDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->headerSize = (int) getnum(value);
    return 0;
}


/*
    LimitResponseBody bytes
 */
static int limitResponseBodyDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->transmissionBodySize = getnum(value);
    return 0;
}


/*
    LimitStageBuffer bytes
 */
static int limitStageBufferDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->stageBufferSize = (int) getnum(value);
    return 0;
}


/*
    LimitUri bytes
 */
static int limitUriDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->uriSize = (int) getnum(value);
    return 0;
}


/*
    LimitUpload bytes
 */
static int limitUploadDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->uploadSize = getnum(value);
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
    char            *ip;
    int             port;
    
#if UNUSED
    int colonCount;
    char *cp, *vp;
    ssize           len;
    
    vp = sclone(value);
    if (isdigit((uchar) *vp) && strchr(vp, '.') == 0 && strchr(vp, ':') == 0) {
        /*
            Port only, listen on all interfaces (ipv4 + ipv6)
         */
        port = (int) stoi(vp);
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
                port = (int) stoi(cp);
            } else {
                port = HTTP_DEFAULT_PORT;
            }

        } else {
            /* ipv4 */
            ip = vp;
            if ((cp = strrchr(vp, ':')) != 0) {
                *cp++ = '\0';
                port = (int) stoi(cp);

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
#else
    mprParseSocketAddress(value, &ip, &port, HTTP_DEFAULT_PORT);
#endif
    if (port == 0) {
        mprError("Bad or missing port %d in Listen directive", port);
        return -1;
    }
    endpoint = httpCreateEndpoint(ip, port, NULL);
    mprAddItem(state->server->endpoints, endpoint);
    return 0;
}


/*
    Load name path
 */
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
    Log options
    Options:
        tx|rx
        conn=NN
        first=NN
        header=NN
        body=NN
        time=NN
        size=NN
        include="ext,ext,ext..."
        exclude="ext,ext,ext..."
 */
static int logDirective(MaState *state, cchar *key, cchar *value)
{
    char        *option, *ovalue, *tok, *include, *exclude;
    ssize       size;
    int         i, dir, levels[HTTP_TRACE_MAX_ITEM];

    include = exclude = 0;
    dir = HTTP_TRACE_RX;
    size = INT_MAX;
    
    for (i = 0; i < HTTP_TRACE_MAX_ITEM; i++) {
        levels[i] = 0;
    }
    for (option = stok(sclone(value), " \t", &tok); option; option = stok(0, " \t", &tok)) {
        option = stok(option, " =\t,", &ovalue);
        ovalue = strim(ovalue, "\"'", MPR_TRIM_BOTH);
        if (smatch(option, "tx")) {
            dir = HTTP_TRACE_TX;

        } else if (smatch(option, "rx")) {
            dir = HTTP_TRACE_RX;

        } else if (smatch(option, "conn")) {
            levels[HTTP_TRACE_CONN] = atoi(ovalue);

        } else if (smatch(option, "first")) {
            levels[HTTP_TRACE_FIRST] = atoi(ovalue);

        } else if (smatch(option, "headers")) {
            levels[HTTP_TRACE_HEADER] = atoi(ovalue);

        } else if (smatch(option, "body")) {
            levels[HTTP_TRACE_BODY] = atoi(ovalue);

        } else if (smatch(option, "limits")) {
            levels[HTTP_TRACE_LIMITS] = atoi(ovalue);

        } else if (smatch(option, "time")) {
            levels[HTTP_TRACE_TIME] = atoi(ovalue);

        } else if (smatch(option, "size")) {
            size = (ssize) getnum(ovalue);

        } else if (smatch(option, "include")) {
            include = ovalue;

        } else if (smatch(option, "exclude")) {
            exclude = ovalue;

        } else {
            mprError("Unknown Cache option '%s'", option);
            return MPR_ERR_BAD_SYNTAX;
        }
    }
    httpSetRouteTraceFilter(state->route, dir, levels, size, include, exclude);
    return 0;
}


/*
    LogRoutes [full]
    Support two formats line for one line, and multiline with more fields
 */
static int logRoutesDirective(MaState *state, cchar *key, cchar *value)
{
    cchar   *full;

    if (!maTokenize(state, value, "?S", &full)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (!(state->flags & MA_PARSE_NON_SERVER)) {
        mprRawLog(0, "\nHTTP Routes for the '%s' host:\n\n", state->host->name ? state->host->name : "default");
        httpLogRoutes(state->host, smatch(full, "full"));
    }
    return 0;
}


/*
    LoadModulePath searchPath
 */
static int loadModulePathDirective(MaState *state, cchar *key, cchar *value)
{
    char    *sep, *path;

    if (!maTokenize(state, value, "%T", &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    /*
		 Search path is: USER_SEARCH : exeDir : /usr/lib/appweb/lib
     */
    sep = MPR_SEARCH_SEP;
    path = sjoin(value, sep, mprGetAppDir(), sep, BIT_BIN_PREFIX, NULL);
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
    LimitKeepAlive count
 */
static int limitKeepAliveDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits = httpGraduateLimits(state->route, state->server->limits);
    state->limits->keepAliveMax = (int) stoi(value);
    return 0;
}


/*
    LimitWorkers count
 */
static int limitWorkersDirective(MaState *state, cchar *key, cchar *value)
{
    int     count;

    count = atoi(value);
    if (count <= 1) {
        mprError("Must have at least one worker");
    }
    mprSetMaxWorkers(count);
    return 0;
}


/*
    MemoryPolicy prune|restart|exit
 */
static int memoryPolicyDirective(MaState *state, cchar *key, cchar *value)
{
    cchar   *policy;
    int     flags;

    flags = MPR_ALLOC_POLICY_EXIT;

    if (!maTokenize(state, value, "%S", &policy)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scmp(value, "exit") == 0) {
        flags = MPR_ALLOC_POLICY_EXIT;

    } else if (scmp(value, "prune") == 0) {
        flags = MPR_ALLOC_POLICY_PRUNE;

    } else if (scmp(value, "restart") == 0) {
        flags = MPR_ALLOC_POLICY_RESTART;

    } else {
        mprError("Unknown memory depletion policy '%s'", policy);
        return MPR_ERR_BAD_SYNTAX;
    }
    mprSetMemPolicy(flags);
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
   Update param var value
   Update cmd commandLine
 */
static int updateDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *rest;

    if (!maTokenize(state, value, "%S %*", &name, &rest)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return addUpdate(state, name, rest, 0);
}


/*
    NameVirtualHost ip[:port]
 */
static int nameVirtualHostDirective(MaState *state, cchar *key, cchar *value)
{
    char    *ip;
    int     port;

    mprLog(4, "NameVirtual Host: %s ", value);
    mprParseSocketAddress(value, &ip, &port, -1);
    httpConfigureNamedVirtualEndpoints(state->http, ip, port);
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
    Param [!] name valuePattern
 */
static int paramDirective(MaState *state, cchar *key, cchar *value)
{
    char    *field;
    int     not;

    if (!maTokenize(state, value, "?! %S %*", &not, &field, &value)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    httpAddRouteParam(state->route, field, value, not);
    return 0;
}


/*
    Prefix /URI-PREFIX
    NOTE: For nested routes, the prefix value will be appended out any existing parent route prefix.
    NOTE: Prefixes do append, but route patterns do not.
 */
static int prefixDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRoutePrefix(state->route, value);
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


//  MOB - this should be renamed to be: FileModify on|off
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
    char        *code, *uri, *path, *target;
    int         status;

    if (value[0] == '/' || sncmp(value, "http://", 6) == 0) {
        if (!maTokenize(state, value, "%S %S", &uri, &path)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        status = HTTP_CODE_MOVED_TEMPORARILY;
    } else {
        if (!maTokenize(state, value, "%S %S ?S", &code, &uri, &path)) {
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
        } else if (snumber(code)) {
            status = atoi(code);
        } else {
            return configError(state, key);
        }
    }
    if (300 <= status && status <= 399 && (!path || *path == '\0')) {
        return configError(state, key);
    }
    if (status <= 0 || uri == 0) {
        return configError(state, key);
    }
    alias = httpCreateAliasRoute(state->route, uri, 0, status);
    target = (path) ? sfmt("%d %s", status, path) : code;
    httpSetRouteTarget(alias, "redirect", target);
    httpFinalizeRoute(alias);
    return 0;
}


/*
    RequestTimeout secs
 */
static int requestTimeoutDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->requestTimeout = gettime(value);
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
        httpSetRequiredAcl(state->auth, httpParseAcl(state->auth, rest));
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
    char    *name;

    if (!maTokenize(state, value, "%S", &name)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if (scasematch(name, "routes")) {
        httpResetRoutes(state->host);

    } else if (scasematch(name, "pipeline")) {
        httpResetRoutePipeline(state->route);

    } else {
        return configError(state, name);
    }
    return 0;
}


/*
    ResetPipeline (alias for Reset routes)
    DEPRECATED
 */
static int resetPipelineDirective(MaState *state, cchar *key, cchar *value)
{
    httpResetRoutePipeline(state->route);
    return 0;
}


/*
    <Route pattern>
    NOTE: The route pattern should include the prefix when declared. When compiling the pattern, the prefix is removed.
 */
static int routeDirective(MaState *state, cchar *key, cchar *value)
{
    HttpRoute   *route;
    char        *pattern, *name;
    int         not;

    state = maPushState(state);
    if (state->enabled) {
        if (!maTokenize(state, value, "%!%S", &not, &pattern)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if (strstr(pattern, "${")) {
            pattern = sreplace(pattern, "${inherit}", state->route->pattern);
        }
        name = (*pattern == '^') ? &pattern[1] : pattern;
        
        if ((route = httpLookupRoute(state->host, name)) != 0) {
            state->route = route;
        } else {
            state->route = httpCreateInheritedRoute(state->route);
            httpSetRoutePattern(state->route, pattern, not);
            httpSetRouteHost(state->route, state->host);
        }
        /* Routes are added when the route block is closed (see closeDirective) */
        state->auth = state->route->auth;
    }
    return 0;
}


/*
    Name routeName
 */
static int nameDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteName(state->route, value);
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
    SessionTimeout secs
 */
static int sessionTimeoutDirective(MaState *state, cchar *key, cchar *value)
{
    state->limits->sessionTimeout = gettime(value);
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
    StartWorkers count
 */
static int startWorkersDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetMinWorkers((int) stoi(value));
    return 0;
}


/*
    Target close [immediate]
    Target redirect [status] URI            # Redirect to a new URI and re-route
    Target run ${DOCUMENT_ROOT}/${request:uri}
    Target run ${controller}-${name} 
    Target write [-r] status "Hello World\r\n"
 */
static int targetDirective(MaState *state, cchar *key, cchar *value)
{
    char    *name, *details;

    if (!maTokenize(state, value, "%S ?*", &name, &details)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    return setTarget(state, name, details);
}


/*
    Template routeName
 */
static int templateDirective(MaState *state, cchar *key, cchar *value)
{
    httpSetRouteTemplate(state->route, value);
    return 0;
}


/*
    ThreadStack bytes
 */
static int threadStackDirective(MaState *state, cchar *key, cchar *value)
{
    mprSetThreadStackSize((int) getnum(value));
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
    if ((state->route->mimeTypes = mprCreateMimeTypes(path)) == 0) {
        mprError("Can't open TypesConfig mime file %s", path);
        state->route->mimeTypes = mprCreateMimeTypes(NULL);
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
    HttpStage   *stage;
    char        *name, *timeout;

    timeout = MA_UNLOAD_TIMEOUT;
    if (!maTokenize(state, value, "%S ?S", &name, &timeout)) {
        return MPR_ERR_BAD_SYNTAX;
    }
    if ((module = mprLookupModule(name)) == 0) {
        mprError("Can't find module stage %s", name);
        return MPR_ERR_BAD_SYNTAX;
    }
    if ((stage = httpLookupStage(state->http, module->name)) != 0 && stage->match) {
        mprError("Can't unload module %s due to match routine", module->name);
        return MPR_ERR_BAD_SYNTAX;
    } else {
        module->timeout = gettime(timeout);
    }
    return 0;
}


/*
    UploadDir path
 */
static int uploadDirDirective(MaState *state, cchar *key, cchar *value)
{
    state->route->uploadDir = httpMakePath(state->route, value);
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
        mprParseSocketAddress(value, &ip, &port, -1);
        /*
            Inherit the current default route configuration (only)
            Other routes are not inherited due to the reset routes below
         */
        state->route = httpCreateInheritedRoute(httpGetHostDefaultRoute(state->host));
        state->host = httpCloneHost(state->host);
        httpResetRoutes(state->host);
        httpSetRouteHost(state->route, state->host);
        httpSetHostDefaultRoute(state->host, state->route);
        httpSetHostIpAddr(state->host, ip, port);
        httpSetRouteName(state->route, sfmt("default-%s", state->host->name));
        state->auth = state->route->auth;
        if ((endpoint = httpLookupEndpoint(state->http, ip, port)) == 0) {
            mprError("Can't find listen directive for virtual host %s", value);
            return MPR_ERR_BAD_SYNTAX;
        } else {
            httpAddHostToEndpoint(endpoint, state->host);
        }
    }
    return 0;
}


bool maValidateServer(MaServer *server)
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
        for (nextRoute = 0; (route = mprGetNextItem(host->routes, &nextRoute)) != 0; ) {
            if (!mprLookupKey(route->extensions, "")) {
                mprError("Route %s in host %s is missing a catch-all handler\n"
                        "Adding: AddHandler fileHandler \"\"", route->name, host->name);
                httpAddRouteHandler(route, "fileHandler", "");
            }
        }
    }
    return 1;
}


int maParsePlatform(cchar *platform, cchar **os, cchar **arch, cchar **profile)
{
    char   *rest;

    if (platform == 0 || *platform == '\0') {
        return MPR_ERR_BAD_ARGS;
    }
    *os = stok(sclone(platform), "-", &rest);
    *arch = sclone(stok(NULL, "-", &rest));
    *profile = sclone(rest);
    if (*os == 0 || *arch == 0 || *profile == 0 || **os == '\0' || **arch == '\0' || **profile == '\0') {
        return MPR_ERR_BAD_ARGS;
    }
    return 0;
}


static bool conditionalDefinition(MaState *state, cchar *key)
{
    cchar   *arch, *os, *profile;
    int     result, not;

    result = 0;
    not = (*key == '!') ? 1 : 0;
    if (not) {
        for (++key; isspace((uchar) *key); key++) {}
    }
    maParsePlatform(state->appweb->platform, &os, &arch, &profile);

    if (scasematch(key, arch)) {
        result = 1;

    } else if (scasematch(key, os)) {
        result = 1;

    } else if (scasematch(key, profile)) {
        result = 1;

    } else if (scasematch(key, state->appweb->platform)) {
        result = 1;

#if BIT_DEBUG
    } else if (scasematch(key, "BIT_DEBUG")) {
        result = BIT_DEBUG;
#endif

    } else if (state->appweb->skipModules) {
        /* ESP utility needs to be able to load mod_esp */
        if (smatch(mprGetAppName(), "esp") && scasematch(key, "ESP_MODULE")) {
            result = BIT_FEATURE_ESP;
        }

    } else {
        if (scasematch(key, "CGI_MODULE")) {
            result = BIT_FEATURE_CGI;

        } else if (scasematch(key, "DIR_MODULE")) {
            result = BIT_FEATURE_DIR;

        } else if (scasematch(key, "EJS_MODULE")) {
            result = BIT_FEATURE_EJSCRIPT;

        } else if (scasematch(key, "ESP_MODULE")) {
            result = BIT_FEATURE_ESP;

        } else if (scasematch(key, "PHP_MODULE")) {
            result = BIT_FEATURE_PHP;

        } else if (scasematch(key, "SSL_MODULE")) {
            result = BIT_FEATURE_SSL;
        }
    }
    return (not) ? !result : result;
}


/*
    Tokenizes a line using %formats. Mandatory tokens can be specified with %. Optional tokens are specified with ?. 
    Supported tokens:
        %B - Boolean. Parses: on/off, true/false, yes/no.
        %N - Number. Parses numbers in base 10.
        %S - String. Removes quotes.
        %T - Template String. Removes quotes and expand ${PathVars}
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
        return 0;
    }
    va_end(ap);
    return 1;
}


static int addCondition(MaState *state, cchar *name, cchar *condition, int flags)
{
    if (httpAddRouteCondition(state->route, name, condition, flags) < 0) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, condition);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int addUpdate(MaState *state, cchar *name, cchar *details, int flags)
{
    if (httpAddRouteUpdate(state->route, name, details, flags) < 0) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, name, details);
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


static int setTarget(MaState *state, cchar *name, cchar *details)
{
    if (httpSetRouteTarget(state->route, name, details) < 0) {
        mprError("Bad \"%s\" directive at line %d in %s\nLine: %s %s %s\n", 
                state->key, state->lineNumber, state->filename, state->key, name, details);
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
    state->route = route;
    state->limits = state->route->limits;
    state->enabled = 1;
    state->lineNumber = 0;
    state->auth = state->route->auth;
    httpSetRouteName(state->route, "default");
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
    state->flags = prev->flags;
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
		mprMark(state->http);
		mprMark(state->server);
		mprMark(state->host);
		mprMark(state->auth);
		mprMark(state->route);
		mprMark(state->file);
		mprMark(state->limits);
		mprMark(state->key);
		mprMark(state->configDir);
		mprMark(state->filename);
		mprMark(state->prev);
		mprMark(state->top);
		mprMark(state->current);
    }
}


static int configError(MaState *state, cchar *key)
{
    mprError("Error in directive \"%s\"\nAt line %d in %s\n\n", key, state->lineNumber, state->filename);
    return MPR_ERR_BAD_SYNTAX;
}


static int64 getnum(cchar *value)
{
    int64   num;

    value = strim(slower(value), " \t", MPR_TRIM_BOTH);
    if (sends(value, "k") || sends(value, "KB")) {
        num = stoi(value) * 1024;
    } else if (sends(value, "mb") || sends(value, "M")) {
        num = stoi(value) * 1024 * 1024;
    } else if (sends(value, "gb") || sends(value, "G")) {
        num = stoi(value) * 1024 * 1024 * 1024;
    } else {
        num = stoi(value);
    }
    return num;
}


static MprTime gettime(cchar *value)
{
    MprTime     when;

    value = strim(slower(value), " \t", MPR_TRIM_BOTH);
    if (sends(value, "min") || sends(value, "mins")) {
        when = stoi(value) * 60;
    } else if (sends(value, "hr") || sends(value, "hrs")) {
        when = stoi(value) * 60 * 60;
    } else if (sends(value, "day") || sends(value, "days")) {
        when = stoi(value) * 60 * 60 * 24;
    } else {
        when = stoi(value);
    }
    return when * MPR_TICKS_PER_SEC;
}


/*
    Get the directive and value details. Return key and *valuep.
 */
static char *getDirective(char *line, char **valuep)
{
    char    *key, *value;
    ssize   len;
    
    mprAssert(line);
    mprAssert(valuep);

    *valuep = 0;
    key = stok(line, " \t", &value);
    key = strim(key, ">", MPR_TRIM_END);
    if (value) {
        value = strim(value, " \t\r\n>", MPR_TRIM_END);
        /*
            Trim quotes if wrapping the entire value and no spaces. Preserve embedded quotes and leading/trailing "" etc.
         */
        len = slen(value);
        if (*value == '\"' && value[len - 1] == '"' && len > 2 && value[1] != '\"' && !strpbrk(value, " \t")) {
            /*
                Can't strip quotes if multiple args are quoted, only if one single arg is quoted
             */
            if (schr(&value[1], '"') == &value[len - 1]) {
                value = snclone(&value[1], len - 2);
            }
        }
        *valuep = value;
    }
    return key;
}


/*
    Break into tokens separated by spaces or commas. Supports quoted args and backquotes.
 */
static char *gettok(char *s, char **tok)
{
    char    *etok;
    int     quote;

    if (s == 0) {
        return 0;
    }
    for (; isspace((uchar) *s); s++);  
    if (*s == '\'' || *s == '"') {
        quote = *s++;
        for (etok = s; *etok && !(*etok == quote && etok[-1] != '\\'); etok++) ;
    } else {
        for (etok = s; *etok && !(isspace((uchar) *etok) || *etok == ','); etok++) ;
    }
    if (*etok == '\0') {
        *tok = NULL;
        etok = NULL;
    } else {
        *etok++ = '\0';
	    for (; isspace((uchar) *etok); etok++);  
    }
    *tok = etok;
    return s;
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
    maAddDirective(appweb, "AddLanguageSuffix", addLanguageSuffixDirective);
    maAddDirective(appweb, "AddLanguageDir", addLanguageDirDirective);
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
    maAddDirective(appweb, "Cache", cacheDirective);
    maAddDirective(appweb, "Chroot", chrootDirective);
    maAddDirective(appweb, "Compress", compressDirective);
    maAddDirective(appweb, "Condition", conditionDirective);
    maAddDirective(appweb, "DefaultLanguage", defaultLanguageDirective);
    maAddDirective(appweb, "Deny", denyDirective);
    maAddDirective(appweb, "DirectoryIndex", directoryIndexDirective);
    maAddDirective(appweb, "DocumentRoot", documentRootDirective);
    maAddDirective(appweb, "<Directory", directoryDirective);
    maAddDirective(appweb, "</Directory", closeDirective);
    maAddDirective(appweb, "<else", elseDirective);
    maAddDirective(appweb, "ErrorDocument", errorDocumentDirective);
    maAddDirective(appweb, "ErrorLog", errorLogDirective);
    maAddDirective(appweb, "ExitTimeout", exitTimeoutDirective);
    maAddDirective(appweb, "Group", groupDirective);
    maAddDirective(appweb, "Header", headerDirective);
    maAddDirective(appweb, "<If", ifDirective);
    maAddDirective(appweb, "</If", closeDirective);
    maAddDirective(appweb, "InactivityTimeout", inactivityTimeoutDirective);
    maAddDirective(appweb, "Include", includeDirective);

    maAddDirective(appweb, "LimitCache", limitCacheDirective);
    maAddDirective(appweb, "LimitCacheItem", limitCacheItemDirective);
    maAddDirective(appweb, "LimitChunk", limitChunkDirective);
    maAddDirective(appweb, "LimitClients", limitClientsDirective);
    maAddDirective(appweb, "LimitKeepAlive", limitKeepAliveDirective);
    maAddDirective(appweb, "LimitMemory", limitMemoryDirective);
    maAddDirective(appweb, "LimitProcesses", limitProcessesDirective);
    maAddDirective(appweb, "LimitRequests", limitRequestsDirective);
    maAddDirective(appweb, "LimitRequestBody", limitRequestBodyDirective);
    maAddDirective(appweb, "LimitRequestForm", limitRequestFormDirective);
    maAddDirective(appweb, "LimitRequestHeaderLines", limitRequestHeaderLinesDirective);
    maAddDirective(appweb, "LimitRequestHeader", limitRequestHeaderDirective);
    maAddDirective(appweb, "LimitResponseBody", limitResponseBodyDirective);
    maAddDirective(appweb, "LimitStageBuffer", limitStageBufferDirective);
    maAddDirective(appweb, "LimitUri", limitUriDirective);
    maAddDirective(appweb, "LimitUpload", limitUploadDirective);
    maAddDirective(appweb, "LimitWorkers", limitWorkersDirective);

    maAddDirective(appweb, "Listen", listenDirective);

    //  MOB - not a great name "Load"
    maAddDirective(appweb, "Load", loadDirective);
    maAddDirective(appweb, "Log", logDirective);
    maAddDirective(appweb, "LogRoutes", logRoutesDirective);
    maAddDirective(appweb, "LoadModulePath", loadModulePathDirective);
    maAddDirective(appweb, "LoadModule", loadModuleDirective);

    maAddDirective(appweb, "MemoryPolicy", memoryPolicyDirective);
    maAddDirective(appweb, "Methods", methodsDirective);
    maAddDirective(appweb, "Name", nameDirective);
    maAddDirective(appweb, "NameVirtualHost", nameVirtualHostDirective);
    maAddDirective(appweb, "Order", orderDirective);
    maAddDirective(appweb, "Param", paramDirective);
    maAddDirective(appweb, "Prefix", prefixDirective);
    maAddDirective(appweb, "Protocol", protocolDirective);
    maAddDirective(appweb, "PutMethod", putMethodDirective);
    maAddDirective(appweb, "Redirect", redirectDirective);
    maAddDirective(appweb, "RequestTimeout", requestTimeoutDirective);
    maAddDirective(appweb, "Require", requireDirective);
    maAddDirective(appweb, "Reset", resetDirective);

    /* Deprecated: ResetPipeline - use Reset */
    maAddDirective(appweb, "ResetPipeline", resetPipelineDirective);
    maAddDirective(appweb, "<Route", routeDirective);
    maAddDirective(appweb, "</Route", closeDirective);
    maAddDirective(appweb, "ServerName", serverNameDirective);
    maAddDirective(appweb, "ServerRoot", serverRootDirective);
    maAddDirective(appweb, "SessionTimeout", sessionTimeoutDirective);
    maAddDirective(appweb, "SetConnector", setConnectorDirective);
    maAddDirective(appweb, "SetHandler", setHandlerDirective);
    maAddDirective(appweb, "Source", sourceDirective);

    maAddDirective(appweb, "StartWorkers", startWorkersDirective);
    maAddDirective(appweb, "Target", targetDirective);
    maAddDirective(appweb, "Template", templateDirective);

    maAddDirective(appweb, "ThreadStack", threadStackDirective);
    maAddDirective(appweb, "TraceMethod", traceMethodDirective);
    maAddDirective(appweb, "TraceMethod", traceMethodDirective);
    maAddDirective(appweb, "TypesConfig", typesConfigDirective);
    maAddDirective(appweb, "Update", updateDirective);
    maAddDirective(appweb, "UnloadModule", unloadModuleDirective);
    maAddDirective(appweb, "UploadAutoDelete", uploadAutoDeleteDirective);
    maAddDirective(appweb, "UploadDir", uploadDirDirective);
    maAddDirective(appweb, "User", userDirective);

    maAddDirective(appweb, "<VirtualHost", virtualHostDirective);
    maAddDirective(appweb, "</VirtualHost", closeDirective);

#if !BIT_FEATURE_ROMFS
    maAddDirective(appweb, "AccessLog", accessLogDirective);
#endif

#if DEPRECATED || 1
    /* Use eprecated use LimitKeepAlive */
    maAddDirective(appweb, "MaxKeepAliveRequests", limitKeepAliveDirective);
    /* Use LimitUri */
    maAddDirective(appweb, "LimitUrl", limitUriDirective);
    /* Use StartWorkers */
    maAddDirective(appweb, "StartThreads", startWorkersDirective);
    /* Use requestTimeout */
    maAddDirective(appweb, "Timeout", requestTimeoutDirective);
    maAddDirective(appweb, "ThreadLimit", limitWorkersDirective);
    maAddDirective(appweb, "WorkerLimit", limitWorkersDirective);
    /* Use LimitRequestHeaderLines */
    maAddDirective(appweb, "LimitRequestFields", limitRequestHeaderLinesDirective);
    /* Use LimitRequestHeader */
    maAddDirective(appweb, "LimitRequestFieldSize", limitRequestHeaderDirective);
    /* Use InactivityTimeout */
    maAddDirective(appweb, "KeepAliveTimeout", inactivityTimeoutDirective);
    /* Use <Route> */
    maAddDirective(appweb, "<Location", routeDirective);
    maAddDirective(appweb, "</Location", closeDirective);
    
#endif

    return 0;
}


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
