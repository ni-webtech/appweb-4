#
#	RPM spec file for the Embedthis Appweb HTTP web server
#
Summary: !!BLD_NAME!! -- Embeddable HTTP Web Server
Name: !!BLD_PRODUCT!!
Version: !!BLD_VERSION!!
Release: !!BLD_NUMBER_ONLY!!
License: Dual GPL/commercial
Group: Applications/Internet
URL: http://www.appwebserver.org
Distribution: Embedthis
Vendor: Embedthis Software
BuildRoot: !!ROOT_DIR!!/RPM/BUILDROOT/!!BLD_PRODUCT!!-!!BLD_VERSION!!-!!BLD_NUMBER_ONLY!!.!!BLD_CPU!!
AutoReqProv: no

%description
Embedthis Appweb is the fast, little web server.

%prep

%build

%install
    if [ -x /usr/lib/appweb/bin/uninstall ] ; then
        appweb_HEADLESS=1 /usr/lib/appweb/bin/uninstall </dev/null 2>&1 >/dev/null
    fi
    mkdir -p !!ROOT_DIR!!/RPM/BUILDROOT/!!BLD_PRODUCT!!-!!BLD_VERSION!!-!!BLD_NUMBER_ONLY!!.!!BLD_CPU!!
    for dir in BIN SRC ; do
        cp -r !!ROOT_DIR!!/${dir}/*  !!ROOT_DIR!!/RPM/BUILDROOT/!!BLD_PRODUCT!!-!!BLD_VERSION!!-!!BLD_NUMBER_ONLY!!.!!BLD_CPU!!
    done

%clean

%files -f binFiles.txt

%post
if [ -x /usr/bin/chcon ] ; then 
	sestatus | grep enabled >/dev/null 2>&1
	if [ $? = 0 ] ; then
		for f in !!ORIG_BLD_LIB_PREFIX!!/*.so ; do
			chcon /usr/bin/chcon -t texrel_shlib_t $f
		done
	fi
fi
!!ORIG_BLD_BIN_PREFIX!!/linkup Install /
ldconfig -n !!ORIG_BLD_LIB_PREFIX!!

%preun

%postun

#
#	Dev package
#
##	%package dev
##	Summary: Embedthis Appweb -- Development headers for !!BLD_NAME!!
##	Group: Applications/Internet
##	Prefix: !!ORIG_BLD_INC_PREFIX!!
##	
##	%description dev
##	Development headers for !!BLD_NAME!!
##	
##	%files dev -f devFiles.txt

#
#	Source package
#
%package src
Summary: Embedthis Appweb -- Source code for !!BLD_NAME!!
Group: Applications/Internet
Prefix: !!ORIG_BLD_SRC_PREFIX!!

%description src
Source code for !!BLD_NAME!!

%files src -f srcFiles.txt
