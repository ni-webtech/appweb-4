/*
    missing.tst - Add extension
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/route/missing-ext/index"
assert(http.status == 200)
assert(http.response.contains == "Hello PHP World")
