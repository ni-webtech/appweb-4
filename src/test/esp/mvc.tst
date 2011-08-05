/*
    mvc.tst - ESP MVC tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.os != "WIN") {

    //  /mvc
    http.get(HTTP + "/mvc")
    assert(http.status == 200)
    assert(http.response.contains("Index Page"))
    http.close()

    //  /mvc/
    http.get(HTTP + "/mvc/")
    assert(http.status == 200)
    assert(http.response.contains("Index Page"))
    http.close()

    //  /mvc/static/layout.css
    http.get(HTTP + "/mvc/static/layout.css")
    assert(http.status == 200)
    assert(http.response.contains("Default layout style sheet"))
    http.close()

    //  /mvc/static/index.esp
    http.get(HTTP + "/mvc/static/index.esp")
    assert(http.status == 200)
    assert(http.response.contains("Index Page"))
    http.close()

    //  /mvc/user/check - this tests a controller without view
    http.get(HTTP + "/mvc/user/check")
    assert(http.status == 200)
    assert(http.response.contains("Check: OK"))

    //  /mvc/user/details - this tests templates
    http.get(HTTP + "/mvc/user/details")
    assert(http.status == 200)
    assert(http.response.contains("<title>MVC Title</title>"))
    assert(http.response.contains("Layout Footnote"))
    assert(http.response.contains("SECRET: 42"))
    http.close()
}
