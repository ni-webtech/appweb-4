/*
    espTemplate.c -- 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
    #include    "esp.h"

/************************************* Local **********************************/

/************************************ Forwards ********************************/

static int buildScript(cchar *path, cchar *name, char *page, char **script, char **err);
static int getEspToken(int state, EspParse *parse);
static char *readFileToMem(cchar *path);

/************************************* Code ***********************************/

static char *getOutDir(cchar *name)
{
#if DEBUG_IDE
    return mprGetAppDir();
#else
    return mprAsprintf("%s/../%s", mprGetAppDir(), name); 
#endif
}


static char *getCompileCommand(HttpConn *conn, cchar *source, cchar *module)
{
    Esp     *esp;
    EspReq  *req;
    MprBuf  *buf;
    char    *cp, *out;
    
    req = conn->data;
    esp = req->esp;
    out = mprTrimPathExtension(module);
    buf = mprCreateBuf(-1, -1);

    for (cp = esp->compile; *cp; cp++) {
        if (cp > esp->compile && cp[-1] == '\\') {
            mprPutStringToBuf(buf, cp);
        } else if (*cp == '$') {
            if (sncmp(cp, "${SRC}", 6) == 0) {
                //  MOB - Was out
                mprPutStringToBuf(buf, source);
                cp += 5;
            } else if (sncmp(cp, "${OUT}", 6) == 0) {
                mprPutStringToBuf(buf, out);
                cp += 5;
            } else if (sncmp(cp, "${INC}", 6) == 0) {
                mprPutStringToBuf(buf, getOutDir(BLD_INC_NAME));
                cp += 5;
            } else if (sncmp(cp, "${LIB}", 6) == 0) {
                mprPutStringToBuf(buf, getOutDir(BLD_LIB_NAME));
                cp += 5;
            } else if (sncmp(cp, "${SHLIB}", 8) == 0) {
                mprPutStringToBuf(buf, BLD_SHLIB);
                cp += 7;
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


bool espCompile(HttpConn *conn, cchar *name, cchar *path, char *module)
{
    MprCmd      *cmd;
    MprFile     *fp;
    EspReq      *req;
    Esp         *esp;
    char        *source, *script, *commandLine, *page, *err, *out;
    ssize       len;

    req = conn->data;
    esp = req->esp;
    if ((page = readFileToMem(path)) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't read view %s", path);
        return 0;
    }
    if (buildScript(path, name, page, &script, &err) < 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't build view %s, error %s", path, err);
        return 0;
    }
    source = mprJoinPathExt(mprTrimPathExtension(module), ".c");
    if ((fp = mprOpenFile(source, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't open compiled script file %s", source);
        return 0;
    }
    len = slen(script);
    if (mprWriteFile(fp, script, len) != len) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't write compiled script file %s", source);
        mprCloseFile(fp);
        return 0;
    }
    mprCloseFile(fp);

    cmd = mprCreateCmd(conn->dispatcher);
    commandLine = getCompileCommand(conn, path, module);
    printf("%s\n", commandLine);
    if (mprRunCmd(cmd, commandLine, &out, &err, 0) != 0) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Can't compile view %s, error %s", path, err);
        return 0;
    }
    if (!esp->keepSource) {
        mprDeletePath(source);
    }
    return 1;
}


static char *joinLine(cchar *str)
{
    cchar   *cp;
    char    *buf, *bp;
    ssize   len;
    int     count;

    for (count = 0, cp = str; *cp; cp++) {
        if (*cp == '\n') {
            count++;
        }
    }
    len = slen(str);
    if ((buf = mprAlloc(len + (count * 3) + 1)) == 0) {
        return 0;
    }
    for (cp = str, bp = buf; *cp; cp++) {
        if (*cp == '\n') {
            *bp++ = '\\';
            *bp++ = 'n';
            *bp++ = '\\';
        }
        *bp++ = *cp;
    }
    *bp = '\0';
    return buf;
}


/*
      Convert an ESP web page into C code
 */
static int buildScript(cchar *path, cchar *name, char *page, char **script, char **err)
{
    EspParse    parse;
    char        *include, *incBuf, *incText;
    int         state, tid, rc;

    mprAssert(script);
    mprAssert(page);

    rc = 0;
    state = ESP_STAGE_BEGIN;
    *script = 0;
    *err = 0;
    memset(&parse, 0, sizeof(parse));
    if ((parse.token = mprAlloc(ESP_TOK_INCR)) == 0) {
        return MPR_ERR_MEMORY;
    }
    parse.token[0] = '\0';
    parse.tokLen = ESP_TOK_INCR;
    parse.endp = &parse.token[parse.tokLen - 1];
    parse.tokp = parse.token;
    parse.inBuf = page;
    parse.inp = parse.inBuf;

    tid = getEspToken(state, &parse);
    while (tid != ESP_TOK_EOF) {
        switch (tid) {
        default:
        case ESP_TOK_ERR:
            return MPR_ERR_BAD_SYNTAX;
            
        case ESP_TOK_LITERAL:
            *script = sjoin(*script, "espWrite(conn, \"", joinLine(parse.token), "\", -1);\n", NULL);
            break;

        case ESP_TOK_ATAT:
            /*
                  Trick to get undefined variables to evaluate to "".  Catenate with "" to cause toString to run. 
             */
            *script = sjoin(*script, "espWrite(conn, \"\" + ", joinLine(parse.token), ", -1);\n", NULL);
            break;

        case ESP_TOK_EQUALS:
            *script = sjoin(*script, "espWrite(conn, \"\" + (", joinLine(parse.token), ", -1));\n", NULL);
            state = ESP_STAGE_IN_ESP_TAG;
            break;

        case ESP_TOK_START_ESP:
            state = ESP_STAGE_IN_ESP_TAG;
            tid = getEspToken(state, &parse);
            while (tid != ESP_TOK_EOF && tid != ESP_TOK_EOF && tid != ESP_TOK_END_ESP) {
                *script = sjoin(*script, joinLine(parse.token), NULL);
                tid = getEspToken(state, &parse);
            }
            state = ESP_STAGE_BEGIN;
            break;

        case ESP_TOK_END_ESP:
            state = ESP_STAGE_BEGIN;
            break;

        case ESP_TOK_INCLUDE:
            /* NOTE: layout parsing not supported */
            if (parse.token[0] == '/') {
                include = sclone(parse.token);
            } else {
                include = mprJoinPath(mprGetPathDir(path), parse.token);
            }
            if ((incText = readFileToMem(include)) == 0) {
                *err = mprAsprintf("Can't read include file: %s", include);
                rc = MPR_ERR_CANT_READ;
                break;
            }
            /* Recurse and process the include script */
            incBuf = 0;
            if ((rc = buildScript(include, NULL, incText, &incBuf, err)) < 0) {
                return rc;
            }
            *script = sjoin(*script, incBuf, NULL);
            state = ESP_STAGE_IN_ESP_TAG;
            break;
        }
        tid = getEspToken(state, &parse);
    }
    if (name) {
        /*
            Wrap the script
         */
        printf("%s", *script);
        *script = sfmt(\
            "/*\n * Generated from %s\n */\n"\
            "#include \"esp.h\"\n\n"\
            "static void %s(HttpConn *conn) {\n"\
            "   %s\n"\
            "}\n\n"\
            "int espInit_%s(Esp *esp, MprModule *module) {\n"\
            "   espDefineView(esp, \"%s\", %s);\n"\
            "   return 0;\n"\
            "}\n",
            path, name, *script, name, path, name);
    }
    return rc;
}


/*
      Get room for more (nchar) characters in the token buffer
 */
static int growTokenBuf(EspParse *parse, int nchar)
{
    char    *newBuf;
    int     extra;

    mprAssert(parse);
    mprAssert(parse->tokp);
    mprAssert(parse->endp);
    mprAssert(nchar > 0);

    if (parse->tokp >= &parse->endp[- nchar]) {
        extra = max(nchar, ESP_TOK_INCR);
        if ((newBuf = mprRealloc(parse->token, parse->tokLen + extra)) == 0) {
            return ESP_TOK_ERR;
        }
        parse->tokp += (newBuf - parse->token);
        parse->endp += (newBuf - parse->token);
        parse->tokLen += extra;
        parse->endp += extra;
        parse->token = newBuf;
        *parse->tokp = '\0';
    }
    return 0;
}

/*
    Get a javascript identifier. Must allow x.y['abc'] or x.y["abc"].
    Must be careful about quoting and only allow quotes inside []. 
 */
static int getIdentifier(EspParse *parse)
{
    int     atQuote, prevC, c;

    mprAssert(parse);

    atQuote = 0;
    prevC = 0;
    c = *parse->inp++;

    while (isalnum(c) || c == '_' || c == '.' || c == '$' || c == '[' || c == ']' || c == '\'' || c == '\"') {
        if (c == '\'' || c == '\"') {
            if (c == atQuote) {
                atQuote = 0;
            } else if (prevC == '[') {
                atQuote = c;
            } else {
                break;
            }
        }
        if (growTokenBuf(parse, 2) < 0) {
            return ESP_TOK_ERR;
        }
        *parse->tokp++ = c;
        prevC = c;
        c = *parse->inp++;
    }

    parse->inp--;
    *parse->tokp = '\0';

    return 0;
}


/*
      Get the next ESP input token. input points to the next input token.
      parse->token will hold the parsed token. The function returns the token id.
 */
static int getEspToken(int state, EspParse *parse)
{
    char    *cp;
    int     tid, done, c, quoted;

    tid = ESP_TOK_LITERAL;
    parse->tokp = parse->token;
    parse->tokp[0] = '\0';
    quoted = 0;

    c = *parse->inp++;
    for (done = 0; !done; c = *parse->inp++) {

        /*
         *    Get room for 3 more characters in the token buffer
         */
        if (growTokenBuf(parse, 3) < 0) {
            return ESP_TOK_ERR;
        }
        switch (c) {
        case 0:
            if (*parse->token) {
                done++;
                parse->inp--;
                break;
            }
            return ESP_TOK_EOF;

        default:
            if (c == '\"' && state != ESP_STAGE_IN_ESP_TAG) {
                *parse->tokp++ = '\\';
            }
            *parse->tokp++ = c;
            quoted = 0;
            break;

        case '\\':
            if (state != ESP_STAGE_IN_ESP_TAG) {
                *parse->tokp++ = c;
            }
            quoted = 1;
            *parse->tokp++ = c;
            break;

        case '@':
            if (*parse->inp == '@' && state != ESP_STAGE_IN_ESP_TAG) {
                if (quoted) {
                    parse->tokp--;
                    quoted = 0;
                } else {
                    if (*parse->token) {
                        parse->inp--;
                    } else {
                        parse->inp++;
                        tid = ESP_TOK_ATAT;
                        if (getIdentifier(parse) < 0) {
                            return ESP_TOK_ERR;
                        }
                    }
                    done++;
                    break;
                }
            }
            *parse->tokp++ = c;
            break;

        case '<':
            if (*parse->inp == '%' && state != ESP_STAGE_IN_ESP_TAG) {
                if (quoted) {
                    parse->tokp--;
                    quoted = 0;
                    *parse->tokp++ = c;
                    break;
                }
                if (*parse->token) {
                    parse->inp--;
                    done++;
                    break;
                }
                parse->inp++;
                while (isspace((int) *parse->inp)) {
                    parse->inp++;
                }
                /*
                    <%= directive
                 */
                if (*parse->inp == '=') {
                    parse->inp++;
                    while (isspace((int) *parse->inp)) {
                        parse->inp++;
                    }
                    while (*parse->inp && (*parse->inp != '%' || parse->inp[1] != '>' || parse->inp[-1] == '\\')) {
                        if (growTokenBuf(parse, 2) < 0) {
                            return ESP_TOK_ERR;
                        }
                        *parse->tokp++ = *parse->inp++;
                    }
                    *parse->tokp = '\0';
                    tid = ESP_TOK_EQUALS;
                    done++;
                    break;
                }
                if (*parse->inp == 'i' && strncmp(parse->inp, "include", 7) == 0 && isspace((int) parse->inp[7])) {
                    tid = ESP_TOK_INCLUDE;
                    parse->inp += 7;
                    while (isspace((int) *parse->inp)) {
                        parse->inp++;
                    }
                //WAS while (*parse->inp && !isspace((int) *parse->inp) && *parse->inp != '%' && parse->tokp < parse->endp) 

                    while (*parse->inp && (*parse->inp != '%' || parse->inp[1] != '>' || parse->inp[-1] == '\\')) {
                        if (growTokenBuf(parse, 2) < 0) {
                            return ESP_TOK_ERR;
                        }
                        *parse->tokp++ = *parse->inp++;
                    }
                    *parse->tokp = '\0';
                    if (parse->token[0] == '"') {
                        parse->tokp = parse->token;
                        for (cp = &parse->token[1]; *cp; ) {
                            if (growTokenBuf(parse, 2) < 0) {
                                return ESP_TOK_ERR;
                            }
                            *parse->tokp++ = *cp++;
                        }
                        if (cp[-1] == '"') {
                            parse->tokp--;
                        }
                        *parse->tokp = '\0';
                    }
                    
                } else {
                    tid = ESP_TOK_START_ESP;
                }
                done++;
                break;
            }
            *parse->tokp++ = c;
            break;

        case '%':
            if (*parse->inp == '>' && state == ESP_STAGE_IN_ESP_TAG) {
                if (quoted) {
                    parse->tokp--;
                    quoted = 0;
                } else {
                    if (*parse->token) {
                        parse->inp--;
                    } else {
                        tid = ESP_TOK_END_ESP;
                        parse->inp++;
                    }
                    done++;
                    break;
                }
            }
            *parse->tokp++ = c;
            break;
        }
    }
    *parse->tokp = '\0';
    parse->inp--;
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
