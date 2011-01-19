/*
    methods.tst - Test misc Http methods
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Put a file
data = Path("test.dat").readString()
http.put(HTTP + "/tmp/test.dat", data)
assert(http.status == 201 || http.status == 204)

//  Delete
http.connect("DELETE", HTTP + "/tmp/test.dat")
assert(http.status == 204)

//  Post
http.post(HTTP + "/index.html", "Some data")
assert(http.status == 200)

//  Options
http.connect("OPTIONS", HTTP + "/index.html")
assert(http.header("Allow") == "OPTIONS,GET,HEAD,POST,PUT,DELETE")

//  Trace - should be disabled
http.connect("TRACE", HTTP + "/index.html")
assert(http.status == 406)
