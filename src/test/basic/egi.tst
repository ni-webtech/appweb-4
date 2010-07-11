/*
    egi.tst - EGI tests
 */

const HTTP = (global.session && session["http"]) || ":4100"
let http: Http = new Http

/* Suport routines */

if (test && test.config["debug"] == 1 && test.config["static"] == 0) {
    function contains(pat): Void {
        assert(http.response.contains(pat))
    }

    function keyword(pat: String): String {
        pat.replace(/\//, "\\/").replace(/\[/, "\\[")
        let reg = RegExp(".*" + pat + "=([^<]*).*", "s")
        return http.response.replace(reg, "$1")
    }

    function match(key: String, value: String): Void {
        assert(keyword(key) == value)
    }


    /* Tests */

    function basic() {
        //  Test various forms to invoke egi programs
        http.get(HTTP + "/egi/egiProgram")
        assert(http.status == 200)
        contains("egiProgram: EGI Output")
    }

    function forms() {
        //  Test various forms to invoke egi programs
        http.get(HTTP + "/egi/egiProgram")
        assert(http.status == 200)
        contains("egiProgram: EGI Output")

        http.get(HTTP + "/egiProgram.egi")
        assert(http.status == 200)
        contains("egiProgram: EGI Output")
    }

    function alias() {
        http.get(HTTP + "/MyInProcScripts/egiProgram.egi")
        assert(http.status == 200)
        contains("egiProgram: EGI Output")
        match("SCRIPT_NAME", "/MyInProcScripts/egiProgram.egi")
        match("QUERY_STRING", "")
    }

    function query() {
        http.get(HTTP + "/egiProgram.egi?a=b&c=d&e=f")
        match("SCRIPT_NAME", "/egiProgram.egi")
        contains("QVAR a=b")
        contains("QVAR c=d")

        //
        //  Query string vars should not be turned into variables for GETs
        //
        http.get(HTTP + "/egi/egiProgram?var1=a+a&var2=b%20b&var3=c")
        match("SCRIPT_NAME", "/egi/egiProgram")
        match("QUERY_STRING", "var1=a+a&var2=b%20b&var3=c")
        match("QVAR var1", "a a")
        match("QVAR var2", "b b")
        match("QVAR var3", "c")

        //
        //  Post data should be turned into variables
        //
        http.form(HTTP + "/egi/egiProgram?var1=a+a&var2=b%20b&var3=c", 
            { name: "Peter", address: "777 Mulberry Lane" })
        match("QUERY_STRING", "var1=a+a&var2=b%20b&var3=c")
        match("QVAR var1", "a a")
        match("QVAR var2", "b b")
        match("QVAR var3", "c")
        match("PVAR name", "Peter")
        match("PVAR address", "777 Mulberry Lane")
    }

    function encoding() {
        http.get(HTTP + "/egi/egiProgram.egi?var%201=value%201")
        match("QUERY_STRING", "var%201=value%201")
        match("SCRIPT_NAME", "/egi/egiProgram.egi")
        match("QVAR var 1", "value 1")
    }

    function status() {
        http.get(HTTP + "/egi/egiProgram?SWITCHES=-s%20711")
        assert(http.status == 711)
    }

    function location() {
        let http = new Http
        http.followRedirects = false
        http.get(HTTP + "/egi/egiProgram?SWITCHES=-l%20http://www.redhat.com/")
        assert(http.status == 302)
        http.close()
    }

    function quoting() {
        http.get(HTTP + "/egi/egiProgram?a+b+c")
        match("QUERY_STRING", "a+b+c")
        match("QVAR a b c", "")

        http.get(HTTP + "/egi/egiProgram?a=1&b=2&c=3")
        match("QUERY_STRING", "a=1&b=2&c=3")
        match("QVAR a", "1")
        match("QVAR b", "2")
        match("QVAR c", "3")

        http.get(HTTP + "/egi/egiProgram?a%20a=1%201+b%20b=2%202")
        match("QUERY_STRING", "a%20a=1%201+b%20b=2%202")
        match("QVAR a a", "1 1 b b=2 2")

        http.get(HTTP + "/egi/egiProgram?a%20a=1%201&b%20b=2%202")
        match("QUERY_STRING", "a%20a=1%201&b%20b=2%202")
        match("QVAR a a", "1 1")
        match("QVAR b b", "2 2")

        http.get(HTTP + "/egi/egiProgram?a|b+c>d+e?f+g>h+i'j+k\"l+m%20n")
        contains("QVAR a|b c>d e?f g>h i'j k\"l m n=")
        match("QUERY_STRING", "a|b+c>d+e?f+g>h+i'j+k\"l+m%20n")
        assert(http.response.contains("QVAR a|b c>d e?f g>h i'j k\"l m n"))
    }

    basic()
    forms()
    alias()
    query()
    encoding()
    status()
    location()
    quoting()

} else {
    test.skip("Run only for debug shared builds")
}
