/*
    thread.tst - Multithreaded test of the Appweb
 */

if (test.multithread) {
    let command = Cmd.locate("testAppweb") + " --host " + tsession["host"] + " --name mpr.api.c --iterations 5 " + 
        test.mapVerbosity(-2)

    for each (threadCount in [2, 4, 8, 16]) {
        sh(command + "--threads " + threadCount)
    }
} else {
    test.skip("Run if multithreaded")
}
