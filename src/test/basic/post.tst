/*
    post.tst - Post method tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"

let http: Http = new Http

if (App.config.bld_ejscript) {
    http.post(HTTP + "/form.ejs", "Some data")
    assert(http.status == 200)
    http.close()

    http.form(HTTP + "/form.ejs", {name: "John", address: "700 Park Ave"})
    assert(http.response.contains('"name": "John"'))
    assert(http.response.contains('"address": "700 Park Ave"'))
    http.close()

} else {
    test.skip("Ejscript not enabled")
}

//  TODO MORE 
