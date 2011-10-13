#
#	Makefile for the simpleHandler sample
#
#	Copyright (c) Embedthis Software, 2003-2011. All Rights Reserved.
#

BUILD_NATIVE	:= 0
NAME			:= simpleHandler
BLD_CXX			:= g++
COMPILE			+= *.cpp

include 		.makedep

TARGETS			+= $(BLD_LIB_DIR)/$(NAME)$(BLD_SHOBJ)

compileExtra: 	$(TARGETS)

$(BLD_LIB_DIR)/$(NAME)$(BLD_SHOBJ): $(BLD_OBJ_DIR)/$(NAME)$(BLD_OBJ)
	@bld --shared --library $(NAME) --libs "$(BLD_APPWEB_LIBS)" $(OBJECTS)

projectsExtra:
	@makeVsProj --project $(NAME).vcproj --library $(NAME) --libs "$(BLD_APPWEB_LIBS)" *.c

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#

