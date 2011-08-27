/*
    optional.tst - Test optional tokens
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Route: ^/optional/{controller}(~/{action}~)

//  Test just a controller
http.get(HTTP + "/route/optional/user")
assert(http.status == 200)
assert(http.response == "user-")

//  With trailing "/"
http.get(HTTP + "/route/optional/user/")
assert(http.status == 200)
assert(http.response == "user-")

//  Test controller/action
http.get(HTTP + "/route/optional/user/login")
assert(http.status == 200)
assert(http.response == "user-login")
