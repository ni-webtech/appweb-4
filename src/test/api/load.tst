/*
 *  load.tst - Load tests
 */

if (false && test.depth >= 3) {
    let command = locate("testAppweb") + " --host " + tsession["host"] + " --name mpr.api.c --iterations 400 " + 
        test.mapVerbosity(-2)

    testCmd(command)
    if (test.multithread) {
        sh(command + "--threads " + 2)
    }
    if (test.multithread) {
        for each (count in [2, 4, 8, 16]) {
            sh(command + "--threads " + count)
        }
    }
} else {
    test.skip("Runs at depth 3")
}
