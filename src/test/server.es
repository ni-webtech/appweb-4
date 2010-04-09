require ejs.web

let server: HttpServer = new HttpServer('.', "./web")
var router = Router(Router.TopRoutes)

server.addListener("readable", function (event, request) {
    Web.serve(request, router)
})
server.attach()

