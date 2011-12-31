/*
    types.tst - Test cache matching by mime type of the file extension
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

/*
    Fetch twice and see if caching is working
 */
function cached(uri): Boolean {
    http.get(HTTP + uri)
    assert(http.status == 200)
    let resp = deserialize(http.response)
    let first = resp.number

    http.get(HTTP + uri)
    assert(http.status == 200)
    resp = deserialize(http.response)
    http.close()
    return (resp.number == first)
}

//  The php request should be cached and the esp should not
if (App.config.bld_php) {
    assert(cached("/types/cache.php"))
}
if (App.config.bld_esp) {
    assert(!cached("/types/cache.esp"))
}
