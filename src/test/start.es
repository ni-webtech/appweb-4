//  NOT - used
require ejs.web
let server: HttpServer = new HttpServer
var router = Router(Router.Top)
server.on("readable", function (event, request) {
    server.serve(request, router)
})
server.listen()
