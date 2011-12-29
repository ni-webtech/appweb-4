/*
    suffix.tst - Test AddLanguage
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

http.setHeader("Accept-Language", "en")
http.get(HTTP + "/lang/suffix/index.html")
assert(http.status == 200)
assert(http.readString().contains("English Suffix"))
