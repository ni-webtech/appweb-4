require ejs.web

let server: HttpServer = new HttpServer({documents: "web"})
var router = Router(Router.Top)

server.observe("readable", function (event, request) {
    serve.serve(request, router)
})
server.attach()
