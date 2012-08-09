/*
    matrixssl.tst - Matrixssl tests
 */

if (!Config.SSL) {
    test.skip("SSL not enabled in ejs")

} else if (App.config.bit_matrixssl) {
    const HTTPS = App.config.uris.matrixssl || "https://127.0.0.1:4210"
    let http: Http = new Http

    /*
        With keep alive
     */
    http.verify = false
    for (i in 110) {
        http.get(HTTPS + "/index.html")
        assert(http.status == 200)
        assert(http.response)
        http.reset()
    }
    http.close()

    /*
        With-out keep alive
     */
    for (i in 50) {
        http.verify = false
        http.get(HTTPS + "/index.html")
        assert(http.status == 200)
        assert(http.response)
        http.close()
    }

} else {
    test.skip("SSL not enabled")
}
