/*
    repeat.tst - Repeated SSL http tests
 */

if (!Config.SSL) {
    test.skip("SSL not enabled in ejs")

} else if (App.config.bld_ssl) {
    const HTTPS = App.config.uris.ssl || "https://127.0.0.1:4110"
    let http: Http = new Http

    /*
        With keep alive
     */
    for (i in 110) {
        http.get(HTTPS + "/index.html")
        http.reset()
    }

    /*
        With-out keep alive
     */
    for (i in 50) {
        http.get(HTTPS + "/index.html")
        http.close()
    }

} else {
    test.skip("SSL not enabled")
}
