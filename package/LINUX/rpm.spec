#
#	RPM spec file for the Embedthis Appweb HTTP web server
#
Summary: ${settings.title} -- Embeddable HTTP Web Server
Name: ${settings.product}
Version: ${settings.version}
Release: ${settings.buildNumber}
License: Dual GPL/commercial
Group: Applications/Internet
URL: http://www.appwebserver.org
Distribution: Embedthis
Vendor: Embedthis Software
BuildRoot: ${dir.pkg}/RPM/BUILDROOT/${settings.product}-${settings.version}-${settings.buildNumber}.${platform.arch}
AutoReqProv: no

%description
Embedthis Appweb is the fast, little web server.

%prep

%build

%install
    if [ -x /usr/lib/appweb/bin/uninstall ] ; then
        appweb_HEADLESS=1 /usr/lib/appweb/bin/uninstall </dev/null 2>&1 >/dev/null
    fi
    mkdir -p ${dir.pkg}/RPM/BUILDROOT/${settings.product}-${settings.version}-${settings.buildNumber}.${platform.arch}
    for dir in BIN SRC ; do
        cp -r ${dir.pkg}/${dir}/* ${dir.pkg}/RPM/BUILDROOT/${settings.product}-${settings.version}-${settings.buildNumber}.${platform.arch}
    done

%clean

%files -f binFiles.txt

%post
if [ -x /usr/bin/chcon ] ; then 
	sestatus | grep enabled >/dev/null 2>&1
	if [ $? = 0 ] ; then
		for f in ${prefixes.lib}/*.so ; do
			chcon /usr/bin/chcon -t texrel_shlib_t $f
		done
	fi
fi
${prefixes.bin}/linkup Install /
ldconfig -n ${prefixes.lib}

%preun

%postun

#
#	Dev package
#
##	%package dev
##	Summary: Embedthis Appweb -- Development headers for ${settings.title}
##	Group: Applications/Internet
##	Prefix: ${prefixes.inc}
##	
##	%description dev
##	Development headers for ${settings.title}
##	
##	%files dev -f devFiles.txt

#
#	Source package
#
%package src
Summary: Embedthis Appweb -- Source code for ${settings.title}
Group: Applications/Internet
Prefix: ${prefixes.src}

%description src
Source code for ${settings.title}

%files src -f srcFiles.txt
