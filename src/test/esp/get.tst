/*
    get.tst - ESP GET tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Basic get. Validate response code and contents
http.get(HTTP + "/test.esp")
assert(http.status == 200)
assert(http.response.contains("Hello"))
http.close()

if (test.os == "WIN") {
    http.get(HTTP + "/teST.eSP")
    assert(http.status == 200)
}
