/*
    Test caching
 */
#include "esp.h"

//  This is configured for caching by API below
static void api() {
    render("{ when: %Ld, uri: '%s', query: '%s' }\r\n", mprGetTicks(), getUri(), getQuery());
}

static void big() { 
    int     i;
    for (i = 0; i < 1000; i++) {
        render("Line: %05d %s", i, "aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>\r\n");
    }
}

static void clear() { 
    espUpdateCache(getConn(), "/cache/manual", 0, 0);
    espUpdateCache(getConn(), "/cache/big", 0, 0);
    espUpdateCache(getConn(), "/cache/api", 0, 0);
    render("done");
}

static void client() { 
    render("client");
}

static void manual() { 
    if (smatch(getQuery(), "send")) {
        setHeader("X-SendCache", "true");
        finalize();
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

ESP_EXPORT int esp_controller_cache(EspRoute *eroute, MprModule *module) {
    espDefineAction(eroute, "cache-cmd-api", api);
    espDefineAction(eroute, "cache-cmd-big", big);
    espDefineAction(eroute, "cache-cmd-clear", clear);
    espDefineAction(eroute, "cache-cmd-client", client);
    espDefineAction(eroute, "cache-cmd-manual", manual);
    espDefineAction(eroute, "cache-cmd-update", update);

    //  This will cache the next request after the first one that triggered the loading of this controller
    HttpRoute *route = httpLookupRoute(eroute->route->host, "/app/*/default");
    espCache(route, "/cache/api", 0, 0);
    return 0;
}
