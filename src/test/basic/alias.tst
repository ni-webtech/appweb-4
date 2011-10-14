/*
    alias.tst - Alias http tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

let http: Http = new Http

http.get(HTTP + "/aliasDir/atest.html")
assert(http.status == 200)
assert(http.response.contains("alias/atest.html"))

http.get(HTTP + "/aliasFile/")
assert(http.status == 200)
assert(http.response.contains("alias/atest.html"))

http.get(HTTP + "/AliasDocs/index.html")
assert(http.status == 200)
assert(http.response.contains("My Documents/index.html"))
http.close()
