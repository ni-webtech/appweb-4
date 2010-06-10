/*
    simpleModule.c - Create a simple AppWeb dynamically loadable module
  
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
 
/******************************* Includes *****************************/

#include    "appweb.h"

/********************************* Code *******************************/
/* 
    Parse any module specific configuration directives from the appweb.conf config file.
 */

static int parseSimple(MaHttp *http, cchar *key, char *value, MaConfigState *state)
{
    if (mprStrcmpAnyCase(key, "CustomConfigKey") == 0) {
        /*
            Do something with value. Return true to indicate we understood and accepted the directive.
         */
        return 1;
    }
    return 0;
}


/*
    Module load initialization. This is called when the module is first loaded. The module name is "Simple".
 */
MprModule *mprSimpleModuleInit(MaHttp *http)
{
    MprModule   *module;
    MaStage     *stage;

    module = mprCreateModule(http, "simpleModule", BLD_VERSION, NULL, NULL, NULL);
    if (module == 0) {
        return 0;
    }
    /*
        Create a stage so we can process configuration file data
     */
    if ((stage = maCreateStage(http, "simpleModule", MA_STAGE_MODULE)) == 0) {
        mprFree(module);
        return 0;
    }
    stage->parse = parseSimple; 
    return module;
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
