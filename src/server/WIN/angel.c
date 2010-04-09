/**
    angel.c - Windows Angel program

    The appweb angel program can run in 2 modes. In the first mode, it starts the appweb service and restarts 
    appweb should it fail. In the second mode, it is a simple command line program that interacts with another 
    instance of the angel to start or stop it via the --start, --stop, --install and --uninstall switches.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"

#if BLD_WIN_LIKE
/*********************************** Locals ***********************************/

#define HEART_BEAT_PERIOD   (10 * 1000) /* Default heart beat period (10 sec) */
#define RESTART_MAX         (15)        /* Max restarts per hour */
#define SERVICE_DESCRIPTION ("Manages " BLD_NAME)

static Mpr          *mpr;               /* Global MPR App context */
static HWND         appHwnd;            /* Application window handle */
static HINSTANCE    appInst;            /* Current application instance */
static cchar        *appName;           /* Angel name */
static cchar        *appTitle;          /* Angel title */
static int          createConsole;      /* Display service console */
static int          exiting;            /* Program should exit */
static int          heartBeatPeriod;    /* Service heart beat interval */
static HANDLE       heartBeatEvent;     /* Heart beat event event to sleep on */
static HWND         otherHwnd;          /* Existing instance window handle */
static int          restartCount;       /* Service restart count */
static cchar        *serviceHome;       /* Service home */
static cchar        *serviceName;       /* Application name */
static char         *serviceProgram;    /* Service program name */
static int          servicePid;         /* Process ID for the service */
static cchar        *serviceTitle;      /* Application title */
static HANDLE       serviceThreadEvent; /* Service event to block on */
static int          serviceStopped;     /* Service stopped */
static HANDLE       threadHandle;       /* Handle for the service thread */
static int          verbose;            /* Run in verbose mode */

static SERVICE_STATUS           svcStatus;
static SERVICE_STATUS_HANDLE    svcHandle;
static SERVICE_TABLE_ENTRY      svcTable[] = {
    { "default",    0   },
    { 0,            0   }
};

static void     WINAPI serviceCallback(ulong code);
static void     initService();
static int      installService(char *homeDir, char *args);
static void     logHandler(MprCtx ctx, int flags, int level, cchar *msg);
static int      registerService();
static int      removeService(int removeFromScmDb);
static void     shutdownAppweb(int timeout);
static int      startDispatcher(LPSERVICE_MAIN_FUNCTION svcMain);
static int      startService();
static int      stopService(int cmd);
static int      tellSCM(long state, long exitCode, long wait);
static void     updateStatus(int status, int exitCode);
static void     writeToOsLog(cchar *message);
static void     angel();

/***************************** Forward Declarations ***************************/

static int      initWindow();
static void     mapPathDelim(char *s);
static long     msgProc(HWND hwnd, uint msg, uint wp, long lp);

static void     serviceThread(void *data);
static void WINAPI serviceMain(ulong argc, char **argv);

/*********************************** Code *************************************/

int APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *args, int junk2)
{
    char    **argv, *argp, *serviceArgs, *homeDir;
    int     argc, err, nextArg, installFlag;

    mpr = mprCreate(0, NULL, NULL);

    appInst = inst;
    serviceArgs = 0;
    err = 0;
    exiting = 0;
    installFlag = 0;
    homeDir = 0;
    heartBeatPeriod = HEART_BEAT_PERIOD;

    initService();

    mprSetAppName(mpr, BLD_PRODUCT "Angel", BLD_NAME "Angel", BLD_VERSION);
    appName = mprGetAppName(mpr);
    appTitle = mprGetAppTitle(mpr);

    mprSetLogHandler(mpr, logHandler, NULL);

    /*
        Create the window 
     */
    if (initWindow() < 0) {
        mprError(mpr, "Can't initialize application Window");
        return FALSE;
    }

    /*
        Parse command line arguments
     */
    if (mprMakeArgv(mpr, "", args, &argc, &argv) < 0) {
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
                serviceArgs = argv[++nextArg];
            }

        } else if (strcmp(argp, "--console") == 0) {
            /*
                Allow the service to interact with the console
             */
            createConsole++;

        } else if (strcmp(argp, "--daemon") == 0) {
            /* Ignored on windows */

        } else if (strcmp(argp, "--heartBeat") == 0) {
            /*
                Set the heart beat period
             */
            if (nextArg >= argc) {
                err++;
            } else {
                heartBeatPeriod = atoi(argv[++nextArg]) * 1000;
            }

        } else if (strcmp(argp, "--home") == 0) {
            /*
                Change to this directory before starting the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = argv[++nextArg];
            }

        } else if (strcmp(argp, "--program") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                serviceProgram = argv[++nextArg];
            }

        } else if (strcmp(argp, "--install") == 0) {
            installFlag++;

        } else if (strcmp(argp, "--start") == 0) {
            /*
                Start the angel
             */
            if (startService() < 0) {
                return FALSE;
            }
            mprSleep(mpr, 2000);    /* Time for service to really start */

        } else if (strcmp(argp, "--stop") == 0) {
            /*
                Stop the  angel
             */
            if (removeService(0) < 0) {
                return FALSE;
            }

        } else if (strcmp(argp, "--uninstall") == 0) {
            /*
                Remove the  angel
             */
            if (removeService(1) < 0) {
                return FALSE;
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            verbose++;

        } else {
            err++;
        }

        if (err) {
            mprUserError(mpr, 
                "Bad command line: %s\n"
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
                args, appName);
            return -1;
        }
    }

    if (installFlag) {
        /*
            Install the angel
         */
        if (installService(homeDir, serviceArgs) < 0) {
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


/*
    Secondary entry point when started by the service control manager. Remember 
    that the main program thread is blocked in the startDispatcher called from
    winMain and will be used on callbacks from WinService.
 */ 
static void WINAPI serviceMain(ulong argc, char **argv)
{
    int     threadId;

    serviceThreadEvent = CreateEvent(0, TRUE, FALSE, 0);
    heartBeatEvent = CreateEvent(0, TRUE, FALSE, 0);

    if (serviceThreadEvent == 0 || heartBeatEvent == 0) {
        mprError(mpr, "Can't create wait events");
        return;
    }

    threadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) serviceThread, (void*) 0, 0, (ulong*) &threadId);

    if (threadHandle == 0) {
        mprError(mpr, "Can't create service thread");
        return;
    }

    WaitForSingleObject(serviceThreadEvent, INFINITE);
    CloseHandle(serviceThreadEvent);

    /*
        We are now exiting, so wakeup the heart beat thread
     */
    exiting = 1;
    SetEvent(heartBeatEvent);
    CloseHandle(heartBeatEvent);
}


static void serviceThread(void *data)
{
    if (registerService() < 0) {
        mprError(mpr, "Can't register service");
        ExitThread(0);
        return;
    }
    updateStatus(SERVICE_RUNNING, 0);

    /*
        Start the service and watch over it.
     */
    angel();
    updateStatus(SERVICE_STOPPED, 0);
    ExitThread(0);
}


static void angel()
{
    PROCESS_INFORMATION procInfo;
    STARTUPINFO         startInfo;
    MprTime             mark;
    ulong               status;
    char                *dir, *homeDir, *serviceArgs;
    char                key[MPR_MAX_FNAME], path[MPR_MAX_FNAME], cmd[MPR_MAX_FNAME];
    int                 createFlags, restartWarned;

    servicePid = 0;
    createFlags = 0;
    restartWarned = 0;

#if USEFUL_FOR_DEBUG
    DebugBreak();
#endif

    /*
        Read the service home directory and args. Default to the current dir if none specified.
     */
    homeDir = 0;
    serviceArgs = 0;
    mprSprintf(key, sizeof(key), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", serviceName);
    mprReadRegistry(mpr, &homeDir, MPR_MAX_FNAME, key, "HomeDir");
    mprReadRegistry(mpr, &serviceArgs, MPR_MAX_FNAME, key, "Args");

    /*
        Expect to find the service executable in the same directory as this angel program.
     */
    if (serviceProgram == 0) {
        GetModuleFileName(0, path, sizeof(path));
        dir = mprGetPathDir(mpr, path);
        mprSprintf(path, sizeof(path), "\"%s\\%s.exe\"", dir, BLD_PRODUCT);
        mprFree(dir);
    } else {
        mprSprintf(path, sizeof(path), "\"%s\"", serviceProgram);
    }

    if (serviceArgs && *serviceArgs) {
        mprSprintf(cmd, sizeof(cmd), "%s %s", path, serviceArgs);
    } else {
        mprSprintf(cmd, sizeof(cmd), "%s", path);
    }

    if (createConsole) {
        createFlags |= CREATE_NEW_CONSOLE;
    }

    mark = mprGetTime(mpr);

    while (! exiting) {

        if (mprGetElapsedTime(mpr, mark) > (3600 * 1000)) {
            mark = mprGetTime(mpr);
            restartCount = 0;
            restartWarned = 0;
        }

        if (servicePid == 0 && !serviceStopped) {
            if (restartCount >= RESTART_MAX) {
                if (! restartWarned) {
                    mprError(mpr, "Too many restarts for %s, %d in ths last hour", appTitle, restartCount);
                    restartWarned++;
                }
                /*
                    This is not a real heart-beat. We are only waiting till the service process exits.
                 */
                WaitForSingleObject(heartBeatEvent, heartBeatPeriod);
                continue;
            }

            memset(&startInfo, 0, sizeof(startInfo));
            startInfo.cb = sizeof(startInfo);

            /*
                Launch the process
             */
            if (! CreateProcess(0, cmd, 0, 0, FALSE, createFlags, 0, homeDir, &startInfo, &procInfo)) {
                mprError(mpr, "Can't create process: %s, %d", cmd, mprGetOsError());

            } else {
                servicePid = (int) procInfo.hProcess;
            }
            restartCount++;
        }
        WaitForSingleObject(heartBeatEvent, heartBeatPeriod);

        if (servicePid) {
            if (GetExitCodeProcess((HANDLE) servicePid, (ulong*) &status)) {
                if (status != STILL_ACTIVE) {
                    CloseHandle((HANDLE) servicePid);
                    servicePid = 0;
                }

            } else {
                CloseHandle((HANDLE) servicePid);
                servicePid = 0;
            }
        }
        if (verbose) {
            mprPrintf(mpr, "%s has exited with status %d\n", serviceProgram, status);
            mprPrintf(mpr, "%s will be restarted in 10 seconds\n", serviceProgram);
        }
    }
    mprFree(homeDir);
    mprFree(serviceArgs);
}


static int startDispatcher(LPSERVICE_MAIN_FUNCTION svcMain)
{
    SC_HANDLE       mgr;
    char            name[80];
    ulong           len;

    if (!(mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        mprError(mpr, "Can't open service manager");
        return MPR_ERR_CANT_OPEN;
    }

    /*
        Is the service installed?
     */
    len = sizeof(name);
    if (GetServiceDisplayName(mgr, serviceName, name, &len) == 0) {
        CloseServiceHandle(mgr);
        return MPR_ERR_CANT_READ;
    }

    /*
        Register this service with the SCM. This call will block and consume the main thread if the service is 
        installed and the app was started by the SCM. If started manually, this routine will return 0.
     */
    svcTable[0].lpServiceProc = svcMain;
    if (StartServiceCtrlDispatcher(svcTable) == 0) {
        mprError(mpr, "Could not start the service control dispatcher");
        return MPR_ERR_CANT_INITIALIZE;
    }
    return 0;
}


/*
    Should be called first thing after the service entry point is called by
    the service manager
 */

static int registerService()
{
    svcHandle = RegisterServiceCtrlHandler(serviceName, serviceCallback);
    if (svcHandle == 0) {
        mprError(mpr, "Can't register handler: %x", GetLastError());
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
        serviceStopped = 1;
        SuspendThread(threadHandle);
        svcStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    case SERVICE_CONTROL_STOP:
        stopService(SERVICE_CONTROL_STOP);
        break;

    case SERVICE_CONTROL_CONTINUE:
        serviceStopped = 0;
        ResumeThread(threadHandle);
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


/*
    Install the window's service
 */ 
static int installService(char *homeDir, char *args)
{
    SC_HANDLE   svc, mgr;
    char        cmd[MPR_MAX_FNAME], key[MPR_MAX_FNAME];
    int         serviceType;

    GetModuleFileName(0, cmd, sizeof(cmd));

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprUserError(mpr, "Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }

    /*
        Install this app as a service
     */
    svc = OpenService(mgr, serviceName, SERVICE_ALL_ACCESS);
    if (svc == NULL) {
        serviceType = SERVICE_WIN32_OWN_PROCESS;
        if (createConsole) {
            serviceType |= SERVICE_INTERACTIVE_PROCESS;
        }
        svc = CreateService(mgr, serviceName, serviceTitle, SERVICE_ALL_ACCESS, serviceType, SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, cmd, NULL, NULL, "", NULL, NULL);
        if (! svc) {
            mprUserError(mpr, "Can't create service: 0x%x == %d", GetLastError(), GetLastError());
            CloseServiceHandle(mgr);
            return MPR_ERR_CANT_CREATE;
        }
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);

    /*
        Write a service description
     */
    mprSprintf(key, sizeof(key), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\" "Services\\%s", serviceName);

    if (mprWriteRegistry(mpr, key, "Description", SERVICE_DESCRIPTION) < 0) {
        return MPR_ERR_CANT_WRITE;
    }

    /*
        Write the home directory
     */
    if (homeDir == 0) {
        homeDir = mprGetPathParent(mpr, mprGetAppDir(mpr));
    }
    if (mprWriteRegistry(mpr, key, "HomeDir", homeDir) < 0) {
        return MPR_ERR_CANT_WRITE;
    }

    /*
        Write the service args
     */
    if (args && *args) {
        if (mprWriteRegistry(mpr, key, "Args", args) < 0) {
            return MPR_ERR_CANT_WRITE;
        }
    }
    return 0;
}


/*
    Remove the application service.
 */ 
static int removeService(int removeFromScmDb)
{
    SC_HANDLE   svc, mgr;

    exiting = 1;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError(mpr, "Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }
    svc = OpenService(mgr, serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        CloseServiceHandle(mgr);
        mprError(mpr, "Can't open service");
        return MPR_ERR_CANT_OPEN;
    }

    /*
        Gracefull shutdown 
     */
    shutdownAppweb(0);

    if (ControlService(svc, SERVICE_CONTROL_STOP, &svcStatus)) {
        mprSleep(mpr, 500);

        while (QueryServiceStatus(svc, &svcStatus)) {
            if (svcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                mprSleep(mpr, 250);
            } else {
                break;
            }
        }
        if (svcStatus.dwCurrentState != SERVICE_STOPPED) {
            mprError(mpr, "Can't stop service: %x", GetLastError());
        }
    }
    if (removeFromScmDb && !DeleteService(svc)) {
        mprError(mpr, "Can't delete service: %x", GetLastError());
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

    exiting = 0;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError(mpr, "Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }

    svc = OpenService(mgr, serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        mprError(mpr, "Can't open service");
        CloseServiceHandle(mgr);
        return MPR_ERR_CANT_OPEN;
    }

    rc = StartService(svc, 0, NULL);
    if (rc == 0) {
        mprError(mpr, "Can't start %s service: %d", serviceName, GetLastError());
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

    exiting = 1;
    serviceStopped = 1;

    /*
        Gracefull shutdown 
     */
    shutdownAppweb(10 * 1000);

    if (cmd == SERVICE_CONTROL_SHUTDOWN) {
        return 0;
    }

    /*
        Wakeup service event. This will make it exit.
     */
    SetEvent(serviceThreadEvent);

    svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    tellSCM(svcStatus.dwCurrentState, NO_ERROR, 1000);

    exitCode = 0;
    GetExitCodeThread(threadHandle, (ulong*) &exitCode);

    while (exitCode == STILL_ACTIVE) {
        GetExitCodeThread(threadHandle, (ulong*) &exitCode);
        mprSleep(mpr, 100);
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
    //  TOOD - serviceName should come from the command line program 
    serviceName = BLD_COMPANY "-" BLD_PRODUCT;
    serviceTitle = BLD_NAME;
    serviceStopped = 0;

    serviceProgram = mprStrcat(mpr, -1, mprGetAppDir(mpr), "/", BLD_PRODUCT, ".exe", NULL);
}

/*
    Initialize the applications's window
 */ 
static int initWindow()
{
    WNDCLASS    wc;
    int         rc;

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = (HINSTANCE) appInst;
    wc.hIcon            = NULL;
    wc.lpfnWndProc      = (WNDPROC) msgProc;
    wc.lpszMenuName     = wc.lpszClassName = appName;

    rc = RegisterClass(&wc);
    if (rc == 0) {
        mprError(mpr, "Can't register windows class");
        return -1;
    }
    appHwnd = CreateWindow(appName, appTitle, WS_OVERLAPPED, CW_USEDEFAULT, 0, 0, 0, NULL, NULL, appInst, NULL);
    if (! appHwnd) {
        mprError(mpr, "Can't create window");
        return -1;
    }
    return 0;
}


/*
    Windows message processing loop
 */
static long msgProc(HWND hwnd, uint msg, uint wp, long lp)
{
    switch (msg) {
    case WM_DESTROY:
        break;

    case WM_QUIT:
        shutdownAppweb(0);
        break;
    
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}


/*
    Default log output is just to the console
 */
static void logHandler(MprCtx ctx, int flags, int level, cchar *msg)
{
    if (flags & MPR_USER_MSG) {
        MessageBoxEx(NULL, msg, mprGetAppTitle(ctx), MB_OK, 0);
    }
    mprWriteToOsLog(mpr, msg, 0, 0);
}


/*
    Gracefull shutdown for Appweb
 */
static void shutdownAppweb(int timeout)
{
    HWND    hwnd;

    hwnd = FindWindow(BLD_PRODUCT, BLD_NAME);
    if (hwnd) {
        PostMessage(hwnd, WM_QUIT, 0, 0L);

        /*
            Wait for up to ten seconds while the service exits
         */
        while (timeout > 0 && hwnd) {
            mprSleep(mpr, 100);
            timeout -= 100;
            hwnd = FindWindow(BLD_PRODUCT, BLD_NAME);
            if (hwnd == 0) {
                return;
            }
        }
    }
    if (servicePid) {
        TerminateProcess((HANDLE) servicePid, 2);
        servicePid = 0;
    }
}


/* TODO - replace with mprGetNativePath */
static void mapPathDelim(char *s)
{
    while (*s) {
        if (*s == '\\') {
            *s = '/';
        }
        s++;
    }
}

#else
int APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *args, int junk2) {}
#endif /* WIN */

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
