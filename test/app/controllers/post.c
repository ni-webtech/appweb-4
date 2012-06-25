/*
    Post controller
 */
#include "esp.h"

static void common() {
}

static void create() { 
    if (updateRec(createRec("post", params()))) {
        inform("New post created");
        redirect("@");
    } else {
        renderView("post-edit");
    }
}

static void destroy() { 
    if (removeRec("post", param("id"))) {
        inform("Post removed");
    }
    redirect("@");
}

static void edit() { 
    readRec("post");
}

static void list() { }

static void init() { 
    createRec("post", 0);
    renderView("post-edit");
}

static void show() { 
    readRec("post");
    renderView("post-edit");
}

static void update() { 
    if (updateFields("post", params())) {
        inform("Post updated successfully.");
        redirect("@");
    } else {
        renderView("post-edit");
    }
}

ESP_EXPORT int esp_controller_post(HttpRoute *route, MprModule *module) 
{
    Edi     *edi;

    espDefineBase(route, common);
    espDefineAction(route, "post-create", create);
    espDefineAction(route, "post-destroy", destroy);
    espDefineAction(route, "post-edit", edit);
    espDefineAction(route, "post-init", init);
    espDefineAction(route, "post-list", list);
    espDefineAction(route, "post-show", show);
    espDefineAction(route, "post-update", update);

    /*
        Add model validations
     */
    edi = espGetRouteDatabase(route->eroute);
    ediAddValidation(edi, "present", "post", "title", 0);
    ediAddValidation(edi, "unique", "post", "title", 0);
    ediAddValidation(edi, "format", "post", "body", "(fox|dog)");
    return 0;
}
