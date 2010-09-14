require ejs.web

App.log.info("Starting server.es")
let server: HttpServer = new HttpServer('.', "./web")
var router = Router(Router.Default)

server.on("readable", function (event, request) {
    Web.serve(request, router)
})
server.listen()
