#!/home/mob/ejs/linux-x86-release/bin/ejs
print("HTTP/1.0 200 OK
Content-Type: text/plain

")
for (let [key,value] in App.env) print(key + "=" + value)
print()
