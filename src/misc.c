/*
    misc.c -- Bits and pieces.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "appweb.h"

/***************************** Forward Declarations ***************************/

#if BLD_UNIX_LIKE
static int allDigits(cchar *s);
#endif

/************************************ Code ************************************/

void maGetUserGroup(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;
    struct group    *gp;

    appweb->uid = getuid();
    if ((pp = getpwuid(appweb->uid)) == 0) {
        mprError("Can't read user credentials: %d. Check your /etc/passwd file.", appweb->uid);
    } else {
        appweb->user = sclone(pp->pw_name);
    }
    appweb->gid = getgid();
    if ((gp = getgrgid(appweb->gid)) == 0) {
        mprError("Can't read group credentials: %d. Check your /etc/group file", appweb->gid);
    } else {
        appweb->group = sclone(gp->gr_name);
    }
#else
    appweb->uid = appweb->gid = -1;
#endif
}


int maSetHttpUser(MaAppweb *appweb, cchar *newUser)
{
#if BLD_UNIX_LIKE
    struct passwd   *pp;

    if (allDigits(newUser)) {
        appweb->uid = atoi(newUser);
        if ((pp = getpwuid(appweb->uid)) == 0) {
            mprError("Bad user id: %d", appweb->uid);
            return MPR_ERR_CANT_ACCESS;
        }
        newUser = pp->pw_name;

    } else {
        if ((pp = getpwnam(newUser)) == 0) {
            mprError("Bad user name: %s", newUser);
            return MPR_ERR_CANT_ACCESS;
        }
        appweb->uid = pp->pw_uid;
    }
    appweb->userChanged = 1;
#endif
    appweb->user = sclone(newUser);
    return 0;
}


int maSetHttpGroup(MaAppweb *appweb, cchar *newGroup)
{
#if BLD_UNIX_LIKE
    struct group    *gp;

    if (allDigits(newGroup)) {
        appweb->gid = atoi(newGroup);
        if ((gp = getgrgid(appweb->gid)) == 0) {
            mprError("Bad group id: %d", appweb->gid);
            return MPR_ERR_CANT_ACCESS;
        }
        newGroup = gp->gr_name;

    } else {
        if ((gp = getgrnam(newGroup)) == 0) {
            mprError("Bad group name: %s", newGroup);
            return MPR_ERR_CANT_ACCESS;
        }
        appweb->gid = gp->gr_gid;
    }
    appweb->groupChanged = 1;
#endif
    appweb->group = sclone(newGroup);
    return 0;
}


int maApplyChangedUser(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    if (appweb->userChanged && appweb->uid >= 0) {
        if ((setuid(appweb->uid)) != 0) {
            mprError("Can't change user to: %s: %d\n"
                "WARNING: This is a major security exposure", appweb->user, appweb->uid);
#if LINUX && PR_SET_DUMPABLE
        } else {
            prctl(PR_SET_DUMPABLE, 1);
#endif
        }
    }
#endif
    return 0;
}


int maApplyChangedGroup(MaAppweb *appweb)
{
#if BLD_UNIX_LIKE
    if (appweb->groupChanged && appweb->gid >= 0) {
        if (setgid(appweb->gid) != 0) {
            mprError("Can't change group to %s: %d\n"
                "WARNING: This is a major security exposure", appweb->group, appweb->gid);
#if LINUX && PR_SET_DUMPABLE
        } else {
            prctl(PR_SET_DUMPABLE, 1);
#endif
        }
    }
#endif
    return 0;
}


/*
    Load a module. Returns 0 if the modules is successfully loaded either statically or dynamically.
 */
int maLoadModule(MaAppweb *appweb, cchar *name, cchar *libname)
{
    MprModule   *module;
    char        entryPoint[MPR_MAX_FNAME];
    char        *path;

    if (strcmp(name, "authFilter") == 0 || strcmp(name, "rangeFilter") == 0 || strcmp(name, "uploadFilter") == 0 ||
            strcmp(name, "fileHandler") == 0 || strcmp(name, "egiHandler") == 0 || strcmp(name, "dirHandler") == 0) {
        mprLog(1, "The %s module is now builtin. No need to use LoadModule", name);
        return 0;
    }
    if (libname == 0) {
        path = sjoin("mod_", name, BLD_SHOBJ, NULL);
    } else {
        path = sclone(libname);
    }
    module = mprLookupModule(path);
    if (module) {
        mprLog(MPR_CONFIG, "Activating module (Builtin) %s", name);
        return 0;
    }
    mprSprintf(entryPoint, sizeof(entryPoint), "ma%sInit", name);
    entryPoint[2] = toupper((int) entryPoint[2]);

    if (mprLoadModule(path, entryPoint, MPR->httpService) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    mprLog(MPR_CONFIG, "Activating module (Loadable) %s", name);
    return 0;
}



#if BLD_UNIX_LIKE
static int allDigits(cchar *s)
{
    return strspn(s, "1234567890") == strlen(s);
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
