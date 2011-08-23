/*
    default.tst - Test DefaultLanguage
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

// http.setHeader("Accept-Language", "en")
http.get(HTTP + "/lang/default/index.html")
assert(http.status == 200)
assert(http.readString().contains("Bonjour"))
