/*
    gzip.tst - Compressed content
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Accept a gzip encoding if present
http.setHeader("Accept-Encoding", "gzip")
http.get(HTTP + "/compress/compressed.txt")
assert(http.status == 200)
