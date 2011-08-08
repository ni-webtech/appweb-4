/*
    dump.tst - ESP dump variables test
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//  Basic get. Validate response code and contents
http.get(HTTP + "/dump.esp")
assert(http.status == 200)
let r = http.response
assert(r.contains("HEADER user-agent=Embedthis-http/"))
assert(r.contains("HEADER date="))
assert(r.contains("HEADER host="))

assert(r.contains("FORM REMOTE_ADDR="))
assert(r.contains("FORM DOCUMENT_ROOT="))
assert(r.contains("FORM REQUEST_TRANSPORT=http"))
assert(r.contains("FORM AUTH_GROUP=null"))
assert(r.contains("FORM AUTH_USER=null"))
assert(r.contains("FORM AUTH_TYPE=null"))
assert(r.contains("FORM SERVER_ROOT="))
assert(r.contains("FORM CONTENT_TYPE=null"))
assert(r.contains("FORM SERVER_SOFTWARE=Embedthis-http/"))
assert(r.contains("FORM SERVER_PROTOCOL=HTTP/1.1"))
assert(r.contains("FORM SERVER_PORT="))
assert(r.contains("FORM SCRIPT_NAME=/dump.esp"))
assert(r.contains("FORM PATH_INFO="))
assert(r.contains("FORM REMOTE_PORT="))
assert(r.contains("FORM REMOTE_USER=null"))
assert(r.contains("FORM SERVER_ADDR="))
assert(r.contains("FORM REQUEST_METHOD=GET"))
assert(r.contains("FORM GATEWAY_INTERFACE=CGI/1.1"))
assert(r.contains("FORM QUERY_STRING=null"))
assert(r.contains("FORM CONTENT_LENGTH=null"))
assert(r.contains("FORM SERVER_NAME="))

//  MOB -- should have way of detecting null
http.close()
