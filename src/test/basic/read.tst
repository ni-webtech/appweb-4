/*
    read.tst - Various Http read tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

/*  TODO
//  Validate readXml()
http.get(HTTP + "/test.xml")
assert(http.readXml().customer.name == "Joe Green")
*/

if (!test || test.config["ejscript"] == 1) {
    //  Test http.read() into a byte array
    http.get(HTTP + "/big.ejs")
    buf = new ByteArray
    count = 0
    while (http.read(buf) > 0) {
        count += buf.available
    }
    if (count != 63201) {
        print("COUNT IS " + count + " code " + http.status)
    }
    assert(count == 63201)
    http.close()
}

http.get(HTTP + "/lines.txt")
lines = http.readLines()
for (l in lines) {
    line = lines[l]
    assert(line.contains("LINE"))
    assert(line.contains((l+1).toString()))
}
assert(http.status == 200)
