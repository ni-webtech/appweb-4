/*
    Test mvc controller
 */
#include "esp.h"

static void common() {
    setParam("page-title", "MVC Title");
}

static void check() { 
    render("Check: OK\r\n");
    finalize();
    /* No view used */
}

static void details() { 
    setParam("secret", "42");
    /* View will be rendered */
}

static void transparent() { 
    render("{ when: %Ld, uri: '%s', query: '%s'\r\n", mprGetTicks(), getUri(), getQuery());
}

static void manual() { 
    if (!espRenderCached(getConn())) {
        render("{ when: %Ld, uri: '%s', query: '%s'\r\n", mprGetTicks(), getUri(), getQuery());
    }
    // void espUpdateCache(getConn(), "Hello World");
}

static void login() { 
    if (getSessionVar("id")[0]) {
        render("Logged in");
        finalize();

    } else if (smatch(param("username"), "admin") && smatch(param("password"), "secret")) {
        render("Valid Login");
        finalize();
        setSessionVar("id", "admin");

    } else if (smatch(getMethod(), "POST")) {
        render("Invalid login, please retry.");
        finalize();

    } else {
        createSession();
    }
}

static void missing() {
    renderError(HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing action");
}

ESP_EXPORT int esp_controller_test(EspRoute *eroute, MprModule *module) {

    //  MOB - should actions be more like URIs:    /test/
    espDefineAction(eroute, "test-missing", missing);
    espDefineAction(eroute, "test-cmd-check", check);
    espDefineAction(eroute, "test-cmd-details", details);
    espDefineAction(eroute, "test-cmd-login", login);

    espDefineAction(eroute, "test-cmd-manual", manual);
    espDefineAction(eroute, "test-cmd-transparent", transparent);

#if 0
    HttpRoute *route = httpLookupRoute(eroute->route->host, "/app/*/default");
    espCache(eroute->route->host, "/test/transparent", 0, 0);
#endif
    return 0;
}
