/*
    Test caching
 */
#include "esp.h"

static void manual() { 
    if (!espRenderCached(getConn())) {
        render("{ when: %Ld, uri: '%s', query: '%s' }\r\n", mprGetTicks(), getUri(), getQuery());
    }
}

static void update() { 
    cchar   *data;
    data = sfmt("{ when: %Ld, uri: '%s', query: '%s' }\r\n", mprGetTicks(), getUri(), getQuery());
    espUpdateCache(getConn(), "/cache/manual", data, 86400);
    render("done");
}

ESP_EXPORT int esp_controller_cache(EspRoute *eroute, MprModule *module) {

    espDefineAction(eroute, "cache-cmd-manual", manual);
    espDefineAction(eroute, "cache-cmd-update", update);

#if 0
    HttpRoute *route = httpLookupRoute(eroute->route->host, "/app/*/default");
    espCache(eroute->route->host, "/test/transparent", 0, 0);
#endif
    return 0;
}
