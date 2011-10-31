/*
    Test caching
 */
#include "esp.h"

static void manual() { 
    if (smatch(getQuery(), "send")) {
        setHeader("X-SendCache", "true");
        render("dummy");
    } else if (!espRenderCached(getConn())) {
        render("{ when: %Ld, uri: '%s', query: '%s' }\r\n", mprGetTicks(), getUri(), getQuery());
    }
}

static void update() { 
    cchar   *data;
    data = sfmt("{ when: %Ld, uri: '%s', query: '%s' }\r\n", mprGetTicks(), getUri(), getQuery());
    espUpdateCache(getConn(), "/cache/manual", data, 86400);
    render("done");
}

static void clear() { 
    espUpdateCache(getConn(), "/cache/manual", 0, 0);
    render("done");
}

ESP_EXPORT int esp_controller_cache(EspRoute *eroute, MprModule *module) {
    espDefineAction(eroute, "cache-cmd-clear", clear);
    espDefineAction(eroute, "cache-cmd-manual", manual);
    espDefineAction(eroute, "cache-cmd-update", update);

#if MOB 
    - check
    HttpRoute *route = httpLookupRoute(eroute->route->host, "/app/*/default");
    espCache(eroute->route->host, "/test/transparent", 0, 0);
#endif
    return 0;
}
