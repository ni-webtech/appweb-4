/*
    session.tst -- Test sessions
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

var http: Http = new Http

//  Create a session cookie
http.get(HTTP + "/session.ejs")
assert(http.status == 200)
let cookie = http.sessionCookie
assert(http.response.trim() == "")
http.close()


//  Issue a request with the cookie to pick up the value set in session.ejs
http.setCookie(cookie)
http.get(HTTP + "/session.ejs")
assert(http.response.trim() == "77")
