/*
    Test mvc controller
 */
#include "esp.h"

static void check(HttpConn *conn) { 
    espWrite(conn, "Check: OK\r\n");
    espFinalize(conn);
    /* No view used */
}

static void details(HttpConn *conn) { 
    espSetVar(conn, "title", "MVC Title");
    espSetIntVar(conn, "secret", 42);
    /* View will be rendered */
}

static void login(HttpConn *conn) { 
#if 0
    char    *username;

    username = espGetVar(conn, "username", 0);
    if (!username) {
        //  MOB - need ApI for referrer
        espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, conn->rx->referrer); 
        return;
    }
    //  MOB - espMatchVar 
    if (espCompareVar(conn, "username", "admin") && espCompareVar(conn, "password", "secret")) {
        //  inform("Welcome Back")
        if (session("referrer")) {
            espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, session("referrer")); 
        } else {
            // espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, "@Dash/show");
            espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, "/");
        }
        espSetSession("id", username);
    } else if (scmp(conn->rx->method, "POST") == 0) {
        //  MOB - was error
        error("Invalid login")
    }
#endif
}

int espController_user_c(EspLoc *esp, MprModule *module) {
    espDefineAction(esp, "user-check", check);
    espDefineAction(esp, "user-details", details);
    espDefineAction(esp, "user-login", login);
    return 0;
}
