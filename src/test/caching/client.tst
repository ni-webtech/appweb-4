/*
    client.tst - Test client caching
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/app/cache/client")
assert(http.status == 200)
assert(http.header("Cache-Control") == "max-age=3600")

http.close()


