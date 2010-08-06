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

// print("Writing " + count * buf.available)

function postTest(url: String) {
    http.post(HTTP + url)
    for (i in count) {
        http.write(buf)
    }
    assert(http.status == 200)
    http.close()
}

postTest("/index.html")
postTest("/form.ejs")
if (test.config["debug"] == 1) {
    postTest("/egiProgram.egi")
}
if (0 && test.config["php"] == 1) {
    postTest("/form.php")
}

/* TODO BUG - fails
    postTest("/cgi-bin/cgiProgram")
*/
