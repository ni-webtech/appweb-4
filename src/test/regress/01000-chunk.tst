/*
    01000-chunk.tst - This tests uploading 3 files using chunked encoding as well.
    Was failing due to the trailing "\r\n" in the upload content
 */

let nc
try { nc = Cmd.sh("which nc"); } catch {}

if (App.test.depth > 0 && nc && Config.OS != "WIN" && App.config.bld_ejscript) {
    const HTTP = App.config.uris.http || "127.0.0.1:4100"
    let [ip,port] = HTTP.split(":")

    Cmd.sh("cat 01000-chunk.dat | nc " + ip + " " + port);
    Cmd.sh("cc -o tcp tcp.c")
    if (Config.OS == "WIN") {
        Cmd.sh("./tcp.exe " + ip + " " + port + " 01000-chunk.dat")
    } else {
        Cmd.sh("./tcp " + ip + " " + port + " 01000-chunk.dat")
    }

} else {
    test.skip("Test requires ejscript, nc and depth >= 1")
}
