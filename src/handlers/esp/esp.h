/*
    esp.h -- Header for the Embedded Server Pages (ESP) Module.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_ESP
#define _h_ESP 1

/********************************* Includes ***********************************/

#include    "appweb.h"

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

#define ESP_LIFESPAN            (86400 * MPR_TICKS_PER_SEC)       /* Default HTML cache lifespan */

/********************************** Defines ***********************************/


/*
      ESP lexical analyser tokens
 */
#define ESP_TOK_ERR            -1            /* Any input error */
#define ESP_TOK_EOF             0            /* End of file */
#define ESP_TOK_START_ESP       1            /* <% */
#define ESP_TOK_END_ESP         2            /* %> */
#define ESP_TOK_ATAT            3            /* @@var */
#define ESP_TOK_LITERAL         4            /* literal HTML */
#define ESP_TOK_INCLUDE         5            /* include file.esp */
#define ESP_TOK_EQUALS          6            /* = var */

/*
      ESP parser states
 */
#define ESP_STAGE_BEGIN         1            /* Starting state */
#define ESP_STAGE_IN_ESP_TAG    2            /* Inside a <% %> group */

/*
    ESP page parser structure
 */
typedef struct EspParse {
    char    *inBuf;                         /* Input data to parse */
    char    *inp;                           /* Next character for input */
    char    *endp;                          /* End of storage (allow for null) */
    char    *tokp;                          /* Pointer to current parsed token */
    char    *token;                         /* Storage buffer for token */
    int     tokLen;                         /* Length of buffer */
} EspParse;

/*
    Primary ESP control. Singleton
 */
typedef struct Esp {
    MprHashTable    *actions;               /* Table of actions */
    MprHashTable    *views;                 /* Table of views */
    MprHashTable    *modules;               /* Compiled modules */
    MprList         *env;                   /* Environment for compiler */
    MprList         *routes;                /* Ordered list of routes */
    char            *compile;               /* Compile template */
    char            *modDir;                /* Directory for cache files */
    MprTime         lifespan;               /* Default cache lifespan */
    int             reload;                 /* Auto-reload modified ESP source */
    int             keepSource;             /* Preserve generated source */
} Esp;

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
    char            *module;                /* Module containing the controller/action code */
    char            *template;              /* URI template for forming links based on this route */
    char            *patternExpression;     /* Pattern regular expression */
    void            *compiledPattern;       /* Compiled pattern regular expression */
    //  MOB - naming - not a RE
    char            *compiledAction;        /* Matching action to run (compiled) */
} EspRoute;

/*
    ESP request state
 */
typedef struct EspReq {
    Esp             *esp;                   /* Convenient back pointer */
    EspRoute        *route;                 /* Route used for request */
    MprBuf          *cacheBuffer;           /* HTML output caching */
    char            *baseName;              /* Base name of intermediate compiled file */
    char            *module;                /* Name of compiled module */
    char            *source;                /* Name of ESP source */
    char            *path;                  /* Path to controller */
    char            *entry;                 /* Module entry point */
    int             responded;              /* Controller/action has responded in part */
    int             autoFinalize;           /* Request is/will-be auto-finalized */
} EspReq;

typedef void (*EspAction)(HttpConn *conn);
typedef void (*EspView)(HttpConn *conn);

extern bool espCompile(HttpConn *conn, cchar *name, cchar *source, char *module);

//  MOB - move to pcre
#define PCRE_GLOBAL     0x1
extern char *pcre_replace(cchar *str, void *pattern, cchar *replacement, MprList **parts, int flags);

extern ssize espWrite(HttpConn *conn, cchar *buf, ssize len);
extern void espRedirect(HttpConn *conn, int status, cchar *target);
extern void espSetStatus(HttpConn *conn, int status);
extern void espFinalize(HttpConn *conn);
extern bool espFinalized(HttpConn *conn); 
extern bool espSetAutoFinalizing(HttpConn *conn, int on);
extern void espAutoFinalize(HttpConn *conn);

extern void espDefineView(Esp *esp, cchar *path, void *view);

#ifdef __cplusplus
} /* extern C */
#endif

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

