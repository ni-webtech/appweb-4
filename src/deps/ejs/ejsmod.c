#include "ejs.h"

/******************************************************************************/
/* 
    This file is an amalgamation of all the individual source code files for
     .
  
    Catenating all the source into a single file makes embedding simpler and
    the resulting application faster, as many compilers can do whole file
    optimization.
  
    If you want to modify , you can still get the whole source
    as individual files if you need.
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
    int         exitOnError;                /* Exit if module file errors are detected */
    int         firstGlobal;                /* First global to examine */
    int         genSlots;                   /* Set if either cslots || jsslots */
    int         listing;                    /* Generate listing file */
    int         showDebug;                  /* Show debug instructions */
    int         verbosity;                  /* Verbosity level */
    int         warnOnError;                /* Warn if module file errors are detected */

    char        *docDir;                    /* Directory to generate HTML doc */
    bool        html;                       /* Generate HTML doc */
    bool        xml;                        /* Generate XML doc */
    
    int         error;                      /* Unresolved error */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         memError;                   /* Memory error */
    
    int         errorCount;                 /* Count of all errors */
    int         warningCount;               /* Count of all warnings */
    int         showAsm;                    /* Display assember bytes */
    
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
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
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
static int       compareClasses(ClassRec **c1, ClassRec **c2);
static int       compareFunctions(FunRec **f1, FunRec **f2);
static int       compareProperties(PropRec **p1, PropRec **p2);
static int       compareNames(char **q1, char **q2);
static EjsDoc    *crackDoc(EjsMod *mp, EjsDoc *doc, EjsName qname);
static MprFile   *createFile(EjsMod *mp, char *name);
static MprKeyValue *createKeyPair(MprChar *key, MprChar *value);
static cchar     *demangle(Ejs *ejs, EjsString *name);
static void      fixupDoc(Ejs *ejs, EjsDoc *doc);
static char      *fmtAccessors(int attributes);
static char      *fmtAttributes(int attributes, int showAccessors);
static char      *fmtClassUrl(Ejs *ejs, EjsName qname);
static char      *fmtDeclaration(Ejs *ejs, EjsName qname);
static char      *fmtNamespace(Ejs *ejs, EjsName qname);
static char      *fmtSpace(Ejs *ejs, EjsName qname);
static char      *fmtType(Ejs *ejs, EjsName qname);
static char      *fmtTypeReference(Ejs *ejs, EjsName qname);
static EjsString *fmtModule(Ejs *ejs, EjsString *name);
static MprChar   *formatExample(Ejs *ejs, EjsString *example);
static int       generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj);
static void      generateClassPage(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static void      generateClassPages(EjsMod *mp);
static void      generateClassPageHeader(EjsMod *mp, EjsObj *obj, EjsName name, EjsTrait *trait, EjsDoc *doc);
static int       generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, int numInhertied);
static int       generateClassGetterTableEntries(EjsMod *mp, EjsObj *obj, int numInherited);
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
static EjsDoc   *getDoc(Ejs *ejs, void *block, int slotNum);
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
        file = mprOpen(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
        if (file == 0) {
            mprError("Can't create %s", path);
            mp->errorCount++;
            mprFree(path);
            return;
        }
        if (mprWrite(file, df->data, df->size) != df->size) {
            mprError("Can't write to buffer");
            mp->errorCount++;
            mprFree(path);
            return;
        }
        mprFree(file);
        mprFree(path);
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

    mprFree(mp->file);
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

    mprFree(mp->file);
    mp->file = 0;
}


static void generateHomeTitle(EjsMod *mp)
{
    mprFree(mp->file);
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

    mprFree(mp->file);
    mp->file = 0;
}


static void generateHomeNavigation(EjsMod *mp)
{
    mprFree(mp->file);
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

    mprFree(mp->file);
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
    namespaces = mprCreateList(mp);

    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        type = ejsGetProperty(ejs, ejs->global, slotNum);
        qname = ejsGetPropertyName(ejs, ejs->global, slotNum);
        if (type == 0 || !ejsIsType(ejs, type) || qname.name == 0 || ejsCompareMulti(ejs, qname.space, "internal-") != 0) {
            continue;
        }
        doc = getDoc(ejs, ejs->global, slotNum);
        if (doc && !doc->hide) {
            addUniqueItem(namespaces, fmtNamespace(ejs, qname));
        }
    }
    mprSortList(namespaces, (MprListCompareProc) compareNames);

    out(mp, "<tr><td><a href='__all-classes.html' target='classes'>All Namespaces</a></td></tr>\n");

    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        out(mp, "<tr><td><a href='%s-classes.html' target='classes'>%s</a></td></tr>\n", namespace, namespace);
    }

    out(mp, "</table>\n");
    out(mp, "</div>\n");

    generateHtmlFooter(mp);
    mprFree(mp->file);
    mp->file = 0;

    /*
        Generate namespace overviews and class list files for each namespace
     */
    for (next = 0; (namespace = (cchar*) mprGetNextItem(namespaces, &next)) != 0; ) {
        generateNamespace(mp, namespace);
    }
    generateNamespace(mp, "__all");
    mprFree(namespaces);
}


static void generateNamespace(EjsMod *mp, cchar *namespace)
{
    Ejs     *ejs;
    char    *path;

    ejs = mp->ejs;

    path = sjoin(namespace, ".html", NULL);
    mp->file = createFile(mp, path);
    mprFree(path);
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
    mprFree(mp->file);
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
    int             next, count;

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
        doc = getDoc(ejs, crec->block ? crec->block : ejs->global, crec->slotNum);
        if (doc && !doc->hide) {
            out(mp, "<td>%s</td></tr>\n", doc->brief);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
    }
    count = mprGetListCount(classes);
    mprFree(classes);
    return count;
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
    classes = mprCreateList(mp);

    count = ejsGetPropertyCount(ejs, ejs->global);
    for (slotNum = 0; slotNum < count; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        if (trait == 0) {
            continue;
        }
        doc = getDoc(ejs, ejs->global, slotNum);
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
    mprSortList(classes, (MprListCompareProc) compareClasses);
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
    mprFree(path);
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
    mprFree(mp->file);
    mp->file = 0;
    mprFree(classes);
}


static void generateOverview(EjsMod *mp)
{
    mprFree(mp->file);
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

    mprFree(mp->file);
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
    mprFree(title);
    
    out(mp, "<body>\n<div class='body'>\n\n");
    out(mp, "<div class=\"content\">\n\n");
}


static void generateTerms(EjsMod *mp)
{
    out(mp,
        "<div class=\"terms\">\n"
        "   <p class=\"terms\">\n"
        "       <a href=\"http://www.embedthis.com/\">"
        "       Embedthis Software LLC, 2003-2010. All rights reserved. "
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

    mprFree(mp->path);
    path = mp->path = mprJoinPath(mp->docDir, name);
    file = mprOpen(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == 0) {
        mprError("Can't open %s", path);
        mp->errorCount++;
        mprFree(path);
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
        mprFree(mp->file);
        mp->file = 0;
        mp->path = mprJoinPath(mp->docDir, getFilename(fmtType(ejs, type->qname)));
        if (mp->path == 0) {
            return;
        }
        trait = ejsGetPropertyTraits(ejs, ejs->global, slotNum);
        doc = getDoc(ejs, ejs->global, slotNum);
        if (doc && !doc->hide) {
            generateClassPage(mp, (EjsObj*) type, qname, trait, doc);
        }
        mprFree(mp->file);
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
    mprAddHash(ejs->doc, key, doc);

    slotNum = ejsGetPropertyCount(ejs, ejs->global);

    qname = N(EJS_EJS_NAMESPACE, EJS_GLOBAL);
    mp->file = createFile(mp, getFilename(fmtType(ejs, qname)));
    if (mp->file == 0) {
        return;
    }
#if UNUSED
    type = ejsCreateType(ejs, &qname, NULL, NULL, NULL, sizeof(EjsType), slotNum, ejs->global->obj.numProp, 
        0, 0, NULL);
    type->constructor.block = *ejs->global;
    type->constructor.block.obj.type = ejs->typeType;
    type->constructor.block.obj.isType = 1;
    type->constructor.block.isGlobal = 1;
#endif

    generateClassPage(mp, ejs->global, qname, trait, doc);

    mprFree(mp->file);
    mp->file = 0;
    mprFree(trait);
    mprFree(doc);
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

    methods = mprCreateList(mp);
    buildMethodList(mp, methods, obj, obj, name);
    if (ejsIsType(ejs, obj)) {
        buildMethodList(mp, methods, (EjsObj*) ((EjsType*) obj)->prototype, obj, name);
    }
    count = generateMethodTable(mp, methods, obj);
    if (count > 0) {
        generateMethodDetail(mp, methods);
    }
    generateContentFooter(mp);
    mprFree(methods);
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
#if UNUSED
    numInherited = type ? type->numInherited : 0;
#endif

    /*
        Loop over all the static properties
     */
    numProp = ejsGetPropertyCount(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        if (trait == 0) {
            continue;
        }
#if UNUSED
        if (slotNum < numInherited) {
            fun = ejsGetProperty(ejs, obj, slotNum);
            if (fun && ejsIsFunction(ejs, fun) && fun->owner != obj) {
                /* Inherited function */
                continue;
            }
        }
#endif
        dp = getDoc(ejs, obj, slotNum);
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
                    doc = getDoc(ejs, prototype, slotNum);
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
        out(mp, "   <tr><td><strong>Definition</strong></td><td>%s class %s</td></tr>\n", 
            fmtAttributes(trait->attributes, 1), qname.name);

        if (type && type->baseType) {
            out(mp, "   <tr><td><strong>Inheritance</strong></td><td>%s", qname.name);
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
        out(mp, "<p class='classDescription'>%s</p>\n\n", doc->description);

        count = mprGetListCount(doc->see);
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
    Ejs     *ejs;
    EjsType *type;
    int     count;

    ejs = mp->ejs;

    out(mp, "<a name='Properties'></a>\n");
    out(mp, "<h2 class='classSection'>Properties</h2>\n");

    out(mp, "<table class='itemTable' summary='properties'>\n");
    out(mp, "   <tr><th>Qualifiers</th><th>Property</th><th>Type</th><th width='95%%'>Description</th></tr>\n");

    count = generateClassPropertyTableEntries(mp, obj, 0);
    count += generateClassGetterTableEntries(mp, obj, 0);

    type = 0;
    if (ejsIsType(ejs, obj)) {
        type = (EjsType*) obj;
        if (type->prototype) {
            count += generateClassPropertyTableEntries(mp, (EjsObj*) type->prototype, type->numInherited);
            count += generateClassGetterTableEntries(mp, (EjsObj*) type->prototype, type->numInherited);
        }
    }
    if (count == 0) {
        out(mp, "   <tr><td colspan='4'>No properties defined</td></tr>");
    }
    out(mp, "</table>\n\n");

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
static MprList *buildPropertyList(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    PropRec         *prec;
    MprList         *list;
    int             start, slotNum, numProp;

    ejs = mp->ejs;

    list = mprCreateList(mp);

    /*
        Loop over all the (non-inherited) properties
     */
    start = numInherited;
    if (obj == ejs->global) {
        start = mp->firstGlobal;
    }
    numProp = ejsGetPropertyCount(ejs, obj);
    for (slotNum = start; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        if (trait) {
            doc = getDoc(ejs, obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (vp == 0 || ejsIsFunction(ejs, vp) || ejsIsType(ejs, vp) || qname.name == 0 || trait == 0) {
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
    mprSortList(list, (MprListCompareProc) compareProperties);
    return list;
}


static MprList *buildGetterList(EjsMod *mp, EjsObj *obj, int numInherited)
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

    list = mprCreateList(mp);

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
        if (!ejsIsFunction(ejs, vp)) {
            continue;
        }
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);
        if (trait) {
            if (!(trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER))) {
                continue;
            }
            doc = getDoc(ejs, obj, slotNum);
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
    mprSortList(list, (MprListCompareProc) compareProperties);
    return list;
}


/*
    Generate the entries for class properties. Will be called once for static properties and once for instance properties
 */
static int generateClassPropertyTableEntries(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsDoc          *doc;
    MprList         *properties;
    PropRec         *prec;
    int             slotNum, count, next, attributes;

    ejs = mp->ejs;
    count = 0;

    properties = buildPropertyList(mp, obj, numInherited);
    type = ejsIsType(ejs, obj) ? (EjsType*) obj : 0;

    for (next = 0; (prec = (PropRec*) mprGetNextItem(properties, &next)) != 0; ) {
        vp = prec->vp;
        trait = prec->trait;
        slotNum = prec->slotNum;
        qname = prec->qname;
        if (ejsStartsWithMulti(ejs, qname.space, "internal") || ejsContainsMulti(ejs, qname.space, "private")) {
            continue;
        }
        if (isalpha((int) qname.name->value[0])) {
            out(mp, "<a name='%s'></a>\n", qname.name);
        }
        attributes = trait->attributes;
        if (type && qname.space == type->qname.space) {
            out(mp, "   <tr><td nowrap align='center'>%s</td><td>%s</td>", 
                fmtAttributes(attributes, 1), qname.name);
        } else {
            out(mp, "   <tr><td nowrap align='center'>%s %s</td><td>%s</td>", fmtNamespace(ejs, qname),
                fmtAttributes(attributes, 1), qname.name);
        }
        if (trait->type) {
            out(mp, "<td>%s</td>", fmtTypeReference(ejs, trait->type->qname));
        } else {
            out(mp, "<td>&nbsp;</td>");
        }
        doc = getDoc(ejs, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}


static int generateClassGetterTableEntries(EjsMod *mp, EjsObj *obj, int numInherited)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsFunction     *fun;
    EjsDoc          *doc;
    MprList         *getters;
    PropRec         *prec;
    MprChar         *name;
    cchar           *set, *tname;
    int             slotNum, count, next;

    ejs = mp->ejs;
    count = 0;
    getters  = buildGetterList(mp, obj, numInherited);

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
#if KEEP
        if (qname.space == type->qname.space)) {
            out(mp, "   <tr><td nowrap align='left'>%s%s</td><td>%s</td>", fmtAttributes(trait->attributes, 1), set, name);
        } else {
#endif
            out(mp, "   <tr><td nowrap align='left'>%s %s%s</td><td>%s</td>", fmtNamespace(ejs, qname),
                fmtAttributes(trait->attributes, 1), set, name);
#if KEEP
        }
#endif
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
        doc = getDoc(ejs, prec->obj, prec->slotNum);
        if (doc) {
            out(mp, "<td>%s %s</td></tr>\n", doc->brief, doc->description);
        } else {
            out(mp, "<td>&nbsp;</td></tr>\n");
        }
        count++;
    }
    return count;
}


static void buildMethodList(EjsMod *mp, MprList *methods, EjsObj *obj, EjsObj *owner, EjsName ownerName)
{
    Ejs             *ejs;
    EjsTrait        *trait;
    EjsName         qname;
    EjsObj          *vp;
    EjsFunction     *fun;
    EjsDoc          *doc;
    FunRec          *fp;
    int             slotNum, numProp, count;

    ejs = mp->ejs;

    if (obj == ejs->global) {
        slotNum = mp->firstGlobal;
    } else {
        slotNum = 0;
    }
    count = 0;
    numProp = ejsGetPropertyCount(ejs, obj);
    for (count = 0; slotNum < numProp; slotNum++) {
        vp = ejsGetProperty(ejs, obj, slotNum);
        trait = ejsGetPropertyTraits(ejs, obj, slotNum);

        fun = (EjsFunction*) vp;
        if (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
            continue;
        }
        if (trait) {
            doc = getDoc(ejs, obj, slotNum);
            if (doc && doc->hide) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
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
#if UNUSED && KEEP
        fun = (EjsFunction*) vp;
        if (slotNum < ((EjsType*) obj)->numInherited) {
            if (fun->owner != obj) {
                /* Inherited function */
                continue;
            }
        }
#endif
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
    mprSortList(methods, (MprListCompareProc) compareFunctions);
}


static int generateMethodTable(EjsMod *mp, MprList *methods, EjsObj *obj)
{
    Ejs             *ejs;
    EjsType         *type;
    EjsTrait        *trait, *argTrait;
    EjsName         qname, argName;
    EjsDoc          *doc;
    EjsFunction     *fun;
    FunRec          *fp;
    cchar           *defaultValue;
    int             i, count, next;

    ejs = mp->ejs;
    type = ejsIsType(ejs, obj) ? ((EjsType*) obj) : 0;

    out(mp, "<a name='Methods'></a>\n");
    out(mp, "<h2 class='classSection'>%S Methods</h2>\n", 
        (type) ? type->qname.name : ejsCreateStringFromAsc(ejs, "Global"));
    out(mp, "<table class='apiIndex' summary='methods'>\n");
    out(mp, "   <tr><th>Qualifiers</th><th width='95%%'>Method</th></tr>\n");

    /*
        Output each method
     */
    count = 0;
    for (next = 0; (fp = (FunRec*) mprGetNextItem(methods, &next)) != 0; ) {
        qname = fp->qname;
        trait = fp->trait;
        fun = fp->fun;

        if (ejsCompareMulti(ejs, qname.space, EJS_INIT_NAMESPACE) == 0) {
            continue;
        }

        if (type && qname.space == type->qname.space) {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s</td>", fmtAttributes(trait->attributes, 1));
        } else {
            out(mp, "   <tr class='apiDef'><td class='apiType'>%s %s</td>", fmtNamespace(ejs, qname), 
                fmtAttributes(trait->attributes, 1));
        }
        out(mp, "<td><a href='#%s'><b>%s</b></a>(", qname.name, demangle(ejs, qname.name));

        doc = getDoc(ejs, fp->obj, fp->slotNum);

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
        out(mp, "   <tr><td colspan='2'>No methods defined</td></tr>\n");
    }
    out(mp, "</table>\n\n");
    if (type && type->baseType) {
        out(mp, "<p class='inheritedLink'><a href='%s#Methods'><i>Inherited Methods</i></a></p>\n\n",
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
                mprError("Missing documentation for parameter \"%S\" in function \"%S\" in type \"%S\"", 
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
    char            *see;
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
    
    doc = getDoc(ejs, obj, slotNum);
    if (doc && doc->hide) {
        return;
    }
    if (doc == 0 || doc->brief == 0) {
        /* One extra to not warn about default constructors */
        if (slotNum >= numInherited + 1) {
            if (mp->warnOnError) {
                mprError("Missing documentation for \"%s.%s\"", fp->ownerName.name, qname.name);
            }
        }
        return;
    }

    if (isalpha((int) qname.name->value[0])) {
        out(mp, "<a name='%s'></a>\n", qname.name);
    }

    if (type && qname.space == type->qname.space) {
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %s(", fmtAttributes(trait->attributes, 1), qname.name);

    } else {
        accessorSep = (trait->attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) ? " ": "";
        spaceSep = qname.space->value[0] ? " ": "";
        out(mp, "<div class='api'>\n");
        out(mp, "<div class='apiSig'>%s %s%s %s%s %s(", fmtAttributes(trait->attributes, 0), spaceSep, fmtSpace(ejs, qname), 
            accessorSep, fmtAccessors(trait->attributes), demangle(ejs, qname.name));
    }

    for (i = 0; i < (int) fun->numArgs;) {
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
        out(mp, "<div class='apiDetail'>\n<p>%s</p>\n", doc->brief);
        if (doc->description) {
            out(mp, "<dl><dt>Description</dt><dd>%s</dd></dl>\n", doc->description);
        }

        count = mprGetListCount(doc->params);
        if (count > 0) {
            out(mp, "<dl><dt>Parameters</dt>\n");
            out(mp, "<dd><table class='parameters' summary ='parameters'>\n");

            checkArgs(mp, ejs, fp->ownerName, fun, qname, doc);
            for (next = 0; (param = mprGetNextItem(doc->params, &next)) != 0; ) {

                defaultValue = getDefault(doc, param->key);
                i = findArg(ejs, fun, param->key);
                if (i < 0) {
                    mprError("Bad @param reference for \"%s\" in function \"%s\" in type \"%s\"", param->key, 
                        qname.name, fp->ownerName.name);
                } else {
                    argName = ejsGetPropertyName(ejs, fun->activation, i);
                    argTrait = ejsGetPropertyTraits(ejs, fun->activation, i);
                    out(mp, "<tr class='param'><td class='param'>");
                    if (argTrait->type) {
                        out(mp, "%s: %s ", fmtDeclaration(ejs, argName), fmtTypeReference(ejs, argTrait->type->qname));
                    } else {
                        out(mp, "%s ", fmtDeclaration(ejs, argName));
                    }
                    out(mp, "</td><td>%s", param->value);
                    if (defaultValue) {
                        if (strstr(param->value, "Not implemented") == NULL) {
                            out(mp, " [default: %s]", defaultValue);
                        }
                    }
                }
                out(mp, "</td></tr>");
            }
            out(mp, "</table></dd>\n");
            out(mp, "</dl>");
        }

        count = mprGetListCount(doc->options);
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
        
        count = mprGetListCount(doc->events);
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
        count = mprGetListCount(doc->throws);
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
        count = mprGetListCount(doc->see);
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


static char *fmtAttributes(int attributes, int accessors)
{
    static char attributeBuf[MPR_MAX_STRING];

    attributeBuf[0] = '\0';

    /*
        Order to look best
     */
    if (attributes & EJS_PROP_STATIC) {
        strcat(attributeBuf, "static ");
    }
    if (attributes & EJS_TRAIT_READONLY) {
        strcat(attributeBuf, "const ");
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
    if (accessors) {
        if (attributes & EJS_TRAIT_GETTER) {
            strcat(attributeBuf, "get ");
        }
        if (attributes & EJS_TRAIT_SETTER) {
            strcat(attributeBuf, "set ");
        }
    }
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
    doc->params = mprCreateList(doc);
    doc->options = mprCreateList(doc);
    doc->events = mprCreateList(doc);
    doc->defaults = mprCreateList(doc);
    doc->see = mprCreateList(doc);
    doc->throws = mprCreateList(doc);

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
    doc->brief = wjoin(NULL, doc->brief, thisBrief, NULL);
    doc->description = wjoin(NULL, doc->description, thisDescription, NULL);
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
        token = mtok(next, "@", &next);
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
                doc->brief = mrejoin(NULL, doc->brief, " ", dup->brief, NULL);
                doc->description = mrejoin(NULL, doc->description, " ", dup->description, NULL);
                if (dup->example) {
                    if (doc->example) {
                        doc->example = mrejoin(NULL, doc->example, " ", dup->example, NULL);
                    } else {
                        doc->example = dup->example;
                    }
                }
                if (dup->requires) {
                    if (doc->requires) {
                        doc->requires = mrejoin(NULL, doc->requires, " ", dup->requires, NULL);
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
        return NULL;
    }
    
    /*
        Copy the string and grow by 1 byte (plus null) to allow for a trailing period.
     */
    len = wlen(str) + 2 * sizeof(MprChar);
    buf = mprAlloc(len);
    if (str == 0) {
        return NULL;
    }
    wcopy(buf, len, str);
    str = buf;
    str[0] = toupper((int) str[0]);

    /*
        We can safely patch one past the end as we always have new lines and white space before the next token or 
        end of comment.
     */
    str = mtrim(str, " \t\r\n.", MPR_TRIM_BOTH);

    /*
        Add a "." if the string does not appear to contain HTML tags
     */
    if (mcontains(str, "</", -1) == 0) {
        len = wlen(str);
        if (str[len - 1] != '.') {
            str[len] = '.';
            str[len+1] = '\0';
        }
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
        if ((end = wchr(cp, '@')) != 0) {
            *end = '\0';
        }
        for (indent = 0; *cp == '\t' || *cp == ' '; indent++, cp++) {}

        buf = mprAlloc(wlen(example) * 4 + 2);
        for (cp = example, dp = buf; *cp; ) {
            for (i = 0; i < indent && *cp; i++, cp++) {}
            for (; *cp && *cp != '\n'; ) {
                if (*cp == '<' && cp[1] == '%') {
                    mtow(dp, 4, "&lt;", 4);
                    dp += 4;
                    cp++;
                    *dp++ = *cp++;
                } else if (*cp == '%' && cp[1] == '>') {
                    *dp++ = *cp++;
                    mtow(dp, 4, "&gt;", 4);
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
        return buf;
    }
    return NULL;
}


static MprChar *wikiFormat(Ejs *ejs, MprChar *start)
{
    EjsLookup   lookup;
    EjsName     qname;
    MprBuf      *buf;
    MprChar     *end, *cp, *klass, *property, *str;
    int         slotNum, sentence;

    if (start == 0 || *start == '\0') {
        return NULL;
    }
    buf = mprCreateBuf(-1, -1);

    end = &start[wlen(start)];
    for (str = start; str < end && *str; str++) {
        /*
            MOB -- expand this to support basic markdown
         */
        if (str[0] == '\n' && str[1] == '\n') {
            /* Two blank lines forces a blank line in the output */
            mprPutStringToWideBuf(buf, "<br/><br/>");
            str++;

        } else if (*str == '$') {
            /* Dollar reference expansion*/
            klass = 0;
            property = 0;
            klass = &str[1];
            for (cp = klass; *cp; cp++) {
                if (isspace((int) *cp)) {
                    *cp++ = '\0';
                    break;
                }
            }
            sentence = (klass[wlen(klass) - 1] == '.');

            if ((property = wchr(str, '.')) != 0) {
                *property++ = '\0';
                if (*property == '\0') {
                    property = &str[1];
                    klass = 0;
                }
                str = &property[wlen(property)];
            } else {
                str = &str[wlen(str)];
                if (islower((int) *klass)) {
                    property = klass;
                    klass = 0;
                }
            }
            if (klass) {
                klass = mtrim(klass, ".", MPR_TRIM_BOTH);
                property = mtrim(property, ".", MPR_TRIM_BOTH);
                //  TODO Functionalize
                ejs->state->bp = ejs->global;
                if ((slotNum = ejsLookupVar(ejs, ejs->global, WEN(klass), &lookup)) < 0) {
                    continue;
                }
                qname = lookup.name;
                if (property) {
                    mprPutFmtToWideBuf(buf, "<a href='%s#%s'>%s.%s</a>", getFilename(fmtType(ejs, qname)), 
                        property, klass, property);
                } else {
                    mprPutFmtToWideBuf(buf, "<a href='%s'>%s</a>", getFilename(fmtType(ejs, qname)), klass);
                }
            } else {
                mprPutFmtToWideBuf(buf, "<a href='#%s'>%s</a>", property, property);
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
        mp->file = mprOpen(mp->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (mp->file == 0) {
            mprError("Can't open %s", mp->path);
            mp->errorCount++;
            return;
        }
    }
    va_start(args, fmt);
    buf = mprAsprintfv(fmt, args);
    if (mprWriteString(mp->file, buf) < 0) {
        mprError("Can't write to buffer");
    }
    mprFree(buf);
}


static EjsString *fmtModule(Ejs *ejs, EjsString *name)
{
    if (ejsCompareMulti(ejs, name, EJS_DEFAULT_MODULE) == 0) {
        return ejs->emptyString;
    }
    return name;
}


static char *fmtClassUrl(Ejs *ejs, EjsName qname)
{
    return getFilename(fmtType(ejs, qname));
}


//  MOB -- should this be EjsString return?

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

    if ((cp = strrchr(buf, ',')) != 0) {
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
            mprSprintf(buf, sizeof(buf), "%s::%s", namespace, qname.name);
        } else {
            mprSprintf(buf, sizeof(buf), "%s", qname.name);
        }
    } else {
        mprSprintf(buf, sizeof(buf), "%s", qname.name);
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
    mprSprintf(buf, sizeof(buf), "<a href='%s'>%s</a>", getFilename(typeName), qname.name);
    return buf;
}


static char *fmtSpace(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);

    if (namespace[0]) {
        mprSprintf(buf, sizeof(buf), "%s", qname.space);
    }
    return buf;
}


static char *fmtDeclaration(Ejs *ejs, EjsName qname)
{
    static char buf[MPR_MAX_STRING];
    char        *namespace;

    namespace = fmtNamespace(ejs, qname);
    if (namespace[0]) {
        mprSprintf(buf, sizeof(buf), "%s %s", qname.space, demangle(ejs, qname.name));
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
    return compareNames((char**) &(*p1)->qname.name, (char**) &(*p2)->qname.name);
}


static int compareFunctions(FunRec **f1, FunRec **f2)
{
    return compareNames((char**) &(*f1)->qname.name, (char**) &(*f2)->qname.name);
}


static int compareClasses(ClassRec **c1, ClassRec **c2)
{
    return compareNames((char**) &(*c1)->qname.name, (char**) &(*c2)->qname.name);
}


static cchar *demangle(Ejs *ejs, EjsString *name)
{
    if (ejsStartsWithMulti(ejs, name, "set-") == 0) {
        return ejsToMulti(ejs, ejsSubstring(ejs, name, 4, -1));
    }
    return ejsToMulti(ejs, name);
}


static cchar *demangleCS(cchar *name)
{
    if (strncmp(name, "set-", 4) == 0) {
        return &name[4];
    }
    return name;
}


static int compareNames(char **q1, char **q2)
{
    char    *s1, *s2, *cp;

    s1 = *q1;
    s2 = *q2;

    s1 = (char*) demangleCS(s1);
    s2 = (char*) demangleCS(s2);

    /*
        Don't sort on the namespace portions of the name
     */
    if ((cp = strrchr(s1, ':')) != 0) {
        s1 = cp + 1;
    }
    if ((cp = strrchr(s2, ':')) != 0) {
        s2 = cp + 1;
    }
    return scasecmp(s1, s2);
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


static EjsDoc *getDoc(Ejs *ejs, void *vp, int slotNum)
{
    char        key[32];

    mprSprintf(key, sizeof(key), "%Lx %d", PTOL(vp), slotNum);
    return (EjsDoc*) mprLookupHash(ejs->doc, key);
}


static EjsDoc *getDuplicateDoc(Ejs *ejs, MprChar *duplicate)
{
    EjsName         qname;
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
        space = NULL;
    }
    if ((property = wchr(klass, '.')) != 0) {
        *property++ = '\0';
    }
    //  TODO Functionalize
    ejs->state->bp = ejs->global;
    if ((slotNum = ejsLookupVar(ejs, ejs->global, WN(space, klass), &lookup)) < 0) {
        mprFree(dup);
        return 0;
    }
    if (property == 0) {
        doc = getDoc(ejs, ejs->global, slotNum);
    } else {
        vp = ejsGetProperty(ejs, ejs->global, slotNum);
        if ((slotNum = ejsLookupVar(ejs, vp, WEN(property), &lookup)) < 0) {
            if (ejsIsType(ejs, vp)) {
                vp = (EjsObj*) ((EjsType*) vp)->prototype;
                if ((slotNum = ejsLookupVar(ejs, vp, qname, &lookup)) < 0) {
                    mprFree(dup);
                    return 0;
                }
            }
#if UNUSED
            if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                    /* Last chance - Not ideal */
                    space = mprAsprintf("%s-%s", space, BLD_VNUM);
                    if ((slotNum = ejsLookupVar(ejs, vp, ejsName(&qname, space, property), &lookup)) < 0) {
                        mprFree(dup);
                        return 0;
                    }
                }
            }
#endif
        }
        doc = getDoc(ejs, (EjsBlock*) vp, slotNum);
    }
    if (doc) {
        if (doc->docString == NULL || doc->docString->value[0] == '\0') {
            mprError("Duplicate entry %s provides no description", duplicate);
            mprFree(dup);
            return 0;
        }
    }
    mprFree(dup);
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
    pair->value = wclone(value);
    return pair;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

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
      0, 72,  0,  0,255,219,  0, 67,  0,  7,  4,  4,  4,  5,  4,  7,
      5,  5,  7, 10,  7,  5,  7, 10, 12,  9,  7,  7,  9, 12, 13, 11,
     11, 12, 11, 11, 13, 17, 13, 13, 13, 13, 13, 13, 17, 13, 15, 16,
     17, 16, 15, 13, 20, 20, 22, 22, 20, 20, 30, 29, 29, 29, 30, 34,
     34, 34, 34, 34, 34, 34, 34, 34, 34,255,219,  0, 67,  1,  8,  7,
      7, 13, 12, 13, 24, 16, 16, 24, 26, 21, 17, 21, 26, 32, 32, 32,
     32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 32, 32, 32, 32, 32,
     32, 33, 33, 33, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 34,
     34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,255,192,
      0, 17,  8,  0, 95,  3,132,  3,  1, 17,  0,  2, 17,  1,  3, 17,
      1,255,196,  0, 27,  0,  0,  3,  1,  1,  1,  1,  1,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  2,  3,  4,  1,  0,  5,  6,  7,255,196,
      0, 66, 16,  0,  1,  3,  3,  2,  4,  3,  6,  3,  4,  8,  5,  5,
      0,  0,  0,  1,  0,  2,  3,  4, 17, 33, 18, 49,  5,  6, 65, 81,
     19, 34, 97,  7, 20, 50,113,129,145, 66,161,177, 21, 82,193,209,
     35, 51, 98,114,130,225,240,241, 22, 36, 55, 83,146, 84,131,162,
    178,195,255,196,  0, 26,  1,  0,  3,  1,  1,  1,  1,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,255,196,
      0, 51, 17,  1,  0,  3,  0,  2,  1,  2,  4,  4,  4,  6,  2,  3,
      0,  0,  0,  0,  1,  2, 17,  3, 18,  4, 33, 49,  5, 19, 34, 65,
     50, 81, 97,129,113,161,177,193, 20, 35, 51, 66,145,240, 52,241,
     98,209,225,255,218,  0, 12,  3,  1,  0,  2, 17,  3, 17,  0, 63,
      0, 64, 56,213,178,240,102, 61, 95, 91, 14,235,243, 79, 20,108,
    114,185,190,161, 72, 57,146, 53,246,107,176,123,116, 68,193,235,
    100,139, 62,138, 98, 21, 22, 41,208, 52,161,164, 88, 62,  1,232,
    167, 21,221,205,241, 91,234, 16,114,100,114,144, 52,244,236, 83,
     78, 12,248,114, 31, 80,151, 82,208, 62, 23,111,191, 98,167,170,
    162,206,108,142, 24,234,133,  8, 61,167,112,130, 46, 72,175,144,
    137,168,137, 41,237, 32,254,169, 43, 92, 45,126,150, 70,  1, 13,
     63, 35,220,163,  0,128,112, 23, 27, 36,  5, 28,239,111,242,186,
     10, 97, 67,103, 99,183,223,178, 17, 53, 25,100,110, 55,217, 24,
     52,183,211,226,199,238,150, 14,192, 48,144,219,148,177, 90, 16,
    196,143,179, 68, 97,  7,162,  2,248,114,156, 26,223, 14,199,  8,
    195,215, 27,254, 44,118, 41, 13, 21,255,  0,221, 35, 16,105, 59,
     38, 76, 55,217,  1,128,217, 32, 54,190,193,  4, 54,200,128, 32,
     90, 74, 64, 97,229,  3, 28,108, 80,  3,165,  1,161,151, 64,209,
     55, 27,236,130,110,172,250, 32,156, 92,128,  7,109,130,133, 22,
    236, 12,160, 58,246, 24,200,238,152, 27, 72,183,170,  0,172, 11,
    125, 80, 69,189,169, 40,151, 49, 51,  1,  4,124,144, 28,  5,208,
     97,124,100,117, 64, 40, 92, 31, 78,182, 66,176, 87, 27, 90,227,
    161, 67, 60,  9,104,220, 32, 52, 99, 32,229, 56, 32,147,125,194,
     65,186, 75,153,110,168, 34, 36,135,186, 70,157,192,176,250, 38,
     38,  2, 66, 17, 53,118,175, 84, 39,  7, 28,228, 36, 83, 10, 89,
     45,194,148,117, 11,192, 61,136, 76,166,160,183,238,253,149, 51,
    234,186,138,115,166,219, 39, 12,111, 69,226, 75,139, 42,134, 51,
     80,187, 57, 74, 83,141,107,242,164, 97,236,112,112,245,178, 33,
     19, 86,  7, 88,250,245, 87,169,234, 38,206, 45,186,122,153,169,
    130,113,245, 79, 81, 52,104,151,205,234,141, 46,173,115,130,173,
     46,169,230,157,218, 72, 10,226, 71, 68, 83,219, 65, 61, 86,145,
     98,154,163,151, 67,239,171, 61,150,157,138,106,130, 86,183, 32,
    165,168,154,149, 20,154, 14, 70, 21,214,238,123, 85,232,210, 84,
    182,250, 14,221, 10,214,183,103, 48,202,150,249,175,155, 43,155,
      4, 21, 13, 37,142, 61, 82,236,111,159,175,131, 67,238, 18,211,
     78,199,105, 27, 35, 77, 91, 46,230,223,248,169,155, 19, 67,116,
    157, 91,122,127,186,157,  2,183,154,223, 91, 35,177,224,163,105,
      7, 38,193,201, 77,134, 40,100, 54,117,186,122, 37,216,241,116,
     31, 25,245, 65, 46,107, 54,236,166,100,150, 82, 51,241, 20,106,
     95, 79,194, 41,188,192,173, 42, 79,173,162,109,152, 45,217,111,
     82, 54, 22,248,149,  8,221, 15, 89,182,107,112,182, 12,113,184,
     68,200,107, 95,100,226, 65,110, 62,112,158,145,247, 10,192,245,
    216, 20,  0, 92,219,213, 45, 55, 94,199,  8,210,  3,222,  0, 64,
     73, 35,242,158,145, 82, 73,215,167,170,173,  5,137, 91,211, 99,
    246, 74,100,155,226, 54,223,193, 26,  2,249,  0,  9,232, 71, 44,
    198,246,111,194,158,146,119,205,161,218,124,195,232,152,126, 79,
     12,247,206,254,191,235, 11,203,181, 31, 97,170, 24, 90,227,126,
    171, 57,170,180,192,214,217, 78, 42, 26, 25,140,125,186, 41,147,
     50, 55, 72,222,246,236,129, 48,104,123, 29,241, 11, 33, 56,221,
     13,182,233, 97,246,  9,138,193, 44, 92, 93,134, 43,124, 65, 37,
    107,132, 96,237,245,  9,141,104,107,153,234, 61, 50,132,200,173,
     20,152,216,163, 14, 44,  3, 76, 70,217, 81,138,236, 29, 47,  9,
    226,130,230, 49,255,  0,234,201, 76,  8,121,252, 95,136,240,238,
     17, 78, 39,175,157,148,240,147,165,166, 67,185,236, 58,149, 84,
    226,181,189,189, 83,203,207, 78, 56,219, 78, 25, 13, 76,114, 64,
    201,226, 34, 72, 36,  1,204,123, 77,218,224,118, 32,168,246,156,
    150,149,203, 70,199,177,173,147, 62, 67,111, 68,  9,169,141, 44,
    126, 54, 61,123, 35, 18, 47, 14,219, 41,235, 34, 44, 38,201, 35,
    126, 45,146, 63,116,124,203, 93, 87, 79,203,213,213,116,115, 24,
    106, 41,160,150,120,220,  3, 93,115, 27, 11,128, 33,225,194,196,
    140,173,120, 34, 38,209, 19,247,151, 63,149,181,227,153,143, 73,
    136,153,120, 82,113,206,106,135,140, 71,  3,101, 19,112,198, 79,
     69, 21, 68,242, 54, 48,241,227,185,129,204,179, 67, 71,159,222,
      5,136, 24,211,234,186, 62, 95, 28,215,245,201,255,  0,191,201,
    199, 60,188,209,124,247,174,215,249,255,  0,239,249, 62,196,198,
     28,184,158,136, 67,109,139,125, 18,195,209, 50, 62,189, 82,195,
    214,249,173,230, 23, 42,112,217,107,252,208, 97, 44, 45,192,234,
    145,235,128, 32,122,160, 24, 44,225,252, 80, 78,209,245, 72,193,
    164,180,225, 51,110,187, 34, 67,115, 97,167, 42,  3, 91, 32,189,
    142,232,  6,  7, 56, 32, 54,247,221,  4,203,188,108, 80,109, 18,
      7,110, 50,129,141,215,209, 50,198,221,  4,227,181,138, 64, 14,
    218,200, 84, 18,239, 41,255,  0, 95,162, 97,186,158,220,244, 64,
     24,126,193,223,146,  6, 15, 85,242,130,113,  0,238,144, 45,209,
    225,  6, 83,163,185,189,240,153,176, 11,116, 64, 11,216, 44,131,
    210,203, 13,143,162, 20, 93,200, 66,113,193,249,219,  9,140, 23,
    197,217,  9,108,118,191,205,  5, 46,149,191,100,142, 19,201, 27,
     93,178,102,157,204,177,244, 73, 32, 66,112, 26,139,111,217, 51,
    195, 34,146,195,184, 73, 51, 85, 13,112, 33, 76,162, 97,185, 66,
    112,232, 30,  2,108,237, 69,176,191, 79,151,167,116,216, 94,135,
    234,105,234, 16,198,106, 23,  2, 60,221, 16, 80, 17, 38,151, 22,
    186,228,125, 19, 62,166,107,  5, 33,212, 15,126,145,132,225, 61,
     75,247,174,234,180, 77, 13, 19,106, 54,  5, 36,207, 25,205,152,
     16, 63, 69, 72,234,  9, 50,  8, 56, 70,142,168,106, 75,193,181,
    246, 10,162,195,162,  9, 73,  6,224,149,113,116,207, 25,110,113,
     57,235,221, 87,102,115, 82, 12,118,125,198, 71, 86,148,226,204,
    237, 77, 49,173,198, 55, 11,122,203,154,213, 55,197,121,110,151,
     42,236,158,165,188, 93,132, 31,162, 52, 99,204,226, 16,220, 91,
    182, 17,164,241,228,143, 67,244,236,174, 12, 80, 76, 98,118,126,
     14,161, 41, 37,131,204,221,175,212, 44,166, 79, 27,215,108,219,
     40,208, 54, 15, 48,109,210,211,122, 16,179,241,116, 81, 54, 10,
     89, 22,196,125,145, 22, 37,113,180,225,214,254, 74,210,244,168,
    226,243,180, 33, 47,168,225, 44,181,187,254, 75, 74,  7,210, 69,
    228, 97,239,109,150,240,149, 84, 81,121,131,186,162,  2,251,220,
     45,180,  0,156, 40,153,  0,241, 70,201,118,  2,102,167,149,173,
     65,219, 97,104, 65,115,251, 41,208,205,101, 71, 99,112,146,231,
    178,122, 18,205, 81,115,100,244,136,124,154, 69,209,164, 68,179,
     95,125,187,166, 29, 27,175,114,152,110,187, 53, 26, 73,229,152,
    235, 54,219,213, 80, 78,249, 90,  7,166, 77,236,153, 34,149,250,
    157,124,253,211,239,134,252,194, 54, 17,141,237,176,244, 92,147,
     47,173, 58, 27,131,230, 24,238,163,174,141,124,223,181, 74,250,
    250, 46, 88,108,244, 53, 18, 83,201,239, 12,111,137, 19,221, 27,
    172, 90,236, 93,164, 27, 43,224,227,142,206, 79,136,242, 77,120,
    253, 39, 61, 95, 63,192,249, 87,159,184,191,  9,131,137, 69,204,
     19, 50, 57,219,169,172,116,245, 23, 25,183,127, 69,119,228,227,
    172,231, 87, 55, 15,143,207,201, 88,183,121,245,253,101,247, 60,
    151,194, 56,215,  9,225,178, 83,241,122,231, 87,212, 62, 82,246,
    202,231,189,246,102,150,141, 55,126,119,  4,174, 94,107, 86,211,
    233, 24,244,252, 78, 27,210,185,123,118,157,124,247, 18,228, 78,
    125,169,226, 53, 51,211,115, 20,144,211,203, 43,223, 20, 66,106,
    129,161,142,113, 45,109,134,  5,134, 48,181,175, 63, 28, 71,225,
    113,242,120, 92,243,105,152,191,166,254,114,242,249, 11,136,115,
     36, 94,209, 93,193,184,143, 18,168,172,101, 63,143, 27,218,249,
    164,124,110, 44,  7, 58, 94, 86,156,245,175,203,216,140,115,248,
    119,228,142,126,182,180,206,111,221,239,123, 90,231,110, 37,192,
    169,233,232, 56, 99,188, 26,154,160,231, 73, 62,238, 99, 27, 97,
    102,223,171,137,223,162,203,197,224,139,122,207,217,211,241, 31,
     46,220,113, 17, 95,121,121,148,254,204,125,160, 79, 78,202,185,
     57,129,236,172,120, 14,211,227, 78,235, 95, 54,214, 15,232, 21,
    255,  0,136,227,246,234,198, 60, 30,121,141,239,235,252,101,247,
     92,183, 67,197,232, 56, 21, 53, 55, 20,151,222, 56,132, 64,137,
    167,212, 93,171,204,109,230,112, 14, 56,182,235,139,150, 98,109,
    233,236,245,188,104,180, 82, 34,222,182, 15, 26,230,206,  1,193,
     88,223,218,117, 76,128,156,182, 51,119, 56,250,134, 55, 83,143,
    217, 28,124,118,183,225,131,230,242, 56,248,255,  0, 20,226, 46,
     21,207,252,163,198, 43, 25, 71, 65, 91,170,178, 75,232,143,195,
    145,132,216, 92,216,185,160,108, 21,223,198,188, 70,204, 50,226,
    243,184,175, 57, 89,245, 89,198,121,183,131,112,  1, 23,237, 90,
    159,  5,179,234,240,174,215,186,250,109,127,128, 59,247,130,142,
     62, 27, 95,217,167, 55, 63, 31, 23,226,156,212,156, 83,218, 63,
     38,112,218,199, 82, 85, 86,143,120,110, 30,214, 53,242,105, 63,
    218, 44,  4, 95,209, 93,124, 91,218, 55, 25,114,121,252, 84,156,
    153,245,122,212,213,244, 53,180,140,170,167,144, 73, 77, 40,212,
    201, 91,150,144,176,183, 30, 78, 58,169,120,180,108,123, 63, 63,
    231,142, 47,236,239,154,105,160,136,241,175,118,150,157,196,199,
     32,130,103,139, 58,218,129,110,150,246, 29, 87,119,143,199,203,
    199,246,215,149,230,243,120,252,241, 31, 94, 76,126,146,250,222,
     87,164,225,180,220,187, 67, 13,  4,158, 61, 19, 98, 30, 12,206,
    184, 46,  7, 55,177,  2,215, 37,113,243,108,222,119,221,233,248,
    149,136,227,136,175,173,112, 21,252,195,192,232,248,172, 60, 46,
    170,163,195,174,168,  0,195, 14,151,121,131,137,104,243,  1,164,
    101,167,114,148,112, 90, 99, 99,217, 87,242,184,235,120,164,207,
    213, 47, 54, 95,104,156,155, 13, 87,186,158, 32,210,224,108, 92,
     26,242,203,255,  0,124,  2,223,173,236,182,175,137,201,155,140,
    109,241, 62, 13,206,207, 94,191,153,120, 95, 12,161,101,117,124,
    225,148, 79,176,142, 96, 11,193,212, 46,219,104, 14,189,192, 89,
     87,138,214,156,143,118,220,188,212,227,175,105,159, 71,231,124,
     71,159, 34,155,218, 44, 53,108,226, 82, 30, 93,141,241,187, 77,
    228, 17, 88, 70, 53,127, 71,107,159, 53,250, 47, 66,190, 54,113,
    102,125, 79, 11,147,206,223, 38, 39,180,252,191,223,250, 63, 65,
    225, 28,237,202,188, 98,176, 81,240,218,193, 45, 81,  5,194, 61,
     18, 55,  3,124,185,160, 47, 63,147,197,189, 99,102, 61, 30,199,
     23,157,197,201, 57, 89,217,123, 26, 91,127, 85,142, 58, 69,119,
     14,170,112,198,215,231, 41,165,172,113,102,110,145, 26,199, 49,
    251,238,140, 30,205, 49, 37, 53, 56,177,101,182, 25,251,168,195,
    215,  4, 30,137,173,  7,  3,  8, 26,226,194,212,176,107,177,107,
     20,164,244,183,179,182,201, 40, 25, 27, 35, 13,183, 39,249, 32,
    218,217, 69,236,128, 96,118, 16, 66,232,164,152,246,245,106, 20,
    230, 58,251,238,128, 35,220, 38, 82, 54,184, 29,208,151, 22,147,
    177, 64,210,237,209, 10, 14,159, 53,186, 32, 51,195,255,  0,116,
      1,183,215,100, 16,245,  6,252,143,228,145,  5,219,225,  6,157,
    196,131,234,133,128,200,115,221,  3, 24, 93,118,227,226, 76,240,
     55, 32,103,234,131, 11,155,116, 25, 47,184, 56, 64,107, 95,183,
    126,161, 52,225,151, 32,220, 33, 39, 91, 91, 65, 73, 50, 68,177,
    184, 28, 12, 32,224,169, 34, 79,  8,137, 34, 35, 32, 97,  4,  7,
     70,122,  4, 28,  0, 51, 79,194,145,225,145, 59, 73, 66,102,170,
      1, 31,201, 12,230,162,233,125,146, 78, 27, 20,207,211,234,154,
    109, 83,227,156, 19,156, 20, 49,181, 13, 18,255,  0,178,108,166,
    174,152,231, 80, 63, 32,148,193,214, 11, 50, 16,115,143,186, 74,
    198,186,111, 46,118, 76,116, 32,188,125, 16, 83, 64,137, 71, 76,
     20,106,122,169,130,168,126, 53, 90,198,213, 57,242, 89,184, 59,
    247, 65, 68, 35,149,196,142,254,137,194,226,168, 38, 22,126, 62,
    202,132,213,128, 92, 42,137,101,106,133,204,199,170,122,202,106,
    193,123,217, 92, 89,149,168,208,110,181,236,195,  2,236,163,178,
    102,  8,170,143, 91, 79, 75,236,141, 79, 87,151, 85, 74,235,105,
     35,252, 73,197,139, 16,186, 18,211,230, 30, 94,133,107,219, 73,
     68, 18, 59,  0,236, 70, 22, 87, 81,194,246, 83,163, 13,136, 29,
     88, 56, 27,165, 50, 79, 66,156,249,123,125,214,114, 30,140, 13,
    184, 22,250,165,169,149,113, 83, 93,155, 99,161, 90,194,101,125,
     19, 44,225,185,249,170, 75,233,184, 91,114, 47,140, 43,226,144,
    247, 90,117,  0,183, 75,210,165,  3, 72, 85,  0,237, 74,180, 21,
     83, 38,149, 51, 38, 64,113, 62,110,168,175,168, 91, 78,124,131,
     86, 74,222,133, 50,217, 79,150,225, 22,176, 33,178,151, 59, 39,
    228,163, 77,174,127,153, 45,  5,185,231,107,167, 18, 73,222, 64,
    125,213, 73, 39,157,228,156,167,  0,153, 94, 45,230,252,149, 68,
    129, 50, 93, 49,252,250, 37,164, 76,211,128,  9, 61, 58, 43,128,
    153,213,  7,162, 52, 18, 73,248,183,183, 69, 51, 33, 51,156,226,
    110,219,217, 68,216, 99,243, 74,106,200,228,104,177,179,198,234,
     38, 31, 88,190, 41, 90,111,175,175, 85, 36,249, 47,108,128,127,
    194, 76, 35,255,  0, 83, 31,255,  0, 87,173,184, 63, 19,135,226,
     95,233,254,239,  7,150,167,246,178,222,  5, 74, 56, 68, 17,187,
    134,105,255,  0,151,115,189,223,225,185,253,247,  7,111,221, 93,
    227,143,125, 92,220, 22,242,122, 71, 95,111,217,247,220,164,121,
    154, 78, 17,171,152, 90,214,113, 17, 35,188,173,209,109, 24,211,
    253, 89,112, 92,188,181,174,253, 47, 83,198,183, 39, 95,175,221,
    235,109,186,198,106,234,139, 63, 47,229,127,250,205, 91,111,251,
    181, 95,197,118,242,127,163, 31,179,197,241,255,  0,242,231,248,
    203,235,125,161,242, 57,230,154, 56, 93, 28,130, 26,250,109, 94,
     11,223,240, 57,174,221,174,181,207, 76, 21,207,193,205,210,127,
     71,119,157,225,124,232,244,247,135,196, 69,199,125,167,114, 48,
    100, 85,140,116,156, 53,158, 86,  9,135,141, 13,186,  6,202,220,
    183, 27, 13, 95, 69,211,211,139,147,219,221,230,124,207, 35,199,
    247,252, 63,201,250, 95, 41,243,143, 15,230, 62,  2,120,164,109,
     49, 24,181, 54,166,  3,230, 44,115, 70,163, 99,139,130, 54, 43,
    143,151,135,172,227,214,241,188,168,228,167,103,230,124,141,193,
     71, 60,243, 77,127, 19,227, 55,150, 56,199,138,248,117, 16,  9,
    121,179, 25,113,157, 45,  3,162,236,231,183,202,164, 69, 94, 87,
    135,199, 30, 71, 44,218,254,175,190,167,228, 94, 91,225,245,241,
    113, 26, 74, 49, 79, 85,  5,252, 55, 48,186,217,105,105,184,189,
    142, 10,225,183, 53,230, 50,103,209,238,113,120,124, 53,180, 90,
     35, 38, 31, 33,237,186, 87, 73, 23, 10,212, 45, 99, 63,255,  0,
    154,233,240, 35,221,231,252,122, 63,  7,239,253,158,149, 31,179,
     46,  5, 47, 40, 24, 89, 24,147,140, 84, 65,226, 54,178, 66,225,
    105,156, 53, 54,214,189,155,124, 40,159, 42,221,255,  0, 70,149,
    248, 85, 62, 79,255,  0, 57,143,127,213,236,251, 61,229,254, 43,
    203,252,  5,252, 59,137,190, 57, 79,140,231,197,225, 56,185,161,
    142,107,113,230, 13,252, 87, 43, 47, 39,146, 47,109,134,254,  7,
    143,126, 42,117,183,230,249,127,107, 60,177,203,220, 43,151,160,
    168,225,180,108,167,153,213, 76, 99,158,209,146,211, 28,134,223,
    112, 22,254, 31, 45,173,111, 89,223, 71, 15,197, 60,110, 58,113,
    196,214, 50,119,255,  0,183,213,242, 73,112,229, 14, 25,219,221,
    217,159,162,230,231,252,114,245, 60, 15,244,107,252, 31,  9,237,
     86,  7,214,115,191, 15,166, 14,208,233,169,225,139, 95,109,115,
    200,219,254,107,179,194,156,227,153,253, 94, 79,197,233,219,200,
    172,126,113, 31,214, 94,199, 53,114, 31, 44,209,242,109, 65,163,
    131, 77,101, 36,126, 35,106, 73, 37,238, 45,248,181,116,243, 11,
    225,101,195,228,218,121, 61,125,165,213,230,124, 55,138,156, 19,
    145,235, 95,185,190,204, 97,162,227,188,149,238, 28, 90, 54,212,
     65, 75, 82,230, 68,199,220,216,105, 15, 31,111, 16,163,202,158,
    156,155, 31,120, 71,195, 43, 28,188, 25,127, 88,137,124,247, 17,
    224, 28, 33,158,214,163,225, 17,211,179,246,113,146, 17,224,126,
     27, 58, 16,227,249,173,235,121,249, 59,247,113,114,240,210, 60,
    190,185,244,250,122,126,207,211,248, 87, 41,114,215, 12,168, 21,
     84, 52, 81,211,212,128, 91,226, 48,102,199,117,193,126, 91, 90,
     50,101,236,241,248,220,116,157,172,100,189, 63,254, 75, 44,116,
    176, 17,107,148,176,198,221, 29,254,234,115, 75, 76,210, 15,116,
    176,155,162,231,  8,192, 54,185,195,111,186,120, 70,105,212, 51,
    241, 41,154,141, 45,236, 32,169,154,171, 89,211, 59, 36, 37, 39,
      7,230, 14, 17,197,132,191,179,231, 19,248, 36, 54, 92, 56,105,
     39, 34,225,192, 42,228,225,181,125,217,241,115,214,255,  0,134,
    119, 22,186, 54,184, 97, 99,141,162, 73,116,111, 14, 75, 21,160,
     45,  4,255,  0, 20, 45,132, 56,122,183,186, 65,186,  3,253, 80,
    167, 51, 91, 78,255,  0, 68,200,214, 27,139,148,146, 61,208, 28,
    232, 65, 72, 21, 61, 77, 61, 44, 47,154,166, 70,197,  3, 50,249,
    100, 33,173, 31, 50,112, 19,138,204,250, 65,218,209, 17,179,233,
      0,161,226,156, 58,185,133,244, 21, 49, 84,177,134,206, 48,189,
    178,  0,125, 75, 73, 69,169,106,251,198, 34,156,149,183,225,152,
    149, 45, 37, 37, 55, 85,207,116,131,  8, 23,194, 97,132,116, 40,
     14, 72,195, 52,208,195,  3,230,148,218, 40,218, 94,243,216, 52,
     92,149, 85,141, 76,206, 70,149, 65, 93, 75,196, 40,162,173,163,
    127,137, 77, 51, 67,226,146,196, 93,167,108, 27, 20,239, 89,172,
    228,251,194,105,120,180,108,123, 73,124, 70,162,158,154,157,245,
     53, 82, 54, 24, 35,203,228,123,131, 90, 62,100,225, 42,214,102,
    114, 23, 55,138,198,207,164, 23,118,158,191, 36,155, 67,156, 72,
    202, 65,217, 57,221, 48,230,139,  2, 10, 10, 65, 43,  9, 65,146,
    219,131,116, 25,205,248, 85, 38, 70,199,120,102,221, 10, 11,  4,
    247,253,186, 36, 88, 23, 55, 55,252,147,212,204, 49,208,141,210,
     65, 18, 71,110,136, 56, 36,233,182, 17,139, 13,134,233, 12, 16,
    193,190,200, 44, 62, 55, 95,115,148, 34, 96,122,111,158,168, 76,
    195,137, 32,161, 19,  3,100,133,167,184, 41,178,154,159,226, 11,
     36,140, 42,103, 11, 33,165, 97, 59,165,177,207,209, 11,234,  1,
     48, 39, 41, 28,208,120,239,148,145,106,  9,147, 17,135, 42,137,
     97,106,155,226,233,177,105,207,117, 76,250,184,249,155,171,114,
     85,  8,130,204,122,133,157,244, 64,146, 95,  7,151, 27,160,117,
     32,130,194, 83,212, 90,141,105, 14, 30,170,181,148,209,218, 71,
    201, 84, 75, 27, 81,174,107,109,253,163,178,173, 99, 52,194,110,
      3,188, 63,205, 26, 58, 21, 84,  3,154,141, 76,209,230, 84,210,
    216, 95,167,232,174,182,101,106,146,198, 16, 79,115,177, 85, 51,
    164,107,119,  3,162,144,169,172,206,  6,233, 18,186,113,102,223,
    162,206, 73,232,209,147,229, 83, 37,143, 98,144, 53,204,236, 21,
    214,201,180, 45,163,131,170,211, 80,246,248, 89, 13,120,  7,170,
    210,158,129,239, 83,244,238,182,209,143, 66, 39,129,129,186,178,
     52,118, 72,210,212,188, 23,119,233,245, 83, 50,101,194,227,125,
     37, 28,114, 75, 99,147, 75,126, 75,166,165, 48,  9,101,212,219,
     40,180,130, 27, 32,189,186,169,208, 96,218,233,130,124, 75, 63,
     42,136,154,151,247,221, 57,176, 76,103,213,111, 77,214,113, 32,
    185,100,236,181,137, 32, 62, 77, 45,176,177, 87,161, 51,156,109,
    242, 74,  0, 29,134,254,119, 74,100, 20,236,149, 19, 32,177,229,
    197,146, 15,199, 98,144,227, 79,221,107, 47,175, 89, 13,124,204,
    199,196, 55, 55,201, 81, 48, 88,240,125,170,214,182,110, 84,107,
     51,171,222, 99, 59, 91,240,185,105,193, 31, 83,207,248,164,127,
    149,251,188,222, 88,246,185, 69,193,184, 13, 55, 12,146,129,242,
    186,157,186,124, 64,240,  1,201, 59, 91,213, 93,248, 54, 92,156,
     31, 16,138, 82, 43,158,207,189,228,190,107,167,230,158, 29, 37,
    108, 48, 58,156, 67, 47,133,161,206, 14, 38,205,107,175,124,126,
    242,231,228,227,234,244, 60,111, 39,230,198,251, 62,110,179,219,
     20, 28, 62,178,178,131,136,112,233, 69, 77, 60,175, 99, 11,  8,
    179,195, 92, 67, 73,213, 98,219,143,154,210, 60,119, 53,190, 37,
    214,102, 38, 61, 97,231,123, 47,225, 28, 95,138,243, 77, 87, 53,
     84,196, 97,167,147,197,116,100,130,  3,223, 49,217,151,221,173,
     23,202,124,243, 17, 94,172,254, 31, 75, 91,146,121, 37,244, 92,
    237,237, 10,175,149, 56,164, 20,230,139,222,105, 38,139, 81,126,
    162,194, 29,168,139,  7, 89,205,216,108,178,227,224,139,195,179,
    202,243,167,134,222,219, 15,152,230, 63,107,108,227,124, 34,110,
     17, 67,195, 94,102,172,111,135,119,187, 93,175,251,173,104,201,
    236,180,167,139,214,119, 92,124,255,  0, 18,249,149,235, 21,247,
    125, 71,178,174, 89,175,224,156,183, 40,226, 49,152,231,173,127,
    136,232, 29,187, 89,167, 72, 14,236, 78, 74,203,201,191,107,122,
    125,157,159, 14,241,230,156,127, 87,188,190, 30,138,126, 57,236,
    215,153,230,116,180,198,126, 31, 53,227, 14,203, 91, 44,119,187,
     92,215,228,  7,142,223,238,186, 39, 57,107,250,188,234,247,241,
     57, 61,182,175,177,224, 62,214,105,248,255,  0, 29,165,225, 84,
    156, 62, 70,182,114,124, 73,164,112,242,  6,180,187,225,104, 61,
    173,186,231,191,139,214, 55, 93,252, 63, 18,249,151,138,196,123,
    188,143,111, 17, 53,145,240,157, 61, 77, 70,223,251,106,252, 47,
    187, 31,140,219,122,254,255,  0,217,246,220, 46,153,195,134, 82,
    185,191,246, 99,199,248, 66,227,188,122,189,190, 11,237, 99,248,
     10,174,179,220,233, 39,169,120, 46, 16, 70,233, 28,  7,102, 11,
    227,236,162, 43,179,139,189,162,181,153,252,159,150,115,231,180,
     58, 30,100,224,241, 80,211,193, 44, 79,100,237,152,186, 66,219,
     88, 49,237,182, 63,190,189, 47, 31,198,154, 78,190,119,226, 31,
     17,167, 53, 58,196, 76,122,189,238, 68,246,145,195,188, 14, 27,
    203,198,154, 95, 31, 75, 96,241,124,186, 46,  6,251,221, 99,228,
    120,211,235,103, 79,129,241, 10,253, 60,121, 59,236,242,253,170,
    213, 71, 73,207, 92, 54,168,141, 76,134,  8, 36,112, 27,144,217,
    228,113, 31,146,211,197,141,227,152, 99,241,107,117,242, 43, 63,
    148, 71,245,150,243, 71,180, 40,249,130,128,240,126,  5, 73, 80,
    103,170,179,100,212,209,171, 72, 55,179, 90,194,251,223,213, 79,
     15,139,210,123, 90,125,151,229,252, 83,231, 83,229,241,196,236,
    190,179,145,121,114,171,129,114,252, 84,211,226,166, 71, 25,167,
    104, 59, 57,214, 22,237,134,180,  5,205,228,223,189,181,234,124,
     59,199,249, 60, 89, 62,243,235, 47,146,231,119, 86,112, 63,104,
     80,115, 19,161,116,148,133,209, 61,174,232,116, 52, 49,204,190,
    193,214, 11,175,199,203,113,117,251,188,159,136, 68,241,121, 49,
    201,159, 79,163,235,185,119,218,127, 11,227,252, 81,188, 58,146,
      9,218,242,215, 61,207,148, 52,  1,167,251,174,114,230,228,241,
    102,145,178,239,241,254, 33,199,205,126,181,137,125, 63,137,139,
    174,124,119, 77, 76, 26,180,227, 33,  9,198,234, 61,111,111,154,
      0,227,146,201, 76,  3, 93,105, 99,115, 13,192,112, 32,150,146,
     14,123, 17,177, 73, 51, 15,134,229,158, 33,199,170,248,221, 63,
      4,170,158,161,206,229,239,120,127, 17,151, 83,175, 83,157, 52,
    161,198,247,117,216,117, 88,239,101,215,203, 90,197,123, 71,251,
    189,191, 79,205,230,112, 94,243,120,164,239,249,123,191,175,228,
    151,146,249,154,166,175,155,248,103,187,207, 32,164,171,142,164,
     84,210,201, 87, 81, 86,224, 88,219,179,198, 18,128,200,223,118,
    236,206,159,157,243,113,229, 39,244,207,182, 50,241,185,230,220,
    149,207,105,223, 77,153,255,  0,157,123, 30,204,120,103, 16,175,
    224,220, 43,143,212,241,106,201,103,123,103, 21, 84,243,200,233,
     98,145,186,157, 28, 98,206, 62, 82,194,221, 87,220,172,188,169,
    136,153,174, 67,111,  6,182,154,214,253,167,238,251, 41, 98, 17,
     49,210, 92,104,110, 73, 61, 45,186,226,234,244,123, 62, 19,150,
    185,147,132,127,196, 28,209, 45, 61, 67,106, 12,165,147, 83,197,
      3,129,124,173,138,159,205,225,119, 34,214, 93, 92,188, 54,235,
     79,251,247,112,112,115,211,191, 38, 78,255,  0,235,236,242, 56,
     23, 49, 87, 79, 87, 86,234, 58,169,  5, 52,188, 26,166,163,193,
     21,149, 21,102, 41,152,  1,105,115,230, 13,209, 43,117,103, 66,
    219,147,134, 50, 54, 63,221, 31,104,143,251, 14,126, 31, 34,102,
    103, 39,211,164,253,230,125,127,127,186,238, 21, 89,197,104, 60,
     41,127,104, 85, 84, 58,175,150,223,196,101,241,158,233,237, 80,
     52,150,185,140,118,  5,181, 90,195,126,170, 57, 41, 91,125,163,
    210,249,251, 53,226,228,181,126,243, 59,197,191,159,168,253,157,
    204,107,184,155,106,191,104,151,134,209, 48, 62,136,215,190,169,
    207,156,159,233, 39,116,101,214,104,200, 22,182, 15,101,151,153,
     94,181,204,251,251,230,122,126, 77, 62, 31,126,214,222,223,237,
    246,237,190,191,155,236,120,161,157,188, 62,160,211,235,247,129,
     27,188, 63, 13,173,123,245, 91, 26, 90,242,214,147,232, 74,224,
    164, 70,198,189, 78, 77,235, 57,238,241,185, 98,110, 63, 45,100,
    159,180,189,243,194,209,228,247,154,122,104, 91,170,227, 99,  4,
    146, 18,109,220, 45,252,136,164, 71,211,159,180,207,247,115,120,
    183,228,153,250,187,126,241, 17,253, 37, 95, 59, 84, 79, 79,202,
     60, 78,104, 30,232,170, 35,167,123,153, 35,  9,107,129,238,  8,
    200, 83,226,196, 79, 36,111,230,211,205,180,199, 13,166, 61, 39,
     31, 45, 61, 79, 48,112,175,218,244,252, 58,170,166,174,127,217,
     80, 85,179,198,121,153,194, 87,184,182, 71,176, 31,135,203,155,
     12, 46,184,173, 45,214,109, 17, 31, 84,195,130,109,201, 78,241,
     89,153,158,145, 62,190,190,168, 34,226,220, 82,159,132, 85,113,
     74,110, 50, 42,105, 41, 69, 29, 68,148,208,213, 84, 84, 74,  8,
    157,161,250,140,173, 97,104,145,133,218,153,123,122, 45, 39,142,
    179,104,172,215, 38,119,237, 17,246, 99, 28,182,138, 77,162,251,
     17,214,125, 38,102,125,255,  0, 95,207,242,123,252,141,204,220,
    123,142,243, 60,206,226, 76, 52,222,233, 68,209, 45, 43, 75,132,
     90,234, 36,241, 98,126,146, 79,155,193,176,191,205,115,121, 92,
     52,227,227,250,125,118,127,167,255,  0,174,175, 11,200,228,229,
    229,158,222,153, 95,111,227,235, 31,201,245,188, 82,126, 29, 75,
     65, 45, 79, 17,115, 25, 68,219,120,174,144, 93,128, 19,108,239,
    139,149,199, 74,204,207,211,238,244,121, 45, 90,215,109,236,248,
     42,110, 39, 73, 71,193,249,171,140, 82, 70,249, 56,100,149,113,
    138,119,210,188,192, 28,  0, 99, 28, 89, 35,  5,218,208, 78, 92,
    213,232, 79, 28,205,184,235, 63,139, 62,254,175, 42,188,149,173,
     57,111, 31,135,183,166,122,127, 52,252, 42,175,143, 87,112,206,
     98,143,135, 87, 72,231,240,243, 77, 85, 64, 98,168,158,118, 18,
    214,185,210, 70,217, 37,211, 36,140,126,130, 44,113,117, 92,149,
    165,109, 78,209,248,182, 39,210, 33, 28, 87,189,171,201,214,223,
    135, 38, 61,102,127,156,250,202,179,204,243,213,114,255,  0, 22,
    230,153, 77, 79,236,234,169,161,167,160,166,100,242, 64, 35,142,
     55, 53,175,151, 91, 46, 88, 29, 37,245, 16, 47, 97,101, 31, 34,
     34,245,227,244,216,137,153,244,214,159,226, 38,220,118,229,157,
    235, 51, 17, 17,185,251,255,  0,202,126, 21,197,120,149, 71,  0,
    230, 10,118,113,118,210,178, 41,224, 20,149,175,158,105, 34, 96,
    144,  2,230,182,162, 80, 37,210,235, 88, 59,167, 69, 92,148,136,
    189, 39,174,250, 78,198, 71,244,246, 71, 23, 37,167,143,146, 59,
    230, 76,100,236,255,  0, 89,245, 53,188,199, 90,238, 72,173,150,
    131,222, 32,240, 43, 89, 13, 85,100,117, 18, 86,218,  2, 90, 37,
    150,154,105, 46,235,  1,246,189,210,249, 49,243, 99,115,214, 61,
     35, 51,215,245,131,255,  0, 17,111,145, 61,118, 50,222,179,189,
    189, 62,243, 18,167,141,215, 80,183,147, 94,254, 92,226,213, 50,
    181,213,212,241,154,175,120,146, 71,176,189,204, 14,107, 94,242,
     77,172,110, 70,217, 81,197, 89,249,191, 93, 99,218,125, 49,167,
     53,235,242,127,203,180,207,213, 30,187, 47, 79,137,240,202, 78,
      7,192,107,253,247,140, 84,138,105, 29,174,145,243, 84, 72,201,
     99,147,195,248,  4,218,131,221,169,205,189,150, 84,228,158, 75,
    198, 86, 55,239,233,253,155,242,113, 71, 23, 29,187, 94,115,237,
    235,253,222, 12,252,198, 42, 57, 11,151,228,111, 22,146, 39,154,
    202, 90,126, 39, 87, 28,196, 72,198,185,143,241,  4,143,201,219,
     57, 93, 49,195,156,215,250,126,211,145,142, 59,121, 27,227,241,
    253, 95,238,136,153,210, 56,157, 61, 71, 18,228, 78, 96,150, 26,
    138,186,174, 27, 75, 80, 79, 11,149,242,202,239, 18, 32, 89,226,
     94,231,250, 88,218,111,164,186,234,233, 49, 94,106,123, 68,204,
    122,255,  0,223,178,121, 34,111,193,201,235, 51, 88,159,167,214,
    125,189, 63,230, 31,103,203,195,132, 79,194,196,156, 42,169,213,
    180,250,179, 43,230,124,231, 85,133,219,170, 66,227,142,203,205,
    231,139,118,250,163, 39,248, 99,215,241, 47, 78,191, 76,246,143,
    227,170,157,169,187,236,178,117,196,176, 62,197, 32, 45, 66,232,
     19, 13,182, 16, 68,201, 21,242, 19, 86,186, 34, 65,193, 79, 70,
     30,  6,161,157,208,137,  9, 30, 93,  7,238,145,235, 53, 57,187,
    228, 32, 76, 26,210, 31,178, 25,204, 23, 43, 19, 74, 73, 99,179,
    189, 83,133,193, 78,  4, 27,148, 41,205, 61,209,128,108,144,181,
    222,137, 76, 20,194,139,218,201, 34, 96, 78,200, 78, 19, 48, 29,
    175,100,229,156,192,245, 27, 37,136,234,238,200,195, 42, 88,238,
     16,168,148,229,164, 20, 74,198, 30,111,101, 37, 48, 60,161,148,
    213,153,  7,117, 90,202, 96, 97,214,  3,247, 83,212,117, 17,127,
    226, 31, 18, 99,168,244,222, 32, 83,211,234,158,102,183,162,157,
     29, 83, 27,  2,158,148,209,204, 57, 79, 88, 90,131,189,240,170,
     37,156,208, 14,141, 83, 30,172, 44, 32,108,141, 41,161, 15,131,
    183,213,168,214,115, 84,178, 82,131,117,113, 44,166,133,178, 34,
    215,250,126, 73,204,167,170,200,226, 83,165, 53, 86,200,240, 46,
     48,166, 89,202,186,102, 22,218,219, 40,144,244,169, 93,110,254,
    169,196,148,189, 90, 95,235, 26,180,137, 78, 61,106, 83,103,  3,
    117,173,101, 47,122,157,214,109,206,221, 10,214, 36, 40, 18, 96,
     93,105,216,134,218,151,233, 57,245,  5, 79, 99, 42, 73, 11,156,
     84,204,128,196,226, 31,146,157, 36,150,107, 93, 17, 32,151, 73,
    111, 45,212, 77,134, 16, 95,165,227,215,170,206,100, 98,166, 29,
     76,244, 90,197,137, 36,143,208,251, 39,  4,150,162,161,206,106,
     38, 79, 18,182, 66, 31,233,213, 78,140, 50,250,143,240, 90,214,
     73, 52,146,220,167,164, 81,126,202,123,  6,200, 78, 10, 90,101,
     18, 84,232,  1,113,186,142,193,248,203, 31,147,126,159, 15,101,
    217, 47,173, 80, 51,218,253, 18,  6,181,224, 11, 56, 91,177,191,
    251,169, 92, 43,134, 82,211,228,112,244,220, 41,193,238,190, 30,
     34, 71,245,131, 61,210, 69,184,213,182, 74,105,139, 11,218,201,
     44,110, 53,  0,109,249, 38,206,104,178, 57, 35,112,176,232,164,
    196, 99,105,102,146,  1,111, 80,129,165, 54,134,146, 35,120,162,
    107,  9,220,180,  1,250, 37, 34, 61, 12,177, 30,170, 84, 92,177,
     71, 32, 32,182,247,220, 29,146,195,137, 37,176, 69,  7,150, 38,
      6, 15,236,139,126,137, 76, 46,184, 38,203,167, 99,148,141,186,
    218,253,247, 79,208,  2, 72, 65,200, 83, 48,186,216,153, 35, 45,
    221, 44, 92, 88,188,165,213,122,252,255,  0,159, 56, 71, 21,170,
    231,206, 21, 83, 77, 73, 52,212,209,182,156, 73, 43, 35,115,152,
    219, 84, 60,157, 78,  2,194,192,221,118,112, 76, 71, 28,195,196,
    248,143, 21,237,228,210, 98, 38, 99,211,250,191, 64,130, 40,226,
     36,196,198,176,159,139, 72,  3,244, 92,126,175,103,172, 29,226,
    179,103,  4,147,213,206,134,  9,129,183, 93,199, 69, 67,182, 16,
     40,219, 78, 52,196,208,214,246,  2,223,162, 83, 10,172,192,155,
     35,199,201, 24,179, 91, 32,213,229, 56, 77, 51, 83, 99,156, 29,
    194, 88,137,161,154, 91,184,199,170, 88,150,  7, 56, 13,210,194,
     57,146, 30,182,245, 64, 50, 57, 27,123,108,167, 19,134, 53,253,
     63, 68,129,172, 61, 58, 36, 76, 44,220,180,220, 35,  0, 90, 67,
     54,217, 35, 15, 16,225,212,156, 87,135, 79, 65, 86, 47, 79, 80,
    199, 71, 43, 90,108,116,184, 88,228, 39, 91, 76, 78,163,146,189,
    163, 39,218, 94,119,  8,228,170, 14, 29, 93, 29,105,158,122,186,
    138,120, 61,218,153,213, 14,105,240,161,253,214,  6, 53,131,166,
    230,229, 87, 39, 36,218, 51,219, 89,241,112,214,147,187, 51, 49,
     25,234,244,165,165,177,212,213,203, 60,110,184,228, 35, 65, 10,
    113,125,156, 65, 63, 14,202,112,220, 28, 54,252, 72, 83,139,  0,
     22,211,111,146,144,209,158,168,  2, 99,136, 64,145,249, 92, 45,
    100,210,194,205, 59,108,140, 48,150,106,237,116,143, 93,163,  8,
     14,181,146, 48, 16,  0,187,112, 83, 14,107,238, 82, 52,220, 87,
    131, 81,241, 79,117,247,173, 86,164,169,142,174, 45, 38,223,210,
     69,125, 55,222,227, 43, 78, 62, 89,166,231,222, 49,143, 47, 12,
     95, 55,237, 58,189,160,127, 37, 45, 24, 90,222,152,  9, 18,121,
     99, 66,225, 52,144, 54,232, 92, 88,151, 54, 86,159, 55,221, 11,
    244, 50, 41,110,166, 97, 51,  6, 28,238, 19, 34,222,219,100, 38,
    112, 56,200,199,116, 20,192,156, 15,209,  9,102,128, 70, 14, 82,
     56,150,  6,  3,232, 80, 24,226, 70, 63, 52,203, 10,151,204, 19,
      9,156,108,108,224,153,192, 28, 70,255,  0,100, 47, 28, 14,108,
    119,232,129,138, 90,235, 27,164,137,131, 64,  5,182, 67, 57,118,
     65, 66, 38,  6,193,168, 95,183, 69, 76,229,218,113,107,231,178,
     82,  3,110,133, 73,129,209,  3,253,228, 42, 36,137, 25,111,154,
     26, 64, 11,156, 54,250,160,166,172,116,165,216,236,132, 77, 28,
    217, 92, 47,127,133, 54, 83, 65, 50, 87,222,225, 50,234,172,236,
    130,138,147, 54, 90,165, 93, 83, 75,123,  0, 83, 76,193, 99,  8,
     68,192,154,229, 90,202,104, 96, 23, 85,172,102,162,209,235,114,
    141, 76,195,140,  1,195, 29,247, 70,162,106, 76,180,217, 35,175,
    117, 81,102, 83, 68,198, 35,171, 31,146,122,143,150,166,150, 35,
    215,174,232,148, 77, 85, 68,204,122,250, 40,214, 51, 85, 17, 71,
     97,140,223,178, 17,138,162,104, 14,206,123, 32,158,181, 35,134,
     47,155,254, 29,213, 85, 47, 94,152,255,  0,178,218,  9,236, 67,
     37,226,  9,196,146,129, 40, 35, 79, 82,175,177, 96, 29, 38, 61,
     66, 93,143, 28,217, 17, 18, 48,113,184, 18,157,100,176,246, 60,
    133,181,100, 21, 81, 46,151, 99,243, 83,105,  5, 74,251,142,202,
     38,198,162,  9, 60,190,171, 90, 89, 56,155,136,108,  8,216,254,
    168,223, 81,143, 61,242,157,207, 68,105,151,  9,214, 73,251, 42,
     35, 53, 27,220,124,138,173, 36, 83, 75,230, 61, 18,236, 88, 15,
     16,234, 81,216,207, 46, 15,106,169,144, 18,194,126, 74, 38, 79,
      9,214,  6,233, 19,241,102, 17,176, 63,162,239,125, 79,115,216,
    238,155,132, 46, 44,162, 41,180,224,225,163,178,153,133,196,158,
    207, 13,227,202,237,186, 36,179, 26,231,183,252,212,142,166,197,
     59,129,193,207, 68, 38,104,174, 14, 36,225,253,104,192,234,150,
     38,120,215,211,214, 53,238,242,188, 91,123, 36,137,170,150,204,
      8,243, 99,253,118, 66, 48,205,198, 17,138,137, 41,206,120, 59,
     36,160,151,176,155, 57,  7,129,125, 59, 95,240,110,166, 97, 80,
    157,236,153,133, 78, 43,176,155, 51,129,179,144, 48,198,185,175,
     24,253, 17,128,183,194, 59,101, 24,168,177, 47, 97,106, 83, 13,
     43, 37, 25,244, 97,216, 29,194, 74,193,  9,163, 61,173,216, 32,
     55,123,233, 37, 60, 35, 88,247,143, 43,178, 16,137,134,190,  6,
     56, 18, 18,193, 91,167,247,121, 69,139,111,126,157, 80,210, 44,
    239, 18, 75,249,135,213,169, 25,209, 84,150,245,212, 16,153,170,
    166, 62, 57, 27,141,209,140,230,173,  5,205, 25,200, 75, 19,134,
     71,165,254,107,229, 29, 83,163, 58,219,190,202,122,141,104,144,
    182,202,112,240,198,212,227,228,144,193,120,172, 63, 52, 23, 81,
    121, 78, 65,179,145, 48,  6,201, 73, 57, 58, 74,146,195,153, 47,
    239, 11,132,203,  1, 37, 60,110,203, 62,202,102,167, 22, 36,211,
    183,107,233, 42, 38,141, 34,236,125, 37,198,217,238,179,154, 42,
     46,152,135, 71,143,213, 70, 52,137, 21,175,158,169, 27, 66,  0,
    218,109,252,147, 41, 29,131,173,100, 19, 11, 77,239,100,139, 92,
    133, 51,123,116, 72,216,230,246,202, 66,  1,160, 55,100, 27, 69,
    186, 32, 55, 87,221, 26, 27,171,161,221, 50, 99,173,182,229, 26,
     53, 60,209,161,113, 41, 36,115,181,100, 33,112, 27,180,252, 56,
     41, 98,140, 99,241,165,200, 76,181,192,219,203,178,  4,  3, 32,
    229, 51, 52, 56,105,206, 80,142,160,117,198,127, 36, 27, 90, 70,
    232, 41,113, 35,232,153,147, 40, 58,176,108,222,233,  8, 37,206,
      7, 14,202,165, 97, 69,132, 52,233,219,162,102,198,218,254,169,
      5, 44,200,217, 36, 73,241,155,182,197, 52, 99,139,173,131,178,
     19,141,140,246,232,156, 34,106,204,132, 39,  2,227,148,149, 16,
     91,158, 65,191,125,146, 84, 84,178,251,159, 84,149,129,177,186,
     12, 15, 25,178, 10, 66, 16,156,108,103,204,155, 59, 85,101,238,
    126,136, 68, 23, 35,176,133, 17, 55,196,141, 24, 11, 34, 17,213,
    160,121,172,154, 38,163,104,179,172,237,145,172,102,166, 53,164,
    238,113,209, 61, 68,212,232, 25,118, 14,227,178, 76,237, 81, 73,
     16, 45,191,228, 48,146, 34, 17, 75,  7,155, 31, 85,113, 98,147,
     41,216,  0, 14, 29,112,111,132, 76,162,212,244, 80,219,  3,253,
    187,116, 83, 50,202,220, 94,138,225,  5,221, 69,172,169,205, 53,
    147,226,140,156, 15,162,114,140, 95, 75,146,  5,173,165, 20, 43,
     67,214,132,173,117, 24,244,224,155,202,221,174,141, 24,111,143,
    231,206, 26, 85, 68,134, 79, 40,232,112, 55, 74,210, 25, 79, 48,
    211,186, 34, 79, 13,141,228, 59, 63, 18,168,148,158,201,199,226,
     90, 69,139,  1, 85, 43, 92,219,157,130, 38,195, 19,178,124,249,
    143,201,103, 50,120,112,159, 23,  5,105, 73, 44, 46,105,154,248,
    205,241,139, 42,210,121,243, 58,205, 39,102,132,104,101, 51,218,
    112, 54,244, 78, 36, 27, 53,180, 27,141,134,234,244, 60,169, 30,
     92,229, 58, 49,161,254, 95, 84,180, 97,180,242,147, 38,151, 12,
     39,161, 67,157,191,232,166,100, 66, 55, 88,187, 40,211,199,226,
    247,105, 55,181,136, 94,147,232,189, 70,215, 19,139,249, 71,228,
    131,236,108,115, 17,135, 37,138,211,227,120, 39,202,108, 78,112,
    148,195,104,177,204,169, 63,136, 95,229,209, 78, 52,137, 61,178,
    196,243,141,253, 55, 75, 21,130,  5,183, 55, 56,245, 72,240, 77,
     46,221,167,110,201, 22, 31, 15, 18,158, 60, 19,113,219,127,213,
      9,235, 10,225,227,  3, 23,242,221,  9,249, 75, 25, 93, 19,199,
    203,126,169, 98,122, 75,140,176,187,225, 54, 40,195,201,  1,238,
    215,  2, 79,170, 88,160, 58,161,251, 28,252,194, 48,117, 37,243,
     54,247,235,233,254,105, 98,162,161,247,176, 13,180,255,  0,139,
    249,163, 21,212,214, 87,198,252, 59, 31,154, 48,186, 29,170, 39,
     12,156,122,165,137,245,132,243,210,191, 38, 35,246, 75, 26, 86,
    232,165, 15,105,177,184, 35,161, 70, 54,137,214, 50,166,219,227,
    242, 64,234,115,106,136,234, 15,168, 70, 38,106,162, 42,224, 62,
     93, 81,140,230,138, 89, 44, 15,  3,107,254,104,196,122,192,101,
    108, 14,235,132,176,226,242, 83,161,112, 55,110, 66, 49,164, 89,
    141, 58, 78,173,146, 30,231,199, 82,219,100,229, 36, 77, 36,219,
    141, 87,111,228,134,106, 34,152,237,186,120,153, 80, 27, 27,207,
    238,165, 53, 78,225,114,210,124,254,138, 39,141, 85,228, 78,230,
    202,223,228, 55, 81,141, 59,107,132,207,106, 15,169,209,212,220,
     91,245, 73, 61, 78,142,161,135,  7, 14, 70, 32,246, 72,210, 71,
     66,140, 72,156, 54, 75,  3,  6, 18,197,  9,208,197, 56,243,  0,
      8,194,139, 82,  4, 94, 97, 28,212, 50,196,110, 62,203, 41,164,
    195,122,242, 68,149,149, 11,104,147, 63,228,140,  2, 18, 14,136,
    209,134,181,205,114, 69,142, 62,191, 68, 19, 12,119,202, 12, 36,
    116,184, 72,226, 65,190, 59, 36, 97,207, 68,212,192, 74, 64, 90,
    175,132, 65, 55, 85,134,155,166,  2,244,  4,210,198, 58, 96,161,
    112,142, 86,145,234,156, 53,171, 35,155, 57,221, 18, 38, 22, 69,
    145,243,232,147, 57,244, 12,144,144,112,131,139,  1,183,  6,196,
     32, 55,204, 10,  3,122, 32,  6,228,108,131, 99,252,204,254,  9,
    148, 37,112,  1,214, 55,183,116, 24, 11, 92, 29,182, 19, 48,129,
    123,160,212,193, 37,133,144,137,131,175,212,108,165, 37,200,254,
    187,142,200, 24,232, 39,201,253, 21,162,213, 53,196, 28,245,234,
    134,108, 36,237,247, 73, 65,123,  6,155,132,142, 19, 16, 75,189,
     83, 89,141,109,254,105, 19,157, 30, 18,  4,202,205, 45, 76, 65,
    108, 34,225,  5,101,144,155,140,162, 24, 76,  1,205, 36,250,162,
     85,  4,200,146,152,198, 31,228,132,225,162, 28,220,161, 51,  2,
     12,242,220, 36,206,106,214,122, 12,254,104,236,139, 80,232,237,
    211,234,170, 88,218,166, 37,172,240,169, 98,235,116,198, 50, 54,
     11, 99,232, 18,214,118,168,182, 55,234,141, 68,213, 93, 27,177,
    140,247,217, 61,115, 90,139, 88,208,118, 40,214, 51, 10,163,  5,
    174, 10,161, 19,171,162,144,108,175, 75,170,186,105, 79,217, 61,
     24,172, 72,210, 48,156, 73, 97, 83, 74, 50,  6,221, 82,236,117,
    128, 82, 78, 53,233, 39, 29, 17, 89, 41,122, 68, 19, 13,219,185,
    236,181,102, 92,178,145,229,217, 43, 73,167,247,146,224,119,191,
    100, 69,142, 96,151, 84, 88,246, 79, 72,198,205,113,117, 81, 36,
      7, 78, 69,251, 39, 50, 73,223, 54,166,144,141, 24,234, 89, 45,
    230, 37, 62,195, 20, 77, 48,181,187,170,236, 30,108,130,206, 83,
    166, 16,108, 55, 78,178, 65,166,151,254, 96,229, 19, 39,143, 65,
    153,143,215,186, 90, 68, 22, 18,112, 21, 27,241, 59,  1,184,254,
     11,210,125,  8,129, 98,103,  3,104, 35,107, 97,  6,107, 73,205,
    254, 91, 33, 81, 39, 52,254, 65, 74,198, 31,113,159,243, 66,187,
     13,149, 14,  4,155,223,211,241,126,137, 76, 53,139, 64,219, 48,
    205,176,123, 29,212,226,245,166, 71,119, 75, 21, 14,241, 78,251,
    132, 12,107,103, 44, 56, 36, 32,250,169,139,137, 56,124, 71,  9,
     39,162,134, 86, 95, 98, 10, 69,213,166,172,245,217,  7, 21,  8,
    158, 39, 96,231,174, 82, 62,178,231,120, 14, 22,218,253,144, 69,
     58, 55,183,173,251, 33, 81, 33,241,228,103,249, 32,253, 36,214,
     87,187, 79,175,170, 19, 52, 52,213, 67, 32,211, 32, 66,114, 99,
    216,185, 41,224,147,225, 63, 85, 51, 11,139,201, 38,153,251,177,
    215, 77,127, 50,  2, 91, 56,205,178,146,189, 28,218,169, 24,122,
    139,102,233, 23, 88,149, 17,241, 65,248,243,126,169,166,120,212,
    178,170, 34, 48,122,108,118, 66, 38,166,104,107,141,217,190,222,
    168, 47,111,112, 22,188,  2, 59, 31,178,156, 61, 50, 57,220,195,
    143,178, 48, 77, 79,142,161,189,243,234,134, 83, 69, 12,169, 22,
     67, 62,138, 99,169,  8,196,205, 69, 35, 88,241,169,182,112,234,
     58,165, 48, 34,112,135, 48, 94,202, 58, 46, 46, 91,161,185,184,
     63, 69, 56,174,194,107, 74, 88, 90,108,110, 34,201,239,217, 39,
    138,139, 12,236,150, 96,195, 11,241,125,199, 69, 18, 77, 99,236,
    127, 84,137, 64,124,114,139, 20, 22, 38,168,162,234,211,242,178,
    206,220,127,147, 90,242, 35,145,142,110, 14, 10,203, 27,118,208,
    219, 10,102, 21,161,187,250,103,248, 41, 56,111,143,251,219,247,
     76,250,136, 74,  6,253, 17,165,141,241, 90, 91,253,158,136,  1,
     36, 91, 39, 40,  1, 62,191,249, 36,173, 97, 40,  2,109,144, 26,
    111,209, 32,219, 13,144,  0,246,227, 34,232, 26,150,106,113,108,
     39, 11,139, 38,116, 86,117,211,105,216,248,141,188,201, 34, 97,
     64,146,227, 61, 80,137,  3,163, 29, 16,122,203, 95,  5,  3, 92,
     90,225,190,201,141, 41,192,  7, 92, 36,112, 11,185,183, 13,234,
    154,152, 90,199,217,167,116, 23,176, 61,217,219,140,132,244,104,
     12, 25,198, 17,170, 49,141, 70,166, 76,143,123,116, 72,181,178,
     67,140, 32,105,  6,  3,124,253,211, 57,245,113,123,190, 99,175,
    116, 51,198,151,234, 29,136,221,  5,141,241, 44,212, 30,  2,247,
    202, 12,200,183,243, 36, 12,235,100,146, 25,163,  4, 38, 33, 25,
    140,135, 20, 98,141,128,144,212, 75, 57,168,200,252, 65, 49,137,
    228, 97,190, 62,168, 51, 34,104,232, 16,133, 81,180,108,165, 34,
    116,105, 22,  0, 71,189,144,153,115,153,129,221, 26,206,104, 96,
    117,242,127,205, 45, 99,106, 52,109,110,234,161,158, 51, 77,177,
    217,102, 48, 37,170,145, 53, 54,154, 66, 29,164,231,176, 77,143,
     37, 94,156, 36, 96,132,220,150,170,182,140, 99,114, 85,106,122,
    152,194,126, 98,233,196,142,170,105,228,180,151,235,213, 57,148,
    245, 89, 28,226,214, 61,  7,100,251, 23, 82,229,200, 29,148,216,
    235,  4, 50, 67, 27,238, 59,225, 21,148,204, 61, 74,106,219,183,
     39,230,183,173,145, 53,100,207,105,243, 55,127,201, 41,  9,117,
    217,246,252, 39,236,167, 76,151,155, 56,252,209,217, 56,223, 16,
    180,119, 85,219,  0,101,144,147,133,123,160,151,142,255,  0,205,
     56, 60,108, 78,242,231,162,153,178,112,219,131,110,234,123,  0,
     74,211,209, 84, 73,163,148,158,153, 87,176, 69,197,229,125,251,
    163, 78, 86,251,205,172, 50,171, 72, 96,184,228, 37, 50, 79,200,
    159, 89, 79, 38,175, 17,150, 61, 44, 48,189,103,185, 20, 34, 65,
      3,143,147,  3,178, 26, 87, 75,211,109,179,242, 67, 88, 28,114,
    141,157,240,246, 66,176,248,222, 29,114, 62,129, 73,224,133,142,
    119,178,  3,142,173,199,254, 40, 84,  4,191,240,157,186, 33,164,
     59,196,232, 47,111,154, 26, 68,183,198,114, 88,166,248,236,235,
    186, 88,166,137, 79, 71,  3,232,167, 21, 18,223, 25,237,206,200,
    197, 25, 31, 16,113,193, 42,102,  7, 67, 29, 83,168,102,223, 84,
    134, 73, 38,169,237, 61,109,232, 83,195,234,209, 90,118,189,130,
     48,117,107,170, 73,  3, 55, 64,234,  3, 82,208,235,237,234,129,
    208,216,248,155,110,117,228,122,161, 63, 45, 83,106,225,127,249,
     36,137,172,140, 72,215,117,219, 96,154, 70, 92,235,119,186,156,
     56, 46, 66,203, 89,201, 99, 72, 74,251,  3,116,214,230,203, 35,
     54,217,  0,248, 56,131,154,114,132, 77, 22, 71, 94,199,225,223,
     68, 35,161,218,245,236,126, 88, 73, 33,247,129,139,164,  4,218,
    166,139,157,121, 27, 32,241, 76, 85,120,  0,253,208,202,220,107,
     99,169,252, 64,219,213, 24,202,104,111,146, 86,146, 48,244, 39,
    216, 36, 16,123,233, 82,122,224,209,212, 96,165,212,244,108,202,
     48,180,207,  1,164, 37,137,214,105,124,121,236,167,169,232,155,
     32,237,146,151, 83, 48, 91,112, 84, 77, 70,156,199,191, 79,113,
    217, 44, 38, 73, 14,166,220,109,212, 31,224,179,154,157,109,137,
     31,  6,147,123, 99,213,103, 53,111, 89, 37,208,147,134,239,213,
     78, 42, 44, 83,226,112,219,236,166, 90,105, 97,238,248, 95,135,
     36, 98,107,254,137,140, 22,171,139,132, 37,193,216,183, 80,150,
      6, 52,132,148, 54,186,199,100, 16,198, 74, 65,186, 65, 40, 26,
    227,111,154, 99, 75,146, 32,239, 66,129,169,166,138,223, 52, 46,
     36,145,229, 54, 66,205, 14,  4, 32,166, 26, 29,220,217,  9, 48,
     88,140, 32,142,  4, 27, 95,160, 66,101, 60,237,105,220, 99,209,
     53, 68,167,115, 92,195,141,187,167, 11, 13,192, 62,168, 49, 50,
     75, 20, 98,112, 68,  3,245, 70, 12,102,130, 62, 91, 32,221,107,
     27,141,210,  2, 14, 32,101,  4,207,135,165,238,130, 34, 70,121,
    176,153,148,246,227,228,169, 32, 50,244, 40,149, 68,  9,143,191,
    201, 35,154,171,129,160,169,103, 38, 24,236,114,144,208, 77,229,
    217,  5,  5,150,130, 61, 80, 97, 12, 35, 61, 19, 38,145,248,135,
     68,200,171, 93,215, 27,148, 40,214, 69,100,145, 38,198,227,170,
    199,100,165, 19, 10, 52,135, 59,108, 36, 76,116, 73,  7, 58, 43,
    183, 24, 74, 18, 67,176,154,102,131,137,218,182, 67,158,244, 25,
      0,139,141,210,148,  7,230,145, 54, 54,  0,253, 64,252,173,252,
     81,172,237, 87,163, 76,252,  3,183,162,113, 46, 75, 85, 83, 95,
    123,101, 86,167,169,237,113,232,115,243, 76,117, 81,  9,234,146,
    102,  2,249, 44, 69,183, 83, 50,113,  2,101, 77,240,228,226,218,
    153,163,164,179,157,229,250, 21, 82, 88,216,106, 60,192,126,125,
    209, 22, 46,170,132,226,230,251, 43,236,137,132,230,107,186,221,
     17,217, 56,  9, 31,114,151,100,204,  2, 73,136,104, 78, 36, 96,
      4,247,254,107, 90,200,195, 65,191,243, 87,163, 25,167, 59,101,
     76,131,  1,182, 22, 82, 88,208,224,110, 22,149,176, 76,248,237,
    186,211, 72,146, 60,218,148,200, 20, 96,151, 99,111, 84,226, 74,
     85, 19,167,  3, 33,105,164,252,126,106, 83, 99,101,235, 61,186,
    242,167,115, 11, 79, 91,161,183,102, 92,247, 73,113, 34,212,134,
    145, 34, 15,183,201, 11, 53,181, 14,239,112,130, 24,157,135,209,
      3,168,252,183, 72,224,183,199,156,125,144,214,161, 62, 35, 91,
    159, 50, 21,  1,108,130,217,184, 33, 41, 83,114,126, 72, 86,179,
    196,145,187, 21, 43,137,113,147, 86,232,149,235, 60, 87, 13,138,
    147,115,106, 53, 97,193,  0, 94, 83,144,128, 27, 63,112, 82,195,
     11,188, 75,225,  1,154,157,109,146, 54,120,164, 28, 32,148, 69,
     95, 35,119, 40, 76,209,108, 92, 74,248,114,105,232,165,181, 12,
    119,148,155,250, 35, 19, 49,140,125, 51, 93,240,156, 36,113, 36,
    186,  7,131,132,149,160,116,100,110, 16, 65,108,165,167,178,102,
    124, 85, 79, 27, 20, 22, 43,134,173,196,103,170, 49, 51, 86,151,
    177,194,223,146,156, 34,203,228,105, 54, 56, 73, 89,  7,195,196,
    167, 97,244,245, 77, 22,227,122, 16,241,  6,190,195,175,100, 49,
    183, 18,182, 85,196,239, 43,146,101, 53, 53,133,195,225, 55,106,
     88, 70, 96,165, 37,174,110,176,125, 59,164,103, 49,247,221,  9,
    199, 24,219,109, 77, 56,234,144,215, 52, 17,150,253,146,195, 49,
    146,141, 94,108,122, 41,152, 37, 17,204, 10,142,165,131,116, 81,
    187,113,245, 83,212,108,165,154,148,183, 55,250,168,154,181,139,
    146,232, 73,234,179,234,211,178, 89,224, 35,166, 20, 77, 90,214,
    228, 55,202,235, 57, 74,196,  6, 60,191,100,131,134,174,202,136,
     90, 90, 77,198,232, 33,  0,150, 13, 23,200,164, 96,115,220, 13,
    156, 16,  4,217, 58,116,236,128, 59,130,144, 42, 70, 14,249, 70,
    156, 74, 57,163, 35,230,156, 52,137,  3, 94, 83,197, 96,219, 38,
     82, 35,163,233,223,186, 17, 42, 26,219, 31,154, 19, 44,149,130,
    217, 64,137, 75, 35, 91,109, 39,232,169, 80,158, 72,201, 22,186,
    106,208,130,111, 97,139, 39,134, 38,203, 99,156, 41,192,124,111,
    212,  8, 41, 37,214, 72, 55, 65,  3,184, 64,  3,129,178, 96,146,
    168,195,164, 17,252, 82, 34,167,135,178,122,112, 76,122,131,191,
     84, 42, 94,157, 43,131,155,234, 20,203, 41, 60,150,223, 41, 36,
    169,156,203,236,156,158,  3,201,175, 10, 67, 11,  0, 63, 53, 73,
     41,231, 65,183, 78,200, 55,104,243, 93,153,  8, 51,155,219,186,
     19,140, 44,210, 80, 38, 20, 68,255,  0, 42,150,115,  6,238, 44,
    140, 75, 28, 44, 17, 33, 44,237, 35, 41,106,176, 48,159, 53,143,
    116,153, 94,167,219,170, 28,248, 95,141,165,246,216,160,186,142,
     51,119, 91,112,147, 59, 66,200, 78, 45,254,135,205, 38, 54,161,
    209,202,236,  7, 97, 92, 23, 69, 48,186,237,  5, 86,148,213, 99,
     94, 45,131,139,101, 41,101, 53, 38, 87,245, 89, 89, 81, 82, 99,
    152,247, 31, 36,232, 58, 25, 28,142,105,199, 81,149,105,154, 57,
    164,147,117,158,167,170,134,204,237, 36, 21,125,139,162,119, 62,
    206,245, 83, 54, 79, 71,120,219,223,  9,214,200,154,151, 36,206,
    232,180,137, 79, 87, 68,252,128, 86,177, 41,154,172,189,128, 90,
    104,195, 88,230, 59, 61,118, 11, 62,195,171, 36,115,113,209, 45,
     60, 47, 93,221,125,172,158,167,169,115, 75,173,222,138,187, 16,
     59, 34,108,156, 54, 54, 20,226, 79,  4, 77,202,211, 74, 97,249,
    131,128, 63,  9,207,101,237,187,226,196, 62, 11,231,170, 77, 98,
    233,100,135, 73,202, 77,171,114, 92,210, 51,209, 38,181,151,106,
    251, 33,174,184,250,236,133,196,184,185,192,219,123,143,129, 10,
    115,106, 28,199,184,125, 16,120,107,106,205,172,118,189,174,128,
     96,120, 62,168, 92, 75, 13,136,219,117, 43,172,128,  6, 27,250,
     33,164, 48,180,164,168,144, 23, 75,190,254,137, 31, 98,204,253,
    198,201, 46, 29,239, 12,182,200,  2, 19,142,134,201,  3, 27, 58,
      1,130, 92,108,153,177,217, 26,182, 72, 18,226,  3,173,213, 32,
    203,158,216, 64, 27,100, 45,217,  0,248,170,172,125, 59,160,177,
     76, 85, 78,181,194, 19,213, 67,106,156,236,148,134, 50, 73,  9,
     65, 19,226,  2,237, 36,116, 76,240, 97,141,233,219,112,132,232,
    180,190,246, 65,232,217, 59,155,142,137,  3, 67,155,164,159,186,
     88, 49,197,151, 23,101,172,150,  0,  7,184,100,109,213, 24, 48,
    248,107,100,107,109,241, 11,245, 73, 63, 46, 22,195,196, 72,182,
    159,178, 25, 91,137,233, 65, 92, 72, 29,125, 10, 24, 91,141, 91,
     31, 27,182,221, 44,102,226,251,100,124,148,170,  5,226, 11,217,
      7,141, 15,185,183, 84,139, 12,109,157,182, 74,146,119,153,153,
     25, 64, 58, 58,139, 12,169,194, 80,199,234, 30,189,148,204, 22,
     96, 36,166,197,217,186,206, 96,251, 16,246,  3,135, 11,168,150,
    145,100, 51,197, 98,162,106,222,182, 40, 55, 61,148, 99, 70,156,
    117,194, 48, 48, 11, 31, 84, 28,193,129,183, 25,220,161, 14,179,
    154, 45,209, 35,113,243, 55,100,129, 96,244,221,  7, 34,105, 35,
    100,  0,187, 81,200,217,  6, 75,198,114,133, 39,149,129,174,238,
    154,226, 88, 29,132,149, 39,196,237, 54, 77,156,174,136,180,128,
    118, 41, 51,152,107,242,222,229,  4,150, 88,199,248,147, 92, 74,
    103,239,101,112,101,185,133,174, 76,244,108,136, 56,250,164,122,
     54,192,225,127,222, 82,157, 99,222, 70, 16, 32,200,157,169,182,
    110,123,169,146,159,119, 22,139, 32, 16,246,142,137,172, 27, 19,
    233,186,160,199,121,153,234,140,  9,195,111, 33, 29, 80,168, 85,
     76,219,  3,223,186,153, 68,156,199, 13,146, 34,170, 47,168,126,
    104, 84, 64, 99,126,115,178,  4,192,220,227,240,156,246, 77, 24,
     84,142,176,185,250, 32,197,  1,207,167, 84, 20,156,224,132, 51,
    173,138, 96,113, 95,232,150, 38, 84,197,156, 36,156, 50, 77,173,
    213, 76,148, 36,122, 75, 32,224,237,158,168,103, 48, 33, 53,240,
     16,137,161, 83,187,204,154, 51, 13,128,157,143, 79,245,141,212,
    178,180, 46,102,205,190,127,116,110,165,142, 26,210,239,136,140,
     38,125, 84,194,108,158,149,170, 51, 55, 78,137,246,103, 52, 17,
    118, 49,249, 37,165,137,188,218,178,167, 79, 13,241, 46,194,123,
     39,168,147, 41,164,184,211,248,148,166, 96,205, 77,  2,232, 44,
     41,198,253, 18, 76,193,101,224,139,132, 84,186,146,101, 14,147,
     79,226, 27,140,173, 98, 83, 52, 53,174, 27,141,214,177, 40,199,
    161, 13,141,175,244, 79, 83,212, 68,216,139,119,194,207, 87,212,
     19, 63, 31,162,122, 88,159,198,118,171, 35,177, 77, 93, 43,192,
     56,222,246, 90, 51,195, 24,108,236,163, 75, 20, 51,204,228,198,
     52,  3,209, 29,143, 31,255,217,
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
    119,105,100,116,104, 58, 32, 57, 48, 48,112,120, 59, 10,  9, 98,
     97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,114, 58,
     32, 35, 70, 70, 70, 70, 70, 70, 59, 32, 10,  9,112,111,115,105,
    116,105,111,110, 58, 32,114,101,108, 97,116,105,118,101, 59, 10,
      9,109, 97,114,103,105,110, 58, 32, 48, 59, 10,  9,112, 97,100,
    100,105,110,103, 58, 32, 48, 59, 10,  9, 98, 97, 99,107,103,114,
    111,117,110,100, 58, 32,117,114,108, 40, 34,105,109, 97,103,101,
    115, 47, 98, 97,110,110,101,114, 46,106,112,103, 34, 41, 32,110,
    111, 45,114,101,112,101, 97,116, 32,116,111,112, 32,108,101,102,
    116, 59, 10,125, 10, 10, 10, 47, 42, 10, 32, 42,  9, 84,111,112,
     32,110, 97,118,105,103, 97,116,105,111,110, 10, 32, 42, 47, 10,
    100,105,118, 46,118,101,114,115,105,111,110, 32,123, 10,  9,102,
    108,111, 97,116, 58, 32,114,105,103,104,116, 59, 10,  9,112, 97,
    100,100,105,110,103, 58, 32, 55, 48,112,120, 32, 49, 48,112,120,
     32, 48, 32, 48, 59, 10,  9,102,111,110,116, 45,115,105,122,101,
     58, 32, 49, 49, 53, 37, 59, 32, 10,  9, 99,111,108,111,114, 58,
     32, 35, 48, 48, 48, 48, 48, 48, 59, 32, 10,125, 10, 10, 47, 42,
     10, 32, 42,  9, 84,111,112, 32,110, 97,118,105,103, 97,116,105,
    111,110, 10, 32, 42, 47, 10,100,105,118, 46,109,101,110,117, 32,
    123, 10,  9,102,108,111, 97,116, 58, 32,108,101,102,116, 59, 10,
      9,112, 97,100,100,105,110,103, 58, 32, 55, 48,112,120, 32, 48,
     32, 48, 32, 50, 50,112,120, 59, 10,  9,102,111,110,116, 45,115,
    105,122,101, 58, 32, 49, 49, 48, 37, 59, 10,  9, 99,111,108,111,
    114, 58, 32, 35, 69, 69, 69, 69, 69, 69, 59, 10,125, 10, 10, 47,
     42, 32, 78,111,110, 45, 73, 69, 32, 42, 47, 10,104,116,109,108,
     62, 98,111,100,121, 32,100,105,118, 46,109,101,110,117, 32,123,
     10,125, 10, 10, 47, 42, 32, 73, 69, 32, 42, 47, 10, 42, 32,104,
    116,109,108, 32,100,105,118, 46,109,101,110,117, 32,123, 10,125,
     10, 10, 47, 42, 32, 83, 65, 70, 65, 82, 73, 32, 42, 47, 10, 98,
    111,100,121, 58,102,105,114,115,116, 45,111,102, 45,116,121,112,
    101, 32,100,105,118, 46,109,101,110,117, 32,123, 10,  9,116,111,
    112, 58, 32, 54, 49,112,120, 59, 10,125, 10, 10, 10,100,105,118,
     46,109,101,110,117, 32, 97, 58,108,105,110,107, 44, 32,100,105,
    118, 46,109,101,110,117, 32, 97, 58,118,105,115,105,116,101,100,
     32,123, 32, 10,  9, 99,111,108,111,114, 58, 32, 35, 70, 70, 70,
     70, 70, 70, 59, 32, 10,  9,116,101,120,116, 45,100,101, 99,111,
    114, 97,116,105,111,110, 58, 32,110,111,110,101, 59, 10,  9,102,
    111,110,116, 45,115,105,122,101, 58, 32, 56, 53, 37, 59, 10,125,
     10, 10,100,105,118, 46,109,101,110,117, 32, 97, 58,104,111,118,
    101,114, 32,123, 32, 10,  9, 99,111,108,111,114, 58, 32, 35, 70,
     70, 70, 70, 70, 70, 59, 32, 10,  9,116,101,120,116, 45,100,101,
     99,111,114, 97,116,105,111,110, 58, 32,117,110,100,101,114,108,
    105,110,101, 59, 32, 10,125, 10, 10, 10, 47, 42, 32, 10, 32, 42,
      9, 83,101, 97,114, 99,104, 32, 10, 32, 42, 47, 10,100,105,118,
     46,115,101, 97,114, 99,104, 32,123, 10,  9,119,105,100,116,104,
     58, 32, 97,117,116,111, 59, 10,  9,112,111,115,105,116,105,111,
    110, 58, 32, 97, 98,115,111,108,117,116,101, 59, 10,  9,100,105,
    115,112,108, 97,121, 58, 32,105,110,108,105,110,101, 59, 10,  9,
    116,111,112, 58, 32, 49, 52,112,120, 59, 10,  9,108,101,102,116,
     58, 32, 55, 53, 48,112,120, 59, 10,  9,116,101,120,116, 45, 97,
    108,105,103,110, 58, 32,114,105,103,104,116, 59, 32,  9,  9, 47,
     42, 32, 77, 97, 99, 32, 42, 47, 10,  9,112, 97,100,100,105,110,
    103, 58, 32, 48, 32, 51, 48,112,120, 32, 48, 32, 48, 59, 10,  9,
    119,104,105,116,101, 45,115,112, 97, 99,101, 58, 32,110,111,119,
    114, 97,112, 59, 32,  9, 47, 42, 32, 79,112,101,114, 97, 32, 42,
     47, 10,  9,122, 45,105,110,100,101,120, 58, 32, 49, 49, 59, 10,
    125, 10, 10, 47, 42, 32, 78,111,110, 45, 73, 69, 32, 42, 47, 10,
    104,116,109,108, 62, 98,111,100,121, 32,100,105,118, 46,115,101,
     97,114, 99,104, 32,123, 10,125, 10, 10, 47, 42, 32, 73, 69, 32,
     42, 47, 10, 42, 32,104,116,109,108, 32,100,105,118, 46,115,101,
     97,114, 99,104, 32,123, 10,  9,116,101,120,116, 45, 97,108,105,
    103,110, 58, 32,114,105,103,104,116, 59, 32,  9,  9, 47, 42, 32,
     77, 97, 99, 32, 42, 47, 10,125, 10, 10,100,105,118, 46,115,101,
     97,114, 99,104, 32,108, 97, 98,101,108, 32,123, 32, 10,  9, 99,
    111,108,111,114, 58, 32, 35,102,102,102, 59, 32,102,111,110,116,
     45,115,105,122,101, 58, 32, 56, 53, 37, 59, 32, 10,125, 10, 10,
    100,105,118, 46,115,101, 97,114, 99,104, 32,102,111,114,109, 32,
    105,110,112,117,116, 32,123, 32, 10,  9,102,111,110,116, 45,115,
    105,122,101, 58, 32, 56, 53, 37, 59, 32, 10,125, 10, 10,100,105,
    118, 46,115,101, 97,114, 99,104, 32,102,111,114,109, 32, 35,115,
    117, 98,109,105,116, 32,123, 10,  9,102,111,110,116, 45,115,105,
    122,101, 58, 32, 56, 53, 37, 59, 10,  9, 98, 97, 99,107,103,114,
    111,117,110,100, 58, 32, 35, 54, 65, 55, 51, 56, 57, 59, 10,  9,
     99,111,108,111,114, 58, 32, 35,102,102,102, 59, 10,  9,112, 97,
    100,100,105,110,103, 58, 32, 49,112,120, 32, 52,112,120, 59, 10,
      9, 98,111,114,100,101,114, 45,114,105,103,104,116, 58, 32, 49,
    112,120, 32,115,111,108,105,100, 32, 35, 50, 56, 51, 48, 52, 51,
     59, 10,  9, 98,111,114,100,101,114, 45, 98,111,116,116,111,109,
     58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 50, 56, 51,
     48, 52, 51, 59, 10,  9, 98,111,114,100,101,114, 45,116,111,112,
     58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 57, 48, 57,
     55, 65, 50, 59, 10,  9, 98,111,114,100,101,114, 45,108,101,102,
    116, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 57, 48,
     57, 55, 65, 50, 59, 10,125, 10, 10,100,105,118, 46,115,101, 97,
    114, 99,104, 32,102,111,114,109, 32, 35,113, 32,123, 10,  9,119,
    105,100,116,104, 58, 32, 49, 55, 48,112,120, 59, 10,  9,102,111,
    110,116, 45,115,105,122,101, 58, 32, 56, 53, 37, 59, 10,  9, 98,
    111,114,100,101,114, 58,  9, 49,112,120, 32,115,111,108,105,100,
     32, 35, 57, 48, 57, 55, 65, 50, 59, 10,  9, 98, 97, 99,107,103,
    114,111,117,110,100, 58, 32, 35, 68, 57, 68, 66, 69, 49, 59, 10,
      9,112, 97,100,100,105,110,103, 58, 32, 50,112,120, 59, 10,125,
     10, 10,100,105,118, 46,115,101, 97,114, 99,104, 32,102,111,114,
    109, 32, 35,113, 58,104,111,118,101,114, 44, 32,100,105,118, 46,
    115,101, 97,114, 99,104, 32,102,111,114,109, 32, 35,113, 58,102,
    111, 99,117,115, 32,123, 10,  9, 98, 97, 99,107,103,114,111,117,
    110,100, 58, 32, 35,102,102,102, 59, 10,125, 10, 10, 10, 47, 42,
     32, 10, 32, 42,  9, 67,111,110,116,101,110,116, 32, 10, 32, 42,
     47, 10,100,105,118, 46, 99,111,110,116,101,110,116, 32,123, 10,
      9,109, 97,114,103,105,110, 58, 32, 50, 48,112,120, 59, 10,125,
     10, 10,104, 50, 46, 99,108, 97,115,115, 83,101, 99,116,105,111,
    110, 32,123, 10,  9,109, 97,114,103,105,110, 58, 32, 48, 32, 48,
     32, 54,112,120, 32, 48, 59, 10,125, 10, 10,104, 51, 46,109,101,
    116,104,111,100, 78, 97,109,101, 32,123, 10,  9,109, 97,114,103,
    105,110, 58, 32, 49, 53,112,120, 32, 48, 32, 52,112,120, 32, 48,
     59, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32, 49, 49,
     48, 37, 59, 10,125, 10, 10,104, 51, 46,109,101,116,104,111,100,
     83,101, 99,116,105,111,110, 32,123, 10,  9,109, 97,114,103,105,
    110, 58, 32, 49, 53,112,120, 32, 48, 32, 52,112,120, 32, 48, 59,
     10,  9,102,111,110,116, 45,115,105,122,101, 58, 32, 49, 48, 48,
     37, 59, 10,  9, 99,111,108,111,114, 58, 32, 35, 52, 48, 54, 48,
     57, 48, 59, 10,125, 10, 10,116, 97, 98,108,101, 46,110, 97,118,
    105,103, 97,116,105,111,110, 32,123, 10, 32, 32, 32, 32, 98,111,
    114,100,101,114, 58, 32,110,111,110,101, 59, 10,  9,109, 97,114,
    103,105,110, 58, 32, 48, 32, 48, 32, 48, 32, 49, 53,112,120, 59,
     10,125, 10, 10,116, 97, 98,108,101, 46,110, 97,118,105,103, 97,
    116,105,111,110, 32,116,100, 32,123, 10, 32, 32, 32, 32, 98,111,
    114,100,101,114, 58, 32,110,111,110,101, 59, 10,  9,112, 97,100,
    100,105,110,103, 58, 32, 53,112,120, 32, 50, 48,112,120, 32, 48,
     32, 48, 59, 10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52,
     48, 55, 48, 59, 10,125, 10, 10,116, 97, 98,108,101, 46, 99,108,
     97,115,115, 72,101, 97,100, 32,123, 10, 32, 32, 32, 32, 98,111,
    114,100,101,114, 58, 32,110,111,110,101, 59, 10,  9,109, 97,114,
    103,105,110, 45, 98,111,116,116,111,109, 58, 32, 49, 53,112,120,
     59, 10,125, 10, 10,116, 97, 98,108,101, 46, 99,108, 97,115,115,
     72,101, 97,100, 32,116,100, 32,123, 10, 32, 32, 32, 32, 98,111,
    114,100,101,114, 58, 32,110,111,110,101, 59, 10,  9,112, 97,100,
    100,105,110,103, 58, 32, 48,112,120, 32, 50, 48,112,120, 32, 55,
    112,120, 32, 48, 59, 10,125, 10, 10,116, 97, 98,108,101, 46,105,
    116,101,109, 84, 97, 98,108,101, 32,123, 10,  9,119,105,100,116,
    104, 58, 32, 57, 53, 37, 59, 10,  9, 98, 97, 99,107,103,114,111,
    117,110,100, 45, 99,111,108,111,114, 58, 32, 35, 70, 65, 70, 65,
     70, 65, 59, 10, 32, 32, 32, 32,112, 97,100,100,105,110,103, 58,
     32, 48, 59, 10, 32, 32, 32, 32,109, 97,114,103,105,110, 58, 32,
     48, 59, 10,  9, 98,111,114,100,101,114, 45,115,112, 97, 99,105,
    110,103, 58, 32, 48, 59, 10,  9, 98,111,114,100,101,114, 58, 32,
     49,112,120, 32,115,111,108,105,100, 32, 35, 99, 99, 99, 59, 10,
     32, 32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,101,
     59, 10, 32, 32, 32, 32,108,105,110,101, 45,104,101,105,103,104,
    116, 58, 32, 49, 49, 48, 37, 59, 10, 32, 32, 32, 32, 98,111,114,
    100,101,114, 45, 99,111,108,108, 97,112,115,101, 58, 32, 99,111,
    108,108, 97,112,115,101, 59, 10,125, 10, 10,116, 97, 98,108,101,
     46,105,116,101,109, 84, 97, 98,108,101, 32,116,114, 32,123, 10,
      9, 98,111,114,100,101,114, 58, 32,110,111,110,101, 59, 10,125,
     10, 10, 10, 47, 42, 32, 65, 80, 73, 32, 73,110,100,101,120, 32,
     42, 47, 10,116, 97, 98,108,101, 46, 97,112,105, 73,110,100,101,
    120, 32,123, 10, 32, 32, 32, 32,119,105,100,116,104, 58, 32, 57,
     53, 37, 59, 10,  9, 98, 97, 99,107,103,114,111,117,110,100, 45,
     99,111,108,111,114, 58, 32, 35, 70, 65, 70, 65, 70, 65, 59, 10,
      9, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,115,111,108,
    105,100, 32, 35, 99, 99, 99, 59, 10, 32, 32, 32, 32, 98,111,114,
    100,101,114, 45, 99,111,108,108, 97,112,115,101, 58, 32, 99,111,
    108,108, 97,112,115,101, 59, 10, 32, 32, 32, 32,112, 97,100,100,
    105,110,103, 58, 32, 48, 59, 10, 32, 32, 32, 32,109, 97,114,103,
    105,110, 58, 32, 48, 59, 10, 32, 32, 32, 32,108,105,110,101, 45,
    104,101,105,103,104,116, 58, 32, 49, 48, 48, 37, 59, 10, 32, 32,
     32, 32,102,111,110,116, 45,115,105,122,101, 58, 32, 57, 48, 37,
     59, 10,125, 10, 10,116,114, 46, 97,112,105, 68,101,102, 32,123,
     10,  9, 98,111,114,100,101,114, 45,116,111,112, 58, 32, 49,112,
    120, 32,115,111,108,105,100, 32, 35, 99, 99, 99, 59, 10,125, 10,
     10,116,114, 46, 97,112,105, 68,101,102, 32,116,100, 32,123, 10,
     32, 32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,110,101,
     59, 10,125, 10, 10,116,114, 46, 97,112,105, 66,114,105,101,102,
     32,116,100, 32,123, 10, 32, 32, 32, 32, 98,111,114,100,101,114,
     58, 32,110,111,110,101, 59, 10,125, 10, 10, 46, 97,112,105, 66,
    114,105,101,102, 32,123, 10,  9,102,111,110,116, 45,115,105,122,
    101, 58, 32, 57, 48, 37, 59, 10,  9, 99,111,108,111,114, 58, 32,
     35, 54, 54, 54, 54, 54, 54, 59, 10,  9,102,111,110,116, 45,115,
    116,121,108,101, 58, 32,105,116, 97,108,105, 99, 59, 10,125, 10,
     10, 46, 97,112,105, 84,121,112,101, 32,123, 10,  9,116,101,120,
    116, 45, 97,108,105,103,110, 58, 32,114,105,103,104,116, 59, 10,
     32, 32, 32, 32,112, 97,100,100,105,110,103, 58, 32, 48, 32, 49,
     48,112,120, 32, 48, 32, 49, 53,112,120, 59, 10, 32, 32, 32, 32,
    119,104,105,116,101, 45,115,112, 97, 99,101, 58, 32,110,111,119,
    114, 97,112, 59, 10,125, 10, 10, 47, 42, 32, 80,101,114, 32, 65,
     80, 73, 32, 98,108,111, 99,107, 32, 42, 47, 10,100,105,118, 46,
     97,112,105, 32,123, 10, 32, 32, 32, 32,109, 97,114,103,105,110,
     58, 32, 49, 48,112,120, 32, 48, 32, 54,112,120, 32, 48, 59, 10,
    125, 10, 10,100,105,118, 46, 97,112,105, 83,105,103, 32,123, 10,
      9,119,104,105,116,101, 45,115,112, 97, 99,101, 58, 32,110,111,
    119,114, 97,112, 59, 10,  9,102,111,110,116, 45,119,101,105,103,
    104,116, 58, 32, 98,111,108,100, 59, 10,  9, 98,111,114,100,101,
    114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 56, 52,
     98, 48, 99, 55, 59,  9, 10,  9, 99,111,108,111,114, 58, 32, 35,
     48, 48, 48, 48, 52, 48, 59, 10,  9,109, 97,114,103,105,110, 45,
    114,105,103,104,116, 58, 32, 50, 48,112,120, 59, 10,  9,112, 97,
    100,100,105,110,103, 58, 32, 56,112,120, 32, 49, 48,112,120, 32,
     56,112,120, 32, 51, 48,112,120, 59, 10, 32, 32, 32, 32,116,101,
    120,116, 45,105,110,100,101,110,116, 58, 32, 45, 50, 49,112,120,
     59, 10,  9, 98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,
    108,111,114, 58, 32, 35,100, 53,101, 49,101, 56, 59, 10,  9,102,
    111,110,116, 45,119,101,105,103,104,116, 58, 32, 98,111,108,100,
     59, 10,  9, 45,119,101, 98,107,105,116, 45, 98,111,114,100,101,
    114, 45,116,111,112, 45,108,101,102,116, 45,114, 97,100,105,117,
    115, 58, 32, 49, 48,112,120, 59, 10,  9, 45,119,101, 98,107,105,
    116, 45, 98,111,114,100,101,114, 45,116,111,112, 45,114,105,103,
    104,116, 45,114, 97,100,105,117,115, 58, 32, 49, 48,112,120, 59,
     10,  9, 45,109,111,122, 45, 98,111,114,100,101,114, 45,114, 97,
    100,105,117,115, 45,116,111,112,108,101,102,116, 58, 32, 49, 48,
    112,120, 59, 10,  9, 45,109,111,122, 45, 98,111,114,100,101,114,
     45,114, 97,100,105,117,115, 45,116,111,112,114,105,103,104,116,
     58, 32, 49, 48,112,120, 59, 10,125, 10, 10,100,105,118, 46, 97,
    112,105, 68,101,116, 97,105,108, 32,123, 10,  9,109, 97,114,103,
    105,110, 45, 98,111,116,116,111,109, 58, 32, 49, 48,112,120, 59,
     10,  9,109, 97,114,103,105,110, 45,114,105,103,104,116, 58, 32,
     50, 48,112,120, 59, 10,  9,112, 97,100,100,105,110,103, 58, 32,
     50,112,120, 32, 49, 48,112,120, 32, 53,112,120, 32, 49, 48,112,
    120, 59, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,
    115,111,108,105,100, 32, 35, 56, 52, 98, 48, 99, 55, 59,  9, 10,
      9, 98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,
    114, 58, 32, 35, 70, 54, 70, 54,102, 65, 59, 10,  9, 98,111,114,
    100,101,114, 45,116,111,112, 45,119,105,100,116,104, 58, 32, 48,
     59, 10,  9, 45,119,101, 98,107,105,116, 45, 98,111,114,100,101,
    114, 45, 98,111,116,116,111,109, 45,108,101,102,116, 45,114, 97,
    100,105,117,115, 58, 32, 49, 48,112,120, 59, 10,  9, 45,119,101,
     98,107,105,116, 45, 98,111,114,100,101,114, 45, 98,111,116,116,
    111,109, 45,114,105,103,104,116, 45,114, 97,100,105,117,115, 58,
     32, 49, 48,112,120, 59, 10,  9, 45,109,111,122, 45, 98,111,114,
    100,101,114, 45,114, 97,100,105,117,115, 45, 98,111,116,116,111,
    109,108,101,102,116, 58, 32, 49, 48,112,120, 59, 10,  9, 45,109,
    111,122, 45, 98,111,114,100,101,114, 45,114, 97,100,105,117,115,
     45, 98,111,116,116,111,109,114,105,103,104,116, 58, 32, 49, 48,
    112,120, 59, 10, 32, 32, 32, 32,108,105,110,101, 45,104,101,105,
    103,104,116, 58, 32, 49, 52, 48, 37, 59, 10,125, 10, 10,100,116,
     32,123, 10,  9,102,111,110,116, 45,119,101,105,103,104,116, 58,
     32, 98,111,108,100, 59, 10,  9, 99,111,108,111,114, 58, 32, 35,
     48, 48, 48, 48, 52, 48, 59, 10,125, 10, 10,116, 97, 98,108,101,
     46,112, 97,114, 97,109,101,116,101,114,115, 32,123, 10, 32, 32,
     32, 32,109, 97,114,103,105,110, 58, 32, 52,112,120, 32, 48, 32,
     48, 32, 48, 59, 10, 32, 32, 32, 32,112, 97,100,100,105,110,103,
     58, 32, 54,112,120, 59, 10, 32, 32, 32, 32, 98,111,114,100,101,
    114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35, 56, 56,
     56, 59, 10, 32, 32, 32, 32, 98,111,114,100,101,114, 45, 99,111,
    108,108, 97,112,115,101, 58, 32, 99,111,108,108, 97,112,115,101,
     59, 10,  9, 45,119,101, 98,107,105,116, 45, 98,111,120, 45,115,
    104, 97,100,111,119, 58, 32, 53,112,120, 32, 53,112,120, 32, 57,
    112,120, 32, 35, 56, 56, 56, 59, 10,125, 10, 10,116, 97, 98,108,
    101, 46,112, 97,114, 97,109,101,116,101,114,115, 32,116,100, 32,
    123, 10, 32, 32, 32, 32, 98,111,114,100,101,114, 58, 32,110,111,
    110,101, 59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 52,112,
    120, 32, 54,112,120, 32, 52,112,120, 32, 56,112,120, 59, 10, 32,
     32, 32, 32, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,115,
    111,108,105,100, 32, 35, 56, 56, 56, 59, 10,125, 10, 10,116,114,
     46,112, 97,114, 97,109, 32,123, 10, 32, 32, 32, 32, 98,111,114,
    100,101,114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35,
     56, 56, 56, 59, 10,125, 10, 10,116,100, 46,112, 97,114, 97,109,
     32,123, 10,  9,102,111,110,116, 45,115,116,121,108,101, 58, 32,
    105,116, 97,108,105, 99, 59, 10,  9,116,101,120,116, 45, 97,108,
    105,103,110, 58, 32,114,105,103,104,116, 59, 10, 32, 32, 32, 32,
    119,104,105,116,101, 45,115,112, 97, 99,101, 58, 32,110,111,119,
    114, 97,112, 59, 10,125, 10, 10,112, 46, 99,108, 97,115,115, 66,
    114,105,101,102, 32,123, 32, 10, 32, 32, 32, 32,109, 97,114,103,
    105,110, 58, 32, 49, 53,112,120, 32, 48,112,120, 32, 53,112,120,
     32, 50, 53,112,120, 59, 10,125, 10, 10,112, 46, 99,108, 97,115,
    115, 68,101,115, 99,114,105,112,116,105,111,110, 32,123, 32, 10,
     32, 32, 32, 32,109, 97,114,103,105,110, 58, 32, 49, 48,112,120,
     32, 48,112,120, 32, 49, 48,112,120, 32, 50, 53,112,120, 59, 10,
    125, 10, 10,112, 46,100,101,116, 97,105,108, 32,123, 32, 10, 32,
     32, 32, 32,109, 97,114,103,105,110, 58, 32, 49, 48,112,120, 32,
     48,112,120, 32, 53,112,120, 32, 50, 53,112,120, 59, 10,125, 10,
     10,112, 46,105,110,104,101,114,105,116,101,100, 76,105,110,107,
     32,123, 32, 10, 32, 32, 32, 32,109, 97,114,103,105,110, 58, 32,
     53,112,120, 32, 48, 32, 53,112,120, 32, 50,112,120, 59, 10,125,
     10, 10, 10, 47, 42, 10, 32, 42,  9, 83,116, 97,110,100, 97,114,
    100, 32,101,108,101,109,101,110,116,115, 10, 32, 42, 47, 10,104,
     49, 32,123, 32, 10,  9,102,111,110,116, 45,115,105,122,101, 58,
     32,120, 45,108, 97,114,103,101, 59, 10,  9, 99,111,108,111,114,
     58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,  9,102,111,110,
    116, 45,119,101,105,103,104,116, 58, 32, 98,111,108,100, 59, 10,
      9,109, 97,114,103,105,110, 58, 32, 49, 52,112,120, 32, 48, 32,
     49, 54,112,120, 32, 48, 59, 10,125, 32, 10, 10,104, 50, 32,123,
     32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,108, 97,
    114,103,101, 59, 10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48,
     52, 48, 55, 48, 59, 32, 10,  9,102,111,110,116, 45,119,101,105,
    103,104,116, 58, 32, 98,111,108,100, 59, 10,  9,109, 97,114,103,
    105,110, 58, 32, 49, 52,112,120, 32, 48, 32, 48, 32, 48, 59, 10,
    125, 10, 10,104, 51, 32,123, 32, 10,  9,102,111,110,116, 45,115,
    105,122,101, 58, 32,109,101,100,105,117,109, 59, 10,  9, 99,111,
    108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,  9,
    102,111,110,116, 45,119,101,105,103,104,116, 58, 32, 98,111,108,
    100, 59, 10,  9,109, 97,114,103,105,110, 58, 32, 56,112,120, 32,
     48, 32, 52,112,120, 32, 48, 59, 10,125, 10, 10,104, 52, 32,123,
     32, 10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55,
     48, 59, 32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,
    115,109, 97,108,108, 59, 10,  9,102,111,110,116, 45,115,116,121,
    108,101, 58, 32,105,116, 97,108,105, 99, 59, 32, 10,125, 10, 10,
     47, 42, 10, 32, 42,  9, 83,116, 97,110,100, 97,114,100, 32,101,
    108,101,109,101,110,116,115, 10, 32, 42, 47, 10,104, 49, 32,123,
     32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,120, 45,
    108, 97,114,103,101, 59, 10,  9, 99,111,108,111,114, 58, 32, 35,
     50, 48, 52, 48, 55, 48, 59, 32, 10,  9,102,111,110,116, 45,119,
    101,105,103,104,116, 58, 32, 98,111,108,100, 59, 10,  9,109, 97,
    114,103,105,110, 58, 32, 49, 52,112,120, 32, 48, 32, 49, 54,112,
    120, 32, 48, 59, 10,125, 32, 10, 10,104, 50, 32,123, 32, 10,  9,
    102,111,110,116, 45,115,105,122,101, 58, 32,108, 97,114,103,101,
     59, 10,  9, 99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55,
     48, 59, 32, 10,  9,102,111,110,116, 45,119,101,105,103,104,116,
     58, 32, 98,111,108,100, 59, 10,  9,109, 97,114,103,105,110, 58,
     32, 49, 52,112,120, 32, 48, 32, 48, 32, 48, 59, 10,125, 10, 10,
    104, 51, 32,123, 32, 10,  9,102,111,110,116, 45,115,105,122,101,
     58, 32,109,101,100,105,117,109, 59, 10,  9, 99,111,108,111,114,
     58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 32, 10,  9,102,111,110,
    116, 45,119,101,105,103,104,116, 58, 32, 98,111,108,100, 59, 10,
      9,109, 97,114,103,105,110, 58, 32, 56,112,120, 32, 48, 32, 52,
    112,120, 32, 48, 59, 10,125, 10, 10,104, 52, 32,123, 32, 10,  9,
     99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 32,
     10,  9,102,111,110,116, 45,115,105,122,101, 58, 32,115,109, 97,
    108,108, 59, 10,  9,102,111,110,116, 45,115,116,121,108,101, 58,
     32,105,116, 97,108,105, 99, 59, 32, 10,125, 10, 10,112, 32,123,
     32, 10,  9,108,105,110,101, 45,104,101,105,103,104,116, 58, 32,
     49, 51, 48, 37, 59, 32, 10,  9,109, 97,114,103,105,110, 45,116,
    111,112, 58, 32, 53,112,120, 59, 10,125, 10, 10,117,108, 32,123,
     32, 10,  9,102,111,110,116, 45,115,105,122,101, 58, 32, 49, 48,
     48, 37, 59, 10,125, 10, 10,111,108, 32,123, 32, 32, 10,  9,102,
    111,110,116, 45,115,105,122,101, 58, 32, 49, 48, 48, 37, 59, 10,
    125, 10, 10,112,114,101, 32,123, 32, 10, 32, 32, 32, 32,102,111,
    110,116, 45,115,105,122,101, 58, 32, 49, 50, 48, 37, 59, 10,  9,
    119,105,100,116,104, 58, 32, 57, 48, 37, 59, 10, 32, 32, 32, 32,
     99,111,108,111,114, 58, 32, 35, 50, 48, 52, 48, 55, 48, 59, 10,
     32, 32, 32, 32,102,111,110,116, 45,102, 97,109,105,108,121, 58,
     32, 67,111,117,114,105,101,114, 32, 78,101,119, 44, 32, 67,111,
    117,114,105,101,114, 44, 32,109,111,110,111,115,112, 97, 99,101,
     59, 10, 32, 32, 32, 32,112, 97,100,100,105,110,103, 58, 32, 53,
    112,120, 32, 49, 53,112,120, 32, 53,112,120, 32, 49, 53,112,120,
     59, 10, 32, 32, 32, 32,109, 97,114,103,105,110, 58, 32, 49, 48,
    112,120, 32, 49,112,120, 32, 53,112,120, 32, 49,112,120, 59, 10,
     32, 32, 32, 32, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,
    115,111,108,105,100, 32, 35, 56, 54, 56, 54, 56, 54, 59, 10,  9,
     98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,114,
     58, 32, 35, 70, 56, 70, 56, 70, 56, 59, 10,125, 10, 10, 46,115,
    109, 97,108,108, 84,101,120,116, 32,123, 32, 10,  9,102,111,110,
    116, 45,115,105,122,101, 58, 32, 57, 48, 37, 59, 32, 10,  9, 99,
    111,108,111,114, 58, 32, 35, 51, 49, 51, 49, 51, 49, 59, 32, 10,
    125, 10, 10, 47, 42, 10, 32, 42,  9, 71,101,110,101,114, 97,108,
     32,108,105,110,107, 32,104,105,103,104,108,105,103,104,116,105,
    110,103, 10, 32, 42, 47, 10, 97, 58,108,105,110,107, 32,123, 32,
     10,  9, 99,111,108,111,114, 58, 32, 35, 51, 49, 54, 57, 57, 67,
     59, 32, 10,125, 10, 10, 97, 58,118,105,115,105,116,101,100, 32,
    123, 32, 10,  9, 99,111,108,111,114, 58, 32, 35, 51, 49, 54, 57,
     57, 67, 59, 32, 10,125, 10, 10, 97, 58,104,111,118,101,114, 32,
    123, 32, 10,  9, 99,111,108,111,114, 58, 32, 35, 67, 67, 52, 52,
     55, 55, 59, 32, 10,125, 10, 10,100,105,118, 46,116,101,114,109,
    115, 32,123, 10,  9,109, 97,114,103,105,110, 45,116,111,112, 58,
     32, 50, 48,112,120, 59, 10,125, 10, 10,100,105,118, 46,116,101,
    114,109,115, 32,112, 32, 97, 58,108,105,110,107, 44, 32,100,105,
    118, 46,116,101,114,109,115, 32,112, 32, 97, 58,118,105,115,105,
    116,101,100, 32,123, 32, 10,  9,102,111,110,116, 45,115,105,122,
    101, 58, 32,120, 45,115,109, 97,108,108, 59, 10,  9, 99,111,108,
    111,114, 58, 32, 35, 67, 48, 67, 48, 67, 48, 59, 32, 10,  9,116,
    101,120,116, 45,100,101, 99,111,114, 97,116,105,111,110, 58, 32,
    110,111,110,101, 59, 10,125, 10, 10,100,105,118, 46,116,101,114,
    109,115, 32,112, 32, 97, 58,104,111,118,101,114, 32,123, 32, 10,
      9,102,111,110,116, 45,115,105,122,101, 58, 32,120, 45,115,109,
     97,108,108, 59, 10,  9,116,101,120,116, 45,100,101, 99,111,114,
     97,116,105,111,110, 58, 32,117,110,100,101,114,108,105,110,101,
     59, 32, 10,125, 10, 10, 47, 42, 10, 32, 42, 32,  9, 68,105,115,
     97, 98,108,101,100, 10, 32, 42, 47, 10,104,114, 32,123, 10,  9,
    104,101,105,103,104,116, 58, 32, 49,112,120, 59, 10,  9, 98, 97,
     99,107,103,114,111,117,110,100, 45, 99,111,108,111,114, 58, 32,
     35, 70, 70, 70, 70, 70, 70, 59, 10,  9, 99,111,108,111,114, 58,
     32, 35, 70, 70, 70, 70, 70, 70, 59, 10,  9, 99,111,108,111,114,
     58, 32, 35, 70, 48, 70, 48, 70, 48, 59, 10,  9,109, 97,114,103,
    105,110, 58, 32, 49, 53,112,120, 32, 48, 32, 49, 48,112,120, 32,
     48, 59, 10,  9,112, 97,100,100,105,110,103, 58, 32, 48, 32, 48,
     32, 48, 32, 48, 59, 10,  9, 98,111,114,100,101,114, 45,115,116,
    121,108,101, 58, 32,110,111,110,101, 59, 10,125, 10, 10,105,109,
    103, 32,123, 10,  9, 98,111,114,100,101,114, 45,115,116,121,108,
    101, 58, 32,110,111,110,101, 59, 10,125, 10, 10,105,109,103, 46,
    119,114, 97,112, 76,101,102,116, 32,123, 10,  9,102,108,111, 97,
    116, 58, 32,108,101,102,116, 59, 10,  9,109, 97,114,103,105,110,
     58, 32, 48,112,120, 32, 49, 48,112,120, 32, 51,112,120, 32, 48,
    112,120, 59, 10,125, 10, 10,105,109,103, 46,119,114, 97,112, 82,
    105,103,104,116, 32,123, 10,  9,102,108,111, 97,116, 58, 32,114,
    105,103,104,116, 59, 10,  9,109, 97,114,103,105,110, 58, 32, 48,
    112,120, 32, 48,112,120, 32, 51,112,120, 32, 49, 48,112,120, 59,
     10,125, 10, 10,116, 97, 98,108,101, 32,123, 10,  9, 98,111,114,
    100,101,114, 58, 32, 49,112,120, 32,115,111,108,105,100, 32, 35,
     65, 48, 65, 48, 65, 48, 59, 10,  9,120, 98,111,114,100,101,114,
     45, 99,111,108,108, 97,112,115,101, 58, 32, 99,111,108,108, 97,
    112,115,101, 59, 10,  9,120, 98,111,114,100,101,114, 45,115,112,
     97, 99,105,110,103, 58, 32, 48,112,120, 59, 10,125, 10, 10,116,
    104, 32,123, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,120,
     32,115,111,108,105,100, 32, 35, 65, 48, 65, 48, 65, 48, 59, 10,
      9, 98, 97, 99,107,103,114,111,117,110,100, 45, 99,111,108,111,
    114, 58, 32, 35, 70, 48, 70, 48, 70, 48, 59, 10,  9, 99,111,108,
    111,114, 58, 32, 35, 52, 48, 54, 48, 57, 48, 59, 10,  9,112, 97,
    100,100,105,110,103, 58, 32, 52,112,120, 32, 56,112,120, 32, 52,
    112,120, 32, 56,112,120, 59, 10,  9,116,101,120,116, 45, 97,108,
    105,103,110, 58, 32,108,101,102,116, 59, 10,125, 10, 10,116,100,
     32,123, 10,  9, 98,111,114,100,101,114, 58, 32, 49,112,120, 32,
    115,111,108,105,100, 32, 35, 65, 48, 65, 48, 65, 48, 59, 10,  9,
    112, 97,100,100,105,110,103, 58, 32, 52,112,120, 32, 56,112,120,
     32, 52,112,120, 32, 56,112,120, 59, 10,125, 10, 10, 97, 32,123,
     10,  9,116,101,120,116, 45,100,101, 99,111,114, 97,116,105,111,
    110, 58, 32,110,111,110,101, 59, 10,125, 10,
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
    { "images/banner.jpg", _file_1, 12664, 1 },
    { "doc.css", _file_2, 6747, 2 },
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




typedef struct App {
    EjsService  *ejsService;
    Ejs         *ejs;
    EjsMod      *mod;
} App;

static void getDepends(Ejs *ejs, MprList *list, EjsModule *mp);
static void logger(int flags, int level, const char *msg);
static void manageApp(App *app, int flags);
static int  process(EjsMod *mp, cchar *output, int argc, char **argv);
static void require(MprList *list, cchar *name);
static int  setLogging(Mpr *mpr, char *logSpec);


MAIN(ejsmodMain, int argc, char **argv)
{
    Mpr             *mpr;
    App             *app;
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
    mpr = mprCreate(argc, argv, MPR_USER_GC);
    mprSetAppName(argv[0], 0, 0);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);

    /*
        Allocate the primary control structure
     */
    if ((mp = mprAlloc(sizeof(EjsMod))) == NULL) {
        return MPR_ERR_MEMORY;
    }
    app->mod = mp;
    mp->lstRecords = mprCreateList(mp);
    mp->blocks = mprCreateList(mp);
    mp->docDir = ".";
    
    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--cslots") == 0) {
            mp->cslots = 1;
            mp->genSlots = 1;
            
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
                mp->docDir = argv[++nextArg];
                mp->html = 1;
            }
            
        } else if (strcmp(argp, "--listing") == 0) {
            mp->listing = 1;
            
        } else if (strcmp(argp, "--log") == 0) {
            /*
                Undocumented switch
             */
            if (nextArg >= argc) {
                err++;
            } else {
                setLogging(mpr, argv[++nextArg]);
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

        } else if (strcmp(argp, "--showDebug") == 0) {
            mp->showDebug++;

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintfError("%s %s-%s\n", BLD_NAME, BLD_VERSION, BLD_NUMBER);  
            exit(0);

        } else if (strcmp(argp, "--require") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                if (requiredModules == 0) {
                    requiredModules = mprCreateList(mpr);
                }
                modules = sclone(argv[++nextArg]);
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
        requiredModules = mprCreateList(mpr);
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
            "  --showDebug           # Show debug instructions\n"
            "  --version             # Emit the program version information\n"
            "  --warn                # Warn about undocumented methods or parameters in doc\n\n", mpr->name);
        return -1;
    }

    /*
        Need an interpreter to load modules
     */
    app->ejsService = ejsCreateService(mpr); 
    if (app->ejsService == 0) {
        return MPR_ERR_MEMORY;
    }
    flags = EJS_FLAG_NO_INIT;
    if (mp->html || mp->xml) {
        flags |= EJS_FLAG_DOC;
    }
    ejs = ejsCreateVm(searchPath, requiredModules, 0, NULL, flags);
    if (ejs == 0) {
        return MPR_ERR_MEMORY;
    }
    app->ejs = ejs;
    mp->ejs = ejs;

    if (nextArg < argc) {
        if (process(mp, output, argc - nextArg, &argv[nextArg]) < 0) {
            err++;
        }
    }
    if (mp->errorCount > 0) {
        err = -1;
    }
    mprFree(ejs);
    if (mprStop(mpr)) {
        mprFree(mpr);
    }
    return err;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->ejsService);
        mprMark(app->ejs);
        mprMark(app->mod);

    } else if (flags & MPR_MANAGE_FREE) {
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
        outfile = mprOpen(output, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664);
    } else {
        outfile = 0;
    }
    ejs->loaderCallback = (mp->listing) ? emListingLoadCallback : 0;
    mp->firstGlobal = ejsGetPropertyCount(ejs, ejs->global);

    /*
        For each module on the command line
     */
    for (i = 0; i < argc && !mp->fatalError; i++) {
        moduleCount = mprGetListCount(ejs->modules);
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
            depends = mprCreateList(ejs);
            for (next = moduleCount; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                getDepends(ejs, depends, module);
            }
            count = mprGetListCount(depends);
            for (next = 1; (module = mprGetNextItem(depends, &next)) != 0; ) {
                int version = module->version;
                mprPrintf("%@-%d.%d.%d%s", module->name, EJS_MAJOR(version), EJS_MINOR(version), EJS_PATCH(version),
                    (next >= count) ? "" : " ");
            }
            printf("\n");
            mprFree(depends);
        }
    }
    if (mp->html || mp->xml) {
        emCreateDoc(mp);
    }
    mprFree(outfile);
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


static int setLogging(Mpr *mpr, char *logSpec)
{
    MprFile     *file;
    char        *levelSpec;
    int         level;

    level = 0;

    if ((levelSpec = strchr(logSpec, ':')) != 0) {
        *levelSpec++ = '\0';
        level = atoi(levelSpec);
    }

    if (strcmp(logSpec, "stdout") == 0) {
        file = mpr->fileSystem->stdOutput;
    } else {
        if ((file = mprOpen(logSpec, O_WRONLY, 0664)) == 0) {
            mprPrintfError("Can't open log file %s\n", logSpec);
            return EJS_ERR;
        }
    }

    mprSetLogLevel(level);
    mprSetLogHandler(logger, (void*) file);

    return 0;
}


static void logger(int flags, int level, const char *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *prefix;

    mpr = mprGetMpr();
    file = (MprFile*) mpr->logData;
    prefix = mpr->name;

    while (*msg == '\n') {
        mprFprintf(file, "\n");
        msg++;
    }

    if (flags & MPR_LOG_SRC) {
        mprFprintf(file, "%s: %d: %s\n", prefix, level, msg);
    } else if (flags & MPR_ERROR_SRC) {
        mprFprintf(file, "%s: Error: %s\n", prefix, msg);
    } else if (flags & MPR_FATAL_SRC) {
        mprFprintf(file, "%s: Fatal: %s\n", prefix, msg);
    } else if (flags & MPR_RAW) {
        mprFprintf(file, "%s", msg);
    }
    if (flags & (MPR_ERROR_SRC | MPR_FATAL_SRC | MPR_ASSERT_SRC)) {
        mprBreakpoint();
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
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
        mprFree(lst);
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
        mprFree(lst);
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
    mprFree(mp->file);
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
    if ((mp->file = mprOpen(path,  O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664)) == 0) {
        mprError("Can't create %s", path);
        mprFree(path);
        return EJS_ERR;
    }
    mprEnableFileBuffering(mp->file, 0, 0);
    mprFprintf(mp->file, "#\n#  %s -- Module Listing for %s\n#\n", path, moduleFilename);
    mprFree(path);
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
    int     i, size;

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
    mprFprintf(mp->file,  ") : %@\n", resultType ? resultType->qname.name : ejs->voidType->qname.name);

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
#if UNUSED
    leadin(mp, module, 0, 0);
#endif
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
            ex->catchType ? ex->catchType->qname.name : ejs->emptyString);
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

        if (mp->showDebug) {
            /*
                Output address [stack] opcode
                Format:  "address: [stackDepth] opcode <args> ..."
             */
            mprFprintf(mp->file,  "    %04d: [%d] %02x ", address, stack, opcode);
            mp->showAsm = 1;

        } else {
            mp->showAsm = 0;
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
    EjsString   *space;

    mprAssert(slotNum >= 0);
    mprAssert(qname);

    space = mapSpace(mp->ejs, qname->space);

    if (qname->name == 0 || qname->name->value[0] == '\0') {
        mprFprintf(mp->file, "%04d    <inherited>\n", slotNum);

    } else if (trait && trait->type) {
        if (trait->type == mp->ejs->functionType) {
            mprFprintf(mp->file, "%04d    %@ function %@\n", slotNum, space, qname->name);

        } else if (trait->type == mp->ejs->functionType) {
            mprFprintf(mp->file, "%04d    %@ class %@\n", slotNum, space, qname->name);

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
    VISITED(obj) = 1;

    if (obj == ejs->global) {
        type = (EjsType*) obj;
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
        trait = ejsGetPropertyTraits(ejs, obj, i);
        qname = ejsGetPropertyName(ejs, obj, i);
        vp = ejsGetProperty(ejs, obj, i);
        if (vp == 0) {
            continue;
        }
        if (ejsIsType(ejs, vp) || ejsIsFunction(ejs, vp) || ejsIsBlock(ejs, vp)) {
            lstSlotAssignments(mp, module, obj, i, vp);
        }
    }
    VISITED(obj) = 0;
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
#if UNUSED
    if (attributes & EJS_TYPE_ORPHAN) {
        strcat(attributeBuf, "orphan ");
    }
#endif
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

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

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
        localFile = file = mprOpen(path, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0664);
    } else {
        path = sclone(file->path);
    }
    if (file == 0) {
        mprError("Can't open %s", path);
        mprFree(path);
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
    type = ejsCreateType(ejs, N(EJS_EJS_NAMESPACE, EJS_GLOBAL), NULL, NULL, NULL, sizeof(EjsType), slotNum, 
        ejsGetPropertyCount(ejs, ejs->global), 0, 0);
    type->constructor.block = *(EjsBlock*) ejs->global;
    TYPE(type) = ejs->typeType;
    type->constructor.block.pot.isType = 1;

    if (genType(bp, file, mp, type, mp->firstGlobal, mp->lastGlobal, 1) < 0) {
        mprError("Can't generate slot file for module %@", mp->name);
        mprFree(path);
        mprFree(localFile);
        return EJS_ERR;
    }
    mprFprintf(file, "\n#define _ES_CHECKSUM_%s   %d\n", moduleName, mp->checksum);
    mprFprintf(file, "\n#endif\n");
    mprFree(localFile);
    mprFree(path);
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
    mprFree(typeStr);
    mprFree(funStr);
    mprFree(nameStr);
}


static void defineSlotCount(EjsMod *bp, MprFile *file, EjsModule *mp, EjsType *type, char *suffix, int numProp)
{
    char        name[MPR_MAX_STRING], *typeStr, *sp;

    typeStr = mapFullName(bp->ejs, &type->qname, 1);
    if (*typeStr == '\0') {
        mprFree(typeStr);
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
    mprFree(typeStr);
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
    EjsTrait        *trait, *lp;
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
                trait = ejsGetPropertyTraits(ejs, (EjsObj*) type, slotNum);
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
            trait = ejsGetPropertyTraits(ejs, prototype, slotNum);
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
        if (trait->type != ejs->functionType) {
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
            lp = ejsGetPropertyTraits(ejs, activation, i);
            lqname = ejsGetPropertyName(ejs, activation, i);
            defineSlot(bp, file, mp, type, (EjsObj*) activation, i, &qname, &lqname);
        }
        for (; i < fun->block.pot.numProp; i++) {
            lp = ejsGetPropertyTraits(ejs, activation, i);
            lqname = ejsGetPropertyName(ejs, activation, i);
            defineSlot(bp, file, mp, type, (EjsObj*) activation, i, &qname, &lqname);
        }
    }

    /*
        Now process nested types.
     */
    for (slotNum = firstClassSlot; slotNum < lastClassSlot; slotNum++) {
        trait = ejsGetPropertyTraits(ejs, (EjsObj*) type, slotNum);
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
        VISITED(vp) = 1;

        count = ejsGetPropertyCount(ejs, (EjsObj*) nt);
        if (genType(bp, file, mp, nt, 0, count, 0) < 0) {
            VISITED(vp) = 0;
            return EJS_ERR;
        }
        VISITED(vp) = 0;
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
  
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
  
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
  
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/cmd/slotGen.c"
 */
/************************************************************************/

