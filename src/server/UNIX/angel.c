/**
 *  angel.c -- Angel monitor program for Unix
 *
 *  The angel starts, monitors and restarts daemon programs.
 *
 *  Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************* Includes ***********************************/

#include    "mpr.h"

#if BLD_HOST_UNIX
/*********************************** Locals ***********************************/

#define RESTART_DELAY (0 * 1000)            /* Default heart beat period (30 sec) */
#define RESTART_MAX   (100)                 /* Max restarts per hour */

static Mpr          *mpr;                   /* Global MPR context */
static cchar        *appName;               /* Angel name */
static int          exiting;                /* Program should exit */
static cchar        *homeDir;               /* Home directory for service */
static cchar        *logSpec;               /* Log directive for service */
static int          restartCount;           /* Service restart count */
static int          runAsDaemon;            /* Run as a daemon */
static int          servicePid;             /* Process ID for the service */
static char         *serviceArgs;           /* Args to pass to service */
static char         *serviceProgram;        /* Program to start */
static char         *serviceName;           /* Basename of service program */
static char         *pidDir;                /* Location for pid file */
static char         *pidPath;               /* Path to the angel pid for this service */
static int          verbose;                /* Run in verbose mode */

/***************************** Forward Declarations ***************************/

static void     angel();
static void     catchSignal(int signo, siginfo_t *info, void *arg);
static void     cleanup();
static int      makeDaemon();
static void     memoryFailure(MprCtx ctx, int64 size, int64 total, bool granted);
static int      readAngelPid();
static void     setAppDefaults(Mpr *mpr);
static int      setupUnixSignals();
static void     stopService(int timeout);
static int      writeAngelPid(int pid);

/*********************************** Code *************************************/

int main(int argc, char *argv[])
{
    char    *argp;
    int     err, nextArg, i, len;

    mpr = mprCreate(argc, argv, memoryFailure);

    err = 0;
    exiting = 0;
    runAsDaemon = 0;
    logSpec = 0;

    setAppDefaults(mpr);

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }

        if (strcmp(argp, "--args") == 0) {
            /*
             *  Args to pass to service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                serviceArgs = argv[++nextArg];
            }

        } else if (strcmp(argp, "--console") == 0) {
            /* Does nothing. Here for compatibility with windows watcher */

        } else if (strcmp(argp, "--daemon") == 0) {
            runAsDaemon++;

#if UNUSED
        } else if (strcmp(argp, "--heartBeat") == 0) {
            /*
             *  Set the frequency to check on the program.
             */
            if (nextArg >= argc) {
                err++;
            } else {
                heartBeatPeriod = atoi(argv[++nextArg]);
            }
#endif

        } else if (strcmp(argp, "--home") == 0) {
            /*
             *  Change to this directory before starting the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = argv[++nextArg];
            }

        } else if (strcmp(argp, "--log") == 0) {
            /*
             *  Pass the log directive through to the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                logSpec = argv[++nextArg];
            }

#if UNUSED
        } else if (strcmp(argp, "--program") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                serviceProgram = argv[++nextArg];
                serviceName = mprGetPathBase(mpr, serviceProgram);
            }
#endif

        } else if (strcmp(argp, "--stop") == 0) {
            /*
             *  Stop a currently running angel
             */
            stopService(10 * 1000);
            return 0;

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            verbose++;

        } else {
            err++;
        }
        if (err) {
            mprUserError(mpr,
                "Bad command line: \n"
                "  Usage: %s [options] [program args ...]\n"
                "  Switches:\n"
                "    --args               # Args to pass to service\n"
                "    --daemon             # Run as a daemon\n"
#if UNUSED
                "    --heartBeat interval # Heart beat interval period (secs) \n"
                "    --program path       # Service program to start\n"
#endif
                "    --home path          # Home directory for service\n"
                "    --log logFile:level  # Log directive for service\n"
                "    --stop               # Stop the service",
                appName);
            return -1;
        }
    }

    if (nextArg < argc) {
        /* TODO - replace with mprJoin() */
        serviceProgram = argv[nextArg++];
        for (len = 0, i = nextArg; i < argc; i++) {
            len += strlen(argv[i]) + 1;
        }
        serviceArgs = mprAlloc(mpr, len + 1);
        for (len = 0, i = nextArg; i < argc; i++) {
            strcpy(&serviceArgs[len], argv[i]);
            len += strlen(argv[i]);
            serviceArgs[len++] = ' ';
        }
        serviceArgs[len] = '\0';
    }

    setupUnixSignals();

    if (runAsDaemon) {
        makeDaemon();
    }
    angel();

    return 0;
}


static void setAppDefaults(Mpr *mpr)
{
    appName = mprGetAppName(mpr);
    homeDir = mprGetAppDir(mpr);
    serviceProgram = mprJoinPath(mpr, homeDir, BLD_PRODUCT);
    serviceName = mprGetPathBase(mpr, serviceProgram);

    if (mprPathExists(mpr, "/var/run", X_OK)) {
        pidDir = "/var/run";
    } else if (mprPathExists(mpr, "/tmp", X_OK)) {
        pidDir = "/tmp";
    } else if (mprPathExists(mpr, "/Temp", X_OK)) {
        pidDir = "/Temp";
    } else {
        pidDir = ".";
    }
    pidPath = mprStrcat(mpr, -1, pidDir, "/angel-", serviceName, ".pid", NULL);
}


static void angel()
{
    MprTime     mark;
    char        **av, *env[3], **argv;
    int         err, i, restartWarned, status, ac, next;

    servicePid = 0;
    restartWarned = 0;

    atexit(cleanup);

    if (verbose) {
        mprPrintf(mpr, "%s: Watching over %s\n", appName, serviceProgram);
    }
    if (access(serviceProgram, X_OK) < 0) {
        mprError(mpr, "start: can't access %s, errno %d", serviceProgram, mprGetOsError());
        return;
    }
    if (writeAngelPid(getpid()) < 0) {
        return;
    }
    mark = mprGetTime(mpr);

    while (! exiting) {

        if (mprGetElapsedTime(mpr, mark) > (3600 * 1000)) {
            mark = mprGetTime(mpr);
            restartCount = 0;
            restartWarned = 0;
        }

        if (servicePid == 0) {
            if (restartCount >= RESTART_MAX) {
                if (! restartWarned) {
                    mprError(mpr, "Too many restarts for %s, %d in ths last hour", serviceProgram, restartCount);
                    mprError(mpr, "Suspending restarts for one hour");
                    restartWarned++;
                }
                continue;
            }

            /*
             *  Create the child
             */
            servicePid = vfork();
            if (servicePid < 0) {
                mprError(mpr, "Can't fork new process to run %s", serviceProgram);
                continue;

            } else if (servicePid == 0) {
                /*
                 *  Child
                 */
                umask(022);
                setsid();

                if (verbose) {
                    mprPrintf(mpr, "%s: Change dir to %s\n", appName, homeDir);
                }
                chdir(homeDir);

                for (i = 3; i < 128; i++) {
                    close(i);
                }

                if (serviceArgs && *serviceArgs) {
                    mprMakeArgv(mpr, "", serviceArgs, &ac, &av);
                } else {
                    ac = 0;
                }

                argv = mprAlloc(mpr, sizeof(char*) * (6 + ac));
                env[0] = mprStrcat(mpr, -1, "LD_LIBRARY_PATH=", homeDir, NULL);
                env[1] = mprStrcat(mpr, -1, "PATH=", getenv("PATH"), NULL);
                env[2] = 0;

                next = 0;
                argv[next++] = (char*) mprGetPathBase(mpr, serviceProgram);
                if (logSpec) {
                    argv[next++] = "--log";
                    argv[next++] = (char*) logSpec;
                }

                for (i = 1; i < ac; i++) {
                    argv[next++] = av[i];
                }
                argv[next++] = 0;

                if (verbose) {
                    mprPrintf(mpr, "%s: Running %s\n", appName, serviceProgram);
                    for (i = 1; argv[i]; i++) {
                        mprPrintf(mpr, "%s: argv[%d] = %s\n", appName, i, argv[i]);
                    }
                }
                execve(serviceProgram, argv, (char**) &env);

                /* Should not get here */
                err = errno;
                mprError(mpr, "%s: Can't exec %s, err %d, cwd %s", appName, serviceProgram, err, homeDir);
                exit(MPR_ERR_CANT_INITIALIZE);
            }

            /*
             *  Parent
             */
            if (verbose) {
                mprPrintf(mpr, "%s: create child %s at pid %d\n", appName, serviceProgram, servicePid);
            }
            restartCount++;

            waitpid(servicePid, &status, 0);
            if (verbose) {
                mprPrintf(mpr, "%s: %s has exited with status %d, restarting ...\n", appName, serviceProgram, 
                    WEXITSTATUS(status));
            }
            servicePid = 0;
        }
    }
}


/*
 *  Stop an another instance of the angel
 */
static void stopService(int timeout)
{
    int     pid;

    pid = readAngelPid();
    if (pid > 1) {
        kill(pid, SIGTERM);
    }
}


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
 *  Catch signals and kill the service
 */
static void catchSignal(int signo, siginfo_t *info, void *arg)
{
    cleanup();
    exit(2);
}


static void cleanup()
{
    if (servicePid > 0) {
        if (verbose) {
            mprPrintf(mpr, "\n%s: Killing %s at pid %d\n", appName, serviceProgram, servicePid);
        }
        kill(servicePid, SIGTERM);
        servicePid = 0;
    }
}


/*
 *  Get the pid for the current running angel service
 */
static int readAngelPid()
{
    int     pid, fd;

    if ((fd = open(pidPath, O_RDONLY, 0666)) < 0) {
        return -1;
    }
    if (read(fd, &pid, sizeof(pid)) != sizeof(pid)) {
        close(fd);
        return -1;
    }
    close(fd);
    return pid;
}


/*
 *  Write the pid so the angel and service can be killed via --stop
 */ 
static int writeAngelPid(int pid)
{
    int     fd;

    if ((fd = open(pidPath, O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) {
        mprError(mpr, "Could not create pid file %s\n", pidPath);
        return MPR_ERR_CANT_CREATE;
    }
    if (write(fd, &pid, sizeof(pid)) != sizeof(pid)) {
        mprError(mpr, "Write to file %s failed\n", pidPath);
        return MPR_ERR_CANT_WRITE;
    }
    close(fd);
    return 0;
}


/*
 *  Conver this Angel to a Deaemon
 */
static int makeDaemon()
{
    struct sigaction    act, old;
    int                 pid, status;

    /*
     *  Handle child death
     */
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = (void (*)(int, siginfo_t*, void*)) SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO /* | SA_NOMASK */;

    if (sigaction(SIGCHLD, &act, &old) < 0) {
        mprError(mpr, "Can't initialize signals");
        return MPR_ERR_BAD_STATE;
    }

    /*
     *  Fork twice to get a free child with no parent
     */
    if ((pid = fork()) < 0) {
        mprError(mpr, "Fork failed for background operation");
        return MPR_ERR_GENERAL;

    } else if (pid == 0) {
        if ((pid = fork()) < 0) {
            mprError(mpr, "Second fork failed");
            exit(127);

        } else if (pid > 0) {
            /* Parent of second child -- must exit */
            exit(0);
        }

        /*
         *  This is the real child that will continue as a daemon
         */
        setsid();
        if (sigaction(SIGCHLD, &old, 0) < 0) {
            mprError(mpr, "Can't restore signals");
            return MPR_ERR_BAD_STATE;
        }
        mprLog(mpr, 2, "Switching to background operation\n");
        return 0;
    }

    /*
     *  Original process waits for first child here. Must get child death notification with a successful exit status
     */
    while (waitpid(pid, &status, 0) != pid) {
        if (errno == EINTR) {
            mprSleep(mpr, 100);
            continue;
        }
        mprError(mpr, "Can't wait for daemon parent.");
        exit(0);
    }
    if (WEXITSTATUS(status) != 0) {
        mprError(mpr, "Daemon parent had bad exit status.");
        exit(0);
    }
    if (sigaction(SIGCHLD, &old, 0) < 0) {
        mprError(mpr, "Can't restore signals");
        return MPR_ERR_BAD_STATE;
    }
    exit(0);
}


/*
 *  Global memory allocation handler
 */
static void memoryFailure(MprCtx ctx, int64 size, int64 total, bool granted)
{
    if (!granted) {
        mprPrintfError(ctx, "Can't allocate memory block of size %d\n", size);
        mprPrintfError(ctx, "Total memory used %d\n", total);
        exit(255);
    }
    mprPrintf(ctx, "Memory request for %d bytes exceeds memory red-line\n", size);
    mprPrintf(ctx, "Total memory used %d\n", total);
}


#else
void __dummyAngel() {
    fprintf(stderr, "Angel not supported on this architecture");
}
#endif /* BLD_HOST_UNIX */
/*
 *  @copy   default
 *  
 *  Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
 *  Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
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
