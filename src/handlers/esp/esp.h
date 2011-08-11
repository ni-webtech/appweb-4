/*
    esp.h -- Header for the Embedded Server Pages (ESP) Module.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_ESP
#define _h_ESP 1

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

#if BLD_TUNE == MPR_TUNE_SIZE || DOXYGEN
    /*  
        Tune for size
     */
    //  MOB - these should be dynamic
    #define ESP_TOK_INCR            1024
    #define ESP_MAX_HEADER          1024
    #define ESP_MAX_ROUTE_MATCHES   64

#elif BLD_TUNE == MPR_TUNE_BALANCED
    /*  
        Tune balancing speed and size
     */
    #define ESP_TOK_INCR            4096
    #define ESP_MAX_HEADER          4096
    #define ESP_MAX_ROUTE_MATCHES   128

#else
    /*  
        Tune for speed
     */
    #define ESP_MAX_ROUTE_MATCHES   256
#endif

/*
    Default listening endpoint
 */
#define ESP_LISTEN  "4000"

/*
    Timeout to wait for ESP requests to complete when reloading modules
 */
#define ESP_UNLOAD_TIMEOUT  (30 * 1000)

/*
    Default compiler settings for ${DEBUG} and ${LIBS} tokens in EspCompile and EspLink
 */
#if BLD_DEBUG
    #if WIN
        #define ESP_DEBUG "-Zi -Od"
    #else
        #define ESP_DEBUG "-g"
    #endif
#else
    #if WIN
        #define ESP_DEBUG "-O"
    #else
        #define ESP_DEBUG "-O2"
    #endif
#endif
#if WIN
    #define ESP_CORE_LIBS "${LIBDIR}\\mod_esp${SHLIB} ${LIBDIR}\\libappweb.lib ${LIBDIR}\\libhttp.lib ${LIBDIR}\\libmpr.lib"
#else
    #define ESP_CORE_LIBS "${LIBDIR}/mod_esp${SHOBJ} -lappweb -lpcre -lhttp -lmpr -lpthread -lm"
#endif
#if BLD_FEATURE_SSL
    #if WIN
        #define ESP_SSL_LIBS " ${LIBDIR}\\libmprssl.lib"
    #else
        #define ESP_SSL_LIBS " -lmprssl"
    #endif
#else
    #define ESP_SSL_LIBS
#endif
#define ESP_LIBS ESP_CORE_LIBS ESP_SSL_LIBS

#define CONTENT_MARKER  "${_ESP_CONTENT_MARKER_}"

#define ESP_LIFESPAN            (86400 * MPR_TICKS_PER_SEC)       /* Default HTML cache lifespan */

/********************************** Defines ***********************************/

#define ESP_NAME "espHandler"

#if BLD_WIN_LIKE
    #define ESP_EXPORT __declspec(dllexport) 
#else
    #define ESP_EXPORT 
#endif
    #define ESP_EXPORT_STRING MPR_STRINGIFY(ESP_EXPORT)

/*
      ESP lexical analyser tokens
 */
#define ESP_TOK_ERR            -1            /* Any input error */
#define ESP_TOK_EOF             0            /* End of file */
#define ESP_TOK_CODE            1            /* <% text %> */
#define ESP_TOK_VAR             2            /* @@var */
#define ESP_TOK_LITERAL         3            /* literal HTML */
#define ESP_TOK_EXPR            4            /* <%= expression %> */
#define ESP_TOK_CONTROL         5            /* <%@ control */

/*
    ESP page parser structure
 */
typedef struct EspParse {
    char    *data;                          /* Input data to parse */
    char    *next;                          /* Next character in input */
    MprBuf  *token;                         /* Storage buffer for token */
    int     lineNumber;                     /* Line number for error reporting */
} EspParse;

/*
    Top level ESP structure
 */
typedef struct Esp {
    MprHashTable    *actions;               /* Table of actions */
    MprHashTable    *views;                 /* Table of views */
    MprCache        *cache;                 /* Session and content cache */
    MprMutex        *mutex;                 /* Multithread lock */
    int             inUse;                  /* Active ESP request counter */
} Esp;

/*
    EspLoc structure. One per location.
 */
typedef struct EspLoc {
    MprHashTable    *modules;               /* Compiled modules */
    MprList         *env;                   /* Environment for compiler */
    MprList         *routes;                /* Ordered list of routes */
    HttpLoc         *loc;                   /* Controlling Http location */
    char            *compile;               /* Compile template */
    char            *link;                  /* Link template */
    char            *searchPath;            /* Search path to use when locating compiler / linker */
    char            *appModuleName;         /* App module name when compiled flat */
    char            *appModulePath;         /* App module path when compiled flat */

    char            *dir;                   /* Base directory for location */
    char            *cacheDir;              /* Directory for cached compiled controllers and views */
    char            *controllersDir;        /* Directory for controllers */
    char            *databasesDir;          /* Directory for databases */
    char            *layoutsDir;            /* Directory for layouts */
    char            *modelsDir;             /* Directory for models */
    char            *staticDir;             /* Directory for static web content */
    char            *viewsDir;              /* Directory for views */

    MprTime         lifespan;               /* Default cache lifespan */
    int             update;                 /* Auto-update modified ESP source */
    int             keepSource;             /* Preserve generated source */
	int				showErrors;				/* Send server errors back to client */
} EspLoc;

void espManageEspLoc(EspLoc *el, int flags);

/*
    ESP Route structure. One per route.
 */
typedef struct EspRoute {
    char            *name;                  /* Route name */
    MprHashTable    *methods;               /* Supported HTTP methods */
    char            *pattern;               /* Original matching URI pattern for the route */
    char            *action;                /* Original matching action to run */
    MprList         *tokens;                /* Tokens in pattern, {name} */
    char            *params;                /* Params to define. Extracted from pattern. (compiled) */
    char            *controllerName;        /* Controller containing actions */
    char            *controllerPath;        /* Full source path source of controller */
    char            *template;              /* URI template for forming links based on this route */
    char            *patternExpression;     /* Pattern regular expression */
    void            *patternCompiled;       /* Compiled pattern regular expression */
    char            *actionReplacement;     /* Matching action to run (compiled) */
} EspRoute;

extern EspRoute *espCreateRoute(cchar *name, cchar *methods, cchar *pattern, cchar *action, cchar *controller);
extern char *espMatchRoute(HttpConn *conn, EspRoute *route);

#define ESP_SESSION             "-esp-session-"

typedef struct EspSession {
    char            *id;                /* Session ID key */
    MprCache        *cache;             /* Cache store reference */
    MprTime         lifespan;           /* Session inactivity timeout (msecs) */
} EspSession;


extern EspSession *espAllocSession(HttpConn *conn, cchar *id, MprTime timeout, int create);
extern void espDestroySession(EspSession *sp);
extern EspSession *espGetSession(HttpConn *conn, int create);
extern cchar *espGetSessionVar(HttpConn *conn, cchar *key, cchar *defaultValue);
extern int espSetSessionVar(HttpConn *conn, cchar *key, cchar *value, MprTime lifespan);
extern void espExpireSessionVar(HttpConn *conn, cchar *key, MprTime lifespan);
extern char *espGetSessionID(HttpConn *conn);

typedef void (*EspActionFn)(HttpConn *conn);
typedef void (*EspViewFn)(HttpConn *conn);

typedef struct EspAction {
    EspActionFn     actionFn;
    MprTime         lifespan;
    char            *uri;
} EspAction;

/*
    ESP request state
 */
typedef struct EspReq {
    EspLoc          *el;                    /* Back pointer to Esp Location */
    EspRoute        *route;                 /* Route used for request */
    EspSession      *session;               /* Session data object */
    EspAction       *action;                /* Action to invoke */
    Esp             *esp;                   /* Convenient esp reference */
    HttpLoc         *loc;                   /* Location reference */
    MprBuf          *cacheBuffer;           /* HTML output caching */
    char            *actionKey;             /* Request actionKey value */
    char            *cacheName;             /* Base name of intermediate compiled file */
    char            *module;                /* Name of compiled module */
    char            *source;                /* Name of ESP source */
    char            *controller;            /* Path to controller */
    char            *view;                  /* Path to view */
    char            *entry;                 /* Module entry point */
    char            *commandLine;           /* Command line for compile/link */
    int             autoFinalize;           /* Request is/will-be auto-finalized */
    int             finalized;              /* Request has been finalized */
    int             sessionProbed;          /* Already probed for session store */
} EspReq;


extern bool espCompile(HttpConn *conn, cchar *source, cchar *module, cchar *cacheName, int isView);
extern char *espBuildScript(EspLoc *el, cchar *page, cchar *path, cchar *name, cchar *layout, char **err);
extern char *espExpandCommand(cchar *command, cchar *source, cchar *module);

//  MOB - move to pcre
#define PCRE_GLOBAL     0x1
extern char *pcre_replace(cchar *str, void *pattern, cchar *replacement, MprList **parts, int flags);

extern void espAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);
extern void espAddHeaderString(HttpConn *conn, cchar *key, cchar *value);
extern void espAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);
extern void espAppendHeaderString(HttpConn *conn, cchar *key, cchar *value);
extern void espAutoFinalize(HttpConn *conn);
extern void espDefineAction(EspLoc *esp, cchar *path, void *action);
extern void espDefineView(EspLoc *esp, cchar *path, void *view);
extern void espDontCache(HttpConn *conn);
extern MprOff espGetContentLength(HttpConn *conn);
extern cchar *espGetCookies(HttpConn *conn);
extern MprHashTable *espGetFormVars(HttpConn *conn);
extern cchar *espGetHeader(HttpConn *conn, cchar *key);
extern MprHashTable *espGetHeaderHash(HttpConn *conn);
extern char *espGetHeaders(HttpConn *conn);
extern int espGetIntVar(HttpConn *conn, cchar *var, int defaultValue);
extern cchar *espGetQueryString(HttpConn *conn);
extern int espGetStatus(HttpConn *conn);
extern char *espGetStatusMessage(HttpConn *conn);
extern cchar *espGetVar(HttpConn *conn, cchar *var, cchar *defaultValue);
extern void espFinalize(HttpConn *conn);
extern bool espFinalized(HttpConn *conn);
extern void espFlush(HttpConn *conn);
extern bool espMatchVar(HttpConn *conn, cchar *var, cchar *value);
extern void espRedirect(HttpConn *conn, int status, cchar *target);
extern void espRedirectBack(HttpConn *conn);
extern int espRemoveHeader(HttpConn *conn, cchar *key);
extern bool espSetAutoFinalizing(HttpConn *conn, int on);
extern void espSetContentLength(HttpConn *conn, MprOff length);
extern void espSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, int lifetime, bool isSecure);
extern void espSetContentType(HttpConn *conn, cchar *mimeType);
extern void espSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);
extern void espSetHeaderString(HttpConn *conn, cchar *key, cchar *value);
extern void espSetIntVar(HttpConn *conn, cchar *var, int value);
extern void espSetStatus(HttpConn *conn, int status);
extern void espSetVar(HttpConn *conn, cchar *var, cchar *value);
extern void espShowRequest(HttpConn *conn);
extern ssize espWrite(HttpConn *conn, cchar *fmt, ...);
extern ssize espWriteBlock(HttpConn *conn, cchar *buf, ssize size);
extern ssize espWriteString(HttpConn *conn, cchar *s);
extern ssize espWriteSafeString(HttpConn *conn, cchar *s);
extern ssize espWriteVar(HttpConn *conn, cchar *name);

//  MOB - rename espCache
extern void espCacheControl(EspLoc *el, cchar *actionKey, int lifesecs, cchar *uri);


#ifdef __cplusplus
} /* extern C */
#endif
#endif /* BLD_FEATURE_ESP */
#endif /* _h_ESP */

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

