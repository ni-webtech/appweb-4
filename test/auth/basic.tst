/*
    basic.tst - Basic authentication tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"

let http: Http = new Http

http.setCredentials("anybody", "PASSWORD WONT MATTER")
http.get(HTTP + "/index.html")
assert(http.status == 200)

//  Access to basic/basic.html accepts by any valid user
http.get(HTTP + "/auth/basic/basic.html")
assert(http.status == 401)

http.setCredentials("joshua", "pass1")
http.get(HTTP + "/auth/basic/basic.html")
assert(http.status == 200)

http.setCredentials("mary", "pass2")
http.get(HTTP + "/auth/basic/basic.html")
assert(http.status == 200)


//  Access accepts joshua only
http.setCredentials(null, null)
http.get(HTTP + "/auth/basic/joshua/user.html")
assert(http.status == 401)

http.setCredentials("joshua", "pass1")
http.get(HTTP + "/auth/basic/joshua/user.html")
assert(http.status == 200)

http.setCredentials("mary", "pass2")
http.get(HTTP + "/auth/basic/joshua/user.html")
assert(http.status == 403)

if (App.config.bit_cgi && global.test && test.hostOs != "VXWORKS") {
    /* Requires /bin/sh */
    http.setCredentials(null, null)
    http.get(HTTP + "/auth/basic/basic.cgi")
    assert(http.status == 401)
    http.setCredentials("joshua", "pass1")
    http.get(HTTP + "/auth/basic/basic.cgi")
    assert(http.status == 200)
    assert(http.response.contains("SCRIPT_NAME=/auth/basic/basic.cgi"))
    assert(!http.response.contains("PATH_INFO"))
    assert(!http.response.contains("PATH_TRANSLATED"))
    assert(http.response.contains("REMOTE_USER=joshua"))
}
