#!/bin/sh

echo "HTTP/1.0 200 OK"
echo "Content-type: text/html"
echo ""
echo "<html>"
echo "</html>"
echo "<html><body>"

cat big.txt big.txt big.txt big.txt
echo "</body></html>"
