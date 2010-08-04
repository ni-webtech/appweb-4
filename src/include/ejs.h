
/******************************************************************************/
/* 
 *  This file is an amalgamation of all the individual source code files for
 *  Embedthis Ejscript 2.0.0.
 *
 *  Catenating all the source into a single file makes embedding simpler and
 *  the resulting application faster, as many compilers can do whole file
 *  optimization.
 *
 *  If you want to modify ejs, you can still get the whole source
 *  as individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "../../src/include/ejsTune.h"
 */
/************************************************************************/

/*
    ejsTune.h - Tunable parameters for the C VM and compiler

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_TUNE
#define _h_EJS_TUNE 1


#include    "mpr.h"

#ifdef __cplusplus
extern "C" {
#endif


#define HEAP_OVERHEAD (MPR_ALLOC_HDR_SIZE + MPR_ALLOC_ALIGN(sizeof(MprRegion) + sizeof(MprHeap) + sizeof(MprDestructor)))

/*
    Tunable Constants
    TODO - consistency of names needs work
 */
#if BLD_TUNE == MPR_TUNE_SIZE || DOXYGEN
    /*
     *  Tune for size
     */
    #define EJS_ARENA_SIZE          ((1 * 1024 * 1024) - MPR_HEAP_OVERHEAD) /* Initial virt memory for objects */
    #define EJS_LOTSA_PROP          256             /* Object with lots of properties. Grow by bigger chunks */
    #define EJS_MAX_OBJ_POOL        512             /* Maximum number of objects per type to cache */
    #define EJS_MAX_RECURSION       10000           /* Maximum recursion */
    #define EJS_MAX_REGEX_MATCHES   32              /* Maximum regular sub-expressions */
    #define EJS_MAX_SQLITE_MEM      (2*1024*1024)   /* Maximum buffering for Sqlite */
    #define EJS_MAX_TYPE            256             /* Maximum number of types */
    #define EJS_MIN_FRAME_SLOTS     16              /* Miniumum number of slots for function frames */
    #define EJS_NUM_GLOBAL          256             /* Number of globals slots to pre-create */
    #define EJS_ROUND_PROP          16              /* Rounding for growing properties */
    #define EJS_WORK_QUOTA          1024            /* Allocations required before garbage colllection */
    #define EJS_XML_MAX_NODE_DEPTH  64              /* Max nesting of tags */

#elif BLD_TUNE == MPR_TUNE_BALANCED
    /*
        Tune balancing speed and size
     */
    #define EJS_ARENA_SIZE          ((4 * 1024 * 1024) - MPR_HEAP_OVERHEAD)
    #define EJS_LOTSA_PROP          256
    #define EJS_MAX_OBJ_POOL        1024
    #define EJS_MAX_SQLITE_MEM      (20*1024*1024)
    #define EJS_MAX_RECURSION       (1000000)
    #define EJS_MAX_REGEX_MATCHES   64
    #define EJS_MIN_FRAME_SLOTS     24
    #define EJS_NUM_GLOBAL          512
    #define EJS_MAX_TYPE            512
    #define EJS_ROUND_PROP          24
    #define EJS_WORK_QUOTA          2048
    #define EJS_XML_MAX_NODE_DEPTH  256

#else
    /*
        Tune for speed
     */
    #define EJS_ARENA_SIZE          ((8 * 1024 * 1024) - MPR_HEAP_OVERHEAD)
    #define EJS_NUM_GLOBAL          1024
    #define EJS_LOTSA_PROP          1024
    #define EJS_MAX_OBJ_POOL        4096
    #define EJS_MAX_RECURSION       (1000000)
    #define EJS_MAX_REGEX_MATCHES   128
    #define EJS_MAX_TYPE            1024
    #define EJS_MAX_SQLITE_MEM      (20*1024*1024)
    #define EJS_MIN_FRAME_SLOTS     32
    #define EJS_ROUND_PROP          32
    #define EJS_WORK_QUOTA          4096
    #define EJS_XML_MAX_NODE_DEPTH  1024
#endif

#define EJS_XML_BUF_MAX             (256 * 1024)    /* Max XML document size */
#define EJS_HASH_MIN_PROP           8               /* Min props to hash */

#define EJS_SQLITE_TIMEOUT          30000           /* Database busy timeout */
#define EJS_SESSION_TIMEOUT         1800
#define EJS_TIMER_PERIOD            1000            /* Timer checks ever 1 second */
#define EJS_FILE_PERMS              0664            /* Default file perms */
#define EJS_DIR_PERMS               0775            /* Default dir perms */


#if BLD_FEATURE_MMU
    #if BLD_TUNE == MPR_TUNE_SIZE
        #define EJS_STACK_MAX       (1024 * 1024)   /* Stack size on virtual memory systems */
    #elif BLD_TUNE == MPR_TUNE_BALANCED
        #define EJS_STACK_MAX       (1024 * 1024 * 4)
    #else
        #define EJS_STACK_MAX       (1024 * 1024 * 16)
    #endif
#else
    /*
        Highly recursive workloads may need to increase the stack values
     */
    #if BLD_TUNE == MPR_TUNE_SIZE
        #define EJS_STACK_MAX       (1024 * 32)     /* Stack size without MMU */
    #elif BLD_TUNE == MPR_TUNE_BALANCED
        #define EJS_STACK_MAX       (1024 * 64)
    #else
        #define EJS_STACK_MAX       (1024 * 128)
    #endif
#endif

/*
    Sanity constants. Only for sanity checking. Set large enough to never be a
    real limit but low enough to catch some errors in development.
 */
#define EJS_MAX_POOL            (4*1024*1024)   /* Size of constant pool */
#define EJS_MAX_ARGS            (8192)          /* Max number of args */
#define EJS_MAX_LOCALS          (10*1024)       /* Max number of locals */
#define EJS_MAX_EXCEPTIONS      (8192)          /* Max number of exceptions */
#define EJS_MAX_TRAITS          (0x7fff)        /* Max number of declared properties per block */

/*
    Should not need to change these
 */
#define EJS_INC_ARGS            8               /* Frame stack increment */
#define EJS_MAX_BASE_CLASSES    256             /* Max inheritance chain */
#define EJS_DOC_HASH_SIZE       1007            /* Hash for doc descriptions */

/*
    Compiler constants
 */
#define EC_MAX_INCLUDE          32              /* Max number of nested includes */
#define EC_LINE_INCR            1024            /* Growth increment for input lines */
#define EC_TOKEN_INCR           256             /* Growth increment for tokens */
#define EC_MAX_LOOK_AHEAD       8
#define EC_BUFSIZE              4096            /* General buffer size */
#define EC_MAX_ERRORS           25              /* Max compilation errors before giving up */

#define EC_CODE_BUFSIZE         4096            /* Initial size of code gen buffer */
#define EC_NUM_PAK_PROP         32              /* Initial number of properties */

/*
    File extensions
    MOB -- move these out of here 
 */
#define EJS_MODULE_EXT          ".mod"
#define EJS_SOURCE_EXT          ".es"
#define EJS_LISTING_EXT         ".lst"

#ifdef __cplusplus
}
#endif
#endif /* _h_EJS_TUNE */

/*
    @copy   

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
/************************************************************************/
/*
 *  End of file "../../src/include/ejsTune.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejsByteCode.h"
 */
/************************************************************************/

/*
    ejsByteCode.h - Ejscript VM Byte Code
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_ejsByteCode
#define _h_EJS_ejsByteCode 1


typedef enum EjsOpCode {
    EJS_OP_ADD,
    EJS_OP_ADD_NAMESPACE,
    EJS_OP_ADD_NAMESPACE_REF,
    EJS_OP_AND,
    EJS_OP_ATTENTION,
    EJS_OP_BRANCH_EQ,
    EJS_OP_BRANCH_STRICTLY_EQ,
    EJS_OP_BRANCH_FALSE,
    EJS_OP_BRANCH_GE,
    EJS_OP_BRANCH_GT,
    EJS_OP_BRANCH_LE,
    EJS_OP_BRANCH_LT,
    EJS_OP_BRANCH_NE,
    EJS_OP_BRANCH_STRICTLY_NE,
    EJS_OP_BRANCH_NULL,
    EJS_OP_BRANCH_NOT_ZERO,
    EJS_OP_BRANCH_TRUE,
    EJS_OP_BRANCH_UNDEFINED,
    EJS_OP_BRANCH_ZERO,
    EJS_OP_BRANCH_FALSE_8,
    EJS_OP_BRANCH_TRUE_8,
    EJS_OP_BREAKPOINT,
    EJS_OP_CALL,
    EJS_OP_CALL_GLOBAL_SLOT,
    EJS_OP_CALL_OBJ_SLOT,
    EJS_OP_CALL_THIS_SLOT,
    EJS_OP_CALL_BLOCK_SLOT,
    EJS_OP_CALL_OBJ_INSTANCE_SLOT,
    EJS_OP_CALL_OBJ_STATIC_SLOT,
    EJS_OP_CALL_THIS_STATIC_SLOT,
    EJS_OP_CALL_OBJ_NAME,
    EJS_OP_CALL_SCOPED_NAME,
    EJS_OP_CALL_CONSTRUCTOR,
    EJS_OP_CALL_NEXT_CONSTRUCTOR,
    EJS_OP_CAST,
    EJS_OP_CAST_BOOLEAN,
    EJS_OP_CLOSE_BLOCK,
    EJS_OP_COMPARE_EQ,
    EJS_OP_COMPARE_STRICTLY_EQ,
    EJS_OP_COMPARE_FALSE,
    EJS_OP_COMPARE_GE,
    EJS_OP_COMPARE_GT,
    EJS_OP_COMPARE_LE,
    EJS_OP_COMPARE_LT,
    EJS_OP_COMPARE_NE,
    EJS_OP_COMPARE_STRICTLY_NE,
    EJS_OP_COMPARE_NULL,
    EJS_OP_COMPARE_NOT_ZERO,
    EJS_OP_COMPARE_TRUE,
    EJS_OP_COMPARE_UNDEFINED,
    EJS_OP_COMPARE_ZERO,
    EJS_OP_DEBUG,
    EJS_OP_DEFINE_CLASS,
    EJS_OP_DEFINE_FUNCTION,
    EJS_OP_DELETE_NAME_EXPR,
    EJS_OP_DELETE_SCOPED_NAME_EXPR,
    EJS_OP_DIV,
    EJS_OP_DUP,
    EJS_OP_DUP2,
    EJS_OP_DUP_STACK,
    EJS_OP_END_CODE,
    EJS_OP_END_EXCEPTION,
    EJS_OP_GOTO,
    EJS_OP_GOTO_8,
    EJS_OP_INC,
    EJS_OP_INIT_DEFAULT_ARGS,
    EJS_OP_INIT_DEFAULT_ARGS_8,
    EJS_OP_INST_OF,
    EJS_OP_IS_A,
    EJS_OP_LOAD_0,
    EJS_OP_LOAD_1,
    EJS_OP_LOAD_2,
    EJS_OP_LOAD_3,
    EJS_OP_LOAD_4,
    EJS_OP_LOAD_5,
    EJS_OP_LOAD_6,
    EJS_OP_LOAD_7,
    EJS_OP_LOAD_8,
    EJS_OP_LOAD_9,
    EJS_OP_LOAD_DOUBLE,
    EJS_OP_LOAD_FALSE,
    EJS_OP_LOAD_GLOBAL,
    EJS_OP_LOAD_INT,
    EJS_OP_LOAD_M1,
    EJS_OP_LOAD_NAMESPACE,
    EJS_OP_LOAD_NULL,
    EJS_OP_LOAD_REGEXP,
    EJS_OP_LOAD_STRING,
    EJS_OP_LOAD_THIS,
    EJS_OP_LOAD_THIS_BASE,
    EJS_OP_LOAD_TRUE,
    EJS_OP_LOAD_UNDEFINED,
    EJS_OP_LOAD_XML,
    EJS_OP_GET_LOCAL_SLOT_0,
    EJS_OP_GET_LOCAL_SLOT_1,
    EJS_OP_GET_LOCAL_SLOT_2,
    EJS_OP_GET_LOCAL_SLOT_3,
    EJS_OP_GET_LOCAL_SLOT_4,
    EJS_OP_GET_LOCAL_SLOT_5,
    EJS_OP_GET_LOCAL_SLOT_6,
    EJS_OP_GET_LOCAL_SLOT_7,
    EJS_OP_GET_LOCAL_SLOT_8,
    EJS_OP_GET_LOCAL_SLOT_9,
    EJS_OP_GET_OBJ_SLOT_0,
    EJS_OP_GET_OBJ_SLOT_1,
    EJS_OP_GET_OBJ_SLOT_2,
    EJS_OP_GET_OBJ_SLOT_3,
    EJS_OP_GET_OBJ_SLOT_4,
    EJS_OP_GET_OBJ_SLOT_5,
    EJS_OP_GET_OBJ_SLOT_6,
    EJS_OP_GET_OBJ_SLOT_7,
    EJS_OP_GET_OBJ_SLOT_8,
    EJS_OP_GET_OBJ_SLOT_9,
    EJS_OP_GET_THIS_SLOT_0,
    EJS_OP_GET_THIS_SLOT_1,
    EJS_OP_GET_THIS_SLOT_2,
    EJS_OP_GET_THIS_SLOT_3,
    EJS_OP_GET_THIS_SLOT_4,
    EJS_OP_GET_THIS_SLOT_5,
    EJS_OP_GET_THIS_SLOT_6,
    EJS_OP_GET_THIS_SLOT_7,
    EJS_OP_GET_THIS_SLOT_8,
    EJS_OP_GET_THIS_SLOT_9,
    EJS_OP_GET_SCOPED_NAME,
    EJS_OP_GET_SCOPED_NAME_EXPR,
    EJS_OP_GET_OBJ_NAME,
    EJS_OP_GET_OBJ_NAME_EXPR,
    EJS_OP_GET_BLOCK_SLOT,
    EJS_OP_GET_GLOBAL_SLOT,
    EJS_OP_GET_LOCAL_SLOT,
    EJS_OP_GET_OBJ_SLOT,
    EJS_OP_GET_THIS_SLOT,
    EJS_OP_GET_TYPE_SLOT,
    EJS_OP_GET_THIS_TYPE_SLOT,
    EJS_OP_IN,
    EJS_OP_LIKE,
    EJS_OP_LOGICAL_NOT,
    EJS_OP_MUL,
    EJS_OP_NEG,
    EJS_OP_NEW,
    EJS_OP_NEW_ARRAY,
    EJS_OP_NEW_OBJECT,
    EJS_OP_NOP,
    EJS_OP_NOT,
    EJS_OP_OPEN_BLOCK,
    EJS_OP_OPEN_WITH,
    EJS_OP_OR,
    EJS_OP_POP,
    EJS_OP_POP_ITEMS,
    EJS_OP_PUSH_CATCH_ARG,
    EJS_OP_PUSH_RESULT,
    EJS_OP_PUT_LOCAL_SLOT_0,
    EJS_OP_PUT_LOCAL_SLOT_1,
    EJS_OP_PUT_LOCAL_SLOT_2,
    EJS_OP_PUT_LOCAL_SLOT_3,
    EJS_OP_PUT_LOCAL_SLOT_4,
    EJS_OP_PUT_LOCAL_SLOT_5,
    EJS_OP_PUT_LOCAL_SLOT_6,
    EJS_OP_PUT_LOCAL_SLOT_7,
    EJS_OP_PUT_LOCAL_SLOT_8,
    EJS_OP_PUT_LOCAL_SLOT_9,
    EJS_OP_PUT_OBJ_SLOT_0,
    EJS_OP_PUT_OBJ_SLOT_1,
    EJS_OP_PUT_OBJ_SLOT_2,
    EJS_OP_PUT_OBJ_SLOT_3,
    EJS_OP_PUT_OBJ_SLOT_4,
    EJS_OP_PUT_OBJ_SLOT_5,
    EJS_OP_PUT_OBJ_SLOT_6,
    EJS_OP_PUT_OBJ_SLOT_7,
    EJS_OP_PUT_OBJ_SLOT_8,
    EJS_OP_PUT_OBJ_SLOT_9,
    EJS_OP_PUT_THIS_SLOT_0,
    EJS_OP_PUT_THIS_SLOT_1,
    EJS_OP_PUT_THIS_SLOT_2,
    EJS_OP_PUT_THIS_SLOT_3,
    EJS_OP_PUT_THIS_SLOT_4,
    EJS_OP_PUT_THIS_SLOT_5,
    EJS_OP_PUT_THIS_SLOT_6,
    EJS_OP_PUT_THIS_SLOT_7,
    EJS_OP_PUT_THIS_SLOT_8,
    EJS_OP_PUT_THIS_SLOT_9,
    EJS_OP_PUT_OBJ_NAME_EXPR,
    EJS_OP_PUT_OBJ_NAME,
    EJS_OP_PUT_SCOPED_NAME,
    EJS_OP_PUT_SCOPED_NAME_EXPR,
    EJS_OP_PUT_BLOCK_SLOT,
    EJS_OP_PUT_GLOBAL_SLOT,
    EJS_OP_PUT_LOCAL_SLOT,
    EJS_OP_PUT_OBJ_SLOT,
    EJS_OP_PUT_THIS_SLOT,
    EJS_OP_PUT_TYPE_SLOT,
    EJS_OP_PUT_THIS_TYPE_SLOT,
    EJS_OP_REM,
    EJS_OP_RETURN,
    EJS_OP_RETURN_VALUE,
    EJS_OP_SAVE_RESULT,
    EJS_OP_SHL,
    EJS_OP_SHR,
    EJS_OP_SPREAD,
    EJS_OP_SUB,
    EJS_OP_SUPER,
    EJS_OP_SWAP,
    EJS_OP_THROW,
    EJS_OP_TYPE_OF,
    EJS_OP_USHR,
    EJS_OP_XOR,
    EJS_OP_FINALLY,
} EjsOpCode;

#endif

/*
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
 */
/************************************************************************/
/*
 *  End of file "../../src/include/ejsByteCode.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejsByteCodeTable.h"
 */
/************************************************************************/

/**
    ejsByteCodeTable.h - Master Byte Code Table
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_BYTECODETABLE_H
#define _h_EJS_BYTECODETABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
    Stack effect special values
 */
#define EBC_POPN            101             /* Operand 1 specifies the stack change (pop) */

/*
    Operands
 */
#define EBC_NONE            0x0             /* No operands */
#define EBC_BYTE            0x1             /* 8 bit integer */
#define EBC_DOUBLE          0x10            /* 64 bit floating */
#define EBC_NUM             0x40            /* Encoded integer */
#define EBC_STRING          0x80            /* Interned string as an encoded integer*/
#define EBC_GLOBAL          0x100           /* Encode global */
#define EBC_SLOT            0x200           /* Slot number as an encoded integer */
#define EBC_JMP             0x1000          /* 32 bit jump offset */
#define EBC_JMP8            0x2000          /* 8 bit jump offset */
#define EBC_INIT_DEFAULT    0x8000          /* Computed goto table, 32 bit jumps  */
#define EBC_INIT_DEFAULT8   0x10000         /* Computed goto table, 8 bit jumps */
#define EBC_ARGC            0x20000         /* Argument count */
#define EBC_ARGC2           0x40000         /* Argument count * 2 */
#define EBC_ARGC3           0x80000         /* Argument count * 3 */
#define EBC_NEW_ARRAY       0x100000        /* New Array: Argument count * 2, byte code */
#define EBC_NEW_OBJECT      0x200000        /* New Object: Argument count * 3, byte code: attributes * 3 */

typedef struct EjsOptable {
    char    *name;
    int     stackEffect;
    int     args[8];
} EjsOptable;

#if EJS_DEFINE_OPTABLE
/*  
        Opcode string         Stack Effect      Operands, ...                                   
 */      
EjsOptable ejsOptable[] = {
    {   "ADD",                      -1,         { EBC_NONE,                               },},
    {   "ADD_NAMESPACE",             0,         { EBC_STRING,                             },},
    {   "ADD_NAMESPACE_REF",        -1,         { EBC_NONE,                               },},
    {   "AND",                      -1,         { EBC_NONE,                               },},
    {   "ATTENTION",                -1,         { EBC_NONE,                               },},
    {   "BRANCH_EQ",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_STRICTLY_EQ",       -1,         { EBC_JMP,                                },},
    {   "BRANCH_FALSE",             -1,         { EBC_JMP,                                },},
    {   "BRANCH_GE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_GT",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_LE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_LT",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_NE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_STRICTLY_NE",       -1,         { EBC_JMP,                                },},
    {   "BRANCH_NULL",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_NOT_ZERO",          -1,         { EBC_JMP,                                },},
    {   "BRANCH_TRUE",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_UNDEFINED",         -1,         { EBC_JMP,                                },},
    {   "BRANCH_ZERO",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_FALSE_8",           -1,         { EBC_JMP8,                               },},
    {   "BRANCH_TRUE_8",            -1,         { EBC_JMP8,                               },},
    {   "BREAKPOINT",                0,         { EBC_NUM, EBC_STRING,                    },},
    {   "CALL",                     -2,         { EBC_ARGC,                               },},
    {   "CALL_GLOBAL_SLOT",          0,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_OBJ_SLOT",            -1,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_THIS_SLOT",            0,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_BLOCK_SLOT",           0,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_OBJ_INSTANCE_SLOT",   -1,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_OBJ_STATIC_SLOT",     -1,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_THIS_STATIC_SLOT",     0,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_OBJ_NAME",            -1,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CALL_SCOPED_NAME",          0,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CALL_CONSTRUCTOR",          0,         { EBC_ARGC,                               },},
    {   "CALL_NEXT_CONSTRUCTOR",     0,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CAST",                     -1,         { EBC_NONE,                               },},
    {   "CAST_BOOLEAN",              0,         { EBC_NONE,                               },},
    {   "CLOSE_BLOCK",               0,         { EBC_NONE,                               },},
    {   "COMPARE_EQ",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_STRICTLY_EQ",      -1,         { EBC_NONE,                               },},
    {   "COMPARE_FALSE",            -1,         { EBC_NONE,                               },},
    {   "COMPARE_GE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_GT",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_LE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_LT",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_NE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_STRICTLY_NE",      -1,         { EBC_NONE,                               },},
    {   "COMPARE_NULL",             -1,         { EBC_NONE,                               },},
    {   "COMPARE_NOT_ZERO",         -1,         { EBC_NONE,                               },},
    {   "COMPARE_TRUE",             -1,         { EBC_NONE,                               },},
    {   "COMPARE_UNDEFINED",        -1,         { EBC_NONE,                               },},
    {   "COMPARE_ZERO",             -1,         { EBC_NONE,                               },},
    {   "DEBUG",                     0,         { EBC_NUM, EBC_STRING,                    },},
    {   "DEFINE_CLASS",              0,         { EBC_GLOBAL,                             },},
    {   "DEFINE_FUNCTION",           0,         { EBC_STRING, EBC_STRING,                 },},
    {   "DELETE_NAME_EXPR",         -2,         { EBC_NONE,                               },},
    {   "DELETE_SCOPED_NAME_EXPR",  -1,         { EBC_NONE,                               },},
    {   "DIV",                      -1,         { EBC_NONE,                               },},
    {   "DUP",                       1,         { EBC_NONE,                               },},
    {   "DUP2",                      2,         { EBC_NONE,                               },},
    {   "DUP_STACK",                 1,         { EBC_BYTE,                               },},
    {   "END_CODE",                  0,         { EBC_NONE,                               },},
    {   "END_EXCEPTION",             0,         { EBC_NONE,                               },},
    {   "GOTO",                      0,         { EBC_JMP,                                },},
    {   "GOTO_8",                    0,         { EBC_JMP8,                               },},
    {   "INC",                       0,         { EBC_BYTE,                               },},
    {   "INIT_DEFAULT_ARGS",         0,         { EBC_INIT_DEFAULT,                       },},
    {   "INIT_DEFAULT_ARGS_8",       0,         { EBC_INIT_DEFAULT8,                      },},
    {   "INST_OF",                  -1,         { EBC_NONE,                               },},
    {   "IS_A",                     -1,         { EBC_NONE,                               },},
    {   "LOAD_0",                    1,         { EBC_NONE,                               },},
    {   "LOAD_1",                    1,         { EBC_NONE,                               },},
    {   "LOAD_2",                    1,         { EBC_NONE,                               },},
    {   "LOAD_3",                    1,         { EBC_NONE,                               },},
    {   "LOAD_4",                    1,         { EBC_NONE,                               },},
    {   "LOAD_5",                    1,         { EBC_NONE,                               },},
    {   "LOAD_6",                    1,         { EBC_NONE,                               },},
    {   "LOAD_7",                    1,         { EBC_NONE,                               },},
    {   "LOAD_8",                    1,         { EBC_NONE,                               },},
    {   "LOAD_9",                    1,         { EBC_NONE,                               },},
    {   "LOAD_DOUBLE",               1,         { EBC_DOUBLE,                             },},
    {   "LOAD_FALSE",                1,         { EBC_NONE,                               },},
    {   "LOAD_GLOBAL",               1,         { EBC_NONE,                               },},
    {   "LOAD_INT",                  1,         { EBC_NUM,                                },},
    {   "LOAD_M1",                   1,         { EBC_NONE,                               },},
    {   "LOAD_NAMESPACE",            1,         { EBC_STRING,                             },},
    {   "LOAD_NULL",                 1,         { EBC_NONE,                               },},
    {   "LOAD_REGEXP",               1,         { EBC_STRING,                             },},
    {   "LOAD_STRING",               1,         { EBC_STRING,                             },},
    {   "LOAD_THIS",                 1,         { EBC_NONE,                               },},
    {   "LOAD_THIS_BASE",            1,         { EBC_NUM,                                },},
    {   "LOAD_TRUE",                 1,         { EBC_NONE,                               },},
    {   "LOAD_UNDEFINED",            1,         { EBC_NONE,                               },},
    {   "LOAD_XML",                  1,         { EBC_STRING,                             },},
    {   "GET_LOCAL_SLOT_0",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_1",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_2",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_3",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_4",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_5",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_6",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_7",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_8",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_9",          1,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_0",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_1",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_2",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_3",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_4",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_5",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_6",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_7",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_8",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_9",            0,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_0",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_1",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_2",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_3",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_4",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_5",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_6",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_7",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_8",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_9",           1,         { EBC_NONE,                               },},
    {   "GET_SCOPED_NAME",           1,         { EBC_STRING, EBC_STRING,                 },},
    {   "GET_SCOPED_NAME_EXPR",      -1,        { EBC_NONE,                               },},
    {   "GET_OBJ_NAME",              0,         { EBC_STRING, EBC_STRING,                 },},
    {   "GET_OBJ_NAME_EXPR",        -2,         { EBC_NONE,                               },},
    {   "GET_BLOCK_SLOT",            1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "GET_GLOBAL_SLOT",           1,         { EBC_SLOT,                               },},
    {   "GET_LOCAL_SLOT",            1,         { EBC_SLOT,                               },},
    {   "GET_OBJ_SLOT",              0,         { EBC_SLOT,                               },},
    {   "GET_THIS_SLOT",             1,         { EBC_SLOT,                               },},
    {   "GET_TYPE_SLOT",             0,         { EBC_SLOT, EBC_NUM,                      },},
    {   "GET_THIS_TYPE_SLOT",        1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "IN",                       -1,         { EBC_NONE,                               },},
    {   "LIKE",                     -1,         { EBC_NONE,                               },},
    {   "LOGICAL_NOT",               0,         { EBC_NONE,                               },},
    {   "MUL",                      -1,         { EBC_NONE,                               },},
    {   "NEG",                       0,         { EBC_NONE,                               },},
    {   "NEW",                       0,         { EBC_NONE,                               },},
    {   "NEW_ARRAY",                 1,         { EBC_GLOBAL, EBC_NEW_ARRAY,              },},
    {   "NEW_OBJECT",                1,         { EBC_GLOBAL, EBC_NEW_OBJECT,             },},
    {   "NOP",                       0,         { EBC_NONE,                               },},
    {   "NOT",                       0,         { EBC_NONE,                               },},
    {   "OPEN_BLOCK",                0,         { EBC_SLOT, EBC_NUM,                      },},
    {   "OPEN_WITH",                 1,         { EBC_NONE,                               },},
    {   "OR",                       -1,         { EBC_NONE,                               },},
    {   "POP",                      -1,         { EBC_NONE,                               },},
    {   "POP_ITEMS",          EBC_POPN,         { EBC_BYTE,                               },},
    {   "PUSH_CATCH_ARG",            1,         { EBC_NONE,                               },},
    {   "PUSH_RESULT",               1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_0",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_1",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_2",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_3",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_4",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_5",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_6",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_7",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_8",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_9",         -1,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_0",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_1",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_2",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_3",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_4",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_5",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_6",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_7",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_8",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_9",           -2,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_0",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_1",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_2",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_3",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_4",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_5",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_6",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_7",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_8",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_9",          -1,         { EBC_NONE,                               },},
    {   "PUT_OBJ_NAME_EXPR",        -4,         { EBC_NONE,                               },},
    {   "PUT_OBJ_NAME",             -2,         { EBC_STRING, EBC_STRING,                 },},
    {   "PUT_SCOPED_NAME",          -1,         { EBC_STRING, EBC_STRING,                 },},
    {   "PUT_SCOPED_NAME_EXPR",     -3,         { EBC_NONE,                               },},
    {   "PUT_BLOCK_SLOT",           -1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "PUT_GLOBAL_SLOT",          -1,         { EBC_SLOT,                               },},
    {   "PUT_LOCAL_SLOT",           -1,         { EBC_SLOT,                               },},
    {   "PUT_OBJ_SLOT",             -2,         { EBC_SLOT,                               },},
    {   "PUT_THIS_SLOT",            -1,         { EBC_SLOT,                               },},
    {   "PUT_TYPE_SLOT",            -2,         { EBC_SLOT, EBC_NUM,                      },},
    {   "PUT_THIS_TYPE_SLOT",       -1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "REM",                      -1,         { EBC_NONE,                               },},
    {   "RETURN",                    0,         { EBC_NONE,                               },},
    {   "RETURN_VALUE",             -1,         { EBC_NONE,                               },},
    {   "SAVE_RESULT",              -1,         { EBC_NONE,                               },},
    {   "SHL",                      -1,         { EBC_NONE,                               },},
    {   "SHR",                      -1,         { EBC_NONE,                               },},
    {   "SPREAD",                    0,         { EBC_NONE,                               },},
    {   "SUB",                      -1,         { EBC_NONE,                               },},
    {   "SUPER",                     0,         { EBC_NONE,                               },},
    {   "SWAP",                      0,         { EBC_NONE,                               },},
    {   "THROW",                     0,         { EBC_NONE,                               },},
    {   "TYPE_OF",                  -1,         { EBC_NONE,                               },},
    {   "USHR",                     -1,         { EBC_NONE,                               },},
    {   "XOR",                      -1,         { EBC_NONE,                               },},
    {   "FINALLY",                   0,         { EBC_NONE,                               },},
    {   NULL,                        0,         { EBC_NONE,                               },},
};
#endif /* EJS_DEFINE_OPTABLE */

extern EjsOptable *ejsGetOptable(MprCtx ctx);

#ifdef __cplusplus
}
#endif

#endif /* _h_EJS_BYTECODETABLE_H */

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

/************************************************************************/
/*
 *  End of file "../../src/include/ejsByteCodeTable.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejsCore.h"
 */
/************************************************************************/

/*
    ejsCore.h - Header for the core types.

    The VM provides core types like numbers, strings and objects. This header provides their API.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#ifndef _h_EJS_CORE
#define _h_EJS_CORE 1

#include    "mpr.h"
#include    "http.h"

#ifdef __cplusplus
extern "C" {
#endif


#if !DOXYGEN
/*
    Forward declare types
 */
struct Ejs;
struct EjsBlock;
struct EjsFrame;
struct EjsFunction;
struct EjsGC;
struct EjsList;
struct EjsNames;
struct EjsModule;
struct EjsNamespace;
struct EjsObj;
struct EjsService;
struct EjsString;
struct EjsState;
struct EjsTrait;
struct EjsTraits;
struct EjsType;
struct EjsTypeHelpers;
struct EjsUri;
struct EjsWorker;
struct EjsXML;
#endif

//  MOB -- move
/*
    Default GC thresholds (not tunable)
 */
#define EJS_MIN_TIME_FOR_GC     300     /* Need 1/3 sec for GC */
#define EJS_SHORT_WORK_QUOTA    50      /* Predict GC short of a full work quota */
    
/*
    GC Object generations
 */
#define EJS_GEN_NEW         0           /* New objects */
#define EJS_GEN_ETERNAL     1           /* Builtin objects that live forever */
#define EJS_MAX_GEN         2           /* Number of generations for object allocation */

/*
    GC Collection modes
 */
#define EJS_GC_ETERNAL      1           /* Collect eternal generation */

/*
    Trait, type, function and property attributes. These are sometimes combined into a single attributes word.
 */
#define EJS_TRAIT_BUILTIN               0x1         /**< Property can take a null value */
#define EJS_TRAIT_CAST_NULLS            0x2         /**< Property casts nulls */
#define EJS_TRAIT_DELETED               0x4         /**< Property has been deleted */
#define EJS_TRAIT_GETTER                0x8         /**< Property is a getter */
#define EJS_TRAIT_FIXED                 0x10        /**< Property is not configurable */
#define EJS_TRAIT_HIDDEN                0x20        /**< !Enumerable */
#define EJS_TRAIT_INITIALIZED           0x40        /**< Readonly property has been initialized */
#define EJS_TRAIT_READONLY              0x80        /**< !Writable */
#define EJS_TRAIT_SETTER                0x100       /**< Property is a settter */
#define EJS_TRAIT_THROW_NULLS           0x200       /**< Property rejects null */

#define EJS_PROP_HAS_VALUE              0x400       /**< Property has a value record */
#define EJS_PROP_NATIVE                 0x800       /**< Property is backed by native code */
#define EJS_PROP_STATIC                 0x1000      /**< Class static property */
#define EJS_PROP_SHARED                 0x2000      /**< Share static method with all subclasses */
#define EJS_PROP_ENUMERABLE             0x4000      /**< Property will be enumerable (compiler use only) */

#define EJS_FUN_CONSTRUCTOR             0x8000      /**< Method is a constructor */
#define EJS_FUN_FULL_SCOPE              0x10000     /**< Function needs closure when defined */
#define EJS_FUN_HAS_RETURN              0x20000     /**< Function has a return statement */
#define EJS_FUN_INITIALIZER             0x40000     /**< Type initializer code */
#define EJS_FUN_OVERRIDE                0x80000     /**< Override base type */
#define EJS_FUN_MODULE_INITIALIZER      0x100000    /**< Module initializer */
#define EJS_FUN_REST_ARGS               0x200000    /**< Parameter is a "..." rest */
#define EJS_TRAIT_MASK                  0x3FFFFF    /**< Mask of trait attributes */

/*
    These attributes are never stored in EjsTrait but are often passed in "attributes"
 */
#define EJS_TYPE_CALLS_SUPER            0x400000    /**< Constructor calls super() */
#define EJS_TYPE_HAS_INSTANCE_VARS      0x800000    /**< Type has non-method instance vars (state) */
#define EJS_TYPE_DYNAMIC_INSTANCE       0x1000000   /**< Instances are not sealed */
#define EJS_TYPE_FINAL                  0x2000000   /**< Type can't be subclassed */
#define EJS_TYPE_FIXUP                  0x4000000   /**< Type needs to inherit base types properties */
#define EJS_TYPE_HAS_CONSTRUCTOR        0x8000000   /**< Type has a constructor */
#define EJS_TYPE_HAS_TYPE_INITIALIZER   0x10000000  /**< Type has an initializer */
#define EJS_TYPE_IMMUTABLE              0x20000000  /**< Instances are immutable */
#define EJS_TYPE_INTERFACE              0x40000000  /**< Class is an interface */

/*
    Interpreter flags
 */
#define EJS_FLAG_EVENT          0x1         /**< Event pending */
#define EJS_FLAG_NO_INIT        0x8         /**< Don't initialize any modules*/
#define EJS_FLAG_MASTER         0x20        /**< Create a master interpreter */
#define EJS_FLAG_DOC            0x40        /**< Load documentation from modules */
#define EJS_FLAG_NOEXIT         0x200       /**< App should service events and not exit */
#define EJS_FLAG_DYNAMIC        0x400       /**< Make a type that is dynamic itself */
#define EJS_STACK_ARG           -1          /* Offset to locate first arg */

/** 
    Configured numeric type
 */
#define BLD_FEATURE_NUM_TYPE double
typedef BLD_FEATURE_NUM_TYPE MprNumber;

/*  
    Sizes (in bytes) of encoded types in a ByteArray
 */
#define EJS_SIZE_BOOLEAN        1
#define EJS_SIZE_SHORT          2
#define EJS_SIZE_INT            4
#define EJS_SIZE_LONG           8
#define EJS_SIZE_DOUBLE         8
#define EJS_SIZE_DATE           8

/*  
    Reserved and system Namespaces
    The empty namespace is special. When seaching for properties, the empty namespace implies to search all namespaces.
    When properties are defined without a namespace, they are defined in the the empty namespace.
 */
#define EJS_EMPTY_NAMESPACE         ""
#define EJS_BLOCK_NAMESPACE         "-block-"
#define EJS_CONSTRUCTOR_NAMESPACE   "-constructor-"
#define EJS_EJS_NAMESPACE           "ejs"
#define EJS_ITERATOR_NAMESPACE      "iterator"
#define EJS_INIT_NAMESPACE          "-initializer-"
#define EJS_INTERNAL_NAMESPACE      "internal"
#define EJS_META_NAMESPACE          "meta"
#define EJS_PRIVATE_NAMESPACE       "private"
#define EJS_PROTECTED_NAMESPACE     "protected"
#define EJS_PROTOTYPE_NAMESPACE     "-prototype-"
#define EJS_PUBLIC_NAMESPACE        "public"
#define EJS_WORKER_NAMESPACE        "ejs.worker"

/*  
    Flags for fast comparison of namespaces
 */
#define EJS_NSP_PRIVATE         0x1
#define EJS_NSP_PROTECTED       0x2


/*  
    When allocating slots, name hashes and traits, we optimize by rounding up allocations
 */
#define EJS_PROP_ROUNDUP(x) (((x) + EJS_ROUND_PROP - 1) / EJS_ROUND_PROP * EJS_ROUND_PROP)

/*  Property enumeration flags
 */
#define EJS_FLAGS_ENUM_INHERITED 0x1            /**< Enumerate inherited base classes */
#define EJS_FLAGS_ENUM_ALL      0x2             /**< Enumerate non-enumerable and fixture properties */

/*  
    Exception flags and structure
 */
#define EJS_EX_CATCH            0x1             /* Definition is a catch block */
#define EJS_EX_FINALLY          0x2             /* Definition is a finally block */
#define EJS_EX_ITERATION        0x4             /* Definition is an iteration catch block */
#define EJS_EX_INC              4               /* Growth increment for exception handlers */

/*  
    Ejscript return codes.
 */
#define EJS_SUCCESS             MPR_ERR_OK
#define EJS_ERR                 MPR_ERR_GENERAL
#define EJS_EXCEPTION           (MPR_ERR_MAX - 1)

/*  
    Xml defines
 */
#define E4X_MAX_ELT_SIZE        (E4X_BUF_MAX-1)
#define E4X_TEXT_PROPERTY       "-txt"
#define E4X_TAG_NAME_PROPERTY   "-tag"
#define E4X_COMMENT_PROPERTY    "-com"
#define E4X_ATTRIBUTES_PROPERTY "-att"
#define E4X_PI_PROPERTY         "-pi"
#define E4X_PARENT_PROPERTY     "-parent"

#define EJS_XML_FLAGS_TEXT      0x1             /* Node is a text node */
#define EJS_XML_FLAGS_PI        0x2             /* Node is a processing instruction */
#define EJS_XML_FLAGS_COMMENT   0x4             /* Node is a comment */
#define EJS_XML_FLAGS_ATTRIBUTE 0x8             /* Node is an attribute */
#define EJS_XML_FLAGS_ELEMENT   0x10            /* Node is an element */

/*  
    XML node kinds
 */
#define EJS_XML_LIST        1
#define EJS_XML_ELEMENT     2
#define EJS_XML_ATTRIBUTE   3
#define EJS_XML_TEXT        4
#define EJS_XML_COMMENT     5
#define EJS_XML_PROCESSING  6

/*  
    Convenient slot aliases
 */
#define EJSLOT_CONSTRUCTOR          EJSLOT_Object___constructor__

/*  
    Default names
 */
#define EJS_GLOBAL                  "global"
#define EJS_DEFAULT_MODULE          "default"
#define EJS_DEFAULT_MODULE_NAME     EJS_DEFAULT_MODULE EJS_MODULE_EXT
#define EJS_BUILTIN_MODULE_NAME     "ejs"  EJS_MODULE_EXT
#define EJS_DEFAULT_CLASS_NAME      "__defaultClass__"
#define EJS_INITIALIZER_NAME        "__initializer__"

#define EJS_NAME                    "ejs"
#define EJS_MOD                     "ejs.mod"

/*  
    Compare if a variable is an instance or sub-type of a given type described by the type's global slot.
 */
#define ejsIs(vp, slot)             _ejsIs((EjsObj*) vp, slot)
extern int _ejsIs(struct EjsObj *vp, int slot);

/* TODO - merge with MprList */
/**
    List type
    @description    The MprList is a dynamic growable array suitable for storing pointers to arbitrary objects.
    @stability      Prototype.
    @see            EjsList mprCreateList mprFree MprBuf
 */
typedef struct EjsList {
    void    **items;                    /* List item data */
    int     length;                     /* Count of used items */
    int     maxSize;                    /* Maximum capacity */
} EjsList;

//  TODO - DOC
extern void     ejsInitList(EjsList *list);
extern int      ejsAddItem(MprCtx ctx, EjsList *list, cvoid *item);
extern int      ejsAddItemToSharedList(MprCtx ctx, EjsList *list, cvoid *item);
extern EjsList  *ejsAppendList(MprCtx ctx, EjsList *list, EjsList *add);
extern int      ejsCopyList(MprCtx ctx, EjsList *dest, EjsList *src);
extern void     ejsClearList(EjsList *lp);
extern void     *ejsGetItem(EjsList *lp, int index);
extern void     *ejsGetLastItem(EjsList *lp);
extern int      ejsGetListCount(EjsList *lp);
extern void     *ejsGetNextItem(EjsList *lp, int *lastIndex);
extern void     *ejsGetPrevItem(EjsList *lp, int *lastIndex);
extern int      ejsLookupItem(EjsList *lp, cvoid *item);
extern int      ejsRemoveItemAtPos(EjsList *lp, int index);
extern int      ejsRemoveLastItem(EjsList *lp);
extern int      ejsSetListDetails(MprCtx ctx, EjsList *list, int initialSize, int maxSize);

/*
    GC Per generation structure
 */
typedef struct EjsGen
{
    uint            totalReclaimed;     /* Total blocks reclaimed on sweeps */
    uint            totalSweeps;        /* Total sweeps */
} EjsGen;

/*
    GC Pool of free objects of a given type. Each type maintains a free pool for faster allocations.
    Types in the pool have a weak reference and may be reclaimed.
 */
typedef struct EjsPool
{
    struct EjsType  *type;              /* Owning type */
    int             allocated;          /* Count of instances created */
    int             peakAllocated;      /* High water mark for allocated */
    int             count;              /* Count in pool */
    int             peakCount;          /* High water mark for count */
    int             reuse;              /* Count of reuses */
} EjsPool;


/*
    Garbage collector control
 */
typedef struct EjsGC {

    EjsGen      *generations[EJS_MAX_GEN];

    //  MOB -- make this dynamic and remove the constant
    EjsPool     *pools[EJS_MAX_TYPE];   /* Object pools */
    int         numPools;               /* Count of object pools */
    uint        allocGeneration;        /* Current generation accepting objects */
    uint        collectGeneration;      /* Current generation doing GC */
    uint        markGenRef;             /* Generation to mark objects */
    uint        firstGlobal;            /* First global slots to examine */
    bool        collecting;             /* Running garbage collection */
    bool        enabled;                /* GC is enabled */
    int         degraded;               /* Have exceeded redlineMemory */
    uint        allocatedTypes;         /* Count of types allocated */
    uint        peakAllocatedTypes;     /* Peak allocated types */ 
    uint        allocatedObjects;       /* Count of objects allocated */
    uint        peakAllocatedObjects;   /* Peak allocated */ 
    uint        peakMemory;             /* Peak memory usage */
    uint        totalAllocated;         /* Total count of allocation calls */
    uint        totalReclaimed;         /* Total blocks reclaimed on sweeps */
    uint        totalOverflows;         /* Total overflows  */
    uint        totalRedlines;          /* Total times redline limit exceeded */
    uint        totalSweeps;            /* Total sweeps */
#if BLD_DEBUG
    int         indent;                 /* Indent formatting in GC reports */
#endif
} EjsGC;


typedef struct EjsLoadState {
    MprList     *typeFixups;        /**< Loaded types to fixup */
    int         firstModule;        /**< First module in ejs->modules for this load */
    int         flags;              /**< Module load flags */
} EjsLoadState;

typedef void (*EjsLoaderCallback)(struct Ejs *ejs, int kind, ...);

/**
    Ejsript Interperter Structure
    @description The Ejs structure contains the state for a single interpreter. The #ejsCreate routine may be used
        to create multiple interpreters and returns a reference to be used in subsequent Ejscript API calls.
    @stability Prototype.
    @defgroup Ejs Ejs
    @see ejsCreate, ejsCreateService, ejsAppendSearchPath, ejsSetSearchPath, ejsEvalFile, ejsEvalScript, ejsExit
 */
typedef struct Ejs {
    struct EjsObj       *exception;         /**< Pointer to exception object */
    struct EjsObj       *result;            /**< Last expression result */
    struct EjsState     *state;             /**< Current evaluation state and stack */
    struct EjsState     *masterState;       /**< Owns the eval stack */

    struct EjsService   *service;           /**< Back pointer to the service */
    struct Ejs          *master;            /**< Inherit builtin types from the master */
    struct EjsGC        gc;                 /**< Garbage collector state */
    EjsGen              *currentGeneration; /**< Current allocation generation */
    MprHeap             *heap;              /**< Allocation heap */
    cchar               *bootSearch;        /**< Module search when bootstrapping the VM */
    struct EjsArray     *search;            /**< Module load search path */
    cchar               *className;         /**< Name of a specific class to run for a program */
    cchar               *methodName;        /**< Name of a specific method to run for a program */

    /*
        Essential types
     */
    struct EjsType      *appType;           /**< App type */
    struct EjsType      *arrayType;         /**< Array type */
    struct EjsType      *blockType;         /**< Block type */
    struct EjsType      *booleanType;       /**< Boolean type */
    struct EjsType      *byteArrayType;     /**< ByteArray type */
    struct EjsType      *dateType;          /**< Date type */
    struct EjsType      *errorType;         /**< Error type */
    struct EjsType      *errorEventType;    /**< ErrorEvent type */
    struct EjsType      *eventType;         /**< Event type */
    struct EjsType      *frameType;         /**< Frame type */
    struct EjsType      *fileType;          /**< File type */
    struct EjsType      *functionType;      /**< Function type */
    struct EjsType      *iteratorType;      /**< Iterator type */
    struct EjsType      *mathType;          /**< Math type */
    struct EjsType      *namespaceType;     /**< Namespace type */
    struct EjsType      *nullType;          /**< Null type */
    struct EjsType      *numberType;        /**< Default numeric type */
    struct EjsType      *objectType;        /**< Object type */
    struct EjsType      *pathType;          /**< Path type */
    struct EjsType      *regExpType;        /**< RegExp type */
    struct EjsType      *requestType;       /**< Request type */
    struct EjsType      *stringType;        /**< String type */
    struct EjsType      *stopIterationType; /**< StopIteration type */
    struct EjsType      *typeType;          /**< Type type */
    struct EjsType      *uriType;           /**< URI type */
    struct EjsType      *voidType;          /**< Void type */
    struct EjsType      *webType;           /**< Web type */
    struct EjsType      *workerType;        /**< Worker type */
    struct EjsType      *xmlType;           /**< XML type */
    struct EjsType      *xmlListType;       /**< XMLList type */

    /*
        Key values
     */
    struct EjsObj       *global;            /**< The "global" object as an EjsObj */
    struct EjsBlock     *globalBlock;       /**< The "global" object as an EjsBlock */

    struct EjsString    *emptyStringValue;  /**< "" value */
    struct EjsObj       *falseValue;        /**< The "false" value */
    struct EjsNumber    *infinityValue;     /**< The infinity number value */
    struct EjsIterator  *iterator;          /**< Default iterator */
    struct EjsNumber    *maxValue;          /**< Maximum number value */
    struct EjsNumber    *minValue;          /**< Minimum number value */
    struct EjsNumber    *minusOneValue;     /**< The -1 number value */
    struct EjsNumber    *nanValue;          /**< The "NaN" value if floating point numbers, else zero */
    struct EjsNumber    *negativeInfinityValue; /**< The negative infinity number value */
    struct EjsFunction  *nopFunction;       /**< The NOP function */
    struct EjsObj       *nullValue;         /**< The "null" value */
    struct EjsNumber    *oneValue;          /**< The 1 number value */
    struct EjsObj       *trueValue;         /**< The "true" value */
    struct EjsObj       *undefinedValue;    /**< The "void" value */
    struct EjsNumber    *zeroValue;         /**< The 0 number value */
    struct EjsFunction  *memoryCallback;    /**< Memory.readline callback */

    struct EjsNamespace *emptySpace;        /**< Empty namespace */
    struct EjsNamespace *ejsSpace;          /**< Ejs namespace */
    struct EjsNamespace *iteratorSpace;     /**< Iterator namespace */
    struct EjsNamespace *internalSpace;     /**< Internal namespace */
    struct EjsNamespace *publicSpace;       /**< Public namespace */

    char                *castTemp;          /**< Temporary string for casting */
    char                *errorMsg;          /**< Error message */
    cchar               **argv;             /**< Command line args */
    int                 argc;               /**< Count of command line args */
    int                 flags;              /**< Execution flags */
    int                 exitStatus;         /**< Status to exit() */
    int                 serializeDepth;     /**< Serialization depth */
    int                 joining;            /**< In Worker.join */
    int                 spreadArgs;         /**< Count of spread args */

    int                 workQuota;          /* Quota of work before GC */
    int                 workDone;           /**< Count of allocations to determining if GC needed */
    int                 gcRequired;         /**< Garbage collection is now required */

    uint                compiling: 1;       /**< Currently executing the compiler */
    uint                empty: 1;           /**< Interpreter will be created empty */
    uint                initialized: 1;     /**< Interpreter fully initialized and not empty */
    uint                hasError: 1;        /**< Interpreter has an initialization error */
    uint                exiting: 1;         /**< VM should exit */

    struct EjsObj       *exceptionArg;      /**< Exception object for catch block */

    MprDispatcher       *dispatcher;        /**< Event dispatcher */
    MprList             *workers;           /**< Worker interpreters */
    MprList             *modules;           /**< Loaded modules */

    void                (*loaderCallback)(struct Ejs *ejs, int kind, ...);
    void                *userData;          /**< User data */
    MprHashTable        *coreTypes;         /**< Core type instances */
    MprHashTable        *standardSpaces;    /**< Hash of standard namespaces (global namespaces) */
    MprHashTable        *doc;               /**< Documentation */
    void                *sqlite;            /**< Sqlite context information */

    Http                *http;              /**< Http service object (copy of EjsService.http) */
    HttpLoc             *loc;               /**< Current HttpLocation object for web start scripts */

    struct EjsObj       *sessions;          /**< Session cache */
    struct EjsType      *sessionType;       /**< Session type object */
    struct EjsObj       *applications;      /**< Application cache */
    MprEvent            *sessionTimer;      /**< Session expiry timer */
    int                 sessionTimeout;     /**< Default session timeout */
    int                 nextSession;        /**< Session ID counter */
    MprMutex            *mutex;             /**< Multithread locking */
} Ejs;


#if !DOXYGEN
/**
    Native Function signature
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Reference to the "this" object. (The object containing the method).
    @param argc Number of arguments.
    @param argv Array of arguments.
    @returns Returns a result variable or NULL on errors and exceptions.
    @stability Prototype.
 */
typedef struct EjsObj *(*EjsFun)(Ejs *ejs, struct EjsObj *vp, int argc, struct EjsObj **argv);
//LEGACY
typedef EjsFun EjsProc;
typedef EjsFun EjsNativeFunction;
#endif

//  TODO is this used?
typedef int (*EjsSortFn)(Ejs *ejs, struct EjsObj *p1, struct EjsObj *p2, cchar *name, int order);

/**
    Qualified name structure
    @description All names in Ejscript consist of a property name and a name space. Namespaces provide discrete
        spaces to manage and minimize name conflicts. These names will soon be converted to unicode.
    @stability Prototype
    @defgroup EjsName EjsName
    @see EjsName ejsName ejsAllocName ejsDupName ejsCopyName
 */       
typedef struct EjsName {
    cchar       *name;                          /**< Property name */
    cchar       *space;                         /**< Property namespace */
} EjsName;


/**
    Initialize a Qualified Name structure
    @description Initialize the statically allocated qualified name structure using a name and namespace.
    @param qname Reference to an existing, uninitialized EjsName structure
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */
extern EjsName *ejsName(struct EjsName *qname, cchar *space, cchar *name);

#define EN(qname, name) ejsName(qname, "", name)

/**
    Allocate and Initialize  a Qualified Name structure
    @description Create and initialize a qualified name structure using a name and namespace.
    @param ctx Any memory context returned by mprAlloc
    @param space Namespace string
    @param name Name string
    @return A reference to an allocated EjsName structure. Caller must free.
    @ingroup EjsName
 */
extern EjsName *ejsAllocName(MprCtx ctx, cchar *space, cchar *name);

extern EjsName *ejsDupName(MprCtx ctx, EjsName *qname);
extern EjsName ejsCopyName(MprCtx ctx, EjsName *qname);

/** 
    Property traits. 
    @description Property traits describe the type and access attributes of a property. The Trait structure
        is used by EjsBlock to describe the attributes of properties defined within a block.
        Note: These traits apply to a property definition and not to the referenced object. ie. two property 
        definitions may have different traits but will refer to the same object.
    @stability Evolving
    @ingroup EjsBlock
 */
typedef struct EjsTrait {
    struct EjsType  *type;                  /**< Property type (prototype) */
    int             attributes;             /**< Modifier attributes */
} EjsTrait;


typedef struct EjsSlot {
    EjsName         qname;                  /**< Property name */
    int             hashChain;              /**< Next property in hash chain */
    EjsTrait        trait;                  /**< Property descriptor traits */
    union {
        struct EjsObj *ref;                 /**< Vector of slots containing property references */
        MprNumber   *number;                /**< Immediate number value */
    } value;
} EjsSlot;

typedef struct EjsSlots {
    struct EjsSlot  *slots;                 /**< Vector of slots containing property references */
    int             sizeSlots;              /**< Current size of traits[] and slots[] */
    int             numSlots;               /**< Number of properties in traits/slots */
} EjsSlots;

typedef struct EjsHash {
    int             size;                   /**< Size of hash */
    int             *buckets;               /**< Hash buckets and head of link chains */
} EjsHash;


/** 
    Object Type. Base type for all objects.
    @description The EjsObj type is the foundation for all types, blocks, functions and scripted classes. 
        It provides storage and hashed lookup for properties.
        \n\n
        EjsObj stores properties in an array of slots. These slots store a reference to the property value. 
        Property names are stored in a names hash. Dynamic objects own their own name hash. Sealed object instances 
        of a type, will simply refer to the hash of names owned by the type.
        \n\n
        EjsObjs may be either dynamic or sealed. Dynamic objects can grow the number of properties. Sealed 
        objects cannot. Sealed objects will store the slot array as part of the EjsObj memory chunk. Dynamic 
        objects will perform a separate allocation for the slot array so that it can grow.
    @stability Evolving.
    @defgroup EjsObj EjsObj
    @see EjsObj ejsIsObject ejsCreateSimpleObject ejsCreateObject ejsCloneObject ejsGrowObject ejsMarkObject
        TODO - change these From Var
        ejsGetVarType ejsAllocObj ejsFreeObj ejsCast ejsClone ejsCreateInstance ejsCreateVar
        ejsDestroyVar ejsDefineProperty ejsDeleteProperty ejsDeletePropertyByName
        ejsGetProperty ejsLookupProperty ejsSetProperty ejsSetPropertyByName ejsSetPropertyName
        ejsSetPropertyTrait ejsDeserialize ejsParseVar
 */
typedef struct EjsObj {
#if BLD_DEBUG
    cchar   *name;                              /**< Name useful when debugging memory allocations */
    int     seq;                                /**< Unique allocations squence */
#endif

#if BLD_HAS_UNNAMED_UNIONS
    union {
        struct {
#endif
            uint    builtin           :  1;     /**< Object is part of ejs-core */
            uint    dynamic           :  1;     /**< Object may add properties */
            uint    isFrame           :  1;     /**< Instance is a frame */
            uint    isFunction        :  1;     /**< Instance is a function */
            uint    isPrototype       :  1;     /**< Object is a type prototype object */
            uint    isType            :  1;     /**< Instance is a type object */
            uint    marked            :  1;     /**< GC marked in use */
            uint    master            :  1;     /**< Allocated in the master interpreter */
            uint    permanent         :  1;     /**< Object is immune from GC */
            uint    separateSlots     :  1;     /**< Object has separate slots[] memory */
            uint    shortScope        :  1;     /**< Don't follow type or base classes */
            uint    visited           :  1;     /**< Has been traversed */
#if BLD_HAS_UNNAMED_UNIONS
        };
        int         bits;
    };
#endif
    struct EjsType  *type;                      /**< Type of this object (not base type). ie. type for Object is EjsType  */

    //  MOB -- convert to EjsSlots *properties;
    struct EjsSlot  *slots;                     /**< Vector of slots containing property references */
    int             sizeSlots;                  /**< Current size of traits[] and slots[] */
    int             numSlots;                   /**< Number of properties in traits/slots */

    EjsHash         *hash;                      /**< Hash buckets and head of link chains */
} EjsObj;

//  LEGACY TODO
typedef EjsObj EjsVar;
typedef EjsObj EO;

#if DOXYGEN
    /** 
        Get a variables type
        @description Get the base type for a variable
        @param vp Variable reference
        @returns A reference to the variables type object
        @ingroup EjsObj
     */
    extern EjsType *ejsGetVarType(EjsObj *vp);
#else
    #define ejsGetVarType(vp)       (vp->type)
    #if BLD_DEBUG
    #define ejsGetDebugName(vp) (ejsIsType(vp) ? ((EjsType*)(vp))->qname.name : mprGetName(vp))
    #define ejsSetDebugName(vp, debugName) \
        if (1) { \
            mprSetName(vp, debugName); \
            ((EjsObj*) vp)->name = debugName; \
        } else
    #else
    #define ejsGetDebugName(vp) (ejsIsType(vp) ? ((EjsType*)(vp))->qname.name : "")
    #define ejsSetDebugName(vp, name)
    #define ejsSetFmtDebugName(vp, fmt, arg)
    #endif
#endif

extern void *ejsAlloc(Ejs *ejs, int size);
extern void ejsFree(Ejs *ejs, void *ptr);

/** 
    Allocate a new variable
    @description This will allocate space for a bare variable. This routine should only be called by type factories
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type object from which to create an object instance
    @param size Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsObj *ejsAllocVar(Ejs *ejs, struct EjsType *type, int size);
extern EjsObj *ejsAllocPooled(Ejs *ejs, int id);

/**
    Free a new variable
    @description This should typically only be called by the destroyVar type helper which is invoked by the GC when
        a variable is no longer needed. It should not be called by normal code.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to free
    @param pool Optional pool id. Set to -1 for defaults.
    @ingroup EjsObj
 */
extern void ejsFreeVar(Ejs *ejs, void *vp, int pool);

/** 
    Cast a variable to a new type
    @description Cast a variable and return a new variable of the required type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to cast
    @param type Type to cast to
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsObj *ejsCast(Ejs *ejs, EjsObj *vp, struct EjsType *type);

/** 
    Clone a variable
    @description Copy a variable and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to clone
    @param deep Set to true to do a deep copy.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsObj *ejsClone(Ejs *ejs, EjsObj *vp, bool deep);

/** 
    Create a new variable instance 
    @description Create a new variable instance and invoke any required constructors with the given arguments.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type from which to create a new instance
    @param argc Count of args in argv
    @param argv Vector of arguments. Each arg is an EjsObj.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsObj *ejsCreateInstance(Ejs *ejs, struct EjsType *type, int argc, EjsObj **argv);

/** 
    Create a variable
    @description Create a variable of the required type. This invokes the createVar helper method for the specified type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type to cast to
    @param numSlots Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsObj *ejsCreate(Ejs *ejs, struct EjsType *type, int numSlots);

/** 
    Destroy a variable
    @description Destroy a variable of the required type. This invokes the destroyVar helper method for the specified type.
        The default action for the destroyVar helper is to simply invoke ejsFreeObj which will return the variable
        storage to a type pool or return the memory to the heap.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Varaible to destroy
    @ingroup EjsObj
 */
extern void ejsDestroy(Ejs *ejs, EjsObj *vp);

/** 
    Define a property
    @description Define a property in a variable and give it a name, base type, attributes and default value.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable in which to define a property
    @param slotNum Slot number in the variable for the property. Slots are numbered sequentially from zero. Set to
        -1 to request the next available slot number.
    @param qname Qualified name containing a name and a namespace.
    @param type Base type of the property. Set to ejs->voidType to leave as untyped.
    @param attributes Attribute traits. Useful attributes include:
        @li EJS_FUN_OVERRIDE
        @li EJS_ATTR_CONST
        @li EJS_ATTR_ENUMERABLE
    @param value Initial value of the property
    @return A postitive slot number or a negative MPR error code.
    @ingroup EjsObj
 */
extern int ejsDefineProperty(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *qname, struct EjsType *type, 
    int64 attributes, EjsObj *value);

/** 
    Delete a property
    @description Delete a variable's property and set its slot to null. The slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable in which to delete the property
    @param slotNum Slot number in the variable for the property to delete.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
extern int ejsDeleteProperty(Ejs *ejs, EjsObj *vp, int slotNum);

/** 
    Delete a property by name
    @description Delete a variable's property by name and set its slot to null. The property is resolved by using 
        ejsLookupProperty with the specified name. Once deleted, the slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable in which to delete the property
    @param qname Qualified name for the property including name and namespace.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
extern int ejsDeletePropertyByName(Ejs *ejs, EjsObj *vp, EjsName *qname);

/** 
    Get a property
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param slotNum Slot number for the requested property.
    @return The variable property stored at the nominated slot.
    @ingroup EjsObj
 */
extern void *ejsGetProperty(Ejs *ejs, void *vp, int slotNum);

/** 
    Get a count of properties in a variable
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @return A positive integer count of the properties stored by the variable. 
    @ingroup EjsObj
 */
extern int ejsGetPropertyCount(Ejs *ejs, EjsObj *vp);

/** 
    Get a variable property's name
    @description Get a property name for the property at a given slot in the  variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param slotNum Slot number for the requested property.
    @return The qualified property name including namespace and name. Caller must not free.
    @ingroup EjsObj
 */
extern EjsName ejsGetPropertyName(Ejs *ejs, void *vp, int slotNum);

/** 
    Get a property by name
    @description Get a property from a variable by name.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param qname Qualified name specifying both a namespace and name.
    @return The variable property stored at the nominated slot.
    @ingroup EjsObj
 */
extern void *ejsGetPropertyByName(Ejs *ejs, void *vp, EjsName *qname);

/** 
    Get a property's traits
    @description Get a property's trait description. The property traits define the properties base type,
        and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param slotNum Slot number for the requested property.
    @return A trait structure reference for the property.
    @ingroup EjsObj
 */
extern struct EjsTrait *ejsGetPropertyTrait(Ejs *ejs, EjsObj *vp, int slotNum);

/** 
    Invoke an opcode on a native type.
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
        Native types would normally implement the invokeOperator helper function to respond to this function call.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsObj
 */
extern EjsObj *ejsInvokeOperator(Ejs *ejs, EjsObj *vp, int opCode, EjsObj *rhs);

/** 
    Lookup a property by name
    @description Search for a property by name in the given variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param qname Qualified name of the property to search for.
    @return The slot number containing the property. Then use #ejsGetProperty to retrieve the property or alternatively
        use ejsGetPropertyByName to lookup and retrieve in one step.
    @ingroup EjsObj
 */
extern int ejsLookupProperty(Ejs *ejs, EjsObj *vp, EjsName *qname);

/** 
    Mark a variable as currently in use.
    @description Mark a variables as currently active so the garbage collector will preserve it. This routine should
        be called by native types in their markVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to mark as currently being used.
    @ingroup EjsObj
 */
extern void ejsMark(Ejs *ejs, void *vp);

/** 
    Set a property's value
    @description Set a value for a property at a given slot in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param slotNum Slot number for the requested property.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
extern int ejsSetProperty(Ejs *ejs, void *vp, int slotNum, void *value);

/** 
    Set a property's value 
    @description Set a value for a property. The property is located by name in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param qname Qualified property name.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
extern int ejsSetPropertyByName(Ejs *ejs, void *vp, EjsName *qname, void *value);

/** 
    Set a property's name 
    @description Set a qualified name for a property at the specified slot in the variable. The qualified name
        consists of a namespace and name - both of which must be persistent. A typical paradigm is for these name
        strings to be owned by the memory context of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param qname Qualified property name.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
extern int ejsSetPropertyName(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *qname);

/** 
    Set a property's traits
    @description Set the traits describing a property. These include the property's base type and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param vp Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param type Base type for the property. Set to NULL for an untyped property.
    @param attributes Integer mask of access attributes.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
extern int ejsSetPropertyTrait(Ejs *ejs, EjsObj *vp, int slotNum, struct EjsType *type, int attributes);

//  TODO - DOC - 
extern EjsObj  *ejsDeserialize(Ejs *ejs, struct EjsString *value);

//  TODO - DOC
extern EjsObj *ejsParse(Ejs *ejs, cchar *str,  int prefType);
extern void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count);
extern void ejsCopySlots(Ejs *ejs, EjsObj *obj, EjsSlot *dest, EjsSlot *src, int count, int dup);

/** 
    Create a simple object
    @description Create a simple object using Object as its base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @return A new object instance
    @ingroup EjsObj
 */
extern EjsObj *ejsCreateSimpleObject(Ejs *ejs);

/** 
    Create an object instance of the specified type
    @description Create a new object using the specified type as a base class. 
        Note: the constructor is not called. If you require the constructor to be invoked, use #ejsCreateInstance
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Base type to use when creating the object instance
    @param size Number of extra slots to allocate when creating the object
    @return A new object instance
    @ingroup EjsObj
 */
extern void *ejsCreateObject(Ejs *ejs, struct EjsType *type, int size);

extern int ejsInsertGrowObject(Ejs *ejs, EjsObj *obj, int numSlots, int offset);
extern int ejsRemoveProperty(Ejs *ejs, EjsObj *obj, int slotNum);
extern int ejsGetOwnNames(Ejs *ejs, EjsObj *obj, int sizeNames);
extern int ejsMakeObjHash(EjsObj *obj);
extern void ejsClearObjHash(EjsObj *obj);

extern int ejsGetOwnTraits(Ejs *ejs, EjsObj *obj, int sizeTraits);
extern void ejsSetTraitType(struct EjsTrait *trait, struct EjsType *type);
extern void ejsSetTraitAttributes(struct EjsTrait *trait, int attributes);
extern EjsTrait *ejsGetTrait(Ejs *ejs, void *obj, int slotNum);
extern int ejsSetTraitDetails(Ejs *ejs, void *vp, int slotNum, struct EjsType *type, int attributes);
extern int ejsHasTrait(EjsObj *obj, int slotNum, int attributes);
extern int ejsGetTraitAttributes(EjsObj *obj, int slotNum);
extern struct EjsType *ejsGetTraitType(EjsObj *obj, int slotNum);
extern int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int overwrite);


//  TODO - inconsistent naming vs ejsCloneVar (clone vs copy)
//
/** 
    Copy an object
    @description Copy an object create a new instance. This may do a shallow or deep copy depending on the value of 
        \a deep. A shallow copy will not copy the property instances, rather it will only duplicate the property 
        reference. A deep copy will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param src Source object to copy
    @param deep Set to true to do a deep copy.
    @return A newly allocated object. Caller must not free as the GC will manage the lifecycle of the variable.
    @ingroup EjsObj
 */
extern void *ejsCloneObject(Ejs *ejs, void *src, bool deep);

/** 
    Grow an object
    @description Grow the property storage for an object. Object properties are stored in slots. To store more 
        properties, you need to grow the slots.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object reference to grow
    @param numSlots New minimum count of properties. If size is less than the current number of properties, the call
        will be ignored, i.e. it will not shrink objects.
    @return Zero if successful
    @ingroup EjsObj
 */
extern int ejsGrowObject(Ejs *ejs, EjsObj *obj, int numSlots);

/** 
    Mark an object as currently in use.
    @description Mark an object as currently active so the garbage collector will preserve it. This routine should
        be called by native types that extend EjsObj in their markVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to mark as currently being used.
    @ingroup EjsObj
 */
extern void     ejsMarkObject(Ejs *ejs, void *obj);

extern int      ejsGetSlot(Ejs *ejs, EjsObj *obj, int slotNum);
extern EjsObj   *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);
extern int      ejsComputeHashCode(EjsObj *obj, EjsName *qname);
extern int      ejsGetHashSize(int numProp);
extern void     ejsCreateObjectHelpers(Ejs *ejs);
extern int      ejsInsertGrowObject(Ejs *ejs, EjsObj *obj, int size, int offset);
#if UNUSED
extern int      ejsLookupSingleProperty(Ejs *ejs, EjsObj *obj, EjsName *qname);
#endif
extern void     ejsMakePropertyDontDelete(EjsObj *vp, int dontDelete);
extern int      ejsMakePropertyEnumerable(EjsObj *vp, bool enumerable);
extern void     ejsMakePropertyReadOnly(EjsObj *vp, int readonly);
extern EjsObj   *ejsObjectOperator(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);
extern int      ejsRebuildHash(Ejs *ejs, EjsObj *obj);
extern void     ejsResetHash(Ejs *ejs, EjsObj *obj);
extern void     ejsRemoveSlot(Ejs *ejs, EjsObj *slots, int slotNum, int compact);
extern void     ejsSetAllocIncrement(Ejs *ejs, struct EjsType *type, int increment);
extern EjsObj   *ejsToSource(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);


/** 
    Block class
    @description The block class is the base class for all program code block scope objects. This is an internal class
        and not exposed to the script programmer.
    Blocks (including types) may describe their properties via traits. The traits store the property 
    type and access attributes and are stored in EjsBlock which is a sub class of EjsObj. See ejsBlock.c for details.
    @stability Evolving
    @defgroup EjsBlock EjsBlock
    @see EjsBlock ejsIsBlock ejsBindFunction
 */
typedef struct EjsBlock {
    EjsObj          obj;                            /**< Extends Object - Property storage */
    EjsList         namespaces;                     /**< Current list of namespaces open in this block of properties */
    struct EjsBlock *scope;                         /**< Lexical scope chain for this block */
    struct EjsBlock *prev;                          /**< Previous block in activation chain */

    //  MOB -- OPT and compress / eliminate some of these fields. Every function has these.
    EjsObj          *prevException;                 /**< Previous exception if nested exceptions */
    EjsVar          **stackBase;                    /**< Start of stack in this block */
    uint            breakCatch: 1;                  /**< Return, break or continue in a catch block */
    uint            isGlobal: 1;                    /**< Block is the global block */
    uint            nobind: 1;                      /**< Don't bind to properties in this block */
} EjsBlock;


#if DOXYGEN
    /** 
        Determine if a variable is a block.
        @description This call tests if the variable is a block.
        @param vp Variable to test
        @returns True if the variable is based on EjsBlock
        @ingroup EjsBlock
     */
    extern bool ejsIsBlock(EjsObj *vp);
#else
    //  MOB -- very slot. Should have EjsObj.isBlock
    #define ejsIsBlock(vp) (ejsIs(vp, ES_Block) || ejsIs(vp, ES_Function) || ejsIs(vp, ES_Type) || ejsIsFrame(vp))
#endif

/** 
    Bind a native C function to a function property
    @description Bind a native C function to an existing javascript function. Functions are typically created
        by compiling a script file of native function definitions into a mod file. When loaded, this mod file 
        will create the function properties. This routine will then bind the specified C function to the 
        function property.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fn Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
extern int ejsBindFunction(Ejs *ejs, void *obj, int slotNum, EjsProc fn);

/*  
    This is all an internal API. Native types should probably not be using these routines. Speak up if you find
    you need these routines in your code.
 */

extern int      ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *blockRef, struct EjsNamespace *namespace);
extern int      ejsAddScope(MprCtx ctx, EjsBlock *block, EjsBlock *scopeBlock);
extern EjsBlock *ejsCreateBlock(Ejs *ejs, int numSlots);

//  TODO - why do we have ejsCloneObject, ejsCloneBlock ... Surely ejsCloneVar is sufficient?
extern EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep);

extern int      ejsCaptureScope(Ejs *ejs, EjsBlock *block, EjsList *scopeChain);
extern int      ejsCopyScope(EjsBlock *block, EjsList *chain);
extern int      ejsGetNamespaceCount(EjsBlock *block);

extern EjsBlock *ejsGetTopScope(EjsBlock *block);
extern void     ejsMarkBlock(Ejs *ejs, EjsBlock *block);
extern void     ejsPopBlockNamespaces(EjsBlock *block, int count);
extern EjsBlock *ejsRemoveScope(EjsBlock *block);
extern void     ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block);

/** 
    Exception Handler Record
    @description Each exception handler has an exception handler record allocated that describes it.
    @ingroup EjsFunction
 */
typedef struct EjsEx {
// TODO - OPT. Should this be compressed via bit fields for flags Could use short for these offsets.
    struct EjsType  *catchType;             /**< Type of error to catch */
    uint            flags;                  /**< Exception flags */
    uint            tryStart;               /**< Ptr to start of try block */
    uint            tryEnd;                 /**< Ptr to one past the end */
    uint            handlerStart;           /**< Ptr to start of catch/finally block */
    uint            handlerEnd;             /**< Ptr to one past the end */
    uint            numBlocks;              /**< Count of blocks opened before the try block */
    uint            numStack;               /**< Count of stack slots pushed before the try block */
} EjsEx;


// TODO OPT. Could compress this.
/** 
    Byte code
    @description This structure describes a sequence of byte code for a function. It also defines a set of
        execption handlers pertaining to this byte code.
    @ingroup EjsFunction
 */
typedef struct EjsCode {
    uchar           *byteCode;              /**< Byte code */
    int             codeLen;                /**< Byte code length */
    struct EjsConst *constants;             /**< Constant pool. Reference to module constant pool */
    int             numHandlers;            /**< Number of exception handlers */
    int             sizeHandlers;           /**< Size of handlers array */
    struct EjsEx    **handlers;             /**< Exception handlers */
} EjsCode;


/** 
    Function class
    @description The Function type is used to represent closures, function expressions and class methods. 
        It contains a reference to the code to execute, the execution scope and possibly a bound "this" reference.
    @stability Evolving
    @defgroup EjsFunction EjsFunction
    @see EjsFunction ejsIsFunction ejsIsNativeFunction ejsIsInitializer ejsCreateFunction ejsCloneFunction
        ejsRunFunctionBySlot ejsRunFunction ejsRunInitializer
 */
typedef struct EjsFunction {
    /*
        A function can store properties like any other object. If it has parameters, it must also must maintain an
        activation object. When compiling, the compiler stores parameters in the normal property "block", it then
        transfers them into the activation block when complete.
     */
    EjsBlock        block;                  /** Activation block for local vars */
    EjsObj          *activation;            /** Activation properties (parameters + locals) */

    //  MOB -- should this be BLD_DEBUG?
    cchar           *name;                  /** Function name for debuggability */

#if FUTURE && MOB
    union {
#endif
        struct EjsFunction *setter;         /**< Setter function for this property */
        struct EjsType  *archetype;         /**< Type to use to create instances */
#if FUTURE && MOB
    } extra;
#endif

    union {
        EjsCode     code;                   /**< Byte code */
        EjsProc     proc;                   /**< Native function pointer */
    } body;

    struct EjsArray *boundArgs;             /**< Bound "args" */
    EjsObj          *boundThis;             /**< Bound "this" object value */
    struct EjsType  *resultType;            /**< Return type of method */

#if BLD_HAS_UNNAMED_UNIONS
    union {
        struct {
#endif
            uint    numArgs: 8;             /**< Count of formal parameters */
            uint    numDefault: 8;          /**< Count of formal parameters with default initializers */
            uint    castNulls: 1;           /**< Cast return values of null */
            uint    fullScope: 1;           /**< Closures must capture full scope */
            uint    hasReturn: 1;           /**< Function has a return stmt */
            uint    inCatch: 1;             /**< Executing catch block */
            uint    inException: 1;         /**< Executing catch/finally exception processing */
            uint    isConstructor: 1;       /**< Function is a constructor */
            uint    isInitializer: 1;       /**< Function is a type initializer */
            uint    isNativeProc: 1;        /**< Function is native procedure */
            uint    moduleInitializer: 1;   /**< Function is a module initializer */
            uint    rest: 1;                /**< Function has a "..." rest of args parameter */
            uint    staticMethod: 1;        /**< Function is a static method */
            uint    strict: 1;              /**< Language strict mode (vs standard) */
            uint    throwNulls: 1;          /**< Return type cannot be null */

#if BLD_HAS_UNNAMED_UNIONS
        };
        int64       bits;
    };
#endif
} EjsFunction;

#if DOXYGEN
    /** 
        Determine if a variable is a function. This will return true if the variable is a function of any kind, including
            methods, native and script functions or initializers.
        @param vp Variable to test
        @return True if the variable is a function
        @ingroup EjsFunction
     */
    extern bool ejsIsFunction(EjsObj *vp);

    /** 
        Determine if the function is a native function. Functions can be either native - meaning the implementation is
            via a C function, or can be scripted.
        @param vp Variable to test
        @return True if the variable is a native function.
        @ingroup EjsFunction
     */
    extern bool ejsIsNativeFunction(EjsObj *vp);

    /** 
        Determine if the function is an initializer. Initializers are special functions created by the compiler to do
            static and instance initialization of classes during construction.
        @param vp Variable to test
        @return True if the variable is an initializer
        @ingroup EjsFunction
     */
    extern bool ejsIsInitializer(EjsObj *vp);
#else
    #define ejsIsFunction(vp)       (vp && ((EjsObj*) vp)->isFunction)
    #define ejsIsNativeFunction(vp) (ejsIsFunction(vp) && (((EjsFunction*) (vp))->isNativeProc))
    #define ejsIsInitializer(vp)    (ejsIsFunction(vp) && (((EjsFunction*) (vp))->isInitializer)
#endif

/** 
    Create a function object
    @description This creates a function object and optionally associates byte code with the function.
    @param ejs Ejs reference returned from #ejsCreate
    @param name Function name used in stack backtraces.
    @param code Pointer to the byte code. The byte code is not copied so this must be a persistent pointer.
    @param codeLen Length of the code.
    @param numArgs Number of formal arguments to the function.
    @param numDefault Number of default args to the function.
    @param numExceptions Number of exception handlers
    @param returnType Return type of the function. Set to NULL for no defined type.
    @param attributes Integer mask of access attributes.
    @param constants Reference to the module constant pool. Some byte code opcodes contain references into the
        constant pool
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
extern EjsFunction *ejsCreateFunction(Ejs *ejs, cchar *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsConst *constants, EjsBlock *scope, 
    int strict);
extern void ejsInitFunction(Ejs *ejs, EjsFunction *fun, cchar *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsConst *constants, EjsBlock *scope, 
    int strict);
extern EjsFunction *ejsCreateSimpleFunction(Ejs *ejs, cchar *name, int attributes);
extern void ejsDisableFunction(Ejs *ejs, EjsFunction *fun);

extern EjsObj *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots);
extern void ejsCompleteFunction(Ejs *ejs, EjsFunction *fun);
extern void ejsUseActivation(Ejs *ejs, EjsFunction *fun);

/** 
    Run the initializer for a module
    @description A module's initializer runs global code defined in the module
    @param ejs Ejs reference returned from #ejsCreate
    @param module Module object reference
    @return The last expression result of global code executed
    @ingroup EjsFunction
 */
extern EjsObj *ejsRunInitializer(Ejs *ejs, struct EjsModule *module);

/** 
    Run a function
    @description Run a function with the given actual parameters
    @param ejs Ejs reference returned from #ejsCreate
    @param fn Function object to run
    @param obj Object to use as the "this" object when running the function
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
extern EjsObj *ejsRunFunction(Ejs *ejs, EjsFunction *fn, EjsObj *obj, int argc, EjsObj **argv);

/** 
    Run a function by slot number
    @description Run a function identified by slot number with the given actual parameters. This will run the function
        stored at \a slotNum in the \a obj variable. 
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object that holds the function at its "slotNum" slot. Also use this object as the "this" object 
        when running the function.
    @param slotNum Slot number in \a obj that contains the function to run.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
extern EjsObj *ejsRunFunctionBySlot(Ejs *ejs, EjsObj *obj, int slotNum, int argc, EjsObj **argv);
extern EjsObj *ejsRunFunctionByName(Ejs *ejs, EjsObj *container, EjsName *qname, EjsObj *obj, int argc, EjsObj **argv);

extern EjsEx *ejsAddException(EjsFunction *mp, uint tryStart, uint tryEnd, struct EjsType *catchType,
    uint handlerStart, uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex);
extern EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *src, int deep);
extern int ejsDefineException(Ejs *ejs, struct EjsType *vp, int slot, uint tryOffset,
    uint tryLength, uint handlerOffset, uint handlerLength, int flags);
extern void ejsOffsetExceptions(EjsFunction *mp, int offset);
extern int  ejsSetFunctionCode(EjsFunction *mp, uchar *byteCode, int len);
extern void ejsMarkFunction(Ejs *ejs, EjsFunction *fun);
extern void ejsShowOpFrequency(Ejs *ejs);
#if UNUSED
extern int ejsLookupFunctionProperty(Ejs *ejs, EjsFunction *fun, EjsName *qname);
#endif

typedef struct EjsFrame {
    EjsFunction     function;               /**< Activation frame for function calls. Stores local variables */
    struct EjsFrame *caller;                /**< Previous invoking frame */
    EjsVar          **stackBase;            /**< Start of stack in this function */
    EjsObj          **stackReturn;          /**< Top of stack to return to */
    int             slotNum;                /**< Slot in owner */
    uchar           *pc;                    /**< Program counter */
    uchar           *attentionPc;           /**< Restoration PC value after attention */
    int             ignoreAttention;        /**< Ignore attention commands */
    int             lineNumber;             /**< Source code line number */
    cchar           *currentLine;           /**< Current source code line */
    cchar           *filename;              /**< Source code file name */
    int             getter;                 /**< Frame is a getter */
    uint            argc;                   /**< Actual parameter count */
} EjsFrame;

#if DOXYGEN
    /** 
        Determine if a variable is a frame. Only used internally in the VM.
        @param vp Variable to test
        @return True if the variable is a frame. 
        @ingroup EjsFrame
     */
    extern bool ejsIsFrame(EjsObj *vp);
#else
    #define ejsIsFrame(vp)       ejsIs(vp, ES_Frame)
#endif

extern EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *src, EjsObj *thisObj, int argc, EjsObj **argv);
extern EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *src);
extern EjsBlock *ejsPopBlock(Ejs *ejs);
extern EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *block);

/** 
    Array class
    @description Arrays provide a growable, integer indexed, in-memory store for objects. An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered). Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list. The Array class can store objects with numerical indicies and can also store 
        any named properties. The named properties are stored in the obj field, whereas the numeric indexed values are
        stored in the data field. Array extends EjsObj and has all the capabilities of EjsObj.
    @stability Evolving
    @defgroup EjsArray EjsArray
    @see EjsArray ejsCreateArray ejsIsArray
 */
typedef struct EjsArray {
    EjsObj          obj;                /**< Extends Object */
    EjsObj          **data;             /**< Array elements */
    int             length;             /**< Array length property */
} EjsArray;


/** 
    Create an array
    @param ejs Ejs reference returned from #ejsCreate
    @param size Initial size of the array
    @return A new array object
    @ingroup EjsArray
 */
extern EjsArray *ejsCreateArray(Ejs *ejs, int size);

#if DOXYGEN
    /** 
        Determine if a variable is an array
        @param vp Variable to test
        @return True if the variable is an array
        @ingroup EjsArray
     */
    extern bool ejsIsArray(EjsObj *vp);
#else
    #define ejsIsArray(vp) ejsIs(vp, ES_Array)
#endif

/** 
    Boolean class
    @description The Boolean class provides the base class for the boolean values "true" and "false".
        EjsBoolean is a primitive native type and extends EjsObj. It is still logically an Object, but implements
        Object properties and methods itself. Only two instances of the boolean class are ever created created
        these are referenced as ejs->trueValue and ejs->falseValue.
    @stability Evolving
    @defgroup EjsBoolean EjsBoolean
    @see EjsBoolean ejsCreateBoolean ejsIsBoolean ejsGetBoolean
 */
typedef struct EjsBoolean {
    EjsObj          obj;                /**< Extends Object - Property storage */
    bool            value;              /**< Boolean value */
} EjsBoolean;

/** 
    Create a boolean
    @description Create a boolean value. This will not actually create a new boolean instance as there can only ever
        be two boolean instances (true and false). Boolean properties are immutable in Ejscript and so this routine
        will simply return the appropriate pre-created true or false boolean value.
    @param ejs Ejs reference returned from #ejsCreate
    @param value Desired boolean value. Set to 1 for true and zero for false.
    @ingroup EjsBoolean
 */
extern EjsBoolean *ejsCreateBoolean(Ejs *ejs, int value);

/** 
    Cast a variable to a boolean 
    @description
    @param ejs Ejs reference returned from #ejsCreate
    @param vp Variable to cast
    @return A new boolean object
    @ingroup EjsBoolean
 */
extern EjsBoolean *ejsToBoolean(Ejs *ejs, EjsObj *vp);

#if DOXYGEN
    /** 
        Determine if a variable is a boolean
        @param vp Variable to test
        @return True if the variable is a boolean
        @ingroup EjsBoolean
     */
    extern bool ejsIsBoolean(EjsObj *vp);

    /** 
        Get the C boolean value from a boolean object
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Boolean variable to access
        @return True or false
        @ingroup EjsBoolean
     */
    extern bool ejsGetBoolean(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsBoolean(vp) ejsIs(vp, ES_Boolean)
    extern int _ejsGetBoolean(Ejs *ejs, EjsObj *vp);
    #define ejsGetBoolean(ejs, vp) _ejsGetBoolean(ejs, (EjsObj*) (vp))
#endif

/*
    Thse constants match Stream.READ, Stream.WRITE, Stream.BOTH
 */
#define EJS_STREAM_READ     0x1
#define EJS_STREAM_WRITE    0x2
#define EJS_STREAM_BOTH     0x3

/** 
    ByteArray class
    @description ByteArrays provide a growable, integer indexed, in-memory store for bytes. ByteArrays can be used as a 
    simple array type to store and encode data as bytes or they can be used as buffered Streams implementing the Stream 
    interface.
    \n\n
    When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
    extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
    can be used to get and put blocks of data. In this mode, the read and write position properties are ignored. 
    Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
    the ByteArray can be designated as growable, in which case the initial size will grow as required to accomodate 
    data and the length property will be updated accordingly.
    \n\n
    When used as a Stream, the byte array additional write methods to store data at the location specified by the 
    $writePosition property and read methods to read from the $readPosition property. The $available method 
    indicates how much data is available between the read and write position pointers. The $reset method can 
    reset the pointers to the start of the array.  When used with for/in, ByteArrays will iterate or 
    enumerate over the available data between the read and write pointers.
    \n\n
    If numeric values are read or written, they will be encoded according to the value of the endian property 
    which can be set to either LittleEndian or BigEndian. 
    \n\n
    In Stream mode ByteArrays can be configured to run in sync or async mode. Adding observers via the $addObserver
    method will put a stream into async mode. Events will then be issued for close, eof, read and write events.
    @stability Evolving
    @defgroup EjsByteArray EjsByteArray
    @see EjsByteArray ejsIsByteArray ejsCreateByteArray ejsSetByteArrayPositions ejsCopyToByteArray
 */
typedef struct EjsByteArray {
    EjsObj          obj;                /**< Extends Object - Property storage (not used) */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    uchar           *value;             /**< Data bytes in the array */
    int             async;              /**< Async mode */
    int             endian;             /**< Endian encoding */
    int             length;             /**< Length property */
    int             growInc;            /**< Current read position */
    int             swap;               /**< I/O must swap bytes due to endian byte ordering */
    int             readPosition;       /**< Current read position */
    int             writePosition;      /**< Current write position */
    bool            growable;           /**< Aray is growable */
    EjsObj          *listeners;         /**< Event listeners in async mode */
} EjsByteArray;

#if DOXYGEN
    /** 
        Determine if a variable is a byte array
        @param vp Variable to test
        @return True if the variable is a byte array
        @ingroup EjsByteArray
     */
    extern bool ejsIsByteArray(EjsObj *vp);
#else
    #define ejsIsByteArray(vp) ejsIs(vp, ES_ByteArray)
#endif

/** 
    Create a byte array
    @description Create a new byte array instance.
    @param ejs Ejs reference returned from #ejsCreate
    @param size Initial size of the byte array
    @return A new byte array instance
    @ingroup EjsByteArray
 */
extern EjsByteArray *ejsCreateByteArray(Ejs *ejs, int size);

/** 
    Set the I/O byte array positions
    @description Set the read and/or write positions into the byte array. ByteArrays implement the Stream interface
        and support sequential and random access reading and writing of data in the array. The byte array maintains
        read and write positions that are automatically updated as data is read or written from or to the array. 
    @param ejs Ejs reference returned from #ejsCreate
    @param ba Byte array object
    @param readPosition New read position to set
    @param writePosition New write position to set
    @ingroup EjsByteArray
 */
extern void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, int readPosition, int writePosition);

/** 
    Copy data into a byte array
    @description Copy data into a byte array at a specified \a offset. 
    @param ejs Ejs reference returned from #ejsCreate
    @param ba Byte array object
    @param offset Offset in the byte array to which to copy the data.
    @param data Pointer to the source data
    @param length Length of the data to copy
    @return Zero if successful, otherwise a negative MPR error code.
 */
extern int ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, int offset, char *data, int length);

extern void ejsResetByteArray(EjsByteArray *ba);
extern int ejsGetByteArrayAvailable(EjsByteArray *ba);
extern int ejsGetByteArrayRoom(EjsByteArray *ba);
extern int ejsGrowByteArray(Ejs *ejs, EjsByteArray *ap, int size);

extern struct EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);
extern bool ejsMakeRoomInByteArray(Ejs *ejs, EjsByteArray *ap, int require);


/** 
    Date class
    @description The Date class is a general purpose class for working with dates and times. 
        is a a primitive native type and extends EjsObj. It is still logically an Object, but implements Object 
        properties and methods itself. 
    @stability Evolving
    @defgroup EjsDate EjsDate
    @see EjsDate EjsIsDate ejsCreateDate
 */
typedef struct EjsDate {
    EjsObj          obj;                /**< Extends Object - Property storage */
    MprTime         value;              /**< Time in milliseconds since "1970/01/01 GMT" */
} EjsDate;

#if DOXYGEN
    /** 
        Determine if a variable is a Date
        @param vp Variable to test
        @return True if the variable is a date
        @ingroup EjsDate
     */
    bool ejsIsDate(EjsObj *vp);
#else
    #define ejsIsDate(vp) ejsIs(vp, ES_Date)
#endif

/** 
    Create a new date instance
    @param ejs Ejs reference returned from #ejsCreate
    @param value Date/time value to set the new date instance to
    @return An initialized date instance
    @ingroup EjsDate
 */
extern EjsDate *ejsCreateDate(Ejs *ejs, MprTime value);

/** 
    Error classes
    @description Base class for error exception objects. Exception objects are created by programs and by the system 
    as part of changing the normal flow of execution when some error condition occurs. 
    When an exception is created and acted upon ("thrown"), the system transfers the flow of control to a 
    pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
    point at which the exception was thrown or not. It may re-throw the exception or pass control up the call stack.
    @stability Evolving.
    @defgroup EjsError EjsError ejsFormatStack ejsGetErrorMsg ejsHasException ejsThrowArgError ejsThrowAssertError
        ejsThrowArithmeticError ejsThrowInstructionError ejsThrowError ejsThrowInternalError ejsThrowIOError
        ejsThrowMemoryError ejsThrowOutOfBoundsError ejsThrowReferenceError ejsThrowResourceError ejsThrowStateError
        ejsThrowStopIteration ejsThrowSyntaxError ejsThrowTypeError
 */
//  MOB -- perhaps remove and just use EjsObj. Ie. no longer native
typedef struct EjsError {
    EjsObj          obj;                /**< Extends Object */
#if UNUSED
    EjsObj          *data;              /**< Error data message */
    char            *message;           /**< Exception message */
    char            *stack;             /**< Execution stack back trace */
    char            *filename;          /**< Source code file name */
    int             lineNumber;         /**< Source code line number */
    int             code;               /**< Unique error lookup code */
#endif
} EjsError;

#define ejsIsError(vp) ejsIs(vp, ES_Error)

extern EjsError *ejsCreateError(Ejs *ejs, struct EjsType *type, EjsObj *message);
extern EjsArray *ejsCaptureStack(Ejs *ejs, int uplevels);

/* 
    DEPRECATED MOB
    Format the stack backtrace
    @description Return a string containing the current interpreter stack backtrace
    @param ejs Ejs reference returned from #ejsCreate
    @param error Error exception object to analyseo analyseo analyseo analyse
    @return A string containing the stack backtrace. The caller must free.
    @ingroup EjsError
extern char *ejsFormatStack(Ejs *ejs, EjsError *error);
 */

/** 
    Get the interpreter error message
    @description Return a string containing the current interpreter error message
    @param ejs Ejs reference returned from #ejsCreate
    @param withStack Set to 1 to include a stack backtrace in the error message
    @return A string containing the error message. The caller must not free.
    @ingroup EjsError
 */
extern cchar *ejsGetErrorMsg(Ejs *ejs, int withStack);

/** 
    Determine if an exception has been thrown
    @param ejs Ejs reference returned from #ejsCreate
    @return True if an exception has been thrown
    @ingroup EjsError
 */
extern bool ejsHasException(Ejs *ejs);

// TODO - DOC
extern EjsObj *ejsGetException(Ejs *ejs);

/** 
    Throw an argument exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an assertion exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an math exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an instruction code exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an general error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an internal error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an IO exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an Memory depletion exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EjsObj *ejsThrowMemoryError(Ejs *ejs);

/** 
    Throw an out of bounds exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an reference exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an resource exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an state exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an stop iteration exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EjsObj *ejsThrowStopIteration(Ejs *ejs);

/** 
    Throw an syntax error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an type error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsObj *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);


/** 
    File class
    @description The File class provides a foundation of I/O services to interact with physical files and directories.
    Each File object represents a single file or directory and provides methods for creating, opening, reading, writing 
    and deleting files, and for accessing and modifying information about the file.
    @stability Prototype
    @defgroup EjsFile EjsFile 
    @see EjsFile ejsCreateFile ejsIsFile
 */
typedef struct EjsFile {
    EjsObj          obj;                /**< Extends Object */
    MprFile         *file;              /**< Open file handle */
    MprPath         info;               /**< Cached file info */
    char            *path;              /**< Filename path */
    char            *modeString;        /**< User supplied mode string */
    int             mode;               /**< Current open mode */
    int             perms;              /**< Posix permissions mask */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsFile;

/** 
    Create a File object
    @description Create a file object associated with the given filename. The filename is not opened, just stored.
    @param ejs Ejs reference returned from #ejsCreate
    @param filename Filename to associate with the file object
    @return A new file object
    @ingroup EjsFile
 */
extern EjsFile *ejsCreateFile(Ejs *ejs, cchar *filename);

extern EjsFile *ejsCreateFileFromFd(Ejs *ejs, int fd, cchar *name, int mode);

#if DOXYGEN
    /** 
        Determine if a variable is a File
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Variable to test
        @return True if the variable is a File
        @ingroup File
     */
    extern bool ejsIsFile(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsFile(ejs, vp) (vp && ((EjsObj*) vp)->type == ejs->fileType)
#endif


/**
    Path class
    @description The Path class provides file path name services.
    @stability Prototype
    @defgroup EjsPath EjsPath 
    @see EjsFile ejsCreatePath ejsIsPath
 */
typedef struct EjsPath {
    EjsObj          obj;                /**< Extends Object */
    MprPath         info;               /**< Cached file info */
    //  TODO - should have pathLength
    //  TODO - rename value
    char            *path;              /**< Filename path */
    MprList         *files;             /**< File list for enumeration */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsPath;


/** 
    Create a Path object
    @description Create a file object associated with the given filename. The filename is not opened, just stored.
    @param ejs Ejs reference returned from #ejsCreate
    @param path Path file name
    @return A new Path object
    @ingroup EjsPath
 */
extern EjsPath *ejsCreatePath(Ejs *ejs, cchar *path);

extern EjsPath *ejsCreatePathAndFree(Ejs *ejs, char *path);

#if DOXYGEN
    /** 
        Determine if a variable is a Path
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Variable to test
        @return True if the variable is a Path
        @ingroup EjsPath
     */
    extern bool ejsIsPath(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsPath(ejs, vp) (vp && ((EjsObj*) vp)->type == ejs->pathType)
#endif


/** 
    Uri class
    @description The Uri class provides file path name services.
    @stability Prototype
    @defgroup EjsUri EjsUri 
    @see EjsFile ejsCreateUri ejsIsUri
 */
typedef struct EjsUri {
    EjsObj          obj;                /**< Extends Object */
    HttpUri         *uri;               /**< Decoded URI */
} EjsUri;


/** 
    Create a Uri object
    @description Create a URI object associated with the given URI string.
    @param ejs Ejs reference returned from #ejsCreate
    @param uri Uri string to parse
    @return A new Uri object
    @ingroup EjsUri
 */
extern EjsUri *ejsCreateUri(Ejs *ejs, cchar *uri);
extern EjsUri *ejsCreateUriAndFree(Ejs *ejs, char *uri);
extern EjsUri *ejsCreateFullUri(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, cchar *reference);

#if DOXYGEN
    /** 
        Determine if a variable is a Uri
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Variable to test
        @return True if the variable is a Uri
        @ingroup EjsUri
     */
    extern bool ejsIsUri(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsUri(ejs, vp) (vp && ((EjsObj*) vp)->type == ejs->uriType)
#endif


/** 
    FileSystem class
    @description The FileSystem class provides file system services.
    @stability Prototype
    @defgroup EjsFileSystem EjsFileSystem 
    @see EjsFile ejsCreateFile ejsIsFile
 */
typedef struct EjsFileSystem {
    EjsObj          obj;                /**< Extends Object */
    char            *path;              /**< Filename path */
    MprFileSystem   *fs;                /**< MPR file system object */
} EjsFileSystem;


/** 
    Create a FileSystem object
    @description Create a file system object associated with the given pathname.
    @param ejs Ejs reference returned from #ejsCreate
    @param path Path to describe the file system. Can be any path in the file system.
    @return A new file system object
    @ingroup EjsPath
 */
extern EjsFileSystem *ejsCreateFileSystem(Ejs *ejs, cchar *path);

#if DOXYGEN
    /**
        Determine if a variable is a Path
        @param vp Variable to test
        @return True if the variable is a FileSystem
        @ingroup EjsFileSystem
     */
    extern bool ejsIsFileSystem(EjsObj *vp);
#else
    #define ejsIsFileSystem(vp) ejsIs(vp, ES_ejs_io_FileSystem)
#endif


/** 
    EjsGlobal cass
    @description The Global class is the base class for the global object. The global object is the top level
        scoping object.
    @stability Stable
    @defgroup EjsGlobal EjsGlobal
 */
typedef EjsObj EjsGlobal;

extern EjsObj *ejsCreateGlobal(Ejs *ejs);
extern void ejsFreezeGlobal(Ejs *ejs);


/** 
    Http Class
    @description
        Http objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttp EjsHttp
    @see EjsHttp ejsCreateHttp ejsIsHttp
 */
typedef struct EjsHttp {
    EjsObj          obj;                        /**< Extends Object (not used) */
    Ejs             *ejs;                       /**< Convenience access to ejs interpreter instance */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsByteArray    *data;                      /**< Buffered write data */
    EjsObj          *limits;                    /**< Limits object */
    EjsObj          *responseCache;             /**< Cached response (only used if response() is used) */
    HttpConn        *conn;                      /**< Http connection object */
    MprBuf          *requestContent;            /**< Request body data supplied */
    MprBuf          *responseContent;           /**< Response data */
    char            *uri;                       /**< Target uri */
    char            *method;                    /**< HTTP method */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    int             dontFinalize;               /**< Suppress finalization */
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    int             readCount;                  /**< Count of body bytes read */
    int             requestContentCount;        /**< Count of bytes written from requestContent */
    int             writeCount;                 /**< Count of bytes written via write() */
} EjsHttp;


/** 
    Create a new Http object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Http object
    @ingroup EjsHttp
 */
extern EjsHttp *ejsCreateHttp(Ejs *ejs);

#if DOXYGEN
    extern bool ejsIsHttp(EjsObj *vp);
#else
    #define ejsIsHttp(vp) ejsIs(vp, ES_ejs_io_Http)
#endif
extern void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, int server);
extern void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, int server);
extern int ejsSetupTrace(Ejs *ejs, MprCtx ctx, HttpTrace *trace, EjsObj *options);


/** 
    Iterator Class
    @description Iterator is a helper class to implement iterators in other native classes
    @stability Prototype
    @defgroup EjsIterator EjsIterator
    @see EjsIterator ejsCreateIterator
 */
typedef struct EjsIterator {
    EjsObj              var;                /**< Extends Object - Property storage (not used) */
    EjsObj              *target;            /**< Object to be enumerated */
    EjsProc             nativeNext;         /**< Native next function */
    bool                deep;               /**< Iterator deep (recursively over all properties) */
    EjsArray            *namespaces;        /**< Namespaces to consider in iteration */
    int                 index;              /**< Current index */
    EjsObj              *indexVar;          /**< Reference to current item */
} EjsIterator;

/** 
    Create an iterator object
    @description The EjsIterator object is a helper class for native types to implement iteration and enumeration.
    @param ejs Ejs reference returned from #ejsCreate
    @param target Target variable to iterate or enumerate 
    @param next Function to invoke to step to the next element
    @param deep Set to true to do a deep iteration/enumeration
    @param namespaces Reserved and not used. Supply NULL.
    @return A new EjsIterator object
    @ingroup EjsIterator
 */
extern EjsIterator *ejsCreateIterator(Ejs *ejs, EjsObj *target, EjsProc next, bool deep, EjsArray *namespaces);

/** 
    Namespace Class
    @description Namespaces are used to qualify names into discrete spaces.
    @stability Evolving
    @defgroup EjsNamespace EjsNamespace
    @see EjsNamespace ejsIsNamespace ejsCreateNamespace ejsLookupNamespace ejsDefineReservedNamespace 
        ejsCreateReservedNamespace ejsFormatReservedNamespace 
 */
typedef struct EjsNamespace {
    EjsObj      obj;                /**< Extends Object - Property storage */
    char        *name;              /**< Textual name of the namespace */
#if TODO || 1 /* REMOVE */
    char        *uri;               /**< Optional uri definition */
    int         flags;              /**< Fast comparison flags */
#endif
} EjsNamespace;


/** 
    Create a namespace object
    @param ejs Ejs reference returned from #ejsCreate
    @param name Space name to use for the namespace
    @param uri URI to associate with the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
extern EjsNamespace *ejsCreateNamespace(Ejs *ejs, cchar *name, cchar *uri);

#if DOXYGEN
    /**
        Determine if a variable is a namespace
        @return True if the variable is a namespace
        @ingroup EjsNamespace
     */
    extern bool ejsIsNamespace(EjsObj *vp)
#else
    #define ejsIsNamespace(vp) ejsIs(vp, ES_Namespace)
#endif

extern EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, cchar *name);
extern EjsNamespace *ejsCreateReservedNamespace(Ejs *ejs, EjsName *typeName, cchar *name);
extern char *ejsFormatReservedNamespace(MprCtx ctx, EjsName *typeName, cchar *spaceName);

/** 
    Null Class
    @description The Null class provides the base class for the singleton null instance. This instance is stored
        in ejs->nullValue.
    @stability Evolving
    @defgroup EjsNull EjsNull
    @see EjsNull ejsCreateIsNull
 */
typedef EjsObj EjsNull;

/** 
    Determine if a variable is a null
    @return True if a variable is a null
    @ingroup EjsNull
 */
#define ejsIsNull(vp) ejsIs(vp, ES_Null)

extern EjsNull *ejsCreateNull(Ejs *ejs);

/** 
    Number class
    @description The Number class provide the base class for all numeric values. 
        The primitive number storage data type may be set via the configure program to be either double, float, int
        or int64. 
    @stability Evolving
    @defgroup EjsNumber EjsNumber
    @see EjsNumber ejsToNumber ejsCreateNumber ejsIsNumber ejsGetNumber ejsGetInt ejsGetDouble ejsIsInfinite ejsIsNan
 */
typedef struct EjsNumber {
    EjsObj      obj;                /**< Extends Object - Property storage */
    MprNumber   value;              /**< Numeric value */
} EjsNumber;


/** 
    Create a number object
    @param ejs Ejs reference returned from #ejsCreate
    @param value Numeric value to initialize the number object
    @return A number object
    @ingroup EjsNumber
 */
extern EjsNumber *ejsCreateNumber(Ejs *ejs, MprNumber value);

/** 
    Cast a variable to a number
    @param ejs Ejs reference returned from #ejsCreate
    @param vp Variable to cast
    @return A number object
    @ingroup EjsNumber
 */
extern struct EjsNumber *ejsToNumber(Ejs *ejs, EjsObj *vp);

extern bool ejsIsInfinite(MprNumber f);
#if WIN
#define ejsIsNan(f) (_isnan(f))
#elif MACOSX
    #define ejsIsNan(f) isnan(f)
#elif VXWORKS
    #define ejsIsNan(f) isnan(f)
#else
    #define ejsIsNan(f) (f == FP_NAN)
#endif

/** 
    Reflect Class
    @description The Reflect class permits introspection into the type and attributes of objects and properties.
    @stability Evolving
    @defgroup EjsNamespace EjsNamespace
    @see EjsReflect
 */
typedef struct EjsReflect {
    EjsObj      obj;                /**< Extends Object - Property storage (unused) */
    EjsObj      *subject;           /**< Object under examination */
} EjsReflect;


extern EjsObj *ejsGetTypeName(Ejs *ejs, EjsObj *vp);
extern EjsObj *ejsGetTypeOf(Ejs *ejs, EjsObj *vp);

/** 
    RegExp Class
    @description The regular expression class provides string pattern matching and substitution.
    @stability Evolving
    @defgroup EjsRegExp EjsRegExp
    @see EjsRegExp ejsCreateRegExp ejsIsRegExp
 */
typedef struct EjsRegExp {
    EjsObj          var;                /**< Extends Object - Property storage */
    char            *pattern;           /**< Pattern to match with */
    void            *compiled;          /**< Compiled pattern */
    bool            global;             /**< Search for pattern globally (multiple times) */
    bool            ignoreCase;         /**< Do case insensitive matching */
    bool            multiline;          /**< Match patterns over multiple lines */
    bool            sticky;
    int             options;            /**< Pattern matching options */
    int             endLastMatch;       /**< End of the last match (one past end) */
    int             startLastMatch;     /**< Start of the last match */
    struct EjsString *matched;          /**< Last matched component */
} EjsRegExp;


/** 
    Create a new regular expression object
    @param ejs Ejs reference returned from #ejsCreate
    @param pattern Regular expression pattern string
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
extern EjsRegExp *ejsCreateRegExp(Ejs *ejs, cchar *pattern);

struct EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp);

#if DOXYGEN
    /** 
        Determine if the variable is a regular expression
        @return True if the variable is a regular expression
        @ingroup EjsRegExp
     */
    extern bool ejsIsRegExp(EjsObj *vp);
#else
    #define ejsIsRegExp(vp) ejsIs(vp, ES_RegExp)
#endif

/**
    Socket Class
    @description
    @stability Prototype
    @defgroup EjsSocket EjsSocket
    @see EjsSocket ejsCreateSocket ejsIsSocket
 */
typedef struct EjsSocket {
    EjsObj          obj;                        /**< Extends Object */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsByteArray    *data;                      /**< Buffered write data */
    MprSocket       *sock;                      /**< Underlying MPR socket object */
    MprWaitHandler  waitHandler;                /**< I/O event wait handler */
    Ejs             *ejs;                       /**< Convenience access to ejs interpreter instance */
    char            *address;                   /**< Remote address */
    int             port;                       /**< Remote port */
    int             async;                      /**< In async mode */
    int             mask;                       /**< IO event mask */
    MprMutex        *mutex;                     /**< Multithread sync */
} EjsSocket;

/** 
    Create a new Socket object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Socket object
    @ingroup EjsSocket
 */
extern EjsSocket *ejsCreateSocket(Ejs *ejs);

#if DOXYGEN
    extern bool ejsIsSocket(EjsObj *vp);
#else
    #define ejsIsSocket(vp) ejsIs(vp, ES_ejs_io_Socket)
#endif

/** 
    String Class
    @description The String class provides the base class for all strings. Each String object represents a single 
    immutable linear sequence of characters. Strings have operators for: comparison, concatenation, copying, 
    searching, conversion, matching, replacement, and, subsetting.
    \n\n
    Strings are currently sequences of UTF-8 characters. They will soon be upgraded to UTF-16.
    @stability Evolving
    @defgroup EjsString EjsString
    @see EjsString ejsToString ejsCreateString ejsCreateBareString ejsCreateStringWithLength ejsDupString
        ejsVarToString ejsStrdup ejsStrcat ejsIsString ejsGetString ejsToJson
 */
typedef struct EjsString {
    EjsObj      obj;                /**< Extends Object - Property storage */
    int         length;             /**< String length (sans null) */
    char        *value;             /**< String value. Currently UTF-8. Will upgrade to UTF-16 soon */
} EjsString;


/** 
    Cast a variable to a string
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to convert
    @return A string object
    @ingroup EjsObj
 */
extern EjsString *ejsToString(Ejs *ejs, EjsObj *obj);

/**
    Convert a variable to a string in JSON format
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to cast
    @param options Options object. See serialize for details.
    @return A string object
    @ingroup EjsObj
 */
extern EjsString *ejsToJSON(Ejs *ejs, EjsObj *obj, EjsObj *options);

/** 
    Create a string object
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @stability Prototype
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateString(Ejs *ejs, cchar *value);

/** 
    Create a string object and free the argument
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @stability Prototype
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateStringAndFree(Ejs *ejs, char *value);

/** 
    Create an empty string object
    @param ejs Ejs reference returned from #ejsCreate
    @param len Length of space to reserve for future string data
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateBareString(Ejs *ejs, int len);

/** 
    Create a string and reserve extra room.
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @param len Length of the string storage to allocate.
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateStringWithLength(Ejs *ejs, cchar *value, int len);

//  TODO - why do we need this. Why not just use ejsCloneVar?
/**
    Duplicate a string object
    @param ejs Ejs reference returned from #ejsCreate
    @param sp String value to copy
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsDupString(Ejs *ejs, EjsString *sp);

#if DOXYGEN
    bool ejsIsString(EjsObj *vp);
    extern cchar *ejsGetString(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsString(vp) ejsIs(vp, ES_String)
    #define ejsGetString(ejs, vp) _ejsGetString(ejs, (EjsObj*) (vp))
    extern cchar *_ejsGetString(Ejs *ejs, EjsObj *vp);
#endif

extern int ejsStrdup(MprCtx ctx, uchar **dest, const void *src, int nbytes);
extern int ejsStrcat(Ejs *ejs, EjsString *dest, EjsObj *src);

/** 
    Timer Class
    @description Timers manage the scheduling and execution of Ejscript functions. Timers run repeatedly 
        until stopped by calling the stop method and are scheduled with a granularity of 1 millisecond. 
    @stability Evolving
    @defgroup EjsTimer EjsTimer
    @see EjsTimer
 */
typedef struct EjsTimer {
    EjsObj          obj;                            /**< Extends Object */
    Ejs             *ejs;                           /**< Need interpreter reference in callback */
    MprEvent        *event;                         /**< MPR event for the timer */
    int             drift;                          /**< Timer event is allowed to drift if system conditions requrie */
    int             repeat;                         /**< Timer repeatedly fires */
    int             period;                         /**< Time in msec between invocations */          
    EjsFunction     *callback;                      /**< Callback function */
    EjsFunction     *onerror;                       /**< onerror function */
    EjsArray        *args;                          /**< Callback args */
} EjsTimer;


#define EJS_WORKER_BEGIN        1                   /**< Worker yet to start */
#define EJS_WORKER_STARTED      2                   /**< Worker has started a script */
#define EJS_WORKER_CLOSED       3                   /**< Inside worker has finished */
#define EJS_WORKER_COMPLETE     4                   /**< Worker has completed all messages */

/** 
    Worker Class
    @description The Worker class provides the ability to create new interpreters in dedicated threads
    @stability Prototype
    @defgroup EjsWorker EjsWorker
    @see EjsObj
 */
typedef struct EjsWorker {
    EjsObj          obj;                            /**< Logically extentends Object */
    char            *name;                          /**< Optional worker name */
    Ejs             *ejs;                           /**< Interpreter */
    EjsObj          *event;                         /**< Current event object */
    struct EjsWorker *pair;                         /**< Corresponding worker object in other thread */
    char            *scriptFile;                    /**< Script or module to run */
    char            *scriptLiteral;                 /**< Literal script string to run */
    int             state;                          /**< Worker state */
    int             inside;                         /**< Running inside the worker */
    int             complete;                       /**< Worker has completed its work */
    int             gotMessage;                     /**< Worker has received a message */
    int             terminated;                     /**< Worker has had terminate() called */
} EjsWorker;

extern EjsWorker *ejsCreateWorker(Ejs *ejs);

/** 
    Void class
    @description The Void class provides the base class for the singleton "undefined" instance. This instance is stored
        in ejs->undefinedValue..
    @stability Evolving
    @defgroup EjsVoid EjsVoid
    @see EjsVoid
 */
typedef EjsObj EjsVoid;

extern EjsVoid  *ejsCreateUndefined(Ejs *ejs);
#define ejsIsUndefined(vp) ejsIs(vp, ES_Void)


/*  
    Xml tag state
 */
typedef struct EjsXmlTagState {
    struct EjsXML   *obj;
    //  TODO these two should be XML also
    EjsObj          *attributes;
    EjsObj          *comments;
} EjsXmlTagState;


/*  
    Xml Parser state
 */
typedef struct EjsXmlState {
    //  MOB -- should not be fixed but should be growable
    EjsXmlTagState  nodeStack[EJS_XML_MAX_NODE_DEPTH];
    Ejs             *ejs;
    struct EjsType  *xmlType;
    struct EjsType  *xmlListType;
    int             topOfStack;
    long            inputSize;
    long            inputPos;
    cchar           *inputBuf;
    cchar           *filename;
} EjsXmlState;


/** 
    XML class
    @description The XML class and API is based on ECMA-357 -- ECMAScript for XML (E4X). The XML class is a 
    core class in the E4X specification; it provides the ability to load, parse and save XML documents.
    @stability Evolving
    @defgroup EjsXML EjsXML
    @see EjsXML ejsIsXML ejsConfigureXML ejsCreateXML ejsLoadXMLString ejsDeepCopyXML ejsXMLDescendants
 */
typedef struct EjsXML {
    EjsObj          obj;                /**< Extends Object - Property storage (unused) */
    EjsName         qname;              /**< XML node name (e.g. tagName) */
    int             kind;               /**< Kind of XML node */
    MprList         *elements;          /**< List elements or child nodes */
    MprList         *attributes;        /**< Node attributes */
    MprList         *namespaces;        /**< List of namespaces as Namespace objects */
    struct EjsXML   *parent;            /**< Parent node reference (XML or XMLList) */
    struct EjsXML   *targetObject;      /**< XML/XMLList object modified when items inserted into an empty list */
    EjsName         targetProperty;     /**< XML property modified when items inserted into an empty list */
    char            *value;             /**< String vale of text|attribute|comment|pi */
    int             flags;
} EjsXML;


#if DOXYGEN
    //  MOB -- missing param doc
    /** 
        Determine if a variable is an XML object
        @return true if the variable is an XML or XMLList object
        @ingroup EjsXML
     */
    extern boolean ejsIsXML(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsXML(ejs, vp) (vp && \
        ((((EjsObj*) vp)->type == ejs->xmlType) || (((EjsObj*) vp)->type == ejs->xmlListType)))
#endif

extern EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName *name, EjsXML *parent, cchar *value);
extern void  ejsLoadXMLString(Ejs *ejs, EjsXML *xml, cchar *xmlString);
extern EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, cchar *name, EjsXML *parent, cchar *value);
extern EjsXML *ejsDeepCopyXML(Ejs *ejs, EjsXML *xml);
extern EjsXML *ejsXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName *qname);

/*  
    Xml private prototypes
 */
extern void ejsMarkXML(Ejs *ejs, EjsXML *xml);
extern MprXml *ejsCreateXmlParser(Ejs *ejs, EjsXML *xml, cchar *filename);
extern int ejsXMLToString(Ejs *ejs, MprBuf *buf, EjsXML *xml, int indentLevel);
extern EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *xml, EjsXML *node);
extern EjsXML *ejsSetXML(Ejs *ejs, EjsXML *xml, int index, EjsXML *node);
extern int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *node);
extern EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName *targetProperty);


extern int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener);
extern int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsObj *listener);
extern int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsObj *thisObj, int argc, EjsObj **argv);
extern int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsObj *thisObj, EjsObj *arg);


#if DOXYGEN
    /** 
        Determine if a variable is a number
        @param vp Variable to examine
        @return True if the variable is a number
        @ingroup EjsNumber
     */
    extern bool ejsIsNumber(EjsObj *vp);

    /** 
        Get the numeric value stored in a EjsNumber object
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Variable to examine
        @return A numeric value
        @ingroup EjsNumber
     */
    extern MprNumber ejsGetNumber(Ejs *ejs, EjsObj *vp);

    /** 
        Get the numeric value stored in a EjsNumber object
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Variable to examine
        @return An integer value
        @ingroup EjsNumber
     */
    extern int ejsGetInt(Ejs *ejs, EjsObj *vp);

    /** 
        Get the numeric value stored in a EjsNumber object
        @param ejs Ejs reference returned from #ejsCreate
        @param vp Variable to examine
        @return A double value
        @ingroup EjsNumber
     */
    extern double ejsGetDouble(Ejs *ejs, EjsObj *vp);
#else
    #define ejsIsNumber(vp) ejsIs(vp, ES_Number)
    extern MprNumber _ejsGetNumber(Ejs *ejs, EjsObj *vp);
    extern double _ejsGetDouble(Ejs *ejs, EjsObj *vp);
    extern int _ejsGetInt(Ejs *ejs, EjsObj *vp);
    #define ejsGetNumber(ejs, vp) _ejsGetNumber(ejs, (EjsObj*) (vp))
    #define ejsGetInt(ejs, vp) _ejsGetInt(ejs, (EjsObj*) (vp))
    #define ejsGetDouble(ejs, vp) _ejsGetDouble(ejs, (EjsObj*) (vp))
#endif

/** 
    Type Helpers
    @description The type helpers interface defines the set of primitive operations a type must support to
        interact with the virtual machine.
    @ingroup EjsType
 */
typedef struct EjsTypeHelpers {
    EjsObj  *(*cast)(Ejs *ejs, EjsObj *vp, struct EjsType *type);
    EjsObj  *(*clone)(Ejs *ejs, EjsObj *vp, bool deep);
    EjsObj  *(*create)(Ejs *ejs, struct EjsType *type, int size);
    int     (*defineProperty)(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *qname, struct EjsType *propType, 
                int64 attributes, EjsObj *value);
    void    (*destroy)(Ejs *ejs, EjsObj *vp);
    int     (*deleteProperty)(Ejs *ejs, EjsObj *vp, int slotNum);
    int     (*deletePropertyByName)(Ejs *ejs, EjsObj *vp, EjsName *qname);
    EjsObj  *(*getProperty)(Ejs *ejs, EjsObj *vp, int slotNum);
    EjsObj  *(*getPropertyByName)(Ejs *ejs, EjsObj *vp, EjsName *qname);
    int     (*getPropertyCount)(Ejs *ejs, EjsObj *vp);
    EjsName (*getPropertyName)(Ejs *ejs, EjsObj *vp, int slotNum);
#if UNUSED
    struct EjsTrait *(*getPropertyTrait)(Ejs *ejs, EjsObj *vp, int slotNum);
#endif
    EjsObj  *(*invokeOperator)(Ejs *ejs, EjsObj *vp, int opCode, EjsObj *rhs);
    int     (*lookupProperty)(Ejs *ejs, EjsObj *vp, EjsName *qname);
    void    (*mark)(Ejs *ejs, EjsObj *vp);
    int     (*setProperty)(Ejs *ejs, EjsObj *vp, int slotNum, EjsObj *value);
    int     (*setPropertyByName)(Ejs *ejs, EjsObj *vp, EjsName *qname, EjsObj *value);
    int     (*setPropertyName)(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *qname);
#if UNUSED
    int     (*setPropertyTrait)(Ejs *ejs, EjsObj *vp, int slotNum, struct EjsType *propType, int attributes);
#endif
} EjsTypeHelpers;


typedef EjsObj  *(*EjsCreateHelper)(Ejs *ejs, struct EjsType *type, int size);
typedef void    (*EjsDestroyHelper)(Ejs *ejs, EjsObj *vp);
typedef EjsObj  *(*EjsCastHelper)(Ejs *ejs, EjsObj *vp, struct EjsType *type);
typedef EjsObj  *(*EjsCloneHelper)(Ejs *ejs, EjsObj *vp, bool deep);
typedef int     (*EjsDefinePropertyHelper)(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *qname, struct EjsType *propType, 
                    int64 attributes, EjsObj *value);
typedef int     (*EjsDeletePropertyHelper)(Ejs *ejs, EjsObj *vp, int slotNum);
typedef int     (*EjsDeletePropertyByNameHelper)(Ejs *ejs, EjsObj *vp, EjsName *qname);
typedef EjsObj  *(*EjsGetPropertyHelper)(Ejs *ejs, EjsObj *vp, int slotNum);
typedef EjsObj  *(*EjsGetPropertyByNameHelper)(Ejs *ejs, EjsObj *vp, EjsName *qname);
typedef int     (*EjsGetPropertyCountHelper)(Ejs *ejs, EjsObj *vp);
typedef EjsName (*EjsGetPropertyNameHelper)(Ejs *ejs, EjsObj *vp, int slotNum);
typedef int     (*EjsLookupPropertyHelper)(Ejs *ejs, EjsObj *vp, EjsName *qname);
typedef EjsObj  *(*EjsInvokeOperatorHelper)(Ejs *ejs, EjsObj *vp, int opCode, EjsObj *rhs);
typedef void    (*EjsMarkHelper)(Ejs *ejs, EjsObj *vp);
typedef int     (*EjsSetPropertyByNameHelper)(Ejs *ejs, EjsObj *vp, EjsName *qname, EjsObj *value);
typedef int     (*EjsSetPropertyHelper)(Ejs *ejs, EjsObj *vp, int slotNum, EjsObj *value);
typedef int     (*EjsSetPropertyNameHelper)(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *qname);
#if UNUSED
typedef int     (*EjsSetPropertyTraitHelper)(Ejs *ejs, EjsObj *vp, int slotNum, struct EjsType *propType, int attributes);
typedef struct EjsTrait *(*EjsGetPropertyTraitHelper)(Ejs *ejs, EjsObj *vp, int slotNum);
#endif

/** 
    Type class
    @description Classes in Ejscript are represented by instances of an EjsType. 
        Types are templates for creating instances of the given type, but they are also are runtime accessible objects.
        Types contain the static properties and methods for objects and store these in their object slots array. 
        They store the instance properties in the type->instance object. EjsType inherits from EjsBlock, EjsObj 
        and EjsObj. 
    @stability Evolving
    @defgroup EjsType EjsType
    @see EjsType ejsIsType ejsIsProperty ejsCreateType ejsDefineFunction ejsIsA ejsIsTypeSubType 
        ejsBindMethod ejsDefineInstanceProperty ejsGetType
 */
typedef struct EjsType {
    EjsFunction     constructor;                    /**< Constructor function and type properties */
    EjsName         qname;                          /**< Qualified name of the type. Type name and namespace */
    EjsObj          *prototype;                     /**< Prototype for instances when using prototype inheritance (only) */
    struct EjsType  *baseType;                      /**< Base class */
    MprList         *implements;                    /**< List of implemented interfaces */
        
    uint            callsSuper              :  1;   /**< Constructor calls super() */
    uint            dontPool                :  1;   /**< Don't pool instances */
    uint            dynamicInstance         :  1;   /**< Object instances may add properties */
    uint            final                   :  1;   /**< Type is final */
    uint            hasBaseConstructors     :  1;   /**< Base types has constructors */
    uint            hasBaseInitializers     :  1;   /**< Base types have initializers */
    uint            hasConstructor          :  1;   /**< Type has a constructor */
    uint            hasInitializer          :  1;   /**< Type has static level initialization code */
    uint            hasInstanceVars         :  1;   /**< Type has non-function instance vars (state) */
    uint            hasMeta                 :  1;   /**< Type has meta methods */
    uint            hasScriptFunctions      :  1;   /**< Block has non-native functions requiring namespaces */
    uint            immutable               :  1;   /**< Instances are immutable */
    uint            initialized             :  1;   /**< Static initializer has run */
    uint            isInterface             :  1;   /**< Interface vs class */
    uint            needFinalize            :  1;   /**< Instances need finalization */
    uint            needFixup               :  1;   /**< Slots need fixup */
    uint            numericIndicies         :  1;   /**< Instances support direct numeric indicies */
    uint            virtualSlots            :  1;   /**< Properties are not stored in slots[] */
    
    int             numInherited;                   /**< Number of inherited prototype properties */
    short           id;                             /**< Unique type id */
    ushort          instanceSize;                   /**< Size of instances in bytes */
    EjsTypeHelpers  helpers;                        /**< Type helper methods */
    struct EjsModule *module;                       /**< Module owning the type - stores the constant pool */
    void            *typeData;                      /**< Type specific data */
} EjsType;


#if DOXYGEN
    /** 
        Determine if a variable is an type
        @param vp Variable to test
        @return True if the variable is a type
        @ingroup EjsType
     */
    extern bool ejsIsType(EjsObj *vp);

    /** 
        Determine if a variable is a prototype object. Types store the template for instance properties in a prototype object
        @param vp Variable to test
        @return True if the variable is a prototype object.
        @ingroup EjsType
     */
    extern bool ejsIsPrototype(EjsObj *vp);
#else
    #define ejsIsType(vp)       (vp && (((EjsObj*) (vp))->isType))
    #define ejsIsPrototype(vp)  (vp && (((EjsObj*) (vp))->isPrototype))
#endif

/** 
    Create a new type object
    @description Create a new type object 
    @param ejs Ejs reference returned from #ejsCreate
    @param name Qualified name to give the type. This name is merely referenced by the type and must be persistent.
        This name is not used to define the type as a global property.
    @param up Reference to a module that will own the type. Set to null if not owned by any module.
    @param baseType Base type for this type.
    @param prototype Prototype object instance properties of this type.
    @param size Size of instances. This is the size in bytes of an instance object.
    @param slotNum Slot number that the type will be installed at. This is used by core types to define a unique type ID. 
        For non-core types, set to -1.
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param numInstanceProp Number of instance properties.
    @param attributes Attribute mask to modify how the type is initialized.
    @param data
    @ingroup EjsType EjsType
 */
extern EjsType *ejsCreateType(Ejs *ejs, EjsName *name, struct EjsModule *up, EjsType *baseType, EjsObj *prototype,
    int size, int slotNum, int numTypeProp, int numInstanceProp, int64 attributes, void *data);

extern EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, struct EjsModule *up, EjsType *baseType, 
    int numTypeProp, int numInstanceProp, int64 attributes);
extern void ejsCompleteType(Ejs *ejs, EjsType *type);

extern EjsObj *ejsCreatePrototype(Ejs *ejs, EjsType *type, int numProp);
extern EjsType *ejsCreateArchetype(Ejs *ejs, struct EjsFunction *fun, EjsObj *prototype);

/** 
    Define a global function
    @description Define a global public function and bind it to the C native function. This is a simple one liner
        to define a public global function. The more typical paradigm to define functions is to create a script file
        of native method definitions and and compile it. This results in a mod file that can be loaded which will
        create the function/method definitions. Then use #ejsBindMethod to associate a C function with a property.
    @ingroup EjsType
 */
extern int ejsDefineGlobalFunction(Ejs *ejs, cchar *name, EjsProc fn);


/** 
    Test if an variable is an instance of a given type
    @description Perform an "is a" test. This tests if a variable is a direct instance or subclass of a given base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param target Target variable to test.
    @param type Type to compare with the target
    @return True if target is an instance of "type" or an instance of a subclass of "type".
    @ingroup EjsType
 */
extern bool ejsIsA(Ejs *ejs, EjsObj *target, EjsType *type);

/** 
    Test if a type is a derived type of a given base type.
    @description Test if a type subclasses a base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param target Target type to test.
    @param baseType Base class to see if the target subclasses it.
    @return True if target is a "baseType" or a subclass of "baseType".
    @ingroup EjsType
 */
extern bool ejsIsTypeSubType(Ejs *ejs, EjsType *target, EjsType *baseType);

/** 
    Bind a native C function to a method property
    @description Bind a native C function to an existing javascript method. Method functions are typically created
        by compiling a script file of native method definitions into a mod file. When loaded, this mod file will create
        the method properties. This routine will then bind the specified C function to the method property.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Type containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fn Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
extern int ejsBindMethod(Ejs *ejs, void *obj, int slotNum, EjsProc fn);
extern int ejsBindAccess(Ejs *ejs, void *obj, int slotNum, EjsProc getter, EjsProc setter);
extern void ejsBindConstructor(Ejs *ejs, EjsType *type, EjsProc nativeProc);

/** 
    Define an instance property
    @description Define an instance property on a type. This routine should not normally be called manually. Instance
        properties are best created by creating a script file of native property definitions and then loading the resultant
        mod file.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type in which to create the instance property
    @param slotNum Instance slot number in the type that will hold the property. Set to -1 to allocate the next available
        free slot.
    @param name Qualified name for the property including namespace and name.
    @param propType Type of the instance property.
    @param attributes Integer mask of access attributes.
    @param value Initial value of the instance property.
    @return The slot number used for the property.
    @ingroup EjsType
 */
extern int ejsDefineInstanceProperty(Ejs *ejs, EjsType *type, int slotNum, EjsName *name, EjsType *propType, 
    int attributes, EjsObj *value);

/** 
    Get a type
    @description Get the type installed at the given slot number. All core-types are installed a specific global slots.
        When Ejscript is built, these slots are converted into C program defines of the form: ES_TYPE where TYPE is the 
        name of the type concerned. For example, you can get the String type object via:
        @pre
        ejsGetType(ejs, ES_String)
    @param ejs Interpreter instance returned from #ejsCreate
    @param slotNum Slot number of the type to retrieve. Use ES_TYPE defines. 
    @return A type object if successful or zero if the type could not be found
    @ingroup EjsType
 */
extern EjsType  *ejsGetType(Ejs *ejs, int slotNum);

extern EjsType  *ejsGetTypeByName(Ejs *ejs, cchar *space, cchar *name);

#define VSPACE(space) space "-" BLD_VNUM
#define ejsGetVType(ejs, space, name) ejsGetTypeByName(ejs, space "-" BLD_VNUM, name)

extern int      ejsCompactClass(Ejs *ejs, EjsType *type);
extern int      ejsCopyBaseProperties(Ejs *ejs, EjsType *type, EjsType *baseType);
extern void     ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type);
extern int      ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
extern int      ejsBlendTypeProperties(Ejs *ejs, EjsType *type, EjsType *typeType);
extern int      ejsGetTypePropertyAttributes(Ejs *ejs, EjsObj *vp, int slot);
extern void     ejsInitializeBlockHelpers(EjsTypeHelpers *helpers);

extern void     ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName *qname);
extern void     ejsTypeNeedsFixup(Ejs *ejs, EjsType *type);
extern int      ejsGetTypeSize(Ejs *ejs, EjsType *type);

extern EjsType  *ejsCreateCoreType(Ejs *ejs, EjsName *name, EjsType *extendsType, int size, int slotNum, 
    int numTypeProp, int numInstanceProp, int64 attributes);

extern EjsType  *ejsCreateNativeType(Ejs *ejs, cchar *space, cchar *name, int id, int size);
extern EjsType  *ejsConfigureNativeType(Ejs *ejs, cchar *space, cchar *name, int size);


extern int      ejsBootstrapTypes(Ejs *ejs);
extern void     ejsCreateArrayType(Ejs *ejs);
extern void     ejsCreateBlockType(Ejs *ejs);
extern void     ejsCreateBooleanType(Ejs *ejs);
extern void     ejsCreateErrorType(Ejs *ejs);
extern void     ejsCreateFrameType(Ejs *ejs);
extern void     ejsCreateFunctionType(Ejs *ejs);
extern void     ejsCreateGlobalBlock(Ejs *ejs);
extern void     ejsCreateIteratorType(Ejs *ejs);
extern void     ejsCreateNamespaceType(Ejs *ejs);
extern void     ejsCreateNullType(Ejs *ejs);
extern void     ejsCreateNumberType(Ejs *ejs);
extern void     ejsCreateObjectType(Ejs *ejs);
extern void     ejsCreateRegExpType(Ejs *ejs);
extern void     ejsCreateStringType(Ejs *ejs);
extern void     ejsCreateTypeType(Ejs *ejs);
extern void     ejsCreateVoidType(Ejs *ejs);
extern void     ejsCreateXMLType(Ejs *ejs);
extern void     ejsCreateXMLListType(Ejs *ejs);

/*  
    Native type configuration
 */
extern void     ejsConfigureAppType(Ejs *ejs);
extern void     ejsConfigureArrayType(Ejs *ejs);
extern void     ejsConfigureBooleanType(Ejs *ejs);
extern void     ejsConfigureByteArrayType(Ejs *ejs);
extern void     ejsConfigureConfigType(Ejs *ejs);
extern void     ejsConfigureDateType(Ejs *ejs);
extern void     ejsConfigureSqliteTypes(Ejs *ejs);
extern void     ejsConfigureDebugType(Ejs *ejs);
extern void     ejsConfigureErrorType(Ejs *ejs);
extern void     ejsConfigureEventType(Ejs *ejs);
extern void     ejsConfigureGCType(Ejs *ejs);
extern void     ejsConfigureGlobalBlock(Ejs *ejs);
extern void     ejsConfigureFileType(Ejs *ejs);
extern void     ejsConfigureFileSystemType(Ejs *ejs);
extern void     ejsConfigureFunctionType(Ejs *ejs);
extern void     ejsConfigureHttpType(Ejs *ejs);
extern void     ejsConfigureIteratorType(Ejs *ejs);
extern void     ejsConfigureJSONType(Ejs *ejs);
extern void     ejsConfigureLoggerType(Ejs *ejs);
extern void     ejsConfigureNamespaceType(Ejs *ejs);
extern void     ejsConfigureMemoryType(Ejs *ejs);
extern void     ejsConfigureMathType(Ejs *ejs);
extern void     ejsConfigureNumberType(Ejs *ejs);
extern void     ejsConfigureNullType(Ejs *ejs);
extern void     ejsConfigureObjectType(Ejs *ejs);
extern void     ejsConfigurePathType(Ejs *ejs);
extern void     ejsConfigureReflectType(Ejs *ejs);
extern void     ejsConfigureRegExpType(Ejs *ejs);
extern void     ejsConfigureStringType(Ejs *ejs);
extern void     ejsConfigureSocketType(Ejs *ejs);
extern void     ejsConfigureSystemType(Ejs *ejs);
extern void     ejsConfigureTimerType(Ejs *ejs);
extern void     ejsConfigureTypes(Ejs *ejs);
extern void     ejsConfigureUriType(Ejs *ejs);
extern void     ejsConfigureVoidType(Ejs *ejs);
extern void     ejsConfigureWorkerType(Ejs *ejs);
extern void     ejsConfigureXMLType(Ejs *ejs);
extern void     ejsConfigureXMLListType(Ejs *ejs);

extern void     ejsCreateCoreNamespaces(Ejs *ejs);
extern int      ejsCopyCoreTypes(Ejs *ejs);
extern int      ejsDefineCoreTypes(Ejs *ejs);
extern int      ejsDefineErrorTypes(Ejs *ejs);
extern void     ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType);
extern void     ejsInitializeDefaultHelpers(EjsTypeHelpers *helpers);
extern void     ejsInitializeFunctionHelpers(EjsTypeHelpers *helpers, int all);
extern void     ejsServiceEvents(Ejs *ejs, int timeout, int flags);
extern void     ejsSetSqliteMemCtx(MprThreadLocal *tls, MprCtx ctx);
extern void     ejsSetSqliteTls(MprThreadLocal *tls);

#if BLD_FEATURE_EJS_ALL_IN_ONE || BLD_FEATURE_STATIC
extern int      ejs_events_Init(MprCtx ctx);
extern int      ejs_xml_Init(MprCtx ctx);
extern int      ejs_io_Init(MprCtx ctx);
extern int      ejs_sys_Init(MprCtx ctx);
#if BLD_FEATURE_SQLITE
extern int      ejs_db_sqlite_Init(MprCtx ctx);
#endif
extern int      ejs_web_Init(MprCtx ctx);
#endif

/* 
    Move some ejsWeb.h declarations here so handlers can just include ejs.h whether they are using the
    all-in-one ejs.h or the pure ejs.h
 */
extern HttpStage *ejsAddWebHandler(Http *http);
extern int ejsHostHttpServer(HttpConn *conn);

extern int ejs_db_sqlite_Init(MprCtx ctx);
extern int ejs_web_init(MprCtx ctx);

/**
    VM Evaluation state. 
    The VM Stacks grow forward in memory. A push is done by incrementing first, then storing. ie. *++top = value
    A pop is done by extraction then decrement. ie. value = *top--
    @ingroup EjsVm
 */
typedef struct EjsState {
    struct EjsFrame     *fp;                /* Current Frame function pointer */
    struct EjsBlock     *bp;                /* Current block pointer */
    struct EjsObj       **stack;            /* Top of stack (points to the last element pushed) */
    struct EjsObj       **stackBase;        /* Pointer to start of stack mem */
    struct EjsState     *prev;              /* Previous state */
    struct EjsNamespace *internal;          /* Current internal namespace */
    int                 stackSize;          /* Stack size */
} EjsState;


/**
    Lookup State.
    @description Location information returned when looking up properties.
    @ingroup EjsVm
 */
typedef struct EjsLookup {
    EjsObj          *obj;                   /* Final object / Type containing the variable */
    int             slotNum;                /* Final slot in obj containing the variable reference */
    uint            nthBase;                /* Property on Nth super type -- count from the object */
    uint            nthBlock;               /* Property on Nth block in the scope chain -- count from the end */
    EjsType         *type;                  /* Type containing property (if on a prototype obj) */
#if UNUSED || 1
    uint            useThis;                /* Property accessible via "this." */
    //  MOB -- check all these being used
    uint            instanceProperty;       /* Property is an instance property */
    //  MOB -- check all these being used
    uint            ownerIsType;            /* Original object owning the property is a type */
    //  MOB -- check all these being used
    struct EjsObj   *originalObj;           /* Original object used for the search */
#endif
    struct EjsObj   *ref;                   /* Actual property reference */
    struct EjsTrait *trait;                 /* Property trait describing the property */
    struct EjsName  name;                   /* Name and namespace used to find the property */
    int             bind;                   /* Whether to bind to this lookup */
} EjsLookup;



extern int      ejsSetGeneration(Ejs *ejs, int generation);
extern void     ejsAnalyzeGlobal(Ejs *ejs);
extern int      ejsCreateGCService(Ejs *ejs);
extern void     ejsDestroyGCService(Ejs *ejs);
extern int      ejsIsTimeForGC(Ejs *ejs, int timeTillNextEvent);
extern void     ejsCollectEverything(Ejs *ejs);
extern void     ejsCollectGarbage(Ejs *ejs, int gen);
extern int      ejsEnableGC(Ejs *ejs, bool on);
extern void     ejsTraceMark(Ejs *ejs, struct EjsObj *vp);
extern void     ejsGracefulDegrade(Ejs *ejs);
extern void     ejsPrintAllocReport(Ejs *ejs);
extern void     ejsMakeEternalPermanent(Ejs *ejs);
extern void     ejsMakePermanent(Ejs *ejs, struct EjsObj *vp);
extern void     ejsMakeTransient(Ejs *ejs, struct EjsObj *vp);

#if BLD_DEBUG
extern void     ejsAddToGcStats(Ejs *ejs, struct EjsObj *vp, int id);
#else
#define         ejsAddToGcStats(ejs, vp, id)
#endif

/**
    Ejscript Service structure
    @description The Ejscript service manages the overall language runtime. It 
        is the factory that creates interpreter instances via #ejsCreate.
    @ingroup EjsService
 */
typedef struct EjsService {
    struct EjsObj           *(*loadScriptLiteral)(Ejs *ejs, cchar *script, cchar *cache);
    struct EjsObj           *(*loadScriptFile)(Ejs *ejs, cchar *path, cchar *cache);
    MprHashTable            *nativeModules;
    Http                    *http;
} EjsService;

#define ejsGetAllocCtx(ejs) ejs->currentGeneration

extern EjsService *ejsGetService(MprCtx ctx);
extern int ejsInitCompiler(EjsService *service);
extern void ejsAttention(Ejs *ejs);
extern void ejsClearAttention(Ejs *ejs);

/**
    Open the Ejscript service
    @description One Ejscript service object is required per application. From this service, interpreters
        can be created.
    @param ctx Any memory context returned by mprAlloc
    @return An ejs service object
    @ingroup Ejs
 */
extern EjsService *ejsCreateService(MprCtx ctx);

/**
    Create an ejs virtual machine 
    @description Create a virtual machine interpreter object to evalute Ejscript programs. Ejscript supports multiple 
        interpreters. One interpreter can be designated as a master interpreter and then it can be cloned by supplying 
        the master interpreter to this call. A master interpreter provides the standard system types and clone interpreters 
        can quickly be created an utilize the master interpreter's types. This saves memory and speeds initialization.
    @param ctx Any memory context returned by mprAlloc
    @param master Optional master interpreter to clone.
    @param search Module search path to use. Set to NULL for the default search path.
    @param require Optional list of required modules to load. If NULL, the following modules will be loaded:
        ejs, ejs.io, ejs.events, ejs.xml, ejs.sys and ejs.unix.
    @param argc Count of command line argumements in argv
    @param argv Command line arguments
    @param flags Optional flags to modify the interpreter behavior. Valid flags are:
        @li    EJS_FLAG_COMPILER       - Interpreter will compile code from source
        @li    EJS_FLAG_NO_EXE         - Don't execute any code. Just compile.
        @li    EJS_FLAG_MASTER         - Create a master interpreter
        @li    EJS_FLAG_DOC            - Load documentation from modules
        @li    EJS_FLAG_NOEXIT         - App should service events and not exit unless explicitly instructed
    @return A new interpreter
    @ingroup Ejs
 */
extern Ejs *ejsCreateVm(MprCtx ctx, Ejs *master, cchar *search, MprList *require, int argc, cchar **argv, int flags);

/**
    Create a search path array. This can be used in ejsCreateVm.
    @description Create and array of search paths.
    @param ejs Ejs interpreter
    @param searchPath Search path string. This is a colon (or semicolon on Windows) separated string of directories.
    @return An array of search paths
    @ingroup Ejs
 */
struct EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *searchPath);

/**
    Set the module search path
    @description Set the ejs module search path. The search path is by default set to the value of the EJSPATH
        environment directory. Ejsript will search for modules by name. The search strategy is:
        Given a name "a.b.c", scan for:
        @li File named a.b.c
        @li File named a/b/c
        @li File named a.b.c in EJSPATH
        @li File named a/b/c in EJSPATH
        @li File named c in EJSPATH

    Ejs will search for files with no extension and also search for modules with a ".mod" extension. If there is
    a shared library of the same name with a shared library extension (.so, .dll, .dylib) and the module requires 
    native code, then the shared library will also be loaded.
    @param ejs Ejs interpreter
    @param search Array of search paths
    @ingroup Ejs
 */
extern void ejsSetSearchPath(Ejs *ejs, struct EjsArray *search);
extern void ejsInitSearchPath(Ejs *ejs);

/**
    Evaluate a file
    @description Evaluate a file containing an Ejscript. This requires linking with the Ejscript compiler library (libec). 
    @param path Filename of the script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
extern int ejsEvalFile(cchar *path);

/*
    Flags for LoadScript and compiling
 */
#define EC_FLAGS_BIND            0x1                    /* Bind global references and type/object properties */
#define EC_FLAGS_DEBUG           0x2                    /* Generate symbolic debugging information */
#define EC_FLAGS_MERGE           0x8                    /* Merge all output onto one output file */
#define EC_FLAGS_NO_OUT          0x10                   /* Don't generate any output file */
#define EC_FLAGS_PARSE_ONLY      0x20                   /* Just parse source. Don't generate code */
#define EC_FLAGS_THROW           0x40                   /* Throw errors when compiling. Used for eval() */
#define EC_FLAGS_VISIBLE         0x80                   /* Make global vars visible to all */

//  TODO - DOC
extern int ejsLoadScriptFile(Ejs *ejs, cchar *path, cchar *cache, int flags);
extern int ejsLoadScriptLiteral(Ejs *ejs, cchar *script, cchar *cache, int flags);

/**
    Evaluate a module
    @description Evaluate a module containing compiled Ejscript.
    @param path Filename of the module to evaluate.
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
extern int ejsEvalModule(cchar *path);

/**
    Evaluate a script
    @description Evaluate a script. This requires linking with the Ejscript compiler library (libec). 
    @param script Script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
extern int ejsEvalScript(cchar *script);

/**
    Instruct the interpreter to exit.
    @description This will instruct the interpreter to cease interpreting any further script code.
    @param ejs Interpeter object returned from #ejsCreate
    @param status Reserved and ignored
    @ingroup Ejs
 */
extern void ejsExit(Ejs *ejs, int status);

/**
    Get the hosting handle
    @description The interpreter can store a hosting handle. This is typically a web server object if hosted inside
        a web server
    @param ejs Interpeter object returned from #ejsCreate
    @return Hosting handle
    @ingroup Ejs
 */
extern void *ejsGetHandle(Ejs *ejs);

/**
    Run a script
    @description Run a script that has previously ben compiled by ecCompile
    @param ejs Interpeter object returned from #ejsCreate
    @return Zero if successful, otherwise a non-zero Mpr error code.
 */
extern int ejsRun(Ejs *ejs);

/**
    Throw an exception
    @description Throw an exception object 
    @param ejs Interpeter object returned from #ejsCreate
    @param error Exception argument object.
    @return The exception argument for chaining.
    @ingroup Ejs
 */
extern EjsObj *ejsThrowException(Ejs *ejs, EjsObj *error);
extern void ejsClearException(Ejs *ejs);

/**
    Report an error message using the MprLog error channel
    @description This will emit an error message of the format:
        @li program:line:errorCode:SEVERITY: message
    @param ejs Interpeter object returned from #ejsCreate
    @param fmt Is an alternate printf style format to emit if the interpreter has no valid error message.
    @param ... Arguments for fmt
    @ingroup Ejs
 */
extern void ejsReportError(Ejs *ejs, char *fmt, ...);

extern EjsObj *ejsCastOperands(Ejs *ejs, EjsObj *lhs, int opcode,  EjsObj *rhs);
extern int ejsCheckModuleLoaded(Ejs *ejs, cchar *name);
extern void ejsClearExiting(Ejs *ejs);
extern EjsObj *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs);
extern MprList *ejsGetModuleList(Ejs *ejs);
extern EjsObj *ejsGetVarByName(Ejs *ejs, EjsObj *vp, EjsName *name, EjsLookup *lookup);
extern int ejsInitStack(Ejs *ejs);
extern void ejsLog(Ejs *ejs, cchar *fmt, ...);

extern int ejsLookupVar(Ejs *ejs, EjsObj *vp, EjsName *name, EjsLookup *lookup);
extern int ejsLookupVarWithNamespaces(Ejs *ejs, EjsObj *vp, EjsName *name, EjsLookup *lookup);

extern int ejsLookupScope(Ejs *ejs, EjsName *name, EjsLookup *lookup);
extern void ejsMemoryFailure(MprCtx ctx, int64 size, int64 total, bool granted);
extern int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName);
extern void ejsSetHandle(Ejs *ejs, void *handle);
extern void ejsShowCurrentScope(Ejs *ejs);
extern void ejsShowStack(Ejs *ejs, EjsFunction *fp);
extern void ejsShowBlockScope(Ejs *ejs, EjsBlock *block);
extern int ejsStartMprLogging(Mpr *mpr, char *logSpec);
extern void ejsCloneObjectHelpers(Ejs *ejs, EjsType *type);
extern void ejsCloneBlockHelpers(Ejs *ejs, EjsType *type);
extern int  ejsParseModuleVersion(cchar *name);

extern void ejsLockVm(Ejs *ejs);
extern void ejsUnlockVm(Ejs *ejs);


#ifdef __cplusplus
}
#endif
#endif /* _h_EJS_CORE */

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

/************************************************************************/
/*
 *  End of file "../../src/include/ejsCore.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejsModule.h"
 */
/************************************************************************/

/*
    ejsModule.h - Module file format.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#ifndef _h_EJS_MODULE
#define _h_EJS_MODULE 1


#ifdef __cplusplus
extern "C" {
#endif

/*
    A module file may contain multiple logical modules.

    Module File Format and Layout:

    (N) Numbers are 1-3 little-endian encoded bytes using the 0x80 as the continuation character
    (S) Strings are pointers into the constant pool encoded as number offsets. Strings are UTF-8.
    (T) Types are encoded and ored with the type encoding masks below. Types are either: untyped, 
        unresolved or primitive (builtin). The relevant mask is ored into the lower 2 bits. Slot numbers and
        name tokens are shifted up 2 bits. Zero means untyped.

    ModuleHeader {
        short       magic
        int         fileVersion
        int         version
        int         flags
    }

    Module {
        byte        section
        string      name
        number      version
        number      checksum
        number      constantPoolLength
        block       constantPool
    }

    Dependencies {
        byte        section
        string      moduleName
        number      minVersion
        number      maxVersion
        number      checksum
        byte        flags
    }

    Type {
        byte        section
        string      typeName
        string      namespace
        number      attributes
        number      slot
        type        baseType
        number      numStaticProperties
        number      numInstanceProperties
        number      numInterfaces
        type        Interfaces ...
        ...
    }

    Property {
        byte        section
        string      name
        string      namespace
        number      attributes
        number      slot
        type        property type
    }

    Function {
        byte        section
        string      name
        string      namespace
        number      nextSlotForSetter
        number      attributes
        byte        languageMode
        type        returnType
        number      slot
        number      argCount
        number      defaultArgCount
        number      localCount
        number      exceptionCount
        number      codeLength
        block       code        
    }

    Exception {
        byte        section
        byte        flags
        number      tryStartOffset
        number      tryEndOffset
        number      handlerStartOffset
        number      handlerEndOffset
        number      numOpenBlocks
        type        catchType
    }

    Block {
        byte        section
        string      name
        number      slot
        number      propCount
    }

    Documentation {
        byte        section
        string      text
    }
 */

/*
    Type encoding masks
 */
#define EJS_ENCODE_GLOBAL_NOREF         0x0
#define EJS_ENCODE_GLOBAL_NAME          0x1
#define EJS_ENCODE_GLOBAL_SLOT          0x2
#define EJS_ENCODE_GLOBAL_MASK          0x3

/*
    Fixup kinds
 */
#define EJS_FIXUP_BASE_TYPE             1
#define EJS_FIXUP_INTERFACE_TYPE        2
#define EJS_FIXUP_RETURN_TYPE           3
#define EJS_FIXUP_TYPE_PROPERTY         4
#define EJS_FIXUP_INSTANCE_PROPERTY     5
#define EJS_FIXUP_LOCAL                 6
#define EJS_FIXUP_EXCEPTION             7

/*
    Number encoding uses one bit per byte plus a sign bit in the first byte
 */ 
#define EJS_ENCODE_MAX_WORD             0x07FFFFFF

typedef struct EjsTypeFixup
{
    int              kind;                       /* Kind of fixup */
    int              slotNum;                    /* Slot of target */
    EjsObj           *target;                    /* Target to fixup */
    EjsName          typeName;                   /* Type name */
    int              typeSlotNum;                /* Type slot number */
} EjsTypeFixup;


#define EJS_MODULE_MAGIC        0xC7DA

/*
    Module file format version
 */
#define EJS_MODULE_VERSION      3
#define EJS_VERSION_FACTOR      1000

/*
    Module content version
 */
#define EJS_COMPAT_VERSION(v1, v2) ((v1 / EJS_VERSION_FACTOR) == (v2 / EJS_VERSION_FACTOR))
#define EJS_MAKE_COMPAT_VERSION(version) (version / EJS_VERSION_FACTOR * EJS_VERSION_FACTOR)
#define EJS_MAKE_VERSION(maj, min, patch) (((((maj) * EJS_VERSION_FACTOR) + (min)) * EJS_VERSION_FACTOR) + (patch))
#define EJS_MAJOR(version)      (((version / EJS_VERSION_FACTOR) / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
#define EJS_MINOR(version)      ((version / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
#define EJS_PATCH(version)      (version % EJS_VERSION_FACTOR)
#define EJS_MAX_VERSION         EJS_MAKE_VERSION(EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1)
#define EJS_VERSION             EJS_MAKE_VERSION(BLD_MAJOR_VERSION, BLD_MINOR_VERSION, BLD_PATCH_VERSION)

/*
    Section types
 */
#define EJS_SECT_MODULE         1           /* Module section */
#define EJS_SECT_MODULE_END     2           /* End of a module */
#define EJS_SECT_DEPENDENCY     3           /* Module dependency */
#define EJS_SECT_CLASS          4           /* Class definition */
#define EJS_SECT_CLASS_END      5           /* End of class definition */
#define EJS_SECT_FUNCTION       6           /* Function */
#define EJS_SECT_FUNCTION_END   7           /* End of function definition */
#define EJS_SECT_BLOCK          8           /* Nested block */
#define EJS_SECT_BLOCK_END      9           /* End of Nested block */
#define EJS_SECT_PROPERTY       10          /* Property (variable) definition */
#define EJS_SECT_EXCEPTION      11          /* Exception definition */
#define EJS_SECT_DOC            12          /* Documentation for an element */
#define EJS_SECT_MAX            13

/*
    Psudo section types for loader callback
 */
#define EJS_SECT_START          (EJS_SECT_MAX + 1)
#define EJS_SECT_END            (EJS_SECT_MAX + 2)

/*
    Align headers on a 4 byte boundary
 */
#define EJS_HDR_ALIGN           4

/*
    File format is little-endian. All headers are aligned on word boundaries.
 */
typedef struct EjsModuleHdr {
    int         magic;                      /* Magic number for Ejscript modules */
    int         fileVersion;                /* Module file format version */
    int         flags;                      /* Module flags */
} EjsModuleHdr;

/*
    Structure for the string constant pool
 */
typedef struct EjsConst {
    char        *pool;                      /* Constant pool storage */
    int         size;                       /* Size of constant pool storage */
    int         len;                        /* Length of active constant pool */
    int         base;                       /* Base used during relocations */
    int         locked;                     /* No more additions allowed */
    MprHashTable *table;                    /* Hash table for fast lookup */
} EjsConst;

/*
    Module. NOTE: not an EjsObj
 */
typedef struct EjsModule {
    char            *name;                  /* Name of this module */
    char            *vname;                 /* Versioned name */
    char            *path;                  /* Module file path name */
    int             version;                /* Made with EJS_MAKE_VERSION */
    int             minVersion;             /* Minimum version when used as a dependency */
    int             maxVersion;             /* Maximum version when used as a dependency */
    int             checksum;               /* Checksum of slots and names */

    EjsLoadState    *loadState;             /* State while loading */
    MprList         *dependencies;          /* Module file dependencies. List of EjsModules */
    MprFile         *file;                  /* File handle for loading and code generation */

    /*
        Used during code generation
     */
    struct EcCodeGen *code;                 /* Code generation buffer */
    MprList         *globalProperties;      /* List of global properties */
    EjsFunction     *initializer;           /* Initializer method */

    /*
        Used only while loading modules
     */
    MprList         *current;               /* Current stack of open objects */
    EjsBlock        *scope;                 /* Lexical scope chain */
    EjsConst        *constants;             /* Constant pool */
    int             nameToken;              /* */
    int             firstGlobal;            /* First global property */
    int             lastGlobal;             /* Last global property + 1*/
    struct EjsFunction  *currentMethod;     /* Current method being loaded */

    uint            compiling       : 1;    /* Module currently being compiled from source */
    uint            configured      : 1;    /* Module types have been configured with native code */
    uint            loaded          : 1;    /* Module has been loaded from an external file */
    uint            nativeLoaded    : 1;    /* Backing shared library loaded */
    uint            hasNative       : 1;    /* Has native property definitions */
    uint            hasInitializer  : 1;    /* Has initializer function */
    uint            initialized     : 1;    /* Initializer has run */
    uint            hasError        : 1;    /* Module has a loader error */
    uint            visited         : 1;    /* Module has been traversed */
    int             flags;                  /* Loading flags */
    char            *doc;                   /* Current doc string */
} EjsModule;


typedef int (*EjsNativeCallback)(Ejs *ejs);

typedef struct EjsNativeModule {
    EjsNativeCallback callback;             /* Callback to configure module native types and properties */
    cchar           *name;                  /* Module name */
    int             checksum;               /* Checksum expected by native code */
    int             flags;                  /* Configuration flags */
} EjsNativeModule;

/*
    Documentation string information
    Element documentation string. The loader will create if required.
 */
typedef struct EjsDoc {
    char        *docString;                         /* Original doc string */
    char        *brief;
    char        *description;
    char        *example;
    char        *requires;
    char        *returns;
    char        *stability;                         /* prototype, evolving, stable, mature, deprecated */
    char        *spec;                              /* Where specified */
    int         deprecated;                         /* Hide doc if true */
    int         hide;                               /* Hide doc if true */
    int         cracked;                            /* Doc has been cracked and tokenized */

    struct EjsDoc *duplicate;                       /* From @duplicate directive */

    MprList     *defaults;                          /* Parameter default values */
    MprList     *params;                            /* Function parameters */
    MprList     *options;                           /* Option parameter values */
    MprList     *events;                            /* Option parameter values */
    MprList     *see;
    MprList     *throws;

    EjsTrait    *trait;                             /* Back pointer to trait */
} EjsDoc;


/*
    Loader flags
 */
#define EJS_LOADER_STRICT     0x1
#define EJS_LOADER_NO_INIT    0x2
#define EJS_LOADER_ETERNAL    0x4                   /* Make all types eternal */
#define EJS_LOADER_BUILTIN    0x8                   /* Loading builtins */
#define EJS_LOADER_DEP        0x10                  /* Loading a dependency */



extern int          ejsAddNativeModule(MprCtx ctx, cchar *name, EjsNativeCallback callback, int checksum, int flags);
extern EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, cchar *name);
extern EjsModule    *ejsCreateModule(struct Ejs *ejs, cchar *name, int version);
extern int          ejsLoadModule(struct Ejs *ejs, cchar *name, int minVer, int maxVer, int flags);
extern char         *ejsSearchForModule(Ejs *ejs, cchar *name, int minVer, int maxVer);
extern int          ejsModuleReadName(struct Ejs *ejs, MprFile *file, char **name, int len);
extern int          ejsModuleReadNumber(struct Ejs *ejs, EjsModule *module, int *number);
extern int          ejsModuleReadByte(struct Ejs *ejs, EjsModule *module, int *number);
extern char         *ejsModuleReadString(struct Ejs *ejs, EjsModule *module);
extern int          ejsModuleReadType(struct Ejs *ejs, EjsModule *module, EjsType **typeRef, EjsTypeFixup **fixup, 
                        EjsName *typeName, int *slotNum);
extern int          ejsSetModuleConstants(struct Ejs *ejs, EjsModule *mp, cchar *pool, int poolSize);
extern double       ejsDecodeDouble(Ejs *ejs, uchar **pp);
extern int64        ejsDecodeNum(uchar **pp);
extern int          ejsDecodeWord(uchar **pp);
extern int          ejsEncodeNum(uchar *pos, int64 number);
extern int          ejsEncodeWord(uchar *pos, int number);
extern int          ejsEncodeDouble(Ejs *ejs, uchar *pos, double number);
extern int          ejsEncodeByteAtPos(uchar *pos, int value);
extern int          ejsEncodeUint(uchar *pos, uint number);
extern int          ejsEncodeWordAtPos(uchar *pos, int value);

extern char         *ejsGetDocKey(struct Ejs *ejs, EjsBlock *block, int slotNum, char *buf, int bufsize);
extern EjsDoc       *ejsCreateDoc(struct Ejs *ejs, void *vp, int slotNum, cchar *docString);

extern int          ejsAddModule(Ejs *ejs, struct EjsModule *up);
extern struct EjsModule *ejsLookupModule(Ejs *ejs, cchar *name, int minVersion, int maxVersion);
extern int          ejsRemoveModule(Ejs *ejs, struct EjsModule *up);

#ifdef __cplusplus
}
#endif
#endif /* _h_EJS_MODULE */

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
/************************************************************************/
/*
 *  End of file "../../src/include/ejsModule.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejs.h"
 */
/************************************************************************/

/*
    ejs.h - Ejscript top level header.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#ifndef _h_EJS_h
#define _h_EJS_h 1

#include    "mpr.h"

#endif /* _h_EJS_h */

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
/************************************************************************/
/*
 *  End of file "../../src/include/ejs.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejsWeb.h"
 */
/************************************************************************/

/**
    ejsWeb.h -- Header for the Ejscript Web Framework
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#ifndef _h_EJS_WEB_h
#define _h_EJS_WEB_h 1



#define EJS_SESSION             "-ejs-session-"

#ifdef  __cplusplus
extern "C" {
#endif


#ifndef EJS_HTTPSERVER_NAME
#define EJS_HTTPSERVER_NAME "ejs-http/" BLD_VERSION
#endif

/** 
    HttpServer Class
    @description
        HttpServer objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttpServer EjsHttpServer
    @see EjsHttpServer
 */
typedef struct EjsHttpServer {
    EjsObj          obj;                        /**< Extends Object */
    Ejs             *ejs;                       /**< Ejscript interpreter handle */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsObj          *limits;                    /**< Limits object */
    HttpServer      *server;                    /**< Http server object */
    struct MprSsl   *ssl;                       /**< SSL configuration */
    EjsArray        *incomingStages;            /**< Incoming Http pipeline stages */
    EjsArray        *outgoingStages;            /**< Outgoing Http pipeline stages */
    HttpTrace       trace[2];                   /**< Default tracing for requests */
    cchar           *connector;                 /**< Pipeline connector */
    cchar           *dir;                       /**< Directory containing web documents */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    char            *protocols;                 /**< SSL protocols */
    char            *ciphers;                   /**< SSL ciphers */
    char            *ip;                        /**< Listening address */
    char            *name;                      /**< Server name */
    int             port;                       /**< Listening port */
    int             async;                      /**< Async mode */
} EjsHttpServer;


/** 
    Request Class
    @description
        Request objects represent a single Http request.
    @stability Prototype
    @defgroup EjsRequest EjsRequest
    @see EjsRequest ejsCreateRequest
 */
typedef struct EjsRequest {
    EjsObj          obj;                /**< Base object storage */
    EjsObj          *cookies;           /**< Cached cookies */
    HttpConn        *conn;              /**< Underlying Http connection object */
    EjsHttpServer   *server;            /**< Owning server */

    EjsObj          *absHome;           /**< Absolute URI to the home of the application from this request */
    EjsObj          *emitter;           /**< Event emitter */
    EjsPath         *dir;               /**< Home directory containing the application */
    EjsObj          *env;               /**< Request.env */
    EjsPath         *filename;          /**< Physical resource filename */
    EjsObj          *files;             /**< Files object */
    EjsObj          *headers;           /**< Headers object */
    EjsUri          *home;              /**< Relative URI to the home of the application from this request */
    EjsObj          *host;              /**< Host property */
    EjsObj          *limits;            /**< Limits object */
    EjsObj          *log;               /**< Log object */
    EjsObj          *originalMethod;    /**< Saved original method */
    EjsObj          *originalUri;       /**< Saved original URI */
    EjsObj          *params;            /**< Form variables */
    EjsObj          *pathInfo;          /**< PathInfo property */
    EjsObj          *port;              /**< Port property */
    EjsObj          *query;             /**< Query property */
    EjsObj          *reference;         /**< Reference property */
    EjsObj          *responseHeaders;   /**< Headers object */
    EjsObj          *scheme;            /**< Scheme property */
    EjsObj          *scriptName;        /**< ScriptName property */
    EjsObj          *uri;               /**< Complete uri */

    Ejs             *ejs;               /**< Ejscript interpreter handle */
    struct EjsSession *session;         /**< Current session */

    int             accepted;           /**< Request has been accepted from the HttpServer */
    int             dontFinalize;       /**< Don't auto-finalize. Must call finalize(force) */
    int             probedSession;      /**< Determined if a session exists */
    int             closed;             /**< Request closed and "close" event has been issued */
    int             error;              /**< Request errored and "error" event has been issued */
    int             running;            /**< Request has started */
} EjsRequest;


/** 
    Create a new request. Create a new request object associated with the given Http connection.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server EjsHttpServer owning this request
    @param conn Http connection object
    @param dir Default directory containing web documents
    @return A new request object.
*/
extern EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir);

/** 
    Clone a new request. This is used to clone a request from one interpreter into another.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Original request to copy
    @param deep Ignored
    @return A new request object.
*/
extern EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep);

/** 
    Session Class
    @description
        Session objects represent a shared session state object into which Http Requests and store and retrieve data
        that persists beyond a single request.
    @stability Prototype
    @defgroup EjsSession EjsSession
    @see EjsSession ejsCreateSession ejsDestroySession
 */
typedef struct EjsSession
{
    EjsObj      obj;
    MprTime     expire;             /* When the session should expire */
    cchar       *id;                /* Session ID */
    int         timeout;            /* Session inactivity lifespan */
    int         index;              /* Index in sesssions[] */
} EjsSession;

/** 
    Create a session object
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Request object creating the session
    @param timeout Timeout in milliseconds
    @param secure If the session is to be given a cookie that is designated as only for secure sessions (SSL)
    @returns A new session object.
*/
extern EjsSession *ejsCreateSession(Ejs *ejs, struct EjsRequest *req, int timeout, bool secure);
extern EjsSession *ejsGetSession(Ejs *ejs, struct EjsRequest *req);

/** 
    Destroy as session. This destroys the session object so that subsequent requests will need to establish a new session.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server Server object owning the session.
    @param session Session object created via ejsCreateSession()
*/
extern int ejsDestroySession(Ejs *ejs, EjsHttpServer *server, EjsSession *session);

extern void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req);
extern void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req);


extern void ejsConfigureHttpServerType(Ejs *ejs);
extern void ejsConfigureRequestType(Ejs *ejs);
extern void ejsConfigureSessionType(Ejs *ejs);
extern void ejsConfigureWebTypes(Ejs *ejs);

#ifdef  __cplusplus
}
#endif
#endif /* _h_EJS_WEB_h */

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
/************************************************************************/
/*
 *  End of file "../../src/include/ejsWeb.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ecCompiler.h"
 */
/************************************************************************/

/*
    ecCompiler.h - Internal compiler header.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */


#ifndef _h_EC_COMPILER
#define _h_EC_COMPILER 1


/*
    Compiler validation modes. From "use standard|strict"
 */
#define PRAGMA_MODE_STANDARD    1               /* Standard unstrict mode */
#define PRAGMA_MODE_STRICT      2               /* Strict mode */

#define STRICT_MODE(cp)         (cp->fileState->strict)

/*
    Variable Kind bits
 */
#define KIND_CONST              0x1
#define KIND_VAR                0x2
#define KIND_LET                0x4

/*
    Phases for AST processing
 */
#define EC_PHASE_DEFINE         0           /* Define types, functions and properties in types */
#define EC_PHASE_CONDITIONAL    1           /* Do conditional processing, hoisting and then type fixups */
#define EC_PHASE_FIXUP          2           /* Fixup type references */
#define EC_PHASE_BIND           3           /* Bind var references to slots and property types */
#define EC_AST_PHASES           4

/*
    Ast node define
 */
#if !DOXYGEN
typedef struct EcNode   *Node;
#endif

typedef struct EcNode {
    //  TODO - need consistency in naming. Nodes should have a node suffix. 
    //  References EjsObj should have a Ref suffix.
    /*
        Ordered for debugging
     */
    char                *kindName;          /* Node kind string */

    /*
        TODO - optimize name+qualifier fields
     */
    EjsName             qname;
    int                 literalNamespace;   /* Namespace is a literal */

    cchar               *filename;          /* File containing source code line */
    char                *currentLine;       /* Current source code line */
    int                 lineNumber;

    Node                left;               /* children[0] */
    Node                right;              /* children[1] */

    EjsBlock            *blockRef;          /* Block scope */
    bool                createBlockObject;  /* Create the block object to contain let scope variables */
    bool                blockCreated;       /* Block object has been created */
    EjsLookup           lookup;
    int                 attributes;         /* Attributes applying to this node */
    bool                specialNamespace;   /* Using a public|private|protected|internal namespace */
    Node                typeNode;           /* Type of name */
    Node                parent;             /* Parent node */
    EjsNamespace        *namespaceRef;      /* Namespace reference */
    MprList             *namespaces;        /* Namespaces for hoisted variables */

    #define N_VALUE     1

#if BLD_DEBUG
    char                *tokenName;
#endif

    int                 tokenId;            /* Lex token */
    int                 groupMask;          /* Token group */
    int                 subId;              /* Sub token */

    /*
        MOB TODO - order in most useful order: name, type, function, binary
        MOB TODO - rename all with a Node suffix. ie. nameNode, importNode
     */

    //  MOB TODO - disable for now
#if !BLD_DEBUG && 0
    union {
#endif

        //  GROUPING ONLY
        #define N_ATTRIBUTES 50

        /*
            Name nodes hold a fully qualified name.
         */
        #define N_QNAME  3
        #define N_VAR 63
        struct {
            Node        nameExpr;           /* Name expression */
            Node        qualifierExpr;      /* Qualifier expression */
            int         isAttribute;        /* Attribute identifier "@" */
            int         isType;             /* Name is a type */
            int         isNamespace;        /* Name is a namespace */
            int         letScope;           /* Variable is defined in let block scope */
            int         instanceVar;        /* Instance or static var (if defined in class) */
            int         isRest;             /* ... rest style args */
            int         varKind;            /* Variable definition kind */
            EjsObj      *nsvalue;           /* Initialization value (MOB - remove) */
        } name;

        /*
            cast, is, to
         */
        #define N_BINARY_TYPE_OP  5

        /*
           + - * / % << >> >>> & | ^ && || instanceof in == != ===
                !== < <= > >= , ]
         */
        #define N_BINARY_OP  6
        #define N_ASSIGN_OP  7

        struct {
            struct EcCodeGen    *rightCode;
        } binary;


        #define N_FOR_IN 33
        struct {
            int         each;                       /* For each used */
            Node        iterVar;
            Node        iterGet;
            Node        iterNext;
            Node        body;
            struct EcCodeGen    *initCode;
            struct EcCodeGen    *bodyCode;
        } forInLoop;


        /*
            Used by for and while
         */
        #define N_FOR   32
        struct {
            Node        initializer;
            Node        cond;
            Node        perLoop;
            Node        body;
            struct EcCodeGen    *condCode;
            struct EcCodeGen    *bodyCode;
            struct EcCodeGen    *perLoopCode;
        } forLoop;


        #define N_DO    51
        struct {
            Node        cond;
            Node        body;
            struct EcCodeGen    *condCode;
            struct EcCodeGen    *bodyCode;
        } doWhile;


        /*  OPT - convert to grouping  */
        #define N_UNARY_OP  9
        struct {
            int         dummy;
        } unary;


        #define N_IF  10
        struct {
            Node                cond;
            Node                thenBlock;
            Node                elseBlock;
            struct EcCodeGen    *thenCode;
            struct EcCodeGen    *elseCode;
        } tenary;


        #define N_HASH  55
        struct {
            Node        expr;
            Node        body;
            bool        disabled;
        } hash;


        #define N_VAR_DEFINITION  11
        /*
            Var definitions have one child per variable. Child nodes can
            be either N_NAME or N_ASSIGN_OP
         */
        struct {
            int         varKind;            /* Variable definition kind */
        } def;


        //  TODO - split apart into separate AST nodes.
        /*
           Pragmas: use strict, use standard
         */
        #define N_PRAGMA  12
        struct {
            Node        decimalContext;     /* use decimal expr */
            uint        strict;             /* Strict mode */
            char        *moduleName;        /* Module name value */
        } pragma;


        /*
            Module defintions
         */
        #define N_MODULE 52
        struct {
            EjsModule   *ref;               /* Module object */
            char        *filename;          /* Module file name */
            char        *name;              /* Module name */
            int         version;
        } module;


        /*
            Use module
         */
        #define N_USE_MODULE 53
        struct {
            int         minVersion;
            int         maxVersion;
        } useModule;


        /*
            use namespace, use default namespace
         */
        #define N_USE_NAMESPACE  49
        struct {
            int         isDefault;          /* "use default" */
            int         isLiteral;          /* use namespace "literal" */
            int         isInternal;         /* internal namespace */
        } useNamespace;


        #define N_FUNCTION 14
        struct {
            uint        operatorFn : 1;     /* operator function */
            uint        getter : 1;         /* getter function */
            uint        setter : 1;         /* setter function */
            uint        call : 1;           /* */
            uint        has : 1;            /* */
            uint        hasRest : 1;        /* Has rest parameter */
            uint        hasReturn : 1;      /* Has a return statement */
            uint        isMethod : 1;       /* Is a class method */
            uint        isConstructor : 1;  /* Is constructor method */
            uint        isDefaultConstructor : 1;/* Is default constructor */
            Node        resultType;         /* Function return type */
            Node        body;               /* Function body */
            Node        parameters;         /* Function formal parameters */
            Node        constructorSettings;/* Constructor settings */

            /*
                Bound definition
             */
            EjsFunction *functionVar;       /* Function variable */
            Node        expressionRef;      /* Reference to the function expression name */

        } function;

        #define N_END_FUNCTION 40

        #define N_PARAMETER 15
        struct {
            char        *type;              /* Parameter type */
            char        *value;             /* Default value */
            int         isRest : 1;         /* Is rest parameter */
        } parameter;


        /*
            Body stored in the child node
         */
        #define N_CLASS  16
        struct {
            char        *extends;           /* Class base class */
            Node        implements;         /* Implemented interfaces */
            MprList     *staticProperties;  /* List of static properties */
            MprList     *instanceProperties;/* Implemented interfaces */
            MprList     *classMethods;      /* Static methods */
            MprList     *methods;           /* Instance methods */
            Node        constructor;        /* Class constructor */
            int         isInterface;        /* This is an interface */

            EjsType     *ref;               /* Type instance ref */
            EjsFunction *initializer;       /* Initializer function */

            EjsNamespace *publicSpace;
            EjsNamespace *internalSpace;

        } klass;


        #define N_DIRECTIVES 18

        #define N_SUPER 35
        struct {
            int         dummy;
        } super;


        #define N_NEW 31
        struct {
            int         callConstructors;   /* Bound type has a constructor */
        } newExpr;


        #define N_TRY 36
        struct {
            /* Children are the catch clauses */
            Node        tryBlock;           /* Try code */
            Node        catchClauses;       /* Catch clauses */
            Node        finallyBlock;       /* Finally code */
            int         numBlocks;          /* Count of open blocks in the try block */
        } exception;


        #define N_CATCH 37
        struct {
            Node        arg;                /* Catch block argument */
        } catchBlock;


        #define N_CALL 28
        struct {
            int         dummmy;
        } call;


        #define N_PROGRAM 20
        struct {
            MprList     *dependencies;      /* Accumulated list of dependent modules */
        } program;


        /*
            Block kinds
         */
        #define EC_CLASS_BLOCK      1
        #define EC_FUNCTION_BLOCK   2
        #define EC_INTERFACE_BLOCK  3
        #define EC_NESTED_BLOCK     4
        #define EC_GLOBAL_BLOCK     5
        #define EC_MODULE_BLOCK     6

        #define N_BLOCK 25

        //  MOB -- what does this actually do? - why not just use children?
        #define N_REF 42
        struct {
            Node        node;               /* Actual node reference */
        } ref;


        #define N_SWITCH 43
        struct {
            int         dummy;
        } switchNode;


        #define EC_SWITCH_KIND_CASE     1   /* Case block */
        #define EC_SWITCH_KIND_DEFAULT  2   /* Default block */

        #define N_CASE_LABEL 44
        struct {
            Node                expression;
            int                 kind;
            struct EcCodeGen    *expressionCode;    /* Code buffer for the case expression */
            int                 nextCaseCode;       /* Goto length to the next case statements */
        } caseLabel;

        #define N_THIS 30

        #define N_THIS_GENERATOR    1
        #define N_THIS_CALLEE       2
        #define N_THIS_TYPE         3
        #define N_THIS_FUNCTION     4

        struct {
            int                 thisKind;   /* Kind of this. See N_THIS_ flags */
        } thisNode;

        #define N_CASE_ELEMENTS 45
        #define N_BREAK         46
        #define N_CONTINUE      47
        #define N_GOTO          48

        #define N_LITERAL       2
        struct {
            EjsObj              *var;       /* Special value */
            MprBuf              *data;      /* XML data */
        } literal;

        #define N_OBJECT_LITERAL    56      /* Array or object literal */
        #define N_DASSIGN 62                /* Destructuring assignment */
        struct {
            Node                typeNode;   /* Type of object */
            int                 isArray;    /* Array literal */
        } objectLiteral;

        /*
            Object (and Array) literal field
         */
        #define FIELD_KIND_VALUE        0x1
        #define FIELD_KIND_FUNCTION     0x2

        #define N_FIELD 57
        struct {
            int                 fieldKind;  /* value or function */
            int                 varKind;    /* Variable definition kind (const) */
            Node                expr;       /* Field expression */
            Node                fieldName;  /* Field element name for objects */
            int                 index;      /* Array index, set to -1 for objects */
        } field;

        #define N_WITH 60
        struct {
            Node                object;
            Node                statement;
        } with;

        #define N_RETURN 27
        struct {
            int                 blockLess;  /* Block-less function expression */
        } ret;

        #define N_EXPRESSIONS 22
        struct {
            char                *space;      /* Namespace qualifier */
        } expr;
#if !BLD_DEBUG && 0
    };
#endif

    int                 kind;               /* Kind of node */
    MprList             *children;

    struct EcCompiler   *cp;                /* Compiler instance reference */

    int                 column;             /* Column of token in currentLine */

    /*
        Bound definition. Applies to names and expression values.
     */

    int                 slotFixed;          /* Slot fixup has been done */
    int                 needThis;           /* Need to push this object */
    int                 needDupObj;         /* Need to dup the object on stack (before) */
    int                 needDup;            /* Need to dup the result (after) */
    int                 slotNum;            /* Allocated slot for variable */
    int                 enabled;            /* Node is enabled via conditional definitions */

    uchar               *patchAddress;      /* For code patching */
    struct EcCodeGen    *code;              /* Code buffer */
    int                 jumpLength;         /* Goto length for exceptions */

    int                 seqno;              /* Unique sequence number */
    EjsName             *globalProp;        /* Set if this is a global property */
    char                *doc;               /* Documentation string */
} EcNode;


/*
    Grouping node types
 */
/* 21 - unused */
#define N_PRAGMAS 23
#define N_TYPE_IDENTIFIERS 24
#define N_DOT 26
#define N_ARGS 29
/* N_new 31 */
/* N_for 32 */
/* N_forIn 33 */
#define N_POSTFIX_OP 34
/* N_try 36 */
/* N_catch 37 */
#define N_CATCH_CLAUSES 38
#define N_THROW 39
#define N_NOP 41
#define N_VOID 54
/* N_HASH 55 */
/* N_OBJECT_LITERAL 56 */
/* N_FIELD 57 */
#define N_ARRAY_LITERAL_UNUSED 58
#define N_CATCH_ARG 59
#define N_WITH 60
#define N_SPREAD 61
/* N_DASSIGN 62 */
/* N_VAR 63 */

#define EC_NUM_NODES                    8
#define EC_TAB_WIDTH                    4

/*
    Fix clash with arpa/nameser.h
 */
#undef T_NULL

#if UNUSED && MOVED
/*
    Flags for ecCompile()
 */
#define EC_FLAGS_BIND            0x1                    /* Bind global references and type/object properties */
#define EC_FLAGS_DEBUG           0x2                    /* Generate symbolic debugging information */
#define EC_FLAGS_MERGE           0x8                    /* Merge all output onto one output file */
#define EC_FLAGS_NO_OUT          0x10                   /* Don't generate any output file */
#define EC_FLAGS_PARSE_ONLY      0x20                   /* Just parse source. Don't generate code */
#define EC_FLAGS_THROW           0x40                   /* Throw errors when compiling. Used for eval() */
#endif

/*
    Lexical tokens (must start at 1)
    ASSIGN tokens must be +1 compared to their non-assignment counterparts.
 */
#define T_AS 1
#define T_ASSIGN 2
#define T_AT 3
#define T_ATTRIBUTE 4
#define T_BIT_AND 5
#define T_BIT_AND_ASSIGN 6
#define T_BIT_OR 7
#define T_BIT_OR_ASSIGN 8
#define T_BIT_XOR 9
#define T_BIT_XOR_ASSIGN 10
#define T_BREAK 11
#define T_CALL 12
#define T_CASE 13
#define T_CAST 14
#define T_CATCH 15
#define T_CLASS 16
/* TODO - remove and resequence #define T_CLOSE_TAG 17 */
#define T_COLON 18
#define T_COLON_COLON 19
#define T_COMMA 20
#define T_CONST 21
#define T_CONTEXT_RESERVED_ID 22
#define T_CONTINUE 23
#define T_DEBUGGER 24
#define T_DECIMAL 25                    //  UNUSED
#define T_DECREMENT 26
#define T_DEFAULT 27
#define T_DELETE 28
#define T_DIV 29
#define T_DIV_ASSIGN 30
#define T_DO 31
#define T_DOT 32
#define T_DOT_DOT 33
#define T_DOT_LESS 34
#define T_DOUBLE 35
#define T_DYNAMIC 36
#define T_EACH 37
#define T_ELIPSIS 38
#define T_ELSE 39
#define T_ENUMERABLE 40
#define T_EOF 41
#define T_EQ 42
#define T_EXTENDS 43
#define T_FALSE 44
#define T_FINAL 45
#define T_FINALLY 46
#define T_FLOAT 47
#define T_FOR 48
#define T_FUNCTION 49
#define T_GE 50
#define T_GET 51
#define T_GOTO 52
#define T_GT 53
#define T_ID 54
#define T_IF 55
#define T_IMPLEMENTS 56
#define T_IMPORT 57
#define T_IN 58
#define T_INCLUDE 59
#define T_INCREMENT 60
#define T_INSTANCEOF 61
#define T_INT 62
#define T_INTERFACE 64
#define T_INTERNAL 65
#define T_INTRINSIC 66
#define T_IS 67
#define T_LBRACE 68
#define T_LBRACKET 69
#define T_LE 70
#define T_LET 71
#define T_LOGICAL_AND 72
#define T_LOGICAL_AND_ASSIGN 73
#define T_LOGICAL_NOT 74
#define T_LOGICAL_OR 75
#define T_LOGICAL_OR_ASSIGN 76
#define T_LOGICAL_XOR 77
#define T_LOGICAL_XOR_ASSIGN 78
#define T_LPAREN 79
#define T_LSH 80
#define T_LSH_ASSIGN 81
#define T_LT 82
#define T_MINUS 83
#define T_MINUS_ASSIGN 84
#define T_MINUS_MINUS 85
#define T_MOD 86
#define T_MOD_ASSIGN 87
#define T_MODULE 88
#define T_MUL 89
#define T_MUL_ASSIGN 90
#define T_NAMESPACE 91
#define T_NATIVE 92
#define T_NE 93
#define T_NEW 94
#define T_NULL 95
#define T_NUMBER 96
/* TODO - remove and resequence #define T_OPEN_TAG 97 */
#define T_OVERRIDE 98
//  TODO #define    T_PACKAGE 99
#define T_PLUS 100
#define T_PLUS_ASSIGN 101
#define T_PLUS_PLUS 102
#define T_PRIVATE 103
#define T_PROTECTED 104
#define T_PROTOTYPE 105
#define T_PUBLIC 106
#define T_QUERY 107
#define T_RBRACE 108
#define T_RBRACKET 109
#define T_READONLY 110
#define T_RETURN 111
#define T_ROUNDING 112
#define T_RPAREN 113
#define T_RSH 114
#define T_RSH_ASSIGN 115
#define T_RSH_ZERO 116
#define T_RSH_ZERO_ASSIGN 117
#define T_SEMICOLON 118
#define T_SET 119
#define T_RESERVED_NAMESPACE 120
#define T_STANDARD 121
#define T_STATIC 122
#define T_STRICT 123
#define T_STRICT_EQ 124
#define T_STRICT_NE 125
#define T_STRING 126
#define T_SUPER 127
#define T_SWITCH 128
#define T_SYNCHRONIZED 129
#define T_THIS 130
#define T_THROW 131
#define T_TILDE 132
#define T_TO 133
#define T_TRUE 134
#define T_TRY 135
#define T_TYPE 136
#define T_TYPEOF 137
#define T_UINT 138
#define T_USE 139
#define T_VAR 140
#define T_VOID 141
#define T_WHILE 142
#define T_WITH 143
#define T_XML 144
#define T_YIELD 145
#define T_EARLY_TODO_REMOVED 146
#define T_ENUM 147
#define T_HAS 148
#define T_PRECISION 149
#define T_UNDEFINED 150
#define T_BOOLEAN 151
#define T_LONG 152
#define T_VOLATILE 153
#define T_ULONG 154
#define T_HASH 155
#define T_ABSTRACT 156
#define T_CALLEE 157
#define T_GENERATOR 158
#define T_NUMBER_WORD 159
#define T_XML_COMMENT_START 161
#define T_XML_COMMENT_END 162
#define T_CDATA_START 163
#define T_CDATA_END 164
#define T_XML_PI_START 165
#define T_XML_PI_END 166
#define T_LT_SLASH 167
#define T_SLASH_GT 168
#define T_LIKE 169
#define T_REGEXP 170
#define T_REQUIRE 171
#define T_SHARED 172
#define T_NOP 174
#define T_ERR 175

/*
    Group masks
 */
#define G_RESERVED          0x1
#define G_CONREV            0x2
#define G_COMPOUND_ASSIGN   0x4                 /* Eg. <<= */
#define G_OPERATOR          0x8                 /* Operator overload*/

/*
    Attributes (including reserved namespaces)
 */
#define A_FINAL         0x1
#define A_OVERRIDE      0x2
#define A_EARLY         0x4                     /* Early binding */
#define A_DYNAMIC       0x8
#define A_NATIVE        0x10
#define A_PROTOTYPE     0x20
#define A_STATIC        0x40
#define A_ENUMERABLE    0x40

#define EC_INPUT_STREAM "__stdin__"

struct EcStream;
typedef int (*EcStreamGet)(struct EcStream *stream);

/*
    Stream flags
 */
#define EC_STREAM_EOL       0x1                 /* End of line */


typedef struct EcStream {
    char        *name;                          /* Stream name / filename */

    char        *currentLine;                   /* Current input source line */
    int         lineNumber;                     /* Line number in source of current token */
    int         column;                         /* Current reading position */

    char        *lastLine;                      /* Save last currentLine */
    int         lastColumn;                     /* Save last column length for putBack */

    /*
        In-memory copy if input source
     */
    char        *buf;                           /* Buffer holding source file */
    char        *nextChar;                      /* Ptr to next input char */
    char        *end;                           /* Ptr to one past end of buf */

    bool        eof;                            /* At end of file */

    int         flags;                          /* Input flags */
    EcStreamGet gets;                           /* Stream get another characters */

    struct EcCompiler *compiler;                /* Compiler back reference */

} EcStream;


/*
    Parse source code from a file
 */
typedef struct EcFileStream {
    EcStream    stream;
    MprFile     *file;
} EcFileStream;



/*
    Parse source code from a memory block
 */
typedef struct EcMemStream {
    EcStream    stream;
} EcMemStream;



/*
    Parse input from the console (or file if using ejsh)
 */
typedef struct EcConsoleStream {
    EcStream    stream;
    MprBuf      *inputBuffer;                   /* Stream input buffer */
} EcConsoleStream;



/*
    Program input tokens
 */
typedef struct EcToken {
    int         tokenId;
    int         subId;
    int         groupMask;

    //  MOB -- convert from uchar
    uchar       *text;                          /* Token text */
    int         textLen;                        /* Length of text */
    int         textBufSize;                    /* Size of text buffer */

    EjsObj   *number;                        /* Any numeric literals */

    char        *filename;
    char        *currentLine;
    int         lineNumber;
    int         column;
    int         eol;                            /* At the end of the line */

    EcStream    *stream;                        /* Current input stream */
    struct EcToken *next;                       /* Putback linkage */
} EcToken;



/*
    Input token parsing state. Includes putback stack for N lookahead.
 */
typedef struct EcInput {
    EcStream    *stream;

    int         state;                  /* Lexer state */

    EcToken     *putBack;               /* List of putback tokens */
    EcToken     *token;                 /* Current token */
    EcToken     *freeTokens;            /* Free list of tokens */

    char        *doc;                   /* Last doc token */

    struct EcLexer *lexer;              /* Owning lexer */
    struct EcInput *next;               /* List of input streams */
    struct EcCompiler *compiler;        /* Reference to compiler */
} EcInput;



typedef struct EcLexer {
    MprHashTable        *keywords;
    EcInput             *input;         /* List of input streams */
    struct EcCompiler   *compiler;      /* Owning compiler */
} EcLexer;



/*
    Jump types
 */
#define EC_JUMP_BREAK       0x1
#define EC_JUMP_CONTINUE    0x2
#define EC_JUMP_GOTO        0x4

typedef struct EcJump {
    int             kind;               /* Break, continue */
    int             offset;             /* Code offset to patch */
    EcNode          *node;              /* Owning node */
} EcJump;


/*
    Structure for code generation buffers
 */
typedef struct EcCodeGen {
    MprBuf      *buf;                   /* Code generation buffer */
    MprList     *jumps;                 /* Break/continues to patch for this code block */
    MprList     *exceptions;            /* Exception handlers for this code block */
    int         jumpKinds;              /* Kinds of jumps allowed */
    int         breakMark;              /* Stack item counter for the target for break/continue stmts */
    int         blockMark;              /* Lexical block counter for the target for break/continue stmts */
    int         stackCount;             /* Current stack item counter */
    int         blockCount;             /* Current block counter */
} EcCodeGen;


/*
    Current parse state. Each non-terminal production has its own state.
    Some state fields are inherited. We keep a linked list from EcCompiler.
 */
typedef struct EcState {
    /*
        TODO - group into EcInheritableState and EcPrivateState. Then can use structure assignment.
     */
    /*
        Inherited fields. These are inherited by new states.
     */
    //  OPT - compress into bit mask
    int             inModule;               /* Inside a module declaration */
    int             inClass;                /* Inside a class declaration */
    int             inFunction;             /* Inside a function declaration */
    int             inMethod;               /* Inside a method declaration */
    int             captureBreak;           /* Capture break/continue inside a catch/finally block */
    int             captureFinally;         /* Capture break/continue with a finally block */
    int             blockIsMethod;          /* Current function is a method */
    int             inHashExpression;       /* Inside a # expression */
    int             inSettings;             /* Inside constructor settings */

    /*
        These are used when parsing
     */
    EjsModule       *currentModule;         /* Current open module definition */
    EjsName         currentClassName;       /* Current open class name - Used only in ecParse */
    EcNode          *currentClassNode;      /* Current open class */
    EcNode          *currentFunctionNode;   /* Current open method */
    int             noin;                   /* Don't allow "in" */
    int             checksumOffset;         /* Location to write checksum when writing out modules */

    //  TODO - rename
    EcNode          *topVarBlockNode;       /* Top var block node */

    /*
        These are used when doing AST processing and code generation
     */
    EjsType         *currentClass;          /* Current open class */

    //  TODO - should be using frame->currentMethod
    EjsFunction     *currentFunction;       /* Current open method */

    //  TODO - can this be derrived from currentMethod?
    cchar           *currentFunctionName;   /* Current method name */

    EjsObj          *letBlock;              /* Block for local block scope declarations */
    EjsObj          *varBlock;              /* Block for var declarations */
    EjsObj          *optimizedLetBlock;     /* Optimized let block declarations - may equal ejs->global */
    EcNode          *letBlockNode;

    /*
        The defaultNamespace comes from "use default namespace NAME" and is used for new declarations in the top level block
        where the pragma was defined. ie. it is not passed into classes or functions (1 level deep).
        If the defaultNamespace is not defined, then the namespace field is used.
     */
    cchar           *namespace;             /* Namespace for declarations */
    cchar           *defaultNamespace;      /* Default namespace for new top level declarations. Does not propagate */
    int             namespaceCount;         /* Count of namespaces originally in block. Used to pop namespaces */

    //  TODO - should change this to include functions also
    EcNode          *currentObjectNode;     /* Left object in "." or "[" */

    int             preserveStackCount;     /* If reset needed, preserve this count of elements */
    int             needsStackReset;        /* Stack must be reset before jumping */
    int             needsValue;             /* Express must yield a value */

    //  MOB -- should rationalize and have only one of these. Parser needs onRight.
    int             onLeft;                 /* On the left of an assignment */

    //  MOB -- unused
    int             onRight;                /* On the right of an assignment */
    int             dupLeft;                /* Dup left side */

    int             saveOnLeft;             /* Saved left of an assignment */
    int             conditional;            /* In branching conditional */
    int             strict;                 /* Compiler checking mode: Strict, standard*/
    int             inheritedTraits;        /* Inherited traits from current block */
    bool            disabled;               /* Disable nodes below this scope */

    struct EcCodeGen    *code;              /* Global and function code buffer */
    struct EcCodeGen    *staticCodeBuf;     /* Class static level code generation buffer */
    struct EcCodeGen    *instanceCodeBuf;   /* Class instance level code generation buffer */

    MprList         *namespaces;            /* List of open namespaces */

    /*
        TODO refactor or source via some other way
     */
    int             inInterface;            /* Inside an interface */
    int             instanceCode;           /* Generating instance class code */

    struct EcState  *prev;                  /* State stack */
    struct EcState  *prevBlockState;        /* Block state stack */
    struct EcState  *breakState;            /* State for breakable blocks */
    struct EcState  *classState;            /* State for current class */
    int             blockNestCount;         /* Count of blocks encountered. Used by ejs shell */
    int             stateLevel;             /* State level counter */
} EcState;


extern int      ecEnterState(struct EcCompiler *cp);
extern void     ecLeaveState(struct EcCompiler *cp);
extern EcNode   *ecLeaveStateWithResult(struct EcCompiler *cp,  struct EcNode *np);
extern int      ecResetModule(struct EcCompiler *cp, struct EcNode *np);
extern void     ecStartBreakableStatement(struct EcCompiler *cp, int kinds);


/*
    Primary compiler control structure
 */
typedef struct EcCompiler {
    /*
        Properties ordered to make debugging easier
     */
    int         phase;                      /* Ast processing phase */
    EcState     *state;                     /* Current state */

#if BLD_DEBUG
    char        *currentLine;               /* Current input source code line */
#endif

    EcToken     *peekToken;                 /* Alias for peek ahead */
    EcToken     *token;                     /* Alias for lexer->input->token */

#if BLD_DEBUG
    char        *tokenName;                 /* Name of last consumed token */
    char        *peekTokenName;             /* Name of lookahead token */
#endif

    EcState     *fileState;                 /* Top level state for the file */
#if UNUSED
    EcState     *classState;                /* State for the current class - used in parse */
#endif
//  MOB -- these are risky and should be moved into state. A nested block, directive class etc willl modify
    EcState     *directiveState;            /* State for the current directive - used in parse and CodeGen */
    EcState     *blockState;                /* State for the current block */

    EjsLookup   lookup;                     /* Lookup residuals */

    int         currentLineNumber;          /* Current input source line number */
    cchar       *currentFilename;           /* Current input file name */

    EcLexer     *lexer;                     /* Lexical analyser */
    EcInput     *input;                     /* Alias for lexer->input */
    EjsService  *vmService;                 /* VM runtime */
    Ejs         *ejs;                       /* Interpreter instance */

    /*
        Compiler command line options
     */
    char        *certFile;                  /* Certificate to sign the module file */
    bool        debug;                      /* Run in debug mode */
    bool        doc;                        /* Include documentation strings in output */
    char        *extraFiles;                /* Extra source files to compile */
    MprList     *require;                   /* Required list of modules to pre-load */
    bool        interactive;                /* Interactive use (ejsh) */
    bool        merge;                      /* Merge all dependent modules */
    bool        bind;                       /* Don't bind properties to slots */
    bool        noout;                      /* Don't generate any module output files */
    bool        visibleGlobals;             /* Make globals visible (no namespace) */
    int         optimizeLevel;              /* Optimization factor (0-9) */
    bool        shbang;                     /* Observe #!/path as the first line of a script */
    int         warnLevel;                  /* Warning level factor (0-9) */

    int         strict;                     /* Compiler default strict mode */
    int         lang;                       /* Language compliance level: ecma|plus|fixed */
    char        *outputFile;                /* Output module file name override */
    MprFile     *file;                      /* Current output file handle */

    int         modver;                     /* Default module version */
    int         parseOnly;                  /* Only parse the code */
    int         strip;                      /* Strip debug symbols */
    int         tabWidth;                   /* For error reporting "^" */

    MprList     *modules;                   /* List of modules to process */
    MprList     *fixups;                    /* Type reference fixups */

    char        *errorMsg;                  /* Aggregated error messages */
    int         error;                      /* Unresolved parse error */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         memError;                   /* Memory error */
    int         errorCount;                 /* Count of all errors */
    int         warningCount;               /* Count of all warnings */
    int         nextSeqno;                  /* Node sequence numbers */
    int         blockLevel;                 /* Level of nest in blocks */

    /*
        TODO - aggregate these into flags
     */
    int         peeking;                    /* Parser is doing peek() */
    int         lastOpcode;                 /* Last opcode encoded */
    int         uid;                        /* Unique identifier generator */
} EcCompiler;


extern int          ecAddModule(EcCompiler *cp, EjsModule *mp);
extern EcNode       *ecAppendNode(EcNode *np, EcNode *child);
extern int          ecAstFixup(EcCompiler *cp, struct EcNode *np);
extern EcNode       *ecChangeNode(EcNode *np, EcNode *oldNode, EcNode *newNode);
extern void         ecGenConditionalCode(EcCompiler *cp, EcNode *np, EjsModule *up);
extern int          ecCodeGen(EcCompiler *cp, int argc, struct EcNode **nodes);
extern int          ecCompile(EcCompiler *cp, int argc, char **path);
extern EcLexer      *ecCreateLexer(EcCompiler *cp);
extern void         ecDestroyLexer(EcCompiler *cp);
EcCompiler          *ecCreateCompiler(struct Ejs *ejs, int flags);
extern EcNode       *ecCreateNode(EcCompiler *cp, int kind);
extern void         ecFreeToken(EcInput *input, EcToken *token);
extern char         *ecGetErrorMessage(EcCompiler *cp);
extern char         *ecGetInputStreamName(EcLexer *lp);
extern int          ecGetToken(EcInput *input);
extern int          ecGetRegExpToken(EcInput *input, cchar *prefix);
extern EcNode       *ecLinkNode(EcNode *np, EcNode *child);
extern EjsModule    *ecLookupModule(EcCompiler *cp, cchar *name, int minVersion, int maxVersion);
extern int          ecLookupScope(EcCompiler *cp, EjsName *name);
extern int          ecLookupVar(EcCompiler *cp, EjsObj *vp, EjsName *name);
extern EcNode       *ecParseWarning(EcCompiler *cp, char *fmt, ...);
extern int          ecPeekToken(EcCompiler *cp);
extern int          ecPutSpecificToken(EcInput *input, EcToken *token);
extern int          ecPutToken(EcInput *input);
extern void         ecSetError(EcCompiler *cp, cchar *severity, cchar *filename, int lineNumber,
                        char *currentLine, int column, char *msg);
extern void         ecResetInput(EcCompiler *cp);
extern EcNode       *ecResetError(EcCompiler *cp, EcNode *np, bool eatInput);
extern int          ecRemoveModule(EcCompiler *cp, EjsModule *mp);
extern void         ecResetParser(EcCompiler *cp);
extern int          ecResetModuleList(EcCompiler *cp);
extern int          ecOpenConsoleStream(EcLexer *lp, EcStreamGet gets);
extern int          ecOpenFileStream(EcLexer *input, cchar *path);
extern int          ecOpenMemoryStream(EcLexer *input, const uchar *buf, int len);
extern void         ecCloseStream(EcLexer *input);
extern void         ecSetOptimizeLevel(EcCompiler *cp, int level);
extern void         ecSetWarnLevel(EcCompiler *cp, int level);
extern void         ecSetStrictMode(EcCompiler *cp, int on);
extern void         ecSetTabWidth(EcCompiler *cp, int width);
extern void         ecSetOutputFile(EcCompiler *cp, cchar *outputFile);
extern void         ecSetCertFile(EcCompiler *cp, cchar *certFile);
extern EcToken      *ecTakeToken(EcInput *input);
extern int          ecAstProcess(struct EcCompiler *cp, int argc,  struct EcNode **nodes);

/*
    Module file creation routines.
 */
extern void     ecAddFunctionConstants(EcCompiler *cp, EjsObj *obj, int slotNum);
extern void     ecAddConstants(EcCompiler *cp, EjsObj *obj);
extern int      ecAddConstant(EcCompiler *cp, cchar *str);
extern int      ecAddNameConstant(EcCompiler *cp, EjsName *qname);
extern int      ecAddDocConstant(EcCompiler *cp, void *vp, int slotNum);
extern int      ecAddModuleConstant(EcCompiler *cp, EjsModule *up, cchar *str);
extern int      ecCreateModuleHeader(EcCompiler *cp);
extern int      ecCreateModuleSection(EcCompiler *cp);


/*
    Encoding emitter routines
 */
extern int      ecEncodeBlock(EcCompiler *cp, uchar *buf, int len);
extern int      ecEncodeByte(EcCompiler *cp, int value);
extern int      ecEncodeUint(EcCompiler *cp, int number);
extern int      ecEncodeNumber(EcCompiler *cp, int64 number);
extern int      ecEncodeName(EcCompiler *cp, EjsName *qname);
extern int      ecEncodeOpcode(EcCompiler *cp, int value);
extern int      ecEncodeString(EcCompiler *cp, cchar *str);
extern int      ecEncodeGlobal(EcCompiler *cp, EjsObj *obj, EjsName *qname);
extern int      ecEncodeWord(EcCompiler *cp, int value);
extern int      ecEncodeDouble(EcCompiler *cp, double value);
extern int      ecEncodeByteAtPos(EcCompiler *cp, uchar *pos, int value);
extern int      ecEncodeWordAtPos(EcCompiler *cp, uchar *pos, int value);
extern void     ecCopyCode(EcCompiler *cp, uchar *pos, int size, int dist);
extern uint     ecGetCodeOffset(EcCompiler *cp);
extern int      ecGetCodeLen(EcCompiler *cp, uchar *mark);
extern void     ecAdjustCodeLength(EcCompiler *cp, int adj);

#endif /* _h_EC_COMPILER */

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
/************************************************************************/
/*
 *  End of file "../../src/include/ecCompiler.h"
 */
/************************************************************************/

#include "ejs.slots.h"
