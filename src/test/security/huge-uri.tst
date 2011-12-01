/*
    Very large URI test
 */ 
const HTTP = (global.tsession && tsession["http"]) || ":4100"
const port: Number = (global.tsession && tsession["port"]) || "4100"

let base = ""
for (i in 100) {
    base += "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678\n"
}
let uri = ""
for (i in 100) {
    uri += base
}

let s = new Socket
s.connect(port)
s.write("GET " + uri + "HTTP/1.1\r\n\r\n")
response = new ByteArray
while ((n = s.read(response, -1)) > 0) { }
assert(response.toString().contains("HTTP/1.1 406 Not Acceptable"))
s.close()

//  Check server still up
http = new Http
http.get(HTTP + "/index.html")
assert(http.status == 200)
http.close()
