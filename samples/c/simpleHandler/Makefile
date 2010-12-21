#
#	Makefile for the simpleHandler sample
#
#	Copyright (c) Embedthis Software, 2003-2011. All Rights Reserved.
#

BUILD_NATIVE	:= 0
EXPORT_OBJECTS	:= 0
NAME			:= simpleHandler

include 		.makedep

TARGETS			+= $(NAME)$(BLD_EXE)

compileExtra: 	$(NAME)$(BLD_SHOBJ)

$(NAME)$(BLD_SHOBJ): $(NAME)$(BLD_OBJ)
	@bld --shared --library $(NAME) --libs "$(BLD_APPWEB_LIBS)" $(OBJECTS)

projectsExtra:
	@makeVsProj --project $(NAME).vcproj --library $(NAME) --libs "$(BLD_APPWEB_LIBS)" *.c

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4
#
