/*
    espAbbrev.c -- ESP Abbreviated API

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"

#if BLD_FEATURE_ESP
/******************************* Abbreviated Controls *************************/ 

void alert(cchar *text, cchar *optionString)
{
    espAlert(getConn(), text, optionString);
}


void anchor(cchar *text, cchar *uri, cchar *optionString) 
{
    espAnchor(getConn(), text, uri, optionString);
}


void button(cchar *name, cchar *value, cchar *optionString)
{
    espButton(getConn(), name, value, optionString);
}


void buttonLink(cchar *text, cchar *uri, cchar *optionString)
{
    espButtonLink(getConn(), text, uri, optionString);
}


void chart(EdiGrid *grid, cchar *optionString)
{
    espChart(getConn(), grid, optionString);
}


void checkbox(cchar *field, cchar *checkedValue, cchar *optionString) 
{
    espCheckbox(getConn(), field, checkedValue, optionString);
}


void division(cchar *body, cchar *optionString) 
{
    espDivision(getConn(), body, optionString);
}


void endform() 
{
    espEndform(getConn());
}


void flash(cchar *kind, cchar *optionString)
{
    espFlash(getConn(), kind, optionString);
}


void form(void *record, cchar *optionString)
{
    HttpConn    *conn;

    conn = getConn();
    if (record == 0) {
        record = conn->record;
    }
    espForm(conn, record, optionString); 
}


void icon(cchar *uri, cchar *optionString)
{
    espIcon(getConn(), uri, optionString);
}


void image(cchar *src, cchar *optionString)
{
    espImage(getConn(), src, optionString);
}


void input(cchar *name, cchar *optionString)
{
    espInput(getConn(), name, optionString);
}


void label(cchar *text, cchar *optionString)
{
    espLabel(getConn(), text, optionString);
}


void dropdown(cchar *name, cchar *choices, cchar *optionString) 
{
    espDropdown(getConn(), name, choices, optionString);
}


void mail(cchar *name, cchar *address, cchar *optionString) 
{
    espMail(getConn(), name, address, optionString);
}


void progress(cchar *data, cchar *optionString)
{
    espProgress(getConn(), data, optionString);
}


void radio(cchar *name, void *choices, cchar *optionString)
{
    espRadio(getConn(), name, choices, optionString);
}


void refresh(cchar *on, cchar *off, cchar *optionString)
{
    espRefresh(getConn(), on, off, optionString);
}


void script(cchar *uri, cchar *optionString)
{
    espScript(getConn(), uri, optionString);
}


void securityToken()
{
    espSecurityToken(getConn());
}


void stylesheet(cchar *uri, cchar *optionString) 
{
    espStylesheet(getConn(), uri, optionString);
}


void table(EdiGrid *grid, cchar *optionString)
{
    espTable(getConn(), grid, optionString);
}


void tabs(EdiRec *rec, cchar *optionString)
{
    espTabs(getConn(), rec, optionString);
}


void text(cchar *field, cchar *optionString)
{
    espText(getConn(), field, optionString);
}


void tree(EdiGrid *grid, cchar *optionString)
{
    espTree(getConn(), grid, optionString);
}

/******************************* Abbreviated API ******************************/

void addHeader(cchar *key, cchar *fmt, ...)
{
    va_list     args;
    cchar       *value;

    va_start(args, fmt);
    value = sfmtv(fmt, args);
    espAddHeaderString(getConn(), key, value);
    va_end(args);
}


EdiRec *createRec(cchar *tableName, MprHash *params)
{
    return espCreateRec(getConn(), tableName, params);
}


void createSession()
{
    espCreateSession(getConn());
}


void destroySession()
{
    EspSession  *sp;
    HttpConn    *conn;

    conn = getConn();
    if ((sp = espGetSession(conn, 0)) != 0) {
        espDestroySession(sp);
    }
}


void dontAutoFinalize()
{
    espSetAutoFinalizing(getConn(), 0);
}


#if UNUSED
void error(cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFlashv(getConn(), "error", fmt, args);
    va_end(args);
}
#endif


void finalize()
{
    espFinalize(getConn());
}


void flush()
{
    espFlush(getConn());
}


MprList *getColumns(EdiRec *rec)
{
    return espGetColumns(getConn(), rec);
}


HttpConn *getConn()
{
    return mprGetThreadData(((Esp*) MPR->espService)->local);
}


cchar *getCookies()
{
    return espGetCookies(getConn());
}


MprOff getContentLength()
{
    return espGetContentLength(getConn());
}


cchar *getContentType()
{
    return getConn()->rx->mimeType;
}


Edi *getDatabase()
{
    return espGetDatabase(getConn());
}


EspRoute *getEspRoute()
{
    return espGetEspRoute(getConn());
}


cchar *getDir()
{
    return getConn()->rx->route->dir;
}


cchar *getField(cchar *field)
{
    return ediGetField(getRec(), field);
}


EdiGrid *getGrid()
{
    return getConn()->grid;
}


cchar *getHeader(cchar *key)
{
    return espGetHeader(getConn(), key);
}


cchar *getMethod()
{
    return espGetMethod(getConn());
}


cchar *getQuery()
{
    return getConn()->rx->parsedUri->query;
}


EdiRec *getRec()
{
    return getConn()->record;
}


cchar *getReferrer()
{
    return espGetReferrer(getConn());
}


cchar *getSessionVar(cchar *key)
{
    return espGetSessionVar(getConn(), key, "");
}


cchar *getTop()
{
    return espGetTop(getConn());
}


MprHash *getUploads()
{
    return espGetUploads(getConn());
}


cchar *getUri()
{
    return espGetUri(getConn());
}


bool hasGrid()
{
    return espHasGrid(getConn());
}


bool hasRec()
{
    return espHasRec(getConn());
}


void inform(cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFlashv(getConn(), "inform", fmt, args);
    va_end(args);
}


bool isEof()
{
    return httpIsEof(getConn());
}


bool isFinalized()
{
    return espIsFinalized(getConn());
}


bool isSecure()
{
    return espIsSecure(getConn());
}


EdiGrid *makeGrid(cchar *contents)
{
    return espMakeGrid(contents);
}


MprHash *makeHash(cchar *fmt, ...)
{
    va_list     args;
    cchar       *str;

    va_start(args, fmt);
    str = sfmtv(fmt, args);
    va_end(args);
    return espMakeHash("%s", str);
}


EdiRec *makeRec(cchar *contents)
{
    return ediMakeRec(contents);
}


cchar *makeUri(cchar *target)
{
    return espUri(getConn(), target);
}


cchar *param(cchar *key)
{
    return espGetParam(getConn(), key, NULL);
}


MprHash *params()
{
    return espGetParams(getConn());
}


ssize receive(char *buf, ssize len)
{
    return httpRead(getConn(), buf, len);
}


EdiRec *readRecWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return setRec(ediReadOneWhere(getDatabase(), tableName, fieldName, operation, value));
}


EdiRec *readRec(cchar *tableName)
{
    return setRec(ediReadRec(getDatabase(), tableName, param("id")));
}


EdiRec *readRecByKey(cchar *tableName, cchar *key)
{
    return setRec(ediReadRec(getDatabase(), tableName, key));
}


EdiGrid *readRecsWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return setGrid(ediReadWhere(getDatabase(), tableName, fieldName, operation, value));
}


EdiGrid *readTable(cchar *tableName)
{
    EdiGrid *grid;
    
    grid = ediReadWhere(getDatabase(), tableName, 0, 0, 0);
    setGrid(grid);
    return grid;
}


void redirect(cchar *target)
{
    espRedirect(getConn(), 302, target);
}


void redirectBack()
{
    espRedirectBack(getConn());
}


bool removeRec(cchar *tableName, cchar *key)
{
    return espRemoveRec(getConn(), tableName, key);
}


ssize render(cchar *fmt, ...)
{
    va_list     args;
    ssize       count;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    count = espRenderString(getConn(), msg);
    va_end(args);
    return count;
}


ssize renderCached()
{
    return espRenderCached(getConn());;
}


void renderError(int status, cchar *fmt, ...)
{
    va_list     args;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmt(fmt, args);
    espRenderError(getConn(), status, "%s", msg);
    va_end(args);
}


ssize renderFile(cchar *path)
{
    return espRenderFile(getConn(), path);
}


ssize renderSafe(cchar *fmt, ...)
{
    va_list     args;
    ssize       count;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    count = espRenderSafeString(getConn(), msg);
    va_end(args);
    return count;
}


void renderView(cchar *view)
{
    espRenderView(getConn(), view);
}


void setConn(HttpConn *conn)
{
    espSetConn(conn);
}


void setContentType(cchar *mimeType)
{
    espSetContentType(getConn(), mimeType);
}


void setCookie(cchar *name, cchar *value, cchar *path, cchar *cookieDomain, MprTime lifespan, bool isSecure)
{
    espSetCookie(getConn(), name, value, path, cookieDomain, lifespan, isSecure);
}


EdiRec *setField(EdiRec *rec, cchar *fieldName, cchar *value)
{
    return espSetField(rec, fieldName, value);
}


EdiRec *setFields(EdiRec *rec, MprHash *params)
{
    return espSetFields(rec, params);
}


void setFlash(cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFlashv(getConn(), kind, fmt, args);
    va_end(args);
}


EdiGrid *setGrid(EdiGrid *grid)
{
    getConn()->grid = grid;
    return grid;
}


void setHeader(cchar *key, cchar *fmt, ...)
{
    va_list     args;
    cchar       *value;

    va_start(args, fmt);
    value = sfmtv(fmt, args);
    espSetHeaderString(getConn(), key, value);
    va_end(args);
}


void setParam(cchar *key, cchar *value)
{
    espSetParam(getConn(), key, value);
}


EdiRec *setRec(EdiRec *rec)
{
    return espSetRec(getConn(), rec);
}


void setSessionVar(cchar *key, cchar *value)
{
    espSetSessionVar(getConn(), key, value);
}


void setStatus(int status)
{
    espSetStatus(getConn(), status);
}


void setTimeout(void *proc, MprTime timeout, void *data)
{
    mprCreateEvent(getConn()->dispatcher, "setTimeout", (int) timeout, proc, data, 0);
}


void showRequest()
{
    espShowRequest(getConn());
}


void updateCache(cchar *uri, cchar *data, int lifesecs)
{
    espUpdateCache(getConn(), uri, data, lifesecs);
}


bool updateField(cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    return espUpdateField(getConn(), tableName, key, fieldName, value);
}


bool updateFields(cchar *tableName, MprHash *params)
{
    return espUpdateFields(getConn(), tableName, params);
}


bool updateRec(EdiRec *rec)
{
    return espUpdateRec(getConn(), rec);
}


#if UNUSED
void warn(cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFlashv(getConn(), "warn", fmt, args);
    va_end(args);
}
#endif

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
