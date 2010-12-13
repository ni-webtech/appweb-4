#
#	Makefile -- Documentation makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include		.makedep

PRE_DIRS	+= api product

gen: 
	touch api/Makefile product/Makefile
	make -C api gen
	make -C product gen

patch: 
	@echo -e "\n  # Perform DSI patching\n"
	for f in dsi/head.html api/*Header.tem ; do \
		cp $$f /tmp/dsi$$.tmp ; \
		sed -e "s^<title>.*</title>^<title>$(BLD_NAME) $(BLD_VERSION) Documentation</title>^g;" < /tmp/dsi$$.tmp | \
		sed -e "s^TITLE-->.*^TITLE-->$(BLD_NAME) $(BLD_VERSION)^g;" > $$f ; \
		rm -f /tmp/dsi$$.tmp ; \
	done
	( find api -name '*.html' ; find guide licenses product ref man -name '*.html' ) | egrep -v 'Bare.html' | dsi -v -I.

tidy:
	chmod +w *.html
	find . -name '*.html' -print | egrep -v 'dsi|old|sav|archive|/html/|Bare' | \
	while read f ; do \
		echo tidy -q -config tidy.conf "$$f" ; \
		tidy -q -config tidy.conf "$$f" ; \
	done
