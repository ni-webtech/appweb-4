/**
    setConfig.c  -- Appweb set configuration program
  
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"

/***************************** Forward Declarations ***************************/

static char *replace(cchar *str, cchar *pattern, cchar *fmt, ...);

/*********************************** Code *************************************/
#if BIT_WIN_LIKE
int APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *command, int junk2) {
    char    **argv;
    cchar   *documents, *home, *logs, *port, *ssl, *argp, *path, *contents, *revised;
    cchar   *user, *group, *cache, *modules, *bak;
    int     argc, err, nextArg;
    static void logHandler(int flags, int level, cchar *msg);

    if (mprCreate(0, NULL, MPR_USER_EVENTS_THREAD) == NULL) {
        exit(1);
    }
    if ((argc = mprMakeArgv(command, &argv, MPR_ARGV_ARGS_ONLY)) < 0) {
        return FALSE;
    }
    mprSetLogHandler(logHandler);
#else
int main(int argc, char **argv) {
    cchar   *documents, *home, *logs, *port, *ssl, *argp, *path, *contents, *revised;
    cchar   *user, *group, *cache, *modules, *bak;
    int     err, nextArg;
    if (mprCreate(argc, argv, MPR_USER_EVENTS_THREAD) == NULL) {
        exit(1);
    }
#endif
    documents = home = port = ssl = logs = user = group = cache = modules = 0;
    for (err = 0, nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (smatch(argp, "--documents") && nextArg < argc) {
            documents = argv[++nextArg];
        } else if (smatch(argp, "--home") && nextArg < argc) {
            home = argv[++nextArg];
        } else if (smatch(argp, "--logs") && nextArg < argc) {
            logs = argv[++nextArg];
        } else if (smatch(argp, "--port") && nextArg < argc) {
            port = argv[++nextArg];
        } else if (smatch(argp, "--ssl") && nextArg < argc) {
            ssl = argv[++nextArg];
        } else if (smatch(argp, "--user") && nextArg < argc) {
            user = argv[++nextArg];
        } else if (smatch(argp, "--group") && nextArg < argc) {
            group = argv[++nextArg];
        } else if (smatch(argp, "--cache") && nextArg < argc) {
            cache = argv[++nextArg];
        } else if (smatch(argp, "--modules") && nextArg < argc) {
            modules = argv[++nextArg];
        } else {
            err++;
        }
    }
    if (nextArg != (argc - 1)) {
        err++;
    }
    if (err) {
#if BIT_WIN_LIKE
        mprUserError("Bad command line:");
#else
        mprUserError("Bad command line:\n"
            "  Usage: pathConfig [options]\n"
            "  Switches:\n"
            "    --cache dir          # Cache dir"
            "    --documents dir      # Static documents directory"
            "    --group groupname    # Group name"
            "    --home dir           # Server home directory"
            "    --logs dir           # Log directory"
            "    --modules dir        # moduels dir"
            "    --port number        # HTTP port number"
            "    --user username      # User name");
#endif
        return 1;
    }
    path = argv[nextArg++];

    if ((contents = mprReadPathContents(path, NULL)) == 0) {
        mprUserError("Can't read %s", path);
        return 1;
    }
	if (port) {
	    contents = replace(contents, "Listen 80", "Listen %s", port);
	}
	if (ssl) {
	    contents = replace(contents, "443", ssl);
	}
    if (documents) {
        contents = replace(contents, "DocumentRoot", "DocumentRoot \"%s\"", documents);
    }
    if (home) {
        contents = replace(contents, "ServerRoot", "ServerRoot \"%s\"", home);
    }
    if (logs) {
        contents = replace(contents, "ErrorLog", "ErrorLog \"%s\"", mprJoinPath(logs, "error.log"));
        contents = replace(contents, "AccessLog", "AccessLog \"%s\"", mprJoinPath(logs, "access.log"));
    }
    if (user) {
        contents = replace(contents, "User", "User %s", user);
    }
    if (group) {
        contents = replace(contents, "Group", "Group %s", group);
    }
    if (cache) {
        contents = replace(contents, "EspDir cache", "EspDir cache \"%s\"", cache);
    }
    if (modules) {
        contents = replace(contents, "LoadModulePath", "LoadModulePath \"%s\"", modules);
    }
    revised = mprGetTempPath(mprGetPathDir(path));
    if (mprWritePathContents(revised, contents, -1, 0644) < 0) {
        mprUserError("Can't write %s", revised);
    }
	bak = sfmt("%s.bak", path);
	mprDeletePath(bak);
	if (rename(path, bak) < 0) {
        mprUserError("Can't save %s to %s: 0x%x", path, bak, mprGetError());
	}
	mprDeletePath(path);
    if (rename(revised, path) < 0) {
        mprUserError("Can't rename %s to %s: 0x%x", revised, path, mprGetError());
		rename(bak, path);
    }
    return 0;
}


/*
    This replace will replace the given pattern and the next word on the same line with the given replacement.
 */
static char *replace(cchar *str, cchar *pattern, cchar *fmt, ...)
{
    va_list     args;
    MprBuf      *buf;
    cchar       *s, *replacement;
    ssize       plen;

    va_start(args, fmt);
    replacement = sfmtv(fmt, args);
    buf = mprCreateBuf(-1, -1);
    plen = slen(pattern);
    for (s = str; *s; s++) {
        if (*s == *pattern && sncmp(s, pattern, plen) == 0) {
            mprPutStringToBuf(buf, replacement);
            for (s += plen; *s && isspace((uchar) *s) && *s != '\n'; s++) ;
            for (; *s && !isspace((uchar) *s) && *s != '\n' && *s != '>'; s++) ;
         
        }
#if BIT_WIN_LIKE
        if (*s == '\n' && s[-1] != '\r') {
            mprPutCharToBuf(buf, '\r');
        }
#endif
        mprPutCharToBuf(buf, *s);
    }
    va_end(args);
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}



#if BIT_WIN_LIKE
/*
    Default log output is just to the console
 */
static void logHandler(int flags, int level, cchar *msg)
{
    if (flags & MPR_USER_MSG) {
        MessageBoxEx(NULL, msg, mprGetAppTitle(), MB_OK, 0);
    }
    mprWriteToOsLog(msg, 0, 0);
    mprPrintfError("%s\n", msg);
}
#endif


/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
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
