/*
    unique.tst - Test unique caching mode. This mode, each URI+params is uniquely cached.
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
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


//  2. Test that different request parameters cache different content
//  Third get, should get different content as URI will be different
http.get(HTTP + "/unique/cache.esp?a=b&c=d")
assert(http.status == 200)
resp = deserialize(http.response)
let firstQuery = resp.number
assert(resp.number != first)
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "a=b&c=d")

//  Get again, should be cached data
http.get(HTTP + "/unique/cache.esp?a=b&c=d")
assert(http.status == 200)
resp = deserialize(http.response)
let firstQuery = resp.number
assert(resp.number != first)
assert(resp.number == firstQuery)
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "a=b&c=d")

//  3. Another query should be uniquely cached
http.get(HTTP + "/unique/cache.esp?w=x&y=z")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.number != first)
assert(resp.number != firstQuery)
let secondQuery = resp.number
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "w=x&y=z")

//  Should be cached uniquely
http.get(HTTP + "/unique/cache.esp?w=x&y=z")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.number != first)
assert(resp.number != firstQuery)
assert(resp.number == secondQuery)
assert(resp.uri == "/unique/cache.esp")
assert(resp.query == "w=x&y=z")

http.close()
