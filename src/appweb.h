/*
    appweb.h -- Primary header for the Embedthis Appweb HTTP Web Server
 */

#ifndef _h_APPWEB
#define _h_APPWEB 1

/********************************* Includes ***********************************/

#include    "mpr.h"
#include    "http.h"

/********************************* Tunables ***********************************/

#define MA_UNLOAD_TIMEOUT       300             /**< Default module inactivity unload timeout */
#define MA_MAX_CONFIG_DEPTH     16              /**< Max nest of directives in config file */
#define MA_MAX_ACCESS_LOG       20971520        /**< Access file size (20 MB) */
#define MA_MAX_REWRITE          10              /**< Maximum recursive URI rewrites */
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
    struct MaServer     *defaultServer;     /**< Default server object */
    MprList             *servers;           /**< List of server objects */
    MprHashTable        *directives;        /**< Config file directives */
    Http                *http;              /**< Http service object */
    char                *user;              /**< O/S application user name */
    char                *group;             /**< O/S application group name */
    //  TODO - should this be in http?
    int                 uid;                /**< User Id */
    int                 gid;                /**< Group Id */
    int                 userChanged;
    int                 groupChanged;
} MaAppweb;


/** Create the Appweb object.
    @description Appweb uses a singleton Appweb object to manage multiple web servers instances.
    @return A Http object. Use mprFree to close and release.
    @ingroup Appweb
 */
extern MaAppweb *maCreateAppweb();

/**
    Start Appweb services
    @description This starts listening for requests on all configured servers.
    @param appweb Appweb object created via #maCreateAppweb
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maStartAppweb(MaAppweb *appweb);

/**
    Stop Appweb services
    @description This stops listening for requests on all configured servers. Shutdown is somewhat graceful.
    @param appweb Appweb object created via #maCreateAppweb
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maStopAppweb(MaAppweb *appweb);

/**
    Set the Http User
    @description Define the user name under which to run the Appweb service
    @param appweb Appweb object created via #maCreateAppweb
    @param user User name. Must be defined in the system password file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maSetHttpUser(MaAppweb *appweb, cchar *user);

//  DOC
extern void maGetUserGroup(MaAppweb *appweb);

/**
    Set the Http Group
    @description Define the group name under which to run the Appweb service
    @param appweb Appweb object created via #maCreateAppweb
    @param group Group name. Must be defined in the system group file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maSetHttpGroup(MaAppweb *appweb, cchar *group);

extern void maAddServer(MaAppweb *appweb, struct MaServer *server);
extern int maApplyChangedGroup(MaAppweb *appweb);
extern int maApplyChangedUser(MaAppweb *appweb);
extern struct MaServer *maLookupServer(MaAppweb *appweb, cchar *name);
extern int maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname);

extern void maSetDefaultServer(MaAppweb *appweb, struct MaServer *server);

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
extern int maEjsHandlerInit(Http *http, MprModule *mp);
extern int maNetConnectorInit(Http *http, MprModule *mp);
extern int maPhpHandlerInit(Http *http, MprModule *mp);
extern int maRangeFilterInit(Http *http, MprModule *mp);
extern int maSslModuleInit(Http *http, MprModule *mp);
extern int maUploadFilterInit(Http *http, MprModule *mp);

extern int maParseInit(MaAppweb *appweb);
extern bool maMatchDir(HttpConn *conn, HttpRoute *route, int direction);

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
        maConfigureServer maSplitConfigValue
 */
typedef struct MaServer {
    char            *name;                  /**< Unique name for this server */
    MaAppweb        *appweb;                /**< Appweb control object */
    Http            *http;                  /**< Http service object (copy of appweb->http) */
    HttpLimits      *limits;                /**< Limits for this server */
    MprList         *endpoints;             /**< List of HttpEndpoints */
#if UNUSED
    struct HttpHost *defaultHost;           /**< Primary host */
#endif
    char            *home;                  /**< Server root */
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
    @description Run a web server configuration. This is will start http services via $maStartServer and will service
        incoming Http requests until instructed to exit. This is often used in conjunction with #maCreateWebServer.
    @param server Server object created via #maCreateWebServer or #maCreateServer.
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
    @param home Home directory for the web server
    @param documents Directory containing the documents to serve.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maRunSimpleWebServer(cchar *ip, int port, cchar *home, cchar *documents);

/** Create an MaServer object
    @description Create new MaServer object. This routine creates a bare MaServer object, loads any required static
        modules  and performs minimal configuration. To use the server object created, more configuration will be 
        required before starting Http services.
        If you want a one-line embedding of Appweb, use #maRunWebServer or #maRunSimpleWebServer.
    @param appweb Http object returned from #maCreateAppweb
    @param name Name of the web server. This name is used as the initial server name.
    @param home Server home directory
    @param ip If not-null, create and open a listening endpoint on this IP address. If you are configuring via a
        config file, use #maConfigureServer and set ip to null.
    @param port Port number to listen on. Set to -1 if you do not want to open a listening endpoint on ip:port
    @return MaServer A newly created MaServer object. Use mprFree to free and release.
    @ingroup MaServer
 */
extern MaServer *maCreateServer(MaAppweb *appweb, cchar *name);

/** 
    Configure a web server.
    @description This will configure a web server based on either a configuration file or using the supplied
        IP address and port. 
    @param server MaServer object created via #maCreateServer
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @param home Admin directory for the server. This overrides the value in the config file.
    @param documents Default directory for web documents to serve. This overrides the value in the config file.
    @param ip IP address to listen on. This overrides the value specified in the config file.
    @param port Port address to listen on. This overrides the value specified in the config file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maConfigureServer(MaServer *server, cchar *configFile, cchar *home, cchar *documents, cchar *ip, int port);

extern void     maAddEndpoint(MaServer *server, HttpEndpoint *endpoint);
extern int      maGetConfigValue(char **arg, char *buf, char **nextToken, int quotes);
extern int      maParseConfig(MaServer *server, cchar *configFile);
extern void     maSetServerAddress(MaServer *server, cchar *ip, int port);
extern void     maSetServerHome(MaServer *server, cchar *path);
extern int      maSplitConfigValue(char **s1, char **s2, char *buf, int quotes);

/**
    Start a server
    @param server Object created via #maCreateServer
 */
extern int      maStartServer(MaServer *server);
extern int      maStopServer(MaServer *server);
extern int      maValidateConfiguration(MaServer *server);
extern HttpHost *maCreateDefaultHost(MaServer *server);


#if UNUSED
extern void     maAddHost(MaServer *server, struct HttpHost *host);
extern HttpHostAddress *maAddHostAddress(MaServer *server, cchar *ip, int port);
extern int      maCreateHostAddresses(MaServer *server, struct HttpHost *host, cchar *value);
extern struct HttpHost *maLookupHost(MaServer *server, cchar *name);
extern void     maNotifyServerStateChange(HttpConn *conn, int state, int notifyFlags);
extern HttpHostAddress *maRemoveHostFromHostAddress(MaServer *server, cchar *ip, int port, struct HttpHost *host);
extern void     maSetServerDefaultHost(MaServer *server, struct HttpHost *host);
extern void     maSetServerDefaultIndex(MaServer *server, cchar *path, cchar *filename);
#endif

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

/********************************** MaUploadFile *********************************/

extern int maOpenDirHandler(Http *http);
extern int maOpenFileHandler(Http *http);
extern int maOpenSendConnector(Http *http);

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
    @defgroup MaState MaState
    @see MaState
 */
typedef struct MaState {
    MaAppweb    *appweb;
    Http        *http;
    MaServer    *server;                /**< Current server */
    HttpHost    *host;                  /**< Current host */
    HttpAuth    *auth;                  /**< Quick alias for route->auth */
    HttpRoute   *route;                 /**< Current route */
    MprFile     *file;                  /**< Config file handle */
    HttpLimits  *limits;                /**< Current limits (host->limits) */
    char        *key;                   /**< Current directive being parsed */
    char        *configDir;             /**< Directory containing config file */
    char        *filename;              /**< Config file name */
    int         lineNumber;             /**< Current line number */
    int         enabled;                /**< True if the current block is enabled */
    struct MaState *prev;
    struct MaState *top;                /**< Top level state */
    struct MaState *current;            /**< Current state */
} MaState;

extern HttpRoute *maCreateLocationAlias(Http *http, MaState *state, cchar *prefix, cchar *path, 
        cchar *handlerName, int flags);

extern char         *maMakePath(HttpHost *host, cchar *file);
extern MaState      *maPopState(MaState *state);
extern MaState      *maPushState(MaState *state);
extern char         *maReplaceReferences(HttpHost *host, cchar *str);
extern void         maRotateLog(cchar *path, int count, int maxSize);
extern void         maSetAccessLog(HttpHost *host, cchar *path, cchar *format);
extern int          maStopLogging();
extern int          maStartLogging(HttpHost *host, cchar *logSpec);
extern void         maSetLogHost(HttpHost *host, HttpHost *logHost);
extern int          maStartAccessLogging(HttpHost *host);
extern int          maStopAccessLogging(HttpHost *host);
extern bool         maTokenize(MaState *state, cchar *line, cchar *fmt, ...);

typedef int         (MaDirective)(MaState *state, cchar *key, cchar *value);

extern void         maAddDirective(MaAppweb *appweb, cchar *directive, MaDirective proc);

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

