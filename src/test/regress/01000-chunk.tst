/*
    01000-chunk.tst - This tests uploading 3 files using chunked encoding as well.
    Was failing due to the trailing "\r\n" in the upload content
 */

let nc
try { nc = Cmd.sh("which nc"); } catch {}

if (!global.test || (test.depth > 0 && nc && Config.OS != "WIN" && test.config["ejscript"] == 1)) {
    const HTTP = (global.tsession && tsession["http"]) || ":4100"
    const PORT = (global.tsession && tsession["port"]) || "4100"
    Cmd.sh("cat 01000-chunk.dat | nc 127.0.0.1 " + PORT);

    Cmd.sh("cc -o tcp tcp.c")
    if (Config.OS == "WIN") {
        Cmd.sh("./tcp.exe 127.0.0.1 " + PORT + " 01000-chunk.dat")
    } else {
        Cmd.sh("./tcp 127.0.0.1 " + PORT + " 01000-chunk.dat")
    }

} else {
    test.skip("Test requires ejscript, nc and depth >= 1")
}
