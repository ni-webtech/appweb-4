/*
    token.tst - Test tokenized parameters
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

http.get(HTTP + "/route/tokens/login?fast")
assert(http.status == 200)
assert(http.response == "login-fast")
