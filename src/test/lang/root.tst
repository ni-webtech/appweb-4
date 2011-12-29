/*
    root.tst - Test AddLanguageRoot
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

http.setHeader("Accept-Language", "en-US,en;q=0.8")
http.get(HTTP + "/lang/root/eng.html")
assert(http.status == 200)
assert(http.readString().contains("Hello English"))
