/*
    ext.tst - Test cache matching by extension.
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
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

if (!test || (test.config["esp"] == 1 && test.config["php"] == 1)) {
    //  The esp request should be cached and the php should not
    assert(cached("/ext/cache.esp"))
    assert(!cached("/ext/cache.php"))
}

