require ejs.web

// App.log.info("Starting server.es")
let server: HttpServer = new HttpServer({documents: "web", own: true})
var router = Router(Router.Top)

server.on("readable", function (event, request) {
    server.serve(request, router)
})

server.listen()
