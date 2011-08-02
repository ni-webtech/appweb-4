/*
    redirect.tst - ESP redirection tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.os != "WIN") {
    //  This will not follow redirects
    http.get(HTTP + "/redirect.esp")
    http.followRedirects = false
    assert(http.status == 301)
    assert(http.response.contains("<h1>Moved Permanently</h1>"))
    http.close()

    //  This will do a transparent redirect to index.esp
    http.followRedirects = true
    http.get(HTTP + "/redirect.esp")
    assert(http.status == 200)
    assert(http.response.contains("Greetings: Hello Home Page"))
    http.close()

    if (global.test && test.os == "WIN") {
        http.get(HTTP + "/teST.eSP")
        assert(http.status == 200)
    }
}
