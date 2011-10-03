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
static EdiRec *createRecFromRow(Edi *edi, MdbRow *row);
static MdbRow *createRow(Mdb *mdb, MdbTable *table);
static cchar *getKey(EdiValue *vp, int type);
static MdbCol *getCol(MdbTable *table, int col);
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
static int parseType(cchar *type);
static EdiField readField(MdbRow *row, int fid);
static void autoIncField(MdbRow *row, MdbCol *col);
static int writeField(MdbRow *row, MdbCol *col, cchar *value);
static int writeFieldFromField(MdbRow *row, MdbCol *col, EdiField *field);

static int mdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static int mdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
static int mdbAddTable(Edi *edi, cchar *tableName);
static int mdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static void mdbClose(Edi *edi);
static EdiRec *mdbCreateRec(Edi *edi, cchar *tableName);
static int mdbDelete(Edi *edi, cchar *path);
static int mdbDeleteRow(Edi *edi, cchar *tableName, cchar *key);
static MprList *mdbGetColumns(Edi *edi, cchar *tableName);
static int mdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid);
static MprList *mdbGetTables(Edi *edi);
static int mdbLoad(Edi *edi, cchar *path);
static int mdbLoadFromString(Edi *edi, cchar *string);
static int mdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName);
static Edi *mdbOpen(cchar *path, int flags);
static EdiGrid *mdbQuery(Edi *edi, cchar *cmd);
static EdiField mdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
static EdiGrid *mdbReadGrid(Edi *edi, cchar *tableName);
static EdiRec *mdbReadRec(Edi *edi, cchar *tableName, cchar *key);
static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);
static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);
static int mdbRemoveTable(Edi *edi, cchar *tableName);
static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);
static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
static int mdbSave(Edi *edi);
static int mdbWriteField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
static int mdbWriteFields(Edi *edi, cchar *tableName, MprHash *params);
static int mdbWriteRec(Edi *edi, EdiRec *rec);

EdiProvider MdbProvider = {
    "mdb",
    mdbAddColumn, mdbAddIndex, mdbAddTable, mdbChangeColumn, mdbClose, mdbCreateRec, mdbDelete, mdbDeleteRow, 
    mdbGetColumns, mdbGetColumnSchema, mdbGetTables, mdbLoad, mdbLookupField, mdbOpen, mdbQuery, mdbReadField, 
    mdbReadGrid, mdbReadRec, mdbRemoveColumn, mdbRemoveIndex, mdbRemoveTable, mdbRenameTable, mdbRenameColumn, 
    mdbSave, mdbWriteField, mdbWriteFields, mdbWriteRec,
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


/*
    Create a free-standing record. Not saved to the database.
 */
static EdiRec *mdbCreateRec(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;
    EdiRec      *rec;
    EdiField    *fp;
    int         f, nfields;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return 0;
    }
    nfields = max(table->schema->ncols, 1);
    if ((rec = mprAllocMem(sizeof(EdiRec) + sizeof(EdiField) * nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetAllocName(rec, "record");
    mprSetManager(rec, ediManageEdiRec);

    rec->edi = edi;
    rec->tableName = table->name;
    rec->nfields = nfields;

    for (f = 0; f < nfields; f++) {
        col = getCol(table, f);
        fp = &rec->fields[f];
        fp->type = col->type;
        fp->name = col->name;
        fp->flags = col->flags;
    }
    return rec;
}


static int mdbDelete(Edi *edi, cchar *path)
{
    return mprDeletePath(path);
}


static Edi *mdbOpen(cchar *source, int flags)
{
    Mdb         *mdb;

    if (flags & EDI_LITERAL) {
        flags |= EDI_NO_SAVE;
        if ((mdb = mdbCreate("literal", flags)) == 0) {
            return 0;
        }
        if (mdbLoadFromString((Edi*) mdb, source) < 0) {
            return 0;
        }

    } else {
        if (!mprPathExists(source, R_OK) && !(flags & EDI_CREATE)) {
            return 0;
        }
        if ((mdb = mdbCreate(source, flags)) == 0) {
            return 0;
        }
        if (mdbLoad((Edi*) mdb, source) < 0) {
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
        /* No need to clone */
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


static int mdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid)
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
    if (cid) {
        *cid = col->cid;
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


static EdiField mdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
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


static EdiGrid *mdbReadGrid(Edi *edi, cchar *tableName)
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
    if ((grid = ediCreateBareGrid(edi, tableName, nrows)) == 0) {
        return 0;
    }
    for (ITERATE_ITEMS(table->rows, row, next)) {
        grid->records[next - 1] = createRecFromRow(edi, row);
    }
    return grid;
}


static EdiRec *mdbReadRec(Edi *edi, cchar *tableName, cchar *key)
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
    if ((rec = createRecFromRow(edi, row)) == 0) {
        return 0;
    }
    return rec;
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
    if (table->keyCol == col) {
        table->keyCol = 0;
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


//  MOB - check sorting
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


static int mdbWriteField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
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
    if ((col = lookupColumn(table, fieldName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    writeField(row, col, kp->data);
    return 0;
}


static int mdbWriteFields(Edi *edi, cchar *tableName, MprHash *params)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    MprKey      *kp;
    cchar       *key;
    int         r, setID;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    mprAssert(table->keyCol);

    if ((key = mprLookupKey(params, table->keyCol->name)) == 0) {
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
            writeField(row, col, kp->data);
            if (col->cid == 0) {
                setID++;
            }
        }
    }
    if (!setID) {
        //  MOB OPT - change to itos when it allocates
        writeField(row, getCol(table, 0), 0);
    }
    autoSave(mdb, table);
    return 0;
}


static int mdbWriteRec(Edi *edi, EdiRec *rec)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    int         f, r;

    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, rec->tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if (rec->id == 0 || (r = lookupRow(table, rec->id)) < 0) {
        row = createRow(mdb, table);
    } else {
        if ((row = getRow(table, r)) == 0) {
            return MPR_ERR_CANT_FIND;
        }
    }
    for (f = 0; f < rec->nfields; f++) {
        if ((col = getCol(table, f)) != 0) {
            writeFieldFromField(row, col, &rec->fields[f]);
        }
    }
    autoSave(mdb, table);
    return 0;
}


static cchar *getKey(EdiValue *vp, int type)
{
    switch (type) {
    case EDI_TYPE_BLOB:
        return vp->blob;

    case EDI_TYPE_BOOL:
        //  MOB OPT - should be mpr cloned string
        return vp->boolean ? sclone("true") : sclone("false");

    case EDI_TYPE_DATE:
        return mprFormatLocalTime(NULL, vp->date);

    case EDI_TYPE_FLOAT:
        return sfmt("%f", vp->num);

    case EDI_TYPE_INT:
        return sfmt("%Ld", vp->inum);

    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
        return vp->str;

    default:
        mprError("Unknown field type %d", type);
    }
    return 0;
}


/******************************** Database Loading ***************************/

static void clearLoadState(Mdb *mdb)
{
    mdb->loadNcols = 0;
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


static MprObj *makeMdbObj(MprJson *jp, bool list)
{
    /* Dummy object creation */
    return (MprObj*) jp;
}


static int checkMdbState(MprJson *jp, cchar *name)
{
    Mdb     *mdb;

    mdb = jp->data;
    if (*jp->tok == ']' || *jp->tok == '}') {
        popState(mdb);
        return 0;
    }
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
            mprJsonParseError(jp, "Bad property '%s'", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;
    
    case MDB_LOAD_SCHEMA:
        if ((mdb->loadCol = createCol(mdb->loadTable, name)) == 0) {
            mprJsonParseError(jp, "Can't create '%s' column", name);
            return MPR_ERR_MEMORY;
        }
        pushState(mdb, MDB_LOAD_COL);
        break;

    case MDB_LOAD_DATA:
        if ((mdb->loadRow = createRow(mdb, mdb->loadTable)) == 0) {
            return MPR_ERR_MEMORY;
        }
        pushState(mdb, MDB_LOAD_FIELD);
        break;

    case MDB_LOAD_HINTS:
    case MDB_LOAD_COL:
    case MDB_LOAD_FIELD:
        break;

    default:
        mprJsonParseError(jp, "Potential corrupt data. Bad state '%d'");
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static int setMdbValue(MprJson *jp, MprObj *obj, int cid, cchar *name, cchar *value, int type)
{
    Mdb         *mdb;
    MdbCol      *col;

    mdb = jp->data;
    switch (mdb->loadState) {
    case MDB_LOAD_BEGIN:
    case MDB_LOAD_TABLE:
    case MDB_LOAD_SCHEMA:
    case MDB_LOAD_DATA:
        break;

    case MDB_LOAD_HINTS:
        if (smatch(name, "ncols")) {
            mdb->loadNcols = atoi(value);
        } else {
            mprJsonParseError(jp, "Unknown hint '%s'", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_COL:
        if (smatch(name, "index")) {
            mdbAddIndex((Edi*) mdb, mdb->loadTable->name, mdb->loadCol->name, NULL);
        } else if (smatch(name, "type")) {
            if ((mdb->loadCol->type = parseType(value)) <= 0) {
                mprJsonParseError(jp, "Bad column type %s", value);
                return MPR_ERR_BAD_FORMAT;
            }
        } else if (smatch(name, "key")) {
            mdb->loadCol->flags |= EDI_KEY;
            mdb->loadTable->keyCol = mdb->loadCol;
        } else if (smatch(name, "autoinc")) {
            mdb->loadCol->flags |= EDI_AUTO_INC;
#if FUTURE && KEEP
        } else if (smatch(name, "notnull")) {
            mdb->loadCol->flags |= EDI_NOT_NULL;
#endif
        } else {
            mprJsonParseError(jp, "Bad property '%s' in column definition", name);
            return MPR_ERR_BAD_FORMAT;
        }
        break;

    case MDB_LOAD_FIELD:
        if (cid < 0) {
            mprJsonParseError(jp, "Bad state '%d' in setMdbValue, cid %d,  potential corrupt data", mdb->loadState, cid);
            return MPR_ERR_BAD_FORMAT;
        }
        col = getCol(mdb->loadTable, cid);
        mprAssert(col);
        if (col) {
            writeField(mdb->loadRow, col, value);
        }
        break;

    default:
        mprJsonParseError(jp, "Bad state '%d' in setMdbValue potential corrupt data", mdb->loadState);
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static int mdbLoadFromString(Edi *edi, cchar *str)
{
    Mdb         *mdb;
    MprObj      *obj;

    mdb = (Mdb*) edi;
    mdb->edi.flags |= EDI_SUPPRESS_SAVE;
    mdb->flags |= MDB_LOADING;
    mdb->loadStack = mprCreateList(0, 0);
    pushState(mdb, MDB_LOAD_BEGIN);
    obj = mprDeserializeCustom(str, makeMdbObj, checkMdbState, setMdbValue, mdb);
    mdb->flags &= ~MDB_LOADING;
    mdb->edi.flags &= ~EDI_SUPPRESS_SAVE;
    mdb->loadStack = 0;
    if (obj == 0) {
        return MPR_ERR_CANT_LOAD;
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
    if (mdb->flags & EDI_NO_SAVE) {
        return MPR_ERR_BAD_STATE;
    }
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
                    autoIncField(row, col);
                }
                field = readField(row, cid);
                //  MOB OPT - inline toString code here
                mprWriteFileFormat(out, "'%s', ", ediFormatField(NULL, field));
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

    if (table->index) {
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
    MdbRow      *row;
    MdbCol      *col;
    int         ncols;

    ncols = max(table->schema->ncols, 1);
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

static EdiField readField(MdbRow *row, int fid)
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


static void autoIncField(MdbRow *row, MdbCol *col)
{
    EdiValue    *vp;

    //  MOB - auto inc should only be supported for INT fields
    vp = &row->fields[col->cid];
    if (col->type == EDI_TYPE_STRING && vp->str == 0) {
        vp->str = itos(col->nextValue++, 10);
    } else if (col->type == EDI_TYPE_INT && vp->inum == 0) {
        vp->inum = col->nextValue++;
    } else if (col->type == EDI_TYPE_FLOAT && vp->num == 0) {
        vp->num = col->nextValue++;
    }
}


static int writeField(MdbRow *row, MdbCol *col, cchar *value)
{
    MdbTable    *table;
    EdiValue    *vp;
    cchar       *key;
    
    mprAssert(row);
    mprAssert(col);

    table = row->table;
    if (col->flags & EDI_INDEX) {
        key = getKey(&row->fields[col->cid], col->type);
        mprRemoveKey(table->index, key);
    } else {
        key = 0;
    }
    vp = &row->fields[col->cid];
    if (col->flags & EDI_AUTO_INC) {
        if (value == 0) {
            autoIncField(row, col);
        } else {
            *vp = ediParseValue(value, col->type);
            col->nextValue = max(col->nextValue, (int64) stoi(value, 10, NULL) + 1);
        }
    } else {
        *vp = ediParseValue(value, col->type);
    }
    if (col->flags & EDI_INDEX) {
        mprAddKey(table->index, key, LTOP(row->rid));
    }
    return 0;
}


static int writeFieldFromField(MdbRow *row, MdbCol *col, EdiField *field)
{
    MdbTable    *table;
    EdiValue    *vp;
    
    mprAssert(row);
    mprAssert(col);

    table = row->table;
    vp = &row->fields[col->cid];

    if (col->flags & EDI_AUTO_INC) {
        if (!field->valid) {
            autoIncField(row, col);
        } else {
            *vp = field->value;
            if (col->type == EDI_TYPE_STRING) {
                col->nextValue = max(col->nextValue, (int64) stoi(field->value.str, 10, NULL) + 1);
            } else if (col->type == EDI_TYPE_INT) {
                col->nextValue = max(col->nextValue, (int64) field->value.inum + 1);
            } else if (col->type == EDI_TYPE_FLOAT) {
                col->nextValue = max(col->nextValue, (int64) field->value.num + 1);
            }
        }
    } else {
        *vp = field->value;
    }
    if (col->type == EDI_TYPE_STRING || col->type == EDI_TYPE_TEXT) {
        vp->str = sclone(vp->str);
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
static EdiRec *createRecFromRow(Edi *edi, MdbRow *row)
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
    return rec;
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
