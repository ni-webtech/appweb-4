/*
    handlers.tst - Test caching with various handler types
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  1. Test that content is being cached
//  Initial get
function testCache(uri) {
    http.get(HTTP + uri)
    assert(http.status == 200)
    let resp = deserialize(http.response)
    let first = resp.number

    //  Second get, should get the same content (number must not change)
    http.get(HTTP + uri)
    assert(http.status == 200)
    resp = deserialize(http.response)
    assert(resp.number == first)
    http.close()
}

if (!test || test.config["php"] == 1) {
    testCache("/combined/cache.php")
}
if (!test || test.config["esp"] == 1) {
    testCache("/combined/cache.esp")
}
if (!test || test.config["ejscript"] == 1) {
    testCache("/combined/cache.ejs")
}
if (!test || test.config["cgi"] == 1) {
    testCache("/combined/cache.cgi")
}

