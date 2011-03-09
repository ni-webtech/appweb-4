/*
    upload.tst - File upload tests
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

/*
http.upload(HTTP + "/upload.ejs", { myfile: "test.dat"} )
assert(http.status == 200)
assert(http.response.contains('"clientFilename": "test.dat"'))
assert(http.response.contains('Uploaded'))
http.wait()
*/

http.upload(HTTP + "/upload.ejs", { myfile: "test.dat"}, {name: "John Smith", address: "100 Mayfair"} )
assert(http.status == 200)
print(http.response)
assert(http.response.contains('"clientFilename": "test.dat"'))
assert(http.response.contains('Uploaded'))
assert(http.response.contains('"address": "100 Mayfair"'))
