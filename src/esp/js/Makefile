#
#	Makefile for the esp/js
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include 	.makedep

DEST		:= ../www/files/static/js
TARGETS		+= $(DEST)/jquery.min.js
TARGETS		+= $(DEST)/jquery.esp.min.js
TARGETS		+= $(DEST)/jquery.simplemodal.min.js
TARGETS		+= $(DEST)/jquery.tablesorter.min.js

# TARGETS	+= $(DEST)/jquery.treeview.min.js

gen: $(TARGETS)

$(DEST)/jquery.esp.min.js: jquery.esp.js 
	$(call log) "[Generate]" "minify $(DEST)/jquery.esp.min.js"
	minify jquery.esp.js
	mv jquery.esp.min.js $(DEST)
	cp jquery.esp.js $(DEST)
	
$(DEST)/jquery.simplemodal.min.js: jquery.simplemodal.js 
	$(call log) "[Generate]" "minify $(DEST)/jquery.simplemodal.min.js"
	minify jquery.simplemodal.js
	mv jquery.simplemodal.min.js $(DEST)
	cp jquery.simplemodal.js $(DEST)
	
$(DEST)/jquery.tablesorter.min.js: jquery.tablesorter.js 
	$(call log) "[Generate]" "minify $(DEST)/jquery.tablesorter.min.js"
	minify jquery.tablesorter.js
	mv jquery.tablesorter.min.js $(DEST)
	cp jquery.tablesorter.js $(DEST)
	
$(DEST)/jquery.treeview.min.js: jquery.treeview.js 
	$(call log) "[Generate]" "minify $(DEST)/jquery.treeview.min.js"
	minify jquery.treeview.js
	mv jquery.treeview.min.js $(DEST)
	cp jquery.treeview.js $(DEST)
	cp -r treeview.css tree-images $(DEST)
	
#
#	Don't generate jquery. Use distributed minified version.
#
$(DEST)/jquery.min.js: jquery.js 
	$(call log) "[Generate]" "minify jquery.min.js"
	cp jquery.js $(DEST)
	cp jquery.min.js $(DEST)
	
cleanExtra:
	rm -rf *.min.js

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
