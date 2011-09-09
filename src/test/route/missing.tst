/*
    missing.tst - Add extension
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!test || test.config["php"] == 1) {
    http.get(HTTP + "/route/missing-ext/index")
    assert(http.status == 200)
    assert(http.response.contains("Hello PHP World"))
} else {
    test.skip("PHP not enabled")
}
