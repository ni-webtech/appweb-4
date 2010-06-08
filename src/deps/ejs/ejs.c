#include "ejs.h"

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
 *  Start of file "../src/cmd/ejs.c"
 */
/************************************************************************/

/**
    ejs.c - Ejscript shell

    Interactive shell that interprets interactive sessions and command files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



#if BLD_CC_EDITLINE
  #include  <histedit.h>
#endif


static int  consoleGets(EcStream *stream);
static int  commandGets(EcStream *stream);
static int  interpretCommands(EcCompiler*cp, cchar *cmd);
static int  interpretFiles(EcCompiler *cp, MprList *files, int argc, char **argv, cchar *className, cchar *method);
static void require(MprList *list, cchar *name);
static void setupUnixSignals(Mpr *mpr);


MAIN(ejsMain, int argc, char **argv)
{
    Mpr             *mpr;
    EcCompiler      *cp;
    EjsService      *ejsService;
    Ejs             *ejs;
    MprList         *requiredModules, *files;
    cchar           *cmd, *className, *method;
    char            *argp, *searchPath, *modules, *name, *tok, *extraFiles;
    int             nextArg, err, ecFlags, stats, merge, bind, noout, debug, optimizeLevel, warnLevel, strict;

    /*  
        Create the Embedthis Portable Runtime (MPR) and setup a memory failure handler
     */
    mpr = mprCreate(argc, argv, ejsMemoryFailure);
    mprSetAppName(mpr, argv[0], 0, 0);
    setupUnixSignals(mpr);

    if (mprStart(mpr) < 0) {
        mprError(mpr, "Can't start mpr services");
        return EJS_ERR;
    }

    err = 0;
    className = 0;
    cmd = 0;
    method = 0;
    searchPath = 0;
    stats = 0;
    merge = 0;
    bind = 1;
    noout = 1;
    debug = 1;
    warnLevel = 1;
    optimizeLevel = 9;
    requiredModules = 0;
    strict = 0;

    files = mprCreateList(mpr);

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }

        if (strcmp(argp, "--bind") == 0) {
            bind = 1;

        } else if (strcmp(argp, "--class") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                className = argv[++nextArg];
            }

        } else if (strcmp(argp, "--cmd") == 0 || strcmp(argp, "-c") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                cmd = argv[++nextArg];
            }

        } else if (strcmp(argp, "--debug") == 0) {
            debug = 1;

        } else if (strcmp(argp, "--files") == 0 || strcmp(argp, "-f") == 0) {
            /* Compatibility with mozilla shell */
            if (nextArg >= argc) {
                err++;
            } else {
                extraFiles = mprStrdup(mpr, argv[++nextArg]);
                name = mprStrTok(extraFiles, " \t", &tok);
                while (name != NULL) {
                    mprAddItem(files, name);
                    name = mprStrTok(NULL, " \t", &tok);
                }
            }

        } else if (strcmp(argp, "--log") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                ejsStartLogging(mpr, argv[++nextArg]);
            }

        } else if (strcmp(argp, "--method") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                method = argv[++nextArg];
            }

        } else if (strcmp(argp, "--nobind") == 0) {
            bind = 0;

        } else if (strcmp(argp, "--nodebug") == 0) {
            debug = 0;

        } else if (strcmp(argp, "--optimize") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                optimizeLevel = atoi(argv[++nextArg]);
            }

        } else if (strcmp(argp, "-s") == 0) {
            /* Compatibility with mozilla shell. Just ignore */

        } else if (strcmp(argp, "--search") == 0 || strcmp(argp, "--searchpath") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (strcmp(argp, "--standard") == 0) {
            strict = 0;

        } else if (strcmp(argp, "--stats") == 0) {
            stats = 1;

        } else if (strcmp(argp, "--strict") == 0) {
            strict = 1;

        } else if (strcmp(argp, "--require") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                if (requiredModules == 0) {
                    requiredModules = mprCreateList(mpr);
                }
                modules = mprStrdup(mpr, argv[++nextArg]);
                name = mprStrTok(modules, " \t", &tok);
                while (name != NULL) {
                    require(requiredModules, name);
                    name = mprStrTok(NULL, " \t", &tok);
                }
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            ejsStartLogging(mpr, "stdout:2");

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintfError(mpr, "%s %s-%s\n", BLD_NAME, BLD_VERSION, BLD_NUMBER);
            exit(0);

        } else if (strcmp(argp, "--warn") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                warnLevel = atoi(argv[++nextArg]);
            }

        } else {
            err++;
            break;
        }
    }

    if (err) {
        /*  
            If --method or --class is specified, then the named class.method will be run (method defaults to "main", class
            defaults to first class with a "main").

            Examples:
                ejs
                ejs script.es arg1 arg2 arg3
                ejs --class "Customer" --method "start" --files "script1.es script2.es" main.es
         */
        mprPrintfError(mpr,
            "Usage: %s [options] script.es [arguments] ...\n"
            "  Ejscript shell program options:\n"
            "  --class className        # Name of class containing method to run\n"
            "  --cmd ejscriptCode       # Literal ejscript statements to execute\n"
            "  --debug                  # Use symbolic debugging information (default)\n"
            "  --files \"files..\"        # Extra source to compile\n"
            "  --log logSpec            # Internal compiler diagnostics logging\n"
            "  --method methodName      # Name of method to run. Defaults to main\n"
            "  --nodebug                # Omit symbolic debugging information\n"
            "  --optimize level         # Set the optimization level (0-9 default is 9)\n"
            "  --search ejsPath         # Module search path\n"
            "  --standard               # Default compilation mode to standard (default)\n"
            "  --stats                  # Print stats on exit\n"
            "  --strict                 # Default compilation mode to strict\n"
            "  --require 'module,...'   # Required list of modules to pre-load\n"
            "  --verbose | -v           # Same as --log stdout:2 \n"
            "  --version                # Emit the compiler version information\n"
            "  --warn level             # Set the warning message level (0-9 default is 0)\n\n",
            mpr->name);
        return -1;
    }
    mprSetDebugMode(mpr, debug);

    ejsService = ejsCreateService(mpr);
    if (ejsService == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    ejsInitCompiler(ejsService);
    ejs = ejsCreateVm(ejsService, NULL, searchPath, requiredModules, 0);
    if (ejs == 0) {
        return MPR_ERR_NO_MEMORY;
    }

    ecFlags = 0;
    ecFlags |= (merge) ? EC_FLAGS_MERGE: 0;
    ecFlags |= (bind) ? EC_FLAGS_BIND: 0;
    ecFlags |= (noout) ? EC_FLAGS_NO_OUT: 0;
    ecFlags |= (debug) ? EC_FLAGS_DEBUG: 0;

    cp = ecCreateCompiler(ejs, ecFlags);
    if (cp == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    //  TODO - should be an arg to create compiler
    cp->require = requiredModules;

    ecSetOptimizeLevel(cp, optimizeLevel);
    ecSetWarnLevel(cp, warnLevel);
    ecSetStrictMode(cp, strict);

    if (nextArg < argc) {
        mprAddItem(files, argv[nextArg]);
    }
    if (cmd) {
        if (interpretCommands(cp, cmd) < 0) {
            err++;
        }
    } else if (mprGetListCount(files) > 0) {
        if (interpretFiles(cp, files, argc - nextArg, &argv[nextArg], className, method) < 0) {
            err++;
        }
    } else {
        /*  
            No args - run as an interactive shell
         */
        if (interpretCommands(cp, NULL) < 0) {
            err++;
        }
    }

#if BLD_DEBUG
    if (stats) {
        mprSetLogLevel(ejs, 1);
        ejsPrintAllocReport(ejs);
    }
#endif
    mprFree(ejs);
    if (mprStop(mpr)) {
        mprFree(mpr);
    }
    return err;
}


/*  
    Compile the source files supplied on the command line. This will compile in-memory and optionally also save to 
    module files.
 */
static int interpretFiles(EcCompiler *cp, MprList *files, int argc, char **argv, cchar *className, cchar *method)
{
    Ejs     *ejs;

    mprAssert(files);

    ejs = cp->ejs;
    ejs->argc = argc;
    ejs->argv = argv;

    if (ecCompile(cp, files->length, (char**) files->items) < 0) {
        mprError(cp, "%s", cp->errorMsg);
        return EJS_ERR;
    }
    if (cp->errorCount == 0) {
        if (ejsRunProgram(ejs, className, method) < 0) {
            ejsReportError(ejs, "Error in program");
            return EJS_ERR;
        }
    }
    return 0;
}


/*  
    Interpret from the console or from a literal command
 */
static int interpretCommands(EcCompiler *cp, cchar *cmd)
{
    Ejs         *ejs;
    EjsString   *result;
    char        *tmpArgv[1];
    int         err;

    ejs = cp->ejs;

    if (ecOpenConsoleStream(cp->lexer, (cmd) ? commandGets: consoleGets) < 0) {
        mprError(cp, "Can't open input");
        return EJS_ERR;
    }
    if (cmd) {
        cp->lexer->input->stream->buf = mprStrdup(cp, cmd);
    } else {
        cp->interactive = 1;
    }
    cp->input = cp->lexer->input;
    cp->token = cp->lexer->input->token;

    ecResetInput(cp);
    tmpArgv[0] = EC_INPUT_STREAM;

    while (!cp->lexer->input->stream->eof) {
        err = 0;
        cp->uid = 0;
        if (ecCompile(cp, 1, tmpArgv) < 0) {
            mprError(cp, "%s", cp->errorMsg);
            ejs->result = ejs->undefinedValue;
            err++;
        }
        if (!err && cp->errorCount == 0) {
            if (ejsRunProgram(ejs, NULL, NULL) < 0) {
                ejsReportError(ejs, "Error in script");
            }
        }
        if (!ejs->exception && ejs->result != ejs->undefinedValue) {
            if (ejsIsDate(ejs->result) || ejsIsType(ejs->result)) {
                if ((result = (EjsString*) ejsToString(ejs, ejs->result)) != 0) {
                    mprPrintf(cp, "%s\n", result->value);
                }
            } else if (ejs->result != ejs->nullValue) {
                if ((result = (EjsString*) ejsToJSON(ejs, ejs->result, NULL)) != 0) {
                    mprPrintf(cp, "%s\n", result->value);
                }
            }
        }
        ecResetInput(cp);
        cp->errorCount = 0;
        cp->fatalError = 0;
        err = 0;
        mprServiceEvents(ejs, ejs->dispatcher, 0, 0);
    }
    ecCloseStream(cp->lexer);
    return 0;
}


#if BLD_CC_EDITLINE
static History  *cmdHistory;
static EditLine *eh; 
static cchar    *prompt;

static cchar *issuePrompt(EditLine *e) {
    return prompt;
}

static EditLine *initEditLine()
{
    EditLine    *e;
    HistEvent   ev; 

    cmdHistory = history_init(); 
    history(cmdHistory, &ev, H_SETSIZE, 100); 
    e = el_init("ejs", stdin, stdout, stderr); 
    el_set(e, EL_EDITOR, "vi");
    el_set(e, EL_HIST, history, cmdHistory);
    el_source(e, NULL);
    return e;
}


/*  
    Prompt for input with the level of current nest (block nest depth)
 */
static char *readline(cchar *msg) 
{ 
    HistEvent   ev; 
    cchar       *str; 
    char        *result;
    int         len, count; 
 
    if (eh == NULL) { 
        eh = initEditLine();
    }
    prompt = msg;
    el_set(eh, EL_PROMPT, issuePrompt);
    str = el_gets(eh, &count); 
    if (str && count > 0) { 
        result = strdup(str); 
        len = strlen(result);
        if (result[len - 1] == '\n') {
            result[len - 1] = '\0'; 
        }
        count = history(cmdHistory, &ev, H_ENTER, result); 
        return result; 
    }  
    return NULL; 
} 

#else

static char *readline(cchar *msg)
{
    char    buf[MPR_MAX_STRING];

    printf("%s", msg);
    if (fgets(buf, sizeof(buf) - 1, stdin) == 0) {
        return NULL;
    }
    return strdup(buf);
}
#endif


/*  
    Read input from the console (stdin)
 */
static int consoleGets(EcStream *stream)
{
    char    prompt[MPR_MAX_STRING], *line, *cp;
    int     len;

    if (stream->flags & EC_STREAM_EOL) {
        return 0;
    }
    mprSprintf(stream, prompt, sizeof(prompt), "%s-%d> ", EJS_NAME, stream->compiler->state->blockNestCount);

    line = readline(prompt);
    if (line == NULL) {
        stream->eof = 1;
        mprPrintf(stream, "\n");
        return -1;
    }
    cp = mprStrTrim(line, "\r\n");
    len = strlen(cp);
    stream->buf = mprStrdup(stream, cp);
    stream->nextChar = stream->buf;
    stream->end = &stream->buf[len];
    stream->currentLine = stream->buf;
    stream->lineNumber = 1;
    stream->flags |= EC_STREAM_EOL;
    return len;
}


/*  
    Read input from a literal command
 */
static int commandGets(EcStream *stream)
{
    /*  We only get to execute one string of commands. So we only come here once. Second time round, nextChar will be set.
     */
    if (stream->nextChar) {
        stream->eof = 1;
        return -1;
    }
    stream->nextChar = stream->buf;
    stream->end = &stream->buf[strlen(stream->buf)];
    stream->currentLine = stream->buf;
    stream->lineNumber = 1;
    return (int) strlen(stream->buf);
}


static void require(MprList *list, cchar *name) 
{
    mprAssert(list);

    if (name && *name) {
        mprAddItem(list, name);
    }
}


static void setupUnixSignals(Mpr *mpr)
{
#if BLD_UNIX_LIKE
    signal(SIGPIPE, SIG_IGN);
#if LINUX
    /*
        Ignore signals from write requests to large files
     */
    signal(SIGXFSZ, SIG_IGN);
#endif
#endif
}


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
 *  End of file "../src/cmd/ejs.c"
 */
/************************************************************************/

