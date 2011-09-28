/*
    mdb.c -- In-Memory Embedded Database (MDB)

    WARNING: This is prototype code

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"
#include    "edi.h"
#include    "mdb.h"

#if BLD_FEATURE_ESP
/************************************* Local **********************************/

#define MDB_LOAD_BEGIN   1       /* Initial state */
#define MDB_LOAD_TABLE   2       /* Parsing a table */
#define MDB_LOAD_HINTS   3       /* Parsing hints */
#define MDB_LOAD_SCHEMA  4       /* Parsing schema */
#define MDB_LOAD_COL     5       /* Parsing column schema */ 
#define MDB_LOAD_DATA    6       /* Parsing data */
#define MDB_LOAD_FIELD   7       /* Parsing fields */

/************************************ Forwards ********************************/

static void autoSave(Mdb *mdb, MdbTable *table);
static MdbCol *createCol(MdbTable *table, cchar *columnName);
static EdiRec *createRec(Edi *edi, MdbRow *row);
static MdbRow *createRow(Mdb *mdb, MdbTable *table);
static cchar *eatSpace(cchar *tok);
static cchar *findEndKeyword(Mdb *mdb, cchar *str);
static cchar *findQuote(cchar *tok, int quote);
static MdbCol *getCol(MdbTable *table, int col);
static EdiField getField(MdbRow *row, int fid);
static MdbRow *getRow(MdbTable *table, int rid);
static MdbTable *getTable(Mdb *mdb, int tid);
MdbSchema *growSchema(MdbTable *table);
static MdbCol *lookupColumn(MdbTable *table, cchar *columnName);
static int lookupRow(MdbTable *table, cchar *key);
static MdbTable *lookupTable(Mdb *mdb, cchar *tableName);
static void manageCol(MdbCol *col, int flags);
static void manageMdb(Mdb *mdb, int flags);
static void manageRow(MdbRow *row, int flags);
static void manageSchema(MdbSchema *schema, int flags);
static void manageTable(MdbTable *table, int flags);
static int parseMdb(Mdb *mdb);
static void parseError(Mdb *mdb, cchar *fmt, ...);
static int parseType(cchar *type);
static void setAutoInc(MdbRow *row, MdbCol *col);
static int setField(MdbRow *row, MdbCol *col, cchar *value);

static int mdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static int mdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
static int mdbAddTable(Edi *edi, cchar *tableName);
static int mdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static void mdbClose(Edi *edi);
static int mdbDelete(Edi *edi, cchar *path);
static int mdbDeleteRow(Edi *edi, cchar *tableName, cchar *key);
static EdiGrid *mdbGetAll(Edi *edi, cchar *tableName);
static MprList *mdbGetColumns(Edi *edi, cchar *tableName);
static EdiField mdbGetField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
static EdiRec *mdbGetRec(Edi *edi, cchar *tableName, cchar *key);
static int mdbGetSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags);
static MprList *mdbGetTables(Edi *edi);
static int mdbLoad(Edi *edi, cchar *path);
static int mdbLoadFromString(Edi *edi, cchar *string);
static int mdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName);
static Edi *mdbOpen(cchar *path, int flags);
static EdiGrid *mdbQuery(Edi *edi, cchar *cmd);
static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);
static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);
static int mdbRemoveTable(Edi *edi, cchar *tableName);
static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);
static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
static int mdbSetField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value, int flags);
static int mdbSetRec(Edi *edi, cchar *tableName, cchar *key, MprHash *params);
static int mdbSave(Edi *edi);

EdiProvider MdbProvider = {
    "mdb",
    mdbAddColumn, mdbAddIndex, mdbAddTable, mdbChangeColumn, mdbClose, mdbDelete, mdbDeleteRow, mdbGetAll, mdbGetColumns,
    mdbGetField, mdbGetRec, mdbGetSchema, mdbGetTables, mdbLoad, mdbLookupField, mdbOpen, mdbQuery, mdbRemoveColumn,
    mdbRemoveIndex, mdbRemoveTable, mdbRenameTable, mdbRenameColumn, mdbSetField, mdbSetRec, mdbSave,
};

/************************************* Code ***********************************/

void mdbInit()
{
    ediAddProvider(&MdbProvider);
}


static Mdb *mdbCreate(cchar *path, int flags)
{
    Mdb      *mdb;

    if ((mdb = mprAllocObj(Mdb, manageMdb)) == 0) {
        return 0;
    }
    mdb->edi.provider = &MdbProvider;
    mdb->edi.flags = flags;
    mdb->path = sclone(path);
    mdb->mutex = mprCreateLock();
    return mdb;
}


static void manageMdb(Mdb *mdb, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mdb->path);
        mprMark(mdb->mutex);
        mprMark(mdb->tables);
        /* Don't mark load fields */
    }
}


static void mdbClose(Edi *edi)
{
    Mdb     *mdb;
   
    mdb = (Mdb*) edi;
    mdb->path = 0;
    mdb->tables = 0;
}


static int mdbDelete(Edi *edi, cchar *path)
{
    return mprDeletePath(path);
}


static Edi *mdbOpen(cchar *path, int flags)
{
    Mdb         *mdb;
    MprFile     *file;

    if ((mdb = mdbCreate(path, flags)) == 0) {
        return 0;
    }
    if (!mprPathExists(path, R_OK)) {
        if (flags & EDI_CREATE) {
            /* Create an empty file */
            if ((file = mprOpenFile(path, 0664, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY)) == 0) {
                return 0;
            }
            mprCloseFile(file);
        }
    } else {
        if (mdbLoad((Edi*) mdb, path) < 0) {
            return 0;
        }
    }
    return (Edi*) mdb;
}


static int mdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mprAssert(edi);
    mprAssert(tableName && *tableName);
    mprAssert(columnName && *columnName);
    mprAssert(type);

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = createCol(table, columnName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    col->type = type;
    col->flags = flags;
    autoSave(mdb, table);
    return 0;
}


/*
    IndexName is not implemented yet
 */
static int mdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mprAssert(edi);
    mprAssert(tableName && *tableName);
    mprAssert(columnName && *columnName);

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((table->index = mprCreateHash(0, 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    table->indexCol = col;
    col->flags |= EDI_INDEX;
    autoSave(mdb, table);
    return 0;
}


static int mdbAddTable(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mprAssert(edi);
    mprAssert(tableName && *tableName);

    mdb = (Mdb*) edi;
    if ((table = mprAllocObj(MdbTable, manageTable)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if ((table->rows = mprCreateList(0, 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    table->name = sclone(tableName);
    if (mdb->tables == 0) {
        mdb->tables = mprCreateList(0, 0);
    }
    if (!growSchema(table)) {
        return MPR_ERR_MEMORY;
    }
    mprAddItem(mdb->tables, table);
    autoSave(mdb, lookupTable(mdb, tableName));
    return 0;
}


static int mdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mprAssert(edi);
    mprAssert(tableName && *tableName);
    mprAssert(columnName && *columnName);
    mprAssert(type);

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    col->name = sclone(columnName);
    col->type = type;
    autoSave(mdb, table);
    return 0;
}


static int mdbDeleteRow(Edi *edi, cchar *tableName, cchar *key)
{
    Mdb         *mdb;
    MdbTable    *table;
    int         r, rc;

    mprAssert(edi);
    mprAssert(tableName && *tableName);
    mprAssert(key && *key);

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((r = lookupRow(table, key)) < 0) {
        return MPR_ERR_CANT_FIND;
    }
    rc = mprRemoveItemAtPos(table->rows, r);
    autoSave(mdb, table);
    return rc;
}


static EdiGrid *mdbGetAll(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    EdiGrid     *grid;
    MdbTable    *table;
    MdbRow      *row;
    int         nrows, next;

    mprAssert(edi);
    mprAssert(tableName && *tableName);

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return 0;
    }
    nrows = mprGetListLength(table->rows);
    if ((grid = ediCreateGrid(edi, tableName, nrows)) == 0) {
        return 0;
    }
    for (ITERATE_ITEMS(table->rows, row, next)) {
        grid->records[next - 1] = createRec(edi, row);
    }
    return grid;
}


/*
    Return a list of column names
 */
static MprList *mdbGetColumns(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbSchema   *schema;
    MprList     *list;
    int         i;

    mprAssert(edi);
    mprAssert(tableName && *tableName);

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return 0;
    }
    schema = table->schema;
    mprAssert(schema);
    list = mprCreateList(schema->ncols, 0);
    for (i = 0; i < schema->ncols; i++) {
        mprAddItem(list, schema->cols[i].name);
    }
    return list;
}


static EdiField makeFieldFromRow(MdbRow *row, MdbCol *col)
{
    EdiField    f;

    f.valid = 1;
    f.value = row->fields[col->cid];
    f.type = col->type;
    f.name = col->name;
    f.flags = col->flags;
    return f;
}


static EdiField mdbGetField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;
    MdbRow      *row;
    EdiField    err;
    int         r;

    mdb = (Mdb*) edi;
    err.valid = 0;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return err;
    }
    if ((col = lookupColumn(table, fieldName)) == 0) {
        return err;
    }
    if ((r = lookupRow(table, key)) < 0) {
        return err;
    }
    row = mprGetItem(table->rows, r);
    return makeFieldFromRow(row, col);
}


static EdiRec *mdbGetRec(Edi *edi, cchar *tableName, cchar *key)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    EdiRec      *rec;
    int         r, nrows;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return 0;
    }
    nrows = mprGetListLength(table->rows);
    if ((r = lookupRow(table, key)) < 0) {
        return 0;
    }
    row = mprGetItem(table->rows, r);
    if ((rec = createRec(edi, row)) == 0) {
        return 0;
    }
    return rec;
}


static int mdbGetSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if (type) {
        *type = col->type;
    }
    if (flags) {
        *flags = col->flags;
    }
    return 0;
}


static MprList *mdbGetTables(Edi *edi)
{
    Mdb         *mdb;
    MprList     *list;
    MdbTable     *table;
    int         tid, ntables;

    mdb = (Mdb*) edi;
    list = mprCreateList(-1, 0);
    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = mprGetItem(mdb->tables, tid);
        mprAddItem(list, table->name);
    }
    return list;
}


static int mdbLoad(Edi *edi, cchar *path)
{
    cchar       *data;
    ssize       len;

    if ((data = mprReadPath(path, &len)) == 0) {
        return MPR_ERR_CANT_READ;
    }
    return mdbLoadFromString(edi, data);
}


static int mdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, fieldName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    return col->cid;
}


static EdiGrid *mdbQuery(Edi *edi, cchar *cmd)
{
    //  TODO
    mprAssert(0);
    return 0;
}


static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbSchema   *schema;
    MdbCol      *col;
    int         c;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if (table->indexCol == col) {
        table->index = 0;
        table->indexCol = 0;
    }
    schema = table->schema;
    mprAssert(schema);
    for (c = col->cid; c < schema->ncols; c++) {
        schema->cols[c] = schema->cols[c + 1];
    }
    schema->ncols--;
    schema->cols[schema->ncols].name = 0;
    mprAssert(schema->ncols >= 0);
    autoSave(mdb, table);
    return 0;
}


//  MOB - indexName is ignored
static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    table->index = 0;
    if (table->indexCol) {
        table->indexCol->flags &= ~EDI_INDEX;
        table->indexCol = 0;
        autoSave(mdb, table);
    }
    return 0;
}


static int mdbRemoveTable(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    int         next;

    mdb = (Mdb*) edi;
    for (ITERATE_ITEMS(mdb->tables, table, next)) {
        if (smatch(table->name, tableName)) {
            mprRemoveItem(mdb->tables, table);
            autoSave(mdb, table);
            break;
        }
    }
    return MPR_ERR_CANT_FIND;;
}


static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    table->name = sclone(newTableName);
    autoSave(mdb, table);
    return 0;
}


static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    col->name = sclone(newColumnName);
    autoSave(mdb, table);
    return 0;
}


static int mdbSetField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value, int flags)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    MprKey      *kp;
    int         r;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((r = lookupRow(table, key)) < 0) {
        row = createRow(mdb, table);
    }
    if ((row = getRow(table, r)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, fieldName)) != 0) {
        setField(row, col, kp->data);
    }
    return 0;
}


static int mdbSetRec(Edi *edi, cchar *tableName, cchar *key, MprHash *params)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    MprKey      *kp;
    int         r, setID;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if (key == 0 || (r = lookupRow(table, key)) < 0) {
        row = createRow(mdb, table);
    } else {
        if ((row = getRow(table, r)) == 0) {
            return MPR_ERR_CANT_FIND;
        }
    }
    setID = 0;
    for (ITERATE_KEYS(params, kp)) {
        if ((col = lookupColumn(table, kp->key)) != 0) {
            setField(row, col, kp->data);
            if (col->cid == 0) {
                setID++;
            }
        }
    }
    if (!setID) {
        //  MOB OPT - change to itos when it allocates
        setField(row, getCol(table, 0), 0);
    }
    autoSave(mdb, table);
    return 0;
}


/******************************** Database Loading ***************************/

static void clearLoadState(Mdb *mdb)
{
    mdb->loadNcols = 0;
    mdb->loadField = 0;
    mdb->loadCol = 0;
    mdb->loadRow = 0;
}


static void pushState(Mdb *mdb, int state)
{
    mprPushItem(mdb->loadStack, LTOP(state));
    mdb->loadState = state;
}


static void popState(Mdb *mdb)
{
    mprPopItem(mdb->loadStack);
    mdb->loadState = (int) PTOL(mprGetLastItem(mdb->loadStack));
}


static cchar *parseQuotedName(Mdb *mdb)
{
    cchar    *etok, *name;
    int      quote;

    quote = *mdb->tok;
    if ((etok = findQuote(++mdb->tok, quote)) == 0) {
        parseError(mdb, "Missing closing quote");
        return 0;
    }
    name = snclone(mdb->tok, etok - mdb->tok);
    mdb->tok = ++etok;
    return name;
}


static cchar *parseUnquotedName(Mdb *mdb)
{
    cchar    *etok, *name;

    etok = findEndKeyword(mdb, mdb->tok);
    name = snclone(mdb->tok, etok - mdb->tok);
    mdb->tok = etok;
    return name;
}


static cchar *parseName(Mdb *mdb)
{
    mdb->tok = eatSpace(mdb->tok);
    if (*mdb->tok == '"' || *mdb->tok == '\'') {
        return parseQuotedName(mdb);
    } else {
        return parseUnquotedName(mdb);
    }
}


static int peekSep(Mdb *mdb)
{
    int     sep;

    mdb->tok = eatSpace(mdb->tok);
    sep = *mdb->tok;
    if (sep != ':' && sep != ',' && sep != ']') {
        parseError(mdb, "Missing ':', ',' or ']' in input");
        return MPR_ERR_BAD_FORMAT;
    } 
    return sep;
}


static int preState(Mdb *mdb, cchar *name)
{
    switch (mdb->loadState) {
    case MDB_LOAD_BEGIN:
        if (mdbAddTable((Edi*) mdb, name) < 0) {
            return MPR_ERR_MEMORY;
        }
        mdb->loadTable = lookupTable(mdb, name);
        clearLoadState(mdb);
        pushState(mdb, MDB_LOAD_TABLE);
        break;
        
    case MDB_LOAD_TABLE:
        if (smatch(name, "hints")) {
            pushState(mdb, MDB_LOAD_HINTS);
        } else if (smatch(name, "schema")) {
            pushState(mdb, MDB_LOAD_SCHEMA);
        } else if (smatch(name, "data")) {
            pushState(mdb, MDB_LOAD_DATA);
        } else {
            parseError(mdb, "Bad property '%s'", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;
    
    case MDB_LOAD_HINTS:
        break;

    case MDB_LOAD_SCHEMA:
        if ((mdb->loadCol = createCol(mdb->loadTable, name)) == 0) {
            parseError(mdb, "Can't create '%s' column", name);
            return MPR_ERR_MEMORY;
        }
        pushState(mdb, MDB_LOAD_COL);
        break;

    case MDB_LOAD_COL:
        break;

    case MDB_LOAD_DATA:
        if ((mdb->loadRow = createRow(mdb, mdb->loadTable)) == 0) {
            return MPR_ERR_MEMORY;
        }
        mdb->loadField = 0;
        pushState(mdb, MDB_LOAD_FIELD);
        break;

    case MDB_LOAD_FIELD:
        break;

    default:
        parseError(mdb, "Potential corrupt data. Bad state '%d'");
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static cchar *parseValue(Mdb *mdb)
{
    cchar   *etok, *value;
    int     quote;

    value = 0;
    if (*mdb->tok == '"' || *mdb->tok == '\'') {
        quote = *mdb->tok;
        if ((etok = findQuote(++mdb->tok, quote)) == 0) {
            parseError(mdb, "Missing closing quote");
            return 0;
        }
        value = snclone(mdb->tok, etok - mdb->tok);

    } else {
        etok = findEndKeyword(mdb, mdb->tok);
        value = snclone(mdb->tok, etok - mdb->tok);
    }
    mdb->tok = etok + 1;
    return value;
}


/*
    Post value processing
 */
static int postState(Mdb *mdb, cchar *name, cchar *value)
{
    MdbCol      *col;

    switch (mdb->loadState) {
    case MDB_LOAD_BEGIN:
        break;

    case MDB_LOAD_TABLE:
    case MDB_LOAD_SCHEMA:
    case MDB_LOAD_DATA:
        break;

    case MDB_LOAD_HINTS:
        if (smatch(name, "ncols")) {
            mdb->loadNcols = atoi(value);
        } else {
            parseError(mdb, "Unknown hint '%s'", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_COL:
        if (smatch(name, "index")) {
            mdbAddIndex((Edi*) mdb, mdb->loadTable->name, mdb->loadCol->name, NULL);
        } else if (smatch(name, "type")) {
            if ((mdb->loadCol->type = parseType(value)) <= 0) {
                parseError(mdb, "Bad column type %s", value);
                return MPR_ERR_BAD_FORMAT;
            }
        } else if (smatch(name, "key")) {
            mdb->loadCol->flags |= EDI_KEY;
        } else if (smatch(name, "autoinc")) {
            mdb->loadCol->flags |= EDI_AUTO_INC;
        } else if (smatch(name, "notnull")) {
            mdb->loadCol->flags |= EDI_NOT_NULL;
        } else {
            parseError(mdb, "Bad property '%s' in column definition", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_FIELD:
        col = getCol(mdb->loadTable, mdb->loadField);
        mprAssert(col);
        if (col) {
            setField(mdb->loadRow, col, value);
        }
        mdb->loadField++;
        break;

    default:
        parseError(mdb, "Bad state in '%s', potential corrupt data");
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static int parseMdb(Mdb *mdb)
{
    cchar   *name, *value;
    int     rc, sep;

    while (*mdb->tok) {
        mdb->tok = eatSpace(mdb->tok);
        switch (*mdb->tok) {
        case '\n':
            mdb->lineNumber++;
            mdb->tok++;
            break;

        case ',':
            mdb->tok++;
            continue;

        case '{':
        case '[':
            ++mdb->tok;
            break;

        case '}':
        case ']':
            /* End of object or array */
            mdb->tok++;
            popState(mdb);
            break;
            
        default:
            if ((name = parseName(mdb)) == 0) {
                return MPR_ERR_BAD_FORMAT;
            }
            if ((rc = preState(mdb, name)) < 0) {
                return rc;
            }
            if ((sep = peekSep(mdb)) < 0) {
                return MPR_ERR;
            }
            if (sep == ':') {
                mdb->tok = eatSpace(mdb->tok + 1);
                if (*mdb->tok == '{' || *mdb->tok == '[') {
                    mdb->tok++;
                    break;
                } else if ((value = parseValue(mdb)) == 0) {
                    return MPR_ERR_BAD_FORMAT;
                }
            } else {
                value = name;
            }
            if ((rc = postState(mdb, name, value)) < 0) {
                return rc;
            }
        }
    }
    return 0;
}


static int mdbLoadFromString(Edi *edi, cchar *str)
{
    Mdb         *mdb;
    int         rc;

    mdb = (Mdb*) edi;
    mdb->edi.flags |= EDI_SUPPRESS_SAVE;
    mdb->flags |= MDB_LOADING;
    mdb->tok = str;
    mdb->loadStack = mprCreateList(0, 0);
    mdb->lineNumber = 1;
    pushState(mdb, MDB_LOAD_BEGIN);
    rc = parseMdb(mdb);
    mdb->loadStack = 0;
    mdb->tok = 0;
    mdb->flags &= ~MDB_LOADING;
    mdb->edi.flags &= ~EDI_SUPPRESS_SAVE;
    if (rc < 0) {
        mdb->path = 0;
        return rc;
    }
    return 0;
}


/******************************** Database Saving ****************************/

static void autoSave(Mdb *mdb, MdbTable *table)
{
    mprAssert(mdb);
    mprAssert(table);

    if (mdb->edi.flags & EDI_AUTO_SAVE && !(mdb->edi.flags & EDI_SUPPRESS_SAVE)) {
        //  MOB - should have dirty bit
        mdbSave((Edi*) mdb);
    }
}


static int mdbSave(Edi *edi)
{
    Mdb         *mdb;
    MdbSchema    *schema;
    MdbTable     *table;
    EdiField     field;
    MdbRow       *row;
    MdbCol       *col;
    char        *path, *npath, *bak, *type;
    MprFile     *out;
    int         cid, rid, tid, ntables, nrows;

    mdb = (Mdb*) edi;
    path = mdb->path;
    if (path == 0) {
        mprError("No database path specified");
        return MPR_ERR_BAD_ARGS;
    }
    npath = mprReplacePathExt(path, "new");
    if ((out = mprOpenFile(npath, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0664)) == 0) {
        return 0;
    }
    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = getTable(mdb, tid);
        schema = table->schema;
        mprAssert(schema);
        mprWriteFileFormat(out, "{\n    '%s': {\n", table->name);
        mprWriteFileFormat(out, "        hints: {\n            ncols: %d\n        },\n", schema->ncols);
        mprWriteFileString(out, "        schema: {\n");
        /* Skip the id which is always the first column */
        for (cid = 0; cid < schema->ncols; cid++) {
            col = getCol(table, cid);
            type = ediGetTypeString(col->type);
            mprWriteFileFormat(out, "            '%s': { type: '%s'", col->name, type);
            if (col->flags & EDI_AUTO_INC) {
                mprWriteFileString(out, ", autoinc: 'true'");
            }
            if (col->flags & EDI_INDEX) {
                mprWriteFileString(out, ", index: 'true'");
            }
            if (col->flags & EDI_KEY) {
                mprWriteFileString(out, ", key: 'true'");
            }
            if (col->flags & EDI_NOT_NULL) {
                mprWriteFileString(out, ", notnull: 'true'");
            }
            mprWriteFileString(out, " },\n");
        }
        mprWriteFileString(out, "        },\n");
        mprWriteFileString(out, "        data: [\n");

        nrows = mprGetListLength(table->rows);
        for (rid = 0; rid < nrows; rid++) {
            mprWriteFileString(out, "            [ ");
            row = getRow(table, rid);
            for (cid = 0; cid < schema->ncols; cid++) {
                if (col->flags & EDI_AUTO_INC) {
                    setAutoInc(row, col);
                }
                field = getField(row, cid);
                //  MOB OPT - inline toString code here
                mprWriteFileFormat(out, "'%s', ", ediToString(NULL, field));
            }
            mprWriteFileString(out, "],\n");
        }
        mprWriteFileString(out, "    ],\n    },\n");
    }
    mprWriteFileString(out, "}\n");
    mprCloseFile(out);

    bak = mprReplacePathExt(path, "bak");
    if (mprDeletePath(bak) < 0) {
        mprError("Can't delete %s", bak);
        return MPR_ERR_CANT_WRITE;
    }
    if (rename(path, bak) < 0) {
        mprError("Can't rename %s to %s", path, bak);
        return MPR_ERR_CANT_WRITE;
    }
    if (rename(npath, path) < 0) {
        mprError("Can't rename %s to %s", npath, path);
        /* Restore backup */
        rename(bak, path);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


/********************************* Table Operations **************************/

static MdbTable *getTable(Mdb *mdb, int tid)
{
    int         ntables;

    ntables = mprGetListLength(mdb->tables);
    if (tid < 0 || tid >= ntables) {
        return 0;
    }
    return mprGetItem(mdb->tables, tid);
}


static MdbTable *lookupTable(Mdb *mdb, cchar *tableName)
{
    MdbTable     *table;
    int         tid, ntables;

    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = mprGetItem(mdb->tables, tid);
        if (smatch(table->name, tableName)) {
            return table;
        }
    }
    return 0;
}


#if UNUSED
static void createID(Mdb *mdb, MdbTable *table)
{
    MdbCol   *col;

    if (mdbAddColumn(mdb, table->name, "id", EDI_TYPE_STRING, EDI_KEY) < 0) {
        mprAssert("Can't create ID");
        return;
    }
    col = lookupColumn(table, "id");
    col->flags |= EDI_KEY | EDI_AUTO_INC | EDI_NOT_NULL;
}
#endif


static void manageTable(MdbTable *table, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(table->name);
        mprMark(table->schema);
        mprMark(table->index);
        mprMark(table->mutex);
        mprMark(table->rows);
#if UNUSED
        nrows = mprGetListLength(table->rows);
        for (rid = 0; rid < nrows; rid++) {
            row = mprGetItem(table->rows, rid);
            manageRow(row, flags);
        }
#endif
    }
}


//  MOB - inconsistent. Should return Row
static int lookupRow(MdbTable *table, cchar *key)
{
    MprKey      *kp;
    MdbRow      *row;
    int         nrows, r, keycol;

    if (table->index == 0) {
        if ((kp = mprLookupKey(table->index, key)) != 0) {
            return (int) PTOL(kp->data);
        } 
    } else {
        nrows = mprGetListLength(table->rows);
        keycol = table->keyCol ? table->keyCol->cid : 0;
        for (r = 0; r < nrows; r++) {
            row = mprGetItem(table->rows, r);
            if (smatch(row->fields[keycol].str, key)) {
                return r;
            }
        }
    }
    return -1;
}


/********************************* Schema / Col ****************************/

MdbSchema *growSchema(MdbTable *table)
{
    if (table->schema == 0) {
        if ((table->schema = mprAllocMem(sizeof(MdbSchema) + 
                sizeof(MdbCol) * MDB_INCR, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
            return 0;
        }
        mprSetAllocName(table->schema, "schema");
        mprSetManager(table->schema, manageSchema);
        table->schema->capacity = MDB_INCR;

    } else if (table->schema->ncols >= table->schema->capacity) {
        if ((table->schema = mprRealloc(table->schema, sizeof(MdbSchema) + 
                (sizeof(MdbCol) * (table->schema->capacity + MDB_INCR)))) == 0) {
            return 0;
        }
        table->schema->capacity += MDB_INCR;
    }
    return table->schema;
}


static MdbCol *createCol(MdbTable *table, cchar *columnName)
{
    MdbSchema    *schema;
    MdbCol       *col;

    if ((col = lookupColumn(table, columnName)) != 0) {
        return 0;
    }
    if ((schema = growSchema(table)) == 0) {
        return 0;
    }
    col = &schema->cols[schema->ncols];
    col->cid = schema->ncols++;
    col->name = sclone(columnName);
    return col;
}


static void manageSchema(MdbSchema *schema, int flags) 
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        for (i = 0; i < schema->ncols; i++) {
            manageCol(&schema->cols[i], flags);
        }
    }
}


static void manageCol(MdbCol *col, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(col->name);
    }
}


static MdbCol *getCol(MdbTable *table, int col)
{
    if (col < 0 || col >= table->schema->ncols) {
        return 0;
    }
    return &table->schema->cols[col];
}


static MdbCol *lookupColumn(MdbTable *table, cchar *columnName)
{
    MdbSchema    *schema;
    MdbCol       *col;

    if ((schema = growSchema(table)) == 0) {
        return 0;
    }
    for (col = schema->cols; col < &schema->cols[schema->ncols]; col++) {
        if (smatch(col->name, columnName)) {
            return col;
        }
    }
    return 0;
}


/********************************* Row Operations **************************/

static MdbRow *createRow(Mdb *mdb, MdbTable *table)
{
    MdbRow       *row;
    MdbCol       *col;
    int         ncols;

    ncols = table->schema->ncols;
    ncols = max(ncols, 1);
    if ((row = mprAllocMem(sizeof(MdbRow) + sizeof(EdiField) * ncols, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetAllocName(row, "row");
    mprSetManager(row, manageRow);
    row->table = table;
    row->nfields = ncols;
    col = getCol(table, 0);
    row->rid = mprAddItem(table->rows, row);
    return row;
}


static void manageRow(MdbRow *row, int flags)
{
    MdbSchema   *schema;
    int         fid, type;

    if (flags & MPR_MANAGE_MARK) {
        schema = row->table->schema;
        mprAssert(schema);
        //  MOB - OPT - may be better to iterate over cols then rows. ie. only select string columns.
        for (fid = 0; fid < row->nfields; fid++) {
            type = schema->cols[fid].type;
            if (type == EDI_TYPE_STRING || type == EDI_TYPE_TEXT) {
                mprMark(row->fields[fid].str);
            }
        }
    }
}


static MdbRow *getRow(MdbTable *table, int rid)
{
    int     nrows;

    nrows = mprGetListLength(table->rows);
    if (rid < 0 || rid > nrows) {
        return 0;
    }
    return mprGetItem(table->rows, rid);
}

/********************************* Field Operations ************************/

static EdiField getField(MdbRow *row, int fid)
{
    EdiField    field;
    MdbCol      *col;
    mprAssert(0 <= fid  && fid < row->nfields);

    col = &row->table->schema->cols[fid];
    field.value = row->fields[fid];
    field.type = col->type;
    field.flags = col->flags;
    field.name = col->name;
    return field;
}


static void setAutoInc(MdbRow *row, MdbCol *col)
{
    EdiValue    *vp;

    vp = &row->fields[col->cid];
    if (col->type == EDI_TYPE_STRING && vp->str == 0) {
        //  MOB OPT replace with stoi
        vp->str = sfmt("%d", col->nextValue++);
    } else if (col->type == EDI_TYPE_INT && vp->inum == 0) {
        vp->inum = col->nextValue++;
    } else if (col->type == EDI_TYPE_FLOAT && vp->num == 0) {
        vp->num = col->nextValue++;
    }
}


static int setField(MdbRow *row, MdbCol *col, cchar *value)
{
    MdbTable    *table;
    EdiValue    *vp;
    
    mprAssert(row);
    mprAssert(col);

    table = row->table;
    vp = &row->fields[col->cid];

    if (col->flags & EDI_AUTO_INC) {
        if (value == 0) {
            setAutoInc(row, col);
        } else {
            *vp = ediParseValue(value, col->type);
            col->nextValue = max(col->nextValue, (int) stoi(value, 10, NULL) + 1);
        }
    } else {
        *vp = ediParseValue(value, col->type);
    }
    return 0;
}


#if UNUSED
int64 mdbGetIntField(MdbRow *row, int fid)
{
    mprAssert(0 <= fid  && fid < row->nfields);
    return row->fields[fid].inum;
}


double mdbGetFloatField(MdbRow *row, int fid)
{
    mprAssert(0 <= fid  && fid < row->nfields);
    return row->fields[fid].num;
}


MprTime mdbGetDateField(MdbRow *row, int fid)
{
    mprAssert(0 <= fid  && fid < row->nfields);
    return row->fields[fid].date;
}


//  MOB - reconsider all these
cchar *edGetStringField(MdbRow *row, int fid)
{
    mprAssert(0 <= fid  && fid < row->nfields);
    return row->fields[fid].str;
}


int mdbSetIntField(MdbRow *row, int fid, int64 value)
{
    EdiField     f;

    mprAssert(0 <= fid  && fid < row->nfields);
    f.value.inum = value;
    row->fields[fid] = f;
    return 0;
}


int mdbSetFloatField(MdbRow *row, int fid, double value)
{
    EdiField     f;

    mprAssert(0 <= fid  && fid < row->nfields);
    f.value.num = value;
    row->fields[fid] = f;
    return 0;
}


int mdbSetDateField(MdbRow *row, int fid, MprTime value)
{
    EdiField     f;

    mprAssert(0 <= fid  && fid < row->nfields);
    f.value.date = value;
    row->fields[fid] = f;
    return 0;
}


int mdbSetStringField(MdbRow *row, int fid, cchar *value)
{
    EdiField     f;

    mprAssert(0 <= fid  && fid < row->nfields);
    f.value.str = value;
    row->fields[fid] = f;
    return 0;
}
#endif


/*********************************** Support *******************************/
/*
    Optimized record creation
 */
static EdiRec *createRec(Edi *edi, MdbRow *row)
{
    EdiRec  *rec;
    MdbCol  *col;
    int     c;

    if ((rec = mprAllocMem(sizeof(EdiRec) + sizeof(EdiField) * row->nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetAllocName(rec, "record");
    mprSetManager(rec, ediManageEdiRec);

    rec->edi = edi;
    rec->tableName = row->table->name;
    //  MOB - assert and check there is a valid ID
    rec->id = row->fields[0].str;
    rec->nfields = row->nfields;
    for (c = 0; c < row->nfields; c++) {
        col = getCol(row->table, c);
        rec->fields[c] = makeFieldFromRow(row, col);
    }
#if UNUSED
    memmove(rec->fields, row->fields, sizeof(EdiField) * row->nfields);
#endif
    return rec;
}


static cchar *eatSpace(cchar *tok)
{
    while (isspace((int) *tok) && *tok != '\n') {
        tok++;
    }
    return tok;
}


static cchar *findQuote(cchar *tok, int quote)
{
    cchar   *cp;

    mprAssert(tok);
    for (cp = tok; *cp; cp++) {
        if (*cp == quote && (cp == tok || *cp != '\\')) {
            return cp;
        }
    }
    return 0;
}


static cchar *findEndKeyword(Mdb *mdb, cchar *str)
{
    cchar   *cp, *etok;

    mprAssert(str);
    for (cp = mdb->tok; *cp; cp++) {
        if ((etok = strpbrk(cp, " \t\n\r:,")) != 0) {
            if (etok == mdb->tok || *etok != '\\') {
                return etok;
            }
        }
    }
    return &str[strlen(str)];
}


static void parseError(Mdb *mdb, cchar *fmt, ...)
{
    va_list     args;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    mprError("%s\nIn file '%s' at line %d", msg, mdb->path, mdb->lineNumber);
    va_end(args);
}


static int parseType(cchar *type)
{
    if (smatch(type, "blob")) {
        return EDI_TYPE_BLOB;
    } else if (smatch(type, "bool")) {
        return EDI_TYPE_BOOL;
    } else if (smatch(type, "date")) {
        return EDI_TYPE_DATE;
    } else if (smatch(type, "float")) {
        return EDI_TYPE_FLOAT;
    } else if (smatch(type, "int")) {
        return EDI_TYPE_INT;
    } else if (smatch(type, "string")) {
        return EDI_TYPE_STRING;
    } else if (smatch(type, "text")) {
        return EDI_TYPE_TEXT;
    }
    return 0;
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
