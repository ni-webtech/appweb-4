/*
    espTemplate.c -- Templated web pages with embedded C code.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
    #include    "esp.h"

/************************************ Forwards ********************************/

static int buildScript(cchar *path, cchar *name, char *page, char **script, char **err);
static int getEspToken(EspParse *parse);
static char *readFileToMem(cchar *path);

/************************************* Code ***********************************/

static char *getOutDir(cchar *name)
{
#if DEBUG_IDE
    return mprGetAppDir();
#else
    return mprGetNormalizedPath(mprAsprintf("%s/../%s", mprGetAppDir(), name)); 
#endif
}


/*
    Tokens:
    ARCH        Build architecture (x86_64)
    CC          Compiler (cc)
    DEBUG       Debug compilation options (-g, -Zi -Od)
    INC         Include directory out/inc
    LIB         Library directory (out/lib, xcode/VS: out/bin)
    OBJ         Name of compiled source (out/lib/view-MD5.o)
    OUT         Output module (view_MD5.dylib)
    SHLIB       Shared library (.lib, .so)
    SHOBJ       Shared Object (.dll, .so)
    SRC         Source code for view or controller (already templated)
 */
static char *expandCommand(HttpConn *conn, cchar *command, cchar *source, cchar *module)
{
    Esp     *esp;
    EspReq  *req;
    MprBuf  *buf;
    cchar   *cp, *out;
    
    req = conn->data;
    esp = req->esp;
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
#if BLD_DEBUG
    #if WIN
                mprPutStringToBuf(buf, "-Zi -Od");
    #else
                mprPutStringToBuf(buf, "-g");
    #endif
#else
    #if WIN
                mprPutStringToBuf(buf, "-O");
    #else
                mprPutStringToBuf(buf, "-O2");
    #endif
#endif
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


static int runCommand(HttpConn *conn, cchar *command, cchar *csource, cchar *module, cchar *source)
{
    EspReq      *req;
    Esp         *esp;
    MprCmd      *cmd;
    char        *commandLine, *err, *out;

    req = conn->data;
    esp = req->esp;

    cmd = mprCreateCmd(conn->dispatcher);
    if ((commandLine = expandCommand(conn, command, csource, module)) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing EspCompile directive for %s", csource);
        return MPR_ERR_CANT_READ;
    }
    mprLog(4, "ESP command: %s\n", commandLine);
    if (esp->env) {
        mprAddNullItem(esp->env);
        mprSetDefaultCmdEnv(cmd, (cchar**) &esp->env->items[0]);
    }
	if (mprRunCmd(cmd, commandLine, &out, &err, 0) != 0) {
		if (err == 0 || *err == '\0') {
			/* Windows puts errors to stdout Ugh! */
			err = out;
		}
		if (esp->showErrors) {
			httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't run command %s, error %s", source, err);
		} else {
			httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't run command %s", source);
		}
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}


/*
    Compile a view or controller.

    name        MD5 base name
    source      ESP source file name
    module      Module file name
 */
bool espCompile(HttpConn *conn, cchar *name, cchar *source, char *module, int isView)
{
    MprFile     *fp;
    EspReq      *req;
    Esp         *esp;
    cchar       *csource;
    char        *script, *page, *err;
    ssize       len;

    req = conn->data;
    esp = req->esp;

    if (isView) {
        if ((page = readFileToMem(source)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't read %s", source);
            return 0;
        }
        if (buildScript(source, name, page, &script, &err) < 0) {
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
    if (runCommand(conn, esp->compile, csource, module, source) < 0) {
        return 0;
    }
    if (esp->link) {
        if (runCommand(conn, esp->link, csource, module, source) < 0) {
            return 0;
        }
#if !(DEBUG_IDE && MACOSX)
        /*
            Xcode needs the object for debug information
         */
        mprDeletePath(mprJoinPathExt(mprTrimPathExtension(module), BLD_OBJ));
#endif
    }
    if (!esp->keepSource && isView) {
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
        @@[%fmt:]var        Substitue the formatted value of a variable.

 */
static int buildScript(cchar *path, cchar *name, char *page, char **script, char **err)
{
    EspParse    parse;
    char        *control, *incBuf, *incText, *export, *global, *token, *body, *where;
    char        *rest, *start, *end, *include, *line, *fmt;
    ssize       len;
    int         tid, rc;

    mprAssert(script);
    mprAssert(page);

    rc = 0;
    body = start = end = global = "";
    *script = 0;
    *err = 0;

    memset(&parse, 0, sizeof(parse));
    parse.data = page;
    parse.next = parse.data;

    if ((parse.token = mprCreateBuf(-1, -1)) == 0) {
        return MPR_ERR_MEMORY;
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
        case ESP_TOK_EXPR:
            /* @@var */
            /* <%= expr %>r */
            if (*token == '%') {
                fmt = stok(token, ": \t\r\n", &token);
                if (token == 0) { 
                    token = "";
                }
            } else {
                fmt = "%s";
            }
            token = strim(token, " \t\r\n", MPR_TRIM_BOTH);
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
            if (scmp(control, "include") == 0) {
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
                if ((incText = readFileToMem(include)) == 0) {
                    *err = mprAsprintf("Can't read include file: %s", include);
                    return MPR_ERR_CANT_READ;
                }
                /* Recurse and process the include script */
                incBuf = 0;
                if ((rc = buildScript(include, NULL, incText, &incBuf, err)) < 0) {
                    return rc;
                }
                body = sjoin(body, incBuf, NULL);

            } else {
                *err = mprAsprintf("Unknown control %s at line %d", control, parse.lineNumber);
                return MPR_ERR_BAD_STATE;;                
            }
            break;

        case ESP_TOK_ERR:
        default:
            return MPR_ERR_BAD_SYNTAX;
        }
        tid = getEspToken(&parse);
    }
    if (name) {
        /*
            Wrap the script
         */
#if BLD_WIN_LIKE
	export = "__declspec(dllexport) ";
#else
	export = "";
#endif
        if (start && start[slen(start) - 1] != '\n') {
            start = sjoin(start, "\n", 0);
        }
        if (end && end[slen(end) - 1] != '\n') {
            end = sjoin(end, "\n", 0);
        }
        *script = sfmt(\
            "/*\n   Generated from %s\n */\n"\
            "#include \"esp.h\"\n"\
            "%s\n"\
            "static void %s(HttpConn *conn) {\n"\
            "%s"\
            "%s"\
            "%s"\
            "}\n\n"\
            "%sint espInit_%s(Esp *esp, MprModule *module) {\n"\
            "   espDefineView(esp, \"%s\", %s);\n"\
            "   return 0;\n"\
            "}\n",
            path, global, name, start, body, end, export, name, mprGetPortablePath(path), name);
printf("SCRIPT: \n%s\n", *script);
    } else {
        *script = body;
    }
    return rc;
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
                    while (isalnum((int) *next) || *next == '[' || *next == ']' || *next == '.' || *next == '$' || 
                            *next == '_' || *next == '"' || *next == '\'' || *next == ',' || *next == '%' || *next == ':') {
                        if (*next == '\n') parse->lineNumber++;
                        if (!addChar(parse, *next++)) {
                            return ESP_TOK_ERR;
                        }
                    }
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


static char *readFileToMem(cchar *path)
{
    MprFile     *file;
    MprPath     info;
    char        *data;
    ssize       len;

    if (mprGetPathInfo(path, &info) < 0) {
        return 0;
    }
    len = (ssize) info.size;
    if ((file = mprOpenFile(path, O_RDONLY | O_BINARY, 0)) == 0) {
        return 0;
    }
    if ((data = mprAlloc(len + 1)) == 0) {
        return 0;
    }
    if (mprReadFile(file, data, len) != len) {
        return 0;
    }
    data[len] = '\0';
    return data;
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
