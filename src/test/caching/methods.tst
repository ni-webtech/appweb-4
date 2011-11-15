/*
    methods.tst - Test cache matching by method
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

/*
    Fetch twice and test if caching is working
 */
function cached(method, uri): Boolean {
    let http: Http = new Http

    //  Clear cache
    http.setHeader("Cache-Control", "no-cache")
    http.connect(method, HTTP + uri)
    http.wait()

    //  First fetch
    http.connect(method, HTTP + uri)
    assert(http.status == 200)
    let resp = deserialize(http.response)
    let first = resp.number

    //  Second fetch
    http.connect(method, HTTP + uri)
    assert(http.status == 200)
    resp = deserialize(http.response)
    http.close()
    return (resp.number == first)
}

if (!test || test.config["esp"] == 1) {
    //  The POST requst should be cached and the GET not
    assert(cached("POST", "/methods/cache.esp"))
    assert(!cached("GET", "/methods/cache.esp"))
}

