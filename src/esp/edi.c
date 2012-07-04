/*
    edi.c -- Embedded Database Interface (EDI)

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "edi.h"
#include    "pcre.h"

#if BIT_FEATURE_ESP
/************************************* Local **********************************/

static void addValidations();
static void manageEdiService(EdiService *es, int flags);
static void manageEdiGrid(EdiGrid *grid, int flags);

/************************************* Code ***********************************/

EdiService *ediCreateService()
{
    EdiService      *es;

    if ((es = mprAllocObj(EdiService, manageEdiService)) == 0) {
        return 0;
    }
    MPR->ediService = es;
    es->providers = mprCreateHash(0, MPR_HASH_STATIC_VALUES);
    addValidations();
    return es;
}


static void manageEdiService(EdiService *es, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(es->providers);
        mprMark(es->validations);
    }
}


int ediAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    return edi->provider->addColumn(edi, tableName, columnName, type, flags);
}


int ediAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName)
{
    return edi->provider->addIndex(edi, tableName, columnName, indexName);
}


void ediAddProvider(EdiProvider *provider)
{
    EdiService  *es;

    es = MPR->ediService;
    mprAddKey(es->providers, provider->name, provider);
}


static EdiProvider *lookupProvider(cchar *providerName)
{
    EdiService  *es;

    es = MPR->ediService;
    return mprLookupKey(es->providers, providerName);
}


int ediAddTable(Edi *edi, cchar *tableName)
{
    return edi->provider->addTable(edi, tableName);
}


static void manageValidation(EdiValidation *vp, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(vp->name);
        mprMark(vp->data);
    }
}


int ediAddValidation(Edi *edi, cchar *name, cchar *tableName, cchar *columnName, cvoid *data)
{
    EdiService          *es;
    EdiValidation       *vp; 
    cchar               *errMsg;
    int                 column;

    es = MPR->ediService;
    if ((vp = mprAllocObj(EdiValidation, manageValidation)) == 0) {
        return MPR_ERR_MEMORY;
    }
    vp->name = sclone(name);
    if ((vp->vfn = mprLookupKey(es->validations, name)) == 0) {
        mprError("Can't find validation '%s'", name);
        return MPR_ERR_CANT_FIND;
    }
    if (smatch(name, "format")) {
        if ((vp->mdata = pcre_compile2(data, 0, 0, &errMsg, &column, NULL)) == 0) {
            mprError("Can't compile validation pattern. Error %s at column %d", errMsg, column); 
            return MPR_ERR_BAD_SYNTAX;
        }
        data = 0;
    }
    vp->data = data;
    return edi->provider->addValidation(edi, tableName, columnName, vp);
}


int ediChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    return edi->provider->changeColumn(edi, tableName, columnName, type, flags);
}


void ediClose(Edi *edi)
{
    edi->provider->close(edi);
}


EdiRec *ediCreateRec(Edi *edi, cchar *tableName)
{
    return edi->provider->createRec(edi, tableName);
}


int ediDelete(Edi *edi, cchar *path)
{
    return edi->provider->delete(path);
}


int ediDeleteRow(Edi *edi, cchar *tableName, cchar *key)
{
    return edi->provider->deleteRow(edi, tableName, key);
}


MprList *ediGetColumns(Edi *edi, cchar *tableName)
{
    return edi->provider->getColumns(edi, tableName);
}


int ediGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid)
{
    return edi->provider->getColumnSchema(edi, tableName, columnName, type, flags, cid);
}


MprList *ediGetRecErrors(EdiRec *rec)
{
    return rec->errors;
}


MprList *ediGetGridColumns(EdiGrid *grid)
{
    MprList     *cols;
    EdiRec      *rec;
    EdiField    *fp;

#if UNUSED
    // Won't work for pivots
    if (grid->tableName && grid->tableName) {
        return ediGetColumns(grid->edi, grid->tableName);
    }
#endif
    cols = mprCreateList(0, 0);
    rec = grid->records[0];
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        mprAddItem(cols, fp->name);
    }
    return cols;
}


EdiField *ediGetField(EdiRec *rec, cchar *fieldName)
{
    EdiField    *fp;

    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            return fp;
        }
    }
    return 0;
}


cchar *ediGetFieldValue(EdiRec *rec, cchar *fieldName)
{
    EdiField    *fp;

    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            return fp->value;
        }
    }
    return 0;
}


int ediGetFieldType(EdiRec *rec, cchar *fieldName)
{
    int     type;
    
    if (ediGetColumnSchema(rec->edi, rec->tableName, fieldName, &type, NULL, NULL) < 0) {
        return 0;
    }
    return type;
}


cchar *ediGetFieldFmt(cchar *fmt, EdiRec *rec, cchar *fieldName)
{
    EdiField    field;

    field = ediGetFieldSchema(rec, fieldName);
    if (!field.valid) {
        return "";
    }
    return ediFormatField(fmt, &field);
}


EdiField ediGetFieldSchema(EdiRec *rec, cchar *fieldName)
{
    EdiField    err, *fp;

    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            return *fp;
        }
    }
    err.valid = 0;
    return err;
}


MprList *ediGetTables(Edi *edi)
{
    return edi->provider->getTables(edi);
}


int ediGetTableSchema(Edi *edi, cchar *tableName, int *numRows, int *numCols)
{
    return edi->provider->getTableSchema(edi, tableName, numRows, numCols);
}


char *ediGetTypeString(int type)
{
    switch (type) {
    case EDI_TYPE_BINARY:
        return "binary";
    case EDI_TYPE_BOOL:
        return "bool";
    case EDI_TYPE_DATE:
        return "date";
    case EDI_TYPE_FLOAT:
        return "float";
    case EDI_TYPE_INT:
        return "int";
    case EDI_TYPE_STRING:
        return "string";
    case EDI_TYPE_TEXT:
        return "text";
    }
    return 0;
}


int ediLoad(Edi *edi, cchar *path)
{
    return edi->provider->load(edi, path);
}


int ediLookupField(Edi *edi, cchar *tableName, cchar *fieldName)
{
    return edi->provider->lookupField(edi, tableName, fieldName);
}


Edi *ediOpen(cchar *path, cchar *providerName, int flags)
{
    EdiProvider     *provider;

    if ((provider = lookupProvider(providerName)) == 0) {
        mprError("Can't find EDI provider '%s'", providerName);
        return 0;
    }
    return provider->open(path, flags);
}


EdiGrid *ediQuery(Edi *edi, cchar *cmd)
{
    return edi->provider->query(edi, cmd);
}


cchar *ediReadField(Edi *edi, cchar *fmt, cchar *tableName, cchar *key, cchar *columnName, cchar *defaultValue)
{
    EdiField    field;

    field = ediReadRawField(edi, tableName, key, columnName);
    if (!field.valid) {
        return defaultValue;
    }
    return ediFormatField(fmt, &field);
}


EdiRec *ediReadOneWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    EdiGrid *grid;
    
    //  MOB - slow to read entire table. Need optimized query in providers
    if ((grid = ediReadWhere(edi, tableName, fieldName, operation, value)) == 0) {
        return 0;
    }
    if (grid->nrecords > 0) {
        return grid->records[0];
    }
    return 0;
}


EdiField ediReadRawField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    return edi->provider->readField(edi, tableName, key, fieldName);
}


EdiRec *ediReadRec(Edi *edi, cchar *tableName, cchar *key)
{
    return edi->provider->readRec(edi, tableName, key);
}


EdiGrid *ediReadWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value)
{
    return edi->provider->readWhere(edi, tableName, fieldName, operation, value);
}


EdiGrid *ediReadTable(Edi *edi, cchar *tableName)
{
    return edi->provider->readWhere(edi, tableName, 0, 0, 0);
}


int edRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName)
{
    return edi->provider->removeColumn(edi, tableName, columnName);
}


int ediRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName)
{
    return edi->provider->removeIndex(edi, tableName, indexName);
}


int ediRemoveTable(Edi *edi, cchar *tableName)
{
    return edi->provider->removeTable(edi, tableName);
}


int ediRenameTable(Edi *edi, cchar *tableName, cchar *newTableName)
{
    return edi->provider->renameTable(edi, tableName, newTableName);
}


int ediRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName)
{
    return edi->provider->renameColumn(edi, tableName, columnName, newColumnName);
}


int ediSave(Edi *edi)
{
    return edi->provider->save(edi);
}


int ediUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    return edi->provider->updateField(edi, tableName, key, fieldName, value);
}


int ediUpdateRec(Edi *edi, EdiRec *rec)
{
    return edi->provider->updateRec(edi, rec);
}


bool ediValidateRec(EdiRec *rec)
{
    mprAssert(rec->edi);
    if (rec->edi == 0) {
        return 0;
    }
    return rec->edi->provider->validateRec(rec->edi, rec);
}


/********************************* Convenience *****************************/
/*
    Create a free-standing grid. Not saved to the database
    The edi and tableName parameters can be null
 */
EdiGrid *ediCreateBareGrid(Edi *edi, cchar *tableName, int nrows)
{
    EdiGrid  *grid;

    if ((grid = mprAllocMem(sizeof(EdiGrid) + sizeof(EdiRec*) * nrows, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(grid, (MprManager) manageEdiGrid);
    grid->nrecords = nrows;
    grid->edi = edi;
    grid->tableName = tableName? sclone(tableName) : 0;
    return grid;
}


/*
    Create a free-standing record. Not saved to the database.
    The tableName parameter can be null. The fields are not initialized (no schema).
 */
EdiRec *ediCreateBareRec(Edi *edi, cchar *tableName, int nfields)
{
    EdiRec      *rec;

    if ((rec = mprAllocMem(sizeof(EdiRec) + sizeof(EdiField) * nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetManager(rec, (MprManager) ediManageEdiRec);
    rec->edi = edi;
    rec->tableName = sclone(tableName);
    rec->nfields = nfields;
    return rec;
}


cchar *ediFormatField(cchar *fmt, EdiField *fp)
{
    MprTime     when;

    if (fmt == 0) {
        fmt = "%s";
    }
    switch (fp->type) {
    case EDI_TYPE_BINARY:
    case EDI_TYPE_BOOL:
        return fp->value;

    case EDI_TYPE_DATE:
        if (mprParseTime(&when, fp->value, MPR_LOCAL_TIMEZONE, 0) == 0) {
            return mprFormatLocalTime(fmt, when);
        }
        return fp->value;

    case EDI_TYPE_FLOAT:
        return sfmt(fmt, atof(fp->value));

    case EDI_TYPE_INT:
        return sfmt("%Ld", stoi(fp->value));

    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
        return sfmt(fmt, fp->value);

    default:
        mprError("Unknown field type %d", fp->type);
    }
    return 0;
}


typedef struct Col {
    EdiGrid     *grid;
    EdiField    *fp;
    int         joinField;      /* Foreign key field index */
    int         field;          /* Field index in the foreign table */
} Col;


static MprList *joinColumns(MprList *cols, EdiGrid *grid, MprHash *grids, int joinField, int follow)
{
    EdiGrid     *foreignGrid;
    EdiRec      *rec;
    EdiField    *fp;
    Col         *col;
    cchar       *tableName;
    
    if (grid->nrecords == 0) {
        return cols;
    }
    rec = grid->records[0];
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (fp->flags & EDI_FOREIGN && follow) {
            tableName = strim(fp->name, "Id", MPR_TRIM_END);
            if (!(foreignGrid = mprLookupKey(grids, tableName))) {
                col = mprAllocObj(Col, 0);
                col->grid = grid;
                col->field = (int) (fp - rec->fields);
                col->joinField = joinField;
                col->fp = fp;
                mprAddItem(cols, col);
            } else {
                cols = joinColumns(cols, foreignGrid, grids, (int) (fp - rec->fields), 0);
            }
        } else {
            if (fp->flags & EDI_KEY && joinField >= 0) {
                /* Don't include ID fields from joined tables */
                continue;
            }
            col = mprAllocObj(Col, 0);
            col->grid = grid;
            col->field = (int) (fp - rec->fields);
            col->joinField = joinField;
            col->fp = fp;
            mprAddItem(cols, col);
        }
    }
    return cols;
}


/*
    List of grids to join must be null terminated
 */
EdiGrid *ediJoin(Edi *edi, ...)
{
    EdiGrid     *primary, *grid, *result, *current;
    EdiRec      *rec;
    EdiField    *dest, *fp;
    MprList     *cols;
    MprHash     *grids;
    Col         *col;
    va_list     vgrids;
    cchar       *keyValue;
    int         nfields, r, next;

    va_start(vgrids, edi);
    if ((primary = va_arg(vgrids, EdiGrid*)) == 0) {
        return 0;
    }
    if ((result = ediCreateBareGrid(edi, NULL, 0)) == 0) {
        return 0;
    }
    if (primary->nrecords == 0) {
        return result;
    }
    /*
        Build list of grids to join
     */
    grids = mprCreateHash(0, 0);
    for (;;) {
        if ((grid = va_arg(vgrids, EdiGrid*)) == 0) {
            break;
        }
        mprAddKey(grids, grid->tableName, grid);
    }
    va_end(vgrids);

    /*
        Get list of columns for the result. Each col object records the target grid and field index.
     */
    cols = joinColumns(mprCreateList(0, 0), primary, grids, -1, 1);
    nfields = mprGetListLength(cols);

    for (r = 0; r < primary->nrecords; r++) {
        if ((rec = ediCreateBareRec(edi, NULL, nfields)) == 0) {
            mprAssert(0);
            return 0;
        }
        result->records[r] = rec;
        dest = rec->fields;
        current = 0;
        for (ITERATE_ITEMS(cols, col, next)) { 
            if (col->grid == primary) {
                *dest = primary->records[r]->fields[col->field];
            } else {
                if (col->grid != current) {
                    current = col->grid;
                    keyValue = primary->records[r]->fields[col->joinField].value;
                    rec = ediReadOneWhere(edi, col->grid->tableName, "id", "==", keyValue);
                }
                mprAssert(rec);
                fp = &rec->fields[col->field];
                *dest = *fp;
                dest->name = sfmt("%s.%s", col->grid->tableName, fp->name);
            }
            dest++;
        }
    }
    result->nrecords = r;
    return result;
}


void ediManageEdiRec(EdiRec *rec, int flags)
{
    int     fid;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(rec->edi);
        mprMark(rec->errors);
        mprMark(rec->tableName);
        mprMark(rec->id);

        for (fid = 0; fid < rec->nfields; fid++) {
            mprMark(rec->fields[fid].value);
        }
    }
}


static void manageEdiGrid(EdiGrid *grid, int flags)
{
    int     r;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(grid->edi);
        mprMark(grid->tableName);
        for (r = 0; r < grid->nrecords; r++) {
            mprMark(grid->records[r]);
        }
    }
}


/*
    grid = ediMakeGrid("[ \
        { id: '1', country: 'Australia' }, \
        { id: '2', country: 'China' }, \
    ]");
 */
EdiGrid *ediMakeGrid(cchar *json)
{
    MprHash     *obj, *row;
    MprKey      *rp, *kp;
    EdiGrid     *grid;
    EdiRec      *rec;
    EdiField    *fp;
    char        rowbuf[16];
    int         r, nrows, nfields;

    if ((obj = mprDeserialize(json)) == 0) {
        mprAssert(0);
        return 0;
    }
    if (!(obj->flags & MPR_HASH_LIST)) {
        mprAssert(obj->flags & MPR_HASH_LIST);
        return 0;
    }
    nrows = mprGetHashLength(obj);
    if ((grid = ediCreateBareGrid(NULL, "", nrows)) == 0) {
        mprAssert(0);
        return 0;
    }
    if (nrows <= 0) {
        return grid;
    }
    for (r = 0; r < nrows; r++) {
        itosbuf(rowbuf, sizeof(rowbuf), r, 10);
        if ((rp = mprLookupKeyEntry(obj, rowbuf)) == 0) {
            continue;
        }
        /* JSON objects are either char* or MprHash */
        if (rp->type == MPR_JSON_STRING) {
            nfields = 1;
            if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
                return 0;
            }
            fp = rec->fields;
            fp->valid = 1;
            fp->name = sclone("value");
            fp->value = rp->data;
            fp->type = EDI_TYPE_STRING;
            fp->flags = 0;
        } else {
            row = (MprHash*) rp->data;
            nfields = mprGetHashLength(row);
            if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
                return 0;
            }
            fp = rec->fields;
            for (ITERATE_KEYS(row, kp)) {
                if (fp >= &rec->fields[nfields]) {
                    break;
                }
                fp->valid = 1;
                fp->name = kp->key;
                fp->type = EDI_TYPE_STRING;
                fp->flags = 0;
                fp++;
            }
            if (ediSetFields(rec, row) == 0) {
                mprAssert(0);
                return 0;
            }
        }
        grid->records[r] = rec;
    }
    return grid;
}


MprHash *ediMakeHash(cchar *fmt, ...)
{
    MprHash     *obj;
    va_list     args;

    va_start(args, fmt);
    obj = mprDeserialize(sfmtv(fmt, args));
    va_end(args);
    return obj;
}


/*
    rec = ediMakeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
 */
EdiRec *ediMakeRec(cchar *json)
{
    MprHash     *obj;
    MprKey      *kp;
    EdiRec      *rec;
    EdiField    *fp;
    int         f, nfields;

    if ((obj = mprDeserialize(json)) == 0) {
        return 0;
    }
    nfields = mprGetHashLength(obj);
    if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
        return 0;
    }
    for (f = 0, ITERATE_KEYS(obj, kp)) {
        if (kp->type == MPR_JSON_ARRAY || kp->type == MPR_JSON_OBJ) {
            continue;
        }
        fp = &rec->fields[f++];
        fp->valid = 1;
        fp->name = kp->key;
        fp->value = kp->data;
        fp->type = EDI_TYPE_STRING;
        fp->flags = 0;
    }
    return rec;
}


int ediParseTypeString(cchar *type)
{
    if (smatch(type, "binary")) {
        return EDI_TYPE_BINARY;
    } else if (smatch(type, "bool") || smatch(type, "boolean")) {
        return EDI_TYPE_BOOL;
    } else if (smatch(type, "date")) {
        return EDI_TYPE_DATE;
    } else if (smatch(type, "float") || smatch(type, "double") || smatch(type, "number")) {
        return EDI_TYPE_FLOAT;
    } else if (smatch(type, "int") || smatch(type, "integer") || smatch(type, "fixed")) {
        return EDI_TYPE_INT;
    } else if (smatch(type, "string")) {
        return EDI_TYPE_STRING;
    } else if (smatch(type, "text")) {
        return EDI_TYPE_TEXT;
    } else {
        return MPR_ERR_BAD_ARGS;
    }
    return 0;
}


EdiGrid *ediPivotGrid(EdiGrid *grid, int flags)
{
    EdiGrid     *result;
    EdiRec      *rec, *first;
    EdiField    *src, *fp;
    cchar       *name;
    int         r, c, nfields, nrows;

    if (grid->nrecords == 0) {
        return grid;
    }
    name = 0;
    first = grid->records[0];
    nrows = first->nfields;
    nfields = grid->nrecords;
    if (flags & EDI_PIVOT_FIELD_NAMES) {
        /* One more field in result */
        nfields++;
        name = sclone("name");
    }
    result = ediCreateBareGrid(grid->edi, grid->tableName, nrows);
    
    for (c = 0; c < nrows; c++) {
        result->records[c] = rec = ediCreateBareRec(grid->edi, grid->tableName, nfields);
        fp = rec->fields;
        if (flags & EDI_PIVOT_FIELD_NAMES) {
            /* Add the field names as the first column */
            fp->valid = 1;
            fp->name = name;
            fp->value = first->fields[c].name;
            fp->type = EDI_TYPE_STRING;
            fp->flags = 0;
            fp++;
        }
        for (r = 0; r < grid->nrecords; r++) {
            src = &grid->records[r]->fields[c];
            fp->valid = 1;
            fp->name = src->name;
            fp->type = src->type;
            fp->value = src->value;
            fp->flags = src->flags;
            fp++; src++;
        }
    }
    return result;
}

EdiGrid *ediCloneGrid(EdiGrid *grid)
{
    EdiGrid     *result;
    EdiRec      *rec;
    EdiField    *src, *dest;
    int         r, c;

    if (grid->nrecords == 0) {
        return grid;
    }
    result = ediCreateBareGrid(grid->edi, grid->tableName, grid->nrecords);
    for (r = 0; r < grid->nrecords; r++) {
        rec = ediCreateBareRec(grid->edi, grid->tableName, grid->records[r]->nfields);
        result->records[r] = rec;
        src = grid->records[r]->fields;
        dest = rec->fields;
        for (c = 0; c < rec->nfields; c++) {
            dest->valid = 1;
            dest->name = src->name;
            dest->value = src->value;
            dest->type = src->type;
            dest->flags = src->flags;
            dest++; src++;
        }
    }
    return result;
}


EdiRec *ediSetField(EdiRec *rec, cchar *fieldName, cchar *value)
{
    EdiField    *fp;

    if (rec == 0) {
        return 0;
    }
    if (fieldName == 0 || value == 0) {
        return 0;
    }
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(fp->name, fieldName)) {
            fp->value = sclone(value);
            return rec;
        }
    }
    return rec;
}


EdiRec *ediSetFields(EdiRec *rec, MprHash *params)
{
    MprKey  *kp;

    if (rec == 0) {
        return 0;
    }
    for (ITERATE_KEYS(params, kp)) {
        if (kp->type == MPR_JSON_ARRAY || kp->type == MPR_JSON_OBJ) {
            continue;
        }
        if (!ediSetField(rec, kp->key, kp->data)) {
            return 0;
        }
    }
    return rec;
}


//  MOB - move
typedef struct GridSort {
    int     sortColumn;         /**< Column to sort on */
    int     sortOrder;          /**< Sort order: ascending == 1, descending == -1 */
} GridSort;

static int sortRec(EdiRec **r1, EdiRec **r2, GridSort *gs)
{
    EdiField    *f1, *f2;

    f1 = &(*r1)->fields[gs->sortColumn];
    f2 = &(*r2)->fields[gs->sortColumn];
    return scmp(f1->value, f2->value) * gs->sortOrder;
}


//  MOB - move
//  MOB - need ediLookupRecField
int ediLookupGridField(EdiGrid *grid, cchar *name)
{
    EdiRec      *rec;
    EdiField    *fp;

    if (grid->nrecords == 0) {
        return MPR_ERR_CANT_FIND;
    }
    rec = grid->records[0];
    for (fp = rec->fields; fp < &rec->fields[rec->nfields]; fp++) {
        if (smatch(name, fp->name)) {
            return (int) (fp - rec->fields);
        }
    }
    return MPR_ERR_CANT_FIND;
}


EdiGrid *ediSortGrid(EdiGrid *grid, cchar *sortColumn, int sortOrder)
{
    GridSort    gs;

    if (grid->nrecords == 0) {
        return grid;
    }
    grid = ediCloneGrid(grid);
    gs.sortColumn = ediLookupGridField(grid, sortColumn);
    gs.sortOrder = sortOrder;
    mprSort(grid->records, grid->nrecords, sizeof(EdiRec*), (MprSortProc) sortRec, &gs);
    return grid;
}


/********************************* Validations *****************************/

static cchar *checkBoolean(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0) {
            return 0;
        }
    }
    return "is not a number";
}


static cchar *checkDate(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    MprTime     time;

    if (value && *value) {
        if (mprParseTime(&time, value, MPR_LOCAL_TIMEZONE, NULL) < 0) {
            return 0;
        }
    }
    return "is not a date or time";
}


static cchar *checkFormat(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    int     matched[HTTP_MAX_ROUTE_MATCHES * 2];

    if (pcre_exec(vp->mdata, NULL, value, (int) slen(value), 0, 0, matched, sizeof(matched) / sizeof(int)) > 0) {
        return 0;
    }
    return "is in the wrong format";
}


static cchar *checkInteger(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        if (snumber(value)) {
            return 0;
        }
    }
    return "is not an integer";
}


static cchar *checkNumber(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        if (strspn(value, "1234567890+-.") == strlen(value)) {
            return 0;
        }
    }
    return "is not a number";
}


static cchar *checkPresent(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    if (value && *value) {
        return 0;
    }
    return "is missing";
}


static cchar *checkUnique(EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value)
{
    EdiRec  *other;

    //  OPT Could require an index to enforce this.
    if ((other = ediReadOneWhere(rec->edi, rec->tableName, fieldName, "==", value)) == 0) {
        return 0;
    }
    if (smatch(other->id, rec->id)) {
        return 0;
    }
    return "is not unique";
}


void ediDefineValidation(cchar *name, EdiValidationProc vfn)
{
    EdiService  *es;

    es = MPR->ediService;
    mprAddKey(es->validations, name, vfn);
}


void ediDefineMigration(Edi *edi, EdiMigration forw, EdiMigration back)
{
    edi->forw = forw;
    edi->back = back;
}


static void addValidations()
{
    EdiService  *es;

    es = MPR->ediService;
    es->validations = mprCreateHash(0, MPR_HASH_STATIC_VALUES);
    ediDefineValidation("boolean", checkBoolean);
    ediDefineValidation("format", checkFormat);
    ediDefineValidation("integer", checkInteger);
    ediDefineValidation("number", checkNumber);
    ediDefineValidation("present", checkPresent);
    ediDefineValidation("date", checkDate);
    ediDefineValidation("unique", checkUnique);
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
