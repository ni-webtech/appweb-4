#!/bin/sh

echo "HTTP/1.0 200 OK"
echo "Content-type: text/plain"
echo ""

# Use the PID as a unique time value

echo "{ time: " $$ " }"
