/*
    mvc.tst - ESP MVC tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  /app
http.get(HTTP + "/app")
assert(http.status == 200)
assert(http.response.contains("Welcome to ESP"))
http.close()

//  /app/
http.get(HTTP + "/app/")
assert(http.status == 200)
assert(http.response.contains("Welcome to ESP"))
http.close()

//  /app/static/layout.css
http.get(HTTP + "/app/static/layout.css")
assert(http.status == 200)
assert(http.response.contains("Default layout style sheet"))
http.close()

//  /app/static/index.esp
http.get(HTTP + "/app/static/index.esp")
assert(http.status == 200)
assert(http.response.contains("Welcome to ESP"))
http.close()

//  /app/test/check - this tests a controller without view
http.post(HTTP + "/app/test/check")
assert(http.status == 200)
assert(http.response.contains("Check: OK"))

//  /app/test/details - this tests templates
http.post(HTTP + "/app/test/details")
assert(http.status == 200)
assert(http.response.contains("<title>Test App</title>"))
assert(http.response.contains("Powered by Appweb"))
assert(http.response.contains("SECRET: 42"))
http.close()
