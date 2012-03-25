/*
    client.tst - Test client caching
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

http.get(HTTP + "/app/cache/client")
assert(http.status == 200)
assert(http.header("Cache-Control") == "max-age=3600")

http.close()


