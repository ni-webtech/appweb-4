/*
    errordoc.tst - Test Error Documents
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http


//  Standard error messages
http.get(HTTP + "/wont-be-there.html")
assert(http.status == 404)
assert(http.response.contains("Access Error: 404 -- Not Found"))
http.close()

//  Error doc (without redirection)
http.get(HTTP + "/error/also-wont-be-there.html")
assert(http.status == 301)
assert(http.response.contains("<title>Moved Permanently</title>"))
http.close()

http = new Http
http.followRedirects = true
http.get(HTTP + "/error/also-wont-be-there.html")
assert(http.status == 200)
assert(http.response.contains("<body>Bad luck - Can't find that document</body>"))
http.close()
