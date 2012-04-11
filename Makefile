#
#   Makefile - Top level Makefile when using "make" to build.
#              Alternatively, use bit directly.
#

ARCH 	:= $(shell uname -m)
PROFILE	:= debug
EXT		:= mk
MAKE	:= make
UNAME 	:= $(shell uname)

ifeq ($(UNAME),Darwin)
	OS	:=	macosx
endif
ifeq ($(UNAME),Linux)
	OS	:=	linux
endif
ifeq ($(UNAME),Solaris)
	OS	:=	SOLARIS
endif
ifeq ($(UNAME),Cygwin)
	OS	:=	win
	MAKE:= nmake
	EXT	:= nmake
endif

all clean clobber compile:
	$(MAKE) -f projects/$(OS)-$(ARCH)-$(PROFILE).$(EXT) $@

build configure generate test package:
	bit $@

version:
	@bit -q version
