/*
    session.tst - ESP session tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  GET /app/user/login
http.get(HTTP + "/app/user/login")
assert(http.status == 200)
assert(http.response.contains("Please Login"))
let cookie = http.header("Set-Cookie")
if (cookie) {
    cookie = cookie.match(/(-esp-session-=.*);/)[1]
}
assert(cookie && cookie.contains("-esp-session-="))
http.close()

//  POST /app/user/login
http.setCookie(cookie)
http.form(HTTP + "/app/user/login", { username: "admin", password: "secret", color: "blue" })
assert(http.status == 200)
assert(http.response.contains("Valid Login"))
assert(!http.sessionCookie)
http.close()


//  GET /app/user/login
http.setCookie(cookie)
http.get(HTTP + "/app/user/login")
assert(http.status == 200)
assert(http.response.contains("Logged in"))
http.close()
