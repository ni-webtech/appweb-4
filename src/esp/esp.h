/*
    esp.h -- Embedded Server Pages (ESP) Module handler.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_ESP
#define _h_ESP 1

/********************************* Includes ***********************************/

#include    "appweb.h"

#if BIT_FEATURE_ESP

#include    "edi.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Forward Declarations **************************/

#if !DOXYGEN
#endif

/********************************** Tunables **********************************/

#define ESP_TOK_INCR        1024                        /**< Growth increment for ESP tokens */
#define ESP_LISTEN          "4000"                      /**< Default listening endpoint for the esp program */
#define ESP_UNLOAD_TIMEOUT  (10)                        /**< Very short timeout for reloading */
#define ESP_LIFESPAN        (3600 * MPR_TICKS_PER_SEC)  /**< Default generated content cache lifespan */

#if UNUSED
/*
    Default compiler settings for ${DEBUG} and ${LIBS} tokens in EspCompile and EspLink
 */
#if BIT_DEBUG
    #if WINDOWS
        #define ESP_DEBUG "-Zi -Od"
    #else
        #define ESP_DEBUG "-g"
    #endif
#else
    #if WINDOWS
        #define ESP_DEBUG "-O"
    #else
        #define ESP_DEBUG "-O2"
    #endif
#endif

#if WINDOWS
    #define ESP_CORE_LIBS "\"${LIBPATH}\\mod_esp${SHLIB}\" \"${LIBPATH}\\libappweb.lib\" \
        \"${LIBPATH}\\libhttp.lib\" \"${LIBPATH}\\libmpr.lib\""
#else
    #define ESP_CORE_LIBS "${LIBPATH}/mod_esp${SHOBJ} -lappweb -lpcre -lhttp -lmpr -lpthread -lm"
#endif

/*
    Default SSL library switches
 */
#if BIT_FEATURE_SSL
    #if WINDOWS
        #define ESP_SSL_LIBS " \"${LIBPATH}\\libmprssl.lib\""
    #else
        #define ESP_SSL_LIBS " -lmprssl"
    #endif
#else
    #define ESP_SSL_LIBS
#endif
#define ESP_LIBS ESP_CORE_LIBS ESP_SSL_LIBS

#if MACOSX
    #define ESP_CCNAME "clang"
#else
    #define ESP_CCNAME "gcc"
#endif
#endif /* UNUSED */

//  MOB - move to bit.h
#define BIT_VISUAL_STUDIO_VERSION "10.0"

/********************************** Defines ***********************************/
/**
    Procedure callback
    @ingroup Esp
 */
typedef void (*EspProc)(HttpConn *conn);

#define CONTENT_MARKER  "${_ESP_CONTENT_MARKER_}"       /* Layout content marker */
#define ESP_SESSION     "-esp-session-"                 /* ESP session cookie name */

#if BIT_WIN_LIKE
    #define ESP_EXPORT __declspec(dllexport)
#else
    #define ESP_EXPORT
#endif
    #define ESP_EXPORT_STRING MPR_STRINGIFY(ESP_EXPORT)

#define ESP_SECURITY_TOKEN_NAME "__esp_security_token__"
#define ESP_FLASH_VAR           "__flash__"

/*
    Default VxWorks environment
 */
#ifndef WIND_BASE
#define WIND_BASE "WIND_BASE-Not-Configured"
#endif
#ifndef WIND_HOME
#define WIND_HOME "WIND_HOME-Not-Configured"
#endif
#ifndef WIND_HOST_TYPE
#define WIND_HOST_TYPE "WIND_HOST_TYPE-Not-Configured"
#endif
#ifndef WIND_PLATFORM
#define WIND_PLATFORM "WIND_PLATFORM-Not-Configured"
#endif
#ifndef WIND_GNU_PATH
#define WIND_GNU_PATH "WIND_GNU_PATH-Not-Configured"
#endif

/********************************** Parsing ***********************************/
/**
    ESP page parser structure
    @defgroup EspParse EspParse
    @see
 */
typedef struct EspParse {
    char    *data;                          /**< Input data to parse */
    char    *next;                          /**< Next character in input */
    MprBuf  *token;                         /**< Storage buffer for token */
    int     lineNumber;                     /**< Line number for error reporting */
} EspParse;

/**
    Top level ESP structure. This is a singleton.
 */
typedef struct Esp {
    MprHash         *actions;               /**< Table of actions */
    MprHash         *views;                 /**< Table of views */
    MprHash         *internalOptions;       /**< Table of internal HTML control options  */
    MprThreadLocal  *local;                 /**< Thread local data */
    MprCache        *sessionCache;          /**< Session state cache */
    MprMutex        *mutex;                 /**< Multithread lock */
    EdiService      *ediService;            /**< Database service */
    int             inUse;                  /**< Active ESP request counter */
} Esp;

/*
    Add HTLM internal options to the Esp.options hash
 */
extern void espInitHtmlOptions(Esp *esp);

/********************************** Routes ************************************/
/**
    EspRoute extended route configuration.
    @defgroup EspRoute EspRoute
    @see
 */
typedef struct EspRoute {
    HttpRoute       *route;                 /**< Back link to the owning route */
    MprHash         *env;                   /**< Environment variables for route */
    char            *compile;               /**< Compile template */
    char            *link;                  /**< Link template */
    char            *searchPath;            /**< Search path to use when locating compiler/linker */
    EspProc         controllerBase;         /**< Initialize base for a controller */

    char            *appModuleName;         /**< App module name when compiled flat */
    char            *appModulePath;         /**< App module path when compiled flat */

    char            *cacheDir;              /**< Directory for cached compiled controllers and views */
    char            *controllersDir;        /**< Directory for controllers */
    char            *dbDir;                 /**< Directory for databases */
    char            *layoutsDir;            /**< Directory for layouts */
    char            *staticDir;             /**< Directory for static web content */
    char            *viewsDir;              /**< Directory for views */

    MprTime         lifespan;               /**< Default cache lifespan */
    int             update;                 /**< Auto-update modified ESP source */
    int             keepSource;             /**< Preserve generated source */
	int				showErrors;				/**< Send server errors back to client */

    Edi             *edi;                   /**< Default database for this route */
} EspRoute;

/**
    Add caching for response content.
    @description This call configures caching for request responses. Caching may be used for any HTTP method, 
    though typically it is most useful for state-less GET requests. Output data may be uniquely cached for requests 
    with different request parameters (query, post and route parameters).
    \n\n
    When server-side caching is requested and manual-mode is not enabled, the request response will be automatically 
    cached. Subsequent client requests will revalidate the cached content with the server. If the server-side cached 
    content has not expired, a HTTP Not-Modified (304) response will be sent and the client will use its client-side 
    cached content.  This results in a very fast transaction with the client as no response data is sent.
    Server-side caching will cache both the response headers and content.
    \n\n
    If manual server-side caching is requested, the response will be automatically cached, but subsequent requests will
    require the handler to explicitly send cached content by calling $httpWriteCached.
    \n\n
    If client-side caching is requested, a "Cache-Control" Http header will be sent to the client with the caching 
    "max-age" set to the lifesecs argument value. This causes the client to serve client-cached 
    content and to not contact the server at all until the max-age expires. 
    Alternatively, you can use $httpSetHeader to explicitly set a "Cache-Control header. For your reference, here are 
    some keywords that can be used in the Cache-Control Http header.
    \n\n
        "max-age" Max time in seconds the resource is considered fresh.
        "s-maxage" Max time in seconds the resource is considered fresh from a shared cache.
        "public" marks authenticated responses as cacheable.
        "private" shared caches may not store the response.
        "no-cache" cache must re-submit request for validation before using cached copy.
        "no-store" response may not be stored in a cache.
        "must-revalidate" forces clients to revalidate the request with the server.
        "proxy-revalidate" similar to must-revalidate except only for proxy caches.
    \n\n
    Use client-side caching for static content that will rarely change or for content for which using "reload" in 
    the browser is an adequate solution to force a refresh. Use manual server-side caching for situations where you need to
    explicitly control when and how cached data is returned to the client. For most other situations, use server-side
    caching.
    @param route HttpRoute object
    @param uri URI to cache. 
        If the URI is set to "*" all URIs for that action are uniquely cached. If the request has POST data, 
        the URI may include such post data in a sorted query format. E.g. {uri: /buy?item=scarf&quantity=1}.
    @param lifesecs Lifespan of cache items in seconds. If not set to positive integer, the lifesecs will
        default to the route lifespan.
    @param flags Cache control flags. Select ESP_CACHE_MANUAL to enable manual mode. In manual mode, cached content
        will not be automatically sent. Use $httpWriteCached in the request handler to write previously cached content.
        \n\n
        Select ESP_CACHE_CLIENT to enable client-side caching. In this mode a "Cache-Control" Http header will be 
        sent to the client with the caching "max-age". WARNING: the client will not send any request for this URI
        until the max-age timeout has expired.
        \n\n
        Select HTTP_CACHE_RESET to first reset existing caching configuration for this route.
        \n\n
        Select HTTP_CACHE_COMBINED, HTTP_CACHE_ONLY or HTTP_CACHE_UNIQUE to define the server-side caching mode. Only
        one of these three mode flags should be specified.
        \n\n
        If the HTTP_CACHE_COMBINED flag is set, the request params (query, post data and route parameters) will be
        ignored and all request for a given URI path will cache to the same cache record.
        \n\n
        Select HTTP_CACHE_UNIQUE to uniquely cache requests with different request parameters. The URIs specified in 
        $uris should not contain any request parameters.
        \n\n
        Select HTTP_CACHE_ONLY to cache only the exact URI with parameters specified in $uris. The parameters must be 
        in sorted www-urlencoded format. For example: /example.esp?hobby=sailing&name=john.
    @return A count of the bytes actually written
    @ingroup EspRoute
 */
extern int espCache(HttpRoute *route, cchar *uri, int lifesecs, int flags);

/**
    Compile a view or controller
    @description This compiles ESP controllers and views into loadable, cached modules
    @param conn Http connection object
    @param source ESP source file name
    @param module Output module file name
    @param cacheName MD5 cache name. Not a full path
    @param isView Set to "true" if the source is a view
    @return "True" if the compilation is successful. Errors are logged and sent back to the client if EspRoute.showErrors
        is true.
    @ingroup EspRoute
 */
extern bool espCompile(HttpConn *conn, cchar *source, cchar *module, cchar *cacheName, int isView);

/**
    Convert an ESP web page into compilable C code
    @description This parses an ESP web page into an equivalent C source view.
    @param route EspRoute object
    @param page ESP web page script.
    @param path Pathname for the ESP web page. This is used to process include directives which are resolved relative
        to this path.
    @param cacheName MD5 cache name. Not a full path.
    @param layout Default layout page.
    @param err Output parameter to hold any relevant error message.
    @return C language code string for the web page
    @ingroup EspRoute
 */
extern char *espBuildScript(HttpRoute *route, cchar *page, cchar *path, cchar *cacheName, cchar *layout, char **err);

/**
    Define an action
    @description Actions are C procedures that are invoked when specific URIs are routed to the controller/action pair.
    @param route Route object
    @param targetKey Target key used to select the action in a HttpRoute target. This is typically a URI prefix.
    @param actionProc EspProc callback procedure to invoke when the action is requested.
    @ingroup EspRoute
 */
extern void espDefineAction(HttpRoute *route, cchar *targetKey, void *actionProc);

/**
    Define a base function to invoke for all controller actions.
    @description A base function can be defined that will be called before calling any controller action. This
        emulates a super class constructor.
    @param route Route object
    @param baseProc Function to call just prior to invoking a controller action.
    @ingroup EspRoute
 */
extern void espDefineBase(HttpRoute *route, EspProc baseProc);

/**
    Define a view
    @description Views are ESP web pages that are executed to return presentation data back to the client.
    @param route Http route object
    @param path Path to the ESP view source code.
    @param viewProc EspViewPrococ callback procedure to invoke when the view is requested.
    @ingroup EspRoute
 */
extern void espDefineView(HttpRoute *route, cchar *path, void *viewProc);

/**
    Expand a compile or link command template
    @description This expands a command template and replaces "${tokens}" with their equivalent value. The supported
        tokens are:
        <ul>
            <li>ARCH - Build architecture (i386, x86_64)</li>
            <li>CC - Compiler pathname</li>
            <li>DEBUG - Compiler debug options (-g, -Zi, -Od)</li>
            <li>INC - Include directory (out/inc)</li>
            <li>LIB - Library directory (out/lib, out/bin)</li>
            <li>LIBS - Required libraries directory (mod_esp, libappweb)</li>
            <li>OBJ - Name of compiled source (out/lib/view-MD5.o)</li>
            <li>OUT - Output module (view_MD5.dylib)</li>
            <li>SHLIB - Shared library extension (.lib, .so)</li>
            <li>SHOBJ - Shared object extension (.dll, .so)</li>
            <li>SRC - Path to source code for view or controller (already templated)</li>
            <li>TMP - System temporary directory</li>
            <li>WINSDK - Path to the Windows SDK</li>
            <li>VS - Path to Visual Studio</li>
        </ul>
    @param eroute Esp route object
    @param command Http connection object
    @param source ESP web page source pathname
    @param module Output module pathname
    @return An expanded command line
    @ingroup EspRoute
 */
extern char *espExpandCommand(EspRoute *eroute, cchar *command, cchar *source, cchar *module);

/*
    Internal
 */
extern void espManageEspRoute(EspRoute *eroute, int flags);

/********************************** Session ***********************************/
/**
    ESP session state object
    @defgroup EspSession EspSession
    @see
 */
typedef struct EspSession {
    char            *id;                    /**< Session ID key */
    MprCache        *cache;                 /**< Cache store reference */
    MprTime         lifespan;               /**< Session inactivity timeout (msecs) */
} EspSession;

/**
    Allocate a new session state object.
    @description
    @param conn Http connection object
    @param id Unique session state ID
    @param lifespan Session lifespan in ticks
    @return A session state object
    @ingroup EspSession
 */
extern EspSession *espAllocSession(HttpConn *conn, cchar *id, MprTime lifespan);

/**
    Create a session object.
    @description This call creates a session object if one does not already exist.
        Session state stores persist across individual HTTP requests.
    @param conn Http connection object
    @return A session state object
    @ingroup EspSession
 */
extern EspSession *espCreateSession(HttpConn *conn);

/**
    Destroy a session state object.
    @description
    @param sp Session state object allocated with #espAllocSession
    @ingroup EspSession
 */
extern void espDestroySession(EspSession *sp);

/**
    Get a session state object.
    @description
    @param conn Http connection object
    @param create Set to "true" to create a session state object if one does not already exist for this client
    @return A session state object
    @ingroup EspSession
 */
extern EspSession *espGetSession(HttpConn *conn, int create);

/**
    Get a session state variable.
    @description
    @param conn Http connection object
    @param name Variable name to get
    @param defaultValue If the variable does not exist, return the defaultValue.
    @return The variable value or defaultValue if it does not exist.
    @ingroup EspSession
 */
extern cchar *espGetSessionVar(HttpConn *conn, cchar *name, cchar *defaultValue);

/**
    Set a session variable.
    @description
    @param conn Http connection object
    @param name Variable name to set
    @param value Variable value to use
    @return A session state object
    @ingroup EspSession
 */
extern int espSetSessionVar(HttpConn *conn, cchar *name, cchar *value);

/**
    Get the session ID.
    @description
    @param conn Http connection object
    @return The session ID string
    @ingroup EspSession
 */
extern char *espGetSessionID(HttpConn *conn);

/**
    Set an object into the session state store.
    @description Store an object in the session state store by serializing all properties.
    @param conn Http connection object
    @param key Session state key
    @param value Object to serialize
    @ingroup EspSession
 */
extern int espSetSessionObj(HttpConn *conn, cchar *key, MprHash *value);

/**
    Get an object from the session state store.
    @description Retrieve an object from the session state store by deserializing all properties.
    @param conn Http connection object
    @param key Session state key
    @ingroup EspSession
 */
extern MprHash *espGetSessionObj(HttpConn *conn, cchar *key);

/********************************** Requests **********************************/
/**
    View procedure callback.
    @param conn Http connection object
    @ingroup EspReq
 */
typedef void (*EspViewProc)(HttpConn *conn);

//  MOB - simplify EspAction to just EspProc
/**
    ESP Action
    @description Actions are run after a request URI is routed to a controller.
    @ingroup EspReq
    @see
 */
typedef struct EspAction {
    EspProc         actionProc;             /**< Action procedure to run to respond to the request */
} EspAction;

void espManageAction(EspAction *ap, int flags);

/**
    ESP request
    @defgroup EspReq EspReq
    @see
 */
typedef struct EspReq {
    HttpRoute       *route;                 /**< Route reference */
    EspRoute        *eroute;                /**< Extended route info */
    EspSession      *session;               /**< Session data object */
    EspAction       *action;                /**< Action to invoke */
    Esp             *esp;                   /**< Convenient esp reference */
    MprHash         *flash;                 /**< New flash messages */
    MprHash         *lastFlash;             /**< Flash messages from the last request */
    char            *cacheName;             /**< Base name of intermediate compiled file */
    char            *controllerName;        /**< Controller name */
    char            *controllerPath;        /**< Path to controller source */
    char            *module;                /**< Name of compiled module */
    char            *source;                /**< Name of ESP source */
    char            *view;                  /**< Path to view */
    char            *entry;                 /**< Module entry point */
    char            *commandLine;           /**< Command line for compile/link */
    int             autoFinalize;           /**< Request is or will be auto-finalized */
    int             sessionProbed;          /**< Already probed for session store */
    int             appLoaded;              /**< App module already probed */
    int             lastDomID;              /**< Last generated DOM ID */
    EdiRec          *record;                /**< Current data record */
} EspReq;

/**
    Add a header to the transmission using a format string.
    @description Add a header if it does not already exist.
    @param conn HttpConn connection object
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup EspReq
 */
extern void espAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/**
    Add a header to the transmission.
    @description Add a header if it does not already exist.
    @param conn HttpConn connection object
    @param key Http response header key
    @param value Value to set for the header
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup EspReq
 */
extern void espAddHeaderString(HttpConn *conn, cchar *key, cchar *value);

/**
    Append a transmission header.
    @description Set the header if it does not already exist. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup EspReq
 */
extern void espAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/**
    Append a transmission header string.
    @description Set the header if it does not already exist. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object
    @param key Http response header key
    @param value Value to set for the header
    @ingroup EspReq
 */
extern void espAppendHeaderString(HttpConn *conn, cchar *key, cchar *value);

/**
    Auto-finalize transmission of the http request.
    @description If auto-finalization is enabled via #espSetAutoFinalizing, this call will finalize writing Http response
    data by writing the final chunk trailer if required. If using chunked transfers, a null chunk trailer is required
    to signify the end of write data.  If the request is already finalized, this call does nothing.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espAutoFinalize(HttpConn *conn);

/**
    Check a security token.
    @description Check the request security token. If a required security token is defined in the session state, the
    request must supply the same token with all POST requests. This helps mitigate potential CSRF threats.
    Security tokens are used to help guard against CSRF threats. If a template web page includes the
        securityToken() control, a security token will be added to the meta section of the generated HTML. When a 
        form is posted from this page, the ESP jQuery script will add the security token to the form parameters.
        This call validates the security token to ensure it matches the security token stored in session state.
    @param conn Http connection object
    @return False if the request is a POST request and the security token does not match the session held token.
        Otherwise return "true".
    @ingroup EspReq
 */
extern bool espCheckSecurityToken(HttpConn *conn);

/**
    Create a record and initialize field values.
    @description This will call $ediCreateRec to create a record based on the given table's schema. It will then
        call $ediSetFields to update the record with the given data.
    @param conn Http connection object
    @param tableName Database table name
    @param data Hash of field values
    @return EdRec instance
    @ingroup EspReq
 */
extern EdiRec *espCreateRec(HttpConn *conn, cchar *tableName, MprHash *data);

/**
    Finalize transmission of the http request.
    @description Finalize writing HTTP data by writing the final chunk trailer if required. If using chunked transfers,
    a null chunk trailer is required to signify the end of write data.
    If the request is already finalized, this call does nothing.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espFinalize(HttpConn *conn);

/**
    Flush transmit data. 
    @description This writes any buffered data.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espFlush(HttpConn *conn);

/**
    Get a list of column names.
    @param conn HttpConn connection object
    @param rec Database record. If set to NULL, the current database record defined via $form() is used.
    @return An MprList of column names in the given table. If there is no record defined, an empty list is returned.
    @ingroup EspReq
 */
extern MprList *espGetColumns(HttpConn *conn, EdiRec *rec);

/**
    Get the current request connection.
    @return The HttpConn connection object
    @ingroup EspReq
 */
extern HttpConn *espGetConn();

/**
    Get the receive body content length.
    @description Get the length of the receive body content (if any). This is used in servers to get the length of posted
        data and, in clients, to get the response body length.
    @param conn HttpConn connection object
    @return A count of the response content data in bytes.
    @ingroup EspReq
 */
extern MprOff espGetContentLength(HttpConn *conn);

/**
    Get the receive body content type.
    @description Get the content mime type of the receive body content (if any).
    @param conn HttpConn connection object
    @return Mime type of any receive content. Set to NULL if not posted data.
    @ingroup EspReq
 */
extern cchar *espGetContentType(HttpConn *conn);

/**
    Get the request cookies.
    @description Get the cookies defined in the current request
    @param conn HttpConn connection object
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup EspReq
 */
extern cchar *espGetCookies(HttpConn *conn);

/**
    Get the current database instance.
    @description A route may have a default database configured via the EspDb Appweb.conf configuration directive. 
    The database will be opened when the web server initializes and will be shared between all requests using the route. 
    @return Edi EDI database handle
    @ingroup EspReq
 */
extern Edi *espGetDatabase(HttpConn *conn);

/**
    Get the current extended route information.
    @return EspRoute instance
    @ingroup EspReq
 */
extern EspRoute *espGetEspRoute(HttpConn *conn);

/**
    Get the default document root directory for the request route.
    @param conn HttpConn connection object
    @return A directory path name 
    @ingroup EspReq
 */
extern cchar *espGetDir(HttpConn *conn);

/**
    Get a flash message.
    @description This retrieves a flash message of a specified type.
        Flash messages are special session state messages that are passed to the next request (only). 
    @param conn HttpConn connection object
    @param type Type of flash message to retrieve. Possible types include: "error", "inform", "warning", "all".
    @ingroup EspReq
 */
extern cchar *espGetFlashMessage(HttpConn *conn, cchar *type);

/**
    Get the current database grid.
    @description The current grid is defined via $setGrid
    @return EdiGrid instance
    @ingroup EdiReq
    @internal
 */
extern EdiGrid *espGetGrid(HttpConn *conn);

/**
    Get an rx http header.
    @description Get a http response header for a given header key.
    @param conn HttpConn connection object
    @param key Name of the header to retrieve. This should be a lower case header name. For example: "Connection"
    @return Value associated with the header key or null if the key did not exist in the response.
    @ingroup EspReq
 */
extern cchar *espGetHeader(HttpConn *conn, cchar *key);

/**
    Get the hash table of rx Http headers.
    @description Get the internal hash table of rx headers
    @param conn HttpConn connection object
    @return Hash table. See MprHash for how to access the hash table.
    @ingroup EspReq
 */
extern MprHash *espGetHeaderHash(HttpConn *conn);

/**
    Get all the request http headers.
    @description Get all the rx headers. The returned string formats all the headers in the form:
        key: value\\nkey2: value2\\n...
    @param conn HttpConn connection object
    @return String containing all the headers. The caller must free this returned string.
    @ingroup EspReq
 */
extern char *espGetHeaders(HttpConn *conn);

/**
    Get a request pararmeter as an integer.
    @description Get the value of a named request parameter as an integer. Form variables are defined via
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return Integer containing the request parameter's value
    @ingroup EspReq
 */
extern int espGetIntParam(HttpConn *conn, cchar *var, int defaultValue);

/**
    Get the HTTP method.
    @description This is a convenience API to return the Http method 
    @return The HttpConn.rx.method property
    @ingroup EspReq
 */
extern cchar *espGetMethod(HttpConn *conn);

/**
    Get a request parameter.
    @description Get the value of a named request parameter. Form variables are defined via www-urlencoded query or post
        data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return String containing the request parameter's value. Caller should not free.
    @ingroup EspReq
 */
extern cchar *espGetParam(HttpConn *conn, cchar *var, cchar *defaultValue);

/**
    Get the request parameter hash table.
    @description This call gets the params hash table for the current request.
        Route tokens, request query data, and www-url encoded form data are all entered into the params table after decoding.
        Use #mprLookupKey to retrieve data from the table.
    @param conn HttpConn connection object
    @return #MprHash instance containing the request parameters
    @ingroup EspReq
 */
extern MprHash *espGetParams(HttpConn *conn);

/**
    Get the request query string.
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup EspReq
 */
extern cchar *espGetQueryString(HttpConn *conn);

/**
    Get the referring URI.
    @description This returns the referring URI as described in the HTTP "referer" (yes the HTTP specification does
        spell it incorrectly) header. If this header is not defined, this routine will return the home URI as returned
        by $espGetHome.
    @param conn HttpConn connection object
    @return String URI back to the referring URI. If no referrer is defined, refers to the home URI.
    @ingroup EspReq
 */
char *espGetReferrer(HttpConn *conn);

/**
    Get the default database defined on a route.
    @param route HttpRoute object
    @return Database instance object
    @ingroup EspReq
 */
Edi *espGetRouteDatabase(HttpRoute *route);

/**
    Get a unique security token.
    @description Security tokens help mitigate against replay attacks. The security token is stored in HttpRx.securityToken
        and in the session store.
    @param conn HttpConn connection object
    @return The security token string
 */
extern cchar *espGetSecurityToken(HttpConn *conn);

/**
    Get the response status.
    @param conn HttpConn connection object
    @return An integer Http response code. Typically 200 is success.
    @ingroup EspReq
 */
extern int espGetStatus(HttpConn *conn);

/**
    Get the Http response status message. 
    @description The HTTP status message is supplied on the first line of the HTTP response.
    @param conn HttpConn connection object
    @returns A Http status message.
    @ingroup EspReq
 */
extern char *espGetStatusMessage(HttpConn *conn);

/**
    Get a relative URI to the top of the application.
    @description This will return an absolute URI for the top of the application. This will be "/" if there is no
        application script name. Otherwise, it will return a URI for the script name for the application.
    @param conn HttpConn connection object
    @return String Absolute URI to the top of the application
    @ingroup EspReq
 */
char *espGetTop(HttpConn *conn);

/**
    Get the uploaded files.
    @description Get the hash table defining the uploaded files.
        This hash is indexed by the file identifier supplied in the upload form. The hash entries are HttpUploadFile
        objects.
    @param conn HttpConn connection object
    @return A hash of HttpUploadFile objects.
    @ingroup EspReq
 */
extern MprHash *espGetUploads(HttpConn *conn);

/**
    Get the request URI string.
    @description This is a convenience API to return the request URI.
    @return The espGetConn()->rx->uri
    @ingroup EspReq
 */
extern cchar *espGetUri(HttpConn *conn);

/**
    Test if a current grid has been defined.
    @description The current grid is defined via $setRec
    @return "True" if a current grid has been defined
    @ingroup EspReq
 */
extern bool espHasGrid(HttpConn *conn);

/**
    Test if a current record has been defined and save to the database.
    @description This call returns "true" if a current record is defined and has been saved to the database with a 
        valid "id" field.
    @return "True" if a current record with a valid "id" is defined.
    @ingroup EspReq
 */
extern bool espHasRec(HttpConn *conn);

/**
    Test if the receive input stream is at end-of-file.
    @param conn HttpConn connection object
    @return "True" if there is no more receive data to read
    @ingroup EspReq
 */
extern bool espIsEof(HttpConn *conn);

/**
    Test if the connection is using SSL and is secure.
    @param conn HttpConn connection object
    @return "True" if the connection is using SSL.
    @ingroup EspReq
 */
extern bool espIsSecure(HttpConn *conn);

/**
    Test if a http request is finalized.
    @description This tests if #espFinalize or #httpFinalize has been called for a request.
    @param conn HttpConn connection object
    @return "True" if the request has been finalized.
    @ingroup EspReq
 */
extern bool espIsFinalized(HttpConn *conn);

/**
    Make a grid.
    @description This call makes a free-standing data grid based on the JSON format content string.
        The record is not saved to the database.
    @param content JSON format content string. The content should be an array of objects where each object is a
        set of property names and values.
    @return An EdiGrid instance
    @example:
grid = ediMakeGrid("[ \\ \n
    { id: '1', country: 'Australia' }, \ \n
    { id: '2', country: 'China' }, \ \n
    ]");
    @ingroup EspReq
 */
extern EdiGrid *espMakeGrid(cchar *content);

/**
    Make a hash table container of property values.
    @description This routine formats the given arguments, parses the result as a JSON string and returns an 
        equivalent hash of property values. The result after formatting should be of the form:
        hash("{ key: 'value', key2: 'value', key3: 'value' }");
    @param fmt Printf style format string
    @param ... arguments
    @return MprHash instance
    @ingroup EspReq
 */
extern MprHash *espMakeHash(cchar *fmt, ...);

/**
    Make a record.
    @description This call makes a free-standing data record based on the JSON format content string.
        The record is not saved to the database.
    @param content JSON format content string. The content should be a set of property names and values.
    @return An EdiRec instance
    @example: rec = ediMakeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
    @ingroup EspReq
 */
extern EdiRec *espMakeRec(cchar *content);

/**
    Match a request parameter with an expected value.
    @description Compare a request parameter and return "true" if it exists and its value matches.
    @param conn HttpConn connection object
    @param var Name of the request parameter
    @param value Expected value to match
    @return "True" if the value matches
    @ingroup EspReq
 */
extern bool espMatchParam(HttpConn *conn, cchar *var, cchar *value);

/**
    Read all the records in table from the database.
    @description This reads a table and returns a grid containing the table data.
    @param conn HttpConn connection object
    @param tableName Database table name
    @return A grid containing all table rows. Returns NULL if the table cannot be found.
    @ingroup EspReq
 */
extern EdiGrid *espReadAllRecs(HttpConn *conn, cchar *tableName);

/**
    Read the identified record. 
    @description Read the record identified by the request params("id") from the nominated table.
    @param conn HttpConn connection object
    @param tableName Database table name
    @return The identified record. Returns NULL if the table or record cannot be found.
    @ingroup EspReq
 */
extern EdiRec *espReadRec(HttpConn *conn, cchar *tableName);

/**
    Read matching records
    @description This runs a simple query on the database and returns matching records in a grid. The query selects
        all rows that have a "field" that matches the given "value".
    @param conn HttpConn connection object
    @param tableName Database table name
    @param fieldName Database field name to evaluate
    @param operation Comparison operation. Set to "==", "!=", "<", ">", "<=" or ">=".
    @param value Data value to compare with the field values.
    @return A grid containing all matching records. Returns NULL if no matching records.
    @ingroup EspReq
 */
extern EdiGrid *espReadRecsWhere(HttpConn *conn, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);

/**
    Read one record.
    @description This runs a simple query on the database and selects the first matching record. The query selects
        a row that has a "field" that matches the given "value".
    @param conn HttpConn connection object
    @param tableName Database table name
    @param fieldName Database field name to evaluate
    @param operation Comparison operation. Set to "==", "!=", "<", ">", "<=" or ">=".
    @param value Data value to compare with the field values.
    @return First matching record. Returns NULL if no matching records.
    @ingroup EspReq
 */
extern EdiRec *espReadRecWhere(HttpConn *conn, cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);

/**
    Read a record identified by the key value.
    @description Read a record from the given table as identified by the key value.
    @param tableName Database table name
    @param key Key value of the record to read 
    @return Record instance of EdiRec.
    @ingroup EspReq
 */
extern EdiRec *eReadRecByKey(cchar *tableName, cchar *key);

/**
    Read receive body content.
    @description Read body content from the client
    @param conn HttpConn connection object
    @param buf Buffer to accept content data
    @param size Size of the buffer
    @return A count of bytes read into the buffer
    @ingroup EspReq
 */
extern ssize espReceive(HttpConn *conn, char *buf, ssize size);

/**
    Redirect the client.
    @description Redirect the client to a new uri.
    @param conn HttpConn connection object
    @param status Http status code to send with the response
    @param target New target uri for the client
    @ingroup EspReq
 */
extern void espRedirect(HttpConn *conn, int status, cchar *target);

/**
    Redirect the client back to the referrer
    @description Redirect the client to the referring URI.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espRedirectBack(HttpConn *conn);

/**
    Remove a header from the transmission
    @description Remove a header if present.
    @param conn HttpConn connection object
    @param key Http response header key
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EspReq
 */
extern int espRemoveHeader(HttpConn *conn, cchar *key);

/**
    Remove a record from a database table
    @description Remove the record identified by the key value from the given table.
    @param conn HttpConn connection object
    @param tableName Database table name
    @param key Key value of the record to remove 
    @return Record instance of EdiRec.
    @ingroup EdiReq
 */
extern bool espRemoveRec(HttpConn *conn, cchar *tableName, cchar *key);

/**
    Render a formatted string.
    @description Render a formatted string of data into packets to the client. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param conn HttpConn connection object
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRender(HttpConn *conn, cchar *fmt, ...);

/**
    Render a block of data to the client.
    @description Render a block of data to the client. Data packets will be created as required to store the write data.
    @param conn HttpConn connection object
    @param buf Buffer containing the write data
    @param size Size of the data in buf
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRenderBlock(HttpConn *conn, cchar *buf, ssize size);

/**
    Render cached content.
    @description Render the saved, cached response from a prior request to this URI. This is useful if the caching
        mode has been set to "manual".
    @param conn HttpConn connection object
    @return A count of the bytes actually written
    @ingroup EspReq
 */
ssize espRenderCached(HttpConn *conn);

/**
    Render an error message back to the client and finalize the request. The output is Html escaped for security.
    @param conn HttpConn connection object
    @param status Http status code
    @param fmt Printf style message format
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRenderError(HttpConn *conn, int status, cchar *fmt, ...);

/**
    Render the contents of a file back to the client.
    @param conn HttpConn connection object
    @param path File path name
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRenderFile(HttpConn *conn, cchar *path);

/**
    Render the value of a request parameter to the client.
    @description This writes the value of a request parameter after HTML escaping its value.
    @param conn HttpConn connection object
    @param name Form variable name
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRenderParam(HttpConn *conn, cchar *name);

/**
    Render a safe string of data to the client.
    @description HTML escape a string and then write the string of data to the client.
        Data packets will be created as required to store the write data. This call may block waiting for the data to
        the client to drain.
    @param conn HttpConn connection object
    @param s String containing the data to write
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRenderSafeString(HttpConn *conn, cchar *s);

/**
    Render a string of data to the client
    @description Render a string of data to the client. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param conn HttpConn connection object
    @param s String containing the data to write
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espRenderString(HttpConn *conn, cchar *s);

/**
    Render a view template to the client
    @description Actions are C procedures that are invoked when specific URIs are routed to the controller/action pair.
    @param conn Http connection object
    @param name view name
    @ingroup EspReq
 */
extern void espRenderView(HttpConn *conn, cchar *name);

/**
    Enable auto-finalizing for this request
    @param conn HttpConn connection object
    @param on Set to "true" to enable auto-finalizing.
    @return "True" if auto-finalizing was enabled prior to this call
    @ingroup EspReq
 */
extern bool espSetAutoFinalizing(HttpConn *conn, bool on);

/**
    Set the current request connection.
    @param conn The HttpConn connection object to define
    @ingroup EspReq
 */
extern void espSetConn(HttpConn *conn);

/**
    Define a content length header in the transmission. 
    @description This will define a "Content-Length: NNN" request header.
    @param conn HttpConn connection object
    @param length Numeric value for the content length header.
    @ingroup EspReq
 */
extern void espSetContentLength(HttpConn *conn, MprOff length);

/**
    Set a cookie in the transmission
    @description Define a cookie to send in the transmission Http header
    @param conn HttpConn connection object
    @param name Cookie name
    @param value Cookie value
    @param path URI path to which the cookie applies
    @param domain String Domain in which the cookie applies. Must have 2-3 "." and begin with a leading ".". 
        For example: domain: .example.com.
    Some browsers will accept cookies without the initial ".", but the spec: (RFC 2109) requires it.
    @param lifespan Duration for the cookie to persist in msec
    @param isSecure Set to "true" if the cookie only applies for SSL based connections
    @ingroup EspReq
 */
extern void espSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *domain, MprTime lifespan,
        bool isSecure);

/**
    Set the transmission (response) content mime type
    @description Set the mime type Http header in the transmission
    @param conn HttpConn connection object
    @param mimeType Mime type string
    @ingroup EspReq
 */
extern void espSetContentType(HttpConn *conn, cchar *mimeType);

/**
    Update a record field without writing to the database
    @description This routine updates the record object with the given value. The record will not be written
        to the database. To write to the database, use $writeRec.
    @param rec Record to update
    @param fieldName Record field name to update
    @param value Value to update
    @return The record instance if successful, otherwise NULL.
    @ingroup EspReq
 */
extern EdiRec *espSetField(EdiRec *rec, cchar *fieldName, cchar *value);

/**
    Update record fields without writing to the database
    @description This routine updates the record object with the given values. The "data' argument supplies 
        a hash of fieldNames and values. The data hash may come from the request $params() or it can be manually
        created via #ediMakeHash to convert a JSON string into an options hash.
        For example: updateFields(rec, hash("{ name: '%s', address: '%s' }", name, address))
        The record will not be written
        to the database. To write to the database, use $ediWriteRec.
    @param rec Record to update
    @param data Hash of field names and values to use for the update
    @return The record instance if successful, otherwise NULL.
    @ingroup EspReq
 */
extern EdiRec *espSetFields(EdiRec *rec, MprHash *data);

//  MOB - improve doc. Should be set not send
/**
    Send a flash message
    @param conn Http connection object
    @param kind Kind of flash message
    @param fmt Printf style formatted string to use as the message
    @ingroup EspReq
 */
void espSetFlash(HttpConn *conn, cchar *kind, cchar *fmt, ...);

//  MOB - improve doc. Should be set not send
/**
    Send a flash message
    @param conn Http connection object
    @param kind Kind of flash message
    @param fmt Printf style formatted string to use as the message
    @param args Varargs style list
    @ingroup EspReq
    @internal
 */
void espSetFlashv(HttpConn *conn, cchar *kind, cchar *fmt, va_list args);

/**
    Set the current database grid
    @return The grid instance. This permits chaining.
    @ingroup EspReq
    @internal
 */
extern EdiGrid *espSetGrid(HttpConn *conn, EdiGrid *grid);

/**
    Set a transmission header
    @description Set a Http header to send with the request. If the header already exists, its value is overwritten.
    @param conn HttpConn connection object
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup EspReq
 */
extern void espSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/**
    Set a simple key/value transmission header
    @description Set a Http header to send with the request. If the header already exists, its value is overwritten.
    @param conn HttpConn connection object
    @param key Http response header key
    @param value String value for the key
    @ingroup EspReq
 */
extern void espSetHeaderString(HttpConn *conn, cchar *key, cchar *value);

/**
    Set an integer request parameter value
    @description Set the value of a named request parameter to an integer value. Form variables are defined via
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to set
    @param value Value to set.
    @ingroup EspReq
 */
extern void espSetIntParam(HttpConn *conn, cchar *var, int value);

/**
    Set a Http response status.
    @description Set the Http response status for the request. This defaults to 200 (OK).
    @param conn HttpConn connection object
    @param status Http status code.
    @ingroup EspReq
 */
extern void espSetStatus(HttpConn *conn, int status);

/**
    Set a request parameter value
    @description Set the value of a named request parameter to a string value. Form variables are defined via
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to set
    @param value Value to set.
    @ingroup EspReq
 */
extern void espSetParam(HttpConn *conn, cchar *var, cchar *value);

/**
    Set the current database record
    @description The current record is used to supply data to various abbreviated controls, such as: text(), input(), 
        checkbox and dropdown()
    @param conn HttpConn connection object
    @param rec Record object to define as the current record.
    @return The grid instance. This permits chaining.
    @ingroup EspReq
    @internal
 */
extern EdiRec *espSetRec(HttpConn *conn, EdiRec *rec);

/**
    Show request details
    @description This e request details back to the client. This is useful as a debugging tool.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espShowRequest(HttpConn *conn);

/**
    Update the cached content for a request
    @description Save the given content for future requests. This is useful if the caching mode has been set to "manual". 
    @param conn HttpConn connection object
    @param uri Request URI to cache for
    @param data Data to cache
    @param lifesecs Time in seconds to cache the data
    @ingroup EspReq
 */
extern void espUpdateCache(HttpConn *conn, cchar *uri, cchar *data, int lifesecs);

/**
    Write a value to a database table field
    @description Update the value of a table field in the selected table row. Note: validations are not run.
    @param conn HttpConn connection object
    @param tableName Database table name
    @param key Key value for the table row to update.
    @param fieldName Column name to update
    @param value Value to write to the database field
    @return "true" if the field  can be successfully written.
    @ingroup EspReq
 */
extern bool espUpdateField(HttpConn *conn, cchar *tableName, cchar *key, cchar *fieldName, cchar *value);

/**
    Write field values to a database row
    @description This routine updates the current record with the given data and then saves the record to
        the database. The "data' argument supplies 
        a hash of fieldNames and values. The data hash may come from the request $params() or it can be manually
        created via #ediMakeHash to convert a JSON string into an options hash.
        For example: ediWriteFields(rec, params());
        The record runs field validations before saving to the database.
    @param conn HttpConn connection object
    @param tableName Database table name
    @param data Hash of field names and values to use for the update
    @return "true" if the field  can be successfully written. Returns false if field validations fail.
    @ingroup EspReq
 */
extern bool espUpdateFields(HttpConn *conn, cchar *tableName, MprHash *data);

/**
    Write a record to the database
    @description The record will be saved to the database after running any field validations. If any field validations
        fail to pass, the record will not be written and error details can be retrieved via $ediGetRecErrors.
        If the record is a new record and the "id" column is EDI_AUTO_INC, then the "id" will be assigned
        prior to saving the record.
    @param conn HttpConn connection object
    @param rec Record to write to the database.
    @return "true" if the record can be successfully written.
    @ingroup EspReq
 */
extern bool espUpdateRec(HttpConn *conn, EdiRec *rec);

//  MOB - should httpLink be renamed to httpUri and espLink to httpUri
/**
    Create a URI. 
    @description Create a URI link by expansions tokens based on the current request and route state.
    The target parameter may contain partial or complete URI information. The missing parts 
    are supplied using the current request and route tables. The resulting URI is a normalized, server-local 
    URI (that begins with "/"). The URI will include any defined route prefix, but will not include scheme, host or 
    port components.
    @param conn HttpConn connection object
    @param target The URI target. The target parameter can be a URI string or JSON style set of options. 
        The target will have any embedded "{tokens}" expanded by using token values from the request parameters.
        If the target has an absolute URI path, that path is used directly after tokenization. If the target begins with
        "~", that character will be replaced with the route prefix. This is a very convenient way to create application 
        top-level relative links.
        \n\n
        If the target is a string that begins with "{AT}" it will be interpreted as a controller/action pair of the 
        form "{AT}Controller/action". If the "controller/" portion is absent, the current controller is used. If 
        the action component is missing, the "list" action is used. A bare "{AT}" refers to the "list" action 
        of the current controller.
        \n\n
        If the target starts with "{" it is interpreted as being a JSON style set of options that describe the link.
        If the target is a relative URI path, it is appended to the current request URI path.  
        \n\n
        If the target is a JSON style of options, it can specify the URI components: scheme, host, port, path, reference and
        query. If these component properties are supplied, these will be combined to create a URI.
        \n\n
        If the target specifies either a controller/action or a JSON set of options, The URI will be created according 
        to the route URI template. The template may be explicitly specified
        via a "route" target property. Otherwise, if an "action" property is specified, the route of the same
        name will be used. If these don't result in a usable route, the "default" route will be used. 
        \n\n
        These are the properties supported in a JSON style "{ ... }" target:
        <ul>
            <li>scheme String URI scheme portion</li>
            <li>host String URI host portion</li>
            <li>port Number URI port number</li>
            <li>path String URI path portion</li>
            <li>reference String URI path reference. Does not include "#"</li>
            <li>query String URI query parameters. Does not include "?"</li>
            <li>controller String Controller name if using a Controller-based route. This can also be specified via
                the action option.</li>
            <li>action String Action to invoke. This can be a URI string or a Controller action of the form
                {AT}Controller/action.</li>
            <li>route String Route name to use for the URI template</li>
        </ul>
    @return A normalized, server-local Uri string.
    @example espUri(conn, "http://example.com/index.html", 0); \n
    espUri(conn, "/path/to/index.html", 0); \n
    espUri(conn, "../images/splash.png", 0); \n
    espUri(conn, "~/static/images/splash.png", 0); \n
    espUri(conn, "${app}/static/images/splash.png", 0); \n
    espUri(conn, "@controller/checkout", 0); \n
    espUri(conn, "@controller/") \n
    espUri(conn, "@init") \n
    espUri(conn, "@") \n
    espUri(conn, "{ action: '@post/create' }", 0); \n
    espUri(conn, "{ action: 'checkout' }", 0); \n
    espUri(conn, "{ action: 'logout', controller: 'admin' }", 0); \n
    espUri(conn, "{ action: 'admin/logout'", 0); \n
    espUri(conn, "{ product: 'candy', quantity: '10', template: '/cart/${product}/${quantity}' }", 0); \n
    espUri(conn, "{ route: '~/STAR/edit', action: 'checkout', id: '99' }", 0); \n
    espUri(conn, "{ template: '~/static/images/${theme}/background.jpg', theme: 'blue' }", 0); 
    @ingroup EspReq
 */
extern cchar *espUri(HttpConn *conn, cchar *target);

/********************************** Controls **********************************/
/**
    There are two groups of  ESP Control APIs. The "full" form and the "abbreviated" form. The "full" form API takes a
    HttpConn request connection object as the first parameter and the function names are prefixed with "esp". 
    The "abbreviated" form APIs are shorter and more convenient. They do not have a connection argument and 
    determine the request connection using Thread-Local storage. They do not have any function prefix. Ocassionally,
    an ESP control name may clash with a function name in another library. If this happens, rename the ESP function
    in the esp-app.h header.
    \n\n
    ESP Controls are grouped into two families: input form controls and general output controls. Input controls
    are typically located inside a form/endform control pair that defines a current database record from which data
    will be utilized. Output controls can be used anywhere on a page outside a form/endform group.
    \n\n
    Input controls are generally of the form: function(field, options) where field is the name of the property
    in the current record that contains the data to display. The options is an object hash that controls and modifies
    how the control will render. The options hash is a JSON string, which is interpreted as a set of property values.
    \n\n
    Various controls have custom options, but most share the following common set of option properties:
    @arg action String Action to invoke. This can be a URI string or a Controller/Action pair of the form
        \@Controller/action. If only the controller is provided (\@Controller/), the "list" action assumed.
    @arg apply String Client JQuery selector identifying the element to apply the remote update.
        Typically "div.ID" where ID is the DOM ID for the element.
    @arg background String Background color. This is a CSS RGB color specification. For example "FF0000" for red.
    @arg click (Boolean|Uri|String) URI to invoke if the control is clicked.
    @arg color String Foreground color. This is a CSS RGB color specification. For example "FF0000" for red.
    @arg confirm String Message to prompt the user to request confirmation before submitting a form or request.
    @arg controller Controller owning the action to invoke when clicked. Defaults to the current controller.
    @arg data-* All data-* names are passed through to the HTML unmodified.
    @arg domid String Client-side DOM-ID to use for the control
    @arg effects String Transition effects to apply when updating a control. Select from: "fadein", "fadeout",
        "highlight".
    @arg escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
        an encoded form.
    @arg height (Number|String) Height of the control. Can be a number of pixels or a percentage string. 
        Defaults to unlimited.
    @arg key Array List of fields to set as the key values to uniquely identify the clicked or edited element. 
        The key will be rendered as a "data-key" HTML attribute and will be passed to the receiving controller 
        when the entry is clicked or edited. Each entry of the key option can be a simple
        string field name or it can be an Object with a single property, where the property name is a simple
        string field name and the property value is the mapped field name to use as the actual key name. This 
        supports using custom key names. NOTE: this option cannot be used if using cell clicks or edits. In that
        case, set click/edit to a callback function and explicitly construct the required URI and parameters.
    @arg keyFormat String Define how the keys will be handled for click and edit URIs. 
        Set to one of the set: ["params", "path", "query"]. Default is "path".
        Set to "query" to add the key/value pairs to the request URI. Each pair is separated using "&" and the
            key and value are formatted as "key=value".
        Set to "params" to add the key/value pair to the request body parameters. 
        Set to "path" to add the key values in order to the request URI. Each value is separated using "/". This
            provides "pretty" URIs that can be easily tokenized by router templates.
        If you require more complex key management, set click or edit to a callback function and format the 
        URI and params manually.
    @arg id Number Numeric database ID for the record that originated the data for the view element.
    @arg method String HTTP method to invoke.
    @arg pass String attributes to pass through unaltered to the client
    @arg params Request parameters to include with a click or remote request
    @arg period Number Period in milliseconds to invoke the $refresh URI to update the control data. If period
        is zero (or undefined), then refresh will be done using a perisistent connection.
    @arg query URI query string to add to click URIs.
    @arg rel String HTML rel attribute. Can be used to generate "rel=nofollow" on links.
    @arg remote (String|URI|Object) Perform the request in the background without changing the browser location.
    @arg refresh (String|URI|Object) URI to invoke in the background to refresh the control's data every $period.
        milliseconds. If period is undefined or zero, a persistent connection may be used to refresh data.
        The refresh option may use the "\@Controller/action" form.
    @arg size (Number|String) Size of the element.
    @arg style String CSS Style to use for the element.
    @arg value Object Override value to display if used without a form control record.
    @arg width (Number|String) Width of the control. Can be a number of pixels or a percentage string. Defaults to
        unlimited.

    <h4>Dynamic Data</h4>
    Most controls can perform background updates of their data after the initial presentation. This is done via
    the refresh and period options.

    @stability Prototype
    @see espAlert
    @defgroup EspControl EspControl
  */
typedef struct EspControl { int dummy; } EspControl;

// MOB - does this do an alert popup or is this a console status widget?
/**
    Display a popup alert message in the client's browser when the web page is displayed.
    @param conn Http connection object
    @param text Alert text to display
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg period -- Polling period in milliseconds for the client to check the server for status message 
    updates. If this is not specifed, the connection to the server will be kept open. This permits the 
    server to "push" alerts to the console, but will consume a connection at the server for each client.
    @ingroup EspControl
    @internal
 */
extern void espAlert(HttpConn *conn, cchar *text, cchar *options);

/**
    Render an HTML anchor link.
    @description. This emits a label inside an anchor reference. i.e. a clickable link.
    @param conn Http connection object
    @param text Anchor text to display for the link
    @param uri URI link for the anchor
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espAnchor(HttpConn *conn, cchar *text, cchar *uri, cchar *options);

/**
    Render an HTML button to use inside a form.
    @description  This creates a button suitable for use inside an input form. When the button is clicked,
        the input form will be submitted.
    @param conn Http connection object
    @param text Button text to display. This text is also used as the name for the form input from this control.
    @param value Form input value to submit when the button is clicked
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espButton(HttpConn *conn, cchar *text, cchar *value, cchar *options);

/**
    Render an HTML button to use outside a form
    @param conn Http connection object
    @param text Button text to display
    @param uri URI to invoke when the button is clicked.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espButtonLink(HttpConn *conn, cchar *text, cchar *uri, cchar *options);

/**
    Render a graphic chart.
    @description The chart control can display static or dynamic tabular data. The client chart control manages
        sorting by column, dynamic data refreshes, pagination, and clicking on rows.
    TODO. This is incomplete.
    @param conn Http connection object
    @param grid Data to display. The data is a grid of data. Use ediCreateGrid or ediReadGrid.
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg columns Object hash of column entries. Each column entry is (in turn) an object hash of options. If unset, 
        all columns are displayed using defaults.
    @arg kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
        motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
        linechart, imagelinechart, imagepiechart, scatterchart (and more).
    @ingroup EspControl
    @internal
 */
extern void espChart(HttpConn *conn, EdiGrid *grid, cchar *options);

/**
    Render an input checkbox. 
    @description This creates a checkbox suitable for use within an input form. 
    @param conn Http connection object
    @param name Name for the input checkbox. This defines the HTML element name, and provides the source of the
        initial value for the checkbox. The field should be a property of the $espForm current record. 
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param checkedValue Value for which the checkbox will be checked.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espCheckbox(HttpConn *conn, cchar *name, cchar *checkedValue, cchar *options);

/**
    Render an HTML division
    @description This creates an HTML element with the required options. It is useful to generate a dynamically 
        refreshing division.
    @param conn Http connection object
    @param body HTML body to render
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espDivision(HttpConn *conn, cchar *body, cchar *options);

/**
    Signify the end of an HTML form. 
    @description This emits a HTML closing form tag.
    @param conn Http connection object
    @ingroup EspControl
 */
extern void espEndform(HttpConn *conn);

/**
    Render flash messages.
    @description Flash messages are one-time messages that are displayed to the client on the next request (only).
    Flash messages use the session state store but persist for only one request.
        See $espSetFlash for how to define flash messages. 
    @param conn Http connection object
    @param kinds Space separated list of flash messages types. Typical types are: "error", "inform", "warning".
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg retain -- Number of seconds to retain the message. If <= 0, the message is retained until another
        message is displayed. Default is 0.
    MOB - this default implies it is displayed for zero seconds
    @ingroup EspControl
 */
extern void espFlash(HttpConn *conn, cchar *kinds, cchar *options);

/**
    Render an HTML form .
    @description This will render an HTML form tag and optionally associate the given record as the current record for
        the request. Abbreviated controls (see $EspAbbrev) use the current record to supply form data fields and values.
        The espForm control can be used without a record. In this case, nested ESP controls may have to provide 
        values via an Options.value field.
    @param conn Http connection object
    @param record Record to use by default to supply form field names and values.
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg hideErrors -- Don't display database record errors. Records retain error diagnostics from the previous
        failed write. Setting this option will prevent the display of such errors.
    @arg modal -- Make the form a modal dialog. This will block all other HTML controls except the form.
    @arg nosecurity -- Don't generate a security token for the form.
    @arg securityToken -- String Override CSRF security token to include when the form is submitted. A default 
        security token will always be generated unless options.nosecurity is defined to be true.
        Security tokens are used by ESP to mitigate cross-site scripting errors.
    @ingroup EspControl
 */
extern void espForm(HttpConn *conn, EdiRec *record, cchar *options);

/**
    Render an HTML icon.
    @param conn Http connection object
    @param uri URI reference for the icon resource
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espIcon(HttpConn *conn, cchar *uri, cchar *options);

/**
    Render an HTML image.
    @param conn Http connection object
    @param uri URI reference for the image resource
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espImage(HttpConn *conn, cchar *uri, cchar *options);

//  MOB DB
//  MOB should be consistent with "name" vs "field"
/**
    Render an input field as part of a form. This is a smart input control that will call the appropriate
        input control based on the database record field data type.
    @param conn Http connection object
    @param field Name for the input field. This defines the HTML element name and provides the source 
        of the initial value to display. The field should be a property of the form current record. 
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espInput(HttpConn *conn, cchar *field, cchar *options);

/**
    Render a text label field. This renders an output-only text field. Use espText() for input fields.
    @param conn Http connection object
    @param text Label text to display.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espLabel(HttpConn *conn, cchar *text, cchar *options);

//  MOB - how to get a choices list from a database
/**
    Render a dropdown selection list.
    @param conn Http connection object
    @param field Record field name to provide the default value for the list. The field should be a property of the 
        form current record.  The field name is used to create the HTML input control name.
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param choices Choices to select from. This is a JSON style set of properties. For example:
        espDropdown(conn, "priority", "{ low: 0, med: 1, high: 2 }", NULL)
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espDropdown(HttpConn *conn, cchar *field, cchar *choices, cchar *options);

/**
    Render a mail link.
    @param conn Http connection object
    @param name Recipient name to display
    @param address Mail recipient address link
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espMail(HttpConn *conn, cchar *name, cchar *address, cchar *options);

/**
    Emit a progress bar.
    @param conn Http connection object
    @param progress Progress percentage (0-100) 
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espProgress(HttpConn *conn, cchar *progress, cchar *options);

/**
    Render a radio button. This creates a radio button suitable for use within an input form. 
    @param conn Http connection object
    @param field Name for the input radio button. This defines the HTML element name and provides the source 
        of the initial value to display. The field should be a property of the form current record. 
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param choices Choices to select from. This is a JSON style set of properties. For example:
        espRadio(conn, "priority", "{ low: 0, med: 1, high: 2, }", NULL)
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espRadio(HttpConn *conn, cchar *field, void *choices, cchar *options);

/**
    Control the refresh of web page dynamic elements.
    @param conn Http connection object
    @param on URI to invoke when turning "on" refresh
    @param off URI to invoke when turning "off" refresh
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg minified -- Set to "true" to select a minified (compressed) version of the script.
    @ingroup EspControl
    @internal
 */
extern void espRefresh(HttpConn *conn, cchar *on, cchar *off, cchar *options);

/**
    Render a script link.
    @param uri Script URI to load. Set to null to get a default set of scripts. See $httpLink for a list of possible
        URI formats.
    @param conn Http connection object
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espScript(HttpConn *conn, cchar *uri, cchar *options);

/**
    Generate a security token.
    @description Security tokens are used to help guard against CSRF threats.
    This call will generate a security token for the page, and emit an HTML meta element for the security token.
    The token will automatically be included whenever forms are submitted and the token is validated by the 
    receiving Controller. Typically, forms will automatically generate the security token. Note that explicitly
    calling this routine is not necessary unless a security token is required for non-form requests such as AJAX
    requests. The $securityToken control should be called inside the &lt;head section of the web page.
    @param conn Http connection object
    @ingroup EspControl
 */
extern void espSecurityToken(HttpConn *conn);

/**
    Render a stylesheet link
    @param uri Stylesheet URI to load. Set to null to get a default set of stylesheets. See $httpLink for a list of possible
        URI formats.
    @param conn Http connection object
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espStylesheet(HttpConn *conn, cchar *uri, cchar *options);

// MOB - review which of these are supported currently
/**
    Render a table.
    @description The table control can display static or dynamic tabular data. The client table control 
        manages sorting by column, dynamic data refreshes, and clicking on rows or cells.
    @param conn Http connection object
    @param grid Data to display. The data is a grid of data. Use ediCreateGrid or ediReadGrid.
    @param options Extra options. See $EspControl for a list of the standard options.
    @param options Optional extra options. See $View for a list of the standard options.
    @arg cell Boolean Set to "true" to make click or edit links apply per cell instead of per row. 
        The default is false.
    @arg columns (Array|Object) The columns list is an object hash of column objects where each column entry is a
        hash of column options.  Column options:
    <ul>
        <li>align - Will right-align numbers by default</li>
        <li>click - URI to invoke if the cell is clicked</li>
        <li>edit - MOB </li>
        <li>formatter - Function to invoke to format the value to display</li>
        <li>header - Header text for the column</li>
        <li>style - Cell styles</li>
        <li>width - Column width. Can be a string percentage or numeric pixel width</li>
    </ul>
    @arg params Object Hash of post parameters to include in the request. This is a hash of key/value items.
    @arg pivot Boolean Pivot the table by swapping rows for columns and vice-versa
    @arg showHeader Boolean Control if column headings are displayed.
    @arg showId Boolean If a column's option is not provided, the id column is normally hidden. 
        To display, set showId to be 'true'.
    @arg sort String Enable row sorting and define the column to sort by. Defaults to the first column.
    @arg sortOrder String Default sort order. Set to "ascending" or "descending". Defaults to ascending.
    @arg style String CSS class to use for the table. The ultimate style to use for a table cell is the 
        combination of style, styleCells, styleColumns, and style Rows.
    @arg styleCells 2D Array of styles to use for the table body cells. Can also provide an array to the 
        column.style property.
    @arg styleColumns Array of styles to use for the table body columns. Can also use the style option in the
        columns option.
    @arg styleRows Array of styles to use for the table body rows
    @arg title String Table title.
    @ingroup EspControl
 */
extern void espTable(HttpConn *conn, EdiGrid *grid, cchar *options);

/**
    Render a tab control. 
    The tab control can manage a set of panes and will selectively show and hide or invoke the selected panes. 
    If the "click" option is defined, the selected pane will be invoked via a foreground click. If the
    "remote" option is defined, the selected pane will be invoked via a background click. If the "toggle" option is
    defined the selected pane will be made visible and other panes will be hidden.
    If using show/hide tabs, define the initial visible pane to be of the class "-ejs-pane-visible" and define
    other panes to be "-ejs-pane-hidden". The control's client side code will toggle these classes to make panes
    visible or hidden.
    @param conn Http connection object
    @param rec Tab data for the control. Tab data is be be a single object where the tab text is the property 
        key and the target to invoke is the property value. 
    @param options Optional extra options. See $View for a list of the standard options.
    @arg click Set to "true" to invoke the selected pane via a foreground click.
    @arg remote Set to "true" to invoke the selected pane via a background click.
    @arg toggle Set to "true" to show the selected pane and hide other panes.
    @ingroup EspControl
 */
extern void espTabs(HttpConn *conn, EdiRec *rec, cchar *options);

/**
    Render a text input field as part of a form.
    @param conn Http connection object
    @param field Name for the input text field. This defines the HTML element name and provides the source 
        of the initial value to display. The field should be a property of the form control record. It can 
        be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
        this call is used without a form control record, the actual data value should be supplied via the 
        options.value property. If the cols or rows option is defined, then a textarea HTML element will be used for
        multiline input.
    @param options Optional extra options. See $View for a list of the standard options.
    @arg cols Number number of text columns
    @arg rows Number number of text rows
    @arg password Boolean The data to display is a password and should be obfuscated.
    @ingroup EspControl
 */
extern void espText(HttpConn *conn, cchar *field, cchar *options);

/**
    Render a tree control. 
    @param conn Http connection object
    @description The tree control can display static or dynamic tree data.
    @param grid Optional initial data for the control. The data option may be used with the refresh option to 
          dynamically refresh the data. The tree data is typically an XML document.
    @param options Optional extra options. See $View for a list of the standard options.
    @ingroup EspControl
    @internal
 */
extern void espTree(HttpConn *conn, EdiGrid *grid, cchar *options);

/***************************** Abbreviated Controls ***************************/
/**
    Abbreviated ESP Controls.
    @description These controls do not take a HttpConn argument and determine the connection object from
        thread-local storage.
    @stability Prototype
    @see espAlert
    @defgroup EspAbbrev EspAbbrev
  */
typedef struct EspAbbrev { int dummy; } EspAbbrev;

/**
    Display a popup alert message in the client's browser when the web page is displayed.
    @param text Alert text to display
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg Polling period in milliseconds for the client to check the server for status message 
    updates. If this is not specifed, the connection to the server will be kept open. This permits the 
    server to "push" alerts to the console, but will consume a connection at the server for each client.
    @ingroup EspAbbrev
    @internal
 */
extern void alert(cchar *text, cchar *options);

/**
    Render an HTML anchor link
    @description. This is emits a label inside an anchor reference. i.e. a clickable link.
    @param text Anchor text to display for the link
    @param uri URI link for the anchor
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void anchor(cchar *text, cchar *uri, cchar *options);

/**
    Render an HTML button to use inside a form.
    @description  This creates a button suitable for use inside an input form. When the button is clicked,
        the input form will be submitted.
    @param text Button text to display. This text is also used as the name for the form input from this control.
    @param value Form input value to submit when the button is clicked
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void button(cchar *text, cchar *value, cchar *options);

/**
    Render an HTML button to use outside a form
    @param text Button text to display
    @param uri URI to invoke when the button is clicked.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void buttonLink(cchar *text, cchar *uri, cchar *options);

/**
    Render a graphic chart
    @description The chart control can display static or dynamic tabular data. The client chart control manages
        sorting by column, dynamic data refreshes, pagination and clicking on rows.
    TODO. This is incomplete.
    @param grid Data to display. The data is a grid of data. Use ediCreateGrid or ediReadGrid.
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
        all columns are displayed using defaults.
    @arg kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
        motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
        linechart, imagelinechart, imagepiechart, scatterchart (and more)
    @ingroup EspAbbrev
    @internal
 */
extern void chart(EdiGrid *grid, cchar *options);

/**
    Render an input checkbox. 
    @description This creates a checkbox suitable for use within an input form. 
    @param field Name for the input checkbox. This defines the HTML element name and provides the source of the
        initial value for the checkbox. The field should be a property of the $espForm current record. 
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param checkedValue Value for which the checkbox will be checked.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void checkbox(cchar *field, cchar *checkedValue, cchar *options);

/**
    Render an HTML division
    @description This creates an HTML element with the required options.It is useful to generate a dynamically 
        refreshing division.
    @param body HTML body to render
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void division(cchar *body, cchar *options);

/**
    Render a dropdown selection list
    @param field Record field name to provide the default value for the list. The field should be a property of the 
        form current record.  The field name is used to create the HTML input control name.
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param choices Choices to select from. This is a JSON style set of properties. For example:
        espDropdown(conn, "priority", "{ low: 0, med: 1, high: 2 }", NULL)
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void dropdown(cchar *field, cchar *choices, cchar *options);

/**
    Signify the end of an HTML form. 
    @description This emits a HTML closing form tag.
    @ingroup EspControl
 */
extern void endform();

/**
    Render flash notices.
    @description Flash notices are one-time messages that are displayed to the client on the next request (only).
        See $espSetFlash for how to define flash messages. 
    @param kinds Space separated list of flash messages types. Typical types are: "error", "inform", "warning".
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg retain -- Number of seconds to retain the message. If <= 0, the message is retained until another
        message is displayed. Default is 0.
    MOB - this default implies it is displayed for zero seconds
    @ingroup EspAbbrev
 */
extern void flash(cchar *kinds, cchar *options);

//  MOB - seems inconsistent that form takes a record?
/**
    Render an HTML form 
    @description This will render an HTML form tag and optionally associate the given record as the current record for
        the request. Abbreviated controls (see $EspAbbrev) use the current record to supply form data fields and values.
        The espForm control can be used without a record. In this case, nested ESP controls may have to provide 
        values via an Options.value field.
    @param record Record to use by default to supply form field names and values.
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg hideErrors -- Don't display database record errors. Records retain error diagnostics from the previous
        failed write. Setting this option will prevent the display of such errors.
    @arg modal -- Make the form a modal dialog. This will block all other HTML controls except the form.
    @arg insecure -- Don't generate a security token for the form.
    @arg securityToken -- String Override CSRF security token to include when the form is submitted. A default 
        security token will always be generated unless options.insecure is defined to be true.
        Security tokens are used by ESP to mitigate cross site scripting errors.
    @ingroup EspAbbrev
 */
extern void form(void *record, cchar *options);

/**
    Render an HTML icon
    @param uri URI reference for the icon resource
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void icon(cchar *uri, cchar *options);

/**
    Render an HTML image
    @param uri URI reference for the image resource
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void image(cchar *uri, cchar *options);

/**
    Render an input field as part of a form. This is a smart input control that will call the appropriate
        input control based on the database record field data type.
    @param field Name for the input field. This defines the HTML element name and provides the source 
        of the initial value to display. The field should be a property of the form current record. 
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void input(cchar *field, cchar *options);

/**
    Render a text label field. This renders an output-only text field. Use espText() for input fields.
    @param text Label text to display.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void label(cchar *text, cchar *options);

/**
    Render a mail link
    @param name Recipient name to display
    @param address Mail recipient address link
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void mail(cchar *name, cchar *address, cchar *options);

/**
    Emit a progress bar.
    @param progress Progress percentage (0-100) 
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void progress(cchar *progress, cchar *options);

/**
    Render a radio button. This creates a radio button suitable for use within an input form. 
    @param field Name for the input radio button. This defines the HTML element name and provides the source 
        of the initial value to display. The field should be a property of the form current record. 
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param choices Choices to select from. This is a JSON style set of properties. For example:
        espRadio(conn, "priority", "{ low: 0, med: 1, high: 2, }", NULL)
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void radio(cchar *field, void *choices, cchar *options);

/**
    Control the refresh of web page dynamic elements
    @param on URI to invoke when turning "on" refresh
    @param off URI to invoke when turning "off" refresh
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg minified Set to "ture" to select a minified (compressed) version of the script.
    @ingroup EspAbbrev
    @internal
 */
extern void refresh(cchar *on, cchar *off, cchar *options);

/**
    Render a script link
    @param uri Script URI to load. Set to null to get a default set of scripts. See $httpLink for a list of possible
        URI formats.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void script(cchar *uri, cchar *options);

/**
    Generate a security token.
    @description Security tokens are used to help guard against CSRF threats.
    This call will generate a security token for the page and emit an HTML meta element for the security token.
    The token will automatically be included whenever forms are submitted and the token be validated by the 
    receiving Controller. Forms will normally automatically generate the security token and that explicitly
    calling this routine is not required unless a security token is required for non-form requests such as AJAX
    requests. The $securityToken control should be called inside the &lt;head section of the web page.
    @ingroup EspAbbrev
 */
extern void securityToken();

/**
    Render a stylesheet link
    @param uri Stylesheet URI to load. Set to null to get a default set of stylesheets. See $httpLink for a list of possible
        URI formats.
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspAbbrev
 */
extern void stylesheet(cchar *uri, cchar *options);

/**
    Render a table.
    @description The table control can display static or dynamic tabular data. The client table control 
        manages sorting by column, dynamic data refreshes and clicking on rows or cells.
    @param grid Data to display. The data is a grid of data. Use ediCreateGrid or ediReadGrid.
    @param options Extra options. See $EspControl for a list of the standard options.
    @param options Optional extra options. See $View for a list of the standard options.
    @arg cell Boolean Set to "true" to make click or edit links apply per cell instead of per row. 
        The default is false.
    @arg columns (Array|Object) The columns list is anobject hash of column objects where each column entry is 
        hash of column options.  Column options:
    <ul>
        <li>align - Will right-align numbers by default</li>
        <li>click - URI to invoke if the cell is clicked</li>
        <li>edit - MOB </li>
        <li>formatter - Function to invoke to format the value to display</li>
        <li>header - Header text for the column</li>
        <li>style - Cell styles</li>
        <li>width - Column width. Can be a string percentage or numeric pixel width</li>
    </ul>
    @arg params Object Hash of post parameters to include in the request. This is a hash of key/value items.
    @arg pivot Boolean Pivot the table by swaping rows for columns and vice-versa
    @arg showHeader Boolean Control if column headings are displayed.
    @arg showId Boolean If a columns option is not provided, the id column is normally hidden. 
        To display, set showId to be "true".
    @arg sort String Enable row sorting and define the column to sort by. Defaults to the first column.
    @arg sortOrder String Default sort order. Set to "ascending" or "descending".Defaults to ascending.
    @arg style String CSS class to use for the table. The ultimate style to use for a table cell is the 
        combination of style, styleCells, styleColumns and style Rows.
    @arg styleCells 2D Array of styles to use for the table body cells. Can also provide an array to the 
        column.style property.
    @arg styleColumns Array of styles to use for the table body columns. Can also use the style option in the
        columns option.
    @arg styleRows Array of styles to use for the table body rows
    @arg title String Table title.
    @ingroup EspAbbrev
*/
extern void table(EdiGrid *grid, cchar *options);

//  MOB - need examples in each of these abbreviated controls -- see ejs.web
/**
    Render a tab control. 
    The tab control can manage a set of panes and will selectively show and hide or invoke the selected panes. 
    If the "click" option is defined, the selected pane will be invoked via a foreground click. If the
    "remote" option is defined, the selected pane will be invoked via a background click. If the "toggle" option is
    defined the selected pane will be made visible and other panes will be hidden.
    If using show/hide tabs, define the initial visible pane to be of the class "-ejs-pane-visible" and define
    other panes to be "-ejs-pane-hidden". The control's client side code will toggle these classes to make panes
    visible or hidden.
    @param rec Tab data for the control. Tab data is a single object where the tab text is the property 
        key and the target to invoke is the property value. 
    @param options Optional extra options. See $View for a list of the standard options.
    @arg click Set to "true" to invoke the selected pane via a foreground click.
    @arg remote Set to "true" to invoke the selected pane via a background click.
    @arg toggle Set to "true" to show the selected pane and hide other panes.
    @ingroup EspAbbrev
 */
extern void tabs(EdiRec *rec, cchar *options);

/**
    Render a text input field as part of a form.
    @param field Name for the input text field. This defines the HTML element name and provides the source 
        of the initial value to display. The field should be a property of the form control record. It can 
        be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
        this call is used without a form control record, the actual data value should be supplied via the 
        options.value property. If the cols or rows option is defined, then a textarea HTML element will be used for
        multiline input.
    @param options Optional extra options. See $View for a list of the standard options.
    @arg cols Number number of text columns
    @arg rows Number number of text rows
    @arg password Boolean The data to display is a password and should be obfuscated.
    @ingroup EspAbbrev
 */
extern void text(cchar *field, cchar *options);

/**
    Render a tree control. 
    @description The tree control can display static or dynamic tree data.
    @param grid Optional initial data for the control. The data option may be used with the refresh option to 
          dynamically refresh the data. The tree data is typically an XML document.
    @param options Optional extra options. See $View for a list of the standard options.
    @ingroup EspAbbrev
    @internal
 */
extern void tree(EdiGrid *grid, cchar *options);

/******************************* Abbreviated API ******************************/

/**
    Add a header to the transmission using a format string.
    @description Add a header if it does not already exist.
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup EspAbbrev
 */
extern void addHeader(cchar *key, cchar *fmt, ...);

/**
    Create a record and initialize field values 
    @description This will call $ediCreateRec to create a record based on the given table's schema. It will then
        call $ediSetFields to update the record with the given data.
    @param tableName Database table name
    @param data Hash of field values
    @return EdRec instance
    @ingroup EspAbbrev
 */
extern EdiRec *createRec(cchar *tableName, MprHash *data);

/**
    Create a session state object. 
    @description The session state object can be used to share state between requests.
    If a session has not already been created, this call will create a new session.
    It will create a response cookie containing a session ID that will be sent to the client 
    with the response. Note: Objects are stored in the session state using JSON serialization.
    This routine calls $espCreateSession.
    @ingroup EspAbbrev
 */
extern void createSession();

/**
    Destroy a session state object
    @description
    @ingroup EspAbbrev
 */
extern void destroySession();

/**
    Don't auto-finalize this request
    @ingroup EspAbbrev
 */
extern void dontAutoFinalize();

#if UNUSED
/**
    Set an error flash notification message.
    @description Flash messages persist for only one request and are a convenient way to pass state information or 
    feedback messages to the next request. 
    @param fmt Printf style message format
    @ingroup EspAbbrev
 */
extern void error(cchar *fmt, ...);
#endif

/**
    Finalize the response.
    @description Signals the end of any and all response data and flushes any buffered write data to the client. 
    If the request has already been finalized, this call has no additional effect.
    This routine calls $espFinalize.
    @ingroup EspAbbrev
 */
extern void finalize();

/**
    Flush transmit data. 
    @description This writes any buffered data.
    @ingroup EspAbbrev
 */
extern void flush();

/**
    Get a list of column names.
    @param rec Database record. If set to NULL, the current database record defined via $form() is used.
    @return An MprList of column names in the given table. If there is no record defined, an empty list is returned.
    @ingroup EdiAbbrev
 */
extern MprList *getColumns(EdiRec *rec);

/**
    Get the request cookies
    @description Get the cookies defined in the current request.
    @return Return a string containing the cookies sent in the Http header of the last request.
    @ingroup EspAbbrev
 */
extern cchar *getCookies();

/**
    Get the connection object
    @description Before a view or controller is run, the current connection object for the request is saved in thread
    local data. Most EspControl APIs take an HttpConn object as an argument.
    @return HttpConn connection instance object.
    @ingroup EspAbbrev
 */
extern HttpConn *getConn();

/**
    Get the receive body content length
    @description Get the length of the receive body content (if any). This is used in servers to get the length of posted
        data and in clients to get the response body length.
    @return A count of the response content data in bytes.
    @ingroup EspAbbrev
 */
extern MprOff getContentLength();

/**
    Get the receive body content type
    @description Get the content mime type of the receive body content (if any).
    @return Mime type of any receive content. Set to NULL if not posted data.
    @ingroup EspAbbrev
 */
extern cchar *getContentType();

/**
    Get the current database instance
    @description A route may have a default database configured via the EspDb Appweb.conf configuration directive. 
    The database will be opened when the web server initializes and will be shared between all requests using the route. 
    @return Edi EDI database handle
    @ingroup EspAbbrev
 */
extern Edi *getDatabase();

/**
    Get the extended route EspRoute structure
    @return EspRoute instance
    @ingroup EspAbbrev
 */
extern EspRoute *getEspRoute();

/**
    Get the default document root directory for the request route.
    @return A directory path name 
    @ingroup EspAbbrev
 */
extern cchar *getDir();

/**
    Get a field from the current database record
    @description The current grid is defined via $setRec
    @param field Field name to return
    @return String value for "field" in the current record.
    @ingroup EdiAbbrev
 */
extern cchar *getField(cchar *field);

/**
    Get the current database grid
    @description The current grid is defined via $setGrid
    @return EdiGrid instance
    @ingroup EdiAbbrev
    @internal
 */
extern EdiGrid *getGrid();

/**
    Get an rx http header.
    @description Get a http response header for a given header key.
    @param key Name of the header to retrieve. This should be a lower case header name. For example: "Connection"
    @return Value associated with the header key or null if the key did not exist in the response.
    @ingroup EspAbbrev
 */
extern cchar *getHeader(cchar *key);

/**
    Get the HTTP method
    @description This is a convenience API to return the Http method 
    @return The HttpConn.rx.method property
    @ingroup EspReq
 */
extern cchar *getMethod();

/**
    Get the HTTP URI query string
    @description This is a convenience API to return the query string for the current request.
    @return The espGetConn()->rx->parsedUri->query property
    @ingroup EspAbbrev
 */
extern cchar *getQuery();

/**
    Get the current database record
    @description The current record is defined via $setRec
    @return EdiRec instance
    @ingroup EdiAbbrev
 */
extern EdiRec *getRec();

/**
    Get the referring URI
    @description This returns the referring URI as described in the HTTP "referer" (yes the HTTP specification does
        spell it incorrectly) header. If this header is not defined, this routine will return the home URI as returned
        by $home.
    @return String URI back to the referring URI. If no referrer is defined, refers to the home URI.
    @ingroup EspAbbrev
 */
extern cchar *getReferrer();

/**
    Get a session state variable
    @description See also $espGetSessionVar and $espGetSessionObj for alternate ways to retrieve session data.
    @param name Variable name to get
    @return The session variable value. Returns NULL if not set.
    @ingroup EspAbbrev
 */
extern cchar *getSessionVar(cchar *name);

/**
    Get a relative URI to the top of the application.
    @description This will return an absolute URI for the top of the application. This will be "/" if there is no
        application script name. Otherwise, it will return a URI for the script name for the application.
        Alternatively, this can be constructed via uri("~")
    @return String Absolute URI to the top of the application
    @ingroup EspAbbrev
 */
cchar *getTop();

/**
    Get the uploaded files
    @description Get the hash table defining the uploaded files.
        This hash is indexed by the file identifier supplied in the upload form. The hash entries are HttpUploadFile
        objects.
    @return A hash of HttpUploadFile objects.
    @ingroup EspAbbrev
 */
extern MprHash *getUploads();

/**
    Get the request URI string
    @description This is a convenience API to return the request URI.
    @return The espGetConn()->rx->uri
    @ingroup EspAbbrev
 */
extern cchar *getUri();

/**
    Test if a current grid has been defined
    @description The current grid is defined via $setRec
    @return "true" if a current grid has been defined
    @ingroup EdiAbbrev
 */
extern bool hasGrid();

/**
    Test if a current record has been defined and save to the database
    @description This call returns "true" if a current record is defined and has been saved to the database with a 
        valid "id" field.
    @return "true" if a current record with a valid "id" is defined.
    @ingroup EdiAbbrev
 */
extern bool hasRec();

/**
    Test if the receive input stream is at end-of-file
    @return "true" if there is no more receive data to read
    @ingroup EspAbbrev
 */
extern bool isEof();

/**
    Test if a http request is finalized.
    @description This tests if #espFinalize or #httpFinalize has been called for a request.
    @return "true" if the request has been finalized.
    @ingroup EspAbbrev
 */
extern bool isFinalized();

/**
    Set an informational flash notification message.
    @description Flash messages persist for only one request and are a convenient way to pass state information or 
    feedback messages to the next request. 
    This routine calls $espInform.
    @param fmt Printf style message format
    @ingroup EspAbbrev
 */
extern void inform(cchar *fmt, ...);

/**
    Test if the connection is using SSL and is secure
    @return "true" if the connection is using SSL.
    @ingroup EspAbbrev
 */
extern bool isSecure();

/**
    Make a grid
    @description This call makes a free-standing data grid based on the JSON format content string.
        The record is not saved to the database.
    @param content JSON format content string. The content should be an array of objects where each object is a
        set of property names and values.
    @return An EdiGrid instance
    @example:
grid = ediMakeGrid("[ \\ \n
    { id: '1', country: 'Australia' }, \ \n
    { id: '2', country: 'China' }, \ \n
    ]");
    @ingroup EspAbbrev
 */
extern EdiGrid *makeGrid(cchar *content);

/**
    Make a hash table container of property values
    @description This routine formats the given arguments, parses the result as a JSON string and returns an 
        equivalent hash of property values. The result after formatting should be of the form:
        hash("{ key: 'value', key2: 'value', key3: 'value' }");
    @param fmt Printf style format string
    @param ... arguments
    @return MprHash instance
    @ingroup EspAbbrev
 */
extern MprHash *makeHash(cchar *fmt, ...);

/**
    Make a record
    @description This call makes a free-standing data record based on the JSON format content string.
        The record is not saved to the database.
    @param content JSON format content string. The content should be a set of property names and values.
    @return An EdiRec instance
    @example: rec = ediMakeRec("{ id: 1, title: 'Message One', body: 'Line one' }");
    @ingroup EspAbbrev
 */
extern EdiRec *makeRec(cchar *content);

/**
    Set an error flash notification message.
    @description Flash messages persist for only one request and are a convenient way to pass state information or 
    feedback messages to the next request. 
    @param fmt Printf style message format
    @ingroup EspAbbrev
 */
extern void notice(cchar *fmt, ...);

/**
    Get a request parameter
    @description Get the value of a named request parameter. Form variables are defined via www-urlencoded query or post
        data contained in the request.
        This routine calls $espGetParam
    @param name Name of the request parameter to retrieve
    @return String containing the request parameter's value. Caller should not free.
    @ingroup EspAbbrev
 */
extern cchar *param(cchar *name);

/**
    Get the request parameter hash table
    @description This call gets the params hash table for the current request.
        Route tokens, request query data, and www-url encoded form data are all entered into the params table after decoding.
        Use #mprLookupKey to retrieve data from the table.
        This routine calls $espGetParams
    @return #MprHash instance containing the request parameters
    @ingroup EspAbbrev
 */
extern MprHash *params();

/**
    Read the identified record 
    @description Read the record identified by the request params("id") from the nominated table.
    @param tableName Database table name
    @return The identified record. Returns NULL if the table or record cannot be found.
    @ingroup EdiAbbrev
 */
extern EdiRec *readRec(cchar *tableName);

/**
    Read matching records
    @description This runs a simple query on the database and returns matching records in a grid. The query selects
        all rows that have a "field" that matches the given "value".
    @param tableName Database table name
    @param fieldName Database field name to evaluate
    @param operation Comparison operation. Set to "==", "!=", "<", ">", "<=" or ">=".
    @param value Data value to compare with the field values.
    @return A grid containing all matching records. Returns NULL if no matching records.
    @ingroup EdiAbbrev
 */
extern EdiGrid *readRecsWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);

/**
    Read one record
    @description This runs a simple query on the database and selects the first matching record. The query selects
        a row that has a "field" that matches the given "value".
    @param tableName Database table name
    @param fieldName Database field name to evaluate
    @param operation Comparison operation. Set to "==", "!=", "<", ">", "<=" or ">=".
    @param value Data value to compare with the field values.
    @return First matching record. Returns NULL if no matching records.
    @ingroup EspAbbrev
 */
extern EdiRec *readRecWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);

/**
    Read a record identified by key value
    @description Read a record from the given table as identified by the key value.
    @param tableName Database table name
    @param key Key value of the record to read 
    @return Record instance of EdiRec.
    @ingroup EspAbbrev
 */
extern EdiRec *readRecByKey(cchar *tableName, cchar *key);

/**
    Read all the records in table from the database
    @description This reads a table and returns a grid containing the table data.
    @param tableName Database table name
    @return A grid containing all table rows. Returns NULL if the table cannot be found.
    @ingroup EdiAbbrev
 */
extern EdiGrid *readTable(cchar *tableName);

/**
    Read receive body content
    @description Read body content from the client
    @param buf Buffer to accept content data
    @param size Size of the buffer
    @return A count of bytes read into the buffer
    @ingroup EspAbbrev
 */
extern ssize receive(char *buf, ssize size);

/**
    Redirect the client
    @description Redirect the client to a new uri. This will redirect with an HTTP 302 status. If a different HTTP status
    code is required, use $espRedirect.
    @param target New target uri for the client
    @ingroup EspAbbrev
 */
extern void redirect(cchar *target);

/**
    Redirect the client back to the referrer
    @description Redirect the client to the referring URI.
    @ingroup EspAbbrev
 */
extern void redirectBack();

/**
    Remove a record from a database table
    @description Remove the record identified by the key value from the given table.
    @param tableName Database table name
    @param key Key value of the record to remove 
    @return Record instance of EdiRec.
    @ingroup EdiAbbrev
 */
extern bool removeRec(cchar *tableName, cchar *key);

/**
    Render a formatted string
    @description Render a formatted string of data into packets to the client. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @return A count of the bytes actually written
    @ingroup EspAbbrev
 */
extern ssize render(cchar *fmt, ...);

/**
    Render cached content
    @description Render the saved, cached response from a prior request to this URI. This is useful if the caching
        mode has been set to "manual".
    @return A count of the bytes actually written
    @ingroup EspAbbrev
 */
ssize renderCached();

/**
    Render an error message back to the client and finalize the request. The output is Html escaped for security.
    @param status Http status code
    @param fmt Printf style message format
    @return A count of the bytes actually written
    @ingroup EspAbbrev
 */
extern void renderError(int status, cchar *fmt, ...);

/**
    Render a file back to the client
    @description Render a formatted string of data and then HTML escape. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param path Filename of the file to send to the client.
    @param ... Arguments for fmt
    @return A count of the bytes actually written
    @ingroup EspAbbrev
 */
extern ssize renderFile(cchar *path);

/**
    Render a formatted string after HTML escaping
    @description Render a formatted string of data and then HTML escape. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @return A count of the bytes actually written
    @ingroup EspAbbrev
 */
extern ssize renderSafe(cchar *fmt, ...);

/**
    Render a view template to the client
    @description Actions are C procedures that are invoked when specific URIs are routed to the controller/action pair.
    @param view view name
    @ingroup EspAbbrev
 */
extern void renderView(cchar *view);

/** 
    Define a cookie header to send with the response. The Path, Domain, and Expires properties can be set to null for 
    default values.
    @param name Cookie name
    @param value Cookie value
    @param path Uri path to which the cookie applies
    @param domain String Domain in which the cookie applies. Must have 2-3 "." and begin with a leading ".". 
        For example: domain: .example.com
        Some browsers will accept cookies without the initial ".", but the spec: (RFC 2109) requires it.
    @param lifespan Lifespan of the cookie. (MOB units?)
    @param isSecure Boolean Set to "true" if the cookie only applies for SSL based connections
    @ingroup EspAbbrev
*/
extern void setCookie(cchar *name, cchar *value, cchar *path, cchar *domain, MprTime lifespan, bool isSecure);

/**
    Set the current request connection.
    @param conn The HttpConn connection object to define
    @ingroup EspAbbrev
 */
extern void setConn(HttpConn *conn);

/**
    Set the transmission (response) content mime type
    @description Set the mime type Http header in the transmission
    @param mimeType Mime type string
    @ingroup EspAbbrev
 */
extern void setContentType(cchar *mimeType);

/**
    Update a record field without writing to the database
    @description This routine updates the record object with the given value. The record will not be written
        to the database. To write to the database, use $writeRec.
    @param rec Record to update
    @param fieldName Record field name to update
    @param value Value to update
    @return The record instance if successful, otherwise NULL.
    @ingroup EspAbbrev
 */
extern EdiRec *setField(EdiRec *rec, cchar *fieldName, cchar *value);

/**
    Update record fields without writing to the database
    @description This routine updates the record object with the given values. The "data' argument supplies 
        a hash of fieldNames and values. The data hash may come from the request $params() or it can be manually
        created via #ediMakeHash to convert a JSON string into an options hash.
        For example: updateFields(rec, hash("{ name: '%s', address: '%s' }", name, address))
        The record will not be written
        to the database. To write to the database, use $ediWriteRec.
    @param rec Record to update
    @param data Hash of field names and values to use for the update
    @return The record instance if successful, otherwise NULL.
    @ingroup EspAbbrev
 */
extern EdiRec *setFields(EdiRec *rec, MprHash *data);

/**
    Set a flash notification message.
    @description Flash messages persist for only one request and are a convenient way to pass state information or 
    feedback messages to the next request. 
    Flash messages use the session state store, but persist only for one request.
    The $inform, $error and $warn convenience methods invoke $notice.
    This routine calls $espSetFlash.
    @param kind Kind of flash message.
    @param fmt Printf style message format
    @ingroup EspAbbrev
 */
extern void setFlash(cchar *kind, cchar *fmt, ...);

/**
    Set the current database grid
    @return The grid instance. This permits chaining.
    @ingroup EdiAbbrev
    @internal
 */
extern EdiGrid *setGrid(EdiGrid *grid);

/**
    Set a transmission header
    @description Set a Http header to send with the request. If the header already exists, its value is overwritten.
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup EspAbbrev
 */
extern void setHeader(cchar *key, cchar *fmt, ...);

/**
    Set a request parameter value
    @description Set the value of a named request parameter to a string value. Form variables are defined via
        www-urlencoded query or post data contained in the request.
    @param name Name of the request parameter to set
    @param value Value to set.
    @ingroup EspAbbrev
 */
extern void setParam(cchar *name, cchar *value);

/**
    Set the current database record
    @description The current record is used to supply data to various abbreviated controls, such as: text(), input(), 
        checkbox and dropdown()
    @return The grid instance. This permits chaining.
    @ingroup EdiAbbrev
    @internal
 */
extern EdiRec *setRec(EdiRec *rec);

/**
    Set a session state variable
    @description See also $espSetSessionVar and $espSetSessionObj for alternate ways to set session data.
    @param name Variable name to set
    @param value Value to set
    @ingroup EspAbbrev
 */
extern void setSessionVar(cchar *name, cchar *value);

/**
    Set a Http response status.
    @description Set the Http response status for the request. This defaults to 200 (OK).
    @param status Http status code.
    @ingroup EspAbbrev
 */
extern void setStatus(int status);

/**
    Create a timeout event 
    @description invoke the given procedure after the timeout
    @param proc Function to invoke
    @param timeout Time in milliseconds to elapse before invoking the timeout
    @param data Argument to pass to proc
    @ingroup EspAbbrev
 */
void setTimeout(void *proc, MprTime timeout, void *data);

/**
    Show request details
    @description This echoes request details back to the client. This is useful as a debugging tool.
    @ingroup EspAbbrev
 */
extern void showRequest();

/**
    Update the cached content for a request
    @description Save the given content for future requests. This is useful if the caching mode has been set to "manual". 
    @param uri Request URI to cache for
    @param data Data to cache
    @param lifesecs Time in seconds to cache the data
    @ingroup EspAbbrev
 */
extern void updateCache(cchar *uri, cchar *data, int lifesecs);

/**
    Write a value to a database table field
    @description Update the value of a table field in the selected table row. Note: validations are not run.
    @param tableName Database table name
    @param key Key value for the table row to update.
    @param fieldName Column name to update
    @param value Value to write to the database field
    @return "true" if the field  can be successfully written.
    @ingroup EspAbbrev
 */
extern bool updateField(cchar *tableName, cchar *key, cchar *fieldName, cchar *value);

/**
    Write field values to a database row
    @description This routine updates the current record with the given data and then saves the record to
        the database. The "data' argument supplies 
        a hash of fieldNames and values. The data hash may come from the request $params() or it can be manually
        created via #ediMakeHash to convert a JSON string into an options hash.
        For example: ediWriteFields(rec, params());
        The record runs field validations before saving to the database.
    @param tableName Database table name
    @param data Hash of field names and values to use for the update
    @return "true" if the field  can be successfully written. Returns false if field validations fail.
    @ingroup EspAbbrev
 */
extern bool updateFields(cchar *tableName, MprHash *data);

/**
    Write a record to the database
    @description The record will be saved to the database after running any field validations. If any field validations
        fail to pass, the record will not be written and error details can be retrieved via $ediGetRecErrors.
        If the record is a new record and the "id" column is EDI_AUTO_INC, then the "id" will be assigned
        prior to saving the record.
    @param rec Record to write to the database.
    @return "true" if the record can be successfully written.
    @ingroup EspAbbrev
 */
extern bool updateRec(EdiRec *rec);

//  MOB - should httpLink be renamed to httpUri and espLink to httpUri
/**
    Create a URI. 
    @description Create a URI link by expansions tokens based on the current request and route state.
    The target parameter may contain partial or complete URI information. The missing parts 
    are supplied using the current request and route tables. The resulting URI is a normalized, server-local 
    URI (that begins with "/"). The URI will include any defined route prefix, but will not include scheme, host or 
    port components.
    @param target The URI target. The target parameter can be a URI string or JSON style set of options. 
        The target will have any embedded "{tokens}" expanded by using token values from the request parameters.
        If the target has an absolute URI path, that path is used directly after tokenization. If the target begins with
        "~", that character will be replaced with the route prefix. This is a very convenient way to create application 
        top-level relative links.
        \n\n
        If the target is a string that begins with "{AT}" it will be interpreted as a controller/action pair of the 
        form "{AT}Controller/action". If the "controller/" portion is absent, the current controller is used. If 
        the action component is missing, the "list" action is used. A bare "{AT}" refers to the "list" action 
        of the current controller.
        \n\n
        If the target starts with "{" it is interpreted as being a JSON style set of options that describe the link.
        If the target is a relative URI path, it is appended to the current request URI path.  
        \n\n
        If the target is a JSON style of options, it can specify the URI components: scheme, host, port, path, reference and
        query. If these component properties are supplied, these will be combined to create a URI.
        \n\n
        If the target specifies either a controller/action or a JSON set of options, The URI will be created according 
        to the route URI template. The template may be explicitly specified
        via a "route" target property. Otherwise, if an "action" property is specified, the route of the same
        name will be used. If these don't result in a usable route, the "default" route will be used. 
        \n\n
        These are the properties supported in a JSON style "{ ... }" target:
        <ul>
            <li>scheme String URI scheme portion</li>
            <li>host String URI host portion</li>
            <li>port Number URI port number</li>
            <li>path String URI path portion</li>
            <li>reference String URI path reference. Does not include "#"</li>
            <li>query String URI query parameters. Does not include "?"</li>
            <li>controller String Controller name if using a Controller-based route. This can also be specified via
                the action option.</li>
            <li>action String Action to invoke. This can be a URI string or a Controller action of the form
                {AT}Controller/action.</li>
            <li>route String Route name to use for the URI template</li>
        </ul>
    @return A normalized, server-local Uri string.
    @example uri("http://example.com/index.html", 0); \n
    uri("/path/to/index.html", 0); \n
    uri("../images/splash.png", 0); \n
    uri("~/static/images/splash.png", 0); \n
    uri("${app}/static/images/splash.png", 0); \n
    uri("@controller/checkout", 0); \n
    uri("@controller/") \n
    uri("@init") \n
    uri("@") \n
    uri("{ action: '@post/create' }", 0); \n
    uri("{ action: 'checkout' }", 0); \n
    uri("{ action: 'logout', controller: 'admin' }", 0); \n
    uri("{ action: 'admin/logout'", 0); \n
    uri("{ product: 'candy', quantity: '10', template: '/cart/${product}/${quantity}' }", 0); \n
    uri("{ route: '~/STAR/edit', action: 'checkout', id: '99' }", 0); \n
    uri("{ template: '~/static/images/${theme}/background.jpg', theme: 'blue' }", 0); 
    @ingroup EspAbbrev
 */
extern cchar *uri(cchar *target);

#if UNUSED
/**
    Set a flash notification message.
    @description Flash messages persist for only one request and are a convenient way to pass state information or 
        feedback messages to the next request. 
    This routine calls $espWarn.
    @param fmt Printf style message format
    @ingroup EspAbbrev
 */
extern void warn(cchar *fmt, ...);
#endif

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* BIT_FEATURE_ESP */
#endif /* _h_ESP */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

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

