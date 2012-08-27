/*
    post.tst - Posted form authentication tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
const HTTPS = App.config.uris.https || "127.0.0.1:4110"

let http: Http = new Http

//  Will be denied
http.setCredentials("anybody", "wrong password")
http.get(HTTP + "/auth/post/index.html")
assert(http.status == 302)
let location = http.header('location')
assert(location.contains('https'))
assert(location.contains('login.esp'))

//  Will return login form
http.get(location)
assert(http.status == 200)
assert(http.response.contains("<form"))
assert(http.response.contains('action="/auth/post/login"'))
let cookie = http.header("Set-Cookie")
assert(cookie.match(/(-http-session-=.*);/)[1])

//  Login. The response should redirct to /auth/post
http.reset()
http.setCookie(cookie)
http.form("https://" + HTTPS + "/auth/post/login", {username: "joshua", password: "pass1"})
assert(http.status == 302)
location = http.header('location')
assert(location.contains('https'))
assert(location.contains('/auth/post'))

//  Now logged in
http.reset()
http.setCookie(cookie)
http.get("https://" + HTTPS + "/auth/post/index.html")
assert(http.status == 200)

http.close()

