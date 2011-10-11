#
#	Makefile -- ESP API Documentation
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

NAME		:= esp
STAGING		:= staging

include		.makedep

gen: dox next rest

dox:
	make -C ../../..
	make SKIP_CLEAN=1 -C ../../.. packageCombo
	rm -fr ./$(STAGING) xml html
	mkdir -p ./$(STAGING) xml html
	tar xf $(BLD_OUT_DIR)/releases/$(BLD_PRODUCT)-combo.tgz -C $(STAGING)
	mv $(STAGING)/appweb-$(BLD_VERSION)/* $(STAGING)
	rmdir $(STAGING)/appweb-$(BLD_VERSION)
	$(call log) "[Generate]" "doxygen $(NAME).spec"
	doxygen $(NAME).spec

next:
	$(call log) "[Transform]" "gendoc"
	ejs $(BLD_TOOLS_DIR)/gendoc.es --title "ESP - Native API" --tags ../mpr.tags --tags ../http.tags --tags ../appweb.tags \
		--out ../$(NAME).html xml/$(NAME)_8h.xml xml/edi_8h.xml xml/group* xml/struct_*.xml

rest:
	ejs $(BLD_TOOLS_DIR)/gendoc.es --bare --title "ESP - Native API" --tags ../mpr.tags --tags ../http.tags \
		--tags ../appweb.tags --out ../$(NAME)Bare.html xml/$(NAME)_8h.xml xml/edi_8h.xml xml/group* xml/struct_*.xml
	rm -fr ./$(STAGING) xml html

cleanExtra:
	rm -rf xml html

clobberExtra:
	rm -fr ./$(STAGING)

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
