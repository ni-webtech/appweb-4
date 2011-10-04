/*
    Post controller
 */
#include "esp.h"

static void common() {
}

static void create() { 
    if (writeRec(createRec("post", params()))) {
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
    //  MOB - is this the best view?
    renderView("post-edit");
}

static void update() { 
    if (writeFields("post", params())) {
        inform("Post updated successfully.");
        redirect("@");
    } else {
        renderView("post-edit");
    }
}

ESP_EXPORT int esp_controller_post(EspRoute *eroute, MprModule *module) 
{
    Edi *edi;

    espDefineBase(eroute, common);
    espDefineAction(eroute, "post-create", create);
    espDefineAction(eroute, "post-destroy", destroy);
    espDefineAction(eroute, "post-edit", edit);
    espDefineAction(eroute, "post-init", init);
    espDefineAction(eroute, "post-list", list);
    espDefineAction(eroute, "post-show", show);
    espDefineAction(eroute, "post-update", update);

    /*
        Add model validations
     */
    edi = getDatabase();
    ediAddValidation(edi, "present", "post", "title", 0);
    ediAddValidation(edi, "unique", "post", "title", 0);
    ediAddValidation(edi, "format", "post", "body", "(fox|dog)");
    return 0;
}
