/*
    http.tst - Test the http command
 */
if (test.depth >= 6) {

    const HTTP = (global.tsession && tsession["http"]) || ":4100"
    const ITER = 10000

    let command = Cmd.locate("http").portable + " --host " + HTTP + " "
    if (test.verbosity > 2) {
        command += "-v "
    }

    function run(args): String {
        // App.log.debug(0, "[TestRun]", command + args)
        try {
print("CMD: ", command + args)
            let cmd = Cmd(command + args)
if (cmd.status != 0) {
    print("NON-ZERO status")
    print(cmd.error)
    print(cmd.response)
}
            assert(cmd.status == 0)
            return cmd.response
        } catch (e) {
            assert(false, e)
        }
        return null
    }
    App.log.write("running\n")

    for each (threads in [2, 3, 4, 5, 6, 7, 8, 16]) {
        let start = new Date
        let count = (ITER / threads).toFixed()
        //  MOB - remove --zero
        run("--zero -q -i " + count + " -t " + threads + " " + HTTP + "/bench/bench.html")
        elapsed = start.elapsed
        App.log.activity("Benchmark", "Throughput %.0f request/sec, with %d threads" % [ITER / elapsed * 1000, threads])
    }
    App.log.write("%12s %s" % ["[Benchmark]", "finalizing ..."])

} else {
    test.skip("Test runs at depth 6")
}
