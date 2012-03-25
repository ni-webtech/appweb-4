/*
    manual.tst - Test manual caching mode
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Prep and clear the cache
http.get(HTTP + "/app/cache/clear")
assert(http.status == 200)

//  1. Test that content is being cached
//  Initial get
http.get(HTTP + "/app/cache/manual")
assert(http.status == 200)
let resp = deserialize(http.response)
let first = resp.number
assert(resp.uri == "/app/cache/manual")
assert(resp.query == "null")

//  Second get, should get the same content (number must not change)
//  This is being done manually by the "manual" method in the cache controller
http.get(HTTP + "/app/cache/manual")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.number == first)
assert(resp.uri == "/app/cache/manual")
assert(resp.query == "null")


//  Update the cache
http.get(HTTP + "/app/cache/update?updated=true")
assert(http.status == 200)
assert(http.response == "done")

//  Get again, should get updated cached data
http.get(HTTP + "/app/cache/manual")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.query == "updated=true") 


//  Test X-SendCache
http.get(HTTP + "/app/cache/manual?send")
assert(http.status == 200)
resp = deserialize(http.response)
assert(resp.query == "updated=true") 

http.close()
