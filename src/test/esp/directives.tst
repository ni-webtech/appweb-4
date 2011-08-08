/*
    directives.tst - Test various ESP directives
 */

const HTTP = (global.tsession && tsession["http"]) || ":4100"
let http: Http = new Http

//	Output should look like:
//
//	<html>
//	<head>
//	    <title>ESP Directives</title>
//	</head>
//	<body>
//	    <h1>ESP Directives</h1>
//	    Today's Message: Hello World
//	    Luck Number: 42
//	    Hostname: local.magnetar.local
//	    Time: Wed Jul 27 2011 09:53:29 GMT-0700 (PDT)
//	    Formatted number: 12,345,678
//	    Safe Strings: &lt;html&gt;
//	</body>
//	</html>

http.get(HTTP + "/directives.esp?weather=sunny&exploit=<html>")
assert(http.status == 200)
let r = http.response
assert(r.contains("ESP Directives"))
assert(r.contains("ESP Directives"))
assert(r.contains("Today's Message: Hello World"))
assert(r.contains("Lucky Number: 42"))
assert(r.contains("Formatted Number: 12,345,678"))
assert(r.contains("Safe Strings: &lt;bold&gt;"))
assert(r.contains("Safe Variables: &lt;html&gt;"))
assert(r.contains("Weather: sunny"))
http.close()
