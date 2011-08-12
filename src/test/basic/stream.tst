/*
    stream.tst - Http tests using streams
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

if (!test || test.config["ejscript"] == 1) {
    http.get(HTTP + "/big.ejs")
    ts = new TextStream(http)
    lines = ts.readLines()
    assert(lines.length == 801)
    assert(lines[0].contains("aaaaabbb") && lines[0].contains("00000"))
    assert(lines[799].contains("aaaaabbb") && lines[799].contains("00799"))

} else {
    test.skip("Ejscript not enabled")
}

//TODO more 
