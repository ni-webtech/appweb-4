#!/bin/bash
#
#	uninstall: Appweb uninstall script
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#	Usage: uninstall [configFile]
#
################################################################################
#
#	Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#	The latest version of this code is available at http://embedthis.com
#
#	This software is open source; you can redistribute it and/or modify it 
#	under the terms of the GNU General Public License as published by the 
#	Free Software Foundation; either version 2 of the License, or (at your 
#	option) any later version.
#
#	This program is distributed WITHOUT ANY WARRANTY; without even the 
#	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
#	See the GNU General Public License for more details at:
#	http://embedthis.com/downloads/gplLicense.html
#	
#	This General Public License does NOT permit incorporating this software 
#	into proprietary programs. If you are unable to comply with the GPL, a 
#	commercial license for this software and support services are available
#	from Embedthis Software at http://embedthis.com
#
################################################################################
#
#	NOTE: We require a saved setup file exist in $BLD_PRD_PREFIX/install.conf
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
BLD_PRD_PREFIX="!!ORIG_BLD_PRD_PREFIX!!"
BLD_SAM_PREFIX="!!ORIG_BLD_SAM_PREFIX!!"
BLD_SPL_PREFIX="!!ORIG_BLD_SPL_PREFIX!!"
BLD_SRC_PREFIX="!!ORIG_BLD_SRC_PREFIX!!"
BLD_WEB_PREFIX="!!ORIG_BLD_WEB_PREFIX!!"

removebin=Y
headless=${!!BLD_PRODUCT!!_HEADLESS:-0}

PATH=$PATH:/sbin:/usr/sbin
export CYGWIN=nodosfilewarning
unset CDPATH

###############################################################################
# 
#	Get a yes/no answer from the user. Usage: ans=`yesno "prompt" "default"`
#	Echos 1 for Y or 0 for N
#

yesno() {
    if [ "$headless" = 1 ] ; then
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


deconfigureService() {
    [ "$headless" != 1 ] && echo -e "Stopping $BLD_NAME service"
    # Fedora will indiscriminately kill appman here too
    # Need this ( ; true) to suppress the Killed message
    (appman -v stop ; true) >/dev/null 2>&1
    [ "$headless" != 1 ] && echo -e "Removing $BLD_NAME service"
    appman disable 
    appman uninstall
    if [ -f "$BLD_BIN_PREFIX/$BLD_PRODUCT" ] ; then
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

    [ "$headless" != 1 ] && echo
    for pkg in bin ; do
        doins=`eval echo \\$install${pkg}`
        if [ "$doins" = Y ] ; then
            suffix="-${pkg}"
            if [ "$pkg" = bin ] ; then
            	name="${BLD_PRODUCT}"
            else 
            	name="${BLD_PRODUCT}${suffix}"
            fi
            if [ "$FMT" = "rpm" ] ; then
            	[ "$headless" != 1 ] && echo -e "Running \"rpm -e $name\""
            	rpm -e $name
            elif [ "$FMT" = "deb" ] ; then
            	[ "$headless" != 1 ] && echo -e "Running \"dpkg -r $name\""
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
    [ $pkg = bin ] && prefix="$BLD_PRD_PREFIX"
    if [ -f "$prefix/fileList.txt" ] ; then
        if [ $BLD_HOST_OS = WIN ] ; then
            cd ${prefix%%:*}:/
        else
            cd /
        fi
        removeFileList "$prefix/fileList.txt"
        cd "$cdir"
        rm -f "$prefix/fileList.txt"
    fi
}


preClean() {
    local f
    local cdir=`pwd`

    cp "$BLD_BIN_PREFIX/linkup" /tmp/linkup$$
    if [ $BLD_HOST_OS != WIN ] ; then
        rm -f /var/lock/subsys/$BLD_PRODUCT /var/lock/$BLD_PRODUCT
        rm -fr /var/log/$BLD_PRODUCT
        rm -rf /var/run/$BLD_PRODUCT
        rm -rf /var/spool/$BLD_PRODUCT
    fi
    if [ -x "$BLD_PRD_PREFIX" ] ; then
        cd "$BLD_PRD_PREFIX"
        removeIntermediateFiles access.log* error.log* '*.log.old' .dummy $BLD_PRODUCT.conf make.log *.obj *.oC
        removeIntermediateFiles *.dylib *.dll *.exp *.lib
    fi
    if [ -x "$BLD_CFG_PREFIX" ] ; then
        cd "$BLD_CFG_PREFIX"
        removeIntermediateFiles access.log* error.log* '*.log.old' .dummy $BLD_PRODUCT.conf make.log $BLD_PRODUCT.conf.bak
    fi
    if [ -x "$BLD_LIB_PREFIX" ] ; then
        cd "$BLD_LIB_PREFIX"
        removeIntermediateFiles access.log* error.log* '*.log.old' .dummy $BLD_PRODUCT.conf make.log
    fi
    if [ -x "$BLD_WEB_PREFIX" ] ; then
        cd "$BLD_WEB_PREFIX"
        removeIntermediateFiles *.mod 
    fi
    if [ -x "$BLD_SPL_PREFIX/cache" ] ; then
        cd "$BLD_SPL_PREFIX/cache"
        removeIntermediateFiles *.mod *.c *.dll *.exp *.lib *.obj *.o *.dylib *.so
    fi
    if [ -d "$BLD_INC_PREFIX" ] ; then
        cd "$BLD_INC_PREFIX"
        make clean >/dev/null 2>&1 || true
        removeIntermediateFiles '*.o' '*.lo' '*.so' '*.a' make.rules .config.h.sav make.log .changes
    fi
    cd "$cdir"
}


postClean() {
    local cdir=`pwd`

    # Legacy
    rm -f "${BLD_CFG_PREFIX}/${BLD_PRODUCT}Install.conf"
    rm -f "${BLD_PRD_PREFIX}/install.conf"

    if [ -d "$BLD_MAN_PREFIX" ] ; then
        rm -rf "$BLD_MAN_PREFIX"/man*
    fi
    cleanDir "$BLD_MAN_PREFIX"
    cleanDir "$BLD_SAM_PREFIX"
    cleanDir "$BLD_INC_PREFIX"
    cleanDir "$BLD_DOC_PREFIX"

    cleanDir "$BLD_PRD_PREFIX"
    cleanDir "$BLD_CFG_PREFIX"
    cleanDir "$BLD_LIB_PREFIX"
    cleanDir "$BLD_WEB_PREFIX"
    cleanDir "$BLD_SPL_PREFIX"

    if [ $BLD_HOST_OS != WIN ] ; then
        if [ -x /usr/share/$BLD_PRODUCT ] ; then
            cleanDir /usr/share/$BLD_PRODUCT
        fi
        if [ -d /var/$BLD_PRODUCT ] ; then
            cleanDir /var/$BLD_PRODUCT
        fi
        rmdir /usr/share/${BLD_PRODUCT} >/dev/null 2>&1
        rmdir $BLD_WEB_PREFIX >/dev/null 2>&1
        rmdir $BLD_MAN_PREFIX >/dev/null 2>&1
        rmdir $BLD_DOC_PREFIX >/dev/null 2>&1
        rmdir $BLD_INC_PREFIX >/dev/null 2>&1
        rmdir $BLD_PRD_PREFIX >/dev/null 2>&1
        rmdir $BLD_CFG_PREFIX >/dev/null 2>&1
        rmdir $BLD_SPL_PREFIX >/dev/null 2>&1
    fi
    /tmp/linkup$$ Remove /
    rm -f /tmp/linkup$$
}


#
#	Clean a directory. Usage: removeFileList fileList
#
removeFileList() {

    if [ -f "$1" ] ; then
        [ "$headless" != 1 ] && echo -e "Removing files in file list \"$1\" ..."
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

    [ ! -d "$dir" ] && return

    cd "$dir"
    if [ "`pwd`" = "/" ] ; then
        echo "Configuration error: clean directory was '/'"
        cd "$cdir"
        return
    fi
    find . -type d -print | sort -r | grep -v '^\.$' | while read d
    do
        count=`ls "$d" 2>/dev/null | wc -l | sed -e 's/ *//'`
        [ "$count" = "0" ] && rmdir "$d" >/dev/null 2>&1
    done 

    if [ -d $cdir ] ; then
        cd $cdir
        count=`ls "$dir" 2>/dev/null | wc -l | sed -e 's/ *//'`
        [ "$count" = "0" ] && rmdir "$dir" >/dev/null 2>&1
        rmdir "$dir" 2>/dev/null
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
    if [ -f "${BLD_PRD_PREFIX}/install.conf" ] ; then
        .  "${BLD_PRD_PREFIX}/install.conf"
    fi
    
    binDir=${binDir:-$BLD_PRD_PREFIX}
    [ "$headless" != 1 ] && echo -e "\n$BLD_NAME !!BLD_VERSION!!-!!BLD_NUMBER!! Removal\n"
}


askUser() {
    local finished

    [ "$headless" != 1 ] && echo "Enter requested information or press <ENTER> to accept the defaults. "

    #
    #	Confirm the configuration
    #
    finished=N
    while [ "$finished" = "N" ]
    do
        [ "$headless" != 1 ] && echo
        if [ -d "$binDir" ] ; then
            removebin=`yesno "Remove binary package" "$removebin"`
        else
            removebin=N
        fi
        if [ "$headless" != 1 ] ; then
            echo -e "\nProceed removing with these instructions:" 
            [ $removebin = Y ] && echo -e "  Remove binary package: $removebin"
        fi
        [ "$headless" != 1 ] && echo
        finished=`yesno "Accept these instructions" "Y"`
        if [ "$finished" != "Y" ] ; then
            exit 0
        fi
    done
    [ "$headless" != 1 ] && echo
}


#
#	Main program
#
cd /
setup $*
askUser

if [ "$removebin" = "Y" ] ; then
    deconfigureService
    preClean
    removeFiles $FMT
    postClean
    [ "$headless" != 1 ] && echo -e "$BLD_NAME uninstall successful"
fi
exit 0

