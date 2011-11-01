/*
    xname.tst - ESP get with .xesp extension
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/test.xesp")
assert(http.status == 200)
