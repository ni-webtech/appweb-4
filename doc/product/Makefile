#
#	Makefile -- Documentation makefile
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

include		.makedep

gen: compileExtra

compileExtra: index.html

index.html: $(BLD_INC_DIR)/buildConfig.h Makefile
	cp version.html version.new
	sed 's/<p>.*<\/p>/<p>$(BLD_VERSION)<\/p>/' < version.new >version.html
	cp index.html index.new
	sed 's/Appweb Documentation.*</Appweb Documentation $(BLD_VERSION)</' < index.new >index.html
	rm -f index.new version.new

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4
#
