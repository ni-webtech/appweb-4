/*
    reload.tst - ESP reload tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  First get
let path = new Path("../web/reload.esp")
path.write('<html><body><% espWrite(conn, "First", -1); %></body></html>')
http.get(HTTP + "/reload.esp")
assert(http.status == 200)
assert(http.response.contains("First"))
http.close()

//  Create a new file and do a second get
path.write('<html><body><% espWrite(conn, "Second", -1); %></body></html>')
http.get(HTTP + "/reload.esp")
assert(http.status == 200)
assert(http.response.contains("Second"))
http.close()

path.remove()
