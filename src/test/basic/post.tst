/*
    post.tst - Post method tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

let http: Http = new Http

http.post(HTTP + "/form.ejs", "Some data")
assert(http.status == 200)

http.form(HTTP + "/form.ejs", {name: "John", address: "700 Park Ave"})
assert(http.response.contains('"name": "John"'))
assert(http.response.contains('"address": "700 Park Ave"'))

//  TODO MORE 
