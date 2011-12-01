/*
    post.tst - Stress test post data
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"

let http: Http = new Http

/* Depths:    0  1  2  3   4   5   6    7    8    9    */
var sizes = [ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 ]

//  Create test buffer 
buf = new ByteArray
for (i in 64) {
    for (j in 15) {
        buf.writeByte("A".charCodeAt(0) + (j % 26))
    }
    buf.writeByte("\n".charCodeAt(0))
}

//  Scale the count by the test depth
count = sizes[test.depth] * 1024

function postTest(url: String) {
    // print("@@@@ Writing " + count * buf.available + " to " + url)
    http.post(HTTP + url)
    // print("Count " + count + " buf " + buf.available + " total " + count * buf.available)
    for (i in count) {
        let n = http.write(buf)
    }
    http.wait(120 * 1000)
    if (http.status != 200) {
        print("STATUS " + http.status)
        print(http.response)
    }
    assert(http.status == 200)
    assert(http.response)
    http.close()
}

postTest("/index.html")

if (test.config["ejscript"] == 1) {
    postTest("/form.ejs")
}

if (test.config["php"] == 1) {
    postTest("/form.php")
}

if (test.config["cgi"] == 1) {
    postTest("/cgi-bin/cgiProgram")
}

if (test.config["esp"] == 1) {
    postTest("/test.esp")
}
