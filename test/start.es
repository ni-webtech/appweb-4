/*
    NOT USED
    Optional startup script for Ejscript requests. 
    This is referenced by appweb.conf/EjsStartup.
    If missing, Ejscript will use an internal, equivalent script.
 */

require ejs.web
let server: HttpServer = new HttpServer

var router = Router(Router.Top)
router.show()
server.on("readable", function (event, request) {
    server.serve(request, router)
})
server.listen()
