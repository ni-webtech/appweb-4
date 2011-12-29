/*
    var.tst - Test Update var
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

http.setHeader("From", "Mars")
http.get(HTTP + "/route/update/var")

assert(http.status == 200)
assert(http.response == "Mars")
http.close()
