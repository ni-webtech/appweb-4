/*
 *  header.tst - Http response header tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
const URL = HTTP + "/index.html"
let http: Http = new Http

http.get(HTTP + "/index.html")
connection = http.header("Connection")
assert(connection == "keep-alive")

http.get(HTTP + "/index.html")
assert(http.statusMessage == "OK")
assert(http.contentType == "text/html")
assert(http.date != "")
assert(http.lastModified != "")

http.post(HTTP + "/index.html")
assert(http.status == 200)

//  Request headers
http.addHeader("key", "value")
http.get(HTTP + "/index.html")
assert(http.status == 200)
