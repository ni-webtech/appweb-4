#include "ejs.h"

/******************************************************************************/
/* 
    This file is an amalgamation of all the individual source code files for the
    Embedthis Ejscript Manager Source.
  
    Catenating all the source into a single file makes embedding simpler and
    the resulting application faster, as many compilers can do whole file
    optimization.
  
    If you want to modify the product, you can still get the whole source as 
    individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "../../src/cmd/ejsmod.h"
 */
/************************************************************************/

/**
    ejsmod.h - Header for the ejsmod: module file list and slot generation program.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#ifndef _h_EJS_MOD
#define _h_EJS_MOD 1


/*
    Listing record structure
 */
typedef struct Lst {
    int         kind;                       /* Record kind */
    EjsModule   *module;                    /* Module holding class, function or property */
    EjsModule   *dependency;                /* Dependant module */
    EjsType     *type;                      /* Class reference */
    EjsObj      *owner;                     /* Owner (type, function, block) for the element */
    EjsFunction *fun;                       /* Relevant function */
    int         numProp;                    /* Number of properties */
    int         slotNum;                    /* Slot number */
    int         attributes;                 /* Property attributes */
    EjsString   *name;                      /* General name (used only for block name) */
    EjsName     typeName;                   /* Property type name */
    EjsName     qname;                      /* Qualified Property name */
} Lst;


/*
    Mod manager control structure
 */
typedef struct EjsMod {
    char        *currentLine;               /* Current input source code line */
    int         currentLineNumber;          /* Current input source line number */
    char        *currentFileName;           /* Current input file name */

    EjsService  *service;                   /* Ejs service manager */
    Ejs         *ejs;                       /* Interpreter handle */
    
    MprList     *lstRecords;                /* Listing records */
    MprList     *packages;                  /* List of packages */
    
    MprList     *blocks;                    /* List of blocks */
    EjsBlock    *currentBlock;              /* Current lexical block being read */

    int         cslots;                     /* Create C slot definitions */
    int         depends;                    /* Print module dependencies */
    int         error;                      /* Unresolved error */
    int         errorCount;                 /* Count of all errors */
    int         exitOnError;                /* Exit if module file errors are detected */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         firstGlobal;                /* First global to examine */
    int         genSlots;                   /* Set if either cslots || jsslots */
    int         listing;                    /* Generate listing file */
    int         memError;                   /* Memory error */
    int         showAsm;                    /* Show assembly bytes */
    int         verbosity;                  /* Verbosity level */
    int         warningCount;               /* Count of all warnings */
    int         warnOnError;                /* Warn if module file errors are detected */

    char        *docDir;                    /* Directory to generate HTML doc */
    bool        html;                       /* Generate HTML doc */
    bool        xml;                        /* Generate XML doc */
    
    char        *path;                      /* Current output file path */
    MprFile     *file;                      /* Current output file handle */
    EjsModule   *module;                    /* Current unit */
    EjsFunction *fun;                       /* Current function to disassemble */
    uchar       *pc;
} EjsMod;


/*
    Image ROM files
 */
typedef struct DocFile {
    cchar           *path;              /* File path */
    uchar           *data;              /* Pointer to file data */
    int             size;               /* Size of file */
    int             inode;              /* Not used */
} DocFile;

extern DocFile docFiles[];


extern void emListingLoadCallback(Ejs *ejs, int kind, ...);
extern void emnDocLoadCallback(Ejs *ejs, int kind, ...);
extern int  emCreateSlotFiles(EjsMod *mp, EjsModule *up, MprFile *file);
extern int  emCreateDoc(EjsMod *mp);

#endif /* _h_EJS_MOD */

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
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/ejsmod.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/cmd/doc.c"
 */
/************************************************************************/

/**
    doc.c - Documentation generator

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    Supported documentation keywords and format.

    The element "one liner" is the first sentance. Rest of description can continue from here and can include embedded html.

    @param argName Description          (Up to next @, case matters on argName)
    @default argName DefaultValue       (Up to next @, case matters on argName)
    @return Sentence                    (Can use return or returns. If sentance starts with lower case, then start 
                                            sentance with "Call returns".
    @event eventName Description        (Up to next @, case matters on eventName)
    @option argName Description         (Up to next @, case matters on argName)
    @throws ExceptionType Explanation   (Up to next @)
    @see Keyword keyword ...            (Case matters)
    @example Description                (Up to next @)
    @stability kind                     (prototype | evolving | stable | mature | deprecated]
    @deprecated version                 Same as @stability deprecated
    @requires ECMA                      (Emit: configuration requires --ejs-ecma)
    @spec                               (ecma-262, ecma-357, ejs-11)
    @hide                               (Hides this entry)
 */



/*
    Structures used when sorting lists
 */
typedef struct FunRec {
    EjsName         qname;
    EjsFunction     *fun;
    EjsObj          *obj;
    int             slotNum;
    EjsObj          *owner;
    EjsName         ownerName;
    EjsTrait        *trait;
} FunRec;

typedef struct ClassRec {
    EjsName         qname;
    EjsBlock        *block;
    int             slotNum;
    EjsTrait        *trait;
} ClassRec;

typedef struct PropRec {
    EjsName         qname;
    EjsObj          *obj;
    int             slotNum;
    EjsTrait        *trait;
    EjsObj          *vp;
} PropRec;

typedef struct List {
    char        *name;
    MprList     *list;
} List;

static Ejs *ejs;


static void      addUniqueItem(MprList *list, cchar *item);
static void      addUniqueClass(MprList *list, ClassRec *item);
static MprList   *buildClassList(EjsMod *mp, cchar *namespace);
static void      buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName ownerName);
static void      buildPropertyList(EjsMod *mp, MprList *list, EjsAny *obj, int numInherited);
static int       compareClasses(ClassRec **c1, ClassRec **c2);
static int       compareFunctions(FunRec **f1, FunRec **f2);
static int       compareProperties(PropRec **p1, PropRec **p2);
static int       compareStrings(EjsString **q1, EjsString **q2);
static int       compareNames(char **q1, char **q2);
static EjsDoc    *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName qname);
static MprFile   *createFile(EjsMod *mp, char *name);
static MprKeyValue *createKeyPair(MprChar *key, MprChar *value);
static cchar     *demangle(Ejs *ejs, EjsString *name);
static void      fixupDoc(Ejs *ejs, EjsDoc *doc);
static char      *fmtAccessors(int attributes);
static char      *fmtAttributes(EjsAny *vp, int attributes, int klass);
static char      *fmtClassUrl(Ejs *ejs, EjsName qname);
static char      *fmtDeclaration(Ejs *ejs, EjsName qname);
static char      *fmtNamespace(Ejs *ejs, EjsName qname);
static char      *fmtSpace(Ejs *ejs, EjsName qname);
static char      *fmtType(Ejs *ejs, EjsName qname);
static char      *fmtTypeReference(Ejs *ejs, EjsName qname);
static EjsString *fmtModule(Ejs *ejs, EjsString *name);
static MprChar   *formatExample(Ejs *ejs, EjsString *example);
static int       generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj, int instanceMethods);
static void      generateClassPage(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static void      generateClassPages(EjsMod *mp);
static void      generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static int       generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, MprList *properties);
static void      generateClassList(EjsMod *mp, cchar *namespace);
static void      generateContentFooter(EjsMod *mp);
static void      generateContentHeader(EjsMod *mp, cchar *fmt, ... );
static void      generateHomeFrameset(EjsMod *mp);
static void      generateHomeNavigation(EjsMod *mp) ;
static void      generateHomePages(EjsMod *mp);
static void      generateHomeTitle(EjsMod *mp);
static void      generateHtmlFooter(EjsMod *mp);
static void      generateHtmlHeader(EjsMod *mp, cchar *script, cchar *title, ... );
static void      generateImages(EjsMod *mp);
static void      generateOverview(EjsMod *mp);
static void      generateMethod(EjsMod *mp, FunRec *fp);
static void      generateMethodDetail(EjsMod *mp, MprList *methods);
static void      generateNamespace(EjsMod *mp, cchar *namespace);
static void      generateNamespaceClassTable(EjsMod *mp, cchar *namespace);
static int       generateNamespaceClassTableEntries(EjsMod *mp, cchar *namespace);
static void      generateNamespaceList(EjsMod *mp);
static void      generatePropertyTable(EjsMod *mp, EjsObj *obj);
static cchar    *getDefault(EjsDoc *doc, cchar *key);
static EjsDoc   *getDoc(Ejs *ejs, cchar *tag, void *block, int slotNum);
static EjsDoc   *getDuplicateDoc(Ejs *ejs, MprChar *duplicate);
static void      getKeyValue(MprChar *str, MprChar **key, MprChar **value);
static char     *getFilename(cchar *name);
static int       getPropertyCount(Ejs *ejs, EjsObj *obj);
static bool      match(MprChar *last, cchar *key);
static void      prepDocStrings(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static void      out(EjsMod *mp, char *fmt, ...);
static MprChar  *skipAtWord(MprChar *str);


int emCreateDoc(EjsMod *mp)
{
    ejs = mp->ejs;

    if (ejs->doc == 0) {
        ejs->doc = mprCreateHash(EJS_DOC_HASH_SIZE, 0);
        if (ejs->doc == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    generateImages(mp);
    generateClassPages(mp);
    generateHomePages(mp);
    return 0;
}


static void generateImages(EjsMod *mp)
{
    DocFile     *df;
    MprFile     *file;
    char        *path;
    int         rc;

    for (df = docFiles; df->path; df++) {
        path = mprJoinPath(mp->docDir, df->path);
        rc = mprMakeDir(mprGetPathDir(path), 0775, 1);
        file = mprOpenFile(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
        if (file == 0) {
            mprError("Can't create %s", path);
            mp->errorCount++;
            return;
        }
        if (mprWriteFile(file, df->data, df->size) != df->size) {
            mprError("Can't write to buffer");
            mp->errorCount++;
            return;
        }
        mprCloseFile(file);
    }
}


static void generateHomePages(EjsMod *mp)
{
    generateHomeFrameset(mp);
    generateHomeTitle(mp);
    generateHomeNavigation(mp);
    generateNamespaceList(mp);
    generateOverview(mp);
}


static void generateHomeFrameset(EjsMod *mp)
{
    cchar   *script;

    mprCloseFile(mp->file);
    mp->file = createFile(mp, "index.html");
    if (mp->file == 0) {
        return;
    }

    script = "function loaded() { content.location.href = '__overview-page.html'; }";
    generateHtmlHeader(mp, script, "Home");

    out(mp, "<frameset rows='90,*' border='0' onload='loaded()'>\n");
    out(mp, "   <frame src='title.html' name='title' scrolling='no' frameborder='0'>\n");
    out(mp, "   <frameset cols='200,*' border='2' framespacing='0'>\n");
    out(mp, "       <frame src='__navigation-left.html' name='navigation' scrolling='auto' frameborder='1'>\n");
    out(mp, "       <frame src='__overview-page.html' name='content' scrolling='auto' frameborder='1'>\n");
    out(mp, "   </frameset>\n");
    out(mp, "  <noframes><body><p>Please use a frames capable client to view this documentation.</p></body></noframes>");
    out(mp, "</frameset>\n");
    out(mp, "</html>\n");

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateHomeTitle(EjsMod *mp)
{
    mprCloseFile(mp->file);
    mp->file = createFile(mp, "title.html");
    if (mp->file == 0) {
        return;
    }
    generateHtmlHeader(mp, NULL, "title");

    out(mp,
        "<body>\n"
        "<div class=\"body\">\n"
        "   <div class=\"top\">\n"
        "       <map name=\"home\" id=\"home\">\n"
        "           <area coords=\"5,15,200,150\" href=\"index.html\" alt=\"doc\"/>\n"
        "       </map>\n"
        "   <div class=\"version\">%s %s</div>\n"
        "   <div class=\"menu\">\n"
        "       <a href=\"http://www.ejscript.org/\" target=\"_top\">Ejscript Home</a>\n"
        "       &gt; <a href=\"index.html\" class=\"menu\" target=\"_top\">Documentation Home</a>\n"
        "   </div>\n"
        "   <div class=\"search\">\n"
        "       <form class=\"smallText\" action=\"search.php\" method=\"post\" name=\"searchForm\" id=\"searchForm\"></form>&nbsp;\n"
        "       <input class=\"smallText\" type=\"text\" name=\"search\" align=\"right\" id=\"searchInput\" size=\"15\" \n"
        "           maxlength=\"50\" value=\"Search\"/>\n"
        "   </div>\n"
        "</div>\n", BLD_NAME, BLD_VERSION);

    generateHtmlFooter(mp);

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateHomeNavigation(EjsMod *mp)
{
    mprCloseFile(mp->file);
    mp->file = createFile(mp, "__navigation-left.html");
    if (mp->file == 0) {
        return;
    }

    generateHtmlHeader(mp, NULL, "Navigation");

    out(mp, "<frameset rows='34%%,*' border='1' framespacing='1'>\n");
    out(mp, "  <frame src='__all-namespaces.html' name='namespaces' scrolling='yes' />\n");
    out(mp, "  <frame src='__all-classes.html' name='classes' scrolling='yes' />\n");
    out(mp, "  <noframes><body><p>Please use a frames capable client to view this documentation.</p></body></noframes>");
    out(mp, "</frameset>\n");
    out(mp, "</html>\n");

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateNamespaceList(EjsMod *mp)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsName         qname;
    EjsDoc          *doc;
    MprList         *namespaces;
    cchar           *namespace;
    int             count, slotNum, next;

    ejs = mp->ejs;

    mp->file = createFile(mp, "__all-namespaces.html");
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }
    generateHtmlHeader(mp, NULL, "Namespaces");

    out(mp, "<body>\n");
    out(mp, "<div class='navigation'>\n");
    out(mp, "<h3>Namespaces</h3>\n");
    out(mp, "<table class='navigation' summary='namespaces'>\n");

    /*
        Build a sorted list of namespaces used by classes
     */
    namespaces = mprCreateList(0, 0);
    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0 || ejsStartsWithMulti(ejs, qname.space, "internal-")) {
            continue;
        }
        doc = getDoc(ejs, "class", ejs->global, slotNum);
        if (doc && !doc->hide) {
            addUniqueItem(namespaces, fmtNamespace(ejs, qname));
        }
    }
    mprSortList(namespaces, compareNames);

    out(mp, "<tr><td><a href='__all-classes.html' target='classes'>All Namespaces</a></td></tr>\n");

    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        out(mp, "<tr><td><a href='%s-classes.html' target='classes'>%s</a></td></tr>\n", namespace, namespace);
    }

    out(mp, "</table>\n");
    out(mp, "</div>\n");

    generateHtmlFooter(mp);
    mprCloseFile(mp->file);
    mp->file = 0;

    /*
        Generate namespace overviews and class list files for each namespace
     */
    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        generateNamespace(mp, namespace);
    }
    generateNamespace(mp, "__all");
}


static void generateNamespace(EjsMod *mp, cchar *namespace)
{
    Ejs     *ejs;
    char    *path;

    ejs = mp->ejs;

    path = sjoin(namespace, ".html", NULL);
    mp->file = createFile(mp, path);
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }
    if (strcmp(namespace, "__all") == 0) {
        generateContentHeader(mp, "All Namespaces");
        generateNamespaceClassTable(mp, namespace);
    } else {
        generateContentHeader(mp, "Namespace %s", namespace);
        generateNamespaceClassTable(mp, namespace);
    }
    generateContentFooter(mp);
    mprCloseFile(mp->file);
    mp->file = 0;

    /*
        Generate an overview page
     */
    generateClassList(mp, namespace);
}


static void generateNamespaceClassTable(EjsMod *mp, cchar *namespace)
{
    Ejs         *ejs;
    int         count;

    ejs = mp->ejs;

    out(mp, "<a name='Classes'></a>\n");

    if (strcmp(namespace, "__all") == 0) {
        out(mp, "<h2 class='classSection'>All Classes</h2>\n");
    } else {
        out(mp, "<h2 class='classSection'>%s Classes</h2>\n", namespace);
    }
    out(mp, "<table class='itemTable' summary='classes'>\n");
    out(mp, "   <tr><th>Class</th><th width='95%%'>Description</th></tr>\n");

    count = generateNamespaceClassTableEntries(mp, namespace);

    if (count == 0) {
        out(mp, "   <tr><td colspan='4'>No properties defined</td></tr>");
    }
    out(mp, "</table>\n\n");
}


/*
    Table of classes in the namespace overview page
 */
static int generateNamespaceClassTableEntries(EjsMod *mp, cchar *namespace)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsDoc          *doc;
    ClassRec        *crec;
    MprList         *classes;
    char            *fmtName;
    int             next;

    ejs = mp->ejs;

    classes = buildClassList(mp, namespace);

    for (next = 0; (crec = (ClassRec*) mprGetNextItem(classes, &next)) != 0; ) {
        qname = crec->qname;
        trait = crec->trait;
        fmtName = fmtType(ejs, crec->qname);
        out(mp, "   <tr><td><a href='%s' target='content'>%@</a></td>", getFilename(fmtName), qname.name);
        if (crec->block == ejs->global && mp->firstGlobal == ejsGetPropertyCount(ejs, ejs->global)) {
            continue;
        }
        doc = getDoc(ejs, "class", crec->block ? crec->block : ejs->global, crec->slotNum);
        if (doc && !doc->hide) {
            out(mp, "<td>%w</td></tr>\n", doc->brief);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
    }
    return mprGetListLength(classes);
}


static MprList *buildClassList(EjsMod *mp, cchar *namespace)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsDoc          *doc;
    EjsName         qname;
    ClassRec        *crec;
    MprList         *classes;
    int             count, slotNum;

    ejs = mp->ejs;

    /*
        Build a sorted list of classes
     */
    classes = mprCreateList(0, 0);
    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        doc = getDoc(ejs, "class", ejs->global, slotNum);
        if (doc == 0 || doc->hide) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0) {
            continue;
        }
        if (strcmp(namespace, "__all") != 0 && strcmp(namespace, fmtNamespace(ejs, qname)) != 0) {
            continue;
        }

        /* Suppress the core language types (should not appear as classes) */

        if (ejsCompareMulti(ejs, qname.space, EJS_EJS_NAMESPACE) == 0) {
            if (ejsCompareMulti(ejs, qname.name, "int") == 0 || 
                ejsCompareMulti(ejs, qname.name, "long") == 0 || ejsCompareMulti(ejs, qname.name, "decimal") == 0 ||
                ejsCompareMulti(ejs, qname.name, "boolean") == 0 || ejsCompareMulti(ejs, qname.name, "double") == 0 || 
                ejsCompareMulti(ejs, qname.name, "string") == 0) {
                continue;
            }
        }
        /* Other fixups */
        if (ejsStartsWithMulti(ejs, qname.space, "internal") || ejsCompareMulti(ejs, qname.space, "private") == 0) {
            continue;
        }
        crec = mprAlloc(sizeof(ClassRec));
        crec->qname = qname;
        crec->trait = trait;
        crec->block = ejs->global;
        crec->slotNum = slotNum;
        addUniqueClass(classes, crec);
    }

    /*
        Add a special type "Global"
     */
    if (strcmp(namespace, "__all") == 0) {
        if (mp->firstGlobal < ejsGetPropertyCount(ejs, ejs->global)) {
            crec = mprAlloc(sizeof(ClassRec));
            crec->qname = N(EJS_EJS_NAMESPACE, EJS_GLOBAL);
            crec->block = ejs->global;
            crec->slotNum = ejsLookupProperty(ejs, ejs->global, crec->qname);
            addUniqueClass(classes, crec);
        }
    }
    mprSortList(classes, compareClasses);
    return classes;
}


static void generateClassList(EjsMod *mp, cchar *namespace)
{
    Ejs         *ejs;
    MprList     *classes;
    ClassRec    *crec;
    cchar       *className, *fmtName;
    char        *path, script[MPR_MAX_STRING], *cp;
    int         next;

    ejs = mp->ejs;

    path = sjoin(namespace, "-classes.html", NULL);
    mp->file = createFile(mp, path);
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }

    /*
        Create the header and auto-load a namespace overview. We do this here because the class list is loaded
        when the user selects a namespace.
     */
    mprSprintf(script, sizeof(script), "parent.parent.content.location = \'%s.html\';", namespace);
    generateHtmlHeader(mp, script, "%s Class List", namespace);

    out(mp, "<body>\n");
    out(mp, "<div class='navigation'>\n");

    if (strcmp(namespace, "__all") == 0) {
        out(mp, "<h3>All Classes</h3>\n");
    } else {
        out(mp, "<h3>%s Classes</h3>\n", namespace);
    }
    out(mp, "<table class='navigation' summary='classList'>\n");

    classes = buildClassList(mp, namespace);

    for (next = 0; (crec = (ClassRec*) mprGetNextItem(classes, &next)) != 0; ) {
        /*
            Strip namespace portion
         */
        fmtName = fmtType(ejs, crec->qname);
        if ((cp = strrchr(fmtName, '.')) != 0) {
            className = ++cp;
        } else {
            className = fmtName;
        }
        if ((cp = strrchr(className, ':')) != 0) {
            className = ++cp;
        }
        out(mp, "<tr><td><a href='%s' target='content'>%s</a></td></tr>\n", getFilename(fmtName), className);
    }

    out(mp, "</table>\n");
    out(mp, "</div>\n");
    out(mp, "&nbsp;<br/>");

    generateHtmlFooter(mp);
    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateOverview(EjsMod *mp)
{
    mprCloseFile(mp->file);
    mp->file = createFile(mp, "__overview-page.html");
    if (mp->file == 0) {
        mp->errorCount++;
        return;
    }
    generateContentHeader(mp, "Overview");

    out(mp, "<h1>%s %s</h1>", BLD_NAME, BLD_VERSION);
    out(mp, "<p>Embedthis Ejscript is an implementation of the Javascript (ECMA 262) language.</p>");
    out(mp, "<p>See <a href='http://www.ejscript.org' target='new'>http://www.ejscript.org</a> for "
        "product details and downloads.</p>");
    out(mp, "<h2>Documentation Conventions</h2>");
    out(mp, "<p>APIs are grouped into Namespaces for logical ordering. Within each namespace, classes, methods "
        "and properties are defined. For each method parameters, events and options are described.</p>");
    out(mp, "<h4>Default Values</h4>");
    out(mp, "<p>Method parameters can take default values if an actual parameter is not provided when calling the API. "
        "The default value is listed in the method signature in the form \"name: Type = defaultValue\". The default "
        "value is also listed in the <em>Parameters</em> section.</p>");

    generateContentFooter(mp);

    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateHtmlHeader(EjsMod *mp, cchar *script, cchar *fmt, ... )
{
    char        *title;
    va_list     args;

    va_start(args, fmt);
    title = mprAsprintfv(fmt, args);
    va_end(args);

    /*
        Header + Style sheet
     */
    out(mp, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
    out(mp, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
    out(mp, "<head>\n   <title>%s</title>\n\n", title);

    out(mp, "   <link rel=\"stylesheet\" type=\"text/css\" href=\"doc.css\" />\n");

    if (script) {
        out(mp, "   <script type=\"text/javascript\">\n      %s\n   </script>\n", script);
    }
    out(mp, "</head>\n\n");
}


static void generateContentHeader(EjsMod *mp, cchar *fmt, ... )
{
    va_list     args;
    char        *title;

    va_start(args, fmt);
    title = mprAsprintfv(fmt, args);
    va_end(args);

    generateHtmlHeader(mp, NULL, title);
    
    out(mp, "<body>\n<div class='body'>\n\n");
    out(mp, "<div class=\"content\">\n\n");
}


static void generateTerms(EjsMod *mp)
{
    out(mp,
        "<div class=\"terms\">\n"
        "   <p class=\"terms\">\n"
        "       <a href=\"http://www.embedthis.com/\">"
        "       Embedthis Software LLC, 2003-2011. All rights reserved. "
        "Embedthis is a trademark of Embedthis Software LLC.</a>\n"
        "   </p>\n"
        "</div>");
}


static void generateHtmlFooter(EjsMod *mp)
{
    out(mp, "</body>\n</html>\n");
}


static void generateContentFooter(EjsMod *mp)
{
    generateTerms(mp);
    out(mp, "</div>\n");
    out(mp, "</div>\n");
    generateHtmlFooter(mp);
}


static MprFile *createFile(EjsMod *mp, char *name)
{
    MprFile *file;
    char    *path;

    path = mp->path = mprJoinPath(mp->docDir, name);
    file = mprOpenFile(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == 0) {
        mprError("Can't open %s", path);
        mp->errorCount++;
        return 0;
    }
    return file;
}


/*
    Generate one page per class/type
 */
static void generateClassPages(EjsMod *mp)
{
    Ejs         *ejs;
    EjsType     *type;
    EjsTrait    *trait;
    EjsDoc      *doc;
    EjsName     qname;
    char        key[32];
    int         count, slotNum;

    ejs = mp->ejs;

    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = mp->firstGlobal; slotNum < count; slotNum++) {
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0 || ejsStartsWithMulti(ejs, qname.space, "internal-")) {
            continue;
        }
        /*
            Setup the output path, but create the file on demand when output is done
         */
        mprCloseFile(mp->file);
        mp->file = 0;
        mp->path = mprJoinPath(mp->docDir, getFilename(fmtType(ejs, type->qname)));
        if (mp->path == 0) {
            return;
        }
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        doc = getDoc(ejs, "class", ejs->global, slotNum);

        if (doc && !doc->hide) {
            generateClassPage(mp, (EjsObj*) type, qname, trait, doc);
        }
        mprCloseFile(mp->file);
        mp->file = 0;
    }

    /*
        Finally do one page specially for "global"
        TODO - Functionalize
     */
    trait = mprAlloc(sizeof(EjsTrait));
    doc = mprAlloc(sizeof(EjsDoc));
    doc->docString = ejsCreateStringFromAsc(ejs, "Global object containing all global functions and variables.");
    doc->returns = doc->example = doc->description = NULL;
    doc->trait = trait;

    mprSprintf(key, sizeof(key), "%Lx %d", PTOL(0), 0);
    mprAddKey(ejs->doc, key, doc);

    slotNum = ejsGetPropertyCount(ejs, ejs->global);

    qname = N(EJS_EJS_NAMESPACE, EJS_GLOBAL);
    mp->file = createFile(mp, getFilename(fmtType(ejs, qname)));
    if (mp->file == 0) {
        return;
    }
    generateClassPage(mp, ejs->global, qname, trait, doc);
    mprCloseFile(mp->file);
    mp->file = 0;
}


static void generateClassPage(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc)
{
    MprList     *methods;
    int         count;

    prepDocStrings(mp, obj, name, trait, doc);
    if (doc->hide) {
        return;
    }
    generateClassPageHeader(mp, obj, name, trait, doc);
    generatePropertyTable(mp, obj);
    
    methods = mprCreateList(0, 0);
    buildMethodList(mp, methods, obj, obj, name);
    if (ejsIsType(ejs, obj)) {
        buildMethodList(mp, methods, (EjsObj*) ((EjsType*) obj)->prototype, obj, name);
    }
    count = generateMethodTable(mp, methods, obj, 0);
    count += generateMethodTable(mp, methods, obj, 1);
    if (count > 0) {
        generateMethodDetail(mp, methods);
    }
    generateContentFooter(mp);
}


static void prepDocStrings(EjsMod *mp, EjsObj *obj, EjsName qname, EjsTrait *typeTrait, EjsDoc *doc)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsPot          *prototype;
    EjsName         pname;
    EjsDoc          *dp;
    char            *combined;
    int             slotNum, numProp, numInherited;

    ejs = mp->ejs;

    if (doc) {
        crackDoc(mp, doc, qname);
    }
    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;

    if (type && type->hasConstructor) {
        slotNum = ejsLookupProperty(ejs, ejs->global, type->qname);
        dp = getDoc(ejs, "fun", ejs->global, slotNum);
        if (dp) {
            crackDoc(mp, dp, type->qname);
        }
    }

    /*
        Loop over all the static properties
     */
    numProp = ejsGetPropertyCount(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        if (trait == 0) {
            continue;
        }
        dp = getDoc(ejs, NULL, obj, slotNum);
        if (dp) {
            pname = ejsGetPropertyName(ejs, obj, slotNum);
            combined = mprAsprintf("%S.%S", qname.name, pname.name);
            crackDoc(mp, dp, EN(combined)); 
        }
    }

    /*
        Loop over all the instance properties
     */
    if (type) {
        prototype = type->prototype;
        if (prototype) {
            numInherited = type->numInherited;
            if (ejsGetPropertyCount(ejs, prototype) > 0) {
                for (slotNum = numInherited; slotNum < prototype->numProp; slotNum++) {
                    trait = ejsGetPropertyTraits(ejs, prototype, slotNum);
                    if (trait == 0) {
                        continue;
                    }
                    doc = getDoc(ejs, NULL, prototype, slotNum);
                    if (doc) {
                        pname = ejsGetPropertyName(ejs, (EjsObj*) prototype, slotNum);
                        crackDoc(mp, doc, qname);
                    }
                }
            }
        }
    }
}


static void generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName qname, EjsTrait *trait, EjsDoc *doc)
{
    Ejs         *ejs;
    EjsType     *t, *type;
    EjsString   *modName;
    cchar       *see, *namespace;
    int         next, count;

    ejs = mp->ejs;

    mprAssert(ejsIsBlock(ejs, obj));

    if (!ejsIsType(ejs, obj)) {
        generateContentHeader(mp, "Global Functions and Variables");
        out(mp, "<a name='top'></a>\n");
        out(mp, "<h1 class='className'>Global Functions and Variables</h1>\n");
    } else {
        generateContentHeader(mp, "Class %S", qname.name);
        out(mp, "<a name='top'></a>\n");
        out(mp, "<h1 class='className'>%S</h1>\n", qname.name);
    }
    out(mp, "<div class='classBlock'>\n");

    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        out(mp, "<table class='classHead' summary='%S'>\n", qname.name);

        if (type && type->module) {
            modName = fmtModule(ejs, type->module->name);
            out(mp, "   <tr><td><strong>Module</strong></td><td>%S</td></tr>\n", modName);
        }
        namespace = fmtNamespace(ejs, qname);
        out(mp, "   <tr><td><strong>Definition</strong></td><td>%s class %@</td></tr>\n", 
            fmtAttributes(obj, trait->attributes, 1), qname.name);

        if (type && type->baseType) {
            out(mp, "   <tr><td><strong>Inheritance</strong></td><td>%@", qname.name);
            for (t = type->baseType; t; t = t->baseType) {
                out(mp, " <img src='images/inherit.gif' alt='inherit'/> %s", fmtTypeReference(ejs, t->qname));
            }
        }
        if (doc) {
            if (doc->requires) {
                out(mp, "<tr><td><strong>Requires</strong></td><td>configure --ejs-%s</td></tr>\n", doc->requires);
            }
            if (doc->spec) {
                out(mp, "<tr><td><strong>Specified</strong></td><td>%s</td></tr>\n", doc->spec);
            }
            if (doc->stability) {
                out(mp, "<tr><td><strong>Stability</strong></td><td>%s</td></tr>\n", doc->stability);
            }
            if (doc->example) {
                out(mp, "<tr><td><strong>Example</strong></td><td><pre>%s</pre></td></tr>\n", doc->example);
            }
        }
        out(mp, "       </td></tr>\n");
        out(mp, "</table>\n\n");
    }
    if (doc) {
        out(mp, "<p class='classBrief'>%s</p>\n\n", doc->brief);
        if (doc->description) {
            out(mp, "<p class='classDescription'>%s</p>\n\n", doc->description);
        }

        count = mprGetListLength(doc->see);
        if (count > 0) {
            out(mp, "<h3>See Also</h3><p class='detail'>\n");
            for (next = 0; (see = mprGetNextItem(doc->see, &next)) != 0; ) {
                out(mp, "<a href='%s'>%s</a>%s\n", getFilename(see), see, (count == next) ? "" : ", ");
            }
            out(mp, "</p>\n");
        }
    }
    out(mp, "</div>\n\n\n");
    out(mp, "<hr />\n");
}


static int getPropertyCount(Ejs *ejs, EjsObj *obj)
{
    EjsObj      *vp;
    EjsPot      *prototype;
    EjsTrait    *trait;
    EjsType     *type;
    int         limit, count, slotNum;

    count = 0;

    limit = ejsGetPropertyCount(ejs, obj);
    for (slotNum = 0; slotNum < limit; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        if (vp) {
            trait = ejsGetPropertyTraits(ejs, obj, slotNum);
            if (trait && trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
                count++;
            } else if (!ejsIsFunction(ejs, vp)) {
                count++;
            }
        }
    }
    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            prototype = type->prototype;
            limit = ejsGetPropertyCount(ejs, prototype);
            for (slotNum = 0; slotNum < limit; slotNum++) {
                vp = ejsGetProperty(ejs, prototype, slotNum);
                if (vp && !ejsIsFunction(ejs, vp)) {
                    count++;
                }
            }
        }
    }
    return count;
}


static void generatePropertyTable(EjsMod *mp, EjsObj *obj)
{
    Ejs         *ejs;
    EjsType     *type;
    MprList     *list;
    int         count;

    ejs = mp->ejs;

    list = mprCreateList(0, 0);
    buildPropertyList(mp, list, obj, 0);

    type = 0;
    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            buildPropertyList(mp, list, type->prototype, type->numInherited);
        }
    }
    mprSortList(list, compareProperties);

    out(mp, "<a name='Properties'></a>\n");
    out(mp, "<h2 class='classSection'>Properties</h2>\n");

    if (mprGetListLength(list) > 0) {
        out(mp, "<table class='itemTable' summary='properties'>\n");
        out(mp, "   <tr><th>Qualifiers</th><th>Property</th><th>Type</th><th width='95%%'>Description</th></tr>\n");
        count = generateClassPropertyTableEntries(mp, obj, list);
        out(mp, "</table>\n\n");
    } else {
        // UNUSED out(mp, "   <tr><td colspan='4'>No properties defined</td></tr>");
        out(mp, "   <p>(No own properties defined)</p>");
    }

    if (type && type->baseType) {
        count = getPropertyCount(ejs, (EjsObj*) type->baseType);
        if (count > 0) {
            out(mp, "<p class='inheritedLink'><a href='%s#Properties'><i>Inherited Properties</i></a></p>\n\n",
                fmtClassUrl(ejs, type->baseType->qname));
        }
    }
    out(mp, "<hr />\n");
}


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static void buildPropertyList(EjsMod *mp, MprList *list, EjsAny *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    PropRec         *prec;
    int             start, slotNum, numProp;

    ejs = mp->ejs;

    /*
        Loop over all the (non-inherited) properties
     */
    start = (obj == ejs->global) ? mp->firstGlobal : numInherited;
    numProp = ejsGetPropertyCount(ejs, obj);
    for (slotNum = start; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (trait) {
            if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
                doc = getDoc(ejs, "fun", obj, slotNum);
            } else {
                doc = getDoc(ejs, NULL, obj, slotNum);
            }
            if (doc && doc->hide) {
                continue;
            }
        }
        if (vp == 0 || ejsIsType(ejs, vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (ejsIsFunction(ejs, vp) && !(trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER))) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_PRIVATE_NAMESPACE) == 0 || 
            ejsContainsMulti(ejs, qname.space, ",private]")) {
            continue;
        }
        prec = mprAlloc(sizeof(PropRec));
        prec->qname = qname;
        prec->obj = obj;
        prec->slotNum = slotNum;
        prec->trait = trait;
        prec->vp = vp;
        mprAddItem(list, prec);
    }
}


#if UNUSED
static void buildGetterList(EjsMod *mp, MprList *list, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    PropRec         *prec;
    MprList         *list;
    int             slotNum, numProp;

    ejs = mp->ejs;
    list = mprCreateList(0, 0);

    /*
        Loop over all the (non-inherited) properties
     */
    if (obj == ejs->global) {
        slotNum = mp->firstGlobal;
    } else {
        slotNum = numInherited;
    }
    numProp = ejsGetPropertyCount(ejs, obj);
    for (; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        printf("Name %s\n", qname.name->value);
        if (!ejsIsFunction(ejs, vp)) {
            continue;
        }
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        if (trait) {
            if (!(trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER))) {
                continue;
            }
            doc = getDoc(ejs, "fun", obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (vp == 0 || ejsIsType(ejs, vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_PRIVATE_NAMESPACE) == 0 || 
                ejsContainsMulti(ejs, qname.space, ",private]")) {
            continue;
        }
        prec = mprAlloc(sizeof(PropRec));
        prec->qname = qname;
        prec->obj = obj;
        prec->slotNum = slotNum;
        prec->trait = trait;
        prec->vp = vp;
        mprAddItem(list, prec);
    }
}
#endif


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static int generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, MprList *properties)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    EjsFunction     *fun;
    PropRec         *prec;
    cchar           *tname;
    int             slotNum, count, next, attributes;

    ejs = mp->ejs;
    count = 0;

    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;

    for (next = 0; (prec = (PropRec*) mprGetNextItem(properties, &next)) != 0; ) {
        vp = prec->vp;
        trait = prec->trait;
        slotNum = prec->slotNum;
        qname = prec->qname;
#if UNUSED
        if (trait->attributes & EJS_TRAIT_SETTER) {
            if (trait->attributes & EJS_TRAIT_GETTER) {
                /* Setter with getter is suppressed - only need getter */
                continue;
            }
        }
#endif
        if (ejsStartsWithMulti(ejs, qname.space, "internal") || ejsContainsMulti(ejs, qname.space, "private")) {
            continue;
        }
#if UNUSED
        if (trait->attributes & EJS_TRAIT_SETTER && ejsStartsWithMulti(ejs, qname.name, "set-")) {
            name = &qname.name->value[4];
            if (isalpha((int) name[0])) {
                out(mp, "<a name='%w'></a>\n", name);
            }
        } else if (isalpha((int) qname.name->value[0])) {
            out(mp, "<a name='%@'></a>\n", qname.name);
        }
#else
        if (isalpha((int) qname.name->value[0])) {
            out(mp, "<a name='%@'></a>\n", qname.name);
        }
#endif
        attributes = trait->attributes;
        if (type && qname.space == type->qname.space) {
            out(mp, "   <tr><td nowrap align='center'>%s</td><td>%@</td>", 
                fmtAttributes(vp, attributes, 0), qname.name);
        } else {
            out(mp, "   <tr><td nowrap align='center'>%s %s</td><td>%@</td>", fmtNamespace(ejs, qname),
                fmtAttributes(vp, attributes, 0), qname.name);
        }
        if (trait->attributes & EJS_TRAIT_GETTER) {
            fun = (EjsFunction*) vp;
            if (fun->resultType) {
                tname = fmtType(ejs, fun->resultType->qname);
                if (scasecmp(tname, "intrinsic::Void") == 0) {
                    out(mp, "<td>&nbsp;</td>");
                } else {
                    out(mp, "<td>%s</td>", fmtTypeReference(ejs, fun->resultType->qname));
                }
            } else {
                out(mp, "<td>&nbsp;</td>");
            }
        } else if (trait->type) {
            out(mp, "<td>%s</td>", fmtTypeReference(ejs, trait->type->qname));
        } else {
            out(mp, "<td>&nbsp;</td>");
        }
        doc = getDoc(ejs, NULL, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description ? doc->description : "");
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}


#if UNUSED
static int generateClassGetterTableEntries(EjsMod *mp, EjsObj *obj, MprList *getters)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsFunction     *fun;
    EjsDoc          *doc;
    PropRec         *prec;
    MprChar         *name;
    cchar           *set, *tname;
    int             slotNum, count, next;

    ejs = mp->ejs;
    count = 0;

    for (next = 0; (prec = (PropRec*) mprGetNextItem(getters, &next)) != 0; ) {
        fun = (EjsFunction*) prec->vp;

        trait = prec->trait;
        slotNum = prec->slotNum;
        qname = prec->qname;

        set = "";
        if (trait->attributes & EJS_TRAIT_SETTER) {
            if (trait->attributes & EJS_TRAIT_GETTER) {
                continue;
            }
        }
        if (ejsStartsWithMulti(ejs, qname.space, "internal") || ejsCompareMulti(ejs, qname.space, "private") == 0) {
            continue;
        }
        if (ejsStartsWithMulti(ejs, qname.name, "set-")) {
            name = &qname.name->value[4];
            if (isalpha((int) name[0])) {
                out(mp, "<a name='%w'></a>\n", name);
            }
        } else {
            name = qname.name->value;
            if (isalpha((int) name[0])) {
                out(mp, "<a name='%s'></a>\n", name);
            }
        }
        out(mp, "   <tr><td nowrap align='left'>%s %s%s</td><td>%s</td>", fmtNamespace(ejs, qname),
            fmtAttributes(fun, trait->attributes, 0), set, name);

        if (fun->resultType) {
            tname = fmtType(ejs, fun->resultType->qname);
            if (scasecmp(tname, "intrinsic::Void") == 0) {
                out(mp, "<td>&nbsp;</td>");
            } else {
                out(mp, "<td>%s</td>", fmtTypeReference(ejs, fun->resultType->qname));
            }
        } else {
            out(mp, "<td>&nbsp;</td>");
        }
        doc = getDoc(ejs, NULL, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description ? doc->description : "");
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}
#endif


static void buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName ownerName)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsFunction     *fun;
    EjsDoc          *doc;
    EjsType         *type;
    FunRec          *fp;
    int             slotNum, numProp, count, numInherited;

    ejs = mp->ejs;
    if (ejsIsType(ejs, owner) && !ejsIsPrototype(ejs, obj) && ((EjsType*) owner)->hasConstructor) {
        type = (EjsType*) owner;
        slotNum = ejsLookupProperty(ejs, ejs->global, ownerName);
        mprAssert(slotNum >= 0);
        fp = mprAlloc(sizeof(FunRec));
        fp->fun = (EjsFunction*) type;
        fp->obj = ejs->global;
        fp->slotNum = slotNum;
        fp->owner = ejs->global;
        fp->ownerName = type->qname;
        fp->qname = type->qname;
        fp->trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (fp->trait) {
            doc = getDoc(ejs, "fun", ejs->global, slotNum);
            if (doc && !doc->hide) {
                mprAddItem(methods, fp);
            }
        }
    }

    numProp = ejsGetPropertyCount(ejs, obj);

    numInherited = 0;
    if (ejsIsPrototype(ejs, obj)) {
        numInherited = ((EjsType*) owner)->numInherited;
    }
    slotNum = (obj == ejs->global) ? mp->firstGlobal : numInherited;

    for (count = 0; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (ejsIsType(ejs, vp)) {
            doc = getDoc(ejs, "class", obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        fun = (EjsFunction*) vp;
        if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
            continue;
        }
        if (trait) {
            doc = getDoc(ejs, "fun", obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        if (vp == 0 || !ejsIsFunction(ejs, vp) || qname.name == 0 || trait == 0) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }
        if (ejsCompareMulti(ejs, qname.space, EJS_PRIVATE_NAMESPACE) == 0 || 
                ejsContainsMulti(ejs, qname.space, ",private]")) {
            continue;
        }
        if (ejsStartsWithMulti(ejs, qname.space, "internal")) {
            continue;
        }
        fp = mprAlloc(sizeof(FunRec));
        fp->fun = fun;
        fp->obj = obj;
        fp->slotNum = slotNum;
        fp->owner = owner;
        fp->ownerName = ownerName;
        fp->qname = qname;
        fp->trait = trait;
        mprAddItem(methods, fp);
    }
    mprSortList(methods, compareFunctions);
}


static int generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj, int instanceMethods)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait, *argTrait;
    EjsName         qname, argName;
    EjsDoc          *doc;
    EjsFunction     *fun;
    FunRec          *fp;
    cchar           *defaultValue;
    int             i, count, next, emitTable;

    ejs = mp->ejs;
    type = ejsIsType(ejs, obj) ? ((EjsType*) obj) : 0;

    if (instanceMethods) {
        out(mp, "<a name='InstanceMethods'></a>\n");
        out(mp, "<h2 class='classSection'>%S Instance Methods</h2>\n", 
            (type) ? type->qname.name : ejsCreateStringFromAsc(ejs, "Global"));
    } else {
        out(mp, "<a name='ClassMethods'></a>\n");
        out(mp, "<h2 class='classSection'>%S Class Methods</h2>\n", 
            (type) ? type->qname.name : ejsCreateStringFromAsc(ejs, "Global"));
    }
    /*
        Output each method
     */
    count = emitTable = 0;
    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        qname = fp->qname;
        trait = fp->trait;
        fun = fp->fun;

        if (!emitTable) {
            out(mp, "<table class='apiIndex' summary='methods'>\n");
            out(mp, "   <tr><th>Qualifiers</th><th width='95%%'>Method</th></tr>\n");
            emitTable = 1;
        }

        if (ejsCompareMulti(ejs, qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }
        if (instanceMethods) {
            if (trait->attributes & EJS_PROP_STATIC) {
                continue;
            }
        } else {
            if (!(trait->attributes & EJS_PROP_STATIC)) {
                continue;
            }
        }

        if (type && qname.space == type->qname.space) {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s</td>", fmtAttributes(fun, trait->attributes, 0));
        } else {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s %s</td>", fmtNamespace(ejs, qname), 
                fmtAttributes(fun, trait->attributes, 0));
        }
        out(mp, "<td><a href='#%@'><b>%s</b></a>(", qname.name, demangle(ejs, qname.name));

        doc = getDoc(ejs, "fun", fp->obj, fp->slotNum);

        for (i = 0; i < (int) fun->numArgs; ) {
            argName = ejsGetPropertyName(ejs, fun->activation, i);
            argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
            if (argTrait->type) {
                out(mp, "%s: %s", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
            } else {
                out(mp, "%s", fmtDeclaration(ejs, argName));
            }
            if (doc) {
                defaultValue = getDefault(doc, ejsToMulti(ejs, argName.name));
                if (defaultValue) {
                    out(mp, " = %s", defaultValue);
                }
            }
            if (++i < (int) fun->numArgs) {
                out(mp, ", ");
            }
        }
        out(mp, ")");

        if (fun->resultType) {
            out(mp, ": %s", fmtTypeReference(ejs, fun->resultType->qname));
        }
        out(mp, "</tr>");

        if (doc) {
            out(mp, "<tr class='apiBrief'><td>&nbsp;</td><td>%s</td></tr>\n", doc->brief);
        }
        count++;
    }
    if (count == 0) {
        // UNUSED out(mp, "   <tr><td colspan='2'>No %s methods defined</td></tr>\n", instanceMethods ? "instance" : "class");
        out(mp, "   <p>(No own %s methods defined)</p>", instanceMethods ? "instance" : "class");
    }
    out(mp, "</table>\n\n");
    if (type && type->baseType) {
        out(mp, "<p class='inheritedLink'><a href='%s#InstanceMethods'><i>Inherited Methods</i></a></p>\n\n",
            fmtClassUrl(ejs, type->baseType->qname));
    }
    out(mp, "<hr />\n");
    return count;
}


static void generateMethodDetail(EjsMod *mp, MprList *methods)
{
    Ejs         *ejs;
    FunRec      *fp;
    int         next;

    ejs = mp->ejs;
    out(mp, "<h2>Method Detail</h2>\n");

    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        generateMethod(mp, fp);
    }
}


static void checkArgs(EjsMod *mp, Ejs *ejs, EjsName ownerName, EjsFunction *fun, EjsName qname, EjsDoc *doc)
{
    EjsName         argName;
    MprKeyValue     *param;
    cchar           *key;
    int             i, next;

    for (i = 0; i < (int) fun->numArgs; i++) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        for (next = 0; (param = mprGetNextItem(doc->params, &next)) != 0; ) {
            key = param->key;
            if (strncmp(key, "...", 3) == 0) {
                key += 3;
            }
            if (strcmp(key, ejsToMulti(ejs, argName.name)) == 0) {
                break;
            }
        }
        if (param == 0) { 
            if (mp->warnOnError) {
                mprWarn("Missing documentation for parameter \"%S\" in function \"%S\" in type \"%S\"", 
                     argName.name, qname.name, ownerName.name);
            }
        }
    }
}


static int findArg(Ejs *ejs, EjsFunction *fun, cchar *name)
{
    EjsName     argName;
    int         i;

    if (strncmp(name, "...", 3) == 0) {
        name += 3;
    }
    for (i = 0; i < (int) fun->numArgs; i++) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        if (argName.name && ejsCompareMulti(ejs, argName.name, name) == 0) {
            return i;
        }
    }
    return EJS_ERR;
}


/*
    Lookup to see if there is doc about a default value for a parameter
 */
static cchar *getDefault(EjsDoc *doc, cchar *key)
{
    MprKeyValue     *def;
    int             next;

    for (next = 0; (def = mprGetNextItem(doc->defaults, &next)) != 0; ) {
        if (strcmp(def->key, key) == 0) {
            return def->value;
        }
    }
    return 0;
}


static void generateMethod(EjsMod *mp, FunRec *fp)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait, *argTrait;
    EjsName         qname, argName, throwName;
    EjsFunction     *fun;
    EjsObj          *obj;
    EjsDoc          *doc;
    EjsLookup       lookup;
    MprKeyValue     *param, *thrown, *option, *event;
    cchar           *defaultValue, *accessorSep, *spaceSep;
    char            *see, *description, *setType;
    int             i, count, next, numInherited, slotNum;

    ejs = mp->ejs;
    obj = fp->obj;
    slotNum = fp->slotNum;

    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;
    fun = (EjsFunction*) ejsGetProperty(ejs, obj, slotNum);

    numInherited = 0;
    if (ejsIsPrototype(ejs, obj)) {
        mprAssert(ejsIsType(ejs, fp->owner));
        numInherited = ((EjsType*) fp->owner)->numInherited;
    }
    mprAssert(ejsIsFunction(ejs, fun));

    qname = ejsGetPropertyName(ejs, obj, slotNum);
    trait = ejsGetPropertyTraits(ejs, obj, slotNum);
    
    doc = getDoc(ejs, "fun", obj, slotNum);
    if (doc && doc->hide) {
        return;
    }
    if (doc == 0 || doc->brief == 0) {
        if (mp->warnOnError) {
            if (!ejsIsType(ejs, fun)) {
                /* Don't warn about default constructors */
                if (mp->warnOnError) {
                    mprWarn("Missing documentation for \"%@.%@\"", fp->ownerName.name, qname.name);
                }
            }
        }
        return;
    }
    if (isalpha((int) qname.name->value[0])) {
        out(mp, "<a name='%@'></a>\n", qname.name);
    }
    if (type && qname.space == type->qname.space) {
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %@(", fmtAttributes(fun, trait->attributes, 0), qname.name);

    } else {
        accessorSep = (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) ? " ": "";
        spaceSep = qname.space->value[0] ? " ": "";
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %s%s %s%s %s(", 
            fmtAttributes(fun, trait->attributes & ~(EJS_TRAIT_GETTER | EJS_TRAIT_SETTER), 0), 
            spaceSep, fmtSpace(ejs, qname), accessorSep, fmtAccessors(trait->attributes), demangle(ejs, qname.name));
    }

    for (i = 0; i < (int) fun->numArgs; ) {
        argName = ejsGetPropertyName(ejs, fun->activation, i);
        argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
        if (argTrait->type) {
            out(mp, "%s: %s", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
        } else {
            out(mp, "%s", fmtDeclaration(ejs, argName));
        }
        if (doc) {
            defaultValue = getDefault(doc, ejsToMulti(ejs, argName.name));
            if (defaultValue) {
                out(mp, " = %s", defaultValue);
            }
        }
        if (++i < (int) fun->numArgs) {
            out(mp, ", ");
        }
    }
    out(mp, ")");
    if (fun->resultType) {
        out(mp, ": %s", fmtTypeReference(ejs, fun->resultType->qname));
    }
    out(mp, "\n</div>\n");

    if (doc) {
        out(mp, "<div class='apiDetail'>\n");
        out(mp, "<dl><dt>Description</dt></dd><dd>%s %s</dd></dl>\n", doc->brief, doc->description ? doc->description : "");
#if UNUSED
        if (doc->description) {
            out(mp, "<dl><dt>Description</dt><dd>%s</dd></dl>\n", doc->description);
        }
#endif
        count = mprGetListLength(doc->params);
        if (count > 0) {
            out(mp, "<dl><dt>Parameters</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='parameters'>\n");

            checkArgs(mp, ejs, fp->ownerName, fun, qname, doc);
            for (next = 0; (param = mprGetNextItem(doc->params, &next)) != 0; ) {
                defaultValue = getDefault(doc, param->key);
                i = findArg(ejs, fun, param->key);
                if (i < 0) {
                    mprError("Bad @param reference for \"%s\" in function \"%@\" in type \"%@\"", param->key, 
                        qname.name, fp->ownerName.name);
                } else {
                    argName = ejsGetPropertyName(ejs, fun->activation, i);
                    argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
                    out(mp, "<tr class='param'><td class='param'>");
                    description = param->value;
                    setType = 0;
                    if (description && description[0] == ':') {
                        setType = stok(sclone(&description[1]), " ", &description);
                    }
                    if (argTrait->type) {
                        out(mp, "%s: %s ", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
                    } else if (setType) {
                        out(mp, "%s: %s", fmtDeclaration(ejs, argName), setType);
                    } else {
                        out(mp, "%s ", fmtDeclaration(ejs, argName));
                    }
                    out(mp, "</td><td>%s", description);
                    if (defaultValue) {
                        if (scontains(description, "Not implemented", -1) == NULL) {
                            out(mp, " [default: %s]", defaultValue);
                        }
                    }
                }
                out(mp, "</td></tr>");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }

        count = mprGetListLength(doc->options);
        if (count > 0) {
            out(mp, "<dl><dt>Options</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='options'>\n");
            for (next = 0; (option = mprGetNextItem(doc->options, &next)) != 0; ) {
                out(mp, "<td class='param'>%s</td><td>%s</td>", option->key, option->value);
                out(mp, "</tr>\n");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }
        
        count = mprGetListLength(doc->events);
        if (count > 0) {
            out(mp, "<dl><dt>Events</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='events'>\n");
            for (next = 0; (event = mprGetNextItem(doc->events, &next)) != 0; ) {
                out(mp, "<td class='param'>%s</td><td>%s</td>", event->key, event->value);
                out(mp, "</tr>\n");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }

        if (doc->returns) {
            out(mp, "<dl><dt>Returns</dt>\n<dd>%s</dd></dl>\n", doc->returns);
        }
        count = mprGetListLength(doc->throws);
        if (count > 0) {
            out(mp, "<dl><dt>Throws</dt><dd>\n");
            for (next = 0; (thrown = (MprKeyValue*) mprGetNextItem(doc->throws, &next)) != 0; ) {
                //  TODO Functionalize
                ejs->state->bp = ejs->global;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, EN(thrown->key), &lookup)) < 0) {
                    continue;
                }
                throwName = lookup.name;
                out(mp, "<a href='%s'>%s</a>: %s%s\n", getFilename(fmtType(ejs, throwName)), thrown->key,
                    thrown->value, (count == next) ? "" : ", ");
            }
            out(mp, "</dd>\n");
            out(mp, "</dl>");
        }
        if (doc->requires) {
            out(mp, "<dl><dt>Requires</dt>\n<dd>configure --ejs-%s</dd></dl>\n", doc->requires);
        }
        if (doc->spec) {
            out(mp, "<dl><dt>Specified</dt>\n<dd>%s</dd></dl>\n", doc->spec);
        }
        if (doc->stability) {
            out(mp, "<dl><dt>Stability</dt>\n<dd>%s</dd></dl>\n", doc->stability);
        }
        if (doc->example) {
            out(mp, "<dl><dt>Example</dt>\n<dd><pre>%s</pre></dd></dl>\n", doc->example);
        }
        count = mprGetListLength(doc->see);
        if (count > 0) {
            out(mp, "<dl><dt>See Also</dt>\n<dd>\n");
            for (next = 0; (see = mprGetNextItem(doc->see, &next)) != 0; ) {
                out(mp, "<a href='%s'>%s</a>%s\n", getFilename(see), see, (count == next) ? "" : ", ");
            }
            out(mp, "</dd></dl>\n");
        }
        out(mp, "</div>\n");
    }
    out(mp, "</div>\n");
    out(mp, "<hr />\n");
}


static char *fmtAttributes(EjsAny *vp, int attributes, int klass)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    /*
        Order to look best
     */
    if (attributes & EJS_PROP_STATIC) {
        strcat(attributeBuf, "static ");
    }
    /* Types are can also be constructor functions. Need klass parameter to differentiate */
    if (ejsIsType(ejs, vp) && klass) {
        if (attributes & EJS_TYPE_FINAL) {
            strcat(attributeBuf, "final ");
        }
        if (attributes & EJS_TYPE_DYNAMIC_INSTANCE) {
            strcat(attributeBuf, "dynamic ");
        }
    } else if (ejsIsFunction(ejs, vp)) {
        if (attributes & EJS_FUN_OVERRIDE) {
            strcat(attributeBuf, "override ");
        }
        if (attributes & EJS_TRAIT_GETTER) {
            strcat(attributeBuf, "get ");
        }
        if (attributes & EJS_TRAIT_SETTER) {
            strcat(attributeBuf, "set ");
        }
    } else {
        if (attributes & EJS_TRAIT_READONLY) {
            strcat(attributeBuf, "const ");
        }
    }
#if UNUSED
    if (accessors) {
        if (attributes & EJS_TRAIT_GETTER) {
            strcat(attributeBuf, "get ");
        }
        if (attributes & EJS_TRAIT_SETTER) {
            strcat(attributeBuf, "set ");
        }
    }
#endif
    return attributeBuf;
}


static char *fmtAccessors(int attributes)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    if (attributes & EJS_TRAIT_GETTER) {
        strcat(attributeBuf, "get ");
    }
    if (attributes & EJS_TRAIT_SETTER) {
        strcat(attributeBuf, "set ");
    }
    return attributeBuf;
}


static MprChar *joinLines(MprChar *str)
{
    MprChar    *cp, *np;

    for (cp = str; cp && *cp; cp++) {
        if (*cp == '\n') {
            for (np = &cp[1]; *np; np++) {
                if (!isspace((int) *np)) {
                    break;
                }
            }
            if (!isspace((int) *np)) {
                *cp = ' ';
            }
        }
    }
    return str;
}


/*
    Merge in @duplicate entries
 */
static MprChar *mergeDuplicates(Ejs *ejs, EjsMod *mp, EjsName qname, EjsDoc *doc, MprChar *spec)
{
    EjsDoc      *dup;
    MprChar     *next, *duplicate, *mark;

    if ((next = mcontains(spec, "@duplicate", -1)) == 0) {
        return spec;
    }
    next = spec = wclone(spec);

    while ((next = mcontains(next, "@duplicate", -1)) != 0) {
        mark = next;
        mtok(next, " \t\n\r", &next);
        if ((duplicate = mtok(next, " \t\n\r", &next)) == 0) {
            break;
        }
        if ((dup = getDuplicateDoc(ejs, duplicate)) == 0) {
            mprError("Can't find @duplicate directive %s for %s", duplicate, qname.name);
        } else {
            crackDoc(mp, dup, WEN(duplicate));
            mprCopyList(doc->params, dup->params);
            mprCopyList(doc->options, dup->options);
            mprCopyList(doc->events, dup->events);
            mprCopyList(doc->see, dup->see);
            mprCopyList(doc->throws, dup->throws);
            doc->brief = dup->brief;
            doc->description = dup->description;
            doc->example = dup->example;
            doc->requires = dup->requires;
            doc->returns = dup->returns;
            doc->stability = dup->stability;
            doc->spec = dup->spec;
        }
        memmove(mark, next, wlen(next) + 1);
        next = mark;
    }
    return spec;
}


/*
    Cleanup text. Remove leading comments and "*" that are part of the comment and not part of the doc.
 */
static void prepText(MprChar *str)
{
    MprChar     *dp, *cp;

    dp = cp = str;
    while (isspace((int) *cp) || *cp == '*') {
        cp++;
    }
    while (*cp) {
        if (cp[0] == '\n') {
            *dp++ = '\n';
            for (cp++; (isspace((int) *cp) || *cp == '*'); cp++) {
                if (*cp == '\n') {
                    *dp++ = '\n';
                }
            }
            if (*cp == '\0') {
                cp--;
            }
        } else {
            *dp++ = *cp++;
        }
    }
    *dp = '\0';
}


static EjsDoc *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName qname)
{
    Ejs         *ejs;
    EjsDoc      *dup;
    MprKeyValue *pair;
    MprChar     *value, *key, *line, *str, *next, *cp, *token, *nextWord, *word, *duplicate;
    MprChar     *thisBrief, *thisDescription, *start;

    ejs = mp->ejs;

    if (doc->cracked) {
        return doc;
    }
    doc->cracked = 1;
    doc->params = mprCreateList(0, 0);
    doc->options = mprCreateList(0, 0);
    doc->events = mprCreateList(0, 0);
    doc->defaults = mprCreateList(0, 0);
    doc->see = mprCreateList(0, 0);
    doc->throws = mprCreateList(0, 0);

    str = mprMemdup(doc->docString->value, doc->docString->length);
    if (str == NULL) {
        return doc;
    }
    prepText(str);
    if (mcontains(str, "@hide", -1) || mcontains(str, "@hidden", -1)) {
        doc->hide = 1;
    } else if (mcontains(str, "@deprecate", -1) || mcontains(str, "@deprecated", -1)) {
        doc->deprecated = 1;
    }
    str = mergeDuplicates(ejs, mp, qname, doc, str);

    thisDescription = NULL;
    thisBrief = NULL;
    next = str;
    
    if (str[0] != '@') {
        if ((thisBrief = mtok(str, "@", &next)) == 0) {
            thisBrief = NULL;
        } else {
            for (cp = thisBrief; *cp; cp++) {
                if (*cp == '.' && (isspace((int) cp[1]) || *cp == '*')) {
                    cp++;
                    *cp++ = '\0';
                    thisDescription = mtrim(cp, " \t\n", MPR_TRIM_BOTH);
                    break;
                }
            }
        }
    }
    doc->brief = wjoin(doc->brief, thisBrief, NULL);
    doc->description = wjoin(doc->description, thisDescription, NULL);
    mtrim(doc->brief, " \t\r\n", MPR_TRIM_BOTH);
    mtrim(doc->description, " \t\r\n", MPR_TRIM_BOTH);
    mprAssert(doc->brief);
    mprAssert(doc->description);

    /*
        This is what we are parsing:
        One liner is the first sentance. Rest of description can continue from here and can include embedded html
 
        @param argName Description          (Up to next @, case matters on argName)
        @default argName DefaultValue       (Up to next @, case matters on argName)
        @return Sentence                    (Can use return or returns. If sentance starts with lower case, then start 
                                                sentance with "Call returns".
        @option argName Description         (Up to next @, case matters on argName)
        @event eventName Description        (Up to next @, case matters on argName)
        @throws ExceptionType Explanation   (Up to next @)
        @see Keyword keyword ...            (Case matters)
        @example Description                (Up to next @)
        @stability kind                     (prototype | evolving | stable | mature | deprecated]
        @deprecated version                 Same as @stability deprecated
        @requires ECMA                      (Emit: configuration requires --ejs-ecma)
        @spec                               (ecma-262, ecma-357, ejs-11)
        @hide                               (Hides this entry)

        Only functions can use return and param.
     */
    start = next;
    while (next) {
        token = next;
        for (cp = next; cp ; ) {
            mtok(cp, "@", &cp);
            if (cp && cp[-2] == '\\') {
                cp[-1] = '@';
                cp[-2] = ' ';
            } else {
                next = cp;
                break;
            }
        }
        line = skipAtWord(token);

        if (token > &start[2] && token[-2] == '\\') {
            continue;
        }
        if (match(token, "duplicate")) {
            duplicate = mtrim(line, " \t\n", MPR_TRIM_BOTH);
            if ((dup = getDuplicateDoc(ejs, duplicate)) == 0) {
                mprError("Can't find @duplicate directive %s for %S", duplicate, qname.name);
            } else {
                crackDoc(mp, dup, WEN(duplicate));
                mprCopyList(doc->params, dup->params);
                mprCopyList(doc->options, dup->options);
                mprCopyList(doc->events, dup->events);
                mprCopyList(doc->see, dup->see);
                mprCopyList(doc->throws, dup->throws);
                doc->brief = mrejoin(doc->brief, " ", dup->brief, NULL);
                doc->description = mrejoin(doc->description, " ", dup->description, NULL);
                if (dup->example) {
                    if (doc->example) {
                        doc->example = mrejoin(doc->example, " ", dup->example, NULL);
                    } else {
                        doc->example = dup->example;
                    }
                }
                if (dup->requires) {
                    if (doc->requires) {
                        doc->requires = mrejoin(doc->requires, " ", dup->requires, NULL);
                    } else {
                        doc->requires = dup->requires;
                    }
                }
                if (dup->returns && doc->returns == 0) {
                    doc->returns = dup->returns;
                }
                if (dup->stability && doc->stability == 0) {   
                    doc->stability = dup->stability;
                }
                if (dup->spec && doc->spec == 0) {
                    doc->spec = dup->spec;
                }
            }

        } else if (match(token, "example") || match(token, "examples")) {
            doc->example = formatExample(ejs, doc->docString);

        } else if (match(token, "event")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = createKeyPair(key, value);
                mprAddItem(doc->events, pair);
            }

        } else if (match(token, "option")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = createKeyPair(key, value);
                mprAddItem(doc->options, pair);
            }

        } else if (match(token, "param")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                key = mtrim(key, ".", MPR_TRIM_BOTH);
                pair = createKeyPair(key, value);
                mprAddItem(doc->params, pair);
            }

        } else if (match(token, "default")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            if (key && *key) {
                pair = createKeyPair(key, value);
                mprAddItem(doc->defaults, pair);
            }

        } else if (match(token, "deprecated")) {
            doc->hide = 1;
            doc->deprecated = 1;
            doc->stability = amtow("deprecated", NULL);

        } else if (match(token, "hide") || match(token, "hidden")) {
            doc->hide = 1;

        } else if (match(token, "spec")) {
            doc->spec = mtrim(line, " \t\n", MPR_TRIM_BOTH);
            wlower(doc->spec);

        } else if (match(token, "stability")) {
            doc->stability = mtrim(line, " \t", MPR_TRIM_BOTH);
            wlower(doc->stability);

        } else if (match(token, "requires")) {
            doc->requires = mtrim(line, " \t", MPR_TRIM_BOTH);
            wlower(doc->requires);

        } else if (match(token, "return") || match(token, "returns")) {
            line = joinLines(line);
            doc->returns = mtrim(line, " \t", MPR_TRIM_BOTH);

        } else if (match(token, "throw") || match(token, "throws")) {
            getKeyValue(line, &key, &value);
            value = joinLines(value);
            pair = createKeyPair(key, value);
            mprAddItem(doc->throws, pair);

        } else if (match(token, "see") || match(token, "seeAlso")) {
            nextWord = line;
            do {
                word = nextWord;
                mtok(word, " \t\r\n", &nextWord);
                mprAddItem(doc->see, mtrim(word, " \t", MPR_TRIM_BOTH));
            } while (nextWord && *nextWord);
        }
    }
    fixupDoc(ejs, doc);
    return doc;
}


static MprChar *fixSentence(MprChar *str)
{
    MprChar     *buf;
    size_t      len;

    if (str == 0 || *str == '\0') {
        return 0;
    }
    
    /*
        Copy the string and grow by 1 byte (plus null) to allow for a trailing period.
     */
    len = wlen(str) + 2 * sizeof(MprChar);
    if ((buf = mprAlloc(len)) == 0) {
        return 0;
    }
    wcopy(buf, len, str);
    str = buf;
    str[0] = toupper((int) str[0]);

    /*
        Append a "." if the string does not appear to contain HTML tags
     */
    if (mcontains(str, "</", -1) == 0) {
        /* Trim period and re-add */
        str = mtrim(str, " \t\r\n.", MPR_TRIM_BOTH);
        len = wlen(str);
        if (str[len - 1] != '.') {
            str[len] = '.';
            str[len + 1] = '\0';
        }
    } else {
        str = mtrim(str, " \t\r\n", MPR_TRIM_BOTH);
    }
    return str;
}


static MprChar *formatExample(Ejs *ejs, EjsString *docString)
{
    MprChar     *example, *cp, *end, *buf, *dp;
    int         i, indent;

    if ((example = mcontains(docString->value, "@example", -1)) != 0) {
        example += 8;
        for (cp = example; *cp && *cp != '\n'; cp++) {}
        if (*cp == '\n') {
            cp++;
        }
        example = cp;
        for (end = example; *end; end++) {
            if (*end == '@' && (end == example || end[-1] != '\\')) {
                break;
            }
        }
        for (indent = 0; *cp == '\t' || *cp == ' '; indent++, cp++) {}

        buf = mprAlloc(wlen(example) * 4 + 2);
        for (cp = example, dp = buf; *cp && cp < end; ) {
            for (i = 0; i < indent && *cp && isspace((int) *cp) && *cp != '\n'; i++, cp++) {}
            for (; *cp && *cp != '\n'; ) {
                if (*cp == '<' && cp[1] == '%') {
                    mtow(dp, 5, "&lt;", 4);
                    dp += 4;
                    cp++;
                    *dp++ = *cp++;
                } else if (*cp == '%' && cp[1] == '>') {
                    *dp++ = *cp++;
                    mtow(dp, 5, "&gt;", 4);
                    dp += 4;
                    cp++;
                } else {
                    *dp++ = *cp++;
                }
            }
            if (*cp == '\n') {
                *dp++ = *cp++;
            }
            *dp = '\0';
        }
        for (--dp; dp > example && isspace((int) *dp); dp--) {}
        *++dp = '\0';
        return buf;
    }
    return NULL;
}


static MprChar *wikiFormat(Ejs *ejs, MprChar *start)
{
    EjsLookup   lookup;
    EjsName     qname;
    MprBuf      *buf;
    MprChar     *end, *cp, *klass, *property, *str, *pref, *space;
    ssize       len;
    int         slotNum, sentence;

    if (start == 0 || *start == '\0') {
        return NULL;
    }
    buf = mprCreateBuf(-1, -1);
    end = &start[wlen(start)];
    
    for (str = start; str < end && *str; str++) {
        /*
            FUTURE -- expand this to support basic markdown
         */
        if (str[0] == '\n' && str[1] == '\n') {
            /* Two blank lines forces a blank line in the output */
            mprPutStringToWideBuf(buf, "<br/><br/>");
            str++;

        } else if (*str == '$') {
            if (str[1] == '$') {
                mprPutCharToWideBuf(buf, *str);
                continue;
            }
            if ((str > start && (str[-1] == '$' || str[-1] == '\\'))) {
                /* Remove backquote */
                mprAdjustBufEnd(buf, - (int) sizeof(MprChar));
                mprPutCharToWideBuf(buf, *str);
                continue;
            }
            /* Dollar reference expansion */
            klass = &str[1];
            for (cp = &str[1]; *cp; cp++) {
                if (isspace((int) *cp)) {
                    break;
                }
            }
            len = cp - str;
            str = cp;
            if (isspace((int) *cp)) {
                cp--;
            }
            klass = snclone(klass, len);
            sentence = (klass[wlen(klass) - 1] == '.');
            mprAssert(strcmp(klass, "ejs.web::Request") != 0);

            if (scontains(klass, "::", -1)) {
                space = stok(klass, "::", &klass);
            } else {
                space = "";
            }
            if ((property = wchr(klass, '.')) != 0) {
                *property++ = '\0';
                if (*property == '\0') {
                    property = klass;
                    klass = 0;
                }
            } else {
                property = klass;
                klass = 0;
            }
            pref = strim(property, "(), \t", MPR_TRIM_END);
            klass = mtrim(klass, "., \t", MPR_TRIM_BOTH);
            property = mtrim(property, "., \t", MPR_TRIM_BOTH);

            if (klass) {
                //  TODO Functionalize
                ejs->state->bp = ejs->global;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, N(space, klass), &lookup)) < 0) {
                    if (klass) {
                        mprPutFmtToWideBuf(buf, "%s.%s", klass, property);
                    } else {
                        mprPutStringToBuf(buf, property);
                    }
                } else {
                    qname = lookup.name;
                    if (property) {
                        mprPutFmtToWideBuf(buf, "<a href='%s#%s'>%s.%s</a>", getFilename(fmtType(ejs, qname)), 
                            pref, klass, property);
                    } else {
                        mprPutFmtToWideBuf(buf, "<a href='%s'>%s</a>", getFilename(fmtType(ejs, qname)), klass);
                    }
                }
            } else {
                mprPutFmtToWideBuf(buf, "<a href='#%s'>%s</a>", pref, property);
            }
            if (sentence) {
                mprPutCharToWideBuf(buf, '.');
            }
            mprPutCharToWideBuf(buf, ' ');

        } else {
            mprPutCharToWideBuf(buf, *str);
        }
    }
    mprAddNullToWideBuf(buf);
    return (MprChar*) mprGetBufStart(buf);
}


static void fixupDoc(Ejs *ejs, EjsDoc *doc)
{
    MprKeyValue     *pair;
    int             next;

    doc->brief = wikiFormat(ejs, fixSentence(doc->brief));
    doc->description = wikiFormat(ejs, fixSentence(doc->description));
    doc->returns = wikiFormat(ejs, fixSentence(doc->returns));
    doc->stability = fixSentence(doc->stability);
    doc->requires = wikiFormat(ejs, fixSentence(doc->requires));
    if (doc->spec) {
        if (mcmp(doc->spec, "ejs") == 0) {
            doc->spec = mfmt("ejscript-%d.%d", BLD_MAJOR_VERSION, BLD_MINOR_VERSION);
        }
    } else {
        doc->spec = NULL;
    }
    if (doc->example == 0) {
        doc->example = NULL;
    }
    for (next = 0; (pair = mprGetNextItem(doc->events, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentence(pair->value));
    }
    for (next = 0; (pair = mprGetNextItem(doc->options, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentence(pair->value));
    }
    for (next = 0; (pair = mprGetNextItem(doc->params, &next)) != 0; ) {
        pair->value = wikiFormat(ejs, fixSentence(pair->value));
    }
    /*
        Don't fix the default value
     */
}


static void out(EjsMod *mp, char *fmt, ...)
{
    va_list     args;
    char        *buf;

    if (mp->file == 0) {
        mp->file = mprOpenFile(mp->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (mp->file == 0) {
            mprError("Can't open %s", mp->path);
            mp->errorCount++;
            return;
        }
    }
    va_start(args, fmt);
    buf = mprAsprintfv(fmt, args);
    if (mprWriteFileString(mp->file, buf) < 0) {
        mprError("Can't write to buffer");
    }
}


static EjsString *fmtModule(Ejs *ejs, EjsString *name)
{
    if (ejsCompareMulti(ejs, name, EJS_DEFAULT_MODULE) == 0) {
        return S(empty);
    }
    return name;
}


static char *fmtClassUrl(Ejs *ejs, EjsName qname)
{
    return getFilename(fmtType(ejs, qname));
}


static char *fmtNamespace(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    cchar       *space;
    char        *cp;

    space = ejsToMulti(ejs, qname.space);
    if (space[0] == '[') {
        scopy(buf, sizeof(buf), &space[1]);

    } else {
        scopy(buf, sizeof(buf), space);
    }
    if (buf[strlen(buf) - 1] == ']') {
        buf[strlen(buf) - 1] = '\0';
    }
    if (strcmp(buf, "ejs") == 0) {
        buf[0] = '\0';

    } else if (strcmp(buf, "public") == 0) {
        buf[0] = '\0';

    } else if ((cp = strrchr(buf, ',')) != 0) {
        ++cp;
        if (strcmp(cp, EJS_PUBLIC_NAMESPACE) == 0) {
            strcpy(buf, EJS_PUBLIC_NAMESPACE);

        } else if (strcmp(cp, EJS_PRIVATE_NAMESPACE) == 0 || strcmp(cp, EJS_CONSTRUCTOR_NAMESPACE) == 0 ||
                   strcmp(cp, EJS_INIT_NAMESPACE) == 0) {
            /*
                Suppress "private" as they are the default for namespaces and local vars
             */
            buf[0] = '\0';
        }
    }

    if (strcmp(buf, EJS_PRIVATE_NAMESPACE) == 0 || strcmp(buf, EJS_CONSTRUCTOR_NAMESPACE) == 0 ||
           strcmp(buf, EJS_INIT_NAMESPACE) == 0) {
        buf[0] = '\0';
    }
    return buf;
}


static char *fmtType(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);

    if (strcmp(namespace, EJS_PUBLIC_NAMESPACE) == 0) {
        *namespace = '\0';
    }
    if (*namespace) {
        if (*namespace) {
            mprSprintf(buf, sizeof(buf), "%s::%@", namespace, qname.name);
        } else {
            mprSprintf(buf, sizeof(buf), "%@", qname.name);
        }
    } else {
        mprSprintf(buf, sizeof(buf), "%@", qname.name);
    }
    return buf;
}


/*
    Map lower case names with a "l-" prefix for systems with case insensitive names
 */
static char *getFilename(cchar *name)
{
    static char buf[MPR_MAX_STRING];
    char        *cp, *sp;

    scopy(buf, sizeof(buf), name);

    if ((cp = strstr(buf, "::")) != 0) {
        *cp++ = '-';
        if (islower((int) cp[1])) {
            *cp++ = '-';
            for (sp = cp; *sp; ) {
                *cp++ = *sp++;
            }

        } else {
            for (sp = cp + 1; *sp; ) {
                *cp++ = *sp++;
            }
        }
        *cp = '\0';
    }
    scopy(&buf[strlen(buf)], sizeof(buf) - (int) strlen(buf) - 1, ".html");
    return buf;
}


static char *fmtTypeReference(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *typeName;

    typeName = fmtType(ejs, qname);
    mprSprintf(buf, sizeof(buf), "<a href='%s'>%@</a>", getFilename(typeName), qname.name);
    return buf;
}


static char *fmtSpace(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);
    if (namespace[0]) {
        mprSprintf(buf, sizeof(buf), "%@", qname.space);
    } else {
        buf[0] = '\0';
    }
    return buf;
}


static char *fmtDeclaration(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);
    if (namespace[0]) {
        mprSprintf(buf, sizeof(buf), "%@ %s", qname.space, demangle(ejs, qname.name));
    } else {
        mprSprintf(buf, sizeof(buf), "%s", demangle(ejs, qname.name));
    }
    return buf;
}


static bool match(MprChar *last, cchar *key)
{
    int     len;

    mprAssert(last);
    mprAssert(key && *key);

    len = (int) strlen(key);
    return mncmp(last, key, len) == 0;
}


static MprChar *skipAtWord(MprChar *str)
{
    while (!isspace((int) *str) && *str)
        str++;
    while (isspace((int) *str))
        str++;
    return str;
}


#if UNUSED && KEEP
static char *getType(char *str, char *typeName, int typeSize)
{
    char    *end, *cp;
    int     i;

    for (end = str; *end && isspace((int) *end); end++)
        ;
    for (; *end && !isspace((int) *end); end++)
        ;
    typeSize--;
    for (i = 0, cp = str; i < typeSize && cp < end; cp++, i++) {
        typeName[i] = *cp;
    }
    typeName[i] = '\0';

    for (; *end && isspace((int) *end); end++)
        ;
    return end;
}
#endif


static void getKeyValue(MprChar *str, MprChar **key, MprChar **value)
{
    MprChar     *end;

    for (end = str; *end && !isspace((int) *end); end++)
        ;
    if (end) {
        *end = '\0';
    }
    if (key) {
        *key = mtrim(str, " \t", MPR_TRIM_BOTH);
    }
    for (str = end + 1; *str && isspace((int) *str); str++) {
        ;
    }
    if (value) {
        *value = mtrim(str, " \t", MPR_TRIM_BOTH);
    }
}


static int compareProperties(PropRec **p1, PropRec **p2)
{
    return compareStrings(&(*p1)->qname.name, &(*p2)->qname.name);
}


static int compareFunctions(FunRec **f1, FunRec **f2)
{
    return compareStrings(&(*f1)->qname.name, &(*f2)->qname.name);
}


static int compareClasses(ClassRec **c1, ClassRec **c2)
{
    return compareStrings(&(*c1)->qname.name, &(*c2)->qname.name);
}


static cchar *demangle(Ejs *ejs, EjsString *name)
{
#if UNUSED
    if (ejsStartsWithMulti(ejs, name, "set-")) {
        return ejsToMulti(ejs, ejsSubstring(ejs, name, 4, -1));
    }
#endif
    return ejsToMulti(ejs, name);
}


static cchar *demangleCS(cchar *name)
{
#if UNUSED
    if (strncmp(name, "set-", 4) == 0) {
        return &name[4];
    }
#endif
    return name;
}


static int compareNames(char **q1, char **q2)
{
    cchar    *s1, *s2, *cp;

    s1 = demangleCS(*q1);
    s2 = demangleCS(*q2);

    /*
        Don't sort on the namespace portions of the name
     */
    if ((cp = strrchr(s1, ':')) != 0) {
        s1 = cp + 1;
    }
    if ((cp = strrchr(s2, ':')) != 0) {
        s2 = cp + 1;
    }
    return scmp(s1, s2);
}


static int compareStrings(EjsString **q1, EjsString **q2)
{
    cchar    *s1, *s2, *cp;

    s1 = demangle(ejs, *q1);
    s2 = demangle(ejs, *q2);

    /*
        Don't sort on the namespace portions of the name
     */
    if ((cp = strrchr(s1, ':')) != 0) {
        s1 = cp + 1;
    }
    if ((cp = strrchr(s2, ':')) != 0) {
        s2 = cp + 1;
    }
    return scmp(s1, s2);
}


static void addUniqueItem(MprList *list, cchar *item)
{
    cchar   *p;
    int     next;

    if (item == 0 || *item == '\0') {
        return;
    }
    for (next = 0; (p = mprGetNextItem(list, &next)) != 0; ) {
        if (strcmp(p, item) == 0) {
            return;
        }
    }
    mprAddItem(list, sclone(item));
}


static void addUniqueClass(MprList *list, ClassRec *item)
{
    ClassRec    *p;
    int         next;

    if (item == 0) {
        return;
    }
    for (next = 0; (p = (ClassRec*) mprGetNextItem(list, &next)) != 0; ) {
        if (p->qname.name == item->qname.name) {
            if (p->qname.space == item->qname.space) {
                return;
            }
        }
    }
    mprAddItem(list, item);
}


static EjsDoc *getDoc(Ejs *ejs, cchar *tag, void *obj, int slotNum)
{
    EjsObj      *vp;
    char        key[32];

    vp = ejsGetProperty(ejs, obj, slotNum);
    if (tag == 0) {
        if (ejsIsType(ejs, vp)) {
            tag = "class";
        } else if (ejsIsFunction(ejs, vp)) {
            tag = "fun";
        } else {
            tag = "var";
        }
    }
    mprSprintf(key, sizeof(key), "%s %Lx %d", tag, PTOL(obj), slotNum);
    return mprLookupHash(ejs->doc, key);
}


static EjsDoc *getDuplicateDoc(Ejs *ejs, MprChar *duplicate)
{
    EjsDoc          *doc;
    EjsObj          *vp;
    EjsLookup       lookup;
    MprChar         *space, *dup, *klass, *property;
    int             slotNum;

    dup = space = wclone(duplicate);
    if ((klass = mcontains(space, "::", -1)) != 0) {
        *klass = '\0';
        klass += 2;
    } else {
        klass = space;
        space = "";
    }
    if ((property = wchr(klass, '.')) != 0) {
        *property++ = '\0';
    }
#if UNUSED
    qname = WN(space, klass);
    if (space == 0) {
        qname.space = 0;
    }
#endif
    //  TODO Functionalize
    ejs->state->bp = ejs->global;
    if ((slotNum = ejsLookupVar(ejs, ejs->global, WN(space, klass), &lookup)) < 0) {
        return 0;
    }
    if (property == 0) {
        doc = getDoc(ejs, NULL, ejs->global, slotNum);
    } else {
        vp = ejsGetProperty(ejs, ejs->global, slotNum);
        if ((slotNum = ejsLookupVar(ejs, vp, WEN(property), &lookup)) < 0) {
            if (ejsIsType(ejs, vp)) {
                vp = (EjsObj*) ((EjsType*) vp)->prototype;
                if ((slotNum = ejsLookupVar(ejs, vp, WEN(property), &lookup)) < 0) {
                    return 0;
                }
            }
#if UNUSED
            if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                    /* Last chance - Not ideal */
                    space = mprAsprintf("%s-%s", space, BLD_VNUM);
                    if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                        return 0;
                    }
                }
            }
#endif
        }
        doc = getDoc(ejs, NULL, (EjsBlock*) vp, slotNum);
    }
    if (doc) {
        if (doc->docString == NULL || doc->docString->value[0] == '\0') {
            mprError("Duplicate entry \"%s\" provides no description", duplicate);
            return 0;
        }
    }
    return doc;
}


static MprKeyValue *createKeyPair(MprChar *key, MprChar *value)
{
    MprKeyValue     *pair;
    
    pair = mprAlloc(sizeof(MprKeyValue));
    if (pair == 0) {
        return 0;
    }
    pair->key = wclone(key);
    pair->value = mtrim(wclone(value), " ", MPR_TRIM_BOTH);
    return pair;
}


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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/doc.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/cmd/docFiles.c"
 */
/************************************************************************/

/*
    defaultRomFiles -- Compiled Files
 */

#if 1
static uchar _file_1[] = {
    255,216,255,224,  0, 16, 74, 70, 73, 70,  0,  1,  1,  1,  0, 72,
      0, 72,  0,  0,255,226, 12, 88, 73, 67, 67, 95, 80, 82, 79, 70,
     73, 76, 69,  0,  1,  1,  0,  0, 12, 72, 76,105,110,111,  2, 16,
      0,  0,109,110,116,114, 82, 71, 66, 32, 88, 89, 90, 32,  7,206,
      0,  2,  0,  9,  0,  6,  0, 49,  0,  0, 97, 99,115,112, 77, 83,
     70, 84,  0,  0,  0,  0, 73, 69, 67, 32,115, 82, 71, 66,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,246,214,  0,  1,
      0,  0,  0,  0,211, 45, 72, 80, 32, 32,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0, 17, 99,112,114,116,  0,  0,
      1, 80,  0,  0,  0, 51,100,101,115, 99,  0,  0,  1,132,  0,  0,
      0,108,119,116,112,116,  0,  0,  1,240,  0,  0,  0, 20, 98,107,
    112,116,  0,  0,  2,  4,  0,  0,  0, 20,114, 88, 89, 90,  0,  0,
      2, 24,  0,  0,  0, 20,103, 88, 89, 90,  0,  0,  2, 44,  0,  0,
      0, 20, 98, 88, 89, 90,  0,  0,  2, 64,  0,  0,  0, 20,100,109,
    110,100,  0,  0,  2, 84,  0,  0,  0,112,100,109,100,100,  0,  0,
      2,196,  0,  0,  0,136,118,117,101,100,  0,  0,  3, 76,  0,  0,
      0,134,118,105,101,119,  0,  0,  3,212,  0,  0,  0, 36,108,117,
    109,105,  0,  0,  3,248,  0,  0,  0, 20,109,101, 97,115,  0,  0,
      4, 12,  0,  0,  0, 36,116,101, 99,104,  0,  0,  4, 48,  0,  0,
      0, 12,114, 84, 82, 67,  0,  0,  4, 60,  0,  0,  8, 12,103, 84,
     82, 67,  0,  0,  4, 60,  0,  0,  8, 12, 98, 84, 82, 67,  0,  0,
      4, 60,  0,  0,  8, 12,116,101,120,116,  0,  0,  0,  0, 67,111,
    112,121,114,105,103,104,116, 32, 40, 99, 41, 32, 49, 57, 57, 56,
     32, 72,101,119,108,101,116,116, 45, 80, 97, 99,107, 97,114,100,
     32, 67,111,109,112, 97,110,121,  0,  0,100,101,115, 99,  0,  0,
      0,  0,  0,  0,  0, 18,115, 82, 71, 66, 32, 73, 69, 67, 54, 49,
     57, 54, 54, 45, 50, 46, 49,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0, 18,115, 82, 71, 66, 32, 73, 69, 67, 54, 49, 57, 54, 54,
     45, 50, 46, 49,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0, 88, 89, 90, 32,  0,  0,  0,  0,  0,  0,
    243, 81,  0,  1,  0,  0,  0,  1, 22,204, 88, 89, 90, 32,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 88, 89,
     90, 32,  0,  0,  0,  0,  0,  0,111,162,  0,  0, 56,245,  0,  0,
      3,144, 88, 89, 90, 32,  0,  0,  0,  0,  0,  0, 98,153,  0,  0,
    183,133,  0,  0, 24,218, 88, 89, 90, 32,  0,  0,  0,  0,  0,  0,
     36,160,  0,  0, 15,132,  0,  0,182,207,100,101,115, 99,  0,  0,
      0,  0,  0,  0,  0, 22, 73, 69, 67, 32,104,116,116,112, 58, 47,
     47,119,119,119, 46,105,101, 99, 46, 99,104,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0, 22, 73, 69, 67, 32,104,116,116,112, 58,
     47, 47,119,119,119, 46,105,101, 99, 46, 99,104,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,100,101,115, 99,  0,  0,
      0,  0,  0,  0,  0, 46, 73, 69, 67, 32, 54, 49, 57, 54, 54, 45,
     50, 46, 49, 32, 68,101,102, 97,117,108,116, 32, 82, 71, 66, 32,
     99,111,108,111,117,114, 32,115,112, 97, 99,101, 32, 45, 32,115,
     82, 71, 66,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 46, 73,
     69, 67, 32, 54, 49, 57, 54, 54, 45, 50, 46, 49, 32, 68,101,102,
     97,117,108,116, 32, 82, 71, 66, 32, 99,111,108,111,117,114, 32,
    115,112, 97, 99,101, 32, 45, 32,115, 82, 71, 66,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,100,101,115, 99,  0,  0,  0,  0,  0,  0,  0, 44, 82,101,
    102,101,114,101,110, 99,101, 32, 86,105,101,119,105,110,103, 32,
     67,111,110,100,105,116,105,111,110, 32,105,110, 32, 73, 69, 67,
     54, 49, 57, 54, 54, 45, 50, 46, 49,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0, 44, 82,101,102,101,114,101,110, 99,101, 32, 86,
    105,101,119,105,110,103, 32, 67,111,110,100,105,116,105,111,110,
     32,105,110, 32, 73, 69, 67, 54, 49, 57, 54, 54, 45, 50, 46, 49,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,118,105,101,119,  0,  0,
      0,  0,  0, 19,164,254,  0, 20, 95, 46,  0, 16,207, 20,  0,  3,
    237,204,  0,  4, 19, 11,  0,  3, 92,158,  0,  0,  0,  1, 88, 89,
     90, 32,  0,  0,  0,  0,  0, 76,  9, 86,  0, 80,  0,  0,  0, 87,
     31,231,109,101, 97,115,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      2,143,  0,  0,  0,  2,115,105,103, 32,  0,  0,  0,  0, 67, 82,
     84, 32, 99,117,114,118,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,
      0,  5,  0, 10,  0, 15,  0, 20,  0, 25,  0, 30,  0, 35,  0, 40,
      0, 45,  0, 50,  0, 55,  0, 59,  0, 64,  0, 69,  0, 74,  0, 79,
      0, 84,  0, 89,  0, 94,  0, 99,  0,104,  0,109,  0,114,  0,119,
      0,124,  0,129,  0,134,  0,139,  0,144,  0,149,  0,154,  0,159,
      0,164,  0,169,  0,174,  0,178,  0,183,  0,188,  0,193,  0,198,
      0,203,  0,208,  0,213,  0,219,  0,224,  0,229,  0,235,  0,240,
      0,246,  0,251,  1,  1,  1,  7,  1, 13,  1, 19,  1, 25,  1, 31,
      1, 37,  1, 43,  1, 50,  1, 56,  1, 62,  1, 69,  1, 76,  1, 82,
      1, 89,  1, 96,  1,103,  1,110,  1,117,  1,124,  1,131,  1,139,
      1,146,  1,154,  1,161,  1,169,  1,177,  1,185,  1,193,  1,201,
      1,209,  1,217,  1,225,  1,233,  1,242,  1,250,  2,  3,  2, 12,
      2, 20,  2, 29,  2, 38,  2, 47,  2, 56,  2, 65,  2, 75,  2, 84,
      2, 93,  2,103,  2,113,  2,122,  2,132,  2,142,  2,152,  2,162,
      2,172,  2,182,  2,193,  2,203,  2,213,  2,224,  2,235,  2,245,
      3,  0,  3, 11,  3, 22,  3, 33,  3, 45,  3, 56,  3, 67,  3, 79,
      3, 90,  3,102,  3,114,  3,126,  3,138,  3,150,  3,162,  3,174,
      3,186,  3,199,  3,211,  3,224,  3,236,  3,249,  4,  6,  4, 19,
      4, 32,  4, 45,  4, 59,  4, 72,  4, 85,  4, 99,  4,113,  4,126,
      4,140,  4,154,  4,168,  4,182,  4,196,  4,211,  4,225,  4,240,
      4,254,  5, 13,  5, 28,  5, 43,  5, 58,  5, 73,  5, 88,  5,103,
      5,119,  5,134,  5,150,  5,166,  5,181,  5,197,  5,213,  5,229,
      5,246,  6,  6,  6, 22,  6, 39,  6, 55,  6, 72,  6, 89,  6,106,
      6,123,  6,140,  6,157,  6,175,  6,192,  6,209,  6,227,  6,245,
      7,  7,  7, 25,  7, 43,  7, 61,  7, 79,  7, 97,  7,116,  7,134,
      7,153,  7,172,  7,191,  7,210,  7,229,  7,248,  8, 11,  8, 31,
      8, 50,  8, 70,  8, 90,  8,110,  8,130,  8,150,  8,170,  8,190,
      8,210,  8,231,  8,251,  9, 16,  9, 37,  9, 58,  9, 79,  9,100,
      9,121,  9,143,  9,164,  9,186,  9,207,  9,229,  9,251, 10, 17,
     10, 39, 10, 61, 10, 84, 10,106, 10,129, 10,152, 10,174, 10,197,
     10,220, 10,243, 11, 11, 11, 34, 11, 57, 11, 81, 11,105, 11,128,
     11,152, 11,176, 11,200, 11,225, 11,249, 12, 18, 12, 42, 12, 67,
     12, 92, 12,117, 12,142, 12,167, 12,192, 12,217, 12,243, 13, 13,
     13, 38, 13, 64, 13, 90, 13,116, 13,142, 13,169, 13,195, 13,222,
     13,248, 14, 19, 14, 46, 14, 73, 14,100, 14,127, 14,155, 14,182,
     14,210, 14,238, 15,  9, 15, 37, 15, 65, 15, 94, 15,122, 15,150,
     15,179, 15,207, 15,236, 16,  9, 16, 38, 16, 67, 16, 97, 16,126,
     16,155, 16,185, 16,215, 16,245, 17, 19, 17, 49, 17, 79, 17,109,
     17,140, 17,170, 17,201, 17,232, 18,  7, 18, 38, 18, 69, 18,100,
     18,132, 18,163, 18,195, 18,227, 19,  3, 19, 35, 19, 67, 19, 99,
     19,131, 19,164, 19,197, 19,229, 20,  6, 20, 39, 20, 73, 20,106,
     20,139, 20,173, 20,206, 20,240, 21, 18, 21, 52, 21, 86, 21,120,
     21,155, 21,189, 21,224, 22,  3, 22, 38, 22, 73, 22,108, 22,143,
     22,178, 22,214, 22,250, 23, 29, 23, 65, 23,101, 23,137, 23,174,
     23,210, 23,247, 24, 27, 24, 64, 24,101, 24,138, 24,175, 24,213,
     24,250, 25, 32, 25, 69, 25,107, 25,145, 25,183, 25,221, 26,  4,
     26, 42, 26, 81, 26,119, 26,158, 26,197, 26,236, 27, 20, 27, 59,
     27, 99, 27,138, 27,178, 27,218, 28,  2, 28, 42, 28, 82, 28,123,
     28,163, 28,204, 28,245, 29, 30, 29, 71, 29,112, 29,153, 29,195,
     29,236, 30, 22, 30, 64, 30,106, 30,148, 30,190, 30,233, 31, 19,
     31, 62, 31,105, 31,148, 31,191, 31,234, 32, 21, 32, 65, 32,108,
     32,152, 32,196, 32,240, 33, 28, 33, 72, 33,117, 33,161, 33,206,
     33,251, 34, 39, 34, 85, 34,130, 34,175, 34,221, 35, 10, 35, 56,
     35,102, 35,148, 35,194, 35,240, 36, 31, 36, 77, 36,124, 36,171,
     36,218, 37,  9, 37, 56, 37,104, 37,151, 37,199, 37,247, 38, 39,
     38, 87, 38,135, 38,183, 38,232, 39, 24, 39, 73, 39,122, 39,171,
     39,220, 40, 13, 40, 63, 40,113, 40,162, 40,212, 41,  6, 41, 56,
     41,107, 41,157, 41,208, 42,  2, 42, 53, 42,104, 42,155, 42,207,
     43,  2, 43, 54, 43,105, 43,157, 43,209, 44,  5, 44, 57, 44,110,
     44,162, 44,215, 45, 12, 45, 65, 45,118, 45,171, 45,225, 46, 22,
     46, 76, 46,130, 46,183, 46,238, 47, 36, 47, 90, 47,145, 47,199,
     47,254, 48, 53, 48,108, 48,164, 48,219, 49, 18, 49, 74, 49,130,
     49,186, 49,242, 50, 42, 50, 99, 50,155, 50,212, 51, 13, 51, 70,
     51,127, 51,184, 51,241, 52, 43, 52,101, 52,158, 52,216, 53, 19,
     53, 77, 53,135, 53,194, 53,253, 54, 55, 54,114, 54,174, 54,233,
     55, 36, 55, 96, 55,156, 55,215, 56, 20, 56, 80, 56,140, 56,200,
     57,  5, 57, 66, 57,127, 57,188, 57,249, 58, 54, 58,116, 58,178,
     58,239, 59, 45, 59,107, 59,170, 59,232, 60, 39, 60,101, 60,164,
     60,227, 61, 34, 61, 97, 61,161, 61,224, 62, 32, 62, 96, 62,160,
     62,224, 63, 33, 63, 97, 63,162, 63,226, 64, 35, 64,100, 64,166,
     64,231, 65, 41, 65,106, 65,172, 65,238, 66, 48, 66,114, 66,181,
     66,247, 67, 58, 67,125, 67,192, 68,  3, 68, 71, 68,138, 68,206,
     69, 18, 69, 85, 69,154, 69,222, 70, 34, 70,103, 70,171, 70,240,
     71, 53, 71,123, 71,192, 72,  5, 72, 75, 72,145, 72,215, 73, 29,
     73, 99, 73,169, 73,240, 74, 55, 74,125, 74,196, 75, 12, 75, 83,
     75,154, 75,226, 76, 42, 76,114, 76,186, 77,  2, 77, 74, 77,147,
     77,220, 78, 37, 78,110, 78,183, 79,  0, 79, 73, 79,147, 79,221,
     80, 39, 80,113, 80,187, 81,  6, 81, 80, 81,155, 81,230, 82, 49,
     82,124, 82,199, 83, 19, 83, 95, 83,170, 83,246, 84, 66, 84,143,
     84,219, 85, 40, 85,117, 85,194, 86, 15, 86, 92, 86,169, 86,247,
     87, 68, 87,146, 87,224, 88, 47, 88,125, 88,203, 89, 26, 89,105,
     89,184, 90,  7, 90, 86, 90,166, 90,245, 91, 69, 91,149, 91,229,
     92, 53, 92,134, 92,214, 93, 39, 93,120, 93,201, 94, 26, 94,108,
     94,189, 95, 15, 95, 97, 95,179, 96,  5, 96, 87, 96,170, 96,252,
     97, 79, 97,162, 97,245, 98, 73, 98,156, 98,240, 99, 67, 99,151,
     99,235,100, 64,100,148,100,233,101, 61,101,146,101,231,102, 61,
    102,146,102,232,103, 61,103,147,103,233,104, 63,104,150,104,236,
    105, 67,105,154,105,241,106, 72,106,159,106,247,107, 79,107,167,
    107,255,108, 87,108,175,109,  8,109, 96,109,185,110, 18,110,107,
    110,196,111, 30,111,120,111,209,112, 43,112,134,112,224,113, 58,
    113,149,113,240,114, 75,114,166,115,  1,115, 93,115,184,116, 20,
    116,112,116,204,117, 40,117,133,117,225,118, 62,118,155,118,248,
    119, 86,119,179,120, 17,120,110,120,204,121, 42,121,137,121,231,
    122, 70,122,165,123,  4,123, 99,123,194,124, 33,124,129,124,225,
    125, 65,125,161,126,  1,126, 98,126,194,127, 35,127,132,127,229,
    128, 71,128,168,129, 10,129,107,129,205,130, 48,130,146,130,244,
    131, 87,131,186,132, 29,132,128,132,227,133, 71,133,171,134, 14,
    134,114,134,215,135, 59,135,159,136,  4,136,105,136,206,137, 51,
    137,153,137,254,138,100,138,202,139, 48,139,150,139,252,140, 99,
    140,202,141, 49,141,152,141,255,142,102,142,206,143, 54,143,158,
    144,  6,144,110,144,214,145, 63,145,168,146, 17,146,122,146,227,
    147, 77,147,182,148, 32,148,138,148,244,149, 95,149,201,150, 52,
    150,159,151, 10,151,117,151,224,152, 76,152,184,153, 36,153,144,
    153,252,154,104,154,213,155, 66,155,175,156, 28,156,137,156,247,
    157,100,157,210,158, 64,158,174,159, 29,159,139,159,250,160,105,
    160,216,161, 71,161,182,162, 38,162,150,163,  6,163,118,163,230,
    164, 86,164,199,165, 56,165,169,166, 26,166,139,166,253,167,110,
    167,224,168, 82,168,196,169, 55,169,169,170, 28,170,143,171,  2,
    171,117,171,233,172, 92,172,208,173, 68,173,184,174, 45,174,161,
    175, 22,175,139,176,  0,176,117,176,234,177, 96,177,214,178, 75,
    178,194,179, 56,179,174,180, 37,180,156,181, 19,181,138,182,  1,
    182,121,182,240,183,104,183,224,184, 89,184,209,185, 74,185,194,
    186, 59,186,181,187, 46,187,167,188, 33,188,155,189, 21,189,143,
    190, 10,190,132,190,255,191,122,191,245,192,112,192,236,193,103,
    193,227,194, 95,194,219,195, 88,195,212,196, 81,196,206,197, 75,
    197,200,198, 70,198,195,199, 65,199,191,200, 61,200,188,201, 58,
    201,185,202, 56,202,183,203, 54,203,182,204, 53,204,181,205, 53,
    205,181,206, 54,206,182,207, 55,207,184,208, 57,208,186,209, 60,
    209,190,210, 63,210,193,211, 68,211,198,212, 73,212,203,213, 78,
    213,209,214, 85,214,216,215, 92,215,224,216,100,216,232,217,108,
    217,241,218,118,218,251,219,128,220,  5,220,138,221, 16,221,150,
    222, 28,222,162,223, 41,223,175,224, 54,224,189,225, 68,225,204,
    226, 83,226,219,227, 99,227,235,228,115,228,252,229,132,230, 13,
    230,150,231, 31,231,169,232, 50,232,188,233, 70,233,208,234, 91,
    234,229,235,112,235,251,236,134,237, 17,237,156,238, 40,238,180,
    239, 64,239,204,240, 88,240,229,241,114,241,255,242,140,243, 25,
    243,167,244, 52,244,194,245, 80,245,222,246,109,246,251,247,138,
    248, 25,248,168,249, 56,249,199,250, 87,250,231,251,119,252,  7,
    252,152,253, 41,253,186,254, 75,254,220,255,109,255,255,255,219,
      0, 67,  0,  5,  3,  4,  4,  4,  3,  5,  4,  4,  4,  5,  5,  5,
      6,  7, 12,  8,  7,  7,  7,  7, 15, 11, 11,  9, 12, 17, 15, 18,
     18, 17, 15, 17, 17, 19, 22, 28, 23, 19, 20, 26, 21, 17, 17, 24,
     33, 24, 26, 29, 29, 31, 31, 31, 19, 23, 34, 36, 34, 30, 36, 28,
     30, 31, 30,255,219,  0, 67,  1,  5,  5,  5,  7,  6,  7, 14,  8,
      8, 14, 30, 20, 17, 20, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
     30, 30, 30, 30, 30, 30, 30, 30,255,192,  0, 17,  8,  0, 95,  7,
      8,  3,  1, 17,  0,  2, 17,  1,  3, 17,  1,255,196,  0, 28,  0,
      0,  2,  3,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      2,  3,  0,  1,  4,  5,  6,  7,  8,255,196,  0, 74, 16,  0,  1,
      3,  2,  3,  5,  6,  4,  3,  4,  8,  4,  4,  7,  1,  0,  1,  0,
      2,  3, 17, 33,  4, 18, 49,  5, 19, 34, 50, 65,  6, 66, 81, 82,
     97,113, 20, 98,129,161,  7,114,145, 35,130,177,240, 51, 67,146,
    162,193,194,209,225,  8, 21,178,241, 83, 99,226,242, 22, 23, 39,
     85,115,131,179,180,255,196,  0, 27,  1,  1,  1,  1,  1,  1,  1,
      1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  1,  3,  4,  5,
      6,  7,255,196,  0, 57, 17,  1,  0,  2,  2,  2,  1,  2,  3,  5,
      7,  1,  7,  5,  0,  0,  0,  0,  1, 17,  2, 18,  3, 19,  4, 33,
     49,  5, 34, 65, 81, 97,113,129,177, 20, 50,145,161,193,209,240,
     21, 35, 36, 66, 82,114,225,241, 51, 52, 53, 98,130,255,218,  0,
     12,  3,  1,  0,  2, 17,  3, 17,  0, 63,  0,200, 13, 77,219, 69,
    249, 30,104,217,253, 33,121,178,119,148,225,134,170, 67,165, 82,
     97,162,105,171, 21,124,160,183,134,148, 14,186,231, 97,194,119,
      6, 80,182,165, 43, 18,140,102, 34,156,220, 30,201,168, 99, 55,
     82, 30,106,251,172,165, 88,164,134,214,251, 40,166,217, 59,174,
    159,229, 87,171,166,192, 48,182,180,234,165,211, 96,156, 48,170,
     58,108, 29,194,138, 59, 20, 97,112,239, 35,118, 76,146,244,122,
    149, 89,153,165,165, 28,214,253, 21, 35,229, 70, 72, 91,166, 96,
    169, 52,112,150,188,205,170, 20,183,203, 25,177,106,157, 83,105,
    186,102,163, 40, 83,170,173, 70, 23,191, 75,251, 44,235, 54, 43,
    116,230,119, 82,151,120,166,241,204,177, 69, 15,124,228, 77, 38,
    246,186,181, 10, 21, 89,229, 91, 65, 79, 96, 62,139, 58,241, 44,
    179,  5, 52, 83, 74,176, 61,159, 53,208,178,197,115, 95, 69, 77,
     21,186, 41,161, 96,120,229,250, 37, 11,225,208,181,197, 40, 23,
      7,154,158,139, 11, 88, 97, 58, 33, 99, 57,217,173,254,200,124,
    169, 28,174,232,234,122,102, 66,142,143, 18,241,234,140,212,214,
     98, 90,121,145, 29,102,  9, 99,175,138, 82,104, 79, 17,189, 41,
     42,220,176,139, 57, 41, 86, 15,135,112, 30, 33, 77, 27, 20,112,
    227, 92,180, 69,108,166, 64,227,222,170, 83,109, 79,139, 47,162,
    154, 86,192,202,134,195,201, 93, 90,176,216, 59,182,132,110,195,
     17,248, 41,162,197,117,148, 90,195, 26,117, 74, 85,161,138,137,
     69,160, 99,186, 33,104,119,163,187,101, 37,132,150,249,168, 80,
     27, 15,229,122,149,  8,122,182,138,133,210,168,  9,140,113,209,
     28,212, 73,  6,136, 64, 72, 63, 42,149, 38,157,212, 22, 92, 80,
     91,  9, 68,143,122,128,179,160, 33, 34, 11,204, 16, 48,  6,210,
    185,170,176, 88,113, 26, 33, 66,206,228, 40, 53,  8, 80, 72,106,
    208, 52, 88, 47, 47,204,130,  6,160, 45,210, 22,176,  8, 70, 88,
    218, 71, 84, 74, 18,122, 32,188,212,  8, 43, 58,  1, 36, 34,149,
    100,104, 13, 79,121,  5,101, 33,  2,222, 71,251,161, 75, 97,240,
    118,117,162, 23, 31, 42,  3,102, 83,222, 64,121,135, 68, 98,218,
     91,213,  4,123, 65,209,  2,203, 86, 52,183,177, 20, 78, 68, 85,
    132,178,154,173, 20,224,224,128, 47,213,  4, 23, 84,209,  6,215,
    188,164, 67, 21, 16, 45,236, 35,188,129, 37,191, 43, 81, 84,182,
     23,119,172,141,211, 17, 84,142, 91,132,115,156, 22, 92,223,252,
     52, 77,  3, 43, 58, 58,133, 15,153, 89,104,176, 65,148,250, 45,
     72,234, 89,200,229,209,133,151, 29,114,221,115,106,112,159, 68,
     23, 31,171,172,140,164,146, 50,110, 52, 70,144, 98, 43,  2, 95,
     10,  4, 62, 34, 52, 90,170, 14, 98,137,212, 36, 34, 53, 87,239,
     34, 39,  5,230, 62,100, 77, 47, 55,204,133, 45,146,184, 33, 70,
    179, 16, 86, 51, 83,217, 48, 42, 92,181, 51,121,244, 11, 13, 84,
    255,  0, 69,166,165,150,142,173,106, 90,117, 13, 27,167, 39,178,
    167, 60,240, 86, 87,  3,221,162,212,234,215,132,156,131, 78,136,
    229,159, 27,165, 14, 34,182, 46,178,168,121,167,133,170, 57, 71,
    154,138,156,250,197,154,183, 58,171,115,152, 11,206,111, 85, 50,
     80, 59,202, 10, 76,253, 20,148,107, 36,166,139, 10, 53,146, 54,
    151, 71, 58, 51, 86, 89, 93,163, 82,201,167,244,138,173,154,141,
    143,183,117,109,157,104, 49,  3, 53, 11,149, 90,117, 30,248,123,
     45,180,117,140, 79,208,182,170,173, 61,107,223,142,226,155, 71,
     88,140,182,186, 89,170, 71, 32,165, 18,205, 68, 95, 65, 76,171,
    165,167, 82,234, 52,254, 41,102,165,156, 70, 67, 79,224,174,206,
    182,108, 68,239,232,174, 51, 58,217,141,238, 85,198,103, 91, 54,
     40,183, 95,242,149,123, 51, 86,121, 36,  7,250,203, 46,145,154,
    117, 34, 88,225,112,174, 91,248,222,171,102, 89, 56, 48,200,193,
     91,253,214, 39, 82, 37,107, 10,108,140,240, 32,102,170, 71, 51,
    207,159,  9,177, 98, 40,114,189,180, 94,142,199, 10,109,139, 19,
    187,184,186,190,198, 83,124, 24,152,228, 23,177, 87, 28,137,165,
     98, 26, 13,195,157, 95,162,235,216,202, 98,120,166,186,169,217,
    172,146,  2,127,209,102,202,103,198,179,246, 92, 41, 99,135,140,
    194,140,133, 77,181,199,123, 50,146, 20,218,128,202,141, 21, 88,
    118,242,157,213, 54, 83, 84, 15,175,149, 45, 52,102, 67, 90,244,
    252,223,236,163, 97, 55,119,169,183,234,178,196,103,236,207, 53,
     71,133,191,197, 44, 19,244,207,196, 22,218,169, 96,102, 21, 89,
    177, 65,221,185,218, 88, 38,197, 30,206, 92,185,173,224, 23, 46,
    197, 81,223, 15,107, 92,125,214,236, 81,145, 68, 73,190,106, 14,
    134,169,177, 77,164, 84,  1,150,158,232, 53,193,201, 78, 36,180,
    182, 70,214,238,233,215,244,254, 43, 45, 35, 17,158,250,128,248,
    152, 75,232, 22,161,208,138, 54,176,129,213, 45,142,238, 15, 13,
     80, 41,162,177,232, 54,102, 20, 54,156, 43,172, 15, 79,179,227,
    110,239,150,235,170, 93, 56,171, 69,232,249, 66,113, 15,232,163,
     96,252,  4,  5,197,111,239, 14,204,109,104, 11,209, 16, 29,153,
     80, 12,197, 54,  0,250, 85, 64, 32,224,182, 36, 16,125, 23, 75,
     11,123,174,166,210, 84, 95,210, 84,170, 27, 42, 21,137, 91,173,
      7, 84, 22,249, 44,129, 97,196,235,162,193, 85, 61, 52, 75, 82,
    200, 65,  3,205, 41,209, 45, 42, 54, 64,185, 36,106, 12,178, 72,
     18,194, 11,238,181,129,116,190, 11,160, 83,223, 65, 83,162, 88,
      1, 35, 29,203,246, 77,146, 45,240,211,248,169,177, 91,209,215,
     43, 82,197,190, 70,143, 84,177, 89,192, 21, 54, 75, 11,146, 95,
    160,252,181, 85,106,100,150,118,131,251, 60,213,245, 90,150,119,
    204,243, 98,150,194,164,118,236, 84,182, 79,162,161,249,246, 60,
     64,147,189,156,248,244,251, 47,133,214,254,152,211, 28,221, 52,
    254,125, 87, 41,192,179, 67,218,251, 59,139,249,244, 92,244,201,
     86, 99,  3,124,202,105, 86, 32,214,234,166,150,178,214,159, 50,
    202,104,131, 70,106, 11,123, 37,100, 47,118,107,227,238,185,139,
     25,193,160,204, 15,162,170, 85, 28, 36,150,151,106, 34,140, 18,
    182,151,101,209, 52, 47,216,145,224, 82,138,201, 50, 49, 77, 43,
     96,152,193,239, 44,166,236,  9,247, 48, 48, 62,121, 99, 99, 73,
     13,  5,238,  0, 84,232, 46,182, 48,153,246,108,243, 70, 62,242,
    134, 51,229, 80,233,216,155,151, 82,185, 80,181,100,111, 86,162,
    173, 50, 48,234,234,123,170,109,140, 66, 79,168, 70, 90, 50, 23,
    131,205, 81,232,228, 77,188, 38,218,237, 15,107,182, 78,199,219,
    248,182, 29,153,143,196,236,220, 84, 56, 76, 60, 17,108,249, 90,
    102,146, 79,135,112, 52,223, 56,155, 76, 90, 26, 46, 72,  6,189,
     23,187,143,135,135, 60,240,143, 88,137,137,159,120,250, 95,221,
    247, 62, 55, 55,149,229,241,113,242,101, 21, 51,140,196, 68, 84,
    250,204,235,247,207,219,236,238,118, 27,111, 79,183,224,218, 88,
    140, 92, 56,102, 69, 22, 61,204,193,186, 38,145,188,195, 58, 56,
    228,137,238,169, 60, 78,100,128,157, 53,161,  0,212, 46, 62, 71,
      6, 60, 83,140, 71,217,235,248,250,196,254,143, 95,133,229,101,
    207, 25,101,148,122, 68,250,126, 21, 19, 19,249,196,189, 27,225,
    134, 65,103, 47, 46,175,161,177,127,  9,243, 40,212,236,  9,129,
    193, 41,182, 18,199,133, 84,232, 26, 59,171, 92,137, 78,  2,235,
    255,  0,210,165, 69,238, 24,123,181,251, 41,166,128,225,200,228,
    253, 44,148, 89,102, 41,124,169, 74,176,101,120,238, 40,182,171,
    124, 65,163,216,174, 21,168,247,140,246, 68,234, 33, 36,148,180,
    141, 62,232,138, 53,178,176,142, 54,241,122, 39,202, 45,140,107,
    149,106,203, 94,232,116, 92,167, 12,147,178,110,221,209,212, 72,
    195, 37,198,120,136, 25,135,170, 23,136,196,244,230,106,203, 40,
    198,226,127, 68,134,106,102,251,244, 90,154, 77,224, 58, 34,107,
     33,141,223,215,243, 33,243,  4,128, 77, 22, 10,220,215,213, 77,
     22, 12,148,242,132,165, 88,173, 78, 84,165, 90,196, 93, 82,139,
     51,119, 65,111,186,153,130,192,  4,157,237, 22, 83,108, 71,210,
     53, 32,117,213,100,  9,146,171, 84, 19, 21, 59,171, 27, 97, 57,
    206,136,171, 16,107,144, 17,183, 34, 36, 76,112,247, 40,197,229,
    105,244, 82, 90,242,124,200,213,110,254, 84,  0, 98, 69, 40, 23,
      5, 66,234, 58,165, 10,222,116,208, 41, 12, 26, 89,213, 82, 42,
    143,170,193, 55,153,121,214,134, 50, 70,158,242,192, 85,114,  2,
    206,228, 74,103,241, 64, 84, 30,  8,  3,136, 45, 82,243,191,204,
    176, 86,254,244, 45, 66,135, 86,244, 68,210,103,162, 42,147, 61,
    249,150,166,151, 85,140,165,173, 18,255,  0, 68, 20,252,167,186,
    176,  6,154, 35, 64,240, 74, 58, 20, 90,124,191,165,150,164, 25,
    128,254,106,128,198,115,163,144,164,222, 61,166,136, 25,157,212,
    169, 65, 24,246,159, 50, 20,102,242,168,148,168,242,160, 45, 80,
      1,107, 79,186,194,192, 35,241, 69,  0,197,127, 68, 11,123,  9,
    176,114,213,  3,115,243, 93, 11, 76,190, 45, 64, 71,195, 42,  0,
     44, 30,100,105,114, 71,127, 20, 85,131, 33,  8,208,200,199, 81,
      2,244, 70, 82,179,159,162, 39, 84,204,124, 16,161,135,248,181,
     85,178,151,152, 34, 80,182,213,225, 65, 65,160,160,108,122, 81,
     74,  6,246, 26, 35, 89,203,124, 81,208,162, 35,211,137, 85,132,
     62,  0,129, 15,143,193,170, 92,195,150,136,  6,200,202, 10,196,
    208, 31,154,182, 86,168, 67, 45, 53,251, 41,110,171,141,254, 14,
    250, 34, 39,141,161,178,248,230, 68,106,115, 30, 60,203, 19, 56,
     14,181, 82,132,  1, 24,151, 66,134,194,  1, 84,231,214,211, 28,
    163, 68,115,207,  6,168,206,238,225,203, 92, 39,  6,152,230,207,
    173,149, 91,207, 60,102,135,180,158,102,149, 40,158, 52,225,175,
     50, 35, 85, 22,212,217,200,144, 94, 51,116, 84,  2, 71, 62, 43,
    230,144,132, 93, 12, 76,  8,227,109,254,138,169, 58,153,188, 30,
    255,  0,207,162,153, 53, 76,205, 70,106,170,170, 78,165,201, 49,
    238, 55,249,250,170, 35,  2,190, 41,222,197,109,171,173, 62, 44,
      3, 68, 71, 89,255,  0, 18,  0,169,211,162,196,244,137,146, 56,
    138,239, 42, 60, 10, 38,120,205,142,113,236,125, 21,163, 81,137,
     65,214,255,  0,188,134,160,121, 57,185, 84,217, 69, 76,  1, 30,
     10,172,213,150,114,232,227,168, 72,204,213,133,238,125, 43,154,
    190,139,174,204,235,101,144,141, 67,156,195,249, 86,246, 39,173,
    159, 51,216,250,230,117,124, 85,246, 38,120, 80,200, 95,171,170,
    155, 35,172,  7, 55,209, 85,162, 96,137, 98,  4, 89,206, 14,241,
     75, 78,170, 12, 45, 28,121, 95,234,174, 57, 28,231,132, 64, 52,
    122,126,234,239, 25,188,185,241,155,148,234, 23,107,113,212,214,
     77, 51, 59,202,118, 53, 71,201,188,215, 95, 85, 91, 20, 95, 23,
    209, 45, 52, 92,236, 18, 91,170,155, 85, 50, 75, 30, 95,217,245,
     75, 28,124, 92, 21, 36, 31,209, 85,165,203,196, 97,247, 98,185,
    104,186,218,136, 28, 75,104, 19, 45,166,171,  6,152, 49,133,182,
    154,234, 41, 45,140,118,126, 46,239,230, 92,166, 85, 66,120,  7,
     70, 87,221, 70,197, 39,245,116,123, 91, 85,118, 35, 52,166,138,
     65, 82,129,  3, 32, 25,141,156,178,218,232,198,208, 69, 23, 57,
    145,162, 56,242,107,111,221, 81,176,124,112,131,202,179,177,131,
    142, 42, 63,205,238,174, 51, 75, 92,108, 46,182, 95,236,171,180,
    156,192, 67,232,168,110,130, 49,170, 33,191,  1, 21,100,169,109,
    145,143, 65,130,140,107,168,244, 91, 13,119,240,  3, 74,125,215,
    163, 12, 88,239,197, 31,236,197,108,189,  9,106,123,242,178,129,
    202,172, 72, 34,222, 93,202,  7, 75,  9, 24, 10,224,109, 14, 94,
    139, 17,233, 96, 26,162,192,188,133,207, 96,189,224,  9,176,134,
     90,165,136, 36,113,178,235,  0,216,194, 74,239, 16, 52, 83, 32,
     86,148,173,123,202,108, 67, 32, 29,212,176,183,191,193,101,136,
    215, 58,139,148,230,164, 15, 62, 84,216, 81,155,162, 88,176,235,
     87, 53, 22,216,207, 46, 38,135,197, 85,165,153,243,151,165,133,
    230,241,117,210,201, 45,243,101, 75, 97,102,119, 16,168, 33,243,
     94,246, 30,171, 69,103,169,224,255,  0, 68, 14, 26, 32,172,222,
      9, 98, 73, 32,209, 45, 36, 98,102,224,182,101,116, 17,190,119,
    211,213,  2,115,181,239,224,191,238,163, 11, 56,129,119,233,251,
    170,154,199, 60,219,202,243, 31,222,167,248,170,219, 81,240,150,
    176,183,135, 51, 67, 60, 50,255,  0,162,249,182,254,144,123, 90,
    105,192,219, 41, 28,222,211,226, 49, 56, 46,205,109, 92,102, 25,
    217, 39,131,  7, 52,177,190,128,229,115, 88, 72, 52, 54, 55, 29,
     83,142, 34,115,136,151, 31, 39, 57,195,135, 60,177,247,136,159,
    209,242,238,201,237,127,197, 78,210, 96, 36,198,236,189,177,135,
     48,199, 41,137,198, 72, 97,105,204,  0, 62, 79,  2, 23,175, 62,
     15, 27,  9,169,199,245,124, 31, 27,159,226, 62, 78, 51,150, 25,
    250,126, 95,217,244,174,192, 69,218,232,240,120,150,246,175, 25,
     14, 34,115, 32,220, 24,195,  0, 13,165,249, 90, 58,175, 23, 55,
     23, 12,204,117,195,237,120, 83,228,198, 51,251, 68,220,253, 61,
    191,163,207,109,156, 23,227,  7,252,219, 24,237,157,181,176,113,
    224, 78, 33,231, 12,215,  8,106, 35,204,114,131, 86, 19,165, 53,
     93,176,199,198,214, 54,143, 95,205,228,230,143,138, 79, 38, 90,
    101, 26,220,215,183,183,211,232,197,248, 87,218,142,215,109, 30,
    221,237, 13,133,183,182,147,113, 35,  9, 12,161,236, 16,198,208,
     36,100,141,105, 32,181,160,157, 74,207, 47,131,143, 30, 40,207,
    142, 61,211,240,207, 59,202,228,242,178,226,230,202,234, 39,233,
     30,241, 49, 31, 72,116, 63, 16,187,117,182,240,221,169,139,178,
     61,148,193, 68,253,164,224,221,228,178, 81,196, 18,220,192, 52,
     18,  0,163,110, 73,175,217,114,241,252, 78, 57,227,237,229,159,
     71,111, 63,226, 92,216,243,199,141,227,199,204,207,131, 31,140,
    248,108, 84, 18,226,102,193, 99, 32, 50, 55,123, 27, 91,  0, 33,
    181,190,129,167, 79,  2,174,127, 99,152,244,244,254, 41,195,253,
     95, 28,162,114,152,152,252,191,236,250,188,146,178, 56,221, 36,
    174,107, 24,208, 75,156,227, 64,  0,234, 74,249,186,219,239,205,
     68, 92,188,182, 39,241, 47,176,184,108, 65,129,253,160,132,188,
     26, 86, 56,164,123,127,180,214,145,247, 94,136,240,185,230, 47,
     87,205,203,226,222, 30, 51, 83,159,235, 63,209,232, 27,182, 54,
    107,246, 27,182,219,113,141,118,206,108, 14,196, 25,218,210, 70,
    237,160,146,104,  5, 77,129,181, 42,184,117,229,190,149,234,245,
    247,241,207, 31,109,252,181,119,247, 57, 31,252,194,236,115,118,
     75, 54,169,219,241,140, 43,228,116,108,115,163,120,115,156,218,
     86,140, 45,204,105, 81,122, 82,235,175,236,124,219,107,175,171,
    201,254,165,226,233,217,191,167,231,250,123,186,253,157,219,251,
     39,180, 56, 35,140,216,248,216,177,176,181,217, 92, 89, 80, 90,
    124, 11, 77,  8,250,133,207,147,135, 62, 57,172,162,158,142, 15,
     35,139,159, 29,184,230,225,242,159,198, 13,181,216,174,215, 71,
    132,193, 14,215,124,  4,216,  9,228, 15, 99,176, 51,189,142, 38,
    128,214,141,212,101,177,245, 43,233,120,124, 92,220, 23, 58, 93,
    253,240,248, 31, 19,231,241,124,168,140,123,106,113,159,178,127,
    179,235,125,153,248,103,118,107,102,124, 30, 41,248,188, 63,194,
     68, 34,157,224,135, 74,220,130,143, 32,220, 18, 47,127, 21,242,
    185, 98,119,155,138,155,125,239, 30,113,234,199, 89,184,168,245,
    251, 89,102,237,111,103, 96,219, 24,237,145, 62,213,138, 28,102,
      2, 17, 62, 41,146, 53,204,108,108, 57,104, 75,200,202,121,219,
     96,107,117, 81,227,114, 78, 49,148, 71,164,166,124,238, 24,207,
     46, 57,202,166, 61,103,252,246,250,177,108, 94,222,246, 75,108,
    237, 22,236,237,157,183,112,242,226,156,114,178, 55, 49,241,231,
     62, 13, 46,  0, 19,236,175, 63, 15,151,  8,219, 44,125, 14, 47,
    138,248,220,185,105,134,126,172,159,137,125,179,216,253,156,217,
    216,237,155,136,218, 99, 11,182, 37,217,242, 75,131,104,133,238,
     37,196, 57,172, 33,193,165,163,137,189, 79, 75,173,241,124, 92,
    249,114,140,162, 47, 27,245,103,196, 62, 37,197,227,225,150, 19,
    149,103, 49, 53,233, 63,125,127, 55,131,252, 34,252, 73,217,155,
     63, 97, 98,217,218,206,208, 98, 29,141,147, 24,231,176,204,201,
    102, 57, 50, 48, 10, 16,210,  0,168, 54, 94,207, 55,193,207, 44,
    227,171, 31, 74,251,161,242,126, 19,241,126, 46, 62, 44,163,201,
    228,157,175,235,115,233, 80,250,238,194,219,155, 63,109,108,184,
    118,158,204,159,125,132,155, 54,238, 76,142,109,104,226,211,103,
      0,117,  7,162,249, 60,156,121,113,229, 56,229, 30,175,210,112,
    115,113,243,241,199, 39, 28,220, 75, 54, 35,100, 96,166, 56,178,
    233, 39, 63, 23,143,131, 29, 37,  8,180,144,238,114,129,110, 83,
    184,101,122,220,220, 90,151, 28,211, 21,247, 68,199,241,191,238,
    101,226, 99, 59,125,243, 19,249,197,127,104, 59,101,108,236, 14,
    205,118, 43,225, 25,187,102, 34, 86,200, 99,  0,  6, 71,150, 40,
    226, 13, 96,  0, 81,161,177,182,222,254,202,115,228,156,234,254,
    159,222,255,  0,171,120,188,124,120,175, 95,175,246,136,244,252,
    161,176,211,163,220, 20,106,181,137, 36, 26, 58,171, 13, 77, 24,
    135,119,150,211, 41,123,200,220,148, 37,147, 86,129,206,166,173,
     81,169, 96, 38, 39, 44,213,105,186, 20,179,154,183, 85, 90,178,
     26, 89, 40,178,200, 36, 95,254,165,133,151, 72,254, 90,165, 44,
    169, 26, 15,153, 41, 65,221,131,110, 34,166,128,136,242,232,234,
    123,170,136, 44, 89, 31,175, 55,178,180,137,179, 74,  5, 50,217,
     36,212,214, 98, 70,133, 69, 34, 96,220,226,149,204,148,145,223,
    221, 40,180,202, 58,182,158,202,117, 45, 65,162,182,119,247,147,
     85, 88,139, 71,153, 77, 10,  1,189,205, 83, 85, 15,138,138, 40,
     14,103,244,114, 82, 77,206,235, 85, 86,186,164,192,239, 20, 10,
    239, 35, 44,209, 43,155,222, 83, 39,239, 13,179,  2,104, 91,245,
     88, 26,  4,103, 69,180,157,148, 98,186,201,193, 81,152, 76, 95,
    153, 78,173,216, 59,178, 59,202, 41, 86, 23,208,107,100, 85,171,
    247,145,182, 58,120,163, 45, 96, 55,202,133,172, 50,168,155, 67,
     10,149, 90,168,239, 50, 83,109, 26,133,161,202, 84, 40,  5,173,
     60,137, 97,111, 97, 70,130,201, 74, 74,191,223,209, 40, 16,127,
    209, 20,174, 36, 23,189, 12,238,160, 99, 36,  5, 41, 52, 48,250,
    160,132,158,142, 82,145, 52,248,172, 17, 21,107,200, 41,234,133,
    148, 67,198,136,164,107,207, 86,160, 99, 75, 80, 21,188,203, 64,
    212,132,115, 53,146, 31,162,198, 10,173, 62,101,162, 17,224,228,
     21,187,125, 57,144,178,116, 69,210,200,  5,  0, 22, 81,  4, 96,
      7,157, 18, 19, 21,252, 66, 41, 11, 60, 44,138,177,212,244,106,
     38,134,202,117, 68,142,158, 14, 70, 38,244, 27, 22,209, 96, 18,
     17,160,191, 68, 85,179,191, 91,162,193,156,244, 65, 70, 83,213,
     10, 11,223,161, 11, 85, 74, 15,169,226, 66,146, 66,107,192,133,
      1,149,235,170, 53, 55,158, 40, 41,247,186,  0,200, 10,197, 89,
     50, 10, 45, 11, 37,192, 32, 56,223,110, 53, 72,212,108,125,252,
     66, 20, 97,165, 63,193, 18,140,115,199,178, 83, 26, 34,118, 96,
    165, 40, 98, 21, 68,149, 44, 39,162, 42,200,163,252, 21, 40, 46,
    141, 41, 50, 83,226, 62, 84,164,150,248, 42, 44,218, 37, 50,201,
     49,190,183,106,148,166,239,193, 85,  5,150, 56,104,223,213, 75,
    173,128, 66,  1,168,230, 64,116,122,162,150, 44,137,156, 15,142,
     69, 46,125,103,  3,243, 89, 28,231,  1,213,189, 17,148, 33,116,
     77, 40,213,189,234, 21,133, 14, 41,158, 13,243, 35, 53, 63,126,
    254,151, 11, 92,186,198,220, 71,139, 81, 25,240,155, 20,205, 58,
     61, 28, 39,140,237,225, 30,200,137,193,123,223,  7, 85, 84, 35,
     81,202,237,227, 40, 29, 68,166,198, 12,228,211,147, 53, 84, 83,
    173, 38,119,117,109,127,159, 69, 52, 72,  4,180,239,127, 21, 70,
    166,111,137,209,203, 78,144, 77, 45,174,135, 91, 57,120,232,135,
     90, 23,142,168,142,176,  9, 25, 84, 79, 90,247,164,104,228,182,
    106,116, 88,151, 42,183, 41,193,173,152,168,180,118,170,237,198,
    112, 51, 59, 77,195,157, 68, 40,185, 38,240,186,198,211, 35,222,
    226,105,153, 34, 21, 68, 77, 95,152,251, 43,117,213,146,120,219,
    243, 51,238,135, 91, 47, 23,211,197,107,157, 12, 42,180, 79, 26,
    200, 85,110, 51,128, 50, 14,171,109,203, 80,200,218,242, 42,180,
    206,  1, 32,133,187, 57,207, 24,133,254, 85,210, 51,114,158, 52,
     14,169,166, 85,218,220, 40,105,179, 40, 61, 60, 19,100, 81,101,
    181,186,171, 77,  6, 90,185,245, 99,104,166,211,171, 30, 46, 12,
    214,234,150,106,195, 46, 22, 70,106,218,251,166,204,167, 63, 17,
    133,205,221,200,170, 51, 41,146, 76, 52,131,250, 54,213,119,142,
    108, 82, 12,131,191, 98,168, 62, 41, 36,201, 70,102,160, 92,115,
    197, 77, 81, 75,152, 80,182,133,121,229,163,111, 50,219, 41,100,
     26,165,131, 35, 47,122,233,108, 62, 44,227,159,236,160,166,236,
     57,249, 91,247, 81, 99,104, 34,138,  6,188, 56,105, 23,202,165,
     45, 17, 70, 73,244, 85, 18,198,152, 32,253,165, 88,218,174,208,
    150,131,133, 26,228,186,187, 65,240,198, 71,121,202,135, 75,  8,
    193,229,253, 81, 46,246, 16, 59, 39, 43, 71,232,167, 12,135,115,
      0,209,144, 30,171,219,199, 35,179, 27,235, 26,235,104, 50,229,
      7, 67,  8,209,213, 88,221, 24,  1, 80, 44,202,236, 76,202,108,
     71,184,  5,155, 12, 50,205, 71,115, 40,149, 23,188,204,167, 12,
    246,  5,118,232,187, 80,215,132, 46, 55, 46,250, 46,152,141,128,
    180,247, 87,162, 28,237,  1,105,244, 73,205, 68, 72, 72,246, 92,
     39, 49,156,226, 29, 90,  7, 44,177,113,184, 27,245, 83,106,161,
    190, 78,133, 77,128, 14, 31, 50,171, 20, 94, 65,170,173,146, 91,
    228,115,143, 55,209,108, 74, 72,151, 85,116, 18, 28,  1,230, 89,
     34,167,145,197,235,113, 25, 36,118,127, 50,161, 51,144,206,234,
     88,205, 36,140,173, 31,170,166, 25,135, 55,181,148,216,113,196,
    119,  2,171,  8,146,111, 27, 45,  8,151, 16,214, 50,166,254,202,
    134,127,136,182,126,137, 50,  0,206,255,  0,127,101, 49, 33,121,
    201,213,212,246, 75,  0, 42,198, 56,229,168,245, 81,176,202,249,
    115,233,155,217, 68,230, 83,225,216,109,163, 14, 74, 72,238, 35,
    209, 69, 63,163, 83,100, 82,182,181,100,159, 69,206,134, 30,219,
    202, 95,216,221,187,155,255,  0,183, 98, 45,255,  0,235,114,174,
     40,249,227,241,121,124,191,253,190,127,132,254,143,142,126, 24,
    193,219,249,118, 38, 32,246, 83, 31,133,195,224,254, 36,137, 27,
     43, 35, 36,201,149,181, 60, 77, 39, 76,171,223,205, 60, 91,124,
    240,252,239,195,240,243, 50,227,158,137,168,191,187,223,248, 62,
    195,248,127,135,237,116, 88, 44, 80,237,118, 47, 15,136,156,200,
     55,  6, 48,192,  3,105,126, 86,142,171,199,203, 56, 95,200,251,
    222, 30, 62, 76, 99, 63,180, 77,207,209,233,216,195,211, 49, 92,
    158,219,124, 95,240,180,127,245,203,180,255,  0,155, 25,255,  0,
    250, 26,189,188,223,250, 24,254, 95,163,243,190,  7,255,  0, 33,
    201,255,  0,235,245,119,255,  0, 18,191, 15,182,158,212,219,236,
    237, 63,102,241,227, 13,181, 26, 27,157,142, 37,153,139, 69,  3,
    154,225,161,165,  5, 13,141, 23, 46, 30,104,199, 29, 51,143, 71,
    179,207,248,118,124,156,157,252, 51, 89, 56,184, 95,196,110,216,
    246, 87, 29, 14,  7,183, 27, 32,201,  3,141, 55,237, 96,107,200,
    234, 65,111,  3,233,224, 41,238,170,124,110, 46, 72,190, 57,121,
    241,248,167,149,227,101, 24,249, 56,250,125,191,231,164,181,255,
      0,196, 38,220,149,157,152,217, 56, 92, 12,231,225,118,169,116,
    206,123, 13, 55,145,180, 52,129,236,115,131,244, 92,252, 46,  8,
    140,230,103,222, 29,190, 57,229, 79, 78, 24,225, 62,153,122,254,
     95,228,189, 23,103,255,  0, 12,123, 39,132,216,208, 97,177,123,
     46, 44,110, 33,209,131, 52,242, 57,213,115,136,185, 20, 54, 30,
     20, 92,121,124,158, 89,202,226,105,236,224,248, 87,139,135, 28,
     70, 88,220,253,101,213,237, 38,207,194,108,159,195, 93,177,179,
    112, 81, 24,240,208,108,172, 75, 98,105,113,118, 81,187,117,170,
    110, 87, 14, 57,156,249,177,202,125,238, 30,207, 35, 14, 62, 47,
     15, 62, 60, 61,163, 25,253, 37,243, 31,192,158,198,108, 93,189,
    179,113,155, 95,108,192,113,130, 41,142, 30, 24, 28, 72, 99,120,
     67,139,172,110,120,190,151,250,125, 15, 55,159, 62, 57,140,113,
    244,126,127,224,190,  7, 15, 62, 25,114,114,197,212,213, 61,151,
    225,143, 97, 54,175,100, 59, 79,181, 49, 95, 25,132,118,203,197,
     49,205,134, 40,222,242,241, 71,213,153,170,208, 44,220,195, 83,
    114,188,158, 79,145,143, 54, 17, 21,235, 15,167,240,239,134,242,
    120,124,217,229,113,172,251,127, 31, 79,228,224,127,196,  7,102,
    246, 46,202,236,180, 27, 67,  1,178,112,216, 92, 76,219, 69,173,
    146, 88,219, 71, 56, 22, 72, 72, 62,228,  3,244, 93,188, 14,108,
    243,228,156,114,159, 74,120,254, 57,226,112,240,240, 70,120, 99,
     83, 51,253, 37,244,207,195,169, 26, 59,  5,176,  1, 20, 35,102,
    193,255,  0,243,106,240,121, 17,254,215, 47,198, 95, 99,193,143,
    247,110, 63,194, 63, 71,201,118,246,192,194,118,159,254, 33,182,
    142,204,199, 75, 43, 48,133,145, 73, 43, 99,177,144, 54,  8,206,
     90,244,  4,209,125, 46, 62, 89,226,241, 35, 40,247,255,  0,188,
    190,  7, 55,139,143,147,241, 76,184,242,159, 79, 79,210, 13,252,
    122,236,134,195,236,238,200,217,123, 95, 97, 96,219,179,166, 24,
    157,203,132, 79,119, 23,  9,115, 93,115,168, 45,215,213,103,131,
    228,103,203,148,227,156,219,126, 51,225,113,120,248, 99,201,197,
     21, 55, 79,113,218,157,149,178, 59, 69,248,101, 39,105,182,166,
    204,195,207,180,255,  0,248,120,204,201,222,222, 38, 29,201,144,
     83,217,206, 37,121,120,178,207,139,155,175, 25,244,191,234,250,
     62, 71, 23, 31,145,226,119,103,141,229,173,223,229,111, 35,255,
      0, 15,189,150,236,230,221,236,150, 55, 21,182, 54, 78, 27, 25,
     59, 49,238,141,175,145,181, 33,187,182, 26,126,164,254,171,209,
    231,243,114, 97,201, 17,140,215,163,193,240, 95, 23,131,155,135,
     44,185, 49,137,155,254,144,250,254,206,217, 88, 13,153,128,143,
      1,179, 32,143, 11,134,138,185, 34, 96,163, 91, 82, 73,251,146,
     87,202,206,114,206,118,202,110, 95,166,226,195, 14, 44, 99, 12,
     34,162, 12,147, 13, 32,242,149, 29,111, 71,115, 59,225,147,202,
    224,167, 85,199, 33, 47, 47,143,253,209,222, 39, 21, 55, 21, 33,
    177,141,212, 77, 77, 81,216,200,226,107,159, 35,131, 90,209, 82,
     73,160,  3,197,103,186,103,  8,136,185, 84,155, 91,102,178, 35,
     36,152,220, 51, 88, 35, 18,151, 58, 86,128, 24,123,213,175, 47,
    174,139,174, 56,101, 63, 71, 25,203,142, 34,231, 40,246,191,127,
    167,219,248, 22,118,246,200,143,  8,204, 81,218,248,  6, 65, 32,
     46,100,142,196, 48, 49,192, 16,  9,  6,180, 52, 36, 15,114, 22,
    245,103, 51, 85, 54,225, 60,252, 49,140,101, 57,197, 79,223, 14,
    140,115,239, 35, 15, 14, 14,107,133, 65,  6,160,133,201,218, 34,
    253, 97,204,193,109,188, 44,241, 99,228,196, 71, 38,  0, 96, 38,
    220,206,113, 46, 99, 64, 57, 24,240,234,181,196, 80,181,237,234,
    186,101,197, 49, 85,235,127, 99,142, 30, 70, 57, 70, 83,151,203,
    172,212,221,125,145, 63,111,222, 55,237,173,138,216, 33,159,254,
    109,130, 17,207, 93,203,206, 33,153,100,161,  0,229, 53,189,200,
     22,234, 66,231,211,157,204, 84,250, 55,246,174, 26,137,218, 42,
    125,189, 96, 88,205,167,179,240,184,200,112,152,157,167,131,131,
     19, 57,  2, 40, 36,157,173,124,132,154, 12,173, 38,166,246,178,
    216,226,207, 40,184,143, 69,207,145,199,134, 81,134, 89, 68, 76,
    251, 69,159, 38,240,223, 47,209, 67,209,120,150, 94,122,216,170,
     91, 35,118,158, 17,216,211,129, 24,220, 49,197,141, 96,222,183,
    120, 44, 15, 45,107,161,  7,234, 22,105,150,187, 87,163,159,111,
     30,250,109, 27,125,151,235,252, 21, 14,217,217,178, 98, 93,134,
    139,104, 97, 36,157,175,200,232,219, 51, 75,131,175,194, 69,107,
     91, 27,122, 21,189,121,196, 92,199,161,143, 55, 22, 89,107, 25,
     69,254, 45, 17,227,112,242,185,237,108,140,121,141,217, 94, 26,
    106, 90,104, 13, 15,129,161,  7,234, 22, 76, 76, 42, 42,110,167,
    217,150, 61,174,207,249,216,217,114, 96,177, 49, 57,208,190,104,
    230,113, 97,142, 70,176,176, 58,148,113,112, 53,145,186,129,213,
     95, 95,201,188, 79,249,254, 67,207, 60,223,237,122,167, 25,143,
     73,155,244,169,170,251,239,235,245,131,176,187, 99,100, 78,247,
     50, 13,169,131,149,205,120,141,193,179,180,144,243,163, 77, 14,
    166,134,222,139, 39,143, 56,247,137, 78, 60,252,121, 77, 99,148,
     79,211,222, 23,138,218,155, 31, 14,220,216,173,165,129,129,187,
    195, 29,100,157,173, 25,198,173,185,215,209, 35,143, 60,189,162,
     76,249,248,176,139,203, 40,143,205, 76,218, 44,151,106,203,128,
    194,224,241, 88,141,195,218,201,231, 97,140, 71, 19,156,208,224,
     14,103,  7, 30, 23, 52,240,131,175,186,190,186,199,105,148, 71,
    145,124,147,134, 56,204,215,188,250, 84,125,126,219,254, 16,210,
    221,165,179,158,217,195,118,158,  8,156, 51,218,201,192,157,191,
    178,115,142, 86,181,215,225, 36,216,  3,169, 89,211,151,167,167,
    185,251, 79, 31,175,205, 30,158,254,190,204,242,246,139, 99,193,
     14, 10, 65,142,195,226, 35,198,226, 91,134,133,208,204,199,  7,
     60,154,107, 91,208,210,180,169,184,178,168,224,206,110, 43,219,
    213,203, 47, 51,138, 35, 25,137,187,154,138,166,233,118,174,201,
    139, 22,112,114,109, 76, 28, 88,144, 72,220,186,118,135,217,161,
    199,134,181,229, 32,251, 26,174,113,197,156,197,196,122, 58, 79,
     63, 28,101,172,229, 23,246, 91, 54,196,219,187, 39,109,178, 71,
    108,221,161,  6, 36,197, 35,216,246, 50, 86,185,195, 43,220,204,
    212,  4,240,146,218,131,212, 16, 85,114,112,103,199,251,208,112,
    121, 92, 92,247,215,148, 77,127,122,255,  0,195,163, 96, 63,163,
    253, 46,162, 33,232, 70, 26,168,156, 26, 42,181, 77, 11,224,239,
    125,214, 75, 44,108,203,230, 77, 75, 50,131,249,114,153,193, 40,
     26, 58,230, 72,128, 84,  4, 81, 85,  8, 35,167,250,169,212, 16,
    168,245, 85, 16,201, 24,123,210,147,102, 10,158,101, 58,170,208,
    198,215,114, 41,158, 51, 98,223, 17, 10, 53,110,197,229, 43, 41,
    118,177, 84, 36, 84, 70, 90,197, 84,209, 99,204, 58,169,152, 34,
     85,149,133,101, 46,192,248,  7, 69, 58,170,201,150, 55,134,172,
    213,182,  0,199, 37, 40, 36, 35, 67,144,123,122,169,181, 90, 22,
    188,114, 58,161, 11, 85,251,205, 64, 99, 41, 69, 38,228,116,106,
    166,216,114, 60,105,100, 97,140,116,128,115, 53,  9, 27, 30,227,
    171, 84,185,153,194,128,171,100, 18,149, 82, 44,197, 84,  1,185,
     31, 84, 82,185, 79,138, 41,122,139,172, 17,190,136,145,102, 43,
     68,171,138, 36, 96,161, 72, 93,245, 88, 32, 32,247, 80, 11,218,
    212, 18,149, 90, 42,225,  4,160, 58,160,151, 88,164, 65, 44,137,
    165,156,171, 69, 85,  4,167,133,144, 38, 75,217, 21,108,178, 68,
     81,118, 81, 62, 40,180, 25, 58, 57, 98,151,116,  1,154,136, 33,
     57,214,137,148,249, 80, 76,174,  5,  1,154, 16,136,  1, 21,238,
    163, 72,124, 70,170,148, 81,  7, 58,149,141,134,158,138,152,108,
    127,170,164,142,133, 45, 41,148,139,230, 70, 81,140,159,163,191,
     85, 37, 13,207, 61, 27,111, 20, 70,165,230, 21, 84,209,150,212,
     35, 38, 10,  2,252,185,146,220,232, 70, 32,123,168,192, 62, 16,
    165, 36,190, 26,119, 80, 33,237,241,106, 46,  3, 70,121, 84,186,
      5,244,233,117, 84, 42,128,235,100,  3,148,  5, 41,161, 17,242,
    161, 67,  6,154, 57, 19,169,140,123,186,185, 25,169,236,161,210,
    235, 19, 48,186, 87, 91,163, 41, 50, 83, 69,174,116, 43,252,202,
    153, 64, 14,112, 82,137,131, 42,117, 18, 59,217, 85, 57, 78,  3,
    100,230,156, 77,253, 44,159,186,137,227,104,100,131, 37, 66, 91,
    157, 33,144, 41, 32, 47,120,247, 71, 77, 89,167,112,173,145,210,
     32,151,200,224,142,148, 29,248,246, 40, 71, 24, 29, 58,198,245,
    173,146,180,169,103, 88,170, 61,209, 51,198,182,129,209,206,246,
     84,229, 60,105,188,144, 37,185, 78,  7,  9,195,173,215,213,108,
     75,158,120, 24, 30,211,108,203,163,149, 32,157,205, 52,107,168,
     22,167, 84, 15, 18,234,235,161,168, 94, 58, 42, 80,104,225,252,
    213, 26, 23,192,199,138,247,145, 20, 80,128, 29, 81, 84, 84,152,
    119, 86,203, 77, 72,146, 39,  3, 68,180,206,  1,204, 90,150,227,
    214,182, 61,167, 87, 42,180, 79, 24,205, 10,171,115,235, 76,182,
    166, 85,182,231, 60, 97,202, 60,202,246,114,158, 17,229,105,215,
    149, 86,206, 83,192, 91,218,195,104,254,246, 85,110, 90,151, 37,
     34,190,170,109, 90,133,143, 18, 95, 79,221, 83,179,122,209,244,
     34,237, 75, 71, 91, 14, 33,128,249, 74,173,145, 56, 49,191,  8,
     14,150, 85,178, 39,  6, 25,176,217, 13, 11,105,235,154,171,164,
    102,225, 69,190, 39, 84, 82,237, 93, 59, 21, 70,210,223,179,183,
    138,230, 15, 77, 50,172, 23, 29, 94,251,162, 91, 12, 96,139, 41,
    176,192,199, 22,242,255,  0,121, 98,154, 48,236,240,115,151, 52,
    181,178,163,158,202, 24,213,134, 38,137, 48,151, 67,  8,231,117,
    209,114, 41,210,194, 17, 91,127,212,186,198,108,167, 69,140,105,
    101,243, 46,246,228,145, 65,153,235,108,116,160,132,105,252, 85,
     90, 91,240,205,162,216, 83,183,179,164,109, 50,102,186,244, 97,
    154, 93,104, 13,  5, 10,239,177, 77,112, 11,218,225, 77,177,189,
    133,173, 93,224, 57,178, 13, 24,182,195,110, 22, 91, 87,170,149,
     51,226,158,222, 76,220, 74,109, 44, 50, 56, 86,153,110,179, 60,
    213, 64,  5,204, 61,229,203, 15,149, 45, 17, 60, 62,197,122,182,
    216,107,129,217,108,187, 64,118,251, 32, 93,109, 20, 15,136,162,
    137,149, 51,200,242,243, 85,202,201, 32,188,  2,166,210, 49, 39,
    130, 40, 66,233,109, 25,229, 85, 12,101,149,234,233, 41, 28,131,
    230, 65, 82,188,245, 87,176,195, 59,219,209, 68,200, 88,197, 82,
    221, 87, 41,144,189,231,143, 85, 81, 34,223, 32,201, 69,214, 37,
     36, 25, 56,237,167,229, 87,  2,234,  7, 30,107,171,176,185, 49,
      7, 78, 20, 25,243,185,134,191,197, 64, 91,205,235,153, 93,129,
      2,249,148,204,128,148,143,101, 22, 23, 35,137,111, 50,205,128,
     16, 60,170,108, 47, 38, 71,215, 47,233,101, 35,243,156, 82, 55,
     77, 79,213,122,159,211, 15,130, 73,  7, 30,158,202, 38, 18,207,
    218, 89,113, 18,246,103,106,197, 30,246,105, 38,193, 76,198,183,
     87, 56,150, 16,  0,  3, 82,152, 69,101, 15, 63,151,140,229,193,
    156, 71,189, 79,232,249,215, 97,118,247,106,251, 41,179, 38,192,
    225,187, 43,139,196,178, 89,140,197,210, 97,228, 20, 37,160, 82,
    195,229, 94,158, 94, 60, 57, 38,231, 39,230,252, 46,111, 47,196,
    194,112,142, 25,155,155,246,159,236,250,111, 97,251, 87,181,182,
    182, 26,121,118,214,200,155,102,186, 55,134,198,215, 68,225,156,
     83, 94, 32, 23,151,147,143, 28,103,229,155,125,223,  7,155,151,
    159, 25,158, 92, 53,175,199,250,184,123, 79,183, 31,136, 27, 55,
    105,227,176,208,246, 99,227,240,219,249, 62, 18,102,192,247,126,
    207, 49,201,155, 33,161,181, 60, 15,138,233,143, 15, 22, 81, 19,
    179,231,242,249, 94,111, 22,121, 99, 28, 87, 23, 53, 53, 62,223,
     79, 99, 63,  5,187, 53,181,182,126,217,218, 61,166,219,237, 17,
     98,241,173,115, 91, 19,136,204,115,188, 61,238,112, 26, 84,129,
     65,238,158, 71, 46, 51, 17,142, 44,248,103,135,203,134,121,115,
    114,250, 76,183,246,199,181, 93,188,216,253,168,197,197,179, 59,
     48, 54,158,201,224, 48, 61,144, 61,206,228,110,107,176,249,179,
    106, 20,241,241,241,101,140, 92,212,175,201,242,188,190, 46, 89,
    140, 48,219, 31,167,167,221,247, 60,191,104,153,219,255,  0,196,
    135, 96,246,110, 43,179,163, 99, 96, 98,155,120,233, 39, 99,153,
     67, 66, 51, 18,251,154,  2,108,209,254,221,112,234,224,185,137,
    185,120,185,255,  0,108,243,235, 12,176,214, 33,237,127, 18,191,
     15, 95,183, 59, 25,179,246,102,200,120, 56,189,147, 27, 91,134,
     18, 26,111, 26, 26, 26, 90, 79, 66, 67, 65,175,136,250,174, 28,
     60,250,231, 51, 63, 87,208,243,188, 14,238, 12,112,195,223, 31,
    103,150,216,253,171,252, 81,217, 24, 24,182, 78, 35,177,152,156,
    108,208, 52, 71, 28,238,195, 72,106,  5,134,103, 55,133,222,245,
     11,174, 92, 92, 57, 78,209,147,199,197,230,121,220, 88,199, 28,
    241, 92,199,214,165,239,246,140,123,107,105,126, 24, 99,206,209,
    193, 22,109,108, 70,202,156, 73,134,133,132,157,225,141,212,107,
     64, 36,147,160,165,202,243,198,184,242,197,123, 91,235,103,219,
    201,226,101,188,124,211,140,250,126, 78,  7,252, 63,108,141,165,
    179,123, 35,141,135,106,108,236, 94,  6, 87, 99,220,230,183, 17,
     11,163,113,110,237,130,160, 56, 11, 84, 27,250, 42,243, 50,140,
    179,137,137,250, 60,191,  5,195, 62, 46, 12,163, 56,152,245,250,
    254, 16,250, 54,234, 74,240,186,203,201, 79,181,111,128,254, 33,
    109,174,220,246,195, 98,195,179, 49, 93,134,218,152, 86, 69,136,
    108,225,236,194, 76, 73, 33,174,109, 46,223,155,236,190,151, 15,
     31, 23, 22, 91, 70,111,202,249,222, 71,151,229,241,198, 25,112,
    204, 84,223,180,189,135,225,135,105,251, 95,191,216,253,155,218,
     61,144,199, 96,118,124, 56,125,195,177,146,193, 43,  3, 68,113,
     28,164,213,180, 21, 45,  3,234,188,254, 71,  7, 31,174,113,149,
    203,232,252, 59,204,242,111, 14, 28,248,166, 49,136,171,169,250,
     67,203,118,163,106,237,109,143,248,245,181,113,219, 19,101,191,
    106, 98,155, 20, 99,225,218,199, 56,150,152, 35,169,163,111,101,
    223,143,143, 28,252,104,199, 41,175,252,188, 30, 71, 55, 47, 15,
    196,179,207,139, 29,167,211,211,242,134,157,167,178,123,127,248,
    155,181,176,113,237,141,146,237,135,178,176,238,169,222, 48,199,
    150,186,154, 59,137,206,160,160,181,  7,165,212, 97, 60, 62, 54,
     51,172,220,175,147,135,204,248,150,120,199, 38, 58,227, 31,231,
    241,125,139,105,108,159,136,236,190, 43, 96,193, 88, 97,151,  4,
    252, 35, 62, 70,150, 22, 15,208, 47,157,142, 83, 25,198,115,246,
    219,244,124,156, 49,151, 12,241, 71,164, 76, 87,242,167,197,123,
     23,140,252, 68,252, 60,102, 51, 99,197,217, 25,246,132, 50,205,
    188, 14,100, 47,123, 67,168,  5, 67,217, 81, 66,  0,213,125, 62,
     88,224,242, 43, 45,169,249,159, 19, 63, 55,192,190, 56,226,218,
     39,241,253, 97,246, 94,199,237, 61,173,180,123, 57,133,198,109,
    189,158, 48, 24,249, 51,153,112,224, 17,146,143,112,109,137, 38,
    237,  0,253, 87,203,229,199, 28,115,152,194,110, 31,164,241, 39,
     46, 78, 24,203,151, 26,202,126,159,155,173,188, 82,244,208,183,
    131,204,164, 76,141,122,  5,124, 36, 89,185, 90,166,151,223,145,
     83, 96, 24,224, 65,204, 65, 89,214,184,231,120,220, 55,100,246,
    174, 19,101,227,160,131, 23, 19,113, 14,116,112,225, 28, 36,115,
     50,225, 99,125, 91, 25,112, 21,107,139, 75,198, 96, 13, 42,  8,
    209,122,114,228,199, 44,162,102, 61, 62,191,140,188, 24,113,114,
    241,241,229,142, 57,122,250, 68,127,211, 19,233, 23,244,154,184,
    184, 86,192,236,190, 63,  3,180,224,197,226,217,134,120,140, 98,
    200,  6, 87,204, 90,102,116, 36, 81,207, 21, 52, 12,120, 36,248,
    250,154,103, 39, 44,101,140,196,125,223,202,221, 60,111, 22,120,
    249, 35, 60,235,211,111,172,207,190,191, 89,252, 39,213,232, 96,
    137,184, 72, 35,195,194,214,199, 20, 76, 12, 99, 24, 40, 26,208,
     40,  0,250, 47, 36,220,205,203,233,227,142, 24, 99, 24,227,237,
     14, 15,192,246,133,240,237,144,209,130,194,201,142,197, 71, 60,
    110,139, 22,242, 64,107, 34, 99,152, 78,236, 22,230,108,103,136,
     84,140,218, 90,171,211,183, 29,227,245,168,251, 63, 31,191,239,
    124,255,  0,217,252,153,142, 79,104,156,166, 38, 42,103,236,136,
    152,246,244,184,143,120,246,183, 52,118, 91,104, 63,  7,139,142,
     73, 48,219,201,176,152,248,153,154,105, 37,200,252, 70,235, 47,
     27,134, 99, 76,142,169,214,226,130,244, 23,251, 70, 55, 31,140,
    127, 43,112,255,  0, 77,206,112,202, 38, 98,230, 51,143,121,159,
     92,170,189,103,215,233, 55, 38,237, 55,227,165,237,196, 66,108,
     22, 50,124, 46, 31, 19, 20,176,  6,181,205,143, 57,143, 41,150,
    162, 50, 14, 80,227, 99, 35,116, 60, 36,210,187,134,177,197,233,
     62,179,254, 87,191,244, 57,113,207, 47, 46, 47, 25,152,137,138,
    247,175,106,191,106,244,255,  0,170, 63, 15,183,212,109,172, 52,
     59, 87,  2,252, 22, 41,217, 99,121,  4,157,212,114,104,107,164,
    141,115,126,203,205,134,115,132,220,127,159,193,244, 57,188, 88,
    230,195, 73,253, 34,127, 88,152, 94,197,194,225,182, 86,  1,184,
     56, 56,152,210, 72, 59,168,227,212,215, 72,218,214,253,150,114,
    101, 57,229,115,254,127, 19,135,197,233,195, 76,127, 72,143,210,
     34, 28,177,177, 36, 27,120,237, 22,191, 15,148,237, 79,139,212,
    230,221,252, 22,227, 46,154,231,189, 52,167, 90,217,117,237,141,
     53,251,171,249,219,207,251, 38,125,221,158,159,189,127,150,154,
    254,191,201,195,103, 99, 49,172,192,197,  3, 29, 11,113, 88,102,
    136,161,198,187,104, 79, 35,139, 11,128,121,221,184,101,140,150,
    130,120, 73, 25,128, 22, 11,172,249, 56,221,253, 39,233, 81,250,
    253, 94, 24,248,127, 36, 97, 17, 30,241,233, 19,182, 83,233,245,
    244,159, 72,244,251, 62,175, 65,217,173,139,138,217,155, 45,208,
     99,102,100,211,186, 87, 57,210, 48,147,152, 10, 53,164,212,107,
    149,173,175,173, 87,159,154, 99, 60,174, 61,159, 71,194,195, 46,
     30, 61,115,155,155,255,  0,180,127, 37, 13,153,143,126,221,102,
     62,102,198,200, 48,248,121, 97,135, 36,165,207,126,241,209,184,
    151,  2,208, 27, 77,216, 20,  4,214,189, 22,124,177,199,172,125,
    127,165,255,  0,119, 72,199, 60,185,227,147, 42,168,137,136,251,
    238, 99,223,211,211,219,239,121,252, 39,101,182,219,153,136,147,
     25, 60, 47,196,156, 43, 89, 28,167, 19, 36,149,157,143, 15,100,
    148,112,163, 27,152, 14, 22,216,122,175, 68,242,225,233, 24,199,
    165,253,222,222,213,247,188, 88,120, 60,243,115,156,250,215,164,
    220,207,172, 77,196,250,251, 69,253, 35,216,252,119,103,118,203,
    182, 70, 27,  7, 22, 38, 35, 88,230,248,177,241, 50, 66,215,205,
     33, 14, 47,171,  5, 92,  3,139,248, 77,  1,204, 43,162,156,121,
    112,140,231, 41,143,178,189,167,210, 23,203,226,114, 79, 22, 56,
     99, 63, 73,191, 89,143, 89,250,250,122,207,173,250,125,109,164,
    236,172,111,198,224, 37,138, 12, 38, 30, 76, 51, 97,107,241,177,
     98, 30, 37,145,140,  3, 51, 28,204,128, 57,167,136, 10,184,210,
    181,165, 86,118, 99, 83, 23,239,126,138,253,151, 45,240,152,136,
    137,138,245,137,155,152,143,120,170,245,137,246,245,159,191,220,
    204, 54,202,218, 46,236,204, 27, 11, 21, 30,  5,208,225,166,194,
    229,126,241,206,223,178, 41,152,247,151,180,182,206, 45,110,149,
    112, 36,154,144,182,121,113,236,156,226,253,111,249,193,143,135,
    201, 60, 17,195,148, 69, 68,227,249,196, 76, 76,220, 87,188,196,
    125,254,174,135,104,176,248,217,240,130, 45,151,133,193,103,147,
     27,  6, 46,119, 77, 51,162,204,232,164,141,195,149,142,169, 34,
     48,218,218,150,213, 71, 22,113, 19,121, 79,210, 99,248,223,247,
     95,149,226,231,150, 53,197, 17,235, 49, 51,115, 94,211, 19,246,
     79,217, 78,116, 24, 45,163,140,219, 91, 83,123,131,195, 97,240,
    242,109, 76, 46, 41,248,130,247, 25, 29,186,134,  7,101, 96,200,
      3,155,153,133,185,179,117,120,162,233, 57, 99,142, 56,250,250,
    212,199,241,153,121,177,241,121,115,229,206,241,136,141,177,155,
    250,250, 70, 62,222,158,215,233,119,246,250, 59, 93,158,195,227,
    112, 39, 21,133,157,152,115,134,118, 38,121,225,149,146,146,247,
    111,101,124,132, 57,165,160, 10,103,165, 67,141,105,209,112,228,
    156,114,169,143,122,143,229, 20,245,120,252, 60,156,119,142, 85,
     87, 51, 30,191,108,204,251, 87,223,246,186,185,206,154,126,171,
    157, 59, 78,  6, 70, 79, 68, 77,  8,231,242,160,188,238,242,160,
    186,248,230,254,210,  8, 31,224,166, 96, 61,146,150,255,  0,178,
    157, 88,107, 36,175,117, 40, 16,114, 82, 70,  0,114, 82, 80, 15,
      5,128,193,127,114,232, 24, 36,232, 91, 66,144,201, 30,241,131,
    153,173, 68,172,182, 25,121, 44,166,149,101,156, 48,175,  3,170,
     84,106,219, 45,241, 61,138,117, 85,130,143, 81, 74,177,  7, 56,
     30, 90,164, 52,192,214,201,165,143,130,170, 16,199, 35,117,186,
     81, 99,165, 71, 50,157, 85,177, 70, 44,198,156, 43, 53,110,197,
    207,133,203,117,207, 85,108, 86, 83,226,162, 97, 86, 18,177,105,
     70,249, 80, 81, 99, 14,154,161, 96,203, 66,164,177,131, 77, 90,
    168,177,210,162,200, 35,193, 13,178,150,163, 13, 46, 80, 88,123,
     95,222,161, 65, 99,243, 93, 72,129,239,170,  6,102, 65,121, 67,
    238,140, 67, 25,162, 52,172,180, 69, 37, 10, 10, 33,221, 22,  9,
    197,213,  5,  7, 15,100, 72,179,248, 57,  1, 32,128,158,139, 69,
    146, 59,237, 65, 89,188, 17, 73,152,172, 17,  0,240,160,148, 29,
     18,202, 16,  7,170,212,174,136,196,200, 21, 83, 84, 99, 82, 89,
    114, 67, 84, 85,178,201,134,165,194, 58,236, 70, 71, 15, 50, 46,
    192, 92, 89,206,138, 85, 70,171, 19, 75, 18,248,185,  1, 23,132,
     41, 96,132, 77, 44, 32,148, 43, 89,101, 73, 24, 39,213, 29, 98,
     73,124, 46, 29,229, 66,133,146,195,163,115,169,204,137,163,154,
     73,213, 19, 34, 44,  4, 35, 44, 12, 14,140,250,122,169, 45, 36,
    107,100,249, 74, 52, 35, 59, 69,  3,144,164, 18, 17,206,177,147,
      6,178, 86, 30,242,212, 81,153,106,142,116,  9, 35,234,169,  4,
     61,168,166,121, 98, 69,145,144,214,223,162,168, 80, 30,210,124,
    200,208,105,234,138, 74,148,160,121,148,208,149, 61, 18,147, 70,
     69, 49, 22, 42,102, 25, 48,212, 52,170,199, 58, 64,227, 94, 85,
    172,163, 42, 74,168, 77, 23, 39, 50,166, 76, 33, 20,209,107,154,
     62,180, 89, 40, 49,143, 52, 83, 72,213, 42,105, 69, 52,106,157,
     40, 21, 82,148,219,119, 85, 83, 66, 88,164,103,150, 47,149, 33,
    120,100, 75,227, 40,233,106, 22, 82,209,231, 35,209, 99, 40, 76,
    113, 40,231,168,238,142,115,130,174, 81,206,112, 75,214,185,150,
    219,157, 32, 39,204,170,220,231,  3, 34, 55,182,170,173, 26,172,
     73, 71,241,234,150,106, 35, 32, 60,250, 45, 53, 13, 75,  7,  3,
     91, 69, 80,106,118, 28, 84, 84,234,138,213, 39,142,133, 77,154,
    147, 35, 35, 34,189, 82,205, 89,164,109,150, 20,204,240,150,158,
    181,102, 11,109,147,194,172,202,173,194,120, 77, 97, 11,109,198,
    120,242, 94,101, 86,158,180,177, 85,179,148,240,168,196,150,227,
    169,123,181,167, 90,196, 87,229, 75, 87, 88, 11, 77, 18,220,167,
    140,131,  3,121,178,208,165,163,172, 38, 23, 27,240,200, 21, 91,
    158,172,114,225,154, 95,228,244, 91,110,115,198, 92,152, 65,229,
    253, 21, 91,159, 89, 78,195,218,163,236,170,209,169,177, 48,131,
     76,191,170,149,117,181,199, 21,232,166,211,169,209,197,116,182,
     83, 92, 44, 32, 40,115,163,  4,126,123,149,140, 58,  8,168, 63,
    159,244, 83, 99, 94, 28, 56,106,162,134,232, 56, 79,121,104,221,
     19,200,215, 50,233,178, 41,208,143, 74,133,118,154,108,192, 86,
    182, 87,105,116,163,252,215, 87,108,109,194, 56, 70, 71,138,235,
     18, 59,152, 82,234,  2, 47,238,186,196,141,193,193,162,250,171,
     14,138, 67, 69,123, 37, 98, 79,154,202,182, 22, 49, 18, 87,208,
     41,217, 70,191, 20,226,202,230,161,244, 89,176, 70, 34, 87, 23,
      3,213, 78,193, 69,213,247, 81, 50, 22, 75,192,174,101, 54, 31,
      3,215,163, 12,210,219,188,225,230, 93,226, 68,207,101,210,194,
     30,236,134,185,151, 57,150, 80, 29, 61,124,161,115,217,180, 75,
    207, 93,125,148, 78,101, 21, 12,220,116, 54, 89, 37, 55, 69, 39,
     64,186, 97,201,139, 13,127, 37,215, 75, 24,103,181,209, 44,251,
    225,230,178,216,  0,252, 81, 75, 25, 49, 19, 60,172,181, 83, 33,
    125,209,180, 56,165,117,111,112,166,217, 70,239, 43,173,130,235,
     18,154, 83,  5, 69,122, 46,176, 51, 73, 48,  5,109,164,147, 37,
    125,150, 88,  0,239,170,153,204,  1,144,146,167,101, 27, 83,145,
     45, 37, 27, 21,150,160, 18,166,197, 84,133,150,144, 61,196,174,
    115,154,161,249,179,122,252,244,110,157,122,255,  0, 21,245, 31,
    210, 15,205, 24, 45,167, 93,107, 85,204,163,235, 93,121, 86, 11,
    103, 53, 91,149,105, 71, 49,206,103,114,163,234,177, 99,222,198,
     77,184, 61,220,165, 77, 44,120,238,202,218,126, 82,162,141,154,
     98,196, 72,223,232,222,215,126,242,154, 52,217,174, 61,161, 55,
    121,149,253,228, 79, 67, 84,123, 73,158,200,227,214,215,  6, 53,
    154,137, 46,137,212,252, 62, 46,167,186,127,121, 25, 89, 27, 28,
    241,220,102,186,198,157, 86, 22,127,217,  1, 70,208, 27,109, 80,
    180,203,226,134,207,  3,133,236, 78,211,139,241,139, 27,219,  7,
    205,131, 59, 58,120, 68,108,140, 61,219,208, 68, 76,101,198, 90,
    106,211,215, 69,218,121,113,158, 24,195,234,249,216,248,121,199,
    155, 62, 69,197, 79,241,246,136,123,157,203, 23,153,245, 44, 89,
     93,230,254,242,145,105, 64, 94,214, 29, 88,148,171, 40,199,224,
    163, 85,246, 22,246, 16,154,171, 96, 84,169,166,252,169,156,121,
    150,124,192,198, 32,141, 28,180, 77,251, 79, 58, 88,156, 39,217,
     87,202, 40,196,195,229, 83, 78,145, 36,152, 40,108,162,157, 59,
      0,248, 95,170,170, 54, 33,192,  5, 52,233, 25,168,133, 21,146,
    246, 75,210,139,117, 45, 22,106, 90,248,186, 53, 78,165,138,164,
    234,229, 71,202, 99, 29, 37, 61, 19,230, 68,194,196,212,213, 62,
    103, 57,193,  4,144,234, 90,140,211, 33, 31,134,147,229,246, 90,
    223,153, 95, 13, 81,251, 57, 28,170,142,242,159,134,115,  5, 51,
     95,213, 76,195,172,115,108, 91,218,241,100,165, 90,103,126, 90,
      4,166,152, 39,117,120,185, 80, 25,123, 53,102,159,170, 50,132,
    201, 78,185,172,169, 29, 98,110, 39,229,170,153,132,245,158,201,
     88,225,202,218,165, 35, 81,229,140,139,106,148,148,202,  7,122,
    133,100,192,161,157, 77, 11,206,238,142, 74,101, 24, 36, 61,236,
    191,218, 64,209, 47,155, 47,209, 98, 68, 38, 29,108, 18,138, 50,
     57,  1,239, 55,234,166, 97, 34,207,126,234,144, 89,193, 64,109,
     57, 59,212, 88,195,152,124,218, 32,  7,177,132,240, 75,127,  4,
     21, 35, 29,146,250,122, 41,166,130,158,  8, 45,146,210,203,  3,
    163,120, 58,229,  8, 72,247,109,213, 41,150, 18,194,167, 86,236,
     45,221,174,166,120,213, 28,154,179, 75,131, 15,184,178,231,210,
    190,242, 31,134,115, 20,244,186,199, 33, 59,183, 41,213, 91, 46,
    132,119, 86, 22,150, 60,138,105,182, 28,143,213,101, 41, 51,244,
    122, 52, 64,131,102, 57, 72, 32,218, 14, 86,169,  2, 91, 84, 22,
     15,204,130,233,224,130,133, 80, 57,146, 16,140, 22,122,234,212,
     74,168,210,169, 86,155,191,  4,162,193, 66,166,154,154,234,133,
    171,119,225,149, 40,180,203,226,148, 90,242,144,177, 73, 66,130,
    144,  7, 11,185,150,128, 57,153,202,138, 77,239,139, 80, 49,133,
     98, 71,127, 50,  2, 14, 90,154, 87, 85, 86, 14,221, 20,137,195,
    245, 64,188,163,197, 24, 84,145, 93, 23, 12,210, 70,139, 46, 72,
     27, 68, 86,249, 17,184, 29, 29,116, 92,102, 91,227,157,158,161,
     23,120,134,174, 23, 67,229, 27, 38, 88,202, 56, 61, 69, 57,208,
    234,239, 45,149,129,177,213,171, 69, 62, 54,244, 88, 89,100, 22,
    247, 86,186, 10, 50,211,170, 38, 77, 99,133,121,150, 34,132, 67,
    186, 54,202,210, 19,234,164, 74, 51,204,130, 58, 46,161,214, 88,
    171,  5,  7, 86,161,107,221,143, 50,209, 46,  7,153, 19, 64,205,
     95, 68,101,  4,184,104,141,162,101, 97,175,250, 42,177,157,249,
    134,173, 66,138,204,218,242,208, 42, 90, 26, 19,195,162,213, 64,
     75,133,104, 22, 42,150, 75,197,250, 35,105, 96,130,106,203,163,
     39,  6,173, 69,212,185,208,227,125, 44,136,163,153,151,204,136,
    152, 89, 96, 40,149, 22,148, 64, 67,136,183,241, 85, 12,212,113,
    150,151,209,107,148,193,217, 40,  9, 60,126,202,144,  6,180,139,
    158, 15,117, 50,213,185,149,190,106,168, 11, 32,133, 34, 88,162,
    168,  5,158, 42,155,  0,124, 13,238, 41,116,137, 40,199, 77, 81,
    210, 36,151,180,244, 69,124,169,112,137,212, 25,228,173,244, 70,
    117,139,226, 40, 41,149, 17,214, 89,113, 38,170,156,167,141, 55,
    143, 54, 26,170, 68,241,155,191, 52,163,154,165, 61, 96,222,184,
    232,137,212,230, 75, 32, 84, 78, 13,172, 36,199, 84, 70,161,247,
    114,149,208, 73,181,148,170,153,167,211,213, 13, 75, 25,131, 42,
     85, 38,153,157,123,170,100,194,172,165,202,151, 90, 42, 76,194,
    243, 37,184,244,136, 56, 45,180,206,  6,  5, 86,227, 60, 98,202,
    182,209,170,100, 26,230,170, 89, 75, 44,161,163, 57, 82,209, 66,
    221,176,156,169,105,212, 47,194,241,242,211,217, 54, 70,164,226,
     48,197,154,223,221, 86,200,212,151,225,237, 80,218, 42,217,207,
    172,137, 34,233,213,109,185,244,100, 29,211,181,202,239,224,137,
    235, 50, 40, 92, 77, 94,168,213,208,142, 51,163,237,235,162,139,
    114,235, 24,105,207, 78, 26, 41,183, 41,193,163, 39,130,151, 61,
     76,142, 16, 77, 88,239,116,115,163,242,247,120,190,200, 81,219,
    179,151,249,255,  0, 68, 97,240,  6,  3,102,160,234,225,137, 35,
    250, 74,126,242, 68,165,187, 14,230,214,153,115,159, 70,174,240,
    199, 74, 35,211, 69,214, 37, 45,177, 83,175, 18,177,214,193,201,
     80,162, 70,167,190,186,174,145,154,104,216,229,160,230,178,233,
    179, 40,123,198,201, 96,183,102,209,123,202,112,159,178,141,138,
      3,222,122,125,212,217, 72, 37,174,154,250, 44,178,140,207,193,
    234,186,217, 65,207, 91,116, 92,237,148,123, 11, 78,139,190, 25,
     54,140,110,170,246, 97,162, 82,187, 68,164, 51,184, 81,100,169,
    137,243,187,202,218, 46, 19, 32,247,153,219,203, 69, 27,  4, 74,
     73, 62, 30,203, 54,105,240, 62,215,251,174,152,102,138,107, 18,
     28,150,186,244, 71, 33, 69,201,149,204,183, 50,140,231, 98,156,
    124, 89, 57,242, 62,199,197,100, 78,165, 51,190, 87,139,106,182,
    212, 81,147,119,114,150, 51,153,115,201,110, 85, 81, 41,150,134,
     10,139, 89,111,202,193,147,106, 21,210, 36, 83, 28,230, 10,102,
    178,221,147, 48,197,139,154,244, 89,177, 76,178, 75,243, 41,216,
    164, 47,181,157,250, 46, 83,152,166, 72,234,166,202,109,142, 80,
     88,174,192,200, 42,125, 22, 76,180,  4, 30,154, 41,178,144,199,
    212,232,162,202,103, 38,135,209, 24,252,210,194,208,254,111,170,
    251, 15,232, 61,135,230,174,142,175,232,138,236, 51, 49,238, 59,
    249,250, 35,123, 13, 14,174,183,254,125,148,186, 89,172,145,205,
    228,109,  2, 83, 98, 79,142,113,223,205,253,149, 11, 50, 55,195,
    163, 92,208,142,134,136,198,173,146,171,  5,130,246,249,148,182,
    134,201, 92, 53,117, 81,154,140, 76,124,212,253, 81, 61,103,199,
    137,120,254,142, 75,162, 58,205,102,210,157,154,199, 83,226,148,
    158,134,184,246,156,108,166,241,183, 83, 76,233,108,102, 62, 58,
    213,178,253, 22, 39, 76,154,240,248,154,223,122,215,250, 88,173,
    114,212,198, 98, 42,110,219,126,136,106,102,242, 58,243,127,118,
    171, 25, 89, 15, 93, 18,155,106, 33,222, 85, 52,178,137,127,149,
    200,160,231,119, 84, 85,  4,200,  2, 20,128,196,237, 80, 81,138,
     39,232,178,154, 89,193,187,186,185,210,172,151,195, 48,254, 66,
    205, 91,190, 37,187,124,205, 82,157, 44,123,217, 24, 42,114,255,
      0,105, 18, 38, 79, 86, 87, 42, 26,141,178,198,125,210,138, 25,
     25,135,254,148,165, 22,232,155,213,168,108, 89,132,116, 74, 94,
    196,152,156, 60,202,105,123, 22,229,148,235, 26,128,189,205,238,
    215,213, 10, 87,197,183, 66,239,238,161, 75, 19, 48,232,230,161,
     75,222,199, 90, 27,250,165,  8,114,147,103,185,109,  2,163,235,
    194,231, 37, 36,192,233,153,222,255,  0, 20,102,167, 50, 74,243,
     53, 41,203, 47,149,102, 38, 27,133,148, 71, 33, 50,225, 91,187,
    239,  4,167, 94,198,118, 97, 94, 71, 51,169,249,145,189,136, 33,
    196, 15,232,244,245,106,149,236,143,146,127, 43, 79,181,145, 86,
    130, 75,221,146,  3,250,162, 78,100,196, 26,137, 81,149,137,241,
    227,  8,231,109,126,233, 72,158, 63,249, 79,143, 17,  9,244, 74,
    115,212,194, 26,237, 18,145, 72, 26,241,162, 83, 38, 23,188,166,
    173, 83, 73,161,  9, 25, 37,139,178,254,234, 82, 77,100,109, 28,
    143,111,221,102,165,143, 44,161, 79, 89, 96,204, 70,171, 53, 85,
    226, 48, 78,186,251,166,162,217, 57,234,166,138, 20,120,171,243,
     41,162,140, 24,154, 30,233, 88,106, 61,243, 53, 43, 83,170,183,
    145,147,193,100,162,133,251, 51,173,254,201, 66,  1, 79,232,223,
    244, 46, 89, 64,247,239,209,205,111,184, 82,202, 50, 57, 63,243,
    126,137, 27, 39, 81,239, 95, 85,101, 28, 36,105, 20,124,109, 88,
     20,248,176,238,210,202,102, 11, 41,248, 95,  7, 85,114,235, 95,
     97,127, 14,  7,117,193,103, 91,164,114,  7,225,154,116,146,234,
    117, 94,201,240,222, 45,105, 81,214,222,226,228,193,158,141,167,
    178,137,227, 87, 99, 57,137,209,158,242,141, 87, 25,135, 59,188,
    200,177,138, 29, 84,137, 69, 34,213,  2, 64, 72,193,102,160,191,
     23,170,164,141,153, 11, 80, 70, 50,189,234, 32,133,191, 42,150,
     88,104, 66, 53, 17, 75,238, 37, 52, 44,186,154, 16, 10,187,153,
     96,142,103,213, 24, 89,105, 69,135,114,196, 82,145, 43,171, 80,
     23,179,144, 94,106, 37,165,  3,207, 68,181,  8, 56,141, 86,136,
      8, 58, 57,201,105, 89,182,169, 97,116,  7, 70,254,169,106,178,
     95, 18, 42,217,229,101, 46,139,182, 89, 28,254,168,210,243, 30,
    173, 71, 68,207, 31,149, 26,159,145,101,  6, 49,207,102,168,195,
     55,141,114, 57,165,  1,228,213,  2,222, 31,161, 90,233, 26,168,
    135,  4, 19, 55,139, 80,163,227,125,185,145,202,112, 89,115, 80,
    162,232, 78,150, 74,  0, 73,105,166,168,209,102,  5,  1,  2,  2,
     49,  9, 26,135, 42,108, 42,160,234,129,114,180, 83,133, 72,204,
      3,198,174,161,245, 69, 68,  1,239,119, 87, 52,170, 85,  4,150,
     29, 85, 20, 78,236,103,253,155,178,173, 21, 35, 94, 22, 52, 45,
    191, 51,127,188,128,216,  7,178, 13, 81,149, 44,145,229, 62, 84,
    114, 54, 39,145,171,115, 34, 76, 20, 42,153, 74, 36,132, 77, 43,
     59,123,200,154, 91, 75, 66,212, 78,  6,  2,230,119,172,169,203,
     82,205,107, 95,243, 33, 75,171,188,202, 77, 66,242,177, 90,130,
    164,169, 85,  5,228,157, 17,122,151,188, 34,193, 27, 64, 50,248,
    161, 74,205, 85, 42,  1,  5, 20,160, 10,  8,246,211,253, 16, 39,
     95, 48, 71, 52,208,255,  0,138, 20, 46,234,166,106, 12,220, 72,
    229, 56, 27, 10, 35, 60, 27,139,255,  0,102,169,202,148,193, 81,
     92,202, 86,172,221, 17, 84, 67,220,106,141, 73, 57, 60, 18,217,
     76,118,232,150, 76, 46,136,229,170,232,169, 58,161,  8,205, 87,
     27,106,136,156,  4, 88,241,222, 75,114,156, 12,102, 94,185,146,
    220,181, 27, 67,186, 58,203,111, 20,234, 38, 50,186,234,136,212,
     88, 97, 71,186,188, 75, 17, 77, 66, 35,168,115,190,200,229, 56,
     47, 32,119,118,254, 57, 81,  4,203,  8,234,215, 60,248,139,125,
    148,218,233,146, 88,  1,103,120,187,242,171,217,140,251,135,101,
     85,178,102, 12,195,198,215,240, 29, 85,236,141, 90,227,109, 56,
     56,105,249,138,229,110, 92,156, 63,241, 25,150,199, 54, 90, 44,
     78, 28, 63,241, 10, 60,185, 41, 27,127, 71, 41,183, 62, 78,  6,
    168,170, 89, 77, 61, 87, 91,120,186,242,105,138, 58,199,119, 53,
     19,168,131, 40, 42,219, 31, 69,168,104,100, 68,138,139, 44,146,
    154,160,118,238,217,126,170, 21,171,110, 24, 95, 58,244, 98,225,
     48,234,198,126, 85,209, 20,124,110,162,108,170,116,176,146,219,
    186,141,166,168,231,171,184, 50,170,182, 10, 73,143, 76,213, 85,
     98, 51, 16,211,105, 29, 66,182,195, 95, 51,  0, 73,148,178,201,
     41, 26,186,203,140,230,170, 47, 15, 48,207, 80,228,182,211, 99,
     38, 30,101,118, 82,139,158, 31, 83,202,169,134,199, 32,165, 91,
    247, 87,105, 53,147, 19,232,174, 51, 73,130,118,117,253, 85,246,
     20, 51,136, 26,  7, 84, 42,236, 41,207,196, 22,  9, 51,155,174,
     83, 33, 31, 16,250,212, 59,133, 69,134, 50,118, 29, 92,213, 19,
     42,163,132,237,120,230, 87,134, 73,152, 25,156,134, 81,143, 94,
    136,146,129,241, 93,199,182,233, 50, 81, 56,183, 51,117, 66,234,
     21, 49, 44,115,250, 80, 42,  8,196, 59,118, 56,156,129, 49,202,
    206,238, 90,248,165,141,209, 56, 60, 80,186,255,  0,149,109,135,
    114,183,199,236,186,216,205,136, 13,100,121,203,104, 82,199, 38,
     89,106,178,202, 32, 23, 19,117, 54, 80,216,235,221, 76,201, 75,
    123,200, 54, 75, 41,123,231,133, 86, 83, 86, 18, 76,226,175,106,
     76,164,247,188,123,  5, 22,165, 87,131,154,222, 10,109,172, 50,
    158,133, 34, 74,126,108,206,  8,166, 91, 47,190,253,189,100,131,
     32,210,200,219,200,108,147, 39, 35,254,136,118, 11,139, 83, 99,
    244, 74, 80,217, 44,141,238,217, 77, 42,206, 19,181, 41,182,102,
    118, 17,155,186,148,239,177,209,185,217, 51,  9, 28, 61, 20, 82,
    246, 48, 98,100,167, 53,252, 20,210,246, 53,152,134,215,142, 55,
     15,186, 82,205,248,136, 75,233,195,247, 11, 41, 66,140,179, 61,
    156,225,236,165,180,132,223,189,252,253, 17, 90,143,130,151,119,
    234,137,164, 99,188, 28,223,237, 41, 96,193,146, 59,177,223,162,
     39, 79,254,166, 71,142,197, 71,163,221,245,170, 51, 86,150,237,
    121, 71, 61,253,154,137,211, 22,136,182,189, 89, 92,180, 68,244,
     52, 69,181,218,241,207,254,  8,206,150,150,109, 56,115,208,255,
      0,170, 81,210, 97,196,199,229,178,202, 78,153, 43,127,135, 41,
     69,100, 28,236,254,174, 70,133, 52,160, 81,175,254,181,181, 70,
    168,151,247, 93,250, 58,191,197, 41, 84, 19,137,145,150, 46,191,
    171, 82,149, 88,169,248,130,121,218,212,213, 61,100, 62, 86,157,
    114,253,214,106,189, 65,191,  3,145,174,167,162, 82,181,  9,197,
      6,247, 43,233,213, 41,186,173,152,209,229,115, 61,220,148,206,
    179, 25,180,193,182, 95,174,100,163,172,230, 98,240,239,247,244,
     82,158,188,141, 18,176,219,120,148,202,201,  8,135,171,155, 95,
     84,162,242, 34, 76, 43, 94,120, 53,241,183,248,169,165,199, 35,
     60,184, 92, 72,182,103, 16,148,235, 28,140,146, 49,241,217,249,
    135,186, 83,166,196, 18,225,124,217,209,215, 31,153,  6, 34,135,
    195,222,200,106,103,197,142,174,250,230,170, 51, 83,153,137, 52,
    231,141,255,  0,100,164,208,134, 49,221, 82,145, 56, 31, 30, 53,
    190,100,164,117,159, 30,208,109, 50, 58,233, 78,121,241,255,  0,
    202,124,120,156, 61, 46,230,253, 82,153,166, 67,  6, 13, 91,148,
    123, 37, 39,230, 83,204, 38,193,223,224,178,149,217,145,  6, 40,
     43,119, 53, 41,177,201,144, 76, 21,254,141,201, 78,187,  4, 50,
     81,221, 83, 74,216, 38,231,150,232, 88,217, 38,239,253,209, 52,
    115, 49, 66,183,119,232,178, 81, 60,102,252, 67,  9,230, 68,105,
    145,143, 44, 34,217, 81, 31, 50, 70, 72,211, 50, 66,127,234,105,
    142, 87,  0,182,147, 58,159, 28,245,213,173, 41, 72,163,106,194,
     57, 26,207, 85,147,  6, 63, 40,132, 49,187,188,167, 86, 79, 32,
     36,193,248, 41,233, 35,152,137, 48,135,255,  0, 48,149, 19,195,
    147,180,115, 18, 89, 40,239, 81, 69, 47,176, 63,181,  7,189,244,
     69, 47,120,244, 53, 91,113, 15, 26,169, 78,166,199,140,166,153,
    130, 51, 83, 68,249,180,213, 19, 67,102, 36,  3, 73, 27,245,  9,
     73, 61,147,198,121, 93,250,165, 48,193, 43,122,165, 32,200,158,
    211,222, 74, 72,233, 85,148,162,168,166,139, 93, 60, 26,148,181,
    138,143, 42,202, 72,234,217, 56, 75, 27,239,153, 39,  6, 94, 64,
    126,  3, 14,124,181, 92, 58,241,116,142,110, 76,127,121,146, 93,
    152,254,150, 11,148,240,100,244, 71,145,139, 59,240,243, 71,221,
    168,241, 89, 78,187,226, 10,145,170,133, 37,208,180, 47, 32, 93,
    168, 90,217, 32,249,191,178,148, 80,243,183,205, 68, 40, 77,145,
     54, 40,193, 35, 74,148,208,248, 81,148,156, 95, 68,  3,193,230,
     70,166, 80,116, 65, 70, 47,149,  0,210,157,230,162,172, 38,250,
     41,109,148, 92, 69,191,202,177, 86, 34,  7,202,181, 32, 32, 34,
    147,139,162, 41, 89,157,252,213, 96,176, 74,  2,205,249,148,165,
    100,215,188,172, 64,114,119,144, 66,250,234,130,143, 42,  5, 17,
    245, 69, 21, 36,108,239,217, 26,205, 35, 60, 29,100, 91, 59,227,
    249,149, 58,193, 18,102, 26, 42, 92,  4, 76,116, 40, 76, 31, 28,
    205,249,148,162,154, 98, 57,189, 20,162,143, 13,175,168, 68, 88,
     95,  0,167,254,148,110,196,110,159,210,232,173,130, 88,224,235,
    183,251,200, 89,128,244, 64, 36, 20, 74,  2,228, 80,144, 66,128,
     28,138,132,206,224,130,  7,  3,173,149, 57,130, 86, 87,212,120,
    164,195,164, 50, 76,220,154, 53,197,  0,112,157,108, 80, 84,145,
    187, 81,117, 80,216,  8,175, 86,162,139, 52,175,130, 36, 89, 60,
     28,180, 62, 39, 22, 44,100,195, 91, 37,181,212,185,208,197, 31,
    162,150, 37, 72,238,160,  7,188,124,200,154, 45,242, 83, 86,213,
     82,168,173,240,175,121, 82,103,  6,184,177, 21, 30, 43, 92,102,
     13,206, 41, 71, 42, 64, 41, 67,204,166,132,169,106,202,104, 42,
     78,173,178,149, 11, 43, 78,150, 70,147, 43, 41,112,138,103,146,
    250,170,133,  4, 53,221, 17,166, 10,245, 82,193,134,120, 41, 19,
    118,128, 31, 21, 80, 47,116, 81, 36, 79, 98,169,112,  0, 90,132,
    142, 58, 86,200,227, 45,113,188,148,112,152,105, 20,200,169,  4,
     56, 26,250, 36,186,192, 50, 30,139,154,139,125, 58,163, 73, 63,
    100, 41, 50,187,166,136,138, 27, 25, 93, 46, 81, 52,102,228,210,
    143,213, 25, 67,100, 33,161, 98,105,  4,117, 40,137,192,111, 97,
      1, 17,168,  0,233,252, 82,115, 79, 73,209,215, 70,183,244, 72,
    151, 41,227, 16,203, 91,182,142,245, 93,156,231,140,230,174, 86,
    229, 70, 81, 45, 19,130,180, 75, 77,  2, 72,171,255,  0,185,116,
     81, 15,138,157,229, 41,165, 49,128, 26,139,126,234,155, 79, 89,
    249,  0,211, 84,183, 29, 67, 67,213,169,105,213, 66,140,209, 99,
     39,  6,140, 59,155,213,206,246, 87,111, 60,240,228,232,196,247,
     82,252,169,110, 25,241,158, 50,123,159,162,108,243,204, 29,147,
    213, 83, 26, 24,203, 34,126, 99, 48,228,131,202,229,118,202,110,
    100,131,230, 91,102,173, 33,245, 85,105,212,198, 77,145, 85,166,
    155,162,145,212,175, 68,181, 81,236,149,139,109, 52,188,193, 85,
    178,149, 44,204,  3,213, 78,197, 51, 73, 41,167, 30,138, 45,219,
     86, 97, 53, 15,236,255,  0, 69, 59, 34, 97,191,  9,136,100,130,
    185,170,124, 23, 88, 67,165,135,203, 47,167,239, 47, 68, 56, 74,
     98,  4,134,204,205,149,100,170, 20,231, 24,227,229, 91,106, 33,
    243,146,222,106,122, 46, 83,153, 76,191, 22,234,228, 54,  9, 25,
    182,134,103, 36, 80, 57,206, 11,166,204,162, 31, 61, 82,194,254,
     40,116,117, 18,195, 99,196, 18, 44,182,  1,239, 85,218, 64,249,
    174,170,192, 63, 16,105, 83,153, 68,202, 89,206, 34,166,171,108,
    162,231,144, 72,170,202, 34, 55,146,254,111,241, 77,138,111,138,
     77,222,174, 77,138,105,223,  7, 50,170,247,200,166,105,166,204,
    202,102, 89, 57,142,108,237,186,157,148, 82, 88, 17, 99,204,150,
    149,147,126,101,214,  6,124, 68,188,124,202,102, 85, 77,120,  9,
     69, 57,148, 89, 48,221, 19,134,126, 59,143,  2,170,210,115,219,
    208, 41,177,146, 70,222,154,149, 80,167,230,108,135,172,127,224,
    190,243,246,182,170,176,119,127,188,128,129,111,153,  1,112,104,
    169,166,  6,229,242,162,150,  3,135,149, 20, 62, 35,195,254, 84,
     44,194,120, 50,255,  0,149, 23, 18,117, 78,236,  5, 43,177,214,
    130,223,101, 37, 44, 72,122,253,209, 86, 60,195,255,  0, 98, 55,
    177,102,119,103,174,127,237,255,  0,217,101, 59,239,137,163, 18,
    230,252,255,  0,147,254,202,105,113,158, 33, 19, 51,121, 71,102,
    103,231,255,  0,178,202, 94,248,153,190,240,114,154, 72,179,187,
    204,148,233,  0,222, 57, 41,107,222,184,233,149, 17, 73,190, 58,
     57,168,218, 77,229, 46, 51, 34,168,214, 98,223,255,  0,136,229,
     39, 89,172,218, 50,183,188,137,235,105,102,212,243, 57,168,206,
    179, 89,141,105,190,118,253,150, 39, 83,  6, 40,244,202,134,170,
    248,183,249, 91,247, 67, 84, 56,176,  5,245, 82,235,166, 64,110,
     38, 61,115, 34,116,200, 91,200, 93,126, 20,102,153, 37, 97,212,
    106,128, 36,108, 15,239, 58,168, 89,123,151,142,251,178,162,236,
     25, 95,210,254,232,221,129,158, 65,229, 67,229, 79,139,144,127,
    178, 26, 98,116,120,231,245,115,145,157,102,252,121,173,145, 61,
    103, 12,120,167,237, 17,203,172, 27,220, 36,188, 69,183,241, 41,
     74,142,204,127,117, 79,143, 12,111,153, 69, 47,124,137,126, 10,
     39,241,  7, 83,244, 83, 75,238, 47,225,171,113, 45,104,170, 21,
    221,136,126, 26,125, 88,229, 74,236,196, 57, 39, 58,181, 75,126,
     80,187,124, 46, 26,224,165, 95, 42,111,228,109,203, 92,133, 98,
     99, 49,178, 71,199,196, 17, 61,120,156,205,168,226, 42,235,170,
    103, 65,195,106, 70, 89,116, 79, 73,236,197,194, 99,254,146,136,
    229,169,163, 17, 25, 20,204,211,238,169,147,134, 67,147,114,230,
      1,154,254,168, 86, 73,185, 60,140,251, 81, 73,120,151,145,236,
    146,155,202,122,  5, 45,217, 36,107,243,114,209,101, 40,123,194,
    209,205,145, 41, 58,154,204, 83,192,239,123,165, 51, 62, 51, 25,
    136,111, 87, 57, 28,186,206,102, 32,116,114, 57,117,159, 30, 37,
     19,214, 96,196,183,204,148,205, 79,143, 20, 60,201, 72,212,230,
     98,107,173,210,147, 48, 34, 97,147, 71,182,190,  5, 77,  4,201,
     22, 93, 99,175,178,202, 92,102, 75,216,  5,254,202,117, 95, 98,
    178, 13,114,169,235, 86,192,124, 77,118,142,162,202, 86,192,220,
    158,142,170,146,198, 26,224,178,147,102,101, 62,100,164,216,227,
     46,  5,111,238,176,200,223,198,155,108, 81,194,122, 61,103, 89,
     66, 56,145,154,134,193, 74,104,208,247, 30, 71, 84,126,101, 32,
    115,248,106,176, 22, 97,230,191,229, 82, 25, 28,168,202,104, 24,
    134,104, 85, 38,132, 89, 12,154,169,160,137, 48, 67,185,150,139,
    151, 70, 46,177,204,197, 46,  9,195,149,215, 92,231,141,218, 57,
    153,223, 20,140,231,141,222,235,148,198, 78,241,204, 73,170, 83,
    109, 96, 40,165, 90,156,166,139, 85, 79, 73, 40,164,176,214, 81,
    221,250,163,104, 66,119,232,120,209, 93,105,190,102,138,173, 52,
    189,230, 68,178,151,190,104,185, 85,101,  8, 78, 63,171, 82, 35,
    228,111, 93, 80,  8, 62, 46,178, 10,224, 58, 58,138, 75,  7, 16,
    249,209, 86,174, 30,174,201,251,168,171, 93,254,136, 43, 55,229,
     65,109,202,124,168, 25,101,137, 85,252,200,164,226, 70,174,195,
    187,116, 98,178,213,  4,202, 77,144, 38, 70,116, 44,250,161,177,
     15,129,135, 68, 94,204,242, 97,213,175, 98, 31,  7,243,101, 78,
    189,128,100, 97,189,213, 37,180,178,246, 10, 83, 71,198,227, 24,
    230, 71, 41, 53,146, 87,186,140,145,103,  6,200,144, 62, 54, 85,
     21, 96,221,248, 33,106,184,245, 66,215,148, 58,249, 85, 89,107,
    201, 39, 68, 45, 68, 30,173,170, 81,100,200,214,158,237, 10,150,
    192, 69, 66, 40,167,139,212, 34,132,249, 36,112,162,162,144, 16,
     25,196,133,  1,144,199, 37,209, 54,175,134,175, 43,149, 22, 31,
    134,120,238,165,170,203,124,  3,202,150,219, 80,195,148,176,198,
     54,138,109,134,100, 85,105, 93,198,138,101, 39, 70,106, 46,164,
    180, 48,130,133,151, 36, 69, 84, 51, 98, 95,  3,252,170,151, 18,
     13,205, 47,162, 38, 96, 89,235,108,213, 33, 28,232, 57,223,223,
    143,244, 66,141,223,208, 81,237,178, 38,130,234,235,155,133,  8,
    129,  7,162,169, 31, 42, 20, 86,124,200, 37,148,180, 76,204,129,
    241,101,242,221,  1,155, 27, 41, 96, 69, 74, 36,101,141,162, 37,
    150, 72,129, 42,169,209,150,120, 82,133,100, 59,180,136,106,226,
    168, 85, 46, 83,131, 80,125,185, 87, 56,115,213, 13, 29,101,213,
    122,171, 41, 96,167, 18,196,147, 35, 65, 69,145,145,245,249, 80,
    147, 24,209,209, 16,211, 27,126, 85, 44, 53,128,125, 84,185,156,
     27, 85,133, 15,116,140,160, 24,209, 52, 91,226,109, 81, 67,201,
    107, 35,148,194, 49,135,170, 35,172,162,206, 58,143,178, 90, 39,
    140,246, 60,228,167,241, 83,110, 83,198, 49, 65,254,203, 45,202,
    120,212, 21,185, 82,245,178,178,149,147, 34,227, 42,164,202,165,
     58,132,181, 82, 39,  5,101, 90,141, 81,133,236,239,101,  8,137,
    193,183, 13, 48,126,156,106,158, 94, 76, 27,226, 45,233,149,174,
     90,242,206, 13, 76, 57, 94,150,231,171, 80,  1,239,169, 85,105,
    212, 84,227,224,106,187, 53, 21, 92, 31,103, 44,181,106,104,113,
    173,219,117,182,157, 70, 94, 43,197,162,171, 78,173,120,105, 72,
     22,209, 78,204,213,166, 57,242,223,252,170,182, 78,167,137,154,
     99,175,249, 86,236,205, 64,199,  7, 10,164,145, 12,175,174,123,
    174, 50,237, 76,239,179,236,170, 28,166, 12,139, 16, 99,127,  6,
    106,174,145,155, 53,116,176,216,255,  0, 62,171,172,102,231, 56,
     55, 71,139,181,220,218, 46,177,154, 53, 84,146,199, 39,150,158,
    138,101, 76,210, 26,114,233,234,160,100,121,234, 53,244, 80,212,
    138, 90,158, 55, 93, 85,148, 86, 34,155,203, 45,217,148, 73,125,
    214,108,154, 24,113,243, 46,145, 34,204,230,171, 59,129, 25,106,
    207,  5,120,103,176,204, 94,237, 51,127,121, 84,148,162,212, 85,
     22,250, 45, 40, 49,184,131,109, 22, 78,104,152, 62,161, 78,197,
     28,199,213,148, 42,118, 40, 44,203,209,101,136, 90,182,218,207,
     35, 92, 23, 75, 25,222,124, 93,117,118,194, 36,115,199,170, 94,
     33, 59,186,220,165,226,179,176,238,161, 75, 76,183,179, 17,148,
    119,150,219, 21,241,117, 61,229, 86, 25,188, 47,209, 37, 47,207,
    210,237,  6, 62, 94, 56,168,223,  0,213,250, 39,234,250, 85, 62,
     35,  4,254,231,234,212, 92, 96, 76,159, 11, 78, 30,  4, 84,108,
     70,237,159, 42, 59,124,193,200,  6,142, 71, 72, 93,252,174, 69,
    218,196,174, 23,225, 70,209,236,150, 51,121, 53, 69, 80,247,194,
     75,  6,180, 15, 90, 41, 85, 31, 90,  5, 41, 14,102,155, 32, 44,
    185,123,168, 33,  5,221,236,158,200,160,241,233,150,190,168,184,
     86,119,199,202,169,112,172,237, 55, 45,191,213, 77, 45, 51,211,
     79,250,146,151,105,188, 39,188,239,237, 35,165,175,124,229,148,
    213,239,136,213, 41, 73,191,111, 86,187,232,148,212,223,199,210,
    223,188,166,139, 22,251,194, 85,148,168,149,239, 95,230,105,253,
    229, 52,171, 65, 35,235,202,170,150,159, 18,246,247,169,236,166,
    138, 27, 54,147,250,185, 77, 29,103,140,118,111, 55,186,196,234,
     63,140, 40,170,200, 15,148,107,154, 53, 45,162, 31,136,120, 54,
    251, 42,162,149,241,142,239,102,254,210, 81,170,198, 56,142, 75,
     20,163, 81,252,105, 55, 50,223,209, 40,213, 99, 20, 78,178,254,
    168,106, 95,196,113,223,136, 41, 53, 83,241, 13,175,167,165, 21,
     43,173, 35,197,  6, 62,206,112, 90,158,179, 14,210, 57,249,170,
    136,235, 61,155, 78, 30,254, 90,172,103, 75, 64,199, 66,123,205,
     82,158,188,134, 49, 17, 31, 95,100, 68,198, 67,223, 52,216, 58,
    131,213, 82, 86, 11, 70,146, 85, 40, 24, 46, 81, 74, 83,220, 77,
    139, 82,154, 18, 67, 82,157, 32,169, 55, 70,229,183, 83, 75,101,
    145,173, 62,100,133,150,105,255,  0,117, 66,195,156, 52, 64,108,
    196,204, 63,221, 19, 38, 51, 29, 35, 81, 19, 13, 81,109, 63, 20,
    103, 75, 67, 54,159,134,158,168,231,210,116,120,184, 93,222,186,
     26,157,191,174,142,106,148, 82,133, 77,195,171,238,218, 32,161,
    136,243,183,248,169, 20,204, 67, 11,208, 95,196,183,121,151,121,
    111,205, 84, 84, 25,241,109,103, 44,181, 68,204, 31,  6, 44,139,
    163,148,241,157, 30, 45,185,234,127,102,124, 81,206,120, 90,227,
    196,180,250,250,170,167, 62,179,198, 37,209,233, 35,169,224,166,
    145, 60,102, 54, 88,229,230,111,213, 41, 26,234, 41, 33, 97, 31,
    178,114,197, 88, 14,102, 51, 38,170, 91, 96,  0,180,114,172,165,
     90,195, 73,186,106, 90, 81,189, 90,154,150, 54, 30,157, 18,147,
    102,  0, 16,216,123,166,149, 52,203, 67,134, 31, 85, 52,155, 86,
    229,225,102,170,180,171,226,243, 38,173,181,137,  1,187,148,234,
     15,122,223, 42,106, 12,101, 61,229, 26,150, 61, 52,114,141, 89,
    102, 70,247,251, 41,136, 12, 47,118,134, 58,170,152, 74,  8,218,
    253, 51,  7,120,116, 92, 39,  0,153, 48,199,174, 95,162,205, 93,
    163,153,145,248,111,  6,185, 68,224,235, 25,151,187,  2,217, 91,
     85,207, 87, 82,204, 77,235,170, 81,176, 14, 30, 83,166,139, 41,
    123, 22,248,  8,244, 42,105,123, 22,246, 60,122,168, 85,151,158,
     86,255,  0, 73,162, 41,121,193,229,117, 16, 88,119,230, 84, 80,
    243,211,188,137,165,135,131,234,137,164, 36,139, 12,200, 40, 73,
    151,213, 40, 89, 61, 66,154, 20,  8, 46,245, 82,232,187, 32, 43,
    142,234,  3, 99,214, 36, 85,170,  2,101, 16, 67, 66,133,160, 96,
     66,214,127,252,136, 90,172,181, 33, 44,  7, 84, 81,  6,  0, 77,
    157, 68, 85,149, 36, 35,170, 54,217,164,141, 23, 18, 89,104,142,
    232,233, 98, 99,186,160, 99, 30, 17, 52, 58,132, 77,  7, 49, 70,
      8, 57,190,116, 12, 97,111,230, 65,116,234,169, 50,210,199,254,
    206,138, 88,140, 12,203,118,181, 99,153, 47,  0,232,182, 29, 49,
    101,145,140,234,218, 42, 93,148,248,188,183, 69,216,  1,120,182,
    138,224,  9,245, 69, 38, 96,176, 22,116,164,204, 24,201,138, 82,
    105,118, 42, 74, 86, 92,214, 85, 66,183, 71, 68,165, 32, 14,228,
     58, 37,136, 88, 58,169,177, 56,134,136, 45,165,200,145,231,181,
    209,128,203,213,  4,207,208,177,  2,101,140, 27,240,253, 21,  4,
    101, 90,208,144, 58,170, 65, 82, 55, 45,208, 77,235,128,230, 69,
     80, 76,128,169, 93, 44, 72,125,191,121, 21,169,140,119,138,199,
     57,193,166, 48,213, 40,105,137,138, 81, 50, 55, 68,117,254, 40,
     68,168, 53,168,169,148,123,  0, 98, 57,179, 61,206, 71, 68,160,
     58,160, 23,198, 41,102,162, 65,186,175,254,213, 73, 88, 14,110,
    168,161,216,143, 84, 74,139, 76,119, 84,162, 94,  4,151,118, 84,
     82,152, 42,104, 16, 52, 68,165,204,198,198, 66, 32, 64,184,104,
    140,163, 34,125, 77,212, 57,181, 26, 82,205, 65, 25, 31,138,  9,
    187,249, 84,128, 49, 93, 99,  6,200,197, 57, 84,164, 15,134,154,
     57, 90,136,123,105,234,136,212, 25,143,209, 28,250, 70,201,  6,
    129, 30,124,248, 78,103,138, 56,206,  2,171, 77,138, 10, 44,167,
    251,168, 72, 81,138,160,246, 68,169,241, 52,242,185,180,245,183,
    251,162, 53, 54, 38,229, 22,115,189,186, 37,163, 60, 91,176,207,
    174,156,  7,199, 47,242, 18,222, 44,240,109,129,228, 14,101,214,
    220,117, 62, 57, 71,254, 34, 89,170,216,234,186,191,116,179, 86,
    144,239, 23, 91,243, 45, 58,199, 87,247, 29,194,134,166,198,115,
    250, 20,180,234,211, 30,151, 88,138, 40,184,  2,166,219, 69,111,
    142,123, 40,183, 74, 27, 49, 46,170,190,199, 62,179,  6, 33,175,
    117,213, 91, 53, 92,143, 97, 28, 42,209, 69, 83,168,224, 62, 42,
     45,180,131, 17,150,199,140,173,217,154,180, 97,241, 36,158,111,
    162,173,141, 90, 12,202,246,115,161,137,219,146,233,179, 41,146,
     73,198,126, 12,169,178,104,  2, 92,197, 54,101, 35,228, 89,178,
    104,162,234,169,217, 52, 38, 74, 66,168,146,136,146,119,103,230,
     87, 18, 80, 78, 34,167,197,117,137, 40, 91,254, 63,  5,210, 36,
    163,115, 19,234,170,202, 29,178,248, 37,148, 12,132,234,220,222,
    170,108,164,107,122,  6,168,144,193, 86,174, 76,165,230, 30,235,
     54,212,164,222,  5,113,152,101, 65, 98,235, 25,140,111,136,214,
    214,247, 93,109, 36,150, 81, 77,133,190,232,  0, 55, 42,137,  5,
     95,  5,177, 33,177, 53,245,190, 90, 46,145, 40,107,  1,172, 22,
    117, 74,184,145,249,241,248, 91,219,245, 95,165,126,170, 57,217,
    165,195, 56, 30, 90,173,116,142, 98,223, 17, 11, 29,251, 49, 85,
     28, 52,204,164,176,212,245,114, 46, 36, 85, 71, 91, 94,100, 94,
    201, 90,121,145, 86, 44,200,187, 94,240, 13,126,213, 68,209,194,
    119,141, 36, 64, 67, 18,122,253,232,128,190, 37,154,229, 67,172,
    123,214,116,114, 20, 46, 29, 17, 95, 50, 10, 82,153, 84,183, 96,
    186, 62, 15,  4,117,130,119,110,167, 53, 81,117,138,  6, 77,214,
     79,162, 10,123,159,214, 54,163,160,  4,180,117, 11, 29, 79, 68,
    104,204,141,243, 83,209, 98,146,164,232,141,176,221, 21,105,153,
    227, 71, 57, 66,236, 91,233, 66, 42,195,190, 37, 27,106,204, 60,
    170, 84,155,194, 52, 82,165,111,228,243, 57,  5, 55, 22,122,160,
    104,153,142,238,160,174,  3,162, 11,160,232,148,208,101, 39,188,
    178,132,164,154,102, 74,  2,252,224,248,165,  1,253,162,  8, 28,
    238,183, 82,  4,188,215,151,245, 69, 38,241,  1,  9,222, 52, 68,
    158,204,108,192,115, 42, 68,224,124,123, 69,195,188,165, 61,109,
    113,109, 55, 35, 58, 79,143,104, 49,214,235,226,170, 19,214,120,
    197, 52,106,244,102,163,100,177,201, 99,198,148,148, 48,198,127,
    163,115,125,148,170, 36,151,224,254,101,141,178,223,134, 35, 68,
     93,131,115, 47,151,245, 66,192, 98,119, 86,163, 44,  5,180,238,
    160,172,206, 26,102, 90,209,140, 65,103,122,200, 26,204, 73,212,
     57, 25, 71,179, 23, 35, 46, 28,137,211, 99,227,198, 61,222,136,
    206,182,145,138, 47, 23,107, 71,170,202, 78,161, 18,199,230,161,
    250,255,  0,130,154, 77,100,174, 18, 59,200,162,101, 47,182, 87,
     41, 93, 33,154,102,255,  0, 89,252, 81, 90, 99,145,195, 27, 55,
    117,206, 63,188,170,209, 60, 13, 81,109,105,244, 57, 81,202,120,
     91, 35,218,108, 45,161,110, 71, 37,185, 79,  3, 76, 24,214,187,
     76,199,217, 83,148,240, 52, 12,115,  5,141,189,212,185, 78,  6,
    140, 68, 50, 41, 69,100, 38,127,229,200,178,131,131,164, 26,165,
     49,121,135,130, 76, 22, 44,162,149,234,160,249, 67, 71,139,163,
     70, 28,234, 85,205,202, 80,163,153, 33,213, 17, 70,  7,  2, 46,
    212, 72, 50, 68,254, 71, 54,170, 75,201, 79,140,199,173,210,157,
     45, 50,143,101, 52, 11,246,140,190,171, 40,249, 69,190,119, 91,
     37, 36,214, 77, 29,115,230,178,137,132,156, 39,  0,174,122,148,
    112,148,121,104,154,178,132, 90,210, 42, 27, 83,234,178,155,243,
     22,112,241, 83,192,169,213,187,228, 68,152, 50,125, 84,117,187,
    119, 50,190,  2,211,205, 69, 19,130,227, 53,110,237,204,213,202,
    157,118, 44,192, 78,191,101, 93,120,155, 19, 38, 28,247, 27,250,
    174, 83,131,172,114, 50,201,  1, 26,181, 69,100,233, 28,140,239,
     99,154,107,196,166,157, 12, 99,153,150,247, 64, 64, 55,217, 72,
    178, 30,137, 87, 23,149,116, 23,196,130,206,237,250,253,150,165,
      3, 60, 29,101,148,  8, 55,233,238,166,139, 18,144, 67,243, 34,
    146,142, 64, 15,116,173,238,217,  0, 54, 90,247, 16, 30,242,154,
     58,136,170, 24,147,169, 68,143, 48, 88, 37,188,171, 66,222,209,
     94,107, 44,176,151, 48,116,114, 90,237,150, 72,157,213,106,237,
    157,236, 32,217,202,157, 32, 57,156, 17,166,135,169,166, 82,111,
     40, 80,165,137, 62,100, 73,209,129,213,181, 68,159, 30,180, 25,
     64,240, 68, 73,185, 58,163, 12,101,197, 17, 50,133,150,229,250,
    172, 44,137, 24,209,175, 18,213, 51,189,180,190,129,116, 89, 50,
     53,146,119,111,226,181,165,100, 35, 87, 34,136,124, 53,125,115,
     42, 85,161,118, 91,101, 65, 99, 50,154,  7,188,112,213,171, 40,
     91,102, 31, 85, 52,153, 57,147, 16, 81, 52,113,163,197, 80, 42,
    148, 64, 95,101, 34,196, 71,164,136, 37, 29,166, 84, 96, 40,169,
    161,123, 92,169, 68,235,163,150,138, 52,246, 64, 34,157,110,176,
     83,216, 10, 38, 66,248,108,131, 36,144,187,162,219,116, 43, 43,
    149, 91, 77,131, 55, 85, 44,151, 67, 13, 43,116, 43, 28,165,185,
    131,194,225,115,151, 42, 49,  0,177,204,234,168, 73, 11, 41, 92,
    182, 84,166,114,232,235,202,185,138,144, 69, 74,172,  3, 29, 50,
    217, 19, 34, 49,117, 86,144,128, 13,138,  5,189,153, 13,172, 17,
     68,239, 78,122,106, 17, 67,123, 99, 58, 58,232, 36,113,200, 59,
    168, 30, 17,206,135, 92,246, 40, 82,140, 65, 10, 14,136,202, 62,
      7,169, 68,193,249,150, 57,209,129, 41,137, 74,148,160, 89,  8,
     64,183,250,169,182,209, 18, 50,186, 37,170,153,100,  4, 20,181,
     80, 46, 31, 82,177,202,112, 63, 14,235,250, 35,207,158,  7,216,
    185, 30,125, 82,148,238,254,170, 83, 68,201, 37, 59,191,162, 50,
    149, 30, 39,160,117, 61,213, 29, 99, 46,189, 88,229, 46, 83,  6,
    197,251, 65,118,215,218,202, 81, 56, 52,192,114,155,112,123,163,
    151, 94, 45, 76,112,167, 43,129, 71, 14,181,199, 36,128,211,162,
    232,174,179, 35,150,166,143,117,  7,209,106,186,207,129,245, 36,
     29, 58, 43,180,234,210,203,121,150, 34,112,105,141,236,221,243,
     93, 28, 39,  3,115,240,221,202,109,148, 75,228,170,137,149,211,
     60,142,201,113,170,228,189, 73,100,196, 30,106, 44,163,172,198,
     74, 67, 45, 35, 73, 94,152,150,117,160,113, 34,250,165,167,172,
    232,177, 15, 12,162, 74, 53, 44,184,151,221,121,230, 83,168,153,
     80,106,170, 51, 53, 61,147, 73, 94,107, 46,187, 29,103, 25,170,
     19,102,117,177, 72,110,166,220,250,203, 18, 28,246,114,142,195,
    172,102, 99,213,111, 98, 39,140, 70, 86,228,230, 93, 99, 52, 78,
      5,111,184, 45,117, 73,212,135,206,242,174, 36,212, 65,197,117,
    137, 70,167,196,240, 79,170,187,102,173,145,174,187, 20,153,146,
    101,166, 50, 70,184,101,234,184, 78,102,166,199,187,208, 58,254,
     11, 54, 86,171,171, 52, 45,186, 91,104,130,230,230,240,244, 70,
     80, 31, 40,209, 19,170, 71, 81,124,203,109,148,152,140, 79, 64,
    186, 70,105,166, 82,107,124,214, 91,178, 68, 40, 69,154,150,202,
      8,214,159,193, 68,230, 81,161,133, 84, 75,104,214, 49, 93,148,
     34,238,138,237,147, 15,136, 22,182,149,169, 11,245,175,173,101,
     22, 59,187,117, 46,177,153, 70, 48,253, 66, 47,176,183,225,188,
     22, 58,246, 51, 73,  1,240,  8,235, 28,133, 24,199, 84,117,140,
    203,115, 40,104,221, 81,182,  3, 81,204,165,222, 51, 93, 77, 43,
    209, 29, 45,121,252, 80,181,241,116,114, 58, 68,134,131,170, 58,
    222, 40,242,241, 29,115,110,199,234,140,180,222,184,129,108,222,
    168,160,203, 35,155, 45, 79,232,133, 28,220, 99,221,194, 13,125,
    208,249, 86,220, 91,131,168, 26,210, 80, 49,184,173,225,167, 41,
     66,199,188, 30, 46, 69,218,195,235,221, 82,173,144, 81,194,185,
     86, 47, 96, 26, 59, 86,163,172, 23,193,154,148, 69,141,236,109,
     44, 81, 84,  2,211, 74,169, 54, 86, 87,116,114, 46,193, 89, 15,
    121, 13,131,158, 70,244,  5, 97,101,153,200,177,  8,180, 51,208,
     84,169,165,172, 78,  8, 70, 43,226, 25,229, 66,132, 38,105,232,
    128,247,199,197, 72, 38,206,128,247,195,202,128,217, 40,167, 42,
    166,175,121,242,160,149,207,106, 81, 72, 91,192,  6,149, 64,185,
     50,141, 86,  0,204, 16, 85, 73,184, 65,121,189, 16, 88,114,  6,
    178,103,  3,125, 16, 49,179,150,154,131, 84, 97,240,226,235,173,
    144,163,226,197, 18, 81, 51,129,236,197, 62,180,  5, 74,104,198,
     99,156,235, 85,200, 81,141,156,245,226,247, 70, 76, 35,165, 52,
    240, 64,137, 31,123,165,133,201, 45,  7, 42,168,111,239, 35, 28,
    215,199,111,186, 50,178, 24,107, 50, 34,109, 98, 62, 14, 28,205,
    253,228, 35, 49,229,125, 20,170,208,  7, 21, 64,195,228,140,216,
     53,  3,  6, 37,221, 90,164,161,176,177,222,235, 40,161,185,192,
     11,232,166,132, 45, 97, 28,  9, 64, 31, 27,197,248, 82,154,  0,
    243,160,119, 23,213, 41,191, 40,131,229,105,226,183,168, 40,157,
     76,131, 23, 35, 95, 86, 72,231, 15, 85, 41,235,197,163,254, 96,
    226,251,181, 19, 60, 13, 80,227,199,129,  8,231, 62, 59, 68, 91,
     69,245,178, 56, 79,  3,163,  6, 60,249,175,249, 81,202,120,154,
    163,198,159, 45, 86,222, 46,125,103,178,104,157,171,110,165, 20,
    115, 75, 77,134,137, 48,144, 87,142,149, 80,171, 67, 35,152,234,
    116, 69,143, 56,  2,168,106,166, 73,153, 19, 66,222, 10,241, 41,
     41, 97,224,138,213,202,104,161, 23,180,182,134,229, 99,  7, 64,
    225, 70,139, 32,133,159, 55,209,  4,222, 57,189, 20,208,108,115,
    185,101, 36,225,137,162,154, 40,246, 75,154,238,178, 82,117, 49,
    212,112,229,178,138, 11,118, 25,154,168,165,108, 76,152, 89, 70,
    135,248, 41,163,176,178,  8,213, 68,186,198,101,152,227,120,187,
     84, 91,173,177, 77, 13, 52,186,157, 93,182,100,124, 99,193, 77,
     58,196,135, 47,202,161,212, 65,191, 50,202, 72,169,234,148,  6,
    190, 41, 64, 40,138,165,182,218, 34,100,224,208,117,  8,137, 94,
    236, 57, 75, 85,186,115, 52, 13, 70, 88, 94, 93,223,251, 35, 68,
    201, 42, 41, 69, 34,164, 96,167,130, 40,157, 17, 67,173, 70,136,
    148, 23,209,  1,177,199,162, 20, 11,184,219, 84, 80, 92,215,141,
     81,164, 72,215, 42, 11,177,215, 85, 43, 38,120,130, 42,  9,165,
     52, 85,107,177, 48,240,215,236,165, 75,207,232,137, 58, 59, 10,
    162, 36,232,164, 53, 84,137,109,137,225,195,149, 74, 36,246,101,
     26, 89, 99,156,194,235, 83, 90,173,166, 81,115, 52,157, 21, 53,
    149,204,243, 35,165,144,232,218, 57, 85, 54, 36,153, 44,182, 21,
    101,106,173,169, 36,  6,149, 73, 44,160,148, 89,185,124, 86, 40,
    200,224,106,155, 45, 12,  7, 61, 78,136,157,140,116, 70,156, 58,
     44,101,128,146,221,117, 68,133,242,221, 29, 96,200, 30, 60, 77,
     84,210, 50, 55,137,221, 44,163, 47,149, 16,172,173, 75,106,158,
    192,181, 68, 61,173,  6,202,150, 83,218,168, 45,227, 46,171, 65,
     86,131, 49,209,  4, 15,  5, 41, 32,149,190,  9, 74,100,152,  1,
    204,138,129, 69, 25, 34,200,218, 58, 54, 16, 86, 34, 97,185,180,
    221,218,202, 80,141,117, 58,169,  6, 28,210,133,  2, 94, 77, 81,
    177, 12,119, 14,185, 52, 84,232,155,202,154,  2, 84,166,143, 99,
    219,161, 68,206,  2, 46,123, 79,202,142,116,167,186,162,185,106,
    169,165,213,196,115, 42, 81, 18, 60, 15,117, 45,165,198, 78,168,
    134,168, 93,155,152, 41,182, 81,217, 65,209, 85,185,133,  4, 37,
      5, 88,170,162, 97,108,173,108,166,144,123,  1,234,148,203, 48,
     84, 41, 75, 75, 69,125,208,163,116, 23, 68, 16,241,152, 85,115,
     91, 43,245,162, 40,167,132,105, 46,  0,115, 41,114,  6,151,165,
    149, 34,140, 19, 55,193, 17,214,189,233, 37, 74,103, 12, 67, 59,
    173,117, 78,122,178, 86,142,186,214,158,210,232,136,174,135,193,
     99,149, 53, 55, 76,212,109, 62,255,  0,193, 75,140,195,118, 31,
     49,103, 11,178,251,184,149, 22,229, 48, 40,203,107, 80, 69,125,
    148,214, 76,163,  1,115,172, 27,253,229,109,161,182,173,230, 85,
    102,167, 51,197, 21,214,212,195,107,165,185, 76, 11,121, 78,165,
     85,167,172, 59,235,221, 54,114,234, 53,135,192,  5, 59, 55, 80,
    185,245,173, 18,202, 99,126,240,149, 22,218, 84,100,213, 44,166,
    129, 55, 78, 42,253, 22,218, 41, 39,117, 25,158,151, 73,148,  6,
      9,171,119,  5,204,213,178, 39,153,  5,150,194, 38, 22, 40, 95,
     78,168, 82,140,141,  2,181,114,196,209,121,195,189,144,212,167,
     16,108, 25,111, 21, 54,154,  3,100,105, 52, 26,169,166, 80, 94,
    234, 26,245,240, 93,113, 58,202,159, 18, 25,207,101,214,211,214,
      8,222, 29,112, 69, 62,170,237,147,129,237,115,122,174,182,229,
    168,235, 67,101,113, 41,213,190, 18,236,137,102,173, 49,134, 31,
    116,217, 61,107,160,107,248, 87, 44,243,116,195,  0,  7,141,237,
    139,150, 91,104, 78,121,113,225,185,245, 91,179, 53,102,149,227,
    234,150, 81,  6, 98, 22,108,117,163, 39,123,141, 22,236,137,192,
    215, 57,186, 21,223, 23, 41,134,119, 63, 36,148,109,214,218,104,
    225, 85,155, 20,100, 78,111,213, 45, 90,159,153, 19,169,145,130,
     86,148,153,104,255,  0, 84,178,159,255,217,
    0 };

static uchar _file_2[] = {
     47, 42, 10, 32, 42,  9, 67,108, 97,115,115, 32,115,101,108,101,
     99,116,111,114,115, 10, 32, 42, 47, 10, 10, 98,111,100,121, 32,
    123, 10,  9, 98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,
    108,111,114, 58, 32, 35, 70, 70, 70, 70, 70, 70, 59, 32, 10,  9,
    109, 97,114,103,105,110, 58, 32, 48,112,120, 59, 10,125, 10, 10,
    100,105,118, 46,110, 97,118,105,103, 97,116,105,111,110, 32,123,
     10,  9,109, 97,114,103,105,110, 58, 32, 53,112,120, 32, 53,112,
    120, 32, 53,112,120, 32, 53,112,120, 59, 10,  9,112, 97,100,100,
    105,110,103, 58, 32, 48, 59, 10,  9,102,111,110,116, 45,102, 97,
    109,105,108,121, 58, 32, 86,101,114,100, 97,110, 97, 44, 32, 65,
    114,105, 97,108, 44, 32, 72,101,108,118,101,116,105, 99, 97, 44,
     32,115, 97,110,115, 45,115,101,114,105,102, 59, 32, 10,  9,102,
    111,110,116, 45,115,105,122,101, 58, 32,115,109, 97,108,108, 59,
     10,  9, 98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,
    111,114, 58, 32, 35, 70, 70, 70, 70, 70, 70, 59, 10,125, 10, 10,
    100,105,118, 46, 98,111,100,121, 32,123, 10,  9, 35,109, 97,120,
     45,119,105,100,116,104, 58, 32, 56, 48,101,109, 59, 10,  9,109,
     97,114,103,105,110, 58, 32, 48, 32, 97,117,116,111, 59, 10,  9,
    112, 97,100,100,105,110,103, 58, 32, 48, 59, 10,  9,102,111,110,
    116, 45,102, 97,109,105,108,121, 58, 32, 86,101,114,100, 97,110,
     97, 44, 32, 65,114,105, 97,108, 44, 32, 72,101,108,118,101,116,
    105, 99, 97, 44, 32,115, 97,110,115, 45,115,101,114,105,102, 59,
     32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,115,109,
     97,108,108, 59, 10,  9, 98, 97, 99,107,103,114,111,117,110,100,
     45, 99,111,108,111,114, 58, 32, 35, 70, 70, 70, 70, 70, 70, 59,
     10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48,
     59, 10,125, 10, 10,100,105,118, 46,116,111,112, 32,123, 10,  9,
    104,101,105,103,104,116, 58, 32, 49, 50, 53,112,120, 59, 10,  9,
    119,105,100,116,104, 58, 32, 49, 56, 48, 48,112,120, 59, 10,  9,
     98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,114,
     58, 32, 35, 70, 70, 70, 70, 70, 70, 59, 32, 10,  9,112,111,115,
    105,116,105,111,110, 58, 32,114,101,108, 97,116,105,118,101, 59,
     10,  9,109, 97,114,103,105,110, 58, 32, 48, 59, 10,  9,112, 97,
    100,100,105,110,103, 58, 32, 48, 59, 10,  9, 98, 97, 99,107,103,
    114,111,117,110,100, 58, 32,117,114,108, 40, 34,105,109, 97,103,
    101,115, 47, 98, 97,110,110,101,114, 46,106,112,103, 34, 41, 32,
    110,111, 45,114,101,112,101, 97,116, 32,116,111,112, 32,108,101,
    102,116, 59, 10,125, 10, 10, 10, 47, 42, 10, 32, 42,  9, 84,111,
    112, 32,110, 97,118,105,103, 97,116,105,111,110, 10, 32, 42, 47,
     10,100,105,118, 46,118,101,114,115,105,111,110, 32,123, 10, 32,
     32, 32, 32,100,105,115,112,108, 97,121, 58, 32, 98,108,111, 99,
    107, 59, 10, 32, 32, 32, 32,112,111,115,105,116,105,111,110, 58,
     32, 97, 98,115,111,108,117,116,101, 59, 10, 32, 32, 32, 32,116,
    111,112, 58, 32, 49, 53,112,120, 59, 10, 32, 32, 32, 32,114,105,
    103,104,116, 58, 32, 49, 50, 48,112,120, 59, 10, 32, 32, 32, 32,
    119,105,100,116,104, 58, 32, 55, 48, 48,112,120, 59, 10,  9, 95,
    102,108,111, 97,116, 58, 32,114,105,103,104,116, 59, 10,  9, 95,
    112, 97,100,100,105,110,103, 58, 32, 55, 48,112,120, 32, 49, 48,
    112,120, 32, 48, 32, 48, 59, 10,  9,102,111,110,116, 45,115,105,
    122,101, 58, 32, 49, 49, 53, 37, 59, 32, 10,  9, 99,111,108,111,
    114, 58, 32, 35, 48, 48, 48, 48, 48, 48, 59, 32, 10,125, 10, 10,
     47, 42, 10, 32, 42,  9, 84,111,112, 32,110, 97,118,105,103, 97,
    116,105,111,110, 10, 32, 42, 47, 10,100,105,118, 46,109,101,110,
    117, 32,123, 10,  9,102,108,111, 97,116, 58, 32,108,101,102,116,
     59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 55, 48,112,120,
     32, 48, 32, 48, 32, 50, 50,112,120, 59, 10,  9,102,111,110,116,
     45,115,105,122,101, 58, 32, 49, 49, 48, 37, 59, 10,  9, 99,111,
    108,111,114, 58, 32, 35, 69, 69, 69, 69, 69, 69, 59, 10,125, 10,
     10, 47, 42, 32, 78,111,110, 45, 73, 69, 32, 42, 47, 10,104,116,
    109,108, 62, 98,111,100,121, 32,100,105,118, 46,109,101,110,117,
     32,123, 10,125, 10, 10, 47, 42, 32, 73, 69, 32, 42, 47, 10, 42,
     32,104,116,109,108, 32,100,105,118, 46,109,101,110,117, 32,123,
     10,125, 10, 10, 47, 42, 32, 83, 65, 70, 65, 82, 73, 32, 42, 47,
     10, 98,111,100,121, 58,102,105,114,115,116, 45,111,102, 45,116,
    121,112,101, 32,100,105,118, 46,109,101,110,117, 32,123, 10,  9,
    116,111,112, 58, 32, 54, 49,112,120, 59, 10,125, 10, 10, 10,100,
    105,118, 46,109,101,110,117, 32, 97, 58,108,105,110,107, 44, 32,
    100,105,118, 46,109,101,110,117, 32, 97, 58,118,105,115,105,116,
    101,100, 32,123, 32, 10,  9, 99,111,108,111,114, 58, 32, 35, 70,
     70, 70, 70, 70, 70, 59, 32, 10,  9,116,101,120,116, 45,100,101,
     99,111,114, 97,116,105,111,110, 58, 32,110,111,110,101, 59, 10,
      9,102,111,110,116, 45,115,105,122,101, 58, 32, 56, 53, 37, 59,
     10,125, 10, 10,100,105,118, 46,109,101,110,117, 32, 97, 58,104,
    111,118,101,114, 32,123, 32, 10,  9, 99,111,108,111,114, 58, 32,
     35, 70, 70, 70, 70, 70, 70, 59, 32, 10,  9,116,101,120,116, 45,
    100,101, 99,111,114, 97,116,105,111,110, 58, 32,117,110,100,101,
    114,108,105,110,101, 59, 32, 10,125, 10, 10, 10, 47, 42, 32, 10,
     32, 42,  9, 83,101, 97,114, 99,104, 32, 10, 32, 42, 47, 10,100,
    105,118, 46,115,101, 97,114, 99,104, 32,123, 10,  9,119,105,100,
    116,104, 58, 32, 97,117,116,111, 59, 10,  9,112,111,115,105,116,
    105,111,110, 58, 32, 97, 98,115,111,108,117,116,101, 59, 10,  9,
    100,105,115,112,108, 97,121, 58, 32,105,110,108,105,110,101, 59,
     10,  9,116,111,112, 58, 32, 49, 52,112,120, 59, 10,  9,108,101,
    102,116, 58, 32, 55, 53, 48,112,120, 59, 10,  9,116,101,120,116,
     45, 97,108,105,103,110, 58, 32,114,105,103,104,116, 59, 32,  9,
      9, 47, 42, 32, 77, 97, 99, 32, 42, 47, 10,  9,112, 97,100,100,
    105,110,103, 58, 32, 48, 32, 51, 48,112,120, 32, 48, 32, 48, 59,
     10,  9,119,104,105,116,101, 45,115,112, 97, 99,101, 58, 32,110,
    111,119,114, 97,112, 59, 32,  9, 47, 42, 32, 79,112,101,114, 97,
     32, 42, 47, 10,  9,122, 45,105,110,100,101,120, 58, 32, 49, 49,
     59, 10, 32, 32, 32, 32,100,105,115,112,108, 97,121, 58, 32,110,
    111,110,101, 59, 10,125, 10, 10, 47, 42, 32, 78,111,110, 45, 73,
     69, 32, 42, 47, 10,104,116,109,108, 62, 98,111,100,121, 32,100,
    105,118, 46,115,101, 97,114, 99,104, 32,123, 10,125, 10, 10, 47,
     42, 32, 73, 69, 32, 42, 47, 10, 42, 32,104,116,109,108, 32,100,
    105,118, 46,115,101, 97,114, 99,104, 32,123, 10,  9,116,101,120,
    116, 45, 97,108,105,103,110, 58, 32,114,105,103,104,116, 59, 32,
      9,  9, 47, 42, 32, 77, 97, 99, 32, 42, 47, 10,125, 10, 10,100,
    105,118, 46,115,101, 97,114, 99,104, 32,108, 97, 98,101,108, 32,
    123, 32, 10,  9, 99,111,108,111,114, 58, 32, 35,102,102,102, 59,
     32,102,111,110,116, 45,115,105,122,101, 58, 32, 56, 53, 37, 59,
     32, 10,125, 10, 10,100,105,118, 46,115,101, 97,114, 99,104, 32,
    102,111,114,109, 32,105,110,112,117,116, 32,123, 32, 10,  9,102,
    111,110,116, 45,115,105,122,101, 58, 32, 56, 53, 37, 59, 32, 10,
    125, 10, 10,100,105,118, 46,115,101, 97,114, 99,104, 32,102,111,
    114,109, 32, 35,115,117, 98,109,105,116, 32,123, 10,  9,102,111,
    110,116, 45,115,105,122,101, 58, 32, 56, 53, 37, 59, 10,  9, 98,
     97, 99,107,103,114,111,117,110,100, 58, 32, 35, 54, 65, 55, 51,
     56, 57, 59, 10,  9, 99,111,108,111,114, 58, 32, 35,102,102,102,
     59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 49,112,120, 32,
     52,112,120, 59, 10,  9, 98,111,114,100,101,114, 45,114,105,103,
    104,116, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 50,
     56, 51, 48, 52, 51, 59, 10,  9, 98,111,114,100,101,114, 45, 98,
    111,116,116,111,109, 58, 32, 49,112,120, 32,115,111,108,105,100,
     32, 35, 50, 56, 51, 48, 52, 51, 59, 10,  9, 98,111,114,100,101,
    114, 45,116,111,112, 58, 32, 49,112,120, 32,115,111,108,105,100,
     32, 35, 57, 48, 57, 55, 65, 50, 59, 10,  9, 98,111,114,100,101,
    114, 45,108,101,102,116, 58, 32, 49,112,120, 32,115,111,108,105,
    100, 32, 35, 57, 48, 57, 55, 65, 50, 59, 10,125, 10, 10,100,105,
    118, 46,115,101, 97,114, 99,104, 32,102,111,114,109, 32, 35,113,
     32,123, 10,  9,119,105,100,116,104, 58, 32, 49, 55, 48,112,120,
     59, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32, 56, 53,
     37, 59, 10,  9, 98,111,114,100,101,114, 58,  9, 49,112,120, 32,
    115,111,108,105,100, 32, 35, 57, 48, 57, 55, 65, 50, 59, 10,  9,
     98, 97, 99,107,103,114,111,117,110,100, 58, 32, 35, 68, 57, 68,
     66, 69, 49, 59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 50,
    112,120, 59, 10,125, 10, 10,100,105,118, 46,115,101, 97,114, 99,
    104, 32,102,111,114,109, 32, 35,113, 58,104,111,118,101,114, 44,
     32,100,105,118, 46,115,101, 97,114, 99,104, 32,102,111,114,109,
     32, 35,113, 58,102,111, 99,117,115, 32,123, 10,  9, 98, 97, 99,
    107,103,114,111,117,110,100, 58, 32, 35,102,102,102, 59, 10,125,
     10, 10, 10, 47, 42, 32, 10, 32, 42,  9, 67,111,110,116,101,110,
    116, 32, 10, 32, 42, 47, 10,100,105,118, 46, 99,111,110,116,101,
    110,116, 32,123, 10,  9,109, 97,114,103,105,110, 58, 32, 50, 48,
    112,120, 59, 10,125, 10, 10,104, 50, 46, 99,108, 97,115,115, 83,
    101, 99,116,105,111,110, 32,123, 10,  9,109, 97,114,103,105,110,
     58, 32, 48, 32, 48, 32, 54,112,120, 32, 48, 59, 10,125, 10, 10,
    104, 51, 46,109,101,116,104,111,100, 78, 97,109,101, 32,123, 10,
      9,109, 97,114,103,105,110, 58, 32, 49, 53,112,120, 32, 48, 32,
     52,112,120, 32, 48, 59, 10,  9,102,111,110,116, 45,115,105,122,
    101, 58, 32, 49, 49, 48, 37, 59, 10,125, 10, 10,104, 51, 46,109,
    101,116,104,111,100, 83,101, 99,116,105,111,110, 32,123, 10,  9,
    109, 97,114,103,105,110, 58, 32, 49, 53,112,120, 32, 48, 32, 52,
    112,120, 32, 48, 59, 10,  9,102,111,110,116, 45,115,105,122,101,
     58, 32, 49, 48, 48, 37, 59, 10,  9, 99,111,108,111,114, 58, 32,
     35, 52, 48, 54, 48, 57, 48, 59, 10,125, 10, 10,116, 97, 98,108,
    101, 46,110, 97,118,105,103, 97,116,105,111,110, 32,123, 10, 32,
     32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,101, 59,
     10,  9,109, 97,114,103,105,110, 58, 32, 48, 32, 48, 32, 48, 32,
     49, 53,112,120, 59, 10,125, 10, 10,116, 97, 98,108,101, 46,110,
     97,118,105,103, 97,116,105,111,110, 32,116,100, 32,123, 10, 32,
     32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,101, 59,
     10,  9,112, 97,100,100,105,110,103, 58, 32, 53,112,120, 32, 50,
     48,112,120, 32, 48, 32, 48, 59, 10,  9, 99,111,108,111,114, 58,
     32, 35, 50, 48, 52, 48, 55, 48, 59, 10,125, 10, 10,116, 97, 98,
    108,101, 46, 99,108, 97,115,115, 72,101, 97,100, 32,123, 10, 32,
     32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,101, 59,
     10,  9,109, 97,114,103,105,110, 45, 98,111,116,116,111,109, 58,
     32, 49, 53,112,120, 59, 10,125, 10, 10,116, 97, 98,108,101, 46,
     99,108, 97,115,115, 72,101, 97,100, 32,116,100, 32,123, 10, 32,
     32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,101, 59,
     10,  9,112, 97,100,100,105,110,103, 58, 32, 48,112,120, 32, 50,
     48,112,120, 32, 55,112,120, 32, 48, 59, 10,125, 10, 10,116, 97,
     98,108,101, 46,105,116,101,109, 84, 97, 98,108,101, 32,123, 10,
      9,119,105,100,116,104, 58, 32, 57, 53, 37, 59, 10,  9, 98, 97,
     99,107,103,114,111,117,110,100, 45, 99,111,108,111,114, 58, 32,
     35, 70, 65, 70, 65, 70, 65, 59, 10, 32, 32, 32, 32,112, 97,100,
    100,105,110,103, 58, 32, 48, 59, 10, 32, 32, 32, 32,109, 97,114,
    103,105,110, 58, 32, 48, 59, 10,  9, 98,111,114,100,101,114, 45,
    115,112, 97, 99,105,110,103, 58, 32, 48, 59, 10,  9, 98,111,114,
    100,101,114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35,
     99, 99, 99, 59, 10, 32, 32, 32, 32, 98,111,114,100,101,114, 58,
     32,110,111,110,101, 59, 10, 32, 32, 32, 32,108,105,110,101, 45,
    104,101,105,103,104,116, 58, 32, 49, 49, 48, 37, 59, 10, 32, 32,
     32, 32, 98,111,114,100,101,114, 45, 99,111,108,108, 97,112,115,
    101, 58, 32, 99,111,108,108, 97,112,115,101, 59, 10,125, 10, 10,
    116, 97, 98,108,101, 46,105,116,101,109, 84, 97, 98,108,101, 32,
    116,114, 32,123, 10,  9, 98,111,114,100,101,114, 58, 32,110,111,
    110,101, 59, 10,125, 10, 10, 10, 47, 42, 32, 65, 80, 73, 32, 73,
    110,100,101,120, 32, 42, 47, 10,116, 97, 98,108,101, 46, 97,112,
    105, 73,110,100,101,120, 32,123, 10, 32, 32, 32, 32,119,105,100,
    116,104, 58, 32, 57, 53, 37, 59, 10,  9, 98, 97, 99,107,103,114,
    111,117,110,100, 45, 99,111,108,111,114, 58, 32, 35, 70, 65, 70,
     65, 70, 65, 59, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,
    120, 32,115,111,108,105,100, 32, 35, 99, 99, 99, 59, 10, 32, 32,
     32, 32, 98,111,114,100,101,114, 45, 99,111,108,108, 97,112,115,
    101, 58, 32, 99,111,108,108, 97,112,115,101, 59, 10, 32, 32, 32,
     32,112, 97,100,100,105,110,103, 58, 32, 48, 59, 10, 32, 32, 32,
     32,109, 97,114,103,105,110, 58, 32, 48, 59, 10, 32, 32, 32, 32,
    108,105,110,101, 45,104,101,105,103,104,116, 58, 32, 49, 48, 48,
     37, 59, 10, 32, 32, 32, 32,102,111,110,116, 45,115,105,122,101,
     58, 32, 57, 48, 37, 59, 10,125, 10, 10,116,114, 46, 97,112,105,
     68,101,102, 32,123, 10,  9, 98,111,114,100,101,114, 45,116,111,
    112, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 99, 99,
     99, 59, 10,125, 10, 10,116,114, 46, 97,112,105, 68,101,102, 32,
    116,100, 32,123, 10, 32, 32, 32, 32, 98,111,114,100,101,114, 58,
     32,110,111,110,101, 59, 10,125, 10, 10,116,114, 46, 97,112,105,
     66,114,105,101,102, 32,116,100, 32,123, 10, 32, 32, 32, 32, 98,
    111,114,100,101,114, 58, 32,110,111,110,101, 59, 10,125, 10, 10,
     46, 97,112,105, 66,114,105,101,102, 32,123, 10,  9,102,111,110,
    116, 45,115,105,122,101, 58, 32, 57, 48, 37, 59, 10,  9, 99,111,
    108,111,114, 58, 32, 35, 54, 54, 54, 54, 54, 54, 59, 10,  9,102,
    111,110,116, 45,115,116,121,108,101, 58, 32,105,116, 97,108,105,
     99, 59, 10,125, 10, 10, 46, 97,112,105, 84,121,112,101, 32,123,
     10,  9,116,101,120,116, 45, 97,108,105,103,110, 58, 32,114,105,
    103,104,116, 59, 10, 32, 32, 32, 32,112, 97,100,100,105,110,103,
     58, 32, 48, 32, 49, 48,112,120, 32, 48, 32, 49, 53,112,120, 59,
     10, 32, 32, 32, 32,119,104,105,116,101, 45,115,112, 97, 99,101,
     58, 32,110,111,119,114, 97,112, 59, 10,125, 10, 10, 47, 42, 32,
     80,101,114, 32, 65, 80, 73, 32, 98,108,111, 99,107, 32, 42, 47,
     10,100,105,118, 46, 97,112,105, 32,123, 10, 32, 32, 32, 32,109,
     97,114,103,105,110, 58, 32, 49, 48,112,120, 32, 48, 32, 54,112,
    120, 32, 48, 59, 10,125, 10, 10,100,105,118, 46, 97,112,105, 83,
    105,103, 32,123, 10,  9,102,111,110,116, 45,119,101,105,103,104,
    116, 58, 32, 98,111,108,100, 59, 10,  9, 98,111,114,100,101,114,
     58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 56, 52, 98,
     48, 99, 55, 59,  9, 10,  9, 99,111,108,111,114, 58, 32, 35, 48,
     48, 48, 48, 52, 48, 59, 10,  9,109, 97,114,103,105,110, 45,114,
    105,103,104,116, 58, 32, 50, 48,112,120, 59, 10,  9,112, 97,100,
    100,105,110,103, 58, 32, 56,112,120, 32, 49, 48,112,120, 32, 56,
    112,120, 32, 51, 48,112,120, 59, 10, 32, 32, 32, 32,116,101,120,
    116, 45,105,110,100,101,110,116, 58, 32, 45, 50, 49,112,120, 59,
     10,  9, 98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,
    111,114, 58, 32, 35,100, 53,101, 49,101, 56, 59, 10,  9,102,111,
    110,116, 45,119,101,105,103,104,116, 58, 32, 98,111,108,100, 59,
     10,  9, 45,119,101, 98,107,105,116, 45, 98,111,114,100,101,114,
     45,116,111,112, 45,108,101,102,116, 45,114, 97,100,105,117,115,
     58, 32, 49, 48,112,120, 59, 10,  9, 45,119,101, 98,107,105,116,
     45, 98,111,114,100,101,114, 45,116,111,112, 45,114,105,103,104,
    116, 45,114, 97,100,105,117,115, 58, 32, 49, 48,112,120, 59, 10,
      9, 45,109,111,122, 45, 98,111,114,100,101,114, 45,114, 97,100,
    105,117,115, 45,116,111,112,108,101,102,116, 58, 32, 49, 48,112,
    120, 59, 10,  9, 45,109,111,122, 45, 98,111,114,100,101,114, 45,
    114, 97,100,105,117,115, 45,116,111,112,114,105,103,104,116, 58,
     32, 49, 48,112,120, 59, 10,125, 10, 10,100,105,118, 46, 97,112,
    105, 68,101,116, 97,105,108, 32,123, 10,  9,109, 97,114,103,105,
    110, 45, 98,111,116,116,111,109, 58, 32, 49, 48,112,120, 59, 10,
      9,109, 97,114,103,105,110, 45,114,105,103,104,116, 58, 32, 50,
     48,112,120, 59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 50,
    112,120, 32, 49, 48,112,120, 32, 53,112,120, 32, 49, 48,112,120,
     59, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,115,
    111,108,105,100, 32, 35, 56, 52, 98, 48, 99, 55, 59,  9, 10,  9,
     98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,114,
     58, 32, 35, 70, 54, 70, 54,102, 65, 59, 10,  9, 98,111,114,100,
    101,114, 45,116,111,112, 45,119,105,100,116,104, 58, 32, 48, 59,
     10,  9, 45,119,101, 98,107,105,116, 45, 98,111,114,100,101,114,
     45, 98,111,116,116,111,109, 45,108,101,102,116, 45,114, 97,100,
    105,117,115, 58, 32, 49, 48,112,120, 59, 10,  9, 45,119,101, 98,
    107,105,116, 45, 98,111,114,100,101,114, 45, 98,111,116,116,111,
    109, 45,114,105,103,104,116, 45,114, 97,100,105,117,115, 58, 32,
     49, 48,112,120, 59, 10,  9, 45,109,111,122, 45, 98,111,114,100,
    101,114, 45,114, 97,100,105,117,115, 45, 98,111,116,116,111,109,
    108,101,102,116, 58, 32, 49, 48,112,120, 59, 10,  9, 45,109,111,
    122, 45, 98,111,114,100,101,114, 45,114, 97,100,105,117,115, 45,
     98,111,116,116,111,109,114,105,103,104,116, 58, 32, 49, 48,112,
    120, 59, 10, 32, 32, 32, 32,108,105,110,101, 45,104,101,105,103,
    104,116, 58, 32, 49, 52, 48, 37, 59, 10,125, 10, 10,100,116, 32,
    123, 10,  9,102,111,110,116, 45,119,101,105,103,104,116, 58, 32,
     98,111,108,100, 59, 10,  9, 99,111,108,111,114, 58, 32, 35, 48,
     48, 48, 48, 52, 48, 59, 10,125, 10, 10,116, 97, 98,108,101, 46,
    112, 97,114, 97,109,101,116,101,114,115, 32,123, 10, 32, 32, 32,
     32,109, 97,114,103,105,110, 58, 32, 52,112,120, 32, 48, 32, 48,
     32, 48, 59, 10, 32, 32, 32, 32,112, 97,100,100,105,110,103, 58,
     32, 54,112,120, 59, 10, 32, 32, 32, 32, 98,111,114,100,101,114,
     58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 56, 56, 56,
     59, 10, 32, 32, 32, 32, 98,111,114,100,101,114, 45, 99,111,108,
    108, 97,112,115,101, 58, 32, 99,111,108,108, 97,112,115,101, 59,
     10,  9, 45,119,101, 98,107,105,116, 45, 98,111,120, 45,115,104,
     97,100,111,119, 58, 32, 53,112,120, 32, 53,112,120, 32, 57,112,
    120, 32, 35, 56, 56, 56, 59, 10,125, 10, 10,116, 97, 98,108,101,
     46,112, 97,114, 97,109,101,116,101,114,115, 32,116,100, 32,123,
     10, 32, 32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,
    101, 59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 52,112,120,
     32, 54,112,120, 32, 52,112,120, 32, 56,112,120, 59, 10, 32, 32,
     32, 32, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,115,111,
    108,105,100, 32, 35, 56, 56, 56, 59, 10,125, 10, 10,116,114, 46,
    112, 97,114, 97,109, 32,123, 10, 32, 32, 32, 32, 98,111,114,100,
    101,114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 56,
     56, 56, 59, 10,125, 10, 10,116,100, 46,112, 97,114, 97,109, 32,
    123, 10,  9,102,111,110,116, 45,115,116,121,108,101, 58, 32,105,
    116, 97,108,105, 99, 59, 10,  9,116,101,120,116, 45, 97,108,105,
    103,110, 58, 32,114,105,103,104,116, 59, 10, 32, 32, 32, 32,119,
    104,105,116,101, 45,115,112, 97, 99,101, 58, 32,110,111,119,114,
     97,112, 59, 10,125, 10, 10,112, 46, 99,108, 97,115,115, 66,114,
    105,101,102, 32,123, 32, 10, 32, 32, 32, 32,109, 97,114,103,105,
    110, 58, 32, 49, 53,112,120, 32, 48,112,120, 32, 53,112,120, 32,
     50, 53,112,120, 59, 10,125, 10, 10,112, 46, 99,108, 97,115,115,
     68,101,115, 99,114,105,112,116,105,111,110, 32,123, 32, 10, 32,
     32, 32, 32,109, 97,114,103,105,110, 58, 32, 49, 48,112,120, 32,
     48,112,120, 32, 49, 48,112,120, 32, 50, 53,112,120, 59, 10,125,
     10, 10,112, 46,100,101,116, 97,105,108, 32,123, 32, 10, 32, 32,
     32, 32,109, 97,114,103,105,110, 58, 32, 49, 48,112,120, 32, 48,
    112,120, 32, 53,112,120, 32, 50, 53,112,120, 59, 10,125, 10, 10,
    112, 46,105,110,104,101,114,105,116,101,100, 76,105,110,107, 32,
    123, 32, 10, 32, 32, 32, 32,109, 97,114,103,105,110, 58, 32, 53,
    112,120, 32, 48, 32, 53,112,120, 32, 50,112,120, 59, 10,125, 10,
     10, 10, 47, 42, 10, 32, 42,  9, 83,116, 97,110,100, 97,114,100,
     32,101,108,101,109,101,110,116,115, 10, 32, 42, 47, 10,104, 49,
     32,123, 32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,
    120, 45,108, 97,114,103,101, 59, 10,  9, 99,111,108,111,114, 58,
     32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,  9,102,111,110,116,
     45,119,101,105,103,104,116, 58, 32, 98,111,108,100, 59, 10,  9,
    109, 97,114,103,105,110, 58, 32, 49, 52,112,120, 32, 48, 32, 49,
     54,112,120, 32, 48, 59, 10,125, 32, 10, 10,104, 50, 32,123, 32,
     10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,108, 97,114,
    103,101, 59, 10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52,
     48, 55, 48, 59, 32, 10,  9,102,111,110,116, 45,119,101,105,103,
    104,116, 58, 32, 98,111,108,100, 59, 10,  9,109, 97,114,103,105,
    110, 58, 32, 49, 52,112,120, 32, 48, 32, 48, 32, 48, 59, 10,125,
     10, 10,104, 51, 32,123, 32, 10,  9,102,111,110,116, 45,115,105,
    122,101, 58, 32,109,101,100,105,117,109, 59, 10,  9, 99,111,108,
    111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,  9,102,
    111,110,116, 45,119,101,105,103,104,116, 58, 32, 98,111,108,100,
     59, 10,  9,109, 97,114,103,105,110, 58, 32, 56,112,120, 32, 48,
     32, 52,112,120, 32, 48, 59, 10,125, 10, 10,104, 52, 32,123, 32,
     10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48,
     59, 32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,115,
    109, 97,108,108, 59, 10,  9,102,111,110,116, 45,115,116,121,108,
    101, 58, 32,105,116, 97,108,105, 99, 59, 32, 10,125, 10, 10, 47,
     42, 10, 32, 42,  9, 83,116, 97,110,100, 97,114,100, 32,101,108,
    101,109,101,110,116,115, 10, 32, 42, 47, 10,104, 49, 32,123, 32,
     10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,120, 45,108,
     97,114,103,101, 59, 10,  9, 99,111,108,111,114, 58, 32, 35, 50,
     48, 52, 48, 55, 48, 59, 32, 10,  9,102,111,110,116, 45,119,101,
    105,103,104,116, 58, 32, 98,111,108,100, 59, 10,  9,109, 97,114,
    103,105,110, 58, 32, 49, 52,112,120, 32, 48, 32, 49, 54,112,120,
     32, 48, 59, 10,125, 32, 10, 10,104, 50, 32,123, 32, 10,  9,102,
    111,110,116, 45,115,105,122,101, 58, 32,108, 97,114,103,101, 59,
     10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48,
     59, 32, 10,  9,102,111,110,116, 45,119,101,105,103,104,116, 58,
     32, 98,111,108,100, 59, 10,  9,109, 97,114,103,105,110, 58, 32,
     49, 52,112,120, 32, 48, 32, 48, 32, 48, 59, 10,125, 10, 10,104,
     51, 32,123, 32, 10,  9,102,111,110,116, 45,115,105,122,101, 58,
     32,109,101,100,105,117,109, 59, 10,  9, 99,111,108,111,114, 58,
     32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,  9,102,111,110,116,
     45,119,101,105,103,104,116, 58, 32, 98,111,108,100, 59, 10,  9,
    109, 97,114,103,105,110, 58, 32, 56,112,120, 32, 48, 32, 52,112,
    120, 32, 48, 59, 10,125, 10, 10,104, 52, 32,123, 32, 10,  9, 99,
    111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,
      9,102,111,110,116, 45,115,105,122,101, 58, 32,115,109, 97,108,
    108, 59, 10,  9,102,111,110,116, 45,115,116,121,108,101, 58, 32,
    105,116, 97,108,105, 99, 59, 32, 10,125, 10, 10,112, 32,123, 32,
     10,  9,108,105,110,101, 45,104,101,105,103,104,116, 58, 32, 49,
     51, 48, 37, 59, 32, 10,  9,109, 97,114,103,105,110, 45,116,111,
    112, 58, 32, 53,112,120, 59, 10,125, 10, 10,117,108, 32,123, 32,
     10,  9,102,111,110,116, 45,115,105,122,101, 58, 32, 49, 48, 48,
     37, 59, 10,125, 10, 10,111,108, 32,123, 32, 32, 10,  9,102,111,
    110,116, 45,115,105,122,101, 58, 32, 49, 48, 48, 37, 59, 10,125,
     10, 10,112,114,101, 32,123, 32, 10, 32, 32, 32, 32,102,111,110,
    116, 45,115,105,122,101, 58, 32, 49, 49, 48, 37, 59, 10,  9,119,
    105,100,116,104, 58, 32, 57, 48, 37, 59, 10, 32, 32, 32, 32, 99,
    111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 10, 32,
     32, 32, 32,102,111,110,116, 45,102, 97,109,105,108,121, 58, 32,
     67,111,117,114,105,101,114, 32, 78,101,119, 44, 32, 67,111,117,
    114,105,101,114, 44, 32,109,111,110,111,115,112, 97, 99,101, 59,
     10, 32, 32, 32, 32,112, 97,100,100,105,110,103, 58, 32, 53,112,
    120, 32, 49, 53,112,120, 32, 53,112,120, 32, 49, 53,112,120, 59,
     10, 32, 32, 32, 32,109, 97,114,103,105,110, 58, 32, 49, 48,112,
    120, 32, 49,112,120, 32, 53,112,120, 32, 49,112,120, 59, 10, 32,
     32, 32, 32, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,115,
    111,108,105,100, 32, 35, 56, 54, 56, 54, 56, 54, 59, 10,  9, 98,
     97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,114, 58,
     32, 35, 70, 56, 70, 56, 70, 56, 59, 10,125, 10, 10, 46,115,109,
     97,108,108, 84,101,120,116, 32,123, 32, 10,  9,102,111,110,116,
     45,115,105,122,101, 58, 32, 57, 48, 37, 59, 32, 10,  9, 99,111,
    108,111,114, 58, 32, 35, 51, 49, 51, 49, 51, 49, 59, 32, 10,125,
     10, 10, 47, 42, 10, 32, 42,  9, 71,101,110,101,114, 97,108, 32,
    108,105,110,107, 32,104,105,103,104,108,105,103,104,116,105,110,
    103, 10, 32, 42, 47, 10, 97, 58,108,105,110,107, 32,123, 32, 10,
      9, 99,111,108,111,114, 58, 32, 35, 51, 49, 54, 57, 57, 67, 59,
     32, 10,125, 10, 10, 97, 58,118,105,115,105,116,101,100, 32,123,
     32, 10,  9, 99,111,108,111,114, 58, 32, 35, 51, 49, 54, 57, 57,
     67, 59, 32, 10,125, 10, 10, 97, 58,104,111,118,101,114, 32,123,
     32, 10,  9, 99,111,108,111,114, 58, 32, 35, 67, 67, 52, 52, 55,
     55, 59, 32, 10,125, 10, 10,100,105,118, 46,116,101,114,109,115,
     32,123, 10,  9,109, 97,114,103,105,110, 45,116,111,112, 58, 32,
     50, 48,112,120, 59, 10,125, 10, 10,100,105,118, 46,116,101,114,
    109,115, 32,112, 32, 97, 58,108,105,110,107, 44, 32,100,105,118,
     46,116,101,114,109,115, 32,112, 32, 97, 58,118,105,115,105,116,
    101,100, 32,123, 32, 10,  9,102,111,110,116, 45,115,105,122,101,
     58, 32,120, 45,115,109, 97,108,108, 59, 10,  9, 99,111,108,111,
    114, 58, 32, 35, 67, 48, 67, 48, 67, 48, 59, 32, 10,  9,116,101,
    120,116, 45,100,101, 99,111,114, 97,116,105,111,110, 58, 32,110,
    111,110,101, 59, 10,125, 10, 10,100,105,118, 46,116,101,114,109,
    115, 32,112, 32, 97, 58,104,111,118,101,114, 32,123, 32, 10,  9,
    102,111,110,116, 45,115,105,122,101, 58, 32,120, 45,115,109, 97,
    108,108, 59, 10,  9,116,101,120,116, 45,100,101, 99,111,114, 97,
    116,105,111,110, 58, 32,117,110,100,101,114,108,105,110,101, 59,
     32, 10,125, 10, 10, 47, 42, 10, 32, 42, 32,  9, 68,105,115, 97,
     98,108,101,100, 10, 32, 42, 47, 10,104,114, 32,123, 10,  9,104,
    101,105,103,104,116, 58, 32, 49,112,120, 59, 10,  9, 98, 97, 99,
    107,103,114,111,117,110,100, 45, 99,111,108,111,114, 58, 32, 35,
     70, 70, 70, 70, 70, 70, 59, 10,  9, 99,111,108,111,114, 58, 32,
     35, 70, 70, 70, 70, 70, 70, 59, 10,  9, 99,111,108,111,114, 58,
     32, 35, 70, 48, 70, 48, 70, 48, 59, 10,  9,109, 97,114,103,105,
    110, 58, 32, 49, 53,112,120, 32, 48, 32, 49, 48,112,120, 32, 48,
     59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 48, 32, 48, 32,
     48, 32, 48, 59, 10,  9, 98,111,114,100,101,114, 45,115,116,121,
    108,101, 58, 32,110,111,110,101, 59, 10,125, 10, 10,105,109,103,
     32,123, 10,  9, 98,111,114,100,101,114, 45,115,116,121,108,101,
     58, 32,110,111,110,101, 59, 10,125, 10, 10,105,109,103, 46,119,
    114, 97,112, 76,101,102,116, 32,123, 10,  9,102,108,111, 97,116,
     58, 32,108,101,102,116, 59, 10,  9,109, 97,114,103,105,110, 58,
     32, 48,112,120, 32, 49, 48,112,120, 32, 51,112,120, 32, 48,112,
    120, 59, 10,125, 10, 10,105,109,103, 46,119,114, 97,112, 82,105,
    103,104,116, 32,123, 10,  9,102,108,111, 97,116, 58, 32,114,105,
    103,104,116, 59, 10,  9,109, 97,114,103,105,110, 58, 32, 48,112,
    120, 32, 48,112,120, 32, 51,112,120, 32, 49, 48,112,120, 59, 10,
    125, 10, 10,116, 97, 98,108,101, 32,123, 10,  9, 98,111,114,100,
    101,114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 65,
     48, 65, 48, 65, 48, 59, 10,  9,120, 98,111,114,100,101,114, 45,
     99,111,108,108, 97,112,115,101, 58, 32, 99,111,108,108, 97,112,
    115,101, 59, 10,  9,120, 98,111,114,100,101,114, 45,115,112, 97,
     99,105,110,103, 58, 32, 48,112,120, 59, 10,125, 10, 10,116,104,
     32,123, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,
    115,111,108,105,100, 32, 35, 65, 48, 65, 48, 65, 48, 59, 10,  9,
     98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,114,
     58, 32, 35, 70, 48, 70, 48, 70, 48, 59, 10,  9, 99,111,108,111,
    114, 58, 32, 35, 52, 48, 54, 48, 57, 48, 59, 10,  9,112, 97,100,
    100,105,110,103, 58, 32, 52,112,120, 32, 56,112,120, 32, 52,112,
    120, 32, 56,112,120, 59, 10,  9,116,101,120,116, 45, 97,108,105,
    103,110, 58, 32,108,101,102,116, 59, 10,125, 10, 10,116,100, 32,
    123, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,115,
    111,108,105,100, 32, 35, 65, 48, 65, 48, 65, 48, 59, 10,  9,112,
     97,100,100,105,110,103, 58, 32, 52,112,120, 32, 56,112,120, 32,
     52,112,120, 32, 56,112,120, 59, 10,125, 10, 10, 97, 32,123, 10,
      9,116,101,120,116, 45,100,101, 99,111,114, 97,116,105,111,110,
     58, 32,110,111,110,101, 59, 10,125, 10,
    0 };

static uchar _file_3[] = {
     71, 73, 70, 56, 57, 97, 14,  0,  9,  0,162,  0,  0, 70, 70, 70,
     49, 49, 49, 37, 37, 37, 33, 33, 33,  0,  0,  0,255,255,255,  0,
      0,  0,  0,  0,  0, 33,249,  4,  1,  7,  0,  5,  0, 44,  0,  0,
      0,  0, 14,  0,  9,  0,  0,  3, 26, 88,186, 11,252,140,  4,  8,
      9, 25, 52, 90,108,187,247, 83,214,  9,153, 98,145,101, 33,165,
    138,179, 36,  0, 59,
    0 };

DocFile docFiles[] = {
    { "images/banner.jpg", _file_1, 23899, 1 },
    { "doc.css", _file_2, 6842, 2 },
    { "images/inherit.gif", _file_3, 85, 3 },
    { 0, 0, 0, 0 },
};
#endif /* 1 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/docFiles.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/cmd/ejsmod.c"
 */
/************************************************************************/

/**
    ejsmod.c - Module manager 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static void getDepends(Ejs *ejs, MprList *list, EjsModule *mp);
static void manageMod(EjsMod *mp, int flags);
static int  process(EjsMod *mp, cchar *output, int argc, char **argv);
static void require(MprList *list, cchar *name);


MAIN(ejsmodMain, int argc, char **argv)
{
    Mpr             *mpr;
    EjsMod          *mp;
    Ejs             *ejs;
    MprList         *requiredModules;
    char            *argp, *searchPath, *output, *modules, *name, *tok;
    int             nextArg, err, flags;

    err = 0;
    output = searchPath = 0;
    requiredModules = 0;
    
    /*
        Initialze the Multithreaded Portable Runtime (MPR)
     */
    mpr = mprCreate(argc, argv, 0);
    mprSetAppName(argv[0], 0, 0);

    /*
        Allocate the primary control structure
     */
    if ((mp = mprAllocObj(EjsMod, manageMod)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(mp);
    mp->lstRecords = mprCreateList(0, 0);
    mp->blocks = mprCreateList(0, 0);
    mp->docDir = sclone(".");
    
    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--cslots") == 0) {
            mp->cslots = 1;
            mp->genSlots = 1;
            
        } else if (strcmp(argp, "--debugger") == 0 || strcmp(argp, "-D") == 0) {
            mprSetDebugMode(1);

        } else if (strcmp(argp, "--depends") == 0) {
            mp->depends = 1;
            
        } else if (strcmp(argp, "--error") == 0) {
            /*
                Undocumented switch
             */
            mp->exitOnError++;
            mp->warnOnError++;
            
        } else if (strcmp(argp, "--html") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                mp->docDir = sclone(argv[++nextArg]);
                mp->html = 1;
            }
            
        } else if (strcmp(argp, "--listing") == 0) {
            mp->listing = 1;
            mp->showAsm = 1;
            
        } else if (strcmp(argp, "--log") == 0) {
            /*
                Undocumented switch
             */
            if (nextArg >= argc) {
                err++;
            } else {
                ejsRedirectLogging(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--out") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                output = argv[++nextArg];
                mp->cslots = 1;
                mp->genSlots = 1;
            }

        } else if (strcmp(argp, "--search") == 0 || strcmp(argp, "--searchpath") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintfError("%s %s-%s\n", BLD_NAME, BLD_VERSION, BLD_NUMBER);  
            return 0;

        } else if (strcmp(argp, "--require") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                if (requiredModules == 0) {
                    requiredModules = mprCreateList(-1, 0);
                }
                modules = sclone(argv[++nextArg]);
#if MACOSX || WIN
                /*  Fix for Xcode and Visual Studio */
                if (modules[0] == ' ' || scmp(modules, "null") == 0) {
                    modules[0] = '\0';                    
                }
#endif
                name = stok(modules, " \t", &tok);
                while (name != NULL) {
                    require(requiredModules, name);
                    name = stok(NULL, " \t", &tok);
                }
            }

        } else if (strcmp(argp, "--warn") == 0) {
            mp->warnOnError++;

        } else if (strcmp(argp, "--xml") == 0) {
            mp->xml = 1;

        } else {
            err++;
            break;
        }
    }
    if (argc == nextArg) {
        err++;
    }
    if (mp->genSlots == 0 && mp->listing == 0 && mp->html == 0 && mp->xml == 0 && mp->depends == 0) {
        mp->listing = 1;
    }
    if (mp->depends && requiredModules == 0) {
        requiredModules = mprCreateList(-1, 0);
    }
    if (err) {
        /*
            Examples:
                ejsmod file.mod                              # Defaults to --listing
                ejsmod --listing embedthis.mod 
                ejsmod --out slots.h embedthis.mod 
         */
        mprPrintfError("Usage: %s [options] modules ...\n"
            "  Ejscript module manager options:\n"
            "  --cslots              # Generate a C slot definitions file\n"
            "  --html dir            # Generate HTML documentation to the specified directory\n"
            "  --listing             # Create assembler listing files (default)\n"
            "  --out                 # Output file for all C slots (implies --cslots)\n"
            "  --require \"modules\"   # List of modules to preload\n"
            "  --search ejsPath      # Module file search path\n"
            "  --version             # Emit the program version information\n"
            "  --warn                # Warn about undocumented methods or parameters in doc\n\n", mpr->name);
        return -1;
    }

    /*
        Need an interpreter to load modules
     */
    flags = EJS_FLAG_NO_INIT;
    if (mp->html || mp->xml) {
        flags |= EJS_FLAG_DOC;
    }
    ejs = ejsCreate(NULL, searchPath, requiredModules, 0, NULL, flags);
    if (ejs == 0) {
        return MPR_ERR_MEMORY;
    }
    mp->ejs = ejs;

    if (nextArg < argc) {
        if (process(mp, output, argc - nextArg, &argv[nextArg]) < 0) {
            err++;
        }
    }
    if (mp->errorCount > 0) {
        err = -1;
    }
    mprRemoveRoot(mp);
    ejsDestroy(ejs);
    mprDestroy(MPR_EXIT_DEFAULT);
    return err;
}


static void manageMod(EjsMod *mp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mp->ejs);
        mprMark(mp->currentLine);
        mprMark(mp->currentFileName);
        mprMark(mp->lstRecords);
        mprMark(mp->packages);
        mprMark(mp->blocks);
        mprMark(mp->currentBlock);
        mprMark(mp->docDir);
        mprMark(mp->path);
        mprMark(mp->file);
        mprMark(mp->module);
        mprMark(mp->fun);
        mprMark(mp->pc);
    }
}


static void require(MprList *list, cchar *name) 
{
    mprAssert(list);

    if (name && *name) {
        mprAddItem(list, name);
    }
}


static int process(EjsMod *mp, cchar *output, int argc, char **argv)
{
    Ejs         *ejs;
    EjsModule   *module;
    MprFile     *outfile;
    MprList     *depends;
    int         count, i, next, moduleCount;

    ejs = mp->ejs;
    
    if (output) {
        outfile = mprOpenFile(output, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664);
    } else {
        outfile = 0;
    }
    ejs->loaderCallback = (mp->listing) ? emListingLoadCallback : 0;
    mp->firstGlobal = ejsGetPropertyCount(ejs, ejs->global);

    /*
        For each module on the command line
     */
    for (i = 0; i < argc && !mp->fatalError; i++) {
        moduleCount = mprGetListLength(ejs->modules);
        ejs->userData = mp;
        if (!mprPathExists(argv[i], R_OK)) {
            mprError("Can't access module %s", argv[i]);
            return EJS_ERR;
        }
        if ((ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, argv[i]), -1, -1, EJS_LOADER_NO_INIT)) < 0) {
            ejs->loaderCallback = NULL;
            mprError("Can't load module %s\n%s", argv[i], ejsGetErrorMsg(ejs, 0));
            return EJS_ERR;
        }
        if (mp->genSlots) {
            for (next = moduleCount; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                emCreateSlotFiles(mp, module, outfile);
            }
        }
        if (mp->depends) {
            depends = mprCreateList(-1, 0);
            for (next = moduleCount; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                getDepends(ejs, depends, module);
            }
            count = mprGetListLength(depends);
            for (next = 1; (module = mprGetNextItem(depends, &next)) != 0; ) {
                int version = module->version;
                mprPrintf("%@-%d.%d.%d%s", module->name, EJS_MAJOR(version), EJS_MINOR(version), EJS_PATCH(version),
                    (next >= count) ? "" : " ");
            }
            printf("\n");
        }
    }
    if (mp->html || mp->xml) {
        emCreateDoc(mp);
    }
    mprCloseFile(outfile);
    return 0;
}


static void getDepends(Ejs *ejs, MprList *list, EjsModule *mp) 
{
    EjsModule   *module;
    int         next;

    if (mprLookupItem(list, mp) < 0) {
        mprAddItem(list, mp);
    }
    for (next = 0; (module = mprGetNextItem(mp->dependencies, &next)) != 0; ) {
        if (mprLookupItem(list, module) < 0) {
            mprAddItem(list, module);
        }
    }
}

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
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/ejsmod.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/cmd/listing.c"
 */
/************************************************************************/

/**
    listing.c - Assembler listing generator.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static EjsString *getBlockName(EjsMod *mp, EjsObj *block, int slotNum);
static uchar getByte(EjsMod *mp);
static uint  getInt32(EjsMod *mp);
static double getDouble(EjsMod *mp);
static int64 getNum(EjsMod *dp);
static EjsString *getString(Ejs *ejs, EjsMod *dp);
static void getGlobal(EjsMod *mp, char *buf, int buflen);
static void lstSlotAssignments(EjsMod *mp, EjsModule *module, EjsObj *parent, int slotNum, EjsObj *obj);
static char *getAttributeString(EjsMod *mp, int attributes);
static void interp(EjsMod *mp, EjsModule *module, EjsFunction *fun);
static void leadin(EjsMod *mp, EjsModule *module, int classDec, int inFunction);
static void lstBlock(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsString *name, int numProp);
static void lstClass(EjsMod *mp, EjsModule *module, int slotNum, EjsType *klass, int attributes);
static void lstClose(EjsMod *mp, MprList *modules, int origin);
static void lstDependency(EjsMod *mp, EjsModule *module, EjsModule *dependant);
static void lstEndModule(EjsMod *mp, EjsModule *module);
static void lstException(EjsMod *mp, EjsModule *module, EjsFunction *fun);
static void lstFunction(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, EjsFunction *fun, 
    int attributes);
static int  lstOpen(EjsMod *mp, char *moduleFilename, EjsModuleHdr *hdr);
static void lstProperty(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, int attributes, 
    EjsName typeName);
static void lstModule(EjsMod *mp, EjsModule *module);
static EjsString *mapSpace(Ejs *ejs, EjsString *space);

/*
    Listing loader callback. This is invoked at key points when loading a module file.
 */
void emListingLoadCallback(Ejs *ejs, int kind, ...)
{
    va_list         args;
    EjsModuleHdr    *hdr;
    EjsMod          *mp;
    Lst             *lst;
    MprList         *modules;
    char            *name;
    int             nextModule;

    va_start(args, kind);
    mp = ejs->userData;
    lst = mprAlloc(sizeof(Lst));

    /*
        Decode the record type and create a list for later processing. We need to process
        after the loader has done fixup for forward type references.
     */
    switch (kind) {

    case EJS_SECT_BLOCK:
        lst->module = va_arg(args, EjsModule*);
        lst->owner = va_arg(args, EjsObj*);
        lst->slotNum = va_arg(args, int);
        lst->name = va_arg(args, EjsString*);
        lst->numProp = va_arg(args, int);
        break;

    case EJS_SECT_BLOCK_END:
        break;

    case EJS_SECT_CLASS:
        lst->module = va_arg(args, EjsModule*);
        lst->slotNum = va_arg(args, int);
        lst->qname = va_arg(args, EjsName);
        lst->type = va_arg(args, EjsType*);
        lst->attributes = va_arg(args, int);
        break;

    case EJS_SECT_CLASS_END:
        break;

    case EJS_SECT_DEPENDENCY:
        lst->module = va_arg(args, EjsModule*);
        lst->dependency = va_arg(args, EjsModule*);
        break;

    case EJS_SECT_END:
        modules = va_arg(args, MprList*);
        nextModule = va_arg(args, int);
        lstClose(mp, modules, nextModule);
        return;

    case EJS_SECT_EXCEPTION:
        lst->module = va_arg(args, EjsModule*);
        lst->fun = va_arg(args, EjsFunction*);
        break;

    case EJS_SECT_FUNCTION:
        lst->module = va_arg(args, EjsModule*);
        lst->owner = va_arg(args, EjsObj*);
        lst->slotNum = va_arg(args, int);
        lst->qname = va_arg(args, EjsName);
        lst->fun = va_arg(args, EjsFunction*);
        lst->attributes = va_arg(args, int);
        //MOB mprAssert(lst->fun->body.code || lst->fun->isNativeProc);
        break;

    case EJS_SECT_FUNCTION_END:
        break;

    case EJS_SECT_START:
        name = va_arg(args, char*);
        hdr = va_arg(args, EjsModuleHdr*);
        lstOpen(mp, name, hdr);
        return;

    case EJS_SECT_PROPERTY:
        lst->module = va_arg(args, EjsModule*);
        lst->owner = va_arg(args, EjsObj*);
        lst->slotNum = va_arg(args, int);
        lst->qname = va_arg(args, EjsName);
        lst->attributes = va_arg(args, int);
        lst->typeName = va_arg(args, EjsName);
        break;

    case EJS_SECT_MODULE:
        break;

    case EJS_SECT_MODULE_END:
        break;
            
    case EJS_SECT_DEBUG:
        break;

    default:
        mprAssert(0);
    }
    lst->kind = kind;
    mprAddItem(mp->lstRecords, lst);
}


/*
    Loader completion routine. Process listing records and emit the listing file.
 */
static void lstClose(EjsMod *mp, MprList *modules, int firstModule)
{
    Ejs         *ejs;
    EjsModule   *module;
    Lst         *lst;
    bool        headerOutput;
    int         next, nextModule, count;

    ejs = mp->ejs;

    for (nextModule = firstModule; (module = (EjsModule*) mprGetNextItem(modules, &nextModule)) != 0; ) {
        headerOutput = 0;
        count = 0;
        for (next = 0; (lst = (Lst*) mprGetNextItem(mp->lstRecords, &next)) != 0; ) {
            if (lst->module != module) {
                continue;
            }
            if (!headerOutput) {
                lstModule(mp, lst->module);
                headerOutput = 1;
            }
            switch (lst->kind) {
            case EJS_SECT_BLOCK:
                lstBlock(mp, lst->module, lst->owner, lst->slotNum, lst->name, lst->numProp);
                count++;
                break;

            case EJS_SECT_CLASS:
                lstClass(mp, lst->module, lst->slotNum, lst->type, lst->attributes);
                count++;
                break;

            case EJS_SECT_DEPENDENCY:
                lstDependency(mp, lst->module, lst->dependency);
                break;

            case EJS_SECT_EXCEPTION:
                lstException(mp, lst->module, lst->fun);
                break;

            case EJS_SECT_FUNCTION:
                lstFunction(mp, lst->module, lst->owner, lst->slotNum, lst->qname, lst->fun, lst->attributes);
                count++;
                break;

            case EJS_SECT_PROPERTY:
                lstProperty(mp, lst->module, lst->owner, lst->slotNum, lst->qname, lst->attributes, lst->typeName);
                count++;
                break;

            default:
            case EJS_SECT_START:
            case EJS_SECT_END:
                mprAssert(0);
                break;
            }
        }
        if (count > 0) {
            lstEndModule(mp, module);
        }
    }
    mprCloseFile(mp->file);
    mp->file = 0;
}


static int lstOpen(EjsMod *mp, char *moduleFilename, EjsModuleHdr *hdr)
{
    char    *path, *name, *ext;

    mprAssert(mp);

    name = mprGetPathBase(moduleFilename);
    if ((ext = strstr(name, EJS_MODULE_EXT)) != 0) {
        *ext = '\0';
    }
    path = sjoin(name, EJS_LISTING_EXT, NULL);
    if ((mp->file = mprOpenFile(path,  O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664)) == 0) {
        mprError("Can't create %s", path);
        return EJS_ERR;
    }
    mprEnableFileBuffering(mp->file, 0, 0);
    mprFprintf(mp->file, "#\n#  %s -- Module Listing for %s\n#\n", path, moduleFilename);
    return 0;
}


static void lstBlock(EjsMod *mp, EjsModule *module, EjsObj *owner, int slotNum, EjsString *name, int numProp)
{
    Ejs         *ejs;
    EjsString   *blockName;

    ejs = mp->ejs;

    blockName = getBlockName(mp, owner, slotNum);
    mprFprintf(mp->file, "BLOCK:      [%@-%02d]  %@ (Slots %d)\n", blockName, slotNum, name, numProp);
}


/*
    List a class (type)
 */
static void lstClass(EjsMod *mp, EjsModule *module, int slotNum, EjsType *klass, int attributes)
{
    Ejs         *ejs;

    ejs = mp->ejs;
    mprFprintf(mp->file, "\n");

    if (klass->baseType) {
        mprFprintf(mp->file, "CLASS:      %sclass %@ extends %@\n", getAttributeString(mp, attributes), 
            klass->qname.name, klass->baseType->qname.name);
    } else {
        mprFprintf(mp->file, "CLASS:      %sclass %@\n", getAttributeString(mp, attributes), klass->qname.name);
    }
    leadin(mp, module, 1, 0);
    mprFprintf(mp->file, 
        "        #  Class Details: %d class traits, %d prototype (instance) traits, %s, requested slot %d\n",
        ejsGetPropertyCount(ejs, (EjsObj*) klass),
        klass->prototype ? ejsGetPropertyCount(ejs, klass->prototype) : 0, 
        klass->hasInstanceVars ? "has-state": "", slotNum);
}


static void lstDependency(EjsMod *mp, EjsModule *module, EjsModule *dependant)
{
    leadin(mp, module, 0, 0);
    mprFprintf(mp->file, "DEPENDENCY: require %@ (sum %d)\n\n", dependant->vname, dependant->checksum);
}


static void lstEndModule(EjsMod *mp, EjsModule *module)
{
    char    *pp, *end;
    ssize   size;
    int     i;

    mprAssert(mp);

    mprFprintf(mp->file,
        "\n----------------------------------------------------------------------------------------------\n");

    lstSlotAssignments(mp, module, NULL, 0, mp->ejs->global);

    /*
        Dump the constant pool
     */
    size = module->constants->poolLength;
    mprFprintf(mp->file,
        "\n----------------------------------------------------------------------------------------------\n"
        "#\n"
        "#  Constant Pool (size %d bytes)\n"
        "#\n", size);

    pp = (char*) module->constants->pool;
    end = &((char*) module->constants->pool)[size];
    for (i = 0; pp < end; i++) {
        mprFprintf(mp->file, "%04d   \"%s\"\n", i, pp);
        pp += strlen(pp) + 1;
    }
}


static void lstFunction(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, EjsFunction *fun, 
        int attributes)
{
    Ejs         *ejs;
    EjsTrait    *trait;
    EjsName     lname;
    EjsType     *resultType;
    EjsPot      *activation;
    EjsString   *space, *blockName;
    int         i, numLocals, numProp;

    ejs = mp->ejs;
    activation = fun->activation;
    numProp = activation ? activation->numProp : 0;
    space = mapSpace(ejs, qname.space);

    mprFprintf(mp->file,  "\nFUNCTION:   ");

    /*
        Do the function declaration
     */
    if (attributes) {
        if (slotNum < 0) {
            /* Special just for global initializers */
            mprFprintf(mp->file,  "[initializer]  %@ %sfunction %@(", space, getAttributeString(mp, attributes), 
                qname.name);
        } else {
            blockName = getBlockName(mp, block, slotNum);
            mprFprintf(mp->file,  "[%@-%02d]  %@ %sfunction %@(", blockName, slotNum, space,
                getAttributeString(mp, attributes), qname.name);
        }
    } else {
        blockName = getBlockName(mp, block, slotNum);
        mprFprintf(mp->file,  "[%@-%02d]  %@ function %@(", blockName, slotNum, space, qname.name);
    }

    for (i = 0; i < (int) fun->numArgs; ) {
        lname = ejsGetPropertyName(ejs, activation, i);
        trait = ejsGetPropertyTraits(ejs, activation, i);
        if (trait->type) {
            mprFprintf(mp->file,  "%@: %@", lname.name, trait->type->qname.name);
        } else {
            mprFprintf(mp->file,  "%@", lname.name);
        }
        if (++i < (int) fun->numArgs) {
            mprFprintf(mp->file,  ", ");
        }
    }

    resultType = fun->resultType;
    mprFprintf(mp->file,  ") : %@\n", resultType ? resultType->qname.name : ST(Void)->qname.name);

    /*
        Repeat the args
     */
    for (i = 0; i < (int) fun->numArgs; i++) {
        lname = ejsGetPropertyName(ejs, activation, i);
        trait = ejsGetPropertyTraits(ejs, activation, i);
        mprFprintf(mp->file,  "     ARG:   [arg-%02d]   %@ %@", i, lname.space, lname.name);
        if (trait->type) {
            mprFprintf(mp->file,  " : %@", trait->type->qname.name);
        }
        mprFprintf(mp->file,  "\n");
    }
    numLocals = numProp - fun->numArgs;
    for (i = 0; i < numLocals; i++) {
        lname = ejsGetPropertyName(ejs, activation, i + fun->numArgs);
        trait = ejsGetPropertyTraits(ejs, activation, i + fun->numArgs);
        mprFprintf(mp->file,  "   LOCAL:   [local-%02d]  var %@", i + fun->numArgs, lname.name);
        if (trait->type) {
            mprFprintf(mp->file,  " : %@", trait->type->qname.name);
        }
        mprFprintf(mp->file,  "\n");
    }
    if (fun->body.code) {
        mprFprintf(mp->file,  "\n");
        interp(mp, module, fun);
    }
    mprFprintf(mp->file,  "\n");
}


void lstException(EjsMod *mp, EjsModule *module, EjsFunction *fun)
{
    Ejs             *ejs;
    EjsEx           *ex;
    EjsCode         *code;
    cchar           *exKind;
    int             i;

    ejs = mp->ejs;
    code = fun->body.code;

    if (code->numHandlers <= 0) {
        return;
    }
    mprFprintf(mp->file,
        "\n"
        "#\n"
        "#  Exception Section\n"
        "#    Kind     TryStart TryEnd  HandlerStart  HandlerEnd   CatchType\n"
        "#\n");

    for (i = 0; i < code->numHandlers; i++) {
        ex = code->handlers[i];

        if (ex->flags & EJS_EX_FINALLY) {
            exKind  = "finally";
        } else if (ex->flags & EJS_EX_ITERATION) {
            exKind  = "iteration";
        } else if (ex->flags & EJS_EX_CATCH) {
            exKind = "catch";
        } else {
            exKind = "unknown";
        }
        mprFprintf(mp->file,
            "%-3d %-10s %5d   %5d      %5d        %5d       %@\n",
            i, exKind, ex->tryStart, ex->tryEnd, ex->handlerStart, ex->handlerEnd,
            ex->catchType ? ex->catchType->qname.name : S(empty));
    }
    mprFprintf(mp->file, "\n");
}


static void lstProperty(EjsMod *mp, EjsModule *module, EjsObj *block, int slotNum, EjsName qname, int attributes, 
        EjsName typeName)
{
    Ejs         *ejs;
    EjsType     *propType;
    EjsString   *space, *blockName;

    ejs = mp->ejs;
    space = mapSpace(ejs, qname.space);
    mprFprintf(mp->file, "VARIABLE:   ");

    blockName = getBlockName(mp, block, slotNum);
    mprFprintf(mp->file, "[%@-%02d]  %@ %svar %@", blockName, slotNum, space,
        getAttributeString(mp, attributes), qname.name);

    if (typeName.name && typeName.name->value[0]) {
        mprFprintf(mp->file, " : %@", typeName.name);
    }
    mprFprintf(mp->file, "\n");

    if (block == 0) {
        /*
            Nested block.
         */
        if (typeName.name) {
            propType = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, typeName);
        } else {
            propType = 0;
        }
        mprAssert(mp->currentBlock && ejsIsBlock(ejs, mp->currentBlock));
        slotNum = ejsDefineProperty(ejs, (EjsObj*) mp->currentBlock, -1, qname, propType, attributes, 0);
    }
}


static void lstModule(EjsMod *mp, EjsModule *module)
{
    mprFprintf(mp->file,
        "\n==============================================================================================\n\n"
        "MODULE:   %@", module->vname);

    if (module->hasInitializer) {
        mprFprintf(mp->file, " <%s>\n", module->hasInitializer ? "hasInitializer, " : "");
    }
    mprFprintf(mp->file, "\n");
}


static int decodeOperands(EjsMod *mp, EjsOptable *opt, char *argbuf, int argbufLen, int address, int *stackEffect)
{
    EjsString   *sval;
    int         *argp;
    char        *bufp;
    uchar       *start;
    double      dval;
    int         i, argc, ival, len, buflen, j, numEntries, att;

    *stackEffect = opt->stackEffect;

    /*
        Keep a local progressive pointer into the argbuf and a length of the remaining room in the buffer.
     */
    *argbuf = '\0';
    bufp = argbuf;
    buflen = argbufLen;

    for (argc = 0, argp = opt->args; *argp; argc++, argp++) ;

    start = mp->pc;
    ival = 0;

    for (i = 0, argp = opt->args; i < argc; i++) {
        switch (opt->args[i]) {
        case EBC_NONE:
            break;

        case EBC_BYTE:
            ival = getByte(mp);
            mprSprintf(bufp, buflen,  "<%d> ", ival);
            break;

        case EBC_DOUBLE:
            dval = getDouble(mp);
            mprSprintf(bufp, buflen,  "<%f> ", dval);
            break;

        case EBC_ARGC:
        case EBC_ARGC2:
            ival = (int) getNum(mp);
            mprSprintf(bufp, buflen,  "<argc: %d> ", ival);
            break;

        case EBC_ARGC3:
            ival = (int) getNum(mp);
            mprSprintf(bufp, buflen,  "<argc: %d> ", ival);
            break;

        case EBC_NEW_ARRAY:
            ival = (int) getNum(mp);     /* argc */
            mprSprintf(bufp, buflen,  "<argc: %d>", ival);
            bufp += strlen(bufp);
            *stackEffect -= (ival * 2);
            break;

        case EBC_NEW_OBJECT:
            ival = (int) getNum(mp);     /* argc */
            mprSprintf(bufp, buflen,  "<argc: %d> <att: ", ival);
            bufp += strlen(bufp);
            for (j = 0; j < ival; j++) {
                att = (int) getNum(mp);
                mprSprintf(bufp, buflen,  "%d ", ival);
                len = (int) strlen(bufp);
                bufp += len;
                buflen -= len;
            }
            mprSprintf(bufp, buflen,  ">", ival);
            *stackEffect -= (ival * 3);
            break;

        case EBC_SLOT:
            ival = (int) getNum(mp);
            mprSprintf(bufp, buflen,  "<slot: %d> ", ival);
            break;

        case EBC_NUM:
            ival = (int) getNum(mp);
            mprSprintf(bufp, buflen,  "<%d> ", ival);
            break;

        case EBC_JMP8:
            ival = getByte(mp);
            mprSprintf(bufp, buflen,  "<addr: %d> ", ((char) ival) + address + 1);
            break;

        case EBC_JMP:
            ival = getInt32(mp);
            mprSprintf(bufp, buflen,  "<addr: %d> ", ival + address + 4);
            break;

        case EBC_INIT_DEFAULT8:
            numEntries = getByte(mp);
            mprSprintf(bufp, buflen,  "<%d> ", numEntries);
            len = (int) strlen(bufp);
            bufp += len;
            buflen -= len;
            for (j = 0; j < numEntries; j++) {
                ival = getByte(mp);
                mprSprintf(bufp, buflen,  "<%d> ", ival + 2);
                len = (int) strlen(bufp);
                bufp += len;
                buflen -= len;
            }
            break;

        case EBC_INIT_DEFAULT:
            numEntries = getByte(mp);
            mprSprintf(bufp, buflen,  "<%d> ", numEntries);
            len = (int) strlen(bufp);
            bufp += len;
            buflen -= len;
            for (j = 0; j < numEntries; j++) {
                ival = getInt32(mp);
                mprSprintf(bufp, buflen,  "<%d> ", ival + 2);
                len = (int) strlen(bufp);
                bufp += len;
                buflen -= len;
            }
            break;

        case EBC_STRING:
            sval = getString(mp->ejs, mp);
            mprAssert(sval);
            mprSprintf(bufp, buflen,  "<%@> ", sval);
            break;

        case EBC_GLOBAL:
            getGlobal(mp, bufp, buflen);
            break;

        default:
            mprError("Bad arg type in opcode table");
            break;
        }
        len = (int) strlen(bufp);
        bufp += len;
        buflen -= len;

        if (opt->args[i] == EBC_ARGC) {
            *stackEffect -= ival;
        } else if (opt->args[i] == EBC_ARGC2) {
            *stackEffect -= (ival * 2);
        } else if (opt->args[i] == EBC_ARGC3) {
            *stackEffect -= (ival * 3);
        }
        if (i == 0 && opt->stackEffect == EBC_POPN) {
            *stackEffect = -ival;
        }
    }
    return (int) (mp->pc - start);
}


/*
    Interpret the code for a function
 */
static void interp(EjsMod *mp, EjsModule *module, EjsFunction *fun)
{
    EjsOptable  *optable, *opt;
    EjsCode     *code;
    MprChar     *currentLine;
    uchar       *start;
    char        argbuf[MPR_MAX_STRING], lineInfo[MPR_MAX_STRING], name[MPR_MAX_STRING];
    char        *currentFile, *src, *dest;
    int         maxOp, opcode, lineNumber, stack, codeLen, address, stackEffect, nbytes, i, lastLine;

    mprAssert(mp);
    mprAssert(module);
    mprAssert(fun);

    /*
        Store so that getNum and getString can easily read instructions
     */
    code = fun->body.code;
    mp->fun = fun;
    mp->module = module;
    mp->pc = code->byteCode;
    codeLen = code->codeLen;
    start = mp->pc;
    stack = 0;
    currentLine = 0;
    lineNumber = 0;
    currentFile = 0;
    lastLine = -1;

    optable = ejsGetOptable(mp);

    for (maxOp = 0, opt = optable; opt->name; opt++) {
        maxOp++;
    }
    
    while ((mp->pc - start) < codeLen) {
        address = (int) (mp->pc - start);
        opcode = *mp->pc++;
        argbuf[0] = '\0';
        stackEffect = 0;

        if (opcode < 0 || opcode >= maxOp) {
            mprError("Bad opcode %x at address %d.\n", opcode, address);
            return;
        }
        opt = &optable[opcode];

        if (mp->showAsm) {
            /*
                Output address [stack] opcode
                Format:  "address: [stackDepth] opcode <args> ..."
             */
            mprFprintf(mp->file,  "    %04d: [%d] %02x ", address, stack, opcode);
        }
        nbytes = decodeOperands(mp, opt, argbuf, (int) sizeof(argbuf), (int) (mp->pc - start), &stackEffect);

        if (mp->showAsm) {
            for (i = 24 - (nbytes * 3); i >= 0; i--) {
                mprFprintf(mp->file, ".");
            }
            for (dest = name, src = opt->name; *src; src++, dest++) {
                if (*src == '_') {
                    *dest = *++src;
                } else {
                    *dest = tolower((int) *src);
                }
            }
            *dest++ = '\0';
            mprFprintf(mp->file,  " %s %s\n", name, argbuf);

        } else {
            for (i = 24 - (nbytes * 3); i >= 0; i--) {
                mprFprintf(mp->file, " ");
            }
            mprFprintf(mp->file,  " %s\n", argbuf);
        }
        stack += stackEffect;

        if (opcode == EJS_OP_RETURN_VALUE || opcode == EJS_OP_RETURN) {
            stack = 0;
        }
        if (stack < 0) {
            if (mp->warnOnError) {
                mprPrintfError("Instruction stack is negative %d\n", stack);
            }
            if (mp->exitOnError) {
                exit(255);
            }
        }
        ejsGetDebugInfo(mp->ejs, fun, mp->pc, &currentFile, &lineNumber, &currentLine);
        if (currentFile && currentLine && *currentLine && lineNumber != lastLine) {
            mprSprintf(lineInfo, sizeof(lineInfo), "%s:%d", currentFile, lineNumber);
            mprFprintf(mp->file, "\n    # %-25s %w\n\n", lineInfo, currentLine);
            lastLine = lineNumber;
        }
    }
}


static void lstVarSlot(EjsMod *mp, EjsModule *module, EjsName *qname, EjsTrait *trait, int slotNum)
{
    Ejs         *ejs;
    EjsString   *space;

    mprAssert(slotNum >= 0);
    mprAssert(qname);

    ejs = mp->ejs;
    space = mapSpace(ejs, qname->space);

    if (qname->name == 0 || qname->name->value[0] == '\0') {
        mprFprintf(mp->file, "%04d    <inherited>\n", slotNum);

    } else if (trait && trait->type) {
        if (trait->type == ST(Function)) {
            mprFprintf(mp->file, "%04d    %@ function %@\n", slotNum, space, qname->name);

        } else {
            mprFprintf(mp->file, "%04d    %@ var %@: %@\n", slotNum, space, qname->name, trait->type->qname.name);
        }

    } else {
        mprFprintf(mp->file, "%04d    %@ var %@\n", slotNum, space, qname->name);
    }
}


/*
    List the various property slot assignments
 */
static void lstSlotAssignments(EjsMod *mp, EjsModule *module, EjsObj *parent, int slotNum, EjsObj *obj)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsType         *type;
    EjsObj          *vp;
    EjsPot          *prototype;
    EjsFunction     *fun;
    EjsBlock        *block;
    EjsName         qname;
    int             i, numInherited, count;

    mprAssert(obj);
    mprAssert(module);

    ejs = mp->ejs;
    if (VISITED(obj)) {
        return;
    }
    SET_VISITED(obj, 1);

    if (obj == ejs->global) {
        mprFprintf(mp->file,  "\n#\n"
            "#  Global slot assignments (Num prop %d)\n"
            "#\n", ejsGetPropertyCount(ejs, obj));

        /*
            List slots for global
         */
        for (i = module->firstGlobal; i < module->lastGlobal; i++) {
            trait = ejsGetPropertyTraits(ejs, ejs->global, i);
            qname = ejsGetPropertyName(ejs, ejs->global, i);
            if (qname.name == 0) {
                continue;
            }
            lstVarSlot(mp, module, &qname, trait, i);
        }

        /*
            List slots for the initializer
         */
        fun = (EjsFunction*) module->initializer;
        if (fun) {
            mprFprintf(mp->file,  "\n#\n"
                "#  Initializer slot assignments (Num prop %d)\n"
                "#\n", ejsGetPropertyCount(ejs, (EjsObj*) fun));

            count = ejsGetPropertyCount(ejs, (EjsObj*) fun);
            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, (EjsObj*) fun, i);
                qname = ejsGetPropertyName(ejs, (EjsObj*) fun, i);
                if (qname.name == 0) {
                    continue;
                }
                mprAssert(trait);
                lstVarSlot(mp, module, &qname, trait, i);
            }
        }

    } else if (ejsIsFunction(ejs, obj)) {
        fun = (EjsFunction*) obj;
        count = ejsGetPropertyCount(ejs, (EjsObj*) obj);
        if (count > 0) {
            mprFprintf(mp->file,  "\n#\n"
                "#  Local slot assignments for the \"%@\" function (Num slots %d)\n"
                "#\n", fun->name, count);

            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, obj, i);
                mprAssert(trait);
                qname = ejsGetPropertyName(ejs, obj, i);
                lstVarSlot(mp, module, &qname, trait, i);
            }
        }

    } else if (ejsIsType(ejs, obj)) {
        /*
            Types
         */
        type = (EjsType*) obj;
        mprFprintf(mp->file,  "\n#\n"
            "#  Class slot assignments for the \"%@\" class (Num slots %d)\n"
            "#\n", type->qname.name,
            ejsGetPropertyCount(ejs, (EjsObj*) type));

        count = ejsGetPropertyCount(ejs, (EjsObj*) type);
        for (i = 0; i < count; i++) {
            trait = ejsGetPropertyTraits(ejs, (EjsObj*) type, i);
            mprAssert(trait);
            qname = ejsGetPropertyName(ejs, obj, i);
            lstVarSlot(mp, module, &qname, trait, i);
        }

        prototype = type->prototype;
        if (type->baseType && type->baseType->prototype) {
            numInherited = ejsGetPropertyCount(ejs, type->baseType->prototype);
        } else {
            numInherited = 0;
        }
        mprFprintf(mp->file,  "\n#\n"
            "#  Instance slot assignments for the \"%@\" class (Num prop %d, num inherited %d)\n"
            "#\n", type->qname.name,
            prototype ? ejsGetPropertyCount(ejs, prototype): 0, numInherited);

        if (prototype) {
            count = ejsGetPropertyCount(ejs, prototype);
            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, prototype, i);
                mprAssert(trait);
                qname = ejsGetPropertyName(ejs, prototype, i);
                if (qname.name) {
                    lstVarSlot(mp, module, &qname, trait, i);
                }
            }
        }

    } else if (ejsIsBlock(ejs, obj)) {
        qname = ejsGetPropertyName(ejs, parent, slotNum);
        block = (EjsBlock*) obj;
        count = ejsGetPropertyCount(ejs, (EjsObj*) block);
        if (count > 0) {
            mprFprintf(mp->file,  
                "\n#\n"
                "#  Block slot assignments for the \"%@\" (Num slots %d)\n"
                "#\n", qname.name, ejsGetPropertyCount(ejs, obj));
            
            count = ejsGetPropertyCount(ejs, obj);
            for (i = 0; i < count; i++) {
                trait = ejsGetPropertyTraits(ejs, obj, i);
                mprAssert(trait);
                qname = ejsGetPropertyName(ejs, obj, i);
                lstVarSlot(mp, module, &qname, trait, i);
            }
        }
    }

    /*
        Now recurse on types, functions and blocks
     */
    if (obj == ejs->global) {
        i = module->firstGlobal;
        count = module->lastGlobal;
    } else {
        i = 0;
        count = ejsGetPropertyCount(ejs, obj);
    }
    for (; i < count; i++) {
        qname = ejsGetPropertyName(ejs, obj, i);
        vp = ejsGetProperty(ejs, obj, i);
        if (vp == 0) {
            continue;
        }
        if (ejsIsType(ejs, vp) || ejsIsFunction(ejs, vp) || ejsIsBlock(ejs, vp)) {
            lstSlotAssignments(mp, module, obj, i, vp);
        }
    }
    SET_VISITED(obj, 0);
}


static EjsString *getBlockName(EjsMod *mp, EjsObj *block, int slotNum)
{
    EjsName         qname;

    if (block) {
        if (ejsIsType(mp->ejs, block)) {
            return ((EjsType*) block)->qname.name;

        } else if (ejsIsFunction(mp->ejs, block)) {
            return ((EjsFunction*) block)->name;
        }
    }
    qname = ejsGetPropertyName(mp->ejs, block, slotNum);
    return qname.name;
}


static char *getAttributeString(EjsMod *mp, int attributes)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    /*
        Order to look best
     */
    if (attributes & EJS_PROP_NATIVE) {
        strcat(attributeBuf, "native ");
    }
    if (attributes & EJS_TRAIT_READONLY) {
        strcat(attributeBuf, "const ");
    }
    if (attributes & EJS_PROP_STATIC) {
        strcat(attributeBuf, "static ");
    }
    if (attributes & EJS_TYPE_FINAL) {
        strcat(attributeBuf, "final ");
    }
    if (attributes & EJS_FUN_OVERRIDE) {
        strcat(attributeBuf, "override ");
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCE) {
        strcat(attributeBuf, "dynamic ");
    }
    if (attributes & EJS_TRAIT_GETTER) {
        strcat(attributeBuf, "get ");
    }
    if (attributes & EJS_TRAIT_SETTER) {
        strcat(attributeBuf, "set ");
    }
    return attributeBuf;
}


static uchar getByte(EjsMod *mp)
{
    if (mp->showAsm) {
        mprFprintf(mp->file, "%02x ",  mp->pc[0] & 0xFF);
    }
    return *mp->pc++ & 0xFF;
}


static uint getInt32(EjsMod *mp)
{
    uchar   *start;
    uint    value;

    start = mp->pc;
    value = ejsDecodeInt32(mp->ejs, &mp->pc);

    if (mp->showAsm) {
        for (; start < mp->pc; start++) {
            mprFprintf(mp->file, "%02x ", *start & 0xff);
        }
    }
    return value;
}


static double getDouble(EjsMod *mp)
{
    uchar   *start;
    double  value;

    start = mp->pc;
    value = ejsDecodeDouble(mp->ejs, &mp->pc);

    if (mp->showAsm) {
        for (; start < mp->pc; start++) {
            mprFprintf(mp->file, "%02x ", *start & 0xff);
        }
    }
    return value;
}


/*
    Get an encoded number
 */
static int64 getNum(EjsMod *mp)
{
    uchar       *start;
    int64       value;

    start = mp->pc;
    value = ejsDecodeNum(mp->ejs, &mp->pc);

    if (mp->showAsm) {
        for (; start < mp->pc; start++) {
            mprFprintf(mp->file, "%02x ", *start & 0xff);
        }
    }
    return value;
}


/*
    Read an interned string constants are stored as token offsets into the constant pool. The pool contains null
    terminated UTF-8 strings.
 */
static EjsString *getString(Ejs *ejs, EjsMod *mp)
{
    int         number;

    number = (int) getNum(mp);
    if (number < 0) {
        mprAssert(number >= 0);
        return 0;
    }
    return ejsCreateStringFromConst(ejs, mp->module, number);
}


/*
    Return the length of bytes added to buf
 */
static void getGlobal(EjsMod *mp, char *buf, int buflen)
{
    Ejs             *ejs;
    EjsName         qname;
    EjsObj          *vp;
    int             t, slotNum;

    ejs = mp->ejs;
    vp = 0;

    if ((t = (int) getNum(mp)) < 0) {
        mprSprintf(buf, buflen,  "<can't read code>");
        return;
    }

    switch (t & EJS_ENCODE_GLOBAL_MASK) {
    default:
        mprAssert(0);
        return;

    case EJS_ENCODE_GLOBAL_NOREF:
        return;

    case EJS_ENCODE_GLOBAL_SLOT:
        /*
            Type is a builtin primitive type or we are binding globals.
         */
        slotNum = t >> 2;
        if (0 <= slotNum && slotNum < ejsGetPropertyCount(ejs, ejs->global)) {
            vp = ejsGetProperty(ejs, ejs->global, slotNum);
        }
        if (vp && ejsIsType(ejs, vp)) {
            mprSprintf(buf, buflen, "<type: 0x%x,  %N> ", t, ((EjsType*) vp)->qname);
        }
        break;

    case EJS_ENCODE_GLOBAL_NAME:
        /*
            Type was unknown at compile time
         */
        qname.name = ejsCreateStringFromConst(ejs, mp->module, t >> 2);
        if (qname.name == 0) {
            mprAssert(0);
            mprSprintf(buf, buflen,  "<var: 0x%x,  missing name> ", t);
            return;
        }
        if ((qname.space = getString(mp->ejs, mp)) == 0) {
            mprSprintf(buf, buflen,  "<var: 0x%x,  missing namespace> ", t);
            return;
        }
        if (qname.name) {
            vp = ejsGetPropertyByName(ejs, ejs->global, qname);
        }
        mprSprintf(buf, buflen, "<var: 0x%x,  %N> ", t, qname);
        break;
    }

    if (vp == 0) {
        mprSprintf(buf, buflen, "<var: %d,  cannot resolve var/typ at slot e> ", t);
    }
}


static void leadin(EjsMod *mp, EjsModule *module, int classDec, int inFunction)
{
    mprFprintf(mp->file, "    ");
}


static EjsString *mapSpace(Ejs *ejs, EjsString *space)
{
    if (ejsContainsMulti(ejs, space, "internal-") != 0) {
        return ejsCreateStringFromAsc(ejs, "internal");
    }
    return space;
}


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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/listing.c"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/cmd/slotGen.c"
 */
/************************************************************************/

/**
    slotGen.c - Generate property slot offset definitions.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */




static char *defaultVersion;


static int  createSlotFile(EjsMod *bp, EjsModule *mp, MprFile *file);
static int  genType(EjsMod *bp, MprFile *file, EjsModule *mp, EjsType *type, int firstClassSlot, int lastClassSlot,
                int isGlobal);
static char *mapFullName(Ejs *ejs, EjsName *qname, int mapTypeName);
static char *mapName(cchar *name, int mapTypeName);
static char *mapNamespace(cchar *space);


int emCreateSlotFiles(EjsMod *bp, EjsModule *mp, MprFile *outfile)
{
    int     rc;

    rc = 0;
    defaultVersion = mprAsprintf("-%d", ejsParseModuleVersion(BLD_VERSION));
    if (bp->cslots) {
        rc += createSlotFile(bp, mp, outfile);
    }
    return rc;
}


static int createSlotFile(EjsMod *bp, EjsModule *mp, MprFile *file)
{
    Ejs         *ejs;
    EjsType     *type;
    MprFile     *localFile;
    char        *path, slotsName[MPR_MAX_FNAME], moduleName[MPR_MAX_FNAME];
    char        *cp, *sp, *dp;
    int         slotNum;

    mprAssert(bp);

    path = 0;
    localFile = 0;
    ejs = bp->ejs;

    scopy(moduleName, sizeof(moduleName), ejsToMulti(ejs, mp->name));
    for (cp = moduleName; *cp; cp++)  {
        if (*cp == '.') {
            *cp = '_';
        }
    }
    mprSprintf(slotsName, sizeof(slotsName), "%@Slots", mp->name);
    slotsName[0] = toupper((int) slotsName[0]);
    for (dp = sp = slotsName; *sp; sp++) {
        if (*sp == '.') {
            ++sp;
            *dp++ = toupper((int) *sp);
        } else {
            *dp++ = *sp;
        }
    }
    *dp = '\0';

    if (file == 0) {
        path = sjoin(ejsToMulti(ejs, mp->name), ".slots.h", NULL);
        localFile = file = mprOpenFile(path, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664);
    } else {
        path = sclone(file->path);
    }
    if (file == 0) {
        mprError("Can't open %s", path);
        return MPR_ERR_CANT_OPEN;
    }
    mprEnableFileBuffering(file, 0, 0);

    mprFprintf(file,
        "/*\n"
        "   %s -- Native property slot definitions for the \"%@\" module\n"
        "  \n"
        "   This file is generated by ejsmod\n"
        "  \n"
        "   Slot definitions. Version %s.\n"
        " */\n"
        "\n", path, mp->name, BLD_VERSION);

    mprFprintf(file,
        "#ifndef _h_SLOTS_%s\n"
        "#define _h_SLOTS_%s 1\n\n",
        slotsName, slotsName);

    mprFprintf(file, "\n/*\n   Slots for the \"%@\" module \n */\n", mp->name);

    slotNum = ejsGetPropertyCount(ejs, ejs->global);
    type = ejsCreateType(ejs, N(EJS_EJS_NAMESPACE, EJS_GLOBAL), NULL, NULL, NULL, sizeof(EjsType), -1, 
        ejsGetPropertyCount(ejs, ejs->global), 0, 0);
    type->constructor.block = *(EjsBlock*) ejs->global;
    SET_TYPE(type, ST(Type));
    type->constructor.block.pot.isType = 1;

    if (genType(bp, file, mp, type, mp->firstGlobal, mp->lastGlobal, 1) < 0) {
        mprError("Can't generate slot file for module %@", mp->name);
        mprCloseFile(localFile);
        return EJS_ERR;
    }
    mprFprintf(file, "\n#define _ES_CHECKSUM_%s   %d\n", moduleName, mp->checksum);
    mprFprintf(file, "\n#endif\n");
    mprCloseFile(localFile);
    return 0;
}


static void defineSlot(EjsMod *bp, MprFile *file, EjsModule *mp, EjsType *type, EjsObj *obj, int slotNum, EjsName *funName, 
        EjsName *name)
{
    Ejs     *ejs;
    char    nameBuf[MPR_MAX_STRING];
    char    *funSep, *sp, *typeStr, *funStr, *nameStr;
    ejs = bp->ejs;

    typeStr = mapFullName(ejs, &type->qname, 1);
#if UNUSED
    if (slotNum < type->numInherited && obj == type->prototype) {
        subStr = mapFullName(ejs, &type->baseType->qname, 1);
        subSep = "_";
    } else {
        subSep = subStr = "";
    }
#endif
    funStr = mapFullName(ejs, funName, 0);
    nameStr = mapFullName(ejs, name, 0);

    if (nameStr[0] != '\0') {
        funSep = (char*) ((*funStr && *typeStr) ? "_" : "");
        if (*typeStr == '\0' && *funStr == '\0') {
            mprSprintf(nameBuf, sizeof(nameBuf), "#define ES_%s", nameStr);
        } else {
            if (!(nameStr[0] == '_' && nameStr[1] == '_')) {
                mprSprintf(nameBuf, sizeof(nameBuf), "#define ES_%s%s%s_%s", typeStr, funSep, funStr, nameStr);
            } else {
                nameBuf[0] = '\0';
            }
        }
        for (sp = nameBuf; *sp; sp++) {
            if (*sp == '.') {
                *sp = '_';
            }
        }
        if (nameBuf[0]) {
            mprFprintf(file, "%-70s %d\n", nameBuf, slotNum);
        }
    }
}


static void defineSlotCount(EjsMod *bp, MprFile *file, EjsModule *mp, EjsType *type, char *suffix, int numProp)
{
    char        name[MPR_MAX_STRING], *typeStr, *sp;

    typeStr = mapFullName(bp->ejs, &type->qname, 1);
    if (*typeStr == '\0') {
        typeStr = sclone(EJS_GLOBAL);
    }
    for (sp = typeStr; *sp; sp++) {
        if (*sp == '.') {
            *sp = '_';
        }
    }
    mprSprintf(name, sizeof(name), "#define ES_%s_NUM_%s_PROP", typeStr, suffix);
    mprFprintf(file, "%-70s %d\n", name, numProp);
    if (strcmp(suffix, "INSTANCE") == 0) {
        mprSprintf(name, sizeof(name), "#define ES_%s_NUM_INHERITED_PROP", typeStr);
        mprFprintf(file, "%-70s %d\n", name, type->numInherited);
    }
}


/*
    Generate the slot offsets for a type
 */
static int genType(EjsMod *bp, MprFile *file, EjsModule *mp, EjsType *type, int firstClassSlot, int lastClassSlot,
    int isGlobal)
{
    Ejs             *ejs;
    EjsPot          *prototype, *activation;
    EjsObj          *vp;
    EjsTrait        *trait;
    EjsType         *nt;
    EjsFunction     *fun;
    EjsName         qname, lqname;
    int             slotNum, i, methodHeader, count, offset;

    mprAssert(bp);
    mprAssert(type);
    mprAssert(ejsIsType(ejs, type));

    ejs = bp->ejs;
    lastClassSlot = max(firstClassSlot, lastClassSlot);

    if (!isGlobal || ejsCompareMulti(ejs, mp->name, "ejs") == 0) {
        /*
            Only emit global property slots for "ejs"
         */
        if (isGlobal) {
            mprFprintf(file, "\n\n/*\n    Global property slots\n */\n");
        } else {
            mprFprintf(file, "\n\n/*\n    Class property slots for the \"%@\" type \n */\n", type->qname.name);
        }
        if (firstClassSlot < lastClassSlot) {
            for (slotNum = firstClassSlot; slotNum < lastClassSlot; slotNum++) {
                qname = ejsGetPropertyName(ejs, (EjsObj*) type, slotNum);
                if (qname.name == 0) {
                    continue;
                }
                defineSlot(bp, file, mp, type, (EjsObj*) type, slotNum, NULL, &qname);
            }
        }
        defineSlotCount(bp, file, mp, type, "CLASS", lastClassSlot);
    }

    /*
        Process instance traits
     */
    prototype = type->prototype;
    if (prototype) {
        mprFprintf(file, "\n/*\n   Prototype (instance) slots for \"%@\" type \n */\n", type->qname.name);
        count = ejsGetPropertyCount(ejs, prototype);
        offset = 0;
        for (slotNum = offset; slotNum < count; slotNum++) {
            qname = ejsGetPropertyName(ejs, prototype, slotNum);
            if (qname.name == 0) {
                continue;
            }
            defineSlot(bp, file, mp, type, (EjsObj*) prototype, slotNum, NULL, &qname);
        }
    } else {
        slotNum = 0;
    }

    /*
        For the global type, only emit the count for the "ejs" module
     */
    if (!isGlobal || ejsCompareMulti(ejs, mp->name, "ejs") == 0) {
        defineSlotCount(bp, file, mp, type, "INSTANCE", slotNum);
    }

    /*
        Now examine methods in the type and define any locals and args
     */
    methodHeader = 0;
    for (slotNum = firstClassSlot; slotNum < lastClassSlot; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, (EjsObj*) type, slotNum);
        qname = ejsGetPropertyName(ejs, (EjsObj*) type, slotNum);
        if (trait == 0 || qname.name == 0) {
            continue;
        }
        if (trait->type != ST(Function)) {
            continue;
        }
        vp = ejsGetProperty(ejs, (EjsObj*) type, slotNum);
        if (vp == 0 || !ejsIsFunction(ejs, vp)) {
            continue;
        }
        fun = ((EjsFunction*) vp);
        activation = fun->activation;
        if (activation == 0) {
            continue;
        }
        if (!methodHeader) {
            if (isGlobal) {
                mprFprintf(file, "\n/*\n    Local slots for global methods \n */\n");
            } else {
                mprFprintf(file, "\n/*\n    Local slots for methods in type \"%@\" \n */\n", type->qname.name);
            }
            methodHeader++;
        }
        /*
            Output the arg names and local variable names.
         */
        for (i = 0; i < (int) fun->numArgs; i++) {
            lqname = ejsGetPropertyName(ejs, activation, i);
            defineSlot(bp, file, mp, type, (EjsObj*) activation, i, &qname, &lqname);
        }
        for (; i < fun->block.pot.numProp; i++) {
            lqname = ejsGetPropertyName(ejs, activation, i);
            defineSlot(bp, file, mp, type, (EjsObj*) activation, i, &qname, &lqname);
        }
    }

    /*
        Now process nested types.
     */
    for (slotNum = firstClassSlot; slotNum < lastClassSlot; slotNum++) {
        qname = ejsGetPropertyName(ejs, (EjsObj*) type, slotNum);

        if (qname.name == 0) {
            continue;
        }
        vp = ejsGetProperty(ejs, (EjsObj*) type, slotNum);
        if (vp == 0) {
            continue;
        }
        if (! ejsIsType(ejs, vp) || VISITED(vp)) {
            continue;
        }
        nt = (EjsType*) vp;
        if (nt->module != mp) {
            continue;
        }
        SET_VISITED(vp, 1);

        count = ejsGetPropertyCount(ejs, (EjsObj*) nt);
        if (genType(bp, file, mp, nt, 0, count, 0) < 0) {
            SET_VISITED(vp, 0);
            return EJS_ERR;
        }
        SET_VISITED(vp, 0);
    }
    return 0;
}


static char *mapFullName(Ejs *ejs, EjsName *qname, int mapTypeName)
{
    cchar       *name, *space;
    char        *cp, *buf;

    if (qname == 0) {
        return sclone("");
    }
    name = mapName(ejsToMulti(ejs, qname->name), mapTypeName);
    space = mapNamespace(ejsToMulti(ejs, qname->space));

    if (*space) {
        buf = sjoin(space, "_", name, NULL);
    } else {
        buf = sclone(name);
    }
    for (cp = buf; *cp; cp++)  {
        if (*cp == '-') {
            *cp = '_';
        }
    }
    return buf;
}


static char *mapName(cchar *name, int mapTypeName)
{
    cchar   *value;
    char    *buf, *cp;

    if (name == 0) {
        name = "";
    }

    value = name;
    if (mapTypeName) {
        if (strcmp(name, EJS_GLOBAL) == 0) {
            value = "";
        }
    }
    if (strcmp(name, "*") == 0) {
        value = "DEFAULT";
    }
    if (strcmp(name, "+") == 0) {
        value = "PLUS";
    } else if (strcmp(name, "-") == 0) {
        value = "MINUS";
    } else if (strcmp(name, "/") == 0) {
        value = "DIV";
    } else if (strcmp(name, "*") == 0) {
        value = "MUL";
    } else if (strcmp(name, "%") == 0) {
        value = "MOD";
    } else if (strcmp(name, "[") == 0) {
        value = "LBRACKET";
    } else if (strcmp(name, "&") == 0) {
        value = "AND";
    } else if (strcmp(name, "|") == 0) {
        value = "OR";
    } else if (strcmp(name, "<<") == 0) {
        value = "LSH";
    } else if (strcmp(name, ">>") == 0) {
        value = "RSH";
    } else if (strcmp(name, "<") == 0) {
        value = "LT";
    } else if (strcmp(name, ">") == 0) {
        value = "GT";
    } else if (strcmp(name, "<=") == 0) {
        value = "LE";
    } else if (strcmp(name, ">=") == 0) {
        value = "GE";
    } else if (strcmp(name, "=") == 0) {
        value = "ASSIGN";
    } else if (strcmp(name, "==") == 0) {
        value = "EQ";
    } else if (strcmp(name, "!=") == 0) {
        value = "NE";
    } else if (strcmp(name, "===") == 0) {
        value = "STRICT_EQ";
    } else if (strcmp(name, "!==") == 0) {
        value = "STRICT_NE";
    } else if (strcmp(name, "!") == 0) {
        value = "NOT";
    } else if (strcmp(name, "~") == 0) {
        value = "NEG";
    } else if (strcmp(name, "< ") == 0) {
        value = "LT_BUG";
    } else if (strcmp(name, "= ") == 0) {
        value = "ASSIGN_BUG";
    }

    buf = sclone(value);
    for (cp = buf; *cp; cp++)  {
        if (*cp == '-') {
            *cp = '_';
        }
    }
    return buf;
}


static char *mapNamespace(cchar *space)
{
    char    *cp, *value;

    if (space == 0) {
        space = "";
    }
    if (strcmp(space, EJS_EJS_NAMESPACE) == 0) {
        space = "";
    } else if (strcmp(space, EJS_PUBLIC_NAMESPACE) == 0) {
        space = "";
    } else if (strcmp(space, EJS_PRIVATE_NAMESPACE) == 0) {
        space = "";
#if UNUSED
    } else if (strcmp(space, EJS_ITERATOR_NAMESPACE) == 0) {
        space = "iter";
#endif
    } else if (strcmp(space, EJS_CONSTRUCTOR_NAMESPACE) == 0) {
        space = "";
    } else if (strstr(space, ",private]") != 0) {
        space = "";
    } else if (strstr(space, ",protected]") != 0) {
        space = "";
    } else if (strstr(space, "internal-") != 0) {
        space = "";
    } else if ((cp = strstr(space, defaultVersion)) != 0 && strcmp(cp, defaultVersion) == 0) {
        space = sclone(space);
        cp = strstr(space, defaultVersion);
        *cp = '\0';
    }
    value = (char*) space;
    return value;
}


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
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/slotGen.c"
 */
/************************************************************************/

