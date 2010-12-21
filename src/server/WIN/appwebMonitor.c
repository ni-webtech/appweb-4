/**
 *  appwebMonitor.c  -- Windows Appweb Monitor program
 *
 *  The Appweb Monitor is a windows monitor program that interacts with the Appweb angel program.

 *  Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"
#include    "appwebMonitor.h"
#include    "resource.h"

#include    <winUser.h>

#if BLD_WIN_LIKE
/*********************************** Locals ***********************************/

static Mpr          *mpr;
static cchar        *appName;           /* Name of appweb */
static cchar        *appTitle;          /* Title of appweb */
static HINSTANCE    appInst;            /* Current application instance */
static HWND         appHwnd;            /* Application window handle */
static HMENU        subMenu;            /* As the name says */
static cchar        *serviceName;       /* Name of appweb service */
static cchar        *serviceTitle;      /* Title of appweb service */
static cchar        *serviceWindowName; /* Name of appweb service */
static cchar        *serviceWindowTitle;/* Title of appweb service */
static int          taskBarIcon;        /* Icon in the task bar */
static HMENU        monitorMenu;        /* As the name says */

#define APPWEB_ICON             "appwebMonitor.ico"
#define APPWEB_MONITOR_ID       0x100

#ifndef MIIM_STRING
#define MIIM_STRING             0x00000040
#define MIIM_BITMAP             0x00000080
#define MIIM_FTYPE              0x00000100
#endif

static SERVICE_STATUS           svcStatus;
static SERVICE_STATUS_HANDLE    svcHandle;
static SERVICE_TABLE_ENTRY      svcTable[] = {
    { "default",    0   },
    { 0,            0   }
};

/***************************** Forward Declarations ***************************/

static void     eventLoop();
static void     closeMonitorIcon();
static int      getAppwebPort();
static char     *getBrowserPath(int size);
static int      findInstance();
static int      initWindow(Mpr *mpr);
static void     logHandler(MprCtx ctx, int flags, int level, cchar *msg);
static long     msgProc(HWND hwnd, uint msg, uint wp, long lp);
static int      openMonitorIcon();
static uint     queryService();
static int      runBrowser(char *page);
static void     stopMonitor();
static int      startService();
static int      stopService();
static int      monitorEvent(HWND hwnd, WPARAM wp, LPARAM lp);
static void     updateMenu(int id, char *text, int enable, int check);

/*********************************** Code *************************************/

int APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *args, int junk2)
{
    char    **argv, *argp;
    int     argc, err, nextArg, manage, stop;

    mpr = mprCreate(0, NULL, NULL);

    err = 0;
    manage = 0;
    appInst = inst;
    stop = 0;
    serviceName = BLD_COMPANY "-" BLD_PRODUCT;
    serviceTitle = BLD_NAME;
    serviceWindowName = BLD_PRODUCT "Angel";
    serviceWindowTitle = BLD_NAME "Angel";

    mprSetAppName(mpr, BLD_PRODUCT "Monitor", BLD_NAME " Monitor", BLD_VERSION);
    appName = mprGetAppName(mpr);
    appTitle = mprGetAppTitle(mpr);

    mprSetLogHandler(mpr, logHandler, NULL);

    chdir(mprGetAppDir(mpr));

    /*
     *  Parse command line arguments
     */
    if (mprMakeArgv(mpr, "", args, &argc, &argv) < 0) {
        return FALSE;
    }
    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--manage") == 0) {
            manage++;

        } else if (strcmp(argp, "--stop") == 0) {
            stop++;

        } else {
            err++;
        }

        if (err) {
            mprUserError(mpr, "Bad command line: %s\n"
                "  Usage: %s [options]\n"
                "  Switches:\n"
                "    --manage             # Launch browser to manage",
                "    --stop               # Stop a running monitor",
                args, appName);
            return -1;
        }
    }

    if (stop) {
        stopMonitor();
        return 0;
    }

    if (findInstance(mpr)) {
        mprUserError(mpr, "Application %s is already active.", mprGetAppTitle(mpr));
        return MPR_ERR_BUSY;
    }

    /*
     *  Create the window
     */ 
    if (initWindow(mpr) < 0) {
        mprUserError(mpr, "Can't initialize application Window");
        return MPR_ERR_CANT_INITIALIZE;
    }

    if (manage) {
        /*
         *  Launch the browser 
         */
        runBrowser("/index.html");

    } else {

        if (openMonitorIcon() < 0) {
            mprUserError(mpr, "Can't open %s tray", appName);

        } else {
            eventLoop();
            closeMonitorIcon();
        }
    }

    // mprFree(mpr);

    return 0;
}


/*
 *  Sample main event loop. This demonstrates how to integrate Mpr with your
 *  applications event loop using select()
 */
void eventLoop()
{
    MSG     msg;

    /*
     *  If single threaded or if you desire control over the event loop, you 
     *  should code an event loop similar to that below:
     */
    while (!mprIsExiting(mpr)) {

        // SetTimer(appHwnd, 0, till, NULL);

        /*
         *  Socket events will be serviced in the msgProc
         */
        if (GetMessage(&msg, NULL, 0, 0) == 0) {
            /*  WM_QUIT received */
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


/*
 *  See if an instance of this product is already running
 */
static int findInstance(Mpr *mpr)
{
    HWND    hwnd;

    hwnd = FindWindow(mprGetAppName(mpr), mprGetAppTitle(mpr));

    if (hwnd) {
        if (IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }
        SetForegroundWindow(hwnd);
        return 1;
    }
    return 0;
}


/*
 *  Initialize the applications's window
 */ 
static int initWindow(Mpr *mpr)
{
    WNDCLASS    wc;
    int         rc;

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW+1);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = (HINSTANCE) appInst;
    wc.hIcon            = NULL;
    wc.lpfnWndProc      = (WNDPROC) msgProc;
    wc.lpszMenuName     = wc.lpszClassName = mprGetAppName(mpr);

    rc = RegisterClass(&wc);
    if (rc == 0) {
        mprError(mpr, "Can't register windows class");
        return -1;
    }

    appHwnd = CreateWindow(mprGetAppName(mpr), mprGetAppTitle(mpr), 
        WS_OVERLAPPED, CW_USEDEFAULT, 0, 0, 0, NULL, NULL, 0, NULL);
    if (! appHwnd) {
        mprError(mpr, "Can't create window");
        return -1;
    }
    if (taskBarIcon > 0) {
        ShowWindow(appHwnd, SW_MINIMIZE);
        UpdateWindow(appHwnd);
    }
    return 0;
}


static void stopMonitor()
{
    HWND    hwnd;

    hwnd = FindWindow(mprGetAppName(mpr), mprGetAppTitle(mpr));
    if (hwnd) {
        PostMessage(hwnd, WM_QUIT, 0, 0L);
    }
}


/*
 *  Windows message processing loop
 */ BOOL CALLBACK dialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message) {
    default:
        return FALSE;
    }
    return TRUE;
}


static long msgProc(HWND hwnd, uint msg, uint wp, long lp)
{
    char    buf[MPR_MAX_FNAME];

    switch (msg) {
    case WM_DESTROY:
    case WM_QUIT:
        mprSignalExit(mpr);
        break;
    
    case APPWEB_MONITOR_MESSAGE:
        return monitorEvent(hwnd, wp, lp);

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case MA_MENU_STATUS:
            break;

        case MA_MENU_DOC:
            runBrowser("/doc/index.html");
            break;

        case MA_MENU_MANAGE:
            runBrowser("/index.html");
            break;

        case MA_MENU_START:
            startService();
            break;

        case MA_MENU_STOP:
            stopService();
            break;

        case MA_MENU_ABOUT:
            /*
             *  Single-threaded users beware. This blocks !!
             */
            mprSprintf(buf, sizeof(buf), "%s %s-%s", BLD_NAME, BLD_VERSION, BLD_NUMBER);
            MessageBoxEx(NULL, buf, mprGetAppTitle(mpr), MB_OK, 0);
            break;

        case MA_MENU_EXIT:
            /* 
             *  FUTURE
             *
             *  h = CreateDialog(appInst, MAKEINTRESOURCE(IDD_EXIT), appHwnd, dialogProc);
             *  ShowWindow(h, SW_SHOW);
             */
            PostMessage(hwnd, WM_QUIT, 0, 0L);
            break;

        default:
            return DefWindowProc(hwnd, msg, wp, lp);
        }
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}


/*
 *  Can be called multiple times 
 */
static int openMonitorIcon()
{
    NOTIFYICONDATA  data;
    HICON           iconHandle;
    HBITMAP         go, stop;
    static int      doOnce = 0;
    int             rc;

    if (monitorMenu == NULL) {
        monitorMenu = LoadMenu(appInst, "monitorMenu");
        if (! monitorMenu) {
            mprError(mpr, "Can't locate monitorMenu");
            return MPR_ERR_CANT_OPEN;
        }
    }

    if (subMenu == NULL) {
        subMenu = GetSubMenu(monitorMenu, 0);
        go = LoadBitmap(appInst, MAKEINTRESOURCE(IDB_GO));
        rc = GetLastError();
        stop = LoadBitmap(appInst, MAKEINTRESOURCE(IDB_STOP));
        SetMenuItemBitmaps(subMenu, MA_MENU_STATUS, MF_BYCOMMAND, stop, go);
    }

    iconHandle = (HICON) LoadImage(appInst, APPWEB_ICON, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (iconHandle == 0) {
        mprError(mpr, "Can't load icon %s", APPWEB_ICON);
        return MPR_ERR_CANT_INITIALIZE;
    }

    data.uID = APPWEB_MONITOR_ID;
    data.hWnd = appHwnd;
    data.hIcon = iconHandle;
    data.cbSize = sizeof(NOTIFYICONDATA);
    data.uCallbackMessage = APPWEB_MONITOR_MESSAGE;
    data.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;

    mprStrcpy(data.szTip, sizeof(data.szTip), mprGetAppTitle(mpr));

    Shell_NotifyIcon(NIM_ADD, &data);

    if (iconHandle) {
        DestroyIcon(iconHandle);
    }
    return 0;
}


/*
 *  Can be caleld multiple times
 */
static void closeMonitorIcon()
{
    NOTIFYICONDATA  data;

    data.uID = APPWEB_MONITOR_ID;
    data.hWnd = appHwnd;
    data.cbSize = sizeof(NOTIFYICONDATA);
    Shell_NotifyIcon(NIM_DELETE, &data);
    if (monitorMenu) {
        DestroyMenu(monitorMenu);
        monitorMenu = NULL;
    }
    if (subMenu) {
        DestroyMenu(subMenu);
        subMenu = NULL;
    }
}


/*
 *  Respond to monitor icon events
 */
static int monitorEvent(HWND hwnd, WPARAM wp, LPARAM lp)
{
    RECT            windowRect;
    POINT           p, pos;
    HWND            h;
    char            textBuf[48];
    uint            msg;
    int             state;

    msg = (uint) lp;

    /*
     *  Show the menu on single right click
     */
    if (msg == WM_RBUTTONUP) {
        state = queryService();

        if (state < 0 || state & SERVICE_STOPPED) {
            mprSprintf(textBuf, sizeof(textBuf), "%s Stopped", BLD_NAME);
            updateMenu(MA_MENU_STATUS, textBuf, 0, -1);
            updateMenu(MA_MENU_START, 0, 1, 0);
            updateMenu(MA_MENU_STOP, 0, -1, 0);
        } else {
            mprSprintf(textBuf, sizeof(textBuf), "%s Running", BLD_NAME);
            updateMenu(MA_MENU_STATUS, textBuf, 0, 1);
            updateMenu(MA_MENU_START, 0, -1, 0);
            updateMenu(MA_MENU_STOP, 0, 1, 0);
        }

        /*
         *  Popup the context menu. Position near the bottom of the screen
         */
        h = GetDesktopWindow();
        GetWindowRect(h, &windowRect);
        GetCursorPos(&pos);

        p.x = pos.x;
        p.y = windowRect.bottom - 20;

        SetForegroundWindow(appHwnd);
        TrackPopupMenu(subMenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, appHwnd, NULL);
        /* Required Windows work-around */
        PostMessage(appHwnd, WM_NULL, 0, 0);

        return 0;
    }

    /*
     *  Launch the Appweb Monitor Manager on a double click
     */
    if (msg == WM_LBUTTONDBLCLK) {
        runBrowser("/index.html");
        return 0;
    }
    return 1;
}


/*
 *  Update menu string if text is non-null
 *  Update enable / disable if "enable" is non-zero. Positive will enable. Negative will disable.
 *  Update checked status if "check" is non-zero. Positive will enable, negative will disable.
 */
static void updateMenu(int id, char *text, int enable, int check)
{
    MENUITEMINFO    menuInfo;
    int             rc;

    if (text) {
        memset(&menuInfo, 0, sizeof(menuInfo));
        menuInfo.fMask = MIIM_STRING;
        menuInfo.cbSize = sizeof(menuInfo);
        menuInfo.fType = MFT_STRING;
        menuInfo.dwTypeData = NULL;
        menuInfo.fMask = MIIM_STRING;
        menuInfo.dwTypeData = text;
        menuInfo.cch = strlen(text) + 1;
        rc = SetMenuItemInfo(subMenu, id, FALSE, &menuInfo);
    }

    if (enable > 0) {
        rc = EnableMenuItem(subMenu, id, MF_BYCOMMAND | MF_ENABLED);
    } else if (enable < 0) {
        rc = EnableMenuItem(subMenu, id, MF_BYCOMMAND | MF_GRAYED);
    }

    if (check > 0) {
        rc = CheckMenuItem(subMenu, id, MF_BYCOMMAND | MF_CHECKED);
    } else if (check < 0) {
        rc = CheckMenuItem(subMenu, id, MF_BYCOMMAND | MF_UNCHECKED);
    }
    rc = DrawMenuBar(appHwnd);
}


/*
 *  Default log output is just to the console
 */
static void logHandler(MprCtx ctx, int flags, int level, cchar *msg)
{
    MessageBoxEx(NULL, msg, mprGetAppTitle(ctx), MB_OK, 0);
}


/*
 *  Gracefull shutdown for Appweb
 */
static void shutdownAppweb()
{
    HWND    hwnd;
    int     i;

    hwnd = FindWindow(serviceWindowName, serviceWindowTitle);
    if (hwnd) {

        PostMessage(hwnd, WM_QUIT, 0, 0L);

        /*
         *  Wait for up to ten seconds while the service exits
         */
        for (i = 0; hwnd && i < 100; i++) {
            mprSleep(mpr, 100);
            hwnd = FindWindow(serviceWindowName, serviceWindowTitle);
        }

    } else {
        mprError(mpr, "Can't find %s to kill", serviceWindowTitle);
        return;
    }
}


/*
 *  Get local port used by Appweb
 */
static int getAppwebPort()
{
    char    *path, portBuf[32];
    int     fd;

    path = mprJoinPath(mpr, mprGetAppDir(mpr), "../.port.log");

    if ((fd = open(path, O_RDONLY, 0666)) < 0) {
        mprError(mpr, "Could not read port file %s", path);
        return -1;
    }
    if (read(fd, portBuf, sizeof(portBuf)) < 0) {
        mprError(mpr, "Read from port file %s failed", path);
        close(fd);
        return 80;
    }
    close(fd);
    return atoi(portBuf);
}


/*
 *  Start the user's default browser
 */
static int runBrowser(char *page)
{
    PROCESS_INFORMATION procInfo;
    STARTUPINFO         startInfo;
    char                cmdBuf[MPR_MAX_STRING];
    char                *path;
    char                *pathArg;
    int                 port;

    port = getAppwebPort();
    if (port < 0) {
        mprError(mpr, "Can't get Appweb listening port");
        return -1;
    }

    path = getBrowserPath(MPR_MAX_STRING);
    if (path == 0) {
        mprError(mpr, "Can't get browser startup command");
        return -1;
    }

    pathArg = strstr(path, "\"%1\"");
    if (*page == '/') {
        page++;
    }

    if (pathArg == 0) {
        mprSprintf(cmdBuf, MPR_MAX_STRING, "%s http://localhost:%d/%s", path, port, page);

    } else {
        /*
         *  Patch out the "%1"
         */
        *pathArg = '\0';
        mprSprintf(cmdBuf, MPR_MAX_STRING, "%s \"http://localhost:%d/%s\"", path, port, page);
    }

    mprLog(mpr, 4, "Running %s\n", cmdBuf);

    memset(&startInfo, 0, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);

    if (! CreateProcess(0, cmdBuf, 0, 0, FALSE, 0, 0, 0, &startInfo, &procInfo)) {
        mprError(mpr, "Can't create process: %s, %d", cmdBuf, mprGetOsError());
        return -1;
    }
    CloseHandle(procInfo.hProcess);

    mprFree(path);
    return 0;
}


/*
 *  Return the path to run the user's default browser. Caller must free the return string.
 */ 
static char *getBrowserPath(int size)
{
    char    cmd[MPR_MAX_STRING];
    char    *type, *cp, *path;

    if (mprReadRegistry(mpr, &type, MPR_MAX_STRING, "HKEY_CLASSES_ROOT\\.htm", "") < 0) {
        return 0;
    }

    mprSprintf(cmd, MPR_MAX_STRING, "HKEY_CLASSES_ROOT\\%s\\shell\\open\\command", type);
    mprFree(type);

    if (mprReadRegistry(mpr, &path, size, cmd, "") < 0) {
        mprFree(cmd);
        return 0;
    }
    for (cp = path; *cp; cp++) {
        if (*cp == '\\') {
            *cp = '/';
        }
        *cp = tolower(*cp);
    }
    mprLog(mpr, 4, "Browser path: %s\n", path);
    return path;
}


/*
 *  Start the window's service
 */ 
static int startService()
{
    SC_HANDLE   svc, mgr;
    int         rc;

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
 *  Stop the service in the current process. 
 */ 
static int stopService()
{
    SC_HANDLE       svc, mgr;
    SERVICE_STATUS  status;
    int             rc;

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
    rc = ControlService(svc, SERVICE_CONTROL_STOP, &status);
    if (rc == 0) {
        mprError(mpr, "Can't stop %s service: %d", serviceName, GetLastError());
        return MPR_ERR_CANT_INITIALIZE;
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);
    return 0;
}


/*
 *  Query the service. Return the service state:
 *
 *      SERVICE_CONTINUE_PENDING
 *      SERVICE_PAUSE_PENDING
 *      SERVICE_PAUSED
 *      SERVICE_RUNNING
 *      SERVICE_START_PENDING
 *      SERVICE_STOP_PENDING
 *      SERVICE_STOPPED
 */ 
static uint queryService()
{
    SC_HANDLE       svc, mgr;
    SERVICE_STATUS  status;
    int             rc;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError(mpr, "Can't open service manager");
        return MPR_ERR_CANT_ACCESS;
    }
    svc = OpenService(mgr, serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        /* No warnings on error. Makes Monitor more useful */
        CloseServiceHandle(mgr);
        return MPR_ERR_CANT_OPEN;
    }
    rc = QueryServiceStatus(svc, &status);
    if (rc == 0) {
        mprError(mpr, "Can't start %s service: %d", serviceName, GetLastError());
        return 0;
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);
    return status.dwCurrentState;
}

#endif /* WIN */

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
 *  Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
