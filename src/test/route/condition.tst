/*
    condition.tst - Test conditions
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/route/cond")
assert(http.status == 200)
assert(http.response == "http")
