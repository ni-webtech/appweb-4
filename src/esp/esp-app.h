/*
    esp-app.h -- User header for ESP Applications and Pages

    This file may be located in the DocumentRoot for a Route. All ESP requests using that Route will
    source this header.
 */

#ifndef _h_ESP_APP
#define _h_ESP_APP 1

#include    "esp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    Put your definitions here

    If you need to remap an abbreviated API name that clashes with an API name of yours, you can rename the
    ESP APIs here. Change the "xx_" to any unique prefix you require. Then use that name in ESP pages and controllers.

    #define table xx_table
*/

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* _h_ESP_APP */
