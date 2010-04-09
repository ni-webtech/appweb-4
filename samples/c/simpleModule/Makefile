#
#	Makefile for the simpleModule sample
#
#	Copyright (c) Embedthis Software, 2003-2010. All Rights Reserved.
#

NAME			:= simpleModule

include 		.makedep

TARGETS			+= $(NAME)$(BLD_EXE)

compileExtra: 	$(NAME)$(BLD_LIB)

$(NAME)$(BLD_LIB): $(OBJECTS)
	@bld --shared --library $(NAME) --libs "$(BLD_APPWEB_LIBS)" $(OBJECTS)

projectsExtra:
	@makeVsProj --project $(NAME).vcproj --library $(NAME) --libs "$(BLD_APPWEB_LIBS)" *.c
