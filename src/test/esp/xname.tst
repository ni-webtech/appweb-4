/*
    xname.tst - ESP get with .xesp extension
 */

const HTTP = App.config.main || "127.0.0.1:4100"
let http: Http = new Http

http.get(HTTP + "/test.xesp")
assert(http.status == 200)
