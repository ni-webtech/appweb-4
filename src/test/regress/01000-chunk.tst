/*
    01000-chunk.tst - This tests uploading 3 files using chunked encoding as well.
    Was failing due to the trailing "\r\n" in the upload content
 */

let nc
try { nc = sh("which nc"); } catch {}

if (test.depth > 0 && nc && Config.OS != "WIN") {
    const HTTP = (global.tsession && tsession["http"]) || ":4100"
    const PORT = (global.tsession && tsession["port"]) || "4100"
    testCmd("cat 01000-chunk.dat | nc 127.0.0.1 " + PORT);

    sh("cc -o tcp tcp.c")
    if (Config.OS == "WIN") {
        testCmd("./tcp.exe 127.0.0.1 " + PORT + " 01000-chunk.dat")
    } else {
        testCmd("./tcp 127.0.0.1 " + PORT + " 01000-chunk.dat")
    }

} else {
    test.skip("Test requires nc and depth >= 1")
}
