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
assert(http.readString().contains("First"))
http.close()


//  Second get
path.write('<html><body><% espWrite(conn, "Second", -1); %></body></html>')
http.get(HTTP + "/reload.esp")
assert(http.status == 200)
assert(http.readString().contains("Second"))
http.close()

path.remove()
