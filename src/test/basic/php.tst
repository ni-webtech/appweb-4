/*
    php.tst - PHP tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"

if (App.config.bld_php) {
    let http: Http = new Http

    //  Simple Get 
    http.get(HTTP + "/index.php")
    assert(http.status == 200)
    assert(http.response.contains("Hello PHP World"))

    //  Get 
    http.get(HTTP + "/form.php")
    assert(http.status == 200)
    assert(http.response.contains("form.php"))

    //  Form
    http.form(HTTP + "/form.php?a=b&c=d", { name: "John Smith", address: "777 Mulberry Lane" })
    assert(http.status == 200)
    assert(http.response.contains("name is John Smith"))
    assert(http.response.contains("address is 777 Mulberry Lane"))
    http.close()

    //  Big output
    http.get(HTTP + "/big.php")
    assert(http.status == 200)
    data = new ByteArray
    while ((count = http.read(data))) {
        assert(data.toString().contains("aaaabbbb"))
    }

} else {
    test.skip("PHP not enabled")
}
