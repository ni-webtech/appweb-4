/*
    session.tst - ESP session tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  GET /app/test/login
http.get(HTTP + "/app/test/login")
assert(http.status == 200)
assert(http.response.contains("Please Login"))
let securityToken = http.header("X-Security-Token")
let cookie = http.header("Set-Cookie")
if (cookie) {
    cookie = cookie.match(/(-http-session-=.*);/)[1]
}

assert(cookie && cookie.contains("-http-session-="))
// print("STATUS", http.status)
// dump("\nPRIOR HEADERS", http.headers)
// print("PRIOR RESPONSE: \"" + http.response + "\"")
http.close()

//  POST /app/test/login
http.setCookie(cookie)
http.setHeader("__esp_security_token__", securityToken)
http.form(HTTP + "/app/test/login", { 
    __esp_security_token__: securityToken,
    username: "admin", 
    password: "secret", 
    color: "blue" 
})
assert(http.status == 200)
assert(http.response.contains("Valid Login"))
assert(!http.sessionCookie)
// print("STATUS", http.status)
// dump("\nXX PRIOR HEADERS", http.headers)
// print("PRIOR RESPONSE: \"" + http.response + "\"")
http.close()


//  GET /app/test/login
http.setCookie(cookie)
http.get(HTTP + "/app/test/login")
assert(http.status == 200)
// print("STATUS", http.status)
// dump("HEADERS", http.headers)
// print("RESPONSE: \"" + http.response + "\"")
assert(http.response.contains("Logged in"))
http.close()
