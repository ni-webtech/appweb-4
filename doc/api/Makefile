#
#	Makefile -- API Documentation makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

include		.makedep

gen: prep mpr.html ejs.html appweb.html

prep:
	rm -f prep mpr.html ejs.html appweb.html

mpr.html: Makefile mprBare.html *.tem
	rm -f mpr.html
	$(call log) "[Generate]" "mpr.html"
	cat mprHeader.tem >mpr.html
	cat mprBare.html apiFooter.tem >>mpr.html

ejs.html: Makefile ejsBare.html *.tem
	rm -f ejs.html
	$(call log) "[Generate]" "ejs.html"
	cat ejsHeader.tem >ejs.html
	cat ejsBare.html apiFooter.tem >>ejs.html

appweb.html: Makefile appwebBare.html *.tem
	make -C appweb gen
	rm -f appweb.html
	$(call log) "[Generate]" "appweb.html"
	cat appwebHeader.tem >appweb.html
	cat appwebBare.html apiFooter.tem >>appweb.html

tidy:
	$(call log) "[Tidy]" "*.html"
	tidy -q -config tidy.conf *.html

dependExtra:
	chmod +w *.html
