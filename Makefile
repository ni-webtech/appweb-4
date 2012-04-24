#
#	Makefile - Top level Makefile when using "make" to build.
#			   Alternatively, use bit directly.
#	

MAKE	:= make
EXT		:= mk
UNAME 	:= $(shell uname)

ifeq ($(UNAME),Darwin)
	OS	:=	macosx
endif
ifeq ($(UNAME),Linux)
	OS	:=	linux
endif
ifeq ($(UNAME),Solaris)
	OS	:=	solaris
endif
ifeq ($(UNAME),CYGWIN_NT-5.1)
	OS	:= win
	MAKE:= nmake
	EXT := nmake
endif

all clean clobber compile:
	$(MAKE) -f projects/*-$(OS).$(EXT) $@

build configure generate test package:
	bit $@

version:
	@bit -q version
