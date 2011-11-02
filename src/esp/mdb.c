/*
    mdb.c -- In-Memory Embedded Database (MDB)

    WARNING: This is prototype code

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"
#include    "edi.h"
#include    "mdb.h"
#include    "pcre.h"

#if BLD_FEATURE_ESP && BLD_FEATURE_MDB
/************************************* Local **********************************/

#define MDB_LOAD_BEGIN   1      /* Initial state */
#define MDB_LOAD_TABLE   2      /* Parsing a table */
#define MDB_LOAD_HINTS   3      /* Parsing hints */
#define MDB_LOAD_SCHEMA  4      /* Parsing schema */
#define MDB_LOAD_COL     5      /* Parsing column schema */ 
#define MDB_LOAD_DATA    6      /* Parsing data */
#define MDB_LOAD_FIELD   7      /* Parsing fields */

/*
    Operations for mdbReadWhere
 */
#define OP_ERR  -1              /* Illegal operation */
#define OP_EQ   0               /* "==" Equal operation */
#define OP_NEQ  0x2             /* "!=" Not equal operation */
#define OP_LT   0x4             /* "<" Less than operation */
#define OP_GT   0x8             /* ">" Greater than operation */
#define OP_LTE  0x10            /* ">=" Less than or equal operation */
#define OP_GTE  0x20            /* "<=" Greater than or equal operation */

/************************************ Forwards ********************************/

static void autoSave(Mdb *mdb, MdbTable *table);
static MdbCol *createCol(MdbTable *table, cchar *columnName);
static EdiRec *createRecFromRow(Edi *edi, MdbRow *row);
static MdbRow *createRow(Mdb *mdb, MdbTable *table);
static MdbCol *getCol(MdbTable *table, int col);
static MdbRow *getRow(MdbTable *table, int rid);
static MdbTable *getTable(Mdb *mdb, int tid);
static MdbSchema *growSchema(MdbTable *table);
static MdbCol *lookupColumn(MdbTable *table, cchar *columnName);
static int lookupRow(MdbTable *table, cchar *key);
static MdbTable *lookupTable(Mdb *mdb, cchar *tableName);
static void manageCol(MdbCol *col, int flags);
static void manageMdb(Mdb *mdb, int flags);
static void manageRow(MdbRow *row, int flags);
static void manageSchema(MdbSchema *schema, int flags);
static void manageTable(MdbTable *table, int flags);
static int parseOperation(cchar *operation);
static int updateField(MdbRow *row, MdbCol *col, cchar *value);
static bool validateField(EdiRec *rec, MdbTable *table, MdbCol *col, cchar *value);

static int mdbAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static int mdbAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
static int mdbAddTable(Edi *edi, cchar *tableName);
static int mdbAddValidation(Edi *edi, cchar *tableName, cchar *fieldName, EdiValidation *vp);
static int mdbChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
static void mdbClose(Edi *edi);
static EdiRec *mdbCreateRec(Edi *edi, cchar *tableName);
static int mdbDelete(cchar *path);
static int mdbDeleteRow(Edi *edi, cchar *tableName, cchar *key);
static MprList *mdbGetColumns(Edi *edi, cchar *tableName);
static int mdbGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid);
static MprList *mdbGetTables(Edi *edi);
static int mdbGetTableSchema(Edi *edi, cchar *tableName, int *numRows, int *numCols);
static int mdbLoad(Edi *edi, cchar *path);
static int mdbLoadFromString(Edi *edi, cchar *string);
static int mdbLookupField(Edi *edi, cchar *tableName, cchar *fieldName);
static Edi *mdbOpen(cchar *path, int flags);
static EdiGrid *mdbQuery(Edi *edi, cchar *cmd);
static EdiField mdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
static EdiRec *mdbReadRec(Edi *edi, cchar *tableName, cchar *key);
static EdiGrid *mdbReadWhere(Edi *edi, cchar *tableName, cchar *columnName, cchar *operation, cchar *value);
static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);
static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);
static int mdbRemoveTable(Edi *edi, cchar *tableName);
static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);
static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
static int mdbSave(Edi *edi);
static int mdbUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
static int mdbUpdateRec(Edi *edi, EdiRec *rec);
static bool mdbValidateRec(Edi *edi, EdiRec *rec);

static EdiProvider MdbProvider = {
    "mdb",
    mdbAddColumn, mdbAddIndex, mdbAddTable, mdbAddValidation, mdbChangeColumn, mdbClose, mdbCreateRec, mdbDelete, 
    mdbDeleteRow, mdbGetColumns, mdbGetColumnSchema, mdbGetTables, mdbGetTableSchema, mdbLoad, mdbLookupField, 
    mdbOpen, mdbQuery, mdbReadField, mdbReadRec, mdbReadWhere, mdbRemoveColumn, mdbRemoveIndex, mdbRemoveTable, 
    mdbRenameTable, mdbRenameColumn, mdbSave, mdbUpdateField, mdbUpdateRec, mdbValidateRec, 
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


static int mdbDelete(cchar *path)
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) != 0) {
        unlock(mdb);
        return MPR_ERR_ALREADY_EXISTS;
    }
    if ((col = createCol(table, columnName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    col->type = type;
    col->flags = flags;
    autoSave(mdb, table);
    unlock(mdb);
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((table->index = mprCreateHash(0, MPR_HASH_STATIC_VALUES)) == 0) {
        unlock(mdb);
        return MPR_ERR_MEMORY;
    }
    table->indexCol = col;
    col->flags |= EDI_INDEX;
    autoSave(mdb, table);
    unlock(mdb);
    return 0;
}


static int mdbAddTable(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mprAssert(edi);
    mprAssert(tableName && *tableName);

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) != 0) {
        unlock(mdb);
        return MPR_ERR_ALREADY_EXISTS;
    }
    if ((table = mprAllocObj(MdbTable, manageTable)) == 0) {
        unlock(mdb);
        return MPR_ERR_MEMORY;
    }
    if ((table->rows = mprCreateList(0, 0)) == 0) {
        unlock(mdb);
        return MPR_ERR_MEMORY;
    }
    table->name = sclone(tableName);
    if (mdb->tables == 0) {
        mdb->tables = mprCreateList(0, 0);
    }
    if (!growSchema(table)) {
        unlock(mdb);
        return MPR_ERR_MEMORY;
    }
    mprAddItem(mdb->tables, table);
    autoSave(mdb, lookupTable(mdb, tableName));
    unlock(mdb);
    return 0;
}


static int mdbAddValidation(Edi *edi, cchar *tableName, cchar *columnName, EdiValidation *vp)
{
    Mdb             *mdb;
    MdbTable        *table;
    MdbCol          *col;

    mprAssert(edi);
    mprAssert(tableName && *tableName);
    mprAssert(columnName && *columnName);

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if (col->validations == 0) {
        col->validations = mprCreateList(0, 0);
    }
    mprAddItem(col->validations, vp);
    unlock(mdb);
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    col->name = sclone(columnName);
    col->type = type;
    autoSave(mdb, table);
    unlock(mdb);
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((r = lookupRow(table, key)) < 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    rc = mprRemoveItemAtPos(table->rows, r);
    if (table->index) {
        mprRemoveKey(table->index, key);
    }
    autoSave(mdb, table);
    unlock(mdb);
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return 0;
    }
    schema = table->schema;
    mprAssert(schema);
    list = mprCreateList(schema->ncols, 0);
    for (i = 0; i < schema->ncols; i++) {
        /* No need to clone */
        mprAddItem(list, schema->cols[i].name);
    }
    unlock(mdb);
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        unlock(mdb);
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
    unlock(mdb);
    return 0;
}


static MprList *mdbGetTables(Edi *edi)
{
    Mdb         *mdb;
    MprList     *list;
    MdbTable     *table;
    int         tid, ntables;

    mdb = (Mdb*) edi;
    lock(mdb);
    list = mprCreateList(-1, 0);
    ntables = mprGetListLength(mdb->tables);
    for (tid = 0; tid < ntables; tid++) {
        table = mprGetItem(mdb->tables, tid);
        mprAddItem(list, table->name);
    }
    unlock(mdb);
    return list;
}


static int mdbGetTableSchema(Edi *edi, cchar *tableName, int *numRows, int *numCols)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if (numRows) {
        *numRows = mprGetListLength(table->rows);
    }
    if (numCols) {
        *numCols = table->schema->ncols;
    }
    unlock(mdb);
    return 0;
}


static int mdbLoad(Edi *edi, cchar *path)
{
    cchar       *data;
    ssize       len;

    if ((data = mprReadPathContents(path, &len)) == 0) {
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
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, fieldName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    unlock(mdb);
    return col->cid;
}


static EdiGrid *mdbQuery(Edi *edi, cchar *cmd)
{
    //  TODO MOB
    mprAssert(0);
    return 0;
}


static EdiField mdbReadField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;
    MdbRow      *row;
    EdiField    field, err;
    int         r;

    mdb = (Mdb*) edi;
    lock(mdb);
    err.valid = 0;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return err;
    }
    if ((col = lookupColumn(table, fieldName)) == 0) {
        unlock(mdb);
        return err;
    }
    if ((r = lookupRow(table, key)) < 0) {
        unlock(mdb);
        return err;
    }
    row = mprGetItem(table->rows, r);
    field = makeFieldFromRow(row, col);
    unlock(mdb);
    return field;
}


static EdiRec *mdbReadRec(Edi *edi, cchar *tableName, cchar *key)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    EdiRec      *rec;
    int         r;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return 0;
    }
    if ((r = lookupRow(table, key)) < 0) {
        unlock(mdb);
        return 0;
    }
    row = mprGetItem(table->rows, r);
    rec = createRecFromRow(edi, row);
    unlock(mdb);
    return rec;
}



static bool matchRow(MdbCol *col, cchar *existing, int op, cchar *value)
{
    if (value == 0 || *value == '\0') {
        return 0;
    }
    //  MOB - must ensure database NEVER has a null
    switch (op) {
    case OP_EQ:
        if (smatch(existing, value)) {
            return 1;
        }
        break;
    case OP_NEQ:
        if (!smatch(existing, value)) {
            return 1;
        }
        break;
#if MOB && TODO
    case OP_LT:
    case OP_GT:
    case OP_LTE:
    case OP_GTE:
#endif
    default:
        mprAssert(0);
    }
    return 0;
}


static EdiGrid *mdbReadWhere(Edi *edi, cchar *tableName, cchar *columnName, cchar *operation, cchar *value)
{
    Mdb         *mdb;
    EdiGrid     *grid;
    MdbTable    *table;
    MdbCol      *col;
    MdbRow      *row;
    int         nrows, next, op, r;

    mprAssert(edi);
    mprAssert(tableName && *tableName);

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return 0;
    }
    nrows = mprGetListLength(table->rows);
    if ((grid = ediCreateBareGrid(edi, tableName, nrows)) == 0) {
        unlock(mdb);
        return 0;
    }
    if (columnName) {
        if ((col = lookupColumn(table, columnName)) == 0) {
            unlock(mdb);
            return 0;
        }
        if ((op = parseOperation(operation)) < 0) {
            unlock(mdb);
            return 0;
        }
        if (col->flags & EDI_INDEX && (op & OP_EQ)) {
            if ((r = lookupRow(table, value)) != 0) {
                row = getRow(table, r);
                grid->records[0] = createRecFromRow(edi, row);
            }
        } else {
            for (ITERATE_ITEMS(table->rows, row, next)) {
                if (!matchRow(col, row->fields[col->cid], op, value)) {
                    continue;
                }
                grid->records[next - 1] = createRecFromRow(edi, row);
            }
        }
    } else {
        for (ITERATE_ITEMS(table->rows, row, next)) {
            grid->records[next - 1] = createRecFromRow(edi, row);
        }
    }
    unlock(mdb);
    return grid;
}


static int mdbRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbSchema   *schema;
    MdbCol      *col;
    int         c;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        unlock(mdb);
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
    unlock(mdb);
    return 0;
}


//  MOB - indexName is ignored
static int mdbRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    table->index = 0;
    if (table->indexCol) {
        table->indexCol->flags &= ~EDI_INDEX;
        table->indexCol = 0;
        autoSave(mdb, table);
    }
    unlock(mdb);
    return 0;
}


static int mdbRemoveTable(Edi *edi, cchar *tableName)
{
    Mdb         *mdb;
    MdbTable    *table;
    int         next;

    mdb = (Mdb*) edi;
    lock(mdb);
    for (ITERATE_ITEMS(mdb->tables, table, next)) {
        if (smatch(table->name, tableName)) {
            mprRemoveItem(mdb->tables, table);
            autoSave(mdb, table);
            unlock(mdb);
            return 0;
        }
    }
    unlock(mdb);
    return MPR_ERR_CANT_FIND;
}


static int mdbRenameTable(Edi *edi, cchar *tableName, cchar *newTableName)
{
    Mdb         *mdb;
    MdbTable    *table;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    table->name = sclone(newTableName);
    autoSave(mdb, table);
    unlock(mdb);
    return 0;
}


static int mdbRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, columnName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    col->name = sclone(newColumnName);
    autoSave(mdb, table);
    unlock(mdb);
    return 0;
}


bool mdbValidateRec(Edi *edi, EdiRec *rec)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbCol      *col;
    bool        pass;
    int         c;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, rec->tableName)) == 0) {
        unlock(mdb);
        return 0;
    }
    pass = 1;
    for (c = 0; c < rec->nfields; c++) {
        col = getCol(table, c);
        if (!col->validations) {
            continue;
        }
        if (!validateField(rec, table, col, rec->fields[c].value)) {
            pass = 0;
            /* Keep going */
        }
    }
    unlock(mdb);
    return pass;
}


static int mdbUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    int         r;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((col = lookupColumn(table, fieldName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if ((r = lookupRow(table, key)) < 0) {
        row = createRow(mdb, table);
    }
    if ((row = getRow(table, r)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    updateField(row, col, value);
    unlock(mdb);
    return 0;
}


#if UNUSED
static int mdbUpdateFields(Edi *edi, cchar *tableName, MprHash *params)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    MprKey      *kp;
    cchar       *key;
    int         r, setID;

    -- MOB - no locking
    mdb = (Mdb*) edi;
    if ((table = lookupTable(mdb, tableName)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    mprAssert(table->keyCol);

    if ((key = mprLookupKey(params, table->keyCol->name)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    setID = 0;
    for (ITERATE_KEYS(params, kp)) {
        if ((col = lookupColumn(table, kp->key)) != 0) {
            if (col->validations && validateField(edi, table, col, kp->data, NULL) != 0) {
                return MPR_ERR_CANT_WRITE;
            }
        }
    }
    if (key == 0 || (r = lookupRow(table, key)) < 0) {
        row = createRow(mdb, table);
    } else {
        if ((row = getRow(table, r)) == 0) {
            return MPR_ERR_CANT_FIND;
        }
    }
    for (ITERATE_KEYS(params, kp)) {
        if ((col = lookupColumn(table, kp->key)) != 0) {
            updateField(row, col, kp->data);
            if (col->cid == 0) {
                setID++;
            }
        }
    }
    if (!setID) {
        updateField(row, getCol(table, 0), 0);
    }
    autoSave(mdb, table);
    return 0;
}
#endif


static int mdbUpdateRec(Edi *edi, EdiRec *rec)
{
    Mdb         *mdb;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    int         f, r;

    mdb = (Mdb*) edi;
    lock(mdb);
    if ((table = lookupTable(mdb, rec->tableName)) == 0) {
        unlock(mdb);
        return MPR_ERR_CANT_FIND;
    }
    if (!mdbValidateRec(edi, rec)) {
        unlock(mdb);
        return MPR_ERR_CANT_WRITE;
    }
    if (rec->id == 0 || (r = lookupRow(table, rec->id)) < 0) {
        row = createRow(mdb, table);
    } else {
        if ((row = getRow(table, r)) == 0) {
            unlock(mdb);
            return MPR_ERR_CANT_FIND;
        }
    }
    for (f = 0; f < rec->nfields; f++) {
        if ((col = getCol(table, f)) != 0) {
            updateField(row, col, rec->fields[f].value);
        }
    }
    autoSave(mdb, table);
    unlock(mdb);
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
            if ((mdb->loadCol->type = ediParseTypeString(value)) <= 0) {
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
            updateField(mdb->loadRow, col, value);
        }
        break;

    default:
        mprJsonParseError(jp, "Bad state '%d' in setMdbValue potential corrupt data", mdb->loadState);
        return MPR_ERR_BAD_FORMAT;
    }
    return 0;
}


static void parseMdbError(MprJson *jp, cchar *msg)
{
    if (jp->path) {
        mprError("%s\nIn file '%s' at line %d", msg, jp->path, jp->lineNumber);
    } else {
        mprError("%s\nAt line %d", msg, jp->lineNumber);
    }
}


static int mdbLoadFromString(Edi *edi, cchar *str)
{
    Mdb             *mdb;
    MprObj          *obj;
    MprJsonCallback cb;

    mdb = (Mdb*) edi;
    mdb->edi.flags |= EDI_SUPPRESS_SAVE;
    mdb->flags |= MDB_LOADING;
    mdb->loadStack = mprCreateList(0, 0);
    pushState(mdb, MDB_LOAD_BEGIN);

    cb.makeObj = makeMdbObj;
    cb.checkState = checkMdbState;
    cb.setValue = setMdbValue;
    cb.parseError = parseMdbError;

    obj = mprDeserializeCustom(str, cb, mdb);
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


/*
    Must be called locked
 */
static int mdbSave(Edi *edi)
{
    Mdb         *mdb;
    MdbSchema   *schema;
    MdbTable    *table;
    MdbRow      *row;
    MdbCol      *col;
    cchar       *value;
    char        *path, *npath, *bak, *type;
    MprFile     *out;
    int         cid, rid, tid, ntables, nrows;

    mdb = (Mdb*) edi;
    if (mdb->edi.flags & EDI_NO_SAVE) {
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
        mprWriteFileFmt(out, "{\n    '%s': {\n", table->name);
        mprWriteFileFmt(out, "        hints: {\n            ncols: %d\n        },\n", schema->ncols);
        mprWriteFileString(out, "        schema: {\n");
        /* Skip the id which is always the first column */
        for (cid = 0; cid < schema->ncols; cid++) {
            col = getCol(table, cid);
            type = ediGetTypeString(col->type);
            mprWriteFileFmt(out, "            '%s': { type: '%s'", col->name, type);
            if (col->flags & EDI_AUTO_INC) {
                mprWriteFileString(out, ", autoinc: 'true'");
            }
            if (col->flags & EDI_INDEX) {
                mprWriteFileString(out, ", index: 'true'");
            }
            if (col->flags & EDI_KEY) {
                mprWriteFileString(out, ", key: 'true'");
            }
#if UNUSED
            if (col->flags & EDI_NOT_NULL) {
                mprWriteFileString(out, ", notnull: 'true'");
            }
#endif
            mprWriteFileString(out, " },\n");
        }
        mprWriteFileString(out, "        },\n");
        mprWriteFileString(out, "        data: [\n");

        nrows = mprGetListLength(table->rows);
        for (rid = 0; rid < nrows; rid++) {
            mprWriteFileString(out, "            [ ");
            row = getRow(table, rid);
            for (cid = 0; cid < schema->ncols; cid++) {
                col = getCol(table, cid);
                value = row->fields[col->cid];
                if (value == 0 && col->flags & EDI_AUTO_INC) {
                    row->fields[col->cid] = itos(++col->lastValue);
                }
#if UNUSED
                field = readField(row, cid);
                //  MOB OPT - inline toString code here
                mprWriteFileFmt(out, "'%s', ", ediFormatField(NULL, field));
#else
                mprWriteFileFmt(out, "'%s', ", value);
#endif
            }
            mprWriteFileString(out, "],\n");
        }
        mprWriteFileString(out, "        ],\n    },\n");
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


static void manageTable(MdbTable *table, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(table->name);
        mprMark(table->schema);
        mprMark(table->index);
        mprMark(table->rows);
    }
}


//  MOB - inconsistent. Should return Row
//  MOB - need lookup by non-key field too
//  MOB - this should be in the provider 
static int lookupRow(MdbTable *table, cchar *key)
{
    MprKey      *kp;
    MdbRow      *row;
    int         nrows, r, keycol;

    if (table->index) {
        if ((kp = mprLookupKeyEntry(table->index, key)) != 0) {
            return (int) PTOL(kp->data);
        } 
    } else {
        nrows = mprGetListLength(table->rows);
        keycol = table->keyCol ? table->keyCol->cid : 0;
        for (r = 0; r < nrows; r++) {
            row = mprGetItem(table->rows, r);
            if (smatch(row->fields[keycol], key)) {
                return r;
            }
        }
    }
    return -1;
}


/********************************* Schema / Col ****************************/

static MdbSchema *growSchema(MdbTable *table)
{
    if (table->schema == 0) {
        if ((table->schema = mprAllocMem(sizeof(MdbSchema) + 
                sizeof(MdbCol) * MDB_INCR, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
            return 0;
        }
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
        mprMark(col->validations);
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
    int         ncols;

    ncols = max(table->schema->ncols, 1);
    if ((row = mprAllocMem(sizeof(MdbRow) + sizeof(EdiField) * ncols, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(row, manageRow);
    row->table = table;
    row->nfields = ncols;
    row->rid = mprAddItem(table->rows, row);
    return row;
}


static void manageRow(MdbRow *row, int flags)
{
    int     fid;

    if (flags & MPR_MANAGE_MARK) {
        for (fid = 0; fid < row->nfields; fid++) {
            mprMark(row->fields[fid]);
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
#if UNUSED && KEEP
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
#endif

static int updateField(MdbRow *row, MdbCol *col, cchar *value)
{
    MdbTable    *table;
    cchar       *key;
    
    mprAssert(row);
    mprAssert(col);

    table = row->table;
    if (col->flags & EDI_INDEX) {
        if ((key = row->fields[col->cid]) != 0) {
            mprRemoveKey(table->index, key);
        }
    } else {
        key = 0;
    }
    //  MOB - refactor
    if (col->flags & EDI_AUTO_INC) {
        if (value == 0) {
            row->fields[col->cid] = value = itos(++col->lastValue);
        } else {
            //  MOB - clone
            row->fields[col->cid] = sclone(value);
            col->lastValue = max(col->lastValue, (int64) stoi(value));
        }
    } else {
        //  MOB - clone
        row->fields[col->cid] = sclone(value);
    }
    if (col->flags & EDI_INDEX && value) {
        mprAddKey(table->index, value, LTOP(row->rid));
    }
    return 0;
}

/********************************* Validations *****************************/

//  MOB - try to hoist these to EDI (if no loss of performance)

static bool validateField(EdiRec *rec, MdbTable *table, MdbCol *col, cchar *value)
{
    EdiValidation   *vp;
    cchar           *error;
    int             next;
    bool            pass;

    mprAssert(rec);

    pass = 1;
    if (col->validations) {
        for (ITERATE_ITEMS(col->validations, vp, next)) {
            if ((error = (*vp->vfn)(vp, rec, col->name, value)) != 0) {
                if (rec->errors == 0) {
                    rec->errors = mprCreateList(0, 0);
                }
                mprAddItem(rec->errors, mprCreateKeyPair(col->name, error));
                pass = 0;
            }
        }
    }
    return pass;
}


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
    mprSetManager(rec, ediManageEdiRec);
    rec->edi = edi;
    rec->tableName = row->table->name;
    //  MOB - assert and check there is a valid ID
    rec->id = row->fields[0];
    rec->nfields = row->nfields;
    for (c = 0; c < row->nfields; c++) {
        col = getCol(row->table, c);
        rec->fields[c] = makeFieldFromRow(row, col);
    }
    return rec;
}


static int parseOperation(cchar *operation)
{
    switch (*operation) {
    case '=':
        if (smatch(operation, "==")) {
            return OP_EQ;
        }
        break;
    case '!':
        if (smatch(operation, "=!")) {
            return OP_EQ;
        }
        break;
    case '<':
        if (smatch(operation, "<")) {
            return OP_LT;
        } else if (smatch(operation, "<=")) {
            return OP_LTE;
        }
        break;
    case '>':
        if (smatch(operation, ">")) {
            return OP_GT;
        } else if (smatch(operation, ">=")) {
            return OP_GTE;
        }
    }
    mprError("Unknown read operation '%s'", operation);
    return OP_ERR;
}


#endif /* BLD_FEATURE_ESP && BLD_FEATURE_ESP && BLD_FEATURE_MDB */
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
