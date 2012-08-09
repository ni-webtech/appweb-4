/*
    missing.tst - Add extension
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

if (App.config.bit_php) {
    http.get(HTTP + "/route/missing-ext/index")
    assert(http.status == 200)
    assert(http.response.contains("Hello PHP World"))
} else {
    test.skip("PHP not enabled")
}
