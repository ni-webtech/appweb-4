/*
 *  load.tst - Load tests
 */

if (false && test.depth >= 3) {
    let command = locate("testAppweb") + " --host " + session["host"] + " --name mpr.api.c --iterations 400 " + 
        test.mapVerbosity(-2)

    testCmd(command)
    if (test.multithread) {
        testCmdNoCapture(command + "--threads " + 2)
    }
    if (test.multithread) {
        for each (count in [2, 4, 8, 16]) {
            testCmdNoCapture(command + "--threads " + count)
        }
    }
} else {
    test.skip("Runs at depth 3")
}
