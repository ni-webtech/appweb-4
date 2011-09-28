/*
    Blogging Post controller
 */
#include "esp.h"

static void common() {
}

//  MOB - Create should only work for POST
static void create() { 
    record(createRec("post", params()));
    if (save()) {
        inform("New post created");
        redirect("@");
    } else {
        writeView("post-edit");
    }
}

static void destroy() { 
    cchar   *id;

    if ((id = param("id", 0)) != 0) {
        removeRec("post", id);
        inform("Post %s removed", id);
    }
    redirect("@");
}

static void edit() { 
    record(findRec("post", param("id", 0)));
}

static void init() { 
    record(createRec("post", 0));
    writeView("post-edit");
}

static void show() { 
    record(createRec("post", 0));
    //  MOB - need to write something here
}

static void update() { 
    //  MOB - what happens if id is not found?
    record(findRec("post", param("id", 0)));
    if (save()) {
        inform("Post updated successfully.");
        redirect("@");
    } else {
        writeView("post-edit");
    }
}

ESP_EXPORT int espInit_controller_post(EspRoute *eroute, MprModule *module) {
    espDefineBase(eroute, common);
    espDefineAction(eroute, "post-create", create);
    espDefineAction(eroute, "post-destroy", destroy);
    espDefineAction(eroute, "post-edit", edit);
    espDefineAction(eroute, "post-init", init);
    espDefineAction(eroute, "post-update", update);
    return 0;
}
