/*
    espRoute.c -- Router for ESP

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP
#include    "esp.h"
//  MOB - does esp require pcre?
#include    "pcre.h"

/************************************* Local **********************************/

static char *expandActionKey(HttpConn *conn, char *actionKey);
static void manageRoute(EspRoute *route, int flags);

/************************************* Code ***********************************/

EspRoute *espCreateRoute(cchar *name, cchar *methods, cchar *pattern, cchar *action, cchar *controller)
{
    EspRoute    *route;

    if ((route = mprAllocObj(EspRoute, manageRoute)) == 0) {
        return 0;
    }
    route->name = sclone(name);

    if (pattern) {
        route->pattern = sclone(pattern);
    }
    if (action) {
        route->action = sclone(action);
    }
    if (controller) {
        route->controllerName = sclone(controller);
    }
    if (methods == 0) {
        methods = "ALL";
    }
    route->methods = sclone(methods);
    if (route->pattern) {
        if (espFinalizeRoute(route) < 0) {
            return 0;
        }
    }
    return route;
}


static void manageRoute(EspRoute *route, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(route->name);
        mprMark(route->methods);
        mprMark(route->methodHash);
        mprMark(route->pattern);
        mprMark(route->action);
        mprMark(route->controllerName);
        mprMark(route->controllerPath);
        mprMark(route->patternExpression);
        mprMark(route->actionReplacement);
        mprMark(route->params);
        mprMark(route->tokens);
        mprMark(route->formFields);
        mprMark(route->headers);

    } else if (flags & MPR_MANAGE_FREE) {
        if (route->patternCompiled) {
            free(route->patternCompiled);
        }
    }
}

/*
    Compile the route into fast forms. This compiles the route->methods into a hash table, route->pattern into 
    a regular expression in route->patternCompiled.
    MOB - refactor into separate functions
 */
int espFinalizeRoute(EspRoute *route)
{
    MprBuf  *pattern, *params, *buf;
    cchar   *errMsg, *item;
    char    *methods, *method, *tok, *cp, *ep, *token, *field;
    int     column, errCode, index, next, braced;

    /*
        Convert the methods into a method hash
     */
    if (route->methodHash == 0) {
        if ((route->methodHash = mprCreateHash(-1, 0)) == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    methods = route->methods;
    if (scasecmp(route->methods, "ALL") == 0) {
        methods = "DELETE, GET, HEAD, OPTIONS, POST, PUT, TRACE";
    }
    methods = sclone(methods);
    while ((method = stok(methods, ", \t\n\r", &tok)) != 0) {
        mprAddKey(route->methodHash, method, route);
        methods = 0;
    }
    /*
        Prepare the pattern. 
            - Extract the tokens and change tokens: "{word}" to "(word)"
            - Change optional sections: "(portion)" to "(?:portion)?"
            - Create a params RE replacement string of the form "$1:$2:$3" for the {tokens}
            - Wrap the pattern in "^" and "$"
     */
    route->tokens = mprCreateList(-1, 0);
    pattern = mprCreateBuf(-1, -1);
    params = mprCreateBuf(-1, -1);

    if (route->pattern[0] == '%') {
        route->patternExpression = sclone(&route->pattern[1]);
        if (route->action) {
            route->actionReplacement = route->action;
        }
        /* Can't have a link template if using regular expressions */

    } else {
        for (cp = route->pattern; *cp; cp++) {
            if (*cp == '(') {
                mprPutStringToBuf(pattern, "(?:");

            } else if (*cp == ')') {
                mprPutStringToBuf(pattern, ")?");

            } else if (*cp == '{' && (cp == route->pattern || cp[-1] != '\\')) {
                if ((ep = schr(cp, '}')) != 0) {
                    /* Trim {} off the token and replace in pattern with "([^/]*)"  */
                    token = snclone(&cp[1], ep - cp - 1);
                    if ((field = schr(token, '=')) != 0) {
                        *field++ = '\0';
                    } else {
                        field = "([^/]*)";
                    }
                    mprPutStringToBuf(pattern, field);

                    index = mprAddItem(route->tokens, token);
                    /* Params ends up looking like "$1:$2:$3:$4" */
                    mprPutCharToBuf(params, '$');
                    mprPutIntToBuf(params, index + 1);
                    mprPutCharToBuf(params, ':');
                    cp = ep;
                }
            } else {
                mprPutCharToBuf(pattern, *cp);
            }
        }
        mprAddNullToBuf(pattern);
        mprAddNullToBuf(params);
        route->patternExpression = sclone(mprGetBufStart(pattern));

        /* Trim last ":" from params */
        route->params = sclone(mprGetBufStart(params));
        route->params[slen(route->params) - 1] = '\0';

        /*
            Prepare the action. Change $token to $N
         */
        if (route->action) {
            buf = mprCreateBuf(-1, -1);
            for (cp = route->action; *cp; cp++) {
                if ((tok = schr(cp, '$')) != 0 && (tok == route->action || tok[-1] != '\\')) {
                    if (tok > cp) {
                        mprPutBlockToBuf(buf, cp, tok - cp);
                    }
                    if ((braced = (*++tok == '{')) != 0) {
                        tok++;
                    }
                    if (*tok == '&' || *tok == '\'' || *tok == '`' || *tok == '$') {
                        mprPutCharToBuf(buf, '$');
                        mprPutCharToBuf(buf, *tok);
                        ep = tok + 1;
                    } else {
                        for (ep = tok; *ep && *ep != '}'; ep++) ;
                        token = snclone(tok, ep - tok);
                        if (schr(token, ':')) {
                            mprPutStringToBuf(buf, "$${");
                            mprPutStringToBuf(buf, token);
                            mprPutCharToBuf(buf, '}');
                        } else {
                            for (next = 0; (item = mprGetNextItem(route->tokens, &next)) != 0; ) {
                                if (scmp(item, token) == 0) {
                                    break;
                                }
                            }
                            if (item) {
                                mprPutCharToBuf(buf, '$');
                                mprPutIntToBuf(buf, next);
                            } else {
                                mprError("Can't find token \"%s\" in template \"%s\"", token, route->pattern);
                            }
                        }
                    }
                    if (braced) {
                        ep++;
                    }
                    cp = ep - 1;
                } else {
                    mprPutCharToBuf(buf, *cp);
                }
            }
            mprAddNullToBuf(buf);
            route->actionReplacement = sclone(mprGetBufStart(buf));
        }

        /*
            Create a template for links. Strip "()" and "/.*" from the pattern.
         */
        route->template = sreplace(route->pattern, "(", "");
        route->template = sreplace(route->template, ")", "");
        route->template = sreplace(route->template, "/.*", "");
    }
    if ((route->patternCompiled = pcre_compile2(route->patternExpression, 0, &errCode, &errMsg, &column, NULL)) == 0) {
        mprError("Can't compile route. Error %s at column %d", errMsg, column); 
    }
    return 0;
}


char *AApcre_extract(cchar *str, cchar *replacement, int *matches, int matchCount)
{
    MprBuf  *result;
    cchar   *end, *cp, *lastReplace;
    int     submatch;

    if (matchCount <= 0) {
        return MPR->emptyString;
    }
    result = mprCreateBuf(-1, -1);

    lastReplace = replacement;
    end = &replacement[slen(replacement)];

    for (cp = replacement; cp < end; ) {
        if (*cp == '$') {
            if (lastReplace < cp) {
                mprPutSubStringToBuf(result, lastReplace, (int) (cp - lastReplace));
            }
            switch (*++cp) {
            case '$':
                mprPutCharToBuf(result, '$');
                break;
            case '&':
                /* Replace the matched string */
                mprPutSubStringToBuf(result, &str[matches[0]], matches[1] - matches[0]);
                break;
            case '`':
                /* Insert the portion that preceeds the matched string */
                mprPutSubStringToBuf(result, str, matches[0]);
                break;
            case '\'':
                /* Insert the portion that follows the matched string */
                mprPutSubStringToBuf(result, &str[matches[1]], slen(str) - matches[1]);
                break;
            default:
                /* Insert the nth submatch */
                if (isdigit((int) *cp)) {
                    submatch = (int) wtoi(cp, 10, NULL);
                    while (isdigit((int) *++cp))
                        ;
                    cp--;
                    if (submatch < matchCount) {
                        submatch *= 2;
                        mprPutSubStringToBuf(result, &str[matches[submatch]], matches[submatch + 1] - matches[submatch]);
                    }
                } else {
                    mprError("Bad replacement $ specification in page");
                    return 0;
                }
            }
            lastReplace = cp + 1;
        }
        cp++;
    }
    if (lastReplace < cp && lastReplace < end) {
        mprPutSubStringToBuf(result, lastReplace, (int) (cp - lastReplace));
    }
    mprAddNullToBuf(result);
    return sclone(mprGetBufStart(result));
}


char *espMatchRoute(HttpConn *conn, EspRoute *route)
{
    EspPair     *pair;
    HttpRx      *rx;
    cchar       *token, *value, *header, *field;
    char        *actionKey;
    int         matches[ESP_MAX_ROUTE_MATCHES * 3], matchCount, next, start, rc;
    int         smatches[10 * 3], count;

    start = 0;
    rx = conn->rx;

    /*
        Match route->pattern
     */
    if ((matchCount = pcre_exec(route->patternCompiled, NULL, rx->pathInfo, (int) slen(rx->pathInfo), start, 0, 
            matches, sizeof(matches) / sizeof(int))) < 0) {
        return 0;
    }

    /*
        Match methods
     */
    if (!mprLookupKey(route->methodHash, rx->method)) {
        return 0;
    }

    if (route->headers) {
        for (next = 0; (pair = mprGetNextItem(route->headers, &next)) != 0; ) {
            if ((header = httpGetHeader(conn, pair->key)) != 0) {
                count = pcre_exec(pair->data, NULL, header, (int) slen(header), 0, 0, smatches, 10); 
                rc = count > 0;
                if (pair->flags & ESP_PAIR_NOT) {
                    rc = !rc;
                }
                if (!rc) {
                    return 0;
                }
            }
        }
    }
    if (route->formFields) {
        for (next = 0; (pair = mprGetNextItem(route->formFields, &next)) != 0; ) {
            if ((field = httpGetFormVar(conn, pair->key, "")) != 0) {
                count = pcre_exec(pair->data, NULL, field, (int) slen(field), 0, 0, smatches, 10); 
                rc = count > 0;
                if (pair->flags & ESP_PAIR_NOT) {
                    rc = !rc;
                }
                if (!rc) {
                    return 0;
                }
            }
        }
    }

    if (route->params) {
        for (next = 0; (token = mprGetNextItem(route->tokens, &next)) != 0; ) {
            value = snclone(&rx->pathInfo[matches[next * 2]], matches[(next * 2) + 1] - matches[(next * 2)]);
            httpSetFormVar(conn, token, value);
        }
    }
    /*
        Determine name of action-key to invoke
     */
    if (route->actionReplacement) {
        /*
            actionReplacement is "$N-$N $N"
         */
        actionKey = AApcre_extract(rx->pathInfo, route->actionReplacement, matches, matchCount);
        actionKey = expandActionKey(conn, actionKey);
    } else {
        actionKey = sclone(&rx->pathInfo[1]);
    }
    return actionKey;
}


/*
    WARNING: actionKey is modified
 */
static char *expandActionKey(HttpConn *conn, char *actionKey)
{
    MprBuf  *buf;
    char    *tok, *cp, *key, *value;

    buf = mprCreateBuf(-1, -1);
    for (cp = actionKey; cp && *cp; ) {
        if ((tok = strstr(cp, "${")) == 0) {
            break;
        }
        if (tok > cp) {
            mprPutBlockToBuf(buf, cp, tok - cp);
        }
        if ((key = stok(&tok[2], ":", &value)) == 0) {
            continue;
        }
        stok(value, "}", &cp);
        if (scasecmp(key, "header") == 0) {
            mprPutStringToBuf(buf, httpGetHeader(conn, value));
        } else if (scasecmp(key, "field") == 0) {
            mprPutStringToBuf(buf, httpGetFormVar(conn, value, ""));
        }
    }
    if (tok) {
        if (tok > cp) {
            mprPutBlockToBuf(buf, tok, tok - cp);
        }
    } else {
        mprPutStringToBuf(buf, cp);
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
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
