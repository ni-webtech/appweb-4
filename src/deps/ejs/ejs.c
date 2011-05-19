#include "ejs.h"

/******************************************************************************/
/* 
    This file is an amalgamation of all the individual source code files for the
    Embedthis Ejscript Shell Source.
  
    Catenating all the source into a single file makes embedding simpler and
    the resulting application faster, as many compilers can do whole file
    optimization.
  
    If you want to modify the product, you can still get the whole source as 
    individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "../../src/cmd/ejs.c"
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


typedef struct App {
    MprList     *files;
    MprList     *modules;
    Ejs         *ejs;
    EcCompiler  *compiler;
} App;

static App *app;

#if BLD_CC_EDITLINE
static History  *cmdHistory;
static EditLine *eh; 
static cchar    *prompt;
#endif

static int  consoleGets(EcStream *stream);
static int  commandGets(EcStream *stream);
static int  interpretCommands(EcCompiler*cp, cchar *cmd);
static int  interpretFiles(EcCompiler *cp, MprList *files, int argc, char **argv, cchar *className, cchar *method);
static void manageApp(App *app, int flags);
static void require(cchar *name);


MAIN(ejsMain, int argc, char **argv)
{
    Mpr             *mpr;
    EcCompiler      *cp;
    Ejs             *ejs;
    cchar           *cmd, *className, *method, *homeDir;
    char            *argp, *searchPath, *modules, *name, *tok, *extraFiles;
    int             nextArg, err, ecFlags, stats, merge, bind, noout, debug, optimizeLevel, warnLevel, strict;

    /*  
        Initialize Multithreaded Portable Runtime (MPR)
     */
    mpr = mprCreate(argc, argv, 0);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    mprAddStandardSignals();

    if (mprStart(mpr) < 0) {
        mprError("Can't start mpr services");
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
    strict = 0;
    app->files = mprCreateList(-1, 0);

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

        } else if (strcmp(argp, "--chdir") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = argv[++nextArg];
                if (chdir((char*) homeDir) < 0) {
                    mprError("Can't change directory to %s", homeDir);
                }
            }

#if BLD_UNIX_LIKE
        } else if (strcmp(argp, "--chroot") == 0) {
            /* Not documented or supported */
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = mprGetAbsPath(argv[++nextArg]);
                if (chroot(homeDir) < 0) {
                    if (errno == EPERM) {
                        mprPrintfError("%s: Must be super user to use the --chroot option", mprGetAppName(mpr));
                    } else {
                        mprPrintfError("%s: Can't change change root directory to %s, errno %d",
                            mprGetAppName(), homeDir, errno);
                    }
                    return 4;
                }
            }
#endif

        } else if (strcmp(argp, "--cmd") == 0 || strcmp(argp, "-c") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                cmd = argv[++nextArg];
            }

        } else if (strcmp(argp, "--debug") == 0) {
            debug = 1;

        } else if (strcmp(argp, "--debugger") == 0 || strcmp(argp, "-D") == 0) {
            mprSetDebugMode(1);

        } else if (strcmp(argp, "--files") == 0 || strcmp(argp, "-f") == 0) {
            /* Compatibility with mozilla shell */
            if (nextArg >= argc) {
                err++;
            } else {
                extraFiles = sclone(argv[++nextArg]);
                name = stok(extraFiles, " \t", &tok);
                while (name != NULL) {
                    mprAddItem(app->files, sclone(name));
                    name = stok(NULL, " \t", &tok);
                }
            }

        } else if (strcmp(argp, "--log") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                ejsRedirectLogging(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--method") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                method = argv[++nextArg];
            }

        } else if (strcmp(argp, "--name") == 0) {
            /* Just ignore. Used to tag commands with a unique command line */ 
            nextArg++;

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
                if (app->modules == 0) {
                    app->modules = mprCreateList(-1, 0);
                }
                modules = sclone(argv[++nextArg]);
                name = stok(modules, " \t", &tok);
                while (name != NULL) {
                    require(name);
                    name = stok(NULL, " \t", &tok);
                }
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            ejsRedirectLogging("stdout:2");

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintfError("%s %s-%s\n", BLD_NAME, BLD_VERSION, BLD_NUMBER);
            return 0;

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
        mprPrintfError("Usage: %s [options] script.es [arguments] ...\n"
            "  Ejscript shell program options:\n"
            "  --class className        # Name of class containing method to run\n"
            "  --cmd ejscriptCode       # Literal ejscript statements to execute\n"
            "  --debug                  # Use symbolic debugging information (default)\n"
            "  --debugger               # Disable timeouts to make using a debugger easier\n"
            "  --files \"files..\"        # Extra source to compile\n"
            "  --log logSpec            # Internal compiler diagnostics logging\n"
            "  --method methodName      # Name of method to run. Defaults to main\n"
            "  --nodebug                # Omit symbolic debugging information\n"
            "  --optimize level         # Set the optimization level (0-9 default is 9)\n"
            "  --search ejsPath         # Module search path\n"
            "  --standard               # Default compilation mode to standard (default)\n"
            "  --stats                  # Print memory stats on exit\n"
            "  --strict                 # Default compilation mode to strict\n"
            "  --require 'module,...'   # Required list of modules to pre-load\n"
            "  --verbose | -v           # Same as --log stdout:2 \n"
            "  --version                # Emit the compiler version information\n"
            "  --warn level             # Set the warning message level (0-9 default is 0)\n\n",
            mpr->name);
        return -1;
    }
    if ((ejs = ejsCreateVM(0, 0, searchPath, app->modules, argc - nextArg, (cchar **) &argv[nextArg], 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    app->ejs = ejs;
    ecFlags = 0;
    ecFlags |= (merge) ? EC_FLAGS_MERGE: 0;
    ecFlags |= (bind) ? EC_FLAGS_BIND: 0;
    ecFlags |= (noout) ? EC_FLAGS_NO_OUT: 0;
    ecFlags |= (debug) ? EC_FLAGS_DEBUG: 0;

    cp = app->compiler = ecCreateCompiler(ejs, ecFlags);
    if (cp == 0) {
        return MPR_ERR_MEMORY;
    }
    ecSetRequire(cp, app->modules);

    ecSetOptimizeLevel(cp, optimizeLevel);
    ecSetWarnLevel(cp, warnLevel);
    ecSetStrictMode(cp, strict);
    if (nextArg < argc) {
        mprAddItem(app->files, sclone(argv[nextArg]));
    }
    if (cmd) {
        if (interpretCommands(cp, cmd) < 0) {
            err++;
        }
    } else if (mprGetListLength(app->files) > 0) {
        if (interpretFiles(cp, app->files, argc - nextArg, &argv[nextArg], className, method) < 0) {
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
        mprSetLogLevel(1);
        mprPrintMem("Memory Usage", 0);
    }
#endif
    app->ejs = 0;
    mprTerminate(MPR_EXIT_DEFAULT);
    ejsDestroyVM(ejs);
    mprDestroy(MPR_EXIT_DEFAULT);
    return err;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->files);
        mprMark(app->ejs);
        mprMark(app->compiler);
        mprMark(app->modules);
    }
}


/*  
    Compile the source files supplied on the command line. This will compile in-memory and optionally also save to 
    module files.
 */
static int interpretFiles(EcCompiler *cp, MprList *files, int argc, char **argv, cchar *className, cchar *method)
{
    Ejs     *ejs;

    mprAssert(files);

    MPR_VERIFY_MEM();
    ejs = cp->ejs;
    if (ecCompile(cp, files->length, (char**) files->items) < 0) {
        mprRawLog(0, "%s\n", cp->errorMsg);
        return EJS_ERR;
    }
    mprAssert(ejs->result == 0 || (MPR_GET_GEN(MPR_GET_MEM(ejs->result)) != MPR->heap.dead));

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
    cp->interactive = 1;

    if (ecOpenConsoleStream(cp, (cmd) ? commandGets: consoleGets, cmd) < 0) {
        mprError("Can't open input");
        return EJS_ERR;
    }
    // MOB ecResetInput(cp);
    tmpArgv[0] = EC_INPUT_STREAM;

    while (!cp->stream->eof && !mprIsStopping()) {
        err = 0;
        cp->uid = 0;
        if (ecCompile(cp, 1, tmpArgv) < 0) {
            mprRawLog(0, "%s", cp->errorMsg);
            ejs->result = S(undefined);
            err++;
        }
        if (!err && cp->errorCount == 0) {
            if (ejsRunProgram(ejs, NULL, NULL) < 0) {
                ejsReportError(ejs, "Error in script");
            }
        }
        if (!ejs->exception && ejs->result != S(undefined)) {
            if (ejsIs(ejs, ejs->result, Date) || ejsIsType(ejs, ejs->result)) {
                if ((result = (EjsString*) ejsToString(ejs, ejs->result)) != 0) {
                    mprPrintf("%@\n", result);
                }
            } else if (ejs->result != S(null)) {
                if ((result = (EjsString*) ejsToJSON(ejs, ejs->result, NULL)) != 0) {
                    mprPrintf("%@\n", result);
                }
            }
        }
        ecResetInput(cp);
        cp->errorCount = 0;
        cp->fatalError = 0;
    }
    ecCloseStream(cp);
    return 0;
}


#if BLD_CC_EDITLINE

static cchar *issuePrompt(EditLine *e) 
{
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
    ssize       len;
    int         count; 
 
    if (eh == NULL) { 
        eh = initEditLine();
    }
    prompt = msg;
    el_set(eh, EL_PROMPT, issuePrompt);
    el_set(eh, EL_SIGNAL, 1);
    str = el_gets(eh, &count); 
    if (str && count > 0) { 
        result = strdup(str); 
        len = slen(result);
        if (result[len - 1] == '\n') {
            result[len - 1] = '\0'; 
        }
        count = history(cmdHistory, &ev, H_ENTER, result); 
        return result; 
    }  
    return NULL; 
} 

#else /* BLD_CC_EDITLINE */

static char *readline(cchar *msg)
{
    char    buf[MPR_MAX_STRING];

    printf("%s", msg);
    if (fgets(buf, sizeof(buf) - 1, stdin) == 0) {
        return NULL;
    }
    return strdup(buf);
}
#endif /* BLD_CC_EDITLINE */


/*  
    Read input from the console (stdin)
 */
static int consoleGets(EcStream *stream)
{
    char    prompt[MPR_MAX_STRING], *line, *cp;
    int     level;

    if (stream->flags & EC_STREAM_EOL) {
        return 0;
    }
    level = stream->compiler->state ? stream->compiler->state->blockNestCount : 0;
    mprSprintf(prompt, sizeof(prompt), "%s-%d> ", EJS_NAME, level);

    mprYield(MPR_YIELD_STICKY);
    line = readline(prompt);
    mprResetYield();
    if (line == NULL) {
        stream->eof = 1;
        mprPrintf("\n");
        return -1;
    }
    cp = strim(line, "\r\n", MPR_TRIM_BOTH);
    ecSetStreamBuf(stream, cp, slen(cp));
    stream->flags |= EC_STREAM_EOL;
    return (int) slen(cp);
}


/*  
    Read input from a literal command
 */
static int commandGets(EcStream *stream)
{
    /*  
        Execute one string of commands. So we only come here once. Second time round, nextChar will be set.
        MOB - this logic is not right
     */
    if (stream->nextChar) {
        stream->eof = 1;
        return -1;
    }
    mprAssert(0);
    //  MOB -- because of the test above, nextChar must be zero
    return (int) (stream->end - stream->nextChar);
}


static void require(cchar *name) 
{
    if (name && *name) {
        mprAddItem(app->modules, sclone(name));
    }
}

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
 *  End of file "../../src/cmd/ejs.c"
 */
/************************************************************************/

