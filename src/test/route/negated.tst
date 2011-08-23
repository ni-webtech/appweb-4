/*
    negated.tst - Add extension via negated route
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/route/test"
assert(http.status == 200)
assert(http.response.contains == "Hello PHP World")
