/*
    put.tst - Put method tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

/*  TODO
//  Test http.put with content data
data = Path("test.dat").readString()
http.put(HTTP + "/tmp/test.dat", data)
assert(http.status == 201 || http.status == 204)
*/

//  This request should hang because we don't write any data and finalize. Wait with a timeout.
http.setHeader("Content-Length", 1000)
http.put(HTTP + "/a.tmp")
assert(http.wait(250) == false)
http.close()

path = Path("test.dat")
http.setHeader("Content-Length", path.size)
http.put(HTTP + "/test.dat")
file = File(path).open()
buf = new ByteArray
while (file.read(buf)) {
    http.write(buf)
    buf.flush()
}
http.finalize()
http.wait()
assert(http.status == 204)
