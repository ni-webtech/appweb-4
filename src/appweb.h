/*
    appweb.h -- Embedthis Appweb HTTP Web Server header
 */

#ifndef _h_APPWEB
#define _h_APPWEB 1

/********************************* Includes ***********************************/

#include    "mpr.h"
#include    "http.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************* Tunables ***********************************/

#define MA_SERVER_NAME          "Embedthis-Appweb/" BLD_VERSION

#define MA_UNLOAD_TIMEOUT       "5mins"         /**< Default module inactivity unload timeout */
#define MA_MAX_CONFIG_DEPTH     16              /**< Max nest of directives in config file */
#define MA_MAX_ACCESS_LOG       20971520        /**< Access file size (20 MB) */
#define MA_MAX_REWRITE          10              /**< Maximum recursive URI rewrites */

#undef HTTP_NAME
#define HTTP_NAME               MA_SERVER_NAME  /**< Default web server software identification */

/********************************** Defines ***********************************/

#if !DOXYGEN
struct MaSsl;
struct MaServer;
#endif

/**
    Appweb Service
    @description There is one instance of MaAppweb per application. It manages a list of HTTP servers running in
        the application.
    @stability Evolving
    @defgroup Appweb Appweb
    @see Http maAddServer maApplyChangedGroup maApplyChangedUser maCreateAppweb maGetUserGroup maLoadModule 
        maLookupServer maMatchDir maParseInit maSetDefaultServer maSetHttpGroup maSetHttpUser maStartAppweb maStopAppweb 
 */
typedef struct MaAppweb {
    struct MaServer     *defaultServer;     /**< Default server object */
    MprList             *servers;           /**< List of server objects */
    MprHash             *directives;        /**< Config file directives */
    Http                *http;              /**< Http service object */
    char                *user;              /**< O/S application user name */
    char                *group;             /**< O/S application group name */
    int                 uid;                /**< User Id */
    int                 gid;                /**< Group Id */
    int                 userChanged;        /**< User name changed */
    int                 groupChanged;       /**< Group name changed */
} MaAppweb;

/**
    Add a server
    @description Add a server to the list of appweb managed web servers
    @param appweb Appweb object created via #maCreateAppweb
    @param server MaServer object
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern void maAddServer(MaAppweb *appweb, struct MaServer *server);

/**
    Apply the changed group
    @description Apply configuration changes and actually change the Appweb group id
    @param appweb Appweb object created via #maCreateAppweb
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maApplyChangedGroup(MaAppweb *appweb);

/**
    Apply the changed user
    @description Apply configuration changes and actually change the Appweb user id
    @param appweb Appweb object created via #maCreateAppweb
    @ingroup Appweb
 */
extern int maApplyChangedUser(MaAppweb *appweb);

/** Create the Appweb object.
    @description Appweb uses a singleton Appweb object to manage multiple web servers instances.
    @return A Http object. Use mprFree to close and release.
    @ingroup Appweb
 */
extern MaAppweb *maCreateAppweb();

/**
    Get the user group
    @description Get the user name and ID for appweb
    @param appweb Appweb object created via #maCreateAppweb
    @ingroup Appweb
 */
extern void maGetUserGroup(MaAppweb *appweb);

/**
    Load an appweb module
    @description Load an appweb module. If the module is already loaded, this call will return successfully without
        reloading. Modules can be dynamically loaded or may also be pre-loaded using static linking.
    @param appweb Appweb object created via #maCreateAppweb
    @param name User name. Must be defined in the system password file.
    @param libname Library path name
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname);

/**
    Lookup a server
    @description Lookup a server by name and return the MaServer object
    @param appweb Appweb object created via #maCreateAppweb
    @param name Server name
    @return MaServer object
    @ingroup Appweb
 */
extern struct MaServer *maLookupServer(MaAppweb *appweb, cchar *name);

/**
    Match routine for the dirHandler
    @param conn Connection object
    @param route Matching route object
    @param direction Set to HTTP_STAGE_TX or HTTP_STAGE_RX
    @return HTTP_ROUTE_OK if the request matches.
    @ingroup Appweb
    @internal
 */
extern int maMatchDir(HttpConn *conn, HttpRoute *route, int direction);

/**
    Initialize the config file parser.
    @param appweb Appweb object created via #maCreateAppweb
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
    @internal
 */
extern int maParseInit(MaAppweb *appweb);

/**
    Set the default server
    @param appweb Appweb object created via #maCreateAppweb
    @param server MaServer object
    @ingroup Appweb
 */
extern void maSetDefaultServer(MaAppweb *appweb, struct MaServer *server);

/**
    Set the Http Group
    @description Define the group name under which to run the Appweb service
    @param appweb Appweb object created via #maCreateAppweb
    @param group Group name. Must be defined in the system group file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maSetHttpGroup(MaAppweb *appweb, cchar *group);

/**
    Set the Http User
    @description Define the user name under which to run the Appweb service
    @param appweb Appweb object created via #maCreateAppweb
    @param user User name. Must be defined in the system password file.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maSetHttpUser(MaAppweb *appweb, cchar *user);

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

/*
    Internal
 */
extern int maCgiHandlerInit(Http *http, MprModule *mp);
extern int maDirHandlerInit(Http *http, MprModule *mp);
extern int maEjsHandlerInit(Http *http, MprModule *mp);
extern int maEspHandlerInit(Http *http, MprModule *mp);
extern int maPhpHandlerInit(Http *http, MprModule *mp);
extern int maSslModuleInit(Http *http, MprModule *mp);
extern int maOpenDirHandler(Http *http);
extern int maOpenFileHandler(Http *http);

/********************************** MaServer **********************************/
/**
    The MaServer object.
    @description An application may have any number of HTTP servers, each managed by a MaServer instance.
    @stability Evolving
    @defgroup MaServer MaServer
    @see MaServer maAddEndpoint maConfigureServer maCreateServer maParseConfig maRunSimpleWebServer 
        maRunWebServer maServiceWebServer maSetServerAddress maSetServerHome maStartServer maStopServer 
        maValidateServer 
 */
typedef struct MaServer {
    char            *name;                  /**< Unique name for this server */
    MaAppweb        *appweb;                /**< Appweb control object */
    Http            *http;                  /**< Http service object (copy of appweb->http) */
    HttpLimits      *limits;                /**< Limits for this server */
    MprList         *endpoints;             /**< List of HttpEndpoints */
    char            *home;                  /**< Server root */
} MaServer;

/**
    Add a listening endpoint
    @param server Server object to modify
    @param endpoint Listening endpoint to add to the server
    @ingroup Appweb
 */
extern void maAddEndpoint(MaServer *server, HttpEndpoint *endpoint);

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

/** 
    Create a MaServer object
    @description Create new MaServer object. This routine creates a bare MaServer object, loads any required static
        modules  and performs minimal configuration. To use the server object created, more configuration will be 
        required before starting Http services.
        If you want a one-line embedding of Appweb, use #maRunWebServer or #maRunSimpleWebServer.
    @param appweb Http object returned from #maCreateAppweb
    @param name Name of the web server. This name is used as the initial server name.
    @return MaServer A newly created MaServer object. Use mprFree to free and release.
    @ingroup MaServer
 */
extern MaServer *maCreateServer(MaAppweb *appweb, cchar *name);

/**
    Parse an Appweb configuration file
    @description Parse the configuration file and configure the server. This creates a default host and route
        and then configures the server based on config file directives.
    @param server MaServer object created via #maCreateServer
    @param path Configuration file pathname.
    @param flags Parse control flags. Reserved. Set to zero.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maParseConfig(MaServer *server, cchar *path, int flags);

/** 
    Create and run a simple web server listening on a single IP address.
    @description Create a simple web server without using a configuration file. The server is created to listen on
        the specified IP address and port. This routine provides a one-line embedding of Appweb. If you want to 
        use a config file, try the #maRunWebServer instead. 
    @param ip IP address on which to listen. Set to "0.0.0.0" to listen on all interfaces.
    @param port Port number to listen to
    @param home Home directory for the web server
    @param documents Directory containing the documents to serve.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maRunSimpleWebServer(cchar *ip, int port, cchar *home, cchar *documents);

/** 
    Create and run a web server based on a configuration file
    @description Create a web server configuration based on the supplied config file. This routine provides 
        a one-line embedding of Appweb. If you don't want to use a config file, try the #maRunSimpleWebServer 
        instead. 
    @param configFile File name of the Appweb configuration file (appweb.conf) that defines the web server configuration.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MaServer
 */
extern int maRunWebServer(cchar *configFile);

/**
    Set the server listen address
    @description Set the internet addresses for all endpoints managed by the server
    @param server MaServer object created via #maCreateServer
    @param ip IP address to set for the server
    @param port Port number to use for the server
    @ingroup Appweb
 */
extern void maSetServerAddress(MaServer *server, cchar *ip, int port);

/**
    Set the server home directory.
    @param server MaServer object created via #maCreateServer
    @param path Path to the directory for the server configuration.
    @ingroup Appweb
 */
extern void maSetServerHome(MaServer *server, cchar *path);

/**
    Start a server
    @param server Object created via #maCreateServer
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
 */
extern int maStartServer(MaServer *server);

/**
    Stop a server
    @param server Object created via #maCreateServer
 */
extern void maStopServer(MaServer *server);

/**
    Validate the configuration of a server
    @param server Server object to validate
    @return True if the configuration is valid
    @ingroup Appweb
 */
extern bool maValidateServer(MaServer *server);

/******************************************************************************/
/*
    State flags
 */
#define MA_PARSE_NON_SERVER     0x1     /**< Command file being parsed by a utility program */

/**
    Current configuration parse state
    @stability Evolving
    @defgroup MaState MaState
    @see MaDirective MaState maAddDirective maArchiveLog maPopState maPushState maSetAccessLog maStartAccessLogging 
        maStartLogging maStopAccessLogging maStopLogging maTokenize 
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
    int         flags;                  /**< Parsing flags */
    struct MaState *prev;               /**< Previous (inherited) state */
    struct MaState *top;                /**< Top level state */
    struct MaState *current;            /**< Current state */
} MaState;

/**
    Directive callback function
    @description Directive callbacks are invoked to parse a directive. Directive callbacks are registered using
        #maAddDirective.
    @param state Current config parse state.
    @param key Directive key name
    @param value Directive key value
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
typedef int (MaDirective)(MaState *state, cchar *key, cchar *value);

/**
    Define a new config directive
    @description The appweb configuration file parse is extensible. New directives can be registered by this call. When
        encountered in the config file, the given callback proc will be invoked to parse.
    @param appweb Appweb object created via #maCreateAppweb
    @param directive Directive name
    @param proc Directive callback procedure of the type #MaDirective. 
    @ingroup Appweb
 */
extern void maAddDirective(MaAppweb *appweb, cchar *directive, MaDirective proc);

/**
    Archive a log file
    @description The current log file is archived by appending ".1" to the log path name. If a "path.1" exists, it will
        be renamed first to "path.2" and so on up to "path.count". 
    @param path Current log file name
    @param count Number of archived log files to preserve
    @param maxSize Reserved
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
 */
extern int maArchiveLog(cchar *path, int count, int maxSize);

/**
    Pop the state 
    @description This is used when parsing config files to handle nested include files and block level directives
    @param state Current state
    @return The next lower level state object
    @ingroup Appweb
 */
extern MaState *maPopState(MaState *state);

/**
    Push the state 
    @description This is used when parsing config files to handle nested include files and block level directives
    @param state Current state
    @return The state passed as a parameter which becomes the new top level state
    @ingroup Appweb
 */
extern MaState *maPushState(MaState *state);

/**
    Define the access log
    @description The access log is used to log details about requests to the web server. Errors are logged in the
        error log.
    @param route HttpRoute object for which to define the logging characteristics.
    @param path Pathname for the log file
    @param format Log file format. The format string argument defines how Appweb will record HTTP accesses to the 
        access log. The following log format specifiers are supported:
        <ul>
            <li>%% - Percent sign</li>
            <li>%a - Remote IP address</li>
            <li>%b - Response bytes written to the client include headers. If zero, "-" is written.</li>
            <li>%B - Response bytes written excluding headers</li>
            <li>%h - Remote hostname</li>
            <li>%O - Bytes written include headers. If zero bytes, "0" is written.</li>
            <li>%r - First line of the request</li>
            <li>%s - HTTP response status code</li>
            <li>%t - Time the request was completed</li>
            <li>%u - Authenticated username</li>
            <li>%{header}i - </li>
        </ul>
    @ingroup Appweb
 */
extern void maSetAccessLog(HttpRoute *route, cchar *path, cchar *format);

/**
    Start access logging
    @description Start access logging for a host
    @param route HttpRoute object
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup Appweb
 */
extern int maStartAccessLogging(HttpRoute *route);

/**
    Stop access logging
    @param route HttpRoute object
    @ingroup Appweb
 */
extern void maStopAccessLogging(HttpRoute *route);

/**
    Tokenize a string based on route data
    @description This is a utility routine to parse a string into tokens given a format specifier. 
    Mandatory tokens can be specified with "%" format specifier. Optional tokens are specified with "?" format. 
    Values wrapped in quotes will have the outermost quotes trimmed.
    @param state Current config parsing state
    @param str String to expand
    @param fmt Format string specifier
    Supported tokens:
    <ul>
    <li>%B - Boolean. Parses: on/off, true/false, yes/no.</li>
    <li>%N - Number. Parses numbers in base 10.</li>
    <li>%S - String. Removes quotes.</li>
    <li>%P - Path string. Removes quotes and expands ${PathVars}. Resolved relative to host->dir (ServerRoot).</li>
    <li>%W - Parse words into a list</li>
    <li>%! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.</li>
    </ul>
    @return True if the string can be successfully parsed.
    @ingroup HttpRoute
 */
extern bool maTokenize(MaState *state, cchar *str, cchar *fmt, ...);

#ifdef __cplusplus
} /* extern C */
#endif

/*
    Permit overrides
 */
#include "customize.h"

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

