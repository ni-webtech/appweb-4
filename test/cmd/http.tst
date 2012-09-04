/*
    http.tst - Test the http command
 */
const HTTP = App.config.uris.http || "127.0.0.1:4100"

let command = Cmd.locate("http").portable + " --host " + HTTP + " "
if (test.verbosity > 2) {
    command += "-v "
}

function run(args): String {
    App.log.debug(5, "[TestRun]", command + args)
    try {
        let cmd = Cmd(command + args)
        if (cmd.status != 0) {
            //  MOB - temp for http
            if (cmd.status < 0 && Config.OS == 'windows') {
                assert(true)
                return cmd.response
            }
        }
        assert(cmd.status == 0) 
        assert(true)
        return cmd.response
    } catch (e) {
        assert(false, e)
    }
    return null
}

//  Empty get
data = run("/empty.html")
assert(data == "")

//  Basic get
data = run("/numbers.txt")
assert(data.startsWith("012345678"))
assert(data.trimEnd().endsWith("END"))

//  Chunked get
data = run("--chunk 256 /big.txt")
assert(data.startsWith("012345678"))
assert(data.trimEnd().endsWith("END OF DOCUMENT"))

//  Chunked empty get
data = run("--chunk 100 /empty.html")
assert(data == "")

//  Multiple requests to test keep-alive
run("-i 300 /index.html")

//  Multiple requests to test keep-alive
run("--chunk 100 -i 300 /index.html")

//  HTTP/1.0
run("--protocol HTTP/1.0 /index.html")
run("-i 10 --protocol HTTP/1.0 /index.html")

//  Explicit HTTP/1.1
run("--protocol HTTP/1.1 /index.html")
run("-i 20 --protocol HTTP/1.0 /index.html")
run("-i 20 --protocol HTTP/1.1 /index.html")

//  Auth
//  TODO - should test failure
run("--user 'joshua:pass1' /auth/basic/basic.html")
run("--user 'joshua' --password 'pass1' /auth/basic/basic.html")

if (App.config.bit_ejscript) {
    //  Form data
    data = run("--form 'name=John+Smith&address=300+Park+Avenue' /form.ejs")
    assert(data.contains('"address": "300 Park Avenue"'))
    assert(data.contains('"name": "John Smith"'))
}

//  PUT file
run("test.dat /tmp/day.tmp")
if (test.threads == 1) {
    assert(Path("../web/tmp/day.tmp").exists)
}

let files = Path(".").files().join(" ")
run(files + " /tmp/")
if (test.threads == 1) {
    assert(Path("../web/tmp/http.tst").exists)
}

//  DELETE
run("test.dat /tmp/test.dat")
assert(Path("../web/tmp/test.dat").exists)
run("--method DELETE /tmp/test.dat")
assert(!Path("../web/tmp/test.dat").exists)

//  Options with show status
run("--method OPTIONS /index.html")
data = run("--zero --showStatus -q --method TRACE /index.html")
assert(data.trim() == "406")

//  Show headers
data = run("--showHeaders /index.html")
assert(data.contains('Content-Type'))

//  Upload
if (App.config.bit_ejscript) {
    let files = Path(".").files().join(" ")
    data = run("--upload " + files + " /upload.ejs")
    assert(data.contains('"clientFilename": "http.tst"'))
    if (test.threads == 1) {
        assert(Path("../web/tmp/http.tst").exists)
    }

    let files = Path(".").files().join(" ")
    data = run("--upload --form 'name=John+Smith&address=300+Park+Avenue' " + files + " /upload.ejs")
    assert(data.contains('"address": "300 Park Avenue"'))
    assert(data.contains('"clientFilename": "http.tst"'))

    data = run("--cookie 'test-id=12341234; $domain=site.com; $path=/dir/' /form.ejs")

    assert(data.contains('"test-id": '))
    assert(data.contains('"domain": "site.com"'))
    assert(data.contains('"path": "/dir/"'))
}

//  Ranges
assert(run("--range 0-4 /numbers.html").trim() == "01234")
assert(run("--range -5 /numbers.html").trim() == "5678")

//  Load test
if (test.depth > 2) {
    run("-i 2000 /index.html")
    run("-i 2000 /big.txt")
}
//  Cleanup
for each (f in Path("../web/tmp").files()) {
    Path(f).remove()
}
