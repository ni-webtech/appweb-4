/*
    alias.tst - ESP MVC tests for an EjsAlias
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.os != "WIN") {

    let app = "/mvc2"
    //  /mvc2
    http.get(HTTP + app)
    assert(http.status == 200)
    assert(http.response.contains("Index Page"))
    http.close()

    //  /mvc2/
    http.get(HTTP + app + "/")
    assert(http.status == 200)
    assert(http.response.contains("Index Page"))
    http.close()

    //  /mvc2/static/layout.css
    http.get(HTTP + app + "/static/layout.css")
    assert(http.status == 200)
    assert(http.response.contains("Default layout style sheet"))
    http.close()

    //  /mvc2/static/index.esp
    http.get(HTTP + app + "/static/index.esp")
    assert(http.status == 200)
    assert(http.response.contains("Index Page"))
    http.close()

    //  /mvc2/user/check - this tests a controller without view
    http.get(HTTP + app + "/user/check")
    assert(http.status == 200)
    assert(http.response.contains("Check: OK"))

    //  /mvc2/user/details - this tests templates
    http.get(HTTP + app + "/user/details")
    assert(http.status == 200)
    assert(http.response.contains("<title>MVC Title</title>"))
    assert(http.response.contains("Layout Footnote"))
    assert(http.response.contains("SECRET: 42"))
    http.close()
}
