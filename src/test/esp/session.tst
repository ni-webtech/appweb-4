/*
    session.tst - ESP session tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!global.test || test.os != "WIN") {

    //  GET /mvc/user/login
    http.get(HTTP + "/mvc/user/login")
    assert(http.status == 200)
    assert(http.response.contains("Please Login"))
    let cookie = http.header("Set-Cookie")
    if (cookie) {
        cookie = cookie.match(/(-esp-session-=.*);/)[1]
    }
    assert(cookie && cookie.contains("-esp-session-="))
    http.close()

    //  POST /mvc/user/login
    http.setCookie(cookie)
    http.form(HTTP + "/mvc/user/login", { username: "admin", password: "secret", color: "blue" })
    assert(http.status == 200)
    assert(http.response.contains("Valid Login"))
    assert(!http.sessionCookie)
    http.close()


    //  GET /mvc/user/login
    http.setCookie(cookie)
    http.get(HTTP + "/mvc/user/login")
    assert(http.status == 200)
    assert(http.response.contains("Logged in"))
    http.close()
}
