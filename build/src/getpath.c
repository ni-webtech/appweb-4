/*
  	getpath.c - Convert from one file format to another
  
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
  
    Converts pathnames from Cygwin format to various windows formats and back.
    Usage:  getpath [-ar] files
   
    Switches:
        -a      canonicalized absolute path
        -c      canonicalized path
        -r      canonicalized relative path
 */
/********************************** Includes **********************************/

#include "posix.h"

#define MAXPATH 1024

/********************************** Forwards **********************************/

static char *absolutePath(char *path);
static char *canonPath(char *str);
static char *relativePath(char *path);

/************************************ Code ************************************/
/*
  	Main program
 */

int main(int argc, char *argv[])
{
	char	*result, *argp;
	int 	errflg, absolute, canon, relative, nextArg;

#if _WIN32
    _setmode(1, _O_BINARY);
#endif

	errflg = 0;
    canon = absolute = relative = 0;

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "-a") == 0) {
			absolute++;
        } else if (strcmp(argp, "-c") == 0) {
			canon++;
        } else if (strcmp(argp, "-r") == 0) {
			relative++;
        } else {
            errflg++;
        }
	}

    if (!absolute && !relative && !canon) {
        canon = 1;
    }
	if (errflg || (absolute + relative + canon) > 1) {
        fprintf(stderr, "Usage: getpath [-arc] files...\n");
        exit(2);
	}

	for (; nextArg < argc; ) {
        if (relative) {
            result = canonPath(relativePath(argv[nextArg]));
        } else if (absolute) {
            result = canonPath(absolutePath(argv[nextArg]));
        } else {
            result = canonPath(argv[nextArg]);
        }
        if (++nextArg < argc) {
            printf("%s ", result);
        } else {
            printf("%s", result);
        }
	}
    printf("\n");

	return 0;
}


/*
    Map delimiters - modifies argument
 */
static char *mapDelim(char *path)
{
    char    *s;

	for (s = path; *s; s++) {
		if (*s == '\\') {
			*s = '/';
        }
    }
    return path;
}


/*
    Trim trailing "/" characters - modifies arg
 */
static char *trim(char *path) 
{
    int     len;

    while (1) {
        len = (int) strlen(path);
        if (path[len - 1] == '/') {
            path[len - 1] = '\0';
        } else {
            return path;
        }
    }
    return path;
}


/*
    Tokenize a path into an array of path segments
 */
static char **getSegments(char *path, int *countp)
{
    char    *cp, **segments;
    int     count, i;

    /*
        Count path segments
     */
    for (count = 1, cp = path; *cp; cp++) {
        if (*cp == '/') {
            count++;
        }
    }

    /*
        Allocate space for each directory segment
     */
    segments = malloc((count + 1) * sizeof(char**));

    /*
        Tokenize each dir segment
     */
    cp = path;
    for (i = 0, cp = strtok(path, "/"); cp; cp = strtok(NULL, "/")) {
        if (*cp) {
            segments[i++] = cp;
        }
    }
    segments[i] = 0;
    *countp = i;

#if FOR_DEBUG
    for (i = 0; i < *countp; i++) {
        printf("Seg[%d] = %s\n", i, segments[i]);
    }
    printf("\n");
#endif

    return segments;
}


/*
    Return an absolute path variation of path
 */
static char *absolutePath(char *path)
{
    char    *str, cwd[MAXPATH], *result;
    int     len;

	if (path == NULL || *path == '\0') {
		return path;
	}

    /*
        Duplicate as we modify in-place and trim trailing "/"
     */
    str = mapDelim(strdup(path));
    if (*str == '/') {
        return str;
    }
#if _WIN32 || __CYGWIN__
    if (strlen(path) > 3 && isalpha((int) path[0]) && path[1] == ':' && path[2] == '/') {
        return str;
    }
#endif

    cwd[sizeof(cwd) - 1] = '\0';
    if (getcwd(cwd, sizeof(cwd) - 1) == 0) {
        fprintf(stderr, "Cant get working directory");
        exit(255);
    }
    len = (int) strlen(cwd);
    result = malloc(len + strlen(path) + 2);

    strcpy(result, cwd);
    result[len] = '/';
    strcpy(&result[len + 1], path);

    return result;
}


/*
    Return a relative path variation of path
 */
static char *relativePath(char *path)
{
    char    cwd[MAXPATH], **strSegments, **cwdSegments, *cp, *result;
    int     cwdCount, strCount, i, len, commonLevels, upLevels;

	if (path == NULL || *path == '\0') {
		return ".";
	}

    strSegments = getSegments(canonPath(absolutePath(path)), &strCount);

    cwd[sizeof(cwd) - 1] = '\0';
    if (getcwd(cwd, sizeof(cwd) - 1) == 0) {
        fprintf(stderr, "Cant get working directory");
        exit(255);
    }
    cwdSegments = getSegments(cwd, &cwdCount);

    len = min(cwdCount, strCount);
    for (i = 0; i < len; i++) {
        if (strcmp(strSegments[i], cwdSegments[i]) != 0) {
            break;
        }
    }
    commonLevels = i;

    upLevels = cwdCount - commonLevels;
    len = (upLevels * 3);
    for (; i < strCount; i++) {
        len += (int) strlen(strSegments[i]) + 1;
    }

    cp = result = malloc(len + 1);

    for (i = 0; i < upLevels; i++) {
        strcpy(cp, "../");
        cp += 3;
    }
    for (i = commonLevels; i < strCount; i++) {
        strcpy(cp, strSegments[i]);
        cp += strlen(strSegments[i]);
        if ((i + 1) < strCount) {
            *cp++ = '/';
        }
    }
    *cp = '\0';

    return result;
}


/*
  	Canonicalize a path. This removes "." and ".." segments and also redundant "/".
 */
static char *canonPath(char *path)
{
    char    *str, *cp, *result, **segments;
    int     count, i, from, to, len;

	if (path == NULL) {
		return NULL;
	}
    if (*path == '\0') {
        return "";
    }

    /*
        Duplicate as we modify in-place and trim trailing "/"
     */
    str = trim(mapDelim(strdup(path)));
    while (*str == '/') {
        str++;
    }

    /*
        Allocate space for the result (conservative)
     */
    result = malloc(strlen(str) + 1);

    segments = getSegments(str, &count);

    /*
        Remove "." and ".." segments
     */
    for (from = to = 0; from < count && segments[from]; from++) {
        if (strcmp(segments[from], ".") == 0) {
            continue;

        } else if (strcmp(segments[from], "..") == 0) {
            if (to > 0 && strcmp(segments[to - 1], "..") != 0) {
                to--;
            } else {
                to++;
            }

        } else {
            segments[to] = segments[from];
            to++;
        }
    }

    /*
        Catenate the result
     */
    cp = result;
    if (*path == '/') {
        *cp++ = '/';
    }
    for (i = 0; i < to; i++) {
        len = (int) strlen(segments[i]);
        strcpy(cp, segments[i]);
        cp += len;
        if ((i + 1) < to) {
            *cp++ = '/';
        }
    }
    *cp = '\0';

	return result;
}

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
  
    @end
 */
