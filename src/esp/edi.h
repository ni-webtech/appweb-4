/*
    edi.h -- Embedded Database Interface (EDI).

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_EDI
#define _h_EDI 1

/********************************* Includes ***********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Forward Declarations **************************/

#if !DOXYGEN
#endif

/********************************** Tunables **********************************/

struct Edi;
struct EdiGrid;
struct EdiProvider;
struct EdiRec;
struct EdiValidation;

/*
   Field data type hints
 */
//  MOB DOC
#define EDI_TYPE_BLOB       1           /**< Arbitrary binary data */
#define EDI_TYPE_BOOL       2           /**< Boolean true|false value */
#define EDI_TYPE_DATE       3           /**< Date type */
#define EDI_TYPE_FLOAT      4           /**< Floating point number */
#define EDI_TYPE_INT        5           /**< Integer number */
#define EDI_TYPE_STRING     6           /**< String */
#define EDI_TYPE_TEXT       7           /**< Multi-line text */

/*
    Field flags
 */
#define EDI_AUTO_INC        0x1         /**< Field flag -- auto increments on new row */
#define EDI_KEY             0x2         /**< Field flag -- Column is the key */
#define EDI_INDEX           0x4         /**< Field flag -- Column is indexed */
 
/**
    Record field structure
    @ingroup Edi
  */
typedef struct EdiField {
    cchar           *value;             /**< Field value */
    cchar           *name;              /**< Field name */
    int             type:  8;           /**< Field type */
    int             valid: 8;           /**< Field validity. Set to true if valid */
    int             flags: 8;           /**< Field flags */
} EdiField;

/**
    Database record structure
    @ingroup Edi
 */
typedef struct EdiRec {
    struct Edi      *edi;               /**< Database handle */
    MprList         *errors;            /**< List of record errors */
    cchar           *tableName;         /**< Base table name for record */
    cchar           *id;                /**< Record key ID */
    int             nfields;            /**< Number of fields in record */
    EdiField        fields[MPR_FLEX];   /**< Field records */
} EdiRec;

/**
    Grid structure
    @ingroup Edi
 */
typedef struct EdiGrid {
    struct Edi      *edi;               /**< Database handle */
    cchar           *tableName;         /**< Base table name for grid */
    int             nrecords;           /**< Number of records in grid */
    EdiRec          *records[MPR_FLEX]; /**< Grid records */
} EdiGrid;

/*
    Field validation callback procedure
    @param vp Validation structure reference
    @param rec Record to validate
    @param fieldName Field name to validate
    @param value Field value to validate 
    @ingroup Edi
 */
typedef cchar *(*EdiValidationProc)(struct EdiValidation *vp, EdiRec *rec, cchar *fieldName, cchar *value);

/**
    Validation structure
    @ingroup Edi
 */
typedef struct EdiValidation {
    cchar               *name;          /**< Validation name */
    EdiValidationProc   vfn;            /**< Validation callback procedure */
    cvoid               *mdata;         /**< Non-GC (malloc) data */
    cvoid               *data;          /**< Allocated data that must be marked for GC */
} EdiValidation;

/*
    Database flags
 */
#define EDI_CREATE          0x1         /**< Create database if not present */
#define EDI_AUTO_SAVE       0x2         /**< Auto-save database if modified in memory */
#define EDI_NO_SAVE         0x4         /**< Prevent saving to disk */
#define EDI_LITERAL         0x8         /**< Literal schema in ediOpen source parameter */

#define EDI_SUPPRESS_SAVE   0x10        /**< Temporarily suppress auto-save */

/*
    Database flags
 */
#define EDI_NOSAVE      0x1             /* Don't save the database on modifications */

/**
    Database structure
    @defgroup Edi Edi
  */
typedef struct Edi {
    struct EdiProvider *provider;       /** Database provider */
    int             flags;              /**< Database flags */
} Edi;

/**
    Database provider interface
    @internal
 */
typedef struct EdiProvider {
    cchar     *name;
    int       (*addColumn)(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
    int       (*addIndex)(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
    int       (*addTable)(Edi *edi, cchar *tableName);
    int       (*addValidation)(Edi *edi, cchar *tableName, cchar *columnName, EdiValidation *vp);
    int       (*changeColumn)(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
    void      (*close)(Edi *edi);
    EdiRec    *(*createRec)(Edi *edi, cchar *tableName);
    int       (*delete)(Edi *edi, cchar *path);
    int       (*deleteRow)(Edi *edi, cchar *tableName, cchar *key);
    MprList   *(*getColumns)(Edi *edi, cchar *tableName);
    int       (*getColumnSchema)(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid);
    MprList   *(*getTables)(Edi *edi);
    int       (*getTableSchema)(Edi *edi, cchar *tableName, int *numRows, int *numCols);
    int       (*load)(Edi *edi, cchar *path);
    int       (*lookupField)(Edi *edi, cchar *tableName, cchar *fieldName);
    Edi       *(*open)(cchar *path, int flags);
    EdiGrid   *(*query)(Edi *edi, cchar *cmd);
    EdiField  (*readField)(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
    EdiRec    *(*readRec)(Edi *edi, cchar *tableName, cchar *key);
    EdiGrid   *(*readWhere)(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);
    int       (*removeColumn)(Edi *edi, cchar *tableName, cchar *columnName);
    int       (*removeIndex)(Edi *edi, cchar *tableName, cchar *indexName);
    int       (*removeTable)(Edi *edi, cchar *tableName);
    int       (*renameTable)(Edi *edi, cchar *tableName, cchar *newTableName);
    int       (*renameColumn)(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
    int       (*save)(Edi *edi);
    bool      (*validateRec)(Edi *edi, EdiRec *rec);
    int       (*writeField)(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
    int       (*writeRec)(Edi *edi, EdiRec *rec);
} EdiProvider;

/**
    Edi service control structure
    @internal
 */
typedef struct EdiService {
    MprHash    *providers;
    MprHash    *validations;
} EdiService;

//  MOB DOC
extern EdiService *ediCreateService();
extern void ediAddProvider(EdiProvider *provider);


/*
    EDI user API
 */
extern int ediAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
extern int ediAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);
extern int ediAddTable(Edi *edi, cchar *tableName);
extern int ediAddValidation(Edi *edi, cchar *name, cchar *tableName, cchar *columnName, cvoid *data);
extern int ediChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);
extern void ediClose(Edi *edi);
extern EdiRec *ediCreateRec(Edi *edi, cchar *tableName);
extern void ediDefineValidation(cchar *name, EdiValidationProc vfn);
extern int ediDelete(Edi *edi, cchar *path);
extern int ediDeleteRow(Edi *edi, cchar *tableName, cchar *key);
extern MprList *ediGetColumns(Edi *edi, cchar *tableName);
extern int ediGetColumnSchema(Edi *edi, cchar *tableName, cchar *columName, int *type, int *flags, int *cid);
extern MprList *ediGetTables(Edi *edi);
extern int ediLoad(Edi *edi, cchar *path);
extern int ediLookupField(Edi *edi, cchar *tableName, cchar *fieldName);
extern Edi *ediOpen(cchar *provider, cchar *source, int flags);
extern EdiGrid *ediQuery(Edi *edi, cchar *cmd);
extern cchar *ediReadField(Edi *edi, cchar *fmt, cchar *tableName, cchar *key, cchar *fieldName, cchar *defaultValue);
extern EdiRec *ediReadOneWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);
extern EdiField ediReadRawField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);
extern EdiRec *ediReadRec(Edi *edi, cchar *tableName, cchar *key);
extern EdiGrid *ediReadGrid(Edi *edi, cchar *tableName);
extern EdiGrid *ediReadWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);
extern int edRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);
extern int ediRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);
extern int ediRemoveTable(Edi *edi, cchar *tableName);
extern int ediRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);
extern int ediRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);
extern int ediSave(Edi *edi);
extern int ediGetTableSchema(Edi *edi, cchar *tableName, int *numRows, int *numCols);
extern EdiRec *ediUpdateField(EdiRec *rec, cchar *fieldName, cchar *value);
extern EdiRec *ediUpdateFields(EdiRec *rec, MprHash *params);
extern int ediValidate(Edi *edi, cchar *name, cchar *tableName, cchar *fieldName, cvoid *data);
extern bool ediValidateRec(Edi *edi, EdiRec *rec);
extern int ediWriteField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
extern int ediWriteFields(Edi *edi, cchar *tableName, MprHash *params);
extern int ediWriteRec(Edi *edi, EdiRec *rec);

/*
    Convenience Routines
 */
extern cchar *ediFormatField(cchar *fmt, EdiField value);
extern EdiField ediGetRawRecField(EdiRec *rec, cchar *fieldName);
extern MprList *ediGetRecErrors(EdiRec *rec);
extern cchar *ediGetRecField(cchar *fmt, EdiRec *rec, cchar *fieldName);
extern int ediGetRecFieldType(EdiRec *rec, cchar *fieldName);
extern MprList *ediGetGridColumns(EdiGrid *grid);
extern bool edValidateRecord(EdiRec *rec);


/*
    NO-DB API
 */
extern EdiGrid *ediMakeGrid(cchar *json);
extern EdiRec *ediMakeRec(cchar *json);

/*
    Support routines for providers
 */
extern EdiGrid *ediCreateBareGrid(Edi *edi, cchar *tableName, int nrows);
extern EdiRec *ediCreateBareRec(Edi *edi, cchar *tableName, int nfields);
extern char *ediGetTypeString(int type);
extern void ediManageEdiRec(EdiRec *rec, int flags);
extern int ediParseTypeString(cchar *type);
extern cchar *ediParseValue(cchar *value, int type);

#if BLD_FEATURE_MDB
extern void mdbInit();
#endif

#if BLD_FEATURE_SDB
extern void sdbInit();
#endif

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* BLD_FEATURE_ESP */
#endif /* _h_EDI */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http: *www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http: *www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

