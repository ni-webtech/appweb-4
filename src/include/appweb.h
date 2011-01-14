/*
    appweb.h -- Primary header for the Embedthis Appweb HTTP Web Server
 */

#ifndef _h_APPWEB
#define _h_APPWEB 1

/********************************* Includes ***********************************/

#include    "mpr.h"
#include    "http.h"

/********************************* Tunables ***********************************/

#define MA_MAX_CONFIG_DEPTH     16              /**< Max nest of directives in config file */
#define MA_MAX_ACCESS_LOG       20971520        /**< Access file size (20 MB) */
#define MA_MAX_REWRITE          10              /**< Maximum recursive URI rewrites */
#define MA_EJS_START            "start.es"      /**< Default ejs startup script */

/*  These constants are to sanity check user input in the http.conf
 */
#define MA_BOT_BODY             512
#define MA_TOP_BODY             (0x7fffffff)        /* 2 GB */
#define MA_BOT_CHUNK_SIZE       512
#define MA_TOP_CHUNK_SIZE       (4 * 1024 * 1024)   /* 4 MB */
#define MA_BOT_NUM_HEADERS      8
#define MA_TOP_NUM_HEADERS      (20 * 1024)
#define MA_BOT_HEADER           512
#define MA_TOP_HEADER           (20 * 1024 * 1024)
#define MA_BOT_URL              64
#define MA_TOP_URL              (255 * 1024 * 1024) /* 256 MB */
#define MA_BOT_RESPONSE_BODY    512
#define MA_TOP_RESPONSE_BODY    0x7fffffff          /* 2 GB */
#define MA_BOT_STACK            (16 * 1024)
#define MA_TOP_STACK            (4 * 1024 * 1024)
#define MA_BOT_STAGE_BUFFER     (2 * 1024)
#define MA_TOP_STAGE_BUFFER     (1 * 1024 * 1024)   /* 1 MB */
#define MA_BOT_UPLOAD_SIZE      1
#define MA_TOP_UPLOAD_SIZE      0x7fffffff          /* 2 GB */
#define MA_TOP_THREADS          100

#define MA_SERVER_NAME          "Embedthis-Appweb/" BLD_VERSION
#undef HTTP_NAME
#define HTTP_NAME               MA_SERVER_NAME

/****************************** Forward Declarations **************************/

#ifdef __cplusplus
extern "C" {
#endif

#if !DOXYGEN
struct MaSsl;
struct MaServer;
#endif

/********************************** Defines ***********************************/
/**
    Appweb Service
    @description There is one instance of MaAppweb per application. It manages a list of HTTP servers running in
        the application.
    @stability Evolving
    @defgroup Appweb Appweb
    @see Http maCreateApweb maStartApweb maStopApweb
 */
typedef struct MaAppweb {
    struct MaServer     *defaultServer;         /**< Default web server object */
    MprList             *servers;               /**< List of meta server objects */
    Http                *http;                  /**< Http service object */

    char                *user;                  /**< O/S application user name */
    char                *group;                 /**< O/S application group name */

    //  MOB - should this be in http?
    int                 uid;                    /**< User Id */
    int                 gid;                    /**< Group Id */
    int                 userChanged;
    int                 groupChanged;
} MaAppweb;


/** Create the Appweb object.
    @description Appweb uses a singleton Appweb object to manage multiple web servers instances.
    @param ctx Any memory context object returned by mprAlloc
    @return A Http object. Use mprFree to close and release.
    @ingroup Appweb
 */
extern MaAppweb *maCreateAppweb();

/**
    Start Appweb services
    @description This starts listening for requests on all configured servers.
    @param http Http object created via #maCreateHttp
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maStartAppweb(MaAppweb *appweb);

/**
    Stop Appweb services
    @description This stops listening for requests on all configured servers. Shutdown is somewhat graceful.
    @param http Http object created via #maCreateHttp
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maStopAppweb(MaAppweb *appweb);

/**
    Set the Http User
    @description Define the user name under which to run the Appweb service
    @param http Http object created via #maCreateHttp
    @param user User name. Must be defined in the system password file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maSetHttpUser(MaAppweb *appweb, cchar *user);

//  MOB
extern void maGetUserGroup(MaAppweb *appweb);

/**
    Set the Http Group
    @description Define the group name under which to run the Appweb service
    @param http Http object created via #maCreateHttp
    @param group Group name. Must be defined in the system group file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maSetHttpGroup(MaAppweb *appweb, cchar *group);

extern void         maAddServer(MaAppweb *appweb, struct MaServer *server);
extern int          maApplyChangedGroup(MaAppweb *appweb);
extern int          maApplyChangedUser(MaAppweb *appweb);
extern struct MaServer *maLookupServer(MaAppweb *appweb, cchar *name);
extern int          maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname);

//  MOB - name
extern void maSetDefaultServer(MaAppweb *appweb, struct MaServer *server);
extern void maUnloadStaticModules(MaAppweb *appweb);

extern void maSetKeepAliveTimeout(MaAppweb *appweb, int timeout);
extern void maSetTimeout(MaAppweb *appweb, int timeout);
extern void maSetMaxKeepAlive(MaAppweb *appweb, int timeout);
extern void maSetForkCallback(MaAppweb *appweb, MprForkCallback callback, void *data);

/*
    Loadable module entry points
 */
extern int maAuthFilterInit(Http *http, MprModule *mp);
extern int maCgiHandlerInit(Http *http, MprModule *mp);
extern int maChunkFilterInit(Http *http, MprModule *mp);
extern int maDirHandlerInit(Http *http, MprModule *mp);
extern int maEgiHandlerInit(Http *http, MprModule *mp);
extern int maEjsHandlerInit(Http *http, MprModule *mp);
extern int maNetConnectorInit(Http *http, MprModule *mp);
extern int maPhpHandlerInit(Http *http, MprModule *mp);
extern int maRangeFilterInit(Http *http, MprModule *mp);
extern int maSslModuleInit(Http *http, MprModule *mp);
extern int maUploadFilterInit(Http *http, MprModule *mp);

/********************************************************************************/

#define MA_LISTEN_DEFAULT_PORT  0x1         /* Use default port 80 */
#define MA_LISTEN_WILD_PORT     0x2         /* Port spec missing */
#define MA_LISTEN_WILD_IP       0x4         /* IP spec missing (first endpoint) */
#define MA_LISTEN_WILD_IP2      0x8         /* IP spec missing (second+ endpoint) */

/******************************** MaHostAddress *******************************/
/*  
    Flags
 */
#define MA_IPADDR_VHOST 0x1

/** Host Address Mapping
    @stability Evolving
    @defgroup MaHostAddress MaHostAddress
    @see MaHostAddress
 */
typedef struct MaHostAddress {
    char            *ip;                    /**< IP Address for this endpoint */
    int             port;                   /**< Port for this endpoint */
    int             flags;                  /**< Mapping flags */
    MprList         *vhosts;                /**< Vhosts using this address */
} MaHostAddress;


extern MaHostAddress *maCreateHostAddress(cchar *ip, int port);
extern MaHostAddress *maLookupHostAddress(struct MaServer *server, cchar *ip, int port);
extern struct MaHost *maLookupVirtualHost(MaHostAddress *hostAddress, cchar *hostStr);
extern void maInsertVirtualHost(MaHostAddress *hostAddress, struct MaHost *vhost);
extern bool maIsNamedVirtualHostAddress(MaHostAddress *hostAddress);
extern void maSetNamedVirtualHostAddress(MaHostAddress *hostAddress);

/********************************** MaServer **********************************/
/**
    MaServer Control - 
    An application may have any number of HTTP servers, each managed by an instance of the Server class. Typically
    there will be only one server in an application. There may be multiple virtual hosts and one default host for
    each server class. A server will typically be configured by calling the configure method for each server which
    parses a file to define the server and virtual host configuration.
    @stability Evolving
    @defgroup MaServer MaServer
    @see MaServer maCreateWebServer maServiceWebServer maRunWebServer maRunSimpleWebServer maCreateServer 
        maConfigureServer maLoadStaticModules maUnloadStaticModules maSplitConfigValue
 */
typedef struct MaServer {
    MaAppweb        *appweb;                /**< Appweb control object */
    Http            *http;                  /**< Http service object (copy of appweb->http) */
    MprList         *httpServers;           /**< List of MaServers */
    struct MaHost   *defaultHost;           /**< Primary host */
    MprList         *hosts;                 /**< List of host objects */
    MprList         *hostAddresses;         /**< List of HostAddress objects */
    char            *name;                  /**< Unique name for this meta-server */
    char            *serverRoot;            /**< Server root */
    bool            alreadyLogging;         /**< Already logging */
} MaServer;

/** Create a web server
    @description Create a web server configuration based on the supplied config file. Once created, the
        web server should be run by calling #maServiceWebServer. Use this routine when you need access to the Http
        object. If you want a one-line embedding of Appweb, use #maRunWebServer or #maRunSimpleWebServer.
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @return Http object.
    @ingroup MaServer
 */
extern MaServer *maCreateWebServer(cchar *configFile);

/** Service a web server
    @description Run a web server configuration. This is will start http services via #maStartHttp and will service
        incoming Http requests until instructed to exit. This is often used in conjunction with #maCreateWebServer.
    @param http Http object created via #maCreateWebServer or #maCreateHttp.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maServiceWebServer(MaServer *server);

/** Create and run a web server based on a configuration file
    @description Create a web server configuration based on the supplied config file. This routine provides 
        is a one-line embedding of Appweb. If you don't want to use a config file, try the #maRunSimpleWebServer 
        instead. If you need more control, try #maCreateWebServer which exposes the Http object.
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maRunWebServer(cchar *configFile);

/** Create and run a simple web server listening on a single IP address.
    @description Create a simple web server without using a configuration file. The server is created to listen on
        the specified IP addresss and port. This routine provides is a one-line embedding of Appweb. If you want to 
        use a config file, try the #maRunWebServer instead. If you need more control, try #maCreateWebServer which 
        exposes the Http object.
    @param ip IP address on which to listen. Set to "0.0.0.0" to listen on all interfaces.
    @param port Port number to listen to
    @param docRoot Directory containing the documents to serve.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maRunSimpleWebServer(cchar *ip, int port, cchar *docRoot);

/** Create an MaServer object
    @description Create new MaServer object. This routine creates a bare MaServer object, loads any required static
        modules  and performs minimal configuration. To use the server object created, more configuration will be 
        required before starting Http services.
        If you want a one-line embedding of Appweb, use #maRunWebServer or #maRunSimpleWebServer.
    @param http Http object returned from #maCreateHttp
    @param name Name of the web server. This name is used as the initial server name.
    @param root Server root directory
    @param ip If not-null, create and open a listening endpoint on this IP address. If you are configuring via a
        config file, use #maConfigureServer and set ip to null.
    @param port Port number to listen on. Set to -1 if you do not want to open a listening endpoint on ip:port
    @return MaServer A newly created MaServer object. Use mprFree to free and release.
    @ingroup MaServer
 */
extern MaServer *maCreateServer(MaAppweb *appweb, cchar *name, cchar *root, cchar *ip, int port);

/** Configure a web server.
    @description This will configure a web server based on either a configuration file or using the supplied
        IP address and port. 
    @param server MaServer object created via #maCreateServer
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @param serverRoot Admin directory for the server. This overrides the value in the config file.
    @param documentRoot Default directory for web documents to serve. This overrides the value in the config file.
    @param ip IP address to listen on. This overrides the value specified in the config file.
    @param port Port address to listen on. This overrides the value specified in the config file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maConfigureServer(MaServer *server, cchar *configFile, cchar *serverRoot, cchar *documentRoot, 
        cchar *ip, int port);

/**
    Load static modules
    @description Load the statically configured modules, handlers, filters and connectors. The configure program
        can specify a static or shared build of Appweb. The #maCreateServer routine will call maLoadStaticModules
        automatically. It should not be called by in user programs.
    @param http Http object created via #maCreateHttp
    @ingroup MaServer
 */
extern void maLoadStaticModules(MaAppweb *appweb);

extern void     maAddHost(MaServer *server, struct MaHost *host);
extern MaHostAddress *maAddHostAddress(MaServer *server, cchar *ip, int port);

//  MOB -- is this right?
extern void     maAddHttpServer(MaServer *server, HttpServer *httpServer);
extern int      maCreateHostAddresses(MaServer *server, struct MaHost *host, cchar *value);
extern struct MaHost *maLookupHost(MaServer *server, cchar *name);
extern int      maGetConfigValue(char **arg, char *buf, char **nextToken, int quotes);
extern void     maNotifyServerStateChange(HttpConn *conn, int state, int notifyFlags);
extern int      maParseConfig(MaServer *server, cchar *configFile);
extern MaHostAddress *maRemoveHostFromHostAddress(MaServer *server, cchar *ip, int port, struct MaHost *host);
extern void     maSetDefaultHost(MaServer *server, struct MaHost *host);
extern void     maSetDefaultIndex(MaServer *server, cchar *path, cchar *filename);
extern void     maSetDocumentRoot(MaServer *server, cchar *path);
extern void     maSetIpAddr(MaServer *server, cchar *ip, int port);
extern void     maSetServerRoot(MaServer *server, cchar *path);
extern int      maSplitConfigValue(char **s1, char **s2, char *buf, int quotes);
extern int      maStartServer(MaServer *server);
extern int      maStopServer(MaServer *server);
extern int      maValidateConfiguration(MaServer *server);

/************************************* Auth *********************************/
#if BLD_FEATURE_AUTH_FILE
/** 
    User Authorization
    File based authorization backend
    @stability Evolving
    @defgroup MaUser
    @see MaUser
 */
typedef struct MaUser {
    bool            enabled;
    HttpAcl         acl;                    /* Union (or) of all group Acls */
    char            *password;
    char            *realm;
    char            *name;
} MaUser;


/** 
    Group Authorization
    @stability Evolving
    @defgroup MaGroup
    @see MaGroup
 */
typedef struct  MaGroup {
    HttpAcl         acl;
    bool            enabled;
    char            *name;
    MprList         *users;                 /* List of users */
} MaGroup;

//  TODO - simplify this API
//  TODO -- all these routines should be generic (not native) and use some switch table to vector to the right backend method

extern int      maAddGroup(HttpAuth *auth, cchar *group, HttpAcl acl, bool enabled);
extern int      maAddUser(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, bool enabled);
extern int      maAddUserToGroup(HttpAuth *auth, MaGroup *gp, cchar *user);
extern int      maAddUsersToGroup(HttpAuth *auth, cchar *group, cchar *users);
extern HttpAuth   *maCreateAuth(HttpAuth *parent);
extern MaGroup  *maCreateGroup(HttpAuth *auth, cchar *name, HttpAcl acl, bool enabled);
extern MaUser   *maCreateUser(HttpAuth *auth, cchar *realm, cchar *name, cchar *password, bool enabled);
extern int      maDisableGroup(HttpAuth *auth, cchar *group);
extern int      maDisableUser(HttpAuth *auth, cchar *realm, cchar *user);
extern int      maEnableGroup(HttpAuth *auth, cchar *group);
extern int      maEnableUser(HttpAuth *auth, cchar *realm, cchar *user);
extern HttpAcl    maGetGroupAcl(HttpAuth *auth, char *group);
extern cchar    *maGetNativePassword(HttpAuth *auth, cchar *realm, cchar *user);
extern bool     maIsGroupEnabled(HttpAuth *auth, cchar *group);
extern bool     maIsUserEnabled(HttpAuth *auth, cchar *realm, cchar *user);
extern HttpAcl    maParseAcl(HttpAuth *auth, cchar *aclStr);
extern int      maRemoveGroup(HttpAuth *auth, cchar *group);
extern int      maReadGroupFile(MaServer *server, HttpAuth *auth, char *path);
extern int      maReadUserFile(MaServer *server, HttpAuth *auth, char *path);
extern int      maRemoveUser(HttpAuth *auth, cchar *realm, cchar *user);
extern int      maRemoveUserFromGroup(MaGroup *gp, cchar *user);
extern int      maRemoveUsersFromGroup(HttpAuth *auth, cchar *group, cchar *users);
extern int      maSetGroupAcl(HttpAuth *auth, cchar *group, HttpAcl acl);
extern void     maSetRequiredAcl(HttpAuth *auth, HttpAcl acl);
extern void     maUpdateUserAcls(HttpAuth *auth);
extern int      maWriteUserFile(MaServer *server, HttpAuth *auth, char *path);
extern int      maWriteGroupFile(MaServer *server, HttpAuth *auth, char *path);
extern bool     maValidateNativeCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
                    cchar *requiredPass, char **msg);
#endif /* AUTH_FILE */

#if BLD_FEATURE_AUTH_PAM
extern cchar    *maGetPamPassword(HttpAuth *auth, cchar *realm, cchar *user);
extern bool     maValidatePamCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
                    cchar *requiredPass, char **msg);
#endif /* AUTH_PAM */

/************************************ MaDir ***********************************/
/**
    Directory Control
    @stability Evolving
    @defgroup MaDir MaDir
    @see MaDir
 */
typedef struct  MaDir {
#if UNUSED
    struct MaHost   *parent;                /**< Owner of the directory */
#endif
    HttpAuth        *auth;                  /**< Authorization control */
    //TODO MOB - Hosts don't own directories. They are outside hosts
    struct MaHost   *host;                  /**< Host owning this directory */
    char            *indexName;             /**< Default index document name */
    char            *path;                  /**< Directory filename */
    size_t          pathLen;                /**< Length of the directory path */
} MaDir;

extern MaDir    *maCreateBareDir(struct MaHost *host, cchar *path);
extern MaDir    *maCreateDir(struct MaHost *host, cchar *path, MaDir *parent);
extern void     maSetDirIndex(MaDir *dir, cchar *name) ;
extern void     maSetDirPath(MaDir *dir, cchar *filename);

/********************************** MaUploadFile *********************************/

extern int maOpenDirHandler(Http *http);
extern int maOpenEgiHandler(Http *http);
extern int maOpenFileHandler(Http *http);
extern int maOpenSendConnector(Http *http);

/*********************************** MaAlias **********************************/
/**
    Aliases 
    @stability Evolving
    @defgroup MaAlias MaAlias
    @see MaAlias maCreateAlias
 */
typedef struct MaAlias {
#if UNUSED
    struct MaHost   *parent;                /**< Owner of the alias */
#endif
    char            *prefix;                /**< Original URI prefix */
    int             prefixLen;              /**< Prefix length */
    char            *filename;              /**< Alias to a physical path name */
    char            *uri;                   /**< Redirect to a uri */
    int             redirectCode;
} MaAlias;

extern MaAlias *maCreateAlias(cchar *prefix, cchar *name, int code);

/******************************* MaMimeType ******************************/
/**
    Mime Type hash table entry (the URL extension is the key)
    @stability Evolving
    @defgroup MaMimeType MaMimeType
    @see MaMimeType
 */
typedef struct MaMimeType {
    char            *type;
    char            *actionProgram;
} MaMimeType;

/************************************ MaHost **********************************/
/*
    Flags
 */
#define MA_HOST_VHOST           0x2         /* Is a virtual host */
#define MA_HOST_NAMED_VHOST     0x4         /* Named virtual host */
#define MA_HOST_NO_TRACE        0x40        /* Prevent use of TRACE */

/**
    Host Object
    A Host object represents a logical host. It may be single listening HTTP connection endpoint or it may be a virtual host.
    @stability Evolving
    @defgroup MaHost MaHost
    @see MaHost
 */
typedef struct MaHost {
    MaServer        *server;                /**< Meta-server owning this host */
    HttpServer      *httpServer;            /**< HttpServer backing this host */
    struct MaHost   *parent;                /**< Parent host for virtual hosts */
    MprFile         *accessLog;             /**< File object for access logging */
    MprList         *dirs;                  /**< List of Directory definitions */
    MprList         *aliases;               /**< List of Alias definitions */

    //  MOB - who is this used with
    char            *ipAddrPort;            /**< IP:PORT address (with wildcards) */
    char            *ip;                    /**< IP address */
    int             port;                   /**< Listening port number */

    char            *documentRoot;          /**< Default directory for web documents */
    int             flags;                  /**< Host flags */

    //  MOB -- has been moved to Http
    int             traceLevel;             /**< Trace activation level */
    int             traceMaxLength;         /**< Maximum trace file length (if known) */
    int             traceMask;              /**< Request/response trace mask */
    MprHashTable    *traceInclude;          /**< Extensions to include in trace */
    MprHashTable    *traceExclude;          /**< Extensions to exclude from trace */

    int             httpVersion;            /**< HTTP/1.X */
    HttpLoc         *loc;                   /**< Default location */
    MprList         *locations;             /**< List of Location defintions */
    struct MaHost   *logHost;               /**< If set, use this hosts logs */
    MprHashTable    *mimeTypes;             /**< Hash table of mime types (key is extension) */
    char            *mimeFile;              /**< Name of the mime types file */
    char            *moduleDirs;            /**< Directories for modules */

    //  MOB -- should this not be down in http?
    char            *name;                  /**< ServerName directive - used for redirects */

    char            *logFormat;             /**< Access log format */
    char            *logPath;               /**< Access log filename */
} MaHost;


/*
    All these APIs are internal
 */
extern void         maAddConn(MaHost *host, struct HttpConn *conn);
extern void         maAddStandardMimeTypes(MaHost *host);
extern MaHost       *maCreateDefaultHost(MaServer *server, cchar *docRoot, cchar *ip, int port);
extern int          maCreateRequestPipeline(MaHost *host, struct HttpConn *conn, MaAlias *alias);
extern MaAlias      *maGetAlias(MaHost *host, cchar *uri);
extern MaAlias      *maLookupAlias(MaHost *host, cchar *prefix);
extern MaDir        *maLookupBestDir(MaHost *host, cchar *path);
extern MaDir        *maLookupDir(MaHost *host, cchar *path);
extern HttpLoc      *maLookupBestLocation(MaHost *host, cchar *uri);
extern MaHost       *maCreateHost(MaServer *server, HttpServer *httpServer, cchar *ip, HttpLoc *loc);
extern MaHost       *maCreateVirtualHost(MaServer *server, cchar *ipAddrPort, MaHost *host);
extern HttpLoc      *maLookupLocation(MaHost *host, cchar *prefix);
extern MaMimeType   *maAddMimeType(MaHost *host, cchar *ext, cchar *mimetype);
extern cchar        *maGetMimeActionProgram(MaHost *host, cchar *mimetype);
extern cchar        *maLookupMimeType(MaHost *host, cchar *ext);
extern int          maInsertAlias(MaHost *host, MaAlias *newAlias);
extern int          maAddLocation(MaHost *host, HttpLoc *newLocation);
extern int          maInsertDir(MaHost *host, MaDir *newDir);
extern int          maOpenMimeTypes(MaHost *host, cchar *path);
extern void         maRemoveConn(MaHost *host, struct HttpConn *conn);
extern void         maSetHostDocumentRoot(MaHost *host, cchar *path);
extern int          maSetMimeActionProgram(MaHost *host, cchar *mimetype, cchar *actionProgram);
extern int          maStartHost(MaHost *host);
extern int          maStopHost(MaHost *host);
extern void         maSetHostIpAddrPort(MaHost *host, cchar *ipAddrPort);
extern void         maSetHostName(MaHost *host, cchar *name);
extern void         maSetHostDirs(MaHost *host, cchar *path);
extern void         maSetHostTrace(MaHost *host, int level, int mask);
extern void         maSetHostTraceFilter(MaHost *host, int len, cchar *include, cchar *exclude);
extern void         maSetHttpVersion(MaHost *host, int version);
extern void         maSetNamedVirtualHost(MaHost *host);
extern void         maSecureHost(MaHost *host, struct MprSsl *ssl);
extern void         maSetHostFilter(MaHost *host, int length, cchar *include, cchar *exclude);
extern int          maSetupTrace(MaHost *host, cchar *ext);
extern void         maSetVirtualHost(MaHost *host);

#if FUTURE
/******************************** HttpReceiverMatch ******************************/
/*
    HttpReceiverMatch stores "If-Match" and "If-None-Match" request headers.
 */
typedef struct HttpReceiverMatch {
    MprList         etags;
    bool            ifMatch;
} HttpReceiverMatch;


/*
    HttpReceiverModified stores "If-Modified-Since" and "If-Unmodified-Since" request headers.
 */
typedef struct  HttpReceiverModified {
    MprTime         since;
    bool            ifModified;
} HttpReceiverModified;

#endif /* FUTURE */

/******************************************************************************/
/**
    Current config parse state
    @stability Evolving
    @defgroup MaConfigState MaConfigState
    @see MaConfigState
 */
typedef struct MaConfigState {
    MaServer    *server;                    /**< Current meta-server */
    MaHost      *host;                      /**< Current host */
    MaDir       *dir;                       /**< Current directory block */
    HttpLoc     *loc;                       /**< Current location */
    HttpAuth    *auth;                      /**< Current auth object */
    MprFile     *file;                      /**< Config file handle */
    char        *filename;                  /** Config file name */
    int         lineNumber;                 /**< Current line number */
    int         enabled;                    /**< True if the current block is enabled */
} MaConfigState;

extern HttpLoc *maCreateLocationAlias(Http *http, MaConfigState *state, cchar *prefix, cchar *path, 
        cchar *handlerName, int flags);

extern char         *maMakePath(MaHost *host, cchar *file);
extern char         *maReplaceReferences(MaHost *host, cchar *str);
extern void         maSetAccessLog(MaHost *host, cchar *path, cchar *format);
extern int          maStopLogging();
extern int          maStartLogging(cchar *logSpec);
extern void         maSetLogHost(MaHost *host, MaHost *logHost);
extern int          maStartAccessLogging(MaHost *host);
extern int          maStopAccessLogging(MaHost *host);

/************************************ EGI *************************************/

typedef void (MaEgiForm)(HttpQueue *q);
extern int maDefineEgiForm(Http *http, cchar *name, MaEgiForm *form);

#ifdef __cplusplus
} /* extern C */
#endif

/*
    Permit overrides
 */
#include    "customize.h"

#endif /* _h_APPWEB */

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

