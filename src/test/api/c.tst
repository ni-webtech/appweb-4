/*
    c.tst - Test the Appweb C API
 */

const HOST = App.config.main || "127.0.0.1:4100"

let command = Cmd.locate("testAppweb").portable + " --host " + HOST + " --name appweb.api.c " + test.mapVerbosity(-3)
Cmd.sh(command)
