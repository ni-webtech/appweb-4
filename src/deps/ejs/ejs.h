
/******************************************************************************/
/* 
    This file is an amalgamation of all the individual source code files for the
    Embedthis Ejscript Header.
  
    Catenating all the source into a single file makes embedding simpler and
    the resulting application faster, as many compilers can do whole file
    optimization.
  
    If you want to modify the product, you can still get the whole source as 
    individual files if you need.
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
#define EJS_MAX_COLLISIONS          4               /* Max intern string collion chain */

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
#define EC_TOKEN_INCR           64              /* Growth increment for tokens */
#define EC_MAX_LOOK_AHEAD       8
#define EC_BUFSIZE              4096            /* General buffer size */
#define EC_MAX_ERRORS           25              /* Max compilation errors before giving up */

#define EC_CODE_BUFSIZE         4096            /* Initial size of code gen buffer */
#define EC_NUM_PAK_PROP         32              /* Initial number of properties */

#ifdef __cplusplus
}
#endif
#endif /* _h_EJS_TUNE */

/*
    @copy   

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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

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
    EJS_OP_LOAD_THIS_LOOKUP,
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
    {   "LOAD_THIS_LOOKUP",          1,         { EBC_NONE,                               },},
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

extern EjsOptable *ejsGetOptable();

#ifdef __cplusplus
}
#endif

#endif /* _h_EJS_BYTECODETABLE_H */

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
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
 *  End of file "../../src/include/ejsByteCodeTable.h"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/include/ejs.h"
 */
/************************************************************************/

/*
    ejs.h - Ejscript header

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
struct EjsHelpers;
struct EjsLoc;
struct EjsMem;
struct EjsNames;
struct EjsModule;
struct EjsNamespace;
struct EjsObj;
struct EjsPot;
struct EjsService;
struct EjsState;
struct EjsTrait;
struct EjsTraits;
struct EjsType;
struct EjsUri;
struct EjsWorker;
struct EjsXML;
#endif

/*
    Trait, type, function and property attributes. These are sometimes combined into a single attributes word.
 */
//  MOB - renumber from 0x1
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
#define EJS_PROP_ENUMERABLE             0x2000      /**< Property will be enumerable (compiler use only) */

#define EJS_FUN_CONSTRUCTOR             0x4000      /**< Method is a constructor */
#define EJS_FUN_FULL_SCOPE              0x8000      /**< Function needs closure when defined */
#define EJS_FUN_HAS_RETURN              0x10000     /**< Function has a return statement */
#define EJS_FUN_INITIALIZER             0x20000     /**< Type initializer code */
#define EJS_FUN_OVERRIDE                0x40000     /**< Override base type */
#define EJS_FUN_MODULE_INITIALIZER      0x80000     /**< Module initializer */
#define EJS_FUN_REST_ARGS               0x100000    /**< Parameter is a "..." rest */
#define EJS_TRAIT_MASK                  0x1FFFFF    /**< Mask of trait attributes */

/*
    These attributes are never stored in EjsTrait but are often passed in "attributes"
 */
#define EJS_TYPE_CALLS_SUPER            0x200000    /**< Constructor calls super() */
#define EJS_TYPE_HAS_INSTANCE_VARS      0x400000    /**< Type has non-method instance vars (state) */
#define EJS_TYPE_DYNAMIC_INSTANCE       0x800000    /**< Instances are not sealed */
#define EJS_TYPE_FINAL                  0x1000000   /**< Type can't be subclassed */
#define EJS_TYPE_FIXUP                  0x2000000   /**< Type needs to inherit base types properties */
#define EJS_TYPE_HAS_CONSTRUCTOR        0x4000000   /**< Type has a constructor */
#define EJS_TYPE_HAS_TYPE_INITIALIZER   0x80000000  /**< Type has an initializer */
#define EJS_TYPE_IMMUTABLE              0x10000000  /**< Instances are immutable */
#define EJS_TYPE_INTERFACE              0x20000000  /**< Class is an interface */

/*
    Interpreter flags
 */
#define EJS_FLAG_EVENT          0x1         /**< Event pending */
#define EJS_FLAG_NO_INIT        0x8         /**< Don't initialize any modules*/
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
#define EJS_ERR                 MPR_ERR
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
#define EJS_DEFAULT_CLASS_NAME      "__defaultClass__"
#define EJS_INITIALIZER_NAME        "__initializer__"

#define EJS_NAME                    "ejs"
#define EJS_MOD                     "ejs.mod"

/*
    File extensions
 */
#define EJS_MODULE_EXT              ".mod"
#define EJS_SOURCE_EXT              ".es"
#define EJS_LISTING_EXT             ".lst"

typedef struct EjsLoc {
    MprChar         *source;
    char            *filename;
    int             lineNumber;
} EjsLoc;

//  MOB -- reorder this file


#define EJS_SHIFT_VISITED       0
#define EJS_SHIFT_DYNAMIC       1
#define EJS_SHIFT_TYPE          0

#define EJS_MASK_VISITED        0x1
#define EJS_MASK_DYNAMIC        0x2
#define EJS_MASK_TYPE           ~(EJS_MASK_VISITED | EJS_MASK_DYNAMIC)

#define DYNAMIC(vp)             ((((EjsObj*) vp)->xtype) & EJS_MASK_DYNAMIC)
#define VISITED(vp)             ((((EjsObj*) vp)->xtype) & EJS_MASK_VISITED)
#define TYPE(vp)                ((EjsType*) ((((EjsObj*) vp)->xtype) & EJS_MASK_TYPE))

#define SET_VISITED(vp, value)  ((EjsObj*) vp)->xtype = \
                                    ((value) << EJS_SHIFT_VISITED) | (((EjsObj*) vp)->xtype & ~EJS_MASK_VISITED)
#define SET_DYNAMIC(vp, value)  ((EjsObj*) vp)->xtype = \
                                    (((size_t) value) << EJS_SHIFT_DYNAMIC) | (((EjsObj*) vp)->xtype & ~EJS_MASK_DYNAMIC)
#if BLD_DEBUG
#define SET_TYPE_NAME(vp, t)    if (1) { \
                                    if (t && ((EjsType*) t)->qname.name) { \
                                        ((EjsObj*) vp)->kind = ((EjsType*) t)->qname.name->value; \
                                    } \
                                    ((EjsObj*) vp)->type = ((EjsType*) t); \
                                } else
#else
#define SET_TYPE_NAME(vp, type)
#endif

#define SET_TYPE(vp, value)      if (1) { \
                                    ((EjsObj*) vp)->xtype = \
                                        (((size_t) value) << EJS_SHIFT_TYPE) | (((EjsObj*) vp)->xtype & ~EJS_MASK_TYPE); \
                                    SET_TYPE_NAME(vp, value); \
                                } else

typedef void EjsAny;

/*
    WARNING: changes to this structure require changes to mpr/src/mprPrintf.c
 */
typedef struct EjsObj {
    ssize           xtype;              /* xtype: typeBits | dynamic << 1 | visited */
#if BLD_DEBUG
    char            *kind;              /* Type name of object (Type->qname.name) */
    struct EjsType  *type;              /* Pointer to object type */
    MprMem          *mem;               /* Pointer to underlying memory block */
#endif
} EjsObj;

    
#if BLD_DEBUG
    #define ejsSetMemRef(obj) if (1) { ((EjsObj*) obj)->mem = MPR_GET_MEM(obj); } else 
#else
    #define ejsSetMemRef(obj) 
#endif

    
/*
    WARNING: changes to this structure require changes to mpr/src/mprPrintf.c
 */
typedef struct EjsString {
    EjsObj           obj;
    struct EjsString *next;              /* Hash chain link when interning */
    struct EjsString *prev;
    ssize            length;
    MprChar          value[0];
} EjsString;

extern void ejsManageString(EjsString *sp, int flags);

/**
    Qualified name structure
    @description All names in Ejscript consist of a property name and a name space. Namespaces provide discrete
        spaces to manage and minimize name conflicts. These names will soon be converted to unicode.
    @stability Prototype
    @defgroup EjsName EjsName
    @see EjsName ejsName ejsAllocName ejsDupName ejsCopyName
 */       
typedef struct EjsName {
    EjsString   *name;                          /**< Property name */
    EjsString   *space;                         /**< Property namespace */
} EjsName;

extern void ejsMarkName(EjsName *qname);


/** 
    Allocation and Type Helpers
    @description The type helpers interface defines the set of primitive operations a type must support to
        interact with the virtual machine.
    @ingroup EjsType
 */
typedef struct EjsHelpers {
    /* Used by objects and values */
    EjsAny  *(*cast)(struct Ejs *ejs, EjsAny *obj, struct EjsType *type);
    void    *(*clone)(struct Ejs *ejs, EjsAny *obj, bool deep);
//  MOB -- rename alloc/free
    EjsAny  *(*create)(struct Ejs *ejs, struct EjsType *type, int size);
    int     (*defineProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                int64 attributes, EjsAny *value);
    int     (*deleteProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    int     (*deletePropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    EjsAny  *(*getProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*getPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*getPropertyCount)(struct Ejs *ejs, EjsAny *obj);
    EjsName (*getPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    struct EjsTrait *(*getPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*invokeOperator)(struct Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
    int     (*lookupProperty)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*setProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
    int     (*setPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
    int     (*setPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
    int     (*setPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);
} EjsHelpers;


//  MOB dividor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct EjsLoadState {
    MprList         *typeFixups;            /**< Loaded types to fixup */
    int             firstModule;            /**< First module in ejs->modules for this load */
    int             flags;                  /**< Module load flags */
} EjsLoadState;

typedef void (*EjsLoaderCallback)(struct Ejs *ejs, int kind, ...);

/*
    Interned string hash shared over all interpreters
 */
typedef struct EjsIntern {
    EjsString       *buckets;               /**< Hash buckets and references to link chains of strings (unicode) */
    MprSpin         *spin;                  /**< Multithread sync */
    struct Ejs      *ejs;                   /**< Owning interpreter */
    int             size;                   /**< Size of hash */
    int             reuse;
    uint64          accesses;
} EjsIntern;



#if UNUSED
#define NS_EJS                  1
#define NS_EMPTY                2
#define NS_INTERNAL             3
#define NS_ITERATOR             4
#define NS_PUBLIC               5

#define S_COMMA_PROTECTED       6
#define S_EMPTY                 7
#define S_LENGTH                8
#define S_PUBLIC                9

#define V_FALSE                 10
#define V_INFINITY              11
#define V_ITERATOR              12
#define V_MAX                   13
#define V_MIN                   14
#define V_MINUS_ONE             15
#define V_NAN                   16
#define V_NEGATIVE_INFINITY     17
#define V_NOP                   18
#define V_NULL                  19
#define V_ONE                   20
#define V_TRUE                  21
#define V_UNDEFINED             22
#define V_ZERO                  23
#endif
#define EJS_MAX_SPECIAL         1

extern void ejsSetSpecial(struct Ejs *ejs, int index, EjsAny *value);
extern EjsAny *ejsGetSpecial(struct Ejs *ejs, int index);

#define V(i) ejsGetSpecial(ejs, i)

/**
    Ejsript Interperter Structure
    @description The Ejs structure contains the state for a single interpreter. The #ejsCreate routine may be used
        to create multiple interpreters and returns a reference to be used in subsequent Ejscript API calls.
    @stability Prototype.
    @defgroup Ejs Ejs
    @see ejsCreate, ejsCreateService, ejsAppendSearchPath, ejsSetSearchPath, ejsEvalFile, ejsEvalScript, ejsExit
 */
typedef struct Ejs {
    char                *name;              /**< Unique interpreter name */
    EjsAny              *exception;         /**< Pointer to exception object */
    EjsAny              *result;            /**< Last expression result */
    struct EjsState     *state;             /**< Current evaluation state and stack */
    struct EjsService   *service;           /**< Back pointer to the service */
    EjsIntern           *intern;            /**< Interned Unicode string hash - shared over all interps */
    cchar               *bootSearch;        /**< Module search when bootstrapping the VM */
    struct EjsArray     *search;            /**< Module load search path */
    cchar               *className;         /**< Name of a specific class to run for a program */
    cchar               *methodName;        /**< Name of a specific method to run for a program */

    /*
        Essential types
     */
    //  MOB - should these be in special values except for the really common ones?
    struct EjsType      *appType;           /**< App type */
    struct EjsType      *arrayType;         /**< Array type */
    struct EjsType      *blockType;         /**< Block type */
    struct EjsType      *booleanType;       /**< Boolean type */
    struct EjsType      *byteArrayType;     /**< ByteArray type */
    struct EjsType      *configType;        /**< Config type */
    struct EjsType      *dateType;          /**< Date type */
    struct EjsType      *errorType;         /**< Error type */
    struct EjsType      *errorEventType;    /**< ErrorEvent type */
    struct EjsType      *eventType;         /**< Event type */
    struct EjsType      *frameType;         /**< Frame type */
    struct EjsType      *fileType;          /**< File type */
    struct EjsType      *fileSystemType;    /**< FileSystem type */
    struct EjsType      *functionType;      /**< Function type */
    struct EjsType      *httpType;          /**< Http type */
    struct EjsType      *httpServerType;    /**< HttpServer type */
    struct EjsType      *iteratorType;      /**< Iterator type */
    struct EjsType      *mathType;          /**< Math type */
    struct EjsType      *namespaceType;     /**< Namespace type */
    struct EjsType      *nullType;          /**< Null type */
    struct EjsType      *numberType;        /**< Default numeric type */
    struct EjsType      *objectType;        /**< Object type */
    struct EjsType      *pathType;          /**< Path type */
    struct EjsType      *regExpType;        /**< RegExp type */
    struct EjsType      *requestType;       /**< Request type */
    struct EjsType      *sessionType;       /**< Session type object */
    struct EjsType      *stringType;        /**< String type */
    struct EjsType      *socketType;        /**< Socket type */
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
    EjsAny              *global;            /**< The "global" object */
#if BLD_DEBUG
    struct EjsBlock     *globalBlock;
#endif

    EjsAny              *values[EJS_MAX_SPECIAL];

    //  MOB -- remove most of these
    EjsAny              *falseValue;        /**< The "false" value */
    struct EjsNumber    *infinityValue;     /**< The infinity number value */
    struct EjsIterator  *iterator;          /**< Default iterator */
    struct EjsNumber    *maxValue;          /**< Maximum number value */
    struct EjsNumber    *minValue;          /**< Minimum number value */
    struct EjsNumber    *minusOneValue;     /**< The -1 number value */
    struct EjsNumber    *nanValue;          /**< The "NaN" value if floating point numbers, else zero */
    struct EjsNumber    *negativeInfinityValue; /**< The negative infinity number value */
    struct EjsFunction  *nopFunction;       /**< The NOP function */
    EjsAny              *nullValue;         /**< The "null" value */
    struct EjsNumber    *oneValue;          /**< The 1 number value */
    EjsAny              *trueValue;         /**< The "true" value */
    EjsAny              *undefinedValue;    /**< The "void" value */
    struct EjsNumber    *zeroValue;         /**< The 0 number value */

    //  MOB - need a more scalable solution (index based on first 4 chars only)
    EjsString           *emptyString;       /**< "" */
    EjsString           *lengthString;      /**< "length" */
    EjsString           *publicString;      /**< "public" */
    EjsString           *commaProtString;   /**< ",protected" */

    struct EjsNamespace *emptySpace;        /**< Empty namespace */
    struct EjsNamespace *ejsSpace;          /**< Ejs namespace */
    struct EjsNamespace *iteratorSpace;     /**< Iterator namespace */
    struct EjsNamespace *internalSpace;     /**< Internal namespace */
    struct EjsNamespace *publicSpace;       /**< Public namespace */

    EjsHelpers          objHelpers;         /**< Default EjsObj helpers */
    EjsHelpers          potHelpers;         /**< Helper methods for Pots */
    char                *errorMsg;          /**< Error message */
    cchar               **argv;             /**< Command line args */
    int                 argc;               /**< Count of command line args */
    int                 flags;              /**< Execution flags */
    int                 exitStatus;         /**< Status to exit() */
    int                 firstGlobal;        /**< First global to examine for GC */
    int                 joining;            /**< In Worker.join */
    int                 serializeDepth;     /**< Serialization depth */
    int                 spreadArgs;         /**< Count of spread args */
    int                 gc;                 /**< GC required (don't make bit field) */
    uint                compiling: 1;       /**< Currently executing the compiler */
    uint                destroying: 1;      /**< Interpreter is being destroyed */
    uint                empty: 1;           /**< Interpreter will be created empty */
    uint                exiting: 1;         /**< VM should exit */
    uint                hasError: 1;        /**< Interpreter has an initialization error */
    uint                initialized: 1;     /**< Interpreter fully initialized and not empty */
    uint                workerComplete: 1;  /**< TEMP MOB */

    EjsAny              *exceptionArg;      /**< Exception object for catch block */

    MprDispatcher       *dispatcher;        /**< Event dispatcher */
    MprList             *workers;           /**< Worker interpreters */
    MprList             *modules;           /**< Loaded modules */
    struct EjsFile      *nativeStream;      /**< Native log output stream */

    void                (*loaderCallback)(struct Ejs *ejs, int kind, ...);

    //  MOB - what is this for?
    void                *userData;          /**< User data */

    EjsObj              *coreTypes;         /**< Core type instances */
    MprHashTable        *doc;               /**< Documentation */
    void                *sqlite;            /**< Sqlite context information */

    Http                *http;              /**< Http service object (copy of EjsService.http) */
    HttpLoc             *loc;               /**< Current HttpLocation object for web start scripts */

#if UNUSED
    EjsAny              *sessions;          /**< Session cache */
    int                 sessionTimeout;     /**< Default session timeout */
    MprEvent            *sessionTimer;      /**< Session expiry timer */
#endif
    EjsAny              *applications;      /**< Application cache */
    int                 nextSession;        /**< Session ID counter */
    MprMutex            *mutex;             /**< Multithread locking */
} Ejs;


#if !DOXYGEN
/**
    Native Function signature
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreate
    @param thisObj Reference to the "this" object. (The object containing the method).
    @param argc Number of arguments.
    @param argv Array of arguments.
    @returns Returns a result variable or NULL on errors and exceptions.
    @stability Prototype.
 */
typedef struct EjsObj *(*EjsFun)(Ejs *ejs, EjsAny *thisObj, int argc, EjsObj **argv);

//LEGACY
typedef EjsFun EjsProc;
typedef EjsFun EjsNativeFunction;
#endif

//  TODO is this used?
typedef int (*EjsSortFn)(Ejs *ejs, EjsAny *p1, EjsAny *p2, cchar *name, int order);

/**
    Initialize a Qualified Name structure
    @description Initialize the statically allocated qualified name structure using a name and namespace.
    @param qname Reference to an existing, uninitialized EjsName structure
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */

//  MOB -- NAMING
extern EjsName ejsEmptyWideName(Ejs *ejs, MprChar *name);
extern EjsName ejsEmptyName(Ejs *ejs, cchar *name);
extern EjsName ejsWideName(Ejs *ejs, MprChar *space, MprChar *name);
extern EjsName ejsName(Ejs *ejs, cchar *space, cchar *name);

//  MOB -- NAMING
#define WEN(name) ejsEmptyWideName(ejs, name)
#define EN(name) ejsEmptyName(ejs, name)
#define N(space, name) ejsName(ejs, space, name)
#define WN(space, name) ejsWideName(ejs, space, name)

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


//  MOB OPT packing
typedef struct EjsSlot {
    EjsName         qname;                  /**< Property name */
    int             hashChain;              /**< Next property in hash chain */
    EjsTrait        trait;                  /**< Property descriptor traits */
    union {
        EjsAny      *ref;                   /**< Property reference */
        MprNumber   *value;                 /**< Immediate number value */
    } value;
} EjsSlot;


typedef struct EjsHash {
    int             size;                   /**< Size of hash */
    int             *buckets;               /**< Hash buckets and head of link chains */
} EjsHash;


typedef struct EjsProperties {
    EjsHash         *hash;                  /**< Hash buckets and head of link chains */
    int             size;                   /**< Current size of slots[] in elements */
    struct EjsSlot  slots[0];               /**< Vector of slots containing property references */
} EjsProperties;


//  MOB revise doc
/** 
    Object Type. Base type for all objects.
    @description The EjsPot type is the foundation for all types, blocks, functions and scripted classes. 
        It provides storage and hashed lookup for properties.
        \n\n
        EjsPot stores properties in an array of slots. These slots store a reference to the property value. 
        Property names are stored in a names hash. Dynamic objects own their own name hash. Sealed object instances 
        of a type, will simply refer to the hash of names owned by the type.
        \n\n
        EjsPots may be either dynamic or sealed. Dynamic objects can grow the number of properties. Sealed 
        objects cannot. Sealed objects will store the slot array as part of the EjsPot memory chunk. Dynamic 
        objects will perform a separate allocation for the slot array so that it can grow.
    @stability Evolving.
    @defgroup EjsPot EjsPot
    @see EjsPot ejsIsPot ejsCreateSimpleObject ejsCreateObject ejsCloneObject ejsGrowObject ejsManageObject
        MOB - change these From Var
        ejsGetVarType ejsAllocObj ejsFreeObj ejsCast ejsClone ejsCreateInstance ejsCreateVar
        ejsDestroyVar ejsDefineProperty ejsDeleteProperty ejsDeletePropertyByName
        ejsGetProperty ejsLookupProperty ejsSetProperty ejsSetPropertyByName ejsSetPropertyName
        ejsSetPropertyTraits ejsDeserialize ejsParseVar
 */
typedef struct EjsPot {
    EjsObj  obj;                                /**< Base object */
    uint    isBlock         : 1;                /**< Instance is a block */
    uint    isFrame         : 1;                /**< Instance is a frame */
    uint    isFunction      : 1;                /**< Instance is a function */
    uint    isPrototype     : 1;                /**< Object is a type prototype object */
    uint    isType          : 1;                /**< Instance is a type object */
    uint    separateHash    : 1;                /**< Object has separate hash memory */
    uint    separateSlots   : 1;                /**< Object has separate slots[] memory */
    uint    shortScope      : 1;                /**< Don't follow type or base classes */

    EjsProperties   *properties;                /** Object properties */
    //  TODO - OPT - merge numProp with bits above (24 bits)
    int             numProp;                    /** Number of properties */
} EjsPot;

#define POT(ejs, ptr)  (TYPE(ptr)->isPot)

extern int ejsIs(EjsAny *obj, int slot);
#define ejsIsPot(ejs, obj) (obj && POT(ejs, obj))

//  MOB -- remove ejs versions and just use mpr version
#define ejsGetName(ptr) mprGetName(ptr)
#define ejsSetName(ptr, name) mprSetName(ptr, name)
#define ejsCopyName(dest, src) mprSetName(dest, mprGetName(src))

/** 
    Allocate a new variable
    @description This will allocate space for a bare variable. This routine should only be called by type factories
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type object from which to create an object instance
    @param extra Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
extern EjsAny *ejsAlloc(Ejs *ejs, struct EjsType *type, ssize extra);

/** 
    Cast a variable to a new type
    @description Cast a variable and return a new variable of the required type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to cast
    @param type Type to cast to
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsCast(Ejs *ejs, EjsAny *obj, struct EjsType *type);

/** 
    Clone a variable
    @description Copy a variable and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to clone
    @param deep Set to true to do a deep copy.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsClone(Ejs *ejs, EjsAny *obj, bool deep);

/** 
    Create a new variable instance 
    @description Create a new variable instance and invoke any required constructors with the given arguments.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type from which to create a new instance
    @param argc Count of args in argv
    @param argv Vector of arguments. Each arg is an EjsAny.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsCreateInstance(Ejs *ejs, struct EjsType *type, int argc, void *argv);

/** 
    Create a variable
    @description Create a variable of the required type. This invokes the createVar helper method for the specified type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param type Type to cast to
    @param numSlots Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsAny
 */
extern EjsAny *ejsCreateObj(Ejs *ejs, struct EjsType *type, int numSlots);

/** 
    Define a property
    @description Define a property in a variable and give it a name, base type, attributes and default value.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object in which to define a property
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
    @ingroup EjsAny
 */
extern int ejsDefineProperty(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *type, int64 attributes, 
    EjsAny *value);

/** 
    Delete a property
    @description Delete a variable's property and set its slot to null. The slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable in which to delete the property
    @param slotNum Slot number in the variable for the property to delete.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsAny
 */
extern int ejsDeleteProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Delete a property by name
    @description Delete a variable's property by name and set its slot to null. The property is resolved by using 
        ejsLookupProperty with the specified name. Once deleted, the slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable in which to delete the property
    @param qname Qualified name for the property including name and namespace.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsAny
 */
extern int ejsDeletePropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The variable property stored at the nominated slot.
    @ingroup EjsAny
 */
extern EjsAny *ejsGetProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a count of properties in a variable
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @return A positive integer count of the properties stored by the variable. 
    @ingroup EjsAny
 */
extern int ejsGetLength(Ejs *ejs, EjsAny *obj);

//  MOB -- globally change
#define ejsGetPropertyCount(ejs, obj) ejsGetLength(ejs, obj)

/** 
    Get a variable property's name
    @description Get a property name for the property at a given slot in the  variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The qualified property name including namespace and name. Caller must not free.
    @ingroup EjsAny
 */
extern EjsName ejsGetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a property by name
    @description Get a property from a variable by name.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param qname Qualified name specifying both a namespace and name.
    @return The variable property stored at the nominated slot.
    @ingroup EjsAny
 */
extern EjsAny *ejsGetPropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property's traits
    @description Get a property's trait description. The property traits define the properties base type,
        and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number for the requested property.
    @return A trait structure reference for the property.
    @ingroup EjsAny
 */
extern struct EjsTrait *ejsGetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Invoke an opcode on a native type.
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
        Native types would normally implement the invokeOperator helper function to respond to this function call.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsAny
 */
extern EjsAny *ejsInvokeOperator(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
extern EjsAny *ejsInvokeOperatorDefault(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);

/** 
    Lookup a property by name
    @description Search for a property by name in the given variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param qname Qualified name of the property to search for.
    @return The slot number containing the property. Then use #ejsGetProperty to retrieve the property or alternatively
        use ejsGetPropertyByName to lookup and retrieve in one step.
    @ingroup EjsAny
 */
extern int ejsLookupProperty(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Set a property's value
    @description Set a value for a property at a given slot in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetProperty(Ejs *ejs, void *obj, int slotNum, void *value);

/** 
    Set a property's value 
    @description Set a value for a property. The property is located by name in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to examine
    @param qname Qualified property name.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetPropertyByName(Ejs *ejs, void *obj, EjsName qname, void *value);

/** 
    Set a property's name 
    @description Set a qualified name for a property at the specified slot in the variable. The qualified name
        consists of a namespace and name - both of which must be persistent. A typical paradigm is for these name
        strings to be owned by the memory context of the variable.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param qname Qualified property name.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);

/** 
    Set a property's traits
    @description Set the traits describing a property. These include the property's base type and access attributes.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param type Base type for the property. Set to NULL for an untyped property.
    @param attributes Integer mask of access attributes.
    @return The slot number of the property updated.
    @ingroup EjsAny
 */
extern int ejsSetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

//  TODO - DOC - 
extern EjsAny *ejsDeserialize(Ejs *ejs, EjsString *value);

//  MOB -- should this be EjsString?
extern EjsAny *ejsParse(Ejs *ejs, MprChar *str,  int prefType);
extern void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count);

//  MOB -- bad signature
extern void ejsCopySlots(Ejs *ejs, EjsPot *pot, EjsSlot *dest, EjsSlot *src, int count);

/** 
    Create an empty property object
    @description Create a simple object using Object as its base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @return A new object instance
    @ingroup EjsObj
 */
extern EjsAny *ejsCreateEmptyPot(Ejs *ejs);

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
extern void *ejsCreatePot(Ejs *ejs, struct EjsType *type, int size);

extern int ejsCompactPot(Ejs *ejs, EjsPot *obj);
extern int ejsInsertPotProperties(Ejs *ejs, EjsPot *pot, int numSlots, int offset);
extern int ejsMakeHash(Ejs *ejs, EjsPot *obj);
extern int ejsPropertyHasTrait(Ejs *ejs, EjsAny *obj, int slotNum, int attributes);
extern int ejsRemovePotProperty(Ejs *ejs, EjsAny *obj, int slotNum);
extern int ejsLookupPotProperty(Ejs *ejs, EjsPot *obj, EjsName qname);
extern EjsName ejsGetPotPropertyName(Ejs *ejs, EjsPot *obj, int slotNum);


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
extern EjsAny *ejsClonePot(Ejs *ejs, EjsAny *src, bool deep);

/** 
    Grow a pot object
    @description Grow the property storage for an object. Object properties are stored in slots. To store more 
        properties, you need to grow the slots.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object reference to grow
    @param numSlots New minimum count of properties. If size is less than the current number of properties, the call
        will be ignored, i.e. it will not shrink objects.
    @return Zero if successful
    @ingroup EjsObj
 */
extern int ejsGrowPot(Ejs *ejs, EjsPot *obj, int numSlots);

/** 
    Mark an object as currently in use.
    @description Mark an object as currently active so the garbage collector will preserve it. This routine should
        be called by native types that extend EjsObj in their markVar helper.
    @param ejs Interpreter instance returned from #ejsCreate
    @param obj Object to mark as currently being used.
    @ingroup EjsObj
 */
extern void ejsManagePot(void *obj, int flags);

extern int      ejsGetSlot(Ejs *ejs, EjsPot *obj, int slotNum);
extern EjsObj   *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);
extern int      ejsComputeStringHashCode(EjsString *name, int size);
extern int      ejsComputeMultiHashCode(cchar *name, int size);
extern int      ejsGetHashSize(int numProp);
extern void     ejsCreatePotHelpers(Ejs *ejs);

//  MOB -- rename
extern void     ejsMakePropertyDontDelete(EjsObj *obj, int dontDelete);
extern int      ejsMakePropertyEnumerable(EjsObj *obj, bool enumerable);
extern void     ejsMakePropertyReadOnly(EjsObj *obj, int readonly);
extern int      ejsRebuildHash(Ejs *ejs, EjsPot *obj);
extern void     ejsResetHash(Ejs *ejs, EjsPot *obj);
extern void     ejsRemoveSlot(Ejs *ejs, EjsPot *slots, int slotNum, int compact);
extern void     ejsSetAllocIncrement(Ejs *ejs, struct EjsType *type, int increment);
extern EjsObj   *ejsToSource(Ejs *ejs, EjsObj *obj, int argc, void *argv);

extern EjsString *ejsObjToString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);
extern EjsString *ejsObjToJSON(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);
extern int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int overwrite);

/** 
    String Class
    @description The String class provides the base class for all strings. Each String object represents a single 
    immutable linear sequence of characters. Strings have operators for: comparison, concatenation, copying, 
    searching, conversion, matching, replacement, and, subsetting.
    \n\n
    Strings are currently sequences of Unicode characters. Depending on the configuration, they may be 8, 16 or 32 bit
    code point values.
    @stability Evolving
    @defgroup EjsString EjsString
    @see EjsString ejsToString ejsCreateString ejsCreateBareString ejsCreateStringWithLength ejsDupString
        ejsVarToString ejsStrdup ejsStrcat ejsIsString
 */
//  MOB -- need definition here

/** 
    Create a string object
    @param ejs Ejs reference returned from #ejsCreate
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @stability Prototype
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateString(Ejs *ejs, MprChar *value, ssize len);
extern EjsString *ejsCreateStringFromConst(Ejs *ejs, struct EjsModule *mp, int index);
extern EjsString *ejsCreateStringFromAsc(Ejs *ejs, cchar *value);
extern EjsString *ejsCreateStringFromBytes(Ejs *ejs, cchar *value, ssize len);
extern EjsString *ejsCreateStringFromMulti(Ejs *ejs, cchar *value, ssize len);

/** 
    Create an empty string object. This creates an uninitialized string object of the requrired size. Once initialized,
        the string must be "interned" via $ejsInternString.
    @param ejs Ejs reference returned from #ejsCreate
    @param len Length of space to reserve for future string data
    @return A string object
    @ingroup EjsString
 */
extern EjsString *ejsCreateBareString(Ejs *ejs, ssize len);

//  MOB DOC
extern EjsString *ejsCreateNonInternedString(Ejs *ejs, MprChar *value, ssize len);

/** 
    Intern a string object. This stores the string in the internal string pool. This is required if the string was
    created via ejsCreateBareString. The ejsCreateString routine will intern the string automatcially.
    @param ejs Ejs reference returned from #ejsCreate
    @param len Length of space to reserve for future string data
    @return The internalized string object. NOTE: this may be different to the object passed in, if the string value
        was already present in the intern pool.
    @ingroup EjsString
 */
extern EjsString *ejsInternString(Ejs *ejs, EjsString *sp);
extern EjsString *ejsInternMulti(struct Ejs *ejs, cchar *value, ssize len);
extern EjsString *ejsInternAsc(struct Ejs *ejs, cchar *value, ssize len);
extern EjsString *ejsInternWide(struct Ejs *ejs, MprChar *value, ssize len);
extern void ejsManageIntern(Ejs *ejs, int flags);
extern EjsIntern *ejsCreateIntern(Ejs *ejs);
extern void ejsDestroyIntern(EjsIntern *intern);

extern int       ejsAtoi(Ejs *ejs, EjsString *sp, int radix);
extern EjsString *ejsCatString(Ejs *ejs, EjsString *dest, EjsString *src);
extern EjsString *ejsCatStrings(Ejs *ejs, EjsString *src, ...);
extern EjsString *ejsSubstring(Ejs *ejs, EjsString *src, ssize start, ssize len);
extern int       ejsCompareString(Ejs *ejs, EjsString *s1, EjsString *s2);
extern int       ejsCompareSubstring(Ejs *ejs, EjsString *s1, EjsString *s2, ssize offset, ssize len);
extern EjsString *ejsToLower(Ejs *ejs, EjsString *sp);
extern EjsString *ejsToUpper(Ejs *ejs, EjsString *sp);
extern EjsString *ejsTruncateString(Ejs *ejs, EjsString *sp, ssize len);

//  MIXED modes
extern int       ejsCompareMulti(Ejs *ejs, EjsString *s1, cchar *s2);
extern int       ejsCompareWide(Ejs *ejs, EjsString *s1, MprChar *s2, ssize len);
extern int       ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat);
extern int       ejsContainsMulti(Ejs *ejs, EjsString *sp, cchar *pat);
extern int       ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat);
extern int       ejsStartsWithMulti(Ejs *ejs, EjsString *sp, cchar *pat);
extern char      *ejsToMulti(Ejs *ejs, void *obj);
extern EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...);

/**
    Convert a variable to a string in JSON format
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Value to cast
    @param options Encoding options. See serialize for details.
    @return A string object
    @ingroup MOB
 */
extern EjsString *ejsToJSON(Ejs *ejs, EjsAny *obj, EjsObj *options);

//  MOB - low level serialize
extern EjsString *ejsSerialize(Ejs *ejs, EjsAny *obj, EjsObj *options);

/** 
    Cast a variable to a string
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to convert
    @return A string object
    @ingroup MOB
 */
extern EjsString *ejsToString(Ejs *ejs, EjsAny *obj);

#if DOXYGEN
    bool ejsIsString(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsString(ejs, obj) (obj && TYPE(obj)->id == ES_String)
#endif

extern EjsString *ejsStringToJSON(Ejs *ejs, EjsObj *obj);

/** 
    Array class
    @description Arrays provide a resizable, integer indexed, in-memory store for objects. An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered). Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list. The Array class can store objects with numerical indicies and can also store 
        any named properties. The named properties are stored in the obj field, whereas the numeric indexed values are
        stored in the data field. Array extends EjsObj and has all the capabilities of EjsObj.
    @stability Evolving
    @defgroup EjsArray EjsArray
    @see EjsArray ejsCreateArray ejsIsArray
 */
typedef struct EjsArray {
    EjsPot          pot;                /**< Property storage */
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
extern EjsArray *ejsCloneArray(Ejs *ejs, EjsArray *ap, bool deep);

extern int ejsAddItem(Ejs *ejs, EjsArray *ap, EjsAny *item);
extern int ejsAppendArray(Ejs *ejs, EjsArray *dest, EjsArray *src);
extern void ejsClearArray(Ejs *ejs, EjsArray *ap);
extern int ejsInsertItem(Ejs *ejs, EjsArray *ap, int index, EjsAny *item);
extern void *ejsGetItem(Ejs *ejs, EjsArray *ap, int index);
extern void *ejsGetFirstItem(Ejs *ejs, EjsArray *ap);
extern void *ejsGetLastItem(Ejs *ejs, EjsArray *ap);
extern void *ejsGetNextItem(Ejs *ejs, EjsArray *ap, int *next);
extern void *ejsGetPrevItem(Ejs *ejs, EjsArray *ap, int *next);
extern int ejsLookupItem(Ejs *ejs, EjsArray *lp, EjsAny *item);
extern int ejsRemoveItem(Ejs *ejs, EjsArray *ap, EjsAny *item);
extern int ejsRemoveLastItem(Ejs *ejs, EjsArray *ap);
extern int ejsRemoveItemAtPos(Ejs *ejs, EjsArray *ap, int index);

#if DOXYGEN
    /** 
        Determine if a variable is an array
        @param obj Object to test
        @return True if the variable is an array
        @ingroup EjsArray
     */
    extern bool ejsIsArray(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsArray(ejs, obj) (obj && TYPE(obj)->id == ES_Array)
#endif


//MOB - add dividors for all classes
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
    EjsPot          pot;                            /**< Property storage */
    MprList         namespaces;                     /**< Current list of namespaces open in this block of properties */
    struct EjsBlock *scope;                         /**< Lexical scope chain for this block */
    struct EjsBlock *prev;                          /**< Previous block in activation chain */

    //  MOB -- OPT and compress / eliminate some of these fields. Every function has these.
    EjsObj          *prevException;                 /**< Previous exception if nested exceptions */
    EjsObj          **stackBase;                    /**< Start of stack in this block */
    uint            breakCatch: 1;                  /**< Return, break or continue in a catch block */
    uint            isGlobal: 1;                    /**< Block is the global block */
    uint            nobind: 1;                      /**< Don't bind to properties in this block */
#if BLD_DEBUG
    struct EjsLine  *line;
#endif
} EjsBlock;


#if DOXYGEN
    /** 
        Determine if a variable is a block.
        @description This call tests if the variable is a block.
        @param obj Object to test
        @returns True if the variable is based on EjsBlock
        @ingroup EjsBlock
     */
    extern bool ejsIsBlock(Ejs *ejs, EjsObj *obj);
#else
    #define ejsIsBlock(ejs, obj) (ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isBlock)
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
    @param fun Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
extern int ejsBindFunction(Ejs *ejs, EjsAny *obj, int slotNum, void *fun);

/*  
    This is all an internal API. Native types should probably not be using these routines. Speak up if you find
    you need these routines in your code.
 */

extern int      ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *blockRef, struct EjsNamespace *nsp);
extern int      ejsAddScope(EjsBlock *block, EjsBlock *scopeBlock);
extern EjsBlock *ejsCreateBlock(Ejs *ejs, int numSlots);

//  TODO - why do we have ejsCloneObject, ejsCloneBlock ... Surely ejsCloneVar is sufficient?
extern EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep);

extern int      ejsCaptureScope(Ejs *ejs, EjsBlock *block, struct EjsArray *scopeChain);
extern int      ejsCopyScope(EjsBlock *block, struct EjsArray *chain);
extern int      ejsGetNamespaceCount(EjsBlock *block);

extern EjsBlock *ejsGetTopScope(EjsBlock *block);
extern void     ejsManageBlock(EjsBlock *block, int flags);
extern void     ejsPopBlockNamespaces(EjsBlock *block, int count);
extern EjsBlock *ejsRemoveScope(EjsBlock *block);
extern void     ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block);

#if BLD_DEBUG
#define ejsSetBlockLocation(block, loc) block->line = loc
#else
#define ejsSetBlockLocation(block, loc)
#endif

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


#define EJS_INDEX_INCR  256

typedef struct EjsConstants {
    char          *pool;                    /**< Constant pool string data */
    ssize         poolSize;                 /**< Size of constant pool storage in bytes */
    ssize         poolLength;               /**< Length of used bytes in constant pool */
    int           indexSize;                /**< Size of index in elements */
    int           indexCount;               /**< Number of constants used in index */
    int           locked;                   /**< No more additions allowed */
    MprHashTable  *table;                   /**< Hash table for fast lookup when compiling */
    EjsString     **index;                  /**< Interned string index */
//  MOB - remove
    struct EjsModule     *mp;
} EjsConstants;

extern EjsConstants *ejsCreateConstants(Ejs *ejs, int count, ssize size);
extern int ejsGrowConstants(Ejs *ejs, EjsConstants *constants, ssize size);
extern int ejsAddConstant(Ejs *ejs, EjsConstants *constants, cchar *str);

#define EJS_DEBUG_INCR 16

typedef struct EjsLine {
    int         offset;                     /**< Optional PC offsets of each line in function */
    MprChar     *source;                    /**< Program source code. Format: path@line: code */         
} EjsLine;


#define EJS_DEBUG_MAGIC     0x78654423
#define EJS_CODE_MAGIC      0x91917128

typedef struct EjsDebug {
    int         magic;
    ssize      size;                        /**< Size of lines[] in elements */
    int        numLines;                    /**< Number of entries in lines[] */
    EjsLine    lines[0];
} EjsDebug;

extern EjsDebug *ejsCreateDebug(Ejs *ejs, int length);
extern int ejsAddDebugLine(Ejs *ejs, EjsDebug **debug, int offset, MprChar *source);
extern EjsLine *ejsGetDebugLine(Ejs *ejs, struct EjsFunction *fun, uchar *pc);
extern int ejsGetDebugInfo(Ejs *ejs, struct EjsFunction *fun, uchar *pc, char **path, int *lineNumber, MprChar **source);

// TODO OPT. Could compress this.
/** 
    Byte code
    @description This structure describes a sequence of byte code for a function. It also defines a set of
        execption handlers pertaining to this byte code.
    @ingroup EjsFunction
 */
typedef struct EjsCode {
    int              magic;                  /**< Debug magic id */
    struct EjsModule *module;                /**< Module owning this function */
    EjsDebug         *debug;                 /**< Source code debug information */
    EjsEx            **handlers;             /**< Exception handlers */
    int              codeLen;                /**< Byte code length */
    int              debugOffset;            /**< Offset in mod file for debug info */
    int              numHandlers;            /**< Number of exception handlers */
    int              sizeHandlers;           /**< Size of handlers array */
    uchar            byteCode[0];            /**< Byte code */
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
    EjsBlock        block;                  /** Run-time properties for local vars */
    EjsPot          *activation;            /** Parameter and local properties */
    EjsString       *name;                  /** Function name */
#if FUTURE
    union {
#endif
        struct EjsFunction *setter;         /**< Setter function for this property */
        struct EjsType  *archetype;         /**< Type to use to create instances */
#if FUTURE && MOB
    } extra;
#endif
    union {
        EjsCode     *code;                  /**< Byte code */
        EjsFun      proc;                   /**< Native function pointer */
    } body;

    struct EjsArray *boundArgs;             /**< Bound "args" */
    EjsAny          *boundThis;             /**< Bound "this" object value */
    struct EjsType  *resultType;            /**< Return type of method */

    uint    numArgs: 8;                     /**< Count of formal parameters */
    uint    numDefault: 8;                  /**< Count of formal parameters with default initializers */
    uint    allowMissingArgs: 1;            /**< Allow unsufficient args for native functions */
    uint    castNulls: 1;                   /**< Cast return values of null */
    uint    fullScope: 1;                   /**< Closures must capture full scope */
    uint    hasReturn: 1;                   /**< Function has a return stmt */
    uint    inCatch: 1;                     /**< Executing catch block */
    uint    inException: 1;                 /**< Executing catch/finally exception processing */
    uint    isConstructor: 1;               /**< Function is a constructor */
    uint    isInitializer: 1;               /**< Function is a type initializer */
    uint    isNativeProc: 1;                /**< Function is native procedure */
    uint    moduleInitializer: 1;           /**< Function is a module initializer */
    uint    rest: 1;                        /**< Function has a "..." rest of args parameter */
    uint    staticMethod: 1;                /**< Function is a static method */
    uint    strict: 1;                      /**< Language strict mode (vs standard) */
    uint    throwNulls: 1;                  /**< Return type cannot be null */

} EjsFunction;

#if DOXYGEN
    /** 
        Determine if a variable is a function. This will return true if the variable is a function of any kind, including
            methods, native and script functions or initializers.
        @param obj Variable to test
        @return True if the variable is a function
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is a native function. Functions can be either native - meaning the implementation is
            via a C function, or can be scripted.
        @param obj Object to test
        @return True if the variable is a native function.
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsNativeFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is an initializer. Initializers are special functions created by the compiler to do
            static and instance initialization of classes during construction.
        @param obj Object to test
        @return True if the variable is an initializer
        @ingroup EjsFunction
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsInitializer(Ejs *ejs, EjsAny *obj);
#else
    //  MOB OPT
    #define ejsIsFunction(ejs, obj)       (obj && POT(ejs, obj) && ((EjsPot*) obj)->isFunction)
    #define ejsIsNativeFunction(ejs, obj) (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isNativeProc))
    #define ejsIsInitializer(ejs, obj)    (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isInitializer)
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
    @param module Reference to the module owning the function.
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
//  MOB - refactor into several functions
extern EjsFunction *ejsCreateFunction(Ejs *ejs, EjsString *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, EjsBlock *scope, 
    int strict);
extern int ejsInitFunction(Ejs *ejs, EjsFunction *fun, EjsString *name, cuchar *code, int codeLen, int numArgs, 
    int numDefault, int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, 
    EjsBlock *scope, int strict);
extern EjsFunction *ejsCreateSimpleFunction(Ejs *ejs, EjsString *name, int attributes);
extern void ejsDisableFunction(Ejs *ejs, EjsFunction *fun);

extern EjsPot *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots);
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
    @param thisObj Object to use as the "this" object when running the function.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
extern EjsAny *ejsRunFunction(Ejs *ejs, EjsFunction *fn, EjsAny *thisObj, int argc, void *argv);

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
extern EjsAny *ejsRunFunctionBySlot(Ejs *ejs, EjsAny *obj, int slotNum, int argc, void *argv);
extern EjsAny *ejsRunFunctionByName(Ejs *ejs, EjsAny *container, EjsName qname, EjsAny *obj, int argc, void *argv);

extern EjsEx *ejsAddException(Ejs *ejs, EjsFunction *mp, uint tryStart, uint tryEnd, struct EjsType *catchType,
    uint handlerStart, uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex);
extern EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *src, int deep);
extern int ejsDefineException(Ejs *ejs, struct EjsType *obj, int slot, uint tryOffset,
    uint tryLength, uint handlerOffset, uint handlerLength, int flags);
extern void ejsOffsetExceptions(EjsFunction *mp, int offset);
extern int ejsSetFunctionCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, ssize len, 
    EjsDebug *debug);
extern EjsCode *ejsCreateCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, ssize len, 
    EjsDebug *debug);
extern void ejsManageFunction(EjsFunction *fun, int flags);
extern void ejsShowOpFrequency(Ejs *ejs);

typedef struct EjsFrame {
    EjsFunction     function;               /**< Activation frame for function calls. Stores local variables */
    EjsFunction     *orig;                  /**< Original function frame is based on */
    struct EjsFrame *caller;                /**< Previous invoking frame */
    EjsObj          **stackBase;            /**< Start of stack in this function */
    EjsObj          **stackReturn;          /**< Top of stack to return to */
    uchar           *pc;                    /**< Program counter */
    uchar           *attentionPc;           /**< Restoration PC value after attention */
    uint            argc;                   /**< Actual parameter count */
    int             slotNum;                /**< Slot in owner */
    uint            getter: 1;              /**< Frame is a getter */
#if BLD_DEBUG
    EjsLine         *line;
#endif
} EjsFrame;

#if DOXYGEN
    /** 
        Determine if a variable is a frame. Only used internally in the VM.
        @param obj Object to test
        @return True if the variable is a frame. 
        @ingroup EjsFrame
     */
    extern bool ejsIsFrame(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFrame(ejs, obj) (ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isFrame)
#endif

extern EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *src, EjsObj *thisObj, int argc, EjsObj **argv);
extern EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *src);
extern EjsBlock *ejsPopBlock(Ejs *ejs);
extern EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *block);
extern int ejsFreeze(Ejs *ejs, int freeze);

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
    EjsObj  obj;                /**< Base object */
    bool    value;              /**< Boolean value */
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
    @param obj Object to cast
    @return A new boolean object
    @ingroup EjsBoolean
 */
extern EjsBoolean *ejsToBoolean(Ejs *ejs, EjsAny *obj);

#if DOXYGEN
    /** 
        Determine if a variable is a boolean
        @param obj Object to test
        @return True if the variable is a boolean
        @ingroup EjsBoolean
     */
    extern bool ejsIsBoolean(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsBoolean(ejs, obj) (obj && TYPE(obj)->id == ES_Boolean)
#endif

/** 
    Get the C boolean value from a boolean object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Boolean variable to access
    @return True or false
    @ingroup EjsBoolean
 */
extern bool ejsGetBoolean(Ejs *ejs, EjsAny *obj);

/*
    Thse constants match Stream.READ, Stream.WRITE, Stream.BOTH
 */
#define EJS_STREAM_READ     0x1
#define EJS_STREAM_WRITE    0x2
#define EJS_STREAM_BOTH     0x3

/** 
    ByteArray class
    @description ByteArrays provide a resizable, integer indexed, in-memory store for bytes. ByteArrays can be used as a 
    simple array type to store and encode data as bytes or they can be used as buffered Streams implementing the Stream 
    interface.
    \n\n
    When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
    extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
    can be used to get and put blocks of data. In this mode, the read and write position properties are ignored. 
    Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
    the ByteArray can be designated as resizable, in which case the initial size will grow as required to accomodate 
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
    EjsObj          obj;                /**< Base object */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    uchar           *value;             /**< Data bytes in the array */
    int             async;              /**< Async mode */
    int             endian;             /**< Endian encoding */
    int             growInc;            /**< Current read position */
    ssize           length;             /**< Length property */
    ssize           readPosition;       /**< Current read position */
    ssize           writePosition;      /**< Current write position */
    int             swap;               /**< I/O must swap bytes due to endian byte ordering */
    bool            resizable;          /**< Aray is resizable */
    EjsObj          *listeners;         /**< Event listeners in async mode */
} EjsByteArray;

#if DOXYGEN
    /** 
        Determine if a variable is a byte array
        @param obj Object to test
        @return True if the variable is a byte array
        @ingroup EjsByteArray
     */
    //  MOB -- convert all ejsIs to take ejs as arg
    extern bool ejsIsByteArray(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsByteArray(ejs, obj) (obj && TYPE(obj)->id == ES_ByteArray)
#endif

/** 
    Create a byte array
    @description Create a new byte array instance.
    @param ejs Ejs reference returned from #ejsCreate
    @param size Initial size of the byte array
    @return A new byte array instance
    @ingroup EjsByteArray
 */
extern EjsByteArray *ejsCreateByteArray(Ejs *ejs, ssize size);

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
extern void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, ssize readPosition, ssize writePosition);

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
extern int ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, ssize offset, char *data, ssize length);

extern void ejsResetByteArray(EjsByteArray *ba);
extern ssize ejsGetByteArrayAvailable(EjsByteArray *ba);
extern ssize ejsGetByteArrayRoom(EjsByteArray *ba);
extern int ejsGrowByteArray(Ejs *ejs, EjsByteArray *ap, ssize size);

extern struct EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ap, int argc, EjsObj **argv);
extern bool ejsMakeRoomInByteArray(Ejs *ejs, EjsByteArray *ap, ssize require);

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
    EjsObj          obj;                /**< Object base */
    MprTime         value;              /**< Time in milliseconds since "1970/01/01 GMT" */
} EjsDate;

#if DOXYGEN
    /** 
        Determine if a variable is a Date
        @param obj Object to test
        @return True if the variable is a date
        @ingroup EjsDate
     */
    bool ejsIsDate(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsDate(ejs, obj) (obj && TYPE(obj)->id == ES_Date)
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
typedef EjsPot EjsError;

#define ejsIsError(ejs, obj) (obj && ejsIsA(ejs, obj, ejs->errorType))

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
extern EjsError *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an assertion exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an math exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an instruction code exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an general error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an internal error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an IO exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an Memory depletion exception
    @param ejs Ejs reference returned from #ejsCreate
    @ingroup EjsError
 */
extern EjsError *ejsThrowMemoryError(Ejs *ejs);

/** 
    Throw an out of bounds exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an reference exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an resource exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an state exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

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
extern EjsError *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);

/** 
    Throw an type error exception
    @param ejs Ejs reference returned from #ejsCreate
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
extern EjsError *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...) PRINTF_ATTRIBUTE(2,3);


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
    EjsObj          obj;                /**< Base object */
    MprFile         *file;              /**< Open file handle */
    MprPath         info;               /**< Cached file info */
    char            *path;              /**< Filename path */
    char            *modeString;        /**< User supplied mode string */
    int             mode;               /**< Current open mode */
    int             perms;              /**< Posix permissions mask */
    int             attached;           /**< Attached to existing descriptor */
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
        @param obj Object to test
        @return True if the variable is a File
        @ingroup File
     */
    extern bool ejsIsFile(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFile(ejs, obj) (obj && TYPE(obj)->id == ES_File)
#endif


/**
    Path class
    @description The Path class provides file path name services.
    @stability Prototype
    @defgroup EjsPath EjsPath 
    @see EjsFile ejsCreatePath ejsIsPath
 */
typedef struct EjsPath {
    EjsObj          obj;                /**< Base object */
    cchar           *value;             /**< Filename path */
    MprPath         info;               /**< Cached file info */
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
extern EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path);
extern EjsPath *ejsCreatePathFromAsc(Ejs *ejs, cchar *path);

#if DOXYGEN
    /** 
        Determine if a variable is a Path
        @param ejs Ejs reference returned from #ejsCreate
        @param obj Object to test
        @return True if the variable is a Path
        @ingroup EjsPath
     */
    extern bool ejsIsPath(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsPath(ejs, obj) (obj && TYPE(obj)->id == ES_Path)
#endif

//  MOB DOC
extern EjsPath *ejsToPath(Ejs *ejs, EjsAny *obj);


/** 
    Uri class
    @description The Uri class provides file path name services.
    @stability Prototype
    @defgroup EjsUri EjsUri 
    @see EjsFile ejsCreateUri ejsIsUri
 */
typedef struct EjsUri {
    EjsObj      obj;            /**< Base object */
    HttpUri     *uri;           /**< Decoded URI */
} EjsUri;


/** 
    Create a Uri object
    @description Create a URI object associated with the given URI string.
    @param ejs Ejs reference returned from #ejsCreate
    @param uri Uri string to parse
    @return A new Uri object
    @ingroup EjsUri
 */
extern EjsUri *ejsCreateUri(Ejs *ejs, EjsString *uri);
extern EjsUri *ejsCreateUriFromMulti(Ejs *ejs, cchar *uri);
extern EjsUri *ejsCreateUriFromParts(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, 
        cchar *reference, int complete);

#if DOXYGEN
    /** 
        Determine if a variable is a Uri
        @param ejs Ejs reference returned from #ejsCreate
        @param obj Object to test
        @return True if the variable is a Uri
        @ingroup EjsUri
     */
    extern bool ejsIsUri(Ejs *ejs, EjsAny *obj);
    extern cchar *ejsGetUri(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsUri(ejs, obj) (obj && TYPE(obj)->id == ES_Uri)
    extern cchar *ejsGetUri(Ejs *ejs, EjsAny *obj);
#endif

//  MOB DOC
extern EjsUri *ejsToUri(Ejs *ejs, EjsAny *obj);

/** 
    FileSystem class
    @description The FileSystem class provides file system services.
    @stability Prototype
    @defgroup EjsFileSystem EjsFileSystem 
    @see EjsFile ejsCreateFile ejsIsFile
 */
typedef struct EjsFileSystem {
    EjsObj          obj;                /**< Base object */
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
        @param obj Object to test
        @return True if the variable is a FileSystem
        @ingroup EjsFileSystem
     */
    extern bool ejsIsFileSystem(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFileSystem(ejs, obj) (obj && TYPE(obj)->id == ES_FileSystem)
#endif


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
    EjsObj          obj;                        /**< Base object */
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
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    ssize           readCount;                  /**< Count of body bytes read */
    ssize           requestContentCount;        /**< Count of bytes written from requestContent */
    ssize           writeCount;                 /**< Count of bytes written via write() */
} EjsHttp;


/** 
    Create a new Http object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Http object
    @ingroup EjsHttp
 */
extern EjsHttp *ejsCreateHttp(Ejs *ejs);

//  MOB - fix up this pattern do don't need both define and function
#if DOXYGEN
    extern bool ejsIsHttp(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsHttpType(ejs, obj) (obj && TYPE(obj)->id == ES_Http)
#endif
extern void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, int server);
extern void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, int server);

//  MOB - rename SetupHttpTrace
extern int ejsSetupTrace(Ejs *ejs, HttpTrace *trace, EjsObj *options);
void ejsLoadHttpService(Ejs *ejs);


/** 
    Iterator Class
    @description Iterator is a helper class to implement iterators in other native classes
    @stability Prototype
    @defgroup EjsIterator EjsIterator
    @see EjsIterator ejsCreateIterator
 */
typedef struct EjsIterator {
    EjsObj          obj;                /**< Base object */
    EjsObj          *target;            /**< Object to be enumerated */
    EjsFun          nativeNext;         /**< Native next function */
    bool            deep;               /**< Iterator deep (recursively over all properties) */
    EjsArray        *namespaces;        /**< Namespaces to consider in iteration */
    int             index;              /**< Current index */
    EjsObj          *indexVar;          /**< Reference to current item */
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
extern EjsIterator *ejsCreateIterator(Ejs *ejs, EjsAny *target, EjsFun next, bool deep, EjsArray *namespaces);

/** 
    Namespace Class
    @description Namespaces are used to qualify names into discrete spaces.
    @stability Evolving
    @defgroup EjsNamespace EjsNamespace
    @see EjsNamespace ejsIsNamespace ejsCreateNamespace ejsLookupNamespace ejsDefineReservedNamespace 
        ejsCreateReservedNamespace ejsFormatReservedNamespace 
 */
typedef struct EjsNamespace {
    EjsObj          obj;                /**< Base object */
    EjsString       *value;             /**< Textual name of the namespace */
} EjsNamespace;


/** 
    Create a namespace object
    @param ejs Ejs reference returned from #ejsCreate
    @param name Space name to use for the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
extern EjsNamespace *ejsCreateNamespace(Ejs *ejs, EjsString *name);

#if DOXYGEN
    /**
        Determine if a variable is a namespace
        @param obj Object to test
        @return True if the variable is a namespace
        @ingroup EjsNamespace
     */
    extern bool ejsIsNamespace(Ejs *ejs, EjsAny *obj)
#else
    #define ejsIsNamespace(ejs, obj) (obj && TYPE(obj)->id == ES_Namespace)
#endif

extern EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, cchar *name);
extern EjsNamespace *ejsCreateReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *name);
extern EjsString *ejsFormatReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *spaceName);

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
#define ejsIsNull(ejs, obj) (obj && TYPE(obj)->id == ES_Null)
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
    EjsObj      obj;                /**< Base object */
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
    @param obj Object to cast
    @return A number object
    @ingroup EjsNumber
 */
extern struct EjsNumber *ejsToNumber(Ejs *ejs, EjsAny *obj);

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
    EjsObj      obj;                /**< Base object */
    EjsObj      *subject;           /**< Object under examination */
} EjsReflect;

extern EjsString *ejsGetTypeName(struct Ejs *ejs, EjsAny *obj);
extern EjsString *ejsGetTypeOf(struct Ejs *ejs, EjsAny *obj);

/** 
    RegExp Class
    @description The regular expression class provides string pattern matching and substitution.
    @stability Evolving
    @defgroup EjsRegExp EjsRegExp
    @see EjsRegExp ejsCreateRegExp ejsIsRegExp
 */
typedef struct EjsRegExp {
    EjsObj          obj;                /**< Base object */
    MprChar         *pattern;           /**< Pattern to match */
    void            *compiled;          /**< Compiled pattern */
    bool            global;             /**< Search for pattern globally (multiple times) */
    bool            ignoreCase;         /**< Do case insensitive matching */
    bool            multiline;          /**< Match patterns over multiple lines */
    bool            sticky;
    int             options;            /**< Pattern matching options */
    int             endLastMatch;       /**< End of the last match (one past end) */
    int             startLastMatch;     /**< Start of the last match */
    EjsString       *matched;           /**< Last matched component */
} EjsRegExp;


/** 
    Create a new regular expression object
    @param ejs Ejs reference returned from #ejsCreate
    @param pattern Regular expression pattern string
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
extern EjsRegExp *ejsCreateRegExp(Ejs *ejs, EjsString *pattern);

EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp);

#if DOXYGEN
    /** 
        Determine if the variable is a regular expression
        @return True if the variable is a regular expression
        @ingroup EjsRegExp
     */
    extern bool ejsIsRegExp(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsRegExp(ejs, obj) (obj && TYPE(obj)->id == ES_RegExp)
#endif

/**
    Socket Class
    @description
    @stability Prototype
    @defgroup EjsSocket EjsSocket
    @see EjsSocket ejsCreateSocket ejsIsSocket
 */
typedef struct EjsSocket {
    EjsObj          obj;                /**< Base object */
    EjsObj          *emitter;           /**< Event emitter */
    EjsByteArray    *data;              /**< Buffered write data */
    MprSocket       *sock;              /**< Underlying MPR socket object */
    cchar           *address;           /**< Remote address */
    int             port;               /**< Remote port */
    int             async;              /**< In async mode */
    int             mask;               /**< IO event mask */
    MprMutex        *mutex;             /**< Multithread sync */
} EjsSocket;

/** 
    Create a new Socket object
    @param ejs Ejs reference returned from #ejsCreate
    @return a new Socket object
    @ingroup EjsSocket
 */
extern EjsSocket *ejsCreateSocket(Ejs *ejs);

#if DOXYGEN
    extern bool ejsIsSocket(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsSocket(ejs, obj) (obj && TYPE(obj)->id == ES_Socket)
#endif


/** 
    Timer Class
    @description Timers manage the scheduling and execution of Ejscript functions. Timers run repeatedly 
        until stopped by calling the stop method and are scheduled with a granularity of 1 millisecond. 
    @stability Evolving
    @defgroup EjsTimer EjsTimer
    @see EjsTimer
 */
typedef struct EjsTimer {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference - needed for background timers */
    MprEvent        *event;             /**< MPR event for the timer */
    int             drift;              /**< Timer event is allowed to drift if system conditions requrie */
    int             repeat;             /**< Timer repeatedly fires */
    int             period;             /**< Time in msec between invocations */          
    EjsFunction     *callback;          /**< Callback function */
    EjsFunction     *onerror;           /**< onerror function */
    EjsArray        *args;              /**< Callback args */
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
    EjsPot          pot;                /**< Property storage */
    char            *name;              /**< Optional worker name */
    Ejs             *ejs;               /**< Interpreter */
    EjsAny          *event;             /**< Current event object */
    struct EjsWorker *pair;             /**< Corresponding worker object in other thread */
    char            *scriptFile;        /**< Script or module to run */
    EjsString       *scriptLiteral;     /**< Literal script string to run */
    int             state;              /**< Worker state */
    int             inside;             /**< Running inside the worker */
    int             complete;           /**< Worker has completed its work */
    int             gotMessage;         /**< Worker has received a message */
    int             terminated;         /**< Worker has had terminate() called */
} EjsWorker;

extern EjsWorker *ejsCreateWorker(Ejs *ejs);
extern void ejsRemoveWorkers(Ejs *ejs);

/** 
    Void class
    @description The Void class provides the base class for the singleton "undefined" instance. This instance is stored
        in ejs->undefinedValue..
    @stability Evolving
    @defgroup EjsVoid EjsVoid
    @see EjsVoid
 */

typedef EjsObj EjsVoid;

extern EjsVoid *ejsCreateUndefined(Ejs *ejs);

#define ejsIsUndefined(ejs, obj) (obj && TYPE(obj)->id == ES_Void)

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
    ssize           inputSize;
    ssize           inputPos;
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
    EjsObj          obj;                /**< Base object */
    EjsName         qname;              /**< XML node name (e.g. tagName) */
    int             kind;               /**< Kind of XML node */
    MprList         *elements;          /**< List elements or child nodes */
    MprList         *attributes;        /**< Node attributes */
    MprList         *namespaces;        /**< List of namespaces as Namespace objects */
    struct EjsXML   *parent;            /**< Parent node reference (XML or XMLList) */
    struct EjsXML   *targetObject;      /**< XML/XMLList object modified when items inserted into an empty list */
    EjsName         targetProperty;     /**< XML property modified when items inserted into an empty list */
    EjsString       *value;             /**< Value of text|attribute|comment|pi */
    int             flags;
} EjsXML;


#if DOXYGEN
    /** 
        Determine if a variable is an XML object
        @param Object to test
        @return true if the variable is an XML or XMLList object
        @ingroup EjsXML
     */
    extern boolean ejsIsXML(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsXML(ejs, obj) (obj && ((TYPE(obj)->id == ES_XML) || TYPE(obj)->id == ES_XMLList))
#endif

extern EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName name, EjsXML *parent, EjsString *value);
extern void  ejsLoadXMLString(Ejs *ejs, EjsXML *xml, EjsString *xmlString);
extern void  ejsLoadXMLFromMulti(Ejs *ejs, EjsXML *xml, cchar *xmlString);
extern EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, EjsString *name, EjsXML *parent, EjsString *value);
extern EjsXML *ejsDeepCopyXML(Ejs *ejs, EjsXML *xml);
extern EjsXML *ejsXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName qname);

/*  
    Xml private prototypes
 */
extern void ejsManageXML(EjsXML *xml, int flags);
extern MprXml *ejsCreateXmlParser(Ejs *ejs, EjsXML *xml, cchar *filename);
extern int ejsXMLToString(Ejs *ejs, MprBuf *buf, EjsXML *xml, int indentLevel);
extern EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *xml, EjsXML *node);
extern EjsXML *ejsSetXML(Ejs *ejs, EjsXML *xml, int index, EjsXML *node);
extern int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *node);
extern EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName targetProperty);


extern int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener);
extern int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsObj *listener);
extern int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, int argc, void *argv);
extern int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, EjsAny *arg);


/** 
    Determine if a variable is a number
    @param obj Object to examine
    @return True if the variable is a number
    @ingroup EjsNumber
 */
#if DOXYGEN
    bool ejsIsNumber(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsNumber(ejs, obj) (obj && TYPE(obj)->id == ES_Number)
#endif

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return A numeric value
    @ingroup EjsNumber
 */
extern MprNumber ejsGetNumber(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return An integer value
    @ingroup EjsNumber
 */
extern int ejsGetInt(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreate
    @param obj Object to examine
    @return A double value
    @ingroup EjsNumber
 */
extern double ejsGetDouble(Ejs *ejs, EjsAny *obj);

//  MOB -- rename alloc/free
typedef EjsAny  *(*EjsCreateHelper)(Ejs *ejs, struct EjsType *type, int size);
typedef EjsAny  *(*EjsCastHelper)(Ejs *ejs, EjsAny *obj, struct EjsType *type);
typedef EjsAny  *(*EjsCloneHelper)(Ejs *ejs, EjsAny *obj, bool deep);
typedef int     (*EjsDefinePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                    int64 attributes, EjsAny *value);
typedef int     (*EjsDeletePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsDeletePropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef EjsAny  *(*EjsGetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsGetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef struct EjsTrait *(*EjsGetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsGetPropertyCountHelper)(Ejs *ejs, EjsAny *obj);
typedef EjsName (*EjsGetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsInvokeOperatorHelper)(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
typedef int     (*EjsLookupPropertyHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef int     (*EjsSetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
typedef int     (*EjsSetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
typedef int     (*EjsSetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
typedef int     (*EjsSetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

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
    EjsPot          *prototype;                     /**< Prototype for instances when using prototype inheritance (only) */
    EjsHelpers      helpers;                        /**< Type helper methods */
    struct EjsType  *baseType;                      /**< Base class */
    MprManager      manager;                        /**< Manager callback */
    struct Ejs      *ejs;                           /**< Interpreter reference */
    MprMutex        *mutex;                         /**< Optional locking for types that require it */

    MprList         *implements;                    /**< List of implemented interfaces */
        
    uint            callsSuper           : 1;       /**< Constructor calls super() */
    uint            dynamicInstance      : 1;       /**< Object instances may add properties */
    uint            final                : 1;       /**< Type is final */
    uint            hasBaseConstructors  : 1;       /**< Base types has constructors */
    uint            hasBaseInitializers  : 1;       /**< Base types have initializers */
    uint            hasConstructor       : 1;       /**< Type has a constructor */
    uint            hasInitializer       : 1;       /**< Type has static level initialization code */
    uint            hasInstanceVars      : 1;       /**< Type has non-function instance vars (state) */
    uint            hasMeta              : 1;       /**< Type has meta methods */
    uint            hasScriptFunctions   : 1;       /**< Block has non-native functions requiring namespaces */
    uint            immutable            : 1;       /**< Instances are immutable */
    uint            initialized          : 1;       /**< Static initializer has run */
    uint            isInterface          : 1;       /**< Interface vs class */
    uint            isPot                : 1;       /**< Instances are based on EjsPot */
    uint            needFixup            : 1;       /**< Slots need fixup */
    uint            numericIndicies      : 1;       /**< Instances support direct numeric indicies */
    uint            virtualSlots         : 1;       /**< Properties are not stored in slots[] */
    
    //  MOB -- pack with above?
    ushort          numInherited;                   /**< Number of inherited prototype properties */
    ushort          instanceSize;                   /**< Size of instances in bytes */
    ushort          id;                             /**< Unique type id */
    struct EjsModule *module;                       /**< Module owning the type - stores the constant pool */
    void            *typeData;                      /**< Type specific data */
} EjsType;


#if DOXYGEN
    /** 
        Determine if a variable is an type
        @param obj Object to test
        @return True if the variable is a type
        @ingroup EjsType
     */
    extern bool ejsIsType(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if a variable is a prototype object. Types store the template for instance properties in a prototype object
        @param obj Object to test
        @return True if the variable is a prototype object.
        @ingroup EjsType
     */
    extern bool ejsIsPrototype(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsType(ejs, obj)       (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isType))
    #define ejsIsPrototype(ejs, obj)  (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isPrototype))
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
extern EjsType *ejsCreateType(Ejs *ejs, EjsName name, struct EjsModule *up, EjsType *baseType, EjsPot *prototype,
    int size, int typeId, int numTypeProp, int numInstanceProp, int64 attributes);
extern EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, struct EjsModule *up, EjsType *baseType, 
    int numTypeProp, int numInstanceProp, int64 attributes);

#define EJS_OBJ_HELPERS 1
#define EJS_POT_HELPERS  2

extern EjsType  *ejsCreateNativeType(Ejs *ejs, EjsName qname, int id, int size, void *manager, int helpers);
extern EjsType  *ejsConfigureNativeType(Ejs *ejs, EjsName qname, int size, void *manager, int helpers);

extern EjsObj *ejsCreatePrototype(Ejs *ejs, EjsType *type, int numProp);
extern EjsType *ejsCreateArchetype(Ejs *ejs, struct EjsFunction *fun, EjsPot *prototype);

/** 
    Define a global function
    @description Define a global public function and bind it to the C native function. This is a simple one liner
        to define a public global function. The more typical paradigm to define functions is to create a script file
        of native method definitions and and compile it. This results in a mod file that can be loaded which will
        create the function/method definitions. Then use #ejsBindMethod to associate a C function with a property.
    @ingroup EjsType
 */
//  XX
extern int ejsDefineGlobalFunction(Ejs *ejs, EjsString *name, EjsFun fn);


/** 
    Test if an variable is an instance of a given type
    @description Perform an "is a" test. This tests if a variable is a direct instance or subclass of a given base type.
    @param ejs Interpreter instance returned from #ejsCreate
    @param target Target variable to test.
    @param type Type to compare with the target
    @return True if target is an instance of "type" or an instance of a subclass of "type".
    @ingroup EjsType
 */
extern bool ejsIsA(Ejs *ejs, EjsAny *target, EjsType *type);

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
extern int ejsBindMethod(Ejs *ejs, EjsAny *obj, int slotNum, void *fn);
extern int ejsBindAccess(Ejs *ejs, EjsAny *obj, int slotNum, void *getter, void *setter);
extern void ejsBindConstructor(Ejs *ejs, EjsType *type, void *nativeProc);

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
extern int ejsDefineInstanceProperty(Ejs *ejs, EjsType *type, int slotNum, EjsName name, EjsType *propType, 
    int attributes, EjsAny *value);

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

extern EjsType  *ejsGetTypeByName(Ejs *ejs, EjsName qname);

#define VSPACE(space) space "-" BLD_VNUM
#define ejsGetVType(ejs, space, name) ejsGetTypeByName(ejs, space "-" BLD_VNUM, name)

extern int      ejsCompactClass(Ejs *ejs, EjsType *type);
extern int      ejsCopyBaseProperties(Ejs *ejs, EjsType *type, EjsType *baseType);
extern void     ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type);
extern int      ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
extern void     ejsInitializeBlockHelpers(EjsHelpers *helpers);

extern void     ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName qname);
extern void     ejsTypeNeedsFixup(Ejs *ejs, EjsType *type);
extern int      ejsGetTypeSize(Ejs *ejs, EjsType *type);
extern EjsPot   *ejsGetPrototype(Ejs *ejs, EjsAny *obj);


extern int      ejsBootstrapTypes(Ejs *ejs);
extern void     ejsCreateArrayType(Ejs *ejs);
extern void     ejsCreateBlockType(Ejs *ejs);
extern void     ejsCreateBooleanType(Ejs *ejs);
extern void     ejsCreateConfigType(Ejs *ejs);
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
extern void     ejsCreateTypeType(Ejs *ejs);
extern void     ejsCreateVoidType(Ejs *ejs);
extern void     ejsCreateXMLType(Ejs *ejs);
extern void     ejsCreateXMLListType(Ejs *ejs);

extern void     ejsInitStringType(Ejs *ejs, EjsType *type);

/*  
    Native type configuration
 */
extern void     ejsConfigureAppType(Ejs *ejs);
extern void     ejsConfigureArrayType(Ejs *ejs);
extern void     ejsConfigureBooleanType(Ejs *ejs);
extern void     ejsConfigureByteArrayType(Ejs *ejs);
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
extern void     ejsSetSqliteMemCtx(MprThreadLocal *tls);
extern void     ejsSetSqliteTls(MprThreadLocal *tls);

#if BLD_FEATURE_SQLITE
extern int      ejs_db_sqlite_Init(Ejs *ejs, MprModule *mp);
#endif
extern int      ejs_web_Init(Ejs *ejs, MprModule *mp);

/* 
    Move some ejsWeb.h declarations here so handlers can just include ejs.h whether they are using the
    all-in-one ejs.h or the pure ejs.h
 */
extern HttpStage *ejsAddWebHandler(Http *http, MprModule *module);
extern int ejsHostHttpServer(HttpConn *conn);

/**
    VM Evaluation state. 
    The VM Stacks grow forward in memory. A push is done by incrementing first, then storing. ie. *++top = value
    A pop is done by extraction then decrement. ie. value = *top--
    @ingroup EjsVm
 */
typedef struct EjsState {
    struct EjsFrame     *fp;                /* Current Frame function pointer */
    struct EjsBlock     *bp;                /* Current block pointer */
    EjsObj              **stack;            /* Top of stack (points to the last element pushed) */
    EjsObj              **stackBase;        /* Pointer to start of stack mem */
    struct EjsState     *prev;              /* Previous state */
    struct EjsNamespace *internal;          /* Current internal namespace */
    int                 stackSize;          /* Stack size */
    uint                frozen: 1;          /* Garbage collection frozen */
    EjsObj              *t1;                /* Temp one for GC */
} EjsState;


/**
    Lookup State.
    @description Location information returned when looking up properties.
    @ingroup EjsVm
 */
typedef struct EjsLookup {
    int             slotNum;                /* Final slot in obj containing the variable reference */
    uint            nthBase;                /* Property on Nth super type -- count from the object */
    uint            nthBlock;               /* Property on Nth block in the scope chain -- count from the end */
    EjsType         *type;                  /* Type containing property (if on a prototype obj) */
    //  MOB -- check all these being used
    uint            instanceProperty;       /* Property is an instance property */
    //  MOB -- check all these being used
    uint            ownerIsType;            /* Original object owning the property is a type */
    uint            useThis;                /* Property accessible via "this." */
    EjsAny          *obj;                   /* Final object / Type containing the variable */
    EjsAny          *originalObj;           /* Original object used for the search */
    EjsAny          *ref;                   /* Actual property reference */
    struct EjsTrait *trait;                 /* Property trait describing the property */
    struct EjsName  name;                   /* Name and namespace used to find the property */
    int             bind;                   /* Whether to bind to this lookup */
} EjsLookup;


/**
    Ejscript Service structure
    @description The Ejscript service manages the overall language runtime. It 
        is the factory that creates interpreter instances via #ejsCreate.
    @ingroup EjsService
 */
typedef struct EjsService {
    EjsObj          *(*loadScriptLiteral)(Ejs *ejs, EjsString *script, cchar *cache);
    EjsObj          *(*loadScriptFile)(Ejs *ejs, cchar *path, cchar *cache);
    Ejs             *master;            /**< Master interpreter */
    MprList         *vmlist;
    MprHashTable    *nativeModules;
    Http            *http;
#if FUTURE
    EjsIntern       intern;             /**< Interned Unicode string hash - shared over all interps */
#endif
    MprMutex        *mutex;             /**< Multithread locking */
} EjsService;

extern int ejsInitCompiler(EjsService *service);
extern void ejsAttention(Ejs *ejs);
extern void ejsClearAttention(Ejs *ejs);

/**
    Create an ejs virtual machine 
    @description Create a virtual machine interpreter object to evalute Ejscript programs. Ejscript supports multiple 
        interpreters. One interpreter can be designated as a master interpreter and then it can be cloned by supplying 
        the master interpreter to this call. A master interpreter provides the standard system types and clone interpreters 
        can quickly be created an utilize the master interpreter's types. This saves memory and speeds initialization.
    @param search Module search path to use. Set to NULL for the default search path.
    @param require Optional list of required modules to load. If NULL, the following modules will be loaded:
        ejs, ejs.io, ejs.events, ejs.xml, ejs.sys and ejs.unix.
    @param argc Count of command line argumements in argv
    @param argv Command line arguments
    @param flags Optional flags to modify the interpreter behavior. Valid flags are:
        @li    EJS_FLAG_COMPILER       - Interpreter will compile code from source
        @li    EJS_FLAG_NO_EXE         - Don't execute any code. Just compile.
        @li    EJS_FLAG_DOC            - Load documentation from modules
        @li    EJS_FLAG_NOEXIT         - App should service events and not exit unless explicitly instructed
    @return A new interpreter
    @ingroup Ejs
 */
extern Ejs *ejsCreate(cchar *search, MprList *require, int argc, cchar **argv, int flags);
extern void ejsDestroy(Ejs *ejs);

/**
    Create a search path array. This can be used in ejsCreate.
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
#define EC_FLAGS_DOC             0x100                  /* Parse inline doc */

//  TODO - DOC
extern int ejsLoadScriptFile(Ejs *ejs, cchar *path, cchar *cache, int flags);
extern int ejsLoadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache, int flags);

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
extern EjsAny *ejsThrowException(Ejs *ejs, EjsAny *error);
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

extern EjsAny *ejsCastOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs);
extern int ejsCheckModuleLoaded(Ejs *ejs, cchar *name);
extern void ejsClearExiting(Ejs *ejs);
extern EjsAny *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs);
extern EjsAny *ejsGetVarByName(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);
extern int ejsInitStack(Ejs *ejs);
extern void ejsLog(Ejs *ejs, cchar *fmt, ...);

extern int ejsLookupVar(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);
extern int ejsLookupVarWithNamespaces(Ejs *ejs, EjsObj *obj, EjsName name, EjsLookup *lookup);

extern int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup);
extern int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName);
extern void ejsSetHandle(Ejs *ejs, void *handle);
extern void ejsShowCurrentScope(Ejs *ejs);
extern void ejsShowStack(Ejs *ejs, EjsFunction *fp);
extern void ejsShowBlockScope(Ejs *ejs, EjsBlock *block);
extern int ejsStartMprLogging(char *logSpec);
extern void ejsCreateObjHelpers(Ejs *ejs);
extern void ejsCloneObjHelpers(Ejs *ejs, EjsType *type);
extern void ejsClonePotHelpers(Ejs *ejs, EjsType *type);
extern void ejsCloneBlockHelpers(Ejs *ejs, EjsType *type);
extern int  ejsParseModuleVersion(cchar *name);

extern void ejsLockVm(Ejs *ejs);
extern void ejsUnlockVm(Ejs *ejs);
extern void ejsLockService(Ejs *ejs);
extern void ejsUnlockService(Ejs *ejs);

/*
    A module file may contain multiple logical modules.

    Module File Format and Layout:

    (N) Numbers are 1-3 little-endian encoded bytes using the 0x80 as the continuation character
    (S) Strings are pointers into the constant pool encoded as number offsets. Strings are UTF-8.
    (T) Types are encoded and ored with the type encoding masks below. Types are either: untyped, 
        unresolved or primitive (builtin). The relevant mask is ored into the lower 2 bits. Slot numbers and
        name tokens are shifted up 2 bits. Zero means untyped.

    ModuleHeader
        short       magic
        int         fileVersion
        int         version
        int         flags

    Module
        byte        section
        string      name
        number      version
        number      checksum
        number      constantPoolLength
        block       constantPool

    Dependencies
        byte        section
        string      moduleName
        number      minVersion
        number      maxVersion
        number      checksum
        byte        flags

    Type
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

    Property
        byte        section
        string      name
        string      namespace
        number      attributes
        number      slot
        type        property type

    Function
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

    Exception
        byte        section
        byte        flags
        number      tryStartOffset
        number      tryEndOffset
        number      handlerStartOffset
        number      handlerEndOffset
        number      numOpenBlocks
        type        catchType

    Debug
        byte        section
        number      countOfLines
        string      fileName
        number      startLine
        number      addressOffset      
        ...

    Block
        byte        section
        string      name
        number      slot
        number      propCount

    Documentation
        byte        section
        string      text
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
#define EJS_SECT_DEBUG          3           /* Module dependency */
#define EJS_SECT_DEPENDENCY     4           /* Module dependency */
#define EJS_SECT_CLASS          5           /* Class definition */
#define EJS_SECT_CLASS_END      6           /* End of class definition */
#define EJS_SECT_FUNCTION       7           /* Function */
#define EJS_SECT_FUNCTION_END   8           /* End of function definition */
#define EJS_SECT_BLOCK          9           /* Nested block */
#define EJS_SECT_BLOCK_END      10          /* End of Nested block */
#define EJS_SECT_PROPERTY       11          /* Property (variable) definition */
#define EJS_SECT_EXCEPTION      12          /* Exception definition */
#define EJS_SECT_DOC            13          /* Documentation for an element */
#define EJS_SECT_MAX            14

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
    int32       magic;                      /* Magic number for Ejscript modules */
    int32       fileVersion;                /* Module file format version */
    int32       flags;                      /* Module flags */
} EjsModuleHdr;


typedef struct EjsModule {
    EjsString       *name;                  /* Name of this module */
    EjsString       *vname;                 /* Versioned name */
    int             version;                /* Made with EJS_MAKE_VERSION */
    int             minVersion;             /* Minimum version when used as a dependency */
    int             maxVersion;             /* Maximum version when used as a dependency */
    int             checksum;               /* Checksum of slots and names */

    EjsConstants    *constants;             /* Constant pool */
    EjsFunction     *initializer;           /* Initializer method */
    Ejs             *ejs;                   /* Back reference for GC */

    uint            compiling       : 1;    /* Module currently being compiled from source */
    uint            configured      : 1;    /* Module types have been configured with native code */
    uint            loaded          : 1;    /* Module has been loaded from an external file */
    uint            nativeLoaded    : 1;    /* Backing shared library loaded */
    uint            hasError        : 1;    /* Module has a loader error */
    uint            hasInitializer  : 1;    /* Has initializer function */
    uint            hasNative       : 1;    /* Has native property definitions */
    uint            initialized     : 1;    /* Initializer has run */
    uint            visited         : 1;    /* Module has been traversed */
    int             flags;                  /* Loading flags */

    /*
        Module loading and residuals 
     */
    EjsLoadState    *loadState;             /* State while loading */
    MprList         *dependencies;          /* Module file dependencies. List of EjsModules */
    MprFile         *file;                  /* File handle for loading and code generation */
    MprList         *current;               /* Current stack of open objects */
    EjsFunction     *currentMethod;         /* Current method being loaded */
    EjsBlock        *scope;                 /* Lexical scope chain */
    EjsString       *doc;                   /* Current doc string */
    char            *path;                  /* Module file path name */
    int             firstGlobal;            /* First global property */
    int             lastGlobal;             /* Last global property + 1*/

    /*
        Used during code generation
     */
    struct EcCodeGen *code;                 /* Code generation buffer */
    MprList         *globalProperties;      /* List of global properties */

} EjsModule;


typedef int (*EjsNativeCallback)(Ejs *ejs);

typedef struct EjsNativeModule {
    EjsNativeCallback callback;             /* Callback to configure module native types and properties */
    char            *name;                  /* Module name */
    int             checksum;               /* Checksum expected by native code */
    int             flags;                  /* Configuration flags */
} EjsNativeModule;

/*
    Documentation string information
    Element documentation string. The loader will create if required.
 */
typedef struct EjsDoc {
    EjsString   *docString;                         /* Original doc string */
    MprChar     *brief;
    MprChar     *description;
    MprChar     *example;
    MprChar     *requires;
    MprChar     *returns;
    MprChar     *stability;                         /* prototype, evolving, stable, mature, deprecated */
    MprChar     *spec;                              /* Where specified */
    struct EjsDoc *duplicate;                       /* From @duplicate directive */
    MprList     *defaults;                          /* Parameter default values */
    MprList     *params;                            /* Function parameters */
    MprList     *options;                           /* Option parameter values */
    MprList     *events;                            /* Option parameter values */
    MprList     *see;
    MprList     *throws;
    EjsTrait    *trait;                             /* Back pointer to trait */
    int         deprecated;                         /* Hide doc if true */
    int         hide;                               /* Hide doc if true */
    int         cracked;                            /* Doc has been cracked and tokenized */
} EjsDoc;


/*
    Loader flags
 */
#define EJS_LOADER_STRICT     0x1
#define EJS_LOADER_NO_INIT    0x2
#define EJS_LOADER_ETERNAL    0x4                   /* Make all types eternal */
#define EJS_LOADER_BUILTIN    0x8                   /* Loading builtins */
#define EJS_LOADER_DEP        0x10                  /* Loading a dependency */

//  MOB -- would this be better with an ascii name?
extern int          ejsLoadModule(Ejs *ejs, EjsString *name, int minVer, int maxVer, int flags);
extern char         *ejsSearchForModule(Ejs *ejs, cchar *name, int minVer, int maxVer);
extern int          ejsSetModuleConstants(Ejs *ejs, EjsModule *mp, EjsConstants *constants);

extern void         ejsModuleReadBlock(Ejs *ejs, EjsModule *module, char *buf, int len);
extern int          ejsModuleReadByte(Ejs *ejs, EjsModule *module);
extern EjsString    *ejsModuleReadConst(Ejs *ejs, EjsModule *module);
extern int          ejsModuleReadInt(Ejs *ejs, EjsModule *module);
extern int          ejsModuleReadInt32(Ejs *ejs, EjsModule *module);
extern EjsName      ejsModuleReadName(Ejs *ejs, EjsModule *module);
extern int64        ejsModuleReadNum(Ejs *ejs, EjsModule *module);
extern char         *ejsModuleReadMulti(Ejs *ejs, EjsModule *mp);
extern MprChar      *ejsModuleReadMultiAsWide(Ejs *ejs, EjsModule *mp);
extern int          ejsModuleReadType(Ejs *ejs, EjsModule *module, EjsType **typeRef, EjsTypeFixup **fixup, 
                        EjsName *typeName, int *slotNum);

extern double       ejsDecodeDouble(Ejs *ejs, uchar **pp);
extern int          ejsDecodeInt32(Ejs *ejs, uchar **pp);
extern int64        ejsDecodeNum(Ejs *ejs, uchar **pp);

extern int          ejsEncodeByteAtPos(Ejs *ejs, uchar *pos, int value);
extern int          ejsEncodeDouble(Ejs *ejs, uchar *pos, double number);
extern int          ejsEncodeInt32(Ejs *ejs, uchar *pos, int number);
extern int          ejsEncodeNum(Ejs *ejs, uchar *pos, int64 number);
extern int          ejsEncodeInt32AtPos(Ejs *ejs, uchar *pos, int value);

extern double       ejsSwapDouble(Ejs *ejs, double a);
extern int          ejsSwapInt32(Ejs *ejs, int word);
extern int64        ejsSwapInt64(Ejs *ejs, int64 word);

extern char         *ejsGetDocKey(Ejs *ejs, EjsBlock *block, int slotNum, char *buf, int bufsize);
extern EjsDoc       *ejsCreateDoc(Ejs *ejs, void *vp, int slotNum, EjsString *docString);

extern int          ejsAddModule(Ejs *ejs, EjsModule *up);
extern EjsModule    *ejsLookupModule(Ejs *ejs, EjsString *name, int minVersion, int maxVersion);
extern void         ejsRemoveModule(Ejs *ejs, EjsModule *up);
extern void         ejsRemoveModules(Ejs *ejs);

extern int  ejsAddNativeModule(Ejs *ejs, cchar *name, EjsNativeCallback callback, int checksum, int flags);
extern EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, cchar *name);

extern EjsModule *ejsCreateModule(Ejs *ejs, EjsString *name, int version, EjsConstants *constants);

#ifdef __cplusplus
}
#endif

/*
    Allow user overrides
 */
#include    "customize.h"

#endif /* _h_EJS_CORE */

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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

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
    EjsPot          pot;                        /**< Extends Object */
    Ejs             *ejs;                       /**< Ejscript interpreter handle */
    HttpServer      *server;                    /**< Http server object */
    MprEvent        *sessionTimer;              /**< Session expiry timer */
    struct MprSsl   *ssl;                       /**< SSL configuration */
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
    EjsObj          *emitter;                   /**< Event emitter */
    EjsObj          *limits;                    /**< Limits object */
    EjsPot          *sessions;                  /**< Session cache */
    EjsArray        *incomingStages;            /**< Incoming Http pipeline stages */
    EjsArray        *outgoingStages;            /**< Outgoing Http pipeline stages */
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
    EjsPot          pot;                /**< Base object storage */
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
    int             dontAutoFinalize;   /**< Suppress auto-finalization */
    int             probedSession;      /**< Determined if a session exists */
    int             closed;             /**< Request closed and "close" event has been issued */
    int             error;              /**< Request errored and "error" event has been issued */
    int             responded;          /**< Request has done some output or changed status */
    int             running;            /**< Request has started */
    ssize           written;            /**< Count of data bytes written to the client */
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
typedef struct EjsSession {
    EjsPot      pot;
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

extern void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, int timeout);
extern void ejsUpdateSessionLimits(Ejs *ejs, EjsHttpServer *server);

extern void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req);
extern void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req);
extern void ejsStopSessionTimer(EjsHttpServer *server);


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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

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


#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct EcLocation {
    MprChar     *source;
    char        *filename;
    int         lineNumber;
    int         column;
} EcLocation;

#define N_ARGS                  1
#define N_ASSIGN_OP             2
#define N_ATTRIBUTES            3
#define N_BINARY_OP             4
#define N_BINARY_TYPE_OP        5
#define N_BLOCK                 6
#define N_BREAK                 7
#define N_CALL                  8
#define N_CASE_ELEMENTS         9
#define N_CASE_LABEL            10
#define N_CATCH                 11
#define N_CATCH_ARG             12
#define N_CATCH_CLAUSES         13
#define N_CLASS                 14
#define N_CONTINUE              15
#define N_DASSIGN               16
#define N_DIRECTIVES            17
#define N_DO                    18
#define N_DOT                   19
#define N_END_FUNCTION          20
#define N_EXPRESSIONS           21
#define N_FIELD                 22
#define N_FOR                   23
#define N_FOR_IN                24
#define N_FUNCTION              25
#define N_GOTO                  26
#define N_HASH                  27
#define N_IF                    28
#define N_LITERAL               29
#define N_MODULE                30
#define N_NEW                   31
#define N_NOP                   32
#define N_OBJECT_LITERAL        33
#define N_PARAMETER             34
#define N_POSTFIX_OP            35
#define N_PRAGMA                36
#define N_PRAGMAS               37
#define N_PROGRAM               38
#define N_QNAME                 39
#define N_REF                   40
#define N_RETURN                41
#define N_SPREAD                42
#define N_SUPER                 43
#define N_SWITCH                44
#define N_THIS                  45
#define N_THROW                 46
#define N_TRY                   47
#define N_TYPE_IDENTIFIERS      48
#define N_UNARY_OP              49
#define N_USE_MODULE            50
#define N_USE_NAMESPACE         51
#define N_VALUE                 52
#define N_VAR                   53
#define N_VAR_DEFINITION        54
#define N_VOID                  55
#define N_WITH                  56

/*
    Ast node define
 */
#if !DOXYGEN
typedef struct EcNode   *Node;
#endif

/*
    Structure for code generation buffers
 */
typedef struct EcCodeGen {
    MprBuf      *buf;                           /* Code generation buffer */
    MprList     *jumps;                         /* Break/continues to patch for this code block */
    MprList     *exceptions;                    /* Exception handlers for this code block */
    EjsDebug    *debug;                         /* Source debug info */ 
    int         jumpKinds;                      /* Kinds of jumps allowed */
    int         breakMark;                      /* Stack item counter for the target for break/continue stmts */
    int         blockMark;                      /* Lexical block counter for the target for break/continue stmts */
    int         stackCount;                     /* Current stack item counter */
    int         blockCount;                     /* Current block counter */
    int         lastLineNumber;                 /* Last line for debug */
} EcCodeGen;


typedef struct EcNode {
    char                *kindName;              /* Node kind string */
#if BLD_DEBUG
    char                *tokenName;
#endif
    EjsName             qname;
    EcLocation          loc;                    /* Source code info */
    EjsBlock            *blockRef;              /* Block scope */
    EjsLookup           lookup;                 /* Lookup residuals */
    EjsNamespace        *namespaceRef;          /* Namespace reference */
    Node                left;                   /* children[0] */
    Node                right;                  /* children[1] */
    Node                typeNode;               /* Type of name */
    Node                parent;                 /* Parent node */
    MprList             *namespaces;            /* Namespaces for hoisted variables */
    MprList             *children;

    int                 kind;                   /* Kind of node */
    int                 attributes;             /* Attributes applying to this node */
    int                 tokenId;                /* Lex token */
    int                 groupMask;              /* Token group */
    int                 subId;                  /* Sub token */
    int                 slotNum;                /* Allocated slot for variable */
    int                 jumpLength;             /* Goto length for exceptions */
    int                 seqno;                  /* Unique sequence number */

    uint                blockCreated      : 1;  /* Block object has been created */
    uint                createBlockObject : 1;  /* Create the block object to contain let scope variables */
    uint                enabled           : 1;  /* Node is enabled via conditional definitions */
    int                 literalNamespace  : 1;  /* Namespace is a literal */
    uint                needThis          : 1;  /* Need to push this object */
    uint                needDupObj        : 1;  /* Need to dup the object on stack (before) */
    uint                needDup           : 1;  /* Need to dup the result (after) */
    uint                slotFixed         : 1;  /* Slot fixup has been done */
    uint                specialNamespace  : 1;  /* Using a public|private|protected|internal namespace */

    uchar               *patchAddress;          /* For code patching */
    EcCodeGen           *code;                  /* Code buffer */

    EjsName             *globalProp;            /* Set if this is a global property */
    EjsString           *doc;                   /* Documentation string */

    struct EcCompiler   *cp;                    /* Compiler instance reference */

#if BLD_CC_UNNAMED_UNIONS
    union {
#endif
        struct {
            Node        expression;
            EcCodeGen   *expressionCode;        /* Code buffer for the case expression */
            int         kind;
            int         nextCaseCode;           /* Goto length to the next case statements */
        } caseLabel;

        struct {
            Node        arg;                    /* Catch block argument */
        } catchBlock;

        /*
            Var definitions have one child per variable. Child nodes can be either N_NAME or N_ASSIGN_OP
         */
        struct {
            int         varKind;                /* Variable definition kind */
        } def;

        struct {
            /* Children are the catch clauses */
            Node        tryBlock;               /* Try code */
            Node        catchClauses;           /* Catch clauses */
            Node        finallyBlock;           /* Finally code */
            int         numBlocks;              /* Count of open blocks in the try block */
        } exception;

        struct {
            Node        expr;                   /* Field expression */
            Node        fieldName;              /* Field element name for objects */
            int         fieldKind;              /* value or function */
            int         index;                  /* Array index, set to -1 for objects */
            int         varKind;                /* Variable definition kind (const) */
        } field;

        struct {
            Node        resultType;             /* Function return type */
            Node        body;                   /* Function body */
            Node        parameters;             /* Function formal parameters */
            Node        constructorSettings;    /* Constructor settings */
#if UNUSED
            Node        expressionRef;          /* Reference to the function expression name */
#endif
            EjsFunction *functionVar;           /* Function variable */
            uint        operatorFn    : 1;      /* operator function */
            uint        getter        : 1;      /* getter function */
            uint        setter        : 1;      /* setter function */
            uint        call          : 1;      /* */
            uint        has           : 1;      /* */
            uint        hasRest       : 1;      /* Has rest parameter */
            uint        hasReturn     : 1;      /* Has a return statement */
            uint        isMethod      : 1;      /* Is a class method */
            uint        isConstructor : 1;      /* Is constructor method */
            uint        isDefault     : 1;      /* Is default constructor */
            uint        isExpression  : 1;      /* Is a function expression */
        } function;

        struct {
            Node        iterVar;
            Node        iterGet;
            Node        iterNext;
            Node        body;
            EcCodeGen   *initCode;
            EcCodeGen   *bodyCode;
            int         each;                   /* For each used */
        } forInLoop;

        struct {
            Node        body;
            Node        cond;
            Node        initializer;
            Node        perLoop;
            EcCodeGen   *condCode;
            EcCodeGen   *bodyCode;
            EcCodeGen   *perLoopCode;
        } forLoop;

        struct {
            Node        body;
            Node        expr;
            bool        disabled;
        } hash;

        struct {
            Node         implements;          /* Implemented interfaces */
            Node         constructor;         /* Class constructor */
            MprList      *staticProperties;   /* List of static properties */
            MprList      *instanceProperties; /* Implemented interfaces */
            MprList      *classMethods;       /* Static methods */
            MprList      *methods;            /* Instance methods */
            EjsType      *ref;                /* Type instance ref */
            EjsFunction  *initializer;        /* Initializer function */
            EjsNamespace *publicSpace;
            EjsNamespace *internalSpace;
            EjsString    *extends;            /* Class base class */
            int          isInterface;         /* This is an interface */
        } klass;

        struct {
            EjsObj      *var;               /* Special value */
            MprBuf      *data;              /* XML data */
        } literal;

        struct {
            EjsModule   *ref;               /* Module object */
            EjsString   *name;              /* Module name */
            char        *filename;          /* Module file name */
            int         version;
        } module;

        /*
            Name nodes hold a fully qualified name.
         */
        struct {
            Node        nameExpr;           /* Name expression */
            Node        qualifierExpr;      /* Qualifier expression */
            EjsObj      *nsvalue;           /* Initialization value (MOB - remove) */
            uint        instanceVar  : 1;   /* Instance or static var (if defined in class) */
            uint        isAttribute  : 1;   /* Attribute identifier "@" */
            uint        isDefault    : 1;   /* use default namespace */
            uint        isInternal   : 1;   /* internal namespace */
            uint        isLiteral    : 1;   /* use namespace "literal" */
            uint        isNamespace  : 1;   /* Name is a namespace */
            uint        isRest       : 1;   /* ... rest style args */
            uint        isType       : 1;   /* Name is a type */
            uint        letScope     : 1;   /* Variable is defined in let block scope */
            int         varKind;            /* Variable definition kind */
        } name;

        struct {
            int         callConstructors;   /* Bound type has a constructor */
        } newExpr;

        struct {
            Node        typeNode;           /* Type of object */
            int         isArray;            /* Array literal */
        } objectLiteral;

        struct {
            uint        strict;             /* Strict mode */
        } pragma;

        struct {
            MprList     *dependencies;      /* Accumulated list of dependent modules */
        } program;

        struct {
            Node        node;               /* Actual node reference */
        } ref;

        struct {
            int         blockless;          /* Function expression */
        } ret;

        struct {
            Node        cond;
            Node        thenBlock;
            Node        elseBlock;
            EcCodeGen   *thenCode;
            EcCodeGen   *elseCode;
        } tenary;

        struct {
            int         thisKind;           /* Kind of this. See EC_THIS_ flags */
        } thisNode;

        struct {
            int         minVersion;
            int         maxVersion;
        } useModule;

        struct {
            Node        object;
            Node        statement;
        } with;
#if BLD_CC_UNNAMED_UNIONS
    };
#endif
} EcNode;


/*
    Various per-node flags
 */
#define EC_THIS_GENERATOR       1
#define EC_THIS_CALLEE          2
#define EC_THIS_TYPE            3
#define EC_THIS_FUNCTION        4

#define EC_SWITCH_KIND_CASE     1   /* Case block */
#define EC_SWITCH_KIND_DEFAULT  2   /* Default block */

/*
    Object (and Array) literal field
 */
#define FIELD_KIND_VALUE        0x1
#define FIELD_KIND_FUNCTION     0x2

#define EC_NUM_NODES            8
#define EC_TAB_WIDTH            4

/*
    Fix clash with arpa/nameser.h
 */
#undef T_NULL

/*
    Lexical tokens (must start at 1)
    ASSIGN tokens must be +1 compared to their non-assignment counterparts.
    (Use genTokens to recreate)
    WARNING: ensure T_MOD and T_MOD_ASSIGN are adjacent. rewriteCompoundAssignment relies on this
 */
#define T_ASSIGN                    1
#define T_AT                        2
#define T_ATTRIBUTE                 3
#define T_BIT_AND                   4
#define T_BIT_AND_ASSIGN            5
#define T_BIT_OR                    6
#define T_BIT_OR_ASSIGN             7
#define T_BIT_XOR                   8
#define T_BIT_XOR_ASSIGN            9
#define T_BREAK                    10
#define T_CALL                     11
#define T_CALLEE                   12
#define T_CASE                     13
#define T_CAST                     14
#define T_CATCH                    15
#define T_CDATA_END                16
#define T_CDATA_START              17
#define T_CLASS                    18
#define T_COLON                    19
#define T_COLON_COLON              20
#define T_COMMA                    21
#define T_CONST                    22
#define T_CONTEXT_RESERVED_ID      23
#define T_CONTINUE                 24
#define T_DEBUGGER                 25
#define T_DECREMENT                26
#define T_DEFAULT                  27
#define T_DELETE                   28
#define T_DIV                      29
#define T_DIV_ASSIGN               30
#define T_DO                       31
#define T_DOT                      32
#define T_DOT_DOT                  33
#define T_DOT_LESS                 34
#define T_DOUBLE                   35
#define T_DYNAMIC                  36
#define T_EACH                     37
#define T_ELIPSIS                  38
#define T_ELSE                     39
#define T_ENUMERABLE               40
#define T_EOF                      41
#define T_EQ                       42
#define T_ERR                      43
#define T_EXTENDS                  44
#define T_FALSE                    45
#define T_FINAL                    46
#define T_FINALLY                  47
#define T_FLOAT                    48
#define T_FOR                      49
#define T_FUNCTION                 50
#define T_GE                       51
#define T_GENERATOR                52
#define T_GET                      53
#define T_GOTO                     54
#define T_GT                       55
#define T_HAS                      56
#define T_HASH                     57
#define T_ID                       58
#define T_IF                       59
#define T_IMPLEMENTS               60
#define T_IN                       61
#define T_INCLUDE                  62
#define T_INCREMENT                63
#define T_INSTANCEOF               64
#define T_INT                      65
#define T_INTERFACE                66
#define T_INTERNAL                 67
#define T_INTRINSIC                68
#define T_IS                       69
#define T_LBRACE                   70
#define T_LBRACKET                 71
#define T_LE                       72
#define T_LET                      73
#define T_LOGICAL_AND              74
#define T_LOGICAL_AND_ASSIGN       75
#define T_LOGICAL_NOT              76
#define T_LOGICAL_OR               77
#define T_LOGICAL_OR_ASSIGN        78
#define T_LOGICAL_XOR              79
#define T_LOGICAL_XOR_ASSIGN       80
#define T_LPAREN                   81
#define T_LSH                      82
#define T_LSH_ASSIGN               83
#define T_LT                       84
#define T_LT_SLASH                 85
#define T_MINUS                    86
#define T_MINUS_ASSIGN             87
#define T_MINUS_MINUS              88
#define T_MODULE                   89
#define T_MOD                      90       // WARNING sorted order manually fixed!!
#define T_MOD_ASSIGN               91
#define T_MUL                      92
#define T_MUL_ASSIGN               93
#define T_NAMESPACE                94
#define T_NATIVE                   95
#define T_NE                       96
#define T_NEW                      97
#define T_NOP                      98
#define T_NULL                     99
#define T_NUMBER                  100
#define T_NUMBER_WORD             101
#define T_OVERRIDE                102
#define T_PLUS                    103
#define T_PLUS_ASSIGN             104
#define T_PLUS_PLUS               105
#define T_PRIVATE                 106
#define T_PROTECTED               107
#define T_PROTOTYPE               108
#define T_PUBLIC                  109
#define T_QUERY                   110
#define T_RBRACE                  111
#define T_RBRACKET                112
#define T_REGEXP                  113
#define T_REQUIRE                 114
#define T_RESERVED_NAMESPACE      115
#define T_RETURN                  116
#define T_RPAREN                  117
#define T_RSH                     118
#define T_RSH_ASSIGN              119
#define T_RSH_ZERO                120
#define T_RSH_ZERO_ASSIGN         121
#define T_SEMICOLON               122
#define T_SET                     123
#define T_SLASH_GT                124
#define T_STANDARD                125
#define T_STATIC                  126
#define T_STRICT                  127
#define T_STRICT_EQ               128
#define T_STRICT_NE               129
#define T_STRING                  130
#define T_SUPER                   131
#define T_SWITCH                  132
#define T_THIS                    133
#define T_THROW                   134
#define T_TILDE                   135
#define T_TO                      136
#define T_TRUE                    137
#define T_TRY                     138
#define T_TYPE                    139
#define T_TYPEOF                  140
#define T_UINT                    141
#define T_UNDEFINED               142
#define T_USE                     143
#define T_VAR                     144
#define T_VOID                    145
#define T_WHILE                   146
#define T_WITH                    147
#define T_XML_COMMENT_END         148
#define T_XML_COMMENT_START       149
#define T_XML_PI_END              150
#define T_XML_PI_START            151
#define T_YIELD                   152

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
    struct EcCompiler *compiler;                /* Compiler back reference */
    EcLocation  loc;                            /* Source code debug info */
    EcLocation  lastLoc;                        /* Location info for a prior line */
    EcStreamGet getInput;                       /* Get more input callback */
    MprChar     *buf;                           /* Buffer holding source file */
    MprChar     *nextChar;                      /* Ptr to next input char */
    MprChar     *end;                           /* Ptr to one past end of buf */
    bool        eof;                            /* At end of file */
    int         flags;                          /* Input flags */
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
} EcConsoleStream;


/*
    Program source input tokens
 */
typedef struct EcToken {
    MprChar     *text;                  /* Token text */
    int         length;                 /* Length of text in characters */
    int         size;                   /* Size of text in characters */
    int         tokenId;
    int         subId;
    int         groupMask;
    int         eol;                    /* At the end of the line */
    EcLocation  loc;                    /* Source code debug info */
    struct EcToken *next;               /* Putback and freelist linkage */
    EcStream    *stream;
#if BLD_DEBUG
    char        *name;                  /* Debug token name */
#endif
} EcToken;


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
    Current parse state. Each non-terminal production has its own state.
    Some state fields are inherited. We keep a linked list from EcCompiler.
 */
typedef struct EcState {
    struct EcState  *next;                  /* State stack */
    uint            blockIsMethod    : 1;   /* Current function is a method */
    uint            captureBreak     : 1;   /* Capture break/continue inside a catch/finally block */
    uint            captureFinally   : 1;   /* Capture break/continue with a finally block */
    uint            conditional      : 1;   /* In branching conditional */
    uint            disabled         : 1;   /* Disable nodes below this scope */
    uint            dupLeft          : 1;   /* Dup left side */
    uint            inClass          : 1;   /* Inside a class declaration */
    uint            inFunction       : 1;   /* Inside a function declaration */
    uint            inHashExpression : 1;   /* Inside a # expression */
    uint            inInterface      : 1;   /* Inside an interface */
    uint            inMethod         : 1;   /* Inside a method declaration */
    uint            inSettings       : 1;   /* Inside constructor settings */
    uint            instanceCode     : 1;   /* Generating instance class code */
    uint            needsValue       : 1;   /* Expression must yield a value */
    uint            noin             : 1;   /* Don't allow "in" */
    uint            onLeft           : 1;   /* On the left of an assignment */
    uint            saveOnLeft       : 1;   /* Saved left of an assignment */
    uint            strict           : 1;   /* Compiler checking mode: Strict, standard*/

    int             blockNestCount;         /* Count of blocks encountered. Used by ejs shell */
    int             namespaceCount;         /* Count of namespaces originally in block. Used to pop namespaces */

    EjsModule       *currentModule;         /* Current open module definition */
    EjsType         *currentClass;          /* Current open class */
    EjsName         currentClassName;       /* Current open class name */
    EcNode          *currentClassNode;      /* Current open class */
    EjsFunction     *currentFunction;       /* Current open method */
    EcNode          *currentFunctionNode;   /* Current open method */
    EcNode          *currentObjectNode;     /* Left object in "." or "[" */
    EcNode          *topVarBlockNode;       /* Top var block node */

    EjsBlock        *letBlock;              /* Block for local block scope declarations */
    EjsBlock        *varBlock;              /* Block for var declarations */
    EjsBlock        *optimizedLetBlock;     /* Optimized let block declarations - may equal ejs->global */
    EcNode          *letBlockNode;          /* Node for the current let block */

    EjsString       *nspace;                /* Namespace for declarations */
    EjsString       *defaultNamespace;      /* Default namespace for new top level declarations. Does not propagate */

    EcCodeGen       *code;                  /* Global and function code buffer */
    EcCodeGen       *staticCodeBuf;         /* Class static level code generation buffer */
    EcCodeGen       *instanceCodeBuf;       /* Class instance level code generation buffer */

    struct EcState  *prevBlockState;        /* Block state stack */
    struct EcState  *breakState;            /* State for breakable blocks */
    struct EcState  *classState;            /* State for current class */
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
    EcToken     *peekToken;                 /* Peek ahead token */
    EcToken     *token;                     /* Current input token */

    /*  Lexer */
    MprHashTable *keywords;
    EcStream    *stream;
    EcToken     *putback;                   /* List of active putback tokens */
    EjsString   *docToken;                  /* Last doc token */

    EcState     *fileState;                 /* Top level state for the file */
//  MOB -- these are risky and should be moved into state. A nested block, directive class etc willl modify
    EcState     *directiveState;            /* State for the current directive - used in parse and CodeGen */
    EcState     *blockState;                /* State for the current block */

    EjsLookup   lookup;                     /* Lookup residuals */
    EjsService  *vmService;                 /* VM runtime */
    Ejs         *ejs;                       /* Interpreter instance */
    MprList     *nodes;                     /* Compiled AST nodes */

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
    int         errorCount;                 /* Count of all errors */
    int         warningCount;               /* Count of all warnings */
    int         nextSeqno;                  /* Node sequence numbers */
    int         blockLevel;                 /* Level of nest in blocks */

    /*
        TODO - aggregate these into flags
     */
    int         lastOpcode;                 /* Last opcode encoded */
    int         uid;                        /* Unique identifier generator */
} EcCompiler;


//  MOB -- reorder
extern int          ecAddModule(EcCompiler *cp, EjsModule *mp);
extern EcNode       *ecAppendNode(EcNode *np, EcNode *child);
extern int          ecAstFixup(EcCompiler *cp, struct EcNode *np);
extern EcNode       *ecChangeNode(EcCompiler *cp, EcNode *np, EcNode *oldNode, EcNode *newNode);
extern void         ecGenConditionalCode(EcCompiler *cp, EcNode *np, EjsModule *up);
extern int          ecCodeGen(EcCompiler *cp);
extern int          ecCompile(EcCompiler *cp, int argc, char **path);
extern EcCompiler   *ecCreateCompiler(struct Ejs *ejs, int flags);
extern void         ecDestroyCompiler(EcCompiler *cp);
extern void         ecInitLexer(EcCompiler *cp);
extern EcNode       *ecCreateNode(EcCompiler *cp, int kind);
extern void         ecFreeToken(EcCompiler *cp, EcToken *token);
extern char         *ecGetErrorMessage(EcCompiler *cp);
extern EjsString    *ecGetInputStreamName(EcCompiler *lp);
extern int          ecGetToken(EcCompiler *cp);
extern int          ecGetRegExpToken(EcCompiler *cp, MprChar *prefix);
extern EcNode       *ecLinkNode(EcNode *np, EcNode *child);

extern EjsModule    *ecLookupModule(EcCompiler *cp, EjsString *name, int minVersion, int maxVersion);
extern int          ecLookupScope(EcCompiler *cp, EjsName name);
extern int          ecLookupVar(EcCompiler *cp, EjsObj *vp, EjsName name);
extern EcNode       *ecParseWarning(EcCompiler *cp, char *fmt, ...);
extern int          ecPeekToken(EcCompiler *cp);
extern int          ecPutSpecificToken(EcCompiler *cp, EcToken *token);
extern int          ecPutToken(EcCompiler *cp);
extern void         ecError(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, ...);
extern void         ecErrorv(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, va_list args);
extern void         ecResetInput(EcCompiler *cp);
extern EcNode       *ecResetError(EcCompiler *cp, EcNode *np, bool eatInput);
extern int          ecRemoveModule(EcCompiler *cp, EjsModule *mp);
extern void         ecResetParser(EcCompiler *cp);
extern int          ecResetModuleList(EcCompiler *cp);
extern int          ecOpenConsoleStream(EcCompiler *cp, EcStreamGet gets, cchar *contents);
extern int          ecOpenFileStream(EcCompiler *cp, cchar *path);
extern int          ecOpenMemoryStream(EcCompiler *cp, cchar *contents, ssize len);
extern void         ecCloseStream(EcCompiler *cp);
extern void         ecSetOptimizeLevel(EcCompiler *cp, int level);
extern void         ecSetWarnLevel(EcCompiler *cp, int level);
extern void         ecSetStrictMode(EcCompiler *cp, int on);
extern void         ecSetTabWidth(EcCompiler *cp, int width);
extern void         ecSetOutputFile(EcCompiler *cp, cchar *outputFile);
extern void         ecSetCertFile(EcCompiler *cp, cchar *certFile);
extern EcToken      *ecTakeToken(EcCompiler *cp);
extern int          ecAstProcess(struct EcCompiler *cp);
extern void         *ecCreateStream(EcCompiler *cp, ssize size, cchar *filename, void *manager);
extern void         ecSetStreamBuf(EcStream *sp, cchar *contents, ssize len);
extern EcNode       *ecParseFile(EcCompiler *cp, char *path);
extern void         ecManageStream(EcStream *sp, int flags);
extern void         ecMarkLocation(EcLocation *loc);
extern void         ecSetRequire(EcCompiler *cp, MprList *modules);


/*
    Module file creation routines.
 */
extern void     ecAddFunctionConstants(EcCompiler *cp, EjsPot *obj, int slotNum);
extern void     ecAddConstants(EcCompiler *cp, EjsAny *obj);
extern int      ecAddStringConstant(EcCompiler *cp, EjsString *sp);
extern int      ecAddCStringConstant(EcCompiler *cp, cchar *str);
extern int      ecAddNameConstant(EcCompiler *cp, EjsName qname);
extern int      ecAddDocConstant(EcCompiler *cp, void *vp, int slotNum);
extern int      ecAddModuleConstant(EcCompiler *cp, EjsModule *up, cchar *str);
extern int      ecCreateModuleHeader(EcCompiler *cp);
extern int      ecCreateModuleSection(EcCompiler *cp);


/*
    Encoding emitter routines
 */
extern void      ecEncodeBlock(EcCompiler *cp, cuchar *buf, int len);
extern void      ecEncodeByte(EcCompiler *cp, int value);
extern void      ecEncodeByteAtPos(EcCompiler *cp, int offset, int value);
extern void      ecEncodeConst(EcCompiler *cp, EjsString *sp);
extern void      ecEncodeDouble(EcCompiler *cp, double value);
extern void      ecEncodeGlobal(EcCompiler *cp, EjsAny *obj, EjsName qname);
extern void      ecEncodeInt32(EcCompiler *cp, int value);
extern void      ecEncodeInt32AtPos(EcCompiler *cp, int offset, int value);
extern void      ecEncodeNum(EcCompiler *cp, int64 number);
extern void      ecEncodeName(EcCompiler *cp, EjsName qname);
extern void      ecEncodeMulti(EcCompiler *cp, cchar *str);
extern void      ecEncodeWideAsMulti(EcCompiler *cp, MprChar *str);
extern void      ecEncodeOpcode(EcCompiler *cp, int value);

extern void     ecCopyCode(EcCompiler *cp, uchar *pos, int size, int dist);
extern uint     ecGetCodeOffset(EcCompiler *cp);
extern int      ecGetCodeLen(EcCompiler *cp, uchar *mark);
extern void     ecAdjustCodeLength(EcCompiler *cp, int adj);

#ifdef __cplusplus
}
#endif
#endif /* _h_EC_COMPILER */

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
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/include/ecCompiler.h"
 */
/************************************************************************/

#include "ejs.slots.h"
