/*
    repeat.tst - Repeated SSL http tests
 */

if (!Config.SSL) {
    test.skip("SSL not enabled in ejs")

} else if (!test || test.config["ssl"] == 1) {
    const HTTP = (global.tsession && tsession["http"]) || ":4100"
    const HTTPS = (global.tsession && tsession["https"]) || "https://127.0.0.1:4110"
    let http: Http = new Http

    /*
        With keep alive
     */
    for (i in 500) {
        http.get(HTTP + "/index.html")
        http.reset()
    }

    /*
        With-out keep alive
     */
    for (i in 500) {
        http.get(HTTP + "/index.html")
        http.close()
    }

} else {
    test.skip("SSL not enabled")
}
