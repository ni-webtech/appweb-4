#
#	Makefile for the simpleServer sample
#
#	Copyright (c) Embedthis Software, 2003-2011. All Rights Reserved.
#

NAME			:= simpleServer

include 		.makedep

TARGETS			+= $(BLD_BIN_DIR)/$(NAME)$(BLD_EXE)

compileExtra: 	$(TARGETS)

$(BLD_BIN_DIR)/$(NAME)$(BLD_EXE): $(OBJECTS)
	@bld --exe $(BLD_BIN_DIR)/$(NAME)$(BLD_EXE) --libs "$(BLD_APPWEB_LIBS)" $(OBJECTS)

run:
	./$(NAME)$(BLD_EXE)

projectsExtra:
	@makeVsProj --project $(NAME).vcproj --exe $(NAME).exe --libs "$(BLD_APPWEB_LIBS)" *.c

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
