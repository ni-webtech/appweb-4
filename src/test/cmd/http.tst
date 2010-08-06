/*
    http.tst - Test the http command
 */

if (test.depth > 1) {

    const HTTP = (global.tsession && tsession["http"]) || ":4100"

    let command = "/usr/bin/env http --host " + HTTP + " "
    if (test.verbosity > 2) {
        command += "-v "
    }

    function run(args): String {
        test.log(2, "[TestRun]", command + args)
        try {
            result = Cmd.run(command + args)
            assert(true)
        } catch (e) {
            assert(false, e)
        }
        return result
    }

    //  Empty get
    data = run("/empty.html")
    assert(data == "")

    //  Basic get
    data = run("/numbers.txt")
    assert(data.startsWith("012345678"))
    assert(data.endsWith("END"))

    //  Chunked get
    data = run("--chunk 256 /big.txt")
    assert(data.startsWith("012345678"))
    assert(data.endsWith("END"))

    //  Chunked empty get
    data = run("--chunk 100 /empty.html")
    assert(data == "")

    //  Multiple requests to test keep-alive
    run("-i 300 /index.html")

    //  Multiple requests to test keep-alive
    run("--chunk 100 -i 300 /index.html")

    //  HTTP/1.0
    run("--http 0 /index.html")
    run("-i 10 --http 0 /index.html")

    //  Explicit HTTP/1.1
    run("--http 1 /index.html")
    run("-i 20 --http 0 /index.html")
    run("-i 20 --http 1 /index.html")

    //  Auth
    //  TODO - should test failure
    run("--user 'joshua:pass1' /basic/basic.html")
    run("--user 'joshua' --password 'pass1' /basic/basic.html")

    //  Form data
    data = run("--form 'name=John+Smith&address=300+Park+Avenue' /form.ejs")
    assert(data.contains('"address": "300 Park Avenue"'))
    assert(data.contains('"name": "John Smith"'))

    //  Validate that header appears
    data = run("--showHeaders --header 'custom: MyHeader' /index.html")
    assert(data.contains('Content-Type'))

    //  PUT file
    run("cmd/test.dat /tmp/day.tmp")
    if (test.threads == 1) {
        assert(exists("web/tmp/day.tmp"))
    }

    run("basic/*.tst /tmp/")
    if (test.threads == 1) {
        assert(exists("web/tmp/methods.tst"))
    }

    //  DELETE
    run("cmd/test.dat /tmp/test.dat")
    assert(exists("web/tmp/test.dat"))
    run("--method DELETE /tmp/test.dat")
    assert(!exists("web/tmp/test.dat"))

    //  Options/Trace
    run("--method OPTIONS /index.html")
    assert(run("--showCode -q --method TRACE /index.html") == "406")

    //  Upload
    data = run("--upload basic/*.tst /upload.ejs")
    assert(data.contains('"clientFilename": "methods.tst"'))
    if (test.threads == 1) {
        assert(exists("web/tmp/methods.tst"))
    }

    data = run("--upload --form 'name=John+Smith&address=300+Park+Avenue' basic/*.tst /upload.ejs")
    assert(data.contains('"address": "300 Park Avenue"'))
    assert(data.contains('"clientFilename": "methods.tst"'))

    data = run("--cookie 'test-id=12341234; $domain=site.com; $path=/dir/' /form.ejs")
    assert(data.contains('"test-id": '))
    assert(data.contains('"name": "test-id",'))
    assert(data.contains('"domain": "site.com",'))
    assert(data.contains('"path": "/dir/",'))

    //  Ranges
    assert(run("--range 0-4 /numbers.html") == "01234")
    assert(run("--range -5 /numbers.html") == "5678")

    //  Load test
    if (test.depth > 2) {
        run("-i 2000 /index.html")
        run("-i 2000 /big.txt")
    }

} else {
    test.skip("Test runs at depth 2")
}
