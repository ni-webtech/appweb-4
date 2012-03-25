/*
    get.tst - ESP GET tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Basic get. Validate response code and contents
http.get(HTTP + "/test.esp")
assert(http.status == 200)
assert(http.response.contains("ESP Test Program"))

/* When running in test, the name may be appweb or forAppwebTest */
assert(http.response.contains("Product Name"))
http.close()

if (App.test.os == "WIN") {
    http.get(HTTP + "/teST.eSP")
    assert(http.status == 200)
    http.close()
}
