/*
    dirlist.tst - Directory listings
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.config["dir"] == 1) {
    http.get(HTTP + "/listing/")
    assert(http.status == 200)
}
