#!/bin/bash
#
#   install: Installation script
#
#   Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
#
#   Usage: install [configFile]
#
################################################################################
#
#   The configFile is of the format:
#       FMT=[rpm|deb|tar]               # Package format to use
#       installbin=[YN]                 # Install binary package
#       runDaemon=[YN]                  # Run the program as a daemon
#       httpPort=portNumber             # Http port to listen on
#       sslPort=portNumber              # SSL port to listen on
#       username=username               # User account for appweb
#       groupname=groupname             # Group account for appweb
#       hostname=groupname              # Serving host
#

HOME=`pwd`
FMT=
SITE=localhost
PAGE=/index.html

HOSTNAME=`hostname`
BLD_COMPANY="!!BLD_COMPANY!!"
BLD_PRODUCT="!!BLD_PRODUCT!!"
BLD_NAME="!!BLD_NAME!!"
BLD_VERSION="!!BLD_VERSION!!"
BLD_NUMBER="!!BLD_NUMBER!!"
BLD_HOST_OS="!!BLD_HOST_OS!!"
BLD_HOST_CPU="!!BLD_HOST_CPU!!"
BLD_HOST_DIST="!!BLD_HOST_DIST!!"

BLD_PREFIX="!!ORIG_BLD_PREFIX!!"
BLD_BIN_PREFIX="!!ORIG_BLD_BIN_PREFIX!!"
BLD_CFG_PREFIX="!!ORIG_BLD_CFG_PREFIX!!"
BLD_DOC_PREFIX="!!ORIG_BLD_DOC_PREFIX!!"
BLD_INC_PREFIX="!!ORIG_BLD_INC_PREFIX!!"
BLD_LIB_PREFIX="!!ORIG_BLD_LIB_PREFIX!!"
BLD_LOG_PREFIX="!!ORIG_BLD_LOG_PREFIX!!"
BLD_MAN_PREFIX="!!ORIG_BLD_MAN_PREFIX!!"
BLD_PRD_PREFIX="!!ORIG_BLD_PRD_PREFIX!!"
BLD_SPL_PREFIX="!!ORIG_BLD_SPL_PREFIX!!"
BLD_SRC_PREFIX="!!ORIG_BLD_SRC_PREFIX!!"
BLD_WEB_PREFIX="!!ORIG_BLD_WEB_PREFIX!!"

installbin=Y
runDaemon=Y
headless=${!!BLD_PRODUCT!!_HEADLESS:-0}
HTTP_PORT=80
SSL_PORT=443

PATH="$PATH:/sbin:/usr/sbin"
export CYGWIN=nodosfilewarning

###############################################################################

setup() {
    umask 022
    if [ $BLD_HOST_OS != WIN -a `id -u` != "0" ] ; then
        echo "You must be root to install this product."
        exit 255
    fi
    #
    #   Headless install
    #
    if [ $# -ge 1 ] ; then
        if [ ! -f $1 ] ; then
            echo "Could not find installation config file \"$1\"." 1>&2
            exit 255
        else
            . $1 
            installFiles $FMT
            
            if [ "$installbin" = "Y" ] ; then
                appman stop disable uninstall >/dev/null 2>&1
                patchConfiguration
                appman install
                if [ "$runDaemon" = "Y" ] ; then
                    appman enable start
                fi
            fi

        fi
        exit 0
    fi
    sleuthPackageFormat
    getAccountDetails
    [ "$headless" != 1 ] && echo -e "\n$BLD_NAME !!BLD_VERSION!!-!!BLD_NUMBER!! Installation\n"

}

getAccountDetails() {

    local g u

    #
    #   Select default username
    #
    for u in www-data _www nobody Administrator 
    do
        grep "$u" /etc/passwd >/dev/null
        if [ $? = 0 ] ; then
            username=$u
            break
        fi
    done

    if [ "$username" = "" ] ; then
        echo "Can't find a suitable username in /etc/passwd for $BLD_PRODUCT" 1>&2
        exit 255
    fi
    
    #
    #   Select default group name
    #
    for g in www-data _www nogroup nobody Administrators
    do
        grep "$g" /etc/group >/dev/null
        if [ $? = 0 ] ; then
            groupname=$g
            break
        fi
    done
    
    if [ "$groupname" = "" ] ; then
        echo "Can't find a suitable group in /etc/group for $BLD_PRODUCT" 1>&2
        exit 255
    fi
}

#
#   Try to guess if we should default to using RPM
#
sleuthPackageFormat() {
    local name

    name=`createPackageName ${BLD_PRODUCT}-bin`
    FMT=
    for f in deb rpm tgz ; do
        if [ -f ${name}.${f} ] ; then
            FMT=${f%.gz}
            break
        fi
    done
    if [ "$FMT" = "" ] ; then
        echo -e "\nYou may be be missing a necessary package file. "
        echo "Check that you have the correct $BLD_NAME package".
        exit 255
    fi
}

askUser() {
    local finished

    [ "$headless" != 1 ] && echo "Enter requested information or press <ENTER> to accept the default. "

    #
    #   Confirm the configuration
    #
    finished=N
    while [ "$finished" = "N" ]
    do
        [ "$headless" != 1 ] && echo
        installbin=`yesno "Install binary package" "$installbin"`
        if [ "$installbin" = "Y" ] ; then
            runDaemon=`yesno "Start $BLD_PRODUCT automatically at system boot" $runDaemon`
            HTTP_PORT=`ask "Enter the HTTP port number" "$HTTP_PORT"`
            # SSL_PORT=`ask "Enter the SSL port number" "$SSL_PORT"`
            username=`ask "Enter the user account for $BLD_PRODUCT" "$username"`
            groupname=`ask "Enter the user group for $BLD_PRODUCT" "$groupname"`
        else
            runDaemon=N
        fi
    
        if [ "$headless" != 1 ] ; then
            echo -e "\nInstalling with this configuration:" 
            echo -e "    Install binary package: $installbin"
        
            if [ "$installbin" = "Y" ] ; then
                echo -e "    Start automatically at system boot: $runDaemon"
                echo -e "    HTTP port number: $HTTP_PORT"
                # echo -e "    SSL port number: $SSL_PORT"
                echo -e "    Username: $username"
                echo -e "    Groupname: $groupname"
            fi
            echo
        fi
        finished=`yesno "Accept this configuration" "Y"`
    done
    [ "$headless" != 1 ] && echo
    
    if [ "$installbin" = "N" ] ; then
        echo -e "Nothing to install, exiting. "
        exit 0
    fi
}

createPackageName() {
    echo ${1}-${BLD_VERSION}-${BLD_NUMBER}-${BLD_HOST_DIST}-${BLD_HOST_OS}-${BLD_HOST_CPU}
}

# 
#   Get a yes/no answer from the user. Usage: ans=`yesno "prompt" "default"`
#   Echos 1 for Y or 0 for N
#
yesno() {
    local ans

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

# 
#   Get input from the user. Usage: ans=`ask "prompt" "default"`
#   Returns the answer or default if <ENTER> is pressed
#
ask() {
    local ans

    default=$2
    if [ "$headless" = 1 ] ; then
        echo "$default"
        return
    fi
    echo -n "$1 [$default] : " 1>&2
    read ans
    if [ "$ans" = "" ] ; then
        echo $default
    fi
    echo $ans
}

saveSetup() {
    local firstChar

    mkdir -p "$BLD_PRD_PREFIX"
    echo -e "FMT=$FMT\nbinDir=\"${BLD_PRD_PREFIX}\"\ninstallbin=$installbin\nrunDaemon=$runDaemon\nhttpPort=$HTTP_PORT\nsslPort=$SSL_PORT\nusername=$username\ngroupname=$groupname\nhostname=$HOSTNAME" >"$BLD_PRD_PREFIX/install.conf"
}

removeOld() {
    if [ -x /usr/lib/appweb/bin/uninstall ] ; then
        appweb_HEADLESS=1 /usr/lib/appweb/bin/uninstall </dev/null 2>&1 >/dev/null
    fi
}

installFiles() {
    local dir pkg doins NAME upper target

    [ "$headless" != 1 ] && echo -e "\nExtracting files ...\n"

    for pkg in bin ; do
        doins=`eval echo \\$install${pkg}`
        if [ "$doins" = Y ] ; then
            upper=`echo $pkg | tr '[:lower:]' '[:upper:]'`
            suffix="-${pkg}"
            #
            #   RPM doesn't give enough control on error codes. So best to
            #   keep going. 
            #
            NAME=`createPackageName ${BLD_PRODUCT}${suffix}`.$FMT
            if [ "$runDaemon" != "Y" ] ; then
                export APPWEB_DONT_START=1
            fi
            if [ "$FMT" = "rpm" ] ; then
                [ "$headless" != 1 ] && echo -e "rpm -Uhv $NAME"
                rpm -Uhv $HOME/$NAME
            elif [ "$FMT" = "deb" ] ; then
                [ "$headless" != 1 ] && echo -e "dpkg -i $NAME"
                dpkg -i $HOME/$NAME >/dev/null
            else
                target=/
                [ $BLD_HOST_OS = WIN ] && target=`cygpath ${HOMEDRIVE}/`
                [ "$headless" != 1 ] && echo tar xzf "$HOME/${NAME}" --strip-components 1 -P -C ${target}
                tar xzf "$HOME/${NAME}" --strip-components 1 -P -C ${target}
            fi
        fi
    done

    if [ -f /etc/redhat-release -a -x /usr/bin/chcon ] ; then 
        if sestatus | grep enabled >/dev/nulll ; then
            for f in $BLD_LIB_PREFIX/*.so ; do
                chcon /usr/bin/chcon -t texrel_shlib_t $f 2>&1 >/dev/null
            done
        fi
    fi

    if [ "$BLD_HOST_OS" = "FREEBSD" ] ; then
        LDCONFIG_OPT=-m
    else
        LDCONFIG_OPT=-n
    fi
    if which ldconfig >/dev/null 2>&1 ; then
        ldconfig /usr/lib/lib$BLD_PRODUCT.so.?.?.?
        ldconfig $LDCONFIG_OPT /usr/lib/$BLD_PRODUCT
        ldconfig $LDCONFIG_OPT /usr/lib/$BLD_PRODUCT/modules
    fi
    "$BLD_BIN_PREFIX/linkup" Install /

    if [ $BLD_HOST_OS = WIN ] ; then
        [ "$headless" != 1 ] && echo -e "\nSetting file permissions ..."
        find "$BLD_CFG_PREFIX" -type d -exec chmod 755 "{}" \;
        find "$BLD_CFG_PREFIX" -type f -exec chmod g+r,o+r "{}" \; 
        find "$BLD_WEB_PREFIX" -type d -exec chmod 755 "{}" \;
        find "$BLD_WEB_PREFIX" -type f -exec chmod g+r,o+r "{}" \;
        mkdir -p "$BLD_CFG_PREFIX/logs"
        chmod 777 "$BLD_CFG_PREFIX/logs"
        mkdir -p "$BLD_SPL_PREFIX"
        chmod 777 "$BLD_SPL_PREFIX"
        chmod 755 "$BLD_WEB_PREFIX"
        find "$BLD_BIN_PREFIX" -name '*.dll' -exec chmod 755 "{}" \;
        find "$BLD_BIN_PREFIX" -name '*.exe' -exec chmod 755 "{}" \;
    fi
    mkdir -p "$BLD_SPL_PREFIX" "$BLD_SPL_PREFIX/cache" "$BLD_LOG_PREFIX"
    chown $username "$BLD_SPL_PREFIX" "$BLD_SPL_PREFIX/cache" "$BLD_LOG_PREFIX"
    chgrp $groupname "$BLD_SPL_PREFIX" "$BLD_SPL_PREFIX/cache" "$BLD_LOG_PREFIX"
    chmod 755 "$BLD_SPL_PREFIX" "$BLD_SPL_PREFIX/cache" "$BLD_LOG_PREFIX"
    [ "$headless" != 1 ] && echo
}

patchConfiguration() {
    if [ ! -f $BLD_PRODUCT.conf -a -f "$BLD_CFG_PREFIX/new.conf" ] ; then
        cp "$BLD_CFG_PREFIX/new.conf" "$BLD_CFG_PREFIX/$BLD_PRODUCT.conf"
    fi
    if [ $BLD_HOST_OS = WIN ] ; then
        "$BLD_BIN_PREFIX/setConfig" --port ${HTTP_PORT} --ssl ${SSL_PORT} --home "." --logs "logs" \
            --documents "web" --modules "bin" --cache "cache" \
            --user $username --group $groupname "${BLD_CFG_PREFIX}/appweb.conf"
    else
        "$BLD_BIN_PREFIX/setConfig" --port ${HTTP_PORT} --ssl ${SSL_PORT} --home "${BLD_CFG_PREFIX}" \
            --logs "${BLD_LOG_PREFIX}" --documents "${BLD_WEB_PREFIX}" --modules "${BLD_LIB_PREFIX}" \
            --cache "${BLD_SPL_PREFIX}/cache" --user $username --group $groupname "${BLD_CFG_PREFIX}/appweb.conf"
    fi
}

startBrowser() {
    if [ "$headless" = 1 ] ; then
        return
    fi
    #
    #   Conservative delay to allow appweb to start and initialize
    #
    sleep 5
    [ "$headless" != 1 ] && echo -e "Starting browser to view the $BLD_NAME Home Page."
    if [ $BLD_HOST_OS = WIN ] ; then
        cygstart --shownormal http://$SITE:$HTTP_PORT$PAGE 
    elif [ $BLD_HOST_OS = MACOSX ] ; then
        open http://$SITE:$HTTP_PORT$PAGE >/dev/null 2>&1 &
    else
        for f in /usr/bin/htmlview /usr/bin/firefox /usr/bin/mozilla /usr/bin/konqueror 
        do
            if [ -x ${f} ] ; then
                sudo -H -b ${f} http://$SITE:$HTTP_PORT$PAGE >/dev/null 2>&1 &
                break
            fi
        done
    fi
}

#
#   Main program for install script
#
setup $*
askUser

if [ "$installbin" = "Y" ] ; then
    [ "$headless" != 1 ] && echo "Disable existing service"
    appman stop disable uninstall >/dev/null 2>&1
fi
removeOld
saveSetup
installFiles $FMT
if [ "$installbin" = "Y" ] ; then
    "$BLD_BIN_PREFIX/appman" stop disable uninstall >/dev/null 2>&1
    patchConfiguration
    "$BLD_BIN_PREFIX/appman" install
    if [ "$runDaemon" = "Y" ] ; then
        "$BLD_BIN_PREFIX/appman" enable
        "$BLD_BIN_PREFIX/appman" start
        #
        #   Don't start browser anymore. Many systems can't determine the logged in user's keychain when run privileged
        # startBrowser
    fi
fi
if [ "$headless" != 1 ] ; then
    echo
    echo -e "$BLD_NAME installation successful."
fi
exit 0
