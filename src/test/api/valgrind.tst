/*
    valgrind.tst - Valgrind tests on Unix-like systems
 */

if (false && test.os == "LINUX") {

    let command = Cmd.locate("testAppweb") + " --host " + host["host"] + " --name mpr.api.valgrind --iterations 5 "
    let valgrind = "/usr/bin/env valgrind -q --tool=memcheck --suppressions=mpr.supp " + command + test.mapVerbosity(-2)

    if (test.depth >= 2) {
        testCmdNoCapture(command)
        if (test.multithread) {
            testCmdNoCapture(valgrind + " --threads " + 8)
        }
    }

} else {
    test.skip("Run on Linux")
}
