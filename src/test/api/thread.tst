/*
    thread.tst - Multithreaded test of the Appweb
 */

/*
    MOB - not working reliably

let command = Cmd.locate("testAppweb").portable + " --host " + App.config.uris.http + " --name mpr.api.c --iterations 5 " + 
    test.mapVerbosity(-2)

for each (threadCount in [2, 4, 8, 16]) {
    print(command + "--threads " + threadCount)
    Cmd.sh(command + "--threads " + threadCount)
}
*/
