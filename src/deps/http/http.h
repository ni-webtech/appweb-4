/*
    http.h -- Header for the Embedthis Http Library.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_HTTP
#define _h_HTTP 1

/********************************* Includes ***********************************/

#include    "mpr.h"

/****************************** Forward Declarations **************************/

#ifdef __cplusplus
extern "C" {
#endif

#if !DOXYGEN
struct Http;
struct HttpAuth;
struct HttpConn;
struct HttpRoute;
struct HttpHost;
struct HttpPacket;
struct HttpLimits;
struct HttpQueue;
struct HttpRx;
struct HttpEndpoint;
struct HttpStage;
struct HttpEndpoint;
struct HttpTx;
struct HttpUri;
#endif

/********************************** Tunables **********************************/

#define HTTP_DEFAULT_PORT 80

#ifndef HTTP_NAME
#define HTTP_NAME "Embedthis-http/" BIT_VERSION             /**< Default library name used in Http headers */
#endif

#if BIT_TUNE == MPR_TUNE_SIZE || DOXYGEN
    /*  
        Tune for size
     */
    #define HTTP_BUFSIZE               (8 * 1024)           /**< Default I/O buffer size */
    #define HTTP_MAX_CACHE_ITEM        (256 * 1024)         /**< Maximum cachable item size */
    #define HTTP_MAX_CHUNK             (8 * 1024)           /**< Maximum chunk size for transfer chunk encoding */
    #define HTTP_MAX_HEADERS           4096                 /**< Maximum size of the headers */
    #define HTTP_MAX_IOVEC             16                   /**< Number of fragments in a single socket write */
    #define HTTP_MAX_NUM_HEADERS       20                   /**< Maximum number of header lines */
    #define HTTP_MAX_RECEIVE_FORM      (1024 * 1024)        /**< Maximum incoming form size */
    #define HTTP_MAX_RECEIVE_BODY      (128 * 1024 * 1024)  /**< Maximum incoming body size */
    #define HTTP_MAX_REQUESTS          20                   /**< Maximum concurrent requests */
    #define HTTP_MAX_CLIENTS           10                   /**< Maximum concurrent client endpoints */
    #define HTTP_MAX_SESSIONS          100                  /**< Maximum concurrent sessions */
    #define HTTP_MAX_STAGE_BUFFER      (32 * 1024)          /**< Maximum buffer for any stage */
    #define HTTP_CLIENTS_HASH          (131)                /**< Hash table for client IP addresses */
    #define HTTP_MAX_ROUTE_MATCHES     32                   /**< Maximum number of submatches in routes */

#elif BIT_TUNE == MPR_TUNE_BALANCED
    /*  
        Tune balancing speed and size
     */
    #define HTTP_BUFSIZE               (16 * 1024)
    #define HTTP_MAX_CACHE_ITEM        (512 * 1024)
    #define HTTP_MAX_CHUNK             (8 * 1024)
    #define HTTP_MAX_HEADERS           (8 * 1024)
    #define HTTP_MAX_IOVEC             24
    #define HTTP_MAX_NUM_HEADERS       40
    #define HTTP_MAX_RECEIVE_FORM      (8 * 1024 * 1024)
    #define HTTP_MAX_RECEIVE_BODY      (128 * 1024 * 1024)
    #define HTTP_MAX_REQUESTS          50
    #define HTTP_MAX_CLIENTS           25
    #define HTTP_MAX_SESSIONS          500
    #define HTTP_MAX_STAGE_BUFFER      (64 * 1024)
    #define HTTP_CLIENTS_HASH          (257)
    #define HTTP_MAX_ROUTE_MATCHES     64

#else
    /*  
        Tune for speed
     */
    #define HTTP_BUFSIZE               (32 * 1024)
    #define HTTP_MAX_CACHE_ITEM        (1024 * 1024)
    #define HTTP_MAX_CHUNK             (16 * 1024) 
    #define HTTP_MAX_HEADERS           (8 * 1024)
    #define HTTP_MAX_IOVEC             32
    #define HTTP_MAX_NUM_HEADERS       256
    #define HTTP_MAX_RECEIVE_FORM      (16 * 1024 * 1024)
    #define HTTP_MAX_RECEIVE_BODY      (256 * 1024 * 1024)
    #define HTTP_MAX_REQUESTS          1000
    #define HTTP_MAX_CLIENTS           500
    #define HTTP_MAX_SESSIONS          5000
    #define HTTP_MAX_STAGE_BUFFER      (128 * 1024)
    #define HTTP_CLIENTS_HASH          (1009)
    #define HTTP_MAX_ROUTE_MATCHES     128
#endif

#define HTTP_MAX_TX_BODY           (INT_MAX)        /**< Maximum buffer for response data */
#define HTTP_MAX_UPLOAD            (INT_MAX)

/*  
    Other constants
 */
#define HTTP_DEFAULT_MAX_THREADS  10                /**< Default number of threads */
#define HTTP_MAX_KEEP_ALIVE       100               /**< Maximum requests per connection */
#define HTTP_MAX_PASS             64                /**< Size of password */
#define HTTP_MAX_SECRET           32                /**< Size of secret data for auth */
#define HTTP_PACKET_ALIGN(x)      (((x) + 0x3FF) & ~0x3FF)
#define HTTP_RANGE_BUFSIZE        128               /**< Size of a range boundary */
#define HTTP_RETRIES              3                 /**< Default number of retries for client requests */
#define HTTP_TIMER_PERIOD         1000              /**< Timer checks ever 1 second */
#define HTTP_MAX_REWRITE          20                /**< Maximum URI rewrites */

#define HTTP_INACTIVITY_TIMEOUT   (60  * 1000)      /**< Keep connection alive timeout */
#define HTTP_SESSION_TIMEOUT      (3600 * 1000)     /**< One hour */
#define HTTP_CACHE_LIFESPAN       (86400 * 1000)    /**< Default cache lifespan to 1 day */

#define HTTP_DATE_FORMAT          "%a, %d %b %Y %T GMT"
#define HTTP_LOG_FORMAT           "%h %l %u %t \"%r\" %>s %b"

/*  
    Hash sizes (primes work best)
 */
#define HTTP_SMALL_HASH_SIZE      31                /* Small hash (less than the alphabet) */
#define HTTP_MED_HASH_SIZE        61                /* Medium */
#define HTTP_LARGE_HASH_SIZE      101               /* Large */

/********************************** Defines ***********************************/
/*
    Standard HTTP/1.1 status codes
 */
#define HTTP_CODE_CONTINUE                  100     /**< Continue with request, only partial content transmitted */
#define HTTP_CODE_OK                        200     /**< The request completed successfully */
#define HTTP_CODE_CREATED                   201     /**< The request has completed and a new resource was created */
#define HTTP_CODE_ACCEPTED                  202     /**< The request has been accepted and processing is continuing */
#define HTTP_CODE_NOT_AUTHORITATIVE         203     /**< The request has completed but content may be from another source */
#define HTTP_CODE_NO_CONTENT                204     /**< The request has completed and there is no response to send */
#define HTTP_CODE_RESET                     205     /**< The request has completed with no content. Client must reset view */
#define HTTP_CODE_PARTIAL                   206     /**< The request has completed and is returning partial content */
#define HTTP_CODE_MOVED_PERMANENTLY         301     /**< The requested URI has moved permanently to a new location */
#define HTTP_CODE_MOVED_TEMPORARILY         302     /**< The URI has moved temporarily to a new location */
#define HTTP_CODE_SEE_OTHER                 303     /**< The requested URI can be found at another URI location */
#define HTTP_CODE_NOT_MODIFIED              304     /**< The requested resource has changed since the last request */
#define HTTP_CODE_USE_PROXY                 305     /**< The requested resource must be accessed via the location proxy */
#define HTTP_CODE_TEMPORARY_REDIRECT        307     /**< The request should be repeated at another URI location */
#define HTTP_CODE_BAD_REQUEST               400     /**< The request is malformed */
#define HTTP_CODE_UNAUTHORIZED              401     /**< Authentication for the request has failed */
#define HTTP_CODE_PAYMENT_REQUIRED          402     /**< Reserved for future use */
#define HTTP_CODE_FORBIDDEN                 403     /**< The request was legal, but the server refuses to process */
#define HTTP_CODE_NOT_FOUND                 404     /**< The requested resource was not found */
#define HTTP_CODE_BAD_METHOD                405     /**< The request HTTP method was not supported by the resource */
#define HTTP_CODE_NOT_ACCEPTABLE            406     /**< The requested resource cannot generate the required content */
#define HTTP_CODE_REQUEST_TIMEOUT           408     /**< The server timed out waiting for the request to complete */
#define HTTP_CODE_CONFLICT                  409     /**< The request had a conflict in the request headers and URI */
#define HTTP_CODE_GONE                      410     /**< The requested resource is no longer available*/
#define HTTP_CODE_LENGTH_REQUIRED           411     /**< The request did not specify a required content length*/
#define HTTP_CODE_PRECOND_FAILED            412     /**< The server cannot satisfy one of the request preconditions */
#define HTTP_CODE_REQUEST_TOO_LARGE         413     /**< The request is too large for the server to process */
#define HTTP_CODE_REQUEST_URL_TOO_LARGE     414     /**< The request URI is too long for the server to process */
#define HTTP_CODE_UNSUPPORTED_MEDIA_TYPE    415     /**< The request media type is not supported by the server or resource */
#define HTTP_CODE_RANGE_NOT_SATISFIABLE     416     /**< The request content range does not exist for the resource */
#define HTTP_CODE_EXPECTATION_FAILED        417     /**< The server cannot satisfy the Expect header requirements */
#define HTTP_CODE_NO_RESPONSE               444     /**< The connection was closed with no response to the client */
#define HTTP_CODE_INTERNAL_SERVER_ERROR     500     /**< Server processing or configuration error. No response generated */
#define HTTP_CODE_NOT_IMPLEMENTED           501     /**< The server does not recognize the request or method */
#define HTTP_CODE_BAD_GATEWAY               502     /**< The server cannot act as a gateway for the given request */
#define HTTP_CODE_SERVICE_UNAVAILABLE       503     /**< The server is currently unavailable or overloaded */
#define HTTP_CODE_GATEWAY_TIMEOUT           504     /**< The server gateway timed out waiting for the upstream server */
#define HTTP_CODE_BAD_VERSION               505     /**< The server does not support the HTTP protocol version */
#define HTTP_CODE_INSUFFICIENT_STORAGE      507     /**< The server has insufficient storage to complete the request */

/*
    Proprietary HTTP status codes
 */
#define HTTP_CODE_START_LOCAL_ERRORS        550
#define HTTP_CODE_COMMS_ERROR               550     /**< The server had a communicationss error responding to the client */

/*
    Flags that can be ored into the status code
 */
#define HTTP_CODE_MASK                      0xFFFF
#define HTTP_ABORT                          0x10000 /* Abort the request and connection */
#define HTTP_CLOSE                          0x20000 /* Close the conn at the completion of the request */

/**
    Lookup password callback
    @param auth HttpAuth object
    @param realm Http authentication realm
    @param user User name for whom to retrieve the password
    @return The user password 
    @ingroup HttpAuth
 */
typedef cchar *(*HttpGetPassword)(struct HttpAuth *auth, cchar *realm, cchar *user);

/**
    Validate user credentials callback
    @param auth HttpAuth object
    @param realm Http authentication realm
    @param user User name for whom to retrieve the password
    @param pass User password
    @param required Authentication "required" field. Set to "user", "group" or "valid".
    @param msg Output parameter to contain any appropriate error message
    @return True if the user credentials can be validated and accepted
    @ingroup HttpAuth
 */
typedef bool (*HttpValidateCred)(struct HttpAuth *auth, cchar *realm, char *user, cchar *pass, cchar *required, char **msg);

/**
    Connection Http state change notification callback
    @param conn HttpConn connection object created via $httpCreateConn
    @param state Http state
    @param flags Additional http state information
    @ingroup HttpConn
 */
typedef void (*HttpNotifier)(struct HttpConn *conn, int state, int flags);

/** 
    Define an callback for IO events on this connection.
    @description The event callback will be invoked in response to I/O events.
    @param conn HttpConn connection object created via $httpCreateConn
    @param fn Callback function. 
    @param arg Data argument to provide to the callback function.
    @return The redirected URI string to use.
    @ingroup HttpConn
 */
typedef cchar *(*HttpRedirectCallback)(struct HttpConn *conn, int *code, struct HttpUri *uri);

/**
    Set environment vars callback. Invoked per request to permit custom form var definition
    @ingroup HttpConn
 */
typedef void (*HttpEnvCallback)(struct HttpConn *conn);

/**
    Listen callback. Invoked after listening on a socket endpoint
    @return "Zero" if the listening endpoint can be opened for service. Otherwise, return a negative MPR error code.
    @ingroup HttpConn
 */
typedef int (*HttpListenCallback)(struct HttpEndpoint *endpoint);

/**
    Set the fork callback.
    @param http Http service object.
    @param proc Fork callback procedure
    @param arg Argument to supply when the callback is invoked.
    @ingroup HttpConn
 */
extern void httpSetForkCallback(struct Http *http, MprForkCallback proc, void *arg);

/************************************ Http **********************************/
/** 
    Http service object
    @description The Http service is managed by a single service object.
    @stability Evolving
    @defgroup Http Http
    @see Http HttpConn HttpEndpoint gettGetDateString httpConfigurenamedVirtualEndpoint httpCreate httpCreateSecret 
        httpDestroy httpGetContext httpGetDateString httpLoadSsl httpLookupEndpoint httpLookupStatus httpLooupHost 
        httpSetContext httpSetDefaultClientHost httpSetDefaultClientPort httpSetDefaultHost httpSetDefaultPort 
        httpSetForkCallback httpSetProxy httpSetSoftware 
 */
typedef struct Http {
    MprList         *endpoints;             /**< Currently configured listening endpoints */
    MprList         *hosts;                 /**< List of host objects */
    MprList         *connections;           /**< Currently open connection requests */
    MprHash         *stages;                /**< Possible stages in connection pipelines */
    MprHash         *statusCodes;           /**< Http status codes */

    MprHash         *routeTargets;          /**< Http route target functions */
    MprHash         *routeConditions;       /**< Http route condition functions */
    MprHash         *routeUpdates;          /**< Http route update functions */

    /*  
        Some standard pipeline stages
     */
    struct HttpStage *netConnector;         /**< Default network connector */
    struct HttpStage *sendConnector;        /**< Optimized sendfile connector */
    struct HttpStage *rangeFilter;          /**< Ranged requests filter */
    struct HttpStage *cacheFilter;          /**< Cache filter */
    struct HttpStage *chunkFilter;          /**< Chunked transfer encoding filter */
    struct HttpStage *cacheHandler;         /**< Cache filter */
    struct HttpStage *cgiHandler;           /**< CGI listing handler */
    struct HttpStage *dirHandler;           /**< Directory listing handler */
    struct HttpStage *egiHandler;           /**< Embedded Gateway Interface (EGI) handler */
    struct HttpStage *ejsHandler;           /**< Ejscript Web Framework handler */
    struct HttpStage *fileHandler;          /**< Static file handler */
    struct HttpStage *passHandler;          /**< Pass through handler */
    struct HttpStage *phpHandler;           /**< PHP through handler */
    struct HttpStage *uploadFilter;         /**< Upload filter */

    struct HttpLimits *clientLimits;        /**< Client resource limits */
    struct HttpLimits *serverLimits;        /**< Server resource limits */
    struct HttpRoute *clientRoute;          /**< Default route for clients */

    MprEvent        *timer;                 /**< Admin service timer */
    MprEvent        *timestamp;             /**< Timestamp timer */
    MprTime         booted;                 /**< Time the server started */
    MprTime         now;                    /**< When was the currentDate last computed */
    MprMutex        *mutex;
    HttpGetPassword getPassword;            /**< Lookup password callback */
    HttpValidateCred validateCred;          /**< Validate user credentials callback */
    char            *software;              /**< Software name and version */
    void            *forkData;

    int             connCount;              /**< Count of connections for Conn.seqno */
    void            *context;               /**< Embedding context */
    MprTime         currentTime;            /**< When currentDate was last calculated */
    char            *currentDate;           /**< Date string for HTTP response headers */
    char            *expiresDate;           /**< Convenient expiry date (1 day in advance) */
    char            *secret;                /**< Random bytes for authentication */

    char            *defaultClientHost;     /**< Default ip address */
    int             defaultClientPort;      /**< Default port */

    char            *protocol;              /**< HTTP/1.0 or HTTP/1.1 */
    char            *proxyHost;             /**< Proxy ip address */
    int             proxyPort;              /**< Proxy port */
    int             processCount;           /**< Count of current active external processes */
    int             sslLoaded;              /**< True when the SSL provider has been loaded */

    /*
        Callbacks
     */
    HttpEnvCallback     envCallback;        /**< SetEnv callback */
    MprForkCallback     forkCallback;       /**< Callback in child after fork() */
    HttpListenCallback  listenCallback;     /**< Invoked when creating listeners */
    HttpRedirectCallback redirectCallback;  /**< Redirect callback */
} Http;

/**
    Create a Http service object
    @description Create a http service object. One http service object should be created per application.
    @return The http service object.
    @ingroup Http
 */
extern Http *httpCreate();

/**
    Configure endpoints with named virtual hosts
    @param http Http service object.
    @param ip IP address for the named virtual host
    @param port address for the named virtual host
    @ingroup HttpEndpoint
 */
extern int httpConfigureNamedVirtualEndpoints(Http *http, cchar *ip, int port);

/**  
    Create the Http secret data for crypto
    @description Create http secret data that is used to seed SSL-based communications.
    @param http Http service object.
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup Http
 */
extern int httpCreateSecret(Http *http);

/**
    Destroy the Http service object
    @param http Http service object.
    @ingroup Http
 */
extern void httpDestroy(Http *http);

/**
    Get the http context object
    @param http Http service object.
    @return The http context object defined via httpSetContext
    @ingroup Http
 */
extern void *httpGetContext(Http *http);

/**
    Get the time as an ISO date string
    @param sbuf Optional path buffer. If supplied, the modified time of the path is used. If NULL, then the current
        time is used.
    @return RFC822 formatted date string. 
    @ingroup Http
 */
extern char *httpGetDateString(MprPath *sbuf);

/**
    Load SSL
    @description This loads the configured SSL provider. SSL providers are configured when the product is built
        from source.
    @param http Http object created via #httpCreate
    @return "Zero" if SSL can be successfully loaded. Otherwise return a negative MPR error code.
    @ingroup Http
 */
extern int httpLoadSsl(Http *http);

/**
    Set the http context object
    @param http Http object created via #httpCreate
    @param context New context object
 */
extern void httpSetContext(Http *http, void *context);

/** 
    Define a default client host
    @description Define a default host to use for client connections if the URI does not specify a host
    @param http Http object created via $httpCreateConn
    @param host Host or IP address
    @ingroup HttpConn
 */
extern void httpSetDefaultClientHost(Http *http, cchar *host);

/** 
    Define a default client port
    @description Define a default port to use for client connections if the URI does not define a port
    @param http Http object created via $httpCreateConn
    @param port Integer port number
    @ingroup HttpConn
 */
extern void httpSetDefaultClientPort(Http *http, int port);

/** 
    Define a Http proxy host to use for all client connect requests.
    @description Define a http proxy host to communicate via when accessing the net.
    @param http Http object created via #httpCreate
    @param host Proxy host name or IP address
    @param port Proxy host port number.
    @ingroup Http
 */
extern void httpSetProxy(Http *http, cchar *host, int port);

/**
    Lookup a Http status code
    @description Lookup the code and return the corresponding text message briefly expaining the status.
    @param http Http object created via #httpCreate
    @param status Http status code
    @return Text message corresponding to the status code
    @ingroup Http
 */
extern cchar *httpLookupStatus(Http *http, int status);

/**
    Lookup a host by name
    @param http Http object created via #httpCreate
    @param name The name of the host to find
    @return The corresponding host object
    @ingroup Http
 */
extern struct HttpHost *httpLookupHost(Http *http, cchar *name);

/**
    Lookup a listening endpoint
    @param http Http object created via #httpCreate
    @param ip Listening IP address to look for
    @param port Listening port number
    @return HttpEndpoint object
 */
extern struct HttpEndpoint *httpLookupEndpoint(Http *http, cchar *ip, int port);

/**
    Set the software description
    @param http Http object created via #httpCreate
    @param description String describing the Http software. By default, this is set to HTTP_NAME.
 */
extern void httpSetSoftware(Http *http, cchar *description);

/* Internal APIs */
extern void httpAddConn(Http *http, struct HttpConn *conn);
extern struct HttpEndpoint *httpGetFirstEndpoint(Http *http);
extern void httpRemoveConn(Http *http, struct HttpConn *conn);
extern void httpAddEndpoint(Http *http, struct HttpEndpoint *endpoint);
extern void httpRemoveEndpoint(Http *http, struct HttpEndpoint *endpoint);
extern void httpAddHost(Http *http, struct HttpHost *host);
extern void httpRemoveHost(Http *http, struct HttpHost *host);
extern void httpDefineRouteBuiltins();

/************************************* Limits *********************************/
/** 
    Http limits
    @stability Evolving
    @defgroup HttpLimits HttpLimits
    @see HttpLimits httpInitLimits httpCreateLimits httpEaseLimits httpEnableTraceMethod
 */
typedef struct HttpLimits {
    ssize   chunkSize;              /**< Maximum chunk size for transfer encoding */
    ssize   headerSize;             /**< Maximum size of the total header */
    ssize   stageBufferSize;        /**< Maximum buffering by any pipeline stage */
    ssize   uriSize;                /**< Maximum size of a uri */
    ssize   cacheItemSize;          /**< Maximum size of a cachable item */

    MprOff  receiveFormSize;        /**< Maximum size of form data */
    MprOff  receiveBodySize;        /**< Maximum size of receive body data */
    MprOff  transmissionBodySize;   /**< Maximum size of transmission body content */
    MprOff  uploadSize;             /**< Maximum size of an uploaded file */

    int     clientMax;              /**< Maximum number of simultaneous clients endpoints */
    int     headerMax;              /**< Maximum number of header lines */
    int     keepAliveMax;           /**< Maximum number of Keep-Alive requests to perform per socket */
    int     requestMax;             /**< Maximum number of simultaneous concurrent requests */
    int     processMax;             /**< Maximum number of processes (CGI) */

    MprTime inactivityTimeout;      /**< Default timeout for keep-alive and idle requests (msec) */
    MprTime requestTimeout;         /**< Default time a request can take (msec) */
    MprTime sessionTimeout;         /**< Default time a session can persist (msec) */

    int     enableTraceMethod;      /**< Trace method enabled */
} HttpLimits;

/**
    Initialize a limits object with default values
    @param limits Limits object to modify
    @param serverSide Set to "true" for server side limits. Set to "false" for client side default limits
    @ingroup HttpLimits
 */
extern void httpInitLimits(HttpLimits *limits, bool serverSide);

/**
    Create a new limits object
    @description Create and initialize a new limits object with default values
    @param serverSide Set to "true" for server side limits. Set to "false" for client side default limits
    @return The allocated limits object
    @ingroup HttpLimits
 */
extern HttpLimits *httpCreateLimits(int serverSide);

/**
    Ease the limits
    @description This increases the receive body size, transmission body size and upload size to the maximum 
        sizes supported by the system.
    @param limits Limits object. This can be either HttpHost.limits HttpConn.limits or HttpEndpoint.limits
    @ingroup HttpLimits
 */
extern void httpEaseLimits(HttpLimits *limits);

/**
    Enable use of the TRACE Http method by the object owning the limits object.
    @param limits Limits 
    @param on Set to true to enable the trace method
    @param on Set to 1 to enable
    @ingroup HttpLimits
 */
extern void httpEnableTraceMethod(struct HttpLimits *limits, bool on);

/************************************* URI Services ***************************/
/** 
    URI management
    @description The HTTP provides routines for formatting and parsing URIs. Routines are also provided
        to escape dangerous characters for URIs as well as HTML content and shell commands.
    @stability Evolving
    @see HttpConn httpCloneUri httpCompleteUri httpCreateUri httpCreateUriFromParts httpFormatUri httpGetRelativeUri 
        httpJoinUri httpJoinUriPath httpLookupMimeType httpMakeUriLocal httpNormalizeUriPath httpResolveUri 
        httpUriToString 
    @defgroup HttpUri HttpUri
 */
typedef struct HttpUri {
    char        *scheme;                /**< URI scheme (http|https|...) */
    char        *host;                  /**< Host name */
    char        *path;                  /**< Uri path (without scheme, host, query or fragements) */
    char        *ext;                   /**< Document extension */
    char        *reference;             /**< Reference fragment within the specified resource */
    char        *query;                 /**< Query string */
    int         port;                   /**< Port number */
    int         flags;                  /**< Flags */
    int         secure;                 /**< Using https */
    char        *uri;                   /**< Original URI (not decoded) */
} HttpUri;

#define HTTP_COMPLETE_URI       0x1     /**< Complete all missing URI fields */
#define HTTP_COMPLETE_URI_PATH  0x2     /**< Complete missing URI path */


/** 
    Create and initialize a URI.
    @description Parse a uri and return a tokenized HttpUri structure.
    @param uri Uri string to parse
    @param flags Set to HTTP_COMPLETE_URI to add missing components. ie. Add scheme, host and port if not supplied. 
    @return A newly allocated HttpUri structure. 
    @ingroup HttpUri
 */
extern HttpUri *httpCreateUri(cchar *uri, int flags);

/**
    Clone a URI
    @description This call copies the base URI and optionally completes missing fields in the URI
    @param base Base URI to copy
    @param flags Set to HTTP_COMPLETE_URI to add missing components. ie. Add scheme, host and port if not supplied. 
    @return A new URI object
    @ingroup HttpUri
 */
extern HttpUri *httpCloneUri(HttpUri *base, int flags);

/** 
    Format a URI
    @description Format a URI string using the input components.
    @param scheme Protocol string for the uri. Example: "http"
    @param host Host or IP address
    @param port TCP/IP port number
    @param path URL path
    @param ref URL reference fragment
    @param query Additiona query parameters.
    @param flags Set to HTTP_COMPLETE_URI to add missing components. ie. Add scheme, host and port if not supplied. 
    @return A newly allocated uri string
    @ingroup HttpUri
 */
extern char *httpFormatUri(cchar *scheme, cchar *host, int port, cchar *path, cchar *ref, cchar *query, 
        int flags);

/**
    Create a URI from parts
    @description This call constructs a URI from the given parts. Various URI parts can be omitted by setting to null.
        The URI path is the only mandatory parameter.
    @param scheme The URI scheme. This is typically "http" or "https".
    @param host The URI host name portion. This can be a textual host and domain name or it can be an IP address.
    @param port The URI port number. Set to zero to accept the default value for the selected scheme.
    @param path The URI path to the requested document.
    @param reference URI reference with an HTML document. This is the URI component after the "#" in the URI path.
    @param query URI query component. This is the URI component after the "?" in the URI.
    @param flags Set to HTTP_COMPLETE_URI to add missing components. ie. Add scheme, host and port if not supplied. 
    @return A new URI 
    @ingroup HttpUri
 */
extern HttpUri *httpCreateUriFromParts(cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, 
        cchar *query, int flags);

/** 
    Get the mime type for an extension.
    This call will return the mime type from a limited internal set of mime types for the given path or extension.
    @param ext Path or extension to examine
    @returns Mime type. This is a static string.
    @ingroup HttpUri
 */
extern cchar *httpLookupMimeType(cchar *ext);

/** 
    Convert a Uri to a string.
    @description Convert the given Uri to a string, optionally completing missing parts such as the host, port and path.
    @param uri A Uri object created via httpCreateUri 
    @param flags Set to HTTP_COMPLETE_URI to add missing components. ie. Add scheme, host and port if not supplied. 
    @return A newly allocated uri string. 
    @ingroup HttpUri
 */
extern char *httpUriToString(HttpUri *uri, int flags);

/** 
    Normalize a URI
    @description Validate and canonicalize a URI path. This removes redundant "./" sequences and simplifies "../dir" 
        references. 
    @param uri Uri path string to normalize. This is the URI path portion without scheme, host and port components.
    @return A new validated uri string. 
    @ingroup HttpUri
 */
extern char *httpNormalizeUriPath(cchar *uri);

/**
    Normalize a URI
    @description Validate and canonicalize a URI. This invokes httpNormalizeUriPath to normalize the URI path.
    @param uri URI object to normalize
    @ingroup HttpUri
 */
extern void httpNormalizeUri(HttpUri *uri);

/**
    Join URIs
    @param base Base URI to being with
    @param argc Count of URIs in others
    @param others Array of URIs to join to the base
    @return The resulting, joined URI
    @ingroup HttpUri
 */
extern HttpUri *httpJoinUri(HttpUri *base, int argc, HttpUri **others);

/**
    Join a URI path
    @param result URI that will be modified with a joined path
    @param base URI supplying the base path 
    @param other Other URI whose path is joined to the base
    @return The result URI
    @ingroup HttpUri
 */
extern HttpUri *httpJoinUriPath(HttpUri *result, HttpUri *base, HttpUri *other);

/**
    Complete the given URI
    @description Complete the URI supplying missing URI components from the other URI. This modifies the supplied URI and
        does not allocate or create a new URI.
    @param uri URI to complete
    @param other Other URI to supply the missing components
    @return The supplied URI.
    @ingroup HttpUri
  */
extern HttpUri *httpCompleteUri(HttpUri *uri, HttpUri *other);

/**
    Get a relative URI from the base to the target
    @description If the target is null, an absolute URI, or if a relative URI from the base cannot be constructed, then
        the target will be returned. If clone is true, then a clone of the target will be returned.
    @param base The base URI considered to be the current URI. Think of this as the current directory.
    @param target The destination URI for which a relative URI will be crafted to reach.
    @param clone If true, the target URI will be cloned if the target is an absolute URI or if a relative URI 
        cannot be constructed.
    @ingroup HttpUri
  */
extern HttpUri *httpGetRelativeUri(HttpUri *base, HttpUri *target, int clone);

/**
    Resolve URIs relative to a base
    @param base Base URI to begin with
    @param argc Count of URIs in the others array
    @param others Array of URIs that are sucessively resolved relative to the base
    @param local If true, the base scheme, host and port are ignored
    @ingroup HttpUri
  */
extern HttpUri *httpResolveUri(HttpUri *base, int argc, HttpUri **others, bool local);

/**
    Make a URI local
    @description This routine removes the scheme, host and port portions of a URI
    @param uri URI to modify
    @return The given URI. 
    @ingroup HttpUri
 */
extern HttpUri *httpMakeUriLocal(HttpUri *uri);

/************************************* Range **********************************/
/** 
    Content range structure
    @pre
        Range:  0,  49  First 50 bytes
        Range: -1, -50  Last 50 bytes
        Range:  1,  -1  Skip first byte then select content to the end
    @stability Evolving
    @defgroup HttpRange HttpRange
    @see HttpRange
 */
typedef struct HttpRange {
    MprOff              start;                  /**< Start of range */
    MprOff              end;                    /**< End byte of range + 1 */
    MprOff              len;                    /**< Redundant range length */
    struct HttpRange    *next;                  /**< Next range */
} HttpRange;

/************************************* Packet *********************************/
/*  
    Packet flags
 */
#define HTTP_PACKET_HEADER    0x1               /**< Packet contains HTTP headers */
#define HTTP_PACKET_RANGE     0x2               /**< Packet is a range boundary packet */
#define HTTP_PACKET_DATA      0x4               /**< Packet contains actual content data */
#define HTTP_PACKET_END       0x8               /**< End of stream packet */

/**
    Callback procedure to fill a packet with data
    @param q Queue owning the packet
    @param packet The packet to fill
    @param off Offset in the packet to fill with data
    @param size Size of packet from the offset to fill.
    @return The number of bytes copied into the packet.
    @ingroup HttpPacket
 */
typedef ssize (*HttpFillProc)(struct HttpQueue *q, struct HttpPacket *packet, MprOff pos, ssize size);

/** 
    Packet object. 
    @description The request/response pipeline sends data and control information in HttpPacket objects. The output
        stream typically consists of a HEADER packet followed by zero or more data packets and terminated by an END
        packet. If the request has input data, the input stream consists of one or more data packets followed by
        an END packet.
        \n\n
        Packets contain data and optional prefix or suffix headers. Packets can be split, joined, filled, or emptied. 
        The pipeline stages will fill or transform packet data as required.
    @stability Evolving
    @defgroup HttpPacket HttpPacket
    @see HttpFillProc HttpPacket HttpQueue httpAdjustPacketEnd httpAdjustPacketStart httpClonePacket 
        httpCreateDataPacket httpCreateEndPacket httpCreateEntityPacket httpCreateHeaderPacket httpCreatePacket 
        httpGetPacket httpGetPacketLength httpJoinPacket 
        httpPutBackPacket httpPutForService httpPutPacket httpPutPacketToNext httpSplitPacket 
 */
typedef struct HttpPacket {
    MprBuf          *prefix;                /**< Prefix message to be emitted before the content */
    MprBuf          *content;               /**< Chunk content */
    MprOff          esize;                  /**< Data size in entity (file) */
    MprOff          epos;                   /**< Data position in entity (file) */
    HttpFillProc    fill;                   /**< Callback to fill packet with data */
    int             flags;                  /**< Packet flags */
    struct HttpPacket *next;                /**< Next packet in chain */
} HttpPacket;

/**
    Adjust the packet starting position.
    This adjusts the packet content by the given size. The packet position is incremented by start and the packet
    length (size) is decremented. If the packet describes entity data, the given size amount to the Packet.epos and 
    decrements the Packet.esize fields. If the packet has actual data buffered in Packet.content, the content buffer
    start is incremeneted by the size amount.
    @param packet Packet to modify
    @param size Size to add to the packet current position.
 */
extern void httpAdjustPacketStart(HttpPacket *packet, MprOff size);

/**
    Adjust the packet end position.
    This adjusts the packet content by the given size. The packet length (size) is decremented by the requested 
    amount. If the packet describes entity data, the Packet.esize field is reduced by the requested size amount. If the 
    packet has actual data buffered in Packet.content, the content buffer end position is reduced by
    by the size amount.
    @param packet Packet to modify
    @param size Size to adjust packet end position.
 */
extern void httpAdjustPacketEnd(HttpPacket *packet, MprOff size);

/**
    Clone a packet
    @param orig Original packet to clone
    @return A new packet equivalent to the original
    @ingroup HttpPacket
 */
extern HttpPacket *httpClonePacket(HttpPacket *orig);

/** 
    Create a data packet
    @description Create a packet and set the HTTP_PACKET_DATA flag
        Data packets convey data through the response pipeline.
    @param size Size of the package data storage.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateDataPacket(ssize size);

/** 
    Create an eof packet
    @description Create an end-of-stream packet and set the HTTP_PACKET_END flag. The end pack signifies the 
        end of data. It is used on both incoming and outgoing streams through the request/response pipeline.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateEndPacket();

/** 
    Create an entity data packet
    @description Create an entity packet and set the HTTP_PACKET_DATA flag.
        Entity packets describe the resource (entity) to send to the client and provide a #HttpFillProc procedure
        used to fill packets with data from the entity.
    @param pos Position within the entity for packet data 
    @param size Size of the entity data
    @param fill HttpFillProc callback to supply the entity data.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateEntityPacket(MprOff pos, MprOff size, HttpFillProc fill);

/** 
    Create a response header packet
    @description Create a response header packet and set the HTTP_PACKET_HEADER flag. 
        A header packet is used by the pipeline to hold the response headers.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateHeaderPacket();

/** 
    Create a data packet
    @description Create a packet of the required size.
    @param size Size of the package data storage.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreatePacket(ssize size);

/** 
    Get the next packet from a queue
    @description Get the next packet. This will remove the packet from the queue and adjust the queue counts
        accordingly. If the queue is full and upstream queues are blocked, they will be enabled.
    @param q Queue reference
    @return The packet removed from the queue.
    @ingroup HttpQueue
 */
extern HttpPacket *httpGetPacket(struct HttpQueue *q);

#if DOXYGEN
/** 
    Get the length of the packet data contents.
    @description Get the content length of a packet. This does not include the prefix or virtual data length -- just
    the pure buffered data contents. 
    @param packet Packet to examine.
    @return Count of bytes contained by the packet.
    @ingroup HttpPacket
 */
extern ssize httpGetPacketLength(HttpPacket *packet);
#else
#define httpGetPacketLength(p) (p->content ? mprGetBufLength(p->content) : 0)
#endif

/** 
    Join two packets
    @description Join the contents of one packet to another by copying the data from the \a other packet into 
        the first packet. 
    @param packet Destination packet
    @param other Other packet to copy data from.
    @return "Zero" if successful, otherwise a negative Mpr error code
    @ingroup HttpPacket
 */
extern int httpJoinPacket(HttpPacket *packet, HttpPacket *other);

/** 
    Split a data packet
    @description Split a data packet at the specified offset. Packets may need to be split so that downstream
        stages can digest their contents. If a packet is too large for the queue maximum size, it should be split.
        When the packet is split, a new packet is created containing the data after the offset. Any suffix headers
        are moved to the new packet.
    @param packet Packet to split
    @param offset Route in the original packet at which to split
    @return New HttpPacket object containing the data after the offset. No need to free, unless you have a very long
        running request. Otherwise the packet memory will be released automatically when the request completes.
    @ingroup HttpPacket
 */
extern HttpPacket *httpSplitPacket(HttpPacket *packet, ssize offset);

/*
    Internal
 */
#define httpGetPacketEntityLength(p) (p->content ? mprGetBufLength(p->content) : packet->esize)

/************************************* Queue *********************************/
/*  
    Queue directions
 */
#define HTTP_QUEUE_TX             0           /**< Send (transmit to client) queue */
#define HTTP_QUEUE_RX             1           /**< Receive (read from client) queue */
#define HTTP_MAX_QUEUE            2           /**< Number of queue types */

/* 
   Queue flags
 */
#define HTTP_QUEUE_OPEN           0x1         /**< Queue's open routine has been called */
#define HTTP_QUEUE_SUSPENDED      0x2         /**< Queue's service routine is suspended due to flow control */
#if UNUSED && MOB 
#define HTTP_QUEUE_DISABLED       0x4         /**< Queue's service routine is disabled. Requires manual enable */
#define HTTP_QUEUE_FULL           0x8         /**< Queue is full */
#endif
#define HTTP_QUEUE_ALL            0x10        /**< Queue has all the data there is and will be */
#define HTTP_QUEUE_SERVICED       0x20        /**< Queue has been serviced at least once */
#define HTTP_QUEUE_EOF            0x40        /**< Queue at end of data */
#define HTTP_QUEUE_STARTED        0x80        /**< Queue started */
#define HTTP_QUEUE_RESERVICE      0x100       /**< Queue requires reservicing */

/*  
    Queue callback prototypes
 */
typedef void (*HttpQueueOpen)(struct HttpQueue *q);
typedef void (*HttpQueueClose)(struct HttpQueue *q);
typedef void (*HttpQueueStart)(struct HttpQueue *q);
typedef void (*HttpQueueData)(struct HttpQueue *q, HttpPacket *packet);
typedef void (*HttpQueueService)(struct HttpQueue *q);

/** 
    Queue object
    @description The request pipeline consists of a full-duplex pipeline of stages. Each stage has two queues,
        one for outgoing data and one for incoming. A HttpQueue object manages the data flow for a request stage
        and has the ability to queue and process data, manage flow control, and schedule packets for service.
        \n\n
        Queue's provide open, close, put, and service methods. These methods manage and respond to incoming packets.
        A queue can respond immediately to an incoming packet by processing or dispatching a packet in its put() method.
        Alternatively, the queue can defer processing by queueing the packet on it's service queue and then waiting for
        it's service() method to be invoked. 
        \n\n
        If a queue does not define a put() method, the default put() method will 
        be used which queues data onto the service queue. The default incoming put() method joins incoming packets
        into a single packet on the service queue.
    @stability Evolving
    @defgroup HttpQueue HttpQueue
    @see HttpConn HttpPacket HttpQueue httpDisableQueue httpDiscardQueueData httpEnableQueue httpFlushQueue httpGetQueueRoom
        httpIsEof httpIsPacketTooBig httpIsQueueEmpty httpJoinPacketForService httpJoinPackets httpOpenQueue
        httpPutBackPacket httpPutForService httpPutPacket httpPutPacketToNext httpRemoveQueue httpResizePacket
        httpResumeQueue httpScheduleQueue httpServiceQueue httpSuspendQueue
        httpWillNextQueueAccept httpWillNextQueueAcceptSize httpWrite httpWriteBlock httpWriteBody httpWriteString 
 */
typedef struct HttpQueue {
    cchar               *owner;                 /**< Name of owning stage */
    ssize               count;                  /**< Bytes in queue (Does not include virt packet data) */
    ssize               max;                    /**< Maxiumum queue size */
    ssize               low;                    /**< Low water mark for flow control */
    ssize               packetSize;             /**< Maximum acceptable packet size */
    int                 flags;                  /**< Queue flags */
    HttpPacket          *first;                 /**< First packet in queue (singly linked) */
    HttpPacket          *last;                  /**< Last packet in queue (tail pointer) */
    struct HttpQueue    *nextQ;                 /**< Downstream queue for next stage */
    struct HttpQueue    *prevQ;                 /**< Upstream queue for prior stage */
    struct HttpStage    *stage;                 /**< Stage owning this queue */
    struct HttpConn     *conn;                  /**< Connection owning this queue */
    HttpQueueOpen       open;                   /**< Open the queue */
    HttpQueueClose      close;                  /**< Close the queue */
    HttpQueueStart      start;                  /**< Start the queue */
    HttpQueueData       put;                    /**< Callback to receive a packet */
    HttpQueueService    service;                /**< Service the queue */
    struct HttpQueue    *scheduleNext;          /**< Next linkage when queue is on the service queue */
    struct HttpQueue    *schedulePrev;          /**< Previous linkage when queue is on the service queue */
    struct HttpQueue    *pair;                  /**< Queue for the same stage in the opposite direction */
    int                 servicing;              /**< Currently being serviced */
    int                 direction;              /**< Flow direction */
    void                *queueData;             /**< Stage instance data */

    /*  
        Connector instance data
     */
    MprIOVec            iovec[HTTP_MAX_IOVEC];
    int                 ioIndex;                /**< Next index into iovec */
    int                 ioFile;                 /**< Sending a file */
    MprOff              ioCount;                /**< Count of bytes in iovec including file I/O */
    MprOff              ioPos;                  /**< Position in file for sendfile */
} HttpQueue;


/** 
    Disable a queue
    @description Disable a queue so that it will not be scheduled for service. The queue will remain disabled until
        httpEnableQueue is called.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpDisableQueue(HttpQueue *q);

/** 
    Discard all data from the queue
    @description Discard data from the queue. If removePackets (not yet implemented) is "true", then remove the packets.
        Oherwise, just discard the data and preserve the packets.
    @param q Queue reference
    @param removePackets If "true", the data packets will be removed from the queue.
    @ingroup HttpQueue
 */
extern void httpDiscardQueueData(HttpQueue *q, bool removePackets);

/** 
    Enable a queue after it has been disabled.
    @description Enable a queue for service and schedule it to run. This will cause the service routine
        to run as soon as possible.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpEnableQueue(HttpQueue *q);

/**
    Flush queue data
    @description This initiates writing buffered queue data (flushes) by scheduling the queue and servicing all
    scheduled queues.  If blocking is requested, the call will block until the queue count falls below the queue
    maximum.
    WARNING: Be very careful when using blocking == true. Should only be used by end applications and not by middleware.
    @param q Queue to flush
    @param block If set to "true", this call will block until the data has drained below the queue maximum.
    @return "True" if there is room for more data in the queue after flushing.
 */
extern bool httpFlushQueue(HttpQueue *q, bool block);

/** 
    Get the room in the queue
    @description Get the amount of data the queue can accept before being full.
    @param q Queue reference
    @return A count of bytes that can be written to the queue
    @ingroup HttpQueue
 */
extern ssize httpGetQueueRoom(HttpQueue *q);

/**
    Test if the connection has received all incoming content
    @description This tests if the connection is at an "End of File condition.
    @param conn HttpConn object created via $httpCreateConn
    @return "True" if all Receive content has been received 
    @ingroup HttpQueue
 */
extern bool httpIsEof(struct HttpConn *conn);

/** 
    Test if a packet is too big 
    @description Test if a packet is too big to fit downstream. If the packet content exceeds the downstream queue's 
        maximum or exceeds the downstream queue's requested packet size -- then this routine will return "true".
    @param q Queue reference
    @param packet Packet to test
    @return "True" if the packet is too big for the downstream queue
    @ingroup HttpQueue
 */
extern bool httpIsPacketTooBig(struct HttpQueue *q, HttpPacket *packet);

/** 
    Determine if the queue is empty
    @description Determine if the queue has no packets queued. This does not test if the queue has no data content.
    @param q Queue reference
    @return "True" if there are no packets queued.
    @ingroup HttpQueue
 */
extern bool httpIsQueueEmpty(HttpQueue *q);

/**
    Join the packets together
    @description This call joins data packets (on the given queue) together - up to the designated maximum size.
        The maximum size is also limited by the downstream queue maximum packet size.
    @param q Queue to examine
    @param size The maximum-sized packet that will be created by joining queue packets is the minimum of the given size
        and the downstream queues maximum packet size.
    @ingroup HttpQueue
 */
extern void httpJoinPackets(HttpQueue *q, ssize size);

/** 
    Join a packet onto the service queue
    @description Add a packet to the service queue. If the queue already has data, then this packet
        will be joined (aggregated) into the existing packet. If serviceQ is true, the queue will be scheduled
        for service.
    @param q Queue reference
    @param packet Packet to join to the queue
    @param serviceQ If true, schedule the queue for service
    @ingroup HttpQueue
 */
extern void httpJoinPacketForService(struct HttpQueue *q, HttpPacket *packet, bool serviceQ);

/** 
    Open the queue. Call the queue open entry point.
    @param q Queue reference
    @param chunkSize Preferred chunk size
    @return "Zero" if successful.
 */
extern int httpOpenQueue(HttpQueue *q, ssize chunkSize);

/** 
    Put a packet back onto a queue
    @description Put the packet back onto the front of the queue. The queue's put() method is not called.
        This is typically used by the queue's service routine when a packet cannot complete processing.
    @param q Queue reference
    @param packet Packet to put back
    @ingroup HttpQueue
 */
extern void httpPutBackPacket(struct HttpQueue *q, HttpPacket *packet);

/*
    Convenience flags for httpPutForService in the serviceQ argument
 */
#define HTTP_DELAY_SERVICE      0           /**< Delay servicing the queue */
#define HTTP_SCHEDULE_QUEUE     1           /**< Schedule the queue for service */

/** 
    Put a packet into the service queue for deferred processing.
    @description Add a packet to the service queue. If serviceQ is true, the queue will be scheduled for service.
    @param q Queue reference
    @param packet Packet to join to the queue
    @param serviceQ If true, schedule the queue for service
    @ingroup HttpQueue
 */
extern void httpPutForService(struct HttpQueue *q, HttpPacket *packet, bool serviceQ);

/** 
    Put a packet to the queue.
    @description The packet is passed to the queue by invoking its put() callback. 
        Note the receiving queue may immediately process the packet or it may choose to defer processing by putting to
        its service queue.  @param q Queue reference
    @param packet Packet to put
    @ingroup HttpQueue
 */
extern void httpPutPacket(struct HttpQueue *q, HttpPacket *packet);

/** 
    Put a packet to the next queue downstream.
    @description Put a packet onto the next downstream queue by calling the downstream queue's put() method. 
        Note the receiving queue may immediately process the packet or it may choose to defer processing by putting to
        its service queue.  @param q Queue reference
    @param q Queue reference. The packet will not be queued on this queue, but rather on the queue downstream.
    @param packet Packet to put
    @ingroup HttpQueue
 */
extern void httpPutPacketToNext(struct HttpQueue *q, HttpPacket *packet);

/** 
    Remove a queue
    @description Remove a queue from the request/response pipeline. This will remove a queue so that it does
        not participate in the pipeline, effectively removing the processing stage from the pipeline. This is
        useful to remove unwanted filters and to speed up pipeline processing
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpRemoveQueue(HttpQueue *q);

/** 
    Resize a packet
    @description Resize a packet, if required, so that it fits in the downstream queue. This may split the packet
        if it is too big to fit in the downstream queue. If it is split, the tail portion is put back on the queue.
    @param q Queue reference
    @param packet Packet to put
    @param size If size is > 0, then also ensure the packet is not larger than this size.
    @return "Zero" if successful, otherwise a negative Mpr error code
    @ingroup HttpQueue
 */
extern int httpResizePacket(struct HttpQueue *q, HttpPacket *packet, ssize size);

/** 
    Resume a queue
    @description Resume a queue for service and schedule it to run. This will cause the service routine
        to run as soon as possible. This is normally called automatically called by the pipeline when downstream
        congestion has cleared.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpResumeQueue(HttpQueue *q);

/** 
    Schedule a queue
    @description Schedule a queue by adding it to the schedule queue. Queues are serviced FIFO.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpScheduleQueue(HttpQueue *q);

/** 
    Service a queue
    @description Service a queue by invoking its service() routine. 
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpServiceQueue(HttpQueue *q);

/** 
    Suspend a queue. 
    @description Suspended a queue so that it will not be scheduled for service. The pipeline will 
    will automatically call httpResumeQueue when the downstream queues are less congested.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpSuspendQueue(HttpQueue *q);

/**
    Verify a queue 
    @param q Queue reference
    @return "True" if the queue verifies
    @internal
 */
extern bool httpVerifyQueue(HttpQueue *q);

/** 
    Determine if the downstream queue will accept this packet.
    @description Test if the downstream queue will accept a packet. The packet will be resized, if required, in an
        attempt to get the downstream queue to accept it. If the downstream queue is full, disable this queue
        and mark the downstream queue as full, and service it immediately to try to relieve the congestion.
    @param q Queue reference
    @param packet Packet to put
    @return "True" if the downstream queue will accept the packet. Use $httpPutPacketToNext to send the packet downstream
    @ingroup HttpQueue
 */
extern bool httpWillNextQueueAcceptPacket(HttpQueue *q, HttpPacket *packet);

/** 
    Determine if the downstream queue will accept a certain amount of data.
    @description Test if the downstream queue will accept data of a given size.
    @param q Queue reference
    @param size Size of data to test for
    @return "True" if the downstream queue will accept the given sized data.
    @ingroup HttpQueue
 */
extern bool httpWillNextQueueAcceptSize(HttpQueue *q, ssize size);

/** 
    Write a formatted string
    @description Write a formatted string of data into packets onto the end of the queue. Data packets will be created
        as required to store the write data. This call always accepts all the data and will buffer as required. 
        This call may block waiting for the downstream queue to drain if it is or becomes full.
    @param q Queue reference
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @return A count of the bytes actually written
    @ingroup HttpQueue
 */
extern ssize httpWrite(HttpQueue *q, cchar *fmt, ...);

/** 
    Write a block of data to the queue
    @description Write a block of data into packets onto the end of the queue. Data packets will be created
        as required to store the write data. This call will either accept and write all the data or it will fail.
        It will never return "short", i.e. with a partial write.
    @param q Queue reference
    @param buf Buffer containing the write data
    @param size of the data in buf
    @return The size value if successful or a negative MPR error code.
    @ingroup HttpQueue
 */
extern ssize httpWriteBlock(HttpQueue *q, cchar *buf, ssize size);

/** 
    Write a string of data to the queue
    @description Write a string of data into packets onto the end of the queue. Data packets will be created
        as required to store the write data. This call may block waiting for the downstream queue to drain if it is 
        or becomes full.
    @param q Queue reference
    @param s String containing the data to write
    @return A count of the bytes actually written
    @ingroup HttpQueue
 */
extern ssize httpWriteString(HttpQueue *q, cchar *s);

/** 
    Write a safe string of data to the queue
    @description This will escape any HTML sequences before writing the string into packets onto the end of the queue. 
        Data packets will be created as required to store the write data. This call may block waiting for the 
        downstream queue to drain if it is or becomes full.
    @param q Queue reference
    @param s String containing the data to write
    @return A count of the bytes actually written
    @ingroup HttpQueue
 */
extern ssize httpWriteString(HttpQueue *q, cchar *s);

/* Internal */
extern HttpQueue *httpFindPreviousQueue(HttpQueue *q);
extern HttpQueue *httpCreateQueueHead(struct HttpConn *conn, cchar *name);
extern HttpQueue *httpCreateQueue(struct HttpConn *conn, struct HttpStage *stage, int dir, HttpQueue *prev);
extern HttpQueue *httpGetNextQueueForService(HttpQueue *q);
extern void httpInitQueue(struct HttpConn *conn, HttpQueue *q, cchar *name);
extern void httpInitSchedulerQueue(HttpQueue *q);
extern void httpInsertQueue(HttpQueue *prev, HttpQueue *q);
extern void httpMarkQueueHead(HttpQueue *q);
extern void httpAssignQueue(HttpQueue *q, struct HttpStage *stage, int dir);

/******************************** Pipeline Stages *****************************/
/*
    Stage Flags
 */
#define HTTP_STAGE_DELETE         HTTP_DELETE       /**< Support DELETE requests */
#define HTTP_STAGE_GET            HTTP_GET          /**< Support GET requests */
#define HTTP_STAGE_HEAD           HTTP_HEAD         /**< Support HEAD requests */
#define HTTP_STAGE_OPTIONS        HTTP_OPTIONS      /**< Support OPTIONS requests */
#define HTTP_STAGE_POST           HTTP_POST         /**< Support POST requests */
#define HTTP_STAGE_PUT            HTTP_PUT          /**< Support PUT requests */
#define HTTP_STAGE_TRACE          HTTP_TRACE        /**< Support TRACE requests */
#define HTTP_STAGE_UNKNOWN        HTTP_UNKNOWN      /**< Support TRACE requests */
#define HTTP_STAGE_METHODS        (HTTP_DELETE|HTTP_GET|HTTP_HEAD|HTTP_POST|HTTP_PUT) /**< Support default methods */
#define HTTP_STAGE_ALL            HTTP_METHOD_MASK  /**< Mask for every possible method including custom methods */

#define HTTP_STAGE_CONNECTOR      0x1000            /**< Stage is a connector  */
#define HTTP_STAGE_HANDLER        0x2000            /**< Stage is a handler  */
#define HTTP_STAGE_FILTER         0x4000            /**< Stage is a filter  */
#define HTTP_STAGE_MODULE         0x8000            /**< Stage is a filter  */
#define HTTP_STAGE_AUTO_DIR       0x10000           /**< Want auto directory redirection */
#define HTTP_STAGE_UNLOADED       0x20000           /**< Stage module library has been unloaded */
#define HTTP_STAGE_RX             0x40000           /**< Stage to be used in the Rx direction */
#define HTTP_STAGE_TX             0x80000           /**< Stage to be used in the Tx direction */

typedef int (*HttpParse)(Http *http, cchar *key, char *value, void *state);

/** 
    Pipeline Stages
    @description The request pipeline consists of a full-duplex pipeline of stages. 
        Stages are used to process client HTTP requests in a modular fashion. Each stage either creates, filters or
        consumes data packets. The HttpStage structure describes the stage capabilities and callbacks.
        Each stage has two queues, one for outgoing data and one for incoming data.
        \n\n
        Stages provide callback methods for parsing configuration, matching requests, open/close, run and the
        acceptance and service of incoming and outgoing data.
    @stability Evolving
    @defgroup HttpStage HttpStage 
    @see HttpConn HttpQueue HttpStage httpCloneStage httpCreateConnector httpCreateFilter httpCreateHandler 
        httpCreateStage httpDefaultOutgoingServiceStage httpGetStageData httpHandleOptionsTrace httpLookupStage 
        httpLookupStageData httpSetStageData 
 */
typedef struct HttpStage {
    char            *name;                  /**< Stage name */
    char            *path;                  /**< Backing module path (from LoadModule) */
    int             flags;                  /**< Stage flags */
    void            *stageData;             /**< Private stage data */
    MprModule       *module;                /**< Backing module */
    MprHash         *extensions;            /**< Matching extensions for this filter */
     
    /*  These callbacks apply to all stages */

    /** 
        Match a request
        @description This routine is invoked to see if the stage wishes to handle the request. For handlers, 
            the match callback is invoked when selecting the appropriate route for the request. For filters, 
            the callback is invoked subsequently when constructing the request pipeline.
            If a filter declines to handle a request, the filter will be removed from the pipeline for the 
            specified direction. The direction argument should be ignored for handlers.
        @param conn HttpConn connection object
        @param route Route object
        @param dir Queue direction. Set to HTTP_QUEUE_TX or HTTP_QUEUE_RX. Always set to HTTP_QUEUE_TX for handlers.
        @return HTTP_ROUTE_OK if the request is acceptable. Return HTTP_ROUTE_REROUTE if the request has been rewritten.
            Return HTTP_ROUTE_REJECT it the request is not acceptable.
        @ingroup HttpStage
      */
    int (*match)(struct HttpConn *conn, struct HttpRoute *route, int dir);

    /** 
        Open the queue
        @description Open the queue instance and initialize for this request.
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*open)(HttpQueue *q);

    /** 
        Close the queue
        @description Close the queue instance
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*close)(HttpQueue *q);

    /** 
        Process outgoing data.
        @description Accept a packet as outgoing data. Not used by handlers.
        @param q Queue instance object
        @param packet Packet of data
        @ingroup HttpStage
     */
    void (*outgoing)(HttpQueue *q, HttpPacket *packet);

    /** 
        Service the outgoing data queue
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*outgoingService)(HttpQueue *q);

    /** 
        Process incoming data.
        @description Accept an incoming packet of data. Not used by connectors.
        @param q Queue instance object
        @param packet Packet of data
        @ingroup HttpStage
     */
    void (*incoming)(HttpQueue *q, HttpPacket *packet);

    /** 
        Service the incoming data queue
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*incomingService)(HttpQueue *q);


    /*  These callbacks apply only to handlers */

    /** 
        Start the handler
        @description The request has been parsed and the handler may start processing. Input data may not have 
        been received yet. Form requests (those with a Content-Type of "application/x-www-form-urlencoded"), will 
        be started before processing input data. All other requests will be started immediately after the
        request headers have been parsed.
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*start)(HttpQueue *q);

    /** 
        The request is now fully ready.
        @description This callback will be invoked when all incoming data has been received. 
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*ready)(HttpQueue *q);

    /** 
        The outgoing pipeline is writable and can accept more response data.
        @description This callback will be invoked after all incoming data has been receeived and whenever the outgoing
        pipeline can absorb more output data (writable). As such, it may be called multiple times and can be effectively
        used for non-blocking generation of a response.
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*writable)(HttpQueue *q);

} HttpStage;


/**
    Create a clone of an existing state. This is used when creating filters configured to match certain extensions.
    @param http Http object returned from #httpCreate
    @param stage Stage object to clone
    @return A new stage object
    @ingroup HttpStage
*/
extern HttpStage *httpCloneStage(Http *http, HttpStage *stage);

/** 
    Create a connector stage
    @description Create a new connector. Connectors are the final stage for outgoing data. Their job is to transmit
        outgoing data to the client.
    @param http Http object returned from #httpCreate
    @param name Name of connector stage
    @param flags Stage flags mask. These specify what Http request methods will be supported by this stage. Or together
        any of the following flags:
        @li HTTP_STAGE_DELETE     - Support DELETE requests
        @li HTTP_STAGE_GET        - Support GET requests
        @li HTTP_STAGE_HEAD       - Support HEAD requests
        @li HTTP_STAGE_OPTIONS    - Support OPTIONS requests
        @li HTTP_STAGE_POST       - Support POST requests
        @li HTTP_STAGE_PUT        - Support PUT requests
        @li HTTP_STAGE_TRACE      - Support TRACE requests
        @li HTTP_STAGE_ALL        - Mask to support all methods
    @param module Optional module object for loadable stages
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateConnector(Http *http, cchar *name, int flags, MprModule *module);

/** 
    Create a filter stage
    @description Create a new filter. Filters transform data generated by handlers and before connectors transmit to
        the client. Filters can apply transformations to incoming, outgoing or bi-directional data.
    @param http Http object
    @param name Name of connector stage
    @param flags Stage flags mask. These specify what Http request methods will be supported by this stage. Or together
        any of the following flags:
        @li HTTP_STAGE_DELETE     - Support DELETE requests
        @li HTTP_STAGE_GET        - Support GET requests
        @li HTTP_STAGE_HEAD       - Support HEAD requests
        @li HTTP_STAGE_OPTIONS    - Support OPTIONS requests
        @li HTTP_STAGE_POST       - Support POST requests
        @li HTTP_STAGE_PUT        - Support PUT requests
        @li HTTP_STAGE_TRACE      - Support TRACE requests
        @li HTTP_STAGE_ALL        - Mask to support all methods
    @param module Optional module object for loadable stages
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateFilter(Http *http, cchar *name, int flags, MprModule *module);

/** 
    Create a request handler stage
    @description Create a new handler. Handlers generate outgoing data and are the final stage for incoming data. 
        Their job is to process requests and send outgoing data downstream toward the client consumer.
        There is ever only one handler for a request.
    @param http Http object
    @param name Name of connector stage
    @param flags Stage flags mask. These specify what Http request methods will be supported by this stage. Or together
        any of the following flags:
        @li HTTP_STAGE_DELETE     - Support DELETE requests
        @li HTTP_STAGE_GET        - Support GET requests
        @li HTTP_STAGE_HEAD       - Support HEAD requests
        @li HTTP_STAGE_OPTIONS    - Support OPTIONS requests
        @li HTTP_STAGE_POST       - Support POST requests
        @li HTTP_STAGE_PUT        - Support PUT requests
        @li HTTP_STAGE_TRACE      - Support TRACE requests
        @li HTTP_STAGE_ALL        - Mask to support all methods
    @param module Optional module object for loadable stages
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateHandler(Http *http, cchar *name, int flags, MprModule *module);

/** 
    Create a connector stage
    @description Create a new stage.
    @param http Http object returned from #httpCreate
    @param name Name of connector stage
    @param flags Stage flags mask. These specify what Http request methods will be supported by this stage. Or together
        any of the following flags:
        @li HTTP_STAGE_HANDLER    - Stage is a handler DELETE requests
        @li HTTP_STAGE_FILTER     - Stage is a filter
        @li HTTP_STAGE_CONNECTOR  - Stage is a connector
        @li HTTP_STAGE_GET        - Support GET requests
        @li HTTP_STAGE_HEAD       - Support HEAD requests
        @li HTTP_STAGE_OPTIONS    - Support OPTIONS requests
        @li HTTP_STAGE_POST       - Support POST requests
        @li HTTP_STAGE_PUT        - Support PUT requests
        @li HTTP_STAGE_TRACE      - Support TRACE requests
        @li HTTP_STAGE_ALL        - Mask to support all methods
    @param module Optional module object for loadable stages
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateStage(Http *http, cchar *name, int flags, MprModule *module);

/** 
    Lookup a stage by name
    @param http Http object
    @param name Name of stage to locate
    @return Stage or NULL if not found
    @ingroup HttpStage
*/
extern struct HttpStage *httpLookupStage(Http *http, cchar *name);

/** 
    Default outgoing data handling
    @description This routine provides default handling of outgoing data for stages. It simply sends all packets
        downstream.
    @param q Queue object
    @ingroup HttpStage
 */
extern void httpDefaultOutgoingServiceStage(HttpQueue *q);

/**
    Get stage data   
    @description Stages can store extra configuration information indexed by key. This is used by handlers, filters,
        connectors and and handlers.
    @param conn HttpConn connection object
    @param key Key index into the stage data
    @return A reference to the stage data. Otherwise return null if the route data for the given key was not found.
    @ingroup HttpRx
 */
extern cvoid *httpGetStageData(struct HttpConn *conn, cchar *key);

/**
    Handle a Http Trace or Options method request
    @description This call responds to a Trace or Options HTTP method request and generates an appropriate response
        to the client
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpStage
 */
extern void httpHandleOptionsTrace(struct HttpConn *conn);

/** 
    Lookup stage data
    @description This looks up the stage by name and returns the private stage data.
    @param http Http object
    @param name Name of the stage concerned
    @return Reference to the stage data block.
    @ingroup HttpStage
 */
extern void *httpLookupStageData(Http *http, cchar *name);

/**
    Set stage data   
    @description Stages can store extra configuration information indexed by key. This is used by handlers, filters,
        connectors and and handlers.
    @param conn HttpConn connection object
    @param key Key index into the stage data
    @param data Reference to custom data allocated via mprAlloc.
    @ingroup HttpRoute
 */
extern void httpSetStageData(struct HttpConn *conn, cchar *key, cvoid *data);

/* Internal APIs */
extern void httpAddStage(Http *http, HttpStage *stage);
extern int httpOpenNetConnector(Http *http);
extern int httpOpenSendConnector(Http *http);
extern int httpOpenChunkFilter(Http *http);
extern int httpOpenCacheHandler(Http *http);
extern int httpOpenPassHandler(Http *http);
extern int httpOpenRangeFilter(Http *http);
extern int httpOpenUploadFilter(Http *http);
extern void httpSendOpen(HttpQueue *q);
extern void httpSendClose(HttpQueue *q);
extern void httpSendOutgoingService(HttpQueue *q);
extern ssize httpFilterChunkData(HttpQueue *q, HttpPacket *packet);

/********************************** HttpConn *********************************/
/** 
    Notification flags
 */
#define HTTP_NOTIFY_READABLE        0x1     /**< The request has data available for reading */
#define HTTP_NOTIFY_WRITABLE        0x2     /**< The request is now writable (post / put data) */
#define HTTP_NOTIFY_CLOSED          0x4     /**< The request is now closed */
#define HTTP_NOTIFY_ERROR           0x8     /**< The request has an error */

/*  
    Connection / Request states
 */
#define HTTP_STATE_BEGIN            1       /**< Ready for a new request */
#define HTTP_STATE_CONNECTED        2       /**< Connection received or made */
#define HTTP_STATE_FIRST            3       /**< First request line has been parsed */
#define HTTP_STATE_PARSED           4       /**< Headers have been parsed, handler can start */
#define HTTP_STATE_CONTENT          5       /**< Reading posted content */
#define HTTP_STATE_READY            6       /**< Handler ready - all body data received  */
#define HTTP_STATE_RUNNING          7       /**< Handler running */
#define HTTP_STATE_COMPLETE         8       /**< Request complete */

/*
    I/O Events
*/
#define HTTP_EVENT_CLOSE           -1       /**< Connection being closed */
#define HTTP_EVENT_IO               0       /**< I/O event on connection */

/*
    Limit validation events
 */
#define HTTP_VALIDATE_OPEN_CONN     1       /**< Open a new connection */
#define HTTP_VALIDATE_CLOSE_CONN    2       /**< Close a connection */
#define HTTP_VALIDATE_OPEN_REQUEST  3       /**< Open a new request */
#define HTTP_VALIDATE_CLOSE_REQUEST 4       /**< Close a request */
#define HTTP_VALIDATE_OPEN_PROCESS  5       /**< Open a new CGI process */
#define HTTP_VALIDATE_CLOSE_PROCESS 6       /**< Close a CGI process */

/*
    Trace directions
 */
#define HTTP_TRACE_RX               0       /**< Trace reception */
#define HTTP_TRACE_TX               1       /**< Trace transmission */
#define HTTP_TRACE_MAX_DIR          2       /**< Trace transmission */

/*
    Trace items
 */
#define HTTP_TRACE_CONN             0       /**< New connections */
#define HTTP_TRACE_FIRST            1       /**< First line of header only */
#define HTTP_TRACE_HEADER           2       /**< Header */
#define HTTP_TRACE_BODY             3       /**< Body content */
#define HTTP_TRACE_LIMITS           4       /**< Request and connection count limits */
#define HTTP_TRACE_TIME             5       /**< Instrument http pipeline */
#define HTTP_TRACE_MAX_ITEM         6

typedef struct HttpTrace {
    int             disable;                     /**< If tracing is disabled for this request */
    int             levels[HTTP_TRACE_MAX_ITEM]; /**< Level at which to trace this item */
    MprOff          size;                        /**< Maximum size of content to trace */
    MprHash         *include;                    /**< Extensions to include in trace */
    MprHash         *exclude;                    /**< Extensions to exclude from trace */
} HttpTrace;

extern void httpManageTrace(HttpTrace *trace, int flags);

/**
    Callback to fill headers 
    @description If defined, the headers callback will run before the standard response headers are generated. This gives an 
    opportunity to pre-populate the response headers.
    @param arg Argument provided to httpSetHeadersCallback when the callback was established.
 */
typedef int (*HttpHeadersCallback)(void *arg);

/**
    Define a headers callback
    @description The headers callback will run before the standard response headers are generated. This gives an 
        opportunity to pre-populate the response headers.
    @param conn HttpConn object created via $httpCreateConn
    @param fn Callback function to invoke
    @param arg Argument to provide when invoking the headers callback
    @ingroup HttpConn
 */
extern void httpSetHeadersCallback(struct HttpConn *conn, HttpHeadersCallback fn, void *arg);

/**
    I/O callback for connections
    @param conn HttpConn object created via $httpCreateConn
    @param event Event object describing the I/O event
    @ingroup HttpConn
    @internal
  */
typedef void (*HttpIOCallback)(struct HttpConn *conn, MprEvent *event);

/**
    Define an I/O callback for connections
    @description The I/O callback is invoked when I/O events are detected on the connection. The default I/O callback
        is #httpEvent.
    @param conn HttpConn object created via $httpCreateConn
    @param fn Callback function to invoke
    @ingroup HttpConn
    @internal
  */
extern void httpSetIOCallback(struct HttpConn *conn, HttpIOCallback fn);

/** 
    Http Connections
    @description The HttpConn object represents a TCP/IP connection to the client. A connection object is created for
        each socket connection initiated by the client. One HttpConn object may service many Http requests due to 
        HTTP/1.1 keep-alive.
    @stability Evolving
    @defgroup HttpConn HttpConn
    @see HttpConn HttpEnvCallback HttpGetPassword HttpListenCallback HttpNotifier HttpQueue HttpRedirectCallback 
        HttpRx HttpStage HttpTx HtttpListenCallback httpCallEvent httpCloseConn 
        httpConnectorComplete httpConnTimeout httpConsumeLastRequest httpCreateConn httpCreateRxPipeline 
        httpCreateTxPipeline httpDestroyConn httpDestroyPipeline httpDiscardData httpDisconnect 
        httpEnableUpload httpError httpEvent httpGetAsync httpGetChunkSize httpGetConnContext httpGetConnHost 
        httpGetError httpGetExt httpGetKeepAliveCount httpMatchHost httpMemoryError httpPrepClientConn 
        httpPrepServerConn httpResetCredentials httpRouteRequest httpProcessHandler httpRunHandlerReady
        httpServiceQueues httpSetAsync httpSetChunkSize httpSetConnContext httpSetConnHost httpSetConnNotifier
        httpSetCredentials httpSetKeepAliveCount httpSetPipelineHandler httpSetProtocol httpSetRetries
        httpSetSendConnector httpSetState httpSetTimeout httpSetTimestamp httpShouldTrace httpStartPipeline
        httpNotifyWritable 
 */
typedef struct HttpConn {
    /*  Ordered for debugability */

    struct HttpRx *rx;                      /**< Rx object */
    struct HttpTx *tx;                      /**< Tx object */
    struct HttpEndpoint *endpoint;          /**< Endpoint object (if set - indicates server-side) */
    struct HttpHost *host;                  /**< Host object (if relevant) */

    int             state;                  /**< Connection state */
    int             error;                  /**< A request error has occurred */
    int             connError;              /**< A connection error has occurred */

    int             responded;              /**< The request has started to respond. Some output has been initiated. */
    int             finalized;              /**< End of response has been signified (set at handler level) */
    int             connectorComplete;      /**< Connector has finished sending the response */
    int             inHttpProcess;          /**< Rre-entrancy prevention for httpProcess() */
    int             refinalize;             /**< Finalize required once the Tx pipeline is created */

    HttpLimits      *limits;                /**< Service limits */
    Http            *http;                  /**< Http service object  */
    MprHash         *stages;                /**< Stages in pipeline */
    MprDispatcher   *dispatcher;            /**< Event dispatcher */
    MprDispatcher   *newDispatcher;         /**< New dispatcher if using a worker thread */
    MprDispatcher   *oldDispatcher;         /**< Original dispatcher if using a worker thread */
    HttpNotifier    notifier;               /**< Connection Http state change notification callback */
    MprWaitHandler  *waitHandler;           /**< I/O wait handler */
    MprSocket       *sock;                  /**< Underlying socket handle */

    struct HttpQueue *serviceq;             /**< List of queues that require service for request pipeline */
    struct HttpQueue *currentq;             /**< Current queue being serviced (just for GC) */

    HttpPacket      *input;                 /**< Header packet */
    HttpQueue       *readq;                 /**< End of the read pipeline */
    HttpQueue       *writeq;                /**< Start of the write pipeline */
    HttpQueue       *connectorq;            /**< Connector write queue */
    MprTime         started;                /**< When the connection started */
    MprTime         lastActivity;           /**< Last activity on the connection */
    MprEvent        *timeoutEvent;          /**< Connection or request timeout event */
    MprEvent        *workerEvent;           /**< Event for running connection via a worker thread */
    void            *context;               /**< Embedding context (EjsRequest) */
    void            *ejs;                   /**< Embedding VM */
    void            *pool;                  /**< Pool of VMs */
    void            *mark;                  /**< Reference for GC marking */
    void            *data;                  /**< Custom data for request */
    void            *grid;                  /**< Current request database grid for MVC apps */
    void            *record;                /**< Current request database record for MVC apps */
    char            *boundary;              /**< File upload boundary */
    char            *errorMsg;              /**< Error message for the last request (if any) */
    char            *ip;                    /**< Remote client IP address */
    char            *protocol;              /**< HTTP protocol */
    int             async;                  /**< Connection is in async mode (non-blocking) */
    int             canProceed;             /**< State machine should continue to process the request */
    int             followRedirects;        /**< Follow redirects for client requests */
    int             keepAliveCount;         /**< Count of remaining Keep-Alive requests for this connection */
    int             http10;                 /**< Using legacy HTTP/1.0 */

    int             port;                   /**< Remote port */
    int             retries;                /**< Client request retries */
    int             secure;                 /**< Using https */
    int             seqno;                  /**< Unique connection sequence number */
    int             writeBlocked;           /**< Transmission writing is blocked */
    int             worker;                 /**< Use worker */

    HttpTrace       trace[2];               /**< Tracing for [rx|tx] */

    /*  
        Authentication for client requests
     */
    char            *authCnonce;            /**< Digest authentication cnonce value */
    char            *authDomain;            /**< Authentication domain */
    char            *authNonce;             /**< Nonce value used in digest authentication */
    int             authNc;                 /**< Digest authentication nc value */
    char            *authOpaque;            /**< Opaque value used to calculate digest session */
    char            *authRealm;             /**< Authentication realm */
    char            *authQop;               /**< Digest authentication qop value */
    char            *authType;              /**< Basic or Digest */
    char            *authUser;              /**< User name credentials for authorized client requests */
    char            *authPassword;          /**< Password credentials for authorized client requests */
    int             sentCredentials;        /**< Sent authorization credentials */

    HttpIOCallback  ioCallback;             /**< I/O event callback */
    HttpHeadersCallback headersCallback;    /**< Callback to fill headers */
    void            *headersCallbackArg;    /**< Arg to fillHeaders */

#if BIT_DEBUG
    MprTime         startTime;              /**< Start time of request */
    uint64          startTicks;             /**< Start tick time of request */
#endif
} HttpConn;


/**
    Call httpEvent with the given event mask
    @param conn HttpConn object created via $httpCreateConn
    @param mask Mask of events. MPR_READABLE | MPR_WRITABLE
    @ingroup HttpConn
 */
extern void httpCallEvent(HttpConn *conn, int mask);

/** 
    Close a connection
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpCloseConn(HttpConn *conn);

/**
    Connector has completed sending the response.
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
    @internal
 */ 
extern void httpConnectorComplete(HttpConn *conn);

/**
    Signal a connection timeout on a connection
    @description This call cancels a connections current request, disconnects the socket and issues an error to the error 
        log. This call is normally invoked by the httpTimer which runs regularly to check for timed out requests.
        This call should not be made on another thread, but should be scheduled to run on the connection's dispatcher to
        avoid thread races.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
  */
extern void httpConnTimeout(HttpConn *conn);

/**
    Consume leftover data from the last request
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
    @internal
 */
extern void httpConsumeLastRequest(HttpConn *conn);

/** 
    Create a connection object.
    @description Most interactions with the Http library are via a connection object. It is used for server-side 
        communications when responding to client requests and it is used to initiate outbound client requests.
    @param http Http object created via #httpCreate
    @param endpoint Endpoint object owning the connection.
    @param dispatcher Disptacher to use for I/O events on the connection
    @returns A new connection object
    @ingroup HttpConn
*/
extern HttpConn *httpCreateConn(Http *http, struct HttpEndpoint *endpoint, MprDispatcher *dispatcher);

/**
    Create the receive request pipeline
    @param conn HttpConn object created via $httpCreateConn
    @param route Route object controlling how the pipeline is configured for the request
    @ingroup HttpConn
 */
extern void httpCreateRxPipeline(HttpConn *conn, struct HttpRoute *route);

/**
    Create the transmit request pipeline
    @param conn HttpConn object created via $httpCreateConn
    @param route Route object controlling how the pipeline is configured for the request
    @ingroup HttpConn
 */
extern void httpCreateTxPipeline(HttpConn *conn, struct HttpRoute *route);

/**
    Destroy the connection object
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpDestroyConn(HttpConn *conn);

/**
    Destroy the pipeline
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpDestroyPipeline(HttpConn *conn);

/**
    Discard buffered transmit pipeline data
    @param conn HttpConn object created via $httpCreateConn
    @param dir Queue direction. Either HTTP_QUEUE_TX or HTTP_QUEUE_RX.
    @ingroup HttpConn
 */
extern void httpDiscardData(HttpConn *conn, int dir);

/**
    Disconnect the connection's socket
    @description This call will close the socket and signal a connection error. Subsequent use of the connection socket
        will not be possible.  This call should not be made on another thread, but should be scheduled to run on the 
        connection's dispatcher to avoid thread races.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
  */
extern void httpDisconnect(HttpConn *conn);

/**
    Enable connection events
    @description Connection events are automatically disabled upon receipt of an I/O event on a connection. This 
        permits a connection to process the I/O without fear of interruption by another I/O event. At the completion
        of processing of the I/O request, the connection should be re-enabled via httpEnableConnEvents. This call is
        made for requests in #httpEvent.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpEnableConnEvents(HttpConn *conn);

/** 
    Enable Multipart-Mime File Upload for this request. This will define a "Content-Type: multipart/form-data..."
    header and will create a mime content boundary for use to delimit the various upload content files and fields.
    @param conn HttpConn connection object
    @ingroup HttpConn
 */
extern void httpEnableUpload(HttpConn *conn);

/** 
    Error handling for the connection.
    @description The httpError call is used to flag the current request as failed.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code. The status code can be ored with the flags HTTP_ABORT to immediately abort the connection
        or HTTP_CLOSE to close the connection at the completion of the request.
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @ingroup HttpConn
 */
extern void httpError(HttpConn *conn, int status, cchar *fmt, ...);

/**
    Http I/O event handler. Invoke when there is an I/O event on the connection. This is normally invoked automatically
    when I/O events are received.
    @param conn HttpConn object created via $httpCreateConn
    @param event Event structure
    @ingroup HttpConn
 */
extern void httpEvent(struct HttpConn *conn, MprEvent *event);

/**
    Get the async mode value for the connection
    @param conn HttpConn object created via $httpCreateConn
    @return True if the connection is in async mode
    @ingroup HttpConn
 */
extern int httpGetAsync(HttpConn *conn);

/**
    Get the preferred chunked size for transfer chunk encoding.
    @param conn HttpConn connection object created via $httpCreateConn
    @return Chunk size. Returns "zero" if not yet defined.
    @ingroup HttpConn
 */
extern ssize httpGetChunkSize(HttpConn *conn);

/**
    Get the connection context object
    @param conn HttpConn object created via $httpCreateConn
    @return The connection context object defined via httpSetConnContext
    @ingroup HttpConn
 */
extern void *httpGetConnContext(HttpConn *conn);

/**
    Get the connection host object
    @param conn HttpConn object created via $httpCreateConn
    @return The connection host object defined via httpSetConnHost
    @ingroup HttpConn
 */
extern void *httpGetConnHost(HttpConn *conn);

/** 
    Get the error message associated with the last request.
    @description Error messages may be generated for internal or client side errors.
    @param conn HttpConn connection object created via $httpCreateConn
    @return A error string. The caller must not free this reference.
    @ingroup HttpConn
 */
extern cchar *httpGetError(HttpConn *conn);

/**
    Get a URI extension 
    @description If the URI has no extension and the response content filename (HttpTx.filename) has been calculated,
        it will be tested for an extension.
    @param conn HttpConn connection object created via $httpCreateConn
    @return The URI extension sans "."
    @ingroup HttpConn
  */
extern char *httpGetExt(HttpConn *conn);

/** 
    Get the count of Keep-Alive requests that will be used for this connection object.
    @description Http Keep-Alive means that the TCP/IP connection is preserved accross multiple requests. This
        typically means much higher performance and better response. Http Keep-Alive is enabled by default 
        for Http/1.1 (the default). Disable Keep-Alive when talking to old, broken HTTP servers.
    @param conn HttpConn connection object created via $httpCreateConn
    @return The maximum count of Keep-Alive requests. 
    @ingroup HttpConn
 */
extern int httpGetKeepAliveCount(HttpConn *conn);

/**
    Match the HttpHost object that should serve this request
    @description This sets the conn->host field to the appropriate host. If no suitable host can be found, #httpError
        will be called and conn->error will be set
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
  */
extern void httpMatchHost(HttpConn *conn);

/**
    Signal a memory allocation error
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpMemoryError(HttpConn *conn);

/**
    Inform notifiers that the connection is now writable
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */ 
extern void httpNotifyWritable(HttpConn *conn);

/**
    Prepare a connection for a new request. This is used internally when using Keep-Alive.
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpPrepServerConn(HttpConn *conn);

/**
    Run the handler process callback.
    @description This optional handler callback is invoked when the service queue has room for more data.
    @param conn HttpConn object created via $httpCreateConn
    @return True if the handler output callback exists and can be invoked.
    @ingroup HttpConn
 */
extern int httpProcessHandler(HttpConn *conn);

/**
    Prepare a client connection for a new request. 
    @param conn HttpConn object created via $httpCreateConn
    @param keepHeaders If true, keep the headers already defined on the connection object
    @ingroup HttpConn
 */
extern void httpPrepClientConn(HttpConn *conn, bool keepHeaders);

/**
    Run the handler ready callback.
    @description This will be called when all incoming data for the request has been fully received.
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpReadyHandler(HttpConn *conn);

/** 
    Reset the current security credentials
    @description Remove any existing security credentials.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpResetCredentials(HttpConn *conn);

/**
    Route the request and select that matching route and handle to process the request.
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpConn
  */
extern void httpRouteRequest(HttpConn *conn);

/**
    Service pipeline queues to flow data.
    @param conn HttpConn object created via $httpCreateConn
 */
extern bool httpServiceQueues(HttpConn *conn);

/**
    Set the async mode value for the connection
    @param conn HttpConn object created via $httpCreateConn
    @param enable Set to 1 to enable async mode
    @return True if the connection is in async mode
    @ingroup HttpConn
 */
extern void httpSetAsync(HttpConn *conn, int enable);

/** 
    Set the chunk size for transfer chunked encoding. When set, a "Transfer-Encoding: Chunked" header will
    be added to the request, and all write data will be broken into chunks of the requested size.
    @param conn HttpConn connection object created via $httpCreateConn
    @param size Requested chunk size.
    @ingroup HttpConn
 */ 
extern void httpSetChunkSize(HttpConn *conn, ssize size);

/**
    Set the connection context object
    @param conn HttpConn object created via $httpCreateConn
    @param context New context object
    @ingroup HttpConn
 */
extern void httpSetConnContext(HttpConn *conn, void *context);

/**
    Set the connection host object
    @param conn HttpConn object created via $httpCreateConn
    @param host New context host
    @ingroup HttpConn
 */
extern void httpSetConnHost(HttpConn *conn, void *host);

/** 
    Define a notifier callback for this connection.
    @description The notifier callback will be invoked as Http requests are processed.
    @param conn HttpConn connection object created via $httpCreateConn
    @param fn Notifier function. 
    @ingroup HttpConn
 */
extern void httpSetConnNotifier(HttpConn *conn, HttpNotifier fn);

/** 
    Set the Http credentials
    @description Define a user and password to use with Http authentication for sites that require it. This will
        be used for the next client connection.
    @param conn HttpConn connection object created via $httpCreateConn
    @param user String user
    @param password Decrypted password string
    @ingroup HttpConn
 */
extern void httpSetCredentials(HttpConn *conn, cchar *user, cchar *password);

/** 
    Control Http Keep-Alive for the connection.
    @description Http Keep-Alive means that the TCP/IP connection is preserved accross multiple requests. This
        typically means much higher performance and better response. Http Keep-Alive is enabled by default 
        for Http/1.1 (the default). Disable Keep-Alive when talking to old, broken HTTP servers.
    @param conn HttpConn connection object created via $httpCreateConn
    @param count Count of Keep-Alive transactions to use before closing the connection. Set to zero to disable keep-alive.
    @ingroup HttpConn
 */
extern void httpSetKeepAliveCount(HttpConn *conn, int count);

/**
    Set the handler to process a client request
    @description This overrides the normal handler selection with the given handler. This can only be called before
        the request pipeline has actually started and should only be done in the "match" or "open" stage functions.
    @param conn HttpConn connection object created via $httpCreateConn
    @param handler Stage handler to process the request
    @ingroup HttpConn
 */
extern void httpSetPipelineHandler(HttpConn *conn, HttpStage *handler);

/** 
    Set the Http protocol variant for this connection
    @description Set the Http protocol variant to use. 
    @param conn HttpConn connection object created via $httpCreateConn
    @param protocol  String representing the protocol variant. Valid values are: "HTTP/1.0", "HTTP/1.1". This parameter
        must be persistent.
    Use HTTP/1.1 wherever possible.
    @ingroup HttpConn
 */
extern void httpSetProtocol(HttpConn *conn, cchar *protocol);

/** 
    Set the Http retry count
    @description Define the number of retries before failing a request. It is normative for network errors
        to require that requests be sometimes retried. The default retries is set to (2).
    @param conn HttpConn object created via $httpCreateConn
    @param retries Count of retries
    @ingroup HttpConn
 */
extern void httpSetRetries(HttpConn *conn, int retries);

/**
    Set the "Send" connector to process the request
    @description If the net connection has been selected, but the response content is a file, the pipeline connector
    can be upgraded to use the "Send" connector.
    @param conn HttpConn connection object created via $httpCreateConn
    @param path File name to send as a response
    @ingroup HttpConn
 */
extern void httpSetSendConnector(HttpConn *conn, cchar *path);

/**
    Set the connection state and invoke notifiers.
    @param conn HttpConn object created via $httpCreateConn
    @param state New state to enter
    @ingroup HttpConn
 */
extern void httpSetState(HttpConn *conn, int state);

/** 
    Set the Http inactivity timeout
    @description Define an inactivity timeout after which the Http connection will be closed. 
    @param conn HttpConn object created via $httpCreateConn
    @param requestTimeout Request timeout in msec. This is the total time for the request. Set to -1 to preserve the
        existing value.
    @param inactivityTimeout Inactivity timeout in msec. This is maximum connection idle time. Set to -1 to preserve the
        existing value.
    @ingroup HttpConn
 */
extern void httpSetTimeout(HttpConn *conn, int requestTimeout, int inactivityTimeout);

/**
    Define a timestamp in the MPR log file.
    @description This routine initiates the writing of a timestamp in the MPR log file
    @param period Time in milliseconds between timestamps
    @ingroup HttpConn
 */
extern void httpSetTimestamp(MprTime period);

/**
    Test if the item should be traced
    @param conn HttpConn connection object created via $httpCreateConn
    @param dir Direction of data flow. Set to HTTP_TRACE_RX or HTTP_TRACE_TX
    @param item Item to trace. Set to HTTP_TRACE_CONN, HTTP_TRACE_FIRST, HTTP_TRACE_HEADER, HTTP_TRACE_BODY, or 
        HTTP_TRACE_TIME
    @param ext URI resource extension (without ".").
    @return The level at which tracing should be done. Returns -1 if tracing should not be done for this item.
    @ingroup HttpConn
  */
extern int httpShouldTrace(HttpConn *conn, int dir, int item, cchar *ext);

/**
    Start the pipeline. This starts the request handler.
    @param conn HttpConn object created via $httpCreateConn
    @ingroup HttpConn
 */
extern void httpStartPipeline(HttpConn *conn);

/**
    Steal a connection from Appweb
    @description Steal a connection from Appweb and assume total responsibility for the connection.
    This removes the connection from active management by Appweb. After calling, request and inactivity
    timeouts will not be enforced. It is the callers responsibility to call mprCloseSocket.
    @param conn HttpConn object created via $httpCreateConn
    @return The connection socket object.
 */
extern MprSocket *httpStealConn(HttpConn *conn);

/** Internal APIs */
extern struct HttpConn *httpAccept(struct HttpEndpoint *endpoint);
extern void httpEnableConnEvents(HttpConn *conn);
extern void httpInitTrace(HttpTrace *trace);
extern void httpParseMethod(HttpConn *conn);
extern HttpLimits *httpSetUniqueConnLimits(HttpConn *conn);
extern int httpShouldTrace(HttpConn *conn, int dir, int item, cchar *ext);
extern void httpTraceContent(HttpConn *conn, int dir, int item, HttpPacket *packet, ssize len, MprOff total);
extern void httpUsePrimary(HttpConn *conn);
extern void httpUseWorker(HttpConn *conn, MprDispatcher *dispatcher, MprEvent *event);

/********************************** HttpAuth *********************************/
/*  
    Deny/Allow order
 */
#define HTTP_ALLOW_DENY             1           /**< Run allow checks before deny checks */
#define HTTP_DENY_ALLOW             2           /**< Run deny checks before allow checks */
#define HTTP_ACL_ALL               -1           /* All bits set */

/*  
    Authentication types
 */
#define HTTP_AUTH_UNKNOWN           0           /**< Authentication method is unknown */
#define HTTP_AUTH_BASIC             1           /**< Basic HTTP authentication (clear text) */
#define HTTP_AUTH_DIGEST            2           /**< Digest authentication */

/*  
    Auth Flags
 */
#define HTTP_AUTH_REQUIRED          0x1         /**< Route requires authentication */

/*  
    Authentication methods
 */
#define HTTP_AUTH_FILE              1           /**< File-based authentication */
#define HTTP_AUTH_PAM               2           /**< Plugable authentication module scheme (Unix) */

typedef long HttpAcl;                           /**< Authentication Access control mask */

/** 
    Authorization
    @description HttpAuth is the foundation authorization object and is used by HttpRoute.
    It stores the authorization configuration information required to determine if a client request should be permitted 
    access to a given resource.
    @stability Evolving
    @defgroup HttpAuth HttpAuth
    @see HttpAuth HttpGetPassword HttpGroup HttpUser HttpValidateCred httpAddGroup httpAddUser httpAddUserToGroup 
        httpAddUsersToGroup httpCreateAuth httpCreateGroup httpCreateUser httpDisableGroup httpDisableUser 
        httpEnableGroup httpEnableUser httpGetFilePassword httpGetGroupAcl httpGetPamPassword httpIsGroupEnabled 
        httpIsUserEnabled httpParseAcl httpReadGroupFile httpReadUserFile httpRemoveGroup httpRemoveUser 
        httpRemoveUserFromGroup httpRemoveUsersFromGroup httpSetAuthAnyValidUser httpSetAuthDeny httpSetAuthOrder 
        httpSetAuthQop httpSetAuthRealm httpSetAuthRequiredGroups httpSetAuthRequiredUsers httpSetAuthUser 
        httpSetGroupAcl httpSetRequiredAcl httpUpdateUserAcls httpValidateFileCredentials httpValidatePamCredentials 
        httpWriteGroupFile httpWriteUserFile 
 */
typedef struct HttpAuth {
    bool            anyValidUser;           /**< If any valid user will do */
    int             type;                   /**< Kind of authorization */

    MprHash         *allow;                 /**< Clients to allow */
    MprHash         *deny;                  /**< Clients to deny */
    char            *requiredRealm;         /**< Realm to use for access */
    char            *requiredGroups;        /**< Authorization group for access */
    char            *requiredUsers;         /**< User name for access */
    HttpAcl         requiredAcl;            /**< ACL for access */

    int             backend;                /**< Authorization method (PAM | FILE) */
    int             flags;                  /**< Authorization flags */
    int             order;                  /**< Order deny/allow, allow/deny */
    char            *qop;                   /**< Digest Qop */

    /*  
        State for file-based authorization
     */
    char            *userFile;              /**< User name authorization file */
    char            *groupFile;             /**< Group authorization file  */
    MprHash         *users;                 /**< Hash of user file  */
    MprHash         *groups;                /**< Hash of group file  */
} HttpAuth;


/**
    Create an authentication object
    @return An empty authentiction object
    @ingroup HttpAuth
    @internal 
 */
extern HttpAuth *httpCreateAuth();

/**
    Allow access by a client
    @param auth Authorization object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param allow Client to allow access. This must be an IP address string.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthAllow(HttpAuth *auth, cchar *allow);

/**
    Allow access by any valid user
    @description This configures the basic or digest authentication for the authorization object
    @param auth Authorization object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthAnyValidUser(HttpAuth *auth);

/**
    Deny access by a client
    @param auth Authorization object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param deny Client to deny access. This must be an IP address string.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthDeny(HttpAuth *auth, cchar *deny);

/**
    Set the auth allow/deny order
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param order Set to HTTP_ALLOW_DENY to run allow checks before deny checks. Set to HTTP_DENY_ALLOW to run deny
        checks before allow.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthOrder(HttpAuth *auth, int order);

/**
    Set the required quality of service for digest authentication
    @description This configures the basic or digest authentication for the auth object
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param qop Quality of service description.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthQop(HttpAuth *auth, cchar *qop);

/**
    Set the required realm for basic or digest authentication
    @description This configures the basic or digest authentication for the auth object
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthRealm(HttpAuth *auth, cchar *realm);

/**
    Define the set of required groups for basic or digest authentication
    @description This configures the basic or digest authentication for the auth object
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param groups Comma or space separated list of acceptable groups.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthRequiredGroups(HttpAuth *auth, cchar *groups);

/**
    Define the set of required users for basic or digest authentication
    @description This configures the basic or digest authentication for the auth object
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param users Comma or space separated list of acceptable users.
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthRequiredUsers(HttpAuth *auth, cchar *users);

/**
    Set the required user for basic or digest authentication
    @description This call is made by the standard authentication when basic or digest authentication is employed.
    @param conn HttpConn connection object created via $httpCreateConn
        auth object.
    @param user User name to set 
    @ingroup HttpAuth
    @internal
 */
extern void httpSetAuthUser(HttpConn *conn, cchar *user);

/*
    Internal
 */
extern void httpInitAuth(Http *http);
extern int httpCheckAuth(HttpConn *conn);
extern HttpAuth *httpCreateInheritedAuth(HttpAuth *parent);


/** 
    User Authorization
    File-based authorization backend
    @stability Evolving
    @ingroup HttpAuth
    @see HttpAuth
 */
typedef struct HttpUser {
    bool            enabled;                /**< User is enabled */
    HttpAcl         acl;                    /**< Access control list. Union (or) of all group Acls */
    char            *password;              /**< User password */
    char            *realm;                 /**< Authentication realm */
    char            *name;                  /**< User name */
} HttpUser;

/** 
    Group Authorization
    @stability Evolving
    @ingroup HttpAuth
    @see HttpAuth
 */
typedef struct  HttpGroup {
    bool            enabled;                /**< Group is enabled */
    HttpAcl         acl;                    /**< Group access control list */
    char            *name;                  /**< Group name */
    MprList         *users;                 /**< List of users */
} HttpGroup;

/**
    Add a group
    @description This creates the group and adds it to the authentication database.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to add
    @param acl Group access control list mask
    @param enabled Set to true to enable the group
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpAddGroup(HttpAuth *auth, cchar *group, HttpAcl acl, bool enabled);

/**
    Add a user
    @description This creates the user and adds the user to the authentication database.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm
    @param user User name to add
    @param password User password. The password should not be encrypted. The backend will encrypt as required.
    @param enabled Set to true to enable the user
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpAddUser(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, bool enabled);

/**
    Add a user to a group
    @description This creates the user and adds the user to the authentication database.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param gp Group object
    @param user User name string
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpAddUserToGroup(HttpAuth *auth, HttpGroup *gp, cchar *user);

/**
    Add users to a group
    @description This creates the users and adds them to the authentication database.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to add
    @param users Space or comma separated list of user names
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpAddUsersToGroup(HttpAuth *auth, cchar *group, cchar *users);

/**
    Create a new group
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param name Group name to add
    @param acl Group access control list mask
    @param enabled Set to true to enable the group
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern HttpGroup *httpCreateGroup(HttpAuth *auth, cchar *name, HttpAcl acl, bool enabled);

/**
    Create a new user
    @description The user is not added to the authentication database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param name User name 
    @param password User password. The password should not be encrypted. The backend will encrypt as required.
    @param realm Authentication realm for this user
    @param enabled Set to true to enable the user
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern HttpUser *httpCreateUser(HttpAuth *auth, cchar *name, cchar *password, cchar *realm, bool enabled);

/**
    Disable a group
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to disable
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpDisableGroup(HttpAuth *auth, cchar *group);

/**
    Disable a user
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm for user
    @param user User name to disable
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
 */
extern int httpDisableUser(HttpAuth *auth, cchar *realm, cchar *user);

/**
    Enable a group
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group User name to enable
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpEnableGroup(HttpAuth *auth, cchar *group);

/**
    Enable a user
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param user User name to enable
    @param realm Authentication realm for user
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal
 */
extern int httpEnableUser(HttpAuth *auth, cchar *realm, cchar *user);

/**
    Get the group ACL
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to examine
    @return The group access control list. If the group does not exist, an empty ACL is returned.
    @ingroup HttpAuth
    @internal
 */
extern HttpAcl httpGetGroupAcl(HttpAuth *auth, char *group);

/**
    Get the password for a user from a file-based authentication database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm for user
    @param user User name to examine
    @return Password string to verify
    @ingroup HttpAuth
    @internal
 */
extern cchar *httpGetFilePassword(HttpAuth *auth, cchar *realm, cchar *user);

/**
    Test if an authentication group is enabled.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to examine
    @return True if the group is enabled
    @ingroup HttpAuth
    @internal
 */
extern bool httpIsGroupEnabled(HttpAuth *auth, cchar *group);

/**
    Test if a user is enabled.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param user User name to examine
    @param realm Authentication realm for user
    @return True if the group is enabled
    @ingroup HttpAuth
    @internal
 */
extern bool httpIsUserEnabled(HttpAuth *auth, cchar *realm, cchar *user);

/**
    Parse an access control list
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param acl Access control list string
    @return Access control list mask
    @ingroup HttpAuth
    @internal 
 */
extern HttpAcl httpParseAcl(HttpAuth *auth, cchar *acl);

/**
    Remove a group
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to remove
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpRemoveGroup(HttpAuth *auth, cchar *group);

/**
    Read an authentication group file
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param path Path name of the group file
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpReadGroupFile(HttpAuth *auth, char *path);

/**
    Remove an authentication user file
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param path Path name of user file
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpReadUserFile(HttpAuth *auth, char *path);

/**
    Remove a user
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm of user to remove
    @param user User name to remove
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpRemoveUser(HttpAuth *auth, cchar *realm, cchar *user);

/**
    Remove user from a group
    @param gp Group object
    @param user User name to remove from the group
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpRemoveUserFromGroup(HttpGroup *gp, cchar *user);

/**
    Remove users from a group
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to modify
    @param users Comma or space separate list of users to remove
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpRemoveUsersFromGroup(HttpAuth *auth, cchar *group, cchar *users);

/**
    Set the group access control list
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param group Group name to remove
    @param acl Group access control list mask
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpSetGroupAcl(HttpAuth *auth, cchar *group, HttpAcl acl);

/**
    Set the required access control list for the authentication object
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param acl New access control list
    @ingroup HttpAuth
    @internal 
 */
extern void httpSetRequiredAcl(HttpAuth *auth, HttpAcl acl);

/**
    Update the user access control list
    @description After modifying the required ACL for the auth object, the ACLs for each user must be updated.
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern void httpUpdateUserAcls(HttpAuth *auth);

/**
    Write out the group authentication database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param path Path name for the group file
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpWriteGroupFile(HttpAuth *auth, char *path);

/**
    Write out the user authentication database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param path Path name for the user file
    @return "Zero" if successful, otherwise a negative MPR error code
    @ingroup HttpAuth
    @internal 
 */
extern int httpWriteUserFile(HttpAuth *auth, char *path);

/**
    Validate credentials using a file-based authentication database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm
    @param user User name
    @param password User password
    @param requiredPass Required user password
    @param msg Output parameter, error message.
    @return True if the user could be successfully validated
    @ingroup HttpAuth
    @internal 
 */
extern bool httpValidateFileCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
    cchar *requiredPass, char **msg);

/**
    Get the user password from a PAM database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm
    @param user User name to examine
    @return The user password. This will be in encrypted form.
    @ingroup HttpAuth
    @internal 
 */
extern cchar *httpGetPamPassword(HttpAuth *auth, cchar *realm, cchar *user);

/**
    Validate credentials using a PAM based authentication database
    @param auth Auth object allocated by #httpCreateAuth. Authenticated routes typically store the reference to an
        auth object.
    @param realm Authentication realm
    @param user User name
    @param password User password
    @param requriedPass Required user password
    @param msg Output parameter, error message.
    @return True if the user could be successfully validated
    @ingroup HttpAuth
    @internal 
 */
extern bool httpValidatePamCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
    cchar *requiredPass, char **msg);

/********************************** HttpLang  ********************************/

#define HTTP_LANG_BEFORE        0x1         /**< Insert suffix before extension */
#define HTTP_LANG_AFTER         0x2         /**< Insert suffix after extension */

/**
    Language definition record for routes
    @ingroup HttpRoute
  */
typedef struct HttpLang {
    char        *path;                      /**< Document directory for the language */
    char        *suffix;                    /**< Suffix to add to filenames */
    int         flags;                      /**< Control suffix position */
} HttpLang;

/********************************** HttpCache  *********************************/

#define HTTP_CACHE_CLIENT           0x1     /**< Cache on the client side */
#define HTTP_CACHE_SERVER           0x2     /**< Cache on the server side */
#define HTTP_CACHE_MANUAL           0x4     /**< Cache manually. User must call httpWriteCache */
#define HTTP_CACHE_RESET            0x8     /**< Don't inherit cache config from outer routes */
#define HTTP_CACHE_ALL              0x10    /**< Cache the same pathInfo together regardless of the request params */
#define HTTP_CACHE_ONLY             0x20    /**< Cache exactly the specified URI with params */
#define HTTP_CACHE_UNIQUE           0x40    /**< Uniquely cache request with different params */

/**
    Cache Control
    @stability Evolving
    @defgroup HttpCache HttpCache
    @see HttpCache httpAddCache httpUpdateCache httpWriteCache
*/
typedef struct HttpCache {
    MprHash     *extensions;                /**< Extensions to cache */
    MprHash     *methods;                   /**< Methods to cache */
    MprHash     *types;                     /**< MimeTypes to cache */
    MprHash     *uris;                      /**< URIs to cache */
    MprTime     clientLifespan;             /**< Lifespan for client cached content */
    MprTime     serverLifespan;             /**< Lifespan for server cached content */
    int         flags;                      /**< Cache control flags */
} HttpCache;

/**
    Add caching for response content
    @description This call configures caching for request responses. Caching may be used for any HTTP method, 
    though typically it is most useful for state-less GET requests. Output data may be uniquely cached for requests 
    with different request parameters (query, post, and route parameters).
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
    "max-age" set to the lifespan argument value (converted to seconds). This causes the client to serve client-cached 
    content and to not contact the server at all until the max-age expires. 
    Alternatively, you can use $httpSetHeader to explicitly set a "Cache-Control header. For your reference, here are 
    some keywords that can be used in the Cache-Control Http header.
    \n\n
        "max-age" Maximum time in seconds the resource is considered fresh.
        "s-maxage" Maximum time in seconds the resource is considered fresh from a shared cache.
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
    @param methods List of methods for which caching should be enabled. Set to a comma or space separated list
        of method names. Method names can be any case. Set to null or "*" for all methods. Example:
        "GET, POST".
    @param uris Set of URIs to cache. 
        If the URI is set to "*" all URIs for that action are uniquely cached. If the request has POST data, 
        the URI may include such post data in a sorted query format. E.g. {uri: /buy?item=scarf&quantity=1}.
    @param extensions List of document extensions for which caching should be enabled. Set to a comma or space 
        separated list of extensions. Extensions should not have a period prefix. Set to null or "*" for all extensions.
        Example: "html, css, js".
    @param types List of document mime types for which caching should be enabled. Set to a comma or space 
        separated list of types. The mime types are those that correspond to the document extension and NOT the
        content type defined by the handler serving the document. Set to null or "*" for all types.
        Example: "image/gif, application/x-php".
    @param clientLifespan Lifespan of client cache items in milliseconds. If not set to positive integer, the lifespan will
        default to the route lifespan.
    @param serverLifespan Lifespan of server cache items in milliseconds. If not set to positive integer, the lifespan will
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
        Select HTTP_CACHE_ALL, HTTP_CACHE_ONLY or HTTP_CACHE_UNIQUE to define the server-side caching mode. Only
        one of these three mode flags should be specified.
        \n\n
        If the HTTP_CACHE_ALL flag is set, the request params (query, post data, and route parameters) will be
        ignored and all requests for a given URI path will cache to the same cache record.
        \n\n
        Select HTTP_CACHE_UNIQUE to uniquely cache requests with different request parameters. The URIs specified in 
        $uris should not contain any request parameters.
        \n\n
        Select HTTP_CACHE_ONLY to cache only the exact URI with parameters specified in $uris. The parameters must be 
        in sorted www-urlencoded format. For example: /example.esp?hobby=sailing&name=john.
    @return A count of the bytes actually written
    @ingroup HttpCache
 */
extern void httpAddCache(struct HttpRoute *route, cchar *methods, cchar *uris, cchar *extensions, cchar *types, 
        MprTime clientLifespan, MprTime serverLifespan, int flags);

/**
    Update the cached content for a URI
    @param conn HttpConn connection object 
    @param uri The request URI for which to update the cache. If using the HTTP_CACHE_ONLY flag when configuring 
        the cached item, then the URI should contain the request parameters in sorted www-urlencoded format.
    @param data Data to cache for the URI. If you wish to cache response headers, include those at the start of the
    data followed by an additional new line. For example: "Content-Type: text/plain\n\nHello World\n".
    @param lifespan Lifespan in milliseconds for the cached content
    @ingroup HttpCache
  */
extern ssize httpUpdateCache(HttpConn *conn, cchar *uri, cchar *data, MprTime lifespan);

/**
    Write the cached content for a URI to the client
    @description This call explicitly writes cached content to the client. It is useful when the caching is 
        configured in manual mode via the HTTP_CACHE_MANUAL flag to $httpAddCache.
    @param conn HttpConn connection object 
    @ingroup HttpCache
  */
extern ssize httpWriteCached(HttpConn *conn);

/********************************** HttpRoute  *********************************/
/*
    Misc route API flags
 */
#define HTTP_ROUTE_NOT            0x1       /**< Negate the route pattern test result */
#define HTTP_ROUTE_FREE           0x2       /**< Free Route.mdata back to malloc when route is freed */
#define HTTP_ROUTE_FREE_PATTERN   0x4       /**< Free Route.patternCompiled back to malloc when route is freed */
#define HTTP_ROUTE_RAW            0x8       /**< Don't html encode the write data */
#define HTTP_ROUTE_PUT_DELETE     0x1000    /**< Support PUT|DELETE */
#define HTTP_ROUTE_GZIP           0x2000    /**< Support gzipped content */
#define HTTP_ROUTE_STARTED        0x4000    /**< Route initialized */

/**
    Route Control
    @stability Evolving
    @defgroup HttpRoute HttpRoute
    @see HttpRoute httpAddRouteCondition httpAddRouteErrorDocument httpAddRouteExpiry httpAddRouteExpiryByType 
        httpAddRouteFilter httpAddRouteHandler httpAddRouteHeader httpAddRouteLanguageDir httpAddRouteLanguageSuffix 
        httpAddRouteLoad httpAddRouteQuery httpAddRouteUpdate httpClearRouteStages httpCreateAliasRoute 
        httpCreateDefaultRoute httpCreateInheritedRoute httpCreateRoute httpDefineRoute
        httpDefineRouteCondition httpDefineRouteTarget httpDefineRouteUpdate httpFinalizeRoute httpGetRouteData 
        httpGetRouteDir httpLink httpLookupRouteErrorDocument httpMakePath httpMatchRoute httpResetRoutePipeline 
        httpSetRouteAuth httpSetRouteAutoDelete httpSetRouteCompression httpSetRouteConnector httpSetRouteData 
        httpSetRouteDefaultLanguage httpSetRouteDir httpSetRouteFlags httpSetRouteHandler httpSetRouteHost 
        httpSetRouteIndex httpSetRouteMethods httpSetRouteName httpSetRoutePathVar httpSetRoutePattern 
        httpSetRoutePrefix httpSetRouteScript httpSetRouteSource httpSetRouteTarget httpSetRouteWorkers httpTemplate 
        httpSetTrace httpSetTraceFilter httpTokenize httpTokenizev 
 */
typedef struct HttpRoute {
    /* Ordered for debugging */
    char            *name;                  /**< Route name */
    char            *pattern;               /**< Original matching URI pattern for the route (includes prefix) */
    char            *startSegment;          /**< Starting literal segment of pattern (includes prefix) */
    char            *startWith;             /**< Starting literal segment of pattern (includes prefix) */
    char            *optimizedPattern;      /**< Processed pattern (excludes prefix) */
    char            *prefix;                /**< Application scriptName prefix */
    char            *tplate;                /**< URI template for forming links based on this route (includes prefix) */
    char            *targetRule;            /**< Target rule */
    char            *target;                /**< Route target details */
    char            *dir;                   /**< Directory filename (DocumentRoot) */
    MprList         *indicies;              /**< Directory index documents */
    char            *methodSpec;            /**< Supported HTTP methods */
    HttpStage       *handler;               /**< Fixed handler */

    int             nextGroup;              /**< Next route with a different startWith */
    int             responseStatus;         /**< Response status code */
    ssize           prefixLen;              /**< Prefix length */
    ssize           startWithLen;           /**< Length of startWith */
    ssize           startSegmentLen;        /**< Prefix length */

    MprList         *caching;               /**< Items to cache */
    MprTime         lifespan;               /**< Default lifespan for all cache items in route */
    HttpAuth        *auth;                  /**< Per route block authentication */
    Http            *http;                  /**< Http service object (copy of appweb->http) */
    struct HttpHost *host;                  /**< Owning host */
    struct HttpRoute *parent;               /**< Parent route */
    int             flags;                  /**< Route flags */

    char            *defaultLanguage;       /**< Default language */
    MprHash         *extensions;            /**< Hash of handlers by extensions */
    MprList         *handlers;              /**< List of handlers for this route */
    MprList         *handlersWithMatch;     /**< List of handlers with match routines */
    HttpStage       *connector;             /**< Network connector to use */
    MprHash         *data;                  /**< Hash of extra data configuration */
    MprHash         *pathTokens;            /**< Path $token refrerences */
    MprHash         *languages;             /**< Languages supported */
    MprList         *inputStages;           /**< Input stages */
    MprList         *outputStages;          /**< Output stages */
    MprHash         *errorDocuments;        /**< Set of error documents to use on errors */
    void            *context;               /**< Hosting context (Appweb == EjsPool) */
    void            *eroute;                /**< Extended route information for handler (only) */
    char            *uploadDir;             /**< Upload directory */
    int             autoDelete;             /**< Automatically delete uploaded files */

    MprFile         *log;                   /**< File object for access logging */
    char            *logFormat;             /**< Access log format */
    char            *logPath;               /**< Access log filename */
    int             logFlags;               /**< Log control flags (append|anew) */
    int             logBackup;              /**< Number of log backups */
    ssize           logSize;                /**< Max log size */
    HttpLimits      *limits;                /**< Host resource limits */
    MprHash         *mimeTypes;             /**< Hash table of mime types (key is extension) */

    HttpTrace       trace[2];               /**< Default route request tracing */
    int             traceMask;              /**< Request/response trace mask */

    /*
        Used by Ejscript
     */
    char            *script;                /**< Startup script for handlers serving this route */
    char            *scriptPath;            /**< Startup script path for handlers serving this route */
    int             workers;                /**< Number of workers to use for this route */

    MprHash         *methods;               /**< Matching HTTP methods */
    MprList         *params;                /**< Matching param field data */
    MprList         *headers;               /**< Matching header values */
    MprList         *conditions;            /**< Route conditions */
    MprList         *updates;               /**< Route and request updates */

    void            *patternCompiled;       /**< Compiled pattern regular expression (not alloced) */
    char            *sourceName;            /**< Source name for route target */
    char            *sourcePath;            /**< Source path for route target */
    MprList         *tokens;                /**< Tokens in pattern, {name} */

    struct MprSsl   *ssl;                   /**< SSL configuration */
    MprMutex        *mutex;                 /**< Multithread sync */
} HttpRoute;


/**
    Route operation record
 */
typedef struct HttpRouteOp {
    char            *name;                  /**< Name of route operation */
    char            *details;               /**< General route operation details */
    char            *var;                   /**< Var to set */
    char            *value;                 /**< Value to assign to var */
    void            *mdata;                 /**< pcre_ data managed by malloc() */
    int             flags;                  /**< Route flags to control freeing mdata */
} HttpRouteOp;

/*
    Route matching return codes
 */
#define HTTP_ROUTE_OK       0             /**< The route matches the request */
#define HTTP_ROUTE_REJECT   1             /**< The route does not match the request */
#define HTTP_ROUTE_REROUTE  2             /**< Request has been modified and must be re-routed */

/**
    General route procedure. Used by targets, conditions and updates.
 */
typedef int (HttpRouteProc)(HttpConn *conn, HttpRoute *route, HttpRouteOp *item);

/**
    Add routes for a resource
    @description This routing adds a set of RESTful routes for a resource. It will add the following routes:
    <table>
        <tr><td>Name</td><td>Method</td><td>Pattern</td><td>Action</td></tr>
        <tr><td>init</td><td>GET</td><td>/NAME/init$</td><td>init</td></tr>
        <tr><td>create</td><td>POST</td><td>/NAME(/)*$</td><td>create</td></tr>
        <tr><td>edit</td><td>GET</td><td>/NAME/edit$</td><td>edit</td></tr>
        <tr><td>show</td><td>GET</td><td>/NAME$</td><td>show</td></tr>
        <tr><td>update</td><td>PUT</td><td>/NAME$</td><td>update</td></tr>
        <tr><td>destroy</td><td>DELETE</td><td>/NAME$</td><td>destroy</td></tr>
        <tr><td>default</td><td>*</td><td>/NAME/{action}$</td><td>cmd-${action}</td></tr>
    </tr>
    </table>
    @param parent Parent route from which to inherit configuration.
    @param resource Resource name. This should be a lower case, single word, alphabetic resource name.
    @ingroup HttpRoute
 */
extern void httpAddResource(HttpRoute *parent, cchar *resource);

/**
    Add routes for a group of resources
    @description This routing adds a set of RESTful routes for a resource group. It will add the following routes:
    <table>
        <tr><td>Name</td><td>Method</td><td>Pattern</td><td>Action</td></tr>
        <tr><td>list</td><td>GET</td><td>/NAME(/)*$</td><td>list</td></tr>
        <tr><td>init</td><td>GET</td><td>/NAME/init$</td><td>init</td></tr>
        <tr><td>create</td><td>POST</td><td>/NAME(/)*$</td><td>create</td></tr>
        <tr><td>edit</td><td>GET</td><td>/NAME/{id=[0-9]+}/edit$</td><td>edit</td></tr>
        <tr><td>show</td><td>GET</td><td>/NAME/{id=[0-9]+}$</td><td>show</td></tr>
        <tr><td>update</td><td>PUT</td><td>/NAME/{id=[0-9]+}$</td><td>update</td></tr>
        <tr><td>destroy</td><td>DELETE</td><td>/NAME/{id=[0-9]+}$</td><td>destroy</td></tr>
        <tr><td>custom</td><td>POST</td><td>/NAME/{action}/{id=[0-9]+}$</td><td>${action}</td></tr>
        <tr><td>default</td><td>*</td><td>/NAME/{action}$</td><td>cmd-${action}</td></tr>
    </tr>
    </table>
    @param parent Parent route from which to inherit configuration.
    @param resource Resource name. This should be a lower case, single word, alphabetic resource name.
    @ingroup HttpRoute
 */
extern void httpAddResourceGroup(HttpRoute *parent, cchar *resource);

/**
    Add a route for the home page.
    @description This will add a home page to route ESP applications. This will add the following route:
    <table>
        <tr><td>Name</td><td>Method</td><td>Pattern</td><td>Target</td></tr>
        <tr><td>home</td><td>GET,POST,PUT</td><td>^/$</td><td>index.esp</td></tr>
    </table>
    @param parent Parent route from which to inherit configuration.
    @ingroup HttpRoute
 */
extern void httpAddHomeRoute(HttpRoute *parent);

/**
    Add a route set package
    @description This will add a set of routes suitable for some application paradigms.
    <table>
        <tr><td>Name</td><td>Method</td><td>Pattern</td><td>Target</td></tr>
        <tr><td>home</td><td>GET,POST,PUT</td><td>^/$</td><td>index.esp</td></tr>
        <tr><td>static</td><td>GET</td><td>^/static(/(.)*$</td><td>$1</td></tr>
    </table>
    @param parent Parent route from which to inherit configuration.
    @param set Route set to select. Use "simple", "mvc", "restful" or "none". 
        \n\n
        The "simple" pack will invoke 
        #httpAddHomeRoute and #httpAddStaticRoute to add "home", and "static" routes. 
        \n\n
        The "mvc" selection will add the default routes and then add the route:
        <table>
            <tr><td>Name</td><td>Method</td><td>Pattern</td><td>Target</td></tr>
            <tr><td>default</td><td>*</td><td>^/{controller}(~/{action}~)$</td><td>${controller}-${action}</td></tr>
        </table>
        \n\n
    @ingroup HttpRoute
 */
extern void httpAddRouteSet(HttpRoute *parent, cchar *set);

/**
    Add a route condition
    @description A route condition is run after matching the route pattern. For a route to be accepted, all conditions
        must match. Route conditions are built-in rules that can be applied to routes.
    @param route Route to modify
    @param name Condition rule to add. Supported conditions are: "auth", "missing", "directory", "exists", and "match".
        The "auth" rule is used internally to implement basic and digest authentication. 
        \n\n
        The "missing" rule tests if the target filename is missing. The "missing" rule takes no arguments. 
        \n\n
        The "directory" rule tests if the condition argument is a directory. The form of the "directory" rule is: 
            "directory pathString". For example: "directory /stuff/${request:pathInfo}.txt"
        \n\n
        The "exists" rule tests if the condition argument is present in the file system. The form of the "exists" rule is:
            "exists pathString". For example: "exists ${request.filename}.gz", 
        \n\n
        The match directory tests a regular expression pattern against the rest of the condition arguments. The form of 
        the match rule is: "match RegExp string". For example: "match https ${request.scheme}".
    @param details Condition parameters. 
        See #httpSetRouteTarget for a list of the token values that can be included in the condition rule details.
    @param flags Set to HTTP_ROUTE_NOT to negate the condition test
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpAddRouteCondition(HttpRoute *route, cchar *name, cchar *details, int flags);

/**
    Add an error document
    @description This defines an error document to be used when the requested document cannot be found. 
        This definition is used by some handlers for error processing.
    @param route Route to modify
    @param status The HTTP status code to use with the error document.
    @param uri URL describing the error document
    @ingroup HttpRoute
 */
extern void httpAddRouteErrorDocument(HttpRoute *route, int status, cchar *uri);

/**
    Cache response content in the client by extension.
    @description This configures default caching lifespans for documents with various extensions. This call causes
        a Cache-Control header to be sent with the response instructing the client to cache the response content.
        This is most useful for client caching static web content. For example: to cache "png" graphic files, use: 
        httpAddRouteExpiry(route, when, "png");
    @param route Route to modify
    @param when Time to expire the item. Use mprGetTime() + milliseconds.
    @param extensions Space or comma separated list of request extensions for which the content should be 
        cached in the client.
    @ingroup HttpRoute
 */
extern void httpAddRouteExpiry(HttpRoute *route, MprTime when, cchar *extensions);

/**
    Cache response content in the client by mime type.
    Add client cache expiry definitions to the route.
    @description This configures default caching lifespans for documents with various mime types. The call causes
        a Cache-Control header to be sent with the response instructing the client to cache the response content.
        It is most useful for client caching static web content. For example: to cache "png" graphic files, use: 
        httpAddRouteExpiry(route, when, "png");
    @param route Route to modify
    @param when Time to expire the item. Use mprGetTime() + milliseconds.
    @param mimeTypes Space or command separated list of response mime types for which the content should be cached 
        in the client.
    @ingroup HttpRoute
 */
extern void httpAddRouteExpiryByType(HttpRoute *route, MprTime when, cchar *mimeTypes);

/**
    Add a route filter
    @description This configures the route pipeline by adding processing filters for a request.
        must match. Route conditions are built-in rules that can be applied to routes.
    @param route Route to modify
    @param name Filter name to add
    @param extensions Request extensions for which the filter will be run. A request extension may come from the URI
        if present or from the corresponding filename.
    @param direction Set to HTTP_STAGE_TX for transmit direction and HTTP_STAGE_RX for receive data flow.
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpAddRouteFilter(HttpRoute *route, cchar *name, cchar *extensions, int direction);

/**
    Add a route handler
    @description This configures the route pipeline by adding the given handler.
    @param route Route to modify
    @param name Filter name to add
    @param extensions Request extensions for which the handler will be selected. A request extension may come from the URI
        if present or from the corresponding filename.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpAddRouteHandler(HttpRoute *route, cchar *name, cchar *extensions);

/**
    Add a route header check
    @description This configures the route to match a request only if the specified header field matches a specific value.
    @param route Route to modify
    @param header Header field to interrogate
    @param value Header value that will match
    @param flags Set to HTTP_ROUTE_NOT to negate the header test
    @ingroup HttpRoute
 */
extern void httpAddRouteHeader(HttpRoute *route, cchar *header, cchar *value, int flags);

/**
    Set the route index document
    @description Set the name of the index document to serve. Index documents may be served when the request corresponds
        to a directory on the file system.
    @param route Route to modify
    @param path Path name to the index document. If the path is a relative path, it may be joined to the route 
        directory to create an absolute path.
    @return A reference to the route data. Otherwise return null if the route data for the given key was not found.
    @ingroup HttpRoute
 */
extern void httpAddRouteIndex(HttpRoute *route, cchar *path);

/**
    Add a route language suffix
    @description This configures the route pipeline by adding the given language for request processing.
        The language definition includes a suffix which will be added to the request filename.
    @param route Route to modify
    @param language Language symbolic name. For example: "en" for english.
    @param suffix Extension suffix to add when creating filenames for the request. For example: "fr" to add to "index.html"
        could produce: "index.fr.html".
    @param flags Set to HTTP_LANG_BEFORE to insert the suffix before the filename extension. Set to HTTP_LANG_AFTER to 
        append after the extension. For example: HTTP_LANG_AFTER would produce "index.html.fr".
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpAddRouteLanguageSuffix(HttpRoute *route, cchar *language, cchar *suffix, int flags);

/**
    Add a route language directory
    @description This configures the route pipeline by adding the given language content directory.
        When creating filenames for matching requests, the language directory is prepended to the request filename.
    @param route Route to modify
    @param language Language symbolic name. For example: "en" for english.
    @param path File system directory to contain content for matching requests.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpAddRouteLanguageDir(HttpRoute *route, cchar *language, cchar *path);

/**
    Add a route param check
    @description This configures the route to match a request only if the specified param field matches a specific value.
    @param route Route to modify
    @param field Param field to interrogate
    @param value Header value that will match
    @param flags Set to HTTP_ROUTE_NOT to negate the query test
    @ingroup HttpRoute
 */
extern void httpAddRouteParam(HttpRoute *route, cchar *field, cchar *value, int flags);

/**
    Add a route update rule
    @description This configures the route pipeline by adding processing update rules for a request.
        Updates are built-in rules that can be applied to routes.
    @param route Route to modify
    @param name Update rule to add. Supported update rules include: "cmd", "field" and "lang". 
        \n\n
        The "cmd" rule is used to run external commands. For example: "cmd touch /tmp/filename".
        \n\n
        The "param" rule is used to set values in the request param fields. For example: "param priority high". 
        \n\n
        The "lang" update rule is used internally to implement the various language options.
        See #httpSetRouteTarget for a list of the token values that can be included in the condition rule details.
    @param details Update rule parameters.
    @param flags Reserved.
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpAddRouteUpdate(HttpRoute *route, cchar *name, cchar *details, int flags);

/**
    Add a route for static content under a "static" directory. This can be used for ESP applications. Use the EspDir
        appweb configuration file directive to modify the "static" directory.
    @description This will add a route for static content. This will add the following route:
    <table>
        <tr><td>Name</td><td>Method</td><td>Pattern</td><td>Target</td></tr>
        <tr><td>static</td><td>GET</td><td>^/static(/(.)*$</td><td>$1</td></tr>
    </table>
    @param parent Parent route from which to inherit configuration.
    @ingroup HttpRoute
 */
extern void httpAddStaticRoute(HttpRoute *parent);

/**
    Backup the route log if required
    @description If the log file is greater than the maximum configured, or MPR_ANEW was set via httpSetRouteLog, then 
    archive the log.
    @param route Route to modify
    @ingroup HttpRoute
 */
extern void httpBackupRouteLog(HttpRoute *route);

/**
    Clear the pipeline stages for the route
    @description This resets the configured pipeline stages for the route.
    @param route Route to modify
    @param direction Set to HTTP_STAGE_TX for transmit direction and HTTP_STAGE_RX for receive data flow.
    @ingroup HttpRoute
 */
extern void httpClearRouteStages(HttpRoute *route, int direction);

/**
    Create a route suitable for use as an alias
    @description The parent supplies the owning host for the route. A route is not added to its owning host until it
        is finalized by calling #httpFinalizeRoute
    @param parent Parent route to inherit from
    @param pattern Pattern to match URIs 
    @param path File system directory containing documents for this route
    @param status Http redirect status for matching requests. Set to zero if not using redirects
    @return Allocated HttpRoute object
    @ingroup HttpRoute
 */
extern HttpRoute *httpCreateAliasRoute(HttpRoute *parent, cchar *pattern, cchar *path, int status);

/**
    Create a configured route 
    @description This creates a route and configures the request pipeline with range, chunk and upload filters.
    @param host HttpHost object owning the route
    @param serverSide Set to "true" if this is a server side route. Set to "false" for client side.
    @return Allocated HttpRoute object
    @ingroup HttpRoute
 */
extern HttpRoute *httpCreateConfiguredRoute(struct HttpHost *host, int serverSide);

/**
    Create a default route for a host
    @description When the route is fully configured, it should be finalized which will add it to its owning host.
    @param host HttpHost object owning the route
    @return Allocated HttpRoute object
    @ingroup HttpRoute
 */
extern HttpRoute *httpCreateDefaultRoute(struct HttpHost *host);

/**
    Create a route inherited from a parent route
    @description When the route is fully configured, it should be finalized which will add it to its owning host.
    @param route Parent route from which to inherit
    @return Allocated HttpRoute object
    @ingroup HttpRoute
 */
extern HttpRoute *httpCreateInheritedRoute(HttpRoute *route);

/**
    Create a route for a host
    @description This call creates a bare route without inheriting from a parent route.
    When the route is fully configured, it should be finalized which will add it to its owning host.
    @param host HttpHost object owning the route
    @return Allocated HttpRoute object
    @ingroup HttpRoute
 */
extern HttpRoute *httpCreateRoute(struct HttpHost *host);

/**
    Define a route condition rule
    @description This creates a new condition rule.
    @param name Condition name 
    @param proc Condition function to process the condition during route matching.
    @ingroup HttpRoute
 */
extern void httpDefineRouteCondition(cchar *name, HttpRouteProc *proc);

/**
    Define a route target rule
    @description This creates a new target rule.
    @param name Target name 
    @param proc Target function to process the target during route matching.
    @ingroup HttpRoute
 */
extern void httpDefineRouteTarget(cchar *name, HttpRouteProc *proc);

/**
    Define a route update rule
    @description This creates a new update rule.
    @param name Update name 
    @param proc Update function to process the update during route matching.
    @ingroup HttpRoute
 */
extern void httpDefineRouteUpdate(cchar *name, HttpRouteProc *proc);

/**
    Finalize a route
    @description A route must be finalized to add it to its owning hosts list of routes.
    @param route Route to modify
    @ingroup HttpRoute
 */
extern void httpFinalizeRoute(HttpRoute *route);

/**
    Get extra route data
    @description Routes can store extra configuration information indexed by key. This is used by handlers, filters,
        connectors and updates to store additional information on a per-route basis.
    @param route Route to modify
    @param key Unique string key to identify the data.
    @return A reference to the route data. Otherwise return null if the route data for the given key was not found.
    @see httpGetRouteData
    @ingroup HttpRoute
 */
extern void *httpGetRouteData(HttpRoute *route, cchar *key);

/**
    Get the route directory
    @description Routes can define a default directory for documents to serve. This value may be used by
        target rules to calculate the response filename.
    @param route Route to modify
    @return The route documents directory pathname.
    @ingroup HttpRoute
 */
extern cchar *httpGetRouteDir(HttpRoute *route);

/**
    Get the route method list
    @param route Route to examine
    @return The the list of support methods. Return NULL if not method list is defined.
    @ingroup HttpRoute
 */
extern cchar *httpGetRouteMethods(HttpRoute *route);

/**
    Graduate the limits from the parent route.
    @description This creates a unique limit structure for the route if it is currently inheriting its parents limits.
    @param route Route to modify
    @param limits Limits to use if graduating.
    @ingroup HttpRoute
 */
extern HttpLimits *httpGraduateLimits(HttpRoute *route, HttpLimits *limits);

/** 
    Create a URI link. 
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
        If the is a JSON style of options, it can specify the URI components: scheme, host, port, path, reference and
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
    @param options Hash of option values for embedded tokens.
    @return A normalized, server-local Uri string.

    <pre>
    httpLink(conn, "http://example.com/index.html", 0);
    httpLink(conn, "/path/to/index.html", 0);
    httpLink(conn, "../images/splash.png", 0);
    httpLink(conn, "~/static/images/splash.png", 0);
    httpLink(conn, "${app}/static/images/splash.png", 0);
    httpLink(conn, "@controller/checkout", 0);
    httpLink(conn, "@controller/")                //  Controller = Controller, action = index
    httpLink(conn, "@init")                       //  Current controller, action = init
    httpLink(conn, "@")                           //  Current controller, action = index
    httpLink(conn, "{ action: '@post/create' }", 0);
    httpLink(conn, "{ action: 'checkout' }", 0);
    httpLink(conn, "{ action: 'logout', controller: 'admin' }", 0);
    httpLink(conn, "{ action: 'admin/logout'", 0);
    httpLink(conn, "{ product: 'candy', quantity: '10', template: '/cart/${product}/${quantity}' }", 0);
    httpLink(conn, "{ route: '~/STAR/edit', action: 'checkout', id: '99' }", 0);
    httpLink(conn, "{ template: '~/static/images/${theme}/background.jpg', theme: 'blue' }", 0);
    </pre>
*/
extern char *httpLink(HttpConn *conn, cchar *target, MprHash *options);

/**
    Lookup an error document by HTTP status code
    @description This looks up error documents configured via #httpAddRouteErrorDocument
    @param route Route to modify
    @param status HTTP status code integer 
    @return URI associated with the error document for the requested status.
    @ingroup HttpRoute
 */
extern cchar *httpLookupRouteErrorDocument(HttpRoute *route, int status);

/**
    Make a filename path
    @description This makes a filename by expanding the tokens "${token}" and then normalizing the path and converting
        to an absolute path name. The supported tokens are:
        <ul>  
            <li>PRODUCT - for the product name</li>
            <li>OS - for the operating system name. E.g. LINUX, MACOSX, VXWORKS, or WIN</li>
            <li>VERSION - for the product version. E.g. 4.0.2</li>
            <li>LIBDIR - for the shared library directory. E.g. /usr/lib/appweb/bin </li>
            <li>DOCUMENT_ROOT - for the default directory containing documents to serve</li>
            <li>SERVER_ROOT - for the directory containing the web server configuration files</li>
        </ul>  
        Additional tokens can be defined via #httpSetRoutePathVar.
    @param route Route to modify
    @param path Path name to examine
    @return An absolute file name.
    @ingroup HttpRoute
 */
extern char *httpMakePath(HttpRoute *route, cchar *path);

/**
    Map the request URI and route target to a filename
    @description This sets the HttpTx filename, ext, etag and info fields.
    @param conn HttpConn connection object 
    @param route Route to modify
 */
extern void httpMapFile(HttpConn *conn, HttpRoute *route);

/**
    Match a route against the current request 
    @description This tests if a route matches the current request on a connection. This call is automatically called
        by #httpRouteRequest for incoming requests to a server.
    @param conn HttpConn connection object 
    @param route Route to modify
    @return HTTP_ROUTE_OK if the request matches. Return HTTP_ROUTE_REMATCH if the request has been modified and
        route matching must be restarted.
    @ingroup HttpRoute
 */
extern int httpMatchRoute(HttpConn *conn, HttpRoute *route);

/**
    Reset the route pipeline
    @description This completely resets the pipeline and discards inherited pipeline configuration. This resets the
        error documents, expiry cache values, extensions, handlers, input and output stage configuration.
    @param route Route to modify
    @ingroup HttpRoute
 */
extern void httpResetRoutePipeline(HttpRoute *route);

/**
    Set the route authentication
    @description This defines the authentication configuration for basic and digest authentication for the route.
    @param route Route to modify
    @param auth Authentication object
    @ingroup HttpRoute
 */
extern void httpSetRouteAuth(HttpRoute *route, HttpAuth *auth);

/**
    Control file upload auto delete
    @description This controls whether files are auto-deleted after the handler runs to service a request.
    @param route Route to modify
    @param on Set to true to enable auto-delete. Auto-delete is enabled by default.
    @ingroup HttpRoute
 */
extern void httpSetRouteAutoDelete(HttpRoute *route, bool on);

/**
    Contol content compression for the route
    @description This configures content compression for the route. Some handlers observe the content compression status
        and will attempt to use or compress content before transmitting to the client. The fileHandler is currently
        the only handler that uses this capability.
    @param route Route to modify
    @param flags Set to HTTP_ROUTE_GZIP to enable the fileHandler to serve eqivalent compressed content with a "gz"
        extension.
    @ingroup HttpRoute
 */
extern void httpSetRouteCompression(HttpRoute *route, int flags);

/**
    Set the connector to use for a route
    @param route Route to modify
    @param name Connector name to use for this route
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpSetRouteConnector(HttpRoute *route, cchar *name);

/**
    Set route data
    @description Routes can store extra configuration information indexed by key. This is used by handlers, filters,
        connectors and updates to store additional information on a per-route basis.
    @param route Route to modify
    @param key Unique string to identify the data
    @param data Data object. This must be allocated via mprAlloc.
    @ingroup HttpRoute
 */
extern void httpSetRouteData(HttpRoute *route, cchar *key, void *data);

/**
    Set the default language for the route
    @description This call defines the default language to serve if the client does not provide an Accept HTTP header
        with language preference instructions.
    @param route Route to modify
    @param language Language symbolic name. For example: "en" for english.
    @ingroup HttpRoute
 */
extern void httpSetRouteDefaultLanguage(HttpRoute *route, cchar *language);

/**
    Set the route directory
    @description Routes can define a default directory for documents to serve. This value may be used by
        target rules to calculate the response filename.
    @param route Route to modify
    @param dir Directory path name for the route content
    @return The route documents directory pathname.
    @ingroup HttpRoute
 */
extern void httpSetRouteDir(HttpRoute *route, cchar *dir);

/**
    Update the route flags
    @description Low level routine to manipulate the route flags
    @param route Route to modify
    @param flags Flags mask 
    @ingroup HttpRoute
    @internal
 */
extern void httpSetRouteFlags(HttpRoute *route, int flags);

/**
    Set the handler to use for a route
    @description This defines the stage handler to use in the request pipline for requests matching this route.
        Note that you can also use httpAddRouteHandler which configures a set of handlers that will match by extension.
    @param route Route to modify
    @param name Handler name to define
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpSetRouteHandler(HttpRoute *route, cchar *name);

/*
    Define the owning host for a route.
    @description WARNING: this should not be called by users.
    @param route Route to modify
    @param host HttpHost object
    @internal
 */
extern void httpSetRouteHost(HttpRoute *route, struct HttpHost *host);

/**
    Define the methods for the route
    @description This defines the set of valid HTTP methods for requests to match this route
    @param route Route to modify
    @param methods Set to a comma or space separated list of methods. Can also set to "All" or "*" for all possible 
        methods.  Standard methods include: "DELETE, GET, OPTIONS, POST, PUT, TRACE".
    @ingroup HttpRoute
 */
extern void httpSetRouteMethods(HttpRoute *route, cchar *methods);

/**
    Set the route name
    @description Symbolic route names are used by httpLink and when displaying route tables.
    @param route Route to modify
    @param name Unique symbolic name for the route. If a name is not defined, the route pattern will be used as the name. 
    @ingroup HttpRoute
 */
extern void httpSetRouteName(HttpRoute *route, cchar *name);

/**
    Define a path token variable
    @description The #httpMakePath routine and route conditions, updates, headers, fields and targets will expand 
        tokenized expressions "${token}". Additional tokens can be defined via this API.
    @param route Route to modify
    @param token Name of the token to define 
    @param value Value of the token
    @ingroup HttpRoute
 */
extern void httpSetRoutePathVar(HttpRoute *route, cchar *token, cchar *value);

/**
    Set the route pattern
    @description This call defines the route regular expression pattern that is used to match against the request URI.
        The route pattern is an enhanced JavaScript-compatibile regular expression. It is enhanced by optionally 
        embedding braced tokens "{name}" in the patter. During request URI matching, these tokens are extracted and
        defined in the request params and are available to the request. The normal regular expression repeat syntax 
        also uses "{}". To use the traditional (uncommon) repeat syntax, back quote with "\\".
        Sub-expressions and token expressions are also available in various rules as numbered tokens "$1". For example:
        the pattern "/app/(.*)(\.html)$" will permit a file target "$1.${request.Language=fr}.$2".
    @param route Route to modify
    @param pattern Route regular expression pattern 
    @param flags Set to HTTP_ROUTE_NOT to negate the pattern match result
    @ingroup HttpRoute
 */
extern void httpSetRoutePattern(HttpRoute *route, cchar *pattern, int flags);

/**
    Set the route prefix
    @description Routes may have a prefix which will be stripped from the request URI if the request matches.
        The prefix is made available as the "${request:prefix}" token and also as the ScriptName via some handlers.
    @param route Route to modify
    @param prefix URI prefix to define for the route. 
    @ingroup HttpRoute
 */
extern void httpSetRoutePrefix(HttpRoute *route, cchar *prefix);

/**
    Set the script to service the route.
    @description This is used by handlers to add a per-route script for processing. Ejscript uses this to specify
        the server script. Either a literal script or a path to a script filename can be provided.
    @param route Route to modify
    @param script Literal script to execute.
    @param scriptPath Pathname to the script file to execute
    @ingroup HttpRoute
    @internal
 */
extern void httpSetRouteScript(HttpRoute *route, cchar *script, cchar *scriptPath);

/**
    Set the source code module for the route
    @description Some handlers can dynamically load web applications and controllers to serve requests.
    @param route Route to modify
    @param source Source path or description 
    @ingroup HttpRoute
 */
extern void httpSetRouteSource(HttpRoute *route, cchar *source);

/**
    Set a route target
    @description This configures the route pipeline by defining a route target. The route target is interpreted by
        the selected route handler to process the request. 
        Route targets can contain symbolic tokens that are expanded at run-time with their corresponding values. There are 
        three classes of tokens:
        <ul>  
            <li>System varibles - such as DOCUMENT_ROOT, LIBDIR, PRODUCT, OS, SERVER_ROOT, VERSION.</li>
            <li>Route URI tokens - these are the braced tokens in the route pattern.</li>
            <li>Request fields - these are request state and property values.</li>
        </ul>
        System and URI tokens are of the form: "${token}" where "token" is the name of the variable or URI token. 
        Request fields are of the form: "${family:name=defaultValue}" where the family defines a set of values. 
        If the named field is not present, an optional default value "=defaultValue" will be used instead.
        These supported request field families are:
        <ul>  
            <li>header - for request HTTP header values</li>
            <li>param - for request params</li>
            <li>query - for request query field values</li>
            <li>request - for request details</li>
            <li>Any URI pattern  token</li>
        </ul>
        For example: "run ${header:User-Agent}" to select the client's browser string passed in the HTTP headers.
        For example: "run ${field:name}" to select the client's browser string passed in the HTTP headers.
        For example: "run ${name}.html" where {name} was a token in the route pattern.
        For example: "run ${name}.html" where {name} was a token in the route pattern.
        The supported request key names are:
        <ul>
            <li>clientAddress - The client IP address</li>
            <li>clientPort - The client port number</li>
            <li>error - Any request or connection error message</li>
            <li>ext - The request extension</li>
            <li>extraPath - The request extra path after the script extension</li>
            <li>filename - The mapped request filename in physical storage</li>
            <li>language - The selected language for the request</li>
            <li>languageDir - The langauge directory</li>
            <li>host - The host name owning the route for the request</li>
            <li>method - The request HTTP method</li>
            <li>originalUri - The original, pre-decoded URI</li>
            <li>pathInfo - The path portion of the URI after the host and port information</li>
            <li>prefix - The route prefix</li>
            <li>query - The request query information</li>
            <li>reference - The request reference fragment. This is the URI portion after "#"</li>
            <li>scheme - The request protocol scheme. E.g. "http"</li>
            <li>scriptName - The request script or application name</li>
            <li>serverAddress - The server IP address</li>
            <li>serverPort - The server port number</li>
            <li>uri - The full request URI. May be modified by routes, handlers and filters</li>
        </ul>
        Also see #httpMakePath for additional tokens (DOCUMENT_ROOT, LIBDIR, PRODUCT, OS, SERVER_ROOT, VERSION).
    @param route Route to modify
    @param name Target rule to add. Supported update rules include:
        "close", "redirect", "run" and "write". 
        \n\n
        The "close" rule is used to do abortive closes for the request. This is useful for ward off known security attackers.
        For example: "close immediate". The "close" rule takes no addition parameters. 
        \n\n
        \n\n
        The "redirect" rule is used to redirect the request to a new resource. For example: "redirect 302 /tryAgain.html". 
        The "redirect" takes the form: "redirect status URI". The status code is used as the HTTP response
        code. The URI can be a fully qualified URI beginning with "http" or it can be a relative URI.
        \n\n
        The "run" target is used to run the configured handler to respond to the request.
        For example: "file ${DOCUMENT_ROOT}/${request.uri}.gz". 
        \n\n
        The "write" rule is used to write literal data back to the client. For example: "write 200 Hello World\r\n". 
        The "write" rule takes the form: "write [-r] status message". Write data is by default HTML encoded to help
        eliminate XSS security exposures. The "-r" option selects "raw" output and bypasses the HTML encoding of the
        write data string. 
        \n\n
        WARNING: Take great care when using raw writes with tokens. Write data is not HTML encoded and echoing back to
        raw data to the client can cause XSS and other security issues.
        The status field defines the HTTP status code to use in the response.
    @param details Update rule parameters.
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRoute
 */
extern int httpSetRouteTarget(HttpRoute *route, cchar *name, cchar *details);

/**
    Set the route template
    @description Set the route URI template uses when constructing URIs via httpLink.
    @param route Route to modify
    @param tplate URI template to use. Templates may contain embedded tokens "{token}" where the token names correspond
        to the token names in the route pattern. 
    @ingroup HttpRoute
 */
extern void httpSetRouteTemplate(HttpRoute *route, cchar *tplate);

/**
    Set the route trace filter
    @description Trace filters include or exclude trace items based on the request filename extension.
    @param route HttpRoute object
    @param dir Trace direction. Set to HTTP_TRACE_TX or HTTP_TRACE_RX
    @param levels Trace class levels. Indicies are: HTTP_TRACE_CONN, HTTP_TRACE_FIRST, HTTP_TRACE_HEADER, HTTP_TRACE_BODY,
        HTTP_TRACE_LIMITS, HTTP_TRACE_TIME.
    @param len Maximum content length eligible for tracing.
    @param include Comma or space separated list of extensions to include in tracing
    @param exclude Comma or space separated list of extensions to exclude from tracing
    @ingroup HttpRoute
 */
extern void httpSetRouteTraceFilter(HttpRoute *route, int dir, int levels[HTTP_TRACE_MAX_ITEM], 
        ssize len, cchar *include, cchar *exclude);

/**
    Define the maximum number of workers for a route
    @param route Route to modify
    @param workers Maximum number of workers for this route
    @ingroup HttpRoute
    @internal
 */
extern void httpSetRouteWorkers(HttpRoute *route, int workers);

/**
    Expand a template string using given options
    @description This expands a string with embedded tokens of the form "${token}" using values from the given options.
    @param conn HttpConn connection object created via $httpCreateConn
    @param tplate Template string to process
    @param options Hash of option values for embedded tokens.
    @ingroup HttpRoute
 */
extern char *httpTemplate(HttpConn *conn, cchar *tplate, MprHash *options);

/**
    Tokenize a string based on route data
    @description This is a utility routine to parse a string into tokens given a format specifier. 
    Mandatory tokens can be specified with "%" format specifier. Optional tokens are specified with "?" format. 
    Supported tokens:
    <ul>
    <li>%B - Boolean. Parses: on/off, true/false, yes/no.</li>
    <li>%N - Number. Parses numbers in base 10.</li>
    <li>%S - String. Removes quotes.</li>
    <li>%P - Path string. Removes quotes and expands ${PathVars}. Resolved relative to host->dir (ServerRoot).</li>
    <li>%W - Parse words into a list</li>
    <li>%! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.</li>
    </ul>
    Values wrapped in quotes will have the outermost quotes trimmed.
    @param route Route to modify
    @param str String to expand
    @param fmt Format string specifier
    @return True if the string can be successfully parsed.
    @ingroup HttpRoute
 */
extern bool httpTokenize(HttpRoute *route, cchar *str, cchar *fmt, ...);

/**
    Tokenize a string based on route data
    @description This is a utility routine to parse a string into tokens given a format specifier. 
    This call is similar to #httpTokenize but uses a va_list argument.
    @param route Route to modify
    @param str String to expand
    @param fmt Format string specifier
    @param args Varargs argument list
    @return True if the string can be successfully parsed.
    @ingroup HttpRoute
 */
extern bool httpTokenizev(HttpRoute *route, cchar *str, cchar *fmt, va_list args);

/**
    Configure the route access log
    @param route Route to modify
    @param path Path for route access log file.
    @param size Maximum size of the log file before archiving
    @param backup Set to true to create a backup of the log file if archiving.
    @param format Log file format
    @param flags Set to MPR_LOG_ANEW to archive the log when the application reboots.
    @ingroup HttpRoute
 */
extern void httpSetRouteLog(HttpRoute *route, cchar *path, ssize size, int backup, cchar *format, int flags);

/**
    Write data to the route access log
    @description Write data after archiving if required.
    @param route Route to modify
    @param buf Data buffer to write
    @param len Size of the data buffer.
    @ingroup HttpRoute
 */
extern void httpWriteRouteLog(HttpRoute *route, cchar *buf, ssize len);

/*
    Internal
 */
extern void httpLogRequest(HttpConn *conn);
extern MprFile *httpOpenRouteLog(HttpRoute *route);
extern int httpStartRoute(HttpRoute *route);
extern void httpStopRoute(HttpRoute *route);

/********************************** HttpUploadFile *********************************/
/**
    Upload File
    @description Each uploaded file has an HttpUploadedFile entry. This is managed by the upload handler.
    @stability Evolving
    @defgroup HttpUploadFile HttpUploadFile
    @see httpAddUploadFile httpRemoveAllUploadedFiles httpRemoveUploadFile
 */
typedef struct HttpUploadFile {
    cchar           *filename;              /**< Local (temp) name of the file */
    cchar           *clientFilename;        /**< Client side name of the file */
    cchar           *contentType;           /**< Content type */
    ssize           size;                   /**< Uploaded file size */
} HttpUploadFile;


/**
    Add an Uploaded file
    @description Add an uploaded file to the Rx.files collection.
    @param conn HttpConn connection object created via $httpCreateConn
    @param id Unique identifier for the file  
    @param file Instance of HttpUploadFile
    @ingroup HttpUploadFile
    @internal
 */
extern void httpAddUploadFile(HttpConn *conn, cchar *id, HttpUploadFile *file);

/**
    Remove all uploaded files
    @description Remove all uploaded files from the temporary file store
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpUploadFile
    @internal
 */
extern void httpRemoveAllUploadedFiles(HttpConn *conn);

/**
    Remove an uploaded file
    @description Remove an uploaded file from the temporary file store
    @param conn HttpConn connection object created via $httpCreateConn
    @param id Identifier used with #httpAddUploadFile for the file
    @ingroup HttpUploadFile
    @internal
 */
extern void httpRemoveUploadFile(HttpConn *conn, cchar *id);

/********************************** HttpRx *********************************/
/* 
    Rx flags
 */
#define HTTP_DELETE             0x1         /**< DELETE method  */
#define HTTP_GET                0x2         /**< GET method  */
#define HTTP_HEAD               0x4         /**< HEAD method  */
#define HTTP_OPTIONS            0x8         /**< OPTIONS method  */
#define HTTP_POST               0x10        /**< Post method */
#define HTTP_PUT                0x20        /**< PUT method  */
#define HTTP_TRACE              0x40        /**< TRACE method  */
#define HTTP_UNKNOWN            0x800       /**< Unknown method  */
#define HTTP_METHOD_MASK        0xFFF       /**< Method mask */
#define HTTP_CREATE_ENV         0x100       /**< Must create env for this request */
#define HTTP_IF_MODIFIED        0x200       /**< If-[un]modified-since supplied */
#define HTTP_CHUNKED            0x400       /**< Content is chunk encoded */
#define HTTP_ADDED_QUERY_PARAMS 0x800       /**< Query added to params */
#define HTTP_ADDED_FORM_PARAMS  0x1000      /**< Form body data added to params */
#define HTTP_LIMITS_OPENED      0x2000      /**< Request limits opened */

/*  
    Incoming chunk encoding states
 */
#define HTTP_CHUNK_UNCHUNKED  0             /**< Data is not transfer-chunk encoded */
#define HTTP_CHUNK_START      1             /**< Start of a new chunk */
#define HTTP_CHUNK_DATA       2             /**< Start of chunk data */
#define HTTP_CHUNK_EOF        3             /**< End of last chunk */

/** 
    Http Rx
    @description Most of the APIs in the rx group still take a HttpConn object as their first parameter. This is
        to make the API easier to remember - APIs take a connection object rather than a rx or tx object.
    @stability Evolving
    @defgroup HttpRx HttpRx
    @see HttpConn HttpRx HttpTx httpAddBodyVars httpAddParamsFromBuf httpAddParamsFromQueue httpContentNotModified 
        httpCreateCGIParams httpGetContentLength httpGetCookies httpGetParam httpGetParams httpGetHeader 
        httpGetHeaderHash httpGetHeaders httpGetIntParam httpGetLanguage httpGetQueryString httpGetStatus 
        httpGetStatusMessage httpMatchParam httpRead httpReadString httpSetParam httpSetIntParam httpSetUri 
        httpTestParam httpTrimExtraPath 
 */
typedef struct HttpRx {
    /* Ordered for debugging */
    char            *method;                /**< Request method */
    char            *uri;                   /**< Current URI (not decoded, may be rewritten) */
    char            *pathInfo;              /**< Path information after the scriptName (Decoded and normalized) */
    char            *scriptName;            /**< ScriptName portion of the uri (Decoded). May be empty or start with "/" */
    char            *extraPath;             /**< Extra path information (CGI|PHP) */
    int             eof;                    /**< All read data has been received (eof) */
    MprOff          bytesRead;              /**< Length of content read by user */
    MprOff          length;                 /**< Content length header value (ENV: CONTENT_LENGTH) */
    MprOff          remainingContent;       /**< Remaining content data to read (in next chunk if chunked) */

    HttpConn        *conn;                  /**< Connection object */
    HttpRoute       *route;                 /**< Route for request */

    MprList         *etags;                 /**< Document etag to uniquely identify the document version */
    HttpPacket      *headerPacket;          /**< HTTP headers */
    MprHash         *headers;               /**< Header variables */
    MprList         *inputPipeline;         /**< Input processing */
    HttpUri         *parsedUri;             /**< Parsed request uri */
    MprHash         *requestData;           /**< General request data storage. Users must create hash table if required */
    MprTime         since;                  /**< If-Modified date */

    int             chunkState;             /**< Chunk encoding state */
    int             flags;                  /**< Rx modifiers */
    int             form;                   /**< Using mime-type application/x-www-form-urlencoded */
    int             streamInput;            /**< Streaming read data. Means !form */
    int             needInputPipeline;      /**< Input pipeline required to process received data */
    int             traceLevel;             /**< General trace level for header level info */
    int             upload;                 /**< Request is using file upload */

    bool            ifModified;             /**< If-Modified processing requested */
    bool            ifMatch;                /**< If-Match processing requested */

    /*  
        Incoming response line if a client request 
     */
    int             status;                 /**< HTTP response status */
    char            *statusMessage;         /**< HTTP Response status message */

    /* 
        Header values
     */
    char            *accept;                /**< Accept header */
    char            *acceptCharset;         /**< Accept-Charset header */
    char            *acceptEncoding;        /**< Accept-Encoding header */
    char            *acceptLanguage;        /**< Accept-Language header */
    char            *cookie;                /**< Cookie header */
    char            *connection;            /**< Connection header */
    char            *contentLength;         /**< Content length string value */
    char            *hostHeader;            /**< Client supplied host name header */

    char            *pragma;                /**< Pragma header */
    char            *mimeType;              /**< Mime type of the request payload (ENV: CONTENT_TYPE) */
    char            *originalMethod;        /**< Original method from the client */
    char            *originalUri;           /**< Original URI passed by the client */
    char            *redirect;              /**< Redirect route header */
    char            *referrer;              /**< Refering URL */
    char            *securityToken;         /**< Security form token */
    char            *userAgent;             /**< User-Agent header */

    MprHash         *params;                /**< Request params (Query and post data variables) */
    MprHash         *svars;                 /**< Server variables */
    HttpRange       *inputRange;            /**< Specified range for rx (post) data */

    /*  
        Auth details
     */
    int             authenticated;          /**< Request has been authenticated */
    char            *authAlgorithm;
    char            *authDetails;
    char            *authStale;             
    char            *authType;              /**< Authorization type (basic|digest) (ENV: AUTH_TYPE) */

    /*  
        Upload details
     */
    MprHash         *files;                 /**< Uploaded files. Managed by the upload filter */
    char            *uploadDir;             /**< Upload directory */
    int             autoDelete;             /**< Automatically delete uploaded files */

    char            *paramString;           /**< Cached param data as a string */
    HttpLang        *lang;                  /**< Selected language */

    /*
        Routing info
     */
    char            *target;                /**< Route target */
    int             matches[HTTP_MAX_ROUTE_MATCHES * 2];
    int             matchCount;
} HttpRx;

/**
    Add query and form body data to params
    @description This adds query data and posted body data to the request params
    @param conn HttpConn connection object
    @ingroup HttpRx
    @internal
 */
extern void httpAddParams(HttpConn *conn);

/**
    Test if the content has not been modified
    @description This call tests if the file content to be served has been modified since the client last
        requested this resource. The client must provide an Etag and Since or If-Modified headers.
    @param conn HttpConn connection object
    @return True if the content is current and has not been modified.
    @ingroup HttpRx
 */
extern bool httpContentNotModified(HttpConn *conn);

/**
    Create CGI parameters
    @description This call creates request params corresponding to the standard CGI/1.1 environment variables. 
    This is used by the CGI and PHP handlers. It may also be useful to handlers that wish to expose CGI style 
    environment variables
    through the form vars interface.
    @param conn HttpConn connection object
    @ingroup HttpRx
 */
extern void httpCreateCGIParams(HttpConn *conn);

/** 
    Get the receive body content length
    @description Get the length of the receive body content (if any). This is used in servers to get the length of posted
        data and in clients to get the response body length.
    @param conn HttpConn connection object created via $httpCreateConn
    @return A count of the response content data in bytes.
    @ingroup HttpRx
 */
extern MprOff httpGetContentLength(HttpConn *conn);

/** 
    Get the request cookies
    @description Get the cookies defined in the current requeset
    @param conn HttpConn connection object created via $httpCreateConn
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup HttpRx
 */
extern cchar *httpGetCookies(HttpConn *conn);

/**
    Get a request param
    @description Get the value of a named request param. Form variables are define via www-urlencoded query or post
        data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request param to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return String containing the request param's value. Caller should not free.
    @ingroup HttpRx
 */
extern cchar *httpGetParam(HttpConn *conn, cchar *var, cchar *defaultValue);

/**
    Get the request params table
    @description This call gets the form var table for the current request.
        Query data and www-url encoded form data is entered into the table after decoding.
        Use $mprLookupKey to retrieve data from the table.
    @param conn HttpConn connection object
    @return $MprHash instance containing the form vars
    @ingroup HttpRx
 */
extern MprHash *httpGetParams(HttpConn *conn);

/**
    Get the request params table as a string
    @description This call gets the request params encoded as a string. The params are always in the same order 
        regardless of the form parameter order. Request parameters include query parameters, form data and routing 
        parameters.
    @param conn HttpConn connection object
    @return A string representation in www-urlencoded format.
    @ingroup HttpRx
 */
extern char *httpGetParamsString(HttpConn *conn);

/** 
    Get an rx http header.
    @description Get a http response header for a given header key.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Name of the header to retrieve. This should be a lower case header name. For example: "Connection"
    @return Value associated with the header key or null if the key did not exist in the response.
    @ingroup HttpRx
 */
extern cchar *httpGetHeader(HttpConn *conn, cchar *key);

/** 
    Get the hash table of rx Http headers
    @description Get the internal hash table of rx headers
    @param conn HttpConn connection object created via $httpCreateConn
    @return Hash table. See MprHash for how to access the hash table.
    @ingroup HttpRx
 */
extern MprHash *httpGetHeaderHash(HttpConn *conn);

/** 
    Get all the request http headers.
    @description Get all the rx headers. The returned string formats all the headers in the form:
        key: value\\nkey2: value2\\n...
    @param conn HttpConn connection object created via $httpCreateConn
    @return String containing all the headers. The caller must free this returned string.
    @ingroup HttpRx
 */
extern char *httpGetHeaders(HttpConn *conn);

/**
    Get a header string from the given hash.
    @description This returns a set of "key: value" lines in HTTP header format.
    @param hash Hash table to examine
    @ingroup HttpRx
    @internal
 */
extern char *httpGetHeadersFromHash(MprHash *hash);

/**
    Get a form variable as an integer
    @description Get the value of a named form variable as an integer. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return Integer containing the form variable's value
    @ingroup HttpRx
 */
extern int httpGetIntParam(HttpConn *conn, cchar *var, int defaultValue);

/**
    Get the language to use for the request
    @description This call tests if the file content to be served has been modified since the client last
        requested this resource. The client must provide an Etag and Since or If-Modified headers.
    @param conn HttpConn connection object
    @param spoken Hash table of HttpLang records. This is typically route->languages. 
    @param defaultLang Default language to use if none specified in the request Accept-Language header.
    @return A HttpLang reference, or null if no language requested or no language found in the spoken table.
    @ingroup HttpRx
 */
extern HttpLang *httpGetLanguage(HttpConn *conn, MprHash *spoken, cchar *defaultLang);

/** 
    Get the request query string
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup HttpRx
 */
extern cchar *httpGetQueryString(HttpConn *conn);

/** 
    Get the response status 
    @param conn HttpConn connection object created via $httpCreateConn
    @return An integer Http response code. Typically 200 is success.
    @ingroup HttpRx
 */
extern int httpGetStatus(HttpConn *conn);

/** 
    Get the Http response status message. The Http status message is supplied on the first line of the Http response.
    @param conn HttpConn connection object created via $httpCreateConn
    @returns A Http status message. 
    @ingroup HttpRx
 */
extern char *httpGetStatusMessage(HttpConn *conn);

/**
    Match a form variable with an expected value
    @description Compare a form variable and return true if it exists and its value matches.
    @param conn HttpConn connection object
    @param var Name of the form variable 
    @param expected Expected value to match with
    @return True if the value matches
    @ingroup HttpRx
 */
extern bool httpMatchParam(HttpConn *conn, cchar *var, cchar *expected);

/** 
    Read rx body data. This will read available body data. If in sync mode, this call may block. If in async
    mode, the call will not block and will return with whatever data is available.
    @param conn HttpConn connection object created via $httpCreateConn
    @param buffer Buffer to receive read data
    @param size Size of buffer. 
    @return The number of bytes read
    @ingroup HttpRx
 */
extern ssize httpRead(HttpConn *conn, char *buffer, ssize size);

/** 
    Read response data as a string. This will read all rx body and return a string that the caller should free. 
    This will block and should not be used in async mode.
    @param conn HttpConn connection object created via $httpCreateConn
    @returns A string containing the rx body. Caller should free.
    @ingroup HttpRx
 */
extern char *httpReadString(HttpConn *conn);

/**
    Set a request param value
    @description Set the value of a named request param to a string value. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request param to retrieve
    @param value Default value to return if the variable is not defined. Can be null.
    @ingroup HttpRx
 */
extern void httpSetParam(HttpConn *conn, cchar *var, cchar *value);

/**
    Set an integer request param value
    @description Set the value of a named request param to an integer value. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the request param to retrieve
    @param value Default value to return if the variable is not defined. Can be null.
    @ingroup HttpRx
 */
extern void httpSetIntParam(HttpConn *conn, cchar *var, int value);

/**
    Set a new HTTP method for processing
    @description This modifies the request method to alter request processing. The original method is preserved in
        the HttpRx.originalMethod field. This is only useful to do before request routing has matched a route.
    @param conn HttpConn connection object
    @param method New method to use. 
    @ingroup HttpRx
 */
extern void httpSetMethod(HttpConn *conn, cchar *method);

/**
    Set a new URI for processing
    @description This modifies the request URI to alter request processing. The original URI is preserved in
        the HttpRx.originalUri field. This is only useful to do before request routing has matched a route.
    @param conn HttpConn connection object
    @param uri New URI to use. The URI can be fully qualified starting with a scheme ("http") or it can be 
        a partial/relative URI. Missing portions of the URI will be completed with equivalent portions from the
        current URI. For example: if the current request URI was http://example.com:7777/index.html, then
        a call to httpSetUri(conn, "/new.html", 0)  will set the request URI to http://example.com:7777/new.html.
        The request script name will be reset and the pathInfo will be set to the path portion of the URI.
    @param query Optional query string to define with the new URI. If query is null, any query string defined
        with the previous URI will be used. If query is set to the empty string, a previous query will be discarded.
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpRx
 */
extern int httpSetUri(HttpConn *conn, cchar *uri, cchar *query);

/**
    Test if a request param is defined
    @param conn HttpConn connection object
    @param var Name of the request param to retrieve
    @return True if the request param is defined
    @ingroup HttpRx
 */
extern int httpTestParam(HttpConn *conn, cchar *var);

/**
    Trim extra path from the URI
    @description This call trims extra path information after the uri extension. This is used by CGI and PHP. 
    The strategy is to heuristically find the script name in the uri. This is assumed to be the original uri 
    up to and including first path component containing a "." Any path information after that is regarded as 
    extra path.  WARNING: Extra path is an old, unreliable, CGI specific technique. Do not use directories 
    with embedded periods.
    @param conn HttpConn connection object
    @ingroup HttpRx
 */
extern void httpTrimExtraPath(HttpConn *conn);

/**
    Pump the Http engine
    @param conn HttpConn connection object
    @param packet Optional packet of input data. Set to NULL if calling from user handlers.
    @ingroup HttpRx
 */
extern void httpPump(HttpConn *conn, HttpPacket *packet);

/* Internal */
extern void httpCloseRx(struct HttpConn *conn);
extern HttpRange *httpCreateRange(HttpConn *conn, MprOff start, MprOff end);
extern HttpRx *httpCreateRx(HttpConn *conn);
extern void httpDestroyRx(HttpRx *rx);
extern bool httpMatchEtag(HttpConn *conn, char *requestedEtag);
extern bool httpMatchModified(HttpConn *conn, MprTime time);
extern bool httpProcessCompletion(HttpConn *conn);
extern void httpProcessWriteEvent(HttpConn *conn);

/********************************** HttpTx *********************************/
/*  
    Tx flags
 */
#define HTTP_TX_NO_BODY             0x1     /**< No transmission body, only sent headers */
#define HTTP_TX_HEADERS_CREATED     0x2     /**< Response headers have been created */
#define HTTP_TX_SENDFILE            0x4     /**< Relay output via Send connector */

/** 
    Http Tx
    @description The tx object controls the transmission of data. This may be client requests or responses to
        client requests. Most of the APIs in the Response group still take a HttpConn object as their first parameter. 
        This is to make the API easier to remember - APIs take a connection object rather than a rx or 
        transmission object.
    @stability Evolving
    @defgroup HttpTx HttpTx
    @see HttpConn HttpRx HttpTx httpAddHeader httpAddHeaderString httpAppendHeader httpAppendHeaderString httpConnect 
        httpCreateTx httpDestroyTx httpFinalize httpFlush httpFollowRedirects httpFormatBody httpFormatError 
        httpFormatErrorV httpFormatResponse httpFormatResponseBody httpFormatResponseError httpFormatResponsev 
        httpGetQueueData httpIsChunked httpIsFinalized httpNeedRetry httpOmitBody httpRedirect httpRemoveHeader 
        httpSetContentLength httpSetContentType httpSetCookie httpSetEntityLength httpSetHeader httpSetHeaderString 
        httpSetResponded httpSetStatus httpSocketBlocked httpWait httpWriteHeaders httpWriteUploadData 
 */
typedef struct HttpTx {
    /* Ordered for debugging */
    MprOff          bytesWritten;           /**< Bytes written including headers */
    MprOff          entityLength;           /**< Original content length before range subsetting */
    ssize           chunkSize;              /**< Chunk size to use when using transfer encoding. Zero for unchunked. */
    cchar           *ext;                   /**< Filename extension */
    char            *etag;                  /**< Unique identifier tag */
    char            *filename;              /**< Name of a real file being served (typically pathInfo mapped) */
    int             flags;                  /**< Response flags */
    HttpStage       *handler;               /**< Server-side request handler stage */
    MprOff          length;                 /**< Transmission content length */
    int             started;                /**< Handler has started */
    int             status;                 /**< HTTP response status */

    HttpUri         *parsedUri;             /**< Client request uri */
    char            *method;                /**< Client request method GET, HEAD, POST, DELETE, OPTIONS, PUT, TRACE */

    struct HttpConn *conn;                  /**< Current connection object */
    MprList         *outputPipeline;        /**< Output processing */
    HttpStage       *connector;             /**< Network connector to send / receive socket data */
    HttpQueue       *queue[2];              /**< Dummy head for the queues */

    MprHash         *headers;               /**< Transmission headers */
    HttpCache       *cache;                 /**< Cache control entry (only set if this request is being cached) */
    MprBuf          *cacheBuffer;           /**< Response caching buffer */
    ssize           cacheBufferLength;      /**< Current size of the cache buffer data */
    cchar           *cachedContent;         /**< Retrieved cached response to send */

    HttpRange       *outputRanges;          /**< Data ranges for tx data */
    HttpRange       *currentRange;          /**< Current range being fullfilled */
    char            *rangeBoundary;         /**< Inter-range boundary */
    MprOff          rangePos;               /**< Current range I/O position in response data */

    char            *altBody;               /**< Alternate transmission for errors */
    int             traceMethods;           /**< Handler methods supported */

    /* File information for file-based handlers */
    MprFile         *file;                  /**< File to be served */
    MprPath         fileInfo;               /**< File information if there is a real file to serve */
    ssize           headerSize;             /**< Size of the header written */
} HttpTx;

/** 
    Add a header to the transmission using a format string.
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @return "Zero" if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup HttpTx
 */
extern void httpAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/** 
    Add a header to the transmission
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value Value to set for the header
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup HttpTx
 */
extern void httpAddHeaderString(HttpConn *conn, cchar *key, cchar *value);

/** 
    Append a transmission header
    @description Set the header if it does not already exists. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup HttpTx
 */
extern void httpAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/** 
    Append a transmission header string
    @description Set the header if it does not already exists. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value Value to set for the header
    @ingroup HttpTx
 */
extern void httpAppendHeaderString(HttpConn *conn, cchar *key, cchar *value);

/** 
    Connect to a server and issue Http client request.
    @description Start a new Http request on the http object and return. This routine does not block.
        After starting the request, you can use httpWait() or httpWForResponse() to wait for the request to 
        achieve a certain state or to complete.
    @param conn HttpConn connection object created via $httpCreateConn
    @param method Http method to use. Valid methods include: "GET", "POST", "PUT", "DELETE", "OPTIONS" and "TRACE" 
    @param uri URI to fetch
    @param ssl SSL configuration to use if a secure connection. 
    @return "Zero" if the request was successfully sent to the server. Otherwise a negative MPR error code is returned.
    @ingroup HttpTx
 */
extern int httpConnect(HttpConn *conn, cchar *method, cchar *uri, struct MprSsl *ssl);

/** 
    Create the tx object. This is used internally by the http library.
    @param conn HttpConn connection object created via $httpCreateConn
    @param headers Optional headers to use for the transmission
    @returns A tx object
    @ingroup HttpTx
 */
extern HttpTx *httpCreateTx(HttpConn *conn, MprHash *headers);

/**
    Destroy the tx object
    @description This is called when the garbage collector frees a connection. It should not be called manually.
    @param tx Tx object
    @ingroup HttpTx
 */
extern void httpDestroyTx(HttpTx *tx);

/** 
    Finalize transmission of the http request
    @description Finalize writing Http data by writing the final chunk trailer if required. If using chunked transfers, 
    a null chunk trailer is required to signify the end of write data. 
    If the request is already finalized, this call does nothing.
    @param conn HttpConn connection object
    @ingroup HttpTx
 */
extern void httpFinalize(HttpConn *conn);

/**
    Flush tx data. This writes any buffered data. 
    @param conn HttpConn connection object created via $httpCreateConn
    @ingroup HttpTx
 */
void httpFlush(HttpConn *conn);

/** 
    Follow redirctions
    @description Enabling follow redirects enables the Http service to transparently follow 301 and 302 redirections
        and fetch the redirected URI.
    @param conn HttpConn connection object created via $httpCreateConn
    @param follow Set to true to enable transparent redirections
    @ingroup HttpTx
 */
extern void httpFollowRedirects(HttpConn *conn, bool follow);

/** 
    Format an error transmission
    @description Format an error message to use instead of data generated by the request processing pipeline.
        This is typically used to send errors and redirections. The message is also sent to the error log.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http response status code
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param ... Arguments for fmt
    @return A count of the number of bytes in the transmission body.
    @ingroup HttpTx
 */
extern void httpFormatError(HttpConn *conn, int status, cchar *fmt, ...);

/** 
    Format an alternate response
    @description Format a response to use instead of data generated by the request processing pipeline.
        This is used for alternate responses that are not errors.
    @param conn HttpConn connection object created via $httpCreateConn
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param ... Arguments for fmt
    @return A count of the number of bytes in the transmission body.
    @ingroup HttpTx
 */
extern ssize httpFormatResponse(HttpConn *conn, cchar *fmt, ...);

/** 
    Format an alternate response
    @description Format a response to use instead of data generated by the request processing pipeline.
        This is similar to #httpFormatResponse.
    @param conn HttpConn connection object created via $httpCreateConn
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param args Varargs style list of arguments
    @return A count of the number of bytes in the transmission body.
    @ingroup HttpTx
 */
extern ssize httpFormatResponsev(HttpConn *conn, cchar *fmt, va_list args);

/** 
    Format a response body.
    @description Format a transmission body to use instead of data generated by the request processing pipeline.
        The body will be created in HTML or in plain text depending on the value of the request Accept header.
        This call is used for alternate responses that are not errors.
    @param conn HttpConn connection object created via $httpCreateConn
    @param title Title string to format into the HTML transmission body.
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param ... Arguments for fmt
    @return A count of the number of bytes in the transmission body.
    @ingroup HttpTx
 */
extern ssize httpFormatResponseBody(HttpConn *conn, cchar *title, cchar *fmt, ...);

/** 
    Format an error response body.
    @description Format an error message transmission body to use instead of data generated by the request 
        processing pipeline.  The body will be created in HTML or in plain text depending on the value of 
        the request Accept header. This is an internal API and users should use #httpError instead.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status HTTP response status code
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param ... Arguments for fmt
    @return A count of the number of bytes in the transmission body.
    @ingroup HttpTx
    @internal
 */
extern void httpFormatResponseError(HttpConn *conn, int status, cchar *fmt, ...);

/**
    Get the queue data for the connection
    @param conn HttpConn connection object created via $httpCreateConn
    @return the private queue data object
 */
extern void *httpGetQueueData(HttpConn *conn);

/** 
    Return whether transfer chunked encoding will be used on this request
    @param conn HttpConn connection object created via $httpCreateConn
    @returns true if chunk encoding will be used
    @ingroup HttpTx
 */ 
extern int httpIsChunked(HttpConn *conn);

/**
    Test if request has been finalized
    @description This call tests if all transmit data has been sent and finalized. Handlers call #httpFinalize
        to signify the end of transmit data.
    @param conn HttpConn connection object
    @ingroup HttpTx
 */
extern int httpIsFinalized(HttpConn *conn);

/** 
    Determine if the transmission needs a transparent retry to implement authentication or redirection. This is used
    by client requests. If authentication is required, a request must first be tried once to receive some authentication 
    key information that must be resubmitted to gain access.
    @param conn HttpConn connection object created via $httpCreateConn
    @param url Reference to a string to receive a redirection URL. Set to NULL if not redirection is required.
    @return true if the request needs to be retried.
    @ingroup HttpTx
 */
extern bool httpNeedRetry(HttpConn *conn, char **url);

/**
    Tell the tx to omit sending any body
    @param conn HttpConn connection object created via $httpCreateConn
 */
extern void httpOmitBody(HttpConn *conn);

/** 
    Redirect the client
    @description Redirect the client to a new uri.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code to send with the response
    @param uri New uri for the client
    @ingroup HttpTx
 */
extern void httpRedirect(HttpConn *conn, int status, cchar *uri);

/** 
    Remove a header from the transmission
    @description Remove a header if present.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @return "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpTx
 */
extern int httpRemoveHeader(HttpConn *conn, cchar *key);

/** 
    Define a content length header in the transmission. This will define a "Content-Length: NNN" request header and
        set Tx.length.
    @param conn HttpConn connection object created via $httpCreateConn
    @param length Numeric value for the content length header.
    @ingroup HttpTx
 */
extern void httpSetContentLength(HttpConn *conn, MprOff length);

/** 
    Set the transmission (response) content mime type
    @description Set the mime type Http header in the transmission
    @param conn HttpConn connection object created via $httpCreateConn
    @param mimeType Mime type string
    @ingroup HttpTx
 */
extern void httpSetContentType(HttpConn *conn, cchar *mimeType);

/*
    Flags for httpSetCookie
 */
#define HTTP_COOKIE_SECURE   0x1         /**< Flag for httpSetCookie for secure cookies (https only) */
#define HTTP_COOKIE_HTTP     0x2         /**< Flag for httpSetCookie for http cookies (http only) */

/** 
    Set a transmission cookie
    @description Define a cookie to send in the transmission Http header
    @param conn HttpConn connection object created via $httpCreateConn
    @param name Cookie name
    @param value Cookie value
    @param path URI path to which the cookie applies
    @param domain Domain in which the cookie applies. Must have 2-3 dots.
    @param lifespan Duration for the cookie to persist in msec
    @param flags Select from HTTP_COOKIE_SECURE for a "secure" cookie, or HTTP_COOKIE_HTTP for a "httponly" cookie.
        See RFC 2109 for details about secure and httponly cookies.
    @ingroup HttpTx
 */
extern void httpSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *domain, 
        MprTime lifespan, int flags);

/**
    Define the length of the transmission content. When static content is used for the transmission body, defining
    the entity length permits the request pipeline to know when all the data has been sent.
    @param conn HttpConn connection object created via $httpCreateConn
    @param len Transmission body length in bytes
 */
extern void httpSetEntityLength(HttpConn *conn, MprOff len);

/** 
    Set a transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup HttpTx
 */
extern void httpSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/** 
    Set a simple key/value transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value String value for the key
    @ingroup HttpTx
 */
extern void httpSetHeaderString(HttpConn *conn, cchar *key, cchar *value);

/** 
    Set a Http response status.
    @description Set the Http response status for the request. This defaults to 200 (OK).
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code.
    @ingroup HttpTx
 */
extern void httpSetStatus(HttpConn *conn, int status);

/**
    Set the responded flag for the request
    @description This call sets the requests responded status. Once status has been defined, headers generated or 
        some output has been generated, the request is regarded as having "responded" in-part to the client.
    @param conn HttpConn connection object
    @ingroup HttpTx
 */
extern void httpSetResponded(HttpConn *conn);

/**
    Indicate that the transmission socket is blocked
    @param conn Http connection object created via $httpCreateConn
    @ingroup HttpTx
 */
extern void httpSocketBlocked(HttpConn *conn);

/** 
    Wait for the connection to achieve the requested state Used for blocking client requests.
    @param conn HttpConn connection object created via $httpCreateConn
    @param state HTTP_STATE_XXX to wait for.
    @param timeout Timeout in milliseconds to wait 
    @return "Zero" if successful. Otherwise return a negative MPR error code. Specific returns include:
        MPR_ERR_TIMEOUT and MPR_ERR_BAD_STATE.
    @ingroup HttpTx
 */
extern int httpWait(HttpConn *conn, int state, MprTime timeout);

/** 
    Write the transmission headers
    @description Write the Http transmission headers into the given packet. This should only be called by connectors
        just prior to sending output to the client. It should be delayed as long as possible if the content length is
        not yet known to give the pipeline a chance to determine the transmission length. This way, a non-chunked 
        transmission can be sent with a content-length header. This is the fastest HTTP transmission.
    @param conn HttpConn connection object created via $httpCreateConn
    @param packet Packet into which to place the headers
    @ingroup HttpTx
 */
extern void httpWriteHeaders(HttpConn *conn, HttpPacket *packet);

/** 
    Write Http upload body data
    @description Write files and form fields as request body data. This will use transfer chunk encoding. This routine 
        will block until all the buffer is written even if a callback is defined.
    @param conn Http connection object created via $httpCreateConn
    @param fileData List of string file names to upload
    @param formData List of strings containing "key=value" pairs. The form data should be already www-urlencoded.
    @return Number of bytes successfully written.
    @ingroup HttpConn
 */
extern ssize httpWriteUploadData(HttpConn *conn, MprList *formData, MprList *fileData);

/********************************* HttpEndpoint ***********************************/
/*  
    Endpoint flags
 */
#define HTTP_NAMED_VHOST    0x1             /**< Using named virtual hosting */

/** 
    Listening endpoints. Endpoints may have multiple virtual named hosts.
    @stability Evolving
    @defgroup HttpEndpoint HttpEndpoint
    @see HttpEndpoint httpAcceptConn httpAddHostToEndpoint httpCreateConfiguredEndpoint httpCreateEndpoint 
        httpDestroyEndpoint httpGetEndpointContext httpHasNamedVirtualHosts httpIsEndpointAsync
        httpLookupHostOnEndpoint httpSecureEndpoint httpSecureEndpointByName httpSetEndpointAddress 
        httpSetEndpointAsync httpSetEndpointContext httpSetEndpointNotifier httpSetHasNamedVirtualHosts 
        httpStartEndpoint httpStopEndpoint httpValidateLimits 
 */
typedef struct HttpEndpoint {
    Http            *http;                  /**< Http service object */
    MprList         *hosts;                 /**< List of host objects */
    HttpLimits      *limits;                /**< Alias for first host, default route resource limits */
    MprWaitHandler  *waitHandler;           /**< I/O wait handler */
    MprHash         *clientLoad;            /**< Table of active client IPs and connection counts */
    char            *ip;                    /**< Listen IP address. May be null if listening on all interfaces. */
    int             port;                   /**< Listen port */
    int             async;                  /**< Listening is in async mode (non-blocking) */
    int             clientCount;            /**< Count of current active clients */
    int             requestCount;           /**< Count of current active requests */
    int             flags;                  /**< Endpoint control flags */
    void            *context;               /**< Embedding context */
    MprSocket       *sock;                  /**< Listening socket */
    MprDispatcher   *dispatcher;            /**< Event dispatcher */
    HttpNotifier    notifier;               /**< Default connection notifier callback */
    struct MprSsl   *ssl;                   /**< Endpoint SSL configuration */
} HttpEndpoint;

#define HTTP_NOTIFY(conn, state, flags) \
    if (1) { \
        if (conn->notifier) { \
            (conn->notifier)(conn, state, flags); \
        } \
    } else \

/**
    Accept a new connection.
    Accept a new client connection on a new socket. If multithreaded, this will come in on a worker thread 
        dedicated to this connection. This is called from the listen wait handler.
    @param endpoint The endpoint on which the server was listening
    @param event Mpr event object
    @return A HttpConn object representing the new connection.
    @internal
 */
extern HttpConn *httpAcceptConn(HttpEndpoint *endpoint, MprEvent *event);

/**
    Add a host to an endpoint
    @description Add the host to the endpoint's list  of hosts. A listening endpoint may have multiple
        virutal hosts.
    @param endpoint Endpoint to which the host will be added.
    @param host HttpHost object to add.
    @return "Zero" if the host can be added.
    @ingroup HttpEndpoint
 */
extern void httpAddHostToEndpoint(HttpEndpoint *endpoint, struct HttpHost *host);

/**
    Create and configure a new endpoint.
    @description Convenience function to create and configure a new endpoint without using a config file.
        An endpoint is created with a default host and default route.
    @param home Home directory for configuration files for the endpoint 
    @param documents Directory containing the 
    @param ip IP address to use for the endpoint. Set to null to listen on all interfaces.
    @param port Listening port number to use for the endpoint
    @return A configured HttpEndpoint object instance
*/
extern HttpEndpoint *httpCreateConfiguredEndpoint(cchar *home, cchar *documents, cchar *ip, int port);

/** 
    Create an endpoint  object.
    @description Creates a listening endpoint on the given IP:PORT. Use httpStartEndpoint to begin listening for client
        connections.
    @param ip IP address on which to listen
    @param port IP port number
    @param dispatcher Dispatcher to use. Can be null.
    @ingroup HttpEndpoint
 */
extern HttpEndpoint *httpCreateEndpoint(cchar *ip, int port, MprDispatcher *dispatcher);

/**
    Destroy the endpoint
    @description This destroys the endpoint created by #httpCreateEndpoint. Calling this routine should not
        normally be necessary as the garbage collector will invoke as required.
    @param endpoint HttpEndpoint object returned from #httpCreateEndpoint.
    @ingroup HttpEndpoint
 */
extern void httpDestroyEndpoint(HttpEndpoint *endpoint);

/**
    Get the endpoint context object
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @return The endpoint context object defined via httpSetEndpointContext
 */
extern void *httpGetEndpointContext(HttpEndpoint *endpoint);

/**
    Test if an endpoint has named virtual hosts.
    @param endpoint Endpoint object to examine
    @return True if the endpoint has named virutal hosts.
    @ingroup HttpEndpoint
 */
extern bool httpHasNamedVirtualHosts(HttpEndpoint *endpoint);

/**
    Get if the endpoint is running in asynchronous mode
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @return True if the endpoint is in async mode
 */
extern int httpIsEndpointAsync(HttpEndpoint *endpoint);

/**
    Lookup a host name
    @description Lookup a host by name in the set of defined hosts for this endpoint.
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @param name Host name to search for
    @return An HttpHost object instance or null if the host cannot be found.
 */
extern struct HttpHost *httpLookupHostOnEndpoint(HttpEndpoint *endpoint, cchar *name);

/**
    Secure an endpoint 
    @description Define the SSL parameters for an endpoint. This must be done before starting listening on
        the endpoint via #httpStartEndpoint.
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @param ssl MprSsl object
    @returns "Zero" if successful, otherwise a negative MPR error code.
 */
extern int httpSecureEndpoint(HttpEndpoint *endpoint, struct MprSsl *ssl);

/**
    Secure an endpoint by name
    @description Define the SSL parameters for an endpoint that is selected by name. This must be done before 
    starting listening on the endpoint via #httpStartEndpoint.
    @param name Endpoint name. The endpoint name is comprised of the IP and port. For example: "127.0.0.1:7777" 
    @param ssl MprSsl object
    @returns Zero if successful, otherwise a negative MPR error code.
 */
extern int httpSecureEndpointByName(cchar *name, struct MprSsl *ssl);

/**
    Set the endpoint IP address
    @description This call defines the endpoint's IP address and port number. If the endpoint has already been
        started, this will stop and restart the endpoint. Current requests will not be disturbed.
        This is useful to modify the endpoints address when using dynamically assigned IP addresses.
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @param ip IP address to use for the endpoint. Set to null to listen on all interfaces.
    @param port Listening port number to use for the endpoint
    @ingroup HttpRx
 */
extern void httpSetEndpointAddress(HttpEndpoint *endpoint, cchar *ip, int port);

/**
    Control if the endpoint is running in asynchronous mode
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @param enable Set to 1 to enable async mode.
 */
extern void httpSetEndpointAsync(HttpEndpoint *endpoint, int enable);

/**
    Set the endpoint context object
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @param context New context object
 */
extern void httpSetEndpointContext(HttpEndpoint *endpoint, void *context);

/** 
    Define a notifier callback for this endpoint.
    @description The notifier callback will be invoked as Http requests are processed.
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @param fn Notifier function. 
    @ingroup HttpConn
 */
extern void httpSetEndpointNotifier(HttpEndpoint *endpoint, HttpNotifier fn);

/**
    Control whether the endpoint has named virtual hosts.
    @param endpoint Endpoint object to examine
    @param on Set to true to enable named virtual hosts
    @ingroup HttpEndpoint
 */
extern void httpSetHasNamedVirtualHosts(HttpEndpoint *endpoint, bool on);

/** 
    Start listening for client connections.
    @description Opens the endpoint socket and starts listening for connections.
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @returns "Zero" if successful, otherwise a negative MPR error code.
    @ingroup HttpEndpoint
 */
extern int httpStartEndpoint(HttpEndpoint *endpoint);

/** 
    Stop the server listening for client connections.
    @description Closes the socket endpoint.
    @param endpoint HttpEndpoint object created via #httpCreateEndpoint
    @ingroup HttpEndpoint
 */
extern void httpStopEndpoint(HttpEndpoint *endpoint);

/**
    Validate the request against defined resource limits
    @description The Http library supports a suite of resource limits that restrict the impact of a request on
        the system. This call validates a processing event for the current request against the server's endpoint
        limits.
    @param endpoint The endpoint on which the server was listening
    @param event Processing event. The supported events are: HTTP_VALIDATE_OPEN_CONN, HTTP_VALIDATE_CLOSE_CONN,
        HTTP_VALIDATE_OPEN_REQUEST and HTTP_VALIDATE_CLOSE_REQUEST.
    @param conn HttpConn connection object
    @return True if the request can be successfully validated against the endpoint limits.
    @ingroup HttpRx
 */
extern bool httpValidateLimits(HttpEndpoint *endpoint, int event, HttpConn *conn);

/********************************** HttpHost ***************************************/
/*
    Flags
 */
#define HTTP_HOST_VHOST         0x1         /**< Host flag to signify host is a virtual host */
#define HTTP_HOST_NAMED_VHOST   0x2         /**< Host flag for a named virtual host */
#define HTTP_HOST_NO_TRACE      0x10        /**< Host flag to disable the of TRACE HTTP method */

/**
    Host Object
    @description A Host object represents a logical host. Several logical hosts may share a single HttpEndpoint.
    @stability Evolving
    @defgroup HttpHost HttpHost
    @see HttpHost httpAddRoute httpCloneHost httpCreateHost httpResetRoutes httpSetHostHome httpSetHostIpAddr 
        httpSetHostName httpSetHostProtocol
*/
typedef struct HttpHost {
    /*
        NOTE: the ip:port names are used for vhost matching when there is only one such address. Otherwise a host may
        be associated with multiple listening endpoints. In that case, the ip:port will store only one of these addresses 
        and will not be used for matching.
     */
    char            *name;                  /**< Host name */
    char            *ip;                    /**< Hostname/ip portion parsed from name */
    int             port;                   /**< Port address portion parsed from name */
    struct HttpHost *parent;                /**< Parent host to inherit aliases, dirs, routes */
    MprCache        *responseCache;         /**< Response content caching store */
    MprList         *routes;                /**< List of Route defintions */
    HttpRoute       *defaultRoute;          /**< Default route for the host */
    char            *home;                  /**< Directory for configuration files */
    char            *protocol;              /**< Defaults to "HTTP/1.1" */
    int             flags;                  /**< Host flags */
    MprMutex        *mutex;                 /**< Multithread sync */
} HttpHost;

/**
    Add an option to the options table
    @param options Option table returned from httpGetOptions
    @param field Field key name
    @param value Value to use for the field
 */
extern void httpAddOption(MprHash *options, cchar *field, cchar *value);

/**
    Add a route to a host
    @description Add the route to the host list of routes. During request route matching, routes are processed 
    in order, so it is important to define routes in the order in which you wish to match them.
    @param host HttpHost object
    @param route Route to add
    @return "Zero" if the route can be added.
    @ingroup HttpHost
 */
extern int httpAddRoute(HttpHost *host, HttpRoute *route);

/**
    Clone a host
    @description The parent host is cloned and a new host returned. The new host inherites the parent's configuration.
    @param parent Parent HttpHost object to clone
    @return The new HttpHost object.
    @ingroup HttpHost
 */
extern HttpHost *httpCloneHost(HttpHost *parent);

/**
    Create a host
    @description Create a new host object. The host is added to the Http service's list of hosts.
    @param home directory for the host's configuration files.
    @return The new HttpHost object.
    @ingroup HttpHost
 */
extern HttpHost *httpCreateHost(cchar *home);

/**
    Define a route
    @description This creates a route and then configures it using the given parameters. The route is finalized and
        added to the parent host.
    @param parent Parent route from which to inherit configuration.
    @param name Route name to define.
    @param methods Http methods for which this route is active
    @param pattern Matching URI pattern for which this route will qualify
    @param target Route target string expression. This is used by handlers to determine the physical or virtual resource
        to serve.
    @param source Source file pattern containing the resource to activate or serve. 
    @return Created route.
    @ingroup HttpRoute
 */
HttpRoute *httpDefineRoute(HttpRoute *parent, cchar *name, cchar *methods, cchar *pattern, cchar *target, cchar *source);

/**
    Return the default route for a host
    @description The host has a default route which holds default configuration. Typically the default route
        is not directly used when routing URIs. Rather other routes inherit from the default route and are used to 
        respond to client requests.
    @param host Host to examine.
    @return Default route object
    @ingroup HttpRoute
 */
extern HttpRoute *httpGetHostDefaultRoute(HttpHost *host);

/**
    Extract a field value from an option string. 
    @param options Option string of the form: "field='value' field='value'..."
    @param field Field key name
    @param defaultValue Value to use if "field" is not found in options
    @return Allocated value string.
 */
extern cchar *httpGetOption(MprHash *options, cchar *field, cchar *defaultValue);

/**
    Get an option value that is itself an object (hash)
    @description This returns an option value that is an instance of MprHash. When deserializing a JSON option string which
    contains multiple levels, this routine can be used to extract lower option container values. 
    @param options Options object to examine.
    @param field Property to return.
    @return An MprHash instance for the given field. This will contain option sub-properties.
    @ingroup HttpRoute
 */
extern MprHash *httpGetOptionHash(MprHash *options, cchar *field);

/**
    Convert an options string into an options table
    @param options Option string of the form: "{field:'value', field:'value'}"
        This is a sub-set of the JSON syntax. Arrays are not supported.
    @return Options table
 */
extern MprHash *httpGetOptions(cchar *options);

/**
    Get a path extension 
    @param path File pathname to examine
    @return The path extension sans "."
  */
extern char *httpGetPathExt(cchar *path);

/**
    Show the current route table to the error log.
    @description This emits the currently defined route table for a host to the route table. If the "full" argument is true,
        a more-complete, multi-line output format will be used. Othewise, a one-line, abbreviated route description will
        be output.
    @param host Host to examine.
    @param full Set to true for a "fuller" output route description.
    @ingroup HttpRoute
 */
extern void httpLogRoutes(HttpHost *host, bool full);

/**
    Lookup a route by name
    @param host HttpHost object owning the route table
    @param name Route name to find. If null or empty, look for "default"
 */
extern HttpRoute *httpLookupRoute(HttpHost *host, cchar *name);

/**
    Remove an option
    @description Remove a property from an options hash
    @param options Options table returned from httpGetOptions
    @param field Property field to remove
    @ingroup HttpRoute
 */
extern void httpRemoveOption(MprHash *options, cchar *field);

/**
    Reset the list of routes for the host
    @param host HttpHost object
    @ingroup HttpHost
 */
extern void httpResetRoutes(HttpHost *host);

/**
    Set the default route for a host
    @description The host has a default route which holds default configuration. Typically the default route
        is not directly used when routing URIs. Rather other routes inherit from the default route and are used to 
        respond to client requests.
    @param host Host to examine.
    @param route Route to define as the default
    @ingroup HttpRoute
 */
extern void httpSetHostDefaultRoute(HttpHost *host, HttpRoute *route);

/**
    Set the home directory for a host
    @description The home directory is used by some host and route components to locate configuration files.
    @param host HttpHost object
    @param dir Directory path for the host home
    @ingroup HttpHost
 */
extern void httpSetHostHome(HttpHost *host, cchar *dir);

/**
    Set the host internet address
    @description Set the host IP and port address.
    @param host HttpHost object
    @param ip Internet address. This can be an IP address or a symbolic domain and host name.
    @param port Port number 
    @return "Zero" if the route can be added.
    @ingroup HttpHost
 */
extern void httpSetHostIpAddr(HttpHost *host, cchar *ip, int port);

/**
    Set the host name
    @description The host name is used when matching virtual hosts with the Http Host header. The host name is also
        used for some redirections.
    in order, so it is important to define routes in the order in which you wish to match them.
    @param host HttpHost object
    @param name Host name to use
    @return Zero if the route can be added.
    @ingroup HttpHost
 */
extern void httpSetHostName(HttpHost *host, cchar *name);

/**
    Set the host HTTP protocol version
    @description Set the host protocol version to either HTTP/1.0 or HTTP/1.1
    @param host HttpHost object
    @param protocol Set to either HTTP/1.0 or HTTP/1.1
    @ingroup HttpHost
 */
extern void httpSetHostProtocol(HttpHost *host, cchar *protocol);

/**
    Set an option
    @description Set a property in an options hash
    @param options Options table returned from httpGetOptions
    @param field Property field to set
    @param value Property value to use
    @ingroup HttpRoute
 */
extern void httpSetOption(MprHash *options, cchar *field, cchar *value);

/*
    Internal
 */
extern int httpStartHost(HttpHost *host);
extern void httpStopHost(HttpHost *host);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _h_HTTP */

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
    details at: http: *embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http: *embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

