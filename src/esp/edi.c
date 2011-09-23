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


int ediDelete(Edi *edi, cchar *path)
{
    return edi->provider->delete(edi, path);
}


int ediDeleteRow(Edi *edi, cchar *tableName, cchar *key)
{
    return edi->provider->deleteRow(edi, tableName, key);
}


EdiGrid *ediGetAll(Edi *edi, cchar *tableName)
{
    return edi->provider->getAll(edi, tableName);
}


MprList *ediGetColumns(Edi *edi, cchar *tableName)
{
    return edi->provider->getColumns(edi, tableName);
}


EdiField ediGetField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName)
{
    return edi->provider->getField(edi, tableName, key, fieldName);
}


EdiRec *ediGetRec(Edi *edi, cchar *tableName, cchar *key)
{
    return edi->provider->getRec(edi, tableName, key);
}


int ediGetSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags)
{
    return edi->provider->getSchema(edi, tableName, columnName, type, flags);
}


MprList *ediGetTables(Edi *edi)
{
    return edi->provider->getTables(edi);
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


int ediSetField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value, int flags)
{
    return edi->provider->setField(edi, tableName, key, fieldName, value, flags);
}


int ediSetRec(Edi *edi, cchar *tableName, cchar *key, MprHashTable *params)
{
    return edi->provider->setRec(edi, tableName, key, params);
}


int ediSave(Edi *edi)
{
    return edi->provider->save(edi);
}


/********************************* Validations *****************************/
#if FUTURE

void ediAddValidation(cchar *name, void *validation)
{
    EdiService  *es;

    es = MPR->ediService;
    mprAddKey(es->validations, name, validation);
}


MprList *edGetErrors(EdiRec *record)
{
    return record->errors;
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
        if (col->validate) {
            if (!(*col->validate)(rec)) {
                pass = 0;
            }
        }
    }
    return pass;
}

#endif
/********************************* Convenience *****************************/

EdiRec *ediCreateRec(Edi *edi, cchar *tableName, cchar *id, int nfields, EdiField *fields)
{
    EdiRec   *rec;

    if ((rec = mprAllocMem(sizeof(EdiRec) + sizeof(EdiField) * nfields, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetAllocName(rec, "record");
    mprSetManager(rec, ediManageEdiRec);

    rec->tableName = sclone(tableName);
    rec->id = sclone(id);
    //  MOB - should this copy the row (YES)
    rec->nfields = nfields;
    memmove(rec->fields, fields, sizeof(EdiField) * nfields);
    return rec;
}


void ediManageEdiRec(EdiRec *rec, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(rec->edi);
        mprMark(rec->errors);
        mprMark(rec->tableName);
        mprMark(rec->id);
#if MOB
        int     fid;
        !!!! NEED TYPES in REC
        for (fid = 0; fid < rec->nfields; fid++) {
            type = rec->table->schema->cols[fid].type;
            if (type == EDI_TYPE_STRING || type == EDI_TYPE_TEXT) {
                mprMark(&row->fields[fid].value.str);
            }
        }
#endif
    }
}


EdiGrid *ediCreateGrid(int nrows)
{
    EdiGrid  *grid;

    if ((grid = mprAllocMem(sizeof(EdiGrid) + sizeof(EdiRec*) * nrows, MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO)) == 0) {
        return 0;
    }
    mprSetAllocName(grid, "grid");
    mprSetManager(grid, manageEdiGrid);
    return grid;
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


int ediGetDbFlags(Edi *edi)
{
    return edi->flags;
}


extern MprList *ediGetErrors(EdiRec *rec)
{
    //  MOB
    return rec->errors;
}


EdiRec *ediGetOneWhere(Edi *edi, cchar *tableName, cchar *expression)
{
    EdiGrid *grid;
    
    if ((grid = ediGetWhere(edi, tableName, expression)) == 0) {
        return 0;
    }
    if (grid->nrecords > 0) {
        return grid->records[0];
    }
    return 0;
}


EdiGrid *ediGetWhere(Edi *edi, cchar *tableName, cchar *expression)
{
    EdiGrid     *grid;
    EdiRec      *rec;
    int         r;

    grid = ediGetAll(edi, tableName);
    for (r = 0; r < grid->nrecords; r++) {
        rec = grid->records[r];
        //  MOB - filter 
    }
    return grid;
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


extern EdiGrid *ediMakeGrid(cchar *str)
{
    //  MOB
    return 0;
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


void ediSetDbFlags(Edi *edi, int flags)
{
    //  MOB - should mask
    edi->flags = flags;
}


cchar *ediToString(cchar *fmt, EdiField field)
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


cchar *ediGetFieldAsString(Edi *edi, cchar *tableName, cchar *key, cchar *columnName, cchar *fmt, cchar *defaultValue)
{
    EdiField    field;
    int         type;

    //  MOB - should getField do a format?
    field = ediGetField(edi, tableName, key, columnName);
    if (!field.valid) {
        return defaultValue;
    }
    ediGetSchema(edi, tableName, columnName, &type, NULL);
    return ediToString(fmt, field);
}


MprList *ediGetGridColumns(EdiGrid *grid)
{
    return ediGetColumns(grid->edi, grid->tableName);
}


int ediGetRecFieldType(EdiRec *rec, cchar *fieldName)
{
    EdiField    field;

    field = ediGetRecField(rec, fieldName);
    if (!field.valid) {
        return 0;
    }
    return field.type;
}


//  MOB rethink position of fmt
cchar *ediGetRecFieldAsString(EdiRec *rec, cchar *fieldName, cchar *fmt)
{
    EdiField    field;

    field = ediGetRecField(rec, fieldName);
    if (!field.valid) {
        return 0;
    }
    return ediToString(fmt, field);
}


EdiField ediGetRecField(EdiRec *rec, cchar *fieldName)
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
