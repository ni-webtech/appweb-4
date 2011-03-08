#
#	Makefile for the spyFilter sample
#
#	Copyright (c) Embedthis Software, 2003-2011. All Rights Reserved.
#

NAME			:= mod_spy

include 		.makedep

TARGETS			+= $(NAME)$(BLD_EXE)

compileExtra: 	$(NAME)$(BLD_LIB)

$(NAME)$(BLD_LIB): $(OBJECTS)
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
