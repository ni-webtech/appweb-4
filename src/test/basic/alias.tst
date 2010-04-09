/*
    alias.tst - Alias http tests
 */

const HTTP = (global.session && session["http"]) || ":4100"

let http: Http = new Http

http.get(HTTP + "/aliasDir/index.html")
assert(http.status == 200)
assert(http.response.contains("alias/index.html"))

http.get(HTTP + "/aliasFile/")
assert(http.status == 200)
assert(http.response.contains("alias/index.html"))

http.get(HTTP + "/AliasForMyDocuments/index.html")
assert(http.status == 200)
assert(http.response.contains("My Documents/index.html"))
