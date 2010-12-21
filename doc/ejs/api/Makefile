#
#	Makefile -- API Documentation makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include		.makedep

gen: prep mpr.html ejs.html

prep:
	rm -f mpr.html ejs.html 

mpr.html: Makefile mprBare.html *.tem
	rm -f mpr.html
	$(call log) "[Generate]" "mpr.html"
	cat mprHeader.tem >mpr.html
	cat mprBare.html apiFooter.tem >>mpr.html

ejs.html: Makefile ejsBare.html *.tem
	make -C ejs gen
	rm -f ejs.html
	$(call log) "[Generate]" "ejs.html"
	cat ejsHeader.tem >ejs.html
	cat ejsBare.html apiFooter.tem >>ejs.html

tidy:
	$(call log) "[Tidy]" "*.html"
	tidy -q -config tidy.conf *.html

dependExtra:
	chmod +w *.html

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
