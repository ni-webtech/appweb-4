/*
    thread.tst - Multithreaded test of the Appweb
 */

let command = Cmd.locate("testAppweb").portable + " --host " + tsession["host"] + " --name mpr.api.c --iterations 5 " + 
    test.mapVerbosity(-2)

for each (threadCount in [2, 4, 8, 16]) {
    Cmd.sh(command + "--threads " + threadCount)
}
