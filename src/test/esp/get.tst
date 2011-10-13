/*
    get.tst - ESP GET tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Basic get. Validate response code and contents
http.get(HTTP + "/test.esp")
assert(http.status == 200)
assert(http.response.contains("ESP Test Program"))

/* When running in test, the name may be appweb or forAppwebTest */
assert(http.response.contains("Product Name"))
http.close()

if (global.test && test.os == "WIN") {
    http.get(HTTP + "/teST.eSP")
    assert(http.status == 200)
    http.close()
}
