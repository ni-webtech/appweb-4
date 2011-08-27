/*
    dirlist.tst - Test mod_dir
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

http.get(HTTP + "/listing/")
assert(http.status == 200)


//  MOB - more here
