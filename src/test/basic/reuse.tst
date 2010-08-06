/*
    reuse.tst - Test Http reuse for multiple requests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

let http: Http = new Http

http.get(HTTP + "/index.html")
assert(http.status == 200)

http.get(HTTP + "/index.html")
assert(http.status == 200)

http.get(HTTP + "/index.html")
assert(http.status == 200)

http.get(HTTP + "/index.html")
assert(http.status == 200)
