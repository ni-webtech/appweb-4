/*
    handlers.tst - Test caching with various handler types
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
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

if (App.config.bld_php) {
    testCache("/combined/cache.php")
}
if (App.config.bld_esp) {
    testCache("/combined/cache.esp")
}
if (App.config.bld_ejscript) {
    testCache("/combined/cache.ejs")
}
if (App.config.bld_cgi) {
    testCache("/combined/cache.cgi")
}

