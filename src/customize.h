/*
    customize.h -- Place to override standard product defintions
 */

#ifndef _h_CUSTOMIZE_h
#define _h_CUSTOMIZE_h 1

/*
    Override definitions here. This file is included after all other headers are parsed.

    Use this to define the default appweb.conf file
        #define BLD_CONFIG_FILE     "/path/to/default/appweb.conf"

    Use this to define the default server root directory
        #define BLD_SERVER_ROOT     "/var/spool/chroot/jail"
 */

/*
    If you require function name remapping in ESP, do so here. Change the "xx_" to any 
    unique prefix you require. Then use that name in ESP pages and controllers.

    #define script xx_script
 */
#endif /* _h_CUSTOMIZE_h */
