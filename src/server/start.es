//  NOT USED
require ejs.web
let server: HttpServer = new HttpServer
var router = Router(Router.Top)
server.observe("readable", function (event, request) {
    serve.serve(request, router)
})
server.listen()
