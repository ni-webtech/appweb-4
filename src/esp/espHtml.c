/*
    espHtml.c -- HTML controls 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"

#if BLD_FEATURE_ESP
/************************************* Local **********************************/

static char *defaultScripts[] = {
    "/js/jquery", 
    "/js/jquery.tablesorter",
    "/js/jquery.simplemodal",
    "/js/jquery.treeview",
    "/js/jquery.ejs",
    0,
};

static char *defaultStylesheets[] = {
    "/layout.css", 
    "/themes/default.css", 
    "/js/treeview.css", 
    0,
};

/************************************* Code ***********************************/

static char *map(cchar *options)
{
    //  Parse and store residuals somewhere
    //  Always return a leading space if returning a non-empty string
    return "";
}


static char *append(cchar *options, cchar *field, cchar *value)
{
    //  Somehow edit in the field
    return (char*) "";
}


static char *set(cchar *options, cchar *field, cchar *value)
{
    //  Set option if not already set
    return (char*) "";
}


static char *get(cchar *options, cchar *field, cchar *defaultValue)
{
    return (char*) "";
}


void espAlert(HttpConn *conn, cchar *text, cchar *options)
{
    //  MOB - what does ejs-alert mean. Consistency with all others
    options = append(options, "style", "-ejs-alert");
    espWrite(conn, "    <div %s>%s</div>\r\n", map(options), text);
}


void espAnchor(HttpConn *conn, cchar *text, cchar *options) 
{
    //  MOB -- what is it doing with data-click?

    char    *click;

    click = get(options, "data-click", "true");
    options = set(options, "data-click", "true");
    espWrite(conn, "<a href='%s' %s>%s</a>\r\n", click, map(options), text);
}


void espButton(HttpConn *conn, cchar *name, cchar *value, cchar *options)
{
    espWrite(conn, "    <input name='%s' type='submit' value='%s' %s />\r\n", name, value, map(options));
}


void espButtonLink(HttpConn *conn, cchar *text, cchar *options)
{
    espWrite(conn, "    <button %s>%s</button>\r\n", map(options), text);
}


void espChart(HttpConn *conn) 
{
    //  TODO
}

void espCheckbox(HttpConn *conn, cchar *field, cchar *value, cchar *checkedValue, cchar *options) 
{
    char    *checked;

    checked = scasematch(value, checkedValue) ? " checked='yes'" : "";
    espWrite(conn, "    <input name='%s' type='checkbox' %s %s value='%s' />\r\n", 
        field, map(options), checked, checkedValue);
    espWrite(conn, "    <input name='%s' type='hidden' %s value='' />\r\n", field, map(options));
}


void espDiv(HttpConn *conn, cchar *body, cchar *options) 
{
    espWrite(conn, "<div %s>%s</div>\r\n", map(options), body);
}


void espEmitFormErrors(HttpConn *conn)
{
    //MOB
}


void espEndform(HttpConn *conn) 
{
    espWriteString(conn, "</form>\r\n");
}


void espFlash(HttpConn *conn, cchar *kind, cchar *msg, cchar *options)
{
    options = append(options, "style", sfmt("-ejs-flash -ejs-flash-%s", kind));
    espWrite(conn, "<div %s>%s</div>\r\n", map(options));
}


void espForm(HttpConn *conn) 
{
    //MOB
}


void espIcon(HttpConn *conn, cchar *uri, cchar *options)
{
    espWrite(conn, "    <link href='%s' rel='shortcut icon' />\r\n", uri);
}


void espImage(HttpConn *conn, cchar *src, cchar *options)
{
    espWrite(conn, "    <img src='%s'%s />\r\n", src, map(options));
}


void espLabel(HttpConn *conn, cchar *text, cchar *options)
{
    espWrite(conn, "    <span %s>%s</span>\r\n", map(options), text);
}


void espList(HttpConn *conn, cchar *name, cchar *choices, cchar *defaultValue, cchar *options) 
{
    //  MOB
}


void espMail(HttpConn *conn, cchar *name, cchar *address, cchar *options) 
{
    espWrite(conn, "<a href='mailto:%s' %s>%s</a>\r\n", address, map(options), name);
}


void espProgress(HttpConn *conn, cchar *data, cchar *options)
{
    //  MOB not right
    options = append(options, "data-progress", data);
    espWrite(conn, "<div class='-ejs-progress'>\r\n    <div class='-ejs-progress-inner' %s>%s %%</div>\r\n</div>>\r\n", 
        map(options), data);
}


void espRadio(HttpConn *conn) {
    //  MOB
}


void espRefresh(HttpConn *conn, cchar *on, cchar *off, cchar *options)
{
    espWrite(conn, "    <img src='%s' data-on='%s' data-off='%s' class='-ejs-refresh' />\r\n", on, on, off);
}


void espScript(HttpConn *conn, cchar *uri, cchar *options)
{
    EspReq      *req;
    cchar       *minified;
    char        **up;

    if (uri) {
        //  MOB uri = httpLink(uri);
        espWrite(conn, "    <script src='%s' type='text/javascript'></script>\r\n", uri);

    } else {
        req = conn->data;
        minified = get(options, "minified", "1");
        for (up = defaultScripts; *up; up++) {
            uri = sjoin("/", mprGetPathBase(req->eroute->staticDir), *up, (*minified == '1') ? ".min.js" : ".js", 0);
            espWrite(conn, "    <link rel='stylesheet' type='text/css' href='%s' />\r\n", uri);
        }
    }
}


void espSecurityToken(HttpConn *conn) 
{
    HttpRx      *rx;
    cchar       *securityToken;

    HttpConn *c;
    EspReq  *req;

    req = conn->data;
    c = mprGetThreadData(req->esp->local);
    rx = conn->rx;
    securityToken = espGetSecurityToken(conn);
    espWrite(conn, "    <meta name='SecurityTokenName' content='%s' />\r\n", ESP_SECURITY_TOKEN_NAME);
    espWrite(conn, "    <meta name='%s' content='%s' />\r\n", ESP_SECURITY_TOKEN_NAME, securityToken);
}


void espStylesheet(HttpConn *conn, cchar *uri, cchar *options) 
{
    EspReq      *req;
    char        **up;

    if (uri) {
        espWrite(conn, "    <link rel='stylesheet' type='text/css' href='%s' />\r\n", uri);
    } else {
        req = conn->data;
        for (up = defaultStylesheets; *up; up++) {
            //  MOB - what about scriptName (same in Ejs)
            uri = sjoin("/", mprGetPathBase(req->eroute->staticDir), *up, 0);
            espWrite(conn, "    <link rel='stylesheet' type='text/css' href='%s' />\r\n", uri);
        }
    }
}


void espTable(HttpConn *conn) {}
void espTabs(HttpConn *conn) {}
void espText(HttpConn *conn) {}
void espTree(HttpConn *conn) {}

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
