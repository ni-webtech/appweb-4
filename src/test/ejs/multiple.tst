/*
    multiple.tst -- Multiple overlapped requests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"

var nap: Http = new Http

//  Issue a nap request. This request takes 2 seconds to complete -- waited on below.
nap.get(HTTP + "/nap.ejs")


//  Overlapped non-blocking request
var http: Http = new Http
for (i in 20) {
    let now = new Date()
    http.get(HTTP + "/index.ejs")
    assert(http.status == 200)
    assert(now.elapsed < 2000)
}
http.close()


//  Wait for nap request
assert(nap.status == 200)
nap.close()
