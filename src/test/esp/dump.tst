/*
    dump.tst - ESP dump variables test
 */

const HTTP = App.config.uris.http || "127.0.0.1:4100"
let http: Http = new Http

//  Basic get. Validate response code and contents
http.get(HTTP + "/dump.esp?a=b&c=d")
assert(http.status == 200)
let r = http.response
assert(r.contains("HEADER User-Agent=Embedthis-http/"))
assert(r.contains("HEADER Date="))
assert(r.contains("HEADER Host="))

assert(r.contains("SERVER REMOTE_ADDR="))
assert(r.contains("SERVER DOCUMENT_ROOT="))
assert(r.contains("SERVER REQUEST_TRANSPORT=http"))
assert(r.contains("SERVER AUTH_USER=null"))
assert(r.contains("SERVER AUTH_TYPE=null"))
assert(r.contains("SERVER SERVER_ROOT="))
assert(r.contains("SERVER CONTENT_TYPE=null"))
assert(r.contains("SERVER SERVER_SOFTWARE=Embedthis-http/"))
assert(r.contains("SERVER SERVER_PROTOCOL=HTTP/1.1"))
assert(r.contains("SERVER SERVER_PORT="))
assert(r.contains("SERVER SCRIPT_NAME=/dump.esp"))
assert(r.contains("SERVER PATH_INFO="))
assert(r.contains("SERVER REMOTE_PORT="))
assert(r.contains("SERVER REMOTE_USER=null"))
assert(r.contains("SERVER SERVER_ADDR="))
assert(r.contains("SERVER REQUEST_METHOD=GET"))
assert(r.contains("SERVER GATEWAY_INTERFACE=CGI/1.1"))
assert(r.contains("SERVER QUERY_STRING=a=b&c=d"))
assert(r.contains("SERVER CONTENT_LENGTH=null"))
assert(r.contains("SERVER SERVER_NAME="))

assert(r.contains("FORM a=b"))
assert(r.contains("FORM c=d"))

http.close()
