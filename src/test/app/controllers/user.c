/*
    User mvc controller
 */
#include "esp.h"

static void common() {
    setParam("title", "MVC Title");
}

static void create() { 
    setRec(createRec("user", params()));
    if (save()) {
        inform("New user created");
        redirect("@");
    } else {
        writeView("edit");
    }
}

static void destroy() { 
    cchar   *id;

    if ((id = param("id", 0)) != 0) {
        inform("User %s removed", id);
        remove(id);
    }
    redirect("@");
}

static void edit() { 
    setRec(findRec("user", param("id", 0)));
}

static void init() { 
    setRec(createRec("user", NULL));
    writeView("edit");
}

static void login() { 
    if (session("id") != 0) {
        render("Logged in");

        //  MOB - need better routine for this
    } else if (smatch(param("username", ""), "admin") && smatch(param("password", ""), "secret")) {
        setSession("id", "admin");
        inform("Welcome back");
        if (session("referrer")) {
            redirect(session("referrer"));
        } else {
            redirect("@show");
        }
        setSession("id", param("username", ""));

    //  MOB - alternatively, set conn->method
    } else if (scmp(getMethod(), "POST") == 0) {
        render("Invalid login, please retry.");

    } else {
        createSession();
    }
    finalize();
}

static void update() { 
    setRec(findRec("user", param("id", 0)));
    if (save()) {
        inform("User updated successfully.");
        redirect("@show");
    } else {
        writeView("edit");
    }
}

ESP_EXPORT int espInit_controller_user(EspRoute *eroute, MprModule *module) {
    espDefineBase(eroute, common);
    espDefineAction(eroute, "user-create", create);
    espDefineAction(eroute, "user-destroy", destroy);
    espDefineAction(eroute, "user-edit", edit);
    espDefineAction(eroute, "user-init", init);
    espDefineAction(eroute, "user-update", update);
    espDefineAction(eroute, "user-login", login);
    return 0;
}
