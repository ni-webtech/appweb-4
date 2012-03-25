/*
    upload.tst - File upload tests
 */

require ejs.unix

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

if (App.config.bld_ejscript) {
    http.upload(HTTP + "/upload.ejs", { myfile: "test.dat"} )
    assert(http.status == 200)
    assert(http.response.contains('"clientFilename": "test.dat"'))
    assert(http.response.contains('Uploaded'))
    http.wait()

    http.upload(HTTP + "/upload.ejs", { myfile: "test.dat"}, {name: "John Smith", address: "100 Mayfair"} )
    assert(http.status == 200)
    assert(http.response.contains('"clientFilename": "test.dat"'))
    assert(http.response.contains('Uploaded'))
    assert(http.response.contains('"address": "100 Mayfair"'))

} else {
    test.skip("Ejscript not enabled")
}
