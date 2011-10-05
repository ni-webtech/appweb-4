#
#	Makefile -- API Documentation makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include		.makedep

PRE_DIRS	= appweb esp

gen: mpr.html http.html appweb.html esp.html

mpr.html: Makefile mprBare.html *.tem
	rm -f mpr.html
	$(call log) "[Generate]" "mpr.html"
	cat mprHeader.tem >mpr.html
	cat mprBare.html apiFooter.tem >>mpr.html

http.html: Makefile httpBare.html *.tem
	rm -f http.html
	$(call log) "[Generate]" "http.html"
	cat httpHeader.tem >http.html
	cat httpBare.html apiFooter.tem >>http.html

appweb.html: Makefile appwebBare.html *.tem
	make -C appweb gen
	rm -f appweb.html
	$(call log) "[Generate]" "appweb.html"
	cat appwebHeader.tem >appweb.html
	cat appwebBare.html apiFooter.tem >>appweb.html

esp.html: Makefile espBare.html *.tem
	make -C esp gen
	rm -f esp.html
	$(call log) "[Generate]" "esp.html"
	cat espHeader.tem >esp.html
	cat espBare.html apiFooter.tem >>esp.html

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
