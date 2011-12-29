/*
    auth.tst - Test authorized condition
 */

const HTTP = App.config.main || "127.0.0.1:4100"
let http: Http = new Http

http.get(HTTP + "/route/auth/basic.html")
assert(http.status == 401)
