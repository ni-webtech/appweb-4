/*
    query.tst - Http query tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

if (App.config.bld_ejscript) {
    http.get(HTTP + "/form.ejs?a&b&c")
    assert(http.status == 200)
    assert(http.response.contains('"a": ""'))
    assert(http.response.contains('"b": ""'))
    assert(http.response.contains('"c": ""'))

    http.get(HTTP + "/form.ejs?a=x&b=y&c=z")
    assert(http.status == 200)
    assert(http.response.contains('"a": "x"'))
    assert(http.response.contains('"b": "y"'))
    assert(http.response.contains('"c": "z"'))

} else {
    test.skip("Ejscript not enabled")
}
