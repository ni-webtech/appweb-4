/*
    query.tst - Test match by query
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/route/query?name=ralph"
assert(http.status == 404)
http.close()

http.get(HTTP + "/route/query?name=peter"
assert(http.status == 200)
assert(http.response == "name=peter")
http.close()
