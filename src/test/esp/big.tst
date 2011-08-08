/*
    big.tst - Various Http read tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Test http.read() into a byte array
http.get(HTTP + "/big.esp")
buf = new ByteArray
count = 0
while (http.read(buf) > 0) {
    count += buf.available
}
if (count != 62401) {
    print("COUNT IS " + count + " code " + http.status)
}
assert(count == 62401)
http.close()
