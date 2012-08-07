/*
    mpr.h -- Header for the Multithreaded Portable Runtime (MPR).

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/**
    @file mpr.h
    The Multithreaded Portable Runtime (MPR) is a portable runtime library for embedded applications.
    \n\n
    The MPR provides management for logging, error handling, events, files, http, memory, ssl, sockets, strings, 
    xml parsing, and date/time functions. It also provides a foundation of safe routines for secure programming, 
    that help to prevent buffer overflows and other security threats. The MPR is a library and a C API that can 
    be used in both C and C++ programs.
    \n\n
    The MPR uses a set extended typedefs for common types. These include: bool, cchar, cvoid, uchar, short, ushort, 
    int, uint, long, ulong, int32, uint32, int64, uint64, float, and double. The cchar type is a const char, cvoid is 
    const void. Several types have "u" prefixes to denote unsigned qualifiers.
    \n\n
    The MPR includes a memory allocator and generational garbage collector. The allocator is a fast, immediate 
    coalescing allocator that will return memory back to the O/S if not required. It is optimized for frequent 
    allocations of small blocks (< 4K) and uses a scheme of free queues for fast allocation. 
    \n\n
    Not all of these APIs are thread-safe. 
 */

#ifndef _h_MPR
#define _h_MPR 1

/********************************** Includes **********************************/

#include "bit.h"

/******************************* Default Features *****************************/

#ifndef BIT_DEBUG
    #define BIT_DEBUG 0
#endif
#ifndef BIT_ASSERT
    #if BIT_DEBUG
        #define BIT_ASSERT 1
    #else
        #define BIT_ASSERT 0
    #endif
#endif
#ifndef BIT_FLOAT
    #define BIT_FLOAT 1
#endif
#ifndef BIT_ROM
    #define BIT_ROM 0
#endif
#ifndef BIT_TUNE
    #define BIT_TUNE MPR_TUNE_SIZE
#endif

/********************************* CPU Families *******************************/
/*
    CPU Architectures
    MOB - change to BIT_CPU
 */
#define MPR_CPU_UNKNOWN     0
#define MPR_CPU_ARM         1           /* Arm */
#define MPR_CPU_ITANIUM     2           /* Intel Itanium */
#define MPR_CPU_X86         3           /* X86 */
#define MPR_CPU_X64         4           /* AMD64 or EMT64 */
#define MPR_CPU_MIPS        5           /* Mips */
#define MPR_CPU_PPC         6           /* Power PC */
#define MPR_CPU_SPARC       7           /* Sparc */

/*
    Use compiler definitions to determine the CPU
 */
#if defined(__alpha__)
    #define BIT_CPU "ALPHA"
    #define BIT_CPU_ARCH MPR_CPU_ALPHA
#elif defined(__arm__)
    #define BIT_CPU "ARM"
    #define BIT_CPU_ARCH MPR_CPU_ARM
#elif defined(__x86_64__) || defined(_M_AMD64)
    #define BIT_CPU "x64"
    #define BIT_CPU_ARCH MPR_CPU_X64
#elif defined(__i386__) || defined(__i486__) || defined(__i585__) || defined(__i686__) || defined(_M_IX86)
    #define BIT_CPU "x86"
    #define BIT_CPU_ARCH MPR_CPU_X86
#elif defined(_M_IA64)
    #define BIT_CPU "IA64"
    #define BIT_CPU_ARCH MPR_CPU_ITANIUM
#elif defined(__mips__)
    #define BIT_CPU "MIPS"
    #define BIT_CPU_ARCH MPR_CPU_SPARC
#elif defined(__ppc__) || defined(__powerpc__) || defined(__ppc64__)
    #define BIT_CPU "PPC"
    #define BIT_CPU_ARCH MPR_CPU_PPC
#elif defined(__sparc__)
    #define BIT_CPU "SPARC"
    #define BIT_CPU_ARCH MPR_CPU_SPARC
#endif

/*
    Operating system defines. Use compiler standard defintions to sleuth. 
    Works for all except VxWorks which does not define any special symbol.
    NOTE: Support for SCOV Unix, LynxOS and UnixWare is deprecated.
 */
#if defined(__APPLE__)
    #define BIT_OS "macosx"
    #define MACOSX 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(__linux__)
    #define BIT_OS "linux"
    #define LINUX 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(__FreeBSD__)
    #define BIT_OS "freebsd"
    #define FREEBSD 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(_WIN32)
    #define BIT_OS "windows"
    #define WINDOWS 1
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 1
#elif defined(__OS2__)
    #define BIT_OS "os2"
    #define OS2 0
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 0
#elif defined(MSDOS) || defined(__DOS__)
    #define BIT_OS "msdos"
    #define WINDOWS 0
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 0
#elif defined(__NETWARE_386__)
    #define BIT_OS "netware"
    #define NETWARE 0
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 0
#elif defined(__bsdi__)
    #define BIT_OS "bsdi"
    #define BSDI 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(__NetBSD__)
    #define BIT_OS "netbsd"
    #define NETBSD 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(__QNX__)
    #define BIT_OS "qnx"
    #define QNX 0
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 0
#elif defined(__hpux)
    #define BIT_OS "hpux"
    #define HPUX 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(_AIX)
    #define BIT_OS "aix"
    #define AIX 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(__CYGWIN__)
    #define BIT_OS "cygwin"
    #define CYGWIN 1
    #define BIT_UNIX_LIKE 1
    #define BIT_WIN_LIKE 0
#elif defined(__VMS)
    #define BIT_OS "vms"
    #define VMS 1
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 0
#elif defined(VXWORKS)
    /* VxWorks does not have a pre-defined symbol */
    #define BIT_OS "vxworks"
    #define BIT_UNIX_LIKE 0
    #define BIT_WIN_LIKE 0
#endif

/********************************* O/S Includes *******************************/

#if __WORDSIZE == 64 || __amd64 || __x86_64 || __x86_64__ || _WIN64
    #define BIT_64 1
    #define BIT_WORDSIZE 64
#else
    #define BIT_WORDSIZE 32
#endif

/*
    Out-of-order definitions and includes. Order really matters in this section
 */
#if WINDOWS
    #undef      _CRT_SECURE_NO_DEPRECATE
    #define     _CRT_SECURE_NO_DEPRECATE 1
    #undef      _CRT_SECURE_NO_WARNINGS
    #define     _CRT_SECURE_NO_WARNINGS 1
    #ifndef     _WIN32_WINNT
        #define _WIN32_WINNT 0x501
    #endif
#endif

/*
    Use GNU extensions for:
        RTLD_DEFAULT for dlsym()
 */
#if LINUX
    #define _GNU_SOURCE 1
    #if !BIT_64
        #define _LARGEFILE64_SOURCE 1
        #define _FILE_OFFSET_BITS 64
    #endif
#endif

#if VXWORKS
    #ifndef _VSB_CONFIG_FILE
        #define _VSB_CONFIG_FILE "vsbConfig.h"
    #endif
    #include    <vxWorks.h>
    #define     HAS_USHORT 1
#endif

#if MACSOX
    #define     HAS_USHORT 1
    #define     HAS_UINT 1
#endif

#if BIT_WIN_LIKE
    #include    <winsock2.h>
    #include    <windows.h>
    #include    <winbase.h>
    #include    <winuser.h>
    #include    <shlobj.h>
    #include    <shellapi.h>
    #include    <wincrypt.h>
#endif

#if WINDOWS
    #include    <ws2tcpip.h>
    #include    <conio.h>
    #include    <process.h>
    #include    <windows.h>
    #include    <shlobj.h>
    #if BIT_DEBUG
        #include <crtdbg.h>
    #endif
#endif
#undef     _WIN32_WINNT

/*
    Includes in alphabetic order
 */
#if CYGWIN
    #include    <arpa/inet.h>
#endif

    #include    <ctype.h>

#if BIT_WIN_LIKE
    #include    <direct.h>
#else
    #include    <dirent.h>
#if !VXWORKS
    #include    <dlfcn.h>
#endif
#endif

    #include    <fcntl.h>
    #include    <errno.h>

#if BIT_FLOAT
    #include    <float.h>
    #define __USE_ISOC99 1
    #include    <math.h>
#endif

#if BIT_UNIX_LIKE
    #include    <grp.h> 
#endif

#if BIT_WIN_LIKE
    #include    <io.h>
#endif

#if MACOSX
    #include    <libgen.h>
#endif

    #include    <limits.h>

#if BIT_UNIX_LIKE || VXWORKS
    #include    <netdb.h>
    #include    <net/if.h>
    #include    <netinet/in.h>
    #include    <netinet/tcp.h>
    #include    <netinet/ip.h>
#endif

#if BIT_UNIX_LIKE
    #include    <pthread.h> 
    #include    <pwd.h> 
#if !CYGWIN
    #include    <resolv.h>
#endif
#endif

    #include    <setjmp.h>
    #include    <signal.h>
    #include    <stdarg.h>

#if BIT_UNIX_LIKE
    #include    <stdint.h>
#endif

    #include    <stdio.h>
    #include    <stdlib.h>
    #include    <string.h>

#if BIT_UNIX_LIKE
    #include    <syslog.h>
#endif

#if LINUX
    #include    <sys/epoll.h>
#endif

#if BIT_UNIX_LIKE
    #include    <sys/ioctl.h>
    #include    <sys/mman.h>
    #include    <sys/poll.h>
#endif

    #include    <sys/stat.h>

#if LINUX
    #include    <sys/prctl.h>
#endif

    #include    <sys/types.h>

#if BIT_UNIX_LIKE
    #include    <sys/resource.h>
    #include    <sys/sem.h>
    #include    <sys/socket.h>
    #include    <sys/select.h>
    #include    <sys/time.h>
    #include    <sys/times.h>
    #include    <sys/utsname.h>
    #include    <sys/uio.h>
    #include    <sys/wait.h>
#endif

    #include    <time.h>

#if BIT_UNIX_LIKE
    #include    <unistd.h>
#endif

#if !VXWORKS
    #include    <wchar.h>
#endif

/*
    Extra includes per O/S
 */
#if CYGWIN
    #include    "w32api/windows.h"
    #include    "sys/cygwin.h"
#endif

#if LINUX && !__UCLIBC__
    #include    <sys/sendfile.h>
#endif

#if MACOSX
    #include    <stdbool.h>
    #include    <mach-o/dyld.h>
    #include    <mach-o/dyld.h>
    #include    <mach/mach_init.h>
    #include    <mach/task.h>
    #include    <sys/sysctl.h>
    #include    <libkern/OSAtomic.h>
#endif

#if VXWORKS
    #include    <vxWorks.h>
    #include    <envLib.h>
    #include    <iosLib.h>
    #include    <loadLib.h>
    #include    <selectLib.h>
    #include    <sockLib.h>
    #include    <inetLib.h>
    #include    <ioLib.h>
    #include    <pipeDrv.h>
    #include    <hostLib.h>
    #include    <sysSymTbl.h>
    #include    <sys/fcntlcom.h>
    #include    <tickLib.h>
    #include    <taskHookLib.h>
    #include    <unldLib.h>
#if _WRS_VXWORKS_MAJOR >= 6
    #include    <wait.h>
#endif
#if _WRS_VXWORKS_MAJOR > 6 || (_WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR >= 8)
    #include    <symSync.h>
    #include    <vxAtomicLib.h>
#endif
#endif

/************************************** Defines *******************************/
/*
    Standard types
 */
#ifndef HAS_BOOL
    #ifndef __cplusplus
        #if !MACOSX
            #define HAS_BOOL 1
            /**
                Boolean data type.
             */
            typedef char bool;
        #endif
    #endif
#endif

#ifndef HAS_UCHAR
    #define HAS_UCHAR 1
    /**
        Unsigned char data type.
     */
    typedef unsigned char uchar;
#endif

#ifndef HAS_SCHAR
    #define HAS_SCHAR 1
    /**
        Signed char data type.
     */
    typedef signed char schar;
#endif

#ifndef HAS_CCHAR
    #define HAS_CCHAR 1
    /**
        Constant char data type.
     */
    typedef const char cchar;
#endif

#ifndef HAS_CUCHAR
    #define HAS_CUCHAR 1
    /**
        Unsigned char data type.
     */
    typedef const unsigned char cuchar;
#endif

#ifndef HAS_USHORT
    #define HAS_USHORT 1
    /**
        Unsigned short data type.
     */
    typedef unsigned short ushort;
#endif

#ifndef HAS_CUSHORT
    #define HAS_CUSHORT 1
    /**
        Constant unsigned short data type.
     */
    typedef const unsigned short cushort;
#endif

#ifndef HAS_CVOID
    #define HAS_CVOID 1
    /**
        Constant void data type.
     */
    typedef const void cvoid;
#endif

#ifndef HAS_INT32
    #define HAS_INT32 1
    /**
        Integer 32 bits data type.
     */
    typedef int int32;
#endif

#ifndef HAS_UINT32
    #define HAS_UINT32 1
    /**
        Unsigned integer 32 bits data type.
     */
    typedef unsigned int uint32;
#endif

#ifndef HAS_UINT
    #define HAS_UINT 1
    /**
        Unsigned integer (machine dependent bit size) data type.
     */
    typedef unsigned int uint;
#endif

#ifndef HAS_ULONG
    #define HAS_ULONG 1
    /**
        Unsigned long (machine dependent bit size) data type.
     */
    typedef unsigned long ulong;
#endif

#ifndef HAS_SSIZE
    #define HAS_SSIZE 1
    #if BIT_UNIX_LIKE || VXWORKS || DOXYGEN
        /**
            Signed integer size field large enough to hold a pointer offset.
         */
        typedef ssize_t ssize;
    #else
        typedef SSIZE_T ssize;
    #endif
#endif

#ifdef __USE_FILE_OFFSET64
    #define BIT_HAS_OFF64 1
#else
    #define BIT_HAS_OFF64 0
#endif

/*
    Windows uses uint for write/read counts (Ugh!)
 */
#if BIT_WIN_LIKE
    typedef uint wsize;
#else
    typedef ssize wsize;
#endif

#ifndef HAS_INT64
    #if BIT_UNIX_LIKE
        __extension__ typedef long long int int64;
    #elif VXWORKS || DOXYGEN
        /**
            Integer 64 bit data type.
         */
        typedef long long int int64;
    #elif BIT_WIN_LIKE
        typedef __int64 int64;
    #else
        typedef long long int int64;
    #endif
#endif

#ifndef HAS_UINT64
    #if BIT_UNIX_LIKE
        __extension__ typedef unsigned long long int uint64;
    #elif VXWORKS || DOXYGEN
        /**
            Unsigned integer 64 bit data type.
         */
        typedef unsigned long long int uint64;
    #elif BIT_WIN_LIKE
        typedef unsigned __int64 uint64;
    #else
        typedef unsigned long long int uint64;
    #endif
#endif

/**
    Signed file offset data type. Supports large files greater than 4GB in size on all systems.
 */
typedef int64 MprOff;

/*
    Socklen_t
 */
#if DOXYGEN
    typedef int MprSocklen;
#elif VXWORKS
    typedef int MprSocklen;
#else
    typedef socklen_t MprSocklen;
#endif

/**
    Date and Time Service
    @stability Evolving
    @see MprTime mprCompareTime mprCreateTimeService mprDecodeLocalTime mprDecodeUniversalTime mprFormatLocalTime 
        mprFormatTm mprGetDate mprGetElapsedTime mprGetRemainingTime mprGetTicks mprGetTimeZoneOffset mprMakeTime 
        mprMakeUniversalTime mprParseTime 
    @defgroup MprTime MprTime
 */
typedef int64 MprTime;

#ifndef BITSPERBYTE
    #define BITSPERBYTE     (8 * sizeof(char))
#endif

#ifndef BITS
    #define BITS(type)      (BITSPERBYTE * (int) sizeof(type))
#endif

#if BIT_FLOAT
    #ifndef MAXFLOAT
        #if BIT_WIN_LIKE
            #define MAXFLOAT        DBL_MAX
        #else
            #define MAXFLOAT        FLT_MAX
        #endif
    #endif
    #if VXWORKS
        #define isnan(n)  ((n) != (n))
        #define isnanf(n) ((n) != (n))
        #define isinf(n)  ((n) == (1.0 / 0.0) || (n) == (-1.0 / 0.0))
        #define isinff(n) ((n) == (1.0 / 0.0) || (n) == (-1.0 / 0.0))
    #endif
    #if BIT_WIN_LIKE
        #define isNan(f) (_isnan(f))
    #elif VXWORKS || MACOSX || LINUX
        #define isNan(f) (isnan(f))
    #else
        #define isNan(f) (fpclassify(f) == FP_NAN)
    #endif
#endif


#ifndef MAXINT
#if INT_MAX
    #define MAXINT      INT_MAX
#else
    #define MAXINT      0x7fffffff
#endif
#endif

#ifndef MAXINT64
    #define MAXINT64    INT64(0x7fffffffffffffff)
#endif

#if SIZE_T_MAX
    #define MAXSIZE     SIZE_T_MAX
#elif BIT_64
    #define MAXSIZE     INT64(0xffffffffffffffff)
#else
    #define MAXSIZE     MAXINT
#endif

#if SSIZE_T_MAX
    #define MAXSSIZE     SSIZE_T_MAX
#elif BIT_64
    #define MAXSSIZE     INT64(0x7fffffffffffffff)
#else
    #define MAXSSIZE     MAXINT
#endif

#if OFF_T_MAX
    #define MAXOFF       OFF_T_MAX
#else
    #define MAXOFF       INT64(0x7fffffffffffffff)
#endif

/*
    Word size and conversions between integer and pointer.
 */
#if BIT_64
    #define ITOP(i)     ((void*) ((int64) i))
    #define PTOI(i)     ((int) ((int64) i))
    #define LTOP(i)     ((void*) ((int64) i))
    #define PTOL(i)     ((int64) i)
#else
    #define ITOP(i)     ((void*) ((int) i))
    #define PTOI(i)     ((int) i)
    #define LTOP(i)     ((void*) ((int) i))
    #define PTOL(i)     ((int64) (int) i)
#endif

#if BIT_WIN_LIKE
    #define INT64(x)    (x##i64)
    #define UINT64(x)   (x##Ui64)
    #define MPR_EXPORT  __declspec(dllexport)
#else
    #define INT64(x)    (x##LL)
    #define UINT64(x)   (x##ULL)
    #define MPR_EXPORT 
#endif

#ifndef max
    #define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef PRINTF_ATTRIBUTE
    #if (__GNUC__ >= 3) && !DOXYGEN && BIT_DEBUG && UNUSED && KEEP
        /** 
            Use gcc attribute to check printf fns.  a1 is the 1-based index of the parameter containing the format, 
            and a2 the index of the first argument. Note that some gcc 2.x versions don't handle this properly 
         */     
        #define PRINTF_ATTRIBUTE(a1, a2) __attribute__ ((format (__printf__, a1, a2)))
    #else
        #define PRINTF_ATTRIBUTE(a1, a2)
    #endif
#endif

/*
    Optimize expression evaluation code depending if the value is likely or not
 */
#undef likely
#undef unlikely
#if (__GNUC__ >= 3)
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif

#if !__UCLIBC__ && !CYGWIN && __USE_XOPEN2K
    #define BIT_HAS_SPINLOCK    1
#endif

#if BIT_HAS_DOUBLE_BRACES
    #define  NULL_INIT    {{0}}
#else
    #define  NULL_INIT    {0}
#endif

#ifndef R_OK
    #define R_OK    4
    #define W_OK    2
#if BIT_WIN_LIKE
    #define X_OK    R_OK
#else
    #define X_OK    1
#endif
    #define F_OK    0
#endif

#if MACSOX
    #define LD_LIBRARY_PATH "DYLD_LIBRARY_PATH"
#else
    #define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#endif

#if VXWORKS
/*
    Old VxWorks can't do array[]
 */
#define MPR_FLEX 0
#else
#define MPR_FLEX
#endif

/*********************************** Fixups ***********************************/

#if BIT_UNIX_LIKE || VXWORKS
    #define MPR_TEXT        ""
    #define MPR_BINARY      ""
#endif

#if BIT_UNIX_LIKE
    #define closesocket(x)  close(x)
    #define SOCKET_ERROR    -1
    #ifndef PTHREAD_MUTEX_RECURSIVE_NP
        #define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
    #endif
    #ifndef PTHREAD_MUTEX_RECURSIVE
        #define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
    #endif
#endif

#if !BIT_WIN_LIKE && !CYGWIN
    #ifndef O_BINARY
        #define O_BINARY    0
    #endif
    #ifndef O_TEXT
        #define O_TEXT      0
    #endif
#endif

#if !LINUX
    #define __WALL          0
#if !CYGWIN
    #define MSG_NOSIGNAL    0
#endif
#endif

#if MACOSX
    /*
        Fix for MAC OS X - getenv
     */
    #if !HAVE_DECL_ENVIRON
        #ifdef __APPLE__
            #include <crt_externs.h>
            #define environ (*_NSGetEnviron())
        #else
            extern char **environ;
        #endif
    #endif
#endif

#if SOLARIS
    #define INADDR_NONE     ((in_addr_t) 0xffffffff)
#endif

#if VXWORKS
    #ifndef SHUT_RDWR
        #define SHUT_RDWR 2
    #endif

    #define HAVE_SOCKLEN_T
    #define getpid mprGetpid

    #if _DIAB_TOOL
        #define inline __inline__
    #endif
    #ifndef closesocket
        #define closesocket(x)  close(x)
    #endif
    #ifndef va_copy
        #define va_copy(d, s) ((d) = (s))
    #endif
    #ifndef strcasecmp
        #define strcasecmp scaselesscmp
    #endif
    #ifndef strncasecmp
        #define strncasecmp sncaselesscmp
    #endif
#endif

#if BIT_WIN_LIKE
    typedef int     uid_t;
    typedef void    *handle;
    typedef char    *caddr_t;
    typedef long    pid_t;
    typedef int     gid_t;
    typedef ushort  mode_t;
    typedef void    *siginfo_t;
    typedef int     socklen_t;

    #define HAVE_SOCKLEN_T
    #define MSG_NOSIGNAL    0
    #define MPR_BINARY      "b"
    #define MPR_TEXT        "t"

    /*
        Error codes 
     */
    #define EPERM           1
    #define ENOENT          2
    #define ESRCH           3
    #define EINTR           4
    #define EIO             5
    #define ENXIO           6
    #define E2BIG           7
    #define ENOEXEC         8
    #define EBADF           9
    #define ECHILD          10
    #define EAGAIN          11
    #define ENOMEM          12
    #define EACCES          13
    #define EFAULT          14
    #define EOSERR          15
    #define EBUSY           16
    #define EEXIST          17
    #define EXDEV           18
    #define ENODEV          19
    #define ENOTDIR         20
    #define EISDIR          21
    #define EINVAL          22
    #define ENFILE          23
    #define EMFILE          24
    #define ENOTTY          25
    #define EFBIG           27
    #define ENOSPC          28
    #define ESPIPE          29
    #define EROFS           30
    #define EMLINK          31
    #define EPIPE           32
    #define EDOM            33
    #define ERANGE          34

    #ifndef EWOULDBLOCK
    #define EWOULDBLOCK     EAGAIN
    #define EINPROGRESS     36
    #define EALREADY        37
    #define ENETDOWN        43
    #define ECONNRESET      44
    #define ECONNREFUSED    45
    #define EADDRNOTAVAIL   49
    #define EISCONN         56
    #define EADDRINUSE      46
    #define ENETUNREACH     51
    #define ECONNABORTED    53
    #endif

    #undef SHUT_RDWR
    #define SHUT_RDWR       2
    
    #define TIME_GENESIS UINT64(11644473600000000)
    #define va_copy(d, s) ((d) = (s))

    #if !WINCE
    #define access      _access
    #define chdir       _chdir
    #define chmod       _chmod
    #define close       _close
    #define fileno      _fileno
    #define fstat       _fstat
    #define getcwd      _getcwd
    #define getpid      _getpid
    #define gettimezone _gettimezone
    #define lseek       _lseek
    #define mkdir(a,b)  _mkdir(a)
    #define open        _open
    #define putenv      _putenv
    #define read        _read
    #define rmdir(a)    _rmdir(a)
    #define stat        _stat
    #define strdup      _strdup
    #define umask       _umask
    #define unlink      _unlink
    #define write       _write
    #endif
    #define strcasecmp scaselesscmp
    #define strncasecmp sncaselesscmp
#endif /* WIN_LIKE */

#if WINCE
    typedef void FILE;
    typedef int off_t;

    #ifndef EOF
        #define EOF        -1
    #endif

    #define O_RDONLY        0
    #define O_WRONLY        1
    #define O_RDWR          2
    #define O_NDELAY        0x4
    #define O_NONBLOCK      0x4
    #define O_APPEND        0x8
    #define O_CREAT         0x100
    #define O_TRUNC         0x200
    #define O_TEXT          0x400
    #define O_EXCL          0x800
    #define O_BINARY        0x1000

    /*
        stat flags
     */
    #define S_IFMT          0170000 
    #define S_IFDIR         0040000
    #define S_IFCHR         0020000         /* character special */
    #define S_IFIFO         0010000
    #define S_IFREG         0100000
    #define S_IREAD         0000400
    #define S_IWRITE        0000200
    #define S_IEXEC         0000100

    #ifndef S_ISDIR
        #define S_ISDIR(X) (((X) & S_IFMT) == S_IFDIR)
    #endif
    #ifndef S_ISREG
        #define S_ISREG(X) (((X) & S_IFMT) == S_IFREG)
    #endif

    #define STARTF_USESHOWWINDOW 0
    #define STARTF_USESTDHANDLES 0

    #define BUFSIZ   MPR_BUFSIZE
    #define PATHSIZE MPR_MAX_PATH
    #define gethostbyname2(a,b) gethostbyname(a)
#endif /* WINCE */

/*********************************** Externs **********************************/

#ifdef __cplusplus
extern "C" {
#endif

#if LINUX
    /*
        For some reason it is removed from fedora 6 pthreads.h and only comes in for UNIX96
     */
    extern int pthread_mutexattr_gettype (__const pthread_mutexattr_t *__restrict
        __attr, int *__restrict __kind) __THROW;
    /* 
        Set the mutex kind attribute in *ATTR to KIND (either PTHREAD_MUTEX_NORMAL,
        PTHREAD_MUTEX_RECURSIVE, PTHREAD_MUTEX_ERRORCHECK, or PTHREAD_MUTEX_DEFAULT).  
     */
    extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind) __THROW;
    extern char **environ;
#endif

#if VXWORKS
    extern int gettimeofday(struct timeval *tv, struct timezone *tz);
    extern uint mprGetpid();
    extern char *strdup(const char *);
    extern int sysClkRateGet();

    #if _WRS_VXWORKS_MAJOR < 6
        #define NI_MAXHOST      128
        extern STATUS access(cchar *path, int mode);
        typedef int     socklen_t;
        struct sockaddr_storage {
            char        pad[1024];
        };
    #else
        /*
            This may or may not be necessary - let us know dev@embedthis.com if your system needs this (and why).
         */
        #if _DIAB_TOOL
            #if BIT_CPU_ARCH == MPR_CPU_PPC
                #define __va_copy(dest, src) memcpy((dest), (src), sizeof(va_list))
            #endif
        #endif
        #define HAVE_SOCKLEN_T
    #endif
#endif  /* VXWORKS */

#if BIT_WIN_LIKE
    struct timezone {
      int  tz_minuteswest;      /* minutes W of Greenwich */
      int  tz_dsttime;          /* type of dst correction */
    };
    extern int  getuid(void);
    extern int  geteuid(void);
    extern int  gettimeofday(struct timeval *tv, struct timezone *tz);
    extern long lrand48(void);
    extern long nap(long);
    extern void srand48(long);
    extern long ulimit(int, ...);
#endif

#if WINCE
    struct stat {
        int     st_dev;
        int     st_ino;
        ushort  st_mode;
        short   st_nlink;
        short   st_uid;
        short   st_gid;
        int     st_rdev;
        long    st_size;
        time_t  st_atime;
        time_t  st_mtime;
        time_t  st_ctime;
    };

    extern int access(cchar *filename, int flags);
    extern int chdir(cchar   dirname);
    extern int chmod(cchar *path, int mode);
    extern int close(int handle);
    extern void exit(int status);
    extern long _get_osfhandle(int handle);
    extern char *getcwd(char* buffer, int maxlen);
    extern char *getenv(cchar *charstuff);
    extern uint getpid();
    extern long lseek(int handle, long offset, int origin);
    extern int mkdir(cchar *dir, int mode);
    extern time_t mktime(struct tm *pt);
    extern int _open_osfhandle(int *handle, int flags);
    extern uint open(cchar *file, int mode,...);
    extern int read(int handle, void *buffer, uint count);
    extern int rename(cchar *from, cchar *to);
    extern int rmdir(cchar   dir);
    extern uint sleep(uint secs);
    extern int stat(cchar *path, struct stat *stat);
    extern char *strdup(char *s);
    extern int write(int handle, cvoid *buffer, uint count);
    extern int umask(int mode);
    extern int unlink(cchar *path);
    extern int errno;

    #undef CreateFile
    #define CreateFile CreateFileA
    WINBASEAPI HANDLE WINAPI CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile);

    #undef CreateProcess
    #define CreateProcess CreateProcessA

    #undef FindFirstFile
    #define FindFirstFile FindFirstFileA
    WINBASEAPI HANDLE WINAPI FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

    #undef FindNextFile
    #define FindNextFile FindNextFileA
    WINBASEAPI BOOL WINAPI FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);

    #undef GetModuleFileName
    #define GetModuleFileName GetModuleFileNameA
    WINBASEAPI DWORD WINAPI GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);

    #undef GetModuleHandle
    #define GetModuleHandle GetModuleHandleA
    WINBASEAPI HMODULE WINAPI GetModuleHandleA(LPCSTR lpModuleName);

    #undef GetProcAddress
    #define GetProcAddress GetProcAddressA

    #undef GetFileAttributes
    #define GetFileAttributes GetFileAttributesA
    extern DWORD GetFileAttributesA(cchar *path);

    extern void GetSystemTimeAsFileTime(FILETIME *ft);

    #undef LoadLibrary
    #define LoadLibrary LoadLibraryA
    HINSTANCE WINAPI LoadLibraryA(LPCSTR lpLibFileName);

    #define WSAGetLastError GetLastError

    #define _get_timezone getTimezone
    extern int getTimezone(int *secs);

    extern struct tm *localtime_r(const time_t *when, struct tm *tp);
    extern struct tm *gmtime_r(const time_t *t, struct tm *tp);
#endif /* WINCE */

#ifdef __cplusplus
}
#endif

/*********************************** Forwards *********************************/

#ifdef __cplusplus
extern "C" {
#endif

struct  tm;
struct  Mpr;
struct  MprMem;
struct  MprBuf;
struct  MprCmd;
struct  MprCond;
struct  MprDispatcher;
struct  MprEvent;
struct  MprEventService;
struct  MprFile;
struct  MprFileSystem;
struct  MprHash;
struct  MprHeap;
struct  MprList;
struct  MprKey;
struct  MprModule;
struct  MprMutex;
struct  MprOsService;
struct  MprPath;
struct  MprSignal;
struct  MprSocket;
struct  MprSocketService;
struct  MprSsl;
struct  MprThread;
struct  MprThreadService;
struct  MprWaitService;
struct  MprWaitHandler;
struct  MprWorker;
struct  MprWorkerService;
struct  MprXml;

/******************************* Tunable Constants ****************************/
/*
    Build tuning
 */
#define MPR_TUNE_SIZE       1       /**< Tune for size */
#define MPR_TUNE_BALANCED   2       /**< Tune balancing speed and size */
#define MPR_TUNE_SPEED      3       /**< Tune for speed, program will use memory more aggressively */

#if BIT_HAS_MMU
    /* 
        If the system supports virtual memory, then stack size should use system default. Only used pages will
        actually consume memory 
     */
    #define MPR_DEFAULT_STACK       (0)           /**< Default thread stack size (0 means use system default) */
#else
    /* 
        No MMU, so the stack size actually consumes memory. Set this as low as possible. 
        NOTE: php and ejs use stack heavily.
     */
    #define MPR_DEFAULT_STACK       (128 * 1024)   /**< Default thread stack size (0 means use system default) */
#endif

#if BIT_TUNE == MPR_TUNE_SIZE || DOXYGEN
    /*
        Reduce size allocations to reduce memory usage
     */
    #define MPR_MAX_FNAME           256           /**< Reasonable filename size */
    #define MPR_MAX_PATH            512           /**< Reasonable path name size */
    #define MPR_MAX_URL             512           /**< Max URL size. Also request URL size. */
    #define MPR_MAX_STRING          1024          /**< Maximum (stack) string size */
    #define MPR_MAX_LOG             (8 * 1024)    /**< Maximum log message size (impacts stack) */
    #define MPR_SMALL_ALLOC         256           /**< Default small. Used in printf. */
    #define MPR_DEFAULT_HASH_SIZE   23            /**< Default size of hash table */ 
    #define MPR_BUFSIZE             4096          /**< Reasonable size for buffers */
    #define MPR_BUF_INCR            4096          /**< Default buffer growth inc */
    #define MPR_EPOLL_SIZE          32            /**< Epoll backlog */
    #define MPR_MAX_BUF             4194304       /**< Max buffer size */
    #define MPR_XML_BUFSIZE         4096          /**< XML read buffer size */
    #define MPR_SSL_BUFSIZE         4096          /**< SSL has 16K max*/
    #define MPR_LIST_INCR           8             /**< Default list growth inc */
    #define MPR_FILES_HASH_SIZE     29            /**< Hash size for rom file system */
    #define MPR_TIME_HASH_SIZE      67            /**< Hash size for time token lookup */
    #define MPR_MEM_REGION_SIZE     (128 * 1024)  /**< Memory allocation chunk size */
    #define MPR_GC_LOW_MEM          (32 * 1024)   /**< Free memory low water mark before invoking GC */
    #define MPR_NEW_QUOTA           (4 * 1024)    /**< Number of new allocations before a GC is worthwhile */
    #define MPR_GC_WORKERS          0             /**< Run garbage collection non-concurrently */
    
#elif BIT_TUNE == MPR_TUNE_BALANCED
    
    /*
        Tune balancing speed and size
     */
    #define MPR_MAX_FNAME           256
    #define MPR_MAX_PATH            1024
    #define MPR_MAX_URL             2048
    #define MPR_MAX_STRING          2048
    #define MPR_MAX_LOG             (32 * 1024)
    #define MPR_SMALL_ALLOC         512
    #define MPR_DEFAULT_HASH_SIZE   43
    #define MPR_BUFSIZE             4096
    #define MPR_BUF_INCR            4096
    #define MPR_MAX_BUF             -1
    #define MPR_EPOLL_SIZE          64
    #define MPR_XML_BUFSIZE         4096
    #define MPR_SSL_BUFSIZE         4096
    #define MPR_LIST_INCR           16
    #define MPR_FILES_HASH_SIZE     61
    #define MPR_TIME_HASH_SIZE      89
    #define MPR_MEM_REGION_SIZE     (256 * 1024)
    #define MPR_GC_LOW_MEM          (64 * 1024)
    #define MPR_NEW_QUOTA           (16 * 1024) 
    #define MPR_GC_WORKERS          1
    
#else
    /*
        Tune for speed
     */
    #define MPR_MAX_FNAME           1024
    #define MPR_MAX_PATH            2048
    #define MPR_MAX_URL             4096
    #define MPR_MAX_LOG             (64 * 1024)
    #define MPR_MAX_STRING          4096
    #define MPR_SMALL_ALLOC         1024
    #define MPR_DEFAULT_HASH_SIZE   97
    #define MPR_BUFSIZE             8192
    #define MPR_MAX_BUF             -1
    #define MPR_EPOLL_SIZE          128
    #define MPR_XML_BUFSIZE         4096
    #define MPR_SSL_BUFSIZE         8192
    #define MPR_LIST_INCR           16
    #define MPR_BUF_INCR            1024
    #define MPR_FILES_HASH_SIZE     61
    #define MPR_TIME_HASH_SIZE      97
    #define MPR_MEM_REGION_SIZE     (1024 * 1024)
    #define MPR_GC_LOW_MEM          (512 * 1024)
    #define MPR_NEW_QUOTA           (32 * 1024) 
    #define MPR_GC_WORKERS          2
#endif

/*
    Select wakeup port. Port can be any free port number. If this is not free, the MPR will use the next free port.
 */
#define MPR_DEFAULT_BREAK_PORT  9473
#define MPR_FD_MIN              32

/* 
    Longest IPv6 is XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX (40 bytes with null) 
 */ 
#define MPR_MAX_IP_NAME         1024
#define MPR_MAX_IP_ADDR         1024
#define MPR_MAX_IP_PORT         8
#define MPR_MAX_IP_ADDR_PORT    1024

/*
    Signal sent on Unix to break out of a select call.
 */
#define MPR_WAIT_SIGNAL         (SIGUSR2)

/*
    Socket event message
 */
#define MPR_SOCKET_MESSAGE      (WM_USER + 32)

/*
    Priorities
 */
#define MPR_BACKGROUND_PRIORITY 15          /**< May only get CPU if idle */
#define MPR_LOW_PRIORITY        25
#define MPR_NORMAL_PRIORITY     50          /**< Normal (default) priority */
#define MPR_HIGH_PRIORITY       75
#define MPR_CRITICAL_PRIORITY   99          /**< May not yield */

#define MPR_EVENT_PRIORITY      50          /**< Normal priority */ 
#define MPR_WORKER_PRIORITY     50          /**< Normal priority */
#define MPR_REQUEST_PRIORITY    50          /**< Normal priority */

/* 
    Timeouts
 */
#if UNUSED && KEEP
#define MPR_TIMEOUT_CMD         60000       /**< Command Request timeout (60 sec) */
#define MPR_TIMEOUT_SOCKETS     10000       /**< Socket connection socket timeout */
#define MPR_TIMEOUT_LOG_STAMP   3600000     /**< Time between log time stamps (1 hr) */
#define MPR_TIMEOUT_STOP_THREAD 10000       /**< Time to stop running threads */
#define MPR_TIMEOUT_HANDLER     10000       /**< Wait period when removing a wait handler */
#endif

#define MPR_TIMEOUT_PRUNER      600000      /**< Time between worker thread pruner runs (10 min) */
#define MPR_TIMEOUT_WORKER      300000      /**< Prune worker that has been idle for 5 minutes */
#define MPR_TIMEOUT_START_TASK  10000       /**< Time to start tasks running */
#define MPR_TIMEOUT_STOP        30000       /**< Default wait when stopping resources (30 sec) */
#define MPR_TIMEOUT_STOP_TASK   10000       /**< Time to stop or reap tasks (vxworks) */
#define MPR_TIMEOUT_LINGER      2000        /**< Close socket linger timeout */
#define MPR_TIMEOUT_GC_SYNC     10000       /**< Wait period for threads to synchronize */
#define MPR_TIMEOUT_NO_BUSY     1000        /**< Wait period to minimize CPU drain */
#define MPR_TIMEOUT_NAP         20          /**< Short pause */

#define MPR_TICKS_PER_SEC       1000        /**< Time ticks per second */
#define MPR_MAX_TIMEOUT         MAXINT


/*
    Default thread counts
 */
#define MPR_DEFAULT_MIN_THREADS 0           /**< Default min threads */
#define MPR_DEFAULT_MAX_THREADS 20          /**< Default max threads */

/*
    Debug control
 */
#define MPR_MAX_BLOCKED_LOCKS   100         /* Max threads blocked on lock */
#define MPR_MAX_RECURSION       15          /* Max recursion with one thread */
#define MPR_MAX_LOCKS           512         /* Total lock count max */
#define MPR_MAX_LOCK_TIME       (60 * 1000) /* Time in msec to hold a lock */

#define MPR_TIMER_TOLERANCE     2           /* Used in timer calculations */
#define MPR_CMD_TIMER_PERIOD    5000        /* Check for expired commands */

/*
    Other tunable constants
 */
#define MPR_MAX_ARGC            32           /**< Reasonable max of args for MAIN */
#define MPR_TEST_POLL_NAP       25
#define MPR_TEST_SLEEP          (60 * 1000)
#define MPR_TEST_MAX_STACK      16
#define MPR_TEST_TIMEOUT        10000       /* Ten seconds */
#define MPR_TEST_LONG_TIMEOUT   300000      /* 5 minutes */
#define MPR_TEST_SHORT_TIMEOUT  200         /* 1/5 sec */
#define MPR_TEST_NAP            50          /* Short timeout to prevent busy waiting */

/*
    Events
 */
#define MPR_EVENT_TIME_SLICE    20          /* 20 msec */

/*
    Maximum number of files
 */
#define MPR_MAX_FILE            256

/*
    Event notification mechanism
 */
#if LINUX || FREEBSD
    #define MPR_EVENT_EPOLL     1
#elif MACOSX || SOLARIS
    #define MPR_EVENT_KQUEUE    1
#elif VXWORKS || WINCE || CYGWIN
    #define MPR_EVENT_SELECT    1
#elif WINDOWS
    #define MPR_EVENT_ASYNC     1
#else
    #define MPR_EVENT_POLL      1
#endif

/*
    Garbage collector tuning
 */
#define MPR_MIN_TIME_FOR_GC     2       /**< Wait till 2 milliseconds of idle time possible */
    
/************************************ Error Codes *****************************/

/* Prevent collisions with 3rd party software */
#undef UNUSED

/*
    Standard errors
 */
#define MPR_ERR_OK                      0       /**< Success */
#define MPR_ERR_BASE                    -1      /**< Base error code */
#define MPR_ERR                         -1      /**< Default error code */
#define MPR_ERR_ABORTED                 -2      /**< Action aborted */
#define MPR_ERR_ALREADY_EXISTS          -3      /**< Item already exists */
#define MPR_ERR_BAD_ARGS                -4      /**< Bad arguments or paramaeters */
#define MPR_ERR_BAD_FORMAT              -5      /**< Bad input format */
#define MPR_ERR_BAD_HANDLE              -6      /**< Bad file handle */
#define MPR_ERR_BAD_STATE               -7      /**< Module is in a bad state */
#define MPR_ERR_BAD_SYNTAX              -8      /**< Input has bad syntax */
#define MPR_ERR_BAD_TYPE                -9      /**< Bad object type */
#define MPR_ERR_BAD_VALUE               -10     /**< Bad or unexpected value */
#define MPR_ERR_BUSY                    -11     /**< Resource is busy */
#define MPR_ERR_CANT_ACCESS             -12     /**< Can't access the file or resource */
#define MPR_ERR_CANT_ALLOCATE           -13     /**< Can't allocate resource */
#define MPR_ERR_CANT_COMPLETE           -14     /**< Operation can't complete */
#define MPR_ERR_CANT_CONNECT            -15     /**< Can't connect to network or resource */
#define MPR_ERR_CANT_CREATE             -16     /**< Can't create the file or resource */
#define MPR_ERR_CANT_DELETE             -17     /**< Can't delete the resource */
#define MPR_ERR_CANT_FIND               -18     /**< Can't find resource */
#define MPR_ERR_CANT_INITIALIZE         -19     /**< Can't initialize resource */
#define MPR_ERR_CANT_LOAD               -20     /**< Can't load the resource */
#define MPR_ERR_CANT_OPEN               -21     /**< Can't open the file or resource */
#define MPR_ERR_CANT_READ               -22     /**< Can't read from the file or resource */
#define MPR_ERR_CANT_WRITE              -23     /**< Can't write to the file or resource */
#define MPR_ERR_DELETED                 -24     /**< Resource has been deleted */
#define MPR_ERR_MEMORY                  -25     /**< Memory allocation error */
#define MPR_ERR_NETWORK                 -26     /**< Underlying network error */
#define MPR_ERR_NOT_INITIALIZED         -27     /**< Module or resource is not initialized */
#define MPR_ERR_NOT_READY               -28     /**< Resource is not ready */
#define MPR_ERR_READ_ONLY               -29     /**< The operation timed out */
#define MPR_ERR_TIMEOUT                 -30     /**< Operation exceeded specified time allowed */
#define MPR_ERR_TOO_MANY                -31     /**< Too many requests or resources */
#define MPR_ERR_WONT_FIT                -32     /**< Requested operation won't fit in available space */
#define MPR_ERR_WOULD_BLOCK             -33     /**< Blocking operation would block */
#define MPR_ERR_MAX                     -34

/**
    Standard logging trace levels are 0 to 9 with 0 being the most verbose. These are ored with the error source
    and type flags. The MPR_LOG_MASK is used to extract the trace level from a flags word. We expect most apps
    to run with level 2 trace enabled.
 */
#define MPR_ERROR           1           /**< Hard error trace level */
#define MPR_WARN            2           /**< Soft warning trace level */
#define MPR_CONFIG          2           /**< Configuration settings trace level. */
#define MPR_INFO            3           /**< Informational trace only */
#define MPR_DEBUG           4           /**< Debug information trace level */
#define MPR_VERBOSE         9           /**< Highest level of trace */
#define MPR_LEVEL_MASK      0xf         /**< Level mask */

/*
    Error source flags
 */
#define MPR_ERROR_SRC       0x10        /**< Originated from mprError */
#define MPR_WARN_SRC        0x20        /**< Originated from mprWarn */
#define MPR_LOG_SRC         0x40        /**< Originated from mprLog */
#define MPR_ASSERT_SRC      0x80        /**< Originated from mprAssert */
#define MPR_FATAL_SRC       0x100       /**< Fatal error. Log and exit */

/*
    Log message type flags. Specify what kind of log / error message it is. Listener handlers examine this flag
    to determine if they should process the message. Assert messages are trapped when in DEV mode. Otherwise ignored.
 */
#define MPR_LOG_MSG         0x1000      /* Log trace message - not an error */
#define MPR_ERROR_MSG       0x2000      /* General error */
#define MPR_ASSERT_MSG      0x4000      /* Assert flags -- trap in debugger */
#define MPR_USER_MSG        0x8000      /* User message */

/*
    Log output modifiers
 */
#define MPR_RAW             0x1000      /**< Raw trace output */

/*
    Error line number information.
 */
#define MPR_LINE(s)         #s
#define MPR_LINE2(s)        MPR_LINE(s)
#define MPR_LINE3           MPR_LINE2(__LINE__)
#define MPR_LOC             __FILE__ ":" MPR_LINE3
#define MPR_NAME(msg)       msg "@" MPR_LOC

#define MPR_STRINGIFY(s)    #s

/*
    Foundational types
 */
#ifndef BIT_CHAR_LEN
    #define BIT_CHAR_LEN 1
#endif
#if BIT_CHAR_LEN == 4
    typedef int MprChar;
    #define T(s) L ## s
#elif BIT_CHAR_LEN == 2
    typedef short MprChar;
    #define T(s) L ## s
#else
    typedef char MprChar;
    #define T(s) s
#endif

/*
    Convenience define to declare a main program entry point that works for Windows, VxWorks and Unix
 */
#if VXWORKS
    #define MAIN(name, _argc, _argv, _envp)  \
        static int innerMain(int argc, char **argv, char **envp); \
        int name(char *arg0, ...) { \
            va_list args; \
            char *argp, *largv[MPR_MAX_ARGC]; \
            int largc = 0; \
            va_start(args, arg0); \
            largv[largc++] = #name; \
            if (arg0) { \
                largv[largc++] = arg0; \
            } \
            for (argp = va_arg(args, char*); argp && largc < MPR_MAX_ARGC; argp = va_arg(args, char*)) { \
                largv[largc++] = argp; \
            } \
            return innerMain(largc, largv, NULL); \
        } \
        static int innerMain(_argc, _argv, _envp)
#elif BIT_WIN_LIKE && BIT_CHAR_LEN > 1
    #define MAIN(name, _argc, _argv, _envp)  \
        APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, LPWSTR command, int junk2) { \
            char *largv[MPR_MAX_ARGC]; \
            extern int main(); \
            char *mcommand[MPR_MAX_STRING]; \
            int largc; \
            wtom(mcommand, sizeof(dest), command, -1);
            largc = mprParseArgs(mcommand, &largv[1], MPR_MAX_ARGC - 1); \
            largv[0] = #name; \
            main(largc, largv, NULL); \
        } \
        int main(argc, argv, _envp)
#elif BIT_WIN_LIKE
    #define MAIN(name, _argc, _argv, _envp)  \
        APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *command, int junk2) { \
            extern int main(); \
            char *largv[MPR_MAX_ARGC]; \
            int largc; \
            largc = mprParseArgs(command, &largv[1], MPR_MAX_ARGC - 1); \
            largv[0] = #name; \
            main(largc, largv, NULL); \
        } \
        int main(_argc, _argv, _envp)
#else
    #define MAIN(name, _argc, _argv, _envp) int main(_argc, _argv, _envp)
#endif

#if BIT_UNIX_LIKE
    typedef pthread_t   MprOsThread;
#elif BIT_64
    typedef int64       MprOsThread;
#else
    typedef int         MprOsThread;
#endif

#if BIT_WIN_LIKE
    #define MPR_INLINE __inline
#else
    #define MPR_INLINE inline
#endif

/*
    Byte orderings
 */
#define MPR_LITTLE_ENDIAN   1
#define MPR_BIG_ENDIAN      2
#define MPR_ENDIAN          BIT_ENDIAN

/************************************** Debug *********************************/
/**
    Trigger a breakpoint.
    @description Triggers a breakpoint and traps to the debugger. 
    @ingroup Mpr
 */
extern void mprBreakpoint();

#if BIT_ASSERT
    #define mprAssert(C)    if (C) ; else mprAssertError(MPR_LOC, #C)
#else
    #define mprAssert(C)    if (1) ; else
#endif

/*********************************** Thread Sync ******************************/
/**
    Multithreaded Synchronization Services
    @see MprCond MprMutex MprSpin mprAtomicAdd mprAtomicAdd64 mprAtomicBarrier mprAtomicCas mprAtomicExchange 
        mprAtomicListInsert mprCreateCond mprCreateLock mprCreateSpinLock mprGlobalLock mprGlobalUnlock mprInitLock 
        mprInitSpinLock mprLock mprResetCond mprSignalCond mprSignalMultiCond mprSpinLock mprSpinUnlock mprTryLock 
        mprTrySpinLock mprUnlock mprWaitForCond mprWaitForMultiCond 
    @stability Evolving.
    @defgroup MprSynch MprSynch
 */
typedef struct MprSynch { int dummy; } MprSynch;

/**
    Condition variable for single and multi-thread synchronization. Condition variables can be used to coordinate 
    activities. These variables are level triggered in that a condition can be signalled prior to another thread 
    waiting. Condition variables can be used when single threaded but mprServiceEvents should be called to  pump events
    until another callback invokes mprWaitForCond.
    @ingroup MprSynch
 */
typedef struct MprCond {
    #if BIT_UNIX_LIKE
        pthread_cond_t cv;          /**< Unix pthreads condition variable */
    #elif BIT_WIN_LIKE
        HANDLE cv;                  /* Windows event handle */
    #elif VXWORKS
        SEM_ID cv;                  /* Condition variable */
    #else
        #warning "Unsupported OS in MprCond definition in mpr.h"
    #endif
    struct MprMutex *mutex;         /**< Thread synchronization mutex */
    volatile int triggered;         /**< Value of the condition */
} MprCond;


/**
    Create a condition lock variable.
    @description This call creates a condition variable object that can be used in #mprWaitForCond and #mprSignalCond calls. 
    @ingroup MprSynch
 */
extern MprCond *mprCreateCond();

/**
    Reset a condition variable. This sets the condition variable to the unsignalled condition.
    @param cond Condition variable object created via #mprCreateCond
 */
extern void mprResetCond(MprCond *cond);

/**
    Wait for a condition lock variable.
    @description Wait for a condition lock variable to be signaled. If the condition is signaled before the timeout
        expires, this call will reset the condition variable and return. This way, it automatically resets the variable
        for future waiters.
    @param cond Condition variable object created via #mprCreateCond
    @param timeout Time in milliseconds to wait for the condition variable to be signaled.
    @return Zero if the event was signalled. Returns < 0 for a timeout.
    @ingroup MprSynch
 */
extern int mprWaitForCond(MprCond *cond, MprTime timeout);

/**
    Signal a condition lock variable.
    @description Signal a condition variable and set it to the \a triggered status. Existing or future caller of
        #mprWaitForCond will be awakened. The condition variable will be automatically reset when the waiter awakes.
        Should only be used for single waiters. Use mprSignalMultiCond for use with multiple waiters.
    @param cond Condition variable object created via #mprCreateCond
    @ingroup MprSynch
 */
extern void mprSignalCond(MprCond *cond);

/**
    Signal a condition lock variable for use with multiple waiters.
    @description Signal a condition variable and set it to the \a triggered status. Existing or future callers of
        #mprWaitForCond will be awakened. The conditional variable will not be automatically reset and must be reset
        manually via mprResetCond.
    @param cond Condition variable object created via #mprCreateCond
    @ingroup MprSynch
 */
extern void mprSignalMultiCond(MprCond *cond);

/**
    Wait for a condition lock variable for use with multiple waiters.
    @description Wait for a condition lock variable to be signaled. Multiple waiters are supported and the 
        condition variable must be manually reset via mprResetCond. The condition may signaled before calling 
        mprWaitForMultiCond.
    @param cond Condition variable object created via #mprCreateCond
    @param timeout Time in milliseconds to wait for the condition variable to be signaled.
    @return Zero if the event was signalled. Returns < 0 for a timeout.
    @ingroup MprSynch
 */
extern int mprWaitForMultiCond(MprCond *cond, MprTime timeout);

/**
    Multithreading lock control structure
    @description MprMutex is used for multithread locking in multithreaded applications.
    @ingroup MprSynch
 */
typedef struct MprMutex {
    #if BIT_WIN_LIKE
        CRITICAL_SECTION cs;            /**< Internal mutex critical section */
    #elif VXWORKS
        SEM_ID      cs;
    #elif BIT_UNIX_LIKE
        pthread_mutex_t  cs;
    #else
        #warning "Unsupported OS in MprMutex definition in mpr.h"
    #endif
#if BIT_DEBUG
        MprOsThread owner;
#endif
} MprMutex;


/**
    Multithreading spin lock control structure
    @description    MprSpin is used for multithread locking in multithreaded applications.
    @ingroup MprSynch
 */
typedef struct MprSpin {
    #if USE_MPR_LOCK
        MprMutex                cs;
    #elif BIT_WIN_LIKE
        CRITICAL_SECTION        cs;            /**< Internal mutex critical section */
    #elif VXWORKS
        #if FUTURE && SPIN_LOCK_TASK_INIT
            spinlockTask_t      cs;
        #else
            SEM_ID              cs;
        #endif
    #elif MACOSX
        OSSpinLock              cs;
    #elif BIT_UNIX_LIKE
        #if BIT_HAS_SPINLOCK
            pthread_spinlock_t  cs;
        #else
            pthread_mutex_t     cs;
        #endif
    #else
        #warning "Unsupported OS in MprSpin definition in mpr.h"
    #endif
    #if BIT_DEBUG
        MprOsThread         owner;
    #endif
} MprSpin;


#undef lock
#undef unlock
#undef spinlock
#undef spinunlock
#define lock(arg)       if (arg) mprLock((arg)->mutex)
#define unlock(arg)     if (arg) mprUnlock((arg)->mutex)
#define spinlock(arg)   if (arg) mprSpinLock((arg)->spin)
#define spinunlock(arg) if (arg) mprSpinUnlock((arg)->spin)

/**
    Create a Mutex lock object.
    @description This call creates a Mutex lock object that can be used in #mprLock, #mprTryLock and #mprUnlock calls. 
    @ingroup MprSynch
 */
extern MprMutex *mprCreateLock();

/**
    Initialize a statically allocated Mutex lock object.
    @description This call initialized a Mutex lock object without allocation. The object can then be used used 
        in #mprLock, #mprTryLock and #mprUnlock calls.
    @param mutex Reference to an MprMutex structure to initialize
    @returns A reference to the supplied mutex. Returns null on errors.
    @ingroup MprSynch
 */
extern MprMutex *mprInitLock(MprMutex *mutex);

/**
    Attempt to lock access.
    @description This call attempts to assert a lock on the given \a lock mutex so that other threads calling 
        mprLock or mprTryLock will block until the current thread calls mprUnlock.
    @returns Returns zero if the successful in locking the mutex. Returns a negative MPR error code if unsuccessful.
    @ingroup MprSynch
 */
extern bool mprTryLock(MprMutex *lock);

/**
    Create a spin lock lock object.
    @description This call creates a spinlock object that can be used in #mprSpinLock, and #mprSpinUnlock calls. Spin locks
        using MprSpin are much faster than MprMutex based locks on some systems.
    @ingroup MprSynch
 */
extern MprSpin *mprCreateSpinLock();

/**
    Initialize a statically allocated spinlock object.
    @description This call initialized a spinlock lock object without allocation. The object can then be used used 
        in #mprSpinLock and #mprSpinUnlock calls.
    @param lock Reference to a static #MprSpin  object.
    @returns A reference to the MprSpin object. Returns null on errors.
    @ingroup MprSynch
 */
extern MprSpin *mprInitSpinLock(MprSpin *lock);

/**
    Attempt to lock access on a spin lock
    @description This call attempts to assert a lock on the given \a spin lock so that other threads calling 
        mprSpinLock or mprTrySpinLock will block until the current thread calls mprSpinUnlock.
    @returns Returns zero if the successful in locking the spinlock. Returns a negative MPR error code if unsuccessful.
    @ingroup MprSynch
 */
extern bool mprTrySpinLock(MprSpin *lock);

/*
    Internal
 */
void mprManageSpinLock(MprSpin *lock, int flags);

/*
    For maximum performance, use the spin lock/unlock routines macros
 */
#if !BIT_DEBUG
#define BIT_USE_LOCK_MACROS 1
#endif
#if BIT_USE_LOCK_MACROS && !DOXYGEN
    /*
        Spin lock macros
     */
    #if MACOSX
        #define mprSpinLock(lock)   if (lock) OSSpinLockLock(&((lock)->cs))
        #define mprSpinUnlock(lock) if (lock) OSSpinLockUnlock(&((lock)->cs))
    #elif BIT_UNIX_LIKE && BIT_HAS_SPINLOCK
        #define mprSpinLock(lock)   if (lock) pthread_spin_lock(&((lock)->cs))
        #define mprSpinUnlock(lock) if (lock) pthread_spin_unlock(&((lock)->cs))
    #elif BIT_UNIX_LIKE
        #define mprSpinLock(lock)   if (lock) pthread_mutex_lock(&((lock)->cs))
        #define mprSpinUnlock(lock) if (lock) pthread_mutex_unlock(&((lock)->cs))
    #elif BIT_WIN_LIKE
        #define mprSpinLock(lock)   if (lock && (((MprSpin*)(lock))->cs.SpinCount)) EnterCriticalSection(&((lock)->cs))
        #define mprSpinUnlock(lock) if (lock) LeaveCriticalSection(&((lock)->cs))
    #elif VXWORKS
        #define mprSpinLock(lock)   if (lock) semTake((lock)->cs, WAIT_FOREVER)
        #define mprSpinUnlock(lock) if (lock) semGive((lock)->cs)
    #endif

    /*
        Lock macros
     */
    #if BIT_UNIX_LIKE
        #define mprLock(lock)       if (lock) pthread_mutex_lock(&((lock)->cs))
        #define mprUnlock(lock)     if (lock) pthread_mutex_unlock(&((lock)->cs))
    #elif BIT_WIN_LIKE
        #define mprLock(lock)       if (lock && (((MprSpin*)(lock))->cs.SpinCount)) EnterCriticalSection(&((lock)->cs))
        #define mprUnlock(lock)     if (lock) LeaveCriticalSection(&((lock)->cs))
    #elif VXWORKS
        #define mprLock(lock)       if (lock) semTake((lock)->cs, WAIT_FOREVER)
        #define mprUnlock(lock)     if (lock) semGive((lock)->cs)
    #endif
#else

    /**
        Lock access.
        @description This call asserts a lock on the given \a lock mutex so that other threads calling mprLock will 
            block until the current thread calls mprUnlock.
        @ingroup MprSynch
     */
    extern void mprLock(MprMutex *lock);

    /**
        Unlock a mutex.
        @description This call unlocks a mutex previously locked via mprLock or mprTryLock.
        @ingroup MprSynch
     */
    extern void mprUnlock(MprMutex *lock);

    /**
        Lock a spinlock.
        @description This call asserts a lock on the given \a spinlock so that other threads calling mprSpinLock will
            block until the curren thread calls mprSpinUnlock.
        @ingroup MprSynch
     */
    extern void mprSpinLock(MprSpin *lock);

    /**
        Unlock a spinlock.
        @description This call unlocks a spinlock previously locked via mprSpinLock or mprTrySpinLock.
        @ingroup MprSynch
     */
    extern void mprSpinUnlock(MprSpin *lock);
#endif

/**
    Globally lock the application.
    @description This call asserts the application global lock so that other threads calling mprGlobalLock will 
        block until the current thread calls mprGlobalUnlock.
    @ingroup MprSynch
 */
extern void mprGlobalLock();

/**
    Unlock the global mutex.
    @description This call unlocks the global mutex previously locked via mprGlobalLock.
    @ingroup MprSynch
 */
extern void mprGlobalUnlock();

/*
    Lock free primitives
 */

/**
    Apply a full (read+write) memory barrier
    @ingroup MprSynch
 */ 
extern void mprAtomicBarrier();

/**
    Atomic list insertion. Inserts "item" at the "head" of the list. The "link" field is the next field in item.
    This is a lock-free function
    @param head list head
    @param link Reference to the list head link field
    @param item Item to insert
    @ingroup MprSynch
 */ 
extern void mprAtomicListInsert(void * volatile *head, volatile void **link, void *item);

/**
    Atomic Compare and Swap. This is a lock free function.
    @param target Address of the target word to swap
    @param expected Expected value of the target
    @param value New value to store at the target
    @return TRUE if the swap was successful
    @ingroup MprSynch
 */
extern int mprAtomicCas(void * volatile * target, void *expected, cvoid *value);

/**
    Atomic Add. This is a lock free function.
    @param target Address of the target word to add to.
    @param value Value to add to the target
    @ingroup MprSynch
 */
extern void mprAtomicAdd(volatile int *target, int value);

/**
    Atomic 64 bit Add. This is a lock free function.
    @param target Address of the target word to add to.
    @param value Value to add to the target
    @ingroup MprSynch
 */
extern void mprAtomicAdd64(volatile int64 *target, int value);

/**
    Exchange the target and a value
    @param target Address of the target word to exchange
    @param value Value to store to the target
    @ingroup MprSynch
 */
extern void *mprAtomicExchange(void * volatile *target, cvoid *value);

/********************************* Memory Allocator ***************************/
/*
    Allocator debug and stats selection
 */
#if BIT_DEBUG
    #define BIT_MEMORY_DEBUG        1                   /**< Fill blocks, verifies block integrity. */
    #define BIT_MEMORY_STATS        1                   /**< Include memory stats routines */
    #define BIT_MEMORY_STACK        1                   /**< Monitor stack usage */
#else
    #define BIT_MEMORY_DEBUG        0
    #define BIT_MEMORY_STATS        0
    #define BIT_MEMORY_STACK        0
#endif

/*
    Alignment bit sizes for the allocator. Blocks are aligned on 4 byte boundaries for 32 bits systems and 8 byte 
    boundaries for 64 bit systems and those systems that require doubles to be 8 byte aligned.
 */
#if !BIT_64 && !(MPR_CPU_MIPS)
    #define MPR_ALIGN               4
    #define MPR_ALIGN_SHIFT         2
#else
    #define MPR_ALIGN               8
    #define MPR_ALIGN_SHIFT         3
#endif

/*
    Maximum bits available for expressing a memory size. On 32-bit systems, this is 0.5GB or 0x20000000.
 */
#define MPR_SIZE_BITS               (BIT_WORDSIZE - 3)

/*
    MprMem.prior field bits. Layout for 32 bits. This field must only be accessed (read|write) while locked.
        prior | last/1 << 1 | hasManager/1
 */
#define MPR_SHIFT_HAS_MANAGER   0
#define MPR_SHIFT_LAST          1
#define MPR_SHIFT_PRIOR         0
#define MPR_MASK_HAS_MANAGER    0x1
#define MPR_MASK_LAST           (((size_t) 0x1) << MPR_SHIFT_LAST)
#define MPR_MASK_PRIOR          ~(MPR_MASK_LAST | MPR_MASK_HAS_MANAGER)

/*
    MprMem.size field bits. Layout for 32 bits. This field can be read while unlocked.
        gen/2 << 30 | free << 29 | size/29 | mark/2
*/
#define MPR_SHIFT_MARK          0
#define MPR_SHIFT_SIZE          0
#define MPR_SHIFT_FREE          (BIT_WORDSIZE - 3)
#define MPR_SHIFT_GEN           (BIT_WORDSIZE - 2)
#define MPR_MASK_MARK           (0x3)
#define MPR_MASK_GEN            (((size_t) 0x3) << MPR_SHIFT_GEN)
#define MPR_MASK_FREE           (((size_t) 0x1) << MPR_SHIFT_FREE)
#define MPR_MASK_SIZE           ~(MPR_MASK_GEN | MPR_MASK_FREE | MPR_MASK_MARK)

/**
    Memory Allocation Service.
    @description The MPR provides an application specific memory allocator to use instead of malloc. This allocator is 
    tailored to the needs of embedded applications and is faster than most general purpose malloc allocators. It is 
    deterministic and allocates and frees in constant time O(1). It exhibits very low fragmentation and accurate 
    coalescing.
    \n\n
    The allocator uses a garbage collector for freeing unused memory. The collector is a generational, cooperative,
    non-compacting, parallel collector.  The allocator is optimized for frequent allocations of small blocks (< 4K) 
    and uses a scheme of free queues for fast allocation. Allocations are aligned on 16 byte boundaries on 64-bit 
    systems and otherwise on 8 byte boundaries.  It will return chunks unused memory back to the O/S. 
    \n\n
    The allocator handles memory allocation errors globally. The application may configure a memory limit so that 
    memory depletion can be proactively detected and handled before memory allocations actually fail.
    \n\n
    A memory block that is being used must be marked as active to prevent the garbage collector from reclaiming it. 
    To mark a block as active, #mprMarkBlock must be called for each garbage collection cycle. When allocating non-temporal
    memroy blocks, a manager callback can be specified via #mprAllocObj. This manager routine will be called by the 
    collector so that dependant memory blocks can be marked as active.
    
    The collector performs the marking phase by invoking the manager routines for a set of root blocks. A block can be 
    added to the set of roots by calling #mprAddRoot. Each root's manager routine will mark other blocks which will cause
    their manager routines to run and so on, until all active blocks have been marked. Non-marked blocks can then safely
    be reclaimed as garbage. A block may alternatively be permanently marked as active by calling #mprHold.

    @stability Evolving
    @defgroup MprMem MprMem
    @see MprFreeMem MprHeap MprManager MprMemNotifier MprRegion mprAddRoot mprAlloc mprAllocMem mprAllocObj 
        mprAllocZeroed mprCreateMemService mprDestroyMemService mprEnableGC mprGetBlockSize mprGetMem 
        mprGetMemStats mprGetMpr mprGetPageSize mprHasMemError mprHold mprIsDead mprIsParent mprIsValid mprMark 
        mprMemcmp mprMemcpy mprMemdup mprPrintMem mprRealloc mprRelease mprRemoveRoot mprRequestGC mprResetMemError 
        mprRevive mprSetAllocLimits mprSetManager mprSetMemError mprSetMemLimits mprSetMemNotifier mprSetMemPolicy 
        mprSetName mprValidateBlock mprVerifyMem mprVirtAlloc mprVirtFree 
 */
typedef struct MprMem {
    /*
        Accesses to field1 must only be done while locked. This includes read access as concurrent writes may leave 
        field1 in a partially updated state. Access to field2 may be done while unlocked as only the marker updates 
        active blocks and it does so, in a lock-free manner.

            field1: prior | last << 1 | hasManager
            field2: gen/2 << 30 | isFree << 29 | size/29 | mark/2
     */
    size_t      field1;             /**< Pointer to adjacent, prior block in memory with last, manager fields */
    size_t      field2;             /**< Internal block length including header with gen and mark fields */

#if BIT_MEMORY_DEBUG
    uint            magic;          /* Unique signature */
    uint            seqno;          /* Allocation sequence number */
    cchar           *name;          /* Debug name */
#endif
} MprMem;


#define MPR_ALLOC_MIN_SPLIT         (32 + sizeof(MprMem))
#define MPR_ALLOC_ALIGN(x)          (((x) + MPR_ALIGN - 1) & ~(MPR_ALIGN - 1))
#define MPR_PAGE_ALIGN(x, psize)    ((((ssize) (x)) + ((ssize) (psize)) - 1) & ~(((ssize) (psize)) - 1))
#define MPR_PAGE_ALIGNED(x, psize)  ((((ssize) (x)) % ((ssize) (psize))) == 0)
#define MPR_ALLOC_MAGIC             0xe814ecab

/*
    The allocator free map is a two dimensional array of free queues. The first dimension is indexed by
    the most significant bits (group) set in the requested block size. The second dimension is the next 
    MPR_ALLOC_BUCKET_SHIFT (4) bits below the MSB.

    +-------------------------------+
    |   Group   |  Bucket   | rest  |
    +-------------------------------+
    | 0 | 0 | 1 | 1 | 1 | 1 | X | X |
    +-------------------------------+
 */
#define MPR_ALLOC_BUCKET_SHIFT      4
#define MPR_ALLOC_BITS_PER_GROUP    (sizeof(void*) * 8)
#define MPR_ALLOC_NUM_GROUPS        (MPR_ALLOC_BITS_PER_GROUP - MPR_ALLOC_BUCKET_SHIFT - MPR_ALIGN_SHIFT - 1)
#define MPR_ALLOC_NUM_BUCKETS       (1 << MPR_ALLOC_BUCKET_SHIFT)
#define MPR_GET_PTR(bp)             ((void*) (((char*) (bp)) + sizeof(MprMem)))
#define MPR_GET_MEM(ptr)            ((MprMem*) (((char*) (ptr)) - sizeof(MprMem)))
#define MPR_GET_GEN(mp)             ((mp->field2 & MPR_MASK_GEN) >> MPR_SHIFT_GEN)
#define MPR_GET_MARK(mp)            (mp->field2 & MPR_MASK_MARK)

/*
    GC Object generations
 */
#define MPR_GEN_ETERNAL             3           /**< Objects immune from collection */
#define MPR_MAX_GEN                 3           /**< Number of generations for object allocation */

/*
    Manager callback flags
 */
#define MPR_MANAGE_FREE             0x1         /**< Block being freed. Free dependant resources */
#define MPR_MANAGE_MARK             0x2         /**< Block being marked by GC. Mark dependant resources */

/*
    VirtAloc flags
 */
#if BIT_WIN_LIKE || VXWORKS
    #define MPR_MAP_READ            0x1
    #define MPR_MAP_WRITE           0x2
    #define MPR_MAP_EXECUTE         0x4
#else
    #define MPR_MAP_READ            PROT_READ
    #define MPR_MAP_WRITE           PROT_WRITE
    #define MPR_MAP_EXECUTE         PROT_EXEC
#endif

#if BIT_MEMORY_DEBUG
    #define MPR_CHECK_BLOCK(bp)     mprCheckBlock(bp)
    #define MPR_VERIFY_MEM()        if (MPR->heap->verify) { mprVerifyMem(); } else
#else
    #define MPR_CHECK_BLOCK(bp) 
    #define MPR_VERIFY_MEM()        
#endif

/*
    Memory depletion policy (mprSetAllocPolicy)
 */
#define MPR_ALLOC_POLICY_NOTHING    0       /**< Do nothing */
#define MPR_ALLOC_POLICY_PRUNE      1       /**< Prune all non-essential memory and continue */
#define MPR_ALLOC_POLICY_RESTART    2       /**< Gracefully restart the app if redline is exceeded */
#define MPR_ALLOC_POLICY_EXIT       3       /**< Exit the app if max exceeded with a MPR_EXIT_NORMAL exit */

/*
    MprMemNotifier cause argument
 */
#define MPR_MEM_REDLINE             0x1         /**< Memory use exceeds redline limit */
#define MPR_MEM_LIMIT               0x2         /**< Memory use exceeds memory limit - invoking policy */
#define MPR_MEM_FAIL                0x4         /**< Memory allocation failed - immediate exit */
#define MPR_MEM_TOO_BIG             0x4         /**< Memory allocation request is too big - immediate exit */

/**
    Memory allocation error callback. Notifiers are called if a low memory condition exists.
    @param policy Memory depletion policy. Set to one of MPR_ALLOC_POLICY_NOTHING, MPR_ALLOC_POLICY_PRUNE,
    MPR_ALLOC_POLICY_RESTART or MPR_ALLOC_POLICY_EXIT.  @param cause Cause of the memory allocation condition. If flags is
    set to MPR_MEM_LOW, the memory pool is low, but the allocation succeeded. If flags contain MPR_MEM_DEPLETED, the
    allocation failed.
    @param size Size of the allocation that triggered the low memory condition.
    @param total Total memory currently in use
    @ingroup MprMem
 */
typedef void (*MprMemNotifier)(int cause, int policy, ssize size, ssize total);

/**
    Mpr memory block manager prototype
    @param ptr Any memory context allocated by the MPR.
    @ingroup MprMem
 */
typedef void (*MprManager)(void *ptr, int flags);

/**
    Block structure when on a free list. This overlays MprMem and replaces sibling and children with forw/back
    The implies a minimum memory block size of 8 bytes in 32 bits and 16 bytes in 64 bits.
    @ingroup MemMem
 */
typedef struct MprFreeMem {
    union {
        //  8
        MprMem          blk;
        struct {
            int         minSize;            /**< Min size of block in queue */
            uint        count;              /**< Number of blocks on the queue */
        } stats;
    } info;
    struct MprFreeMem *next;                /**< Next free block */
    struct MprFreeMem *prev;                /**< Previous free block */
} MprFreeMem;


#if BIT_MEMORY_STATS
#define MPR_TRACK_HASH        2053          /* Size of location name hash */
#define MPR_TRACK_NAMES       8             /* Length of collision chain */

typedef struct MprLocationStats {
    ssize           count;                  /* Total allocations for this manager */
    cchar           *names[MPR_TRACK_NAMES];/* Manager names */
} MprLocationStats;
#endif


/**
    Memory allocator statistics
    @ingroup MemMem
  */
typedef struct MprMemStats {
    int             inMemException;         /**< Recursive protect */
    uint            errors;                 /**< Allocation errors */
    uint            numCpu;                 /**< Number of CPUs */
    uint            pageSize;               /**< System page size */
    ssize           bytesAllocated;         /**< Bytes currently allocated */
    ssize           bytesFree;              /**< Bytes currently free */
    ssize           freed;                  /**< Bytes freed in last sweep */
    ssize           redLine;                /**< Warn if allocation exceeds this level */
    ssize           maxMemory;              /**< Max memory that can be allocated */
    ssize           rss;                    /**< OS calculated resident stack size in bytes */
    int64           user;                   /**< System user RAM size in bytes (excludes kernel) */
    int64           ram;                    /**< System RAM size in bytes */
    int             markVisited;
    int             marked;
    int             sweepVisited;
    int             swept;

#if BIT_MEMORY_STATS
    /*
        Optional memory stats
     */
    uint64          allocs;                 /**< Count of times a block was split Calls to allocate memory from the O/S */
    uint64          joins;                  /**< Count of times a block was joined (coalesced) with its neighbours */
    uint64          requests;               /**< Count of memory requests */
    uint64          reuse;                  /**< Count of times a block was reused from a free queue */
    uint64          splits;                 /**< Count of times a block was split */
    uint64          unpins;                 /**< Count of times a block was unpinned and released back to the O/S */

    MprLocationStats locations[MPR_TRACK_HASH]; /* Per location allocation stats */
#endif
} MprMemStats;


/**
   Memmory regions allocated from the O/S
    @ingroup MemMem
 */
typedef struct MprRegion {
    struct MprRegion *next;                 /**< Next region */
    MprMem           *start;                /**< Start of region data */
    MprSpin          lock;                  /**< Region multithread lock */
    ssize            size;                  /**< Size of region including region header */
    int              freeable;              /**< Set to true when completely unused */
} MprRegion;


/**
    Memory allocator heap
    @ingroup MemMem
 */
typedef struct MprHeap {
    MprFreeMem       freeq[MPR_ALLOC_NUM_GROUPS * MPR_ALLOC_NUM_BUCKETS];
    ulong            bucketMap[MPR_ALLOC_NUM_GROUPS];
    MprFreeMem       *freeEnd;
    ssize            groupMap;
    struct MprList   *roots;                 /**< List of GC root objects */
    MprMemStats      stats;
    MprMemNotifier   notifier;               /**< Memory allocation failure callback */
    MprSpin          heapLock;               /**< Heap allocation lock */
    MprSpin          rootLock;               /**< Root locking */
    MprCond          *markerCond;            /**< Marker sleep cond var */
    MprMutex         *mutex;                 /**< Locking for state changes */
    MprRegion        *regions;               /**< List of memory regions */
    struct MprThread *marker;                /**< Marker thread */
    struct MprThread *sweeper;               /**< Optional sweeper thread */

    int              eternal;                /**< Eternal generation (permanent and dead blocks) */
    int              active;                 /**< Active generation for new and active blocks */
    int              stale;                  /**< Stale generation for blocks that may have no references*/
    int              dead;                   /**< Dead generation (blocks about to be freed) */

    int              allocPolicy;            /**< Memory allocation depletion policy */
    int              chunkSize;              /**< O/S memory allocation chunk size */
    int              collecting;             /**< Manual GC is running */
    int              destroying;             /**< Destroying the heap */
    int              enabled;                /**< GC is enabled */
    int              flags;                  /**< GC operational control flags */
    int              from;                   /**< Eligible mprCollectGarbage flags */
    int              gc;                     /**< GC has been requested */
    int              hasError;               /**< Memory allocation error */
    int              hasSweeper;             /**< Has dedicated sweeper thread */
    int              iteration;              /**< GC iteration counter (debug only) */
    int              marking;                /**< Actually marking objects now */
    int              mustYield;              /**< Threads must yield for GC which is due */
    int              newCount;               /**< Count of new gen allocations */
    int              earlyYieldQuota;        /**< Quota of new allocations before yielding threads early to cleanup */
    int              newQuota;               /**< Quota of new allocations before idle GC worthwhile */
    int              nextSeqno;              /**< Next sequence number */
    int              pauseGC;                /**< Pause GC (short) */
    int              pageSize;               /**< System page size */
    int              priorNewCount;          /**< Last sweep new count */
    ssize            priorFree;              /**< Last sweep free memory */
    int              rootIndex;              /**< Marker root scan index */
    int              scribble;               /**< Scribble over freed memory (slow) */
    int              track;                  /**< Track memory allocations */
    int              verify;                 /**< Verify memory contents (very slow) */
} MprHeap;

/**
    Create and initialize the Memory service
    @param manager Memory manager to manage the Mpr object
    @param flags Memory initialization control flags
    @return The Mpr control structure
    @ingroup MprMem
 */
extern struct Mpr *mprCreateMemService(MprManager manager, int flags);

/**
    Destroy the memory service. Called as the last thing before exiting
    @ingroup MprMem
 */
extern void mprDestroyMemService();

/*
    Flags for mprAllocMem
 */
#define MPR_ALLOC_MANAGER           0x1         /**< Reserve room for a manager */
#define MPR_ALLOC_ZERO              0x2         /**< Zero memory */
#define MPR_ALLOC_PAD_MASK          0x1         /**< Flags that impact padding */

/**
    Allocate a block of memory.
    @description This is the lowest level of memory allocation routine. Memory is freed via the garbage collector. 
    To protect an active memory block memory block from being reclaimed, it must have a reference to it. Memory blocks 
    can specify a manager routine via #mprAllocObj. The manager is is invoked by the garbage collector to "mark" 
    dependant active blocks. Marked blocks will not be reclaimed by the garbage collector.
    @param size Size of the memory block to allocate.
    @param flags Allocation flags. Supported flags include: MPR_ALLOC_MANAGER to reserve room for a manager callback and
        MPR_ALLOC_ZERO to zero allocated memory.
    @return Returns a pointer to the allocated block. If memory is not available the memory exhaustion handler 
        specified via mprCreate will be called to allow global recovery.
    @remarks Do not mix calls to malloc and mprAlloc.
    @ingroup MprMem
 */
extern void *mprAllocMem(ssize size, int flags);

/**
    Return the current allocation memory statistics block
    @returns a reference to the allocation memory statistics. Do not modify its contents.
    @ingroup MprMem
 */
extern MprMemStats *mprGetMemStats();

/**
    Return the amount of memory currently used by the application. On Unix, this returns the total application memory
    size including code, stack, data and heap. On Windows, VxWorks and other operatings systems, it returns the
    amount of allocated heap memory.
    @returns the amount of memory used by the application in bytes.
    @ingroup MprMem
 */
extern ssize mprGetMem();

/**
    Get the current O/S virtual page size
    @returns the page size in bytes
    @ingroup MprMem
 */
extern int mprGetPageSize();

/**
    Get the allocated size of a memory block
    @param ptr Any memory allocated by mprAlloc
    @returns the block size in bytes
    @ingroup MprMem
 */
extern ssize mprGetBlockSize(cvoid *ptr);

/**
    Determine if the MPR has encountered memory allocation errors.
    @description Returns true if the MPR has had a memory allocation error. Allocation errors occur if any
        memory allocation would cause the application to exceed the configured redline limit, or if any O/S memory
        allocation request fails.
    @return TRUE if a memory allocation error has occurred. Otherwise returns FALSE.
    @ingroup MprMem
 */
extern bool mprHasMemError();

/*
    Test if a memory block is unreferenced by the last garbage collection sweep.
    @param ptr Reference to an allocated memory block.
    @return TRUE if the given memory block is unreferenced and ready for collection.
    @internal
    @ingroup MprMem
 */
extern int mprIsDead(cvoid* ptr);

/**
    Test is a pointer is a valid memory context. This is used to test if a block has been dynamically allocated.
    @param ptr Any memory context allocated by mprAlloc or mprCreate.
    @ingroup MprMem
 */
extern int mprIsValid(cvoid *ptr);

/**
    Compare two byte strings.
    @description Safely compare two byte strings. This is a safe replacement for memcmp.
    @param b1 Pointer to the first byte string.
    @param b1Len Length of the first byte string.
    @param b2 Pointer to the second byte string.
    @param b2Len Length of the second byte string.
    @return Returns zero if the byte strings are identical. Otherwise returns -1 if the first string is less than the 
        second. Returns 1 if the first is greater than the first.
    @ingroup MprMem
 */
extern int mprMemcmp(cvoid *b1, ssize b1Len, cvoid *b2, ssize b2Len);

/**
    Safe copy for a block of data.
    @description Safely copy a block of data into an existing memory block. The call ensures the destination 
        block is not overflowed and returns the size of the block actually copied. This is similar to memcpy, but 
        is a safer alternative.
    @param dest Pointer to the destination block.
    @param destMax Maximum size of the destination block.
    @param src Block to copy
    @param nbytes Size of the source block
    @return Returns the number of characters in the allocated block.
    @ingroup MprMem
 */
extern ssize mprMemcpy(void *dest, ssize destMax, cvoid *src, ssize nbytes);

/**
    Duplicate a block of memory.
    @description Copy a block of memory into a newly allocated block.
    @param ptr Pointer to the block to duplicate.
    @param size Size of the block to copy.
    @return Returns an allocated block.
    @ingroup MprMem
 */
extern void *mprMemdup(cvoid *ptr, ssize size);

/**
    Print a memory usage report to stdout
    @param msg Prefix message to the report
    @param detail If true, print free queue detail report
    @ingroup MprMem
 */
extern void mprPrintMem(cchar *msg, int detail);

/**
    Reallocate a block
    @description Reallocates a block increasing its size. If the specified size is less than the current block size,
        the call will ignore the request and simply return the existing block. The memory is not zeroed.
    @param ptr Memory to reallocate. If NULL, call malloc.
    @param size New size of the required memory block.
    @return Returns a pointer to the allocated block. If memory is not available the memory exhaustion handler 
        specified via mprCreate will be called to allow global recovery.
    @remarks Do not mix calls to realloc and mprRealloc.
    @ingroup MprMem
 */
extern void *mprRealloc(void *ptr, ssize size);

/**
    Reset the memory allocation error flag
    @description Reset the alloc error flag triggered.
    @ingroup MprMem
 */
extern void mprResetMemError();

/*
    Revive a memory block scheduled for collection. This should only ever be called in the manager routine for a block
    when the manage flags parameter is set to MPR_MANAGE_FREE. Reviving a block aborts its collection.
    @param ptr Reference to an allocated memory block.
    @internal
 */
extern void mprRevive(cvoid* ptr);

/**
    Verify all memory. This checks the integrity of all memory blocks by verifying the block headers and contents
    of all free memory blocks. Will only do anything meaningful when the product is compiled in debug mode.
    @ingroup MprMem
 */
extern void mprVerifyMem();

/**
    Define a memory notifier
    @description A notifier callback will be invoked for memory allocation errors for the given memory context.
    @param cback Notifier callback function
    @ingroup MprMem
 */
extern void mprSetMemNotifier(MprMemNotifier cback);

/**
    Set an memory allocation error condition on a memory context. This will set an allocation error condition on the
    given context and all its parents. This way, you can test the ultimate parent and detect if any memory allocation
    errors have occurred.
    @ingroup MprMem
 */
extern void mprSetMemError();

/**
    Configure the application memory limits
    @description Configure memory limits to constrain memory usage by the application. The memory allocation subsystem
        will check these limits before granting memory allocation requrests. The redLine is a soft limit that if exceeded
        will invoke the memory allocation callback, but will still honor the request. The maxMemory limit is a hard limit.
        The MPR will prevent allocations which exceed this maximum. The memory callback handler is defined via 
        the #mprCreate call.
    @param redline Soft memory limit. If exceeded, the request will be granted, but the memory handler will be invoked.
    @param maxMemory Hard memory limit. If exceeded, the request will not be granted, and the memory handler will be invoked.
    @ingroup MprMem
 */
extern void mprSetMemLimits(ssize redline, ssize maxMemory);

/**
    Set the memory allocation policy for when allocations fail.
    @param policy Set to MPR_ALLOC_POLICY_EXIT for the application to immediately exit on memory allocation errors.
        Set to MPR_ALLOC_POLICY_RESTART to restart the appplication on memory allocation errors.
*/
extern void mprSetMemPolicy(int policy);

/**
    Update the manager for a block of memory.
    @description This call updates the manager for a block of memory allocated via mprAllocWithManager.
    @param ptr Memory to free. If NULL, take no action.
    @param manager Manager function to invoke when the memory is released.
    @return Returns the original object
    @ingroup MprMem
 */
extern void *mprSetManager(void *ptr, MprManager manager);

/**
    Validate a memory block and issue asserts if the memory block is not valid.
    @param ptr Pointer to allocated memory
    @ingroup MprMem
 */
extern void mprValidateBlock(void *ptr);

/**
    Memory virtual memory into the applications address space.
    @param size of virtual memory to map. This size will be rounded up to the nearest page boundary.
    @param mode Mask set to MPR_MAP_READ | MPR_MAP_WRITE
    @ingroup MprMem
 */
extern void *mprVirtAlloc(ssize size, int mode);

/**
    Free (unpin) a mapped section of virtual memory
    @param ptr Virtual address to free. Should be page aligned
    @param size Size of memory to free in bytes
    @ingroup MprMem
 */
extern void mprVirtFree(void *ptr, ssize size);

/*
    Macros. When building documentation (DOXYGEN), define pretend function defintions for the documentation.
 */
/*
    In debug mode, all memory blocks can have a debug name
 */
#if BIT_MEMORY_DEBUG
    extern void *mprSetName(void *ptr, cchar *name);
    extern void *mprCopyName(void *dest, void *src);
    #define mprGetName(ptr) (MPR_GET_MEM(ptr)->name)
    extern void *mprSetAllocName(void *ptr, cchar *name);
#else
    #define mprCopyName(dest, src)
    #define mprGetName(ptr) ""
    #define mprSetAllocName(ptr, name) ptr
    #define mprSetName(ptr, name)
#endif

#define mprAlloc(size) mprSetAllocName(mprAllocMem(size, 0), MPR_LOC)
#define mprMemdup(ptr, size) mprSetAllocName(mprMemdupMem(ptr, size), MPR_LOC)
#define mprRealloc(ptr, size) mprSetAllocName(mprReallocMem(ptr, size), MPR_LOC)
#define mprAllocZeroed(size) mprSetAllocName(mprAllocMem(size, MPR_ALLOC_ZERO), MPR_LOC)
#define mprAllocBlock(size, flags) mprSetAllocName(mprAllocMem(size, flags), MPR_LOC)
#define mprAllocObj(type, manage) \
    ((type*) mprSetManager( \
        mprSetAllocName(mprAllocMem(sizeof(type), MPR_ALLOC_MANAGER | MPR_ALLOC_ZERO), #type "@" MPR_LOC), (MprManager) manage))
#define mprAllocStruct(type) ((type*) mprSetAllocName(mprAllocMem(sizeof(type), MPR_ALLOC_ZERO), #type "@" MPR_LOC))

#if DOXYGEN
typedef void *Type;
/**
    Allocate a block of memory
    @description Allocates a block of memory of the required size. The memory is not zeroed.
    @param size Size of the memory block to allocate.
    @return Returns a pointer to the allocated block. If memory is not available the memory exhaustion handler 
        specified via mprCreate will be called to allow global recovery.
    @remarks Do not mix calls to malloc and mprAlloc.
    @ingroup MprMem
 */
extern void *mprAlloc(ssize size);

/**
    Allocate an object of a given type.
    @description Allocates a zeroed block of memory large enough to hold an instance of the specified type with a 
        manager callback. This call associates a manager function with an object that will be invoked when the 
        object is freed or the garbage collector needs the object to mark internal properties as being used.  
        This call is implemented as a macro.
    @param type Type of the object to allocate
    @param manager Manager function to invoke when the allocation is managed.
    @return Returns a pointer to the allocated block. If memory is not available the memory exhaustion handler 
        specified via mprCreate will be called to allow global recovery.
    @remarks Do not mix calls to malloc and mprAlloc.
    @stability Evolving. This function names are highly likely to be refactored.
    @ingroup MprMem
 */
extern void *mprAllocObj(Type type, MprManager manager) { return 0;}

/**
    Allocate a zeroed block of memory
    @description Allocates a zeroed block of memory.
    @param size Size of the memory block to allocate.
    @return Returns a pointer to the allocated block. If memory is not available the memory exhaustion handler 
        specified via mprCreate will be called to allow global recovery.
    @remarks Do not mix calls to malloc and mprAlloc.
    @ingroup MprMem
 */
extern void *mprAllocZeroed(ssize size);

#else /* !DOXYGEN */
extern void *mprAllocMem(ssize size, int flags);
extern void *mprReallocMem(void *ptr, ssize size);
extern void *mprMemdupMem(cvoid *ptr, ssize size);
extern void mprCheckBlock(MprMem *bp);
#endif

/*
    Internal APIs
 */
extern void mprStartGCService();
extern void mprStopGCService();

/******************************** Garbage Coolector ***************************/
/**
    Add a memory block as a root for garbage collection
    @param ptr Any memory pointer
    @ingroup MprMem
 */
extern void mprAddRoot(void *ptr);

#define MPR_FORCE_GC        0x1     /* Force a GC whether it is required or not */
#define MPR_COMPLETE_GC     0x2     /* Do a complete collection (3 sweeps) */
#define MPR_WAIT_GC         0x4     /* Wait for GC to complete */

/**
    Collect garbage
    @description Initiates garbage collection to free unreachable memory blocks. This call may return before collection 
    is complete if garbage collection has been configured via mprCreate() to use dedicated threads for collection. 
    A single garbage collection may not free all memory. Use mprRequestGC(1) to free all unused memory blocks.
    @param flags Flags to control the collection. Set flags to MPR_GC_FORCE to force one sweep. Set to zero
    to perform a conditional sweep where the sweep is only performed if there is sufficient garbage to warrant a collection.
    Other flags include MPR_GC_FROM_EVENTS which must be specified if calling mprCollectGarbage from a routine that 
    also blocks on mprServiceEvents. Similarly, use MPR_GC_FROM_OWN if managing garbage collections manually.
    @ingroup MprMem
  */
extern void mprRequestGC(int flags);

/**
    Enable or disable the garbage collector
    @param on Set to one to enable and zero to disable.
    @return Returns one if the collector was previously enabled. Otherwise returns zero.
    @ingroup MprMem
 */
extern bool mprEnableGC(bool on);

/**
    Hold a memory block
    @description This call will protect a memory block from freeing by the garbage collector. Call mprRelease to
        allow the block to be collected.
    @param ptr Any memory block
    @ingroup MprMem
  */
extern void mprHold(void *ptr);

/**
    Mark a block as "in-use" for the Garbage Collector.
    The MPR memory garbage collector requires that all allocated memory be marked as in-use during a garbage collection
    sweep. When a memory block is allocated, it may provide a "manage" callback function that will be invoked during
    garbage collection so the block can be marked as "in-use".
    @param ptr Reference to the block to mark as currently being used.
 */
extern void mprMarkBlock(cvoid *ptr);

/**
    Release a memory block
    @description This call is used to allow a memory block to be freed by the garbage collector after calling
        mprHold.
    @param ptr Any memory block
    @ingroup MprMem
  */
extern void mprRelease(void *ptr);

/**
    remove a memory block as a root for garbage collection
    @param ptr Any memory pointer
    @ingroup MprMem
  */
extern void mprRemoveRoot(void *ptr);

#if DOXYGEN
    /**
        Mark a memory block as in-use
        @description To prevent a memory block being freed by the garbage collector, it must be marked as in-use. This
            routine will mark a memory block as being used. 
     */
    extern void mprMark(void* ptr);
    @ingroup MprMem
#else
    #define mprMark(ptr) if (ptr) { mprMarkBlock(ptr); } else
#endif

#if FUTURE
#define void mprMark(cvoid *ptr) \
    if (ptr) { \
        MprMem *mp = MPR_GET_MEM(ptr); \
        if (mp->mark != heapActive) { \
            mp->gen = mp->mark = heapActive; \
            if (mp->hasManager) { \
                ((MprManager)(((char*) (mp)) + mp->size - sizeof(void*)))(ptr, MPR_MANAGE_MARK); \
            } \
        } \
    } else 
#endif

/*
    Internal
 */
extern int  mprCreateGCService();
extern void mprWakeGCService();
extern void mprResumeThreads();
extern int  mprSyncThreads(MprTime timeout);

/********************************** Safe Strings ******************************/
/**
    Safe String Module
    @description The MPR provides a suite of safe ascii string manipulation routines to help prevent buffer overflows
        and other potential security traps.
    @defgroup MprString MprString
    @see MprString itos itosradix itosbuf mprPrintf mprPrintfError mprSprintf scamel scaselesscmp scaselessmatch schr 
        sclone scmp scontains scopy sends sfmt sfmtv shash shashlower sjoin sjoinv slen slower smatch sncaselesscmp snclone
        sncmp sncopy snumber spascal spbrk srchr srejoin srejoinv sreplace sspn sstarts ssub stemplate stoi stoiradix
        stok strim supper sncontains mprFprintf mprSprintfv
 */
typedef struct MprString { void *dummy; } MprString;

/**
    Convert an integer to a string.
    @description This call converts the supplied 64 bit integer to a string using base 10.
    @param value Integer value to convert
    @return An allocated string with the converted number.
    @ingroup MprString
 */
extern char *itos(int64 value);

/**
    Convert an integer to a string.
    @description This call converts the supplied 64 bit integer to a string according to the specified radix.
    @param value Integer value to convert
    @param radix The base radix to use when encoding the number
    @return An allocated string with the converted number.
    @ingroup MprString
 */
extern char *itosradix(int64 value, int radix);

/**
    Convert an integer to a string buffer.
    @description This call converts the supplied 64 bit integer into a string formatted into the supplied buffer according
        to the specified radix.
    @param buf Pointer to the buffer that will hold the string.
    @param size Size of the buffer.
    @param value Integer value to convert
    @param radix The base radix to use when encoding the number
    @return Returns a reference to the string.
    @ingroup MprString
 */
extern char *itosbuf(char *buf, ssize size, int64 value, int radix);

/**
    Compare strings ignoring case. This is a safe replacement for strcasecmp. It can handle NULL args.
    @description Compare two strings ignoring case differences. This call operates similarly to strcmp.
    @param s1 First string to compare.
    @param s2 Second string to compare. 
    @return Returns zero if the strings are equivalent, < 0 if s1 sorts lower than s2 in the collating sequence 
        or > 0 if it sorts higher.
    @ingroup MprString
 */
extern int scaselesscmp(cchar *s1, cchar *s2);

/**
    Compare strings ignoring case. This is similar to scaselesscmp but it returns a boolean.
    @description Compare two strings ignoring case differences.
    @param s1 First string to compare.
    @param s2 Second string to compare. 
    @return Returns true if the strings are equivalent, otherwise false.
    @ingroup MprString
 */
extern bool scaselessmatch(cchar *s1, cchar *s2);

/**
    Create a camel case version of the string
    @description Copy a string into a newly allocated block and make the first character lower case
    @param str Pointer to the block to duplicate.
    @return Returns a newly allocated string.
    @ingroup MprMem
 */
extern char *scamel(cchar *str);

/**
   Find a character in a string. 
   @description This is a safe replacement for strchr. It can handle NULL args.
   @param str String to examine
   @param c Character to search for
   @return If the character is found, the call returns a reference to the character position in the string. Otherwise,
        returns NULL.
    @ingroup MprString
 */
extern char *schr(cchar *str, int c);

/**
    Clone a string.
    @description Copy a string into a newly allocated block.
    @param str Pointer to the block to duplicate.
    @return Returns a newly allocated string.
    @ingroup MprMem
 */
extern char *sclone(cchar *str);

/**
    Compare strings.
    @description Compare two strings. This is a safe replacement for strcmp. It can handle null args.
    @param s1 First string to compare.
    @param s2 Second string to compare.
    @return Returns zero if the strings are identical. Return -1 if the first string is less than the second. Return 1
        if the first string is greater than the second.
    @ingroup MprString
 */
extern int scmp(cchar *s1, cchar *s2);

/**
    Find a pattern in a string.
    @description Locate the first occurrence of pattern in a string.
    @param str Pointer to the string to search.
    @param pattern String pattern to search for.
    @return Returns a reference to the start of the pattern in the string. If not found, returns NULL.
    @ingroup MprString
 */
extern char *scontains(cchar *str, cchar *pattern);

/**
    Copy a string.
    @description Safe replacement for strcpy. Copy a string and ensure the destination buffer is not overflowed. 
        The call returns the length of the resultant string or an error code if it will not fit into the target
        string. This is similar to strcpy, but it will enforce a maximum size for the copied string and will 
        ensure it is always terminated with a null.
    @param dest Pointer to a pointer that will hold the address of the allocated block.
    @param destMax Maximum size of the target string in characters.
    @param src String to copy
    @return Returns the number of characters in the target string.
    @ingroup MprString
 */
extern ssize scopy(char *dest, ssize destMax, cchar *src);

/**
    Test if the string ends with a given pattern.
    @param str String to examine
    @param suffix Pattern to search for
    @return Returns TRUE if the pattern was found. Otherwise returns zero.
    @ingroup MprString
 */
extern bool sends(cchar *str, cchar *suffix);

/**
    Format a string. This is a secure verion of printf that can handle null args.
    @description Format the given arguments according to the printf style format. See mprPrintf for a full list of the
        format specifies. This is a secure replacement for sprintf, it can handle null arguments without crashes.
    @param fmt Printf style format string
    @param ... Variable arguments for the format string
    @return Returns a newly allocated string
    @ingroup MprString
 */
extern char *sfmt(cchar *fmt, ...);

/**
    Format a string. This is a secure verion of printf that can handle null args.
    @description Format the given arguments according to the printf style format. See mprPrintf for a full list of the
        format specifies. This is a secure replacement for sprintf, it can handle null arguments without crashes.
    @param fmt Printf style format string
    @param args Varargs argument obtained from va_start.
    @return Returns a newly allocated string
    @ingroup MprString
 */
extern char *sfmtv(cchar *fmt, va_list args);

/**
    Compute a hash code for a string
    @param str String to examine
    @param len Length in characters of the string to include in the hash code
    @return Returns an unsigned integer hash code
    @ingroup MprString
 */
extern uint shash(cchar *str, ssize len);

/**
    Compute a caseless hash code for a string
    @description This computes a hash code for the string after converting it to lower case.
    @param str String to examine
    @param len Length in characters of the string to include in the hash code
    @return Returns an unsigned integer hash code
    @ingroup MprString
 */
extern uint shashlower(cchar *str, ssize len);

/**
    Catenate strings.
    @description This catenates strings together with an optional string separator.
        If the separator is NULL, not separator is used. This call accepts a variable list of strings to append, 
        terminated by a null argument. 
    @param str First string to catentate
    @param ... Variable number of string arguments to append. Terminate list with NULL.
    @return Returns an allocated string.
    @ingroup MprString
 */
extern char *sjoin(cchar *str, ...);

/**
    Catenate strings.
    @description This catenates strings together with an optional string separator.
        If the separator is NULL, not separator is used. This call accepts a variable list of strings to append, 
        terminated by a null argument. 
    @param str First string to catentate
    @param args Varargs argument obtained from va_start.
    @return Returns an allocated string.
    @ingroup MprString
 */
extern char *sjoinv(cchar *str, va_list args);

/**
    Return the length of a string.
    @description Safe replacement for strlen. This call returns the length of a string and tests if the length is 
        less than a given maximum. It will return zero for NULL args.
    @param str String to measure.
    @return Returns the length of the string
    @ingroup MprString
 */
extern ssize slen(cchar *str);

/**
    Convert a string to lower case. 
    @description Convert a string to its lower case equivalent.
    @param str String to convert.
    @ingroup MprString
 */
extern char *slower(cchar *str);

/**
    Compare strings.
    @description Compare two strings. This is similar to #scmp but it returns a boolean.
    @param s1 First string to compare.
    @param s2 Second string to compare.
    @return Returns true if the strings are equivalent, otherwise false.
    @ingroup MprString
 */
extern bool smatch(cchar *s1, cchar *s2);

/**
    Compare strings ignoring case.
    @description Compare two strings ignoring case differences for a given string length. This call operates 
        similarly to strncasecmp.
    @param s1 First string to compare.
    @param s2 Second string to compare.
    @param len Length of characters to compare.
    @return Returns zero if the strings are equivalent, < 0 if s1 sorts lower than s2 in the collating sequence 
        or > 0 if it sorts higher.
    @ingroup MprString
 */
extern int sncaselesscmp(cchar *s1, cchar *s2, ssize len);

/**
    Clone a substring.
    @description Copy a substring into a newly allocated block.
    @param str Pointer to the block to duplicate.
    @param len Number of bytes to copy. The actual length copied is the minimum of the given length and the length of
        the supplied string. The result is null terminated.
    @return Returns a newly allocated string.
    @ingroup MprMem
 */
extern char *snclone(cchar *str, ssize len);

/**
    Compare strings.
    @description Compare two strings for a given string length. This call operates similarly to strncmp.
    @param s1 First string to compare.
    @param s2 Second string to compare.
    @param len Length of characters to compare.
    @return Returns zero if the strings are equivalent, < 0 if s1 sorts lower than s2 in the collating sequence 
        or > 0 if it sorts higher.
    @ingroup MprString
 */
extern int sncmp(cchar *s1, cchar *s2, ssize len);

/**
    Find a pattern in a string with a limit.
    @description Locate the first occurrence of pattern in a string, but do not search more than the given character limit. 
    @param str Pointer to the string to search.
    @param pattern String pattern to search for.
    @param limit Count of characters in the string to search.
    @return Returns a reference to the start of the pattern in the string. If not found, returns NULL.
    @ingroup MprString
 */
extern char *sncontains(cchar *str, cchar *pattern, ssize limit);

/**
    Copy characters from a string.
    @description Safe replacement for strncpy. Copy bytes from a string and ensure the target string is not overflowed. 
        The call returns the length of the resultant string or an error code if it will not fit into the target
        string. This is similar to strcpy, but it will enforce a maximum size for the copied string and will 
        ensure it is terminated with a null.
    @param dest Pointer to a pointer that will hold the address of the allocated block.
    @param destMax Maximum size of the target string in characters.
    @param src String to copy
    @param len Maximum count of characters to copy
    @return Returns a reference to the destination if successful or NULL if the string won't fit.
    @ingroup MprString
 */
extern ssize sncopy(char *dest, ssize destMax, cchar *src, ssize len);

/*
    Test if a string is a radix 10 number.
    @return true if all characters are digits
 */
extern bool snumber(cchar *s);

/**
    Create a Pascal case version of the string
    @description Copy a string into a newly allocated block and make the first character upper case
    @param str Pointer to the block to duplicate.
    @return Returns a newly allocated string.
    @ingroup MprMem
 */
extern char *spascal(cchar *str);

/**
    Locate the a character in a string.
    @description This locates in the string the first occurence of any character from a given set of characters.
    @param str String to examine
    @param set Set of characters to scan for
    @return Returns a reference to the first character from the given set. Returns NULL if none found.
    @ingroup MprString
  */
extern char *spbrk(cchar *str, cchar *set);

/**
    Find a character in a string by searching backwards.
    @description This locates in the string the last occurence of a character.
    @param str String to examine
    @param c Character to scan for
    @return Returns a reference in the string to the requested character. Returns NULL if none found.
    @ingroup MprString
  */
extern char *srchr(cchar *str, int c);

/**
    Append strings to an existing string and reallocate as required.
    @description Append a list of strings to an existing string. The list of strings is terminated by a 
        null argument. The call returns the size of the allocated block. 
    @param buf Existing (allocated) string to reallocate. May be null. May not be a string literal.
    @param ... Variable number of string arguments to append. Terminate list with NULL
    @return Returns an allocated string.
    @ingroup MprString
 */
extern char *srejoin(char *buf, ...);

/**
    Append strings to an existing string and reallocate as required.
    @description Append a list of strings to an existing string. The list of strings is terminated by a 
        null argument. The call returns the size of the allocated block. 
    @param buf Existing (allocated) string to reallocate. May be null. May not be a string literal.
    @param args Varargs argument obtained from va_start.
    @return Returns an allocated string.
    @ingroup MprString
 */
extern char *srejoinv(char *buf, va_list args);

/*
    Replace a pattern in a string
    @param str
    @param pattern
    @param replacmenet
    @return
 */
extern char *sreplace(cchar *str, cchar *pattern, cchar *replacement);

/**
    Find the end of a spanning prefix
    @description This scans the given string for characters from the set and returns a reference to the 
    first character not in the set.
    @param str String to examine
    @param set Set of characters to span
    @return Returns a reference to the first character after the spanning set.
    @ingroup MprString
  */
extern ssize sspn(cchar *str, cchar *set);

/**
    Test if the string starts with a given pattern.
    @param str String to examine
    @param prefix Pattern to search for
    @return Returns TRUE if the pattern was found. Otherwise returns zero.
 */
extern bool sstarts(cchar *str, cchar *prefix);

/**
    Replace template tokens in a string with values from a lookup table. Tokens are ${variable} references. 
    @param str String to expand
    @param tokens Hash table of token values to use
    @return An expanded string. May return the original string if no "$" references are present.
  */
extern char *stemplate(cchar *str, struct MprHash *tokens);

/**
    Convert a string to an integer.
    @description This call converts the supplied string to an integer using base 10.
    @param str Pointer to the string to parse.
    @return Returns the integer equivalent value of the string. 
    @ingroup MprString
 */
extern int64 stoi(cchar *str);

/**
    Convert a string to an integer.
    @description This call converts the supplied string to an integer using the specified radix (base).
    @param str Pointer to the string to parse.
    @param radix Base to use when parsing the string
    @param err Return error code. Set to 0 if successful.
    @return Returns the integer equivalent value of the string. 
    @ingroup MprString
 */
extern int64 stoiradix(cchar *str, int radix, int *err);

/**
    Tokenize a string
    @description Split a string into tokens.
    @param str String to tokenize.
    @param delim String of characters to use as token separators.
    @param last Last token pointer.
    @return Returns a pointer to the next token.
    @ingroup MprString
 */
extern char *stok(char *str, cchar *delim, char **last);

/**
    Create a substring
    @param str String to examine
    @param offset Starting offset within str for the beginning of the substring
    @param length Length of the substring in characters
    @return Returns a newly allocated substring
    @ingroup MprString
 */
extern char *ssub(cchar *str, ssize offset, ssize length);

/*
    String trim flags
 */
#define MPR_TRIM_START  0x1             /**< Flag for #strim to trim from the start of the string */
#define MPR_TRIM_END    0x2             /**< Flag for #strim to trim from the end of the string */
#define MPR_TRIM_BOTH   0x3             /**< Flag for #strim to trim from both the start and the end of the string */

/**
    Trim a string.
    @description Trim leading and trailing characters off a string.
    @param str String to trim.
    @param set String of characters to remove.
    @param where Flags to indicate trim from the start, end or both. Use MPR_TRIM_START, MPR_TRIM_END, MPR_TRIM_BOTH.
    @return Returns a pointer to the trimmed string. May not equal \a str.
    @ingroup MprString
 */
extern char *strim(cchar *str, cchar *set, int where);

/**
    Convert a string to upper case.
    @description Convert a string to its upper case equivalent.
    @param str String to convert.
    @return Returns a pointer to the converted string. Will always equal str.
    @ingroup MprString
 */
extern char *supper(cchar *str);

/************************************ Unicode *********************************/
/*
    Low-level unicode wide string support. Unicode characters are build-time configurable to be 1, 2 or 4 bytes

    This API is not yet public
 */
extern MprChar *amtow(cchar *src, ssize *len);
extern char    *awtom(MprChar *src, ssize *len);
extern MprChar *wfmt(MprChar *fmt, ...);

#if BIT_CHAR_LEN > 1
extern ssize   wtom(char *dest, ssize count, MprChar *src, ssize len);
extern ssize   mtow(MprChar *dest, ssize count, cchar *src, ssize len);

extern MprChar  *itow(MprChar *buf, ssize bufCount, int64 value, int radix);
extern MprChar  *wchr(MprChar *s, int c);
extern int      wcasecmp(MprChar *s1, MprChar *s2);
extern MprChar  *wclone(MprChar *str);
extern int      wcmp(MprChar *s1, MprChar *s2);
extern MprChar  *wcontains(MprChar *str, MprChar *pattern, ssize limit);
extern ssize    wcopy(MprChar *dest, ssize destMax, MprChar *src);
extern int      wends(MprChar *str, MprChar *suffix);
extern MprChar  *wfmtv(MprChar *fmt, va_list arg);
extern uint     whash(MprChar *name, ssize len);
extern uint     whashlower(MprChar *name, ssize len);
extern MprChar  *wjoin(MprChar *sep, ...);
extern MprChar  *wjoinv(MprChar *sep, va_list args);
extern ssize    wlen(MprChar *s);

extern MprChar  *wlower(MprChar *s);
extern int      wncaselesscmp(MprChar *s1, MprChar *s2, ssize len);
extern int      wncmp(MprChar *s1, MprChar *s2, ssize len);
extern ssize    wncopy(MprChar *dest, ssize destCount, MprChar *src, ssize len);
extern MprChar  *wpbrk(MprChar *str, MprChar *set);
extern MprChar  *wrchr(MprChar *s, int c);
extern MprChar  *wrejoin(MprChar *buf, MprChar *sep, ...);
extern MprChar  *wrejoinv(MprChar *buf, MprChar *sep, va_list args);
extern ssize    wspn(MprChar *str, MprChar *set);
extern int      wstarts(MprChar *str, MprChar *prefix);
extern MprChar  *wsub(MprChar *str, ssize offset, ssize len);
extern int64    wtoi(MprChar *str);
extern int64    wtoiradix(MprChar *str, int radix, int *err);
extern MprChar  *wtok(MprChar *str, MprChar *delim, MprChar **last);
extern MprChar  *wtrim(MprChar *str, MprChar *set, int where);
extern MprChar  *wupper(MprChar *s);
#else

/* CHAR_LEN == 1 */
#define wtom(dest, count, src, len)         sncopy(dest, count, src, len)
#define mtow(dest, count, src, len)         sncopy(dest, count, src, len)
#define itowbuf(buf, bufCount, value, radix) itosbuf(buf, bufCount, value, radix)
#define wchr(str, c)                        schr(str, c)
#define wclone(str)                         sclone(str)
#define wcasecmp(s1, s2)                    scaselesscmp(s1, s2)
#define wcmp(s1, s2)                        scmp(s1, s2)
#define wcontains(str, pattern)             scontains(str, pattern)
#define wncontains(str, pattern, limit)     sncontains(str, pattern, limit)
#define wcopy(dest, count, src)             scopy(dest, count, src)
#define wends(str, suffix)                  sends(str, suffix)
#define wfmt                                sfmt
#define wfmtv(fmt, arg)                     sfmtv(fmt, arg)
#define whash(name, len)                    shash(name, len)
#define whashlower(name, len)               shashlower(name, len)
#define wjoin                               sjoin
#define wjoinv(sep, args)                   sjoinv(sep, args)
#define wlen(str)                           slen(str)
#define wlower(str)                         slower(str)
#define wncmp(s1, s2, len)                  sncmp(s1, s2, len)
#define wncaselesscmp(s1, s2, len)          sncaselesscmp(s1, s2, len)
#define wncopy(dest, count, src, len)       sncopy(dest, count, src, len)
#define wpbrk(str, set)                     spbrk(str, set)
#define wrchr(str, c)                       srchr(str, c)
#define wrejoin                             srejoin
#define wrejoinv(buf, sep, args)            srejoinv(buf, sep, args)
#define wspn(str, set)                      sspn(str, set)
#define wstarts(str, prefix)                sstarts(str, prefix)
#define wsub(str, offset, len)              ssub(str, offset, len)
#define wtoi(str)                           stoi(str)
#define wtoiradix(str, radix, err)          stoiradix(str, radix, err)
#define wtok(str, delim, last)              stok(str, delim, last)
#define wtrim(str, set, where)              strim(str, set, where)
#define wupper(str)                         supper(str)

#endif /* BIT_CHAR_LEN > 1 */

/********************************* Mixed Strings ******************************/
/*
    These routines operate on wide strings mixed with a multibyte/ascii operand
    This API is not yet public
 */
#if BIT_CHAR_LEN > 1
extern int      mcasecmp(MprChar *s1, cchar *s2);
extern int      mcmp(MprChar *s1, cchar *s2);
extern MprChar *mcontains(MprChar *str, cchar *pattern);
extern MprChar *mncontains(MprChar *str, cchar *pattern, ssize limit);
extern ssize   mcopy(MprChar *dest, ssize destMax, cchar *src);
extern int      mends(MprChar *str, cchar *suffix);
extern MprChar *mfmt(cchar *fmt, ...);
extern MprChar *mfmtv(cchar *fmt, va_list arg);
extern MprChar *mjoin(cchar *sep, ...);
extern MprChar *mjoinv(cchar *sep, va_list args);
extern int      mncmp(MprChar *s1, cchar *s2, ssize len);
extern int      mncaselesscmp(MprChar *s1, cchar *s2, ssize len);
extern ssize   mncopy(MprChar *dest, ssize destMax, cchar *src, ssize len);
extern MprChar *mpbrk(MprChar *str, cchar *set);
extern MprChar *mrejoin(MprChar *buf, cchar *sep, ...);
extern MprChar *mrejoinv(MprChar *buf, cchar *sep, va_list args);
extern ssize   mspn(MprChar *str, cchar *set);
extern int      mstarts(MprChar *str, cchar *prefix);
extern MprChar *mtok(MprChar *str, cchar *delim, MprChar **last);
extern MprChar *mtrim(MprChar *str, cchar *set, int where);

#else
#define mcasecmp(s1, s2)                scaselesscmp(s1, s2)
#define mcmp(s1, s2)                    scmp(s1, s2)
#define mcontains(str, pattern)         scontains(str, pattern)
#define mncontains(str, pattern, limit) sncontains(str, pattern, limit)
#define mcopy(dest, count, src)         scopy(dest, count, src)
#define mends(str, suffix)              sends(str, suffix)
#define mfmt                            sfmt
#define mfmtv(fmt, arg)                 sfmtv(fmt, arg)
#define mjoin                           sjoin
#define mjoinv(sep, args)               sjoinv(sep, args)
#define mncmp(s1, s2, len)              sncmp(s1, s2, len)
#define mncaselesscmp(s1, s2, len)      sncaselesscmp(s1, s2, len)
#define mncopy(dest, count, src, len)   sncopy(dest, count, src, len)
#define mpbrk(str, set)                 spbrk(str, set)
#define mrejoin                         srejoin
#define mrejoinv(buf, sep, args)        srejoinv(buf, sep, args)
#define mspn(str, set)                  sspn(str, set)
#define mstarts(str, prefix)            sstarts(str, prefix)
#define mtok(str, delim, last)          stok(str, delim, last)
#define mtrim(str, set, where)          strim(str, set, where)
#endif /* BIT_CHAR_LEN > 1 */

/************************************ Formatting ******************************/
/**
    Print a formatted message to the standard error channel
    @description This is a secure replacement for fprintf(stderr). 
    @param fmt Printf style format string
    @param ... Variable arguments to format
    @return Returns the number of bytes written
    @ingroup MprString
 */
extern ssize mprPrintfError(cchar *fmt, ...);

/**
    Formatted print. This is a secure verion of printf that can handle null args.
    @description This is a secure replacement for printf. It can handle null arguments without crashes.
    @param fmt Printf style format string
    @param ... Variable arguments to format
    @return Returns the number of bytes written
    @ingroup MprString
 */
extern ssize mprPrintf(cchar *fmt, ...);

/**
    Print a formatted message to a file descriptor
    @description This is a replacement for fprintf as part of the safe string MPR library. It minimizes 
        memory use and uses a file descriptor instead of a File pointer.
    @param file MprFile object returned via #mprOpenFile.
    @param fmt Printf style format string
    @param ... Variable arguments to format
    @return Returns the number of bytes written
    @ingroup MprString
 */
extern ssize mprFprintf(struct MprFile *file, cchar *fmt, ...);

/**
    Format a string into a statically allocated buffer.
    @description This call format a string using printf style formatting arguments. A trailing null will 
        always be appended. The call returns the size of the allocated string excluding the null.
    @param buf Pointer to the buffer.
    @param maxSize Size of the buffer.
    @param fmt Printf style format string
    @param ... Variable arguments to format
    @return Returns the buffer.
    @ingroup MprString
 */
extern char *mprSprintf(char *buf, ssize maxSize, cchar *fmt, ...);

/**
    Format a string into a statically allocated buffer.
    @description This call format a string using printf style formatting arguments. A trailing null will 
        always be appended. The call returns the size of the allocated string excluding the null.
    @param buf Pointer to the buffer.
    @param maxSize Size of the buffer.
    @param fmt Printf style format string
    @param args Varargs argument obtained from va_start.
    @return Returns the buffer;
    @ingroup MprString
 */
extern char *mprSprintfv(char *buf, ssize maxSize, cchar *fmt, va_list args);

/**
    Format a string into an allocated buffer.
    @description This call will dynamically allocate a buffer up to the specified maximum size and will format the 
        supplied arguments into the buffer.  A trailing null will always be appended. The call returns
        the size of the allocated string excluding the null.
        This call is deprecated. Use #sfmt instead.
    @param fmt Printf style format string
    @param ... Variable arguments to format
    @return Returns the number of characters in the string.
    @ingroup MprString
    @internal
 */
extern char *mprAsprintf(cchar *fmt, ...);

/**
    Allocate a buffer of sufficient length to hold the formatted string.
    @description This call will dynamically allocate a buffer up to the specified maximum size and will format 
        the supplied arguments into the buffer. A trailing null will always be appended. The call returns
        the size of the allocated string excluding the null.
        This call is deprecated. Use #sfmtv instead.
    @param fmt Printf style format string
    @param arg Varargs argument obtained from va_start.
    @return Returns the number of characters in the string.
    @ingroup MprString
    @internal
 */
extern char *mprAsprintfv(cchar *fmt, va_list arg);

/********************************* Floating Point *****************************/
#if BIT_FLOAT
/**
    Floating Point Services
    @stability Evolving
    @see mprDota mprIsInfinite mprIsNan mprIsZero 
    @defgroup MprFloat MprFloat
  */
typedef struct MprFloat { int dummy; } MprFloat;

/*
   Mode values for mprDtoa
 */
#define MPR_DTOA_ALL_DIGITS         0       /**< Return all digits */
#define MPR_DTOA_N_DIGITS           2       /**< Return total N digits */
#define MPR_DTOA_N_FRACTION_DIGITS  3       /**< Return total fraction digits */

/*
    Flags for mprDtoa
 */
#define MPR_DTOA_EXPONENT_FORM      0x10    /**< Result in exponent form (N.NNNNe+NN) */
#define MPR_DTOA_FIXED_FORM         0x20    /**< Emit in fixed form (NNNN.MMMM)*/

/**
    Convert a double to ascii
    @param value Value to convert
    @param ndigits Number of digits to render
    @param mode Modes are:
         0   Shortest string,
         1   Like 0, but with Steele & White stopping rule,
         2   Return ndigits of result,
         3   Number of digits applies after the decimal point.
    @param flags Format flags
 */
extern char *mprDtoa(double value, int ndigits, int mode, int flags);

/**
    Test if a double value is infinte
    @param value Value to test
    @return True if the value is +Infinity or -Infinity
 */
extern int mprIsInfinite(double value);

/**
    Test if a double value is zero
    @param value Value to test
    @return True if the value is zero
 */
extern int mprIsZero(double value);

/**
    Test if a double value is not-a-number
    @param value Value to test
    @return True if the value is NaN
 */
extern int mprIsNan(double value);

#endif /* BIT_FLOAT */
/********************************* Buffering **********************************/
/**
    Buffer refill callback function
    @description Function to call when the buffer is depleted and needs more data.
    @param buf Instance of an MprBuf
    @param arg Data argument supplied to #mprSetBufRefillProc
    @returns The callback should return 0 if successful, otherwise a negative error code.
    @ingroup MprBuf
 */
typedef int (*MprBufProc)(struct MprBuf* bp, void *arg);

/**
    Dynamic Buffer Module
    @description MprBuf is a flexible, dynamic growable buffer structure. It has start and end pointers to the
        data buffer which act as read/write pointers. Routines are provided to get and put data into and out of the
        buffer and automatically advance the appropriate start/end pointer. By definition, the buffer is empty when
        the start pointer == the end pointer. Buffers can be created with a fixed size or can grow dynamically as 
        more data is added to the buffer. 
    \n\n
    For performance, the specification of MprBuf is deliberately exposed. All members of MprBuf are implicitly public.
    However, it is still recommended that wherever possible, you use the accessor routines provided.
    @stability Evolving.
    @see MprBuf MprBufProc mprAddNullToBuf mprAddNullToWideBuf mprAdjustBufEnd mprAdjustBufStart mprCloneBuf 
        mprCompactBuf mprCreateBuf mprFlushBuf mprGetBlockFromBuf mprGetBufEnd mprGetBufLength mprGetBufOrigin 
        mprGetBufRefillProc mprGetBufSize mprGetBufSpace mprGetBufStart mprGetCharFromBuf mprGrowBuf 
        mprInsertCharToBuf mprLookAtLastCharInBuf mprLookAtNextCharInBuf mprPutBlockToBuf mprPutCharToBuf 
        mprPutCharToWideBuf mprPutFmtToBuf mprPutFmtToWideBuf mprPutIntToBuf mprPutPadToBuf mprPutStringToBuf 
        mprPutStringToWideBuf mprPutSubStringToBuf mprRefillBuf mprResetBufIfEmpty mprSetBufMax mprSetBufRefillProc 
        mprSetBufSize 
    @defgroup MprBuf MprBuf
 */
typedef struct MprBuf {
    char            *data;              /**< Actual buffer for data */
    char            *endbuf;            /**< Pointer one past the end of buffer */
    char            *start;             /**< Pointer to next data char */
    char            *end;               /**< Pointer one past the last data chr */
    ssize           buflen;             /**< Current size of buffer */
    ssize           maxsize;            /**< Max size the buffer can ever grow */
    ssize           growBy;             /**< Next growth increment to use */
    MprBufProc      refillProc;         /**< Auto-refill procedure */
    void            *refillArg;         /**< Refill arg - must be alloced memory */
} MprBuf;

/**
    Add a null character to the buffer contents.
    @description Add a null byte but do not change the buffer content lengths. The null is added outside the
        "official" content length. This is useful when calling #mprGetBufStart and using the returned pointer 
        as a "C" string pointer.
    @param buf Buffer created via mprCreateBuf
    @ingroup MprBuf
 */
extern void mprAddNullToBuf(MprBuf *buf);

/**
    Adjust the buffer end position
    @description Adjust the buffer end position by the specified amount. This is typically used to advance the
        end position as content is appended to the buffer. Adjusting the start or end position will change the value 
        returned by #mprGetBufLength. If using the mprPutBlock or mprPutChar routines, adjusting the end position is
        done automatically.
    @param buf Buffer created via mprCreateBuf
    @param count Positive or negative count of bytes to adjust the end position.
    @ingroup MprBuf
 */
extern void mprAdjustBufEnd(MprBuf *buf, ssize count);

/**
    Adjust the buffer start position
    @description Adjust the buffer start position by the specified amount. This is typically used to advance the
        start position as content is consumed. Adjusting the start or end position will change the value returned
        by #mprGetBufLength. If using the mprGetBlock or mprGetChar routines, adjusting the start position is
        done automatically.
    @param buf Buffer created via mprCreateBuf
    @param count Positive or negative count of bytes to adjust the start position.
    @ingroup MprBuf
 */
extern void mprAdjustBufStart(MprBuf *buf, ssize count);

/**
    Create a new buffer
    @description Create a new buffer.
    @param initialSize Initial size of the buffer
    @param maxSize Maximum size the buffer can grow to
    @return a new buffer
    @ingroup MprBuf
 */
extern MprBuf *mprCreateBuf(ssize initialSize, ssize maxSize);

/**
    Clone a buffer
    @description Copy the buffer and contents into a newly allocated buffer
    @param orig Original buffer to copy
    @return Returns a newly allocated buffer
 */
extern MprBuf *mprCloneBuf(MprBuf *orig);

/**
    Compact the buffer contents
    @description Compact the buffer contents by copying the contents down to start the the buffer origin.
    @param buf Buffer created via mprCreateBuf
    @ingroup MprBuf
 */
extern void mprCompactBuf(MprBuf *buf);

/**
    Flush the buffer contents
    @description Discard the buffer contents and reset the start end content pointers.
    @param buf Buffer created via mprCreateBuf
    @ingroup MprBuf
 */
extern void mprFlushBuf(MprBuf *buf);

/**
    Get a block of data from the buffer
    @description Get a block of data from the buffer start and advance the start position. If the requested
        length is greater than the available buffer content, then return whatever data is available.
    @param buf Buffer created via mprCreateBuf
    @param blk Destination block for the read data. 
    @param count Count of bytes to read from the buffer.
    @return The count of bytes read into the block or -1 if the buffer is empty.
    @ingroup MprBuf
 */
extern ssize mprGetBlockFromBuf(MprBuf *buf, char *blk, ssize count);

/**
    Get a reference to the end of the buffer contents
    @description Get a pointer to the location immediately after the end of the buffer contents.
    @param buf Buffer created via mprCreateBuf
    @returns Pointer to the end of the buffer data contents. Points to the location one after the last data byte.
    @ingroup MprBuf
 */
extern char *mprGetBufEnd(MprBuf *buf);

/**
    Get the buffer content length.
    @description Get the length of the buffer contents. This is not the same as the buffer size which may be larger.
    @param buf Buffer created via mprCreateBuf
    @returns The length of the content stored in the buffer in bytes
    @ingroup MprBuf
 */
extern ssize mprGetBufLength(MprBuf *buf);

/**
    Get the buffer refill procedure
    @description Return the buffer refill callback function.
    @param buf Buffer created via mprCreateBuf
    @returns The refill call back function if defined.
    @ingroup MprBuf
 */
extern MprBufProc mprGetBufRefillProc(MprBuf *buf);

/**
    Get the origin of the buffer content storage.
    @description Get a pointer to the start of the buffer content storage. This is always and allocated block.
    @param buf Buffer created via mprCreateBuf
    @returns A pointer to the buffer content storage.
    @ingroup MprBuf
 */
extern char *mprGetBuf(MprBuf *buf);

/**
    Get the current size of the buffer content storage.
    @description This returns the size of the memory block allocated for storing the buffer contents.
    @param buf Buffer created via mprCreateBuf
    @returns The size of the buffer content storage.
    @ingroup MprBuf
 */
extern ssize mprGetBufSize(MprBuf *buf);

/**
    Get the space available to store content
    @description Get the number of bytes available to store content in the buffer
    @param buf Buffer created via mprCreateBuf
    @returns The number of bytes available
    @ingroup MprBuf
 */
extern ssize mprGetBufSpace(MprBuf *buf);

//  MOB - need mprBufToString() which adds a null and returns mprGetBufStart()
/**
    Get the start of the buffer contents
    @description Get a pointer to the start of the buffer contents. Use #mprGetBufLength to determine the length
        of the content. Use #mprGetBufEnd to get a pointer to the location after the end of the content.
    @param buf Buffer created via mprCreateBuf
    @returns Pointer to the start of the buffer data contents
    @ingroup MprBuf
 */
extern char *mprGetBufStart(MprBuf *buf);

/**
    Get a character from the buffer
    @description Get the next byte from the buffer start and advance the start position.
    @param buf Buffer created via mprCreateBuf
    @return The character or -1 if the buffer is empty.
    @ingroup MprBuf
 */
extern int mprGetCharFromBuf(MprBuf *buf);

/**
    Grow the buffer
    @description Grow the storage allocated for content for the buffer. The new size must be less than the maximum
        limit specified via #mprCreateBuf or #mprSetBufSize.
    @param buf Buffer created via mprCreateBuf
    @param count Count of bytes by which to grow the buffer content size. 
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprGrowBuf(MprBuf *buf, ssize count);

/**
    Insert a character into the buffer
    @description Insert a character into to the buffer prior to the current buffer start point.
    @param buf Buffer created via mprCreateBuf
    @param c Character to append.
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprInsertCharToBuf(MprBuf *buf, int c);

/**
    Peek at the next character in the buffer
    @description Non-destructively return the next character from the start position in the buffer. 
        The character is returned and the start position is not altered.
    @param buf Buffer created via mprCreateBuf
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprLookAtNextCharInBuf(MprBuf *buf);

/**
    Peek at the last character in the buffer
    @description Non-destructively return the last character from just prior to the end position in the buffer. 
        The character is returned and the end position is not altered.
    @param buf Buffer created via mprCreateBuf
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprLookAtLastCharInBuf(MprBuf *buf);

/**
    Put a block to the buffer.
    @description Append a block of data  to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param ptr Block to append
    @param size Size of block to append
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern ssize mprPutBlockToBuf(MprBuf *buf, cchar *ptr, ssize size);

/**
    Put a character to the buffer.
    @description Append a character to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param c Character to append
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprPutCharToBuf(MprBuf *buf, int c);

/**
    Put a formatted string to the buffer.
    @description Format a string and append to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param fmt Printf style format string
    @param ... Variable arguments for the format string
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern ssize mprPutFmtToBuf(MprBuf *buf, cchar *fmt, ...);

/**
    Put an integer to the buffer.
    @description Append a integer to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param i Integer to append to the buffer
    @returns Number of characters added to the buffer, otherwise a negative error code 
    @ingroup MprBuf
 */
extern ssize mprPutIntToBuf(MprBuf *buf, int64 i);

/**
    Put padding characters to the buffer.
    @description Append padding characters to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param c Character to append
    @param count Count of pad characters to put
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern ssize mprPutPadToBuf(MprBuf *buf, int c, ssize count);

/**
    Put a string to the buffer.
    @description Append a null terminated string to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param str String to append
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern ssize mprPutStringToBuf(MprBuf *buf, cchar *str);

/**
    Put a substring to the buffer.
    @description Append a null terminated substring to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param str String to append
    @param count Put at most count characters to the buffer
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern ssize mprPutSubStringToBuf(MprBuf *buf, cchar *str, ssize count);

/**
    Refill the buffer with data
    @description Refill the buffer by calling the refill procedure specified via #mprSetBufRefillProc
    @param buf Buffer created via mprCreateBuf
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprRefillBuf(MprBuf *buf);

/**
    Reset the buffer
    @description If the buffer is empty, reset the buffer start and end pointers to the beginning of the buffer.
    @param buf Buffer created via mprCreateBuf
    @ingroup MprBuf
 */
extern void mprResetBufIfEmpty(MprBuf *buf);

/**
    Set the maximum buffer size
    @description Update the maximum buffer size set when the buffer was created
    @param buf Buffer created via mprCreateBuf
    @param maxSize New maximum size the buffer can grow to
    @ingroup MprBuf
 */
extern void mprSetBufMax(MprBuf *buf, ssize maxSize);

/**
    Set the buffer refill procedure
    @description Define a buffer refill procedure. The MprBuf module will not invoke or manage this refill procedure.
        It is simply stored to allow upper layers to use and provide their own auto-refill mechanism.
    @param buf Buffer created via mprCreateBuf
    @param fn Callback function to store.
    @param arg Callback data argument.
    @ingroup MprBuf
 */
extern void mprSetBufRefillProc(MprBuf *buf, MprBufProc fn, void *arg);

/**
    Set the buffer size
    @description Set the current buffer content size and maximum size limit. Setting a current size will
        immediately grow the buffer to be this size. If the size is less than the current buffer size, 
        the requested size will be ignored. ie. this call will not shrink the buffer. Setting a maxSize 
        will define a maximum limit for how big the buffer contents can grow. Set either argument to 
        -1 to be ignored.
    @param buf Buffer created via mprCreateBuf
    @param size Size to immediately make the buffer. If size is less than the current buffer size, it will be ignored.
        Set to -1 to ignore this parameter.
    @param maxSize Maximum size the buffer contents can grow to.
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
 */
extern int mprSetBufSize(MprBuf *buf, ssize size, ssize maxSize);

#if DOXYGEN || BIT_CHAR_LEN > 1
/**
    Add a wide null character to the buffer contents.
    @description Add a null character but do not change the buffer content lengths. The null is added outside the
        "official" content length. This is useful when calling #mprGetBufStart and using the returned pointer 
        as a string pointer.
    @param buf Buffer created via mprCreateBuf
    @ingroup MprBuf
  */
extern void mprAddNullToWideBuf(MprBuf *buf);

/**
    Put a wide character to the buffer.
    @description Append a wide character to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param c Character to append
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
  */
extern int mprPutCharToWideBuf(MprBuf *buf, int c);

/**
    Put a wide string to the buffer.
    @description Append a null terminated wide string to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param str String to append
    @returns Zero if successful and otherwise a negative error code 
    @ingroup MprBuf
*/
extern int mprPutStringToWideBuf(MprBuf *buf, cchar *str);

/**
    Put a formatted wide string to the buffer.
    @description Format a string and append to the buffer at the end position and increment the end pointer.
    @param buf Buffer created via mprCreateBuf
    @param fmt Printf style format string
    @param ... Variable arguments for the format string
    @returns Zero if successful and otherwise a negative error code 
 */
extern int mprPutFmtToWideBuf(MprBuf *buf, cchar *fmt, ...);

#else /* BIT_CHAR_LEN == 1 */
#define mprAddNullToWideBuf     mprAddNullToBuf
#define mprPutCharToWideBuf     mprPutCharToBuf
#define mprPutStringToWideBuf   mprPutStringToBuf
#define mprPutFmtToWideBuf      mprPutFmtToBuf
#endif

#if MPR_BUF_MACROS || 1
#define mprGetBufLength(bp) ((bp)->end - (bp)->start)
#define mprGetBufSize(bp) ((bp)->buflen)
#define mprGetBufSpace(bp) ((bp)->endbuf - (bp)->end)
#define mprGetBuf(bp) ((bp)->data)
#define mprGetBufStart(bp) ((bp)->start)
#define mprGetBufEnd(bp) ((bp)->end)
#endif

/******************************** Date and Time *******************************/
/**
    Format a date according to RFC822: (Fri, 07 Jan 2003 12:12:21 PDT)
 */
#define MPR_RFC_DATE        "%a, %d %b %Y %T %Z"

/**
    Default date format used in mprFormatLocalTime/mprFormatUniversalTime when no format supplied
 */
#define MPR_DEFAULT_DATE    "%a %b %d %T %Y %Z"

/**
    Date format for use in HTTP (headers)
 */
#define MPR_HTTP_DATE       "%a, %d %b %Y %T GMT"

/**
    Mpr time structure.
    @description MprTime is the cross platform time abstraction structure. Time is stored as milliseconds
        since the epoch: 00:00:00 UTC Jan 1 1970. MprTime is typically a 64 bit quantity.
    @ingroup MprTime
 */
extern int mprCreateTimeService();

/**
    Compare two times
    @description compare two times and return a code indicating which is greater, less or equal
    @param t1 First time
    @param t2 Second time
    @returns Zero if equal, -1 if t1 is less than t2 otherwise one.
    @ingroup MprTime
 */
extern int mprCompareTime(MprTime t1, MprTime t2);

/**
    Decode a time value into a tokenized local time value.
    @description Safe replacement for localtime. This call converts the time value to local time and formats 
        the as a struct tm.
    @param timep Pointer to a tm structure to hold the result
    @param time Time to format
    @ingroup MprTime
 */
extern void mprDecodeLocalTime(struct tm *timep, MprTime time);

/**
    Decode a time value into a tokenized UTC time structure.
    @description Safe replacement for gmtime. This call converts the supplied time value
        to UTC time and parses the result into a tm structure.
    @param timep Pointer to a tm structure to hold the result.
    @param time The time to format
    @ingroup MprTime
 */
extern void mprDecodeUniversalTime(struct tm *timep, MprTime time);

/**
    Convert a time value to local time and format as a string.
    @description Safe replacement for ctime. 
    @param fmt Time format string
    @param time Time to format. Use mprGetTime to retrieve the current time.
    @return The formatting time string
    @ingroup MprTime
 */
extern char *mprFormatLocalTime(cchar *fmt, MprTime time);

/**
    Convert a time value to universal time and format as a string.
    @description Safe replacement for ctime.
    @param fmt Time format string
    @param time Time to format. Use mprGetTime to retrieve the current time.
    @return The formatting time string
    @ingroup MprTime
 */
extern char *mprFormatUniversalTime(cchar *fmt, MprTime time);

/**
    Format a time value as a local time.
    @description This call formats the time value supplied via \a timep.
    @param fmt The time format to use.
    @param timep The time value to format.
    @return The formatting time string.
    @ingroup MprTime
 */
extern char *mprFormatTm(cchar *fmt, struct tm *timep);

/**
    Get the system time.
    @description Get the system time in milliseconds.
    @return Returns the time in milliseconds since boot.
    @ingroup MprTime
 */
extern MprTime mprGetTime();

/**
    Get a string representation of the current date/time
    @description Get the current date/time as a string according to the given format.
    @param fmt Date formatting string. See strftime for acceptable date format specifiers. 
        If null, then this routine uses the #MPR_DEFAULT_DATE format.
    @return An allocated date string
 */
extern char *mprGetDate(char *fmt);

/**
    Get the CPU tick count.
    @description Get the current CPU tick count. This is a system dependant high resolution timer. On some systems, 
    this return time in nanosecond resolution. 
    @return Returns the CPU time in ticks. Will return the system time if CPU ticks are not available.
    @ingroup MprTime
 */
extern uint64 mprGetTicks();

#if (LINUX || MACOSX || WINDOWS) && (BIT_CPU_ARCH == MPR_CPU_X86 || BIT_CPU_ARCH == MPR_CPU_X64)
    #define MPR_HIGH_RES_TIMER 1
#else
    #define MPR_HIGH_RES_TIMER 0
#endif

#if BIT_DEBUG
    #if MPR_HIGH_RES_TIMER
        #define MPR_MEASURE(level, tag1, tag2, op) \
            if (1) { \
                MprTime elapsed, start = mprGetTime(); \
                uint64  ticks = mprGetTicks(); \
                op; \
                elapsed = mprGetTime() - start; \
                if (elapsed < 1000) { \
                    mprLog(level, "TIME: %s.%s elapsed %,d msec, %,d ticks", tag1, tag2, elapsed, mprGetTicks() - ticks); \
                } else { \
                    mprLog(level, "TIME: %s.%s elapsed %,d msec", tag1, tag2, elapsed); \
                } \
            } else 
    #else
        #define MPR_MEASURE(level, tag1, tag2, op) \
            if (1) { \
                MprTime start = mprGetTime(); \
                op; \
                mprLog(level, "TIME: %s.%s elapsed %,d msec", tag1, tag2, mprGetTime() - start); \
            } else 
    #endif
#else
    #define MPR_MEASURE(level, tag1, tag2, op) op
#endif


/**
    Return the time remaining until a timeout has elapsed
    @param mark Starting time stamp 
    @param timeout Time in milliseconds
    @return Time in milliseconds until the timeout elapses  
    @ingroup MprTime
 */
extern MprTime mprGetRemainingTime(MprTime mark, MprTime timeout);

/**
    Get the elapsed time since a time mark. Create the time mark with mprGetTime()
    @param mark Starting time stamp 
    @returns the time elapsed since the mark was taken.
 */
extern MprTime mprGetElapsedTime(MprTime mark);

/*
    Convert a time structure into a time value using local time.
    @param timep Pointer to a time structure
    @return a time value
 */
extern MprTime mprMakeTime(struct tm *timep);

/*
    Convert a time structure into a time value using UTC time.
    @param timep Pointer to a time structure
    @return a time value
 */
MprTime mprMakeUniversalTime(struct tm *tm);

/**
    Constants for mprParseTime
 */
#define MPR_LOCAL_TIMEZONE     MAXINT       /**< Use local timezone */
#define MPR_UTC_TIMEZONE        0           /**< Use UTC timezone */

/*
    Parse a string into a time value
    @param time Pointer to a time value to receive the parsed time value
    @param dateString String to parse
    @param timezone Timezone in which to interpret the date
    @param defaults Date default values to use for missing components
    @returns Zero if successful
 */
extern int mprParseTime(MprTime *time, cchar *dateString, int timezone, struct tm *defaults);

/**
    Get the current timezone offset for a given time
    @description Calculate the current timezone (including DST)
    @param when Time to examine to extract the timezone
    @returns Returns a timezone offset in msec.  Local time == (UTC + offset).
 */
extern int mprGetTimeZoneOffset(MprTime when);

/*********************************** Lists ************************************/
/*
    List flags
 */
#define MPR_OBJ_LIST            0x1     /**< Object is a hash */
#define MPR_LIST_STATIC_VALUES  0x20    /**< Flag for #mprCreateList when values are permanent */

/**
    List data structure.
    @description The MprList is a dynamic, growable list suitable for storing pointers to arbitrary objects.
    @stability Evolving.
    @see MprList MprListCompareProc mprAddItem mprAddNullItem mprAppendList mprClearList mprCloneList mprCopyList 
        mprCreateKeyPair mprCreateList mprGetFirstItem mprGetItem mprGetLastItem mprGetListCapacity mprGetListLength 
        mprGetNextItem mprGetPrevItem mprInitList mprInsertItemAtPos mprLookupItem mprLookupStringItem mprPopItem mprPushItem 
        mprRemoveItem mprRemoveItemAtPos mprRemoveRangeOfItems mprRemoveStringItem mprSetItem mprSetListLimits mprSortList 
    @defgroup MprList MprList
 */
typedef struct MprList {
    int         flags;                  /**< Control flags */
    //  MOB - rename size for compat with MprHash
    int         capacity;               /**< Current list size */ 
    int         length;                 /**< Current length of the list contents */
    int         maxSize;                /**< Maximum capacity */
    MprMutex    *mutex;                 /**< Multithread lock */
    void        **items;                /**< List item data */
} MprList;

/**
    List comparison procedure for sorting
    @description Callback function signature used by #mprSortList
    @param arg1 First list item to compare
    @param arg2 Second list item to compare
    @returns Return zero if the items are equal. Return -1 if the first arg is less than the second. Otherwise return 1.
    @ingroup MprList
 */
typedef int (*MprListCompareProc)(cvoid *arg1, cvoid *arg2);

/**
    Add an item to a list
    @description Add the specified item to the list. The list must have been previously created via 
        mprCreateList. The list will grow as required to store the item
    @param list List pointer returned from #mprCreateList
    @param item Pointer to item to store
    @return Returns a positive list index for the inserted item. If the item cannot be inserted due 
        to a memory allocation failure, -1 is returned
    @ingroup MprList
 */
extern int mprAddItem(MprList *list, cvoid *item);

/**
    Add a null item to the list.
    @description Add a null item to the list. This item does not count in the length returned by #mprGetListLength
    and will not be visible when iterating using #mprGetNextItem.
 */
extern int mprAddNullItem(MprList *list);

/**
    Append a list
    @description Append the contents of one list to another. The list will grow as required to store the item
    @param list List pointer returned from #mprCreateList
    @param add List whose contents are added
    @return Returns a pointer to the original list if successful. Returns NULL on memory allocation errors.
    @ingroup MprList
 */
extern MprList *mprAppendList(MprList *list, MprList *add);

/**
    Clears the list of all items.
    @description Resets the list length to zero and clears all items.
    @param list List pointer returned from mprCreateList.
    @ingroup MprList
 */
extern void mprClearList(MprList *list);

/**
    Clone a list and all elements
    @description Copy the contents of a list into a new list. 
    @param src Source list to copy
    @return Returns a new list reference
    @ingroup MprList
 */
extern MprList *mprCloneList(MprList *src);

/**
    Copy list contents
    @description Copy the contents of a list into an existing list. The destination list is cleared first and 
        has its dimensions set to that of the source clist.
    @param dest Destination list for the copy
    @param src Source list
    @return Returns zero if successful, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprCopyListContents(MprList *dest, MprList *src);

/**
    Create a list.
    @description Creates an empty list. MprList's can store generic pointers. They automatically grow as 
        required when items are added to the list.
    @param size Initial capacity of the list.
    @param flags Control flags. Possible values are: MPR_LIST_STATIC_VALUES to indicate list items are static
        and should not be marked for GC.
    @return Returns a pointer to the list. 
    @ingroup MprList
 */
extern MprList *mprCreateList(int size, int flags);

/**
    Get the first item in the list.
    @description Returns the value of the first item in the list. After calling this routine, the remaining 
        list items can be walked using mprGetNextItem.
    @param list List pointer returned from mprCreateList.
    @ingroup MprList
 */
extern void *mprGetFirstItem(MprList *list);

/**
    Get an list item.
    @description Get an list item specified by its index.
    @param list List pointer returned from mprCreateList.
    @param index Item index into the list. Indexes have a range from zero to the lenghth of the list - 1.
    @ingroup MprList
 */
extern void *mprGetItem(MprList *list, int index);

/**
    Get the last item in the list.
    @description Returns the value of the last item in the list. After calling this routine, the remaining 
        list items can be walked using mprGetPrevItem.
    @param list List pointer returned from mprCreateList.
    @ingroup MprList
 */
extern void *mprGetLastItem(MprList *list);

/**
    Get the current capacity of the list.
    @description Returns the capacity of the list. This will always be equal to or greater than the list length.
    @param list List pointer returned from mprCreateList.
    @ingroup MprList
 */
extern int mprGetListCapacity(MprList *list);

/**
    Get the number of items in the list.
    @description Returns the number of items in the list. This will always be less than or equal to the list capacity.
    @param list List pointer returned from mprCreateList.
    @ingroup MprList
 */
extern int mprGetListLength(MprList *list);

/**
    Get the next item in the list.
    @description Returns the value of the next item in the list. Before calling
        this routine, mprGetFirstItem must be called to initialize the traversal of the list.
    @param list List pointer returned from mprCreateList.
    @param lastIndex Pointer to an integer that will hold the last index retrieved.
    @ingroup MprList
 */
extern void *mprGetNextItem(MprList *list, int *lastIndex);

/**
    Get the previous item in the list.
    @description Returns the value of the previous item in the list. Before 
        calling this routine, mprGetFirstItem and/or mprGetNextItem must be
        called to initialize the traversal of the list.
    @param list List pointer returned from mprCreateList.
    @param lastIndex Pointer to an integer that will hold the last index retrieved.
    @ingroup MprList
 */
extern void *mprGetPrevItem(MprList *list, int *lastIndex);

/**
    Initialize a list structure
    @description If a list is statically declared inside another structure, mprInitList can be used to 
        initialize it before use.
    @param list Reference to the MprList struct.
    @ingroup MprList
 */
extern void mprInitList(MprList *list);

/**
    Insert an item into a list at a specific position
    @description Insert the item into the list before the specified position. The list will grow as required 
        to store the item
    @param list List pointer returned from #mprCreateList
    @param index Location at which to store the item. The previous item at this index is moved up to make room.
    @param item Pointer to item to store
    @return Returns the position index (positive integer) if successful. If the item cannot be inserted due 
        to a memory allocation failure, -1 is returned
    @ingroup MprList
 */
extern int mprInsertItemAtPos(MprList *list, int index, cvoid *item);

/**
    Find an item and return its index.
    @description Search for an item in the list and return its index.
    @param list List pointer returned from mprCreateList.
    @param item Pointer to value stored in the list.
    @return Positive list index if found, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprLookupItem(MprList *list, cvoid *item);

/**
    Find a string item and return its index.
    @description Search for the first matching string in the list and return its index.
    @param list List pointer returned from mprCreateList.
    @param str Pointer to string to look for.
    @return Positive list index if found, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprLookupStringItem(MprList *list, cchar *str);

/**
    Remove an item from the list
    @description Search for a specified item and then remove it from the list.
    @param list List pointer returned from mprCreateList.
    @param item Item pointer to remove. 
    @return Returns the positive index of the removed item, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprRemoveItem(MprList *list, cvoid *item);

/**
    Remove an item from the list
    @description Removes the element specified by \a index, from the list. The
        list index is provided by mprInsertItem.
    @return Returns the positive index of the removed item, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprRemoveItemAtPos(MprList *list, int index);

/**
    Remove the last item from the list
    @description Remove the item at the highest index position.
    @param list List pointer returned from mprCreateList.
    @return Returns the positive index of the removed item, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprRemoveLastItem(MprList *list);

/**
    Remove a range of items from the list.
    @description Remove a range of items from the list. The range is specified
        from the \a start index up to and including the \a end index.
    @param list List pointer returned from mprCreateList.
    @param start Starting item index to remove (inclusive)
    @param end Ending item index to remove (inclusive)
    @return Returns zero if successful, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprRemoveRangeOfItems(MprList *list, int start, int end);

/**
    Remove a string item from the list
    @description Search for the first matching string and then remove it from the list.
    @param list List pointer returned from mprCreateList.
    @param str String value to remove. 
    @return Returns the positive index of the removed item, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprRemoveStringItem(MprList *list, cchar *str);

/**
    Set a list item
    @description Update the list item stored at the specified index
    @param list List pointer returned from mprCreateList.
    @param index Location to update
    @param item Pointer to item to store
    @return Returns the old item previously at that location index
    @ingroup MprList
 */
extern void *mprSetItem(MprList *list, int index, cvoid *item);

/**
    Define the list size limits
    @description Define the list initial size and maximum size it can grow to.
    @param list List pointer returned from mprCreateList.
    @param initialSize Initial size for the list. This call will allocate space for at least this number of items.
    @param maxSize Set the maximum limit the list can grow to become.
    @return Returns zero if successful, otherwise a negative MPR error code.
    @ingroup MprList
 */
extern int mprSetListLimits(MprList *list, int initialSize, int maxSize);

typedef int (*MprSortProc)(cvoid *p1, cvoid *p2, void *ctx);
extern void mprSort(void *base, ssize num, ssize width, MprSortProc compare, void *ctx);


/**
    Sort a list
    @description Sort a list using the sort ordering dictated by the supplied compare function.
    @param list List pointer returned from mprCreateList.
    @param compare Comparison function. If null, then a default string comparison is used.
    @param ctx Context to provide to comparison function
    @ingroup MprList
 */
extern void mprSortList(MprList *list, MprSortProc compare, void *ctx);

/**
    Key value pairs for use with MprList or MprKey
    @ingroup MprList
 */
typedef struct MprKeyValue {
    void        *key;               /**< Key string */
    void        *value;             /**< Associated value for the key */
} MprKeyValue;

/**
    Create a key / value pair
    @description Allocate and initialize a key value pair for use by the MprList or MprHash modules.
    @param key Key string
    @param value Key value string
    @returns An initialized MprKeyValue
    @ingroup MprList
 */
extern MprKeyValue *mprCreateKeyPair(cchar *key, cchar *value);

/**
    Pop an item
    @description Treat the list as a stack and pop the last pushed item
    @param list List pointer returned from mprCreateList.
    @return Returns the last pushed item. If the list is empty, returns NULL.
  */
extern void *mprPopItem(MprList *list);

/** 
    Push an item onto the list
    @description Treat the list as a stack and push the last pushed item
    @param list List pointer returned from mprCreateList.
    @param item Item to push onto the list
    @return Returns a positive integer list index for the inserted item. If the item cannot be inserted due 
        to a memory allocation failure, -1 is returned
  */
extern int mprPushItem(MprList *list, cvoid *item);

#define MPR_GET_ITEM(list, index) list->items[index]
#define ITERATE_ITEMS(list, item, next) next = 0; list && (item = mprGetNextItem(list, &next)) != 0; 
#define mprGetListLength(lp) ((lp) ? (lp)->length : 0)

/********************************** Logging ***********************************/
/**
    Logging Services
    @stability Evolving
    @defgroup MprLog MprLog
    @see MprLogHandler mprAssertError mprError mprFatalError mprGetLogFile mprGetLogHandler mprLog mprMemoryError 
        mprRawLog mprSetLogFile mprSetLogHandler mprSetLogLevel mprStaticError mprUserError mprUsingDefaultLogHandler 
        mprWarn 
 */
typedef struct MprLog { int dummy; } MprLog;

/**
    Log handler callback type.
    @description Callback prototype for the log handler. Used by mprSetLogHandler to define 
        a message logging handler to process log and error messages. 
    @param file Source filename. Derived by using __FILE__.
    @param line Source line number. Derived by using __LINE__.
    @param flags Error flags.
    @param level Message logging level. Levels are 0-9 with zero being the most verbose.
    @param msg Message being logged.
    @ingroup MprLog
 */
typedef void (*MprLogHandler)(int flags, int level, cchar *msg);

/**
    Initialize the log service
    @ingroup MprLog
 */
extern void mprCreateLogService();

/**
    Start logging 
    @param logSpec Set the log file name and level. The format is "pathName[:level]".
    The following levels are generally observed:
    <ul>
        <li>0 - Essential messages, fatal errors and critical warnings</li>
        <li>1 - Hard errors</li>
        <li>2 - Configuration setup and soft warnings</li>
        <li>3 - Useful informational messages</li>
        <li>4 - Debug information</li>
        <li>5-9 - Increasing levels of internal Appweb trace useful for debugging</li>
    </ul>
    @param showConfig Set to true to log an initial system configuration.
    @return Zero if successful, otherwise a negative Mpr error code. See the Appweb log for diagnostics.
    @ingroup MprLog
*/
extern int mprStartLogging(cchar *logSpec, int showConfig);

/**
    Emit a descriptive log header
    @ingroup MprLog
 */
extern void mprLogHeader();

/**
    Backup a log
    @param path Base log filename
    @param count Count of archived logs to keep
    @ingroup MprLog
 */
extern int mprBackupLog(cchar *path, int count);

/**
    Set the log rotation parameters
    @param logSize If the size is zero, then the log file will be rotated on each application boot. Otherwise,
        the log file will be rotated if on application boot, the log file is larger than this size.
    @param backupCount Count of the number of log files to keep
    @param flags Set to MPR_LOG_APPEND to append to existing log files. Set to MPR_LOG_TRUNCATE to truncate log files
        on application restart.
    @ingroup MprLog
 */
extern void mprSetLogBackup(ssize logSize, int backupCount, int flags);

/**
    Output an assertion failed message.
    @description This will emit an assertion failed message to the standard error output. It will bypass the logging
        system.
    @param loc Source code location string. Use MPR_LOC to define a file name and line number string suitable for this
        parameter.
    @param msg Simple string message to output
    @ingroup MprLog
 */
extern void mprAssertError(cchar *loc, cchar *msg);

/**
    Log an error message.
    @description Send an error message to the MPR debug logging subsystem. The 
        message will be to the log handler defined by #mprSetLogHandler. It 
        is up to the log handler to respond appropriately and log the message.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @ingroup MprLog
 */
extern void mprError(cchar *fmt, ...);

/**
    Log a fatal error message and exit.
    @description Send a fatal error message to the MPR debug logging subsystem and then exit the application by
        calling exit(). The message will be to the log handler defined by #mprSetLogHandler. It 
        is up to the log handler to respond appropriately and log the message.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @ingroup MprLog
 */
extern void mprFatalError(cchar *fmt, ...);

/**
    Get the log file object
    @description Returns the MprFile object used for logging
    @returns An MprFile object for logging
    @ingroup MprLog
 */
extern struct MprFile *mprGetLogFile();

/**
    Get the current MPR debug log handler.
    @description Get the log handler defined via #mprSetLogHandler
    @returns A function of the signature #MprLogHandler
    @ingroup MprLog
 */
extern MprLogHandler mprGetLogHandler();

/**
    Write a message to the diagnostic log file.
    @description Send a message to the MPR logging subsystem.
    @param level Logging level for this message. The level is 0-9 with zero being the most verbose.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @remarks mprLog is highly useful as a debugging aid when integrating or when developing new modules. 
    @ingroup MprLog
 */
extern void mprLog(int level, cchar *fmt, ...);

/**
    Log a memory error message.
    @description Send a memory error message to the MPR debug logging subsystem. The message will be 
        passed to the log handler defined by #mprSetLogHandler. It is up to the log handler to respond appropriately
        to the fatal message, the MPR takes no other action other than logging the message. Typically, a memory 
        message will be logged and the application will be shutdown. The preferred method of operation is to define
        a memory depletion callback via #mprCreate. This will be invoked whenever a memory allocation error occurs.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @ingroup MprLog
 */
extern void mprMemoryError(cchar *fmt, ...);

/**
    Write a raw log message to the diagnostic log file.
    @description Send a raw message to the MPR logging subsystem. Raw messages do not have any application prefix
        attached to the message and do not append a newline to the message.
    @param level Logging level for this message. The level is 0-9 with zero being the most verbose.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @remarks mprLog is highly useful as a debugging aid when integrating or when developing new modules. 
    @ingroup MprLog
 */
extern void mprRawLog(int level, cchar *fmt, ...);

/**
    Set a file to be used for logging
    @param file MprFile object instance
 */
extern void mprSetLogFile(struct MprFile *file);

/**
    Set an MPR debug log handler.
    @description Defines a callback handler for MPR debug and error log messages. When output is sent to 
        the debug channel, the log handler will be invoked to accept the output message.
    @param handler Callback handler
 */
extern void mprSetLogHandler(MprLogHandler handler);

/**
    Display an error message to the console without allocating any memory.
    @description Display an error message to the console. This will bypass the MPR logging subsystem.
        It will not allocated any memory and is used by low level memory allocating and garbage collection routines.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @ingroup MprLog
 */
extern void mprStaticError(cchar *fmt, ...);

/**
    Display an error message to the user.
    @description Display an error message to the user and then send it to the 
        MPR debug logging subsystem. The message will be passed to the log 
        handler defined by mprSetLogHandler. It is up to the log handler to 
        respond appropriately and display the message to the user.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @ingroup MprLog
 */
extern void mprUserError(cchar *fmt, ...);

/**
    Determine if the app is using the default MPR log handler.
    @description Returns true if no custom log handler has been installed.
    @returns True if using the default log handler
    @ingroup MprLog
 */
extern int mprUsingDefaultLogHandler();

/**
    Log a warning message.
    @description Send a warning message to the MPR debug logging subsystem. The 
        message will be to the log handler defined by #mprSetLogHandler. It 
        is up to the log handler to respond appropriately and log the message.
    @param fmt Printf style format string. Variable number of arguments to 
    @param ... Variable number of arguments for printf data
    @ingroup MprLog
 */
extern void mprWarn(cchar *fmt, ...);

/*
    Optimized logging calling sequence. This compiles out for release mode.
 */
#if BIT_DEBUG
#define LOG mprLog
#else
#define LOG if (0) mprLog
#endif

/*
    Just for easy debugging. Adds a "\n" automatically.
    @internal
 */
extern int print(cchar *fmt, ...);

/************************************ Hash ************************************/
/**
    Hash table entry structure.
    @description The hash structure supports growable hash tables with high performance, collision resistant hashes.
    Each hash entry has a descriptor entry. This is used to manage the hash table link chains.
    @see MprKey MprHashProc MprHash mprAddDuplicateHash mprAddKey mprAddKeyFmt mprCloneHash mprCreateHash 
        mprGetFirstKey mprGetHashLength mprGetKeyBits mprGetNextKey mprLookupKey mprLookupKeyEntry mprRemoveKey 
        mprSetKeyBits mprBlendHash
    @stability Evolving.
    @defgroup MprHash MprHash
 */
typedef struct MprKey {
    struct MprKey   *next;              /**< Next symbol in hash chain */
    char            *key;               /**< Hash key */
    cvoid           *data;              /**< Pointer to symbol data */
    int             type: 4;            /**< Data type */
    int             bucket: 28;         /**< Hash bucket index */
} MprKey;

/**
    Hashing function to use for the table
    @param name Name to hash
    @param len Length of the name to hash
    @return An integer hash index
    @internal
*/
typedef uint (*MprHashProc)(cvoid *name, ssize len);

/*
    Flags for MprHash
 */
#define MPR_OBJ_HASH            0x1     /**< Object is a hash */
#define MPR_HASH_CASELESS       0x10    /**< Key comparisons ignore case */
#define MPR_HASH_UNICODE        0x20    /**< Hash keys are unicode strings */
#define MPR_HASH_STATIC_KEYS    0x40    /**< Keys are permanent - don't dup or mark */
#define MPR_HASH_STATIC_VALUES  0x80    /**< Values are permanent - don't mark */
#define MPR_HASH_LIST           0x100   /**< Hash keys are numeric indicies */
#define MPR_HASH_STATIC_ALL     (MPR_HASH_STATIC_KEYS | MPR_HASH_STATIC_VALUES)

/**
    Hash table control structure
    @see MprHash
 */
typedef struct MprHash {
    int             flags;              /**< Hash control flags */
    int             size;               /**< Size of the buckets array */
    int             length;             /**< Number of symbols in the table */
    MprKey          **buckets;          /**< Hash collision bucket table */
    MprHashProc     fn;                 /**< Hash function */             
    MprMutex        *mutex;             /**< GC marker sync */
} MprHash;

/*
    Macros
 */
#define ITERATE_KEYS(table, item) item = 0; table && (item = mprGetNextKey(table, item)) != 0; 

/**
    Add a duplicate symbol value into the hash table
    @description Add a symbol to the hash which may clash with an existing entry. Duplicate symbols can be added to
        the hash, but only one may be retrieved via #mprLookupKey. To recover duplicate entries walk the hash using
        #mprGetNextKey.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param key String key of the symbole entry to delete.
    @param ptr Arbitrary pointer to associate with the key in the table.
    @return Integer count of the number of entries.
    @ingroup MprHash
 */
extern MprKey *mprAddDuplicateKey(MprHash *table, cvoid *key, cvoid *ptr);

/**
    Add a symbol value into the hash table
    @description Associate an arbitrary value with a string symbol key and insert into the symbol table.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param key String key of the symbole entry to delete.
    @param ptr Arbitrary pointer to associate with the key in the table.
    @return Integer count of the number of entries.
    @ingroup MprHash
 */
extern MprKey *mprAddKey(MprHash *table, cvoid *key, cvoid *ptr);

/**
    Add a key with a formatting value into the hash table
    @description Associate a formatted value with a key and insert into the symbol table.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param key String key of the symbole entry to delete.
    @param fmt Format string. See #mprPrintf.
    @return Integer count of the number of entries.
    @ingroup MprHash
 */
extern MprKey *mprAddKeyFmt(MprHash *table, cvoid *key, cchar *fmt, ...);

/**
    Copy a hash table
    @description Create a new hash table and copy all the entries from an existing table.
    @param table Symbol table returned via mprCreateSymbolTable.
    @return A new hash table initialized with the contents of the original hash table.
    @ingroup MprHash
 */
extern MprHash *mprCloneHash(MprHash *table);

/**
    Create a hash table
    @description Creates a hash table that can store arbitrary objects associated with string key values.
    @param hashSize Size of the hash table for the symbol table. Should be a prime number.
    @param flags Table control flags. Use MPR_HASH_CASELESS for case insensitive comparisions, MPR_HASH_UNICODE
        if the hash keys are unicode strings, MPR_HASH_STATIC_KEYS if the keys are permanent and should not be
        managed for Garbage collection, and MPR_HASH_STATIC_VALUES if the values are permanent.
    @return Returns a pointer to the allocated symbol table.
    @ingroup MprHash
 */
extern MprHash *mprCreateHash(int hashSize, int flags);

/**
    Create a hash of words
    @description Create a hash table of words from the given string. The hash key entry is the same as the key.
    @param str String containing white space or comma separated words
    @return Returns a hash of words
    @ingroup MprHash
 */
extern MprHash *mprCreateHashFromWords(cchar *str);

/**
    Return the first symbol in a symbol entry
    @description Prepares for walking the contents of a symbol table by returning the first entry in the symbol table.
    @param table Symbol table returned via mprCreateSymbolTable.
    @return Pointer to the first entry in the symbol table.
    @ingroup MprHash
 */
extern MprKey *mprGetFirstKey(MprHash *table);

/**
    Return the next symbol in a symbol entry
    @description Continues walking the contents of a symbol table by returning
        the next entry in the symbol table. A previous call to mprGetFirstSymbol
        or mprGetNextSymbol is required to supply the value of the \a last
        argument.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param last Symbol table entry returned via mprGetFirstSymbol or mprGetNextSymbol.
    @return Pointer to the first entry in the symbol table.
    @ingroup MprHash
 */
extern MprKey *mprGetNextKey(MprHash *table, MprKey *last);

/**
    Return the count of symbols in a symbol entry
    @description Returns the number of symbols currently existing in a symbol table.
    @param table Symbol table returned via mprCreateSymbolTable.
    @return Integer count of the number of entries.
    @ingroup MprHash
 */
extern int mprGetHashLength(MprHash *table);

/**
    Lookup a symbol in the hash table.
    @description Lookup a symbol key and return the value associated with that key.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param key String key of the symbole entry to delete.
    @return Value associated with the key when the entry was inserted via mprInsertSymbol.
    @ingroup MprHash
 */
extern void *mprLookupKey(MprHash *table, cvoid *key);

/**
    Lookup a symbol in the hash table and return the hash entry
    @description Lookup a symbol key and return the hash table descriptor associated with that key.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param key String key of the symbole entry to delete.
    @return MprKey for the entry
    @ingroup MprHash
 */
extern MprKey *mprLookupKeyEntry(MprHash *table, cvoid *key);

/**
    Remove a symbol entry from the hash table.
    @description Removes a symbol entry from the symbol table. The entry is looked up via the supplied \a key.
    @param table Symbol table returned via mprCreateSymbolTable.
    @param key String key of the symbole entry to delete.
    @return Returns zero if successful, otherwise a negative MPR error code is returned.
    @ingroup MprHash
 */
extern int mprRemoveKey(MprHash *table, cvoid *key);

/**
    Blend two hash tables
    @description Blend a hash table into a target hash
    @param target Target hash to receive the properties from the other hash 
    @param other Hash to provide properties to blend
    @return Returns target
    @ingroup MprHash
 */
extern MprHash *mprBlendHash(MprHash *target, MprHash *other);

/*********************************** Files ************************************/
/*
    Prototypes for file system switch methods
 */
typedef bool    (*MprAccessFileProc)(struct MprFileSystem *fs, cchar *path, int omode);
typedef int     (*MprDeleteFileProc)(struct MprFileSystem *fs, cchar *path);
typedef int     (*MprDeleteDirProc)(struct MprFileSystem *fs, cchar *path);
typedef int     (*MprGetPathInfoProc)(struct MprFileSystem *fs, cchar *path, struct MprPath *info);
typedef char   *(*MprGetPathLinkProc)(struct MprFileSystem *fs, cchar *path);
typedef int     (*MprMakeDirProc)(struct MprFileSystem *fs, cchar *path, int perms, int owner, int group);
typedef int     (*MprMakeLinkProc)(struct MprFileSystem *fs, cchar *path, cchar *target, int hard);
typedef int     (*MprCloseFileProc)(struct MprFile *file);
typedef ssize   (*MprReadFileProc)(struct MprFile *file, void *buf, ssize size);
typedef MprOff  (*MprSeekFileProc)(struct MprFile *file, int seekType, MprOff distance);
typedef int     (*MprSetBufferedProc)(struct MprFile *file, ssize initialSize, ssize maxSize);
typedef int     (*MprTruncateFileProc)(struct MprFileSystem *fs, cchar *path, MprOff size);
typedef ssize   (*MprWriteFileProc)(struct MprFile *file, cvoid *buf, ssize count);

#if !DOXYGEN
/* Work around doxygen bug */
typedef struct MprFile* (*MprOpenFileProc)(struct MprFileSystem *fs, cchar *path, int omode, int perms);
#endif

/**
    File system service
    @description The MPR provides a file system abstraction to support non-disk based file access such as flash or 
        other ROM based file systems. The MprFileSystem structure defines a virtual file system interface that
        will be invoked by the various MPR file routines.
    @see MprRomInode mprAddFileSystem mprCreateDiskFileSystem mprCreateFileSystem mprCreateRomFileSystem 
        mprLookupFileSystem mprSetPathNewline mprSetPathSeparators mprSetRomFileSystem 
    @defgroup MprFileSystem MprFileSystem
 */
typedef struct MprFileSystem {
    MprAccessFileProc   accessPath;     /**< Virtual access file routine */
    MprDeleteFileProc   deletePath;     /**< Virtual delete file routine */
    MprGetPathInfoProc  getPathInfo;    /**< Virtual get file information routine */
    MprGetPathLinkProc  getPathLink;    /**< Virtual get the symbolic link target */
    MprMakeDirProc      makeDir;        /**< Virtual make directory routine */
    MprMakeLinkProc     makeLink;       /**< Virtual make link routine */
    MprOpenFileProc     openFile;       /**< Virtual open file routine */
    MprCloseFileProc    closeFile;      /**< Virtual close file routine */
    MprReadFileProc     readFile;       /**< Virtual read file routine */
    MprSeekFileProc     seekFile;       /**< Virtual seek file routine */
    MprSetBufferedProc  setBuffered;    /**< Virtual set buffered I/O routine */
    MprWriteFileProc    writeFile;      /**< Virtual write file routine */
    MprTruncateFileProc truncateFile;   /**< Virtual truncate file routine */
    bool                caseSensitive;  /**< Path comparisons are case sensitive */
    bool                hasDriveSpecs;  /**< Paths can have drive specifications */
    char                *separators;    /**< Filename path separators. First separator is the preferred separator. */
    char                *newline;       /**< Newline for text files */
    cchar               *root;          /**< Root file path */
#if BIT_WIN_LIKE || CYGWIN
    char                *cygwin;        /**< Cygwin install directory */
    char                *cygdrive;      /**< Cygwin drive root */
#endif
} MprFileSystem;


#if BIT_ROM
/**
    A RomInode is created for each file in the Rom file system.
    @ingroup FileSystem
 */
typedef struct  MprRomInode {
    char            *path;              /**< File path */
    uchar           *data;              /**< Pointer to file data */
    int             size;               /**< Size of file */
    int             num;                /**< Inode number */
} MprRomInode;

typedef struct MprRomFileSystem {
    MprFileSystem   fileSystem;
    MprHash         *fileIndex;
    MprRomInode     *romInodes;
    int             rootLen;
} MprRomFileSystem;
#else /* !BIT_ROM */

typedef MprFileSystem MprDiskFileSystem;
#endif

/*
    File system initialization routines
 */
/**
    Create and initialize the FileSystem subsystem. 
    @description This is an internal routine called by the MPR during initialization.
    @param path Path name to the root of the file system.
    @return Returns a new file system object
    @ingroup MprFileSystem
 */
extern MprFileSystem *mprCreateFileSystem(cchar *path);


#if BIT_ROM
/**
    Create and initialize the ROM FileSystem. 
    @description This is an internal routine called by the MPR during initialization.
    @param path Path name to the root of the file system.
    @return Returns a new file system object
    @ingroup MprFileSystem
*/
extern MprRomFileSystem *mprCreateRomFileSystem(cchar *path);

/**
    Set the ROM file system data. 
    @description This defines the list of files present in the ROM file system. Use makerom to generate the inodeList data.
    @param inodeList Reference to the ROM file system list of files (inodes). This is generated by the makerom tool.
    @return Returns zero if successful.
    @ingroup MprFileSystem
 */
extern int mprSetRomFileSystem(MprRomInode *inodeList);
#else

/**
    Create and initialize the disk FileSystem. 
    @description This is an internal routine called by the MPR during initialization.
    @param path Path name to the root of the file system.
    @return Returns a new file system object
    @ingroup MprFileSystem
 */
extern MprDiskFileSystem *mprCreateDiskFileSystem(cchar *path);
#endif

/**
    Create and initialize the disk FileSystem. 
    @description This is an internal routine called by the MPR during initialization.
    @param fs File system object 
    @ingroup MprFileSystem
 */
extern void mprAddFileSystem(MprFileSystem *fs);

/**
    Lookup a file system
    @param path Path representing a file in the file system.
    @return Returns a file system object.
    @ingroup MprFileSystem
  */
extern MprFileSystem *mprLookupFileSystem(cchar *path);

/**
    Set the file system path separators
    @param path Path representing a file in the file system.
    @param separators String containing the directory path separators. Defaults to "/". Windows uses "/\/".
    @ingroup MprFileSystem
 */
extern void mprSetPathSeparators(cchar *path, cchar *separators);

/**
    Set the file system new line character string
    @param path Path representing a file in the file system.
    @param newline String containing the newline character(s). "\\n". Windows uses "\\r\\n".
    @ingroup MprFileSystem
 */
extern void mprSetPathNewline(cchar *path, cchar *newline);

/**
    File I/O Module
    @description MprFile is the cross platform File I/O abstraction control structure. An instance will be
         created when a file is created or opened via #mprOpenFile.
    @stability Evolving.
    @see MprFile mprAttachFileFd mprCloseFile mprDisableFileBuffering mprEnableFileBuffering mprFlushFile mprGetFileChar 
        mprGetFilePosition mprGetFileSize mprGetStderr mprGetStdin mprGetStdout mprOpenFile 
        mprPeekFileChar mprPutFileChar mprPutFileString mprReadFile mprReadLine mprSeekFile mprTruncateFile mprWriteFile 
        mprWriteFormat mprWriteString 
        mprGetFileFd
    @defgroup MprFile MprFile
 */
typedef struct MprFile {
    char            *path;              /**< Filename */
    MprFileSystem   *fileSystem;        /**< File system owning this file */
    MprBuf          *buf;               /**< Buffer for I/O if buffered */
    MprOff          pos;                /**< Current read position  */
    MprOff          iopos;              /**< Raw I/O position  */
    MprOff          size;               /**< Current file size */
    int             mode;               /**< File open mode */
    int             perms;              /**< File permissions */
    int             fd;                 /**< File handle */
    int             attached;           /**< Attached to existing descriptor */
#if BIT_ROM
    MprRomInode     *inode;             /**< Reference to ROM file */
#endif
} MprFile;


/**
    Attach to an existing file descriptor
    @description Attach a file to an open file decriptor and return a file object.
    @param fd File descriptor to attach to
    @param name Descriptive name for the file.
    @param omode Posix style file open mode mask. The open mode may contain 
        the following mask values ored together:
        @li O_RDONLY Open read only
        @li O_WRONLY Open write only
        @li O_RDWR Open for read and write
        @li O_CREAT Create or re-create
        @li O_TRUNC Truncate
        @li O_BINARY Open for binary data
        @li O_TEXT Open for text data
        @li O_EXCL Open with an exclusive lock
        @li O_APPEND Open to append
    @return Returns an MprFile object to use in other file operations.
    @ingroup MprFile
 */
extern MprFile *mprAttachFileFd(int fd, cchar *name, int omode);

/**
    Close a file
    @description This call closes a file without destroying the file object. 
    @param file File instance returned from #mprOpenFile
    @return Returns zero if successful, otherwise a negative MPR error code..
    @ingroup MprFile
*/
extern int mprCloseFile(MprFile *file);

/**
    Disable file buffering
    @description Disable any buffering of data when using the buffer.
    @param file File instance returned from #mprOpenFile
    @ingroup MprFile
 */
extern void mprDisableFileBuffering(MprFile *file);

/**
    Enable file buffering
    @description Enable data buffering when using the buffer.
    @param file File instance returned from #mprOpenFile
    @param size Size to allocate for the buffer.
    @param maxSize Maximum size the data buffer can grow to
    @ingroup MprFile
 */
extern int mprEnableFileBuffering(MprFile *file, ssize size, ssize maxSize);

/**
    Flush any buffered write data
    @description Write buffered write data and then reset the internal buffers.
    @param file Pointer to an MprFile object returned via MprOpen.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup MprFile
 */
extern int mprFlushFile(MprFile *file);

/**
    Read a character from the file.
    @description Read a single character from the file and advance the read position.
    @param file Pointer to an MprFile object returned via MprOpen.
    @return If successful, return the character just read. Otherwise return a negative MPR error code.
        End of file is signified by reading 0.
    @ingroup MprFile
 */
extern int mprGetFileChar(MprFile *file);

/**
    Get the file descriptor for a file
    @param file File object returned via #mprOpenFile
    @return An integer O/S file descriptor
    @ingroup MprFile
 */
extern int mprGetFileFd(MprFile *file);

/**
    Return the current file position
    @description Return the current read/write file position.
    @param file A file object returned from #mprOpenFile
    @returns The current file offset position if successful. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern MprOff mprGetFilePosition(MprFile *file);

/**
    Get the size of the file
    @description Return the current file size
    @param file A file object returned from #mprOpenFile
    @returns The current file size if successful. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern MprOff mprGetFileSize(MprFile *file);

/**
    Return a file object for the Stderr I/O channel
    @returns A file object
    @ingroup MprFile
 */
extern MprFile *mprGetStderr();

/**
    Return a file object for the Stdin I/O channel
    @returns A file object
    @ingroup MprFile
 */
extern MprFile *mprGetStdin();

/**
    Return a file object for the Stdout I/O channel
    @returns A file object
    @ingroup MprFile
 */
extern MprFile *mprGetStdout();

/**
    Open a file
    @description Open a file and return a file object.
    @param filename String containing the filename to open or create.
    @param omode Posix style file open mode mask. The open mode may contain 
        the following mask values ored together:
        @li O_RDONLY Open read only
        @li O_WRONLY Open write only
        @li O_RDWR Open for read and write
        @li O_CREAT Create or re-create
        @li O_TRUNC Truncate
        @li O_BINARY Open for binary data
        @li O_TEXT Open for text data
        @li O_EXCL Open with an exclusive lock
        @li O_APPEND Open to append
    @param perms Posix style file permissions mask.
    @return Returns an MprFile object to use in other file operations.
    @ingroup MprFile
 */
extern MprFile *mprOpenFile(cchar *filename, int omode, int perms);

/**
    Non-destructively read a character from the file.
    @description Read a single character from the file without advancing the read position.
    @param file Pointer to an MprFile object returned via MprOpen.
    @return If successful, return the character just read. Otherwise return a negative MPR error code.
        End of file is signified by reading 0.
    @ingroup MprFile
 */
extern int mprPeekFileChar(MprFile *file);

/**
    Write a character to the file.
    @description Writes a single character to the file. Output is buffered and is
        flushed as required or when mprClose is called.
    @param file Pointer to an MprFile object returned via MprOpen.
    @param c Character to write
    @return One if successful, otherwise returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern ssize mprPutFileChar(MprFile *file, int c);

/**
    Write a string to the file.
    @description Writes a string to the file. Output is buffered and is flushed as required or when mprClose is called.
    @param file Pointer to an MprFile object returned via MprOpen.
    @param str String to write
    @return The number of characters written to the file. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern ssize mprPutFileString(MprFile *file, cchar *str);

/**
    Read data from a file.
    @description Reads data from a file. 
    @param file Pointer to an MprFile object returned via MprOpen.
    @param buf Buffer to contain the read data.
    @param size Size of \a buf in characters.
    @return The number of characters read from the file. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern ssize mprReadFile(MprFile *file, void *buf, ssize size);

/**
    Read a line from the file.
    @description Read a single line from the file. Lines are delimited by the newline character. The newline is not 
        included in the returned buffer. This call will read lines up to the given size in length. If no newline is
        found, all available characters, up to size, will be returned.
    @param file Pointer to an MprFile object returned via MprOpen.
    @param size Maximum number of characters in a line.
    @param len Pointer to an integer to hold the length of the returned string.
    @return An allocated string and sets *len to the number of bytes read. 
    @ingroup MprFile
 */
extern char *mprReadLine(MprFile *file, ssize size, ssize *len);

/**
    Seek the I/O pointer to a new location in the file.
    @description Move the position in the file to/from which I/O will be performed in the file. Seeking prior 
        to a read or write will cause the next I/O to occur at that location.
    @param file Pointer to an MprFile object returned via MprOpen.
    @param seekType Seek type may be one of the following three values:
        @li SEEK_SET    Seek to a position relative to the start of the file
        @li SEEK_CUR    Seek relative to the current position
        @li SEEK_END    Seek relative to the end of the file
    @param distance A positive or negative byte offset.
    @return Returns the new file position if successful otherwise a negative MPR error code is returned.
    @ingroup MprFile
 */
extern MprOff mprSeekFile(MprFile *file, int seekType, MprOff distance);

/**
    Truncate a file
    @description Truncate a file to a given size. Note this works on a path and not on an open file.
    @param path File to truncate
    @param size New maximum size for the file.
    @returns Zero if successful.
    @ingroup MprFile
 */
extern int mprTruncateFile(cchar *path, MprOff size);

/**
    Write data to a file.
    @description Writes data to a file. 
    @param file Pointer to an MprFile object returned via MprOpen.
    @param buf Buffer containing the data to write.
    @param count Cound of characters in \a buf to write
    @return The number of characters actually written to the file. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern ssize mprWriteFile(MprFile *file, cvoid *buf, ssize count);

/**
    Write formatted data to a file.
    @description Writes a formatted string to a file. 
    @param file Pointer to an MprFile object returned via MprOpen.
    @param fmt Format string
    @return The number of characters actually written to the file. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern ssize mprWriteFileFmt(MprFile *file, cchar *fmt, ...);

/**
    Write a string to a file.
    @description Writes a string to a file. 
    @param file Pointer to an MprFile object returned via MprOpen.
    @param str String to write
    @return The number of characters actually written to the file. Returns a negative MPR error code on errors.
    @ingroup MprFile
 */
extern ssize mprWriteFileString(MprFile *file, cchar *str);

/*********************************** Paths ************************************/
/**
    Path (filename) Information
    @description MprPath is the cross platform Path (filename) information structure.
    @stability Evolving.
    @see MprDirEntry MprFile MprPath mprCopyPath mprDeletePath mprGetAbsPath mprGetCurrentPath 
        mprGetFirstPathSeparator mprGetLastPathSeparator mprGetNativePath mprGetPathBase 
        mprGetPathDir mprGetPathExt mprGetPathFiles mprGetPathLink mprGetPathNewline mprGetPathParent 
        mprGetPathSeparators mprGetPortablePath mprGetRelPath mprGetTempPath mprGetWinPath mprIsAbsPath 
        mprIsRelPath mprJoinPath mprJoinPathExt mprMakeDir mprMakeLink mprMapSeparators mprNormalizePath
        mprPathExists mprReadPathContents mprReplacePathExt mprResolvePath mprSamePath mprSamePathCount mprSearchPath 
        mprTransformPath mprTrimPathExt mprTruncatePath 
    @defgroup MprPath MprPath
 */
typedef struct MprPath {
    MprTime         atime;              /**< Access time */
    MprTime         ctime;              /**< Create time */
    MprTime         mtime;              /**< Modified time */
    MprOff          size;               /**< File length */
    int64           inode;              /**< Inode number */
    bool            isDir;              /**< Set if directory */
    bool            isLink;             /**< Set if a symbolic link  */
    bool            isReg;              /**< Set if a regular file */
    bool            caseMatters;        /**< Case comparisons matter */
    int             perms;              /**< Permission mask */
    int             owner;              /**< Owner ID */
    int             group;              /**< Group ID */
    int             valid;              /**< Valid data bit */
    int             checked;            /**< Path has been checked */
} MprPath;

/**
    Directory entry description
    @description The MprGetDirList will create a list of directory entries.
    @ingroup MprPath
 */
typedef struct MprDirEntry {
    char            *name;              /**< Name of the file */
    MprTime         lastModified;       /**< Time the file was last modified */
    MprOff          size;               /**< Size of the file */
    bool            isDir;              /**< True if the file is a directory */
    bool            isLink;             /**< True if the file is a symbolic link */
} MprDirEntry;

/*
    Search path separator
 */
#if BIT_WIN_LIKE
    #define MPR_SEARCH_SEP      ";"
    #define MPR_SEARCH_SEP_CHAR ';'
#else
    #define MPR_SEARCH_SEP      ":"
    #define MPR_SEARCH_SEP_CHAR ':'
#endif

/**
    Copy a file
    @description Create a new copy of a file with the specified open permissions mode.
    @param from Path of the existing file to copy
    @param to Name of the new file copy
    @param omode Posix style file open mode mask. See #mprOpenFile for the various modes.
    @returns True if the file exists and can be accessed
    @ingroup MprPath
 */
extern int mprCopyPath(cchar *from, cchar *to, int omode);

/**
    Delete a file.
    @description Delete a file or directory.
    @param path String containing the path to delete. 
    @return Returns zero if successful otherwise a negative MPR error code is returned.
    @ingroup MprPath
 */
extern int mprDeletePath(cchar *path);

/**
    Convert a path to an absolute path
    @description Get an absolute (canonical) equivalent representation of a path. On windows this path will have
        back-slash directory separators and will have a drive specifier. On Cygwin, the path will be a Cygwin style 
        path with forward-slash directory specifiers and without a drive specifier. If the path is outside the
        cygwin filesystem (outside c:/cygwin), the path will have a /cygdrive/DRIVE prefix. To get a windows style 
        path, use mprGetWinPath.
    @param path Path to examine
    @returns An absolute path.
    @ingroup MprPath
 */
extern char *mprGetAbsPath(cchar *path);

/**
    Return the current working directory
    @return Returns an allocated string with the current working directory as an absolute path.
 */
extern char *mprGetCurrentPath();

/**
    Get the first path separator in a path
    @param path Path to examine
    @return Returns a reference to the first path separator in the given path
 */
extern cchar *mprGetFirstPathSeparator(cchar *path);

/*
    Get the last path separator in a path
    @param path Path to examine
    @return Returns a reference to the last path separator in the given path
 */
extern cchar *mprGetLastPathSeparator(cchar *path);

/**
    Get a path formatted according to the native O/S conventions.
    @description Get an equivalent absolute path formatted using the directory separators native to the O/S platform.
    On Windows, it will use backward slashes ("\") as the directory separator and will contain a drive specification.
    @param path Path name to examine
    @returns An allocated string containing the new path.
    @ingroup MprPath
 */
extern char *mprGetNativePath(cchar *path);

/**
    Get the base portion of a path
    @description Get the base portion of a path by stripping off all directory components
    @param path Path name to examine
    @returns A path without any directory portion.
    @ingroup MprPath
 */
extern char *mprGetPathBase(cchar *path);

/**
    Get a reference to the base portion of a path
    @description Get the base portion of a path by stripping off all directory components. This returns a reference
        into the original path.
    @param path Path name to examine
    @returns A path without any directory portion. The path is a reference into the original file string.
    @ingroup MprPath
 */
extern cchar *mprGetPathBaseRef(cchar *path);

/**
    Get the directory portion of a path
    @description Get the directory portion of a path by stripping off the base name.
    @param path Path name to examine
    @returns A new string containing the directory name.
    @ingroup MprPath
 */
extern char *mprGetPathDir(cchar *path);

/**
    Get the file extension portion of a path
    @description Get the file extension portion of a path. The file extension is the portion starting with the last "."
        in the path. It thus includes "." as the first charcter.
    @param path Path name to examine
    @returns A path extension. 
    @ingroup MprPath
 */
extern char *mprGetPathExt(cchar *path);

/*
    Flags for mprGetPathFiles
 */
#define MPR_PATH_DESCEND        0x1             /**< Flag for mprGetPathFiles to traverse subdirectories */
#define MPR_PATH_DEPTH_FIRST    0x2             /**< Flag for mprGetPathFiles to do a depth-first traversal */
#define MPR_PATH_INC_HIDDEN     0x4             /**< Flag for mprGetPathFiles to include hidden files */
#define MPR_PATH_NODIRS         0x8             /**< Flag for mprGetPathFiles to exclude subdirectories */
#define MPR_PATH_RELATIVE       0x10            /**< Flag for mprGetPathFiles to return paths relative to the directory */

/**
    Create a list of files in a directory or subdirectories. This call returns a list of MprDirEntry objects.
    @description Get the list of files in a directory and return a list.
    @param dir Directory to list.
    @param flags The flags may be set to #MPR_PATH_DESCEND to traverse subdirectories. Set $MPR_PATH_NO_DIRS 
        to exclude directories from the results. Set to MPR_PATH_HIDDEN to include hidden files that start with ".".
        Set to MPR_PATH_DEPTH_FIRST to do a depth-first traversal, i.e. traverse subdirectories before considering 
        adding the directory to the list.
    @returns A list (MprList) of MprDirEntry objects.
    @ingroup MprPath
 */
extern MprList *mprGetPathFiles(cchar *dir, int flags);

/**
    Get the first directory portion of a path
    @param path Path name to examine
    @returns A new string containing the directory name.
    @ingroup MprPath
 */
extern char *mprGetPathFirstDir(cchar *path);

/**
    Return information about a file represented by a path.
    @description Returns file status information regarding the \a path.
    @param path String containing the path to query.
    @param info Pointer to a pre-allocated MprPath structure.
    @return Returns zero if successful, otherwise a negative MPR error code is returned.
    @ingroup MprPath
 */
extern int mprGetPathInfo(cchar *path, MprPath *info);

/**
    Get the target of a symbolic link.
    @description Return the path pointed to by a symbolic link. Not all platforms support symbolic links.
    @param path Path name to examine
    @returns A path representing the target of the symbolic link.
    @ingroup MprPath
 */
extern char *mprGetPathLink(cchar *path);

/**
    Get the file newline character string for a given path.
    Return the character string used to delimit new lines in text files.
    @param path Use this path to specify either the root of the file system or a file on the file system.
    @returns A string used to delimit new lines. This is typically "\n" or "\r\n"
    @ingroup MprPath
 */
extern cchar *mprGetPathNewline(cchar *path);

/**
    Get the parent directory of a path
    @param path Path name to examine
    @returns An allocated string containing the parent directory.
    @ingroup MprPath
 */
extern char *mprGetPathParent(cchar *path);

/**
    Get the path directory separator.
    Return the directory separator characters used to separate directories on a given file system. Typically "/" or "\"
        The first entry is the default separator.
    @param path Use this path to specify either the root of the file system or a file on the file system.
    @returns The string of path separators. The first entry is the default separator.
    @ingroup MprPath
 */
extern cchar *mprGetPathSeparators(cchar *path);

/**
    Get a portable path 
    @description Get an equivalent absolute path that is somewhat portable. 
        This means it will use forward slashes ("/") as the directory separator. This call will not remove drive specifiers.
    @param path Path name to examine
    @returns An allocated string containing the new path.
    @ingroup MprPath
 */
extern char *mprGetPortablePath(cchar *path);

/**
    Get a relative path
    @description Get a relative path path from an origin path to a destination. 
    @param dest Destination file 
    @param origin Starting location from which to compute a relative path to the destination
        If the origin is null, use the application's current working directory as the origin.
    @returns An allocated string containing the relative directory.
    @ingroup MprPath
 */
extern char *mprGetRelPath(cchar *dest, cchar *origin);

/**
    Make a temporary file.
    @description Thread-safe way to make a unique temporary file. 
    @param tmpDir Base directory in which the temp file will be allocated.
    @return An allocated string containing the path of the temp file.
    @ingroup MprPath
 */
extern char *mprGetTempPath(cchar *tmpDir);

/**
    Convert a path to an absolute windows path
    @description Get a windows style, absolute (canonical) equivalent representation of a path. This path will 
        have back-slash delimiters and a drive specifier. On non-windows systems, this returns an absolute path using
        mprGetAbsPath.
    @param path Path to examine
    @returns A windows-style absolute path.
    @ingroup MprPath
 */
extern char *mprGetWinPath(cchar *path);

/**
    Determine if a path is absolute
    @param path Path name to examine
    @returns True if the path is absolue
    @ingroup MprPath
 */ 
extern bool mprIsPathAbs(cchar *path);

/**
    Determine if a path is a directory
    @param path Path name to examine
    @returns True if the path is a directory
    @ingroup MprPath
 */ 
extern bool mprIsPathDir(cchar *path);

/**
    Determine if a path is relative
    @param path Path name to examine
    @returns True if the path is relative
    @ingroup MprPath
 */ 
extern bool mprIsPathRel(cchar *path);

/**
    Test if a character is a path separarator
    @param path Path name to identify the file system 
    @param c Character to test
    @return Returns true if the character is a path separator on the file system containing the given path
 */
extern bool mprIsPathSeparator(cchar *path, cchar c);

/**
    Join paths
    @description Join a path to a base path. If path is absolute, it will be returned.
    @param base Directory path name to use as the base.
    @param path Other path name to join to the base path.
    @returns Allocated string containing the resolved path.
    @ingroup MprPath
 */
extern char *mprJoinPath(cchar *base, cchar *path);

/**
    Join an extension to a path
    @description Add an extension to a path if it does not already have one.
    @param path Path name to use as a base. Path is not modified.
    @param ext Extension to add. Must should not have a  period prefix.
    @returns Allocated string containing the resolved path.
    @ingroup MprPath
 */
extern char *mprJoinPathExt(cchar *path, cchar *ext);

/**
    Make a directory
    @description Make a directory using the supplied path. Intermediate directories are created as required.
    @param path String containing the directory pathname to create.
    @param makeMissing If true make all required intervening directory segments.
    @param perms Posix style file permissions mask.
    @param owner User to own the directory. Set to -1 not change the owner.
    @param group Group to own the directory. Set to -1 not change the group.
    @return Returns zero if successful, otherwise a negative MPR error code is returned.
    @ingroup MprPath
 */
extern int mprMakeDir(cchar *path, int perms, int owner, int group, bool makeMissing);

/**
    Make a link
    @description Make a link to the specified target path. This will make symbolic or hard links depending on the value
        of the hard parameter
    @param path String containing the directory pathname to create.
    @param target String containing the target file or directory to link to.
    @param hard If true, make a hard link, otherwise make a soft link.
    @return Returns zero if successful, otherwise a negative MPR error code is returned.
    @ingroup MprPath
 */
extern int mprMakeLink(cchar *path, cchar *target, bool hard);

/**
    Map the separators in a path.
    @description Map the directory separators in a path to the specified separators. This is useful to change from
        backward to forward slashes when dealing with Windows paths.
    @param path Path name to examine
    @param separator Separator character to use.
    @returns An allocated string containing the parent directory.
    @ingroup MprPath
 */
extern void mprMapSeparators(char *path, int separator);

/**
    Normalize a path
    @description A path is normalized by redundant segments such as "./" and "../dir" and duplicate 
        path separators. Path separators are mapped. Paths are not converted to absolute paths.
    @param path First path to compare
    @returns A newly allocated, clean path. 
    @ingroup MprPath
 */
extern char *mprNormalizePath(cchar *path);

/**
    Determine if a file exists for a path name and can be accessed
    @description Test if a file can be accessed for a given mode
    @param path Path name to test
    @param omode Posix style file open mode mask. See #mprOpenFile for the various modes.
    @returns True if the file exists and can be accessed
    @ingroup MprPath
 */
extern bool mprPathExists(cchar *path, int omode);

/*
    Read the contents of a file
    @param path Filename to open and read
    @param lenp Optional pointer to a ssize integer to contain the length of the returns data string. Set to NULL if not
        required.
    @return An allocated string containing the file contents and return the data length in lenp.
 */
extern char *mprReadPathContents(cchar *path, ssize *lenp);

/**
    Replace an extension to a path
    @description Remove any existing path extension and then add the given path extension.
    @param dir Directory path name to test use as the base/dir.
    @param ext Extension to add. The extension should not have a period prefix.
    @returns Allocated string containing the resolved path.
    @ingroup MprPath
 */
extern char *mprReplacePathExt(cchar *dir, cchar *ext);

/**
    Resolve paths
    @description Resolve paths in the neighborhood of this path. Resolve operates like join, except that it joins the 
    given paths to the directory portion of the current ("this") path. For example: 
    Path("/usr/bin/ejs/bin").resolve("lib") will return "/usr/lib/ejs/lib". i.e. it will return the
    sibling directory "lib".

    Resolve operates by determining a virtual current directory for this Path object. It then successively 
    joins the given paths to the directory portion of the current result. If the next path is an absolute path, 
    it is used unmodified.  The effect is to find the given paths with a virtual current directory set to the 
    directory containing the prior path.

    Resolve is useful for creating paths in the region of the current path and gracefully handles both 
    absolute and relative path segments.

    Returns a joined (normalized) path.
    If path is absolute, then return path. If path is null, empty or "." then return path.
    @param base Base path to use as the base.
    @param path Path name to resolve against base.
    @returns Allocated string containing the resolved path.
    @ingroup MprPath
 */
extern char *mprResolvePath(cchar *base, cchar *path);

/**
    Compare two paths if they are the same
    @description Compare two paths to see if they are equal. This normalizes the paths to absolute paths first before
        comparing. It does handle case sensitivity appropriately.
    @param path1 First path to compare
    @param path2 Second path to compare
    @returns True if the file exists and can be accessed
    @ingroup MprPath
 */
extern int mprSamePath(cchar *path1, cchar *path2);

/**
    Compare two paths if they are the same for a given length.
    @description Compare two paths to see if they are equal. This normalizes the paths to absolute paths first before
        comparing. It does handle case sensitivity appropriately. The len parameter 
        if non-zero, specifies how many characters of the paths to compare.
    @param path1 First path to compare
    @param path2 Second path to compare
    @param len How many characters to compare.
    @returns True if the file exists and can be accessed
    @ingroup MprPath
 */
extern int mprSamePathCount(cchar *path1, cchar *path2, ssize len);

/*
    Flags for mprSearchPath
 */
#define MPR_SEARCH_EXE      0x1         /* Search for an executable */
#define MPR_SEARCH_DIR      0x2         /* Search for a directory */
#define MPR_SEARCH_FILE     0x4         /* Search for regular file */

/**
    Search for a path
    @description Search for a file using a given set of search directories
    @param path Path name to locate. Must be an existing file or directory.
    @param flags Flags.
    @param search Variable number of directories to search.
    @returns Allocated string containing the full path name of the located file.
    @ingroup MprPath
 */
extern char *mprSearchPath(cchar *path, int flags, cchar *search, ...);

/*
    Flags for mprTransformPath
 */
#define MPR_PATH_ABS            0x1     /* Normalize to an absolute path */
#define MPR_PATH_REL            0x2     /* Normalize to an relative path */
#define MPR_PATH_WIN            0x4     /* Normalize to a windows path */
#define MPR_PATH_NATIVE_SEP     0x8     /* Use native path separators */

/**
    Transform a path
    @description A path is transformed by cleaning and then transforming according to the flags.
    @param path First path to compare
    @param flags Flags to modify the path representation.
    @returns A newly allocated, clean path.
    @ingroup MprPath
 */
extern char *mprTransformPath(cchar *path, int flags);

/**
    Trim an extension from a path
    @description Trim a file extension (".ext") from a path name.
    @param path Path to examine
    @returns An allocated string with the trimmed path.
    @ingroup MprPath
 */
extern char *mprTrimPathExt(cchar *path);

/**
    Trim the drive from a path
    @description Trim a drive specifier ("c:") from the start of a path.
    @param path Path to examine
    @returns An allocated string with the trimmed drive.
    @ingroup MprPath
 */
extern char *mprTrimPathDrive(cchar *path);

/**
    Create a file and write contents
    @description The file is created, written and closed. If the file already exists, it is recreated.
    @param path Filename to create
    @param buf Buffer of data to write to the file
    @param len Size of the buf parameter in bytes
    @param mode File permissions with which to create the file. E.g. 0644.
    @return The number of bytes written. Should equal len. Otherwise return a negative MPR error code.
 */
extern ssize mprWritePathContents(cchar *path, cchar *buf, ssize len, int mode);

/********************************** O/S Dep ***********************************/
/**
    Create and initialze the O/S dependent subsystem
    @ingroup Mpr
 */
extern int mprCreateOsService();

/**
    Start the O/S dependent subsystem
    @ingroup Mpr
 */
extern int mprStartOsService();

/**
    Stop the O/S dependent subsystem
    @ingroup Mpr
 */
extern void mprStopOsService();

/********************************* Modules ************************************/
/**
    Loadable module service
    @see mprCreateModuleService mprStartModuleService mprStopModuleService
    @defgroup MprModuleSerivce MprModuleService
 */
typedef struct MprModuleService {
    MprList         *modules;               /**< List of defined modules */
    char            *searchPath;            /**< Module search path to locate modules */
    struct MprMutex *mutex;
} MprModuleService;


/**
    Create and initialize the module service
    @return MprModuleService object
    @ingroup MprModuleService
 */
extern MprModuleService *mprCreateModuleService();

/**
    Start the module service
    @description This calls the start entry point for all registered modules
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup MprModuleService
 */
extern int mprStartModuleService();

/**
    Stop the module service
    @description This calls the stop entry point for all registered modules
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup MprModuleService
 */
extern void mprStopModuleService();

/**
    Module start/stop point function signature
    @param mp Module object reference returned from #mprCreateModule
    @returns zero if successful, otherwise return a negative MPR error code.
    @ingroup MprModule
 */ 
typedef int (*MprModuleProc)(struct MprModule *mp);

/*
    Module flags
 */
#define MPR_MODULE_STARTED     0x1     /**< Module stared **/
#define MPR_MODULE_STOPPED     0x2     /**< Module stopped */

/**
    Loadable Module Service
    @description The MPR provides services to load and unload shared libraries.
    @see MprModule MprModuleEntry MprModuleProc mprCreateModule mprGetModuleSearchPath mprLoadModule mprLoadNativeModule 
        mprLookupModule mprLookupModuleData mprSearchForModule mprSetModuleFinalizer mprSetModuleSearchPath 
        mprSetModuleTimeout mprStartModule mprStopModule mprUnloadModule mprUnloadNativeModule 
    @stability Evolving.
    @defgroup MprModule MprModule
 */
typedef struct MprModule {
    char            *name;              /**< Unique module name */
    char            *path;              /**< Module library filename */
    char            *entry;             /**< Module library init entry point */
    void            *moduleData;        /**< Module specific data - must be alloced data */
    void            *handle;            /**< O/S shared library load handle */
    MprTime         modified;           /**< When the module file was last modified */
    MprTime         lastActivity;       /**< When the module was last used */
    MprTime         timeout;            /**< Inactivity unload timeout */
    int             flags;              /**< Module control flags */
    MprModuleProc   start;              /**< Start the module */
    MprModuleProc   stop;               /**< Stop the module. Should be unloadable after stopping */
} MprModule;

/**
    Loadable module entry point signature. 
    @description Loadable modules can have an entry point that is invoked automatically when a module is loaded. 
    @param data Data passed to mprCreateModule
    @param mp Module object reference returned from #mprCreateModule
    @return a new MprModule structure for the module. Return NULL if the module can't be initialized.
    @ingroup MprModule
 */
typedef int (*MprModuleEntry)(void *data, MprModule *mp);

/**
    Create a module
    @description This call will create a module object for a loadable module. This should be invoked by the 
        module itself in its module entry point to register itself with the MPR.
    @param name Name of the module
    @param path Optional filename of a module library to load. When loading, the filename will be searched using 
        the defined module search path (see #mprSetModuleSearchPath). The filename may or may not include a platform 
        specific shared library extension such as .dll, .so or .dylib. By omitting the library extension, code can 
        portably load shared libraries.
    @param entry Name of function to invoke after loading the module.
    @param data Arbitrary data pointer. This will be defined in MprModule.data and passed into the module initialization
        entry point.
    @returns A module object for this module
    @ingroup MprModule
 */
extern MprModule *mprCreateModule(cchar *name, cchar *path, cchar *entry, void *data);

/**
    Get the module search path
    @description Get the directory search path used by the MPR when loading dynamic modules. This is a colon separated (or 
        semicolon on Windows) set of directories.
    @returns The module search path.
    @ingroup MprModule
 */
extern cchar *mprGetModuleSearchPath();

/**
    Load a module
    @description Load a module library. This will load a dynamic shared object (shared library) and call the
        modules library entry point. If the module is already loaded, this call will do nothing.
    @param mp Module object created via $mprCreateModule.
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup MprModule
 */
extern int mprLoadModule(MprModule *mp);

#if BIT_HAS_DYN_LOAD || DOXYGEN
/**
    Load a native module
    @param mp Module object created via $mprCreateModule.
    @returns Zero if successful, otherwise a negative MPR error code.
*/
extern int mprLoadNativeModule(MprModule *mp);

/**
    Unload a native module
    @param mp Module object created via $mprCreateModule.
    @returns Zero if successful, otherwise a negative MPR error code.
*/
extern int mprUnloadNativeModule(MprModule *mp);
#endif

/**
    Lookup a module
    @description Lookup a module by name and return the module object.
    @param name Name of the module specified to #mprCreateModule.
    @returns A module object for this module created in the module entry point by calling #mprCreateModule
    @ingroup MprModule
 */
extern MprModule *mprLookupModule(cchar *name);

/**
    Lookup a module and return the module data
    @description Lookup a module by name and return the module specific data defined via #mprCreateModule.
    @param name Name of the module specified to #mprCreateModule.
    @returns The module data.
    @ingroup MprModule
 */
extern void *mprLookupModuleData(cchar *name);

/**
    Search for a module on the current module path
    @param module Name of the module to locate.
    @return A string containing the full path to the module. Returns NULL if the module filename cannot be found.
 */
extern char *mprSearchForModule(cchar *module);

/**
    Define a module finalizer that will be called before a module is stopped
    @param module Module object to modify
    @param stop Callback function to invoke before stopping the module
 */
extern void mprSetModuleFinalizer(MprModule *module, MprModuleProc stop);

/**
    Set the module search path
    @description Set the directory search path used by the MPR when loading dynamic modules. This path string must
        should be a colon separated (or semicolon on Windows) set of directories. 
    @param searchPath Colon separated set of directories
    @returns The module search path.
    @ingroup MprModule
 */
extern void mprSetModuleSearchPath(char *searchPath);

/**
    Set a module timeout
    @param module Module object to modify
    @param timeout Inactivity timeout in milliseconds before unloading the module
    @internal
 */
extern void mprSetModuleTimeout(MprModule *module, MprTime timeout);

/**
    Start a module
    @description Invoke the module start entry point. The start routine is only called once.
    @param mp Module object returned via #mprLookupModule
    @ingroup MprModule
 */
extern int mprStartModule(MprModule *mp);

/**
    Stop a module
    @description Invoke the module stop entry point. The stop routine is only called once.
    @param mp Module object returned via #mprLookupModule
    @ingroup MprModule
 */
extern int mprStopModule(MprModule *mp);

/**
    Unload a module
    @description Unload a module from the MPR. This will unload a dynamic shared object (shared library). This routine
        is not fully supported by the MPR and is often fraught with issues. A module must usually be completely inactive 
        with no allocated memory when it is unloaded. USE WITH CARE.
    @param mp Module object returned via #mprLookupModule
    @return Zero if the module can be unloaded. Otherwise a negative MPR error code.
    @ingroup MprModule
 */
extern int mprUnloadModule(MprModule *mp);

/********************************* Events *************************************/
/*
    Flags for mprCreateEvent
 */
#define MPR_EVENT_CONTINUOUS        0x1     /**< Timer event runs is automatically rescheduled */
#define MPR_EVENT_QUICK             0x2     /**< Execute inline without executing via a thread */
#define MPR_EVENT_DONT_QUEUE        0x4     /**< Don't queue the event. User must call mprQueueEvent */
#define MPR_EVENT_STATIC_DATA       0x8     /**< Event data is permanent and should not be marked by GC */

#define MPR_EVENT_MAGIC         0x12348765
#define MPR_DISPATCHER_MAGIC    0x23418877

/**
    Event callback function
    @return Return non-zero if the dispatcher is deleted. Otherwise return 0
    @ingroup MprEvent
 */
typedef void (*MprEventProc)(void *data, struct MprEvent *event);

/**
    Event object
    @description The MPR provides a powerful priority based eventing mechanism. Events are described by MprEvent objects
        which are created and queued via #mprCreateEvent. Each event may have a priority and may be one-shot or 
        be continuously rescheduled according to a specified period. The event subsystem provides the basis for 
        callback timers. 
    @see MprDispatcher MprEvent MprEventProc MprEventService mprCreateDispatcher mprCreateEvent mprCreateEventService 
        mprCreateTimerEvent mprDestroyDispatcher mprEnableContinuousEvent mprEnableDispatcher mprGetDispatcher 
        mprQueueEvent mprRemoveEvent mprRescheduleEvent mprRestartContinuousEvent mprServiceEvents 
        mprSignalDispatcher mprStopContinuousEvent mprWaitForEvent mprCreateEventOutside
    @defgroup MprEvent MprEvent
 */
typedef struct MprEvent {
    int magic;
    cchar               *name;          /**< Static debug name of the event */
    MprEventProc        proc;           /**< Callback procedure */
    MprTime             timestamp;      /**< When was the event created */
    MprTime             due;            /**< When is the event due */
    void                *data;          /**< Event private data */
    int                 fd;             /**< File descriptor if an I/O event */
    int                 continuous;     /**< Event runs continuously */
    int                 flags;          /**< Event flags */
    int                 mask;           /**< I/O mask of events */
    MprTime             period;         /**< Reschedule period */
    struct MprEvent     *next;          /**< Next event linkage */
    struct MprEvent     *prev;          /**< Previous event linkage */
    struct MprDispatcher *dispatcher;   /**< Event dispatcher service */
    struct MprWaitHandler *handler;     /**< Optional wait handler */
} MprEvent;

/*
    Event Dispatcher
 */
typedef struct MprDispatcher {
    int             magic;
    cchar           *name;              /**< Dispatcher name / purpose */
    MprEvent        *eventQ;            /**< Event queue */
    MprEvent        *current;           /**< Current event */
    MprCond         *cond;              /**< Multi-thread sync */
    int             enabled;            /**< Dispatcher enabled to run events */
    int             waitingOnCond;      /**< Waiting on the cond */
    int             destroyed;          /**< Dispatcher has been destroyed */
    struct MprDispatcher *next;         /**< Next dispatcher linkage */
    struct MprDispatcher *prev;         /**< Previous dispatcher linkage */
    struct MprDispatcher *parent;       /**< Queue pointer */
    struct MprEventService *service;
    struct MprWorker *requiredWorker;   /**< Worker affinity */
    MprOsThread     owner;              /**< Owning thread of the dispatcher */
} MprDispatcher;


typedef struct MprEventService {
    MprTime         now;                /**< Current notion of time for the dispatcher service */
    MprTime         willAwake;          /**< Time the even service will next awake */
    MprDispatcher   *runQ;              /**< Queue of running dispatchers */
    MprDispatcher   *readyQ;            /**< Queue of dispatchers with events ready to run */
    MprDispatcher   *waitQ;             /**< Queue of waiting (future) events */
    MprDispatcher   *idleQ;             /**< Queue of idle dispatchers */
    MprDispatcher   *pendingQ;          /**< Queue of pending dispatchers (waiting for resources) */
    MprOsThread     serviceThread;      /**< Thread running the dispatcher service */
    int             eventCount;         /**< Count of events */
    int             waiting;            /**< Waiting for I/O (sleeping) */
    struct MprCond  *waitCond;          /**< Waiting sync */
    struct MprMutex *mutex;             /**< Multi-thread sync */
} MprEventService;

/**
    Create a new event dispatcher
    @param name Useful name for debugging
    @param enable If true, enable the dispatcher
    @returns a Dispatcher object that can manage events and be used with mprCreateEvent
 */
extern MprDispatcher *mprCreateDispatcher(cchar *name, int enable);

/**
    Destroy a dispatcher
    @param dispatcher Dispatcher to destroy
 */
extern void mprDestroyDispatcher(MprDispatcher *dispatcher);

/**
    Enable a dispatcher to service events. The mprCreateDispatcher routiner creates dispatchers in the disabled state.
    Use mprEnableDispatcher to enable them to begin servicing events.
    @param dispatcher Dispatcher to enable
 */
extern void mprEnableDispatcher(MprDispatcher *dispatcher);

/**
    Get the MPR primary dispatcher
    @returns the MPR dispatcher object
 */
extern MprDispatcher *mprGetDispatcher();

/*
    mprServiceEvents parameters
 */
#define MPR_SERVICE_ONE_THING   0x4         /**< Wait for one event or one I/O */
#define MPR_SERVICE_NO_GC       0x8         /**< Don't run GC */

/**
    Service events. This can be called by any thread. Typically an app will dedicate one thread to be an event service 
    thread. This call will service events until the timeout expires or if MPR_SERVICE_ONE_THING is specified in flags, 
    after one event. This will service all enabled dispatcher queues and pending I/O events.
    @param delay Time in milliseconds to wait. Set to zero for no wait. Set to -1 to wait forever.
    @param flags If set to MPR_SERVICE_ONE_THING, this call will service at most one event. Otherwise set to zero.
    @returns The number of events serviced. Returns MPR_ERR_BUSY is another thread is servicing events and timeout is zero.
    @ingroup MprEvent
 */
extern int mprServiceEvents(MprTime delay, int flags);

/**
    Wait for an event to occur on the given dispatcher
    @param dispatcher Event dispatcher to monitor
    @param timeout for waiting in milliseconds
    @return Zero if successful and an event occurred before the timeout expired. Returns #MPR_ERR_TIMEOUT if no event
        is fired before the timeout expires.
 */
extern int mprWaitForEvent(MprDispatcher *dispatcher, MprTime timeout);

/**
    Signal the dispatcher to wakeup and re-examine its queues
    @param dispatcher Event dispatcher to monitor
    @internal
 */
extern void mprSignalDispatcher(MprDispatcher *dispatcher);

/**
    Create a new event
    @description Create a new event for service
    @param dispatcher Dispatcher object created via mprCreateDispatcher
    @param name Debug name of the event
    @param period Time in milliseconds used by continuous events between firing of the event.
    @param proc Function to invoke when the event is run
    @param data Data to associate with the event and stored in event->data. The data must be either an allocated memory
        object or MPR_EVENT_STATIC_DATA must be specified in flags.
    @param flags Flags to modify the behavior of the event. Valid values are: MPR_EVENT_CONTINUOUS to create an 
        event which will be automatically rescheduled accoring to the specified period.
        Use MPR_EVENT_STATIC_DATA if the data argument does not point to an allocated memory object.
    @return Returns the event object if successful.
    @ingroup MprEvent
 */
extern MprEvent *mprCreateEvent(MprDispatcher *dispatcher, cchar *name, MprTime period, void *proc, void *data, int flags);

/**
    Create an event outside the MPR
    @description Create a new event when executing a non-MPR thread
    @param dispatcher Dispatcher object created via mprCreateDispatcher
    @param proc Function to invoke when the event is run
    @param data Data to associate with the event and stored in event->data. The data must be non-MPR memory.
    @ingroup MprEvent
 */
extern void mprCreateEventOutside(MprDispatcher *dispatcher, void *proc, void *data);

/*
    Queue a new event for service.
    @description Queue an event for service
    @param dispatcher Dispatcher object created via mprCreateDispatcher
    @param event Event object to queue
    @ingroup MprEvent
 */
extern void mprQueueEvent(MprDispatcher *dispatcher, MprEvent *event);

/**
    Remove an event
    @description Remove a queued event. This is useful to remove continuous events from the event queue.
    @param event Event object returned from #mprCreateEvent
    @ingroup MprEvent
 */
extern void mprRemoveEvent(MprEvent *event);

/**
    Stop an event
    @description Stop a continuous event and remove from the queue.
    @param event Event object returned from #mprCreateEvent
    @ingroup MprEvent
 */
extern void mprStopContinuousEvent(MprEvent *event);

/**
    Restart an event
    @description Restart a continuous event after it has been stopped via #mprStopContinuousEvent. This call will 
        add the event to the event queue and it will run after the configured event period has expired.
    @param event Event object returned from #mprCreateEvent
    @ingroup MprEvent
 */
extern void mprRestartContinuousEvent(MprEvent *event);

/**
    Enable or disable an event being continous
    @description This call will modify the continuous property for an event. 
    @param event Event object returned from #mprCreateEvent
    @param enable Set to 1 to enable continous scheduling of the event
    @ingroup MprEvent
 */
extern void mprEnableContinuousEvent(MprEvent *event, int enable);

/**
    Create a timer event
    @description Create and queue a timer event for service. This is a convenience wrapper to create continuous
        events over the #mprCreateEvent call.
    @param dispatcher Dispatcher object created via #mprCreateDispatcher
    @param name Debug name of the event
    @param proc Function to invoke when the event is run
    @param period Time in milliseconds used by continuous events between firing of the event.
    @param data Data to associate with the event and stored in event->data.
    @param flags Not used.
    @ingroup MprEvent
 */
extern MprEvent *mprCreateTimerEvent(MprDispatcher *dispatcher, cchar *name, MprTime period, void *proc, void *data, 
    int flags);

/**
    Reschedule an event
    @description Reschedule a continuous event by modifying its period.
    @param event Event object returned from #mprCreateEvent
    @param period Time in milliseconds used by continuous events between firing of the event.
    @ingroup MprEvent
 */
extern void mprRescheduleEvent(MprEvent *event, MprTime period);

/**
    Relay an event to a dispatcher. This invokes the callback proc as though it was invoked from the given dispatcher. 
    @param dispatcher Dispatcher object created via #mprCreateDispatcher
    @param proc Procedure to invoke
    @param data Argument to proc
    @param event Event object
    @internal
 */
extern void mprRelayEvent(MprDispatcher *dispatcher, void *proc, void *data, MprEvent *event);

/* Internal API */
extern MprEvent *mprCreateEventQueue();
extern MprDispatcher *mprGetNonBlockDispatcher();
extern void mprWakeDispatchers();
extern int mprDispatchersAreIdle();
extern void mprClaimDispatcher(MprDispatcher *dispatcher);
extern MprEventService *mprCreateEventService();
extern void mprStopEventService();
extern MprEvent *mprGetNextEvent(MprDispatcher *dispatcher);
extern int mprGetEventCount(MprDispatcher *dispatcher);
extern void mprInitEventQ(MprEvent *q);
extern void mprScheduleDispatcher(MprDispatcher *dispatcher);
extern void mprQueueTimerEvent(MprDispatcher *dispatcher, MprEvent *event);
extern void mprDedicateWorkerToDispatcher(MprDispatcher *dispatcher, struct MprWorker *worker);
extern void mprReleaseWorkerFromDispatcher(MprDispatcher *dispatcher, struct MprWorker *worker);
extern bool mprDispatcherHasEvents(MprDispatcher *dispatcher);
extern void mprWakePendingDispatchers();

/*********************************** XML **************************************/
/*
    XML parser states. The states that are passed to the user handler have "U" appended to the comment.
    The error states (ERR and EOF) must be negative.
 */
#define MPR_XML_ERR                 -1      /**< Error */
#define MPR_XML_EOF                 -2      /**< End of input */
#define MPR_XML_BEGIN               1       /**< Before next tag               */
#define MPR_XML_AFTER_LS            2       /**< Seen "<"                      */
#define MPR_XML_COMMENT             3       /**< Seen "<!--" (usr)        U    */
#define MPR_XML_NEW_ELT             4       /**< Seen "<tag" (usr)        U    */
#define MPR_XML_ATT_NAME            5       /**< Seen "<tag att"               */
#define MPR_XML_ATT_EQ              6       /**< Seen "<tag att" =             */
#define MPR_XML_NEW_ATT             7       /**< Seen "<tag att = "val"   U    */
#define MPR_XML_SOLO_ELT_DEFINED    8       /**< Seen "<tag../>"          U    */
#define MPR_XML_ELT_DEFINED         9       /**< Seen "<tag...>"          U    */
#define MPR_XML_ELT_DATA            10      /**< Seen "<tag>....<"        U    */
#define MPR_XML_END_ELT             11      /**< Seen "<tag>....</tag>"   U    */
#define MPR_XML_PI                  12      /**< Seen "<?processingInst"  U    */
#define MPR_XML_CDATA               13      /**< Seen "<![CDATA["         U    */

/*
    Lex tokens
 */
typedef enum MprXmlToken {
    MPR_XMLTOK_ERR,
    MPR_XMLTOK_TOO_BIG,                     /* Token is too big */
    MPR_XMLTOK_CDATA,
    MPR_XMLTOK_COMMENT,
    MPR_XMLTOK_INSTRUCTIONS,
    MPR_XMLTOK_LS,                          /* "<" -- Opening a tag */
    MPR_XMLTOK_LS_SLASH,                    /* "</" -- Closing a tag */
    MPR_XMLTOK_GR,                          /* ">" -- End of an open tag */
    MPR_XMLTOK_SLASH_GR,                    /* "/>" -- End of a solo tag */
    MPR_XMLTOK_TEXT,
    MPR_XMLTOK_EQ,
    MPR_XMLTOK_EOF,
    MPR_XMLTOK_SPACE
} MprXmlToken;

/**
    XML callback handler
    @param xp XML instance reference
    @param state XML state
    @param tagName Current XML tag
    @param attName Current XML attribute
    @param value Current XML element value
    @ingroup MprXml
  */
typedef int (*MprXmlHandler)(struct MprXml *xp, int state, cchar *tagName, cchar* attName, cchar* value);

/**
    XML input stream function
    @param xp XML instance reference
    @param arg to input stream
    @param buf Buffer into which to read data
    @param size Size of buf
 */
typedef ssize (*MprXmlInputStream)(struct MprXml *xp, void *arg, char *buf, ssize size);

/**
    Per XML session structure
    @defgroup MprXml MprXml
    @see MprXml MprXmlHandler MprXmlInputStream mprXmlGetErrorMsg mprXmlGetLineNumber mprXmlGetParseArg mprXmlOpen 
        mprXmlParse mprXmlSetInputStraem mprXmlSetParseArg mprXmlSetParseHandler 
 */
typedef struct MprXml {
    MprXmlHandler       handler;            /**< Callback function */
    MprXmlInputStream   readFn;             /**< Read data function */
    MprBuf              *inBuf;             /**< Input data queue */
    MprBuf              *tokBuf;            /**< Parsed token buffer */
    int                 quoteChar;          /**< XdbAtt quote char */
    int                 lineNumber;         /**< Current line no for debug */
    void                *parseArg;          /**< Arg passed to mprXmlParse() */
    void                *inputArg;          /**< Arg for mprXmlSetInputStream() */
    char                *errMsg;            /**< Error message text */
} MprXml;

/**
    Get the XML error message if mprXmlParse fails
    @param xp XML parser instance returned from mprXmlOpen
    @return A descriptive null-terminated string
    @ingroup MprXml
 */
extern cchar *mprXmlGetErrorMsg(MprXml *xp);

/**
    Get the source XML line number. 
    @description This call can be used from within the parser callback or when mprXmlParse fails.
    @param xp XML parser instance returned from mprXmlOpen
    @return The line number for the current token or error.
    @ingroup MprXml
 */
extern int mprXmlGetLineNumber(MprXml *xp);

/**
    Get the XML callback argument
    @param xp XML parser instance returned from mprXmlOpen
    @return Argument defined to use for the callback
    @ingroup MprXml
 */
extern void *mprXmlGetParseArg(MprXml *xp);

/**
    Open an XML parser instance.
    @param initialSize Initialize size of XML in-memory token buffer
    @param maxSize Maximum size of XML in-memory token buffer. Set to -1 unlimited.
    @return An XML parser instance
    @ingroup MprXml
 */
extern MprXml *mprXmlOpen(ssize initialSize, ssize maxSize);

/**
    Run the XML parser
    @param xp XML parser instance returned from mprXmlOpen
    @return Zero if successful. Otherwise returns a negative MPR error code.
    @ingroup MprXml
 */
extern int mprXmlParse(MprXml *xp);

/**
    Define the XML parser input stream. This 
    @param xp XML parser instance returned from mprXmlOpen
    @param fn Callback function to provide data to the XML parser. The callback is invoked with the signature: 
        ssize callbac(MprXml *xp, void *arg, char *buf, ssize size);
    @param arg Callback argument to pass to the 
    @ingroup MprXml
 */
extern void mprXmlSetInputStream(MprXml *xp, MprXmlInputStream fn, void *arg);

/**
    Set the XML callback argument
    @param xp XML parser instance returned from mprXmlOpen
    @param parseArg Argument to use for the callback
    @ingroup MprXml
 */
extern void mprXmlSetParseArg(MprXml *xp, void *parseArg);

/**
    Set the XML parser data handle
    @param xp XML parser instance returned from mprXmlOpen
    @param h Arbitrary data to associate with the parser
    @ingroup MprXml
 */
extern void mprXmlSetParserHandler(MprXml *xp, MprXmlHandler h);

/******************************** JSON ****************************************/
/*
    Flags for mprSerialize
 */
#define MPR_JSON_PRETTY     0x1         /**< Serialize output in a more human readable, multiline "pretty" format */

/*
    Data types for obj property values (must fit into MprKey.type)
 */
#define MPR_JSON_UNKNOWN     0          /**< The type of a property is unknown */
#define MPR_JSON_STRING      1          /**< The property is a string (char*) */
#define MPR_JSON_OBJ         2          /**< The property is an object (MprHash) */
#define MPR_JSON_ARRAY       3          /**< The property is an array (MprHash with numeric keys) */

struct MprJson;

/**
    Object container for JSON parse trees.
    @internal
 */
typedef void MprObj;

/**
    JSON callbacks
    @ingroup MprJson
 */
typedef struct MprJsonCallback {
    /**
        Check state callback for JSON deserialization. This function is called at the conclusion of object levels when
        a "}" or "]" is encountered in the input stream. It is also invoked after each "name:" is parsed.
     */
    int (*checkState)(struct MprJson *jp, cchar *name);

    /**
        MakeObject callback for JSON deserialization. This function is called to construct an object for each level 
        in the object tree. Objects will be either arrays or objects.
     */
    MprObj *(*makeObj)(struct MprJson *jp, bool list);

    /**
        Handle a parse error. This function is called from mprJsonParseError to handle error reporting.
     */
    void (*parseError)(struct MprJson *jp, cchar *msg);

    /**
        SetValue callback for JSON deserialization. This function is called to a property value in an object.
     */
    int (*setValue)(struct MprJson *jp, MprObj *obj, int index, cchar *name, cchar *value, int valueType);
} MprJsonCallback;


/**
    JSON parser
    @see MprObj MprCheckState MprSetValue MprMakeObj mprSerialize mprDeserialize mprJsonParseError
    @defgroup MprJson MprJson
 */
typedef struct MprJson {
    cchar           *path;          /* Optional JSON filename */
    cchar           *tok;           /* Current parse token */
    int             lineNumber;     /* Current line number in path */
    MprJsonCallback callback;       /* JSON callbacks */
    int             state;          /* Custom extended state */
    void            *data;          /* Custom data handle */
} MprJson;

/**
    Serialize a JSON object tree into a string
    @description Serializes a top level JSON object created via mprDeserialize into a characters string in JSON format.
    @param obj Object returned via #mprDeserialize
    @param flags Serialization flags. Supported flags include MPR_JSON_PRETTY.
    @return Returns a serialized JSON character string.
    @ingroup MprJson
 */
extern cchar *mprSerialize(MprObj *obj, int flags);

/**
    Custom deserialization from a JSON string into an object tree.
    @description Serializes a top level JSON object created via mprDeserialize into a characters string in JSON format.
        This extended deserialization API takes callback functions to control how the object tree is constructed. 
    @param str JSON string to deserialize.
    @param callback Callback functions. This is an instance of the $MprJsonCallback structure.
    @param data Opaque object to pass to the given callbacks
    @return Returns a serialized JSON character string.
    @ingroup MprJson
    @internal
 */
extern MprObj *mprDeserializeCustom(cchar *str, MprJsonCallback callback, void *data);

/**
    Deserialize a JSON string into an object tree.
    @description Serializes a top level JSON object created via mprDeserialize into a characters string in JSON format.
    @param str JSON string to deserialize.
    @return Returns a tree of objects. Each object represents a level in the JSON input stream. Each object is a 
        hash table (MprHash). The hash table key entry will store the property type in the MprKey.type field. This will
        be set to MPR_JSON_STRING, MPR_JSON_OBJ or MPR_JSON_ARRAY.
    @ingroup MprJson
 */
extern MprObj *mprDeserialize(cchar *str);

/**
    Signal a parse error in the JSON input stream.
    @description JSON callback functions will invoke mprJsonParseError when JSON parse or data semantic errors are 
        encountered.
    @param jp JSON control structure
    @param fmt Printf style format string
    @ingroup MprJson
 */
extern void mprJsonParseError(MprJson *jp, cchar *fmt, ...);

/********************************* Threads ************************************/
/**
    Thread service
    @ingroup MprThread
 */
typedef struct MprThreadService {
    MprList         *threads;           /**< List of all threads */
    struct MprThread *mainThread;       /**< Main application Mpr thread id */
    MprCond         *cond;              /**< Multi-thread sync */
    ssize           stackSize;          /**< Default thread stack size */
} MprThreadService;

/**
    Thread main procedure
    @param arg Argument to the thread main
    @param tp Thread instance reference
    @ingroup MprThread
 */
typedef void (*MprThreadProc)(void *arg, struct MprThread *tp);

/*
    Internal
 */
extern MprThreadService *mprCreateThreadService();
extern void mprStopThreadService();

/**
    Thread Service. 
    @description The MPR provides a cross-platform thread abstraction above O/S native threads. It supports 
        arbitrary thread creation, thread priorities, thread management and thread local storage. By using these
        thread primitives with the locking and synchronization primitives offered by #MprMutex, #MprSpin and 
        #MprCond - you can create cross platform multi-threaded applications.
    @stability Evolving
    @see MprThread MprThreadProc MprThreadService mprCreateThread mprGetCurrentOsThread mprGetCurrentThread 
        mprGetCurrentThreadName mprGetThreadName mprGetThreadPriority mprResetYield mprSetCurrentThreadPriority 
        mprSetThreadPriority mprStartThread mprYield 
    @defgroup MprThread MprThread
 */
typedef struct MprThread {
    MprOsThread     osThread;           /**< O/S thread id */
#if BIT_WIN_LIKE
    handle          threadHandle;       /**< Threads OS handle for WIN */
#endif
    MprThreadProc   entry;              /**< Users thread entry point */
    MprMutex        *mutex;             /**< Multi-thread locking */
    MprCond         *cond;              /**< Multi-thread synchronization */
    void            *data;              /**< Data argument */
    char            *name;              /**< Name of thead for trace */
    ulong           pid;                /**< Owning process id */
    int             isMain;             /**< Is the main thread */
    int             priority;           /**< Current priority */
    ssize           stackSize;          /**< Only VxWorks implements */
#if BIT_MEMORY_STACK
    void            *stackBase;         /**< Base of stack (approx) */
    int             peakStack;          /**< Peak stack usage */
#endif
    int             stickyYield;        /**< Yielded does not auto-clear after GC */
    int             yielded;            /**< Thread has yielded to GC */
} MprThread;


/**
    Thread local data storage
    @internal
 */
typedef struct MprThreadLocal {
#if BIT_UNIX_LIKE
    pthread_key_t   key;                /**< Data key */
#elif BIT_WIN_LIKE
    DWORD           key;
#else
    MprHash         *store;
#endif
} MprThreadLocal;


/**
    Create a new thread
    @description MPR threads are usually real O/S threads and can be used with the various locking services (#MprMutex,
        #MprCond, #MprSpin) to enable scalable multithreaded applications.
    @param name Unique name to give the thread
    @param proc Entry point function for the thread. #mprStartThread will invoke this function to start the thread
    @param data Thread private data stored in MprThread.data
    @param stackSize Stack size to use for the thread. On VM based systems, increasing this value, does not 
        necessarily incurr a real memory (working-set) increase. Set to zero for a default stack size.
    @returns A MprThread object
    @ingroup MprThread
 */
extern MprThread *mprCreateThread(cchar *name, void *proc, void *data, ssize stackSize);

/**
    Get the O/S thread
    @description Get the O/S thread ID for the currently executing thread.
    @return Returns a platform specific O/S thread ID. On Unix, this is a pthread reference. On other systems it is
        a thread integer value.
    @ingroup MprThread
 */
extern MprOsThread mprGetCurrentOsThread();

/**
    Get the currently executing thread.
    @description Get the thread object for the currently executing O/S thread.
    @return Returns a thread object representing the current O/S thread.
    @ingroup MprThread
 */
extern MprThread *mprGetCurrentThread();

/**
    Return the name of the current thread
    @returns a static thread name.
 */
extern cchar *mprGetCurrentThreadName();

/**
    Get the thread name.
    @description MPR threads are usually real O/S threads and can be used with the various locking services (#MprMutex,
        #MprCond, #MprSpin) to enable scalable multithreaded applications.
    @param thread Thread object returned from #mprCreateThread
    @return Returns a string name for the thread.
    @ingroup MprThread
 */
extern cchar *mprGetThreadName(MprThread *thread);

/**
    Get the thread priroity
    @description Get the current priority for the specified thread.
    @param thread Thread object returned by #mprCreateThread
    @returns An integer MPR thread priority between 0 and 100 inclusive.
    @ingroup MprThread
 */
extern int mprGetThreadPriority(MprThread *thread);

/**
    Set the thread priroity for the current thread.
    @description Set the current priority for the specified thread.
    @param priority Priority to associate with the thread. Mpr thread priorities are are integer values between 0 
        and 100 inclusive with 50 being a normal priority. The MPR maps these priorities in a linear fashion onto 
        native O/S priorites. Useful constants are: 
        @li MPR_LOW_PRIORITY
        @li MPR_NORMAL_PRIORITY
        @li MPR_HIGH_PRIORITY
    @ingroup MprThread
 */
extern void mprSetCurrentThreadPriority(int priority);

/**
    Set the thread priroity
    @description Set the current priority for the specified thread.
    @param thread Thread object returned by #mprCreateThread
    @param priority Priority to associate with the thread. Mpr thread priorities are are integer values between 0 
        and 100 inclusive with 50 being a normal priority. The MPR maps these priorities in a linear fashion onto 
        native O/S priorites. Useful constants are: 
        @li MPR_LOW_PRIORITY
        @li MPR_NORMAL_PRIORITY
        @li MPR_HIGH_PRIORITY
    @ingroup MprThread
 */
extern void mprSetThreadPriority(MprThread *thread, int priority);

/**
    Start a thread
    @description Start a thread previously created via #mprCreateThread. The thread will begin at the entry function 
        defined in #mprCreateThread.
    @param thread Thread object returned from #mprCreateThread
    @return Returns zero if successful, otherwise a negative MPR error code.
    @ingroup MprThread
 */
extern int mprStartThread(MprThread *thread);

#define MPR_YIELD_BLOCK     0x1
#define MPR_YIELD_STICKY    0x2

/**
    Yield a thread to allow garbage collection
    @description Long running threads should regularly call mprYield to allow the garbage collector to run.
    All transient memory must have references from "managed" objects (see mprAlloc) to ensure required memory is
    retained.
    @param flags Set to MPR_YIELD_BLOCK to wait until the garbage collection is complete. This is not normally required.
    Set to MPR_YIELD_STICKY to remain in the yielded state. This is useful when sleeping or blocking waiting for I/O.
    #mprResetYield must be called after setting a sticky yield.
 */
extern void mprYield(int flags);

/**
    Reset a sticky yield
    @description This call resets a sticky yield established with #mprYield.
 */
extern void mprResetYield();

/*
    Internal APIs
 */
extern int mprMapMprPriorityToOs(int mprPriority);
extern int mprMapOsPriorityToMpr(int nativePriority);
extern void mprSetThreadStackSize(ssize size);
extern int mprSetThreadData(MprThreadLocal *tls, void *value);
extern void *mprGetThreadData(MprThreadLocal *tls);
extern MprThreadLocal *mprCreateThreadLocal();

/******************************** I/O Wait ************************************/

#define MPR_READABLE           0x2          /**< Read event mask */
#define MPR_WRITABLE           0x4          /**< Write event mask */

#define MPR_READ_PIPE          0            /* Read side of breakPipe */
#define MPR_WRITE_PIPE         1            /* Write side of breakPipe */

#if BIT_WIN_LIKE
typedef long (*MprMsgCallback)(HWND hwnd, UINT msg, UINT wp, LPARAM lp);
#endif

/**
    Wait Service
    @ingroup MprWaitHandler
 */
typedef struct MprWaitService {
    MprList         *handlers;              /* List of handlers */
    int             needRecall;             /* A handler needs a recall due to buffered data */
    int             wakeRequested;          /* Wakeup of the wait service has been requested */
#if MPR_EVENT_EPOLL
    int             epoll;                  /* Kqueue() return descriptor */
    struct epoll_event *events;             /* Events triggered */
    int             eventsMax;              /* Max size of events/interest */
    struct MprWaitHandler **handlerMap;     /* Map of fds to handlers */
    int             handlerMax;             /* Size of the handlers array */
    int             breakPipe[2];           /* Pipe to wakeup select */
#elif MPR_EVENT_KQUEUE
    int             kq;                     /* Kqueue() return descriptor */
    struct kevent   *interest;              /* Events of interest */
    int             interestMax;            /* Size of the interest array */
    int             interestCount;          /* Last used entry in the interest array */
    struct kevent   *stableInterest;        /* Stable copy of interest during kevent() */
    int             stableInterestCount;    /* Last used entry in the stableInterest array */
    struct kevent   *events;                /* Events triggered */
    int             eventsMax;              /* Max size of events/interest */
    struct MprWaitHandler **handlerMap;     /* Map of fds to handlers */
    int             handlerMax;             /* Size of the handlers array */
    int             breakPipe[2];           /* Pipe to wakeup select */
#elif MPR_EVENT_POLL
    struct MprWaitHandler **handlerMap;     /* Map of fds to handlers (indexed by fd) */
    int             handlerMax;             /* Size of the handlers array */
    struct pollfd   *fds;                   /* Master set of file descriptors to poll */
    struct pollfd   *pollFds;               /* Set of descriptors used in poll() */
    int             fdsCount;               /* Last used entry in the fds array */
    int             fdMax;                  /* Size of the fds array */
    int             breakPipe[2];           /* Pipe to wakeup select */
#elif MPR_EVENT_ASYNC
    struct MprWaitHandler **handlerMap;     /* Map of fds to handlers */
    int             handlerMax;             /* Size of the handlers array */
    int             nfd;                    /* Last used entry in the handlerMap array */
    int             fdmax;                  /* Size of the fds array */
    HWND            hwnd;                   /* Window handle */
    int             socketMessage;          /* Message id for socket events */
    MprMsgCallback  msgCallback;            /* Message handler callback */
#elif MPR_EVENT_SELECT
    struct MprWaitHandler **handlerMap;     /* Map of fds to handlers */
    int             handlerMax;             /* Size of the handlers array */
    fd_set          readMask;               /* Current read events mask */
    fd_set          writeMask;              /* Current write events mask */
    fd_set          stableReadMask;         /* Read events mask used for select() */
    fd_set          stableWriteMask;        /* Write events mask used for select() */
    int             highestFd;              /* Highest socket in masks + 1 */
    int             breakSock;              /* Socket to wakeup select */
    struct sockaddr_in breakAddress;        /* Address of wakeup socket */
#endif /* EVENT_SELECT */
    MprMutex        *mutex;                 /* General multi-thread sync */
    MprSpin         *spin;                  /* Fast short locking */
} MprWaitService;


/*
    Internal
 */
extern MprWaitService *mprCreateWaitService();
extern void mprTermOsWait(MprWaitService *ws);
extern int  mprStartWaitService(MprWaitService *ws);
extern int  mprStopWaitService(MprWaitService *ws);
extern void mprSetWaitServiceThread(MprWaitService *ws, MprThread *thread);
extern void mprWakeNotifier();
extern int  mprInitWindow();
#if MPR_EVENT_KQUEUE
    extern void mprManageKqueue(MprWaitService *ws, int flags);
#endif
#if MPR_EVENT_EPOLL
    extern void mprManageEpoll(MprWaitService *ws, int flags);
#endif
#if MPR_EVENT_POLL
    extern void mprManagePoll(MprWaitService *ws, int flags);
#endif
#if MPR_EVENT_SELECT
    extern void mprManageSelect(MprWaitService *ws, int flags);
#endif
#if BIT_WIN_LIKE
    extern void mprSetWinMsgCallback(MprMsgCallback callback);
    extern void mprServiceWinIO(MprWaitService *ws, int sockFd, int winMask);
#endif

/**
    Wait for I/O. This call waits for any I/O events on wait handlers until the given timeout expires.
    @param ws Wait service object
    @param timeout Timeout in milliseconds to wait for an event.
    @ingroup MprWaitHandler
 */
extern void mprWaitForIO(MprWaitService *ws, MprTime timeout);

/**
    Wait for I/O on a file descriptor. No processing of the I/O event is done.
    @param fd File descriptor to examine
    @param mask Mask of events of interest (MPR_READABLE | MPR_WRITABLE)
    @param timeout Timeout in milliseconds to wait for an event.
    @returns A count of events received.
    @ingroup MprWaitHandler
 */
extern int mprWaitForSingleIO(int fd, int mask, MprTime timeout);

/*
    Handler Flags
 */
#define MPR_WAIT_RECALL_HANDLER     0x1     /**< Wait handler flag to recall the handler asap */
#define MPR_WAIT_NEW_DISPATCHER     0x2     /**< Wait handler flag to create a new dispatcher for each I/O event */

/**
    Wait Handler Service
    @description Wait handlers provide callbacks for when I/O events occur. They provide a wait to service many
        I/O file descriptors without requiring a thread per descriptor.
    @see MprEvent MprWaitHandler mprCreateWaitHandler mprQueueIOEvent mprRecallWaitHandler mprRecallWaitHandlerByFd 
        mprRemoveWaitHandler mprUpdateWaitHandler mprWaitOn 
    @defgroup MprWaitHandler MprWaitHandler
 */
typedef struct MprWaitHandler {
    int             desiredMask;        /**< Mask of desired events */
    int             presentMask;        /**< Mask of current events */
    int             fd;                 /**< O/S File descriptor (sp->sock) */
    int             notifierIndex;      /**< Index for notifier */
    int             flags;              /**< Control flags */
    void            *handlerData;       /**< Argument to pass to proc */
    MprEvent        *event;             /**< Event object to process I/O events */
    MprWaitService  *service;           /**< Wait service pointer */
    MprDispatcher   *dispatcher;        /**< Event dispatcher to use for I/O events */
    MprEventProc    proc;               /**< Callback event procedure */
    struct MprWaitHandler *next;        /**< List linkage */
    struct MprWaitHandler *prev;
    struct MprWorker *requiredWorker;   /**< Designate the required worker thread to run the callback */
    struct MprThread *thread;           /**< Thread executing the callback, set even if worker is null */
    MprCond         *callbackComplete;  /**< Signalled when a callback is complete */
} MprWaitHandler;


/**
    Create a wait handler
    @description Create a wait handler that will be invoked when I/O of interest occurs on the specified file handle
        The wait handler is registered with the MPR event I/O mechanism.
    @param fd File descriptor
    @param mask Mask of events of interest. This is made by oring MPR_READABLE and MPR_WRITABLE
    @param dispatcher Dispatcher object to use for scheduling the I/O event.
    @param proc Callback function to invoke when an I/O event of interest has occurred.
    @param data Data item to pass to the callback
    @param flags Wait handler flags. Use MPR_WAIT_NEW_DISPATCHER to auto-create a new dispatcher for each I/O event.
    @returns A new wait handler registered with the MPR event mechanism
    @ingroup MprWaitHandler
 */
extern MprWaitHandler *mprCreateWaitHandler(int fd, int mask, MprDispatcher *dispatcher, void *proc, void *data, int flags);

/**
    Queue an IO event for dispatch on the wait handler dispatcher
    @param wp Wait handler created via #mprCreateWaitHandler
 */
extern void mprQueueIOEvent(MprWaitHandler *wp);

/**
    Recall a wait handler
    @description Signal that a wait handler should be recalled at the earliest opportunity. This is useful
        when a protocol stack has buffered data that must be processed regardless of whether more I/O occurs. 
    @param wp Wait handler to recall
    @ingroup MprWaitHandler
 */
extern void mprRecallWaitHandler(MprWaitHandler *wp);

/**
    Recall a wait handler by fd
    @description Signal that a wait handler should be recalled at the earliest opportunity. This is useful
        when a protocol stack has buffered data that must be processed regardless of whether more I/O occurs. 
    @param fd File descriptor that matches that of a wait handler to recall
    @ingroup MprWaitHandler
 */
extern void mprRecallWaitHandlerByFd(int fd);

/**
    Disconnect a wait handler from its underlying file descriptor. This is used to prevent further I/O wait events while
    still preserving the wait handler.
    @param wp Wait handler object
 */
extern void mprRemoveWaitHandler(MprWaitHandler *wp);

/**
    Apply wait handler updates. While a wait handler is in use, wait event updates are buffered. This routine applies
        such buffered updates.
    @param wp Wait handler created via #mprCreateWaitHandler
    @param wakeup Wake up the service events thread. Typically it is safest to wake up the service events thread if the
        wait handler event masks have been modified. However, there are some cases where it can be useful to suppress
        this behavior.
 */
extern void mprUpdateWaitHandler(MprWaitHandler *wp, bool wakeup);

/**
    Subscribe for desired wait events
    @description Subscribe to the desired wait events for a given wait handler.
    @param wp Wait handler created via #mprCreateWaitHandler
    @param desiredMask Mask of desired events (MPR_READABLE | MPR_WRITABLE)
    @ingroup MprWaitHandler
 */
extern void mprWaitOn(MprWaitHandler *wp, int desiredMask);

/*
   Internal
 */
extern void mprDoWaitRecall(MprWaitService *ws);

/******************************* Notification *********************************/
/*
    Internal
 */
extern int mprCreateNotifierService(MprWaitService *ws);

/*
    Begin I/O notification services on a wait handler
    @param wp Wait handler associated with the file descriptor
    @param mask Mask of events of interest. This is made by oring MPR_READABLE and MPR_WRITABLE
    @return Zero if successful, otherwise a negative MPR error code.
 */
extern int mprNotifyOn(MprWaitService *ws, MprWaitHandler *wp, int mask);

/********************************** Sockets ***********************************/
/**
    Socket I/O callback procedure. Proc returns non-zero if the socket has been deleted.
    @ingroup MprSocket
 */
typedef int (*MprSocketProc)(void *data, int mask);

/**
    Socket service provider interface.
    @ingroup MprSocket
 */
typedef struct MprSocketProvider {
    void    *data;
    void    (*closeSocket)(struct MprSocket *socket, bool gracefully);
    void    (*disconnectSocket)(struct MprSocket *socket);
    ssize   (*flushSocket)(struct MprSocket *socket);
    int     (*listenSocket)(struct MprSocket *socket, cchar *host, int port, int flags);
    ssize   (*readSocket)(struct MprSocket *socket, void *buf, ssize len);
    ssize   (*writeSocket)(struct MprSocket *socket, cvoid *buf, ssize len);
    int     (*upgradeSocket)(struct MprSocket *socket, struct MprSsl *ssl, int server);
} MprSocketProvider;

/**
    Callback before binding a socket
    @ingroup MprSocket
*/
typedef int (*MprSocketPrebind)(struct MprSocket *sock);


/**
    Mpr socket service class
    @ingroup MprSocket
 */
typedef struct MprSocketService {
    int             maxAccept;                  /**< Maximum number of accepted client socket connections */
    int             numAccept;                  /**< Count of client socket connections */
    MprSocketProvider *standardProvider;        /**< Socket provider for non-SSL connections */
    char            *defaultProvider;           /**< Default secure provider for SSL connections */
    MprHash         *providers;                 /**< Secure socket providers */         
    MprSocketPrebind prebind;                   /**< Prebind callback */
    MprList         *secureSockets;             /**< List of secured (matrixssl) sockets */
    MprMutex        *mutex;                     /**< Multithread locking */
} MprSocketService;


/*
    Internal
 */
extern MprSocketService *mprCreateSocketService();

/**
    Determine if SSL is available
    @returns True if SSL is available
    @ingroup MprSocket
 */
extern bool mprHasSecureSockets();

/**
    Set the maximum number of accepted client connections that are permissable
    @param max New maximum number of accepted client connections.
    @ingroup MprSocket
 */
extern int mprSetMaxSocketAccept(int max);

/**
    Add a secure socket provider for SSL communications
    @param name Name of the secure socket provider
    @param provider Socket provider object
    @ingroup MprSocket
 */
extern void mprAddSocketProvider(cchar *name, MprSocketProvider *provider);

/*
    Socket close flags
 */
#define MPR_SOCKET_GRACEFUL     1           /**< Do a graceful shutdown */

/*
    Socket event types
 */
#define MPR_SOCKET_READABLE     MPR_READABLE
#define MPR_SOCKET_WRITABLE     MPR_WRITABLE

/*
    Socket Flags
 */
#define MPR_SOCKET_BLOCK        0x1         /**< Use blocking I/O */
#define MPR_SOCKET_BROADCAST    0x2         /**< Broadcast mode */
#define MPR_SOCKET_CLOSED       0x4         /**< MprSocket has been closed */
#define MPR_SOCKET_CONNECTING   0x8         /**< MprSocket is connecting */
#define MPR_SOCKET_DATAGRAM     0x10        /**< Use datagrams */
#define MPR_SOCKET_EOF          0x20        /**< Seen end of file */
#define MPR_SOCKET_LISTENER     0x40        /**< MprSocket is server listener */
#define MPR_SOCKET_NOREUSE      0x80        /**< Don't set SO_REUSEADDR option */
#define MPR_SOCKET_NODELAY      0x100       /**< Disable Nagle algorithm */
#define MPR_SOCKET_THREAD       0x400       /**< Process callbacks on a worker thread */
#define MPR_SOCKET_CLIENT       0x800       /**< Socket is a client */
#define MPR_SOCKET_PENDING      0x1000      /**< Pending buffered read data */
#define MPR_SOCKET_TRACED       0x2000      /**< Socket has been traced to the log */

/**
    Socket Service
    @description The MPR Socket service provides IPv4 and IPv6 capabilities for both client and server endpoints.
    Datagrams, Broadcast and point to point services are supported. The APIs can be used in both blocking and
    non-blocking modes.
    \n\n
    The socket service integrates with the MPR worker thread pool and eventing services. Socket connections can be handled
    by threads from the worker thread pool for scalable, multithreaded applications.
    @stability Evolving
    @see MprSocket MprSocketPrebind MprSocketProc MprSocketProvider MprSocketService mprAddSocketHandler 
        mprCloseSocket mprConnectSocket mprCreateSocket mprCreateSocketService mprCreateSsl mprCloneSsl
        mprDisconnectSocket mprEnableSocketEvents mprFlushSocket mprGetSocketBlockingMode mprGetSocketError 
        mprGetSocketFd mprGetSocketInfo mprGetSocketPort mprHasSecureSockets mprIsSocketEof mprIsSocketSecure 
        mprListenOnSocket mprLoadSsl mprParseIp mprReadSocket mprSendFileToSocket mprSetSecureProvider 
        mprSetSocketBlockingMode mprSetSocketCallback mprSetSocketEof mprSetSocketNoDelay mprSetSslCaFile 
        mprSetSslCaPath mprSetSslCertFile mprSetSslCiphers mprSetSslKeyFile mprSetSslSslProtocols 
        mprSetSslVerifySslClients mprWriteSocket mprWriteSocketString mprWriteSocketVector 
        mprSocketHasPendingData mprUpgradeSocket
    @defgroup MprSocket MprSocket
 */
typedef struct MprSocket {
    MprSocketService *service;          /**< Socket service */
    int             error;              /**< Last error */
    MprDispatcher   *dispatcher;        /**< Event dispatcher for I/O events */
    MprWaitHandler  *handler;           /**< Wait handler */
    char            *acceptIp;          /**< Server addresss that accepted a new connection (actual interface) */
    char            *ip;                /**< Server listen address or remote client address */
    char            *errorMsg;          /**< Connection related error messages */
    int             acceptPort;         /**< Server port doing the listening */
    int             port;               /**< Port to listen or connect on */
    int             fd;                 /**< Actual socket file handle */
    int             flags;              /**< Current state flags */
    MprSocketProvider *provider;        /**< Socket implementation provider */
    struct MprSocket *listenSock;       /**< Listening socket */
    void            *sslSocket;         /**< Extended SSL socket state */
    struct MprSsl   *ssl;               /**< SSL configuration */
    MprMutex        *mutex;             /**< Multi-thread sync */
} MprSocket;


/**
    Vectored write array
 */
typedef struct MprIOVec {
    char            *start;             /**< Start of block to write */
    ssize           len;                /**< Length of block to write */
} MprIOVec;


/**
    Accept an incoming connection
    @param listen Listening server socket
    @returns A new socket connection
    @ingroup MprSocket
 */
MprSocket *mprAcceptSocket(MprSocket *listen);

/**
    Add a wait handler to a socket.
    @description Create a wait handler that will be invoked when I/O of interest occurs on the specified socket.
        The wait handler is registered with the MPR event I/O mechanism.
    @param sp Socket object created via mprCreateSocket
    @param mask Mask of events of interest. This is made by oring MPR_READABLE and MPR_WRITABLE
    @param dispatcher Dispatcher object to use for scheduling the I/O event.
    @param proc Callback function to invoke when an I/O event of interest has occurred.
    @param data Data item to pass to the callback
    @param flags Socket handler flags
    @returns A new wait handler registered with the MPR event mechanism
    @ingroup MprSocket
 */
extern MprWaitHandler *mprAddSocketHandler(MprSocket *sp, int mask, MprDispatcher *dispatcher, void *proc, 
        void *data, int flags);

/**
    Close a socket
    @description Close a socket. If the \a graceful option is true, the socket will first wait for written data to drain
        before doing a graceful close.
    @param sp Socket object returned from #mprCreateSocket
    @param graceful Set to true to do a graceful close. Otherwise, an abortive close will be performed.
    @ingroup MprSocket
 */
extern void mprCloseSocket(MprSocket *sp, bool graceful);

/**
    Connect a client socket
    @description Open a client connection
    @param sp Socket object returned via #mprCreateSocket
    @param hostName Host or IP address to connect to.
    @param port TCP/IP port number to connect to.
    @param flags Socket flags may use the following flags ored together:
        @li MPR_SOCKET_BLOCK - to use blocking I/O. The default is non-blocking.
        @li MPR_SOCKET_BROADCAST - Use IPv4 broadcast
        @li MPR_SOCKET_DATAGRAM - Use IPv4 datagrams
        @li MPR_SOCKET_NOREUSE - Set NOREUSE flag on the socket
        @li MPR_SOCKET_NODELAY - Set NODELAY on the socket
        @li MPR_SOCKET_THREAD - Process callbacks on a separate thread.
    @return Zero if the connection is successful. Otherwise a negative MPR error code.
    @ingroup MprSocket
 */
extern int mprConnectSocket(MprSocket *sp, cchar *hostName, int port, int flags);

/**
    Create a socket
    @description Create a new socket
    @return A new socket object
    @ingroup MprSocket
 */
extern MprSocket *mprCreateSocket();

/**
    Disconnect a socket by closing its underlying file descriptor. This is used to prevent further I/O wait events while
    still preserving the socket object.
    @param sp Socket object
    @ingroup MprSocket
 */
extern void mprDisconnectSocket(MprSocket *sp);

/**
    Enable socket events for a socket callback
    @param sp Socket object returned from #mprCreateSocket
    @param mask Mask of events to enable
    @ingroup MprSocket
 */
extern void mprEnableSocketEvents(MprSocket *sp, int mask);

/**
    Flush a socket
    @description Flush any buffered data in a socket. Standard sockets do not use buffering and this call will do nothing.
        SSL sockets do buffer and calling mprFlushSocket will write pending written data.
    @param sp Socket object returned from #mprCreateSocket
    @return A count of bytes actually written. Return a negative MPR error code on errors.
    @ingroup MprSocket
 */
extern ssize mprFlushSocket(MprSocket *sp);

/**
    Get the socket blocking mode.
    @description Return the current blocking mode setting.
    @param sp Socket object returned from #mprCreateSocket
    @return True if the socket is in blocking mode. Otherwise false.
    @ingroup MprSocket
 */
extern bool mprGetSocketBlockingMode(MprSocket *sp);

/**
    Get a socket error code
    @description This will map a Windows socket error code into a posix error code.
    @param sp Socket object returned from #mprCreateSocket
    @return A posix error code. 
    @ingroup MprSocket
 */
extern int mprGetSocketError(MprSocket *sp);

/**
    Get the socket file descriptor.
    @description Get the file descriptor associated with a socket.
    @param sp Socket object returned from #mprCreateSocket
    @return The integer file descriptor used by the O/S for the socket.
    @ingroup MprSocket
 */
extern int mprGetSocketFd(MprSocket *sp);

/**
    Get the socket for an IP:Port address
    @param ip IP address or hostname 
    @param port Port number 
    @param family Output parameter to contain the Internet protocol family
    @param protocol Output parameter to contain the Internet TCP/IP protocol
    @param addr Output parameter to contain the sockaddr description of the socket address
    @param addrlen Output parameter to hold the length of the sockaddr object 
    @return Zero if the call is successful. Otherwise return a negative MPR error code.
    @ingroup MprSocket
  */
extern int mprGetSocketInfo(cchar *ip, int port, int *family, int *protocol, struct sockaddr **addr, MprSocklen *addrlen);

/**
    Get the port used by a socket
    @description Get the TCP/IP port number used by the socket.
    @param sp Socket object returned from #mprCreateSocket
    @return The integer TCP/IP port number used by the socket.
    @ingroup MprSocket
 */
extern int mprGetSocketPort(MprSocket *sp);

/**
    Determine if the IP address is an IPv6 address
    @param ip IP address
    @return True if the address is an IPv6 address, otherwise zero.
    @internal
 */
extern bool mprIsIPv6(cchar *ip);

/**
    Determine if the socket is secure
    @description Determine if the socket is using SSL to provide enhanced security.
    @param sp Socket object returned from #mprCreateSocket
    @return True if the socket is using SSL, otherwise zero.
    @ingroup MprSocket
 */
extern bool mprIsSocketSecure(MprSocket *sp);

/**
    Determine if the socket is using IPv6
    Currently only works for server side addresses.
    @param sp Socket object returned from #mprCreateSocket
    @return True if the socket is using IPv6, otherwise zero.
    @internal
 */
extern bool mprIsSocketV6(MprSocket *sp);

/**
    Test if the other end of the socket has been closed.
    @description Determine if the other end of the socket has been closed and the socket is at end-of-file.
    @param sp Socket object returned from #mprCreateSocket
    @return True if the socket is at end-of-file.
    @ingroup MprSocket
 */
extern bool mprIsSocketEof(MprSocket *sp);

/**
    Listen on a server socket for incoming connections
    @description Open a server socket and listen for client connections.
    @param sp Socket object returned via #mprCreateSocket
    @param ip IP address to bind to. Set to 0.0.0.0 to bind to all possible addresses on a given port.
    @param port TCP/IP port number to connect to. 
    @param flags Socket flags may use the following flags ored together:
        @li MPR_SOCKET_BLOCK - to use blocking I/O. The default is non-blocking.
        @li MPR_SOCKET_BROADCAST - Use IPv4 broadcast
        @li MPR_SOCKET_DATAGRAM - Use IPv4 datagrams
        @li MPR_SOCKET_NOREUSE - Set NOREUSE flag on the socket
        @li MPR_SOCKET_NODELAY - Set NODELAY on the socket
        @li MPR_SOCKET_THREAD - Process callbacks on a separate thread.
    @return Zero if the connection is successful. Otherwise a negative MPR error code.
    @ingroup MprSocket
 */
extern int mprListenOnSocket(MprSocket *sp, cchar *ip, int port, int flags);

/**
    Parse an socket address IP address. 
    @description This parses a string containing an IP:PORT specification and returns the IP address and port 
    components. Handles ipv4 and ipv6 addresses. 
    @param ipSpec An IP:PORT specification. The :PORT is optional. When an IP address contains an ipv6 port it should be 
    written as
        aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh:iiii    or
       [aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh:iiii]:port
    @param ip Pointer to receive a dynamically allocated IP string.
    @param port Pointer to an integer to receive the port value.
    @param defaultPort The default port number to use if the ipSpec does not contain a port
    @ingroup MprSocket
 */
extern int mprParseSocketAddress(cchar *ipSpec, char **ip, int *port, int defaultPort);

/**
    Read from a socket
    @description Read data from a socket. The read will return with whatever bytes are available. If none and the socket
        is in blocking mode, it will block untill there is some data available or the socket is disconnected.
    @param sp Socket object returned from #mprCreateSocket
    @param buf Pointer to a buffer to hold the read data. 
    @param size Size of the buffer.
    @return A count of bytes actually read. Return a negative MPR error code on errors.
    @ingroup MprSocket
 */
extern ssize mprReadSocket(MprSocket *sp, void *buf, ssize size);

/**
    Remove a socket wait handler.
    @description Removes the socket wait handler created via mprAddSocketHandler.
    @param sp Socket object created via mprCreateSocket
    @ingroup MprSocket
 */
extern void mprRemoveSocketHandler(MprSocket *sp);

#if !BIT_ROM
/**
    Send a file to a socket
    @description Write the contents of a file to a socket. If the socket is in non-blocking mode (the default), the write
        may return having written less than the required bytes. This API permits the writing of data before and after
        the file contents. 
    @param file File to write to the socket
    @param sock Socket object returned from #mprCreateSocket
    @param offset offset within the file from which to read data
    @param bytes Length of file data to write
    @param beforeVec Vector of data to write before the file contents
    @param beforeCount Count of entries in beforeVect
    @param afterVec Vector of data to write after the file contents
    @param afterCount Count of entries in afterCount
    @return A count of bytes actually written. Return a negative MPR error code on errors.
    @ingroup MprSocket
 */
extern MprOff mprSendFileToSocket(MprSocket *sock, MprFile *file, MprOff offset, MprOff bytes, MprIOVec *beforeVec, 
    int beforeCount, MprIOVec *afterVec, int afterCount);
#endif

/**
    Set the socket blocking mode.
    @description Set the blocking mode for a socket. By default a socket is in non-blocking mode where read / write
        calls will not block.
    @param sp Socket object returned from #mprCreateSocket
    @param on Set to zero to put the socket into non-blocking mode. Set to non-zero to enable blocking mode.
    @return The old blocking mode if successful or a negative MPR error code.
    @ingroup MprSocket
 */
extern int mprSetSocketBlockingMode(MprSocket *sp, bool on);

/**
    Set an EOF condition on the socket
    @param sp Socket object returned from #mprCreateSocket
    @param eof Set to true to set an EOF condition. Set to false to clear it.
    @ingroup MprSocket
 */
extern void mprSetSocketEof(MprSocket *sp, bool eof);

/**
    Set the socket delay mode.
    @description Set the socket delay behavior (nagle algorithm). By default a socket will partial packet writes
        a little to try to accumulate data and coalesce TCP/IP packages. Setting the delay mode to false may
        result in higher performance for interactive applications.
    @param sp Socket object returned from #mprCreateSocket
    @param on Set to non-zero to put the socket into no delay mode. Set to zero to enable the nagle algorithm.
    @return The old delay mode if successful or a negative MPR error code.
    @ingroup MprSocket
 */
extern int mprSetSocketNoDelay(MprSocket *sp, bool on);

/**
    Test if the socket has buffered read data.
    @description Use this function to avoid waiting for incoming I/O if data is already buffered.
    @param sp Socket object returned from #mprCreateSocket
    @return True if the socket has pending read data.
    @ingroup MprSocket
 */
extern bool mprSocketHasPendingData(MprSocket *sp);

/**
    Upgrade a socket to use SSL/TLS
    @param sp Socket to upgrade
    @param ssl SSL configuration to use. Set to NULL to use the default.
    @param server Set to one for server-side, set to zero for client side.
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup MprSocket
 */
int mprUpgradeSocket(MprSocket *sp, struct MprSsl *ssl, int server);

/**
    Write to a socket
    @description Write a block of data to a socket. If the socket is in non-blocking mode (the default), the write
        may return having written less than the required bytes. 
    @param sp Socket object returned from #mprCreateSocket
    @param buf Reference to a block to write to the socket
    @param len Length of data to write. This may be less than the requested write length if the socket is in non-blocking
        mode. Will return a negative MPR error code on errors.
    @return A count of bytes actually written. Return a negative MPR error code on errors.
    @ingroup MprSocket
 */
extern ssize mprWriteSocket(MprSocket *sp, cvoid *buf, ssize len);

/**
    Write to a string to a socket
    @description Write a string  to a socket. If the socket is in non-blocking mode (the default), the write
        may return having written less than the required bytes. 
    @param sp Socket object returned from #mprCreateSocket
    @param str Null terminated string to write.
    @return A count of bytes actually written. Return a negative MPR error code on errors.
    @ingroup MprSocket
 */
extern ssize mprWriteSocketString(MprSocket *sp, cchar *str);

/**
    Write a vector to a socket
    @description Do scatter/gather I/O by writing a vector of buffers to a socket.
    @param sp Socket object returned from #mprCreateSocket
    @param iovec Vector of data to write before the file contents
    @param count Count of entries in beforeVect
    @return A count of bytes actually written. Return a negative MPR error code on errors.
    @ingroup MprSocket
 */
extern ssize mprWriteSocketVector(MprSocket *sp, MprIOVec *iovec, int count);

/************************************ SSL *************************************/

#define MPR_DEFAULT_SERVER_CERT_FILE    "server.crt"
#define MPR_DEFAULT_SERVER_KEY_FILE     "server.key.pem"
#define MPR_DEFAULT_CLIENT_CERT_FILE    "client.crt"
#define MPR_DEFAULT_CLIENT_CERT_PATH    "certs"

typedef struct MprSsl {
    char            *providerName;      /**< SSL provider to use - null if default */
    struct MprSocketProvider *provider; /**< Cached SSL provider to use */
    char            *key;               /**< Key string */
    char            *cert;              /**< Cert string */
    char            *keyFile;           /**< Alternatively, locate the key in a file */
    char            *certFile;          /**< Alternatively, locate the cert in a file */
    char            *caFile;            /**< Certificate verification cert file or bundle */
    char            *caPath;            /**< Certificate verification cert directory */
    char            *ciphers;           /**< Candidate ciphers to use */
    int             configured;         /**< Set if this SSL configuration has been processed */
    void            *pconfig;           /**< Extended provider SSL configuration */
    int             verifyPeer;         /**< Verify the peer verificate */
    int             verifyIssuer;       /**< Set if the certificate issuer should be also verified */
    int             verifyDepth;        /**< Set if the cert chain depth should be verified */
    int             protocols;          /**< SSL protocols */
} MprSsl;


/*
    SSL protocols
 */
#define MPR_PROTO_SSLV2    0x1              /**< SSL V2 protocol */
#define MPR_PROTO_SSLV3    0x2              /**< SSL V3 protocol */
#define MPR_PROTO_TLSV1    0x4              /**< TLS V1 protocol */
#define MPR_PROTO_TLSV11   0x8              /**< TLS V1.1 protocol */
#define MPR_PROTO_ALL      0xf              /**< All SSL protocols */

/*
    Default SSL configuration
 */
#define MPR_DEFAULT_CIPHER_SUITE "HIGH:MEDIUM"  /**< Default cipher suite */

/**
    Load the SSL module.
    @ingroup MprSocket
 */
extern int mprLoadSsl();

/**
    Create the SSL control structure
    @ingroup MprSocket
 */
extern struct MprSsl *mprCreateSsl();

/**
    Create the a new SSL control structure based on an existing structure
    @param src Structure to clone
    @ingroup MprSocket
 */
extern struct MprSsl *mprCloneSsl(MprSsl *src);

/**
    Set the ciphers to use for SSL
    @param ssl SSL instance returned from #mprCreateSsl
    @param ciphers Cipher string
    @ingroup MprSocket
 */
extern void mprSetSslCiphers(struct MprSsl *ssl, cchar *ciphers);

/**
    Set the key file to use for SSL
    @param ssl SSL instance returned from #mprCreateSsl
    @param keyFile Path to the SSL key file
    @ingroup MprSocket
 */
extern void mprSetSslKeyFile(struct MprSsl *ssl, cchar *keyFile);

/**
    Set certificate to use for SSL
    @param ssl SSL instance returned from #mprCreateSsl
    @param certFile Path to the SSL certificate file
    @ingroup MprSocket
 */
extern void mprSetSslCertFile(struct MprSsl *ssl, cchar *certFile);

/**
    Set the client certificate file to use for SSL
    @param ssl SSL instance returned from #mprCreateSsl
    @param caFile Path to the SSL client certificate file
    @ingroup MprSocket
 */
extern void mprSetSslCaFile(struct MprSsl *ssl, cchar *caFile);

/**
    Set the path for the client certificate directory
    @param ssl SSL instance returned from #mprCreateSsl
    @param caPath Path to the SSL client certificate directory
    @ingroup MprSocket
 */
extern void mprSetSslCaPath(struct MprSsl *ssl, cchar *caPath);

/**
    Set the SSL protocol to use
    @param ssl SSL instance returned from #mprCreateSsl
    @param protocols SSL protocols mask
    @ingroup MprSocket
 */
extern void mprSetSslProtocols(struct MprSsl *ssl, int protocols);

/**
    Set the SSL provider to use
    @param ssl SSL instance returned from #mprCreateSsl
    @param provider SSL provider name (openssl | matrixssl)
    @ingroup MprSocket
 */
extern void mprSetSslProvider(MprSsl *ssl, cchar *provider);

/**
    Require verification of peer certificates
    @param ssl SSL instance returned from #mprCreateSsl
    @param on Set to true to enable peer SSL certificate verification.
    @ingroup MprSocket
 */
extern void mprVerifySslPeer(struct MprSsl *ssl, bool on);

/**
    Control the verification of SSL certificate issuers
    @param ssl SSL instance returned from #mprCreateSsl
    @param on Set to true to enable SSL certificate issuer verification.
    @ingroup MprSocket
 */
extern void mprVerifySslIssuer(struct MprSsl *ssl, bool on);

/**
    Control the depth of SSL SSL certificate verification
    @param ssl SSL instance returned from #mprCreateSsl
    @param depth Set to the number of intermediate certificates to verify. Defaults to 1.
    @ingroup MprSocket
 */
extern void mprVerifySslDepth(struct MprSsl *ssl, int depth);

#if BIT_PACK_MATRIXSSL
    extern int mprCreateMatrixSslModule();
#endif
#if BIT_PACK_OPENSSL
    extern int mprCreateOpenSslModule();
#endif

/******************************* Worker Threads *******************************/
/**
    Worker thread callback signature
    @param data worker callback data. Set via mprStartWorker or mprActivateWorker
    @param worker Reference to the worker thread object
    @ingroup MprWorker
 */
typedef void (*MprWorkerProc)(void *data, struct MprWorker *worker);

/**
    Statistics for Workers
    @ingroup MprWorker
 */
typedef struct MprWorkerStats {
    int             maxThreads;         /**< Configured max number of threads */
    int             minThreads;         /**< Configured minimum */
    int             numThreads;         /**< Configured minimum */
    int             maxUse;             /**< Max used */
    int             idleThreads;        /**< Current idle */
    int             busyThreads;        /**< Current busy */
} MprWorkerStats;

/**
    Worker Thread Service
    @description The MPR provides a worker thread pool for rapid starting and assignment of threads to tasks.
    @stability Evolving
    @ingroup MprWorker
 */
typedef struct MprWorkerService {
    MprList         *busyThreads;       /**< List of threads to service tasks */
    MprList         *idleThreads;       /**< List of threads to service tasks */
    int             maxThreads;         /**< Max # threads in worker pool */
    int             maxUseThreads;      /**< Max threads ever used */
    int             minThreads;         /**< Max # threads in worker pool */
    int             nextThreadNum;      /**< Unique next thread number */
    int             numThreads;         /**< Current number of threads in worker pool */
    ssize           stackSize;          /**< Stack size for worker threads */
    MprMutex        *mutex;             /**< Per task synchronization */
    struct MprEvent *pruneTimer;        /**< Timer for excess threads pruner */
    MprWorkerProc   startWorker;        /**< Worker thread startup hook */
} MprWorkerService;


/*
    Internal
 */
extern MprWorkerService *mprCreateWorkerService();
extern int mprStartWorkerService();
extern void mprWakeWorkers();
extern void mprSetWorkerStartCallback(MprWorkerProc start);

/**
    Get the count of available worker threads
    Return the count of free threads in the worker thread pool.
    @returns An integer count of worker threads.
    @ingroup MprWorker
 */
extern int mprGetAvailableWorkers();

/**
    Set the default worker stack size
    @param size Stack size in bytes
 */
extern void mprSetWorkerStackSize(int size);

/**
    Set the minimum count of worker threads
    Set the count of threads the worker pool will have. This will cause the worker pool to pre-create at least this 
    many threads.
    @param count Minimum count of threads to use.
    @ingroup MprWorker
 */
extern void mprSetMinWorkers(int count);

/**
    Set the maximum count of worker threads
    Set the maximum number of worker pool threads for the MPR. If this number if less than the current number of threads,
        excess threads will be gracefully pruned as they exit.
    @param count Maximum limit of threads to define.
    @ingroup MprWorker
 */
extern void mprSetMaxWorkers(int count);

/**
    Get the maximum count of worker pool threads
    Get the maximum limit of worker pool threads. 
    @return The maximum count of worker pool threads.
    @ingroup MprWorker
 */
extern int mprGetMaxWorkers();

/**
    Get the Worker service statistics
    @param ws Worker service object
    @param stats Reference to stats object to receive the stats
 */
extern void mprGetWorkerServiceStats(MprWorkerService *ws, MprWorkerStats *stats);

/*
    Worker Thread State
 */
#define MPR_WORKER_BUSY        0x1          /**< Worker currently running to a callback */
#define MPR_WORKER_PRUNED      0x2          /**< Worker has been pruned and will be terminated */
#define MPR_WORKER_IDLE        0x4          /**< Worker is sleeping (idle) on idleCond */

/**
    Worker thread structure. Worker threads are allocated and dedicated to tasks. When idle, they are stored in
    an idle worker pool. An idle worker pruner runs regularly and terminates idle workers to save memory.
    @defgroup MprWorker MprWorker
    @see MPrWorkerProc MprWorkerService MprWorkerStats mprActivateWorker mprDedicateWorker mprGetCurrentWorker 
        mprGetMaxWorkers mprGetWorkerServiceStats mprReleaseWorker mprSetMaxWorkers mprSetMinWorkers 
        mprSetWorkerStackSize mprStartWorker 
 */
typedef struct MprWorker {
    MprWorkerProc   proc;                   /**< Procedure to run */
    MprWorkerProc   cleanup;                /**< Procedure to cleanup after run before sleeping */
    void            *data;                  /**< User per-worker data */
    int             state;                  /**< Worker state */
    int             flags;                  /**< Worker flags */
    MprThread       *thread;                /**< Thread associated with this worker */
    MprTime         lastActivity;           /**< When the worker was last used */
    MprWorkerService *workerService;        /**< Worker service */
    MprCond         *idleCond;              /**< Used to wait for work */
} MprWorker;

extern void mprActivateWorker(MprWorker *worker, MprWorkerProc proc, void *data);

/**
    Dedicate a worker thread to a current real thread. This implements thread affinity and is required on some platforms
        where some APIs (waitpid on uClibc) cannot be called on a different thread.
    @param worker Worker object
    @param worker Worker thread reference
 */
extern void mprDedicateWorker(MprWorker *worker);

/*
    Get the worker object if the current thread is actually a worker thread.
    @returns A worker thread object if the thread is a worker thread. Otherwise, NULL.
 */
extern MprWorker *mprGetCurrentWorker();

/**
    Release a worker thread. This releases a worker thread to be assignable to any real thread.
    @param worker Worker object
    @param worker Worker thread reference
 */
extern void mprReleaseWorker(MprWorker *worker);

/**
    Start a worker thread
    @description Start a worker thread executing the given worker procedure callback.
    @param proc Worker procedure callback
    @param data Data parameter to the callback
    @returns Zero if successful, otherwise a negative MPR error code.
 */
extern int mprStartWorker(MprWorkerProc proc, void *data);

/* Internal */
extern int mprAvailableWorkers();

/********************************** Crypto ************************************/
/**
    Return a random number
    @returns A random integer
    @ingroup Mpr
 */
extern int mprRandom();

/**
    Decode a null terminated string using base-46 encoding.
    Decoding will terminate at the first null or '='.
    @param str String to decode
    @returns Buffer containing the encoded data
    @ingroup Mpr
 */
extern char *mprDecode64(cchar *str);

/**
    Decode base 64 blocks up to a NULL or equals
 */
#define MPR_DECODE_TOKEQ 1

/**
    Decode a null terminated string  using base-46 encoding.
    @param buf String to decode
    @param len Return parameter with the Length of the decoded data
    @param flags Set to MPR_DECODE_TOKEQ to stop at the first '='
    @returns Buffer containing the encoded data and returns length in len.
    @ingroup Mpr
  */
extern char *mprDecode64Block(cchar *buf, ssize *len, int flags);

/**
    Encode a string using base-46 encoding.
    @param str String to encode
    @returns Buffer containing the encoded string.
    @ingroup Mpr
 */
extern char *mprEncode64(cchar *str);

/**
    Encode buffer using base-46 encoding.
    @param buf Buffer to encode
    @param len Length of the buffer to encode
    @returns Buffer containing the encoded string.
    @ingroup Mpr
 */
extern char *mprEncode64Block(cchar *buf, ssize len);

/**
    Get an MD5 checksum
    @param s String to examine
    @returns An allocated MD5 checksum string.
    @ingroup Mpr
 */
extern char *mprGetMD5(cchar *s);

/**
    Get an MD5 checksum with optional prefix string and buffer length
    @param buf Buffer to checksum
    @param len Size of the buffer
    @param prefix String prefix to insert at the start of the result
    @returns An allocated MD5 checksum string.
    @ingroup Mpr
 */
extern char *mprGetMD5WithPrefix(cchar *buf, ssize len, cchar *prefix);

/********************************* Encoding ***********************************/
/*  
    Character encoding masks
 */
#define MPR_ENCODE_HTML             0x1
#define MPR_ENCODE_SHELL            0x2
#define MPR_ENCODE_URI              0x4             /* Encode for ejs Uri.encode */
#define MPR_ENCODE_URI_COMPONENT    0x8             /* Encode for ejs Uri.encodeComponent */
#define MPR_ENCODE_JS_URI           0x10            /* Encode according to ECMA encodeUri() */
#define MPR_ENCODE_JS_URI_COMPONENT 0x20            /* Encode according to ECMA encodeUriComponent */

/** 
    Encode a string escaping typical command (shell) characters
    @description Encode a string escaping all dangerous characters that have meaning for the unix or MS-DOS command shells.
    @param cmd Command string to encode
    @param escChar Escape character to use when encoding the command.
    @return An allocated string containing the escaped command.
    @ingroup Mpr
 */
extern char *mprEscapeCmd(cchar *cmd, int escChar);

/**
    Encode a string by escaping typical HTML characters
    @description Encode a string escaping all dangerous characters that have meaning in HTML documents
    @param html HTML content to encode
    @return An allocated string containing the escaped HTML.
    @ingroup Mpr
 */
extern char *mprEscapeHtml(cchar *html);

/** 
    Encode a string by escaping URI characters
    @description Encode a string escaping all characters that have meaning for URIs.
    @param uri URI to encode
    @param map Map to encode characters. Select from MPR_ENCODE_URI or MPR_ENCODE_URI_COMPONENT.
    @return An allocated string containing the encoded URI. 
    @ingroup Mpr
 */
extern char *mprUriEncode(cchar *uri, int map);

/** 
    Decode a URI string by de-scaping URI characters
    @description Decode a string with www-encoded characters that have meaning for URIs.
    @param uri URI to decode
    @return A reference to the buf argument.
    @ingroup Mpr
 */
extern char *mprUriDecode(cchar *uri);

/********************************* Signals ************************************/

#if MACOSX
    #define MPR_MAX_SIGNALS 40              /**< Max signals that can be managed */
#elif LINUX
    #define MPR_MAX_SIGNALS 48
#else
    #define MPR_MAX_SIGNALS 40
#endif

/**
    Signal callback procedure
    @ingroup MprSignal
 */
typedef void (*MprSignalProc)(void *arg, struct MprSignal *sp);


/**
    Per signal structure
    @ingroup MprSignal
 */
typedef struct MprSignalInfo {
    int             triggered;              /**< Set to true when triggered */
} MprSignalInfo;


/**
    Signal control structure 
    @defgroup MprSignal MprSignal
    @see MprSignalProc MprSignalService MprSingalInfo mprAddSignalHandler mprAddStandardSignals 
 */
typedef struct MprSignal {
    struct MprSignal *next;                 /**< Chain of handlers on the same signo */
    MprSignalProc   handler;                /**< Signal handler (non-native) */
    void            (*sigaction)();         /**< Prior sigaction handler */
    void            *data;                  /**< Handler data */
    MprDispatcher   *dispatcher;            /**< Dispatcher to service handler */
    int             flags;                  /**< Control flags */
    int             signo;                  /**< Signal number */
} MprSignal;


/**
    Signal service control
    @ingroup MprSignal
 */
typedef struct MprSignalService {
    MprSignal       **signals;              /**< Signal handlers */
    MprList         *standard;              /**< Standard signal handlers */
    MprMutex        *mutex;                 /**< Multithread sync */
    MprSignalInfo   info[MPR_MAX_SIGNALS];  /**< Actual signal info and arg */
    int             hasSignals;             /**< Signal sent to process */
#if BIT_UNIX_LIKE
    struct sigaction prior[MPR_MAX_SIGNALS];/**< Prior sigaction handler before hooking */
#endif
} MprSignalService;


/*
    Internal
 */
extern MprSignalService *mprCreateSignalService();
extern void mprStopSignalService();
extern void mprRemoveSignalHandler(MprSignal *sp);
extern void mprServiceSignals();

/**
    Add standard trapping of system signals. The trapped signals are SIGINT, SIGQUIT, SIGTERM, SIGPIPE and SIGXFSZ. 
    SIGPIPE and SIGXFSZ are ignored. A graceful shutdown is initiated for SIGTERM whereas SIGINT and SIGQUIT will 
    do an immediate exit.
 */
extern void mprAddStandardSignals();

#define MPR_SIGNAL_BEFORE   0x1             /**< Flag to mprAddSignalHandler to run handler before existing handlers */
#define MPR_SIGNAL_AFTER    0x2             /**< Flag to mprAddSignalHandler to run handler after existing handlers */

/**
    Add a signal handler. The signal handling mechanism will trap the specified signal if issued and create an
    event on the given dispatcher. This will cause the handler function to be safely run by the dispatcher.
    Normally, signal handlers are difficult to write as the code must be Async-safe. This API permits the use of 
    common, single-threaded code to be used for signal handlers without worrying about pre-emption by other signals
    or threads.
    @param signo Signal number to handle
    @param handler Call back procedure to invoke. This has the signature #MprSignalProc.
    @param arg Argument to provide to the handler.
    @param dispatcher Event dispatcher on which to queue an event to run the handler.
    @param flags Set to either MPR_SIGNAL_BEFORE or MPR_SIGNAL_AFTER to run the handler before/after existing handlers.
 */
extern MprSignal *mprAddSignalHandler(int signo, void *handler, void *arg, MprDispatcher *dispatcher, int flags);

/******************************** Commands ************************************/
/**
    Callback function before doing a fork()
    @ingroup MprCmd
 */
typedef void (*MprForkCallback)(void *arg);

/*
    Command execution service
 */
typedef struct MprCmdService {
    MprList         *cmds;              /* List of all commands */
    MprMutex        *mutex;             /* Multithread sync */
} MprCmdService;

/*
    Internal
 */
extern MprCmdService *mprCreateCmdService();
extern void mprStopCmdService();

/*
    Child status structure. Designed to be async-thread safe.
 */
typedef struct MprCmdChild {
    ulong           pid;                /*  Process ID */
    int             exitStatus;         /*  Exit status */
} MprCmdChild;

#define MPR_CMD_EOF_COUNT       2
#define MPR_CMD_VXWORKS_EOF     "_ _EOF_ _"     /**< Special string for VxWorks CGI to emit to signal EOF */
#define MPR_CMD_VXWORKS_EOF_LEN 9               /**< Length of MPR_CMD_VXWORKS_EOF */

/*
    Channels for clientFd and serverFd
 */
#define MPR_CMD_STDIN           0       /**< Stdout for the client side */
#define MPR_CMD_STDOUT          1       /**< Stdin for the client side */
#define MPR_CMD_STDERR          2       /**< Stderr for the client side */
#define MPR_CMD_MAX_PIPE        3

/*
    Handler for command output and completion
    Cmd procs must return the number of bytes read or -1 for errors.
 */
typedef ssize (*MprCmdProc)(struct MprCmd *cmd, int channel, void *data);

/*
    Flags for mprRunCmd
 */
#define MPR_CMD_NEW_SESSION     0x1     /**< mprRunCmd flag to create a new session on unix */
#define MPR_CMD_SHOW            0x2     /**< mprRunCmd flag to show the window of the created process on windows */
#define MPR_CMD_DETACH          0x4     /**< mprRunCmd flag to detach the child process and don't wait */
#define MPR_CMD_EXACT_ENV       0x8     /**< mprRunCmd flag to use the exact environment (no inherit from parent) */
#define MPR_CMD_IN              0x1000  /**< mprRunCmd flag to connect to stdin */
#define MPR_CMD_OUT             0x2000  /**< mprRunCmd flag to capture stdout */
#define MPR_CMD_ERR             0x4000  /**< mprRunCmd flag to capture stdout */

typedef struct MprCmdFile {
    char            *name;
    int             fd;
    int             clientFd;
#if BIT_WIN_LIKE
    HANDLE          handle;
#endif
} MprCmdFile;

/**
    Command execution Service
    @description The MprCmd service enables execution of local commands. It uses three full-duplex pipes to communicate
        read, write and error data with the command. 
    @stability Evolving.
    @see mprCloseCmdFd mprCreateCmd mprDestroyCmd mprDisableCmdEvents mprDisconnectCmd mprEnableCmdEvents 
        mprFinalizeCmd mprGetCmdBuf mprGetCmdExitStatus mprGetCmdFd mprIsCmdComplete mprIsCmdRunning mprPollCmd 
        mprReadCmd mprReapCmd mprRunCmd mprRunCmdV mprSetCmdCallback mprSetCmdDir mprSetCmdEnv mprSetCmdSearchPath 
        mprStartCmd mprStopCmd mprWaitForCmd mprWriteCmd 
    @defgroup MprCmd MprCmd
 */
typedef struct MprCmd {
    /*  Ordered for debugging */

    cchar           *program;           /**< Program path name */
    int             pid;                /**< Process ID of the created process */
    int             status;             /**< Command exit status */
    int             flags;              /**< Control flags (userFlags not here) */
    int             eofCount;           /**< Count of end-of-files */
    int             requiredEof;        /**< Number of EOFs required for an exit */
    int             complete;           /**< All channels EOF and status gathered */
    int             stopped;            /**< Command stopped */
    cchar           **makeArgv;         /**< Allocated argv */ 
    cchar           **argv;             /**< List of args. Null terminated */
    MprList         *env;               /**< List of environment variables. Null terminated. */
    char            *dir;               /**< Current working dir for the process */
    cchar           **defaultEnv;       /**< Environment to use if no env passed to mprStartCmd */
    char            *searchPath;        /**< Search path to use to locate the command */
    int             argc;               /**< Count of args in argv */
#if UNUSED && FUTURE
    MprTime         timestamp;          /**< Timeout timestamp for last I/O  */
    MprTime         timeoutPeriod;      /**< Timeout value */
#endif
    int             timedout;           /**< Request has timedout */
    MprCmdFile      files[MPR_CMD_MAX_PIPE]; /**< Stdin, stdout for the command */
    MprWaitHandler  *handlers[MPR_CMD_MAX_PIPE];
    MprDispatcher   *dispatcher;        /**< Dispatcher to use for wait events */
    MprCmdProc      callback;           /**< Handler for client output and completion */
    void            *callbackData;
    MprForkCallback forkCallback;       /**< Forked client callback */
    MprSignal       *signal;            /**< Signal handler for SIGCHLD */
    void            *forkData;
    MprBuf          *stdoutBuf;         /**< Standard output from the client */
    MprBuf          *stderrBuf;         /**< Standard error output from the client */
    void            *userData;          /**< User data storage */
    int             userFlags;          /**< User flags storage */
#if BIT_WIN_LIKE
    HANDLE          thread;             /**< Handle of the primary thread for the created process */
    HANDLE          process;            /**< Process handle for the created process */
    char            *command;           /**< Windows command line */          
    char            *arg0;              /**< Windows sanitized argv[0] */          
#endif

#if VXWORKS
    /*
        Don't use MprCond so we can build single-threaded and still use MprCmd
     */
    SEM_ID          startCond;          /**< Synchronization semaphore for task start */
    SEM_ID          exitCond;           /**< Synchronization semaphore for task exit */
#endif
    MprMutex        *mutex;             /**< Multithread sync */
} MprCmd;


/**
    Close the command channel
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to close. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @ingroup MprCmd
 */
extern void mprCloseCmdFd(MprCmd *cmd, int channel);

/**
    Create a new Command object 
    @returns A newly allocated MprCmd object.
    @ingroup MprCmd
 */
extern MprCmd *mprCreateCmd(MprDispatcher *dispatcher);

/**
    Destroy the command
    @param cmd MprCmd object created via mprCreateCmd
    @ingroup MprCmd
 */
extern void mprDestroyCmd(MprCmd *cmd);

/**
    Disable command I/O events. This disables events on a given channel.
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to close. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @ingroup MprCmd
 */
extern void mprDisableCmdEvents(MprCmd *cmd, int channel);

/**
    Disconnect a command its underlying I/O channels. This is used to prevent further I/O wait events while
    still preserving the MprCmd object.
    @param cmd MprCmd object created via mprCreateCmd
    @ingroup MprCmd
 */
extern void mprDisconnectCmd(MprCmd *cmd);

/**
    Enable command I/O events. This enables events on a given channel.
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to close. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @ingroup MprCmd
 */
extern void mprEnableCmdEvents(MprCmd *cmd, int channel);

/**
    Finalize the writing of data to the command process
    @param cmd MprCmd object created via mprCreateCmd
    @ingroup MprCmd
 */
extern void mprFinalizeCmd(MprCmd *cmd);

/**
    Get the underlying buffer for a channel
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to close. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @return A reference to the MprBuf buffer structure
    @ingroup MprCmd
 */
extern MprBuf *mprGetCmdBuf(MprCmd *cmd, int channel);

/**
    Get the command exit status
    @param cmd MprCmd object created via mprCreateCmd
    @return status If the command has exited, a status between 0 and 255 is returned. Otherwise, a negative error
    code is returned.
    @ingroup MprCmd
 */
extern int mprGetCmdExitStatus(MprCmd *cmd);

/**
    Get the underlying file descriptor for an I/O channel
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to close. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @return The file descriptor 
    @ingroup MprCmd
 */
extern int mprGetCmdFd(MprCmd *cmd, int channel);

/**
    Test if a command is complete. A command is complete when the child has exited and all command output and error
    output has been received.
    @param cmd MprCmd object created via mprCreateCmd
 */
extern int mprIsCmdComplete(MprCmd *cmd);

/**
    Test if the command is still running.
    @param cmd MprCmd object created via mprCreateCmd
    @return True if the command is still running
    @ingroup MprCmd
 */
extern bool mprIsCmdRunning(MprCmd *cmd);

/**
    Poll for I/O on the command pipes. This is only used on windows which can't adequately detect EOF on a named pipe.
    @param cmd MprCmd object created via mprCreateCmd
    @param timeout Time in milliseconds to wait for the command to complete and exit.
    @ingroup MprCmd
 */
extern void mprPollCmd(MprCmd *cmd, MprTime timeout);

/**
    Make the I/O channels to send and receive data to and from the command.
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to read from. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @param buf Buffer to read into
    @param bufsize Size of buffer
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup MprCmd
 */
extern ssize mprReadCmd(MprCmd *cmd, int channel, char *buf, ssize bufsize);

/**
    Reap the command. This waits for and collect the command exit status. 
    @param cmd MprCmd object created via mprCreateCmd
    @param timeout Time in milliseconds to wait for the command to complete and exit.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup MprCmd
 */
extern int mprReapCmd(MprCmd *cmd, MprTime timeout);

/**
    Run a command using a string command line. This starts the command via mprStartCmd() and waits for its completion.
    @param cmd MprCmd object created via mprCreateCmd
    @param command Command line to run
    @param envp Array of environment strings. Each environment string should be of the form: "KEY=VALUE". The array
        must be null terminated.
    @param out Reference to a string to receive the stdout from the command.
    @param err Reference to a string to receive the stderr from the command.
    @param timeout Time in milliseconds to wait for the command to complete and exit.
    @param flags Flags to modify execution. Valid flags are:
        MPR_CMD_NEW_SESSION     Create a new session on Unix
        MPR_CMD_SHOW            Show the commands window on Windows
        MPR_CMD_IN              Connect to stdin
        MPR_CMD_EXACT_ENV       Use the exact environment supplied. Don't inherit and blend with existing environment.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup MprCmd
 */
extern int mprRunCmd(MprCmd *cmd, cchar *command, cchar **envp, char **out, char **err, MprTime timeout, int flags);

/**
    Run a command using an argv[] array of arguments. This invokes mprStartCmd() and waits for its completion.
    @param cmd MprCmd object created via mprCreateCmd
    @param argc Count of arguments in argv
    @param argv Command arguments array
    @param envp Array of environment strings. Each environment string should be of the form: "KEY=VALUE". The array
        must be null terminated.
    @param out Reference to a string to receive the stdout from the command.
    @param err Reference to a string to receive the stderr from the command.
    @param timeout Time in milliseconds to wait for the command to complete and exit.
    @param flags Flags to modify execution. Valid flags are:
        MPR_CMD_NEW_SESSION     Create a new session on Unix
        MPR_CMD_SHOW            Show the commands window on Windows
        MPR_CMD_IN              Connect to stdin
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup MprCmd
 */
extern int mprRunCmdV(MprCmd *cmd, int argc, cchar **argv, cchar **envp, char **out, char **err, MprTime timeout, int flags);

/**
    Define a callback to be invoked to receive response data from the command.
    @param cmd MprCmd object created via mprCreateCmd
    @param callback Function of the signature MprCmdProc which will be invoked for receive notification
        for data from the commands stdout and stderr channels. MprCmdProc has the signature:
        int callback(MprCmd *cmd, int channel, void *data) {}
    @param data User defined data to be passed to the callback.
    @ingroup MprCmd
 */
extern void mprSetCmdCallback(MprCmd *cmd, MprCmdProc callback, void *data);

/**
    Set the default environment to use for commands.
    @description  This environment is used if one is not defined via #mprStartCmd
    @param cmd MprCmd object created via mprCreateCmd
    @param env Array of environment "KEY=VALUE" strings. Null terminated.
    @internal
 */
extern void mprSetCmdDefaultEnv(MprCmd *cmd, cchar **env);

/**
    Set the home directory for the command
    @param cmd MprCmd object created via mprCreateCmd
    @param dir String directory path name.
    @ingroup MprCmd
 */
extern void mprSetCmdDir(MprCmd *cmd, cchar *dir);

/**
    Set the command environment
    @param cmd MprCmd object created via mprCreateCmd
    @param env Array of environment strings. Each environment string should be of the form: "KEY=VALUE". The array
        must be null terminated.
    @ingroup MprCmd
 */
extern void mprSetCmdEnv(MprCmd *cmd, cchar **env);

/**
    Set the default command search path.
    @description The search path is used to locate the program to run for the command.
    @param cmd MprCmd object created via mprCreateCmd
    @param search Search string. This is in a format similar to the PATH environment variable.
 */
extern void mprSetCmdSearchPath(MprCmd *cmd, cchar *search);

/**
    Start the command. This starts the command but does not wait for its completion. Once started, mprWriteCmd
    can be used to write to the command and response data can be received via mprReadCmd.
    @param cmd MprCmd object created via mprCreateCmd
    @param argc Count of arguments in argv
    @param argv Command arguments array
    @param envp Array of environment strings. Each environment string should be of the form: "KEY=VALUE". The array
        must be null terminated.
    @param flags Flags to modify execution. Valid flags are:
        MPR_CMD_NEW_SESSION     Create a new session on Unix
        MPR_CMD_SHOW            Show the commands window on Windows
        MPR_CMD_IN              Connect to stdin
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup MprCmd
 */
extern int mprStartCmd(MprCmd *cmd, int argc, cchar **argv, cchar **envp, int flags);

/**
    Stop the command. The command is immediately killed.
    @param cmd MprCmd object created via mprCreateCmd
    @param signal Signal to send to the command to kill if required
    @ingroup MprCmd
 */
extern int mprStopCmd(MprCmd *cmd, int signal);

/**
    Wait for the command to complete.
    @param cmd MprCmd object created via mprCreateCmd
    @param timeout Time in milliseconds to wait for the command to complete and exit.
    @return Zero if successful. Otherwise a negative MPR error code.
    @ingroup MprCmd
 */
extern int mprWaitForCmd(MprCmd *cmd, MprTime timeout);

/**
    Write data to an I/O channel
    @param cmd MprCmd object created via mprCreateCmd
    @param channel Channel number to read from. Should be either MPR_CMD_STDIN, MPR_CMD_STDOUT or MPR_CMD_STDERR.
    @param buf Buffer to read into
    @param bufsize Size of buffer
    @ingroup MprCmd
 */
extern ssize mprWriteCmd(MprCmd *cmd, int channel, char *buf, ssize bufsize);

/********************************** Cache *************************************/

#define MPR_CACHE_SHARED        0x1     /**< Use shared cache */
#define MPR_CACHE_ADD           0x2     /**< Add key if not already existing */
#define MPR_CACHE_SET           0x4     /**< Update key value, create if required */
#define MPR_CACHE_APPEND        0x8     /**< Set and append if already existing */
#define MPR_CACHE_PREPEND       0x10    /**< Set and prepend if already existing */

/**
    In-memory caching. The MprCache provides a fast, in-memory caching of cache items. Cache items are string key / value 
    pairs. Cache items have a configurable lifespan and the Cache manager will automatically prune expired items. 
    Items also have an associated version number that can be used when writing to do transactional writes.
    @defgroup MprCache MprCache
    @see mprCreateCache mprDestroyCache mprExpireCache mprIncCache mprReadCache mprRemoveCache mprSetCacheLimits 
        mprWriteCache 
 */
typedef struct MprCache {
    MprHash         *store;             /**< Key/value store */
    MprMutex        *mutex;             /**< Cache lock*/
    MprEvent        *timer;             /**< Pruning timer */
    MprTime         lifespan;           /**< Default lifespan (msec) */
    int             resolution;         /**< Frequence for pruner */
    ssize           usedMem;            /**< Memory in use for keys and data */
    ssize           maxKeys;            /**< Max number of keys */
    ssize           maxMem;             /**< Max memory for session data */
    struct MprCache *shared;            /**< Shared common cache */
} MprCache;

/**
    Create a new cache object
    @param options Set of option flags. Select from #MPR_CACHE_SHARED, #MPR_CACHE_ADD, #MPR_CACHE_ADD, #MPR_CACHE_SET,
        #MPR_CACHE_APPEND, #MPR_CACHE_PREPEND.
    @return A cache instance object. On error, return null.
    @ingroup MprCache
 */
extern MprCache *mprCreateCache(int options);

/**
    Destroy a new cache object
    @param cache The cache instance object returned from #mprCreateCache.
 */
extern void *mprDestroyCache(MprCache *cache);

/**
    Set the expiry date for a cache item
    @param cache The cache instance object returned from #mprCreateCache.
    @param key Cache item key
    @param expires Time when the cache item will expire. If expires is zero, the item is immediately removed from the cache.
    @return Zero if the expiry is successfully updated. Return MPR_ERR_CANT_FIND if the cache item is not present in the
        cache.
    @ingroup MprCache
 */
extern int mprExpireCache(MprCache *cache, cchar *key, MprTime expires);

/**
    Increment a numeric cache item
    @param cache The cache instance object returned from #mprCreateCache.
    @param key Cache item key
    @param amount Numeric amount to increment the cache item. This may be a negative number to decrement the item.
    @return The new value for the cache item after incrementing.
    @ingroup MprCache
 */
extern int64 mprIncCache(MprCache *cache, cchar *key, int64 amount);

/**
    Prune the cache
    @description Prune the cache and discard all cached items
    @param cache The cache instance object returned from #mprCreateCache.
    @ingroup MprCache
 */
extern void mprPruneCache(MprCache *cache);

/**
    Read an item from the cache.
    @param cache The cache instance object returned from #mprCreateCache.
    @param key Cache item key
    @param modified Optional MprTime value reference to receive the last modified time of the cache item. Set to null
        if not required.
    @param version Optional int64 value reference to receive the version number of the cache item. Set to null
        if not required. Cache items have a version number that is incremented every time the item is updated.
    @return The cache item value
    @ingroup MprCache
  */
extern char *mprReadCache(MprCache *cache, cchar *key, MprTime *modified, int64 *version);

/**
    Remove items from the cache
    @param cache The cache instance object returned from #mprCreateCache.
    @param key Cache item key. If set to null, then remove all keys from the cache.
    @return True if the cache item was removed.
    @ingroup MprCache
  */
extern bool mprRemoveCache(MprCache *cache, cchar *key);

/**
    Set the cache resource limits
    @param cache The cache instance object returned from #mprCreateCache.
    @param keys Set the maximum number of keys the cache can store
    @param lifespan Set the default lifespan for cache items in milliseconds
    @param memory Memory limit in bytes for all cache keys and items.
    @param resolution Set the cache item pruner resolution. This defines how frequently the cache manager will check
        items for expiration.
    @ingroup MprCache
  */
extern void mprSetCacheLimits(MprCache *cache, int64 keys, int64 lifespan, int64 memory, int resolution);

/**
    Write a cache item
    @param cache The cache instance object returned from #mprCreateCache.
    @param key Cache item key to write
    @param value Value to set for the cache item
    @param modified Value to set for the cache last modified time. If set to zero, the current time is obtained via
        #mprGetTime.
    @param lifespan Lifespan of the item in milliseconds. The item will be removed from the cache by the Cache manager
        when the lifetime expires unless it is rewritten to extend the lifespan.
    @param version Expected version number of the item. This is used to do transactional writes to the cache item.
        First the version number is retrieved via #mprReadCache and that version number is supplied to mprWriteCache when
        the item is updated. If another caller updates the item in between the read/write, the version number will not 
        match when the item is subsequently written and this call will fail with the #MPR_ERR_BAD_STATE return code. Set to
        zero if version checking is not required.
    @param options Options to control how the item value is updated. Use #MPR_CACHE_SET to update the cache item and 
        create if it does not exist. Use #MPR_CACHE_ADD to add the item only if it does not already exits. Use 
        #MPR_CACHE_APPEND to append the parameter value to any existing cache item value. Use #MPR_CACHE_PREPEND to 
        prepend the value.
    @return If writing the cache item was successful this call returns the number of bytes written. Otherwise a negative 
        MPR error code is returned. #MPR_ERR_BAD_STATE will be returned if an invalid version number is supplied.
        #MPR_ERR_ALREADY_EXISTS will be returned if #MPR_CACHE_ADD is specified and the cache item already exists.
    @ingroup MprCache
 */
extern ssize mprWriteCache(MprCache *cache, cchar *key, cchar *value, MprTime modified, MprTime lifespan, 
        int64 version, int options);

/******************************** Mime Types **********************************/
/**
    Mime Type hash table entry (the URL extension is the key)
    @stability Evolving
    @defgroup MprMime MprMime
    @see MprMime mprAddMime mprCreateMimeTypes mprGetMimeProgram mprLookupMime mprSetMimeProgram
 */
typedef struct MprMime {
    char    *type;                          /**< Mime type string */
    char    *program;                       /**< Mime type string */
} MprMime;

/**
    Add a mime type to the mime type table
    @param table type hash table returned by #mprCreateMimeTypes
    @param ext Filename extension to use as a key for the given mime type
    @param mimeType Mime type string to associate with the ext key
    @return Mime type entry object. This is owned by the mime type table.
    @ingroup MprMime
 */
extern MprMime *mprAddMime(MprHash *table, cchar *ext, cchar *mimeType);

/**
    Create the mime types
    @param path Filename of a mime types definition file
    @return Hash table of mime types keyed by file extension 
    @ingroup MprMime
 */
extern MprHash *mprCreateMimeTypes(cchar *path);

/**
    Get the mime type program for a given mimeType
    @param table type hash table returned by #mprCreateMimeTypes
    @param mimeType Mime type to update
    @return The program name associated with this mime type
    @ingroup MprMime
 */
extern cchar *mprGetMimeProgram(MprHash *table, cchar *mimeType);

/** 
    Get the mime type for an extension.
    This call will return the mime type from a limited internal set of mime types for the given path or extension.
    @param table Hash table of mime types to examine
    @param ext Path or extension to examine
    @returns Mime type string
    @ingroup MprMime
 */
extern cchar *mprLookupMime(MprHash *table, cchar *ext);

/**
    Set the mime type program
    @param table type hash table returned by #mprCreateMimeTypes
    @param mimeType Mime type to update
    @param program Program name to associate with this mime type
    @return Zero if the update is successful. Otherwise return MPR_ERR_CANT_FIND if the mime type is not present in 
        the mime type table.
    @ingroup MprMime
 */
extern int mprSetMimeProgram(MprHash *table, cchar *mimeType, cchar *program);

/************************************ MPR *************************************/
/*
    Mpr state
 */
#define MPR_STARTED                 0x1     /**< Mpr services started */
#define MPR_STOPPING                0x2     /**< App is stopping. Services should not take new requests */
#define MPR_STOPPING_CORE           0x4     /**< Stopping core services: GC and event dispatch */
#define MPR_FINISHED                0x8     /**< Mpr object destroyed  */

/*
    MPR flags
 */
#define MPR_LOG_APPEND              0x10    /**< Append to existing log files */
#define MPR_LOG_ANEW                0x20    /**< Start anew on boot (rotate) */

typedef bool (*MprIdleCallback)();
typedef void (*MprTerminator)(int how, int status);

/**
    Primary MPR application control structure
    @description The Mpr structure stores critical application state information.
    @stability Evolving.
    @see mprAddTerminator mprBreakpoint mprCreate mprCreateOsService mprDecode64 mprDestroy mprEmptyString mprEncode64
    mprEscapeCmd mprEscapseHtml mprGetApp mprGetAppDir mprGetAppName mprGetAppPath mprGetAppTitle mprGetAppVersion
    mprGetCmdlineLogging mprGetDebugMode mprGetDomainName mprGetEndian mprGetError mprGetErrorMsg mprGetHostName
    mprGetHwnd mprGetInst mprGetIpAddr mprGetKeyValue mprGetLogLevel mprGetMD5 mprGetMD5WithPrefix mprGetOsError
    mprGetRandomBytes mprGetServerName mprIsExiting mprIsFinished mprIsIdle mprIsStopping mprIsStoppingCore mprMakeArgv
    mprRandom mprReadRegistry mprRemoveKeyValue mprRestart mprServicesAreIdle mprSetAppName mprSetCmdlineLogging
    mprSetDebugMode mprSetDomainName mprSetExitStrategy mprSetHostName mprSetHwnd mprSetIdleCallback mprSetInst
    mprSetIpAddr mprSetLogLevel mprSetServerName mprSetSocketMessage mprShouldAbortRequests mprShouldDenyNewRequests
    mprSignalExit mprSleep mprStart mprStartEventsThread mprStartOsService mprStopOsService mprTerminate mprUriDecode
    mprUriEncode mprWaitTillIdle mprWriteRegistry 
    @defgroup Mpr Mpr
 */
typedef struct Mpr {
    MprHeap         *heap;                  /**< Memory heap control */
    bool            debugMode;              /**< Run in debug mode (no timers) */
    int             logLevel;               /**< Log trace level */
    int             logBackup;              /**< Number of log files preserved when backing up */
    ssize           logSize;                /**< Maximum log size */
    cchar           *logPath;               /**< Log path name */
    MprLogHandler   logHandler;             /**< Current log handler callback */
    MprFile         *logFile;               /**< Log file */
    MprHash         *mimeTypes;             /**< Table of mime types */
    MprHash         *timeTokens;            /**< Date/Time parsing tokens */
    MprFile         *stdError;              /**< Standard error file */
    MprFile         *stdInput;              /**< Standard input file */
    MprFile         *stdOutput;             /**< Standard output file */
    MprTime         start;                  /**< When the MPR started */
    MprTime         exitTimeout;            /**< Request timeout when exiting */
    char            *pathEnv;               /**< Cached PATH env var. Used by MprCmd */
    char            *name;                  /**< Product name */
    char            *title;                 /**< Product title */
    char            *version;               /**< Product version */
    int             argc;                   /**< Count of command line args */
    cchar           **argv;                 /**< Application command line args (not alloced) */
    char            **argBuf;               /**< Space for allocated argv */
    char            *domainName;            /**< Domain portion */
    char            *hostName;              /**< Host name (fully qualified name) */
    char            *ip;                    /**< Public IP Address */
    char            *serverName;            /**< Server name portion (no domain) */
    char            *appPath;               /**< Path name of application executable */
    char            *appDir;                /**< Path of directory containing app executable */
    int             eventing;               /**< Servicing events thread is active */
    int             exitStrategy;           /**< How to exit the app (normal, immediate, graceful) */
    int             exitStatus;             /**< Proposed program exit status */
    int             flags;                  /**< Misc flags */
    int             hasError;               /**< Mpr has an initialization error */
    int             state;                  /**< Processing state */

    bool            cmdlineLogging;         /**< App has specified --log on the command line */

    /*
        Service pointers
     */
    struct MprCmdService    *cmdService;    /**< Command service object */
    struct MprEventService  *eventService;  /**< Event service object */
    struct MprFileSystem    *fileSystem;    /**< File system service object */
    struct MprModuleService *moduleService; /**< Module service object */
    struct MprOsService     *osService;     /**< O/S service object */
    struct MprSignalService *signalService; /**< Signal service object */
    struct MprSocketService *socketService; /**< Socket service object */
    struct MprThreadService *threadService; /**< Thread service object */
    struct MprWorkerService *workerService; /**< Worker service object */
    struct MprWaitService   *waitService;   /**< IO Waiting service object */

    struct MprDispatcher    *dispatcher;    /**< Primary dispatcher */
    struct MprDispatcher    *nonBlock;      /**< Nonblocking dispatcher */

    /*
        These are here to optimize access to these singleton service objects
     */
    void            *appwebService;         /**< Appweb service object */
    void            *ediService;            /**< EDI object */
    void            *ejsService;            /**< Ejscript service */
    void            *espService;            /**< ESP service object */
    void            *httpService;           /**< Http service object */
    void            *testService;           /**< Test service object */

    MprList         *terminators;           /**< Termination callbacks */
    MprIdleCallback idleCallback;           /**< Invoked to determine if the process is idle */
    MprOsThread     mainOsThread;           /**< Main OS thread ID */
    MprMutex        *mutex;                 /**< Thread synchronization */
    MprSpin         *spin;                  /**< Quick thread synchronization */
    MprSpin         *dtoaSpin[2];           /**< Dtoa thread synchronization */
    MprCond         *cond;                  /**< Sync after starting events thread */

    char            *emptyString;           /**< Empty string */
#if BIT_WIN_LIKE
    HINSTANCE       appInstance;            /**< Application instance (windows) */
#endif
} Mpr;

extern void mprNop(void *ptr);

#if DOXYGEN || BIT_WIN_LIKE
/**
    Return the MPR control instance.
    @description Return the MPR singleton control object. 
    @return Returns the MPR control object.
    @stability Evolving.
    @ingroup Mpr
 */
extern Mpr *mprGetMpr();
#define MPR mprGetMpr()
#else
    #define mprGetMpr() MPR
    extern Mpr *MPR;
#endif

#define MPR_DISABLE_GC          0x1         /**< Disable GC */
#define MPR_MARK_THREAD         0x4         /**< Start a dedicated marker thread for garbage collection */
#define MPR_SWEEP_THREAD        0x8         /**< Start a dedicated sweeper thread for garbage collection (unsupported) */
#define MPR_USER_EVENTS_THREAD  0x10        /**< User will explicitly manage own mprServiceEvents calls */
#define MPR_NO_WINDOW           0x20        /**< Don't create a windows Window */

#if BIT_TUNE == MPR_TUNE_SPEED
    #define MPR_THREAD_PATTERN (MPR_MARK_THREAD)
#else
    #define MPR_THREAD_PATTERN (MPR_MARK_THREAD)
#endif

/**
    Add a terminator callback
    @description When the MPR terminates, all terminator callbacks are invoked to give the application notice of
    impending exit.
    @param terminator MprTerminator callback function
    @ingroup Mpr
  */
extern void mprAddTerminator(MprTerminator terminator);

/**
    Create an instance of the MPR.
    @description Initializes the MPR and creates an Mpr control object. The Mpr Object manages all MPR facilities 
        and services.
    @param argc Count of command line args
    @param argv Command line arguments for the application. Arguments may be passed into the Mpr for retrieval
        by the unit test framework.
    @param flags
    @return Returns a pointer to the Mpr object. 
    @ingroup Mpr
 */
extern Mpr *mprCreate(int argc, char **argv, int flags);

/**
    Destroy the MPR
    @param how Exit strategy to use when exiting. Set to MPR_EXIT_DEFAULT to use the existing exit strategy.
    Set to MPR_EXIT_IMMEDIATE for an immediate abortive shutdown. Finalizers will not be run. Use MPR_EXIT_NORMAL to 
    allow garbage collection and finalizers to run. Use MPR_EXIT_GRACEFUL to allow all current requests and commands 
    to complete before exiting.
 */
extern void mprDestroy(int how);

/**
    Reference to a permanent preallocated empty string.
    @return An empty string
    @ingroup Mpr
 */
extern char* mprEmptyString();

/**
    Get the application directory
    @description Get the directory containing the application executable.
    @returns A string containing the application directory.
    @ingroup Mpr
 */
extern char *mprGetAppDir();

/**
    Get the application name defined via mprSetAppName
    @returns the one-word lower case application name defined via mprSetAppName
    @ingroup Mpr
 */
extern cchar *mprGetAppName();

/**
    Get the application executable path
    @returns A string containing the application executable path.
    @ingroup Mpr
 */
extern char *mprGetAppPath();

/**
    Get the application title string
    @returns A string containing the application title string.
    @ingroup Mpr
 */
extern cchar *mprGetAppTitle();

/**
    Get the application version string
    @returns A string containing the application version string.
    @ingroup Mpr
 */
extern cchar *mprGetAppVersion();

/**
    Get if command line logging is being used.
    @description Logging may be initiated by invoking an MPR based program with a "--log" switch. This API assists
        programs to tell the MPR that command line logging has been used.
    @return True if command line logging is in use.
*/
extern bool mprGetCmdlineLogging();

/**
    Get the debug mode.
    @description Returns whether the debug mode is enabled. Some modules
        observe debug mode and disable timeouts and timers so that single-step
        debugging can be used.
    @return Returns true if debug mode is enabled, otherwise returns false.
    @ingroup Mpr
 */
extern bool mprGetDebugMode();

/**
    Get the application domain name string
    @returns A string containing the application domain name string.
    @ingroup Mpr
 */
extern cchar *mprGetDomainName();

/**
    Return the endian byte ordering for the application
    @return MPR_LITTLE_ENDIAN or MPR_BIG_ENDIAN.
    @ingroup Mpr
 */
extern int mprGetEndian();

/**
    Return the error code for the most recent system or library operation.
    @description Returns an error code from the most recent system call. 
        This will be mapped to be either a POSIX error code or an MPR error code.
    @return The mapped error code.
    @ingroup Mpr
 */
extern int mprGetError();

/**
    Get the exit status
    @description Get the exit status set via mprTerminate()
    @return The proposed application exit status
    @ingroup Mpr
 */
extern int mprGetExitStatus();

/**
    Get the application host name string
    @returns A string containing the application host name string.
    @ingroup Mpr
 */
extern cchar *mprGetHostName();

/**
    Get the application IP address string
    @returns A string containing the application IP address string.
    @ingroup Mpr
 */
extern cchar *mprGetIpAddr();

/**
    Get the current logging level
    @return The current log level.
    @ingroup Mpr
 */
extern int mprGetLogLevel();

/**
    Get some random data
    @param buf Reference to a buffer to hold the random data
    @param size Size of the buffer
    @param block Set to true if it is acceptable to block while accumulating entropy sufficient to provide good 
        random data. Setting to false will cause this API to not block and may return random data of a lower quality.
  */
extern int mprGetRandomBytes(char *buf, ssize size, bool block);

/**
    Return the O/S error code.
    @description Returns an O/S error code from the most recent system call. 
        This returns errno on Unix systems or GetLastError() on Windows..
    @return The O/S error code.
    @ingroup Mpr
 */
extern int mprGetOsError();

/**
    Get the application server name string
    @returns A string containing the application server name string.
    @ingroup Mpr
 */
extern cchar *mprGetServerName();

/**
    Determine if the MPR is exiting
    @description Returns true if the MPR is in the process of exiting
    @returns True if the App has been instructed to exit.
    @ingroup Mpr
 */
extern bool mprIsExiting();

/**
    Determine if the MPR has finished. 
    @description This is true if the MPR services have been shutdown completely. This is typically
    used to determine if the App has been gracefully shutdown.
    @returns True if the App has been instructed to exit and all the MPR services have completed.
    @ingroup Mpr
 */
extern bool mprIsFinished();

/**
    Determine if the App is idle. 
    @description This call returns true if the App is not currently servicing any requests. By default this returns true
    if the MPR dispatcher, thread, worker and command subsytems are idle. Callers can replace or augment the standard
    idle testing by definining a new idle callback via mprSetIdleCallback.
    @return True if the App are idle.
    @ingroup Mpr
 */
extern bool mprIsIdle();

/**
    Test if the application is stopping
    @return True if the application is in the process of exiting
    @ingroup Mpr
 */
extern bool mprIsStopping();

/**
    Test if the application is stopping and core services are being terminated
    @return True if the application is in the process of exiting and core services should also exit.
    @ingroup Mpr
 */
extern bool mprIsStoppingCore();

#define MPR_ARGV_ARGS_ONLY    0x1     /**< Command is missing program name */

/**
    Make a argv style array of command arguments
    @description The given command is parsed and broken into separate arguments and returned in a null-terminated, argv
        array. Arguments in the command may be quoted with single or double quotes to group words into one argument. 
        Use back-quote "\\" to escape quotes.
    @param command Command string to parse.
    @param argv Output parameter containing the parsed arguments. 
    @param flags Set to MPR_ARGV_ARGS_ONLY if the command string does not contain a program name. In this case, argv[0] 
        will be set to "".
    @return The count of arguments in argv
    @ingroup Mpr
 */
extern int mprMakeArgv(cchar *command, cchar ***argv, int flags);

/**
    Nap for a while
    @description This routine blocks and does not yield for GC. Only use it for very short naps.
    @param msec Number of milliseconds to sleep
    @ingroup Mpr
*/
extern void mprNap(MprTime msec);

/**
    Make a argv style array of command arguments
    @description The given command is parsed and broken into separate arguments and returned in a null-terminated, argv
        array. Arguments in the command may be quoted with single or double quotes to group words into one argument. 
        Use back-quote "\\" to escape quotes. This routine modifies command and does not allocate any memory and may be
        used before mprCreate is invoked.
    @param command Command string to parse.
    @param argv Array for the arguments.
    @param maxArgs Size of the argv array.
    @return The count of arguments in argv
    @ingroup Mpr
 */
extern int mprParseArgs(char *command, char **argv, int maxArgs);

/**
    Restart the application
    @description This call immediately restarts the application. The standard input, output and error I/O channels are
    preserved. All other open file descriptors are closed.
    @ingroup Mpr
 */
extern void mprRestart();

/**
    Determine if the MPR services.
    @description This is the default routine invoked by mprIsIdle().
    @return True if the MPR services are idle.
    @ingroup Mpr
 */
extern bool mprServicesAreIdle();

/**
    Set the application name, title and version
    @param name One word, lower case name for the app.
    @param title Pascal case multi-word descriptive name.
    @param version Version of the app. Major-Minor-Patch. E.g. 1.2.3.
    @returns Zero if successful. Otherwise a negative MPR error code.
    @ingroup Mpr
 */
extern int mprSetAppName(cchar *name, cchar *title, cchar *version);

/**
    Set the application executable path
    @param path A string containing the application executable path.
    @ingroup Mpr
 */
extern void mprSetAppPath(cchar *path);

/**
    Set if command line logging was requested.
    @description Logging may be initiated by invoking an MPR based program with a "--log" switch. This API assists
        programs to tell the MPR that command line logging has been used.
    @param on Set to true to indicate command line logging is being used.
    @return True if command line logging was enabled before this call.
 */
extern bool mprSetCmdlineLogging(bool on);

/** 
    Turn on debug mode.
    @description Debug mode disables timeouts and timers. This makes debugging
        much easier.
    @param on Set to true to enable debugging mode.
    @ingroup Mpr
 */
extern void mprSetDebugMode(bool on);

/**
    Set the current logging level.
    @description This call defines the maximum level of messages that will be
        logged. Calls to mprLog specify a message level. If the message level
        is greater than the defined logging level, the message is ignored.
    @param level New logging level. Must be 0-9 inclusive.
    @return Returns the previous logging level.
    @ingroup MprLog
 */
extern void mprSetLogLevel(int level);

/**
    Set the application domain name string
    @param s New value to use for the application domain name.
    @ingroup Mpr
 */
extern void mprSetDomainName(cchar *s);

/**
    Set an environment variable value
    @param key Variable name
    @param value Variable value
    @ingroup Mpr
 */
extern void mprSetEnv(cchar *key, cchar *value);

/**
    Set the exit strategy for when the application terminates
    @param strategy Set strategy to MPR_EXIT_IMMEDIATE for the application to exit immediately when terminated.
        Set to MPR_EXIT_GRACEFUL for the application to exit gracefully after allowing all current requests to complete
        before terminating.
    @ingroup Mpr
  */
extern void mprSetExitStrategy(int strategy);

/**
    Set the exit timeout for a graceful shutdown or restart. A graceful shutdown waits for existing requests to 
    complete before exiting.
    @param timeout Time in milliseconds to wait when terminating the MPR
    @ingroup Mpr
 */
void mprSetExitTimeout(MprTime timeout);

/**
    Set the application host name string. This is internal to the application and does not affect the O/S host name.
    @param s New host name to use within the application
    @ingroup Mpr
 */
extern void mprSetHostName(cchar *s);

/**
    Define a new idle callback to be invoked by mprIsIdle().
    @param idleCallback Callback function to invoke to test if the application is idle.
    @ingroup Mpr
 */
MprIdleCallback mprSetIdleCallback(MprIdleCallback idleCallback);

/**
    Sete the application IP address string
    @param ip IP address string to store for the application
    @ingroup Mpr
 */
extern void mprSetIpAddr(cchar *ip);

/**
    Set the application server name string
    @param s New application server name to use within the application.
    @ingroup Mpr
 */
extern void mprSetServerName(cchar *s);

/**
    Test if requests should be aborted. 
    @description This is useful to determine if current requests should be terminated due to
    the application exiting.
    @return True if new requests should be denied.
    @ingroup Mpr
 */
extern bool mprShouldAbortRequests();

/**
    Test if new requests should be denied. This is useful in denying new requests when doing a graceful shutdown while
    continuing to service existing requests.
    @return True if new requests should be denied.
    @ingroup Mpr
 */
extern bool mprShouldDenyNewRequests();

/**
    Sleep for a while
    @description This routine blocks for the given time and yields for GC during that time. Ensure all memory
        is held before the sleep.
    @param msec Number of milliseconds to sleep
    @ingroup Mpr
*/
extern void mprSleep(MprTime msec);

/**
    Start the Mpr services
 */
extern int mprStart();

/**
    Start an thread dedicated to servicing events. This will create a new thread and invoke mprServiceEvents.
    @return Zero if successful.
 */
extern int mprStartEventsThread();

/*
    Terminate and Destroy flags
 */
#define MPR_EXIT_DEFAULT    0x1         /**< Exit as per MPR->defaultStrategy */
#define MPR_EXIT_IMMEDIATE  0x2         /**< Immediate exit */
#define MPR_EXIT_NORMAL     0x4         /**< Normal shutdown without waiting for requests to complete  */
#define MPR_EXIT_GRACEFUL   0x8         /**< Graceful shutdown waiting for requests to complete */
#define MPR_EXIT_RESTART    0x10        /**< Restart after exiting */

/**
    Terminate the application.
    @description Terminates the application and disposes of all allocated resources. The mprTerminate function will
    recursively free all memory allocated by the MPR.
    @param flags Termination control flags. Use MPR_EXIT_IMMEDIATE for an immediate abortive shutdown. Finalizers will
        not be run. Use MPR_EXIT_NORMAL to allow garbage collection and finalizers to run. Use MPR_EXIT_GRACEFUL to
        allow all current requests and commands to complete before exiting.
    @param status Proposed program exit status.
    @ingroup Mpr
 */
extern void mprTerminate(int flags, int status);

/**
    Wait until the application is idle
    @description This call blocks until application services are idle.
    @param timeout Time in milliseconds to wait for the application to be idle
    @return True if the application is idle.
    @ingroup Mpr
 */
extern int mprWaitTillIdle(MprTime timeout);

#if BIT_WIN_LIKE
/**
    Get the Windows window handle
    @return the windows HWND reference
 */
extern HWND mprGetHwnd();

/**
    Get the windows application instance
    @return The application instance identifier
 */
extern long mprGetInst();

/**
    Set the MPR windows handle
    @param handle Set the MPR default windows handle
 */
extern void mprSetHwnd(HWND handle);

/**
    Set the windows application instance
    @param inst The new windows application instance to set
 */
extern void mprSetInst(HINSTANCE inst);

/**
    Set the socket message number.
    @description Set the socket message number to use when using WSAAsyncSelect for windows.
    @param message Message number to use.
  */
extern void mprSetSocketMessage(int message);
#endif

#if (BIT_WIN_LIKE && !WINCE) || CYGWIN
/**
    Read a key from the Windows registry
    @param key Windows registry key to read
    @param name Windows registry name to read.
    @return The key/name setting
  */
extern char *mprReadRegistry(cchar *key, cchar *name);

/**
    Write a key value the Windows registry
    @param key Windows registry key to write
    @param name Windows registry name to write.
    @param value Value to set the key/name to.
    @return Zero if successful. Otherwise return a negative MPR error code.
  */
extern int mprWriteRegistry(cchar *key, cchar *name, cchar *value);
#endif /* (BIT_WIN_LIKE && !WINCE) || CYGWIN */

/*
    Internal
 */
extern void mprWriteToOsLog(cchar *msg, int flags, int level);
extern void mprUnlockDtoa(int n);
extern void mprLockDtoa(int n);

/*********************************** External *********************************/
/*
   Double conversions
 */
extern char *dtoa(double d, int mode, int ndigits, int* decpt, int* sign, char** rve);
extern void freedtoa(char* ptr);

/************************************* Test ***********************************/

struct MprTestGroup;

/**
    Unit test callback procedure
    @ingroup MprTestService
 */
typedef void (*MprTestProc)(struct MprTestGroup *tp);

/**
    Test case structure
    @ingroup MprTestServicer
 */
typedef struct MprTestCase {
    char            *name;
    int             level;
    MprTestProc     proc;
    int             (*init)(struct MprTestGroup *gp);
    int             (*term)(struct MprTestGroup *gp);
} MprTestCase;

/**
    Test case definition
    @ingroup MprTestService
 */
typedef struct MprTestDef {
    char                *name;
    struct MprTestDef   **groupDefs;
    int                 (*init)(struct MprTestGroup *gp);
    int                 (*term)(struct MprTestGroup *gp);
    MprTestCase         caseDefs[32];
} MprTestDef;


#undef  assert
/**
    Assert macro for use by unit tests
    @ingroup MprTestService
 */
#define assert(C)   assertTrue(gp, MPR_LOC, C, #C)

#define MPR_TEST(level, functionName) { #functionName, level, functionName, 0, 0 }

/**
    Test service facility
    @stability Evolving
    @defgroup MprTestService MprTestService
 */
typedef struct MprTestService {
    int             argc;                   /**< Count of arguments */
    char            **argv;                 /**< Arguments for test (not alloced) */
    int             activeThreadCount;      /**< Currently active test threads */
    char            *commandLine;
    bool            continueOnFailures;     /**< Keep testing on failures */
    bool            debugOnFailures;        /**< Break to the debugger */
    int             echoCmdLine;            /**< Echo the command line */
    int             firstArg;               /**< Count of arguments */
    MprList         *groups;                /**< Master list of test groups */
    MprList         *threadData;            /**< Per thread objects */
    int             iterations;             /**< Times to run the test */
    bool            singleStep;             /**< Pause between tests */
    cchar           *name;                  /**< Name for entire test */
    int             numThreads;             /**< Number of test threads */
    int             workers;                /**< Count of worker threads */
    MprTime         start;                  /**< When testing began */
    int             testDepth;              /**< Depth of entire test */
    int             totalFailedCount;       /**< Total count of failing tests */
    int             totalTestCount;         /**< Total count of all tests */
    MprList         *testFilter;            /**< Test groups to run */
    int             verbose;                /**< Output activity trace */
    MprMutex        *mutex;                 /**< Multi-thread sync */
} MprTestService;

/**
    Callback parser for non-standard command line arguments
    @ingroup MprTestService
 */
typedef int (*MprTestParser)(int argc, char **argv);

/**
    Create the test service
    @return An MprTestService control object
    @ingroup MprTestService
    @internal
 */
extern MprTestService *mprCreateTestService();

/**
    Parse test command arguments
    @param ts Test service object returned from #mprCreateTestService
    @param argc Count of arguments in argv
    @param argv Argument array
    @param extraParser Callback function to invoke to parse non-standard arguments.
    @return Zero if the command have been successfully parsed. Otherwise return a negative MPR error code.
    @ingroup MprTestService
 */
extern int mprParseTestArgs(MprTestService *ts, int argc, char **argv, MprTestParser extraParser);

/**
    Run the define unit tests
    @param ts Test service object returned from #mprCreateTestService
    @ingroup MprTestService
 */
extern int mprRunTests(MprTestService *ts);

/**
    Report the test results
    @description Test results are written to stdout.
    @param ts Test service object returned from #mprCreateTestService
    @ingroup MprTestService
 */
extern void mprReportTestResults(MprTestService *ts);

/**
    A test group is a group of tests to cover a unit of functionality. A test group may contain other test groups.
    @ingroup MprTestService
 */
typedef struct MprTestGroup {
    char            *name;                  /**< Name of test */
    char            *fullName;              /**< Fully qualified name of test */
    int             testDepth;              /**< Depth at which test should run */
    bool            skip;                   /**< Skip this test */
    bool            skipWarned;             /**< Warned that test will be skipped */
    bool            success;                /**< Result of last run */
    int             failedCount;            /**< Total failures of this test */
    int             testCount;              /**< Count of tests */
    int             testComplete;           /**< Test complete signal */
    MprList         *failures;              /**< List of all failures */

    MprTestService  *service;               /**< Reference to the service */
    MprDispatcher   *dispatcher;            /**< Per group thread dispatcher */
    struct MprTestGroup *parent;            /**< Parent test group */
    struct MprTestGroup *root;              /**< Top level test group parent */

    MprList         *groups;                /**< List of groups */
    MprList         *cases;                 /**< List of tests in this group */
    MprTestDef      *def;                   /**< Test definition ref */

    struct Http     *http;                  /**< Http service */
    struct HttpConn *conn;                  /**< Http connection for this group */
    char            *content;               /**< Cached response content */

    void            *data;                  /**< Test specific data */
    int             hasInternet;            /**< Convenience flag for internet available for use */
    int             hasIPv6;                /**< Convenience flag for IPv6 service */
    MprMutex        *mutex;                 /**< Multi-thread sync */
} MprTestGroup;


/**
    Add a test group to the test service
    @param ts Test service object returned from #mprCreateTestService
    @param def Test group definition to add
    @return MprTestGroup record for the test group 
    @ingroup MprTestService
 */
extern MprTestGroup *mprAddTestGroup(MprTestService *ts, MprTestDef *def);

/**
    Reset a test group.
    @description This resets the success/fail flag for the group
    @param gp Test group reference
    @ingroup MprTestService
 */
extern void mprResetTestGroup(MprTestGroup *gp);

/**
    Assert test
    @description This is the primary assertion test routine for unit tests. This is typically invoked by using the
        #assert macro. 
    @param gp Test group reference
    @param loc Program location string including filename and line number
    @param success Boolean indicating whether the test passed or failed.
    @param msg Message to display if the test failed.
    @ingroup MprTestService
 */
extern bool assertTrue(MprTestGroup *gp, cchar *loc, bool success, cchar *msg);

/**
    Signal a test is complete
    @description This awakens a thread that has called mprWaitForTestToComplete
    @param gp Test group reference
    @ingroup MprTestService
 */
extern void mprSignalTestComplete(MprTestGroup *gp);

/**
    Signal a test is complete.
    @description This awakens a thread that has called mprWaitForTest2ToComplete
    @param gp Test group reference
    @ingroup MprTestService
    @internal
 */
extern void mprSignalTest2Complete(MprTestGroup *gp);

/**
    Wait for a test to complete
    @description This blocks a thread until a test has completed and is signalled by
        another thread calling #mprSignalTestComplete.
    @param gp Test group reference
    @param timeout Timeout in milliseconds to block waiting for the test to complete
    @return True if the test was completed within the timeout
    @ingroup MprTestService
 */
extern bool mprWaitForTestToComplete(MprTestGroup *gp, MprTime timeout);

/**
    Wait for a test to complete
    @description This blocks a thread until a test has completed and is signalled by
        another thread calling #mprSignalTest2Complete.
    @param gp Test group reference
    @param timeout Timeout in milliseconds to block waiting for the test to complete
    @return True if the test was completed within the timeout
    @ingroup MprTestService
    @internal
 */
extern bool mprWaitForTest2ToComplete(MprTestGroup *gp, MprTime timeout);

/**
    Test failure record
    @ingroup MprTestService
    @internal
 */
typedef struct MprTestFailure {
    char            *loc;               /**< Program location of the failure */
    char            *message;           /**< Failure message typically the assertion program text */
} MprTestFailure;


#ifdef __cplusplus
}
#endif
#endif /* _h_MPR */

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
