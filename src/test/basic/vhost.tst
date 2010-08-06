/*
 *  vhost.tst - Virtual Host tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
const VHOST = (global.tsession && tsession["vhost"]) || ":4100"
const IPHOST = (global.tsession && tsession["iphost"]) || ":4200"

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
    let port = tsession["vhostPort"]
    let vhost = "http://localhost:" + port
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
    vhost = "http://127.0.0.1:" + port
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
    http.get(IPHOST + "/private.html")
    assert(http.status == 200)
}

mainHost()
namedHost()
ipHost()
