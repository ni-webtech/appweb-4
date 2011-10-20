/*
    cmd.tst - Test Update cmd
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

let path = Path("../route-update-cmd.tmp")
path.remove()
assert(!path.exists)

//  Run a command that creates ../route-update-cmd.tmp
  
http.get(HTTP + "/route/update/cmd")
assert(http.status == 200)
assert(http.response == "UPDATED")
assert(path.exists)
assert(path.remove())
http.close()
