/*
    Test mvc controller
 */
#include "esp.h"


static void common(HttpConn *conn) {
    espSetVar(conn, "title", "MVC Title");
}


static void check(HttpConn *conn) { 
    espWrite(conn, "Check: OK\r\n");
    espFinalize(conn);
    /* No view used */
}


static void details(HttpConn *conn) { 
    common(conn);
    espSetVar(conn, "title", "MVC Title");
    espSetIntVar(conn, "secret", 42);
    /* View will be rendered */
}

static void cached(HttpConn *conn) { 
    common(conn);
    espWrite(conn, "When: %s\r\n", mprGetDate(0));
    espWrite(conn, "URI: %s\r\n", conn->rx->uri);
    espWrite(conn, "QUERY: %s\r\n", conn->rx->parsedUri->query);
    espFinalize(conn);
}

static void login(HttpConn *conn) { 
    common(conn);
    if (espGetSessionVar(conn, "id", 0) != 0) {
        espWrite(conn, "Logged in");
        espFinalize(conn);

    } else if (espMatchVar(conn, "username", "admin") && espMatchVar(conn, "password", "secret")) {
        espWrite(conn, "Valid Login");
        espFinalize(conn);
        espSetSessionVar(conn, "id", "admin", 0);

    } else if (scmp(conn->rx->method, "POST") == 0) {
        espWrite(conn, "Invalid login, please retry.");
        espFinalize(conn);

    } else {
        espGetSession(conn, 1);
    }
}

static void missing(HttpConn *conn) {
    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Missing action");
}

ESP_EXPORT int espInit_controller_user(EspRoute *eroute, MprModule *module) {
    espDefineAction(eroute, "missing", check);
    espDefineAction(eroute, "user-check", check);
    espDefineAction(eroute, "user-cached", cached);
    espDefineAction(eroute, "user-details", details);
    espDefineAction(eroute, "user-login", login);

    espCacheControl(eroute, "user-cached", 0, "*");
    return 0;
}
