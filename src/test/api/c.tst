/*
    c.tst - Test the Appweb C API
 */

dump(App.config.test)
dump("TSESSION", tsession)
let command = locate("testAppweb") + " --host " + tsession["host"] + " --name appweb.api.c " + test.mapVerbosity(-3)
print(command)
testCmdNoCapture(command)
