/*
    Test mvc controller
 */
#include "esp.h"

static void check(HttpConn *conn) { 
    espWrite(conn, "Check: OK\r\n");
    espFinalize(conn);
    /* No view used */
}

static void login(HttpConn *conn) { 
    espSetVar(conn, "title", "MVC Title");
    espSetIntVar(conn, "secret", 42);
}

int espController_user_c(EspLoc *esp, MprModule *module) {
    espDefineAction(esp, "user-check", check);
    espDefineAction(esp, "user-login", login);
    return 0;
}
