#
#	Makefile -- Man page makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include		.makedep

compileExtra: \
	ejsc.txt  ejsweb.txt  ejs.txt  ejsmod.txt  ejsvm.txt  http.txt\
	ejsc.html ejsweb.html ejs.html ejsmod.html ejsvm.html http.html \
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
