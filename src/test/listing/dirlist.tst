/*
    dirlist.tst - Directory listings
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"

let http: Http = new Http

if (App.config.dir) {
    http.get(HTTP + "/listing/")
    assert(http.status == 200)
}
