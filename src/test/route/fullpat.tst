/*
    fullpat.tst - Test pattern matching features
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Test route-01  ^/route/(user|admin)/{action}/[^a-z]\{2}(\.[hH][tT][mM][lL])$>
http.get(HTTP + "/route/user/login/AA.html")
assert(http.status == 200)
assert(http.response == "user")

http.get(HTTP + "/route/user/login/aA.html")
assert(http.status == 404)
