/*
    espHtml.c -- HTML controls 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"
#include    "edi.h"

#if BIT_FEATURE_ESP
/************************************* Local **********************************/

static char *defaultScripts[] = {
    "/js/jquery", 
    "/js/jquery.tablesorter",
    "/js/jquery.simplemodal",
    "/js/jquery.esp",
    0,
};

static char *defaultStylesheets[] = {
    "/layout.css", 
    "/themes/default.css", 
    0,
};

#if MOB
    Actions
        - P
        - Have internal mappings san-data to add "data-"
        - Define all options and document
        - Rename "app" into "mapp" and have test pages
        - Create unit tests

    Principles
        - All unknown options are passed through to the client
        - Internal options are filtered or mapped to data-*
        - Remote only takes a "true" value
    - Support flash as modal, transparent box. See "sales" app.
    - Support "feedback" transparent overlay

    Kinds of click requests:
        - Click   (remote: true|false)  data-click-*
        - Refresh                       data-refresh-*
        - Both take
            URI:        data-click
            Method:     data-click-method
            Params:     Data to pass to Uri construction. Or is this just general options.
                            (Should not pass all options as names could clash:
                                 scheme, host, port, path, ref, query
                                 route, action, controller, uri, template,
            KeyFormat   How to apply keys: "params" | "path" | "query"
            keys        Set of keys to pass with the request

    - Action/Controller are they needed?
        => But they must be needed for click|refresh
#endif

/*
    MOB - must document styles and themes with "esp" prefix
    MOB - document the various click, refresh, remote events
    MOB - How is data for click, remote, refresh etc handled
    MOB - which of these should be data-*
    MOB - could make these pairs and map from one to the other
*/
static char *internalOptions[] = {
    "action",                       /* Controller/Action to invoke */
    "cell",                         /* table(): If set, table clicks apply to the cell and not to the row */

//  MOB data-click
    "click",                        /* general: URI to invoke if the control is clicked */

    "columns",                      /* table(): Column options */
    "controller",                   /* general: Controller to use for click events */
    "escape",                       /* general: Html escape the data */
    "feedback",                     /* general: User feedback overlay after a clickable event */
//  MOB
"field",

//  MOB - not implemented. Change to format
    "formatter",                    /* general: Printf style format string to apply to data */

    "hidden",                       /* text(): Field should be hidden */
    "hideErrors",                   /* form(): Hide record validation errors */
    "insecure",                     /* form(): Don't generate a CSRF security token */

//  MOB - rename keys
    "key",                          /* general: key property/value data to provide for clickable events */
//  MOB - not implemented yet
    "keyFormat",                    /* General: How keys are handled for click events: "params" | "path" | "query" */
"kind",
    "minified",                     /* script(): Use minified script variants */
"params",                           /* general: Parms to pass on click events */
"pass",
    "password",                     /* text(): Text input is a password */
    "pivot",                        /* table(): Pivot the table data */
    "remote",                       /* general: Set to true to make click event operate in the background */
"retain",
    "securityToken",                /* form(): Name of security token to use */
    "showHeader",                   /* table(): Show table column names header  */
    "showId",                       /* table(): Show the ID column */

    //  MOB - not implemented yet
    "sort",                         /* table(): Column to sort rows by */
    "sortOrder",                    /* table(): Row sort order */
    "styleCells",                   /* table(): Styles to use for table cells */
    "styleColumns",                 /* table(): Styles to use for table columns */
    "styleRows",                    /* table(): Styles to use for table rows */
    "title",                        /* table(): Table title to display */
//  MOB - is data-toggle
    "toggle",                       /* tabs(): Toggle tabbed panes */
    "value",                        /* general: Value to use instead of record-bound data */
    0
};

#if DOC_ONLY
//  MOB - what are these
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

//  MOB - what are thse
//  MOB -check against jquery.ejs.js
static char *dataOptions[] = {
    "data-apply",
    "data-click",
    "data-click-method",
    "data-click-params",
    "data-confirm",
//  MOB - what is this "edit"?
    "data-edit",
//  MOB - jquery uses esp-pptions
    "data-effects",
    "data-method",
    "data-modal",
    "data-period",
    "data-pivot",
    "data-refresh",
    "data-remote",
    "data-remote-method",
    "data-sort",
    "data-sort-order",
    0
};
#endif

static void emitFormErrors(HttpConn *conn, EdiRec *record, MprHash *options);
static cchar *escapeValue(cchar *value, MprHash *options);
static cchar *formatValue(EdiField *fp, MprHash *options);
static cchar *getValue(HttpConn *conn, cchar *field, MprHash *options);
static cchar *map(HttpConn *conn, MprHash *options);
static void textInner(HttpConn *conn, cchar *field, MprHash *options);

/************************************* Code ***********************************/

//  MOB - what is this really doing?
void espAlert(HttpConn *conn, cchar *text, cchar *optionString)
{
    MprHash     *options;
   
    options = httpGetOptions(optionString);
    httpAddOption(options, "class", "-esp-alert");
    text = escapeValue(text, options);
    espRender(conn, "<div%s>%s</div>", map(conn, options), text);
}


void espAnchor(HttpConn *conn, cchar *text, cchar *uri, cchar *optionString) 
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    text = escapeValue(text, options);
    espRender(conn, "<a href='%s'%s>%s</a>", uri, map(conn, options), text);
}


void espButton(HttpConn *conn, cchar *name, cchar *value, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espRender(conn, "<input name='%s' type='submit' value='%s'%s />", name, value, map(conn, options));
}


void espButtonLink(HttpConn *conn, cchar *text, cchar *uri, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    httpSetOption(options, "data-click", httpLink(conn, uri, NULL));
    espRender(conn, "<button%s>%s</button>", map(conn, options), text);
}


void espChart(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    //  TODO
}


/*
    checkbox field 

    checkedValue -- Value for which the checkbox will be checked. Defaults to true.
    Example:
        checkbox("admin", "true")
 */
void espCheckbox(HttpConn *conn, cchar *field, cchar *checkedValue, cchar *optionString) 
{
    MprHash     *options;
    cchar       *value, *checked;
   
    options = httpGetOptions(optionString);
    value = getValue(conn, field, options);
    checked = scasematch(value, checkedValue) ? " checked='yes'" : "";
    espRender(conn, "<input name='%s' type='checkbox'%s%s value='%s' />\r\n", 
        field, map(conn, options), checked, checkedValue);
    espRender(conn, "    <input name='%s' type='hidden'%s value='' />", field, map(conn, options));
}


void espDivision(HttpConn *conn, cchar *body, cchar *optionString) 
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espRender(conn, "<div%s>%s</div>", map(conn, options), body);
}


void espEndform(HttpConn *conn) 
{
    espRenderString(conn, "</form>");
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
            espRender(conn, "<div%s>%s</div>", map(conn, options), msg);
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
        If record provided, get the record id. Can be overridden using options.recid
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
    espRender(conn, "<form method='%s'  action='%s'%s >\r\n", method, uri, map(conn, options));

    if (recid) {
        espRender(conn, "    <input name='recid' type='hidden' value='%s' />\r\n", recid);
    }
    if (!httpGetOption(options, "insecure", 0)) {
        if ((token = httpGetOption(options, "securityToken", 0)) == 0) {
            token = espGetSecurityToken(conn);
        }
        espRender(conn, "    <input name='%s' type='hidden' value='%s' />\r\n", ESP_SECURITY_TOKEN_NAME, token);
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
    espRender(conn, "<link href='%s' rel='shortcut icon'%s />", httpLink(conn, uri, NULL), map(conn, options));
}


void espImage(HttpConn *conn, cchar *uri, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espRender(conn, "<img src='%s'%s />", httpLink(conn, uri, NULL), map(conn, options));
}


void espInput(HttpConn *conn, cchar *fieldName, cchar *optionString)
{
    MprHash     *options;
    EspReq      *req;
    EdiRec      *rec;
    int         type;

    req = conn->data;
    rec = req->record;
    type = ediGetFieldType(rec, fieldName);

    switch (type) {
    case EDI_TYPE_BINARY:
    case EDI_TYPE_BOOL:
        /* MOB - could do radio */
    case EDI_TYPE_DATE:
        /* MOB - could do calendar control */
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
        /* MOB - could add numeric validation */
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
        httpError(conn, 0, "Unknown field type %d", type);
        espText(conn, fieldName, optionString);
        break;
    }
}


void espLabel(HttpConn *conn, cchar *text, cchar *optionString)
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espRender(conn, "<span%s>%s</span>", map(conn, options), text);
}


/*
    dropdown("priority", makeGrid("[{ id: 0, low: 0}, { id: 1, med: 1}, {id: 2, high: 2}]"), NULL)

    Options can provide the defaultValue in a "value" property.
 */
void espDropdown(HttpConn *conn, cchar *field, EdiGrid *choices, cchar *optionString) 
{
    MprHash     *options;
    cchar       *id, *currentValue, *selected, *value;
    int         r;

    if (choices == 0) {
        return;
    }
    options = httpGetOptions(optionString);
    currentValue = getValue(conn, field, options);
    if (field == 0) {
        field = httpGetOption(options, "field", "id");
    }
    espRender(conn, "<select name='%s'%s>\r\n", field, map(conn, options));
    for (r = 0; r < choices->nrecords; r++) {
        id = ediGetField(choices->records[r], "id");
        value = ediGetField(choices->records[r], field);
        selected = (smatch(value, currentValue)) ? " selected='yes'" : "";
        espRender(conn, "        <option value='%s'%s>%s</option>\r\n", id, selected, value);
    }
    espRender(conn, "    </select>");
}


void espMail(HttpConn *conn, cchar *name, cchar *address, cchar *optionString) 
{
    MprHash     *options;

    options = httpGetOptions(optionString);
    espRender(conn, "<a href='mailto:%s'%s>%s</a>", address, map(conn, options), name);
}


void espProgress(HttpConn *conn, cchar *percent, cchar *optionString)
{
    MprHash     *options;
   
    options = httpGetOptions(optionString);
    httpAddOption(options, "data-progress", percent);
    espRender(conn, "<div class='-esp-progress'>\r\n");
    espRender(conn, "    <div class='-esp-progress-inner'%s>%s %%</div>\r\n</div>", map(conn, options), percent);
}


/*
    radio("priority", "{low: 0, med: 1, high: 2}", NULL)
    radio("priority", "{low: 0, med: 1, high: 2}", "{value:'2'}")  //  MOB - without a record
 */
void espRadio(HttpConn *conn, cchar *field, void *choicesString, cchar *optionsString)
{
    MprKey      *kp;
    MprHash     *choices, *options;
    cchar       *value, *checked;

    choices = httpGetOptions(choicesString);
    options = httpGetOptions(optionsString);
    value = getValue(conn, field, options);
    for (kp = 0; (kp = mprGetNextKey(choices, kp)) != 0; ) {
        checked = (smatch(kp->data, value)) ? " checked" : "";
        espRender(conn, "        %s <input type='radio' name='%s' value='%s'%s%s />\r\n", 
            spascal(kp->key), field, kp->data, checked, map(conn, options));
    }
}


/*
    Control the refresh of dynamic elements in the page
 */
void espRefresh(HttpConn *conn, cchar *on, cchar *off, cchar *optionString)
{
    MprHash     *options;
   
    options = httpGetOptions(optionString);
    espRender(conn, "<img src='%s' data-on='%s' data-off='%s%s' class='-esp-refresh' />", on, on, off, map(conn, options));
}


void espScript(HttpConn *conn, cchar *uri, cchar *optionString)
{
    EspReq      *req;
    cchar       *indent, *newline;
    char        **up;
    MprHash     *options;
    bool        minified;
   
    options = httpGetOptions(optionString);
    if (uri) {
        espRender(conn, "<script src='%s' type='text/javascript'></script>", httpLink(conn, uri, NULL));
    } else {
        req = conn->data;
        minified = smatch(httpGetOption(options, "minified", 0), "true");
        indent = "";
        for (up = defaultScripts; *up; up++) {
            uri = httpLink(conn, sjoin("~/", mprGetPathBase(req->eroute->staticDir), *up, 
                minified ? ".min.js" : ".js", NULL), NULL);
            newline = up[1] ? "\r\n" :  "";
            espRender(conn, "%s<script src='%s' type='text/javascript'></script>%s", indent, uri, newline);
            indent = "    ";
        }
    }
}


/*
    Get a security token. This will use and existing token or create if not present. It will store in the session store.
 */
cchar *espGetSecurityToken(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;

    if (rx->securityToken == 0) {
        rx->securityToken = (char*) espGetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, 0);
        if (rx->securityToken == 0) {
            rx->securityToken = mprGetMD5(sfmt("%d-%p", mprGetTicks(), conn->rx));
            espSetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, rx->securityToken);
        }
    }
    return rx->securityToken;
}


/*
    Generate a security token
 */
void espSecurityToken(HttpConn *conn) 
{
    cchar   *securityToken;

    /*
        Add the token to headers for an alternative easy access via header APIs
     */
    securityToken = espGetSecurityToken(conn);
    espAddHeaderString(conn, "X-Security-Token", securityToken);
#if UNUSED && KEEP
    espRender(conn, "<meta name='SecurityTokenName' content='%s' />\r\n", ESP_SECURITY_TOKEN_NAME);
#endif
    espRender(conn, "<meta name='%s' content='%s' />", ESP_SECURITY_TOKEN_NAME, securityToken);
}


void espSetConn(HttpConn *conn)
{
    mprSetThreadData(((Esp*) MPR->espService)->local, conn);
}


void espStylesheet(HttpConn *conn, cchar *uri, cchar *optionString) 
{
    EspReq      *req;
    cchar       *indent, *newline;
    char        **up;
   
    if (uri) {
        espRender(conn, "<link rel='stylesheet' type='text/css' href='%s' />", httpLink(conn, uri, NULL));
    } else {
        req = conn->data;
        indent = "";
        for (up = defaultStylesheets; *up; up++) {
            uri = httpLink(conn, sjoin("~/", mprGetPathBase(req->eroute->staticDir), *up, NULL), NULL);
            newline = up[1] ? "\r\n" :  "";
            espRender(conn, "%s<link rel='stylesheet' type='text/css' href='%s' />%s", indent, uri, newline);
            indent = "    ";
        }
    }
}


static void pivot(EdiGrid *grid)
{
    //  TODO
    return;
}


static MprList *filterCols(EdiGrid *grid, MprHash *options, MprHash *columns)
{
    MprList     *gridCols, *result;
    cchar       *columnName;
    int         next, showId;

    gridCols = ediGetGridColumns(grid);
    result = mprCreateList(0, MPR_LIST_STATIC_VALUES);
    if (columns) {
        for (next = 0; (columnName = mprGetNextItem(gridCols, &next)) != 0; ) {
            if (mprLookupKey(columns, columnName)) {
                mprAddItem(result, ITOP(next - 1));
            }
        }
    } else {
        showId = smatch(httpGetOption(options, "showId", "true"), "true");
        for (next = 0; (columnName = mprGetNextItem(gridCols, &next)) != 0; ) {
            if (smatch(columnName, "id") && !showId) {
                continue;
            }
            mprAddItem(result, ITOP(next - 1));
        }
    }
    return result;
}


/*
    Render a table from a data grid

    Examples:

        table(grid, "{ refresh:'@update', period:'1000', pivot:'true' }");
        table(grid, "{ click:'@edit' }");
        table(grid, "columns: { \
            product:     { header: 'Product', width: '20%' }, \
            date:        { format: '%m-%d-%y' }, \
            'user.name': {} \
        }");
        table(readTable("users"));
        table(makeGrid("[{'name': 'peter', age: 23 }, {'name': 'mary', age: 22}]")

    MOB - sort
 */
void espTable(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    MprHash     *options, *columns, *colOptions, *rowOptions;
    MprList     *cols;
    EdiRec      *rec;
    EdiField    *fp;
    cchar       *title, *width, *o, *header, *value;
    int         i, c, r, ncols;
   
    mprAssert(grid);
    if (grid == 0) {
        return;
    }
    options = httpGetOptions(optionString);
    if (httpGetOption(options, "pivot", 0)) {
        pivot(grid);
    }
    if (grid->nrecords == 0) {
        espRender(conn, "<p>No Data</p>\r\n");
        return;
    }
    columns = httpGetOptionHash(options, "columns");
    cols = filterCols(grid, options, columns);
    ncols = mprGetListLength(cols);

    httpAddOption(options, "class", "-esp-table");
    espRender(conn, "<table%s>\r\n", map(conn, options));

    if (smatch(httpGetOption(options, "showHeader", "true"), "true")) {
        /*
            Emit header
         */
        espRender(conn, "    <thead>\r\n");
        if ((title = httpGetOption(options, "title", 0)) != 0) {
            espRender(conn, "        <tr class='-esp-table-title'><td colspan='%s'>%s</td></tr>\r\n", 
                mprGetListLength(cols), title);
        }
        espRender(conn, "        <tr class='-esp-table-header'>\r\n");

        rec = grid->records[0];
        for (i = 0; i < ncols; i++) {
            c = PTOI(mprGetItem(cols, i));
            mprAssert(c <= rec->nfields);
            fp = &rec->fields[c];
            width = ((o = httpGetOption(options, "width", 0)) != 0) ? sfmt(" width='%s'", o) : "";
            colOptions = (MprHash*) httpGetOption(columns, fp->name, 0);
            header = httpGetOption(colOptions, "header", spascal(fp->name));
            espRender(conn, "            <th%s>%s</th>\r\n", width, header);
        }
        espRender(conn, "        </tr>\r\n    </thead>\r\n");
    }
    espRender(conn, "    <tbody>\r\n");

    /*
        Emit rows
     */
    rowOptions = mprCreateHash(0, 0);
    httpSetOption(rowOptions, "data-click", httpGetOption(options, "data-click", 0));
    
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        httpSetOption(rowOptions, "id", rec->id);
        //  MOB - implement rowOptions: edit, key, params, remote
        espRender(conn, "        <tr%s>\r\n", map(conn, rowOptions));
        for (i = 0; i < ncols; i++) {
            c = PTOI(mprGetItem(cols, i));
            fp = &rec->fields[c];
            colOptions = (MprHash*) httpGetOption(columns, fp->name, 0);
            if (colOptions == 0) colOptions = mprCreateHash(0, 0);
            if (httpGetOption(colOptions, "align", 0) == 0) {
                if (fp->type == EDI_TYPE_INT || fp->type == EDI_TYPE_FLOAT) {
                    httpAddOption(colOptions, "align", "right");
                }
            }
            value = formatValue(fp, colOptions);
            espRender(conn, "            <td%s>%s</td>\r\n", map(conn, colOptions), value);
        }
    }
    espRender(conn, "        </tr>\r\n");
    espRender(conn, "    </tbody>\r\n</table>\r\n");
}


/*
    tabs(makeRec("{ Status: 'pane-1', Edit: 'pane-2' }"))
 */
void espTabs(HttpConn *conn, EdiRec *rec, cchar *optionString)
{
    MprHash     *options;
    EdiField    *fp;
    cchar       *attr, *uri;
    int         toggle;

    options = httpGetOptions(optionString);
    httpAddOption(options, "class", "-esp-tabs");
    attr = httpGetOption(options, "toggle", "data-click");
    if ((toggle = smatch(attr, "true")) != 0) {
        attr = "data-toggle";
    }
    espRender(conn, "<div%s>\r\n    <ul>\r\n", map(conn, options));
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        uri = toggle ? fp->value : httpLink(conn, fp->value, 0);
        espRender(conn, "      <li %s='%s'>%s</li>\r\n", attr, uri, fp->name);
    }
    espRender(conn, "    </ul>\r\n</div>\r\n");
}


static void textInner(HttpConn *conn, cchar *field, MprHash *options)
{
    cchar   *rows, *cols, *type, *value;

    type = "text";
    value = getValue(conn, field, options);
    if (value == 0 || *value == '\0') {
        value = espGetParam(conn, field, "");
    }
    if ((rows = httpGetOption(options, "rows", 0)) != 0) {
        cols = httpGetOption(options, "cols", "60");
        if (httpGetOption(options, "password", 0)) {
            type = "password";
        } else if (httpGetOption(options, "hidden", 0)) {
            type = "hidden";
        }
        espRender(conn, "<textarea name='%s' type='%s' cols='%s' rows='%s'%s>%s</textarea>", field, type, 
            cols, rows, map(conn, options), value);
    } else {
          espRender(conn, "<input name='%s' type='%s' value='%s'%s />", field, type, value, map(conn, options));
    }
}


void espText(HttpConn *conn, cchar *field, cchar *optionString)
{
    textInner(conn, field, httpGetOptions(optionString));
}


void espTree(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    //  MOB - implement
}

//  MOB - need control to render a partial view

/**************************************** Support *************************************/ 

static void emitFormErrors(HttpConn *conn, EdiRec *rec, MprHash *options)
{
    MprList         *errors;
    MprKeyValue     *error;
    int             count, next;
   
    if (!rec->errors || httpGetOption(options, "hideErrors", 0)) {
        return;
    }
    errors = ediGetRecErrors(rec);
    if (errors) {
        count = mprGetListLength(errors);
        espRender(conn, "<div class='-esp-form-error'><h2>The %s has %s it being saved.</h2>\r\n",
            spascal(rec->tableName), count <= 1 ? "an error that prevents" : "errors that prevent");
        espRender(conn, "    <p>There were problems with the following fields:</p>\r\n");
        espRender(conn, "    <ul>\r\n");
        for (next = 0; (error = mprGetNextItem(errors, &next)) != 0; ) {
            espRender(conn, "        <li>%s %s</li>\r\n", error->key, error->value);
        }
        espRender(conn, "    </ul>\r\n");
        espRender(conn, "</div>");
    }
}


static cchar *escapeValue(cchar *value, MprHash *options)
{
    if (httpGetOption(options, "escape", 0)) {
        return mprEscapeHtml(value);
    }
    return value;
}


static cchar *formatValue(EdiField *fp, MprHash *options)
{
    return ediFormatField(httpGetOption(options, "format", 0), fp);
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
        value = ediGetField(record, fieldName);
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


MprHash *makeParams(cchar *fmt, ...)
{
    va_list     args;
    MprObj      *obj;

    va_start(args, fmt);
    obj = mprDeserialize(sfmtv(fmt, args));
    va_end(args);
    return obj;
}


/*
    Map options to an attribute string. Remove all internal control specific options and transparently handles
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
        if (kp->type != MPR_JSON_OBJ && kp->type != MPR_JSON_ARRAY && !mprLookupKey(esp->internalOptions, kp->key)) {
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


void espInitHtmlOptions(Esp *esp)
{
    char   **op;

    for (op = internalOptions; *op; op++) {
        mprAddKey(esp->internalOptions, *op, op);
    }
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
