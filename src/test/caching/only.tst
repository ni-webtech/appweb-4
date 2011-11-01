/*
    only.tst - Test caching in "only" mode. This caches only the exact URIs specified.
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  1. Test that content is being cached
//  Initial get
http.get(HTTP + "/unique/cache.esp")
assert(http.status == 200)
let resp = deserialize(http.response)
let first = resp.number
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "null")

//  Second get, should get the same content (number must not change)
http.get(HTTP + "/unique/cache.esp")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.number == first)
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "null")


//  2. Test that a URI with query will not be cached.
//  Third get, should get different content as URI will be different
http.get(HTTP + "/unique/cache.esp?a=b&c=d")
assert(http.status == 200)
resp = deserialize(http.response)
let firstQuery = resp.number
assert(resp.number != first)
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "a=b&c=d")

http.get(HTTP + "/unique/cache.esp?w=x&y=z")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.number != first)
assert(resp.number != firstQuery)
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "w=x&y=z")

http.close()
