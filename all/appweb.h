#include "appwebConfig.h"
#include "http.h"
/*
    appweb.h -- Primary header for the Embedthis Appweb HTTP Web Server
 */

#ifndef _h_APPWEB
#define _h_APPWEB 1

/********************************* Includes ***********************************/

#include    "mpr.h"

/********************************* Tunables ***********************************/

#define MA_MAX_CONFIG_DEPTH     16          /**< Max nest of directives in config file */
#define MA_MAX_ACCESS_LOG       20971520    /**< Access file size (20 MB) */
#define MA_MAX_REWRITE          10          /**< Maximum recursive URI rewrites */

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
    @description There is one instance of Http per application. It manages a list of HTTP servers running in
        the application.
    @stability Evolving
    @defgroup Http Http
    @see Http maCreateHttp maStartHttp maStopHttp
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
} MaAppweb;


/** Create the Appweb object.
    @description Appweb uses a singleton Appweb object to manage multiple web servers instances.
    @param ctx Any memory context object returned by mprAlloc
    @return A Http object. Use mprFree to close and release.
    @ingroup Http
 */
extern MaAppweb *maCreateAppweb(MprCtx ctx);

/**
    Start Appweb services
    @description This starts listening for requests on all configured servers.
    @param http Http object created via #maCreateHttp
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Http
 */
extern int maStartAppweb(MaAppweb *appweb);

/**
    Stop Appweb services
    @description This stops listening for requests on all configured servers. Shutdown is somewhat graceful.
    @param http Http object created via #maCreateHttp
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Http
 */
extern int maStopAppweb(MaAppweb *appweb);

/**
    Set the Http User
    @description Define the user name under which to run the Appweb service
    @param http Http object created via #maCreateHttp
    @param user User name. Must be defined in the system password file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Http
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
    @ingroup Http
 */
extern int maSetHttpGroup(MaAppweb *appweb, cchar *group);

extern void         maAddServer(MaAppweb *appweb, struct MaServer *server);
extern int          maApplyChangedGroup(MaAppweb *appweb);
extern int          maApplyChangedUser(MaAppweb *appweb);
extern struct MaServer *maLookupServer(MaAppweb *appweb, cchar *name);
extern int          maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname);

//  MOB - name
extern void         maSetDefaultServer(MaAppweb *appweb, struct MaServer *server);

extern void maSetKeepAliveTimeout(MaAppweb *appweb, int timeout);
extern void maSetTimeout(MaAppweb *appweb, int timeout);
extern void maSetMaxKeepAlive(MaAppweb *appweb, int timeout);

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

/********************************* HttpServer ***********************************/

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


extern MaHostAddress *maCreateHostAddress(MprCtx ctx, cchar *ip, int port);
extern MaHostAddress *maLookupHostAddress(struct MaServer *server, cchar *ip, int port);
extern struct MaHost *maLookupVirtualHost(MaHostAddress *hostAddress, cchar *hostStr);
extern void maInsertVirtualHost(MaHostAddress *hostAddress, struct MaHost *vhost);
extern bool maIsNamedVirtualHostAddress(MaHostAddress *hostAddress);
extern void maSetNamedVirtualHostAddress(MaHostAddress *hostAddress);

/********************************** HttpServer **********************************/
/**
    MaServer Control - 
    An application may have any number of HTTP servers, each managed by an instance of the Server class. Typically
    there will be only one server in an application. There may be multiple virtual hosts and one default host for
    each server class. A server will typically be configured by calling the configure method for each server which
    parses a file to define the server and virtual host configuration.
    @stability Evolving
    @defgroup HttpServer HttpServer
    @see HttpServer maCreateWebServer maServiceWebServer maRunWebServer maRunSimpleWebServer maCreateServer 
        maConfigureServer maLoadStaticModules maUnloadStaticModules maSplitConfigValue
 */
typedef struct MaServer {
    MaAppweb        *appweb;                /**< Appweb control object */
    Http            *http;                  /**< Http service object (copy of appweb->http) */
    MprList         *httpServers;           /**< List of HttpServers */
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
    @ingroup HttpServer
 */
extern MaServer *maCreateWebServer(cchar *configFile);

/** Service a web server
    @description Run a web server configuration. This is will start http services via #maStartHttp and will service
        incoming Http requests until instructed to exit. This is often used in conjunction with #maCreateWebServer.
    @param http Http object created via #maCreateWebServer or #maCreateHttp.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup HttpServer
 */
extern int maServiceWebServer(MaServer *server);

/** Create and run a web server based on a configuration file
    @description Create a web server configuration based on the supplied config file. This routine provides 
        is a one-line embedding of Appweb. If you don't want to use a config file, try the #maRunSimpleWebServer 
        instead. If you need more control, try #maCreateWebServer which exposes the Http object.
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup HttpServer
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
    @ingroup HttpServer
 */
extern int maRunSimpleWebServer(cchar *ip, int port, cchar *docRoot);

/** Create a HttpServer object
    @description Create new HttpServer object. This routine creates a bare HttpServer object, loads any required static
        modules  and performs minimal configuration. To use the server object created, more configuration will be 
        required before starting Http services.
        If you want a one-line embedding of Appweb, use #maRunWebServer or #maRunSimpleWebServer.
    @param http Http object returned from #maCreateHttp
    @param name Name of the web server. This name is used as the initial server name.
    @param root Server root directory
    @param ip If not-null, create and open a listening endpoint on this IP address. If you are configuring via a
        config file, use #maConfigureServer and set ip to null.
    @param port Port number to listen on. Set to -1 if you do not want to open a listening endpoint on ip:port
    @return HttpServer A newly created HttpServer object. Use mprFree to free and release.
    @ingroup HttpServer
 */
extern MaServer *maCreateServer(MaAppweb *appweb, cchar *name, cchar *root, cchar *ip, int port);

//  TODO - this seems to be missing the server root directory for when using IP:port. It has a document root.
//  TODO - Seems better to split this into 2 APIs. One for a config file and one for manual configuration.
/** Configure a web server.
    @description This will configure a web server based on either a configuration file or using the supplied
        IP address and port. 
    @param server MaServer object created via #maCreateServer
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @param ip If using a config file, set to null. Otherwise, set to a host name or IP address.
    @param port If using a config file, set to -1. Otherwise, set to the port number to listen on.
    @param documentRoot If not using a config file, set this to the directory containing the web documents to serve.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup HttpServer
 */
extern int maConfigureServer(MaServer *server, cchar *configFile, cchar *ip, int port, cchar *documentRoot);

/**
    Load static modules
    @description Load the statically configured modules, handlers, filters and connectors. The configure program
        can specify a static or shared build of Appweb. The #maCreateServer routine will call maLoadStaticModules
        automatically. It should not be called by in user programs.
    @param http Http object created via #maCreateHttp
    @ingroup HttpServer
 */
extern void maLoadStaticModules(MaAppweb *appweb);

extern void     maAddHost(MaServer *server, struct MaHost *host);
extern void     maAddHttpServer(MaServer *server, HttpServer *httpServer);
extern int      maCreateHostAddresses(MaServer *server, struct MaHost *host, cchar *value);
extern struct MaHost *maLookupHost(MaServer *server, cchar *name);
extern int      maGetConfigValue(MprCtx ctx, char **arg, char *buf, char **nextToken, int quotes);
extern void     maNotifyServerStateChange(HttpConn *conn, int state, int notifyFlags);
extern int      maParseConfig(MaServer *server, cchar *configFile);
extern void     maSetDefaultHost(MaServer *server, struct MaHost *host);
extern void     maSetDefaultIndex(MaServer *server, cchar *path, cchar *filename);
extern void     maSetServerRoot(MaServer *server, cchar *path);
extern int      maSplitConfigValue(MprCtx ctx, char **s1, char **s2, char *buf, int quotes);
extern int      maStartServer(MaServer *server);
extern int      maStopServer(MaServer *server);
extern void     maUnloadStaticModules(MaAppweb *appweb);

/************************************* HttpAuth *********************************/
#if BLD_FEATURE_AUTH_FILE
/** User Authorization
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


/** Group Authorization
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
extern HttpAuth   *maCreateAuth(MprCtx ctx, HttpAuth *parent);
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
    HttpAuth        *auth;                  /**< Authorization control */
    //TODO MOB - Hosts don't own directories. They are outside hosts
    struct MaHost   *host;                  /**< Host owning this directory */
    char            *indexName;             /**< Default index document name */
    char            *path;                  /**< Directory filename */
    int             pathLen;                /**< Length of the directory path */
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
    char            *prefix;                /**< Original URI prefix */
    int             prefixLen;              /**< Prefix length */
    char            *filename;              /**< Alias to a physical path name */
    char            *uri;                   /**< Redirect to a uri */
    int             redirectCode;
} MaAlias;

extern MaAlias *maCreateAlias(MprCtx ctx, cchar *prefix, cchar *name, int code);

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
    A Host object represents a logical host. It may be single listening HTTP connection endpoint or it may be a virtual
    host. 
    @stability Evolving
    @defgroup MaHost MaHost
    @see MaHost
 */
typedef struct MaHost {
    MaServer        *server;                /**< Meta-server owning this host */
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

    int             httpVersion;            /**< HTTP/1.X */
    HttpLocation    *location;              /**< Default location */
    MprList         *locations;             /**< List of Location defintions */
    struct MaHost   *logHost;               /**< If set, use this hosts logs */
    MprHashTable    *mimeTypes;             /**< Hash table of mime types (key is extension) */
    char            *mimeFile;              /**< Name of the mime types file */
    char            *moduleDirs;            /**< Directories for modules */
    char            *name;                  /**< ServerName directive */

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
extern HttpLocation *maLookupBestLocation(MaHost *host, cchar *uri);
extern MaHost       *maCreateHost(MaServer *server, cchar *ip, HttpLocation *location);
extern MaHost       *maCreateVirtualHost(MaServer *server, cchar *ipAddrPort, MaHost *host);
extern HttpLocation *maLookupLocation(MaHost *host, cchar *prefix);
extern MaMimeType   *maAddMimeType(MaHost *host, cchar *ext, cchar *mimetype);
extern cchar        *maGetMimeActionProgram(MaHost *host, cchar *mimetype);
extern cchar        *maLookupMimeType(MaHost *host, cchar *ext);
extern int          maInsertAlias(MaHost *host, MaAlias *newAlias);
extern int          maAddLocation(MaHost *host, HttpLocation *newLocation);
extern int          maInsertDir(MaHost *host, MaDir *newDir);
extern int          maOpenMimeTypes(MaHost *host, cchar *path);
extern void         maRemoveConn(MaHost *host, struct HttpConn *conn);
extern int          maSetMimeActionProgram(MaHost *host, cchar *mimetype, cchar *actionProgram);
extern int          maStartHost(MaHost *host);
extern int          maStopHost(MaHost *host);
extern void         maSetDocumentRoot(MaHost *host, cchar *dir) ;
extern void         maSetHostIpAddrPort(MaHost *host, cchar *ipAddrPort);
extern void         maSetHostName(MaHost *host, cchar *name);
extern void         maSetHttpVersion(MaHost *host, int version);

extern void         maSetNamedVirtualHost(MaHost *host);
extern void         maSecureHost(MaHost *host, struct MprSsl *ssl);
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
    HttpLocation *location;                 /**< Current location */
    HttpAuth    *auth;                      /**< Current auth object */
    MprFile     *file;                      /**< Config file handle */
    char        *filename;                  /** Config file name */
    int         lineNumber;                 /**< Current line number */
    int         enabled;                    /**< True if the current block is enabled */
} MaConfigState;

extern HttpLocation *maCreateLocationAlias(Http *http, MaConfigState *state, cchar *prefix, cchar *path, 
        cchar *handlerName, int flags);

extern char         *maMakePath(MaHost *host, cchar *file);
extern char         *maReplaceReferences(MaHost *host, cchar *str);
extern void         maSetAccessLog(MaHost *host, cchar *path, cchar *format);
extern int          maStopLogging(MprCtx ctx);
extern int          maStartLogging(MprCtx ctx, cchar *logSpec);
extern void         maSetLogHost(MaHost *host, MaHost *logHost);
extern int          maStartAccessLogging(MaHost *host);
extern int          maStopAccessLogging(MaHost *host);

/************************************ EGI *************************************/

typedef void (MaEgiForm)(HttpQueue *q);
extern int maDefineEgiForm(Http *http, cchar *name, MaEgiForm *form);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _h_APPWEB */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.

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

    @end
 */

/**
 *  appwebMonitor.h - Monitor Header
 *
 *  Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#ifndef _h_APPWEB_MONITOR
#define _h_APPWEB_MONITOR 1

#define APPWEB_MONITOR_MESSAGE      (WM_USER + 30)
#define APPWEB_QUERY_PORT_MESSAGE   (WM_USER + 31)
#define APPWEB_SOCKET_MESSAGE       (WM_USER + 32)

#endif /* _h_APPWEB_MONITOR  */

/*
 *  @copy   default
 *  
 *  Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
 *  
 *  This software is distributed under commercial and open source licenses.
 *  You may use the GPL open source license described below or you may acquire 
 *  a commercial license from Embedthis Software. You agree to be fully bound 
 *  by the terms of either license. Consult the LICENSE.TXT distributed with 
 *  this software for full details.
 *  
 *  This software is open source; you can redistribute it and/or modify it 
 *  under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version. See the GNU General Public License for more 
 *  details at: http://www.embedthis.com/downloads/gplLicense.html
 *  
 *  This program is distributed WITHOUT ANY WARRANTY; without even the 
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *  
 *  This GPL license does NOT permit incorporating this software into 
 *  proprietary programs. If you are unable to comply with the GPL, you must
 *  acquire a commercial license to use this software. Commercial licenses 
 *  for this software and support services are available from Embedthis 
 *  Software at http://www.embedthis.com 
 *  
 *  @end
 */


/******************************************************************************/
/* 
 *  This file is an amalgamation of all the individual source code files for
 *  Http Library 0.5.0.
 *
 *  Catenating all the source into a single file makes embedding simpler and
 *  the resulting application faster, as many compilers can do whole file
 *  optimization.
 *
 *  If you want to modify http, you can still get the whole source
 *  as individual files if you need.
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
struct HttpLocation;
struct HttpPacket;
struct HttpLimits;
struct HttpQueue;
struct HttpReceiver;
struct HttpServer;
struct HttpStage;
struct HttpTransmitter;
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
    #define HTTP_CONN_MEM              ((16 * 1024) - MPR_HEAP_OVERHEAD) /**< Virt memory per connection */
    #define HTTP_REC_MEM               ((1 * 1024 * 1024) - MPR_HEAP_OVERHEAD) /**< Initial virt memory arena size */
    #define HTTP_MAX_CHUNK             (8 * 1024)            /**< Max chunk size for transfer chunk encoding */
    #define HTTP_MAX_HEADERS           2048                  /**< Max size of the headers */
    #define HTTP_MAX_IOVEC             16                    /**< Number of fragments in a single socket write */
    #define HTTP_MAX_NUM_HEADERS       20                    /**< Max number of header lines */
    #define HTTP_MAX_RECEIVE_BODY      (128 * 1024 * 1024)   /**< Maximum incoming body size */
    #define HTTP_MAX_REQUESTS          20                    /**< Max concurrent requests */
    #define HTTP_MAX_TRANSMISSION_BODY (128 * 1024 * 1024)   /**< Max buffer for response data */
    #define HTTP_MAX_STAGE_BUFFER      (4 * 1024)            /**< Max buffer for any stage */
    #define HTTP_MAX_UPLOAD            (10 * 1024 * 1024)    /* Max size of uploaded document */

#elif BLD_TUNE == MPR_TUNE_BALANCED
    /*  
        Tune balancing speed and size
     */
    #define HTTP_BUFSIZE               (4 * 1024)
    #define HTTP_CONN_MEM              ((32 * 1024) - MPR_HEAP_OVERHEAD)
    #define HTTP_REC_MEM               ((2 * 1024 * 1024) - MPR_HEAP_OVERHEAD)
    #define HTTP_MAX_CHUNK             (8 * 1024)
    #define HTTP_MAX_HEADERS           (8 * 1024)
    #define HTTP_MAX_IOVEC             24
    #define HTTP_MAX_NUM_HEADERS       40
    #define HTTP_MAX_RECEIVE_BODY      (128 * 1024 * 1024)
    #define HTTP_MAX_REQUESTS          50
    #define HTTP_MAX_TRANSMISSION_BODY (256 * 1024 * 1024)
    #define HTTP_MAX_STAGE_BUFFER      (32 * 1024)
    #define HTTP_MAX_UPLOAD            0x7fffffff

#else
    /*  
        Tune for speed
     */
    #define HTTP_BUFSIZE               (8 * 1024)
    #define HTTP_CONN_MEM              ((64 * 1024) - MPR_HEAP_OVERHEAD)
    #define HTTP_REC_MEM               ((4 * 1024 * 1024) - MPR_HEAP_OVERHEAD)
    #define HTTP_MAX_CHUNK             (8 * 1024) 
    #define HTTP_MAX_HEADERS           (8 * 1024)
    #define HTTP_MAX_IOVEC             32
    #define HTTP_MAX_NUM_HEADERS       256
    #define HTTP_MAX_RECEIVE_BODY      (256 * 1024 * 1024)
    #define HTTP_MAX_REQUESTS          1000
    #define HTTP_MAX_TRANSMISSION_BODY 0x7fffffff
    #define HTTP_MAX_STAGE_BUFFER      (64 * 1024)
    #define HTTP_MAX_UPLOAD            0x7fffffff
#endif


/*  
    Other constants
 */
#define HTTP_DEFAULT_MAX_THREADS  10                /**< Default number of threads */
#define HTTP_KEEP_TIMEOUT         60000             /**< Keep connection alive timeout */
#define HTTP_MAX_KEEP_ALIVE       100               /**< Maximum requests per connection */
#define HTTP_MAX_PASS             64                /**< Size of password */
#define HTTP_MAX_SECRET           32                /**< Size of secret data for auth */
#define HTTP_PACKET_ALIGN(x)      (((x) + 0x3FF) & ~0x3FF)
#define HTTP_RANGE_BUFSIZE        128               /**< Size of a range boundary */
#define HTTP_RETRIES              3                 /**< Default number of retries for client requests */
#define HTTP_SERVER_TIMEOUT       (300 * 1000)
#define HTTP_TIMER_PERIOD         1000              /**< Timer checks ever 1 second */

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
#define HTTP_CODE_REQUEST_TIME_OUT          408
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
#define HTTP_CODE_GATEWAY_TIME_OUT          504
#define HTTP_CODE_BAD_VERSION               505
#define HTTP_CODE_INSUFFICIENT_STORAGE      507

/*
    Proprietary HTTP status codes.
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
    struct HttpLimits *limits;              /**< Pointer to http->limits */
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
    struct HttpStage *dirHandler;           /**< Directory listing handler */
    struct HttpStage *egiHandler;           /**< Embedded Gateway Interface (EGI) handler */
    struct HttpStage *ejsHandler;           /**< Ejscript Web Framework handler */
    struct HttpStage *fileHandler;          /**< Static file handler */
    struct HttpStage *passHandler;          /**< Pass through handler */
    struct HttpStage *uploadFilter;         /**< Upload filter */

    struct HttpLocation *location;          /**< Default location block */

    MprEvent        *timer;                 /**< Admin service timer */
    MprTime         now;                    /**< When was the currentDate last computed */
    MprMutex        *mutex;
    HttpGetPassword getPassword;            /**< Lookup password callback */
    HttpValidateCred validateCred;          /**< Validate user credentials callback */

    void            *context;               /**< Embedding context */
    char            *currentDate;           /**< Date string for HTTP response headers */
    char            *expiresDate;           /**< Convenient expiry date (1 day in advance) */
    char            *secret;                /**< Random bytes for authentication */
    char            *defaultHost;           /**< Default ip address */
    int             defaultPort;            /**< Default port */
    char            *proxyHost;             /**< Proxy ip address */
    int             proxyPort;              /**< Proxy port */
    int             keepAliveTimeout;       /**< Default timeout for keep-alive */
    int             maxKeepAlive;           /**< Default maximum keep-alive count */
    int             sslLoaded;              /**< True when the SSL provider has been loaded */
    int             timeout;                /**< Default time a request can take */
    int             traceEnabled;           /**< Trace method enabled */

    void            (*rangeService)(struct HttpQueue *q, HttpRangeProc fill);
} Http;

/**
    Create a Http connection object
    @description Create a new http connection object. This creates an object that can be initialized and then
        used with mprHttpRequest. Destroy with mprFree.
    @param ctx Any memory allocation context created by MprAlloc
    @return A newly allocated HttpConn structure. Caller must free using mprFree.
    @ingroup Http
 */
extern Http *httpCreate(MprCtx ctx);

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
extern void httpEnableTraceMethod(Http *http, bool on);

/**
    Get the http context object
    @param http Http service object.
    @return The http context object defined via httpSetContext
 */
extern void *httpGetContext(Http *http);

/**
    Get the time as an ISO date string
    @param ctx Any memory allocation context created by MprAlloc
    @param sbuf Optional path buffer. If supplied, the modified time of the path is used. If NULL, then the current
        time is used.
    @return RFC822 formatted date string. Caller must free.
 */
extern char *httpGetDateString(MprCtx ctx, MprPath *sbuf);

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

/** 
    Http limits
    @stability Evolving
    @defgroup HttpLimits HttpLimits
    @see HttpLimits
 */
typedef struct HttpLimits {
    int             maxReceiveBody;         /**< Max size of receive data */
    int             maxChunkSize;           /**< Max chunk size for transfer encoding */
    int             maxHeader;              /**< Max size of the total header */
    int             maxNumHeaders;          /**< Max number of lines of header */
    int             maxTransmissionBody;    /**< Max size of transmission body content */
    int             maxRequests;            /**< Maximum number of concurrent requests */
    int             maxStageBuffer;         /**< Max buffering by any pipeline stage */
    int             maxThreads;             /**< Max number of pool threads */
    int             minThreads;             /**< Min number of pool threads */
    int             maxUploadSize;          /**< Max size of an uploaded file */
    int             maxUri;                 /**< Max size of a uri */
    int             threadStackSize;        /**< Stack size for each pool thread */
} HttpLimits;

/** 
    URI management
    @description The HTTP provides routines for formatting and parsing URIs. Routines are also provided
        to escape dangerous characters for URIs as well as HTML content and shell commands.
    @stability Evolving
    @see HttpConn, httpCreateUri, httpCreateUriFromParts, httpFormatUri, httpEscapeCmd, httpEscapeHtml, httpUrlEncode, 
        httpUrlDecode, httpValidateUri, httpLookupMimeType
    @defgroup HttpUri HttpUri
 */
typedef struct HttpUri {
    /*  
        These are pointers into the parsedUriBuf.
     */
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
    char        *parsedUriBuf;          /**< Allocated storage for parsed uri */
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
    @param ctx Any memory allocation context created by MprAlloc
    @param uri Uri string to parse
    @param complete Add missing components. ie. Add scheme, host and port if not supplied. 
    @return A newly allocated HttpUri structure. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern HttpUri *httpCreateUri(MprCtx ctx, cchar *uri, int complete);

/** 
    Format a URI
    @description Format a URI string using the input components.
    @param ctx Any memory allocation context created by MprAlloc
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
extern char *httpFormatUri(MprCtx ctx, cchar *scheme, cchar *host, int port, cchar *path, cchar *ref, cchar *query, 
        int complete);

extern HttpUri *httpCreateUriFromParts(MprCtx ctx, cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, 
        cchar *query);

/** 
    Encode a string escaping typical command (shell) characters
    @description Encode a string escaping all dangerous characters that have meaning for the unix or MS-DOS command shells.
    @param ctx Any memory allocation context created by MprAlloc
    @param cmd Command string to encode
    @param escChar Escape character to use when encoding the command.
    @return An allocated string containing the escaped command. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern char *httpEscapeCmd(MprCtx ctx, cchar *cmd, int escChar);

/**
    Encode a string by escaping typical HTML characters
    @description Encode a string escaping all dangerous characters that have meaning in HTML documents
    @param ctx Any memory allocation context created by MprAlloc
    @param html HTML content to encode
    @return An allocated string containing the escaped HTML. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern char *httpEscapeHtml(MprCtx ctx, cchar *html);

/** 
    Get the mime type for an extension.
    This call will return the mime type from a limited internal set of mime types for the given path or extension.
    @param ctx Any memory allocation context created by MprAlloc
    @param ext Path or extension to examine
    @returns Mime type. This is a static string. Caller must not free.
 */
extern cchar *httpLookupMimeType(MprCtx ctx, cchar *ext);

/** 
    Encode a string by escaping URL characters
    @description Encode a string escaping all characters that have meaning for URLs.
    @param ctx Any memory allocation context created by MprAlloc
    @param url URL to encode
    @return An allocated string containing the encoded URL. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern char *httpUrlEncode(MprCtx ctx, cchar *url);

/** 
    Decode a URL string by de-scaping URL characters
    @description Decode a string with www-encoded characters that have meaning for URLs.
    @param ctx Any memory allocation context created by MprAlloc
    @param url URL to decode
    @return A reference to the buf argument.
    @ingroup HttpUri
 */
extern char *httpUrlDecode(MprCtx ctx, cchar *url);

/** 
    Convert a Uri to a string.
    @description Convert the given Uri to a string, optionally completing missing parts such as the host, port and path.
    @param ctx Any memory allocation context created by MprAlloc
    @param uri A Uri object created via httpCreateUri 
    @param complete Fill in missing parts of the uri
    @return A newly allocated uri string. Caller must free using $mprFree.
    @ingroup HttpUri
 */
extern char *httpUriToString(MprCtx ctx, HttpUri *uri, int complete);

/** 
    Validate a URL
    @description Validate and canonicalize a URL. This removes redundant "./" sequences and simplifies "../dir" 
        references. This operates in-situ and modifies the existing string.
    @param ctx Any memory allocation context created by MprAlloc
    @param url Uri string to validate
    @return A validated url.
    @ingroup HttpUri
 */
extern char *httpValidateUri(MprCtx ctx, char *url);

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
    MprOffset           start;                  /**< Start of range */
    MprOffset           end;                    /**< End byte of range + 1 */
    int                 len;                    /**< Redundant range length */
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
    @see HttpPacket HttpQueue httpCreatePacket, httpCreateConnPacket httpCreateDataPacket httpCreateEndPacket 
        httpFreePacket httpJoinPacket httpSplitPacket httpGetPacketLength httpCreateHeaderPacket httpGetPacket
        httpJoinPacketForService httpPutForService httpIsPacketTooBig httpSendPacket httpPutBackPacket 
        httpPutForService httpSendPacketToNext httpResizePacket
 */
typedef struct HttpPacket {
    MprBuf          *prefix;                /**< Prefix message to be emitted before the content */
    MprBuf          *content;               /**< Chunk content */
    MprBuf          *suffix;                /**< Prefix message to be emitted after the content */
    int             flags;                  /**< Packet flags */
    int             entityLength;           /**< Entity length. Content is null. */
    struct HttpPacket *next;                /**< Next packet in chain */
} HttpPacket;


/** 
    Create a data packet
    @description Create a packet of the required size.
    @param ctx A conn or request memory context object.
    @param size Size of the package data storage.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreatePacket(MprCtx ctx, int size);

/**
    Create a connection packet packet
    @description Create a packet of the required size that is owned by the connection
    @param conn HttpConn connection object
    @param size Size of the package data storage.
    @return HttpPacket object.
*/
extern HttpPacket *httpCreateConnPacket(struct HttpConn *conn, int size);

/** 
    Free a packet. This recycles a packet for rapid reuse
    @param q Queue reference
    @param packet Packet to free
 */
extern void httpFreePacket(struct HttpQueue *q, HttpPacket *packet);

/** 
    Create a data packet
    @description Create a packet and set the HTTP_PACKET_DATA flag
        Data packets convey data through the response pipeline.
    @param ctx A conn or request memory context object.
    @param size Size of the package data storage.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateDataPacket(MprCtx ctx, int size);

/** 
    Create an eof packet
    @description Create an end-of-stream packet and set the HTTP_PACKET_END flag. The end pack signifies the 
        end of data. It is used on both incoming and outgoing streams through the request/response pipeline.
    @param ctx A conn or request memory context object.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateEndPacket(MprCtx ctx);

/** 
    Create a response header packet
    @description Create a response header packet and set the HTTP_PACKET_HEADER flag. 
        A header packet is used by the pipeline to hold the response headers.
    @param ctx A conn or request memory context object.
    @return HttpPacket object.
    @ingroup HttpPacket
 */
extern HttpPacket *httpCreateHeaderPacket(MprCtx ctx);

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
    @param ctx Any memory allocation context created by MprAlloc
    @param packet Packet to split
    @param offset Location in the original packet at which to split
    @return New HttpPacket object containing the data after the offset. No need to free, unless you have a very long
        running request. Otherwise the packet memory will be released automatically when the request completes.
    @ingroup HttpPacket
 */
extern HttpPacket *httpSplitPacket(MprCtx ctx, HttpPacket *packet, int offset);

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
    Put a packet onto the service queue
    @description Add a packet to the service queue. If serviceQ is true, the queue will be scheduled for service.
    @param q Queue reference
    @param packet Packet to join to the queue
    @param serviceQ If true, schedule the queue for service
    @ingroup HttpQueue
 */
extern void httpPutForService(struct HttpQueue *q, HttpPacket *packet, bool serviceQ);


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
    Put a packet onto a service queue
    @description Put the packet onto the service queue and optionally schedule the queue for service.
    @param q Queue reference
    @param packet Packet to put
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
extern int httpResizePacket(struct HttpQueue *q, HttpPacket *packet, int size);

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
#define HTTP_QUEUE_DISABLED       0x2         /**< Queue's service routine is disabled */
#define HTTP_QUEUE_FULL           0x4         /**< Queue is full */
#define HTTP_QUEUE_ALL            0x8         /**< Queue has all the data there is and will be */
#define HTTP_QUEUE_SERVICED       0x10        /**< Queue has been serviced at least once */
#define HTTP_QUEUE_EOF            0x20        /**< Queue at end of data */
#define HTTP_QUEUE_STARTED        0x40        /**< Queue started */

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
        httpPutForService httpPutNext httpRemoveQueue httpResizePacket httpScheduleQueue httpSendPacket httpSendPackets 
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
    int                 count;                  /**< Bytes in queue */
    int                 max;                    /**< Maxiumum queue size */
    int                 low;                    /**< Low water mark for flow control */
    int                 flags;                  /**< Queue flags */
    int                 packetSize;             /**< Maximum acceptable packet size */
    int                 direction;              /**< Flow direction */
    void                *queueData;             /**< Stage instance data */

    /*  
        Connector instance data. Put here to save a memory allocation.
     */
    MprIOVec            iovec[HTTP_MAX_IOVEC];
    int                 ioIndex;                /**< Next index into iovec */
    int                 ioCount;                /**< Count of bytes in iovec */
    int                 ioFileEntry;            /**< Has file entry in iovec */
    int                 ioFileOffset;           /**< The next file position to use */
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
extern int httpGetQueueRoom(HttpQueue *q);

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
extern void httpOpenQueue(HttpQueue *q, int chunkSize);

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
extern int httpWrite(HttpQueue *q, cchar *fmt, ...);

/** 
    Write a block of data to the queue
    @description Write a block of data into packets onto the end of the queue. Data packets will be created
        as required to store the write data.
    @param q Queue reference
    @param buf Buffer containing the write data
    @param size of the data in buf
    @param block Set to true to block and wait for data to drain if the downstream queue is full. If false, the
        call may return not having written all the data. The return value indicates how many bytes were actually written.
    @return A count of the bytes actually written
    @ingroup HttpQueue
 */
extern int httpWriteBlock(HttpQueue *q, cchar *buf, int size, bool block);

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
extern int httpWriteString(HttpQueue *q, cchar *s);

/* Internal */
extern HttpQueue *httpFindPreviousQueue(HttpQueue *q);
extern bool httpDrainQueue(HttpQueue *q, bool block);
extern HttpQueue *httpCreateQueue(struct HttpConn *conn, struct HttpStage *stage, int direction, HttpQueue *prev);
extern HttpQueue *httpGetNextQueueForService(HttpQueue *q);
extern void httpInitQueue(struct HttpConn *conn, HttpQueue *q, cchar *name);
extern void httpInitSchedulerQueue(HttpQueue *q);
extern void httpInsertQueue(HttpQueue *prev, HttpQueue *q);
extern int httpIsEof(struct HttpConn *conn);

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

#define HTTP_STAGE_INCOMING       0x400000
#define HTTP_STAGE_OUTGOING       0x800000

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
extern int httpOpenAuthFilter(Http *http);
extern int httpOpenChunkFilter(Http *http);
extern int httpOpenPassHandler(Http *http);
extern int httpOpenRangeFilter(Http *http);
extern int httpOpenUploadFilter(Http *http);

/* 
    Connection flags
 */
#define HTTP_CONN_PIPE_CREATED      0x1     /**< Request pipeline created */

/** 
    Notification flags
 */
#define HTTP_NOTIFY_READABLE        0x1     /**< The request has data available for reading */
#define HTTP_NOTIFY_WRITABLE        0x2     /**< The request is now writable (post / put data) */

/*  
    Connection / Request states
 */
#define HTTP_STATE_BEGIN            1       /**< Ready for a new request */
#define HTTP_STATE_STARTED          2       /**< A new request has started */
#define HTTP_STATE_WAIT             3       /**< Waiting for the response */
#define HTTP_STATE_PARSED           4       /**< Headers have been parsed, handler can start */
#define HTTP_STATE_CONTENT          5       /**< Reading posted content */
#define HTTP_STATE_PROCESS          6       /**< Content received, handler can process */
#define HTTP_STATE_RUNNING          7       /**< Handler running */
#define HTTP_STATE_ERROR            8       /**< Request in error */
#define HTTP_STATE_COMPLETE         9       /**< Request complete */

/** 
    Notifier and event callbacks.
 */
typedef MprEventProc HttpCallback;

/** 
    Http Connections
    @description The HttpConn object represents a TCP/IP connection to the client. A connection object is created for
        each socket connection initiated by the client. One HttpConn object may service many Http requests due to 
        HTTP/1.1 keep-alive.
    @stability Evolving
    @defgroup HttpConn HttpConn
    @see HttpConn HttpReceiver HttpReceiver HttpTransmitter HttpQueue HttpStage
        httpCreateConn httpCloseConn httpCompleteRequest httpCreatePipeline httpDestroyPipeline httpDiscardPipeline
        httpError httpGetAsync httpGetConnContext httpGetError httpGetKeepAliveCount httpPrepConn httpProcessPipeline
        httpServiceQueues httpSetAsync httpSetCredentials httpSetCallback httpSetConnContext
        httpSetConnNotifier httpSetKeepAliveCount httpSetProtocol httpSetRetries httpSetState httpSetTimeout
        httpStartPipeline httpWritable
 */
typedef struct HttpConn {
    /*  Ordered for debugability */

    int             state;                  /**< Connection state */
    int             flags;                  /**< Connection flags */
    int             error;                  /**< A request error has occurred */
    int             threaded;               /**< Request running in a thread */

    Http            *http;                  /**< Http service object  */
    HttpCallback    callback;               /**< Http I/O event callback */
    void            *callbackArg;
    HttpLimits      *limits;                /**< Service limits */
    MprHashTable    *stages;                /**< Stages in pipeline */
    MprDispatcher   *dispatcher;            /**< Event dispatcher */
    HttpNotifier    notifier;               /**< Connection Http state change notification callback */
    HttpNotifier    requestNotifier;        /**< Request Http state change notification callback */
    MprWaitHandler  waitHandler;            /**< I/O wait handler */
    struct HttpServer *server;              /**< Server object (if releveant) */
    MprSocket       *sock;                  /**< Underlying socket handle */
    char            *documentRoot;          /**< Directory for documents */

    struct HttpReceiver *receiver;          /**< Receiver object */
    struct HttpTransmitter *transmitter;    /**< Transmitter object */
    struct HttpQueue serviceq;              /**< List of queues that require service for request pipeline */

    HttpPacket      *input;                 /**< Header packet */
    HttpPacket      *freePackets;           /**< Free list of packets */
    HttpQueue       *readq;                 /**< End of the read pipeline */
    HttpQueue       *writeq;                /**< Start of the write pipeline */
    MprTime         started;                /**< When the connection started */
    MprTime         expire;                 /**< When the connection should expire */
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
    int             connError;              /**< A connection error has occurred */
    int             followRedirects;        /**< Follow redirects for client requests */
    int             keepAliveCount;         /**< Count of remaining keep alive requests for this connection */
    int             legacy;                 /**< Using legacy HTTP/1.0 */
    int             startingThread;         /**< Need to allocate worker thread */
    int             port;                   /**< Remote port */
    int             retries;                /**< Client request retries */
    int             secure;                 /**< Using https */
    int             status;                 /**< Request status */
    int             timeout;                /**< Timeout period in msec */
    int             writeBlocked;           /**< Transmission writing is blocked */

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
} HttpConn;


//  MOB -- all APIS need ingroup

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

/**
    Create a request pipeline
    @param conn HttpConn object created via $httpCreateConn
    @param location Location object controlling how the pipeline is configured for the request
    @param handler Handler to process the request for server side requests
 */
extern void httpCreatePipeline(HttpConn *conn, struct HttpLocation *location, HttpStage *handler);

/**
    Destroy the pipeline
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpDestroyPipeline(HttpConn *conn);

/**
    Discard buffered pipeline data
    @param conn HttpConn object created via $httpCreateConn
 */
extern void httpDiscardPipeData(HttpConn *conn);

/** 
    Error handling for the connection.
    @description The httpError call is used to flag the current request as failed.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code
    @param fmt Printf style formatted string
    @param ... Arguments for fmt
    @ingroup HttpTransmitter
 */
extern void httpError(HttpConn *conn, int status, cchar *fmt, ...);

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
extern int httpGetChunkSize(HttpConn *conn);

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

//  MOB
#define HTTP_NEW_REQUEST 0
#define HTTP_RETRY_REQUEST 1
extern void httpPrepClientConn(HttpConn *conn, int retry);
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
extern void httpSetChunkSize(HttpConn *conn, int size);

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
    @param timeout Timeout in milliseconds
    @ingroup HttpConn
 */
extern void httpSetTimeout(HttpConn *conn, int timeout);

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
    HttpAuth is the foundation authorization object and is used as base class by HttpDirectory and HttpLocation.
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
extern HttpAuth *httpCreateAuth(MprCtx ctx, HttpAuth *parent);
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
extern HttpAuth *httpCreateAuth(MprCtx ctx, HttpAuth *parent);
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
extern cchar    *maGetPamPassword(HttpAuth *auth, cchar *realm, cchar *user);
extern bool     maValidatePamCredentials(HttpAuth *auth, cchar *realm, cchar *user, cchar *password, 
                    cchar *requiredPass, char **msg);
#endif /* AUTH_PAM */

/*
    Location flags
 */
#define HTTP_LOC_APP              0x2       /**< Location defines an application */
#define HTTP_LOC_APP_DIR          0x4       /**< Location defines a directory of applications */
#define HTTP_LOC_AUTO_SESSION     0x8       /**< Auto create sessions in this location */
#define HTTP_LOC_BROWSER          0x10      /**< Send errors back to the browser for this location */
#define HTTP_LOC_PUT_DELETE       0x20      /**< Support PUT|DELETE */

/**
    Location Control
    @stability Evolving
    @defgroup HttpLocation HttpLocation
    @see HttpLocation
 */
typedef struct HttpLocation {
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
    MprList         *handlers;              /**< List of handlers for this location */
    MprList         *inputStages;           /**< Input stages */
    MprList         *outputStages;          /**< Output stages */
    MprHashTable    *errorDocuments;        /**< Set of error documents to use on errors */
    struct HttpLocation *parent;            /**< Parent location */
    void            *context;               /**< Hosting context */
    char            *uploadDir;             /**< Upload directory */
    int             autoDelete;             /**< Auto delete uploaded files */
    char            *searchPath;            /**< Search path */
    char            *script;                /**< Startup script */
    struct MprSsl   *ssl;                   /**< SSL configuration */
} HttpLocation;

extern void httpAddErrorDocument(HttpLocation *location, cchar *code, cchar *url);

extern void httpFinalizeLocation(HttpLocation *location);
extern struct HttpStage *httpGetHandlerByExtension(HttpLocation *location, cchar *ext);
extern cchar *httpLookupErrorDocument(HttpLocation *location, int code);
extern void httpResetPipeline(HttpLocation *location);
extern void httpSetLocationAuth(HttpLocation *location, HttpAuth *auth);
extern void httpSetLocationHandler(HttpLocation *location, cchar *name);
extern void httpSetLocationPrefix(HttpLocation *location, cchar *uri);
extern void httpSetLocationFlags(HttpLocation *location, int flags);
extern int httpSetConnector(HttpLocation *location, cchar *name);
extern int httpAddHandler(HttpLocation *location, cchar *name, cchar *extensions);

extern HttpLocation *httpCreateLocation(Http *http);
extern HttpLocation *httpCreateInheritedLocation(Http *http, HttpLocation *location);
extern int httpSetHandler(HttpLocation *location, cchar *name);
extern int httpAddFilter(HttpLocation *location, cchar *name, cchar *extensions, int direction);

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
    int             size;                   /* Uploaded file size */
} HttpUploadFile;

extern void httpAddUploadFile(HttpConn *conn, cchar *id, HttpUploadFile *file);
extern void httpRemoveAllUploadedFiles(HttpConn *conn);
extern void httpRemoveUploadFile(HttpConn *conn, cchar *id);

/* 
    Receiver flags
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
    Http Receiver
    @description Most of the APIs in the receiver group still take a HttpConn object as their first parameter. This is
        to make the API easier to remember - APIs take a connection object rather than a receiver or transmitter object.
    @stability Evolving
    @defgroup HttpReceiver HttpReceiver
    @see HttpReceiver HttpConn HttpTransmitter httpWriteBlocked httpGetCookies httpGetQueryString
 */
typedef struct HttpReceiver {

    char            *method;                /**< Request method */
    char            *uri;                   /**< Original URI (not decoded) */
    char            *scriptName;            /**< ScriptName portion of the url (Decoded) */
    char            *pathInfo;              /**< Extra path information (Decoded) */
    char            *pathTranslated;        /**< Mapped pathInfo to storage (Decoded) */

    MprHeap         *arena;                 /**< Memory arena */
    HttpConn        *conn;                  /**< Connection object */
    HttpPacket      *packet;                /**< Current input packet */
    HttpPacket      *headerPacket;          /**< HTTP headers */
    HttpUri         *parsedUri;             /**< Parsed request url */
    HttpLocation    *location;              /**< Location block */
    MprList         *inputPipeline;         /**< Input processing */
    MprHashTable    *headers;               /**< Header variables */
    MprList         *etags;                 /**< Document etag to uniquely identify the document version */
    MprTime         since;                  /**< If-Modified date */

    int             length;                 /**< Declared content length (ENV: CONTENT_LENGTH) */
    int             chunkState;             /**< Chunk encoding state */
    int             chunkSize;              /**< Size of the next chunk */
    int             chunkRemainingData;     /**< Remaining chunk data to read */
    int             flags;                  /**< Receiver modifiers */
    int             needInputPipeline;      /**< Input pipeline required to process received data */
    int             remainingContent;       /**< Remaining content data to read (in next chunk if chunked) */
    int             receivedContent;        /**< Length of content actually received */
    int             readContent;            /**< Length of content read by user */
    int             readComplete;           /**< All read data has been received */

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
    char            *pragma;                /**< Pragma header */
    char            *mimeType;              /**< Mime type of the request payload (ENV: CONTENT_TYPE) */
    char            *redirect;              /**< Redirect location header */
    char            *referer;               /**< Refering URL */
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
} HttpReceiver;


/** 
    Get a receiver content length
    @description Get the length of the receiver body content (if any). This is used in servers to get the length of posted
        data and in clients to get the response body length.
    @param conn HttpConn connection object created via $httpCreateConn
    @return A count of the response content data in bytes.
    @ingroup HttpReceiver
 */
extern int httpGetContentLength(HttpConn *conn);

/** 
    Get the request cookies
    @description Get the cookies defined in the current requeset
    @param conn HttpConn connection object created via $httpCreateConn
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup HttpReceiver
 */
extern cchar *httpGetCookies(HttpConn *conn);

/** 
    Get a receiver http header.
    @description Get a http response header for a given header key.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Name of the header to retrieve. This should be a lower case header name. For example: "Connection"
    @return Value associated with the header key or null if the key did not exist in the response.
    @ingroup HttpReceiver
 */
extern cchar *httpGetHeader(HttpConn *conn, cchar *key);

/** 
    Get all the response http headers.
    @description Get all the receiver headers. The returned string formats all the headers in the form:
        key: value\\nkey2: value2\\n...
    @param conn HttpConn connection object created via $httpCreateConn
    @return String containing all the headers. The caller must free this returned string.
    @ingroup HttpReceiver
 */
extern char *httpGetHeaders(HttpConn *conn);

/** 
    Get the hash table of receiver Http headers
    @description Get the internal hash table of receiver headers
    @param conn HttpConn connection object created via $httpCreateConn
    @return Hash table. See MprHash for how to access the hash table.
    @ingroup HttpReceiver
 */
extern MprHashTable *httpGetHeaderHash(HttpConn *conn);

/** 
    Get the request query string
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup HttpReceiver
 */
extern cchar *httpGetQueryString(HttpConn *conn);

/** 
    Get a status associated with a response to a client request.
    @param conn HttpConn connection object created via $httpCreateConn
    @return An integer Http response code. Typically 200 is success.
    @ingroup HttpReceiver
 */
extern int httpGetStatus(HttpConn *conn);

/** 
    Get the Http status message associated with a response to a client request. The Http status message is supplied 
    on the first line of the Http response.
    @param conn HttpConn connection object created via $httpCreateConn
    @returns A Http status message. Caller must not free.
    @ingroup HttpReceiver
 */
extern char *httpGetStatusMessage(HttpConn *conn);

/** 
    Read receiver body data. This will read available body data. If in sync mode, this call may block. If in async
    mode, the call will not block and will return with whatever data is available.
    @param conn HttpConn connection object created via $httpCreateConn
    @param buffer Buffer to receive read data
    @param size Size of buffer. 
    @return The number of bytes read
    @ingroup HttpReceiver
 */
extern int httpRead(HttpConn *conn, char *buffer, int size);

/** 
    Read response data as a string. This will read all receiver body and return a string that the caller should free. 
    This will block and should not be used in async mode.
    @param conn HttpConn connection object created via $httpCreateConn
    @returns A string containing the receiver body. Caller should free.
    @ingroup HttpReceiver
 */
extern char *httpReadString(HttpConn *conn);

/* Internal */
extern HttpReceiver *httpCreateReceiver(HttpConn *conn);
extern void httpDestroyReceiver(HttpConn *conn);
extern bool httpContentNotModified(HttpConn *conn);
extern HttpRange *httpCreateRange(HttpConn *conn, int start, int end);
extern void  httpConnError(struct HttpConn *conn, int status, cchar *fmt, ...);
extern void  httpAdvanceReceiver(HttpConn *conn, HttpPacket *packet);
extern void  httpProcessWriteEvent(HttpConn *conn);
extern bool  httpProcessCompletion(HttpConn *conn);
extern int   httpSetUri(HttpConn *conn, cchar *newUri);
extern void  httpSetEtag(HttpConn *conn, MprPath *info);
extern bool httpMatchEtag(HttpConn *conn, char *requestedEtag);
extern bool httpMatchModified(HttpConn *conn, MprTime time);

/**
    Add query and post form variables
    @description Add new variables encoded in the supplied buffer
    @param conn HttpConn connection object
    @param buf Buffer containing www-urlencoded data
    @param len Length of buf
    @ingroup HttpReceiver
 */
extern void httpAddVars(HttpConn *conn, cchar *buf, int len);

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
    @ingroup HttpReceiver
 */
extern int httpCompareFormVar(HttpConn *conn, cchar *var, cchar *value);

/**
    Get the cookies
    @description Get the cookies defined in the current requeset
    @param conn HttpConn connection object
    @return Return a string containing the cookies sent in the Http header of the last request
    @ingroup HttpReceiver
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
    @ingroup HttpReceiver
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
    @ingroup HttpReceiver
 */
extern int httpGetIntFormVar(HttpConn *conn, cchar *var, int defaultValue);

/**
    Get the request query string
    @description Get query string sent with the current request.
    @param conn HttpConn connection object
    @return String containing the request query string. Caller should not free.
    @ingroup HttpReceiver
 */
extern cchar *httpGetQueryString(HttpConn *conn);

/**
    Set a form variable value
    @description Set the value of a named form variable to an integer value. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @param value Default value to return if the variable is not defined. Can be null.
    @ingroup HttpReceiver
 */
extern void httpSetIntFormVar(HttpConn *conn, cchar *var, int value);

/**
    Set a form variable value
    @description Set the value of a named form variable to a string value. Form variables are define via 
        www-urlencoded query or post data contained in the request.
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @param value Default value to return if the variable is not defined. Can be null.
    @ingroup HttpReceiver
 */
extern void httpSetFormVar(HttpConn *conn, cchar *var, cchar *value);

/**
    Test if a form variable is defined
    @param conn HttpConn connection object
    @param var Name of the form variable to retrieve
    @return True if the form variable is defined
    @ingroup HttpReceiver
 */
extern int httpTestFormVar(HttpConn *conn, cchar *var);

//  MOB 
extern void httpCreateEnvVars(HttpConn *conn);

/*  
    Transmitter flags
 */
#define HTTP_TRANS_DONT_CACHE          0x1     /**< Add no-cache to the transmission */
#define HTTP_TRANS_NO_BODY             0x2     /**< No transmission body, only sent headers */
#define HTTP_TRANS_HEADERS_CREATED     0x4     /**< Response headers have been created */

/** 
    Http Transmitter
    @description The transmitter object controls the transmission of data. This may be client requests or responses to
        client requests. Most of the APIs in the Response group still take a HttpConn object as their first parameter. 
        This is to make the API easier to remember - APIs take a connection object rather than a receiver or 
        transmission object.
    @stability Evolving
    @defgroup HttpTransmitter HttpTransmitter
    @see HttpTransmitter HttpReceiver HttpConn httpSetCookie httpError httpFormatBody
 */
typedef struct HttpTransmitter {
    struct HttpConn *conn;                  /**< Current connection object */
    MprList         *outputPipeline;        /**< Output processing */
    HttpStage       *handler;               /**< Server-side request handler stage */
    HttpStage       *connector;             /**< Network connector to send / receive socket data */
    HttpPacket      *freePackets;           /**< List of free packets */
    MprHashTable    *headers;               /**< Custom transmission headers */
    HttpQueue       queue[2];               /**< Dummy head for the queues */

    HttpUri         *parsedUri;             /**< Request uri. Only used for requests */
    HttpRange       *currentRange;          /**< Current range being fullfilled */
    char            *rangeBoundary;         /**< Inter-range boundary */

    char            *etag;                  /**< Unique identifier tag */
    char            *method;                /**< Request method GET, HEAD, POST, DELETE, OPTIONS, PUT, TRACE */
    char            *altBody;               /**< Alternate transmission for errors */
    int             chunkSize;              /**< Chunk size to use when using transfer encoding. Zero for unchunked. */
    int             flags;                  /**< Response flags */
    int             finalized;              /**< Finalization done */
    int             length;                 /**< Transmission content length */
    int             pos;                    /**< Current I/O position */
    int             status;                 /**< HTTP response status */
    int             traceMethods;           /**< Handler methods supported */
    int             writeComplete;          /**< All write data has been sent */

    /* File information for file based handlers */
    MprFile         *file;                  /**< File to be served */
    MprPath         fileInfo;               /**< File information if there is a real file to serve */
    char            *filename;              /**< Name of a real file being served */
    cchar           *extension;             /**< Filename extension */
    int             entityLength;           /**< Original content length before range subsetting */
    int             bytesWritten;           /**< Bytes written including headers */
    int             headerSize;             /**< Size of the header written */
} HttpTransmitter;

/** 
    Add a header to the transmission using a format string.
    @description Add a header if it does not already exits.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @return Zero if successful, otherwise a negative MPR error code. Returns MPR_ERR_ALREADY_EXISTS if the header already
        exists.
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
 */
extern void httpAddSimpleHeader(HttpConn *conn, cchar *key, cchar *value);

/** 
    Append a transmission header
    @description Set the header if it does not already exists. Append with a ", " separator if the header already exists.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
 */
extern int httpConnect(HttpConn *conn, cchar *method, cchar *uri);

/** 
    Create the transmitter object. This is used internally by the http library.
    @param conn HttpConn connection object created via $httpCreateConn
    @param headers Hash table of Http headers. Used to preserve headers from one request to another.
    @returns A transmitter object
 */
extern HttpTransmitter *httpCreateTransmitter(HttpConn *conn, MprHashTable *headers);

/** 
    Dont cache the transmission 
    @description Instruct the client not to cache the transmission body. This is done by setting the Cache-control Http
        header.
    @param conn HttpConn connection object
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
 */
extern void httpFinalize(HttpConn *conn);

//  MOB
extern int httpIsFinalized(HttpConn *conn);

/**
    Flush transmitter data. This attempts to write transmission data. If in async mode, this call will not block and
    may return before writing all data.
    @param conn HttpConn connection object created via $httpCreateConn
 */
void httpFlush(HttpConn *conn);

/** 
    Follow redirctions
    @description Enabling follow redirects enables the Http service to transparently follow 301 and 302 redirections
        and fetch the redirected URI.
    @param conn HttpConn connection object created via $httpCreateConn
    @param follow Set to true to enable transparent redirections
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
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
    @ingroup HttpTransmitter
 */ 
extern int httpIsChunked(HttpConn *conn);

/** 
    Determine if the transmission needs a transparent retry to implement authentication or redirection. This is used
    by client requests. If authentication is required, a request must first be tried once to receive some authentication 
    key information that must be resubmitted to gain access.
    @param conn HttpConn connection object created via $httpCreateConn
    @param url Reference to a string to receive a redirection URL. Set to NULL if not redirection is required.
    @return true if the request needs to be retried.
    @ingroup HttpTransmitter
 */
extern bool httpNeedRetry(HttpConn *conn, char **url);

/**
    Tell the transmitter to omit sending any body
    @param conn HttpConn connection object created via $httpCreateConn
 */
extern void httpOmitBody(HttpConn *conn);

/** 
    Redirect the client
    @description Redirect the client to a new uri.
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code to send with the response
    @param uri New uri for the client
    @ingroup HttpTransmitter
 */
extern void httpRedirect(HttpConn *conn, int status, cchar *uri);

/** 
    Remove a header from the transmission
    @description Remove a header if present.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup HttpTransmitter
 */
extern int httpRemoveHeader(HttpConn *conn, cchar *key);

/** 
    Define a content length header in the transmission. This will define a "Content-Length: NNN" request header.
    @param conn HttpConn connection object created via $httpCreateConn
    @param length Numeric value for the content length header.
    @ingroup HttpConn
 */
extern void httpSetContentLength(HttpConn *conn, int length);

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
    @ingroup HttpTransmitter
 */
extern void httpSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *domain, int lifetime, bool secure);

/**
    Define the length of the transmission content. When a static content is used for the transmission body, defining
    the entity length permits the request pipeline to know when all the data has been sent.
    @param conn HttpConn connection object created via $httpCreateConn
    @param len Transmission body length in bytes
 */
extern void httpSetEntityLength(HttpConn *conn, int len);

/** 
    Set a transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param fmt Printf style formatted string to use as the header key value
    @param ... Arguments for fmt
    @ingroup HttpTransmitter
 */
extern void httpSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...);

//MOB
extern void httpSetResponseBody(HttpConn *conn, int status, cchar *msg);

/** 
    Set a Http response status.
    @description Set the Http response status for the request. This defaults to 200 (OK).
    @param conn HttpConn connection object created via $httpCreateConn
    @param status Http status code.
    @ingroup HttpTransmitter
 */
extern void httpSetStatus(HttpConn *conn, int status);

/** 
    Set the transmission (response)  mime type
    @description Set the mime type Http header in the transmission
    @param conn HttpConn connection object created via $httpCreateConn
    @param mimeType Mime type string
    @ingroup HttpTransmitter
 */
extern void httpSetMimeType(HttpConn *conn, cchar *mimeType);

/** 
    Set a simple key/value transmission header
    @description Set a Http header to send with the request. If the header already exists, it its value is overwritten.
    @param conn HttpConn connection object created via $httpCreateConn
    @param key Http response header key
    @param value String value for the key
    @ingroup HttpTransmitter
 */
extern void httpSetSimpleHeader(HttpConn *conn, cchar *key, cchar *value);

/** 
    Wait for the connection to achieve the requested state Used for blocking client requests.
    @param conn HttpConn connection object created via $httpCreateConn
    @param state HTTP_STATE_XXX to wait for.
    @param timeout Timeout in milliseconds to wait 
    @ingroup HttpTransmitter
 */
extern int httpWait(HttpConn *conn, int state, int timeout);

/** 
    Write the transmission headers
    @description Write the Http transmission headers into the given packet. This should only be called by connectors
        just prior to sending output to the client. It should be delayed as long as possible if the content length is
        not yet known to give the pipeline a chance to determine the transmission length. This way, a non-chunked 
        transmission can be sent with a content-length header. This is the fastest HTTP transmission.
    @param conn HttpConn connection object created via $httpCreateConn
    @param packet Packet into which to place the headers
    @ingroup HttpTransmitter
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
extern int httpWriteUploadData(HttpConn *conn, MprList *formData, MprList *fileData);

/**
    Indicate that the transmission socket is blocked
    @param conn Http connection object created via $httpCreateConn
 */
extern void httpWriteBlocked(HttpConn *conn);

/** 
    Server endpoint
    @stability Evolving
    @defgroup HttpServer HttpServer
    @see HttpServer httpCreateServer httpStartServer httpStopServer
 */
typedef struct  HttpServer {
    Http            *http;                  /**< Http service object */
    MprWaitHandler  waitHandler;            /**< I/O wait handler */
    char            *serverRoot;            /**< Directory for server configuration */
    char            *documentRoot;          /**< Directory for documents */
    char            *name;                  /**< ServerName */
    char            *ip;                    /**< Listen IP address */
    int             port;                   /**< Listen port */
    int             async;                  /**< Listening is in async mode (non-blocking) */
    void            *context;               /**< Embedding context */
    void            *meta;                  /**< Meta server object */
    MprSocket       *sock;                  /**< Listening socket */
    MprDispatcher   *dispatcher;            /**< Event dispatcher */
    HttpNotifier    notifier;               /**< Http state change notification callback */
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

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* _h_HTTP */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.

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

    @end
 */

/************************************************************************/
/*
 *  End of file "../src/include/http.h"
 */
/************************************************************************/

