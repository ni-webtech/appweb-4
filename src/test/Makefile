#
# Makefile for the Appweb unit tests
#
# Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#

BUILD_DEPTH		?= 2
TEST_SYSTEM		:= 127.0.0.1:4100

include 		.makedep

TARGETS			+= $(BLD_BIN_DIR)/testAppweb$(BLD_EXE) 
ifeq ($(BLD_FEATURE_STATIC),0)
	TARGETS		+= $(BLD_MOD_DIR)/egiTest$(BLD_SHOBJ)
endif

ifeq			($(BLD_FEATURE_CGI),1)
	CGIP		:= $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
	TARGETS		+= cgi-bin/testScript cgi-bin/cgiProgram$(BLD_EXE)
endif

compileExtra: 	$(TARGETS)

#
#	Build testAppweb
#
$(BLD_BIN_DIR)/testAppweb$(BLD_EXE): $(OBJECTS) $(call lib, appweb)
	bld --exe $(BLD_BIN_DIR)/testAppweb$(BLD_EXE) --libs "http $(BLD_APPWEB_LIBS)" \
		testHttp$(BLD_OBJ) testAppweb$(BLD_OBJ)

#
#	EGI test module
#
$(BLD_MOD_DIR)/egiTest$(BLD_SHOBJ): $(call lib, appweb) $(BLD_OBJ_DIR)/testEgi$(BLD_OBJ)
	bld --shared --library $(BLD_MOD_DIR)/egiTest --libs "$(LIBS) $(BLD_APPWEB_LIBS)" testEgi$(BLD_OBJ)

ifneq	($(BUILDING_CROSS),1)
testExtra:
	mkdir -p web/tmp
	find . -name '*.mod' | xargs rm -f >/dev/null 2>&1 ; true
	if [ ! -f $(BLD_BIN_DIR)/ejs ] ; then \
		if ! which ejs >/dev/null ; then \
			$(call log) "[Test]" "Skip unit tests. Required \"ejs\" is installed." ; \
		else \
			ejs $(BLD_TOOLS_DIR)/utest -v ; \
		fi ; \
	else \
		$(BLD_BIN_DIR)/ejs $(BLD_TOOLS_DIR)/utest -v ; \
	fi
endif

#
#	Update the shbang path in testScript. Must be an absolute path on some systems and for valgrind.
#
cgi-bin/testScript: $(CGIP) Makefile $(BLD_TOP)/buildConfig.make
	echo "#!$(BLD_ABS_BIN_DIR)/cgiProgram$(BLD_EXE)" >cgi-bin/testScript
	chmod +x cgi-bin/testScript

#
#	cgiProgram
#
cgi-bin/cgiProgram$(BLD_EXE): $(BLD_BIN_DIR)/cgiProgram$(BLD_EXE)
	cp $(CGIP)  cgi-bin/cgiProgram$(BLD_EXE)
	cp $(CGIP)  cgi-bin/nph-cgiProgram$(BLD_EXE)
	cp $(CGIP) 'cgi-bin/cgi Program$(BLD_EXE)'
	cp $(CGIP)  web/cgiProgram.cgi
	chmod +x cgi-bin/* web/cgiProgram.cgi

valgrindTest:
	if type valgrind >/dev/null 2>/dev/null ; then \
		$(call log) "[Test]" "Valgrind tests" ; \
		valgrind -q --tool=memcheck --suppressions=appweb.supp $(BLD_BIN_DIR)/appweb --log stdout:0 --timeout 20 ; \
	fi

server: 
	$(BLD_BIN_DIR)/appweb --name APPWEB --log stdout:1

first:
	utest -n -v -i 9999999 --threads 8 ; \

second:
	utest -2 -v -i 9999999 --threads 8 ; \

#
#	Test using the http program
#
http:
	iterations=4000 ; \
	set -e ; while : ; \
	do \
		set -x ; \
		$(BLD_BIN_DIR)/http -v -t 1 -w 1 -i $$iterations -q $(TEST_SYSTEM)/index.html ; \
		$(BLD_BIN_DIR)/http -v -t 2 -w 2 -i $$iterations -q $(TEST_SYSTEM)/index.html ; \
		$(BLD_BIN_DIR)/http -v -t 4 -w 4 -i $$iterations -q $(TEST_SYSTEM)/index.html ; \
		$(BLD_BIN_DIR)/http -v -t 8 -w 8 -i $$iterations -q $(TEST_SYSTEM)/index.html ; \
		set +x ; \
	done

mem:
	ejs $(BLD_TOOLS_DIR)/memmon APPWEB

users.db:
	echo "# Make authorization databases"
	rm -f users.db groups.db
	@set -e ; \
	httpPassword -c -p pass1 users.db "Acme Inc" joshua ; \
	httpPassword -p pass2 users.db "Acme Inc" mary ; \
	httpPassword -p pass3 users.db "Coyote Corp" peter ; \
	httpPassword -p pass4 users.db "Coyote Corp" julie ; \
	echo "1: 1023: coyoteUsers: peter julie" >groups.db ; \
	echo "1: FFFF: acmeUsers: joshua mary" >>groups.db ; \
	echo "1: FFFF: acmeExec: mary" >>groups.db

scan:
	cd ~/src/nikto ; perl nikto.pl -h 127.0.0.1 -p 4010
	# cd ~/src/nikto ; perl nikto.pl -h 127.0.0.1 -p 4010,4011,4012,4443

scanUpdate:
	perl ~/src/nikto/nikto.pl -update

apacheBench:
	ab -k -c 10 -n 1000 http://127.0.0.1:4010/bench.html 

cleanExtra:
	rm -f *.obj *.pdb
	rm -f access.log error.log leak.log
	rm -f $(MODULES) $(BLD_BIN_DIR)/testAppweb$(BLD_EXE) testAppweb$(BLD_EXE)
	rm -f *.o *.lo *.obj *.out */*.mod */*/*.mod
	rm -f ../cgi-bin/*cgi* ../cgi-bin/testScript

#
#	Don't copy this. Ensure you set minimal permissions on upload and temp directories
#
dependExtra:
	mkdir -p web/upload web/tmp
	chmod 777 web/upload web/tmp
