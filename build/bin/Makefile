#
#	Makefile -- Build bin directory
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

BUILD_CROSS	:= 0
PRE_DIRS	:= src

include .makedep

dependExtra:
	chmod +x *
	chmod -x Makefile
	rm -f dsi* edep* getpath*

cleanExtra:
	rm -f *.pdb *.exe
