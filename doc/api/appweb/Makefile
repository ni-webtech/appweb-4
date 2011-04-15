#
#	Makefile -- Appweb API Documentation
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

STAGING		:= staging

include		.makedep

gen:
	make -C ../../../package/dist dist
	rm -fr ./$(STAGING) xml html
	mkdir -p ./$(STAGING) xml html
	tar xf $(BLD_OUT_DIR)/releases/$(BLD_PRODUCT)-dist.tgz -C $(STAGING)
	mv $(STAGING)/appweb-$(BLD_VERSION)/* $(STAGING)
	rmdir $(STAGING)/appweb-$(BLD_VERSION)
	$(call log) "[Generate]" "doxygen appweb.spec"
	doxygen appweb.spec
	$(call log) "[Transform]" "gendoc"
	ejs $(BLD_TOOLS_DIR)/gendoc.es --bare --title "Appweb Native API" --tags ../appweb.tags \
		xml/appweb_8h.xml xml/group* xml/struct_*.xml >../appwebBare.html
	ejs $(BLD_TOOLS_DIR)/gendoc.es --title "Appweb Native API" --tags ../appweb.tags \
		xml/appweb_8h.xml xml/group* xml/struct_*.xml >../appweb.html

cleanExtra:
	rm -rf xml html ../appweb.tags

clobberExtra:
	rm -f ../appweb.html ../appwebBare.html ../ejs.html ../mpr.html

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
