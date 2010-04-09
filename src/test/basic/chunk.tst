/*
    chunk.tst - Test chunked transfer encoding for response data
 */

const HTTP = (global.session && session["http"]) || ":4100"
let http: Http = new Http

http.post(HTTP + "/index.html")
http.wait()
assert(http.status == 200)

//  TODO - more here. Test various chunk sizes.
//  TODO - want to be able to set the chunk size?
