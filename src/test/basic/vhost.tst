/*
    vhost.tst - Virtual Host tests
 */

const HTTP = App.config.main || "127.0.0.1:4100"
const NAMED = App.config.named || "127.0.0.1:4111"
const VIRT = App.config.virt || "127.0.0.1:4112"

let http: Http = new Http

function mainHost() {
    http.get(HTTP + "/main.html")
    http.response.contains("HTTP SERVER")
    assert(http.status == 200)

    //  These should fail
    http.get(HTTP + "/iphost.html")
    assert(http.status == 404)
    http.get(HTTP + "/vhost1.html")
    assert(http.status == 404)
    http.get(HTTP + "/vhost2.html")
    assert(http.status == 404)
}


//  The first virtual host listens to "localhost", the second listens to "127.0.0.1". Both on the same port (4111)

function namedHost() {
    let vhost = "http://localhost:" + Uri(NAMED).port
    http.get(vhost + "/vhost1.html")
    assert(http.status == 200)

    //  These should fail
    http.get(vhost + "/main.html")
    assert(http.status == 404)
    http.get(vhost + "/iphost.html")
    assert(http.status == 404)
    http.get(vhost + "/vhost2.html")
    assert(http.status == 404)

    //  Now try the second vhost on 127.0.0.1
    vhost = "http://127.0.0.1:" + Uri(NAMED).port
    http.get(vhost + "/vhost2.html")
    assert(http.status == 200)

    //  These should fail
    http.get(vhost + "/main.html")
    assert(http.status == 404)
    http.get(vhost + "/iphost.html")
    assert(http.status == 404)
    http.get(vhost + "/vhost1.html")
    assert(http.status == 404)
}

function ipHost() {
    let http: Http = new Http
    http.setCredentials("mary", "pass2")
    http.get(VIRT + "/private.html")
    assert(http.status == 200)
}

mainHost()
namedHost()
ipHost()
