/*
    espTemplate.c -- Templated web pages with embedded C code.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"

#if BIT_FEATURE_ESP

/************************************ Defines *********************************/
/*
      ESP lexical analyser tokens
 */
#define ESP_TOK_ERR            -1            /* Any input error */
#define ESP_TOK_EOF             0            /* End of file */
#define ESP_TOK_CODE            1            /* <% text %> */
#define ESP_TOK_VAR             2            /* @@var */
#define ESP_TOK_FIELD           3            /* @#field */
#define ESP_TOK_LITERAL         4            /* literal HTML */
#define ESP_TOK_EXPR            5            /* <%= expression %> */
#define ESP_TOK_CONTROL         6            /* <%@ control */

/************************************ Forwards ********************************/

static int getEspToken(EspParse *parse);
static cchar *getDebug();
static cchar *getEnvString(EspRoute *eroute, cchar *key, cchar *defaultValue);
static cchar *getShlibExt(cchar *os);
static cchar *getShobjExt(cchar *os);
static cchar *getCompilerName(cchar *os, cchar *arch);
static cchar *getCompilerPath(cchar *os, cchar *arch);
static cchar *getLibs(cchar *os);
static cchar *getMappedArch(cchar *arch);
static cchar *getObjExt(cchar *os);
static cchar *getVisualStudio();
static cchar *getWinSDK();
static cchar *getVxCPU(cchar *arch);
static bool matchToken(cchar **str, cchar *token);

/************************************* Code ***********************************/
/*
    Tokens:
    ARCH        Build architecture (64)
    GCC_ARCH    ARCH mapped to gcc -arch switches (x86_64)
    CC          Compiler (cc)
    DEBUG       Debug compilation options (-g, -Zi -Od)
    INC         Include directory out/inc
    LIB         Library directory (out/lib, xcode/VS: out/bin)
    LIBS        Libraries required to link with ESP
    OBJ         Name of compiled source (out/lib/view-MD5.o)
    MOD         Output module (view_MD5.dylib)
    SHLIB       Host Shared library (.lib, .so)
    SHOBJ       Host Shared Object (.dll, .so)
    SRC         Source code for view or controller (already templated)
    TMP         Temp directory
    VS          Visual Studio directory
    WINSDK      Windows SDK directory
 */
char *espExpandCommand(EspRoute *eroute, cchar *command, cchar *source, cchar *module)
{
    MprBuf      *buf;
    MaAppweb    *appweb;
    cchar       *cp, *outputModule, *os, *arch, *profile;
    char        *tmp;
    
    if (command == 0) {
        return 0;
    }
    appweb = MPR->appwebService;
    outputModule = mprTrimPathExt(module);
    maParsePlatform(appweb->platform, &os, &arch, &profile);
    buf = mprCreateBuf(-1, -1);

    for (cp = command; *cp; ) {
		if (*cp == '$') {
            if (matchToken(&cp, "${ARCH}")) {
                /* Target architecture (x86|mips|arm|x64) */
                mprPutStringToBuf(buf, arch);

            } else if (matchToken(&cp, "${GCC_ARCH}")) {
                /* Target architecture mapped to GCC mtune|mcpu values */
                mprPutStringToBuf(buf, getMappedArch(arch));

            } else if (matchToken(&cp, "${INC}")) {
                /* Include directory for the configuration */
                mprPutStringToBuf(buf, mprJoinPath(appweb->platformDir, "inc")); 

            } else if (matchToken(&cp, "${LIBPATH}")) {
                /* Library directory for Appweb libraries for the target */
                mprPutStringToBuf(buf, mprJoinPath(appweb->platformDir, "bin")); 

            } else if (matchToken(&cp, "${LIBS}")) {
                /* Required libraries to link. These may have nested ${TOKENS} */
                mprPutStringToBuf(buf, espExpandCommand(eroute, getLibs(os), source, module));

            } else if (matchToken(&cp, "${MOD}")) {
                /* Output module path in the cache without extension */
                mprPutStringToBuf(buf, outputModule);

            } else if (matchToken(&cp, "${OBJ}")) {
                /* Output object with extension (.o) in the cache directory */
                mprPutStringToBuf(buf, mprJoinPathExt(outputModule, getObjExt(os)));

            } else if (matchToken(&cp, "${OS}")) {
                /* Target architecture (freebsd|linux|macosx|windows|vxworks) */
                mprPutStringToBuf(buf, os);

            } else if (matchToken(&cp, "${SHLIB}")) {
                /* .dll, .so, .dylib */
                mprPutStringToBuf(buf, getShlibExt(os));

            } else if (matchToken(&cp, "${SHOBJ}")) {
                /* .dll, .so, .dylib */
                mprPutStringToBuf(buf, getShobjExt(os));

            } else if (matchToken(&cp, "${SRC}")) {
                /* View (already parsed into C code) or controller source */
                mprPutStringToBuf(buf, source);

            } else if (matchToken(&cp, "${TMP}")) {
                if ((tmp = getenv("TMPDIR")) == 0) {
                    if ((tmp = getenv("TMP")) == 0) {
                        tmp = getenv("TEMP");
                    }
                }
                mprPutStringToBuf(buf, tmp ? tmp : ".");

            } else if (matchToken(&cp, "${VS}")) {
                mprPutStringToBuf(buf, getVisualStudio());

            } else if (matchToken(&cp, "${VXCPU}")) {
                mprPutStringToBuf(buf, getVxCPU(arch));

            } else if (matchToken(&cp, "${WINSDK}")) {
                mprPutStringToBuf(buf, getWinSDK());

            /*
                These vars can be configured from environment variables.
                NOTE: the default esp.conf includes "esp->vxworks.conf" which has EspEnv definitions for the configured 
                VxWorks toolchain
             */
            } else if (matchToken(&cp, "${CC}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "CC", getCompilerPath(os, arch)));
            } else if (matchToken(&cp, "${LINK}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "LINK", ""));
            } else if (matchToken(&cp, "${CFLAGS}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "CFLAGS", ""));
            } else if (matchToken(&cp, "${DEBUG}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "DEBUG", getDebug()));
            } else if (matchToken(&cp, "${LDFLAGS}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "LDFLAGS", ""));

            } else if (matchToken(&cp, "${WIND_BASE}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "WIND_BASE", WIND_BASE));
            } else if (matchToken(&cp, "${WIND_HOME}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "WIND_HOME", WIND_HOME));
            } else if (matchToken(&cp, "${WIND_HOST_TYPE}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "WIND_HOST_TYPE", WIND_HOST_TYPE));
            } else if (matchToken(&cp, "${WIND_PLATFORM}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "WIND_PLATFORM", WIND_PLATFORM));
            } else if (matchToken(&cp, "${WIND_GNU_PATH}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "WIND_GNU_PATH", WIND_GNU_PATH));
            } else if (matchToken(&cp, "${WIND_CCNAME}")) {
                mprPutStringToBuf(buf, getEnvString(eroute, "WIND_CCNAME", getCompilerName(os, arch)));
            } else {
                mprPutCharToBuf(buf, *cp++);
            }
        } else {
            mprPutCharToBuf(buf, *cp++);
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


static int runCommand(HttpConn *conn, cchar *command, cchar *csource, cchar *module)
{
    EspReq      *req;
    EspRoute    *eroute;
    MprCmd      *cmd;
    MprKey      *var;
    MprList     *elist;
    cchar       **env;
    char        *err, *out;

    req = conn->data;
    eroute = req->route->eroute;

    cmd = mprCreateCmd(conn->dispatcher);
    if ((req->commandLine = espExpandCommand(eroute, command, csource, module)) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing EspCompile directive for %s", csource);
        return MPR_ERR_CANT_READ;
    }
    mprLog(4, "ESP command: %s\n", req->commandLine);
    if (eroute->env) {
        elist = mprCreateList(0, 0);
        for (ITERATE_KEYS(eroute->env, var)) {
            mprAddItem(elist, sfmt("%s=%s", var->key, var->data));
        }
        mprAddNullItem(elist);
        env = (cchar**) &elist->items[0];
    } else {
        env = 0;
    }
    if (eroute->searchPath) {
        mprSetCmdSearchPath(cmd, eroute->searchPath);
    }
    //  WARNING: GC will run here
	if (mprRunCmd(cmd, req->commandLine, env, &out, &err, -1, 0) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
		if (eroute->showErrors) {
			httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't run command %s, error %s", req->commandLine, err);
		} else {
			mprError("ESP: Can't run command %s, error %s", req->commandLine, err);
			httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't compile view");
		}
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}


/*
    Compile a view or controller

    cacheName   MD5 cache name (not a full path)
    source      ESP source file name
    module      Module file name
 */
bool espCompile(HttpConn *conn, cchar *source, cchar *module, cchar *cacheName, int isView)
{
    MprFile     *fp;
    HttpRx      *rx;
    HttpRoute   *route;
    EspRoute    *eroute;
    cchar       *csource;
    char        *layout, *script, *page, *err;
    ssize       len;

    rx = conn->rx;
    route = rx->route;
    eroute = route->eroute;
    layout = 0;

    if (isView) {
        if ((page = mprReadPathContents(source, &len)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't read %s", source);
            return 0;
        }
        /*
            Use layouts iff there is a source defined on the route. Only MVC/controllers based apps do this.
         */
        if (eroute->layoutsDir) {
#if UNUSED
        layout = mprSamePath(eroute->layoutsDir, route->dir) ? 0 : mprJoinPath(eroute->layoutsDir, "default.esp");
#else
        layout = mprJoinPath(eroute->layoutsDir, "default.esp");
#endif
        }
        if ((script = espBuildScript(route, page, source, cacheName, layout, &err)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't build %s, error %s", source, err);
            return 0;
        }
        csource = mprJoinPathExt(mprTrimPathExt(module), ".c");
        if ((fp = mprOpenFile(csource, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't open compiled script file %s", csource);
            return 0;
        }
        len = slen(script);
        if (mprWriteFile(fp, script, len) != len) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't write compiled script file %s", csource);
            mprCloseFile(fp);
            return 0;
        }
        mprCloseFile(fp);
    } else {
        csource = source;
    }
    mprMakeDir(eroute->cacheDir, 0775, -1, -1, 1);

    /* WARNING: GC yield here */
    if (runCommand(conn, eroute->compile, csource, module) < 0) {
        return 0;
    }
    if (eroute->link) {
        /* WARNING: GC yield here */
        if (runCommand(conn, eroute->link, csource, module) < 0) {
            return 0;
        }
#if !(BIT_DEBUG && MACOSX)
        /*
            MAC needs the object for debug information
         */
        mprDeletePath(mprJoinPathExt(mprTrimPathExt(module), &BIT_OBJ[1]));
#endif
    }
#if BIT_WIN_LIKE
    {
        /*
            Windows leaves intermediate object in the current directory
         */
        cchar   *obj;
        obj = mprReplacePathExt(mprGetPathBase(csource), "obj");
        if (mprPathExists(obj, F_OK)) {
            mprDeletePath(obj);
        }
    }
#endif
    if (!eroute->keepSource && isView) {
        mprDeletePath(csource);
    }
    return 1;
}


static char *joinLine(cchar *str, ssize *lenp)
{
    cchar   *cp;
    char    *buf, *bp;
    ssize   len;
    int     count, bquote;

    for (count = 0, cp = str; *cp; cp++) {
        if (*cp == '\n') {
            count++;
        }
    }
    len = slen(str);
    if ((buf = mprAlloc(len + (count * 3) + 1)) == 0) {
        return 0;
    }
    bquote = 0;
    for (cp = str, bp = buf; *cp; cp++) {
        if (*cp == '\n') {
            *bp++ = '\\';
            *bp++ = 'n';
            *bp++ = '\\';
        } else if (*cp == '\\' && cp[1] != '\\') {
            bquote++;
        }
        *bp++ = *cp;
    }
    *bp = '\0';
    *lenp = len - bquote;
    return buf;
}


/*
    Convert an ESP web page into C code
    Directives:

        <%@ include "file"  Include an esp file
        <%@ layout "file"   Specify a layout page to use. Use layout "" to disable layout management
        <%@ content         Mark the location to substitute content in a layout pag

        <%                  Begin esp section containing C code
        <%^ global          Put esp code at the global level
        <%^ start           Put esp code at the start of the function
        <%^ end             Put esp code at the end of the function

        <%=                 Begin esp section containing an expression to evaluate and substitute
        <%= [%fmt]          Begin a formatted expression to evaluate and substitute. Format is normal printf format.
                            Use %S for safe HTML escaped output.
        %>                  End esp section
        -%>                 End esp section and trim trailing newline

        @@var               Substitue the value of a variable. Var can also be simple expressions (without spaces)
        @#field             Lookup the current record for the value of the field.

 */
char *espBuildScript(HttpRoute *route, cchar *page, cchar *path, cchar *cacheName, cchar *layout, char **err)
{
    EspParse    parse;
    EspRoute    *eroute;
    char        *control, *incBuf, *incText, *global, *token, *body, *where;
    char        *rest, *start, *end, *include, *line, *fmt, *layoutPage, *layoutBuf;
    ssize       len;
    int         tid;

    mprAssert(page);

    eroute = route->eroute;
    body = start = end = global = "";
    *err = 0;

    memset(&parse, 0, sizeof(parse));
    parse.data = (char*) page;
    parse.next = parse.data;

    if ((parse.token = mprCreateBuf(-1, -1)) == 0) {
        return 0;
    }
    tid = getEspToken(&parse);
    while (tid != ESP_TOK_EOF) {
        token = mprGetBufStart(parse.token);
#if FUTURE
        if (parse.lineNumber != lastLine) {
            body = sfmt("\n# %d \"%s\"\n", parse.lineNumber, path);
        }
#endif
        switch (tid) {
        case ESP_TOK_CODE:
            if (*token == '^') {
                for (token++; *token && isspace((uchar) *token); token++) ;
                where = stok(token, " \t\r\n", &rest);
                if (rest == 0) {
                    ;
                } else if (scmp(where, "global") == 0) {
                    global = sjoin(global, rest, NULL);

                } else if (scmp(where, "start") == 0) {
                    if (*start == '\0') {
                        start = "  ";
                    }
                    start = sjoin(start, rest, NULL);

                } else if (scmp(where, "end") == 0) {
                    if (*end == '\0') {
                        end = "  ";
                    }
                    end = sjoin(end, rest, NULL);
                }
            } else {
                body = sjoin(body, token, NULL);
            }
            break;

        case ESP_TOK_CONTROL:
            /* NOTE: layout parsing not supported */
            control = stok(token, " \t\r\n", &token);
            if (scmp(control, "content") == 0) {
                body = sjoin(body, CONTENT_MARKER, NULL);

            } else if (scmp(control, "include") == 0) {
                if (token == 0) {
                    token = "";
                }
                token = strim(token, " \t\r\n\"", MPR_TRIM_BOTH);
                token = mprNormalizePath(token);
                if (token[0] == '/') {
                    include = sclone(token);
                } else {
                    include = mprJoinPath(mprGetPathDir(path), token);
                }
                if ((incText = mprReadPathContents(include, &len)) == 0) {
                    *err = sfmt("Can't read include file: %s", include);
                    return 0;
                }
                /* Recurse and process the include script */
                incBuf = 0;
                if ((incBuf = espBuildScript(route, incText, include, NULL, NULL, err)) == 0) {
                    return 0;
                }
                body = sjoin(body, incBuf, NULL);

            } else if (scmp(control, "layout") == 0) {
                token = strim(token, " \t\r\n\"", MPR_TRIM_BOTH);
                if (*token == '\0') {
                    layout = 0;
                } else {
                    token = mprNormalizePath(token);
                    if (token[0] == '/') {
                        layout = sclone(token);
                    } else {
                        layout = mprJoinPath(eroute->layoutsDir ? eroute->layoutsDir : mprGetPathDir(path), token);
                    }
                    if (!mprPathExists(layout, F_OK)) {
                        *err = sfmt("Can't access layout page %s", layout);
                        return 0;
                    }
                }

            } else {
                *err = sfmt("Unknown control %s at line %d", control, parse.lineNumber);
                return 0;                
            }
            break;

        case ESP_TOK_ERR:
            return 0;

        case ESP_TOK_EXPR:
            /* <%= expr %> */
            if (*token == '%') {
                fmt = stok(token, ": \t\r\n", &token);
                if (token == 0) { 
                    token = "";
                }
            } else {
                fmt = "%s";
            }
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            body = sjoin(body, "  espRender(conn, \"", fmt, "\", ", token, ");\n", NULL);
            break;

        case ESP_TOK_FIELD:
            /* @#field */
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            body = sjoin(body, "  espRender(conn, getField(\"", token, "\"));\n", NULL);
            break;

        case ESP_TOK_LITERAL:
            line = joinLine(token, &len);
            body = sfmt("%s  espRenderBlock(conn, \"%s\", %d);\n", body, line, len);
            break;

        case ESP_TOK_VAR:
            /* @@var */
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            /* espRenderParam uses espRenderSafeString */
            body = sjoin(body, "  espRenderParam(conn, \"", token, "\");\n", NULL);
            break;

        default:
            return 0;
        }
        tid = getEspToken(&parse);
    }
    if (cacheName) {
        /*
            CacheName will only be set for the outermost invocation
         */
        if (layout && mprPathExists(layout, R_OK)) {
            if ((layoutPage = mprReadPathContents(layout, &len)) == 0) {
                *err = sfmt("Can't read layout page: %s", layout);
                return 0;
            }
            layoutBuf = 0;
            if ((layoutBuf = espBuildScript(route, layoutPage, layout, NULL, NULL, err)) == 0) {
                return 0;
            }
#if BIT_DEBUG
            if (!scontains(layoutBuf, CONTENT_MARKER, -1)) {
                *err = sfmt("Layout page is missing content marker: %s", layout);
                return 0;
            }
#endif
            body = sreplace(layoutBuf, CONTENT_MARKER, body);
        }
        if (start && start[slen(start) - 1] != '\n') {
            start = sjoin(start, "\n", NULL);
        }
        if (end && end[slen(end) - 1] != '\n') {
            end = sjoin(end, "\n", NULL);
        }
        mprAssert(slen(path) > slen(route->dir));
        mprAssert(sncmp(path, route->dir, slen(route->dir)) == 0);
        if (sncmp(path, route->dir, slen(route->dir)) == 0) {
            path = &path[slen(route->dir) + 1];
        }
        body = sfmt(\
            "/*\n   Generated from %s\n */\n"\
            "#include \"esp-app.h\"\n"\
            "%s\n"\
            "static void %s(HttpConn *conn) {\n"\
            "%s%s%s"\
            "}\n\n"\
            "%s int esp_%s(HttpRoute *route, MprModule *module) {\n"\
            "   espDefineView(route, \"%s\", %s);\n"\
            "   return 0;\n"\
            "}\n",
            path, global, cacheName, start, body, end, ESP_EXPORT_STRING, cacheName, mprGetPortablePath(path), cacheName);
        mprLog(6, "Create ESP script: \n%s\n", body);
    }
    return body;
}


static bool addChar(EspParse *parse, int c)
{
    if (mprPutCharToBuf(parse->token, c) < 0) {
        return 0;
    }
    mprAddNullToBuf(parse->token);
    return 1;
}


static char *eatSpace(EspParse *parse, char *next)
{
    for (; *next && isspace((uchar) *next); next++) {
        if (*next == '\n') {
            parse->lineNumber++;
        }
    }
    return next;
}


static char *eatNewLine(EspParse *parse, char *next)
{
    for (; *next && isspace((uchar) *next); next++) {
        if (*next == '\n') {
            parse->lineNumber++;
            next++;
            break;
        }
    }
    return next;
}


/*
    Get the next ESP input token. input points to the next input token.
    parse->token will hold the parsed token. The function returns the token id.
 */
static int getEspToken(EspParse *parse)
{
    char    *start, *end, *next;
    int     tid, done, c;

    start = next = parse->next;
    end = &start[slen(start)];
    mprFlushBuf(parse->token);
    tid = ESP_TOK_LITERAL;

    for (done = 0; !done && next < end; next++) {
        c = *next;
        switch (c) {
        case '<':
            if (next[1] == '%' && ((next == start) || next[-1] != '\\')) {
                next += 2;
                if (mprGetBufLength(parse->token) > 0) {
                    next -= 3;
                } else {
                    next = eatSpace(parse, next);
                    if (*next == '=') {
                        /*
                            <%= directive
                         */
                        tid = ESP_TOK_EXPR;
                        next = eatSpace(parse, ++next);
                        while (next < end && !(*next == '%' && next[1] == '>' && next[-1] != '\\')) {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }

                    } else if (*next == '@') {
                        tid = ESP_TOK_CONTROL;
                        next = eatSpace(parse, ++next);
                        while (next < end && !(*next == '%' && next[1] == '>' && next[-1] != '\\')) {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }
                        
                    } else {
                        tid = ESP_TOK_CODE;
                        while (next < end && !(*next == '%' && next[1] == '>' && next[-1] != '\\')) {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }
                    }
                    if (*next && next > start && next[-1] == '-') {
                        /* Remove "-" */
                        mprAdjustBufEnd(parse->token, -1);
                        mprAddNullToBuf(parse->token);
                        next = eatNewLine(parse, next + 2) - 1;
                    } else {
                        next++;
                    }
                }
                done++;
            } else {
                if (!addChar(parse, c)) {
                    return ESP_TOK_ERR;
                }                
            }
            break;

        case '@':
            if ((next == start) || next[-1] != '\\') {
                if (next[1] == '@' || next[1] == '*') {
                    next += 2;
                    if (mprGetBufLength(parse->token) > 0) {
                        next -= 3;
                    } else {
                        tid = next[-1] == '@' ? ESP_TOK_VAR : ESP_TOK_FIELD;
                        next = eatSpace(parse, next);
                        while (isalnum((uchar) *next) || *next == '_') {
                            if (*next == '\n') parse->lineNumber++;
                            if (!addChar(parse, *next++)) {
                                return ESP_TOK_ERR;
                            }
                        }
                        next--;
                    }
                    done++;
                }
            }
            break;

        case '\n':
            parse->lineNumber++;
            /* Fall through */

        case '\r':
        default:
            if (c == '\"' || c == '\\') {
                if (!addChar(parse, '\\')) {
                    return ESP_TOK_ERR;
                }
            }
            if (!addChar(parse, c)) {
                return ESP_TOK_ERR;
            }
            break;
        }
    }
    if (mprGetBufLength(parse->token) == 0) {
        tid = ESP_TOK_EOF;
    }
    parse->next = next;
    return tid;
}


static cchar *getEnvString(EspRoute *eroute, cchar *key, cchar *defaultValue)
{
    cchar   *value;

    if (!eroute || !eroute->env || (value = mprLookupKey(eroute->env, key)) == 0) {
        if ((value = getenv(key)) == 0) {
            if (defaultValue) {
                value = defaultValue;
            } else {
                value = sfmt("${%s}", key);
            }
        }
    }
    return value;
}


static cchar *getShobjExt(cchar *os)
{
    if (smatch(os, "macosx")) {
        return ".dylib";
    } else if (smatch(os, "windows")) {
        return ".dll";
    } else if (smatch(os, "vxworks")) {
        return ".out";
    } else {
        return ".so";
    }
}


static cchar *getShlibExt(cchar *os)
{
    if (smatch(os, "macosx")) {
        return ".dylib";
    } else if (smatch(os, "windows")) {
        return ".lib";
    } else if (smatch(os, "vxworks")) {
        return ".a";
    } else {
        return ".so";
    }
}


static cchar *getObjExt(cchar *os)
{
    if (smatch(os, "windows")) {
        return ".obj";
    }
    return ".o";
}


static cchar *getCompilerName(cchar *os, cchar *arch)
{
    cchar       *name;

    name = "gcc";
    if (smatch(os, "vxworks")) {
        if (smatch(arch, "x86") || smatch(arch, "i586") || smatch(arch, "i686") || smatch(arch, "pentium")) {
            name = "ccpentium";
        } else if (scontains(arch, "86", -1)) {
            name = "cc386";
        } else if (scontains(arch, "ppc", -1)) {
            name = "ccppc";
        } else if (scontains(arch, "xscale", -1) || scontains(arch, "arm", -1)) {
            name = "ccarm";
        } else if (scontains(arch, "68", -1)) {
            name = "cc68k";
        } else if (scontains(arch, "sh", -1)) {
            name = "ccsh";
        } else if (scontains(arch, "mips", -1)) {
            name = "ccmips";
        }
    } else if (smatch(os, "macosx")) {
        name = "clang";
    }
    return name;
}


static cchar *getVxCPU(cchar *arch)
{
    if (smatch(arch, "i386")) {
        return "I80386";
    } else if (smatch(arch, "i486")) {
        return "I80486";
    } else if (smatch(arch, "x86") | sends(arch, "86")) {
        return "PENTIUM";
    } 
    return supper(arch);
}


static cchar *getDebug()
{
    MaAppweb    *appweb;
    int         debug;

    appweb = MPR->appwebService;
    debug = sends(appweb->platform, "-debug");
    if (scontains(appweb->platform, "windows-", -1)) {
        return (debug) ? "-DBIT_DEBUG -Zi -Od" : "-O";
    }
    return (debug) ? "-DBIT_DEBUG -g" : "-O2";
}


static cchar *getLibs(cchar *os)
{
    cchar       *libs;

    if (smatch(os, "windows")) {
        libs = "\"${LIBPATH}\\mod_esp${SHLIB}\" \"${LIBPATH}\\libappweb.lib\" \"${LIBPATH}\\libhttp.lib\" \"${LIBPATH}\\libmpr.lib\" \"${LIBPATH}\\libmprssl.lib\"";
    } else {
        libs = "${LIBPATH}/mod_esp${SHOBJ} -lappweb -lpcre -lhttp -lmpr -lmprssl -lpthread -lm";
    }
    return libs;
}


#if UNUSED
static char *getLibpath(cchar *name)
{
    return mprNormalizePath(sfmt("%s/../%s", mprGetAppDir(), name)); 
    eroute->
         mprNormalizePath(mprJoinPath(mprGetAppDir(), "../inc")
    return mprJoin
#if DEBUG_IDE
    //  MOB - not right for cross dev
    return mprGetAppDir();
#else
    //  MOB - not right for cross dev
    return mprNormalizePath(sfmt("%s/../%s", mprGetAppDir(), name)); 
#endif
}
#endif


static bool matchToken(cchar **str, cchar *token)
{
    ssize   len;

    len = slen(token);
    if (sncmp(*str, token, len) == 0) {
        *str += len;
        return 1;
    }
    return 0;
}


//  MOB order
static cchar *getMappedArch(cchar *arch)
{
    if (smatch(arch, "x64")) {
        arch = "x86_64";
    } else if (smatch(arch, "x86")) {
        arch = "i686";
    }
    return arch;
}

static cchar *getWinSDK()
{
#if WINDOWS
    cchar   *path;

    path = mprReadRegistry("HKLM\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows", "CurrentInstallFolder");
    if (!path) {
        path = "${WINSDK}";
    }
    return strim(path, "\\", MPR_TRIM_END);
#else
    return "";
#endif
}


static cchar *getVisualStudio()
{
#if WINDOWS
    cchar   *path;

    //  MOB - check this key. Does 'Visual Studio' have a space?
    path = mprReadRegistry("HKLM\\SOFTWARE\\Microsoft\\VisualStudio\\" BIT_VISUAL_STUDIO_VERSION, "ShellFolder");
    if (!path) {
        path = mprReadRegistry("HKCU\\SOFTWARE\\Microsoft\\VisualStudio\\" BIT_VISUAL_STUDIO_VERSION "_Config", 
            "ShellFolder");
        if (!path) {
            path = "${VS}";
        }
    }
    return strim(path, "\\", MPR_TRIM_END);
#else
    return "";
#endif
}


static cchar *getCompilerPath(cchar *os, cchar *arch)
{
#if WINDOWS
    /* 
        Get the real system architecture (32 or 64 bit)
     */
    MaAppweb *appweb = MPR->appwebService;
    cchar *path = getVisualStudio();
    if (scontains(appweb->platform, "-x64-", -1)) {
        int is64BitSystem = smatch(getenv("PROCESSOR_ARCHITECTURE"), "AMD64") || getenv("PROCESSOR_ARCHITEW6432");
        if (is64BitSystem) {
            path = mprJoinPath(path, "VC/bin/amd64/cl.exe");
        } else {
            /* Cross building on a 32-bit system */
            path = mprJoinPath(path, "VC/bin/x86_amd64/cl.exe");
        }
    } else {
        path = mprJoinPath(path, "VC/bin/cl.exe");
    }
    return path;
#else
    return getCompilerName(os, arch);
#endif
}

#endif /* BIT_FEATURE_ESP */
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
