/*
    include.tst - ESP include directives
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.os != "WIN") {
    //  Basic get. Validate response code and contents
    http.get(HTTP + "/outer.esp")
    assert(http.status == 200)
    assert(http.response.contains("Hello from inner text"))
    http.close()
}
