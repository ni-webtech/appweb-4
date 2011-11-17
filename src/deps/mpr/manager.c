/**
    manager.c -- Manager program 

    The manager watches over daemon programs.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************* Includes ***********************************/

#include    "mpr.h"

#if 0
#define SERVICE_PROGRAM BLD_BIN_PREFIX BLD_PRODUCT
#define SERVICE_NAME BLD_PRODUCT
#else
#define SERVICE_PROGRAM "/Users/mob/git/appweb/out/bin/appweb"
#define SERVICE_NAME "appweb"
#endif

#if BLD_UNIX_LIKE
/*********************************** Locals ***********************************/

#define RESTART_DELAY (0 * 1000)        /* Default heart beat period (30 sec) */
#define RESTART_MAX   (100)             /* Max restarts per hour */

typedef struct App {
    cchar   *appName;                   /* Manager name */
    int     exiting;                    /* Program should exit */
    int     retries;                    /* Number of times to retry staring app */
    char    *logSpec;                   /* Log directive for service */
    char    *pidDir;                    /* Location for pid file */
    char    *pidPath;                   /* Path to the manager pid for this service */
    int     restartCount;               /* Service restart count */
    int     restartWarned;              /* Has user been notified */
    int     runAsDaemon;                /* Run as a daemon */
    int     servicePid;                 /* Process ID for the service */
    char    *serviceArgs;               /* Args to pass to service */
    char    *serviceHome;               /* Service home */
    char    *serviceName;               /* Basename of service program */
    char    *serviceProgram;            /* Program to start */
} App;

static App *app;

/***************************** Forward Declarations ***************************/

static void cleanup();
static bool killPid();
static int  makeDaemon();
static void manageApp(void *unused, int flags);
static int  readPid();
static int  process(cchar *operation);
static void runService();
static void setAppDefaults(Mpr *mpr);
static int  writePid(int pid);

/*********************************** Code *************************************/

int main(int argc, char *argv[])
{
    Mpr     *mpr;
    char    *argp, *operation;
    int     err, nextArg, status;

    err = 0;
    mpr = mprCreate(argc, argv, 0);
//MOB
mprSetAppName("appman", "Appman", "1.0");
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    setAppDefaults(mpr);

    for (nextArg = 1; nextArg < argc && !err; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--args") == 0) {
            /*
                Args to pass to service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceArgs = argv[++nextArg];
            }

        } else if (strcmp(argp, "--console") == 0) {
            /* Does nothing. Here for compatibility with windows watcher */

        } else if (strcmp(argp, "--daemon") == 0) {
            app->runAsDaemon++;

#if FUTURE
        } else if (strcmp(argp, "--heartBeat") == 0) {
            /*
                Set the frequency to check on the program.
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->heartBeatPeriod = atoi(argv[++nextArg]);
            }
#endif

        } else if (strcmp(argp, "--home") == 0) {
            /*
                Change to this directory before starting the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceHome = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--log") == 0) {
            /*
                Pass the log directive through to the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->logSpec = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--program") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceProgram = argv[++nextArg];
                app->serviceName = spascal(mprGetPathBase(app->serviceProgram));
            }

        } else if (strcmp(argp, "--retries") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->retries = atoi(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            mprSetLogLevel(1);

        } else {
            err++;
            break;
        }
    }
    if (nextArg >= argc) {
        err++;
    }
    if (err) {
        mprUserError("Bad command line: \n"
            "  Usage: %s [commands]\n"
            "  Switches:\n"
            "    --args               # Args to pass to service\n"
            "    --daemon             # Run manager as a daemon\n"
            "    --home path          # Home directory for service\n"
            "    --log logFile:level  # Log directive for service\n"
            "    --retries count      # Max count of app restarts\n"
#if FUTURE
            "    --heartBeat interval # Heart beat interval period (secs) \n"
            "    --program path       # Service program to start\n"
#endif
            "  Commands:\n"
            "    disable              # Disable the service\n"
            "    enable               # Enable the service\n"
            "    start                # Start the service\n"
            "    stop                 # Start the service\n"
            "    run                  # Run and watch over the service\n"
            , app->appName);
        return -1;
    }
    operation = sclone(argv[nextArg++]);
    status = 0;;                                                                    

//MOB
    if (0 && getuid() != 0) {
        mprUserError("Must run with administrator privilege. Use sudo.");
        status = MPR_ERR_BAD_STATE;                                                                    

    } else if (mprStart() < 0) {
        mprUserError("Can't start MPR for %s", mprGetAppName());                                           
        status = MPR_ERR_CANT_INITIALIZE;                                                                    

    } else if (!process(operation)) {
        status = MPR_ERR_CANT_COMPLETE;                                                                    
    }
    mprDestroy(MPR_EXIT_DEFAULT);                                                                      
    return status;                                                                    
}


static void manageApp(void *ptr, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->appName);
        mprMark(app->logSpec);
        mprMark(app->pidDir);
        mprMark(app->pidPath);
        mprMark(app->serviceArgs);
        mprMark(app->serviceHome);
        mprMark(app->serviceName);
        mprMark(app->serviceProgram);
    }
}


static void setAppDefaults(Mpr *mpr)
{
    app->appName = mprGetAppName();
    app->serviceProgram = sclone(SERVICE_PROGRAM);
    app->serviceName = sclone(SERVICE_NAME);
    app->serviceHome = sclone(".");
    app->retries = RESTART_MAX;

    if (mprPathExists("/var/run", X_OK) && getuid() == 0) {
        app->pidDir = sclone("/var/run");
    } else if (mprPathExists("/tmp", X_OK)) {
        app->pidDir = sclone("/tmp");
    } else if (mprPathExists("/Temp", X_OK)) {
        app->pidDir = sclone("/Temp");
    } else {
        app->pidDir = sclone(".");
    }
    app->pidPath = sjoin(app->pidDir, app->serviceName, ".pid", NULL);
}


static bool exists(cchar *fmt, ...)
{
    va_list     args;
    cchar       *path;
    bool        rc;

    va_start(args, fmt);
    path = sfmtv(fmt, args);
    rc = mprPathExists(path, F_OK);
    va_end(args);
    return rc;
}


static bool run(cchar *fmt, ...)
{
    va_list     args;
    MprCmd      *cmd;
    char        *out, *err, *command;

    va_start(args, fmt);
    command = sfmtv(fmt, args);
    mprLog(1, "Run: %s", command);

    cmd = mprCreateCmd(NULL);
    if (mprRunCmd(cmd, command, &out, &err, 0) != 0) {
        mprError("Can't run command %s\n%s\n", command, err);
        return 0;
    }
    if (err && *err) {
        mprLog(1, "Error: %s", err); 
    }
    if (out && *out) {
        mprLog(1, "Output: %s", err); 
    }
    va_end(args);
    return 1;
}


static int process(cchar *operation)
{
    cchar   *name, *command;
    int     launch, update, service, upstart;

    /*
        No systemd support yet
     */
    name = app->serviceName;
    launch = upstart = update = service = 0;

    if (exists("/bin/launchctl") && exists("/Library/LaunchDaemons/com.%s.%s.plist", slower(BLD_COMPANY), name)) {
        launch++;
    } else if (exists("/sbin/start") && (exists("/etc/init/%s.conf", name) || exists("/etc/init/%s.disable", name))) {
        upstart++;
    } else if (exists("/usr/sbin/update-rc.d") && exists("/etc/init.d/%s", name)) {
        update++;
    } else if (exists("/sbin/service") && exists("/etc/init.d/%s", name)) {
        service++;
    } else {
        mprError("Can't locate system tool to manage service");
        return MPR_ERR_CANT_OPEN;
    }

    if (smatch(operation, "run")) {
        if (app->runAsDaemon) {
            makeDaemon();
        }
        runService();
        return 0;
    }
    command = 0;

    if (smatch(operation, "disable")) {
        /* Stop service and leave in disabled state (won't start on reboot) */
        if (launch) {
            return run("/bin/launchctl unload -w /Library/LaunchDaemons/com.%s.%s.plist", slower(BLD_COMPANY), name);
        } else if (update) {
            return run("/usr/sbin/update-rc.d %s disable 2345", name);
        } else if (service) {
            return run("/sbin/chkconfig %s off", name);
        } else if (upstart) {
            if (exists("/etc/init/%s.conf", name)) {
                return run("mv /etc/init/%s.conf /etc/init/%s.off", name, name);
            }
        }

    } else if (smatch(operation, "enable")) {
        /* Enable service (will start on reboot), and start service */
        if (launch) {
            return run("/bin/launchctl load -w /Library/LaunchDaemons/com.%s.%s.plist", slower(BLD_COMPANY), name);
        } else if (update) {
            if (!run("/usr/sbin/update-rc.d %s enable 2345", name)) {
                return MPR_ERR_CANT_COMPLETE;
            }
            return process("start");
        } else if (service) {
            if (!run("/sbin/chkconfig %s on", name)) {
                return MPR_ERR_CANT_COMPLETE;
            }
            return process("start");
        } else if (upstart) {
            if (exists("/etc/init/%s.off", name)) {
                if (!run("mv /etc/init/%s.off /etc/init/%s.conf", name, name)) {
                    return MPR_ERR_CANT_COMPLETE;
                }
            }
            return process("start");
        }

    } else if (smatch(operation, "install")) {
        //  MOB - define install. Does it do enable and start
        if (launch) {
            return process("enable");
        } else if (update) {
            if (!run("/usr/sbin/update-rc.d %s defaults 90 10", name)) {
                return MPR_ERR_CANT_COMPLETE;
            }
            return process("enable");
        } else if (service) {
            if (!run("/sbin/chkconfig --del %s ; /sbin/chkconfig --add %s ; /sbin/chkconfig --level 5 %s", 
                    name, name, name)) {
                return MPR_ERR_CANT_COMPLETE;
            }
            return process("enable");
        } else if (upstart) {
            return process("enable");
        }

    } else if (smatch(operation, "reload")) {
        return process("restart");

    } else if (smatch(operation, "restart")) {
        if (!process("stop")) {
            return MPR_ERR_CANT_COMPLETE;
        }
        return process("start");

    } else if (smatch(operation, "start")) {
        if (launch) {
            return run("/bin/launchctl load /Library/LaunchDaemons/com.%s.%s.plist", slower(BLD_COMPANY), name);
        } else if (service) {
            return run("/sbin/service %s start");
        } else if (update) {
            return run("/usr/sbin/invoke-rc.d --quiet %s start", name);
        } else if (upstart) {
            return run("/sbin/start %s", name);
        }

    } else if (smatch(operation, "stop")) {
        if (launch) {
            return run("/bin/launchctl unload /Library/LaunchDaemons/com.%s.%s.plist", slower(BLD_COMPANY), name);
        } else if (service) {
            if (!run("/sbin/service %s stop")) {
                return killPid();
            }
            return 1;
        } else if (update) {
            if (!run("/usr/sbin/invoke-rc.d --quiet %s start", name)) {
                return killPid();
            }
            return 1;
        } else if (upstart) {
            return run("/sbin/stop %s", name);
        }

    } else if (smatch(operation, "uninstall")) {
        if (launch) {
            process("unload");
        } else if (service) {
            return run("/sbin/chkconfig --del %s", name);
        } else if (update) {
            return run("/usr/sbin/update-rc.d -f %s remove", name);
        } else if (upstart) {
            return process("disable");
        }
    }
    return 1;
}


static void runService()
{
    MprTime     mark;
    char        **av, *env[3], **argv;
    int         err, i, status, ac, next;

    app->servicePid = 0;
    atexit(cleanup);

    mprLog(1, "%s: Watching over %s", app->appName, app->serviceProgram);

    if (access(app->serviceProgram, X_OK) < 0) {
        mprError("start: can't access %s, errno %d", app->serviceProgram, mprGetOsError());
        return;
    }
    if (writePid(getpid()) < 0) {
        return;
    }
    mark = mprGetTime();

    while (!mprIsStopping()) {
        if (mprGetElapsedTime(mark) > (3600 * 1000)) {
            mark = mprGetTime();
            app->restartCount = 0;
            app->restartWarned = 0;
        }
        if (app->servicePid == 0) {
            if (app->restartCount >= app->retries) {
                if (! app->restartWarned) {
                    mprError("Too many restarts for %s, %d in ths last hour", app->serviceProgram, app->restartCount);
                    mprError("Suspending restarts for one hour");
                    app->restartWarned++;
                }
                mprSleep(60 * 1000);
                continue;
            }

            /*
                Create the child
             */
            app->servicePid = vfork();
            if (app->servicePid < 0) {
                mprError("Can't fork new process to run %s", app->serviceProgram);
                continue;

            } else if (app->servicePid == 0) {
                /*
                    Child
                 */
                umask(022);
                setsid();

                mprLog(1, "%s: Change dir to %s", app->appName, app->serviceHome);
                if (chdir(app->serviceHome) < 0) {}

                for (i = 3; i < 128; i++) {
                    close(i);
                }
                if (app->serviceArgs && *app->serviceArgs) {
                    ac = mprMakeArgv(app->serviceArgs, &av, 0);
                } else {
                    ac = 0;
                }
                argv = mprAlloc(sizeof(char*) * (6 + ac));
                env[0] = sjoin("LD_LIBRARY_PATH=", app->serviceHome, NULL);
                env[1] = sjoin("PATH=", getenv("PATH"), NULL);
                env[2] = 0;

                next = 0;
                argv[next++] = app->serviceProgram;
                if (app->logSpec) {
                    argv[next++] = "--log";
                    argv[next++] = (char*) app->logSpec;
                }
                for (i = 0; i < ac; i++) {
                    argv[next++] = av[i];
                }
                argv[next++] = 0;

                mprLog(1, "%s: Running %s", app->appName, app->serviceProgram);
                for (i = 1; argv[i]; i++) {
                    mprLog(1, "%s: argv[%d] = %s", app->appName, i, argv[i]);
                }
                execve(app->serviceProgram, argv, (char**) &env);

                /* Should not get here */
                err = errno;
                mprError("%s: Can't exec %s, err %d, cwd %s", app->appName, app->serviceProgram, err, app->serviceHome);
                exit(MPR_ERR_CANT_INITIALIZE);
            }

            /*
                Parent
             */
            mprLog(1, "%s: create child %s at pid %d", app->appName, app->serviceProgram, app->servicePid);
            app->restartCount++;

            waitpid(app->servicePid, &status, 0);
            mprLog(1, "%s: %s has exited with status %d, restarting (%d/%d)...", 
                app->appName, app->serviceProgram, WEXITSTATUS(status), app->restartCount, app->retries);
            app->servicePid = 0;
        }
    }
}


#if UNUSED
static int setupUnixSignals()
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = catchSignal;
    act.sa_flags = 0;

    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGCHLD);
    sigaddset(&act.sa_mask, SIGALRM);
    sigaddset(&act.sa_mask, SIGINT);
    sigaddset(&act.sa_mask, SIGPIPE);
    sigaddset(&act.sa_mask, SIGTERM);
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaddset(&act.sa_mask, SIGUSR2);
    sigaddset(&act.sa_mask, SIGTERM);

    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);

    signal(SIGPIPE, SIG_IGN);
    return 0;
}


/*
    Catch signals and kill the service
 */
static void catchSignal(int signo, siginfo_t *info, void *arg)
{
    cleanup();
    mprTerminate(0, -1);
}
#endif


static void cleanup()
{
    if (app->servicePid > 0) {
        mprLog(1, "%s: Killing %s at pid %d", app->appName, app->serviceProgram, app->servicePid);
        kill(app->servicePid, SIGTERM);
        app->servicePid = 0;
    }
}


/*
    Get the pid for the current running manager service
 */
static int readPid()
{
    int     pid, fd;

    if ((fd = open(app->pidPath, O_RDONLY, 0666)) < 0) {
        return -1;
    }
    if (read(fd, &pid, sizeof(pid)) != sizeof(pid)) {
        close(fd);
        return -1;
    }
    close(fd);
    return pid;
}


static bool killPid()
{
    int     pid;

    if ((pid = readPid()) > 1) {
        return kill(pid, SIGTERM) == 0;
    }
    return 0;
}


/*
    Write the pid so the manager and service can be killed via --stop
 */ 
static int writePid(int pid)
{
    int     fd;

    if ((fd = open(app->pidPath, O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) {
        mprError("Could not create pid file %s\n", app->pidPath);
        return MPR_ERR_CANT_CREATE;
    }
    if (write(fd, &pid, sizeof(pid)) != sizeof(pid)) {
        mprError("Write to file %s failed\n", app->pidPath);
        return MPR_ERR_CANT_WRITE;
    }
    close(fd);
    return 0;
}


/*
    Convert this Manager to a Deaemon
 */
static int makeDaemon()
{
    struct sigaction    act, old;
    int                 pid, status;

    /*
        Handle child death
     */
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = (void (*)(int, siginfo_t*, void*)) SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO /* | SA_NOMASK */;

    if (sigaction(SIGCHLD, &act, &old) < 0) {
        mprError("Can't initialize signals");
        return MPR_ERR_BAD_STATE;
    }

    /*
        Fork twice to get a free child with no parent
     */
    if ((pid = fork()) < 0) {
        mprError("Fork failed for background operation");
        return MPR_ERR;

    } else if (pid == 0) {
        if ((pid = fork()) < 0) {
            mprError("Second fork failed");
            exit(127);

        } else if (pid > 0) {
            /* Parent of second child -- must exit */
            exit(0);
        }

        /*
            This is the real child that will continue as a daemon
         */
        setsid();
        if (sigaction(SIGCHLD, &old, 0) < 0) {
            mprError("Can't restore signals");
            return MPR_ERR_BAD_STATE;
        }
        mprLog(2, "Switching to background operation");
        return 0;
    }

    /*
        Original process waits for first child here. Must get child death notification with a successful exit status
     */
    while (waitpid(pid, &status, 0) != pid) {
        if (errno == EINTR) {
            mprSleep(100);
            continue;
        }
        mprError("Can't wait for daemon parent.");
        exit(0);
    }
    if (WEXITSTATUS(status) != 0) {
        mprError("Daemon parent had bad exit status.");
        exit(0);
    }
    if (sigaction(SIGCHLD, &old, 0) < 0) {
        mprError("Can't restore signals");
        return MPR_ERR_BAD_STATE;
    }
    exit(0);
}


#elif BLD_WIN_LIKE
/*********************************** Locals ***********************************/

#define HEART_BEAT_PERIOD   (10 * 1000) /* Default heart beat period (10 sec) */
#define RESTART_MAX         (15)        /* Max restarts per hour */
#define SERVICE_DESCRIPTION ("Manages " BLD_NAME)

static Mpr          *mpr;            /* Global MPR App context */

typedef struct App {
    HWND         hwnd;               /* Application window handle */
    HINSTANCE    appInst;            /* Current application instance */
    cchar        *appName;           /* Manager name */
    cchar        *appTitle;          /* Manager title */
    int          createConsole;      /* Display service console */
    int          exiting;            /* Program should exit */
    int          heartBeatPeriod;    /* Service heart beat interval */
    HANDLE       heartBeatEvent;     /* Heart beat event event to sleep on */
    HWND         otherHwnd;          /* Existing instance window handle */
    int          restartCount;       /* Service restart count */
    int          restartWarned;      /* Has user been notified */
    char         *serviceArgs;       /* Args to pass to service */
    cchar        *serviceHome;       /* Service home */
    cchar        *serviceName;       /* Application name */
    char         *serviceProgram;    /* Service program name */
    int          servicePid;         /* Process ID for the service */
    cchar        *serviceTitle;      /* Application title */
    HANDLE       serviceThreadEvent; /* Service event to block on */
    int          serviceStopped;     /* Service stopped */
    HANDLE       threadHandle;       /* Handle for the service thread */
} App;

static App *app;

static SERVICE_STATUS           svcStatus;
static SERVICE_STATUS_HANDLE    svcHandle;
static SERVICE_TABLE_ENTRY      svcTable[] = {
    { "default",    0   },
    { 0,            0   }
};

static void     WINAPI serviceCallback(ulong code);
static void     initService();
static int      installService();
static void     logHandler(int flags, int level, cchar *msg);
static int      registerService();
static int      removeService(int removeFromScmDb);
static void     gracefulShutdown(MprTime timeout);
static int      startDispatcher(LPSERVICE_MAIN_FUNCTION svcMain);
static int      startService();
static int      stopService(int cmd);
static int      tellSCM(long state, long exitCode, long wait);
static void     updateStatus(int status, int exitCode);
static void     writeToOsLog(cchar *message);
static void     run();

/***************************** Forward Declarations ***************************/

static void     manageApp(void *unused, int flags);
static LRESULT  msgProc(HWND hwnd, uint msg, uint wp, long lp);

static void     serviceThread(void *data);
static void WINAPI serviceMain(ulong argc, char **argv);

/*********************************** Code *************************************/

int APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *args, int junk2)
{
    char    **argv, *argp;
    int     argc, err, nextArg, installFlag;

    mpr = mprCreate(0, NULL, 0);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    mprAddStandardSignals();

    err = 0;
    installFlag = 0;
    app->appInst = inst;
    app->heartBeatPeriod = HEART_BEAT_PERIOD;

    initService();

    mprSetAppName(BLD_PRODUCT "Manager", BLD_NAME "Manager", BLD_VERSION);
    app->appName = mprGetAppName();
    app->appTitle = mprGetAppTitle(mpr);
    mprSetLogHandler(logHandler);
    mprSetWinMsgCallback((MprMsgCallback) msgProc);

    if ((argc = mprMakeArgv(args, &argv, MPR_ARGV_ARGS_ONLY)) < 0) {
        return FALSE;
    }
    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-' || strcmp(argp, "--") == 0) {
            break;
        }
        if (strcmp(argp, "--args") == 0) {
            /*
                Args to pass to service when it starts
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceArgs = argv[++nextArg];
            }

        } else if (strcmp(argp, "--console") == 0) {
            /*
                Allow the service to interact with the console
             */
            app->createConsole++;

        } else if (strcmp(argp, "--daemon") == 0) {
            /* Ignored on windows */

        } else if (strcmp(argp, "--heartBeat") == 0) {
            /*
                Set the heart beat period
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->heartBeatPeriod = atoi(argv[++nextArg]) * 1000;
            }

        } else if (strcmp(argp, "--home") == 0) {
            /*
                Change to this directory before starting the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceHome = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--program") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceProgram = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--install") == 0) {
            installFlag++;

        } else if (strcmp(argp, "--start") == 0) {
//  MOB - use same operation style 
            /*
                Start the manager
             */
            if (startService() < 0) {
                return FALSE;
            }
            mprSleep(2000);    /* Time for service to really start */

        } else if (strcmp(argp, "--stop") == 0) {
            /*
                Stop the  manager
             */
            if (removeService(0) < 0) {
                return FALSE;
            }

        } else if (strcmp(argp, "--uninstall") == 0) {
            /*
                Remove the  manager
             */
            if (removeService(1) < 0) {
                return FALSE;
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            mprSetLogLevel(1);

        } else {
            err++;
        }
        if (err) {
            mprUserError("Bad command line: %s\n"
                "  Usage: %s [options] [program args]\n"
                "  Switches:\n"
                "    --args               # Args to pass to service\n"
                "    --console            # Display the service console\n"
                "    --heartBeat interval # Heart beat interval period (secs)\n"
                "    --home path          # Home directory for service\n"
                "    --install            # Install the service\n"
                "    --program            # Service program to start\n"
                "    --start              # Start the service\n"
                "    --stop               # Stop the service\n"
                "    --uninstall          # Uninstall the service",
                args, app->appName);
            return -1;
        }
    }
    //  MOB - operation style
    if (installFlag) {
        /*
            Install the manager
         */
        if (installService(app->serviceArgs) < 0) {
            return FALSE;
        }
    }
    if (argc <= 1) {
        /*
            This will block if we are a service and are being started by the
            service control manager. While blocked, the svcMain will be called
            which becomes the effective main program. 
         */
        startDispatcher(serviceMain);
    }
    return 0;
}

static void manageApp(void *ptr, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->appName);
        mprMark(app->appTitle);
        mprMark(app->serviceHome);
        mprMark(app->serviceName);
        mprMark(app->serviceProgram);
        mprMark(app->serviceTitle);
        mprMark(app->serviceArgs);
    }
}

/*
    Secondary entry point when started by the service control manager. Remember 
    that the main program thread is blocked in the startDispatcher called from
    winMain and will be used on callbacks from WinService.
 */ 
static void WINAPI serviceMain(ulong argc, char **argv)
{
    int     threadId;

    app->serviceThreadEvent = CreateEvent(0, TRUE, FALSE, 0);
    app->heartBeatEvent = CreateEvent(0, TRUE, FALSE, 0);

    if (app->serviceThreadEvent == 0 || app->heartBeatEvent == 0) {
        mprError("Can't create wait events");
        return;
    }
    app->threadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) serviceThread, (void*) 0, 0, (ulong*) &threadId);

    if (app->threadHandle == 0) {
        mprError("Can't create service thread");
        return;
    }
    WaitForSingleObject(app->serviceThreadEvent, INFINITE);
    CloseHandle(app->serviceThreadEvent);

    /*
        We are now exiting, so wakeup the heart beat thread
     */
    app->exiting = 1;
    SetEvent(app->heartBeatEvent);
    CloseHandle(app->heartBeatEvent);
}


static void serviceThread(void *data)
{
    if (registerService() < 0) {
        mprError("Can't register service");
        ExitThread(0);
        return;
    }
    updateStatus(SERVICE_RUNNING, 0);

    /*
        Start the service and watch over it.
     */
    run();
    updateStatus(SERVICE_STOPPED, 0);
    ExitThread(0);
}


static void run()
{
    PROCESS_INFORMATION procInfo;
    STARTUPINFO         startInfo;
    MprTime             mark;
    ulong               status;
    char                *path, *cmd, *key;
    int                 createFlags;

    createFlags = 0;

#if USEFUL_FOR_DEBUG
    DebugBreak();
#endif

    /*
        Read the service home directory and args. Default to the current dir if none specified.
     */
    key = sfmt("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", app->serviceName);
    app->serviceHome = mprReadRegistry(key, "HomeDir");
    app->serviceArgs = mprReadRegistry(key, "Args");

    /*
        Expect to find the service executable in the same directory as this manager program.
     */
    if (app->serviceProgram == 0) {
        path = sfmt("\"%s\\%s.exe\"", mprGetAppDir(), BLD_PRODUCT);
    } else {
        path = sfmt("\"%s\"", app->serviceProgram);
    }
    if (app->serviceArgs && *app->serviceArgs) {
        cmd = sfmt("%s %s", path, app->serviceArgs);
    } else {
        cmd = path;
    }
    if (app->createConsole) {
        createFlags |= CREATE_NEW_CONSOLE;
    }
    mark = mprGetTime();

    while (! app->exiting) {
        if (mprGetElapsedTime(mark) > (3600 * 1000)) {
            mark = mprGetTime();
            app->restartCount = 0;
            app->restartWarned = 0;
        }
        if (app->servicePid == 0 && !app->serviceStopped) {
            if (app->restartCount >= RESTART_MAX) {
                if (! app->restartWarned) {
                    mprError("Too many restarts for %s, %d in ths last hour", app->appTitle, app->restartCount);
                    app->restartWarned++;
                }
                /*
                    This is not a real heart-beat. We are only waiting till the service process exits.
                 */
                WaitForSingleObject(app->heartBeatEvent, app->heartBeatPeriod);
                continue;
            }
            memset(&startInfo, 0, sizeof(startInfo));
            startInfo.cb = sizeof(startInfo);

            /*
                Launch the process
             */
            if (! CreateProcess(0, cmd, 0, 0, FALSE, createFlags, 0, app->serviceHome, &startInfo, &procInfo)) {
                mprError("Can't create process: %s, %d", cmd, mprGetOsError());
            } else {
                app->servicePid = (int) procInfo.hProcess;
            }
            app->restartCount++;
        }
        WaitForSingleObject(app->heartBeatEvent, app->heartBeatPeriod);

        if (app->servicePid) {
            if (GetExitCodeProcess((HANDLE) app->servicePid, (ulong*) &status)) {
                if (status != STILL_ACTIVE) {
                    CloseHandle((HANDLE) app->servicePid);
                    app->servicePid = 0;
                }
            } else {
                CloseHandle((HANDLE) app->servicePid);
                app->servicePid = 0;
            }
        }
        mprLog(1, "%s has exited with status %d", app->serviceProgram, status);
        mprLog(1, "%s will be restarted in 10 seconds", app->serviceProgram);
    }
}


static int startDispatcher(LPSERVICE_MAIN_FUNCTION svcMain)
{
    SC_HANDLE       mgr;
    char            name[80];
    ulong           len;

    if (!(mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        mprError("Can't open service manager");
        return MPR_ERR_CANT_OPEN;
    }
    /*
        Is the service installed?
     */
    len = sizeof(name);
    if (GetServiceDisplayName(mgr, app->serviceName, name, &len) == 0) {
        CloseServiceHandle(mgr);
        return MPR_ERR_CANT_READ;
    }
    /*
        Register this service with the SCM. This call will block and consume the main thread if the service is 
        installed and the app was started by the SCM. If started manually, this routine will return 0.
     */
    svcTable[0].lpServiceProc = svcMain;
    if (StartServiceCtrlDispatcher(svcTable) == 0) {
        mprError("Could not start the service control dispatcher");
        return MPR_ERR_CANT_INITIALIZE;
    }
    return 0;
}


/*
    Should be called first thing after the service entry point is called by the service manager
 */

static int registerService()
{
    svcHandle = RegisterServiceCtrlHandler(app->serviceName, serviceCallback);
    if (svcHandle == 0) {
        mprError("Can't register handler: %x", GetLastError());
        return MPR_ERR_CANT_INITIALIZE;
    }
    /*
        Report the svcStatus to the service control manager.
     */
    svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    svcStatus.dwServiceSpecificExitCode = 0;

    if (!tellSCM(SERVICE_START_PENDING, NO_ERROR, 1000)) {
        tellSCM(SERVICE_STOPPED, 0, 0);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


static void updateStatus(int status, int exitCode)
{
    tellSCM(status, exitCode, 10000);
}


/*
    Service callback. Invoked by the SCM.
 */ 
static void WINAPI serviceCallback(ulong cmd)
{
    switch(cmd) {
    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_PAUSE:
        app->serviceStopped = 1;
        SuspendThread(app->threadHandle);
        svcStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    case SERVICE_CONTROL_STOP:
        stopService(SERVICE_CONTROL_STOP);
        break;

    case SERVICE_CONTROL_CONTINUE:
        app->serviceStopped = 0;
        ResumeThread(app->threadHandle);
        svcStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        stopService(SERVICE_CONTROL_SHUTDOWN);
        return;

    default:
        break;
    }
    tellSCM(svcStatus.dwCurrentState, NO_ERROR, 0);
}


static int installService()
{
    SC_HANDLE   svc, mgr;
    char        cmd[MPR_MAX_FNAME], key[MPR_MAX_FNAME];
    int         serviceType;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprUserError("Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }
    /*
        Install this app as a service
     */
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (svc == NULL) {
        serviceType = SERVICE_WIN32_OWN_PROCESS;
        if (app->createConsole) {
            serviceType |= SERVICE_INTERACTIVE_PROCESS;
        }
        GetModuleFileName(0, cmd, sizeof(cmd));
        svc = CreateService(mgr, app->serviceName, app->serviceTitle, SERVICE_ALL_ACCESS, serviceType, SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, cmd, NULL, NULL, "", NULL, NULL);
        if (! svc) {
            mprUserError("Can't create service: 0x%x == %d", GetLastError(), GetLastError());
            CloseServiceHandle(mgr);
            return MPR_ERR_CANT_CREATE;
        }
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);

    /*
        Write a service description
     */
    mprSprintf(key, sizeof(key), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\" "Services\\%s", app->serviceName);

    if (mprWriteRegistry(key, "Description", SERVICE_DESCRIPTION) < 0) {
        return MPR_ERR_CANT_WRITE;
    }

    /*
        Write the home directory
     */
    if (app->serviceHome == 0) {
        app->serviceHome = mprGetPathParent(mprGetAppDir());
    }
    if (mprWriteRegistry(key, "HomeDir", app->serviceHome) < 0) {
        return MPR_ERR_CANT_WRITE;
    }

    /*
        Write the service args
     */
    if (app->serviceArgs && *app->serviceArgs) {
        if (mprWriteRegistry(key, "Args", app->serviceArgs) < 0) {
            return MPR_ERR_CANT_WRITE;
        }
    }
    return 0;
}


/*
    Remove the application service
 */ 
static int removeService(int removeFromScmDb)
{
    SC_HANDLE   svc, mgr;

    app->exiting = 1;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError("Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        CloseServiceHandle(mgr);
        mprError("Can't open service");
        return MPR_ERR_CANT_OPEN;
    }
    gracefulShutdown(0);

    if (ControlService(svc, SERVICE_CONTROL_STOP, &svcStatus)) {
        mprSleep(500);

        while (QueryServiceStatus(svc, &svcStatus)) {
            if (svcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                mprSleep(250);
            } else {
                break;
            }
        }
        if (svcStatus.dwCurrentState != SERVICE_STOPPED) {
            mprError("Can't stop service: %x", GetLastError());
        }
    }
    if (removeFromScmDb && !DeleteService(svc)) {
        mprError("Can't delete service: %x", GetLastError());
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);
    return 0;
}


/*
    Start the window's service
 */ 
static int startService()
{
    SC_HANDLE   svc, mgr;
    int         rc;

    app->exiting = 0;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError("Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        mprError("Can't open service");
        CloseServiceHandle(mgr);
        return MPR_ERR_CANT_OPEN;
    }
    rc = StartService(svc, 0, NULL);
    if (rc == 0) {
        mprError("Can't start %s service: %d", app->serviceName, GetLastError());
        return MPR_ERR_CANT_INITIALIZE;
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);
    return 0;
}


/*
    Stop the service in the current process. 
 */ 
static int stopService(int cmd)
{
    int     exitCode;

    app->exiting = 1;
    app->serviceStopped = 1;

    gracefulShutdown(10 * 1000);
    if (cmd == SERVICE_CONTROL_SHUTDOWN) {
        return 0;
    }

    /*
        Wakeup service event. This will make it exit.
     */
    SetEvent(app->serviceThreadEvent);

    svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    tellSCM(svcStatus.dwCurrentState, NO_ERROR, 1000);

    exitCode = 0;
    GetExitCodeThread(app->threadHandle, (ulong*) &exitCode);

    while (exitCode == STILL_ACTIVE) {
        GetExitCodeThread(app->threadHandle, (ulong*) &exitCode);
        mprSleep(100);
        tellSCM(svcStatus.dwCurrentState, NO_ERROR, 125);
    }
    svcStatus.dwCurrentState = SERVICE_STOPPED;
    tellSCM(svcStatus.dwCurrentState, exitCode, 0);
    return 0;
}


/*
    Tell SCM our current status
 */ 
static int tellSCM(long state, long exitCode, long wait)
{
    static ulong generation = 1;

    svcStatus.dwWaitHint = wait;
    svcStatus.dwCurrentState = state;
    svcStatus.dwWin32ExitCode = exitCode;

    if (state == SERVICE_START_PENDING) {
        svcStatus.dwControlsAccepted = 0;
    } else {
        svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
    }
    if ((state == SERVICE_RUNNING) || (state == SERVICE_STOPPED)) {
        svcStatus.dwCheckPoint = 0;
    } else {
        svcStatus.dwCheckPoint = generation++;
    }

    /*
        Report the svcStatus of the service to the service control manager
     */
    return SetServiceStatus(svcHandle, &svcStatus);
}


static void initService()
{
    app->serviceName = BLD_COMPANY "-" BLD_PRODUCT;
    app->serviceTitle = BLD_NAME;
    app->serviceStopped = 0;
    app->serviceProgram = sjoin(mprGetAppDir(), "/", BLD_PRODUCT, ".exe", NULL);
}

/*
    Windows message processing loop
 */
static LRESULT msgProc(HWND hwnd, uint msg, uint wp, long lp)
{
    switch (msg) {
    case WM_DESTROY:
        break;

    case WM_QUIT:
        gracefulShutdown(0);
        break;
    
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}


/*
    Default log output is just to the console
 */
static void logHandler(int flags, int level, cchar *msg)
{
    if (flags & MPR_USER_MSG) {
        MessageBoxEx(NULL, msg, mprGetAppTitle(), MB_OK, 0);
    }
    mprWriteToOsLog(msg, 0, 0);
}


static void gracefulShutdown(MprTime timeout)
{
    HWND    hwnd;

    hwnd = FindWindow(BLD_PRODUCT, BLD_NAME);
    if (hwnd) {
        PostMessage(hwnd, WM_QUIT, 0, 0L);

        /*
            Wait for up to ten seconds while the service exits
         */
        while (timeout > 0 && hwnd) {
            mprSleep(100);
            timeout -= 100;
            hwnd = FindWindow(BLD_PRODUCT, BLD_NAME);
            if (hwnd == 0) {
                return;
            }
        }
    }
    if (app->servicePid) {
        TerminateProcess((HANDLE) app->servicePid, 2);
        app->servicePid = 0;
    }
}


#else
void stubManager() {
    fprintf(stderr, "Manager not supported on this architecture");
}
#endif /* BLD_WIN_LIKE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
