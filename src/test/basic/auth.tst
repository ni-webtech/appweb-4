/*
    auth.tst - Authentication http tests
 */

const HTTP = (global.session && session["http"]) || ":4100"

let http: Http = new Http

http.get(HTTP + "/basic/basic.html")
assert(http.status == 401)
http.setCredentials("joshua", "pass1")
http.get(HTTP + "/basic/basic.html")
assert(http.status == 200)

//  TODO DIGEST
