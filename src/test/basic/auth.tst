/*
    auth.tst - Authentication http tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

let http: Http = new Http


//  Access to basic/basic.html accepts by any valid user
http.get(HTTP + "/basic/basic.html")
assert(http.status == 401)

http.setCredentials("joshua", "pass1")
http.get(HTTP + "/basic/basic.html")
assert(http.status == 200)

http.setCredentials("mary", "pass2")
http.get(HTTP + "/basic/basic.html")
assert(http.status == 200)


//  Access to basic/user/user.html accepts joshua only
http.get(HTTP + "/basic/basic.html")
assert(http.status == 401)

http.setCredentials("joshua", "pass1")
http.get(HTTP + "/basic/basic.html")
assert(http.status == 200)

http.setCredentials("mary", "pass2")
http.get(HTTP + "/basic/basic.html")
assert(http.status == 401)



//  TODO DIGEST
