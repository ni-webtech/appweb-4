
/******************************************************************************/
/* 
    This file is an amalgamation of all the individual source code files for the
    Http Library Header.
  
    Catenating all the source into a single file makes embedding simpler and
    the resulting application faster, as many compilers can do whole file
    optimization.
  
    If you want to modify the product, you can still get the whole source as 
    individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "../src/include/http.h"
 */
/************************************************************************/

/*
    http.h -- Header for the Embedthis Http Library.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

#ifndef _h_HTTP
#define _h_HTTP 1


#include    "mpr.h"


#ifdef __cplusplus
extern "C" {
#endif

#if !DOXYGEN
struct Http;
struct HttpAuth;
struct HttpConn;
struct HttpLoc;
struct HttpPacket;
struct HttpLimits;
struct HttpQueue;
struct HttpRx;
struct HttpServer;
struct HttpStage;
struct HttpTx;
struct MaAlias;
struct MaDir;
#endif


#define HTTP_DEFAULT_PORT 80

#ifndef HTTP_NAME
#define HTTP_NAME "Embedthis-http/" BLD_VERSION
#endif

#if BLD_TUNE == MPR_TUNE_SIZE || DOXYGEN
    /*  
        Tune for size
     */
    #define HTTP_BUFSIZE               (4 * 1024)            /**< Default I/O buffer size */
    #define HTTP_MAX_CHUNK             (8 * 1024)            /**< Max chunk size for transfer chunk encoding */
    #define HTTP_MAX_HEADERS           2048                  /**< Max size of the headers */
    #define HTTP_MAX_IOVEC             16                    /**< Number of fragments in a single socket write */
    #define HTTP_MAX_NUM_HEADERS       20                    /**< Max number of header lines */
    #define HTTP_MAX_RECEIVE_BODY      (128 * 1024 * 1024)   /**< Maximum incoming body size */
    #define HTTP_MAX_REQUESTS          20                    /**< Max concurrent requests */
    #define HTTP_MAX_CLIENTS           10                    /**< Max concurrent client endpoints */
    #define HTTP_MAX_SESSIONS          100                   /**< Max concurrent sessions */
    #define HTTP_MAX_STAGE_BUFFER      (16 * 1024)           /**< Max buffer for any stage */
    #define HTTP_CLIENTS_HASH          (131)                 /**< Hash table for client IP addresses */

#elif BLD_TUNE == MPR_TUNE_BALANCED
    /*  
        Tune balancing speed and size
     */
    #define HTTP_BUFSIZE               (8 * 1024)
    #define HTTP_MAX_CHUNK             (8 * 1024)
    #define HTTP_MAX_HEADERS           (8 * 1024)
    #define HTTP_MAX_IOVEC             24
    #define HTTP_MAX_NUM_HEADERS       40
    #define HTTP_MAX_RECEIVE_BODY      (128 * 1024 * 1024)
    #define HTTP_MAX_REQUESTS          50
    #define HTTP_MAX_CLIENTS           25
    #define HTTP_MAX_SESSIONS          500
    #define HTTP_MAX_STAGE_BUFFER      (32 * 1024)
    #define HTTP_CLIENTS_HASH          (257)

#else
    /*  
        Tune for speed
     */
    #define HTTP_BUFSIZE               (16 * 1024)
    #define HTTP_MAX_CHUNK             (16 * 1024) 
    #define HTTP_MAX_HEADERS           (8 * 1024)
    #define HTTP_MAX_IOVEC             32
    #define HTTP_MAX_NUM_HEADERS       256
    #define HTTP_MAX_RECEIVE_BODY      (256 * 1024 * 1024)
    #define HTTP_MAX_REQUESTS          1000
    #define HTTP_MAX_CLIENTS           500
    #define HTTP_MAX_SESSIONS          5000
    #define HTTP_MAX_STAGE_BUFFER      (64 * 1024)
    #define HTTP_CLIENTS_HASH          (1009)
#endif

#define HTTP_MAX_TRANSMISSION_BODY (INT_MAX)             /**< Max buffer for response data */
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

#define HTTP_INACTIVITY_TIMEOUT   (60  * 1000)      /**< Keep connection alive timeout */
#define HTTP_SESSION_TIMEOUT      (3600 * 1000)     /**< One hour */

#define HTTP_DATE_FORMAT          "%a, %d %b %Y %T GMT"

/*  
    Hash sizes (primes work best)
 */
#define HTTP_SMALL_HASH_SIZE      31                /* Small hash (less than the alphabet) */
#define HTTP_MED_HASH_SIZE        61                /* Medium */
#define HTTP_LARGE_HASH_SIZE      101               /* Large */

/*
    Standard HTTP/1.1 status codes
 */
#define HTTP_CODE_CONTINUE                  100
#define HTTP_CODE_OK                        200
#define HTTP_CODE_CREATED                   201
#define HTTP_CODE_ACCEPTED                  202
#define HTTP_CODE_NOT_AUTHORITATIVE         203
#define HTTP_CODE_NO_CONTENT                204
#define HTTP_CODE_RESET                     205
#define HTTP_CODE_PARTIAL                   206
#define HTTP_CODE_MOVED_PERMANENTLY         301
#define HTTP_CODE_MOVED_TEMPORARILY         302
#define HTTP_CODE_NOT_MODIFIED              304
#define HTTP_CODE_USE_PROXY                 305
#define HTTP_CODE_TEMPORARY_REDIRECT        307
#define HTTP_CODE_BAD_REQUEST               400
#define HTTP_CODE_UNAUTHORIZED              401
#define HTTP_CODE_PAYMENT_REQUIRED          402
#define HTTP_CODE_FORBIDDEN                 403
#define HTTP_CODE_NOT_FOUND                 404
#define HTTP_CODE_BAD_METHOD                405
#define HTTP_CODE_NOT_ACCEPTABLE            406
#define HTTP_CODE_REQUEST_TIMEOUT           408
#define HTTP_CODE_CONFLICT                  409
#define HTTP_CODE_GONE                      410
#define HTTP_CODE_LENGTH_REQUIRED           411
#define HTTP_CODE_REQUEST_TOO_LARGE         413
#define HTTP_CODE_REQUEST_URL_TOO_LARGE     414
#define HTTP_CODE_UNSUPPORTED_MEDIA_TYPE    415
#define HTTP_CODE_RANGE_NOT_SATISFIABLE     416
#define HTTP_CODE_EXPECTATION_FAILED        417
#define HTTP_CODE_INTERNAL_SERVER_ERROR     500
#define HTTP_CODE_NOT_IMPLEMENTED           501
#define HTTP_CODE_BAD_GATEWAY               502
#define HTTP_CODE_SERVICE_UNAVAILABLE       503
#define HTTP_CODE_GATEWAY_TIMEOUT           504
#define HTTP_CODE_BAD_VERSION               505
#define HTTP_CODE_INSUFFICIENT_STORAGE      507

/*
    Proprietary HTTP status codes
 */
#define HTTP_CODE_START_LOCAL_ERRORS        550
#define HTTP_CODE_COMMS_ERROR               550
#define HTTP_CODE_CLIENT_ERROR              551
#define HTTP_CODE_LIMIT_ERROR               552

typedef int (*HttpRangeProc)(struct HttpQueue *q, struct HttpPacket *packet);
typedef cchar *(*HttpGetPassword)(struct HttpAuth *auth, cchar *realm, cchar *user);
typedef bool (*HttpValidateCred)(struct HttpAuth *auth, cchar *realm, char *user, cchar *pass, cchar *required, char **msg);
typedef void (*HttpNotifier)(struct HttpConn *conn, int state, int flags);

/** 
    Http service object
    The Http service is managed by a single service object.
    @stability Evolving
    @defgroup Http Http
    @see Http HttpConn HttpServer httpCreate httpCreateSecret httpGetContext httpGetDateString httpSetContext
    httpSetDefaultHost httpSetDefaultPort httpSetProxy
 */
typedef struct Http {
    MprList         *connections;           /**< Currently open connection requests */
    MprHashTable    *stages;                /**< Possible stages in connection pipelines */
    MprHashTable    *mimeTypes;             /**< Mime Types */
    MprHashTable    *statusCodes;           /**< Http status codes */

    /*  
        Some standard pipeline stages
     */
    struct HttpStage *netConnector;         /**< Default network connector */
    struct HttpStage *sendConnector;        /**< Optimized sendfile connector */
    struct HttpStage *authFilter;           /**< Authorization filter (digest and basic) */
    struct HttpStage *rangeFilter;          /**< Ranged requests filter */
    struct HttpStage *chunkFilter;          /**< Chunked transfer encoding filter */
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
    struct HttpLoc *clientLocation;         /**< Default location block for clients */

    MprEvent        *timer;                 /**< Admin service timer */
    MprTime         now;                    /**< When was the currentDate last computed */
    MprMutex        *mutex;
    HttpGetPassword getPassword;            /**< Lookup password callback */
    HttpValidateCred validateCred;          /**< Validate user credentials callback */
    MprForkCallback forkCallback;
    void            *forkData;

    int             connCount;              /**< Count of connections for Conn.seqno */
    void            *context;               /**< Embedding context */
    char            *currentDate;           /**< Date string for HTTP response headers */
    char            *expiresDate;           /**< Convenient expiry date (1 day in advance) */
    char            *secret;                /**< Random bytes for authentication */
    char            *defaultHost;           /**< Default ip address */
    int             defaultPort;            /**< Default port */
    char            *protocol;              /**< HTTP/1.0 or HTTP/1.1 */
    char            *proxyHost;             /**< Proxy ip address */
    int             proxyPort;              /**< Proxy port */
    int             sslLoaded;              /**< True when the SSL provider has been loaded */

    void            (*rangeService)(struct HttpQueue *q, HttpRangeProc fill);
} Http;

/**
    Create a Http connection object
    @description Create a new http connection object. This creates an object that can be initialized and then
        used with mprHttpRequest. Destroy with mprFree.
    @return A newly allocated HttpConn structure. Caller must free using mprFree.
    @ingroup Http
 */
extern Http *httpCreate();
extern void httpDestroy(Http *http);

/**
    Create a Http client
    @param http HttpConn object created via $httpCreateConn
    @param dispatcher Dispatcher to use. Can be null.
    @return Returns a HttpConn connection object
 */
extern struct HttpConn *httpCreateClient(Http *http, MprDispatcher *dispatcher);

/**  
    Create the Http secret data for crypto
    @description Create http secret data that is used to seed SSL based communications.
    @param http Http service object.
    @return Zero if successful, otherwise a negative MPR error code
    @ingroup Http
 */
extern int httpCreateSecret(Http *http);

/**
    Enable use of the TRACE Http method
    @param http Http service object.
    @param on Set to 1 to enable
 */
extern void httpEnableTraceMethod(struct HttpLimits *limits, bool on);

/**
    Get the http context object
    @param http Http service object.
    @return The http context object defined via httpSetContext
 */
extern void *httpGetContext(Http *http);

/**
    Get the time as an ISO date string
    @param sbuf Optional path buffer. If supplied, the modified time of the path is used. If NULL, then the current
        time is used.
    @return RFC822 formatted date string. Caller must free.
 */
extern char *httpGetDateString(MprPath *sbuf);

/**
    Set the http context object
    @param http Http object created via #httpCreate
    @param context New context object
 */
extern void httpSetContext(Http *http, void *context);

/** 
    Define a default host
    @description Define a default host to use for connections if the URI does not specify a host
    @param http Http object created via $httpCreateConn
    @param host Host or IP address
    @ingroup HttpConn
 */
extern void httpSetDefaultHost(Http *http, cchar *host);

/** 
    Define a default port
    @description Define a default port to use for connections if the URI does not define a port
    @param http Http object created via $httpCreateConn
    @param port Integer port number
    @ingroup HttpConn
 */
extern void httpSetDefaultPort(Http *http, int port);

/** 
    Define a Http proxy host to use for all client connect requests.
    @description Define a http proxy host to communicate via when accessing the net.
    @param http Http object created via #httpCreate
    @param host Proxy host name or IP address
    @param port Proxy host port number.
    @ingroup Http
 */
extern void httpSetProxy(Http *http, cchar *host, int port);

/* Internal APIs */
extern void httpAddConn(Http *http, struct HttpConn *conn);
extern void httpRemoveConn(Http *http, struct HttpConn *conn);
extern cchar *httpLookupStatus(Http *http, int status);
extern void httpSetForkCallback(Http *http, MprForkCallback callback, void *data);

/** 
    Http limits
    @stability Evolving
    @defgroup HttpLimits HttpLimits
    @see HttpLimits
 */
typedef struct HttpLimits {
    ssize   chunkSize;              /**< Max chunk size for transfer encoding */
    ssize   headerSize;             /**< Max size of the total header */
    ssize   receiveBodySize;        /**< Max size of receive body data */
    ssize   stageBufferSize;        /**< Max buffering by any pipeline stage */
    ssize   transmissionBodySize;   /**< Max size of transmission body content */
    ssize   uploadSize;             /**< Max size of an uploaded file */
    ssize   uriSize;                /**< Max size of a uri */

    int     clientCount;            /**< Max number of simultaneous clients endpoints */
    int     headerCount;            /**< Max number of header lines */
    int     keepAliveCount;         /**< Max number of keep alive requests to perform per socket */
    int     requestCount;           /**< Max number of simultaneous concurrent requests */
    int     sessionCount;           /**< Max number of opened session state stores */

    int     inactivityTimeout;      /**< Default timeout for keep-alive and idle requests (msec) */
    int     requestTimeout;         /**< Default time a request can take (msec) */
    int     sessionTimeout;         /**< Default time a session can persist (msec) */

    int     enableTraceMethod;      /**< Trace method enabled */
} HttpLimits;

extern void httpInitLimits(HttpLimits *limits, int serverSide);
extern HttpLimits *httpCreateLimits(int serverSide);

/** 
    URI management
    @description The HTTP provides routines for formatting and parsing URIs. Routines are also provided
        to escape dangerous characters for URIs as well as HTML content and shell commands.
    @stability Evolving
    @see HttpConn, httpCreateUri, httpCreateUriFromParts, httpFormatUri, httpNormalizeUriPath, httpLookupMimeType
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
    int         flags;                  /** Flags */
    int         secure;                 /** Using https */
    char        *uri;                   /**< Original URI (not decoded) */
} HttpUri;

/*  
    Character escaping masks
 */
#define HTTP_ESCAPE_HTML            0x1
#define HTTP_ESCAPE_SHELL           0x2
#define HTTP_ESCAPE_URL             0x4

/** 
    Create and initialize a URI.
    @description Parse a uri and return a tokenized HttpUri structure.
    @param uri Uri string to parse
    @param complete Add missing components. ie. Add scheme, host and port if not supplied. 
    @return A newly allocated HttpUri structure. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern HttpUri *httpCreateUri(cchar *uri, int complete);

extern HttpUri *httpCloneUri(HttpUri *base, int complete);

/** 
    Format a URI
    @description Format a URI string using the input components.
    @param scheme Protocol string for the uri. Example: "http"
    @param host Host or IP address
    @param port TCP/IP port number
    @param path URL path
    @param ref URL reference fragment
    @param query Additiona query parameters.
    @param complete Add missing elements. For example, if scheme is null, then add "http".
    @return A newly allocated uri string. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern char *httpFormatUri(cchar *scheme, cchar *host, int port, cchar *path, cchar *ref, cchar *query, 
        int complete);

extern HttpUri *httpCreateUriFromParts(cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, 
        cchar *query, int complete);

/** 
    Get the mime type for an extension.
    This call will return the mime type from a limited internal set of mime types for the given path or extension.
    @param ext Path or extension to examine
    @returns Mime type. This is a static string. Caller must not free.
 */
extern cchar *httpLookupMimeType(cchar *ext);

/** 
    Convert a Uri to a string.
    @description Convert the given Uri to a string, optionally completing missing parts such as the host, port and path.
    @param uri A Uri object created via httpCreateUri 
    @param complete Fill in missing parts of the uri
    @return A newly allocated uri string. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern char *httpUriToString(HttpUri *uri, int complete);

/** 
    Validate a URL
    @description Validate and canonicalize a URL. This removes redundant "./" sequences and simplifies "../dir" references. 
    @param uri Uri path string to normalize
    @return A new validated uri string. Caller must free.
    @ingroup HttpUri
 */
extern char *httpNormalizeUriPath(cchar *uri);
extern void httpNormalizeUri(HttpUri *uri);

extern HttpUri *httpJoinUri(HttpUri *uri, int argc, HttpUri **others);
extern HttpUri *httpJoinUriPath(HttpUri *uri, HttpUri *base, HttpUri *other);
extern HttpUri *httpCompleteUri(HttpUri *uri, HttpUri *missing);
extern HttpUri *httpGetRelativeUri(HttpUri *base, HttpUri *target, int dup);
extern HttpUri *httpResolveUri(HttpUri *base, int argc, HttpUri **others, int local);
extern HttpUri *httpMakeUriLocal(HttpUri *uri);

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
    ssize               start;                  /**< Start of range */
    ssize               end;                    /**< End byte of range + 1 */
    ssize               len;                    /**< Redundant range length */
    struct HttpRange    *next;                  /**< Next range */
} HttpRange;

/*  
    Packet flags
 */
#define HTTP_PACKET_HEADER    0x1               /**< Packet contains HTTP headers */
#define HTTP_PACKET_RANGE     0x2               /**< Packet is a range boundary packet */
#define HTTP_PACKET_DATA      0x4               /**< Packet contains actual content data */
#define HTTP_PACKET_END       0x8               /**< End of stream packet */

/** 
    Packet object. 
    @description The request/response pipeline sends data and control information in HttpPacket objects. The output
        stream typically consists of a HEADER packet followed by zero or more data packets and terminated by an END
        packet. If the request has input data, the input stream is consists of one or more data packets followed by
        an END packet.
        \n\n
        Packets contain data and optional prefix or suffix headers. Packets can be split, joined, filled or emptied. 
        The pipeline stages will fill or transform packet data as required.
    @stability Evolving
    @defgroup HttpPacket HttpPacket
    @see HttpPacket HttpQueue httpCreatePacket, ttphttpCreateDataPacket httpCreateEndPacket 
        httpJoinPacket httpSplitPacket httpGetPacketLength httpCreateHeaderPacket httpGetPacket
        httpJoinPacketForService httpPutForService httpIsPacketTooBig httpSendPacket httpPutBackPacket 
        httpSendPacketToNext httpResizePacket
 */
typedef struct HttpPacket {
    MprBuf          *prefix;                /**< Prefix message to be emitted before the content */
    MprBuf          *content;               /**< Chunk content */
    MprBuf          *suffix;                /**< Prefix message to be emitted after the content */
    ssize           entityLength;           /**< Entity length. Content is null. */
    int             flags;                  /**< Packet flags */
    struct HttpPacket *next;                /**< Next packet in chain */
} HttpPacket;


/** 
    Create a data packet
    @description Create a packet of the required size.
    @param size Size of the package data storage.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreatePacket(ssize size);
extern HttpPacket *httpDup(HttpPacket *orig);

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
    Create a response header packet
    @description Create a response header packet and set the HTTP_PACKET_HEADER flag. 
        A header packet is used by the pipeline to hold the response headers.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateHeaderPacket();

/** 
    Join two packets
    @description Join the contents of one packet to another by copying the data from the \a other packet into 
        the first packet. 
    @param packet Destination packet
    @param other Other packet to copy data from.
    @return Zero if successful, otherwise a negative Mpr error code
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
    @param offset Location in the original packet at which to split
    @return New HttpPacket object containing the data after the offset. No need to free, unless you have a very long
        running request. Otherwise the packet memory will be released automatically when the request completes.
    @ingroup HttpPacket
 */
extern HttpPacket *httpSplitPacket(HttpPacket *packet, ssize offset);

#if DOXYGEN
/** 
    Get the length of the packet data contents
    @description Get the content length of a packet. This does not include the prefix or suffix data length -- just
        the pure data contents.
    @param packet Packet to examine.
    @return Count of bytes contained by the packet.
    @ingroup HttpPacket
 */
extern int httpGetPacketLength(HttpPacket *packet);
#else
#define httpGetPacketLength(p) (p->content ? mprGetBufLength(p->content) : p->entityLength)
#endif

/** 
    Get the next packet from a queue
    @description Get the next packet. This will remove the packet from the queue and adjust the queue counts
        accordingly. If the queue was full and upstream queues were blocked, they will be enabled.
    @param q Queue reference
    @return The packet removed from the queue.
    @ingroup HttpQueue
 */
extern HttpPacket *httpGetPacket(struct HttpQueue *q);

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
    Test if a packet is too big 
    @description Test if a packet is too big to fit downstream. If the packet content exceeds the downstream queue's 
        maximum or exceeds the downstream queue's requested packet size -- then this routine will return true.
    @param q Queue reference
    @param packet Packet to test
    @return True if the packet is too big for the downstream queue
    @ingroup HttpQueue
 */
extern bool httpIsPacketTooBig(struct HttpQueue *q, HttpPacket *packet);

//  MOB -- why called SendPacket, rename back to PutPacket
/** 
    Put a packet onto a queue
    @description Put the packet onto the end of queue by calling the queue's put() method. 
    @param q Queue reference
    @param packet Packet to put
    @ingroup HttpQueue
 */
extern void httpSendPacket(struct HttpQueue *q, HttpPacket *packet);

/** 
    Put a packet back onto a queue
    @description Put the packet back onto the front of the queue. The queue's put() method is not called.
        This is typically used by the queue's service routine when a packet cannot complete processing.
    @param q Queue reference
    @param packet Packet to put back
    @ingroup HttpQueue
 */
extern void httpPutBackPacket(struct HttpQueue *q, HttpPacket *packet);

/** 
    Put a packet onto the service queue
    @description Add a packet to the service queue. If serviceQ is true, the queue will be scheduled for service.
    @param q Queue reference
    @param packet Packet to join to the queue
    @param serviceQ If true, schedule the queue for service
    @ingroup HttpQueue
 */
extern void httpPutForService(struct HttpQueue *q, HttpPacket *packet, bool serviceQ);


/** 
    Put a packet onto the next queue
    @description Put a packet onto the next downstream queue by calling the downstreams queue's put() method. 
    @param q Queue reference. The packet will not be queued on this queue, but rather on the queue downstream.
    @param packet Packet to put
    @ingroup HttpQueue
 */
extern void httpSendPacketToNext(struct HttpQueue *q, HttpPacket *packet);

/** 
    Resize a packet
    @description Resize a packet if required so that it fits in the downstream queue. This may split the packet
        if it is too big to fit in the downstream queue. If it is split, the tail portion is put back on the queue.
    @param q Queue reference
    @param packet Packet to put
    @param size If size is > 0, then also ensure the packet is not larger than this size.
    @return Zero if successful, otherwise a negative Mpr error code
    @ingroup HttpQueue
 */
extern int httpResizePacket(struct HttpQueue *q, HttpPacket *packet, ssize size);

/*  
    Queue directions
 */
#define HTTP_QUEUE_TRANS          0           /**< Send (transmit to client) queue */
#define HTTP_QUEUE_RECEIVE        1           /**< Receive (read from client) queue */
#define HTTP_MAX_QUEUE            2           /**< Number of queue types */

/* 
   Queue flags
 */
#define HTTP_QUEUE_OPEN           0x1         /**< Queue's open routine has been called */
#define HTTP_QUEUE_DISABLED       0x2         /**< Queue's service routine is disabled. Can accept data, but not send */
#define HTTP_QUEUE_FULL           0x4         /**< Queue is full */
#define HTTP_QUEUE_ALL            0x8         /**< Queue has all the data there is and will be */
#define HTTP_QUEUE_SERVICED       0x10        /**< Queue has been serviced at least once */
#define HTTP_QUEUE_EOF            0x20        /**< Queue at end of data */
#define HTTP_QUEUE_STARTED        0x40        /**< Queue started */
#define HTTP_QUEUE_RESERVICE      0x80        /**< Queue requires reservicing */

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
        and has the ability to queue and process data, manage flow control and schedule packets for service.
        \n\n
        Queue's provide open, close, put and service methods. These methods manage and respond to incoming packets.
        A queue can respond immediately to an incoming packet by processing or dispatching a packet in its put() method.
        Alternatively, the queue can defer processing by queueing the packet on it's service queue and then waiting for
        it's service() method to be invoked. 
        \n\n
        If a queue does not define a put() method, the default put method will 
        be used which queues data onto the service queue. The default incoming put() method joins incoming packets
        into a single packet on the service queue.
    @stability Evolving
    @defgroup HttpQueue HttpQueue
    @see HttpQueue HttpPacket HttpConn httpDiscardData httpGet httpJoinPacketForService httpPutForService httpDefaultPut 
        httpDisableQueue httpEnableQueue httpGetQueueRoom httpIsQueueEmpty httpIsPacketTooBig httpPut httpPutBack 
        httpPutForService httpRemoveQueue httpResizePacket httpScheduleQueue httpSendPacket httpSendPackets 
        httpSendEndPacket httpServiceQueue httpWillNextQueueAccept httpWrite httpWriteBlock httpWriteBody httpWriteString
 */
typedef struct HttpQueue {
    cchar               *owner;                 /**< Name of owning stage */
    struct HttpStage    *stage;                 /**< Stage owning this queue */
    struct HttpConn     *conn;                  /**< Connection ownning this queue */
    HttpQueueOpen       open;                   /**< Open the queue */
    HttpQueueClose      close;                  /**< Close the queue */
    HttpQueueStart      start;                  /**< Start the queue */
    HttpQueueData       put;                    /**< Put a message on the queue */
    HttpQueueService    service;                /**< Service the queue */
    struct HttpQueue    *nextQ;                 /**< Downstream queue for next stage */
    struct HttpQueue    *prevQ;                 /**< Upstream queue for prior stage */
    struct HttpQueue    *scheduleNext;          /**< Next linkage when queue is on the service queue */
    struct HttpQueue    *schedulePrev;          /**< Previous linkage when queue is on the service queue */
    struct HttpQueue    *pair;                  /**< Queue for the same stage in the opposite direction */
    HttpPacket          *first;                 /**< First packet in queue (singly linked) */
    HttpPacket          *last;                  /**< Last packet in queue (tail pointer) */
    ssize               count;                  /**< Bytes in queue */
    ssize               max;                    /**< Maxiumum queue size */
    ssize               low;                    /**< Low water mark for flow control */
    int                 flags;                  /**< Queue flags */
    int                 servicing;              /**< Currently being serviced */
    ssize               packetSize;             /**< Maximum acceptable packet size */
    int                 direction;              /**< Flow direction */
    void                *queueData;             /**< Stage instance data */

    /*  
        Connector instance data. Put here to save a memory allocation.
     */
    MprIOVec            iovec[HTTP_MAX_IOVEC];
    int                 ioIndex;                /**< Next index into iovec */
    ssize               ioCount;                /**< Count of bytes in iovec */
    int                 ioFileEntry;            /**< Has file entry in iovec */
    MprOffset           ioFileOffset;           /**< The next file position to use */
} HttpQueue;


/** 
    Discard all data from the queue
    @description Discard data from the queue. If removePackets (not yet implemented) is true, then remove the packets
        otherwise, just discard the data and preserve the packets.
    @param q Queue reference
    @param removePackets If true, the data packets will be removed from the queue
    @ingroup HttpQueue
 */
extern void httpDiscardData(HttpQueue *q, bool removePackets);

/** 
    Disable a queue
    @description Mark a queue as disabled so that it will not be scheduled for service.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpDisableQueue(HttpQueue *q);

/** 
    Enable a queue
    @description Enable a queue for service and schedule it to run. This will cause the service routine
        to run as soon as possible.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpEnableQueue(HttpQueue *q);

/** 
    Get the room in the queue
    @description Get the amount of data the queue can accept before being full.
    @param q Queue reference
    @return A count of bytes that can be written to the queue
    @ingroup HttpQueue
 */
extern ssize httpGetQueueRoom(HttpQueue *q);

/** 
    Determine if the queue is empty
    @description Determine if the queue has no packets queued. This does not test if the queue has no data content.
    @param q Queue reference
    @return True if there are no packets queued.
    @ingroup HttpQueue
 */
extern bool httpIsQueueEmpty(HttpQueue *q);

/** 
    Open the queue. Call the queue open entry point.
    @param q Queue reference
    @param chunkSize Preferred chunk size
 */
extern void httpOpenQueue(HttpQueue *q, ssize chunkSize);

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
    Schedule a queue
    @description Schedule a queue by adding it to the schedule queue. Queues are serviced FIFO.
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpScheduleQueue(HttpQueue *q);

/** 
    Send all queued packets
    @description Send all queued packets downstream
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpSendPackets(HttpQueue *q);

/** 
    Send an end packet
    @description Create and send an end-of-stream packet downstream
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpSendEndPacket(HttpQueue *q);

/** 
    Service a queue
    @description Service a queue by invoking its service() routine. 
    @param q Queue reference
    @ingroup HttpQueue
 */
extern void httpServiceQueue(HttpQueue *q);

/** 
    Determine if the downstream queue will accept this packet.
    @description Test if the downstream queue will accept a packet. The packet will be resized if required in an
        attempt to get the downstream queue to accept it. If the downstream queue is full, disable this queue
        and mark the downstream queue as full and service it immediately to try to relieve the congestion.
    @param q Queue reference
    @param packet Packet to put
    @return True if the downstream queue will accept the packet. Use $httpSendPacketToNext to send the packet downstream
    @ingroup HttpQueue
 */
extern bool httpWillNextQueueAcceptPacket(HttpQueue *q, HttpPacket *packet);

/** 
    Write a formatted string
    @description Write a formatted string of data into packets onto the end of the queue. Data packets will be created
        as required to store the write data. This call may block waiting for the downstream queue to drain if it is 
        or becomes full.
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
        as required to store the write data.
    @param q Queue reference
    @param buf Buffer containing the write data
    @param size of the data in buf
    @return A count of the bytes actually written
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

/* Internal */
extern HttpQueue *httpFindPreviousQueue(HttpQueue *q);
extern bool httpFlushQueue(HttpQueue *q, bool block);
extern HttpQueue *httpCreateQueue(struct HttpConn *conn, struct HttpStage *stage, int direction, HttpQueue *prev);
extern HttpQueue *httpGetNextQueueForService(HttpQueue *q);
extern void httpInitQueue(struct HttpConn *conn, HttpQueue *q, cchar *name);
extern void httpInitSchedulerQueue(HttpQueue *q);
extern void httpInsertQueue(HttpQueue *prev, HttpQueue *q);
extern int httpIsEof(struct HttpConn *conn);
extern void httpJoinPackets(HttpQueue *q, ssize size);
extern void httpMarkQueueHead(HttpQueue *q);

/*
    The Http processing pipeline is comprised of a sequence of "stages". 
    Stage Flags. Use request method flags as-is so we can quickly validate methods for stages.
 */
#define HTTP_STAGE_DELETE         HTTP_DELETE       /**< Support DELETE requests */
#define HTTP_STAGE_GET            HTTP_GET          /**< Support GET requests */
#define HTTP_STAGE_HEAD           HTTP_HEAD         /**< Support HEAD requests */
#define HTTP_STAGE_OPTIONS        HTTP_OPTIONS      /**< Support OPTIONS requests */
#define HTTP_STAGE_POST           HTTP_POST         /**< Support POST requests */
#define HTTP_STAGE_PUT            HTTP_PUT          /**< Support PUT requests */
#define HTTP_STAGE_TRACE          HTTP_TRACE        /**< Support TRACE requests */
#define HTTP_STAGE_ALL            HTTP_METHOD_MASK  /**< Mask for all methods */

#define HTTP_STAGE_CONNECTOR      0x1000            /**< Stage is a connector  */
#define HTTP_STAGE_HANDLER        0x2000            /**< Stage is a handler  */
#define HTTP_STAGE_FILTER         0x4000            /**< Stage is a filter  */
#define HTTP_STAGE_MODULE         0x8000            /**< Stage is a filter  */
#define HTTP_STAGE_VARS           0x10000           /**< Create query and form variables table */
#define HTTP_STAGE_ENV_VARS       0x20000           /**< Create CGI style environment variables table */
#define HTTP_STAGE_VIRTUAL        0x40000           /**< Handler serves virtual resources not the physical file system */
#define HTTP_STAGE_PATH_INFO      0x80000           /**< Always do path info processing */
#define HTTP_STAGE_AUTO_DIR       0x100000          /**< Want auto directory redirection */
#define HTTP_STAGE_VERIFY_ENTITY  0x200000          /**< Verify the request entity exists */
#define HTTP_STAGE_THREAD         0x400000          /**< Run handler dispatcher in a worker thread */
#define HTTP_STAGE_MISSING_EXT    0x800000          /**< Support URIs with missing extensions */

#define HTTP_STAGE_INCOMING       0x1000000
#define HTTP_STAGE_OUTGOING       0x2000000

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
    @see HttpStage HttpQueue HttpConn httpCreateConnector httpCreateFilter httpCreateHandler httpDefaultOutgoingServiceStage
        httpLookupStageData
 */
typedef struct HttpStage {
    char            *name;                  /**< Stage name */
    int             flags;                  /**< Stage flags */
    void            *stageData;             /**< Private stage data */
    MprHashTable    *extensions;            /**< Matching extensions for this filter */

    /**
        Modify a request
        @description This method is invoked to potentially modify a request. 
        @param conn MaConn connection object
        @param stage Stage object
        @return True if the stage wishes to process this request.
        @ingroup MaStage
     */
    bool (*modify)(struct HttpConn *conn, struct HttpStage *stage);

    /** 
        Match a request
        @description This method is invoked to see if the stage wishes to handle the request. If a stage denies to
            handle a request, it will be removed from the pipeline.
        @param conn HttpConn connection object
        @param stage Stage object
        @return True if the stage wishes to process this request.
        @ingroup HttpStage
     */
    bool (*match)(struct HttpConn *conn, struct HttpStage *stage);

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
        Start the handler
        @description The request has been parsed and the handler may start processing. Input data may not have 
        been received yet.
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*start)(HttpQueue *q);

    /** 
        Process the data.
        @description All incoming data has been received. The handler can now complete the request.
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*process)(HttpQueue *q);

    /** 
        Process outgoing data.
        @description Accept a packet as outgoing data
        @param q Queue instance object
        @param packet Packet of data
        @ingroup HttpStage
     */
    void (*outgoingData)(HttpQueue *q, HttpPacket *packet);

    /** 
        Service the outgoing data queue
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*outgoingService)(HttpQueue *q);

    /** 
        Process incoming data.
        @description Accept an incoming packet of data
        @param q Queue instance object
        @param packet Packet of data
        @ingroup HttpStage
     */
    void (*incomingData)(HttpQueue *q, HttpPacket *packet);

    /** 
        Service the incoming data queue
        @param q Queue instance object
        @ingroup HttpStage
     */
    void (*incomingService)(HttpQueue *q);

    /** 
        Parse configuration data.
        @description This is invoked when parsing appweb configuration files
        @param http Http object
        @param key Configuration directive name
        @param value Configuration directive value
        @param state Current configuration parsing state
        @return Zero if the key was not relevant to this stage. Return 1 if the directive applies to this stage and
            was accepted.
        @ingroup HttpStage
     */
    int (*parse)(Http *http, cchar *key, char *value, void *state);

} HttpStage;


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
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateStage(Http *http, cchar *name, int flags);

/**
    Create a clone of an existing state. This is used when creating filters configured to match certain extensions.
    @param http Http object returned from #httpCreate
    @param stage Stage object to clone
    @return A new stage object
    @ingroup HttpStage
*/
extern HttpStage *httpCloneStage(Http *http, HttpStage *stage);

/** 
    Lookup a stage by name
    @param http Http object
    @param name Name of stage to locate
    @return Stage or NULL if not found
    @ingroup HttpStage
*/
extern struct HttpStage *httpLookupStage(Http *http, cchar *name);

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
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateConnector(Http *http, cchar *name, int flags);

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
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateFilter(Http *http, cchar *name, int flags);

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
    @return A new stage object
    @ingroup HttpStage
 */
extern HttpStage *httpCreateHandler(Http *http, cchar *name, int flags);

/** 
    Default outgoing data handling
    @description This routine provides default handling of outgoing data for stages. It simply sends all packets
        downstream.
    @param q Queue object
    @ingroup HttpStage
 */
extern void httpDefaultOutgoingServiceStage(HttpQueue *q);

/** 
    Lookup stage data
    @description This looks up the stage by name and returns the private stage data.
    @param http Http object
    @param name Name of the stage concerned
    @return Reference to the stage data block.
    @ingroup HttpStage
 */
extern void *httpLookupStageData(Http *http, cchar *name);

/* Internal APIs */
extern void httpRegisterStage(Http *http, HttpStage *stage);
extern int httpOpenNetConnector(Http *http);
extern int httpOpenSendConnector(Http *http);
extern int httpOpenAuthFilter(Http *http);
extern int httpOpenChunkFilter(Http *http);
extern int httpOpenPassHandler(Http *http);
extern int httpOpenRangeFilter(Http *http);
extern int httpOpenUploadFilter(Http *http);

extern void httpSendOpen(HttpQueue *q);
extern void httpSendOutgoingService(HttpQueue *q);

/* 
    Connection flags
 */
#define HTTP_CONN_PIPE_CREATED      0x1     /**< Request pipeline created */

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
//  MOB - make origin zero
#define HTTP_STATE_BEGIN            1       /**< Ready for a new request */
#define HTTP_STATE_CONNECTED        2       /**< Connection received or made */
#define HTTP_STATE_FIRST            3       /**< First request line has been parsed */
#define HTTP_STATE_PARSED           4       /**< Headers have been parsed, handler can start */
#define HTTP_STATE_CONTENT          5       /**< Reading posted content */
#define HTTP_STATE_RUNNING          6       /**< Handler running */
#define HTTP_STATE_COMPLETE         7       /**< Request complete */


/*
    Limit validation events
 */
#define HTTP_VALIDATE_OPEN_CONN     1       /**< Open a new connection */
#define HTTP_VALIDATE_CLOSE_CONN    2       /**< Close a connection */
#define HTTP_VALIDATE_OPEN_REQUEST  3       /**< Open a new request */
#define HTTP_VALIDATE_CLOSE_REQUEST 4       /**< Close a request */

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
#define HTTP_TRACE_TIME             4       /**< Instrument http pipeline */
#define HTTP_TRACE_MAX_ITEM         5

typedef struct HttpTrace {
    int             disable;                     /**< If tracing is disabled for this request */
    int             levels[HTTP_TRACE_MAX_ITEM]; /**< Level at which to trace this item */
    ssize           size;                        /**< Maximum size at which to trace body content */
    MprHashTable    *include;                    /**< Extensions to include in trace */
    MprHashTable    *exclude;                    /**< Extensions to exclude from trace */
} HttpTrace;

extern void httpManageTrace(HttpTrace *trace, int flags);

#if BLD_DEBUG
#define HTTP_TIME(conn, tag1, tag2, op) \
    if (httpShouldTrace(conn, 0, HTTP_TRACE_TIME, NULL) >= 0) { \
        MEASURE(tag1, tag2, op); \
    } else op
#else
#define HTTP_TIME(conn, tag1, tag2, op) op
#endif

/** 
    Notifier and event callbacks.
 */
typedef MprEventProc HttpCallback;

typedef int (*HttpFillHeadersProc)(void *data);

/** 
    Http Connections
    @description The HttpConn object represents a TCP/IP connection to the client. A connection object is created for
        each socket connection initiated by the client. One HttpConn object may service many Http requests due to 
        HTTP/1.1 keep-alive.
    @stability Evolving
    @defgroup HttpConn HttpConn
    @see HttpConn HttpRx HttpRx HttpTx HttpQueue HttpStage
        httpCreateConn httpCloseConn httpCompleteRequest httpCreatePipeline httpDestroyPipeline httpDiscardTransmitData
        httpError httpGetAsync httpGetConnContext httpGetError httpGetKeepAliveCount httpPrepConn httpProcessPipeline
        httpServiceQueues httpSetAsync httpSetCredentials httpSetCallback httpSetConnContext
        httpSetConnNotifier httpSetKeepAliveCount httpSetProtocol httpSetRetries httpSetState httpSetTimeout
        httpStartPipeline httpWritable
 */
typedef struct HttpConn {
    /*  Ordered for debugability */

    struct HttpRx *rx;                      /**< Rx object */
    struct HttpTx *tx;                      /**< Tx object */
    int             state;                  /**< Connection state */
    int             flags;                  /**< Connection flags */
    int             abortPipeline;          /**< Connection errors (not proto errors) abort the pipeline */
    int             advancing;              /**< In httpProcess (reentrancy prevention) */
    int             complete;               /**< Request is complete and should step through all remaining states */
    int             writeComplete;          /**< All write data has been sent for the current request */
    int             error;                  /**< A request error has occurred */
    int             connError;              /**< A connection error has occurred */
    int             protoError;             /**< A protocol error has occurred - try to respond */

    //  MOB implement
    int             threaded;               /**< Request running in a thread */

    HttpCallback    callback;               /**< Http I/O event callback */
    void            *callbackArg;           /**< Arg to callback */
    HttpFillHeadersProc fillHeaders;        /**< Callback to fill headers */
    void            *fillHeadersArg;        /**< Arg to fillHeaders */
    HttpLimits      *limits;                /**< Service limits */
    Http            *http;                  /**< Http service object  */
    MprHashTable    *stages;                /**< Stages in pipeline */
    MprDispatcher   *dispatcher;            /**< Event dispatcher */
    HttpNotifier    notifier;               /**< Connection Http state change notification callback */
    HttpNotifier    requestNotifier;        /**< Request Http state change notification callback */
    MprWaitHandler  waitHandler;            /**< I/O wait handler */
    struct HttpServer *server;              /**< Server object (if releveant) */
    MprSocket       *sock;                  /**< Underlying socket handle */

    /* NOTE: documentRoot may be different for virtual hosts, so can't use server->documentRoot */
    char            *documentRoot;          /**< Directory for documents */

    struct HttpQueue serviceq;              /**< List of queues that require service for request pipeline */
    struct HttpQueue *currentq;             /**< Current queue being serviced */

    HttpPacket      *input;                 /**< Header packet */
    HttpQueue       *readq;                 /**< End of the read pipeline */
    HttpQueue       *writeq;                /**< Start of the write pipeline */
    MprTime         started;                /**< When the connection started */
    MprTime         lastActivity;           /**< Last activity on the connection */
    MprTime         time;                   /**< Cached current time */
    MprEvent        runEvent;               /**< Event when running the handler */
    void            *context;               /**< Embedding context */
    char            *boundary;              /**< File upload boundary */
    char            *errorMsg;              /**< Error message for the last request (if any) */
    void            *host;                  /**< Embedding host */
    char            *ip;                    /**< Remote client IP address */
    cchar           *protocol;              /**< HTTP protocol */
    int             async;                  /**< Connection is in async mode (non-blocking) */
    int             canProceed;             /**< State machine should continue to process the request */
    int             followRedirects;        /**< Follow redirects for client requests */
    int             keepAliveCount;         /**< Count of remaining keep alive requests for this connection */
    int             http10;                 /**< Using legacy HTTP/1.0 */
    int             startingThread;         /**< Need to allocate worker thread */
    int             port;                   /**< Remote port */
    int             retries;                /**< Client request retries */
    int             secure;                 /**< Using https */
    int             seqno;                  /**< Unique connection sequence number */
    int             writeBlocked;           /**< Transmission writing is blocked */

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
    char            *authGroup;             /**< Group name credentials for authorized client requests */
    char            *authUser;              /**< User name credentials for authorized client requests */
    char            *authPassword;          /**< Password credentials for authorized client requests */
    int             sentCredentials;        /**< Sent authorization credentials */
#if BLD_DEBUG
    MprTime         startTime;              /**< Start time of request */
    uint64          startTicks;             /**< Start tick time of request */
#endif
} HttpConn;


//  MOB -- all APIs need ingroup

/**
    Call httpEvent with the given event mask
    @param conn HttpConn object created via $httpCreateConn
    @param mask Mask of events. MPR_READABLE | MPR_WRITABLE
 */
extern void httpCallEvent(HttpConn *conn, int mask);

/**
    Http I/O event handler. Invoke when there is an I/O event on the connection. This is normally invoked automatically
    when I/O events are received.
    @param conn HttpConn object created via $httpCreateConn
    @param event Event structure
 */
extern void httpEvent(HttpConn *conn, MprEvent *event);

/** 
    Close a connection
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpCloseConn(HttpConn *conn);

/**
    Signal writing transmission body is complete. This is called by connectors when writing data is complete.
    @param conn HttpConn object created via $httpCreateConn
 */ 
extern void httpCompleteWriting(HttpConn *conn);

/**
    Signal the request is complete. This is called by connectors when the request is complete
    @param conn HttpConn object created via $httpCreateConn
 */ 
extern void httpCompleteRequest(HttpConn *conn);

/** 
    Create a connection object
    Most interactions with the Http library are via a connection object. It is used for server-side communications when
    responding to client requests and it is used to initiate outbound client requests.
    @param http Http object created via #httpCreate
    @param server Server object owning the connection.
    @returns A new connection object
*/
extern HttpConn *httpCreateConn(Http *http, struct HttpServer *server);
extern void httpDestroyConn(HttpConn *conn);

/**
    Create a request pipeline
    @param conn HttpConn object created via $httpCreateConn
    @param location Location object controlling how the pipeline is configured for the request
    @param handler Handler to process the request for server side requests
 */
extern void httpCreatePipeline(HttpConn *conn, struct HttpLoc *location, HttpStage *handler);

/**
    Destroy the pipeline
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpDestroyPipeline(HttpConn *conn);

/**
    Discard buffered transmit pipeline data
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpDiscardTransmitData(HttpConn *conn);

/** 
    Error handling for the connection.
    @description The httpError call is used to flag the current request as failed.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @ingroup HttpTx
 */
extern void httpError(HttpConn *conn, int status, cchar *fmt, ...);
extern void httpLimitError(HttpConn *conn, int status, cchar *fmt, ...);

/**
    Get the async mode value for the connection
    @param conn HttpConn object created via $httpCreateConn
    @return True if the connection is in async mode
 */
extern int httpGetAsync(HttpConn *conn);

/**
    Get the preferred chunked size for transfer chunk encoding.
    @param conn HttpConn connection object created via $httpCreateConn
    @return Chunk size. Returns zero if not yet defined.
 */
extern ssize httpGetChunkSize(HttpConn *conn);

/**
    Get the connection context object
    @param conn HttpConn object created via $httpCreateConn
    @return The connection context object defined via httpSetConnContext
 */
extern void *httpGetConnContext(HttpConn *conn);

/**
    Get the connection host object
    @param conn HttpConn object created via $httpCreateConn
    @return The connection host object defined via httpSetConnHost
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
    Get the count of Keep-Alive requests that will be used for this connection object.
    @description Http keep alive means that the TCP/IP connection is preserved accross multiple requests. This
        typically means much higher performance and better response. Http keep alive is enabled by default 
        for Http/1.1 (the default). Disable keep alive when talking to old, broken HTTP servers.
    @param conn HttpConn connection object created via $httpCreateConn
    @return The maximum count of keep alive requests. 
    @ingroup HttpConn
 */
extern int httpGetKeepAliveCount(HttpConn *conn);

/**
    Prepare a connection for a new request. This is used internally when using Keep-Alive.
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpPrepServerConn(HttpConn *conn);

#if UNUSED
//  MOB
#define HTTP_NEW_REQUEST 0
#define HTTP_RETRY_REQUEST 1
#endif
    
extern void httpPrepClientConn(HttpConn *conn, int keepHeaders);
extern void httpConsumeLastRequest(HttpConn *conn);

/**
    Process the pipeline. This starts invokes the process method of the  request handler. This is called when all 
    incoming data has been received.
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpStartPipeline(HttpConn *conn);
extern void httpProcessPipeline(HttpConn *conn);

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
 */
extern void httpSetAsync(HttpConn *conn, int enable);

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
    Define an callback for IO events on this connection.
    @description The event callback will be invoked in response to I/O events.
    @param conn HttpConn connection object created via $httpCreateConn
    @param fn Callback function. 
    @param arg Data argument to provide to the callback function.
    @ingroup HttpConn
 */
extern void httpSetCallback(HttpConn *conn, HttpCallback fn, void *arg);

/** 
    Set the chunk size for transfer chunked encoding. When set a "Transfer-Encoding: Chunked" header will
    be added to the request and all write data will be broken into chunks of the requested size.
    @param conn HttpConn connection object created via $httpCreateConn
    @param size Requested chunk size.
    @ingroup HttpConn
 */ 
extern void httpSetChunkSize(HttpConn *conn, ssize size);

/**
    Set the connection context object
    @param conn HttpConn object created via $httpCreateConn
    @param context New context object
 */
extern void httpSetConnContext(HttpConn *conn, void *context);

/**
    Set the connection host object
    @param conn HttpConn object created via $httpCreateConn
    @param context New context host
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

//  MOB
extern void httpSetRequestNotifier(HttpConn *conn, HttpNotifier fn);

/** 
    Control Http Keep-Alive for the connection.
    @description Http keep alive means that the TCP/IP connection is preserved accross multiple requests. This
        typically means much higher performance and better response. Http keep alive is enabled by default 
        for Http/1.1 (the default). Disable keep alive when talking to old, broken HTTP servers.
    @param conn HttpConn connection object created via $httpCreateConn
    @param count Count of keep alive transactions to use before closing the connection. Set to zero to disable keep-alive.
    @ingroup HttpConn
 */
extern void httpSetKeepAliveCount(HttpConn *conn, int count);

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
    Set the connection state and invoke notifiers.
    @param conn HttpConn object created via $httpCreateConn
    @param state New state to enter
 */
extern void httpSetState(HttpConn *conn, int state);

/** 
    Set the Http inactivity timeout
    @description Define an inactivity timeout after which the Http connection will be closed. 
    @param conn HttpConn object created via $httpCreateConn
    @param requestTimeout Request timeout in seconds. This is the total time for the request
    @param inactivityTimeout Inactivity timeout in seconds. This is maximum connection idle time.
    @ingroup HttpConn
 */
extern void httpSetTimeout(HttpConn *conn, int requestTimeout, int inactivityTimeout);

/**
    Start the pipeline. This starts the request handler.
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpStartPipeline(HttpConn *conn);

/**
    Inform notifiers that the connection is now writable
    @param conn HttpConn object created via $httpCreateConn
 */ 
extern void httpWritable(HttpConn *conn);

/** Internal APIs */
extern struct HttpConn *httpAccept(struct HttpServer *server);
extern void httpEnableConnEvents(HttpConn *conn);
extern HttpPacket *httpGetConnPacket(HttpConn *conn);
extern void httpSetPipeHandler(HttpConn *conn, HttpStage *handler);
extern void httpSetSendConnector(HttpConn *conn, cchar *path);

extern void httpInitTrace(HttpTrace *trace);
extern int httpShouldTrace(HttpConn *conn, int dir, int item, cchar *ext);
extern void httpTraceContent(HttpConn *conn, int dir, int item, HttpPacket *packet, ssize len, ssize total);
extern HttpLimits *httpSetUniqueConnLimits(HttpConn *conn);

/*  
    Deny/Allow order. TODO - this is not yet implemented.
 */
#define HTTP_ALLOW_DENY           1
#define HTTP_DENY_ALLOW           2
#define HTTP_ACL_ALL             -1         /* All bits set */

/*  
    Authentication types
 */
#define HTTP_AUTH_UNKNOWN         0
#define HTTP_AUTH_BASIC           1         /* Basic HTTP authentication (clear text) */
#define HTTP_AUTH_DIGEST          2         /* Digest authentication */

/*  
    Auth Flags
 */
#define HTTP_AUTH_REQUIRED        0x1       /* Dir/Location requires auth */

/*  
    Authentication methods
 */
#define HTTP_AUTH_METHOD_FILE     1         /* Appweb httpPassword file based authentication */
#define HTTP_AUTH_METHOD_PAM      2         /* Plugable authentication module scheme (Unix) */


typedef long HttpAcl;                       /* Access control mask */

/** 
    Authorization
    HttpAuth is the foundation authorization object and is used as base class by HttpDirectory and HttpLoc.
    It stores the authorization configuration information required to determine if a client request should be permitted 
    access to a given resource.
    @stability Evolving
    @defgroup HttpAuth HttpAuth
    @see HttpAuth
 */
typedef struct HttpAuth {
    bool            anyValidUser;           /**< If any valid user will do */
    int             type;                   /**< Kind of authorization */

    char            *allow;                 /**< Clients to allow */
    char            *deny;                  /**< Clients to deny */
    char            *requiredRealm;         /**< Realm to use for access */
    char            *requiredGroups;        /**< Auth group for access */
    char            *requiredUsers;         /**< User name for access */
    HttpAcl         requiredAcl;            /**< ACL for access */

    int             backend;                /**< Authorization method (PAM | FILE) */
    int             flags;                  /**< Auth flags */
    int             order;                  /**< Order deny/allow, allow/deny */
    char            *qop;                   /**< Digest Qop */

    /*  
        State for file based authorization
     */
    char            *userFile;              /**< User name auth file */
    char            *groupFile;             /**< Group auth file  */
    MprHashTable    *users;                 /**< Hash of user file  */
    MprHashTable    *groups;                /**< Hash of group file  */
} HttpAuth;


//  TODO - Document
extern void httpInitAuth(Http *http);
extern HttpAuth *httpCreateAuth(HttpAuth *parent);
extern void httpSetAuthAllow(HttpAuth *auth, cchar *allow);
extern void httpSetAuthAnyValidUser(HttpAuth *auth);
extern void httpSetAuthDeny(HttpAuth *auth, cchar *deny);
extern void httpSetAuthGroup(HttpConn *conn, cchar *group);
extern void httpSetAuthOrder(HttpAuth *auth, int order);
extern void httpSetAuthQop(HttpAuth *auth, cchar *qop);
extern void httpSetAuthRealm(HttpAuth *auth, cchar *realm);
extern void httpSetAuthRequiredGroups(HttpAuth *auth, cchar *groups);
extern void httpSetAuthRequiredUsers(HttpAuth *auth, cchar *users);
extern void httpSetAuthUser(HttpConn *conn, cchar *user);

#if BLD_FEATURE_AUTH_FILE
/** 
    User Authorization
    File based authorization backend
    @stability Evolving
    @defgroup HttpUser
    @see HttpUser
 */
typedef struct HttpUser {
    bool            enabled;
    HttpAcl         acl;                    /* Union (or) of all group Acls */
    char            *password;
    char            *realm;
    char            *name;
} HttpUser;


/** 
    Group Authorization
    @stability Evolving
    @defgroup HttpGroup
    @see HttpGroup
 */
typedef struct  HttpGroup {
    HttpAcl         acl;
    bool            enabled;
    char            *name;
    MprList         *users;                 /* List of users */
} HttpGroup;

//  TODO - simplify this API
//  TODO -- all these routines should be generic (not native) and use some switch table to vector to the right backend method

extern int      httpAddGroup(HttpAuth *auth, cchar *group, HttpAcl acl, bool enabled);
extern int      httpAddUser(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, bool enabled);
extern int      httpAddUserToGroup(HttpAuth *auth, HttpGroup *gp, cchar *user);
extern int      httpAddUsersToGroup(HttpAuth *auth, cchar *group, cchar *users);
extern HttpAuth *httpCreateAuth(HttpAuth *parent);
extern HttpGroup *httpCreateGroup(HttpAuth *auth, cchar *name, HttpAcl acl, bool enabled);
extern HttpUser *httpCreateUser(HttpAuth *auth, cchar *realm, cchar *name, cchar *password, bool enabled);
extern int      httpDisableGroup(HttpAuth *auth, cchar *group);
extern int      httpDisableUser(HttpAuth *auth, cchar *realm, cchar *user);
extern int      httpEnableGroup(HttpAuth *auth, cchar *group);
extern int      httpEnableUser(HttpAuth *auth, cchar *realm, cchar *user);
extern HttpAcl  httpGetGroupAcl(HttpAuth *auth, char *group);
extern cchar    *httpGetNativePassword(HttpAuth *auth, cchar *realm, cchar *user);
extern bool     httpIsGroupEnabled(HttpAuth *auth, cchar *group);
extern bool     httpIsUserEnabled(HttpAuth *auth, cchar *realm, cchar *user);
extern HttpAcl  httpParseAcl(HttpAuth *auth, cchar *aclStr);
extern int      httpRemoveGroup(HttpAuth *auth, cchar *group);
extern int      httpReadGroupFile(Http *server, HttpAuth *auth, char *path);
extern int      httpReadUserFile(Http *server, HttpAuth *auth, char *path);
extern int      httpRemoveUser(HttpAuth *auth, cchar *realm, cchar *user);
extern int      httpRemoveUserFromGroup(HttpGroup *gp, cchar *user);
extern int      httpRemoveUsersFromGroup(HttpAuth *auth, cchar *group, cchar *users);
extern int      httpSetGroupAcl(HttpAuth *auth, cchar *group, HttpAcl acl);
extern void     httpSetRequiredAcl(HttpAuth *auth, HttpAcl acl);
extern void     httpUpdateUserAcls(HttpAuth *auth);
extern int      httpWriteUserFile(Http *server, HttpAuth *auth, char *path);
extern int      httpWriteGroupFile(Http *server, HttpAuth *auth, char *path);
extern bool     httpValidateNativeCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
                    cchar *requiredPass, char **msg);
#endif /* AUTH_FILE */

#if BLD_FEATURE_AUTH_PAM
extern cchar    *httpGetPamPassword(HttpAuth *auth, cchar *realm, cchar *user);
extern bool     httpValidatePamCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
                    cchar *requiredPass, char **msg);
#endif /* AUTH_PAM */


#define HTTP_LOC_PUT_DELETE       0x20      /**< Support PUT|DELETE */

/**
    Location Control
    @stability Evolving
    @defgroup HttpLoc HttpLoc
    @see HttpLoc
 */
typedef struct HttpLoc {
    HttpAuth        *auth;                  /**< Per location block authentication */
    Http            *http;                  /**< Http service object (copy of appweb->http) */
    int             flags;                  /**< Location flags */
    char            *prefix;                /**< Location prefix name */
    int             prefixLen;              /**< Length of the prefix name */
    int             sessionTimeout;         /**< Session timeout for this location */
    HttpStage       *connector;             /**< Network connector to use */
    HttpStage       *handler;               /**< Fixed handler */
    void            *handlerData;           /**< Data reserved for the handler */
    MprHashTable    *extensions;            /**< Hash of handlers by extensions */
    MprHashTable    *expires;               /**< Expiry of content by mime type */
    MprList         *handlers;              /**< List of handlers for this location */
    MprList         *inputStages;           /**< Input stages */
    MprList         *outputStages;          /**< Output stages */
    MprHashTable    *errorDocuments;        /**< Set of error documents to use on errors */
    struct HttpLoc  *parent;                /**< Parent location */
    void            *context;               /**< Hosting context */
    char            *uploadDir;             /**< Upload directory */
    int             autoDelete;             /**< Auto delete uploaded files */
    char            *searchPath;            /**< Search path */
    char            *script;                /**< Startup script */
    struct MprSsl   *ssl;                   /**< SSL configuration */
} HttpLoc;

extern HttpLoc *httpInitLocation(Http *http, int serverSide);
extern void httpAddErrorDocument(HttpLoc *location, cchar *code, cchar *url);

extern void httpFinalizeLocation(HttpLoc *location);
extern struct HttpStage *httpGetHandlerByExtension(HttpLoc *location, cchar *ext);
extern cchar *httpLookupErrorDocument(HttpLoc *location, int code);
extern void httpResetPipeline(HttpLoc *location);
extern void httpSetLocationAuth(HttpLoc *location, HttpAuth *auth);
extern void httpSetLocationAutoDelete(HttpLoc *location, int enable);
extern void httpSetLocationFlags(HttpLoc *location, int flags);
extern void httpSetLocationHandler(HttpLoc *location, cchar *name);
extern void httpSetLocationPrefix(HttpLoc *location, cchar *uri);
extern void httpSetLocationScript(HttpLoc *location, cchar *script);
extern int httpSetConnector(HttpLoc *location, cchar *name);
extern int httpAddHandler(HttpLoc *location, cchar *name, cchar *extensions);

extern HttpLoc *httpCreateLocation(Http *http);
extern HttpLoc *httpCreateInheritedLocation(Http *http, HttpLoc *location);
extern int httpSetHandler(HttpLoc *location, cchar *name);
extern int httpAddFilter(HttpLoc *location, cchar *name, cchar *extensions, int direction);
extern void httpClearStages(HttpLoc *location, int direction);
extern void httpAddLocationExpiry(HttpLoc *location, MprTime when, cchar *mimeTypes);

/**
    Upload File
    Each uploaded file has an HttpUploadedFile entry. This is managed by the upload handler.
    @stability Evolving
    @defgroup HttpUploadFile HttpUploadFile
    @see HttpUploadFile
 */
typedef struct HttpUploadFile {
    cchar           *filename;              /* Local (temp) name of the file */
    cchar           *clientFilename;        /* Client side name of the file */
    cchar           *contentType;           /* Content type */
    ssize           size;                   /* Uploaded file size */
} HttpUploadFile;

extern void httpAddUploadFile(HttpConn *conn, cchar *id, HttpUploadFile *file);
extern void httpRemoveAllUploadedFiles(HttpConn *conn);
extern void httpRemoveUploadFile(HttpConn *conn, cchar *id);

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
#define HTTP_METHOD_MASK        0x7F        /**< Method mask */
#define HTTP_REC_CREATE_ENV     0x80        /**< Must create env for this request */
#define HTTP_REC_IF_MODIFIED    0x100       /**< If-[un]modified-since supplied */
#define HTTP_REC_CHUNKED        0x200       /**< Content is chunk encoded */

/*  
    Incoming chunk encoding states
 */
#define HTTP_CHUNK_START      1             /**< Start of a new chunk */
#define HTTP_CHUNK_DATA       2             /**< Start of chunk data */
#define HTTP_CHUNK_EOF        3             /**< End of last chunk */

/** 
    Http Rx
    @description Most of the APIs in the rx group still take a HttpConn object as their first parameter. This is
        to make the API easier to remember - APIs take a connection object rather than a rx or tx object.
    @stability Evolving
    @defgroup HttpRx HttpRx
    @see HttpRx HttpConn HttpTx httpSetWriteBlocked httpGetCookies httpGetQueryString
 */
typedef struct HttpRx {
    char            *method;                /**< Request method */
    char            *uri;                   /**< Original URI (alias for parsedUri->uri) (not decoded) */
    char            *scriptName;            /**< ScriptName portion of the url (Decoded). May be empty or start with "/" */
    char            *pathInfo;              /**< Path information after the scriptName (Decoded and normalized) */

    HttpConn        *conn;                  /**< Connection object */

    MprList         *etags;                 /**< Document etag to uniquely identify the document version */
    HttpPacket      *headerPacket;          /**< HTTP headers */
    MprHashTable    *headers;               /**< Header variables */
    MprList         *inputPipeline;         /**< Input processing */
    HttpLoc         *loc;                   /**< Location block */
    HttpUri         *parsedUri;             /**< Parsed request uri */
    MprHashTable    *requestData;           /**< General request data storage. Users must create hash table if required */
    MprTime         since;                  /**< If-Modified date */

    int             eof;                    /**< All read data has been received (eof) */
    ssize           length;                 /**< Declared content length (ENV: CONTENT_LENGTH) */
    int             chunkState;             /**< Chunk encoding state */
    ssize           chunkSize;              /**< Size of the next chunk */
    int             chunkRemainingData;     /**< Remaining chunk data to read */
    int             flags;                  /**< Rx modifiers */
    int             needInputPipeline;      /**< Input pipeline required to process received data */
    ssize           remainingContent;       /**< Remaining content data to read (in next chunk if chunked) */
    ssize           receivedContent;        /**< Length of content actually received */
    ssize           readContent;            /**< Length of content read by user */

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
    char            *cookie;                /**< Cookie header */
    char            *connection;            /**< Connection header */
    char            *contentLength;         /**< Content length string value */
    char            *hostName;              /**< Client supplied host name */

    //  MOB -- is this needed if Tx.filename is pathInfo => storage */ 
    char            *pathTranslated;        /**< Mapped pathInfo to storage. Set by handlers if required. (Decoded) */
    char            *pragma;                /**< Pragma header */
    char            *mimeType;              /**< Mime type of the request payload (ENV: CONTENT_TYPE) */
    char            *redirect;              /**< Redirect location header */
    char            *referrer;              /**< Refering URL */
    char            *userAgent;             /**< User-Agent header */
    int             form;                   /**< Using mime-type application/x-www-form-urlencoded */
    int             upload;                 /**< Using uploadFilter ("multipart/form-data) */

    MprHashTable    *formVars;              /**< Query and post data variables */
    HttpRange       *ranges;                /**< Data ranges for body data */
    HttpRange       *inputRange;            /**< Specified range for input (post) data */

    /*  
        Auth details
     */
    HttpAuth        *auth;                  /**< Auth object */
    char            *authAlgorithm;
    char            *authDetails;
    char            *authStale;             
    char            *authType;              /**< Authorization type (basic|digest) (ENV: AUTH_TYPE) */

    /*  
        Upload details
     */
    MprHashTable    *files;                 /**< Uploaded files. Managed by the upload filter */
    char            *uploadDir;             /**< Upload directory */
    int             autoDelete;             /**< Auto delete uploaded files */

    /*
        Extensions for Appweb. Inline for performance.
     */
    struct MaAlias  *alias;                 /**< Matching alias */
    struct MaDir    *dir;                   /**< Best matching dir (PTR only) */
} HttpRx;


/** 
    Get a rx content length
    @description Get the length of the rx body content (if any). This is used in servers to get the length of posted
        data and in clients to get the response body length.
    @param conn HttpConn connection object created via $httpCreateConn
    @return A count of the response content data in bytes.
    @ingroup HttpRx
 */
extern ssize httpGetContentLength(HttpConn *conn);

/** 
    Get the request cookies
    @description Get the cookies defined in the current requeset
    @param conn HttpConn connection object created via $httpCreateConn
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup HttpRx
 */
extern cchar *httpGetCookies(HttpConn *conn);

/** 
    Get a rx http header.
    @description Get a http response header for a given header key.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Name of the header to retrieve. This should be a lower case header name. For example: "Connection"
    @return Value associated with the header key or null if the key did not exist in the response.
    @ingroup HttpRx
 */
extern cchar *httpGetHeader(HttpConn *conn, cchar *key);

/** 
    Get all the response http headers.
    @description Get all the rx headers. The returned string formats all the headers in the form:
        key: value\\nkey2: value2\\n...
    @param conn HttpConn connection object created via $httpCreateConn
    @return String containing all the headers. The caller must free this returned string.
    @ingroup HttpRx
 */
extern char *httpGetHeaders(HttpConn *conn);

/** 
    Get the hash table of rx Http headers
    @description Get the internal hash table of rx headers
    @param conn HttpConn connection object created via $httpCreateConn
    @return Hash table. See MprHash for how to access the hash table.
    @ingroup HttpRx
 */
extern MprHashTable *httpGetHeaderHash(HttpConn *conn);

/** 
    Get the request query string
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup HttpRx
 */
extern cchar *httpGetQueryString(HttpConn *conn);

/** 
    Get a status associated with a response to a client request.
    @param conn HttpConn connection object created via $httpCreateConn
    @return An integer Http response code. Typically 200 is success.
    @ingroup HttpRx
 */
extern int httpGetStatus(HttpConn *conn);

/** 
    Get the Http status message associated with a response to a client request. The Http status message is supplied 
    on the first line of the Http response.
    @param conn HttpConn connection object created via $httpCreateConn
    @returns A Http status message. Caller must not free.
    @ingroup HttpRx
 */
extern char *httpGetStatusMessage(HttpConn *conn);

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

//  MOB DOC
extern void httpSetStageData(HttpConn *conn, cchar *key, cvoid *data);
extern cvoid *httpGetStageData(HttpConn *conn, cchar *key);


/* Internal */
extern HttpRx *httpCreateRx(HttpConn *conn);
extern void httpDestroyRx(HttpRx *rx);
extern void httpCloseRx(struct HttpConn *conn);
extern bool httpContentNotModified(HttpConn *conn);
extern HttpRange *httpCreateRange(HttpConn *conn, int start, int end);
extern void httpConnError(struct HttpConn *conn, int status, cchar *fmt, ...);
extern void httpProtocolError(struct HttpConn *conn, int status, cchar *fmt, ...);
extern void httpProcess(HttpConn *conn, HttpPacket *packet);
extern void httpProcessWriteEvent(HttpConn *conn);
extern bool httpProcessCompletion(HttpConn *conn);
extern int  httpSetUri(HttpConn *conn, cchar *newUri);
extern void httpSetEtag(HttpConn *conn, MprPath *info);
extern bool httpMatchEtag(HttpConn *conn, char *requestedEtag);
extern bool httpMatchModified(HttpConn *conn, MprTime time);

/**
    Add query and post form variables
    @description Add new variables encoded in the supplied buffer
    @param conn HttpConn connection object
    @param buf Buffer containing www-urlencoded data
    @param len Length of buf
    @ingroup HttpRx
 */
extern void httpAddVars(HttpConn *conn, cchar *buf, ssize len);

/**
    Add env vars from body data
    @param q Queue reference
 */
extern void httpAddVarsFromQueue(HttpQueue *q);

/**
    Compare a form variable
    @description Compare a form variable and return true if it exists and its value matches.
    @param conn HttpConn connection object
    @param var Name of the form variable 
    @param value Value to compare
    @return True if the value matches
    @ingroup HttpRx
 */
extern int httpCompareFormVar(HttpConn *conn, cchar *var, cchar *value);

/**
    Get the cookies
    @description Get the cookies defined in the current requeset
    @param conn HttpConn connection object
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup HttpRx
 */
extern cchar *httpGetCookies(HttpConn *conn);

/**
    Get a form variable
    @description Get the value of a named form variable. Form variables are define via www-urlencoded query or post
        data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @param defaultValue Default value to return if the variable is not defined. Can be null.
    @return String containing the form variable's value. Caller should not free.
    @ingroup HttpRx
 */
extern cchar *httpGetFormVar(HttpConn *conn, cchar *var, cchar *defaultValue);

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
extern int httpGetIntFormVar(HttpConn *conn, cchar *var, int defaultValue);

/**
    Get the request query string
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup HttpRx
 */
extern cchar *httpGetQueryString(HttpConn *conn);

/**
    Set a form variable value
    @description Set the value of a named form variable to an integer value. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @param value Default value to return if the variable is not defined. Can be null.
    @ingroup HttpRx
 */
extern void httpSetIntFormVar(HttpConn *conn, cchar *var, int value);

/**
    Set a form variable value
    @description Set the value of a named form variable to a string value. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @param value Default value to return if the variable is not defined. Can be null.
    @ingroup HttpRx
 */
extern void httpSetFormVar(HttpConn *conn, cchar *var, cchar *value);

/**
    Test if a form variable is defined
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @return True if the form variable is defined
    @ingroup HttpRx
 */
extern int httpTestFormVar(HttpConn *conn, cchar *var);

//  MOB 
extern void httpCreateEnvVars(HttpConn *conn);

/*  
    Tx flags
 */
#define HTTP_TX_DONT_CACHE          0x1     /**< Add no-cache to the transmission */
#define HTTP_TX_NO_BODY             0x2     /**< No transmission body, only sent headers */
#define HTTP_TX_HEADERS_CREATED     0x4     /**< Response headers have been created */
#define HTTP_TX_SENDFILE            0x8     /**< Relay output via Send connector */

typedef cchar *(*HttpRedirectCallback)(HttpConn *conn, int *code, HttpUri *uri);
typedef void (*HttpEnvCallback)(HttpConn *conn);

/** 
    Http Tx
    @description The tx object controls the transmission of data. This may be client requests or responses to
        client requests. Most of the APIs in the Response group still take a HttpConn object as their first parameter. 
        This is to make the API easier to remember - APIs take a connection object rather than a rx or 
        transmission object.
    @stability Evolving
    @defgroup HttpTx HttpTx
    @see HttpTx HttpRx HttpConn httpSetCookie httpError httpFormatBody
 */
typedef struct HttpTx {
    struct HttpConn *conn;                  /**< Current connection object */
    MprList         *outputPipeline;        /**< Output processing */
    HttpStage       *handler;               /**< Server-side request handler stage */
    HttpStage       *connector;             /**< Network connector to send / receive socket data */
    HttpQueue       queue[2];               /**< Dummy head for the queues */

    HttpUri         *parsedUri;             /**< Request uri. Only used for requests */
    HttpRange       *currentRange;          /**< Current range being fullfilled */
    MprDispatcher   *dispatcher;            /**< Request has its own dispatcher */
    MprHashTable    *headers;               /**< Transmission headers */
    char            *rangeBoundary;         /**< Inter-range boundary */

    char            *etag;                  /**< Unique identifier tag */
    char            *method;                /**< Request method GET, HEAD, POST, DELETE, OPTIONS, PUT, TRACE */
    char            *altBody;               /**< Alternate transmission for errors */
    ssize           chunkSize;              /**< Chunk size to use when using transfer encoding. Zero for unchunked. */
    int             flags;                  /**< Response flags */
    int             finalized;              /**< Finalization done */
    ssize           length;                 /**< Transmission content length */
    ssize           pos;                    /**< Current I/O position */
    int             status;                 /**< HTTP request status */
    int             traceMethods;           /**< Handler methods supported */

    /* File information for file based handlers */
    MprFile         *file;                  /**< File to be served */
    MprPath         fileInfo;               /**< File information if there is a real file to serve */
    char            *filename;              /**< Name of a real file being served (typically pathInfo mapped) */
    cchar           *extension;             /**< Filename extension */
    ssize           entityLength;           /**< Original content length before range subsetting */
    ssize           bytesWritten;           /**< Bytes written including headers */
    ssize           headerSize;             /**< Size of the header written */

    HttpRedirectCallback redirectCallback;  /**< Redirect callback */
    HttpEnvCallback envCallback;            /**< SetEnv callback */
} HttpTx;

#if UNUSED
extern void httpCreateTxHeaders(HttpConn *conn);
#endif

/** 
    Add a header to the transmission using a format string.
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup HttpTx
 */
extern void httpAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

/** 
    Add a header to the transmission
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value Key value to use
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup HttpTx
 */
extern void httpAddSimpleHeader(HttpConn *conn, cchar *key, cchar *value);

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
    Connect to a server and issue Http client request.
    @description Start a new Http request on the http object and return. This routine does not block.
        After starting the request, you can use httpWait() or httpWForResponse() to wait for the request to 
        achieve a certain state or to complete.
    @param conn HttpConn connection object created via $httpCreateConn
    @param method Http method to use. Valid methods include: "GET", "POST", "PUT", "DELETE", "OPTIONS" and "TRACE" 
    @param uri URI to fetch
    @return Zero if the request was successfully sent to the server. Otherwise a negative MPR error code is returned.
    @ingroup HttpTx
 */
extern int httpConnect(HttpConn *conn, cchar *method, cchar *uri);

/** 
    Create the tx object. This is used internally by the http library.
    @param conn HttpConn connection object created via $httpCreateConn
    @param headers Optional headers to use for the transmission
    @returns A tx object
 */
extern HttpTx *httpCreateTx(HttpConn *conn, MprHashTable *headers);

//  MOB DOC
extern void httpDestroyTx(HttpTx *tx);

/** 
    Dont cache the transmission 
    @description Instruct the client not to cache the transmission body. This is done by setting the Cache-control Http
        header.
    @param conn HttpConn connection object
    @ingroup HttpTx
 */
extern void httpDontCache(HttpConn *conn);

/** 
    Enable Multipart-Mime File Upload for this request. This will define a "Content-Type: multipart/form-data..."
    header and will create a mime content boundary for use to delimit the various upload content files and fields.
    @param conn HttpConn connection object
    @ingroup HttpConn
 */
extern void httpEnableUpload(HttpConn *conn);

/** 
    Finalize transmission of the http request
    @description Finalize writing Http data by writing the final chunk trailer if required. If using chunked transfers, 
    a null chunk trailer is required to signify the end of write data. 
    @param conn HttpConn connection object
    @ingroup HttpTx
 */
extern void httpFinalize(HttpConn *conn);

//  MOB
extern int httpIsFinalized(HttpConn *conn);

/**
    Flush tx data. This writes any buffered data. 
    @param conn HttpConn connection object created via $httpCreateConn
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
    Format an alternate transmission body
    @description Format a transmission body to use instead of data generated by the request processing pipeline.
    @param conn HttpConn connection object created via $httpCreateConn
    @param title Title string to format into the HTML transmission body.
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param ... Arguments for fmt
    @return A count of the number of bytes in the transmission body.
    @ingroup HttpTx
 */
extern int httpFormatBody(HttpConn *conn, cchar *title, cchar *fmt, ...);

/** 
    Format an error transmission
    @description Format an error message to use instead of data generated by the request processing pipeline.
        This is typically used to send errors and redirections.
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
    Format an error transmission using a va_list
    @description Format an error message to use instead of data generated by the request processing pipeline.
        This is typically used to send errors and redirections.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http response status code
    @param fmt Printf style formatted string. This string may contain HTML tags and is not HTML encoded before
        sending to the user. NOTE: Do not send user input back to the client using this method. Otherwise you open
        large security holes.
    @param args Arguments for fmt
    @ingroup HttpTx
 */
extern void httpFormatErrorV(HttpConn *conn, int status, cchar *fmt, va_list args);

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
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup HttpTx
 */
extern int httpRemoveHeader(HttpConn *conn, cchar *key);

/** 
    Define a content length header in the transmission. This will define a "Content-Length: NNN" request header.
    @param conn HttpConn connection object created via $httpCreateConn
    @param length Numeric value for the content length header.
    @ingroup HttpConn
 */
extern void httpSetContentLength(HttpConn *conn, ssize length);

/** 
    Set a transmission cookie
    @description Define a cookie to send in the transmission Http header
    @param conn HttpConn connection object created via $httpCreateConn
    @param name Cookie name
    @param value Cookie value
    @param path URI path to which the cookie applies
    @param domain Domain in which the cookie applies. Must have 2-3 dots.
    @param lifetime Duration for the cookie to persist in seconds
    @param secure Set to true if the cookie only applies for SSL based connections
    @ingroup HttpTx
 */
extern void httpSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *domain, int lifetime, bool secure);

/**
    Define the length of the transmission content. When a static content is used for the transmission body, defining
    the entity length permits the request pipeline to know when all the data has been sent.
    @param conn HttpConn connection object created via $httpCreateConn
    @param len Transmission body length in bytes
 */
extern void httpSetEntityLength(HttpConn *conn, ssize len);

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

//MOB
extern void httpSetResponseBody(HttpConn *conn, int status, cchar *msg);

/** 
    Set a Http response status.
    @description Set the Http response status for the request. This defaults to 200 (OK).
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code.
    @ingroup HttpTx
 */
extern void httpSetStatus(HttpConn *conn, int status);

/** 
    Set the transmission (response)  mime type
    @description Set the mime type Http header in the transmission
    @param conn HttpConn connection object created via $httpCreateConn
    @param mimeType Mime type string
    @ingroup HttpTx
 */
extern void httpSetMimeType(HttpConn *conn, cchar *mimeType);

/** 
    Set a simple key/value transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value String value for the key
    @ingroup HttpTx
 */
extern void httpSetSimpleHeader(HttpConn *conn, cchar *key, cchar *value);

/** 
    Wait for the connection to achieve the requested state Used for blocking client requests.
    @param conn HttpConn connection object created via $httpCreateConn
    @param dispatcher Mpr dispatcher to use for waiting
    @param state HTTP_STATE_XXX to wait for.
    @param timeout Timeout in milliseconds to wait 
    @return MOB
    @ingroup HttpTx
 */
extern int httpWait(HttpConn *conn, MprDispatcher *dispatcher, int state, int timeout);

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

/**
    Indicate that the transmission socket is blocked
    @param conn Http connection object created via $httpCreateConn
 */
extern void httpSetWriteBlocked(HttpConn *conn);


typedef int (*HttpListenCallback)(struct HttpServer *server);

/** 
    Server endpoint
    @stability Evolving
    @defgroup HttpServer HttpServer
    @see HttpServer httpCreateServer httpStartServer httpStopServer
 */
typedef struct  HttpServer {
    Http            *http;                  /**< Http service object */
    HttpLoc         *loc;                   /**< Default location block */
    HttpLimits      *limits;                /**< Server resource limits */
    MprWaitHandler  waitHandler;            /**< I/O wait handler */
    MprHashTable    *clientLoad;            /**< Table of active client IPs and connection counts */
    char            *serverRoot;            /**< Directory for server configuration */
    char            *documentRoot;          /**< Directory for documents */
    char            *name;                  /**< Published name of the server (ServerName directive) */
    char            *ip;                    /**< Listen IP address */
    int             port;                   /**< Listen port */
    int             async;                  /**< Listening is in async mode (non-blocking) */
    int             clientCount;            /**< Count of current active clients */
    int             requestCount;           /**< Count of current active requests */
    void            *context;               /**< Embedding context */
    void            *meta;                  /**< Meta server object */
    char            *software;              /**< Server software name and version */
    MprSocket       *sock;                  /**< Listening socket */
    MprDispatcher   *dispatcher;            /**< Event dispatcher */
    HttpNotifier    notifier;               /**< Http state change notification callback */
    HttpListenCallback listenCallback;      /**< Invoked when creating listeners */
    struct MprSsl   *ssl;                   /**< Server SSL configuration */
} HttpServer;

#define HTTP_NOTIFY(conn, state, flags) \
    if (1) { \
        if (conn->notifier) { \
            (conn->notifier)(conn, state, flags); \
        } \
        if (conn->requestNotifier) { \
            (conn->requestNotifier)(conn, state, flags); \
        } \
    } else \

/** 
    Create a server endpoint.
    @description Creates a listening server on the given IP:PORT. Use httpStartServer to begin listening for client
        connections.
    @param http Http object created via #httpCreate
    @param ip IP address on which to listen
    @param port IP port number
    @param dispatcher Dispatcher to use. Can be null.
    @ingroup HttpServer
 */
extern HttpServer *httpCreateServer(Http *http, cchar *ip, int port, MprDispatcher *dispatcher);
extern void httpDestroyServer(HttpServer *server);

extern HttpConn *httpAcceptConn(HttpServer *server);
extern int httpValidateLimits(HttpServer *server, int event, HttpConn *conn);

/**
    Get the meta server object
    @param server HttpServer object created via #httpCreateServer
    @return The server context object defined via httpSetMetaServer
 */
extern void *httpGetMetaServer(HttpServer *server);

/**
    Get if the server is running in asynchronous mode
    @param server HttpServer object created via #httpCreateServer
    @return True if the server is in async mode
 */
extern int httpGetServerAsync(HttpServer *server);

/**
    Get the server context object
    @param server HttpServer object created via #httpCreateServer
    @return The server context object defined via httpSetServerContext
 */
extern void *httpGetServerContext(HttpServer *server);

extern void httpSetServerName(HttpServer *server, cchar *name);
extern void httpSetServerSoftware(HttpServer *server, cchar *software);
extern void httpSetServerSsl(HttpServer *server, struct MprSsl *ssl);
extern int httpLoadSsl(Http *http);

/**
    Set the meta server object
    @param server HttpServer object created via #httpCreateServer
    @param context New meta server object
 */
extern void httpSetMetaServer(HttpServer *server, void *context);

/**
    Control if the server is running in asynchronous mode
    @param server HttpServer object created via #httpCreateServer
    @param enable Set to 1 to enable async mode.
 */
extern void httpSetServerAsync(HttpServer *server, int enable);

/**
    Set the server context object
    @param server HttpServer object created via #httpCreateServer
    @param context New context object
 */
extern void httpSetServerContext(HttpServer *server, void *context);

/** 
    Define a notifier callback for this server.
    @description The notifier callback will be invoked as Http requests are processed.
    @param server HttpServer object created via #httpCreateServer
    @param fn Notifier function. 
    @ingroup HttpConn
 */
extern void httpSetServerNotifier(HttpServer *server, HttpNotifier fn);

/** 
    Start listening for client connections.
    @description Opens the server socket and starts listening for connections.
    @param server HttpServer object created via #httpCreateServer
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup HttpServer
 */
extern int httpStartServer(HttpServer *server);

/** 
    Stop the server listening for client connections.
    @description Closes the server socket endpoint.
    @param server HttpServer object created via #httpCreateServer
    @ingroup HttpServer
 */
extern void httpStopServer(HttpServer *server);

//  MOB
extern void httpSetDocumentRoot(HttpServer *server, cchar *path);
extern void httpSetServerRoot(HttpServer *server, cchar *path);
extern void httpSetIpAddr(HttpServer *server, cchar *ip, int port);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _h_HTTP */

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

/************************************************************************/
/*
 *  End of file "../src/include/http.h"
 */
/************************************************************************/

