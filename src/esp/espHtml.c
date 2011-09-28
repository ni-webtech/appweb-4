/*
    espHtml.c -- HTML controls 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"
#include    "edi.h"

#if BLD_FEATURE_ESP
/************************************* Local **********************************/

#if TODO
    MOB - review and document somewhere
    -http-method-
    -esp- prefixes all internal styles
    -esp-alert
    -esp-flash
    -esp-flash-inform
    -esp-flash-warn
    -esp-flash-error
    -esp-hidden
    -esp-tabs
    -esp-field-error
    -esp-progress
    -esp-progress-inner
    -esp-table
    -esp-table-download
    -esp-even-row
    -esp-field-error
    -esp-form-error
#endif

static char *defaultScripts[] = {
    "/js/jquery", 
    "/js/jquery.tablesorter",
    "/js/jquery.simplemodal",
    "/js/jquery.treeview",              //  MOB - treeview not yet supported
    "/js/jquery.esp",
    0,
};

static char *defaultStylesheets[] = {
    "/layout.css", 
    "/themes/default.css", 
    "/js/treeview.css", 
    0,
};


//  MOB - add comments for each

static char *internalOptions[] = {
    "action",
    "cell",
    "click",
    "columns",
    "controller",
    "escape",
    "field",
    "formatter",
    "hidden",
    "hideErrors",
    "key",
    "keyFormat",
    "kind",
    "minified",
    "nosecurity",
    "params",
    "pass",
    "password",
    "pivot",
    "retain",
    "securityToken",
    "showHeader",
    "showId",
    "styleCells",
    "styleColumns",
    "styleRows",
    "title",
    "toggle",
    "value",
    0
};

#if MOB
    what about these
        params          - Params to add to click
        query           - To add to click - MOB - why is this needed
#endif

#if DOC_ONLY
static char *htmlOptions[] = {
    "background",
    "class",
    "color",
    "cols",
    "rows",
    "id",
    "height",
    "rel",
    "size",
    "width",
    0
};

static char *dataOptions[] = {
    "data-apply",
    "data-confirm",
    "data-edit",
    "data-effects",
    "data-method",
    "data-modal",
    "data-period",
    "data-pivot",
    "data-refresh",
    "data-remote",
    "data-sort",
    "data-sortOrder",
    0
};
#endif

static void emitFormErrors(HttpConn *conn, EdiRec *record, MprHash *options);
static cchar *escapeValue(cchar *value, MprHash *options);
static cchar *formatValue(EdiField value, MprHash *options);
static cchar *getValue(HttpConn *conn, cchar *field, MprHash *options);
static cchar *map(HttpConn *conn, MprHash *options);
static void textInner(HttpConn *conn, cchar *field, MprHash *options);

/************************************* Code ***********************************/

void espAlert(HttpConn *conn, cchar *text, cchar *optionString)
{
    MprHash     *options;
   
    options = httpGetOptions(optionString);
    httpAddOption(options, "class", "-esp-alert");
    text = escapeValue(text, options);
    espWrite(conn, "<div%s>%s</div>", map(conn, options), text);
}


void espAnchor(HttpConn *conn, cchar *text, cchar *uri, cchar *optionString) 
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    text = escapeValue(text, options);
    espWrite(conn, "<a href='%s'%s>%s</a>", uri, map(conn, options), text);
}


void espButton(HttpConn *conn, cchar *name, cchar *value, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espWrite(conn, "<input name='%s' type='submit' value='%s'%s />", name, value, map(conn, options));
}


void espButtonLink(HttpConn *conn, cchar *text, cchar *uri, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    httpSetOption(options, "data-click", httpLink(conn, uri, NULL));
    espWrite(conn, "<button%s>%s</button>", map(conn, options), text);
}


void espChart(HttpConn *conn) 
{
    //  TODO
}


/*
    checkbox field 
    checkedValue Value for which the checkbox will be checked. Defaults to true.
    Example:
        checkbox("admin", "true")

 */
void espCheckbox(HttpConn *conn, cchar *field, cchar *checkedValue, cchar *optionString) 
{
    MprHash     *options;
    cchar           *value, *checked;
   
    options = httpGetOptions(optionString);
    value = getValue(conn, field, options);
    checked = scasematch(value, checkedValue) ? " checked='yes'" : "";
    espWrite(conn, "<input name='%s' type='checkbox'%s%s value='%s' />\r\n", 
        field, map(conn, options), checked, checkedValue);
    espWrite(conn, "    <input name='%s' type='hidden'%s value='' />", field, map(conn, options));
}


void espDivision(HttpConn *conn, cchar *body, cchar *optionString) 
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espWrite(conn, "<div%s>%s</div>", map(conn, options), body);
}


void espEndform(HttpConn *conn) 
{
    espWriteString(conn, "</form>");
}


void espFlash(HttpConn *conn, cchar *kinds, cchar *optionString)
{
    EspReq      *req;
    MprHash     *options;
    MprKey      *kp;
    cchar       *msg;
   
    //  MOB -- need APIs to get messages into flash
    req = conn->data;
    options = httpGetOptions(optionString);
    if (kinds == 0 || req->flash == 0 || mprGetHashLength(req->flash) == 0) {
        return;
    }
    for (kp = 0; (kp = mprGetNextKey(req->flash, kp)) != 0; ) {
        msg = kp->data;
        if (strstr(kinds, kp->key) || strstr(kinds, "all")) {
            httpAddOption(options, "class", sfmt("-esp-flash -esp-flash-%s", kp->key));
            espWrite(conn, "<div%s>%s</div>", map(conn, options), msg);
        }
    }
}


void espForm(HttpConn *conn, EdiRec *record, cchar *optionString)
{
    EspReq      *req;
    MprHash     *options;
    cchar       *action, *recid, *method, *uri, *token;
   
    req = conn->data;
    options = httpGetOptions(optionString);
    recid = 0;

    /*
        If record provided, the record id. Can be overridden using options.recid
     */
    if (record) {
        req->record = record;
        if (record->id && !httpGetOption(options, "recid", 0)) {
            httpAddOption(options, "recid", record->id);
        }
        recid = httpGetOption(options, "recid", 0);
        emitFormErrors(conn, record, options);
    }
    if ((method = httpGetOption(options, "method", 0)) == 0) {
        method = (recid) ? "PUT" : "POST";
    }
    if (!scasematch(method, "GET") && !scasematch(method, "POST")) {
        /* All methods use POST and tunnel method in data-method */
        httpAddOption(options, "data-method", method);
        method = "POST";
    }
    if ((action = httpGetOption(options, "action", 0)) == 0) {
        action = (recid) ? "@update" : "@create";
    }
    uri = httpLink(conn, action, NULL);
    espWrite(conn, "<form method='%s'  action='%s'%s >\r\n", method, uri, map(conn, options));

    if (recid) {
        espWrite(conn, "    <input name='recid' type='hidden' value='%s' />\r\n", recid);
    }
    if (!httpGetOption(options, "insecure", 0)) {
        if ((token = httpGetOption(options, "securityToken", 0)) == 0) {
            token = conn->rx->securityToken;
        }
        espWrite(conn, "    <input name='%s' type='hidden' value='%s' />\r\n", ESP_SECURITY_TOKEN_NAME, token);
    }
}


void espIcon(HttpConn *conn, cchar *uri, cchar *optionString)
{
    MprHash     *options;
    EspReq      *req;

    options = httpGetOptions(optionString);
    if (uri == 0) {
        req = conn->data;
        uri = sjoin("~/", mprGetPathBase(req->eroute->staticDir), "/images/favicon.ico", NULL);
    }
    espWrite(conn, "<link href='%s' rel='shortcut icon'%s />", httpLink(conn, uri, NULL), map(conn, options));
}


void espImage(HttpConn *conn, cchar *src, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espWrite(conn, "<img src='%s'%s />", src, map(conn, options));
}


void espInput(HttpConn *conn, cchar *fieldName, cchar *optionString)
{
    MprHash     *options;
    EspReq      *req;
    EdiRec      *rec;
    int         type;

    req = conn->data;
    rec = req->record;
    type = ediGetRecFieldType(rec, fieldName);

    switch (type) {
    case EDI_TYPE_BLOB:
    case EDI_TYPE_BOOL:
    case EDI_TYPE_DATE:
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
    case EDI_TYPE_STRING:
        espText(conn, fieldName, optionString);
        break;
    case EDI_TYPE_TEXT:
        options = httpGetOptions(optionString);
        if (!httpGetOption(options, "rows", 0)) {
            httpSetOption(options, "rows", "10");
        }
        textInner(conn, fieldName, options);
        break;
    default:
        mprError("Unknown field type %d", type);
        espText(conn, fieldName, optionString);
        break;
    }
}


void espLabel(HttpConn *conn, cchar *text, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espWrite(conn, "<span%s>%s</span>", map(conn, options), text);
}


/*
    list("priority", "low='0' med='1' high='2'", NULL)
    list("priority", "low='0' med='1' high='2'", "value='2'")   //  MOB - without a record
 */
void espList(HttpConn *conn, cchar *field, cchar *choices, cchar *optionString) 
{
    MprHash     *options;
    MprKey      *kp;
    cchar       *value, *selected;

    options = httpGetOptions(optionString);
    value = getValue(conn, field, options);
    espWrite(conn, "<select name='%s'%s>\r\n", field, map(conn, options));
    for (kp = 0; (kp = mprGetNextKey(options, kp)) != 0; ) {
        selected = (smatch(kp->data, value)) ? " selected='yes'" : "";
        espWrite(conn, "        <option value='%s'%s>%s</option>\r\n", kp->key, selected, kp->data);
    }
    espWrite(conn, "    </select>");
}


void espMail(HttpConn *conn, cchar *name, cchar *address, cchar *optionString) 
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espWrite(conn, "<a href='mailto:%s'%s>%s</a>", address, map(conn, options), name);
}


void espProgress(HttpConn *conn, cchar *percent, cchar *optionString)
{
    MprHash     *options;
   
    options = httpGetOptions(optionString);
    httpAddOption(options, "data-progress", percent);
    espWrite(conn, "<div class='-esp-progress'>\r\n");
    espWrite(conn, "    <div class='-esp-progress-inner'%s>%s %%</div>\r\n</div>", map(conn, options), percent);
}


/*
    radio("priority", "low='0' med='1' high='2'", NULL)
    radio("priority", "low='0' med='1' high='2'", "value='2'")  //  MOB - without a record
 */
void espRadio(HttpConn *conn, cchar *field, void *choices, cchar *optionString)
{
    MprKey      *kp;
    MprHash     *options;
    cchar       *value, *checked;

    options = httpGetOptions(optionString);
    value = getValue(conn, field, options);
    for (kp = 0; (kp = mprGetNextKey(options, kp)) != 0; ) {
        checked = (smatch(kp->data, value)) ? " checked" : "";
        espWrite(conn, "        %s <input type='radio' name='%s' value='%s'%s%s />\r\n", 
            spascal(kp->key), kp->key, kp->data, checked, map(conn, options));
    }
}


void espRefresh(HttpConn *conn, cchar *on, cchar *off, cchar *optionString)
{
    MprHash     *options;
   
    options = httpGetOptions(optionString);
    espWrite(conn, "<img src='%s' data-on='%s' data-off='%s%s' class='-esp-refresh' />", on, on, off, map(conn, options));
}


void espScript(HttpConn *conn, cchar *uri, cchar *optionString)
{
    EspReq      *req;
    cchar       *minified, *indent, *newline;
    char        **up;
    MprHash     *options;
   
    options = httpGetOptions(optionString);


    if (uri) {
        espWrite(conn, "<script src='%s' type='text/javascript'></script>", httpLink(conn, uri, NULL));
    } else {
        req = conn->data;
        minified = httpGetOption(options, "minified", 0);
        indent = "";
        for (up = defaultScripts; *up; up++) {
            uri = httpLink(conn, sjoin("~/", mprGetPathBase(req->eroute->staticDir), *up, 
                minified ? ".min.js" : ".js", NULL), NULL);
            newline = up[1] ? "\r\n" :  "";
            espWrite(conn, "%s<script src='%s' type='text/javascript'></script>%s", indent, uri, newline);
            indent = "    ";
        }
    }
}


/*
    Generate a security token
 */
void espSecurityToken(HttpConn *conn) 
{
    HttpRx      *rx;
    cchar       *securityToken;

    rx = conn->rx;
    securityToken = espGetSecurityToken(conn);
    espWrite(conn, "<meta name='SecurityTokenName' content='%s' />\r\n", ESP_SECURITY_TOKEN_NAME);
    espWrite(conn, "    <meta name='%s' content='%s' />", ESP_SECURITY_TOKEN_NAME, securityToken);
}


void espStylesheet(HttpConn *conn, cchar *uri, cchar *optionString) 
{
    EspReq      *req;
    cchar       *indent, *newline;
    char        **up;
   
    if (uri) {
        espWrite(conn, "<link rel='stylesheet' type='text/css' href='%s' />", httpLink(conn, uri, NULL));
    } else {
        req = conn->data;
        indent = "";
        for (up = defaultStylesheets; *up; up++) {
            uri = httpLink(conn, sjoin("~/", mprGetPathBase(req->eroute->staticDir), *up, NULL), NULL);
            newline = up[1] ? "\r\n" :  "";
            espWrite(conn, "%s<link rel='stylesheet' type='text/css' href='%s' />%s", indent, uri, newline);
            indent = "    ";
        }
    }
}


static void pivot(EdiGrid *grid)
{
    //  TODO
    return;
}


static void filterCols(MprList *cols, MprHash *options, MprHash *colOptions)
{
    cchar       *columnName;
    int         next;

    if (colOptions) {
        for (next = 0; (columnName = mprGetNextItem(cols, &next)) != 0; ) {
            if (!mprLookupKey(options, columnName)) {
                mprRemoveItem(cols, columnName);
            }
        }
    }
    if (mprLookupKey(options, "showId") == 0) {
        mprRemoveItemAtPos(cols, 0);
    }
}


/*
    MOB - revise this doc

    table(grid, "{ refresh:'@update', period:'1000', pivot:'true' }");
    table(grid, "{ click:'@edit' }");
    table(grid, "{ click:'@edit', sort:'name' }");
    table(grid, "columns: { \
        product: { header: 'Product', width: '20%' }, \
        date:    { format: '%m-%d-%y' }, \
    }");

    //  MOB - review edGetTable
    table(edGetTable(ed, "Users"));

    //  MOB - review arg for makeTable
    table(makeTable("[[ 'name', 'age' ], [ 'peter', 23 ], [ 'mary', 22 ] ]")
    Per-column data:
        title/header
        format string
        width
        align

        columns=''

IF JSON
    table(grid, "columns: { \
        product: { header: 'Product', width: '25%' } \
        date:    { header: 'Product', width: '25%' } \
    }");
    table(makeTable("{ \
        schema: [ \
            field: 'name', type: 'string' }, \
            field: 'name', type: 'string' }, \
            field: 'name', type: 'string' }, \
        ], \
        data: [[ 'name', 'age' ], [ 'peter', 23 ], [ 'mary', 22 ]] \
    ")

    table(makeTable("\
        data: [[ 'name', 'age' ], [ 'peter', 23 ], [ 'mary', 22 ]] \
    ")
 */
void espTable(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    MprHash     *options, *colOptions, *rowOptions;
    MprList     *cols;
    EdiRec      *rec;
    cchar       *title, *width, *o, *header, *columnName, *value, *showHeader;
    int         next, c, r, ncols, type;
   
    options = httpGetOptions(optionString);
    if (httpGetOption(options, "pivot", 0)) {
        pivot(grid);
    }
    if (grid->nrecords == 0) {
        espWrite(conn, "<p>No Data</p>\r\n");
        return;
    }
    httpAddOption(options, "class", "-esp-table");

//  MOB - somehow suppress data-click for this
    espWrite(conn, "<table%s>\r\n", map(conn, options));

    cols = ediGetGridColumns(grid); 

    //  MOB - need key bits
    colOptions = httpGetOptionHash(options, "columns");
    filterCols(cols, options, colOptions);

    if ((showHeader = httpGetOption(options, "showHeader", 0)) == 0 || smatch(showHeader, "true")) {
        /*
            Emit header
         */
        espWrite(conn, "    <thead>\r\n");
        if ((title = httpGetOption(options, "title", 0)) != 0) {
            espWrite(conn, "        <tr class='-esp-table-title'><td colspan='%s'>%s</td></tr>\r\n", 
                mprGetListLength(cols), title);
        }
        espWrite(conn, "        <tr class='-esp-table-header'>\r\n");
        for (next = 0; (columnName = mprGetNextItem(cols, &next)) != 0; ) {
            width = ((o = httpGetOption(options, "width", 0)) != 0) ? sfmt(" width='%s'", o) : "";
            header = 0;
            if (colOptions) {
                header = httpGetOption(colOptions, "header", 0);
            }
            if (header == 0) {
                header = spascal(columnName);
            }
            espWrite(conn, "            <th%s>%s</th>\r\n", width, header);
        }
        espWrite(conn, "        </tr>\r\n    </thead>\r\n");
    }
    espWrite(conn, "    <tbody>\r\n");

    /*
        Emit rows
     */

    //  MOB - review this list
    rowOptions = mprCreateHash(0, 0);
    httpSetOption(rowOptions, "data-click", httpGetOption(options, "data-click", 0));
/*
    httpSetOption(rowOptions, httpGetOption(options, "edit", 0));
    httpSetOption(rowOptions, httpGetOption(options, "key", 0));
    httpSetOption(rowOptions, httpGetOption(options, "params", 0));
    httpSetOption(rowOptions, httpGetOption(options, "remote", 0));
*/
    //  MOB - ejs also sets values, field and record

    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        httpSetOption(rowOptions, "id", rec->id);
        espWrite(conn, "        <tr%s>\r\n", map(conn, rowOptions));
        ncols = mprGetListLength(cols);
        for (c = 0; c < ncols; c++) {
            columnName = mprGetItem(cols, c);
            //  MOB - must find right column in record
            type = rec->fields[c].type;
            if (httpGetOption(colOptions, "align", 0) == 0) {
                if (type == EDI_TYPE_INT || type == EDI_TYPE_FLOAT) {
                    httpAddOption(colOptions, "align", "right");
                }
            }
            value = formatValue(rec->fields[c], colOptions);
            espWrite(conn, "            <td%s>%s</td>\r\n", map(conn, colOptions), value);
        }
    }
    espWrite(conn, "        </tr>\r\n");
    espWrite(conn, "    </tbody>\r\n</table>\r\n");
}


void espTabs(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    //MOB todo
}


static void textInner(HttpConn *conn, cchar *field, MprHash *options)
{
    cchar   *rows, *cols, *type, *value;

    type = "text";
    value = getValue(conn, field, options);
    if ((rows = httpGetOption(options, "rows", 0)) != 0) {
        cols = httpGetOption(options, "cols", "60");
        if (httpGetOption(options, "password", 0)) {
            type = "password";
        } else if (httpGetOption(options, "hidden", 0)) {
            type = "hidden";
        }
        espWrite(conn, "<textarea name='%s' type='%s' cols='%s' rows='%s'%s>%s</textarea>", field, type, 
            cols, rows, map(conn, options), value);
    } else {
          espWrite(conn, "<input name='%s' type='%s' value='%s' />", field, type, value);
    }
}


void espText(HttpConn *conn, cchar *field, cchar *optionString)
{
    return textInner(conn, field, httpGetOptions(optionString));
}


void espTree(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    //  MOB
}

//  MOB - render a partial view

/************************************ Abbreviated Controls ****************************/ 

HttpConn *espGetConn()
{
    return mprGetThreadData(((Esp*) MPR->espService)->local);
}


void alert(cchar *text, cchar *optionString)
{
    espAlert(espGetConn(), text, optionString);
}


void anchor(cchar *text, cchar *uri, cchar *optionString) 
{
    espAnchor(espGetConn(), text, uri, optionString);
}


void button(cchar *name, cchar *value, cchar *optionString)
{
    espButton(espGetConn(), name, value, optionString);
}


void buttonLink(cchar *text, cchar *uri, cchar *optionString)
{
    espButtonLink(espGetConn(), text, uri, optionString);
}


void chart(HttpConn *conn) 
{
    espChart(espGetConn());
}


void checkbox(cchar *field, cchar *checkedValue, cchar *optionString) 
{
    espCheckbox(espGetConn(), field, checkedValue, optionString);
}


void division(cchar *body, cchar *optionString) 
{
    espDivision(espGetConn(), body, optionString);
}


void endform() 
{
    espEndform(espGetConn());
}


void flash(cchar *kind, cchar *optionString)
{
    espFlash(espGetConn(), kind, optionString);
}


void form(void *record, cchar *optionString)
{
    HttpConn    *conn;

    conn = espGetConn();
    if (record == 0) {
        record = conn->record;
    }
    espForm(conn, record, optionString); 
}


void icon(cchar *uri, cchar *optionString)
{
    espIcon(espGetConn(), uri, optionString);
}


void image(cchar *src, cchar *optionString)
{
    espImage(espGetConn(), src, optionString);
}


void input(cchar *name, cchar *optionString)
{
    espInput(espGetConn(), name, optionString);
}


void label(cchar *text, cchar *optionString)
{
    espLabel(espGetConn(), text, optionString);
}


void list(cchar *name, cchar *choices, cchar *optionString) 
{
    espList(espGetConn(), name, choices, optionString);
}


void mail(HttpConn *conn, cchar *name, cchar *address, cchar *optionString) 
{
    espMail(espGetConn(), name, address, optionString);
}


void progress(cchar *data, cchar *optionString)
{
    espProgress(espGetConn(), data, optionString);
}


void radio(cchar *name, void *choices, cchar *optionString)
{
    espRadio(espGetConn(), name, choices, optionString);
}


void refresh(cchar *on, cchar *off, cchar *optionString)
{
    espRefresh(espGetConn(), on, off, optionString);
}


void script(cchar *uri, cchar *optionString)
{
    espScript(espGetConn(), uri, optionString);
}


void securityToken()
{
    espSecurityToken(espGetConn());
}


void stylesheet(cchar *uri, cchar *optionString) 
{
    espStylesheet(espGetConn(), uri, optionString);
}


void table(EdiGrid *grid, cchar *optionString)
{
    espTable(espGetConn(), grid, optionString);
}


void tabs(EdiGrid *grid, cchar *optionString)
{
    espTabs(espGetConn(), grid, optionString);
}


void text(cchar *field, cchar *optionString)
{
    espText(espGetConn(), field, optionString);
}


void tree(EdiGrid *grid, cchar *optionString)
{
    espTree(espGetConn(), grid, optionString);
}


void espInitHtmlOptions(Esp *esp)
{
    char   **op;

    for (op = internalOptions; *op; op++) {
        mprAddKey(esp->internalOptions, *op, op);
    }
}


/*
    Map options to an attribute string. Remove all internal control specific options and transparently handle 
    URI link options.

    WARNING: this returns a non-cloned reference and relies on no GC yield until the returned value is used or cloned.
    This is done as an optimization to reduce memeory allocations.
 */
static cchar *map(HttpConn *conn, MprHash *options)
{
    Esp         *esp;
    EspReq      *req;
    MprHash     *params;
    MprKey      *kp;
    MprBuf      *buf;
    cchar       *value;
    char        *pstr;

    if (options == 0 || mprGetHashLength(options) == 0) {
        return MPR->emptyString;
    }
    req = conn->data;
    if (httpGetOption(options, "data-refresh", 0) && httpGetOption(options, "id", 0)) {
        httpAddOption(options, "id", sfmt("id_%s", req->lastDomID++));
    }
    esp = MPR->espService;
    buf = mprCreateBuf(-1, -1);
    for (kp = 0; (kp = mprGetNextKey(options, kp)) != 0; ) {
        if (!mprLookupKey(esp->internalOptions, kp->key)) {
            mprPutCharToBuf(buf, ' ');
            value = kp->data;
            /*
                Support link template resolution for these options
             */
            if (smatch(kp->key, "data-click") || smatch(kp->key, "data-remote") || smatch(kp->key, "data-refresh")) {
//@edit
                value = httpLink(conn, value, options);
                if ((params = httpGetOptionHash(options, "params")) != 0) {
                    pstr = (char*) "";
                    for (kp = 0; (kp = mprGetNextKey(params, kp)) != 0; ) {
                        pstr = sjoin(pstr, mprUriEncode(kp->key, MPR_ENCODE_URI_COMPONENT), "=", 
                            mprUriEncode(kp->data, MPR_ENCODE_URI_COMPONENT), "&", NULL);
                    }
                    if (pstr[0]) {
                        /* Trim last "&" */
                        pstr[strlen(pstr) - 1] = '\0';
                    }
                    mprPutFmtToBuf(buf, "%s-params='%s", params);
                }
            }
            mprPutStringToBuf(buf, kp->key);
            mprPutStringToBuf(buf, "='");
            mprPutStringToBuf(buf, value);
            mprPutCharToBuf(buf, '\'');
        }
    }
    mprAddNullToBuf(buf);
    return mprGetBufStart(buf);
}


#if 0
static cchar *xx(HttpConn *conn, MprHash *options)
{
    if (httpGetOption(options, "hasError", 0)) {
    }
}
#endif


static void emitFormErrors(HttpConn *conn, EdiRec *rec, MprHash *options)
{
    MprList         *errors;
    MprKeyValue     *error;
    int             count, next;
   
    if (!rec->errors || !httpGetOption(options, "hideErrors", 0)) {
        return;
    }
    errors = ediGetErrors(rec);
    if (errors) {
        count = mprGetListLength(errors);
        espWrite(conn, "<div class='-esp-form-error'><h2>The %s has %s it being saved.</h2>\r\n",
            count, count <= 1 ? "error that prevents" : "errors that prevent");
        espWrite(conn, "    <p>There were problems with the following fields:</p>\r\n");
        espWrite(conn, "    <ul>\r\n");
        for (next = 0; (error = mprGetNextItem(errors, &next)) != 0; ) {
            espWrite(conn, "        <li>%s %s</li>\r\n", error->key, error->value);
        }
        espWrite(conn, "    </ul>\r\n");
        espWrite(conn, "</div>");
    }
}


static cchar *getValue(HttpConn *conn, cchar *fieldName, MprHash *options)
{
    EspReq      *req;
    EdiRec      *record;
    MprKeyValue *error;
    cchar       *value;
    int         next;

    req = conn->data;
    record = req->record;
    value = 0;

    if (record) {
        value = ediGetRecFieldAsString(record, fieldName, NULL);
        if (record->errors) {
            for (next = 0; (error = mprGetNextItem(record->errors, &next)) != 0; ) {
                if (smatch(error->key, fieldName)) {
                    httpAddOption(options, "class", "-esp-field-error");
                }
            }
        }
    }
    if (value == 0) {
        value = httpGetOption(options, "value", 0);
    }
    if (httpGetOption(options, "escape", 0)) {
        value = mprEscapeHtml(value);
    }
    return value;
}


static cchar *formatValue(EdiField value, MprHash *options)
{
    return ediToString(httpGetOption(options, "format", 0), value);
}


static cchar *escapeValue(cchar *value, MprHash *options)
{
    if (httpGetOption(options, "escape", 0)) {
        return mprEscapeHtml(value);
    }
    return value;
}


//  MOB - not just database
/************************************ Database *******************************/
//  MOB - need ESP equivalents
EdiRec *findRec(cchar *tableName, cchar *key)
{
    return ediGetRec(getEdi(), tableName, key);
}


EdiGrid *findAll(cchar *tableName)
{
    return ediGetAll(getEdi(), tableName);
}


HttpConn *getConn()
{
    return espGetConn();
}


static EdiField initField(cchar *name, EdiValue value, int type, int flags)
{
    EdiField    f;

    f.valid = 1;
    f.value = value;
    f.type = type;
    f.name = name;
    f.flags = flags;
    return f;
}


EdiRec *createRec(cchar *tableName, MprHash *params)
{
    Edi         *edi;
    EdiRec      *rec;
    MprList     *cols;
    MprKey      *kp;
    int         f, ncols, type, flags;

    edi = getEdi();
    cols = ediGetColumns(edi, tableName);
    ncols = mprGetListLength(cols);
    rec = ediCreateRec(edi, tableName, 0, ncols, NULL);
    for (f = 0, ITERATE_KEYS(params, kp)) {
        //  MOB - must check table exists
        if (ediGetSchema(edi, tableName, kp->key, &type, &flags) == 0 && type > 0) {
            rec->fields[f++] = initField(kp->key, ediParseValue(kp->data, type), type, flags);
        }
    }
    return rec;
}


//  MOB - review name
void createSession()
{
    espGetSession(espGetConn(), 1);
}


void error(cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(espGetConn(), "error", fmt, args);
    va_end(args);
}


void finalize()
{
    espFinalize(espGetConn());
}


Edi *getEdi()
{
    EspRoute    *eroute;

    eroute = getEroute();
    return eroute->edi;
}


EspRoute *getEroute()
{
    HttpConn    *conn;
    EspReq      *req;

    conn = espGetConn();
    req = conn->data;
    return req->eroute;
}


void inform(cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(espGetConn(), "inform", fmt, args);
    va_end(args);
}


void notice(cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(espGetConn(), kind, fmt, args);
    va_end(args);
}


cchar *getMethod()
{
    return espGetConn()->rx->method;
}


MprHash *params()
{
    return espGetParams(espGetConn());
}


cchar *param(cchar *key, cchar *defaultValue)
{
    return espGetParam(espGetConn(), key, defaultValue);
}


void redirect(cchar *target)
{
    espRedirect(espGetConn(), 302, target);
}


//  MOB - name
EdiRec *getRec()
{
    EdiRec  *rec;

    rec = espGetConn()->record;
    return rec;
}


//  MOB - name
bool testRec()
{
    EdiRec  *rec;

    rec = espGetConn()->record;
    return (rec && rec->id) ? 1 : 0;
}


bool removeRec(cchar *tableName, cchar *key)
{
    if (ediDeleteRow(getEdi(), tableName, key) < 0) {
        return 0;
    }
    return 1;
}

bool save()
{
    return saveUpdate(espGetParams(getConn()));
}


bool saveUpdate(MprHash *fields)
{
    HttpConn    *conn;
    Edi         *edi;
    EdiRec      *rec;

    conn = espGetConn();
    edi = getEdi();
    if ((rec = getRec()) == 0) {
        return 0;
    }
    if (ediSetRec(edi, rec->tableName , rec->id, fields) < 0) {
        return 0;
    } 
    return 1;
}


cchar *session(cchar *key)
{
    return espGetSessionVar(espGetConn(), key, 0);
}


void setParam(cchar *key, cchar *value)
{
    espSetParam(espGetConn(), key, value);
}


void setSession(cchar *key, cchar *value)
{
    espSetSessionVar(espGetConn(), key, value);
}


void record(EdiRec *rec)
{
    espGetConn()->record = rec;
}


ssize render(cchar *fmt, ...)
{
    va_list     args;
    ssize       count;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    count = espWriteString(espGetConn(), msg);
    va_end(args);
    return count;
}


void warn(cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espNoticev(espGetConn(), "warn", fmt, args);
    va_end(args);
}


void writeView(cchar *view)
{
    espWriteView(espGetConn(), view);
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
