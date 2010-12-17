#
# 	Makefile for the Appweb server 
#
# 	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
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

ifeq	($(BLD_FEATURE_STATIC),1)
	ifeq	($(BLD_FEATURE_EJS),1)
		VLIBDEP			+= ejs
	endif
endif

ifeq	($(BUILD_CROSS_OR_COMPLETE_NATIVE),1)
	TARGETS			+= $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE)
endif
ifeq	($(BLD_FEATURE_CGI),1)
ifeq	($(BUILD_NATIVE_OR_COMPLETE_CROSS),1)
	EXTRA_TARGETS 	+= cgi-bin/cgiProgram$(BLD_EXE)
endif
endif

ifeq ($(BLD_OS),VXWORKS)
ifeq ($(BLD_FEATURE_STATIC),0)
	# EXTRA_OBJECTS	+= $(patsubst %, $(BLD_LIB_DIR)/lib%.out, appweb)
	# EXTRA_OBJECTS	+= $(patsubst %, $(BLD_LIB_DIR)/lib%-$(BLD_VERSION).out, mpr pcre)
endif
endif
TARGETS				+= appweb.conf

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

#
#	Patch the appweb.conf configuration file based on the configure --config=dir setting
#
appweb.conf: $(BLD_INC_DIR)/buildConfig.h
	(dir=`pwd` ; cd $(BLD_FEATURE_CONFIG) ; find . -type f | \
		egrep -v 'CVS/|RCS/|SCCS/|.svn/' | cpio -pdum $$dir >/dev/null 2>&1)
	BLD_CFG_PREFIX=. BLD_WEB_PREFIX=./web BLD_MOD_PREFIX=$(BLD_MOD_DIR) BLD_DOC_PREFIX=../../doc \
	BLD_SERVER=`hostname` BLD_LOG_PREFIX=./logs BLD_HTTP_PORT=$(BLD_HTTP_PORT) BLD_SSL_PORT=$(BLD_SSL_PORT) \
	$(call log) "[Patch]" "appweb.conf"
	patchAppwebConf appweb.conf conf/hosts/ssl-default.conf conf/log.conf conf/doc.conf

cgi-bin/cgiProgram$(BLD_EXE): $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
	cp $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE) cgi-bin
	chmod +x cgi-bin/*

run:
	$(call setlibpath) ; $(BLD_BIN_DIR)/$(BLD_PRODUCT) --log stdout:2 --config appweb.conf

cleanExtra:
	rm -f $(TARGETS) $(BLD_PRODUCT)$(BLD_EXE)
	rm -f trace.txt access.log error.log leak.log
	rm -f *.obj *.pdb
	rm -f logs/*.log logs/*.old
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
		--show-reachable=yes --track-fds=yes ../bin/appweb -r . -f appweb.conf -l stdout:1  2>&1) | tee val.out

mem:
	echo -e "\nImage and code sizes:"
	ls -hs  $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE)
	size $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE)
	pid=`ps -ef | grep 'appweb.*conf' | grep -v grep | awk '{ print \$$2 }'` ; \
	echo -e "\nMemory usage for $(BLD_BIN_DIR)/$(BLD_PRODUCT)$(BLD_EXE), pid $$pid:" ; \
	cat /proc/$$pid/status | egrep 'VmSize|VmPeak|VmRSS|VmData|VmStk|VmExe'

dependExtra:
	mkdir -p logs
