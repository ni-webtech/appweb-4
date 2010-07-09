#
#	Makefile -- Appweb API Documentation
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

STAGING		:= staging
include		.makedep

gen:
	make -C ../../../package/all combo
	rm -fr ./$(STAGING) xml html
	mkdir -p ./$(STAGING) xml html
	tar xf ../../../releases/$(BLD_PRODUCT)-all.tgz -C $(STAGING)
	$(call log) "[Generate]" "doxygen appweb.spec"
	doxygen appweb.spec
	ejs $(BLD_TOOLS_DIR)/gendoc.es --bare --title "Appweb Native API" --tags ../appweb.tags \
		xml/appweb_8h.xml xml/group* xml/struct_*.xml >../appwebBare.html
	ejs $(BLD_TOOLS_DIR)/gendoc.es --title "Appweb Native API" --tags ../appweb.tags \
		xml/appweb_8h.xml xml/group* xml/struct_*.xml >../appweb.html

cleanExtra:
	rm -rf xml html ../appweb.tags

clobberExtra:
	rm -f ../appweb.html ../appwebBare.html ../ejs.html ../mpr.html
