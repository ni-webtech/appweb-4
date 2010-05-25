#!/bin/bash
#
#	uninstall: Appweb uninstall script
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#
#	Usage: uninstall [configFile]
#
################################################################################
#
#	Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
#	The latest version of this code is available at http://www.embedthis.com
#
#	This software is open source; you can redistribute it and/or modify it 
#	under the terms of the GNU General Public License as published by the 
#	Free Software Foundation; either version 2 of the License, or (at your 
#	option) any later version.
#
#	This program is distributed WITHOUT ANY WARRANTY; without even the 
#	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
#	See the GNU General Public License for more details at:
#	http://www.embedthis.com/downloads/gplLicense.html
#	
#	This General Public License does NOT permit incorporating this software 
#	into proprietary programs. If you are unable to comply with the GPL, a 
#	commercial license for this software and support services are available
#	from Embedthis Software at http://www.embedthis.com
#
################################################################################
#
#	NOTE: We require a saved setup file exist in $BLD_CFG_PREFIX/{PRODUCT}Install.conf
#	This is created by install.
#

HOME=`pwd`
FMT=

BLD_PRODUCT="!!BLD_PRODUCT!!"
BLD_COMPANY="!!BLD_COMPANY!!"
BLD_NAME="!!BLD_NAME!!"
BLD_VERSION="!!BLD_VERSION!!"
BLD_NUMBER="!!BLD_NUMBER!!"
BLD_BUILD_OS="!!BLD_BUILD_OS!!"
BLD_HOST_OS="!!BLD_HOST_OS!!"
BLD_HOST_CPU="!!BLD_HOST_CPU!!"

BLD_PREFIX="!!ORIG_BLD_PREFIX!!"				# Fixed and can't be relocated
BLD_BIN_PREFIX="!!ORIG_BLD_BIN_PREFIX!!"
BLD_CFG_PREFIX="!!ORIG_BLD_CFG_PREFIX!!"
BLD_DOC_PREFIX="!!ORIG_BLD_DOC_PREFIX!!"
BLD_INC_PREFIX="!!ORIG_BLD_INC_PREFIX!!"
BLD_LIB_PREFIX="!!ORIG_BLD_LIB_PREFIX!!"
BLD_MAN_PREFIX="!!ORIG_BLD_MAN_PREFIX!!"
BLD_MOD_PREFIX="!!ORIG_BLD_MOD_PREFIX!!"
BLD_SAM_PREFIX="!!ORIG_BLD_SAM_PREFIX!!"
BLD_SRC_PREFIX="!!ORIG_BLD_SRC_PREFIX!!"
BLD_WEB_PREFIX="!!ORIG_BLD_WEB_PREFIX!!"

removebin=Y
removedev=Y

PATH=$PATH:/sbin:/usr/sbin
unset CDPATH

###############################################################################
# 
#	Get a yes/no answer from the user. Usage: ans=`yesno "prompt" "default"`
#	Echos 1 for Y or 0 for N
#

yesno() {
	if [ "$!!BLD_PRODUCT!!_HEADLESS" = 1 ] ; then
		echo "Y"
		return
	fi
	echo -n "$1 [$2] : " 1>&2
	while [ 1 ] 
	do
		read ans
		if [ "$ans" = "" ] ; then
			echo $2 ; break
		elif [ "$ans" = "Y" -o "$ans" = "y" ] ; then
			echo "Y" ; break
		elif [ "$ans" = "N" -o "$ans" = "n" ] ; then
			echo "N" ; break
		fi
		echo -e "\nMust enter a 'y' or 'n'\n " 1>&1
	done
}


#
#	Modify service
#	Usage:	configureService start|stop|install
#
configureService() {
	local action=$1

	case $action in
	start|stop)
		if [ $BLD_HOST_OS = WIN ] ; then
            if [ $action = start ] ; then
                if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
                    "$BLD_BIN_PREFIX/angel" --start $BLD_BIN_PREFIX/$BLD_PRODUCT
                    "$BLD_BIN_PREFIX/${BLD_PRODUCT}Monitor" &
                fi
            else
                if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
                    "$BLD_BIN_PREFIX/angel" --stop $BLD_BIN_PREFIX/$BLD_PRODUCT
                    "$BLD_BIN_PREFIX/${BLD_PRODUCT}Monitor" --stop
                fi
            fi
		elif which launchctl >/dev/null 2>&1 ; then
            local company=`echo $BLD_COMPANY | tr '[:upper:]' '[:lower:']`
            if [ $action = start ] ; then
                launchctl start com.${company}.${BLD_PRODUCT}
            else
                launchctl stop /Library/LaunchDaemons/com.${company}.${BLD_PRODUCT} 2>/dev/null
            fi
		elif which service >/dev/null 2>&1 ; then
			service $BLD_PRODUCT $action >/dev/null 2>&1
		elif which invoke-rc.d >/dev/null 2>&1 ; then
			if [ -f /etc/init.d/$BLD_PRODUCT ] ; then
				invoke-rc.d $BLD_PRODUCT $action
			fi
		fi
		;;

	install)
		if [ $BLD_HOST_OS = WIN ] ; then
			if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
				"$BLD_BIN_PREFIX/angel" --install $BLD_BIN_PREFIX/$BLD_PRODUCT
			fi
		elif which launchctl >/dev/null 2>&1 ; then
            local company=`echo $BLD_COMPANY | tr '[:upper:]' '[:lower:']`
            launchctl load /Library/LaunchDaemons/com.${company}.${BLD_PRODUCT}.plist
		elif which chkconfig >/dev/null 2>&1 ; then
			/sbin/chkconfig --add $BLD_PRODUCT >/dev/null
			/sbin/chkconfig --level 5 $BLD_PRODUCT on >/dev/null

		elif which update-rc.d >/dev/null 2>&1 ; then
			update-rc.d $BLD_PRODUCT defaults 90 10 >/dev/null
		fi
		;;

	uninstall)
		if [ $BLD_HOST_OS = WIN ] ; then
			if [ -x "$BLD_BIN_PREFIX/angel" ] ; then
				"$BLD_BIN_PREFIX/angel" --uninstall $BLD_BIN_PREFIX/$BLD_PRODUCT
			fi
		elif which launchctl >/dev/null 2>&1 ; then
            local company=`echo $BLD_COMPANY | tr '[:upper:]' '[:lower:']`
            launchctl unload -w /Library/LaunchDaemons/com.${company}.${BLD_PRODUCT}.plist 2>/dev/null
		elif which chkconfig >/dev/null 2>&1 ; then
			/sbin/chkconfig --del $BLD_PRODUCT >/dev/null 2>&1
		elif which update-rc.d >/dev/null 2>&1 ; then
			update-rc.d -f $BLD_PRODUCT remove >/dev/null
		fi
		;;
	esac
}


deconfigureService() {

	if [ "$removebin" = "Y" ] ; then
		echo -e "\nStopping $BLD_NAME service"
		configureService stop
		echo -e "\nRemoving $BLD_NAME service"
		configureService uninstall
	fi
	if [ -f $BLD_BIN_PREFIX/angel ] ; then
        $BLD_BIN_PREFIX/angel --stop $BLD_BIN_PREFIX/$BLD_PRODUCT 2>&1 >/dev/null
    fi
    if [ -f $BLD_BIN_PREFIX/$BLD_PRODUCT ] ; then
		if which pidof >/dev/null 2>&1 ; then
            pid=`pidof $BLD_BIN_PREFIX/$BLD_PRODUCT`
        else
            pid=`ps -ef | grep $BLD_BIN_PREFIX/$BLD_PRODUCT | grep -v 'grep' | awk '{print $2}'`
        fi
		[ "$pid" != "" ] && kill -9 $pid >/dev/null 2>&1
	fi
} 


removeFiles() {
	local pkg doins name

	echo
	for pkg in dev doc bin ; do
		
		doins=`eval echo \\$install${pkg}`
		if [ "$doins" = Y ] ; then
			suffix="-${pkg}"
			if [ "$pkg" = bin ] ; then
				name="${BLD_PRODUCT}"
			else 
				name="${BLD_PRODUCT}${suffix}"
			fi
			if [ "$FMT" = "rpm" ] ; then
				echo -e "Running \"rpm -e $name\""
				rpm -e $name
			elif [ "$FMT" = "deb" ] ; then
				echo -e "Running \"dpkg -r $name\""
				dpkg -r $name >/dev/null
			else
				removeTarFiles $pkg
			fi
        elif [ "$doins" = "" ] ; then
            removeTarFiles $pkg
		fi
	done
}


removeTarFiles() {
	local pkg prefix
	local cdir=`pwd`

	pkg=$1

	[ $pkg = bin ] && prefix="$BLD_CFG_PREFIX"
	[ $pkg = dev ] && prefix="$BLD_INC_PREFIX"
	[ $pkg = doc ] && prefix="$BLD_DOC_PREFIX"

	if [ "$prefix/fileList.txt" ] ; then
        cd /
        removeFileList "$prefix/fileList.txt"
        cd "$cdir"
        rm -f "$prefix/fileList.txt"
	fi
}


preClean() {
	local f
	local cdir=`pwd`

    cp $BLD_BIN_PREFIX/linkup /tmp/linkup$$

	if [ $BLD_HOST_OS != WIN ] ; then
        rm -f /var/lock/subsys/$BLD_PRODUCT /var/lock/$BLD_PRODUCT
        rm -fr /var/log/$BLD_PRODUCT
        rm -rf /var/run/$BLD_PRODUCT
    fi

	if [ "$removebin" = "Y" ] ; then
		cd "$BLD_CFG_PREFIX"
		removeIntermediateFiles access.log error.log '*.log.old' .dummy $BLD_PRODUCT.conf make.log
		cd "$cdir"
		cd "$BLD_LIB_PREFIX"
		removeIntermediateFiles access.log error.log '*.log.old' .dummy $BLD_PRODUCT.conf make.log
		cd "$cdir"
	fi

	if [ "$removedev" = "Y" ] ; then
		if [ -d "$BLD_INC_PREFIX" ] ; then
			cd "$BLD_INC_PREFIX"
			make clean >/dev/null 2>&1 || true
			removeIntermediateFiles '*.o' '*.lo' '*.so' '*.a' make.rules .config.h.sav make.log .changes
			cd "$cdir"
		fi
	fi
}


postClean() {
	local cdir=`pwd`

	echo

	rm -f "${BLD_CFG_PREFIX}/${BLD_PRODUCT}Install.conf"

	if [ "$removebin" = "Y" ] ; then
		cleanDir "$BLD_CFG_PREFIX"
		cleanDir "$BLD_LIB_PREFIX"
		cleanDir "$BLD_WEB_PREFIX"

		if [ -d "$BLD_MAN_PREFIX" ] ; then
			rm -f "$BLD_MAN_PREFIX/${BLD_PRODUCT}.1.gz"
		fi
	fi

	if [ "$removedev" = "Y" ] ; then
		cleanDir "$BLD_SAM_PREFIX"
		cleanDir "$BLD_INC_PREFIX"
		cleanDir "$BLD_DOC_PREFIX"
	fi

	if [ $BLD_HOST_OS != WIN ] ; then
        if [ -x /usr/share/$BLD_PRODUCT ] ; then
            cleanDir /usr/share/$BLD_PRODUCT
        fi
        if [ -d /var/$BLD_PRODUCT ] ; then
            cleanDir /var/$BLD_PRODUCT
        fi
        rmdir /usr/share/${BLD_PRODUCT} >/dev/null 2>&1
        rmdir $BLD_CFG_PREFIX >/dev/null 2>&1
    fi
    /tmp/linkup$$ Remove
    rm -f /tmp/linkup$$
}


#
#	Clean a directory. Usage: removeFileList fileList
#
removeFileList() {

    if [ -f "$1" ] ; then
        echo -e "Removing files in file list \"$1\" ..."
        cat "$1" | while read f
        do
            rm -f "$f"
        done
    fi
}


#
#	Cleanup empty directories. Usage: cleanDir directory
#
cleanDir() {
	local dir
	local cdir=`pwd`

	dir="$1"

	if [ "$dir" = "" ] ; then
		echo "WARNING: clean directory is empty"
	fi

	[ ! -d "$dir" ] && return

	cd "$dir"
	echo "Cleaning `pwd` ..."
	if [ "`pwd`" = "/" ] ; then
		echo "Configuration error: clean directory was '/'"
		cd "$cdir"
		return
	fi
	find . -type d -print | sort -r | grep -v '^\.$' | while read d
	do
		count=`ls "$d" | wc -l | sed -e 's/ *//'`
		[ "$count" = "0" ] && rmdir "$d"
		if [ "$count" != "0" ] ; then
			f=`echo "$d" | sed -e 's/\.\///'`
			echo "Directory `pwd`/${f}, still has user data"
		fi
	done 

    if [ -d $cdir ] ; then
        cd $cdir
        count=`ls "$dir" | wc -l | sed -e 's/ *//'`
        [ "$count" = "0" ] && rmdir "$dir"
        if [ "$count" != "0" ] ; then
            echo "Directory ${dir}, still has user data"
        fi
        rmdir "$dir" 2>/dev/null || true
    fi
}


#
#	Cleanup intermediate files
#
removeIntermediateFiles() {
	local cdir=`pwd`

	find "`pwd`" -type d -print | while read d
	do
		cd "${d}"
		eval rm -f "$*"
		cd "${cdir}"
	done
}


setup() {

    if [ `id -u` != "0" -a $BLD_HOST_OS != WIN ] ; then
		echo "You must be root to remove this product."
		exit 255
	fi
	
	#
	#	Headless removal. Expect an argument that supplies a config file.
	#
    if [ $# -ge 1 ] ; then
        if [ ! -f $1 ] ; then
			echo "Could not find config file \"$1\""
			exit 255
		else
			. $1 
			removeFiles $FMT
		fi
		exit 0
	fi
	
	#
	#	Get defaults from the installation configuration file
	#
    if [ -f "${BLD_CFG_PREFIX}/${BLD_PRODUCT}Install.conf" ] ; then
		.  "${BLD_CFG_PREFIX}/${BLD_PRODUCT}Install.conf"
	fi
	
	binDir=${binDir:-$BLD_CFG_PREFIX}
	devDir=${devDir:-$BLD_INC_PREFIX}
	srcDir=${srcDir:-$BLD_SRC_PREFIX}

	echo -e "\n$BLD_NAME !!BLD_VERSION!!-!!BLD_NUMBER!! Removal\n"
}


askUser() {
	local finished

	echo "Enter requested information or press <ENTER> to accept the defaults. "

	#
	#	Confirm the configuration
	#
	finished=N
	while [ "$finished" = "N" ]
	do
		echo
		if [ -d "$binDir" ] ; then
			removebin=`yesno "Remove binary package" "$removebin"`
		else
			removebin=N
		fi
		if [ -d "$devDir" ] ; then
			removedev=`yesno "Remove development headers and samples package" "$removedev"`
		else
			removedev=N
		fi

		echo -e "\nProceed removing with these instructions:" 
		[ $removebin = Y ] && echo -e "  Remove binary package: $removebin"
		[ $removedev = Y ] && echo -e "  Remove development package: $removedev"

		echo
		finished=`yesno "Accept these instructions" "Y"`

		if [ "$finished" != "Y" ] ; then
			exit 0
		fi
	done
}


#
#	Main program
#
setup $*
askUser

if [ $BLD_PRODUCT = appweb ] ; then
    deconfigureService
fi

preClean
removeFiles $FMT
postClean

echo -e "\n$BLD_NAME removal successful.\n"
