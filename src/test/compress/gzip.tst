/*
    gzip.tst - Compressed content
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Accept a gzip encoding if present
http.setHeader("Accept-Encoding", "gzip")
http.get(HTTP + "/compress/compressed.txt")
assert(http.status == 200)
