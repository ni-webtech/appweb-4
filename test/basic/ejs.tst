/*
    ejs.tst - Ejscript basic tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

if (App.config.bit_ejscript) {
    /* Tests */

    function basic() {
        http.get(HTTP + "/ejsProgram.ejs")
        assert(http.status == 200)
        deserialize(http.response)
    }

    function forms() {
        http.get(HTTP + "/ejsProgram.ejs")
        assert(http.status == 200)
        deserialize(http.response)

        /* 
            Not supporting caseless extension matching 

            if (test.os == "WIN") {
                http.get(HTTP + "/ejsProgRAM.eJS")
                assert(http.status == 200)
                deserialize(http.response)
            }
        */
    }

    /*  MOB - not yet supported
    function alias() {
        http.get(HTTP + "/SimpleAlias/ejsProgram.ejs")
        assert(http.status == 200)
        let resp = deserialize(http.response)
        assert(resp.uri == "/SimpleAlias/ejsProgram.ejs")
        assert(resp.query == null)
    }
    */

    function query() {
        http.get(HTTP + "/ejsProgram.ejs?a=b&c=d&e=f")
        let resp = deserialize(http.response)
        assert(resp.pathInfo == "/ejsProgram.ejs")
        assert(resp.params["a"] == "b")
        assert(resp.params["c"] == "d")

        //  Query string vars should not be turned into variables for GETs
        
        http.get(HTTP + "/ejsProgram.ejs?var1=a+a&var2=b%20b&var3=c")
        let resp = deserialize(http.response)
        assert(resp.pathInfo == "/ejsProgram.ejs")
        assert(resp.query ==  "var1=a+a&var2=b%20b&var3=c")
        assert(resp.params["var1"] == "a a")
        assert(resp.params["var2"] == "b b")
        assert(resp.params["var3"] == "c")

        //  Post data should be turned into variables
        
        http.form(HTTP + "/ejsProgram.ejs?var1=a+a&var2=b%20b&var3=c", 
            { name: "Peter", address: "777 Mulberry Lane" })
        let resp = deserialize(http.response)
        assert(resp.query == "var1=a+a&var2=b%20b&var3=c")
        let params = resp.params
        assert(params["var1"] == "a a")
        assert(params["var2"] == "b b")
        assert(params["var3"] == "c")
        assert(params["name"] == "Peter")
        assert(params["address"] == "777 Mulberry Lane")
    }

    function encoding() {
        http.get(HTTP + "/ejsProgram.ejs?var%201=value%201")
        let resp = deserialize(http.response)
        assert(resp.query == "var%201=value%201")
        assert(resp.pathInfo == "/ejsProgram.ejs")
        assert(resp.params["var 1"] == "value 1")
    }

    function status() {
        http.get(HTTP + "/ejsProgram.ejs?code=711")
        assert(http.status == 711)
    }

    function location() {
        let http = new Http
        http.followRedirects = false
        http.get(HTTP + "/ejsProgram.ejs?redirect=http://www.redhat.com/")
        assert(http.status == 302)
        http.close()
    }

    function quoting() {
        http.get(HTTP + "/ejsProgram.ejs?a+b+c")
        let resp = deserialize(http.response)
        let params = resp.params
        assert(resp.query == "a+b+c")
        assert(params["a b c"] == "")

        http.get(HTTP + "/ejsProgram.ejs?a=1&b=2&c=3")
        let resp = deserialize(http.response)
        let params = resp.params
        assert(resp.query == "a=1&b=2&c=3")
        assert(params["a"] == "1")
        assert(params["b"] == "2")
        assert(params["c"] == "3")

        http.get(HTTP + "/ejsProgram.ejs?a%20a=1%201+b%20b=2%202")
        let resp = deserialize(http.response)
        let params = resp.params
        assert(resp.query == "a%20a=1%201+b%20b=2%202")
        assert(params["a a"] == "1 1 b b=2 2")

        http.get(HTTP + "/ejsProgram.ejs?a%20a=1%201&b%20b=2%202")
        let resp = deserialize(http.response)
        let params = resp.params
        assert(resp.query == "a%20a=1%201&b%20b=2%202")
        assert(params["a a"] == "1 1")
        assert(params["b b"] == "2 2")

        http.get(HTTP + "/ejsProgram.ejs?a|b+c>d+e?f+g>h+i'j+k\"l+m%20n=1234")
        let resp = deserialize(http.response)
        assert(resp.params["a|b c>d e?f g>h i'j k\"l m n"] == 1234)
        assert(resp.query == "a|b+c>d+e?f+g>h+i\'j+k\"l+m%20n=1234")
    }

    basic()
    forms()
    /* MOB - not using aliases
        alias()
    */
    query()
    encoding()
    status()
    location()
    quoting()

} else {
    test.skip("Ejscript not enabled")
}
