/*
    cmd.tst - Test Update cmd
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

let path = Path("/tmp/route-update.tmp")
path.remove()
assert(!path.exists())

//  Run a command that creates /tmp/route-update.tmp
  
http.get(HTTP + "/route/update/cmd")
assert(http.status == 200)
assert(http.response == "test-update.tmp")
assert(path.exists())
assert(path.remove())
http.close()
