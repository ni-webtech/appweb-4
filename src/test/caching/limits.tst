/*
    limits.tst - Test caching limits
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Get a document that will normally require chunking
http.get(HTTP + "/app/cache/huge")
assert(http.status == 200)
assert(http.header("Transfer-Encoding") == "chunked")
assert(!http.header("Content-Length"))
assert(http.response.contains("Line: 09999 aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>"))

//  Because of the LimitCacheItem, huge won't be cached,
http.get(HTTP + "/app/cache/huge")
assert(http.status == 200)
assert(http.header("Transfer-Encoding") == "chunked")
assert(!http.header("Content-Length"))
assert(http.response.contains("Line: 09999 aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>"))

http.close()
