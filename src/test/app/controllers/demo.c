/*
    Demo controller
 */
#include "esp.h"

static void list() {
    render("DONE\n");
}

static void start() {
    maStartAppweb(MPR->appwebService);
}

static void stop() {
    maStopAppweb(MPR->appwebService);
    mprCreateEvent(NULL, "restart", 5000, start, 0, 0);
}

ESP_EXPORT int esp_controller_demo(EspRoute *eroute, MprModule *module) {
    cchar   *schema;

    schema = "{ \
        post: { \
            schema: { \
                id: { type: 'string', autoinc: 'true', key: 'true' }, \
                title: { type: 'string' }, \
                body: { type: 'text' }, \
            }, data: [ \
                [ '1', 'Message one', 'Line one' ], \
                [ '2', 'Message one', 'Line one' ], \
            ], \
        }, \
    ";
    eroute->edi = ediOpen("mdb", schema, EDI_LITERAL);

    EdiGrid *grid = makeGrid("[ \
        { id: '1', country: 'Australia' }, \
        { id: '2', country: 'China' }, \
    ]");

    EdiRec *rec = makeRec("{ id: 1, title: 'Message One', body: 'Line one' }");

    espDefineAction(eroute, "demo-list", list);
    espDefineAction(eroute, "demo-cmd-stop", stop);
    espDefineAction(eroute, "demo-cmd-start", start);
    return 0;
}
