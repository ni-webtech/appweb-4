/*
    c.tst - Test the Appweb C API
 */

const HOST = (global.tsession && tsession["host"]) || "http://127.0.0.1:4100"

let command = locate("testAppweb") + " --host " + HOST + " --name appweb.api.c " + test.mapVerbosity(-3)
sh(command)
