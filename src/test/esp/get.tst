/*
    get.tst - ESP GET tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.os != "WIN") {
    //  Basic get. Validate response code and contents
    http.get(HTTP + "/test.esp")
    assert(http.status == 200)
    assert(http.response.contains("ESP Test Program"))
    assert(http.response.contains("Product Name: appweb"))
    http.close()

    if (global.test && test.os == "WIN") {
        http.get(HTTP + "/teST.eSP")
        assert(http.status == 200)
    }
}
