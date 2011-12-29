/*
    single.tst -- Single non blocking request
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"

var http: Http = new Http

http.get(HTTP + "/index.ejs")
assert(http.status == 200)
http.close()
