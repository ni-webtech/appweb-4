/*
    ext.tst - Test cache matching by extension.
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

/*
    Fetch twice and see if caching is working
 */
function cached(uri, expected): Boolean {
    http.get(HTTP + uri)
    assert(http.status == 200)
    let resp = deserialize(http.response)
    let first = resp.number

    http.get(HTTP + uri)
    assert(http.status == 200)
    resp = deserialize(http.response)
    http.close()
    if (expected != (resp.number == first)) {
        print("\nFirst number:  " + first)
        print("\nSecond number: " + resp.number)
        dump(http.response)
    }
    return (resp.number == first)
}

//  The esp request should be cached and the php should not
if (App.config.bld_esp) {
    assert(cached("/ext/cache.esp", true))
}
if (App.config.bld_php) {
    assert(!cached("/ext/cache.php", false))
}

