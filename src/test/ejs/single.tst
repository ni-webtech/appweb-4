/*
    single.tst -- Single non blocking request
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

var http: Http = new Http

http.get(HTTP + "/index.ejs")
assert(http.status == 200)
http.close()
