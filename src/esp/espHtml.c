/*
    espHtml.c -- HTML controls 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"
#include    "edi.h"

#if BIT_PACK_ESP
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
    - Which internal options need to have data- appended and then passed to client.

    Principles
        - All unknown options are passed through to the client
        - Internal options are filtered or mapped to data-*
        - Remote only takes a "true" value

    - Support flash as modal, transparent box
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
//  MOB - left aligned field seem to not be used
"field",

//  MOB - not implemented. Change to format
    "formatter",                    /* general: Printf style format string to apply to data */

    "header",                       /* table(): Column options header */
    "hidden",                       /* text(): Field should be hidden */
    "hideErrors",                   /* form(): Hide record validation errors */
    "insecure",                     /* form(): Don't generate a CSRF security token */

//  MOB - rename keys
    "key",                          /* general: key property/value data to provide for clickable events */
//  MOB - not implemented yet
    "keyFormat",                    /* General: How keys are handled for click events: "params" | "path" | "query" */
"kind",
    "minified",                     /* script(): Use minified script variants */
    "name",                         /* table(): Column options name */
"params",                           /* general: Parms to pass on click events */
"pass",
    "password",                     /* text(): Text input is a password */
    "pivot",                        /* table(): Pivot the table data */
    "remote",                       /* general: Set to true to make click event operate in the background */
"retain",
    "securityToken",                /* form(): Name of security token to use */
    "showHeader",                   /* table(): Show table column names header  */
    "showId",                       /* table(): Show the ID column */

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
//  MOB - what to do with these?
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
//  MOB -check against jquery.esp.js
static char *dataOptions[] = {
    //  MOB - Comment each one 
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
    httpInsertOption(options, "class", "-esp-alert");
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
    checked = scaselessmatch(value, checkedValue) ? " checked='yes'" : "";
    espRender(conn, "<input name='%s' type='checkbox'%s%s value='%s' />\r\n", field, map(conn, options), checked, checkedValue);
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
            httpInsertOption(options, "class", sfmt("-esp-flash -esp-flash-%s", kp->key));
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
    if (!scaselessmatch(method, "GET") && !scaselessmatch(method, "POST")) {
        /* All methods use POST and tunnel method in data-method */
        httpAddOption(options, "data-method", method);
        method = "POST";
    }
    if ((action = httpGetOption(options, "action", 0)) == 0) {
        action = (recid) ? "@update" : "@create";
    }
    uri = httpLink(conn, action, NULL);
    //  MOB - refactor
    if (smatch(httpGetOption(options, "remote", 0), "true")) {
        espRender(conn, "<form method='%s' data-remote='%s'%s >\r\n", method, uri, map(conn, options));
    } else {
        espRender(conn, "<form method='%s'  action='%s'%s >\r\n", method, uri, map(conn, options));
    }
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
    int         type, flags;

    req = conn->data;
    rec = req->record;
    if (ediGetColumnSchema(rec->edi, rec->tableName, fieldName, &type, &flags, NULL) < 0) {
        type = -1;
    }
    switch (type) {
    case EDI_TYPE_BOOL:
        espRadio(conn, fieldName, "{off: 0, on: 1}", optionString);
        break;
    case EDI_TYPE_DATE:
        /* MOB - could do calendar control */
        break;
    case EDI_TYPE_FLOAT:
    case EDI_TYPE_INT:
#if FUTURE
        if (flags & EDI_FOREIGN && send(fieldName, "Id")) {
            espDropdown(conn, fieldName, EdiGrid *choices, optionString);
            break;
        }
#endif
        /* Fall through */
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
    case EDI_TYPE_BINARY:
    default:
        httpError(conn, 0, "espInput: unknown field type %d", type);
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
    dropdown("priority", makeGrid("[{ id: 0, low: 0}, { id: 1, med: 1}, {id: 2, high: 2}]"), 0)
    dropdown("priority", makeGrid("[{ low: 0}, { med: 1}, {high: 2}]"), 0)
    dropdown("priority", makeGrid("[0, 10, 100]"), 0)

    Options can provide the defaultValue in a "value" property.
 */
void espDropdown(HttpConn *conn, cchar *field, EdiGrid *choices, cchar *optionString) 
{
    EdiRec      *rec;
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
        rec = choices->records[r];
        if (rec->nfields == 1) {
            value = rec->fields[0].value;
        } else {
            value = ediGetFieldValue(rec, field);
        }
        if ((id = ediGetFieldValue(choices->records[r], "id")) == 0) {
            id = value;
        }
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
void espRadio(HttpConn *conn, cchar *field, cchar *choicesString, cchar *optionsString)
{
    MprKey      *kp;
    MprHash     *choices, *options;
    cchar       *value, *checked;

    choices = httpGetOptions(choicesString);
    options = httpGetOptions(optionsString);
    value = getValue(conn, field, options);
    for (kp = 0; (kp = mprGetNextKey(choices, kp)) != 0; ) {
        checked = (smatch(kp->data, value)) ? " checked" : "";
        espRender(conn, "%s <input type='radio' name='%s' value='%s'%s%s />\r\n", 
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
        rx->securityToken = (char*) httpGetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, 0);
        if (rx->securityToken == 0) {
            rx->securityToken = mprGetMD5(sfmt("%d-%p", mprGetTicks(), conn->rx));
            httpSetSessionVar(conn, ESP_SECURITY_TOKEN_NAME, rx->securityToken);
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


#if UNUSED
static int findCol(MprHash *columns, cchar *columnName)
{
    char    key[8];
    int     i;

    len = mprGetHashLength(columns);
    for (i = 0; i < len; i++) {
        itosbuf(key, sizeof(key), i);
        if (mprLookupKey(columns, key)) {
            return i;
        }
    }
    return MPR_ERR_CANT_FIND;
}
#endif


/*
    Grid is modified. Columns are removed and sorted as required.
 */
static void filterCols(EdiGrid *grid, MprHash *options, MprHash *colOptions)
{
    MprList     *gridCols;
    MprHash     *cp;
    EdiRec      *rec;
    EdiField    f;
    cchar       *columnName;
    char        key[8];
    int         ncols, r, fnum, currentPos, c, index, *desired, *location, pos, t;

    gridCols = ediGetGridColumns(grid);

    if (colOptions) {
        if (!(colOptions->flags & MPR_HASH_LIST)) {
            mprError("Grid columns must be an array");
            return;
        }
        /*
            Sort grid record columns into the order specified by the column options
         */
        ncols = grid->records[0]->nfields;
        location = mprAlloc(sizeof(int) * ncols);
        for (c = 0; c < ncols; c++) {
            location[c] = c;
        }
        ncols = mprGetHashLength(colOptions);
        desired = mprAlloc(sizeof(int) * ncols);
        for (c = 0; c < ncols; c++) {
            cp = mprLookupKey(colOptions, itosbuf(key, sizeof(key), c, 10));
            if ((columnName = mprLookupKey(cp, "name")) == 0) {
                mprError("Can't locate \"name\" field for column in table");
                return;
            }
            pos = mprLookupStringItem(gridCols, columnName);
            mprAssert(pos >= 0);
            desired[c] = pos;
            location[c] = c;
        }
        for (c = 0; c < ncols; c++) {
            for (r = 0; r < grid->nrecords; r++) {
                rec = grid->records[r];
                rec->nfields = ncols;
                fnum = desired[c];
                if (fnum < 0) {
                    continue;
                }
                currentPos = location[fnum];

                f = rec->fields[c];
                rec->fields[c] = rec->fields[currentPos];
                rec->fields[currentPos] = f;
            }
            t = location[c];
            location[c] = location[fnum];
            location[fnum] = t;
        }
        
    } else {
        /*
            If showId is false, remove the "id" column
         */
        if (httpOption(options, "showId", "false", 0) && (index = mprLookupStringItem(gridCols, "id")) >= 0) {
            for (r = 0; r < grid->nrecords; r++) {
                rec = grid->records[r];
                rec->nfields--;
                mprMemcpy(rec->fields, sizeof(EdiField) * rec->nfields, &rec->fields[index],
                    sizeof(EdiField) * (rec->nfields - 1));
            }
        }
    }
}


static char *hashToString(MprHash *hash, cchar *sep)
{
    MprBuf  *buf;
    cchar   *data;
    char    key[8];
    int     i, len;

    len = mprGetHashLength(hash);
    buf = mprCreateBuf(0, 0);
    mprPutCharToBuf(buf, '{');
    for (i = 0; i < len; ) {
        data = mprLookupKey(hash, itosbuf(key, sizeof(key), i, 10));
        mprPutStringToBuf(buf, data);
        if (++i < len) {
            mprPutStringToBuf(buf, sep ? sep : ",");
        }
    }
    mprPutCharToBuf(buf, '}');
    mprAddNullToBuf(buf);
    return mprGetBufStart(buf);
}


static EdiGrid *hashToGrid(MprHash *hash)
{
    EdiGrid *grid;
    EdiRec  *rec;
    cchar   *data;
    char    key[8];
    int     i, len;

    len = mprGetHashLength(hash);
    grid = ediCreateBareGrid(NULL, "grid", len);
    for (i = 0; i < len; i++) {
        data = mprLookupKey(hash, itosbuf(key, sizeof(key), i, 10));
        grid->records[i] = rec = ediCreateBareRec(NULL, "grid", 1);
        rec->fields[0].name = sclone("value");
        rec->fields[0].type = EDI_TYPE_STRING;
        rec->fields[0].value = data;
    }
    return grid;
}


/*
    Render a table from a data grid

    Examples:

        table(grid, "{ refresh:'@update', period:'1000', pivot:'true' }");
        table(grid, "{ click:'@edit' }");
        table(grid, "columns: [ \
            { name: product, header: 'Product', width: '20%' }, \
            { name: date,    format: '%m-%d-%y' }, \
            { name: 'user.name' }, \
        ]");
        table(readTable("users"));
        table(makeGrid("[{'name': 'peter', age: 23 }, {'name': 'mary', age: 22}]")
        table(grid, "{ \
            columns: [ \
                { name: 'speed',         header: 'Speed', dropdown: [100, 1000, 40000] }, \
                { name: 'adminMode',     header: 'Admin Mode', radio: ['Up', 'Down'] }, \
                { name: 'state',         header: 'State', radio: ['Enabled', 'Disabled'] }, \
                { name: 'autoNegotiate', header: 'Auto Negotiate', checkbox: ['enabled'] }, \
                { name: 'type',          header: 'Type' }, \
            ], \
            edit: true, \
            pivot: true, \
            showHeader: false, \
            class: '-esp-pivot', \
        }");

 */
static void pivotTable(HttpConn *conn, EdiGrid *grid, MprHash *options)
{
    MprHash     *colOptions, *rowOptions, *thisCol, *dropdown, *radio;
    MprList     *cols;
    EdiRec      *rec;
    EdiField    *fp;
    cchar       *title, *width, *o, *header, *name, *checkbox;
    char        index[8];
    int         c, r, ncols;
   
    mprAssert(grid);
    if (grid->nrecords == 0) {
        espRender(conn, "<p>No Data</p>\r\n");
        return;
    }
    colOptions = httpGetOptionHash(options, "columns");
    cols = ediGetGridColumns(grid);
    ncols = mprGetListLength(cols);
    rowOptions = mprCreateHash(0, 0);
    httpSetOption(rowOptions, "data-click", httpGetOption(options, "data-click", 0));
    httpInsertOption(options, "class", "-esp-pivot");
    httpInsertOption(options, "class", "-esp-table");
    espRender(conn, "<table%s>\r\n", map(conn, options));

    /*
        Table header
     */
//  MOB -- debug if pivot
    if (httpOption(options, "showHeader", "true", 1)) {
        espRender(conn, "    <thead>\r\n");
        if ((title = httpGetOption(options, "title", 0)) != 0) {
            espRender(conn, "        <tr class='-esp-table-title'><td colspan='%s'>%s</td></tr>\r\n", 
                mprGetListLength(cols), title);
        }
        espRender(conn, "        <tr class='-esp-table-header'>\r\n");
        rec = grid->records[0];
        for (r = 0; r < ncols; r++) {
            mprAssert(r <= grid->nrecords);
            width = ((o = httpGetOption(options, "width", 0)) != 0) ? sfmt(" width='%s'", o) : "";
            thisCol = mprLookupKey(colOptions, itosbuf(index, sizeof(index), r, 10));
            header = httpGetOption(thisCol, "header", spascal(rec->id));
            espRender(conn, "            <th%s>%s</th>\r\n", width, header);
        }
        espRender(conn, "        </tr>\r\n    </thead>\r\n");
    }
    espRender(conn, "    <tbody>\r\n");

    /*
        Table body data
        TODO OPT
        MOB implement rowOptions: edit, key, params, remote
     */
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        httpSetOption(rowOptions, "id", rec->id);
        espRender(conn, "        <tr%s>\r\n", map(conn, rowOptions));
        for (c = 0; c < ncols; c++) {
            fp = &rec->fields[c];
            thisCol = mprLookupKey(colOptions, itosbuf(index, sizeof(index), r, 10));
            if (httpGetOption(thisCol, "align", 0) == 0) { // MOB OPT
                if (fp->type == EDI_TYPE_INT || fp->type == EDI_TYPE_FLOAT) {
                    if (!thisCol) {
                        thisCol = mprCreateHash(0, 0);
                    }
                    httpInsertOption(thisCol, "align", "right");
                }
            }
            if (c == 0) {
                /* 
                    Render column name
                 */
                name = httpGetOption(thisCol, "header", spascal(rec->id));
                //  MOB - need httpGetOptionAsGrid, httpGetOptionAsString
                //  MOB - converting back via hashToString is very inefficient. Perhaps inline Dropdown, Radio and checkbox here
                if (httpOption(options, "edit", "true", 0) && httpOption(thisCol, "edit", "true", 1)) {
                    espRender(conn, "            <td%s>%s</td><td>", map(conn, thisCol), name);
                    if ((dropdown = httpGetOption(thisCol, "dropdown", 0)) != 0) {
                        espDropdown(conn, fp->name, hashToGrid(dropdown), 0);
                    } else if ((radio = httpGetOption(thisCol, "radio", 0)) != 0) {
                        espRadio(conn, fp->name, hashToString(radio, 0), 0);
                    } else if ((checkbox = httpGetOption(thisCol, "checkbox", 0)) != 0) {
                        /* MOB - but need to type check. What if checkbox is not a string? */
                        espCheckbox(conn, fp->name, checkbox, 0);
                    } else {
                        espInput(conn, fp->name, 0);
                    }
                    espRender(conn, "</td>\r\n");
                } else {
                    espRender(conn, "            <td%s>%s</td><td>%s</td>\r\n", map(conn, thisCol), name, fp->value);
                }                
            } else {
                espRender(conn, "            <td%s>%s</td>\r\n", map(conn, thisCol), fp->value);
            }
        }
    }
    espRender(conn, "        </tr>\r\n");
    espRender(conn, "    </tbody>\r\n</table>\r\n");
}


void espTable(HttpConn *conn, EdiGrid *grid, cchar *optionString)
{
    MprHash     *options, *colOptions, *rowOptions, *thisCol;
    MprList     *cols;
    EdiRec      *rec;
    EdiField    *fp;
    cchar       *title, *width, *o, *header, *value, *sortColumn;
    char        index[8];
    int         c, r, ncols, sortOrder;
   
    mprAssert(grid);
    if (grid == 0) {
        return;
    }
    options = httpGetOptions(optionString);
    if (grid->nrecords == 0) {
        espRender(conn, "<p>No Data</p>\r\n");
        return;
    }
    if (grid->flags & EDI_GRID_READ_ONLY) {
        grid = ediCloneGrid(grid);
    }
    if ((sortColumn = httpGetOption(options, "sort", 0)) != 0) {
        sortOrder = httpOption(options, "sortOrder", "ascending", 1);
        ediSortGrid(grid, sortColumn, sortOrder);
    }
    colOptions = httpGetOptionHash(options, "columns");

    //  MOB - this modifies the grid. Need to ensure it is not a database grid.
    filterCols(grid, options, colOptions);

    if (httpOption(options, "pivot", "true", 0) != 0) {
        return pivotTable(conn, ediPivotGrid(grid, 1), options);
    }
    cols = ediGetGridColumns(grid);
    ncols = mprGetListLength(cols);
    rowOptions = mprCreateHash(0, 0);
    httpSetOption(rowOptions, "data-click", httpGetOption(options, "data-click", 0));
    
    httpInsertOption(options, "class", "-esp-table");
    espRender(conn, "<table%s>\r\n", map(conn, options));

    /*
        Table header
     */
    if (httpOption(options, "showHeader", "true", 1)) {
        espRender(conn, "    <thead>\r\n");
        if ((title = httpGetOption(options, "title", 0)) != 0) {
            espRender(conn, "        <tr class='-esp-table-title'><td colspan='%s'>%s</td></tr>\r\n", 
                mprGetListLength(cols), title);
        }
        espRender(conn, "        <tr class='-esp-table-header'>\r\n");
        rec = grid->records[0];
        for (c = 0; c < ncols; c++) {
            mprAssert(c <= rec->nfields);
            fp = &rec->fields[c];
            width = ((o = httpGetOption(options, "width", 0)) != 0) ? sfmt(" width='%s'", o) : "";
            thisCol = mprLookupKey(colOptions, itosbuf(index, sizeof(index), c, 10));
            header = httpGetOption(thisCol, "header", spascal(fp->name));
            espRender(conn, "            <th%s>%s</th>\r\n", width, header);
        }
        espRender(conn, "        </tr>\r\n    </thead>\r\n");
    }
    espRender(conn, "    <tbody>\r\n");

    /*
        Table body data
        TODO OPT
        MOB implement rowOptions: edit, key, params, remote
     */
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        httpSetOption(rowOptions, "id", rec->id);
        espRender(conn, "        <tr%s>\r\n", map(conn, rowOptions));
        for (c = 0; c < ncols; c++) {
            fp = &rec->fields[c];
            thisCol = mprLookupKey(colOptions, itosbuf(index, sizeof(index), c, 10));

            if (httpGetOption(thisCol, "align", 0) == 0) { // MOB OPT
                if (fp->type == EDI_TYPE_INT || fp->type == EDI_TYPE_FLOAT) {
                    if (!thisCol) {
                        thisCol = mprCreateHash(0, 0);
                    }
                    httpInsertOption(thisCol, "align", "right");
                }
            }
            value = formatValue(fp, thisCol);
            espRender(conn, "            <td%s>%s</td>\r\n", map(conn, thisCol), value);
        }
        espRender(conn, "        </tr>\r\n");
    }
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
    httpInsertOption(options, "class", "-esp-tabs");
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
        value = ediGetFieldValue(record, fieldName);
        if (record->errors) {
            for (next = 0; (error = mprGetNextItem(record->errors, &next)) != 0; ) {
                if (smatch(error->key, fieldName)) {
                    httpInsertOption(options, "class", "-esp-field-error");
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
    Map options to an attribute string. Remove all internal control specific options and transparently handle URI link options.
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


#endif /* BIT_PACK_ESP */
/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
