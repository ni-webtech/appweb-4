/*
    ranges.tst - Ranged get tests
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http


//  Get first 5 bytes
http.setHeader("Range", "bytes=0-4")
http.get(HTTP + "/big.txt")
assert(http.status == 206)
assert(http.response == "01234")
http.close()


//  Get last 5 bytes
http.setHeader("Range", "bytes=-5")
http.get(HTTP + "/big.txt")
assert(http.status == 206)
assert(http.response.trim() == "MENT")
http.close()


//  Get from specific position till the end
http.setHeader("Range", "bytes=117000-")
http.get(HTTP + "/big.txt")
assert(http.status == 206)
assert(http.response.trim() == "END OF DOCUMENT")
http.close()


//  Multiple ranges
http.setHeader("Range", "bytes=0-5,25-30,-5")
http.get(HTTP + "/big.txt")
assert(http.status == 206)
assert(http.response.contains("Content-Range: bytes 0-5/117016"))
assert(http.response.contains("Content-Range: bytes 25-30/117016"))
assert(http.response.contains("Content-Range: bytes 117011-117015/117016"))
assert(http.response.contains("012345"))
assert(http.response.contains("567890"))
assert(http.response.contains("MENT"))
http.close()
