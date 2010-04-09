/*
 *  posix.h -- Include some standard headers and make windows a bit more unix like
 *  
 *  Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/******************************************************************************/

#ifndef _h_POSIX
#define _h_POSIX 1

/*
 *  Suppress MS VS warnings
 */
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include    <ctype.h>
#include    <signal.h>
#include    <sys/stat.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>

#if _WIN32
    #include    <direct.h>
    #include    <fcntl.h>
    #include    <io.h>
    #include    <windows.h>
	
	#define access  _access
	#define close   _close
	#define fileno  _fileno
	#define fstat   _fstat
	#define getpid  _getpid
	#define open    _open
	#define putenv  _putenv
	#define read    _read
	#define stat    _stat
	#define umask   _umask
	#define unlink  _unlink
	#define write   _write
	#define strdup  _strdup
	#define lseek   _lseek
	#define getcwd  _getcwd
	#define chdir   _chdir
	#define strnset _strnset
	#define chmod   _chmod
	
	#define mkdir(a,b)  _mkdir(a)
	#define rmdir(a)    _rmdir(a)
	
	#define     R_OK        4
	#define     W_OK        2
	#define     MPR_TEXT    "t"

    #ifndef R_OK
    #define R_OK        4
    #endif

    extern void     srand48(long);
    extern long     lrand48(void);
    extern long     ulimit(int, ...);
    extern long     nap(long);
    extern int      getuid(void);
    extern int      geteuid(void);

#else
    #include    <libgen.h>
    #include    <unistd.h>
	#define     MPR_TEXT    ""
    #include    <unistd.h>
    #define     min(a,b) ((a) < (b) ? (a) : (b))
#endif /* _WIN32 */

#endif /* _h_POSIX */
/*
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
 */
