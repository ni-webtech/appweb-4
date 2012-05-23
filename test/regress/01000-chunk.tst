/*
    01000-chunk.tst - This tests uploading 3 files using chunked encoding as well.
    Was failing due to the trailing "\r\n" in the upload content
 */

let nc
try { nc = Cmd.sh("which nc"); } catch {}

if (App.test.depth > 0 && nc && Config.OS != "windows" && App.config.bld_ejscript) {
    const HTTP = Uri(App.config.uris.http || "127.0.0.1:4100")
    let ip = HTTP.host
    let port = HTTP.port

    Cmd.sh("cat 01000-chunk.dat | nc " + ip + " " + port);
    Cmd.sh("cc -o tcp tcp.c")
    if (Config.OS == "windows") {
        Cmd.sh("./tcp.exe " + ip + " " + port + " 01000-chunk.dat")
    } else {
        Cmd.sh("./tcp " + ip + " " + port + " 01000-chunk.dat")
    }

} else {
    test.skip("Test requires ejscript, nc and depth >= 1")
}
