/**
    appweb.c  -- AppWeb main programs.

    usage: %s [options] [IpAddr[:port]] [documentRoot]
            --config configFile     # Use config file instead of IP address (default: appweb.conf)
            --debug                 # Run in debug mode
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

/***************************** Forward Declarations ***************************/

extern void appwebOsTerm();
static char *findConfigFile(Mpr *mpr, char *configFile);
static void memoryFailure(MprCtx ctx, int64 askSize, int64 totalHeapMem, bool granted);
extern int  osInit(Mpr *mpr);
static void usageError(Mpr *mpr);
extern int  checkEnvironment(Mpr *mpr, cchar *program, cchar *home);

#if BLD_FEATURE_EJS
static void createEjsAlias(MaAppweb *appweb, MaServer *server, cchar *ejsAlias, cchar *homeDir);
#endif
#if BLD_UNIX_LIKE
static void catchSignal(int signo, siginfo_t *info, void *arg);
static int  unixSecurityChecks(Mpr *mpr, cchar *program, cchar *home);
static int  setupUnixSignals(Mpr *mpr);
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
    cchar       *ipAddrPort, *documentRoot, *argp, *logSpec, *ejsAlias;
    char        *configFile, *ip, *homeDir, *timeText;
    int         workers, outputVersion, argind, port;
    
    configFile = 0;
    documentRoot = 0;
    ejsAlias = 0;
    homeDir = 0;
    ipAddrPort = 0;
    ip = 0;
    port = -1;
    logSpec = 0;
    server = 0;
    outputVersion = 0;
    workers = -1;

    mpr = mprCreate(argc, argv, memoryFailure);
    argc = mpr->argc;
    argv = mpr->argv;

#if BLD_FEATURE_ROMFS
    extern MprRomInode romFiles[];
    mprSetRomFileSystem(mpr, romFiles);
#endif

    if (osInit(mpr) < 0) {
        exit(2);
    }
    if (mprStart(mpr) < 0) {
        mprUserError(mpr, "Can't start MPR for %s", mprGetAppName(mpr));
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
                usageError(mpr);
            }
            configFile = argv[++argind];

#if BLD_UNIX_LIKE
        } else if (strcmp(argp, "--chroot") == 0) {
            if (argind >= argc) {
                usageError(mpr);

            }
            homeDir = mprGetAbsPath(mpr, argv[++argind]);
            chdir(homeDir);
            if (chroot(homeDir) < 0) {
                if (errno == EPERM) {
                    mprPrintfError(mpr, "%s: Must be super user to use the --chroot option", mprGetAppName(mpr));
                } else {
                    mprPrintfError(mpr, "%s: Can't change change root directory to %s, errno %d",
                        mprGetAppName(mpr), homeDir, errno);
                }
                exit(4);
            }
#endif

        } else if (strcmp(argp, "--debug") == 0 || strcmp(argp, "-d") == 0) {
            mprSetDebugMode(mpr, 1);

        } else if (strcmp(argp, "--ejs") == 0) {
            if (argind >= argc) {
                usageError(mpr);
            }
            ejsAlias = argv[++argind];

        } else if (strcmp(argp, "--home") == 0) {
            if (argind >= argc) {
                usageError(mpr);
            }
            homeDir = mprGetAbsPath(mpr, argv[++argind]);
            if (chdir((char*) homeDir) < 0) {
                mprPrintfError(mpr, "%s: Can't change directory to %s\n", mprGetAppName(mpr), homeDir);
                exit(2);
            }

        } else if (strcmp(argp, "--log") == 0 || strcmp(argp, "-l") == 0) {
            if (argind >= argc) {
                usageError(mpr);
            }
            logSpec = argv[++argind];
            maStartLogging(mpr, logSpec);

        } else if (strcmp(argp, "--name") == 0 || strcmp(argp, "-n") == 0) {
            if (argind >= argc) {
                usageError(mpr);
            }
            mprSetAppName(mpr, argv[++argind], 0, 0);

        } else if (strcmp(argp, "--threads") == 0) {
            if (argind >= argc) {
                usageError(mpr);
            }
            workers = atoi(argv[++argind]);

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            maStartLogging(mpr, "stdout:2");

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            outputVersion++;

        } else {
            mprPrintfError(mpr, "Unknown switch \"%s\"\n", argp);
            usageError(mpr);
            exit(2);
        }
    }

    if (argc > argind) {
        if (argc > (argind + 2)) {
            usageError(mpr);
        }
        ipAddrPort = argv[argind++];
        if (argc > argind) {
            documentRoot = argv[argind++];
        } else {
            documentRoot = ".";
        }
    }
    if (outputVersion) {
        mprPrintf(mpr, "%s %s-%s\n", mprGetAppTitle(mpr), BLD_VERSION, BLD_NUMBER);
        exit(0);
    }
    if (ipAddrPort) {
        mprParseIp(mpr, ipAddrPort, &ip, &port, HTTP_DEFAULT_PORT);
    } else {
        if (configFile == 0) {
            configFile = findConfigFile(mpr, configFile);
        }
    }
    homeDir = mprGetCurrentPath(mpr);
    if (checkEnvironment(mpr, argv[0], homeDir) < 0) {
        exit(3);
    }

    /*  
        Create the top level HTTP service and default HTTP server. Set the initial server root to "."
     */
    appweb = maCreateAppweb(mpr);
    if (appweb == 0) {
        mprUserError(mpr, "Can't create HTTP service for %s", mprGetAppName(mpr));
        return MPR_ERR_CANT_INITIALIZE;
    }
    server = maCreateServer(appweb, "default", ".", NULL, -1);
    if (server == 0) {
        mprUserError(mpr, "Can't create HTTP server for %s", mprGetAppName(mpr));
        return MPR_ERR_CANT_INITIALIZE;
    }
    if (maConfigureServer(server, configFile, ip, port, documentRoot) < 0) {
        /* mprUserError(mpr, "Can't configure the server, exiting."); */
        exit(6);
    }
    if (mpr->ip) {
        mprLog(mpr, 2, "Server IP address %s", mpr->ip);
    }
    timeText = mprFormatLocalTime(mpr, mprGetTime(mpr));
    mprLog(mpr, 1, "Started at %s", timeText);
    mprFree(timeText);

#if BLD_FEATURE_EJS
    if (ejsAlias) {
        createEjsAlias(appweb, server, ejsAlias, homeDir);
    }
#endif

    if (workers >= 0) {
        mprSetMaxWorkers(appweb, workers);
    }
#if BLD_WIN_LIKE
    if (!ejsAlias) {
        writePort(server->defaultHost);
    }
#endif

    if (maStartAppweb(appweb) < 0) {
        mprUserError(mpr, "Can't start HTTP service, exiting.");
        exit(7);
    }
    mprLog(mpr, 1, "HTTP services are ready with max %d worker threads", mprGetMaxWorkers(mpr));

    /*
        Service I/O events until instructed to exit
     */
    mprServiceEvents(mpr, mpr->dispatcher, -1, 0);

    mprLog(appweb, 1, "Exiting ...");
    maStopAppweb(appweb);
    mprLog(appweb, 1, "Exit complete");
    mprFree(mpr);
    return 0;
}


static char *findConfigFile(Mpr *mpr, char *configFile)
{
    if (configFile == 0) {
        configFile = mprJoinPathExt(mpr, mprGetAppName(mpr), ".conf");
    }
    if (!mprPathExists(mpr, configFile, R_OK)) {
        mprFree(configFile);
        configFile = mprAsprintf(mpr, -1, "%s/../lib/%s.conf", mprGetAppDir(mpr), mprGetAppName(mpr));
        if (!mprPathExists(mpr, configFile, R_OK)) {
            mprPrintfError(mpr, "Can't open config file %s\n", configFile);
            exit(2);
        }
    }
    return mprGetAbsPath(mpr, configFile);
}


#if BLD_FEATURE_EJS
/*
    Create an ejs application location block and alias
 */
static void createEjsAlias(MaAppweb *appweb, MaServer *server, cchar *ejsAlias, cchar *homeDir)
{
    MaAlias         *alias;
    MaHost          *host;
    MaDir           *dir, *parent;
    HttpLocation    *location;
    char            *path;
    int             flags, sep;

    host = server->defaultHost;
    flags = host->location->flags & (HTTP_LOC_BROWSER | HTTP_LOC_AUTO_SESSION);

    if ((path = strchr(ejsAlias, ':')) != 0) {
        *path++ = '\0';
        path = mprJoinPath(appweb, homeDir, path);
        path = mprStrcat(path, -1, path, "/", NULL);
    } else {
        path = mprStrcat(appweb, -1, homeDir, "/", NULL);
    }
    sep = mprGetPathSeparator(appweb, ejsAlias);

    if (!mprIsAbsPath(appweb, ejsAlias) /* UNUSED || ejsAlias[strlen(ejsAlias) - 1] != sep */) {
        mprError(appweb, "The ejs alias should be an absolute path");
    }
    alias = maCreateAlias(host, ejsAlias, path, 0);
    maInsertAlias(host, alias);
    mprLog(appweb, 4, "Alias \"%s\" for \"%s\"", ejsAlias, path);

    if (maLookupLocation(host, ejsAlias)) {
        mprError(appweb, "Location block already exists for \"%s\"", ejsAlias);
        mprFree(path);
        return;
    }
    location = httpCreateInheritedLocation(appweb->http, host->location);
    httpSetLocationAuth(location, host->location->auth);
    httpSetLocationPrefix(location, ejsAlias);
    httpSetLocationFlags(location, HTTP_LOC_APP | flags);
    maAddLocation(host, location);
    httpSetHandler(location, "ejsHandler");

    /* Upload configuration */
    location->autoDelete = 1;

    /*
        Make sure there is a directory for the alias target
     */
    dir = maLookupBestDir(host, path);
    if (dir == 0) {
        parent = mprGetFirstItem(host->dirs);
        dir = maCreateDir(host, alias->filename, parent);
        maInsertDir(host, dir);
    }
    mprFree(path);
}
#endif


/*
    Display the program command line usage
 */
static void usageError(Mpr *mpr)
{
    cchar   *name;

    name = mprGetAppName(mpr);

    mprPrintfError(mpr, "\n\n%s Usage:\n\n"
    "  %s [options]\n"
    "  %s [options] [IPaddress][:port] [documentRoot] \n\n"
    "  Options:\n"
    "    --config configFile    # Use named config file instead appweb.conf\n"
    "    --chroot directory     # Change root directory to run more securely (Unix)\n"
    "    --debug                # Run in debug mode\n"
    "    --ejs appSpec          # Create an ejs application at the path\n"
    "    --home directory       # Change to directory to run\n"
    "    --name uniqueName      # Unique name for this instance\n"
    "    --log logFile:level    # Log to file file at verbosity level\n"
    "    --threads maxThreads   # Set maximum worker threads\n"
    "    --version              # Output version information\n\n"
    "  Without IPaddress, %s will read the appweb.conf configuration file.\n\n",
        name, name, name, name, name);
    exit(2);
}


/*
    Global memory failure hook
 */
static void memoryFailure(MprCtx ctx, int64 size, int64 total, bool granted)
{
    if (!granted) {
        mprPrintf(ctx, "Can't allocate memory block of size %d\n", size);
        mprPrintf(ctx, "Total memory used %d\n", total);
        exit(255);
    }
    mprPrintf(ctx, "Memory request for %d bytes exceeds memory red-line\n", size);
    mprPrintf(ctx, "Total memory used %d\n", total);
}


int osInit(Mpr *mpr)
{
#if BLD_UNIX_LIKE
    setupUnixSignals(mpr);
#endif
    return 0;
}


/*
    Security checks. Make sure we are staring with a safe environment
 */
int checkEnvironment(Mpr *mpr, cchar *program, cchar *home)
{
#if BLD_UNIX_LIKE
    if (unixSecurityChecks(mpr, program, home) < 0) {
        return -1;
    }
#endif
#if BLD_UNIX_LIKE
{
    char    *path;

    /*
        Ensure the binaries directory is in the path. Used by ejs to run ejsweb from /usr/local/bin
     */
    path = mprStrcat(mpr, -1, "PATH=", getenv("PATH"), ":", mprGetAppDir(mpr), NULL);
    putenv(path);
}
#endif
    return 0;
}


#if BLD_UNIX_LIKE
/*
    Security checks. Make sure we are staring with a safe environment
 */
static int unixSecurityChecks(Mpr *mpr, cchar *program, cchar *home)
{
    struct stat     sbuf;

    if (((stat(home, &sbuf)) != 0) || !(S_ISDIR(sbuf.st_mode))) {
        mprUserError(mpr, "Can't access directory: %s", home);
        return MPR_ERR_BAD_STATE;
    }
    if ((sbuf.st_mode & S_IWOTH) || (sbuf.st_mode & S_IWGRP)) {
        mprUserError(mpr, "Security risk, directory %s is writeable by others", home);
    }

    /*
        Should always convert the program name into a fully qualified path. Otherwise this fails.
     */
    if (*program == '/') {
        if ((stat(program, &sbuf)) != 0) {
            mprUserError(mpr, "Can't access program: %s", program);
            return MPR_ERR_BAD_STATE;
        }
        if ((sbuf.st_mode & S_IWOTH) || (sbuf.st_mode & S_IWGRP)) {
            mprUserError(mpr, "Security risk, Program %s is writeable by others", program);
        }
        if (sbuf.st_mode & S_ISUID) {
            mprUserError(mpr, "Security risk, %s is setuid", program);
        }
        if (sbuf.st_mode & S_ISGID) {
            mprUserError(mpr, "Security risk, %s is setgid", program);
        }
    }
    return 0;
}


/*
    Signals need a global reference to the mpr
 */
static Mpr *_signalMpr;

static int setupUnixSignals(Mpr *mpr)
{
    struct sigaction    act;

    _signalMpr = mpr;

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

    if (!mprGetDebugMode(mpr)) {
        sigaddset(&act.sa_mask, SIGINT);
    }

    /*
        Catch thse signals
     */
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    
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
    Mpr     *mpr;

    mpr = _signalMpr;
    mprLog(mpr, 1, "\n%s: Received signal %d\nExiting ...\n", mprGetAppName(mpr), signo);
    if (mpr) {
#if DEBUG_IDE
        if (signo != 2) {
            mprTerminate(mpr, 1);
        }
#elif BLD_DEBUG
        exit(1);
#else
        mprTerminate(mpr, 1);
#endif
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
        mprError(host, "Could not create port file %s\n", path);
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
        mprError(host, "Write to file %s failed\n", path);
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
