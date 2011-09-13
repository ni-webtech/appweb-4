/*
    param.tst - Test match by param
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Should fail
http.get(HTTP + "/route/param?name=ralph")
assert(http.status == 404)
http.close()

http.get(HTTP + "/route/param?name=peter")
assert(http.status == 200)
assert(http.response == "peter")
http.close()

//  Should fail
http.form(HTTP + "/route/param", {name: "ralph"})
assert(http.status == 404)
http.close()

http.form(HTTP + "/route/param", {name: "peter"})
assert(http.status == 200)
assert(http.response == "peter")
http.close()
