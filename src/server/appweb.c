/**
    appweb.c  -- AppWeb main program

    usage: %s [options] [IpAddr][:port] [documentRoot]
            --config configFile     # Use given config file instead 
            --debugger              # Disable timeouts to make debugging easier
            --ejs name:path         # Create an ejs application at the path
            --log logFile:level     # Log to file file at verbosity level
            --name uniqueName       # Name for this instance
            --threads maxThreads    # Set maximum worker threads
            --version               # Output version information
            -v                      # Same as --log stdout:2

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "appweb.h"

/********************************** Locals ************************************/

typedef struct App {
    char    *documentRoot;      //MOB UNUSED
    char    *serverRoot;        //MOB UNUSED
    char    *configFile;
    char    *pathVar;
} App;

static App *app;

/***************************** Forward Declarations ***************************/

extern void appwebOsTerm();
static int changeRoot(cchar *jail);
extern int checkEnvironment(cchar *program);
static void findConfigFile();
static void manageApp(App *app, int flags);
extern int  osInit();
static MaAppweb *initialize(cchar *ip, int port, MprList *script, int workers);
static void usageError();

#if BLD_FEATURE_EJS
static int setupEjsApps(MaAppweb *appweb, MaServer *server, MprList *scripts);
#endif
#if BLD_UNIX_LIKE
static void catchSignal(int signo, siginfo_t *info, void *arg);
static int  unixSecurityChecks(cchar *program, cchar *home);
static int  setupUnixSignals();
#endif

#if BLD_WIN_LIKE
static int writePort(MaHost *host);
static long msgProc(HWND hwnd, uint msg, uint wp, long lp);
#endif

/*********************************** Code *************************************/

MAIN(appweb, int argc, char **argv)
{
    Mpr         *mpr;
    MaAppweb    *appweb;
    MaServer    *server;
    MprList     *scripts;
    cchar       *ipAddrPort, *argp, *logSpec, *jail;
    char        *ip;
    int         workers, outputVersion, argind, port;

    ipAddrPort = 0;
    ip = 0;
    jail = 0;
    logSpec = 0;
    port = -1;
    outputVersion = 0;
    server = 0;
    scripts = 0;
    workers = -1;

    mpr = mprCreate(argc, argv, MPR_USER_GC);

    argc = mpr->argc;
    argv = mpr->argv;

    if ((app = mprAllocObj(App, manageApp)) == NULL) {
        exit(1);
    }
    mprAddRoot(app);

    app->serverRoot = mprGetCurrentPath();
    app->documentRoot = app->serverRoot;

#if BLD_FEATURE_ROMFS
    extern MprRomInode romFiles[];
    mprSetRomFileSystem(romFiles);
#endif

    if (osInit() < 0) {
        exit(2);
    }
    if (mprStart() < 0) {
        mprUserError("Can't start MPR for %s", mprGetAppName());
        mprFree(mpr);
        return MPR_ERR_CANT_INITIALIZE;
    }

    for (argind = 1; argind < argc; argind++) {
        argp = argv[argind];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--config") == 0) {
            if (argind >= argc) {
                usageError();
            }
            app->configFile = argv[++argind];

#if BLD_UNIX_LIKE
        } else if (strcmp(argp, "--chroot") == 0) {
            if (argind >= argc) {
                usageError();
            }
            jail = mprGetAbsPath(argv[++argind]);
#endif

        } else if (strcmp(argp, "--debugger") == 0 || strcmp(argp, "-d") == 0) {
            mprSetDebugMode(1);

        } else if (strcmp(argp, "--ejs") == 0) {
            if (argind >= argc) {
                usageError();
            }
            if (scripts == 0) {
                scripts = mprCreateList();
            }
            mprAddItem(scripts, argv[++argind]);

        } else if (strcmp(argp, "--home") == 0) {
            if (argind >= argc) {
                usageError();
            }
            app->serverRoot = mprGetAbsPath(argv[++argind]);
            if (chdir(app->serverRoot) < 0) {
                mprPrintfError("%s: Can't change directory to %s\n", mprGetAppName(), app->serverRoot);
                exit(2);
            }

        } else if (strcmp(argp, "--log") == 0 || strcmp(argp, "-l") == 0) {
            if (argind >= argc) {
                usageError();
            }
            logSpec = argv[++argind];
            maStartLogging(logSpec);

        } else if (strcmp(argp, "--name") == 0 || strcmp(argp, "-n") == 0) {
            if (argind >= argc) {
                usageError();
            }
            mprSetAppName(argv[++argind], 0, 0);

        } else if (strcmp(argp, "--threads") == 0) {
            if (argind >= argc) {
                usageError();
            }
            workers = atoi(argv[++argind]);

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            maStartLogging("stdout:2");

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            outputVersion++;

        } else {
            mprPrintfError("Unknown switch \"%s\"\n", argp);
            usageError();
            exit(2);
        }
    }
    if (argc > argind) {
        if (argc > (argind + 2)) {
            usageError();
        }
        ipAddrPort = argv[argind++];
        if (argc > argind) {
            app->documentRoot = argv[argind++];
        }
    }
    if (outputVersion) {
        mprPrintf("%s %s-%s\n", mprGetAppTitle(), BLD_VERSION, BLD_NUMBER);
        exit(0);
    }
    findConfigFile();
    if (ipAddrPort) {
        mprParseIp(ipAddrPort, &ip, &port, HTTP_DEFAULT_PORT);
    }
    if (checkEnvironment(argv[0]) < 0) {
        exit(3);
    }
    if (jail && changeRoot(jail) < 0) {
        exit(4);
    }
    if ((appweb = initialize(ip, port, scripts, workers)) == 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    if (maStartAppweb(appweb) < 0) {
        mprUserError("Can't start HTTP service, exiting.");
        exit(5);
    }

    /*
        Service I/O events until instructed to exit
     */
    mprServiceEvents(mpr->dispatcher, -1, 0);

    mprLog(1, "Exiting ...");
    maStopAppweb(appweb);
    mprLog(1, "Exit complete");

    mprStop(mpr);
    mprFree(mpr);
    return 0;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->configFile);
        mprMark(app->documentRoot);
        mprMark(app->serverRoot);
        mprMark(app->pathVar);

    } else if (flags & MPR_MANAGE_FREE) {
    }
}


/*
    Move into a chroot jail
 */
static int changeRoot(cchar *jail)
{
#if BLD_UNIX_LIKE
    if (chroot(jail) < 0) {
        if (errno == EPERM) {
            mprPrintfError("%s: Must be super user to use the --chroot option", mprGetAppName());
        } else {
            mprPrintfError("%s: Can't change change root directory to %s, errno %d",
                mprGetAppName(), jail, errno);
        }
        return MPR_ERR_CANT_INITIALIZE;
    }
#endif
    return 0;
}


static MaAppweb *initialize(cchar *ip, int port, MprList *scripts, int workers)
{
    MaAppweb    *appweb;
    MaServer    *server;
    
    if ((appweb = maCreateAppweb()) == 0) {
        mprUserError("Can't create HTTP service for %s", mprGetAppName());
        return 0;
    }
    if ((server = maCreateServer(appweb, "default", NULL, NULL, -1)) == 0) {
        mprUserError("Can't create HTTP server for %s", mprGetAppName());
        return 0;
    }
    if (maConfigureServer(server, app->configFile, app->serverRoot, app->documentRoot, ip, port) < 0) {
        /* mprUserError("Can't configure the server, exiting."); */
        exit(6);
    }
#if BLD_FEATURE_EJS
#if BLD_EJS_PRODUCT && UNUSED
    if (scripts == 0) {
        scripts = mprCreateList();
        mprAddItem(scripts, MA_EJS_STARTUP);
    }
#endif
    if (scripts) {
        if (setupEjsApps(appweb, server, scripts) < 0) {
            exit(7);
        }
    }
#endif
    if (workers >= 0) {
        mprSetMaxWorkers(workers);
    }
#if BLD_WIN_LIKE
    if (!scripts) {
        writePort(server->defaultHost);
    }
#endif
    return appweb;
}


static void findConfigFile()
{
    if (app->configFile == 0) {
        app->configFile = mprJoinPathExt(mprGetAppName(), ".conf");
    }
    if (!mprPathExists(app->configFile, R_OK)) {
        //  MOB -- will BLD_LIB_NAME be bad for cross-compilation?
        app->configFile = mprAsprintf("%s/../%s/%s.conf", mprGetAppDir(), BLD_LIB_NAME, mprGetAppName());
        if (!mprPathExists(app->configFile, R_OK)) {
            mprPrintfError("Can't open config file %s\n", app->configFile);
            exit(2);
        }
    }
#if UNUSED
    return mprGetAbsPath(app->configFile);
#endif
}


#if BLD_FEATURE_EJS
/*
    Create the ejs application aliases
 */
static int setupEjsApps(MaAppweb *appweb, MaServer *server, MprList *scripts)
{
    MaHost      *host;
    HttpLoc     *loc;
    char        *home, *path, *uri, *script;
    int         next;

    host = server->defaultHost;
    home = mprGetCurrentPath(appweb);
    uri = "/";

    for (next = 0; (script = mprGetNextItem(scripts, &next)) != 0; ) {
        path = mprGetPathDir(mprJoinPath(home, script));
#if UNUSED
        alias = maCreateAlias("/", path, 0);
        maInsertAlias(host, alias);
#endif
        mprLog(3, "Ejs Alias \"%s\" for \"%s\"", uri, path);

        if (maLookupLocation(host, uri)) {
            mprError("Location block already exists for \"%s\"", uri);
            return MPR_ERR_ALREADY_EXISTS;
        }
        loc = httpCreateInheritedLocation(appweb->http, host->loc);
#if UNUSED
        httpSetLocationAuth(loc, host->loc->auth);
#endif
        httpSetLocationPrefix(loc, uri);
        httpSetLocationScript(loc, script);
        httpSetLocationAutoDelete(loc, 1);
        maAddLocation(host, loc);
        httpSetHandler(loc, "ejsHandler");
        
#if UNUSED
        /* Make sure there is a directory for the alias target */
        dir = maLookupBestDir(host, path);
        if (dir == 0) {
            parent = mprGetFirstItem(host->dirs);
#if UNUSED
            dir = maCreateDir(host, alias->filename, parent);
#else
            dir = maCreateDir(host, path, parent);
#endif
            dir = maCreateDir(host, alias->filename, parent);
            maInsertDir(host, dir);
        }
#endif
        uri = "/web";
        if (!maLookupLocation(host, uri)) {
            loc = httpCreateInheritedLocation(appweb->http, host->loc);
            httpSetLocationPrefix(loc, uri);
            maAddLocation(host, loc);
        }
    }
    return 0;
}
#endif


/*
    Display the program command line usage
 */
static void usageError(Mpr *mpr)
{
    cchar   *name;

    name = mprGetAppName();

    mprPrintfError("\n\n%s Usage:\n\n"
    "  %s [options] [IPaddress][:port] [documentRoot]\n\n"
    "  Options:\n"
    "    --config configFile    # Use named config file instead appweb.conf\n"
    "    --chroot directory     # Change root directory to run more securely (Unix)\n"
    "    --debugger             # Disable timeouts to make debugging easier\n"
    "    --ejs script           # Ejscript startup script\n"
    "    --home directory       # Change to directory to run\n"
    "    --name uniqueName      # Unique name for this instance\n"
    "    --log logFile:level    # Log to file file at verbosity level\n"
    "    --threads maxThreads   # Set maximum worker threads\n"
    "    --version              # Output version information\n\n"
    "  Without IPaddress, %s will read the appweb.conf configuration file.\n\n",
        name, name, name, name, name);
    exit(2);
}


int osInit()
{
#if BLD_UNIX_LIKE
    setupUnixSignals();
#endif
    return 0;
}


/*
    Security checks. Make sure we are staring with a safe environment
 */
int checkEnvironment(cchar *program)
{
#if BLD_UNIX_LIKE
    char   *home;
    home = mprGetCurrentPath();
    if (unixSecurityChecks(program, home) < 0) {
        return -1;
    }
    /*
        Ensure the binaries directory is in the path. Used by ejs to run ejsweb from /usr/local/bin
     */
    app->pathVar = sjoin("PATH=", getenv("PATH"), ":", mprGetAppDir(), NULL);
    putenv(app->pathVar);
#endif
    return 0;
}


#if BLD_UNIX_LIKE
/*
    Security checks. Make sure we are staring with a safe environment
 */
static int unixSecurityChecks(cchar *program, cchar *home)
{
    struct stat     sbuf;

    if (((stat(home, &sbuf)) != 0) || !(S_ISDIR(sbuf.st_mode))) {
        mprUserError("Can't access directory: %s", home);
        return MPR_ERR_BAD_STATE;
    }
    if ((sbuf.st_mode & S_IWOTH) || (sbuf.st_mode & S_IWGRP)) {
        mprUserError("Security risk, directory %s is writeable by others", home);
    }

    /*
        Should always convert the program name into a fully qualified path. Otherwise this fails.
     */
    if (*program == '/') {
        if ((stat(program, &sbuf)) != 0) {
            mprUserError("Can't access program: %s", program);
            return MPR_ERR_BAD_STATE;
        }
        if ((sbuf.st_mode & S_IWOTH) || (sbuf.st_mode & S_IWGRP)) {
            mprUserError("Security risk, Program %s is writeable by others", program);
        }
        if (sbuf.st_mode & S_ISUID) {
            mprUserError("Security risk, %s is setuid", program);
        }
        if (sbuf.st_mode & S_ISGID) {
            mprUserError("Security risk, %s is setgid", program);
        }
    }
    return 0;
}


static int setupUnixSignals()
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = catchSignal;
    act.sa_flags = 0;
   
    /*
        Mask these when processing signals
     */
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    sigaddset(&act.sa_mask, SIGCHLD);
    sigaddset(&act.sa_mask, SIGPIPE);
    sigaddset(&act.sa_mask, SIGTERM);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaddset(&act.sa_mask, SIGUSR2);

    if (!mprGetDebugMode()) {
        sigaddset(&act.sa_mask, SIGINT);
    }

    /*
        Catch thse signals
     */
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGUSR1, &act, 0);
    
    /*
        Ignore pipe signals
     */
    signal(SIGPIPE, SIG_IGN);

#if LINUX
    /*
        Ignore signals from write requests to large files
     */
    signal(SIGXFSZ, SIG_IGN);
#endif
    return 0;
}


/*
    Catch signals. Do a graceful shutdown.
 */
static void catchSignal(int signo, siginfo_t *info, void *arg)
{
#if DEBUG_IDE
    if (signo == SIGINT) {
        return;
    }
#endif
    mprLog(2, "Received signal %d", signo);
    if (signo == SIGINT || signo == SIGTERM) {
        mprLog(1, "Exiting immediately ...");
        mprTerminate(0);
    } else {
        mprLog(1, "Executing a graceful exit. Waiting for all requests to complete");
        mprTerminate(MPR_GRACEFUL);
    }
}
#endif /* BLD_HOST_UNIX */


#if BLD_WIN_LIKE
/*
    Write the port so the monitor can manage
 */ 
static int writePort(MaHost *host)
{
    char    *cp, numBuf[16], *path;
    int     fd, port, len;

    path = mprJoinPath(host, mprGetAppDir(host), "../.port.log");
    if ((fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
        mprError(host, "Could not create port file %s", path);
        return MPR_ERR_CANT_CREATE;
    }

    cp = host->ipAddrPort;
    if ((cp = strchr(host->ipAddrPort, ':')) != 0) {
        port = atoi(++cp);
    } else {
        port = 80;
    }
    mprItoa(numBuf, sizeof(numBuf), port, 10);

    len = (int) strlen(numBuf);
    numBuf[len++] = '\n';
    if (write(fd, numBuf, len) != len) {
        mprError(host, "Write to file %s failed", path);
        return MPR_ERR_CANT_WRITE;
    }
    close(fd);
    return 0;
}

#endif /* BLD_WIN_LIKE */


#if VXWORKS
/*
    VxWorks link resolution
 */
int _cleanup() {
    return 0;
}
int _exit() {
    return 0;
}

/*
    Create a routine to pull in the GCC support routines for double and int64 manipulations. Do this
    incase any modules reference these routines. Without this, the modules have to reference them. Which leads to
    multiple defines if two modules include them.
 */
double  __dummy_appweb_floating_point_resolution(double a, double b, int64 c, int64 d, uint64 e, uint64 f) {
    /*
        Code to pull in moddi3, udivdi3, umoddi3, etc .
     */
    a = a / b; a = a * b; c = c / d; c = c % d; e = e / f; e = e % f;
    c = (int64) a;
    d = (uint64) a;
    a = (double) c;
    a = (double) e;
    if (a == b) {
        return a;
    } return b;
}

#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    @end
 */
