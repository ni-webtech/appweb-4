/*
    xname.tst - ESP get with .xesp extension
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

if (!test.cross) {
    http.get(HTTP + "/test.xesp")
    assert(http.status == 200)
} else {
    test.skip("Disabled if cross-compiling")
}
