#
# 	Makefile for the Appweb server 
#
# 	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#

POST_DIRS			:= package

include 			.makedep 

PRE_DIRS			+= $(BLD_OS) test

#
#	Extra libraries
#
ifeq	($(BLD_OS),WIN)
	SYSLIBS			+= shell32.lib
	RES				= --resources appweb.rc
endif
ifeq	($(BLD_FEATURE_AUTH_PAM),1)
	LIBS			+= pam
endif

LIBDEP				+= appweb
VLIBDEP				+= mpr
ifeq	($(BLD_FEATURE_SSL),1)
	VLIBDEP			+= mprssl
endif

TARGETS				+= $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE)

compileExtra: $(TARGETS) $(EXTRA_TARGETS)

compileFirst: romFiles.c

ifeq ($(BLD_FEATURE_ROMFS),1)
romFiles.c: *.db mime.types appweb.conf web/*.html
	$(call log) "[Generate]" "romFiles.c"
	echo -e ".\nappweb.conf\nmime.types\nusers.db\ngroups.db" >rom.files
	find conf -print | egrep -v 'ejs.conf|php.conf|fcgi.conf|ssl|dummy' >>rom.files
	find web -print | egrep -v 'CVS|\.svn|\.cvsignore|\.ignore|\.tmp|\.sav|\.php|\.ejs|php' >>rom.files
	$(call setlibpath) ; sort rom.files | uniq | xargs makerom --name romFiles >romFiles.c
else
romFiles.c: 
	echo . >rom.files
	$(call setlibpath) ; makerom --name romFiles . >romFiles.c
endif

#
#	Build the appweb program
#
$(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE): $(OBJECTS) $(call lib, $(LIBDEP)) $(call vlib $(VLIBDEP))
	bld --exe $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE) --search "$(BLD_APPWEB_LIBPATHS)" \
		--libs "$(BLD_APPWEB_LIBS) $(LIBS)" --syslibs "$(SYSLIBS)" $(RES) $(OBJECTS) $(EXTRA_OBJECTS)

run:
	$(call setlibpath) ; sudo $(BLD_BIN_DIR)/$(BLD_PRODUCT) -v

cleanExtra:
	rm -f $(TARGETS) $(BLD_PRODUCT)$(BLD_EXE) $(EXTRA_TARGETS)
	rm -f trace.txt access.log error.log leak.log
	rm -f *.obj *.pdb
	rm -f error.log* access.log*
	rm -fr target
	rm -f web/*.mod web/*.es

vxSetup:
	mkdir -p target
	subst v: c:\usr\svn\appweb\trunk\appweb\target

size:
	@echo -e "    #\n    #	Size of $(BLD_PRODUCT)\n    #"
	size $(BLD_PRODUCT) | sed -e 's/^/    # /'

curl:
	curl --trace trace.tmp --dump-header header.tmp https://127.0.0.1:4443/index.html

val:
	(valgrind -v --tool=memcheck --suppressions=d.supp --leak-check=yes --num-callers=12 --leak-resolution=high \
		--show-reachable=yes --track-fds=yes ../out/bin/appweb -v 2>&1) | tee val.out

mem:
	echo -e "\nImage and code sizes:"
	ls -hs  $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE)
	size $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE)
	pid=`ps -ef | grep 'appweb.*conf' | grep -v grep | awk '{ print \$$2 }'` ; \
	echo -e "\nMemory usage for $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE), pid $$pid:" ; \
	cat /proc/$$pid/status | egrep 'VmSize|VmPeak|VmRSS|VmData|VmStk|VmExe'

dependExtra:
	mkdir -p logs
	find web -name .DS_Store | xargs rm -f

#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
