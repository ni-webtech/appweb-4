/*
    customize.h -- Place to override standard product defintions
 */

#ifndef _h_CUSTOMIZE_h
#define _h_CUSTOMIZE_h 1

/*
    Override definitions here. This file is included after all other headers are parsed.

    Use this to override the default appweb.conf file
        #define BIT_CONFIG_FILE     "/path/to/default/appweb.conf"

    Use this to override the default server root directory
        #define BIT_SERVER_ROOT     "/var/spool/chroot/jail"

    Use this to override the path to the appweb executable
        #define BIT_APPWEB_PATH     "/path/to/appweb"
 */

/*
    If you require function name remapping in ESP, do so here. Change the "xx_" to any 
    unique prefix you require. Then use that name in ESP pages and controllers.

    #define script xx_script
 */
#endif /* _h_CUSTOMIZE_h */
