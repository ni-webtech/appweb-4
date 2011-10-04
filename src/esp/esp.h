/*
    esp.h -- Embedded Server Pages (ESP) Module handler.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_ESP
#define _h_ESP 1

/********************************* Includes ***********************************/

#include    "appweb.h"

#if BLD_FEATURE_ESP

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
#define ESP_UNLOAD_TIMEOUT  (30 * 1000)                 /**< Timeout for ESP requests to complete when reloading modules */
#define ESP_LIFESPAN        (3600 * MPR_TICKS_PER_SEC)  /**< Default generated content cache lifespan */

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
    #define ESP_CORE_LIBS "${LIB}\\mod_esp${SHLIB} ${LIB}\\libappweb.lib ${LIB}\\libhttp.lib ${LIB}\\libmpr.lib"
#else
    #define ESP_CORE_LIBS "${LIB}/mod_esp${SHOBJ} -lappweb -lpcre -lhttp -lmpr -lpthread -lm"
#endif
#if BLD_FEATURE_SSL
    #if WIN
        #define ESP_SSL_LIBS " ${LIB}\\libmprssl.lib"
    #else
        #define ESP_SSL_LIBS " -lmprssl"
    #endif
#else
    #define ESP_SSL_LIBS
#endif
#define ESP_LIBS ESP_CORE_LIBS ESP_SSL_LIBS

/********************************** Defines ***********************************/
/**
    Procedure callback
    @ingroup Esp
 */
typedef void (*EspProc)(HttpConn *conn);

#define CONTENT_MARKER  "${_ESP_CONTENT_MARKER_}"       /* Layout content marker */
#define ESP_NAME        "espHandler"                    /* Name of the ESP handler */
#define ESP_SESSION     "-esp-session-"                 /* ESP session cookie name */

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

#define ESP_SECURITY_TOKEN_NAME "__esp_security_token__"
#define ESP_FLASH_VAR           "__flash__"

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
    MprCache        *cache;                 /**< Session and content cache */
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
    MprList         *env;                   /**< Environment for compiler */
    char            *compile;               /**< Compile template */
    char            *link;                  /**< Link template */
    char            *searchPath;            /**< Search path to use when locating compiler / linker */
    EspProc         controllerBase;         /**< Initialize base for a controller */

    char            *appModuleName;         /**< App module name when compiled flat */
    char            *appModulePath;         /**< App module path when compiled flat */

    char            *dir;                   /**< Base directory (alias for route->dir) */
    char            *cacheDir;              /**< Directory for cached compiled controllers and views */
    char            *controllersDir;        /**< Directory for controllers */
    char            *databasesDir;          /**< Directory for databases */
    char            *layoutsDir;            /**< Directory for layouts */
    char            *modelsDir;             /**< Directory for models */
    char            *staticDir;             /**< Directory for static web content */
    char            *viewsDir;              /**< Directory for views */

    MprTime         lifespan;               /**< Default cache lifespan */
    int             update;                 /**< Auto-update modified ESP source */
    int             keepSource;             /**< Preserve generated source */
	int				showErrors;				/**< Send server errors back to client */

    Edi             *edi;                   /**< Default database for this route */
} EspRoute;

/**
    Control the caching of content for a given targetKey
    @description This routine the response data content caching.
    @param eroute EspRoute object
    @param targetKey The HttpRoute target key to cache.
    @param lifesecs Lifespan of cache items in seconds.
    @param uri Optional cache URI when using per-URI caching. Set to "*" to cache all matching URIs on a per-URI basis.
    @return A count of the bytes actually written
    @ingroup EspRoute
 */
extern void espCacheControl(EspRoute *eroute, cchar *targetKey, int lifesecs, cchar *uri);

/**
    Compile a view or controller
    @description This compiles ESP controllers and views into loadable, cached modules
    @param conn Http connection object
    @param source ESP source file name
    @param module Output module file name
    @param cacheName MD5 cache name. Not a full path
    @param isView Set to true if the source is a view
    @return True if the compilation is successful. Errors are logged and sent back to the client if EspRoute.showErrors
        is true.
    @ingroup EspRoute
 */
extern bool espCompile(HttpConn *conn, cchar *source, cchar *module, cchar *cacheName, int isView);

/**
    Convert an ESP web page into compilable C code
    @description This parses an ESP web page into a equivalent C source view.
    @param eroute EspRoute object
    @param page ESP web page script.
    @param path Pathname for the ESP web page. This is used to process include directives which are resolved relative
        to this path.
    @param cacheName MD5 cache name. Not a full path.
    @param layout Default layout page.
    @param err Output parameter to hold any relevant error message.
    @return C language code string for the web page
    @ingroup EspRoute
 */
extern char *espBuildScript(EspRoute *eroute, cchar *page, cchar *path, cchar *cacheName, cchar *layout, char **err);

/**
    Define an action
    @description Actions are C procedures that are invoked when specific URIs are routed to the controller/action pair.
    @param eroute ESP route object
    @param targetKey Target key used to select the action in a HttpRoute target. This is typically a URI prefix.
    @param actionProc EspProc callback procedure to invoke when the action is requested.
    @ingroup EspRoute
 */
extern void espDefineAction(EspRoute *eroute, cchar *targetKey, void *actionProc);

/**
    Define a base function to invoke for all controller actions.
    @description A base function can be defined that will be called before calling any controller action. This
        emulates a super class constructor.
    @param eroute ESP route object
    @param baseProc Function to call just prior to invoking a controller action.
    @ingroup EspRoute
 */
extern void espDefineBase(EspRoute *eroute, EspProc baseProc);

/**
    Define a view
    @description Views are ESP web pages that are executed to return presentation data back to the client.
    @param eroute ESP route object
    @param path Path to the ESP view source code.
    @param viewProc EspViewPrococ callback procedure to invoke when the view is requested.
    @ingroup EspRoute
 */
extern void espDefineView(EspRoute *eroute, cchar *path, void *viewProc);

/**
    Expand a compile or link command template
    @description This expands a command template and replaces "${tokens}" with their equivalent value. The supported
        tokens are:
        <ul>
            <li>ARCH - Build architecture (i386, x86_64)</li>
            <li>CC - Compiler pathame</li>
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
    @param command Http connection object
    @param source ESP web page soure pathname
    @param module Output module pathname
    @return An expanded command line
    @ingroup EspRoute
 */
extern char *espExpandCommand(cchar *command, cchar *source, cchar *module);

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
    Allocate a new session state object
    @description
    @param conn Http connection object
    @param id Unique session state ID
    @param lifespan Session lifespan in ticks
    @return A session state object
    @ingroup EspSession
 */
extern EspSession *espAllocSession(HttpConn *conn, cchar *id, MprTime lifespan);

/**
    Create a session object
    @description This call creates a session object if one does not already exist.
        Session state stores persist across individual HTTP requests.
    @param conn Http connection object
    @return A session state object
    @ingroup EspSession
 */
extern EspSession *espCreateSession(HttpConn *conn);

/**
    Destroy a session state object
    @description
    @param sp Session state object allocated with #espAllocSession
    @ingroup EspSession
 */
extern void espDestroySession(EspSession *sp);

/**
    Get a session state object
    @description
    @param conn Http connection object
    @param create Set to true to create a session state object if one does not already exist for this client
    @return A session state object
    @ingroup EspSession
 */
extern EspSession *espGetSession(HttpConn *conn, int create);

/**
    Get a session state variable
    @description
    @param conn Http connection object
    @param name Variable name to get
    @param defaultValue If the variable does not exist, return the defaultValue.
    @return The variable value or defaultValue if it does not exist.
    @ingroup EspSession
 */
extern cchar *espGetSessionVar(HttpConn *conn, cchar *name, cchar *defaultValue);

/**
    Set a session variable
    @description
    @param conn Http connection object
    @param name Variable name to set
    @param value Variable value to use
    @return A session state object
    @ingroup EspSession
 */
extern int espSetSessionVar(HttpConn *conn, cchar *name, cchar *value);

/**
    Get the session ID
    @description
    @param conn Http connection object
    @return The session ID string
    @ingroup EspSession
 */
extern char *espGetSessionID(HttpConn *conn);

/**
    Set an object into the session state store
    @description Store an object into the session state store by serialing all properties.
    @param conn Http connection object
    @param key Session state key
    @param value Object to serialize
    @ingroup EspSession
 */
extern int espSetSessionObj(HttpConn *conn, cchar *key, MprHash *value);

/**
    Get an object from the session state store
    @description Retrieve an object from the session state store by deserializing all properties.
    @param conn Http connection object
    @param key Session state key
    @ingroup EspSession
 */
extern MprHash *espGetSessionObj(HttpConn *conn, cchar *key);

/********************************** Requests **********************************/
/**
    View procedure callback
    @param conn Http connection object
    @ingroup EspReq
 */
typedef void (*EspViewProc)(HttpConn *conn);

/**
    ESP Action
    @description Actions are run after a request URI is routed to a controller.
    @ingroup EspReq
    @see
 */
typedef struct EspAction {
    EspProc         actionProc;             /**< Action procedure to run to respond to the request */
    MprTime         lifespan;               /**< Lifespan for cached action content */
    char            *cacheUri;              /**< Per-URI caching details */
} EspAction;

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
    MprBuf          *cacheBuffer;           /**< HTML output caching */
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
    int             autoFinalize;           /**< Request is/will-be auto-finalized */
    int             finalized;              /**< Request has been finalized */
    int             sessionProbed;          /**< Already probed for session store */
    int             appLoaded;              /**< App module already probed */
    int             lastDomID;              /**< Last generated DOM ID */
    EdiRec          *record;                /**< Current data record */
} EspReq;

/**
    Add a header to the transmission using a format string.
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup EspReq
 */
extern void espAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/**
    Add a header to the transmission
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value Value to set for the header
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup EspReq
 */
extern void espAddHeaderString(HttpConn *conn, cchar *key, cchar *value);

/**
    Append a transmission header
    @description Set the header if it does not already exists. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup EspReq
 */
extern void espAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/**
    Append a transmission header string
    @description Set the header if it does not already exists. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value Value to set for the header
    @ingroup EspReq
 */
extern void espAppendHeaderString(HttpConn *conn, cchar *key, cchar *value);

/**
    Auto finalize transmission of the http request
    @description If auto-finalization is enabled via #espSetAutoFinalizing, this call will finalize writing Http response
    data by writing the final chunk trailer if required. If using chunked transfers, a null chunk trailer is required
    to signify the end of write data.  If the request is already finalized, this call does nothing.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espAutoFinalize(HttpConn *conn);

/**
    Get the receive body content length
    @description Get the length of the receive body content (if any). This is used in servers to get the length of posted
        data and in clients to get the response body length.
    @param conn HttpConn connection object created via $httpCreateConn
    @return A count of the response content data in bytes.
    @ingroup EspReq
 */
extern MprOff espGetContentLength(HttpConn *conn);

/**
    Get the request cookies
    @description Get the cookies defined in the current requeset
    @param conn HttpConn connection object created via $httpCreateConn
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup EspReq
 */
extern cchar *espGetCookies(HttpConn *conn);

/**
    Get the request parameter hash table
    @description This call gets the params hash table for the current request.
        Query data and www-url encoded form data is entered into the params table after decoding.
        Use #mprLookupKey to retrieve data from the table.
    @param conn HttpConn connection object
    @return #MprHash instance containing the request parameters
    @ingroup EspReq
 */
extern MprHash *espGetParams(HttpConn *conn);

/**
    Get an rx http header.
    @description Get a http response header for a given header key.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Name of the header to retrieve. This should be a lower case header name. For example: "Connection"
    @return Value associated with the header key or null if the key did not exist in the response.
    @ingroup EspReq
 */
extern cchar *espGetHeader(HttpConn *conn, cchar *key);

/**
    Get the hash table of rx Http headers
    @description Get the internal hash table of rx headers
    @param conn HttpConn connection object created via $httpCreateConn
    @return Hash table. See MprHash for how to access the hash table.
    @ingroup EspReq
 */
extern MprHash *espGetHeaderHash(HttpConn *conn);

/**
    Get all the requeset http headers.
    @description Get all the rx headers. The returned string formats all the headers in the form:
        key: value\\nkey2: value2\\n...
    @param conn HttpConn connection object created via $httpCreateConn
    @return String containing all the headers. The caller must free this returned string.
    @ingroup EspReq
 */
extern char *espGetHeaders(HttpConn *conn);

/**
    Get a relative URI to the home ("/") of the application.
    @param conn HttpConn connection object created via $httpCreateConn
    @return String Relative URI to the pathInfo of "/"
    @ingroup EspReq
 */
char *espGetHome(HttpConn *conn);

/**
    Get a request pararmeter as an integer
    @description Get the value of a named request parameter as an integer. Form variables are define via
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return Integer containing the request parameter's value
    @ingroup EspReq
 */
extern int espGetIntParam(HttpConn *conn, cchar *var, int defaultValue);

/**
    Get the request query string
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup EspReq
 */
extern cchar *espGetQueryString(HttpConn *conn);

/**
    Get the referring URI
    @param conn HttpConn connection object created via $httpCreateConn
    @return String URI back to the referring URI. If no referrer is defined, refers to the home URI.
    @ingroup EspReq
 */
char *espGetReferrer(HttpConn *conn);

/**
    Get a unique security token
    @description Security tokens help mitigate against replay attacks. The security token is stored in HttpRx.securityToken
        and in the session store.
    @param conn HttpConn connection object
    @return The security token string
 */
extern cchar *espGetSecurityToken(HttpConn *conn);

/**
    Get the response status
    @param conn HttpConn connection object created via $httpCreateConn
    @return An integer Http response code. Typically 200 is success.
    @ingroup EspReq
 */
extern int espGetStatus(HttpConn *conn);

/**
    Get the Http response status message. 
    @description The HTTP status message is supplied on the first line of the HTTP response.
    @param conn HttpConn connection object created via $httpCreateConn
    @returns A Http status message.
    @ingroup EspReq
 */
extern char *espGetStatusMessage(HttpConn *conn);

/**
    Get a request parameter
    @description Get the value of a named request parameter. Form variables are define via www-urlencoded query or post
        data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return String containing the request parameter's value. Caller should not free.
    @ingroup EspReq
 */
extern cchar *espGetParam(HttpConn *conn, cchar *var, cchar *defaultValue);

/**
    Finalize transmission of the http request
    @description Finalize writing HTTP data by writing the final chunk trailer if required. If using chunked transfers,
    a null chunk trailer is required to signify the end of write data.
    If the request is already finalized, this call does nothing.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espFinalize(HttpConn *conn);

/**
    Test if a http request is finalized.
    @description This tests if #espFinalize or #httpFinalize has been called for a request.
    @param conn HttpConn connection object
    @return True if the request has been finalized.
    @ingroup EspReq
 */
extern bool espFinalized(HttpConn *conn);

/**
    Flush transmit data. 
    @description This writes any buffered data.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup EspReq
 */
extern void espFlush(HttpConn *conn);

/**
    Match a request parameter with an expected value
    @description Compare a request parameter and return true if it exists and its value matches.
    @param conn HttpConn connection object
    @param var Name of the request parameter
    @param value Expected value to match
    @return True if the value matches
    @ingroup EspReq
 */
extern bool espMatchParam(HttpConn *conn, cchar *var, cchar *value);

/**
    Redirect the client
    @description Redirect the client to a new uri.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code to send with the response
    @param target New target uri for the client
    @ingroup EspReq
 */
extern void espRedirect(HttpConn *conn, int status, cchar *target);

/**
    Redirect the client back to the referrer
    @description Redirect the client to the referring URI.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup EspReq
 */
extern void espRedirectBack(HttpConn *conn);

/**
    Remove a header from the transmission
    @description Remove a header if present.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EspReq
 */
extern int espRemoveHeader(HttpConn *conn, cchar *key);

/**
    Enable auto-finalizing for this request
    @description Remove a header if present.
    @param conn HttpConn connection object created via $httpCreateConn
    @param on Set to true to enable auto-finalizing.
    @return True if auto-finalizing was enabled prior to this call
    @ingroup EspReq
 */
extern bool espSetAutoFinalizing(HttpConn *conn, bool on);

/**
    Define a content length header in the transmission. 
    @description This will define a "Content-Length: NNN" request header.
    @param conn HttpConn connection object created via $httpCreateConn
    @param length Numeric value for the content length header.
    @ingroup EspReq
 */
extern void espSetContentLength(HttpConn *conn, MprOff length);

/**
    Set a cookie in the transmission
    @description Define a cookie to send in the transmission Http header
    @param conn HttpConn connection object created via $httpCreateConn
    @param name Cookie name
    @param value Cookie value
    @param path URI path to which the cookie applies
    @param cookieDomain Domain in which the cookie applies. Must have 2-3 dots.
    @param lifespan Duration for the cookie to persist in msec
    @param isSecure Set to true if the cookie only applies for SSL based connections
    @ingroup EspReq
 */
extern void espSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, MprTime lifespan,
        bool isSecure);

/**
    Set the transmission (response) content mime type
    @description Set the mime type Http header in the transmission
    @param conn HttpConn connection object created via $httpCreateConn
    @param mimeType Mime type string
    @ingroup EspReq
 */
extern void espSetContentType(HttpConn *conn, cchar *mimeType);

/**
    Set a transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup EspReq
 */
extern void espSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/**
    Set a simple key/value transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value String value for the key
    @ingroup EspReq
 */
extern void espSetHeaderString(HttpConn *conn, cchar *key, cchar *value);

/**
    Set an integer request parameter value
    @description Set the value of a named request parameter to an integer value. Form variables are define via
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to set
    @param value value to set.
    @ingroup EspReq
 */
extern void espSetIntParam(HttpConn *conn, cchar *var, int value);

/**
    Set a Http response status.
    @description Set the Http response status for the request. This defaults to 200 (OK).
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code.
    @ingroup EspReq
 */
extern void espSetStatus(HttpConn *conn, int status);

/**
    Set a request parameter value
    @description Set the value of a named request parameter to a string value. Form variables are define via
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request parameter to set
    @param value Value to set.
    @ingroup EspReq
 */
extern void espSetParam(HttpConn *conn, cchar *var, cchar *value);

/**
    Show request details
    @description This echos request details back to the client. This is useful as a debugging tool.
    @param conn HttpConn connection object
    @ingroup EspReq
 */
extern void espShowRequest(HttpConn *conn);

/**
    Write a formatted string
    @description Write a formatted string of data into packets to the client. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param conn HttpConn connection object
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @return A count of the bytes actually written
    @ingroup EspReq
 */
//  MOB - should all these be render?
extern ssize espWrite(HttpConn *conn, cchar *fmt, ...);

//  MOB - can this return short?
/**
    Write a block of data to the client
    @description Write a block of data to the client. Data packets will be created as required to store the write data.
    @param conn HttpConn connection object
    @param buf Buffer containing the write data
    @param size of the data in buf
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espWriteBlock(HttpConn *conn, cchar *buf, ssize size);

/**
    Write a string of data to the client
    @description Write a string of data to the client. Data packets will be created
        as required to store the write data. This call may block waiting for data to drain to the client.
    @param conn HttpConn connection object
    @param s String containing the data to write
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espWriteString(HttpConn *conn, cchar *s);

/**
    Write a safe string of data to the client
    @description HTML escape a string and then write the string of data to the client.
        Data packets will be created as required to store the write data. This call may block waiting for the data to
        the client to drain.
    @param conn HttpConn connection object
    @param s String containing the data to write
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espWriteSafeString(HttpConn *conn, cchar *s);

/**
    Write the value of a request parameter to the client
    @description This writes the value of request parameter after HTML escaping its value.
    @param conn HttpConn connection object
    @param name Form variable name
    @return A count of the bytes actually written
    @ingroup EspReq
 */
extern ssize espWriteParam(HttpConn *conn, cchar *name);

/**
    Write a view template to the client
    @description Actions are C procedures that are invoked when specific URIs are routed to the controller/action pair.
    @param conn Http connection object
    @param name view name
    @ingroup EspReq
 */
extern void espWriteView(HttpConn *conn, cchar *name);

/**
    Get the current request connection.
    @return The HttpConn connection object
    @ingroup EspReq
 */
extern HttpConn *espGetConn();

/**
    Send an "inform" flash message
    @description Flash messages are passed to the next request (only) for display. Use the flash() control to
        display.
    @param conn Http connection object
    @param fmt Printf style formatted string to use as the message
    @ingroup EspReq
 */
void espInform(HttpConn *conn, cchar *fmt, ...);

/**
    Send an "error" flash message
    @description Flash messages are passed to the next request (only) for display. Use the flash() control to
        display.
    @param conn Http connection object
    @param fmt Printf style formatted string to use as the message
    @ingroup EspReq
 */
void espError(HttpConn *conn, cchar *fmt, ...);

/**
    Send an "warn" flash message
    @description Flash messages are passed to the next request (only) for display. Use the flash() control to
        display.
    @param conn Http connection object
    @param fmt Printf style formatted string to use as the message
    @ingroup EspReq
 */
void espWarn(HttpConn *conn, cchar *fmt, ...);

/**
    Send a flash message
    @param conn Http connection object
    @param kind Kind of flash message
    @param fmt Printf style formatted string to use as the message
    @ingroup EspReq
 */
void espNotice(HttpConn *conn, cchar *kind, cchar *fmt, ...);

/**
    Send a flash message
    @param conn Http connection object
    @param kind Kind of flash message
    @param fmt Printf style formatted string to use as the message
    @param args Varargs style list
    @ingroup EspReq
    @internal
 */
void espNoticev(HttpConn *conn, cchar *kind, cchar *fmt, va_list args);

/**
    Set the current request connection.
    @param conn The HttpConn connection object to define
    @ingroup EspReq
 */
extern void espSetConn(HttpConn *conn);

/********************************** Controls **********************************/
/**
    ESP Controls
    MOB - Overview needed
    MOB - Talk about abbreviated forms
    @stability Prototype
    @see espAlert
    @defgroup EspControl EspControl
  */
typedef struct EspControl { int dummy; } EspControl;

/**
    Display a popup alert message in the clients browser when the web page is displayed.
    @param conn Http connection object
    @param text Alert text to display
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg period Polling period in milliseconds for the client to check the server for status message 
    updates. If this is not specifed, the connection to the server will be kept open. This permits the 
    server to "push" alerts to the console, but will consume a connection at the server for each client.
    @ingroup EspControl
    @internal
    MOB - does this do an alert popup or is this a console status widget?
 */
extern void espAlert(HttpConn *conn, cchar *text, cchar *options);

/**
    Render an HTML anchor link
    @description. This is emits a label inside an anchor reference. i.e. a clickable link.
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
    Render an graphic chart
    @description The chart control can display static or dynamic tabular data. The client chart control manages
        sorting by column, dynamic data refreshes, pagination and clicking on rows.
    TODO. This is incomplete.
    @param conn Http connection object
    @param grid Data to display. The data is a grid of data. Use ediCreateGrid or ediReadGrid.
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
        all columns are displayed using defaults.
    @arg kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
        motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
        linechart, imagelinechart, imagepiechart, scatterchart (and more)
    @ingroup EspControl
    @internal
 */
extern void espChart(HttpConn *conn, EdiGrid *grid, cchar *options);

//  MOB DB
/**
    Render an input checkbox. 
    @description This creates a checkbox suitable for use within an input form. 
    @param conn Http connection object
    @param name Name for the input checkbox. This defines the HTML element name and provides the source of the
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
    @description This creates an HTML element with the required options.It is useful to generate a dynamically 
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
        See $espNotice for how to display flash messages. 
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
    Render an HTML form 
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
        Security tokens are used by ESP to mitigate cross site scripting errors.
    @ingroup EspControl
 */
extern void espForm(HttpConn *conn, EdiRec *record, cchar *options);

/**
    Render an HTML icon
    @param conn Http connection object
    @param uri URI reference for the icon resource
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espIcon(HttpConn *conn, cchar *uri, cchar *options);

/**
    Render an HTML image
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

//  MOB DB
//  MOB - how to get a choices list from a database
/**
    Render a dropdown selection list
    @param conn Http connection object
    @param field Record field name to provide the default value for the list. The field should be a property of the 
        form current record.  The field name is used to create the HTML input control name.
        If this call is used without a form control record, the actual data value should be supplied via the 
        options.value property.
    @param choices Choices to select from. This is a JSON style set of properties. For example:
        espDropList(conn, "priority", "{ low: 0, med: 1, high: 2 }", NULL)
    @param options Extra options. See $EspControl for a list of the standard options.
    @ingroup EspControl
 */
extern void espDropList(HttpConn *conn, cchar *field, cchar *choices, cchar *options);

/**
    Render a mail link
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

//  MOB DB
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
    Control the refresh of web page dynamic elements
    @param conn Http connection object
    @param on URI to invoke when turning "on" refresh
    @param off URI to invoke when turning "off" refresh
    @param options Extra options. See $EspControl for a list of the standard options.
    @arg minified Set to true to select a minified (compressed) version of the script.
    @ingroup EspControl
    @internal
 */
extern void espRefresh(HttpConn *conn, cchar *on, cchar *off, cchar *options);

/**
    Render a script link
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
    This call will generate a security token for the page and emit an HTML meta element for the security token.
    The token will automatically be included whenever forms are submitted and the token be validated by the 
    receiving Controller. Forms will normally automatically generate the security token and that explicitly
    calling this routine is not required unless a security token is required for non-form requests such as AJAX
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
        manages sorting by column, dynamic data refreshes and clicking on rows or cells.
    @param conn Http connection object
    @param grid Data to display. The data is a grid of data. Use ediCreateGrid or ediReadGrid.
    @param options Extra options. See $EspControl for a list of the standard options.
    @param options Optional extra options. See $View for a list of the standard options.
    @arg cell Boolean Set to true to make click or edit links apply per cell instead of per row. 
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
        To display, set showId to be true.
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
    @param grid Tab data for the control. Tab data can be be a single object where the tab text is the property 
        key and the target to invoke is the property value. It can also be an an array of objects, one per tab. 
    @param options Optional extra options. See $View for a list of the standard options.
    @arg click Set to true to invoke the selected pane via a foreground click.
    @arg remote Set to true to invoke the selected pane via a background click.
    @arg toggle Set to true to show the selected pane and hide other panes.
    @ingroup EspControl
 */
extern void espTabs(HttpConn *conn, EdiGrid *grid, cchar *options);

//MOB DB
/**
    Render a text input field as part of a form.
    @param name Name for the input text field. This defines the HTML element name and provides the source 
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
    @description The tree control can display static or dynamic tree data.
    @param grid Optional initial data for the control. The data option may be used with the refresh option to 
          dynamically refresh the data. The tree data is typically an XML document.
    @param options Optional extra options. See $View for a list of the standard options.
    @ingroup EspControl
    @internal
 */
extern void espTree(HttpConn *conn, EdiGrid *grid, cchar *options);

/*
    Abbreviated forms. Conn comes from thread local data
 */
extern void alert(cchar *text, cchar *options);
extern void anchor(cchar *text, cchar *uri, cchar *options);
extern void button(cchar *name, cchar *value, cchar *options);
extern void buttonLink(cchar *text, cchar *uri, cchar *options);
extern void chart(EdiGrid *grid, cchar *options);
extern void checkbox(cchar *field, cchar *checkedValue, cchar *options);
extern void division(cchar *body, cchar *options);
extern void endform();
extern void flash(cchar *kind, cchar *options);

//  MOB - seems inconsistent that form takes a record?
extern void form(void *rec, cchar *options);

extern void icon(cchar *uri, cchar *options);
extern void image(cchar *uri, cchar *options);
extern void input(cchar *field, cchar *options);
extern void label(cchar *text, cchar *options);
extern void droplist(cchar *field, cchar *choices, cchar *options);
extern void mail(HttpConn *conn, cchar *name, cchar *address, cchar *options);
extern void progress(cchar *data, cchar *options);
extern void radio(cchar *field, void *choices, cchar *options);
extern void refresh(cchar *on, cchar *off, cchar *options);
extern void script(cchar *uri, cchar *options);
extern void securityToken();
extern void stylesheet(cchar *uri, cchar *options);
extern void table(EdiGrid *grid, cchar *options);
extern void tabs(EdiGrid *grid, cchar *options);
extern void text(cchar *field, cchar *options);
extern void tree(EdiGrid *grid, cchar *options);

/*
    Misc
 */
extern EspSession *createSession();
extern void error(cchar *fmt, ...);
extern void finalize();
extern HttpConn *getConn();
extern Edi *getDatabase();
extern cchar *getMethod();
extern cchar *getQuery();
extern cchar *getUri();
extern void inform(cchar *fmt, ...);
extern void notice(cchar *kind, cchar *fmt, ...);
extern MprHash *params();
extern cchar *param(cchar *key);
extern void redirect(cchar *target);
extern ssize render(cchar *msg, ...);
extern void renderView(cchar *view);
extern void setParam(cchar *key, cchar *value);
extern cchar *session(cchar *key);
extern void setSession(cchar *key, cchar *value);
extern void warn(cchar *fmt, ...);

//  TODO MOB
extern cchar *referrer();

/*
    Database
 */
extern EdiRec *createRec(cchar *tableName, MprHash *params);
extern MprList *getColumns(EdiRec *rec);
extern EdiGrid *getGrid();
extern EdiRec *getRec();
extern bool hasGrid();
extern bool hasRec();
extern MprHash *makeObj(cchar *json, ...);
extern EdiGrid *readGrid(cchar *tableName);
extern EdiRec *readRec(cchar *tableName);
extern EdiGrid *readWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);
extern EdiRec *readOneWhere(cchar *tableName, cchar *fieldName, cchar *operation, cchar *value);
extern EdiRec *readRecByKey(cchar *tableName, cchar *key);
extern bool removeRec(cchar *tableName, cchar *key);
extern EdiGrid *setGrid(EdiGrid *grid);
extern EdiRec *setRec(EdiRec *rec);

/* These write to the database */
extern bool writeRec(EdiRec *rec);
extern bool writeField(cchar *tableName, cchar *key, cchar *fieldName, cchar *value);
extern bool writeFields(cchar *tableName, MprHash *params);

/* 
    NO-DB API
    These work locally on a record - no saving to database
 */
extern EdiRec *makeRec(cchar *json);
extern EdiGrid *makeGrid(cchar *json);
extern EdiRec *updateField(EdiRec *rec, cchar *fieldName, cchar *value);
extern EdiRec *updateFields(EdiRec *rec, MprHash *params);

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

