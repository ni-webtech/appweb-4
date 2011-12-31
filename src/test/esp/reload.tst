/*
    reload.tst - ESP reload tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

if (App.config.bld_debug && !test.cross) {
    //  First get
    let path = new Path("../web/reload.esp")
    path.write('<html><body><% espRender(conn, "First", -1); %></body></html>')
    http.get(HTTP + "/reload.esp")
    assert(http.status == 200)
    assert(http.response.contains("First"))
    http.close()

    //  To ensure all file system record a different mtime for the file
    App.sleep(1100);

    //  Create a new file and do a second get
    path.write('<html><body><% espRender(conn, "Second", -1); %></body></html>')
    http.get(HTTP + "/reload.esp")
    assert(http.status == 200)
    assert(http.response.contains("Second"))
    http.close()

    path.remove()

} else {
    test.skip("Run only in debug builds")
}
