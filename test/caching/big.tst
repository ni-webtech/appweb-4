/*
    big.tst - Test caching a big file
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Clear cached data
http.get(HTTP + "/app/cache/clear")
assert(http.status == 200)

//  Get a document that will normally require chunking
http.get(HTTP + "/app/cache/big")
assert(http.status == 200)
assert(http.header("Transfer-Encoding") == "chunked")
assert(!http.header("Content-Length"))
assert(http.response.contains("Line: 00999 aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>"))

//  Get again, this time will be cached and will be sent not chunked
http.get(HTTP + "/app/cache/big")
assert(http.status == 200)
assert(!http.header("Transfer-Encoding"))
assert(http.header("Content-Length") == 78000)
assert(http.response.contains("Line: 00999 aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>"))

http.close()
