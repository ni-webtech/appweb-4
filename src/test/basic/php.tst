/*
    php.tst - PHP tests
 */

if (!global.test || test.config["php"] == 1) {
    const HTTP = (global.tsession && tsession["http"]) || ":4100"
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
