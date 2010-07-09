require ejs.web

App.log.info("Starting server.es")
let server: HttpServer = new HttpServer('.', "./web")
var router = Router(Router.TopRoutes)

server.observe("readable", function (event, request) {
    Web.serve(request, router)
})
server.attach()
