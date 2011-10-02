/*
    edi.c -- Embedded Database Interface (EDI)

    WARNING: This is prototype code

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "edi.h"

#if BLD_FEATURE_ESP
#if BLD_FEATURE_EDI || 1
/************************************* Local **********************************/

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
    return es;
}


static void manageEdiService(EdiService *es, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(es->providers);
        /* Don't mark load fields */
    }
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


int ediAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    return edi->provider->addColumn(edi, tableName, columnName, type, flags);
}


int ediAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName)
{
    return edi->provider->addIndex(edi, tableName, columnName, indexName);
}


int ediAddTable(Edi *edi, cchar *tableName)
{
    return edi->provider->addTable(edi, tableName);
}


int ediChangeCol(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags)
{
    return edi->provider->changeColumn(edi, tableName, columnName, type, flags);
}


void ediClose(Edi *edi)
{
    return edi->provider->close(edi);
}


EdiRec *ediCreateRec(Edi *edi, cchar *tableName)
{
    return edi->provider->createRec(edi, tableName);
}


int ediDelete(Edi *edi, cchar *path)
{
    return edi->provider->delete(edi, path);
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
    return ediGetColumns(grid->edi, grid->tableName);
}


EdiField ediGetRawRecField(EdiRec *rec, cchar *fieldName)
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


int ediGetRecFieldType(EdiRec *rec, cchar *fieldName)
{
    int     type;
    
    if (ediGetColumnSchema(rec->edi, rec->tableName, fieldName, &type, NULL, NULL) < 0) {
        return 0;
    }
    return type;
}


cchar *ediGetRecField(cchar *fmt, EdiRec *rec, cchar *fieldName)
{
    EdiField    field;

    field = ediGetRawRecField(rec, fieldName);
    if (!field.valid) {
        return "";
    }
    return ediFormatField(fmt, field);
}


MprList *ediGetTables(Edi *edi)
{
    return edi->provider->getTables(edi);
}


char *ediGetTypeString(int type)
{
    switch (type) {
    case EDI_TYPE_BLOB:
        return "blob";
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


Edi *ediOpen(cchar *providerName, cchar *path, int flags)
{
    EdiProvider  *provider;

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
#if UNUSED
    ediGetColumnSchema(edi, tableName, columnName, &type, NULL, NULL);
#endif
    return ediFormatField(fmt, field);
}


EdiGrid *ediReadGrid(Edi *edi, cchar *tableName)
{
    return edi->provider->readGrid(edi, tableName);
}


EdiRec *ediReadOneWhere(Edi *edi, cchar *tableName, cchar *expression)
{
    EdiGrid *grid;
    
    if ((grid = ediReadWhere(edi, tableName, expression)) == 0) {
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


EdiGrid *ediReadWhere(Edi *edi, cchar *tableName, cchar *expression)
{
    EdiGrid     *grid;
    EdiRec      *rec;
    int         r;

    grid = ediReadGrid(edi, tableName);
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        //  MOB - TODO 
    }
    return grid;
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


int ediWriteField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value)
{
    return edi->provider->writeField(edi, tableName, key, fieldName, value);
}


int ediWriteFields(Edi *edi, cchar *tableName, MprHash *params)
{
    return edi->provider->writeFields(edi, tableName, params);
}


int ediWriteRec(Edi *edi, EdiRec *rec)
{
    return edi->provider->writeRec(edi, rec);
}

/********************************* Validations *****************************/
#if FUTURE
//  MOB - complete
void ediAddValidation(cchar *name, void *validation)
{
    EdiService  *es;

    es = MPR->ediService;
    mprAddKey(es->validations, name, validation);
}


static bool checkFormat(EdiRec *rec, cchar *fieldName, EdiField *value)
{
    if (value) {
        mprAddItem(rec->errors, mprCreateKeyPair(fieldName, "is in the wrong format"));
    }
    return 1;
}


static bool checkNumber(EdiRec *rec)
{
    if (value) {
        mprAddItem(rec->errors, mprCreateKeyPair(field, "is not a number"));
    return 1;
}


static bool checkPresent(EdiRec *rec)
{
    if (value) {
        mprAddItem(rec->errors, mprCreateKeyPair(field, "is missing"));
    }
    return 1;
}


static bool checkUnique(EdiRec *rec)
{
#if 0
    if (value) {
        if (rec->errors == 0) {
            rec->errors = mprCreateList(rec->row->nfields, -1);
        }
        mprAddItem(rec->errors, mprCreateKeyPair(field, "is not unique"));
    }
#endif
    return 1;
}


bool edValidateRecord(EdiRec *rec)
{
    EdiSchema   *schema;
    EdiTable    *table;
    EdiCol      *col;
    bool        pass;

    rec->errors = mprCreateList(0, 0);
    if ((table = edLookupTable(rec->edi, rec->tableName)) == 0) {
        return 0;
    }
    schema = table->schema;
    pass = 1;
    for (col = schema->cols; col < &schema->cols[schema->ncols]; col++) {
        if (col->validate && !(*col->validate)(rec)) {
            pass = 0;
        }
    }
    return pass;
}

#endif
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
    mprSetAllocName(grid, "grid");
    mprSetManager(grid, manageEdiGrid);
    grid->nrecords = nrows;
    grid->edi = edi;
    grid->tableName = sclone(tableName);
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
    mprSetAllocName(rec, "record");
    mprSetManager(rec, ediManageEdiRec);

    rec->edi = edi;
    rec->tableName = sclone(tableName);
    rec->nfields = nfields;
    return rec;
}


cchar *ediFormatField(cchar *fmt, EdiField field)
{
    switch (field.type) {
    case EDI_TYPE_BLOB:
        //  MOB - use printable code from MPR
        return field.value.blob;

    case EDI_TYPE_BOOL:
        //  MOB OPT - should be mpr cloned string
        return field.value.boolean ? sclone("true") : sclone("false");

    case EDI_TYPE_DATE:
        return mprFormatLocalTime(fmt, field.value.date);

    case EDI_TYPE_FLOAT:
        if (fmt == 0) {
            fmt = "%f";
        }
        return sfmt(fmt, field.value.num);

    case EDI_TYPE_INT:
        return sfmt("%Ld", field.value.inum);

    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
        if (fmt) {
            return sfmt(fmt, field.value.str);
        }
        return field.value.str;

    default:
        mprError("Unknown field type %d", field.type);
    }
    return 0;
}


void ediManageEdiRec(EdiRec *rec, int flags)
{
    int     fid, type;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(rec->edi);
        mprMark(rec->errors);
        mprMark(rec->tableName);
        mprMark(rec->id);

        for (fid = 0; fid < rec->nfields; fid++) {
            type = rec->fields[fid].type;
            if (type == EDI_TYPE_STRING || type == EDI_TYPE_TEXT) {
                mprMark(&rec->fields[fid].value.str);
            }
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
    MprKey      *kp;
    EdiGrid     *grid;
    EdiRec      *rec;
    int         r, nrows, nfields;

    if ((obj = mprDeserialize(json)) == 0) {
        return 0;
    }
    nrows = mprGetHashLength(obj);
    if ((grid = ediCreateBareGrid(NULL, "", nrows)) == 0) {
        return 0;
    }
    for (r = 0, ITERATE_KEYS(obj, kp)) {
        if (kp->type != MPR_JSON_OBJ) {
            continue;
        }
        row = (MprHash*) kp->data;
        nfields = mprGetHashLength(row);
        if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
            return 0;
        }
        if (ediUpdateFields(rec, row) == 0) {
            return 0;
        }
        grid->records[r++] = rec;
    }
    return 0;
}


/*
    rec = ediMakeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
 */
EdiRec *ediMakeRec(cchar *json)
{
    MprHash     *obj;
    EdiRec      *rec;
    int         nfields;

    if ((obj = mprDeserialize(json)) == 0) {
        return 0;
    }
    nfields = mprGetHashLength(obj);
    if ((rec = ediCreateBareRec(NULL, "", nfields)) == 0) {
        return 0;
    }
    return ediUpdateFields(rec, obj);
}



EdiValue ediParseValue(cchar *value, int type)
{
    EdiValue v;

    switch (type) {
    case EDI_TYPE_BLOB:
        /* Don't clone as value is already allocated */
        v.blob = (char*) value;
        break;
    case EDI_TYPE_BOOL:
        v.boolean = smatch(value, "true") ? 1 : 0;
        break;
    case EDI_TYPE_DATE:
        v.date = mprParseTime(&v.date, value, MPR_UTC_TIMEZONE, NULL);
        break;
    case EDI_TYPE_FLOAT:
        v.num = atof(value);
        break;
    case EDI_TYPE_INT:
        v.inum = atoi(value);
        break;
    case EDI_TYPE_STRING:
    case EDI_TYPE_TEXT:
        /* Don't clone as value is already allocated */
        v.str = (char*) value;
        break;
    default:
        mprError("Unknown field type '%d'", type);
    }
    return v;
}


int ediParseTypeString(cchar *type)
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


EdiRec *ediUpdateField(EdiRec *rec, cchar *fieldName, cchar *value)
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
            fp->value = ediParseValue(value, fp->type);
            return rec;
        }
    }
    return rec;
}


EdiRec *ediUpdateFields(EdiRec *rec, MprHash *params)
{
    MprKey  *kp;

    if (rec == 0) {
        return 0;
    }
    for (ITERATE_KEYS(params, kp)) {
        if (kp->type == MPR_JSON_ARRAY || kp->type == MPR_JSON_OBJ) {
            continue;
        }
        if (!ediUpdateField(rec, kp->key, kp->data)) {
            return 0;
        }
    }
    return rec;
}


#endif /* BLD_FEATURE_EDI || 1 */
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
