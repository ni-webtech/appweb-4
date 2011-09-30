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

static void cached() { 
    render("When: %s\r\n", mprGetDate(0));
    render("URI: %s\r\n", getUri());
    render("QUERY: %s\r\n", getQuery());
    finalize();
}

static void login() { 
    if (session("id")[0]) {
        render("Logged in");
        finalize();

    } else if (smatch(param("username"), "admin") && smatch(param("password"), "secret")) {
        render("Valid Login");
        finalize();
        setSession("id", "admin");

    } else if (smatch(getMethod(), "POST")) {
        render("Invalid login, please retry.");
        finalize();

    } else {
        createSession();
    }
}

static void missing() {
    httpError(getConn(), HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing action");
}

ESP_EXPORT int espInit_controller_test(EspRoute *eroute, MprModule *module) {
    espDefineAction(eroute, "missing", missing);
    espDefineAction(eroute, "test-cmd-check", check);
    espDefineAction(eroute, "test-cmd-cached", cached);
    espDefineAction(eroute, "test-cmd-details", details);
    espDefineAction(eroute, "test-cmd-login", login);

    //  MOB - test this
    espCacheControl(eroute, "test-cached", 0, "*");
    return 0;
}
