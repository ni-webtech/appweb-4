/*
    espTemplate.c -- Templated web pages with embedded C code.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
#include    "esp.h"

/************************************ Forwards ********************************/

static int getEspToken(EspParse *parse);

/************************************* Code ***********************************/

static char *getOutDir(cchar *name)
{
#if DEBUG_IDE
    return mprGetAppDir();
#else
    return mprGetNormalizedPath(sfmt("%s/../%s", mprGetAppDir(), name)); 
#endif
}


/*
    Tokens:
    ARCH        Build architecture (x86_64)
    CC          Compiler (cc)
    DEBUG       Debug compilation options (-g, -Zi -Od)
    INC         Include directory out/inc
    LIBDIR      Library directory (out/lib, xcode/VS: out/bin)
    OBJ         Name of compiled source (out/lib/view-MD5.o)
    OUT         Output module (view_MD5.dylib)
    SHLIB       Shared library (.lib, .so)
    SHOBJ       Shared Object (.dll, .so)
    SRC         Source code for view or controller (already templated)
 */
char *espExpandCommand(cchar *command, cchar *source, cchar *module)
{
    MprBuf  *buf;
    cchar   *cp, *out;
    
    if (command == 0) {
        return 0;
    }
    out = mprTrimPathExtension(module);
    buf = mprCreateBuf(-1, -1);

    for (cp = command; *cp; cp++) {
		if (*cp == '$') {
            if (sncmp(cp, "${ARCH}", 7) == 0) {
                /* Build architecture */
                mprPutStringToBuf(buf, BLD_HOST_CPU);
                cp += 6;
            } else if (sncmp(cp, "${DEBUG}", 8) == 0) {
                mprPutStringToBuf(buf, ESP_DEBUG);
                cp += 7;
            } else if (sncmp(cp, "${INC}", 6) == 0) {
                /* Include directory (out/inc) */
                mprPutStringToBuf(buf, mprResolvePath(mprGetAppDir(), BLD_INC_NAME));
                cp += 5;
            } else if (sncmp(cp, "${LIBDIR}", 9) == 0) {
                /* Library directory. IDE's use bin dir */
                mprPutStringToBuf(buf, getOutDir(BLD_LIB_NAME));
                cp += 8;
            } else if (sncmp(cp, "${OBJ}", 6) == 0) {
                /* Output object with extension (.o) */
                mprPutStringToBuf(buf, mprJoinPathExt(out, BLD_OBJ));
                cp += 5;
            } else if (sncmp(cp, "${OUT}", 6) == 0) {
                /* Output modules */
                mprPutStringToBuf(buf, out);
                cp += 5;
            } else if (sncmp(cp, "${SHLIB}", 8) == 0) {
                /* .lib */
                mprPutStringToBuf(buf, BLD_SHLIB);
                cp += 7;
            } else if (sncmp(cp, "${SHOBJ}", 8) == 0) {
                /* .dll */
                mprPutStringToBuf(buf, BLD_SHOBJ);
                cp += 7;
            } else if (sncmp(cp, "${SRC}", 6) == 0) {
                /* View (already parsed into C code) or controller source */
                mprPutStringToBuf(buf, source);
                cp += 5;
            } else if (sncmp(cp, "${LIBS}", 7) == 0) {
                /* Required libraries to link. These may have nested ${TOKENS} */
                mprPutStringToBuf(buf, espExpandCommand(ESP_LIBS, source, module));
                cp += 6;
            } else {
                mprPutCharToBuf(buf, *cp);
            }
        } else {
            mprPutCharToBuf(buf, *cp);
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


static int runCommand(HttpConn *conn, cchar *command, cchar *csource, cchar *module)
{
    EspReq      *req;
    EspLoc      *el;
    MprCmd      *cmd;
    char        *err, *out;

    req = conn->data;
    el = req->el;

    cmd = mprCreateCmd(conn->dispatcher);
    if ((req->commandLine = espExpandCommand(command, csource, module)) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing EspCompile directive for %s", csource);
        return MPR_ERR_CANT_READ;
    }
    mprLog(4, "ESP command: %s\n", req->commandLine);
    if (el->env) {
        mprAddNullItem(el->env);
        mprSetCmdDefaultEnv(cmd, (cchar**) &el->env->items[0]);
    }
    if (el->searchPath) {
        mprSetCmdSearchPath(cmd, el->searchPath);
    }
    //  WARNING: GC will run here
	if (mprRunCmd(cmd, req->commandLine, &out, &err, 0) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
		if (el->showErrors) {
			httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't run command %s, error %s", req->commandLine, err);
		} else {
			httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't run command %s", req->commandLine);
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
    EspReq      *req;
    EspLoc      *el;
    cchar       *csource;
    char        *layout, *script, *page, *err;
    ssize       len;

    req = conn->data;
    el = req->el;

    if (isView) {
        if ((page = mprReadPath(source)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't read %s", source);
            return 0;
        }
        /*
            Use layouts iff there is a controller provided on the route
         */
        layout = (req->route->controllerName) ? mprJoinPath(el->layoutsDir, "default.esp") : 0;
        if ((script = espBuildScript(el, page, source, cacheName, layout, &err)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't build %s, error %s", source, err);
            return 0;
        }
        csource = mprJoinPathExt(mprTrimPathExtension(module), ".c");
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
    mprMakeDir(el->cacheDir, 0775, 1);
    /*
        WARNING: GC yield here
     */
    if (runCommand(conn, el->compile, csource, module) < 0) {
        return 0;
    }
    if (el->link) {
        /*
            WARNING: GC yield here
         */
        if (runCommand(conn, el->link, csource, module) < 0) {
            return 0;
        }
#if !(BLD_DEBUG && MACOSX)
        /*
            MAC needs the object for debug information
         */
        mprDeletePath(mprJoinPathExt(mprTrimPathExtension(module), BLD_OBJ));
#endif
    }
    if (!el->keepSource && isView) {
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
#if UNUSED
            for (s = &cp[1]; *s; s++) {
                if (!isspace((int) *s)) {
                    break;
                }
            }
            if (*s == '\0') {
                /* Improve readability - pull onto one line if the rest of the token is white-space */
                len--;
                continue;
            }
#endif
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

 */
char *espBuildScript(EspLoc *el, cchar *page, cchar *path, cchar *cacheName, cchar *layout, char **err)
{
    EspParse    parse;
    char        *control, *incBuf, *incText, *global, *token, *body, *where;
    char        *rest, *start, *end, *include, *line, *fmt, *layoutPage, *layoutBuf;
    ssize       len;
    int         tid;

    mprAssert(page);

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
        case ESP_TOK_LITERAL:
            line = joinLine(token, &len);
            body = sfmt("%s  espWriteBlock(conn, \"%s\", %d);\n", body, line, len);
            break;

        case ESP_TOK_VAR:
            /* @@var */
            token = strim(token, " \t\r\n;", MPR_TRIM_BOTH);
            body = sjoin(body, "  espWriteVar(conn, \"", token, "\");\n", NULL);
            //MOB  body = sjoin(body, "  espWriteSafeString(conn, espGetVar(conn, \"", token, "\", \"\"));\n", NULL);
            break;


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
            body = sjoin(body, "  espWrite(conn, \"", fmt, "\", ", token, ");\n", NULL);
            break;

        case ESP_TOK_CODE:
            if (*token == '^') {
                for (token++; *token && isspace((int) *token); token++) ;
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
                token = mprGetNormalizedPath(token);
                if (token[0] == '/') {
                    include = sclone(token);
                } else {
                    include = mprJoinPath(mprGetPathDir(path), token);
                }
                if ((incText = mprReadPath(include)) == 0) {
                    *err = sfmt("Can't read include file: %s", include);
                    return 0;
                }
                /* Recurse and process the include script */
                incBuf = 0;
                if ((incBuf = espBuildScript(el, incText, include, NULL, NULL, err)) == 0) {
                    return 0;
                }
                body = sjoin(body, incBuf, NULL);

            } else if (scmp(control, "layout") == 0) {
                token = strim(token, " \t\r\n\"", MPR_TRIM_BOTH);
                if (*token == '\0') {
                    layout = 0;
                } else {
                    token = mprGetNormalizedPath(token);
                    if (token[0] == '/') {
                        layout = sclone(token);
                    } else {
                        layout = mprJoinPath(mprGetPathDir(path), token);
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
        default:
            return 0;
        }
        tid = getEspToken(&parse);
    }
    if (cacheName) {
        /*
            CacheName will only be set for the outermost invocation
         */
        if (layout) {
            if ((layoutPage = mprReadPath(layout)) == 0) {
                *err = sfmt("Can't read layout page: %s", layout);
                return 0;
            }
            layoutBuf = 0;
            if ((layoutBuf = espBuildScript(el, layoutPage, layout, NULL, NULL, err)) == 0) {
                return 0;
            }
            body = sreplace(layoutBuf, CONTENT_MARKER, body);
        }
        if (start && start[slen(start) - 1] != '\n') {
            start = sjoin(start, "\n", 0);
        }
        if (end && end[slen(end) - 1] != '\n') {
            end = sjoin(end, "\n", 0);
        }
        mprAssert(slen(path) > slen(el->dir));
        mprAssert(sncmp(path, el->dir, slen(el->dir)) == 0);
        path = &path[slen(el->dir) + 1];
        
        body = sfmt(\
            "/*\n   Generated from %s\n */\n"\
            "#include \"esp.h\"\n"\
            "%s\n"\
            "static void %s(HttpConn *conn) {\n"\
            "%s%s%s"\
            "}\n\n"\
            "%s int espInit_%s(EspLoc *el, MprModule *module) {\n"\
            "   espDefineView(el, \"%s\", %s);\n"\
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
    for (; *next && isspace((int) *next); next++) {
        if (*next == '\n') {
            parse->lineNumber++;
        }
    }
    return next;
}


static char *eatNewLine(EspParse *parse, char *next)
{
    for (; *next && isspace((int) *next); next++) {
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
            if (next[1] == '@' && ((next == start) || next[-1] != '\\')) {
                next += 2;
                if (mprGetBufLength(parse->token) > 0) {
                    next -= 3;
                } else {
                    tid = ESP_TOK_VAR;
                    next = eatSpace(parse, next);
                    /* Format is:  @@[%5.2f],var */
#if UNUSED
                    while (isalnum((int) *next) || *next == '[' || *next == ']' || *next == '.' || *next == '$' || 
                            *next == '_' || *next == '"' || *next == '\'' || *next == ',' || *next == '%' || *next == ':' ||
                            *next == '(' || *next == ')') {
                        if (*next == '\n') parse->lineNumber++;
                        if (!addChar(parse, *next++)) {
                            return ESP_TOK_ERR;
                        }
                    }
#else
                    while (isalnum((int) *next) || *next == '_') {
                        if (*next == '\n') parse->lineNumber++;
                        if (!addChar(parse, *next++)) {
                            return ESP_TOK_ERR;
                        }
                    }
#endif
                    next--;
                }
                done++;
            }
            break;

        case '\n':  //  MOB - line number must be tracked in other sections above. Same in ejs.template
            parse->lineNumber++;
        case '\r':
        default:
            if (c == '\"' || c == '\\') {
                if (!addChar(parse, '\\')) {
                    return ESP_TOK_ERR;
                }
            }
#if UNUSED
            if (!isspace(c) || mprGetBufLength(parse->token) > 0) {
#endif
                if (!addChar(parse, c)) {
                    return ESP_TOK_ERR;
                }
#if UNUSED
            }
#endif
            break;
        }
    }
    if (mprGetBufLength(parse->token) == 0) {
        tid = ESP_TOK_EOF;
    }
    parse->next = next;
    return tid;
}


#endif /* BLD_FEATURE_ESP */
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
