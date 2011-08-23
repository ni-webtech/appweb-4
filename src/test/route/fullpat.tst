/*
    fullpat.tst - Test pattern matching features
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Test route-01  ^/(user|admin)/{action}/[^a-z]\{2}(\.[hH][tT][mM][lL])$>
http.get(HTTP + "/user/login/AA.html"
assert(http.status == 200)
assert(http.response == "user")

http.get(HTTP + "/user/login/aA.html"
assert(http.status == 404)
