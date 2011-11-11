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

/********************************** Defines ***********************************/

struct Edi;
struct EdiGrid;
struct EdiProvider;
struct EdiRec;
struct EdiValidation;

/**
    Edi service control structure
    @defgroup EdiService EdiService
 */
typedef struct EdiService {
    MprHash    *providers;
    MprHash    *validations;
} EdiService;

/**
    Create the EDI service
    @return EdiService object
    @ingroup EdiService
    @internal
 */
extern EdiService *ediCreateService();

/**
    Add a database provider. 
    @description This should only be called by database providers. 
    @ingroup EdiService
 */
extern void ediAddProvider(struct EdiProvider *provider);

/**
    Field validation callback procedure
    @param vp Validation structure reference
    @param rec Record to validate
    @param fieldName Field name to validate
    @param value Field value to validate 
    @ingroup EdiService
 */
typedef cchar *(*EdiValidationProc)(struct EdiValidation *vp, struct EdiRec *rec, cchar *fieldName, cchar *value);

/**
    Validation structure
    @ingroup EdiService
 */
typedef struct EdiValidation {
    cchar               *name;          /**< Validation name */
    EdiValidationProc   vfn;            /**< Validation callback procedure */
    cvoid               *mdata;         /**< Non-GC (malloc) data */
    cvoid               *data;          /**< Allocated data that must be marked for GC */
} EdiValidation;

/**
    Define a field validation procedure
    @param name Validation name
    @param vfn Validation callback to invoke when validating field data.
    @ingroup EdiService
 */
extern void ediDefineValidation(cchar *name, EdiValidationProc vfn);

/*
   Field data type hints
 */
#define EDI_TYPE_BINARY     1           /**< Arbitrary binary data */
#define EDI_TYPE_BOOL       2           /**< Boolean true|false value */
#define EDI_TYPE_DATE       3           /**< Date type */
#define EDI_TYPE_FLOAT      4           /**< Floating point number */
#define EDI_TYPE_INT        5           /**< Integer number */
#define EDI_TYPE_STRING     6           /**< String */
#define EDI_TYPE_TEXT       7           /**< Multi-line text */

/*
    Field flags
 */
#define EDI_AUTO_INC        0x1         /**< Field flag -- Automatic increments on new row */
#define EDI_KEY             0x2         /**< Field flag -- Column is the key */
#define EDI_INDEX           0x4         /**< Field flag -- Column is indexed */
 
/**
    EDI Record field structure
    @description The EdiField stores record field data and minimal schema information such as the data type and
        source column name.
    @defgroup EdiField EdiField
  */
typedef struct EdiField {
    cchar           *value;             /**< Field data value */
    cchar           *name;              /**< Field name. Sourced from the database column name  */
    int             type:  8;           /**< Field data type. Set to one of EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE
                                             EDI_TYPE_FLOAT, EDI_TYPE_INT, EDI_TYPE_STRING, EDI_TYPE_TEXT  */
    int             valid: 8;           /**< Field validity. Set to true if valid */
    int             flags: 8;           /**< Field flags. Flag mask set to EDI_AUTO_INC, EDI_KEY and/or EDI_INDEX */
} EdiField;

/**
    Database record structure
    @description Records may capture database row data, or may be free-standing without a backing database.
    @defgroup EdiRec EdiRec
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
    @description A grid is a tabular (grid) of rows and records.
        Grids may capture database table data, or may be free-standing without a backing database.
    @defgroup EdiGrid EdiGrid
 */
typedef struct EdiGrid {
    struct Edi      *edi;               /**< Database handle */
    cchar           *tableName;         /**< Base table name for grid */
    int             nrecords;           /**< Number of records in grid */
    EdiRec          *records[MPR_FLEX]; /**< Grid records */
} EdiGrid;

/*
    Database flags
 */
#define EDI_CREATE          0x1         /**< Create database if not present */
#define EDI_AUTO_SAVE       0x2         /**< Auto-save database if modified in memory */
#define EDI_NO_SAVE         0x4         /**< Prevent saving to disk */
#define EDI_LITERAL         0x8         /**< Literal schema in ediOpen source parameter */

#define EDI_SUPPRESS_SAVE   0x10        /**< Temporarily suppress auto-save */

#if UNUSED
/*
    Database flags
 */
#define EDI_NOSAVE      0x1             /**< ediOpen flag -- Don't save the database on modifications */
#endif

/**
    Database structure
    @description The Embedded Database Interface (EDI) defines an abstract interface atop various relational 
    database providers. Providers are supplied for SQLite and for the ESP Memory Database (MDB).
    @defgroup Edi Edi
  */
typedef struct Edi {
    struct EdiProvider *provider;       /**< Database provider */
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
    int       (*delete)(cchar *path);
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
    int       (*updateField)(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
    int       (*updateRec)(Edi *edi, EdiRec *rec);
    bool      (*validateRec)(Edi *edi, EdiRec *rec);
} EdiProvider;

/*************************** EDI Interface Wrappers **************************/
/**
    Add a column to a table
    @param edi Database handle
    @param tableName Database table name
    @param columnName Database column name
    @param type Column data type. Set to one of EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE
        EDI_TYPE_FLOAT, EDI_TYPE_INT, EDI_TYPE_STRING, EDI_TYPE_TEXT 
    @param flags Control column attributes. Set to a set of: EDI_AUTO_INC for auto incrementing columns, 
        EDI_KEY if the column is the key column and/or EDI_INDEX to create an index on the column.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediAddColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);

/**
    Add an index to a table
    @param edi Database handle
    @param tableName Database table name
    @param columnName Database column name
    @param indexName Ignored. Set to null.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediAddIndex(Edi *edi, cchar *tableName, cchar *columnName, cchar *indexName);

/**
    Add a table to a database
    @param edi Database handle
    @param tableName Database table name
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediAddTable(Edi *edi, cchar *tableName);

/**
    Add a validation
    @description Validations are run when calling ediUpdateRec. A validation is used to validate field data
        using builtin validators.
    @param edi Database handle
    @param name Validation name. Select from: 
        @arg boolean -- to validate field data as "true" or "false"
        @arg date -- to validate field data as a date or time.
        @arg format -- to validate field data against a regular expression supplied in the "data" argument
        @arg integer -- to validate field data as an integral value
        @arg number -- to validate field data as a number. It may be an integer or floating point number.
        @arg present -- to validate field data as not null.
        @arg unique -- to validate field data as being unique in the database table.
    @param tableName Database table name
    @param columnName Database column name
    @param data Argument data for the validator. For example: the "format" validator requires a regular expression.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediAddValidation(Edi *edi, cchar *name, cchar *tableName, cchar *columnName, cvoid *data);

/**
    Change a column schema definition
    @param edi Database handle
    @param tableName Database table name
    @param columnName Database column name
    @param type Column data type. Set to one of EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE
        EDI_TYPE_FLOAT, EDI_TYPE_INT, EDI_TYPE_STRING, EDI_TYPE_TEXT 
    @param flags Control column attributes. Set to a set of: EDI_AUTO_INC for auto incrementing columns, 
        EDI_KEY if the column is the key column and/or EDI_INDEX to create an index on the column.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediChangeColumn(Edi *edi, cchar *tableName, cchar *columnName, int type, int flags);

/**
    Close a database
    @param edi Database handle
    @ingroup Edi
 */
extern void ediClose(Edi *edi);

/**
    Create a record
    @description This will create a record using the given database tableName to supply the record schema. Use
        $ediCreateBareRec to create a free-standing record without requiring a database.
        The record is allocated and room is reserved to store record values. No record field values are stored.
    @param edi Database handle
    @param tableName Database table name
    @return Record instance.
    @ingroup Edi
 */
extern EdiRec *ediCreateRec(Edi *edi, cchar *tableName);


/**
    Delete the database at the given path.
    @param edi Database handle. This is required to identify the database provider. The database should be closed before
        deleting.
    @param path Database path name.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediDelete(Edi *edi, cchar *path);

//  MOB - should this be DeleteRec?
/**
    Delete a row in a database table
    @param edi Database handle
    @param tableName Database table name
    @param key Row key column value to delete.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediDeleteRow(Edi *edi, cchar *tableName, cchar *key);

/**
    Get a list of column names.
    @param edi Database handle
    @param tableName Database table name
    @return An MprList of column names in the given table.
    @ingroup Edi
 */
extern MprList *ediGetColumns(Edi *edi, cchar *tableName);

/**
    Get the column schema
    @param edi Database handle
    @param tableName Database table name
    @param columnName Database column name
    @param type Output parameter to receive the column data type. Will be set to one of:
        EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE, EDI_TYPE_FLOAT, EDI_TYPE_INT, EDI_TYPE_STRING, EDI_TYPE_TEXT.
        Set to null if this data is not required.
    @param flags Output parameter to receive the column control flags. Will be set to one or more of:
            EDI_AUTO_INC, EDI_KEY and/or EDI_INDEX 
        Set to null if this data is not required.
    @param cid Output parameter to receive the ordinal column index in the database table.
        Set to null if this data is not required.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediGetColumnSchema(Edi *edi, cchar *tableName, cchar *columnName, int *type, int *flags, int *cid);

/**
    Get a list of database tables.
    @param edi Database handle
    @return An MprList of table names in the database.
    @ingroup Edi
 */
extern MprList *ediGetTables(Edi *edi);

/**
    Load the database file.
    @param edi Database handle
    @param path Database path name
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
    @internal
 */
extern int ediLoad(Edi *edi, cchar *path);

//  MOB - should this be LookupColumn?
/**
    Lookup a field by name.
    @param edi Database handle
    @param tableName Database table name
    @param fieldName Database column name
    @return The ordinal field (column) index in the table.
    @ingroup Edi
 */
extern int ediLookupField(Edi *edi, cchar *tableName, cchar *fieldName);

/**
    Open a database.
    @description This opens a database using the specified database provider.
    @param source Database path name. If using the "mdb" provider with the EDI_LITERAL flag, then the source argument can
        be set to a literal JSON database content string.
    @param provider Database provider. Set to "mdb" for the Memory Database or "sqlite" for the SQLite provider.
    @param flags Set to:
        @arg EDI_CREATE  -- Create database if not present.
        @arg EDI_AUTO_SAVE -- Auto-save database if modified in memory. This option is only supported by the "mdb" provider.
        @arg EDI_NO_SAVE  -- Prevent saving to disk. This option is only supported by the "mdb" provider.
        @arg EDI_LITERAL -- Literal schema in ediOpen source parameter. This option is only supported by the "mdb" provider.
    @return If successful, returns an EDI database instance object. Otherwise returns zero.
    @ingroup Edi
 */
extern Edi *ediOpen(cchar *source, cchar *provider, int flags);

//  MOB - how do you get query errors back?  Should have an cchar *err argument.
/**
    Run a query.
    @description This runs a provider dependant query. For the SQLite provider, this runs an SQL statement.
    The "mdb" provider does not implement this API. To do queries using the "mdb" provider, use:
        $ediReadRec, $ediReadOneWhere, $ediReadWhere, $ediReadField and $ediReadTable.
    @param edi Database handle
    @param cmd Query command to execute.
    @return If succesful, returns tabular data in the form of an EgiGrid structure. Returns NULL on errors.
    @ingroup Edi
 */
extern EdiGrid *ediQuery(Edi *edi, cchar *cmd);

/**
    Read a field from the database and format the result.
    @description This reads a field from the database and formats the result using an optional format string.
        If the field has a null or empty value, the supplied defaultValue will be returned.
    @param edi Database handle
    @param fmt Printf style format string to use in formatting the result.
    @param tableName Database table name
    @param key Row key column value to read.
    @param fieldName Column name to read
    @param defaultValue Default value to return if the field is null or empty.
    @return Field value or default value if field is null or empty. Returns null if no matching record is found.
    @ingroup Edi
 */
extern cchar *ediReadField(Edi *edi, cchar *fmt, cchar *tableName, cchar *key, cchar *fieldName, cchar *defaultValue);

//  MOB - rename ReadRecWhere
/**
    Read one record.
    @description This runs a simple query on the database and selects the first matching record. The query selects
        a row that has a "field" that matches the given "value".
    @param edi Database handle
    @param tableName Database table name
    @param fieldName Database field name to evaluate
    @param operation Comparision operation. Set to "==", "!=", "<", ">", "<=" or ">=".
    @param value Data value to compare with the field values.
    @return First matching record. Returns NULL if no matching records.
    @ingroup Edi
 */
extern EdiRec *ediReadOneWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);

/**
    Read a field from the database.
    @description This reads a field from the database.
    @param edi Database handle
    @param tableName Database table name
    @param key Row key column value to read.
    @param fieldName Column name to read
    @return Field value or null if the no record is found. May return null or empty if the field is null or empty.
    @ingroup Edi
 */
extern EdiField ediReadRawField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName);

/**
    Read a record.
    @description Read a record from the given table as identified by the key value.
    @param edi Database handle
    @param tableName Database table name
    @param key Key value of the record to read 
    @return Record instance of EdiRec.
    @ingroup Edi
 */
extern EdiRec *ediReadRec(Edi *edi, cchar *tableName, cchar *key);

//  MOB - rename ReadGridWhere
/**
    Read matching records.
    @description This runs a simple query on the database and returns matching records in a grid. The query selects
        all rows that have a "field" that matches the given "value".
    @param edi Database handle
    @param tableName Database table name
    @param fieldName Database field name to evaluate
    @param operation Comparision operation. Set to "==", "!=", "<", ">", "<=" or ">=".
    @param value Data value to compare with the field values.
    @return A grid containing all matching records. Returns NULL if no matching records.
    @ingroup Edi
 */
extern EdiGrid *ediReadWhere(Edi *edi, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);

/**
    Read a table.
    @description This reads all the records in a table and returns a grid containing the results.
    @param edi Database handle
    @param tableName Database table name
    @return A grid containing all records. Returns NULL if no matching records.
    @ingroup Edi
 */
extern EdiGrid *ediReadTable(Edi *edi, cchar *tableName);

/**
    Remove a column from a table.
    @param edi Database handle
    @param tableName Database table name
    @param columnName Database column name
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int edRemoveColumn(Edi *edi, cchar *tableName, cchar *columnName);

/**
    Remove a table index.
    @param edi Database handle
    @param tableName Database table name
    @param indexName Ignored. Set to null. This call will remove the table index.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediRemoveIndex(Edi *edi, cchar *tableName, cchar *indexName);

/**
    Remove a table from the database.
    @param edi Database handle
    @param tableName Database table name
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediRemoveTable(Edi *edi, cchar *tableName);

/**
    Rename a table.
    @param edi Database handle
    @param tableName Database table name
    @param newTableName New database table name
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediRenameTable(Edi *edi, cchar *tableName, cchar *newTableName);

/**
    Rename a column. 
    @param edi Database handle
    @param tableName Database table name
    @param columnName Database column name
    @param newColumnName New column name
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediRenameColumn(Edi *edi, cchar *tableName, cchar *columnName, cchar *newColumnName);

/**
    Save in-memory database contents to disk.
    @description How this call behaves is provider dependant. If the provider is "mdb" and the database is not opened
        with AutoSave, then this call will save the in-memory contents. If the "mdb" database is opened with AutoSave,
        then this call will do nothing. For the "sdb" SQLite provider, this call does nothing.
    @param edi Database handle
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediSave(Edi *edi);

/**
    Set a record field without writing to the database.
    @description This routine updates the record object with the given value. The record will not be written
        to the database. To write to the database, use $ediUpdateRec.
    @param rec Record to update
    @param fieldName Record field name to update
    @param value Value to update
    @return The record instance if successful, otherwise NULL.
    @ingroup Edi
 */
extern EdiRec *ediSetField(EdiRec *rec, cchar *fieldName, cchar *value);

/**
    Set record fields without writing to the database.
    @description This routine updates the record object with the given values. The "data' argument supplies 
        a hash of fieldNames and values. The data hash may come from the request $params() or it can be manually
        created via #ediMakeHash to convert a JSON string into an options hash.
        For example: ediSetFields(rec, ediMakeHash("{ name: '%s', address: '%s' }", name, address))
        The record will not be written
        to the database. To write to the database, use $ediUpdateRec.
    @param rec Record to update
    @param data Hash of field names and values to use for the update
    @return The record instance if successful, otherwise NULL.
    @ingroup Edi
 */
extern EdiRec *ediSetFields(EdiRec *rec, MprHash *data);

/**
    Get table schema information.
    @param edi Database handle
    @param tableName Database table name
    @param numRows Output parameter to receive the number of rows in the table
        Set to null if this data is not required.
    @param numCols Output parameter to receive the number of columns in the table
        Set to null if this data is not required.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediGetTableSchema(Edi *edi, cchar *tableName, int *numRows, int *numCols);

/**
    Write a value to a database table field
    @description Update the value of a table field in the selected table row. Note: field validations are not run.
    @param edi Database handle
    @param tableName Database table name
    @param key Key value for the table row to update.
    @param fieldName Column name to update
    @param value Value to write to the database field
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediUpdateField(Edi *edi, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);

#if UNUSED
/**
    Write field values to a database row.
    @description This routine updates a database row with the given values.  The "data' argument supplies 
        a hash of fieldNames and values. The data hash may come from the request $params() or it can be manually
        created via #ediMakeHash to convert a JSON string into an options hash.
        For example: ediUpdateFields(rec, params());
        Note: field validations are not run.
    @param edi Database handle
    @param tableName Database table name
    @param data Hash of field names and values to use for the update
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediUpdateFields(Edi *edi, cchar *tableName, MprHash *data);
#endif

//  MOB - change Write to Update
/**
    Write a record to the database.
    @description If the record is a new record and the "id" column is EDI_AUTO_INC, then the "id" will be assigned
        prior to saving the record.
    @param edi Database handle
    @param rec Record to write to the database.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup Edi
 */
extern int ediUpdateRec(Edi *edi, EdiRec *rec);

/**
    Validate a record.
    @description Run defined field validations and return true if the record validates. Field validations are defined
        via $ediAddValidation calls. If any validations fail, error messages will be added to the record and can be 
        retrieved via $ediGetRecErrors.
    @param rec Record to validate
    @return True if all field valiations pass.
    @ingroup Edi
 */
extern bool ediValidateRec(EdiRec *rec);

/**************************** Convenience Routines ****************************/
/**
    Format a field value.
    @param fmt Printf style format string
    @param value Field value
    @return Formatted value string
    @ingroup Edi
 */
extern cchar *ediFormatField(cchar *fmt, EdiField value);

/**
    Get a record field
    @param rec Database record
    @param fieldName Field in the record to extract
    @return An EdiField structure containing the record field value and details.
    @ingroup Edi
 */
extern cchar *ediGetField(EdiRec *rec, cchar *fieldName);

/**
    Get and format a record field value.
    @param fmt Record field value
    @param rec Record to examine
    @param fieldName Record field to examine
    @return String value of the field
    @ingroup Edi
 */
extern cchar *ediGetFieldFmt(cchar *fmt, EdiRec *rec, cchar *fieldName);

/**
    Get the record field schema.
    @description This returns the actual EdiField which contains the field name, type, value and flags.
    @param rec Database record
    @param fieldName Field in the record to extract
    @return An EdiField structure containing the record field value and details.
    @ingroup Edi
 */
extern EdiField ediGetFieldSchema(EdiRec *rec, cchar *fieldName);

/**
    Get the data type of a record field.
    @param rec Record to examine
    @param fieldName Field to examine
    @return The field type. Returns one of: EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE, EDI_TYPE_FLOAT, 
        EDI_TYPE_INT, EDI_TYPE_STRING, EDI_TYPE_TEXT. 
    @ingroup Edi
 */
extern int ediGetFieldType(EdiRec *rec, cchar *fieldName);

/**
    Get record validation errors.
    @param rec Database record
    @return A list of validation errors. If validation passed, then this call returns NULL.
    @ingroup Edi
 */
extern MprList *ediGetRecErrors(EdiRec *rec);

/**
    Get a list of grid column names.
    @param grid Database grid
    @return An MprList of column names in the given grid.
    @ingroup Edi
 */
extern MprList *ediGetGridColumns(EdiGrid *grid);

/**
    Make a hash container of property values.
    @description This routine formats the given arguments, parses the result as a JSON string and returns an 
        equivalent hash of property values. The result after formatting should be of the form:
        ediMakeHash("{ key: 'value', key2: 'value', key3: 'value' }");
    @param fmt Printf style format string
    @param ... arguments
    @return MprHash instance
    @ingroup Edi
 */
extern MprHash *ediMakeHash(cchar *fmt, ...);

/**
    Make a grid.
    @description This call makes a free-standing data grid based on the JSON format content string.
    @param content JSON format content string. The content should be an array of objects where each object is a
        set of property names and values.
    @return An EdiGrid instance
    @example:
grid = ediMakeGrid("[ \\ \n
    { id: '1', country: 'Australia' }, \ \n
    { id: '2', country: 'China' }, \ \n
    ]");
    @ingroup Edi
 */
extern EdiGrid *ediMakeGrid(cchar *content);

/**
    Make a record.
    @description This call makes a free-standing data record based on the JSON format content string.
    @param content JSON format content string. The content should be a set of property names and values.
    @return An EdiRec instance
    @example: rec = ediMakeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
    @ingroup Edi
 */
extern EdiRec *ediMakeRec(cchar *content);

/**
    Create a bare grid.
    @description This creates an empty grid based on the given table's schema.
    @param edi Database handle
    @param tableName Database table name
    @param nrows Number of rows to reserve in the grid
    @return EdiGrid instance
    @ingroup Edi
 */
extern EdiGrid *ediCreateBareGrid(Edi *edi, cchar *tableName, int nrows);

/**
    Create a bare record.
    @description This creates an empty record based on the given table's schema.
    @param edi Database handle
    @param tableName Database table name
    @param nfields Number of fields to reserve in the record
    @return EdiGrid instance
    @ingroup Edi
 */
extern EdiRec *ediCreateBareRec(Edi *edi, cchar *tableName, int nfields);

/**
    Convert an EDI type to a string.
    @param type Column data type. Set to one of EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE
        EDI_TYPE_FLOAT, EDI_TYPE_INT, EDI_TYPE_STRING, EDI_TYPE_TEXT 
    @return Type string. This will be set to one of: "binary", "bool", "date", "float", "int", "string" or "text".
    @ingroup Edi
 */
extern char *ediGetTypeString(int type);

/**
    Parse an EDI type string.
    @param type Type string set to one of: "binary", "bool", "date", "float", "int", "string" or "text".
    @return Type code. Set to one of EDI_TYPE_BINARY, EDI_TYPE_BOOL, EDI_TYPE_DATE, EDI_TYPE_FLOAT, EDI_TYPE_INT, 
        EDI_TYPE_STRING, EDI_TYPE_TEXT.
    @ingroup Edi
 */
extern int ediParseTypeString(cchar *type);

/**
    Manage an EdiRec instance for garbage collection.
    @param rec Record instance
    @param flags GC management flag
    @ingroup Edi
    @internal
 */
extern void ediManageEdiRec(EdiRec *rec, int flags);

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
    You may use the GPL open source license described below, or you may acquire
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

