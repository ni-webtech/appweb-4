#
#	Makefile - Top level Makefile when using "make" to build.
#			   Alternatively, use bit directly.
#	

ARCH 	:= $(shell uname -m)
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
	OS	:=	SOLARIS
endif
ifeq ($(UNAME),CYGWIN_NT-5.1)
	OS	:= win
	MAKE:= nmake
	EXT := nmake
endif

all clean clobber compile:
	$(MAKE) -f projects/appweb-$(OS)-$(ARCH).$(EXT) $@

build configure generate test package:
	bit $@

version:
	@bit -q version
