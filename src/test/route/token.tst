/*
    token.tst - Test tokenized parameters
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/tokens/login?fast"
assert(http.status == 200)
assert(http.response == "login-fast")
