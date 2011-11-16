#
#	Makefile -- Man page makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include		.makedep

compileExtra: \
	appweb.txt  appman.txt  appwebMonitor.txt  http.txt  makerom.txt  auth.txt \
	appweb.html appman.html appwebMonitor.html http.html makerom.html auth.html \
	esp.txt  \
	esp.html \
	regexp.txt \
	regexp.html

%.txt: %.1
	man ./$*.1 | sed 's/.\//g' >$*.txt

%.html: %.1
	man ./$*.1 | man2html -u >$*.html

%.txt: %.3
	man ./$*.3 | sed 's/.\//g' >$*.txt

%.html: %.3
	man ./$*.3 | man2html -u >$*.html

cleanExtra:
	rm -f *.txt *.html

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
