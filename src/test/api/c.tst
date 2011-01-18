/*
    c.tst - Test the Appweb C API
 */

let command = locate("testAppweb") + " --host " + tsession["host"] + " --name appweb.api.c " + test.mapVerbosity(-3)
sh(command)
