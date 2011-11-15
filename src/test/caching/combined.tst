/*
    Combined.tst - Test combined caching. Params are ignored for caching URIs.
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Clear cache
http.setHeader("Cache-Control", "no-cache")
http.get(HTTP + "/combined/cache.esp")
http.wait()

//  1. Test that content is being cached
//  Initial get
http.get(HTTP + "/combined/cache.esp")
assert(http.status == 200)
let resp = deserialize(http.response)
let first = resp.number
assert(resp.uri == "/combined/cache.esp")
assert(resp.query == "null")

//  Second get, should get the same content (number must not change)
http.get(HTTP + "/combined/cache.esp")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.number == first)
assert(resp.uri == "/combined/cache.esp")
assert(resp.query == "null")


//  2. Test that different request parameters cache the same
http.get(HTTP + "/combined/cache.esp?a=b&c=d")
assert(http.status == 200)
resp = deserialize(http.response)
let firstQuery = resp.number
assert(resp.number == first)
assert(resp.uri == "/combined/cache.esp")
assert(resp.query == "null")

http.close()
