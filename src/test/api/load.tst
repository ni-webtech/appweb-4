/*
    load.tst - Load tests
 */

const HOST = App.config.uris.http || "127.0.0.1:4100"

if (App.test.depth >= 4) {
    let command = Cmd.locate("testAppweb").portable + " --host " + HOST + 
        " --name mpr.api.c --iterations 400 " + test.mapVerbosity(-2)

    Cmd.sh(command)
    for each (count in [2, 4, 8, 16]) {
        Cmd.sh(command + " --threads " + count)
    }
} else {
    test.skip("Runs at depth 4")
}
