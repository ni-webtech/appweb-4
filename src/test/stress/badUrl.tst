/*
    badUrl.tst - Stress test malformed URLs 
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  TODO - this will be more meaningful when ejs supports unicode
http.get(HTTP + "/index\x01.html")
assert(http.status == 404)
assert(http.response.contains("Not Found"))
http.close()
