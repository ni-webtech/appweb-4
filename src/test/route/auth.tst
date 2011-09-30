/*
    auth.tst - Test authorized condition
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/route/auth/basic.html")
assert(http.status == 401)
