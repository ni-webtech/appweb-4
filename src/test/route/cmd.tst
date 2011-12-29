/*
    cmd.tst - Test Update cmd
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
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
