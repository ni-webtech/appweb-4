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
#       srcDir=sourcePath               # Where to install the doc
#       devDir=documentationPath        # Where to install the doc
#       installbin=[YN]                 # Install binary package
#       installdev=[YN]                 # Install dev headers package
#       runDaemon=[YN]                  # Run the program as a daemon
#       httpPort=portNumber             # Http port to listen on
#       sslPort=portNumber              # SSL port to listen on
#       username=username               # User account for appweb
#       groupname=groupname             # Group account for appweb
#       hostname=groupname              # Serving host
#

HOME=`pwd`
FMT=
SITE=127.0.0.1
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
BLD_SRC_PREFIX="!!ORIG_BLD_SRC_PREFIX!!"
BLD_WEB_PREFIX="!!ORIG_BLD_WEB_PREFIX!!"

installbin=Y
installdev=Y
runDaemon=Y
HTTP_PORT=7777
SSL_PORT=4443

PATH=$PATH:/sbin:/usr/sbin

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
                patchConfiguration
                if [ "$runDaemon" = "Y" ] ; then
                    startService
                    startBrowser
                fi
            fi

        fi
        exit 0
    fi

    sleuthPackageFormat
    getAccountDetails
    echo -e "\n$BLD_NAME !!BLD_VERSION!!-!!BLD_NUMBER!! Installation\n"

}

getAccountDetails() {

    local g u

    #
    #   Select default username
    #
    for u in nobody www-data Administrator 
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
    for g in nobody nogroup www-data Administrators
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

    echo "Enter requested information or press <ENTER> to accept the default. "
    
    #
    #   Confirm the configuration
    #
    finished=N
    while [ "$finished" = "N" ]
    do
        echo
        installbin=`yesno "Install binary package" "$installbin"`
        installdev=`yesno "Install development headers, samples and documentation package" "$installdev"`
    
        if [ "$installbin" = "Y" ] ; then
            runDaemon=`yesno "Start $BLD_PRODUCT automatically at system boot" $runDaemon`
            HTTP_PORT=`ask "Enter the HTTP port number" "$HTTP_PORT"`
            SSL_PORT=`ask "Enter the SSL port number" "$SSL_PORT"`
            username=`ask "Enter the user account for $BLD_PRODUCT" "$username"`
            groupname=`ask "Enter the user group for $BLD_PRODUCT" "$groupname"`
        else
            runDaemon=N
        fi
    
        echo -e "\nInstalling with this configuration:" 
        echo -e "    Install binary package: $installbin"
        echo -e "    Install development headers and samples package: $installdev"
    
        if [ "$installbin" = "Y" ] ; then
            echo -e "    Start automatically at system boot: $runDaemon"
            echo -e "    HTTP port number: $HTTP_PORT"
            echo -e "    SSL port number: $SSL_PORT"
            echo -e "    Username: $username"
            echo -e "    Groupname: $groupname"
        fi
        echo
        finished=`yesno "Accept this configuration" "Y"`
    done
    
    if [ "$installbin" = "N" -a "$installdev" = "N" ] ; then
        echo -e "\nNothing to install, exiting. "
        exit 0
    fi
    
    #
    #   Save the install settings. Remove.sh will need this
    #
    saveSetup
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
#   Get input from the user. Usage: ans=`ask "prompt" "default"`
#   Returns the answer or default if <ENTER> is pressed
#
ask() {
    local ans

    default=$2

    if [ "$!!BLD_PRODUCT!!_HEADLESS" = 1 ] ; then
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

    mkdir -p "$BLD_CFG_PREFIX"
    echo -e "FMT=$FMT\nbinDir=\"${BLD_PRD_PREFIX}\"\ninstallbin=$installbin\ninstalldev=$installdev\nrunDaemon=$runDaemon\nhttpPort=$HTTP_PORT\nsslPort=$SSL_PORT\nusername=$username\ngroupname=$groupname\nhostname=$HOSTNAME" \
        >"$BLD_PRD_PREFIX/install.conf"
}

#
#   Copy of the script in build/bin/patchAppwebConf
#
patchAppwebConf()
{
    conf="$1"
    ssl="$2"
    log="$3"
    doc="$4"
    docPrefix="${BLD_DOC_PREFIX}"
    
    if [ -f "$conf" ] ; then
        sed -e "
            s!^ServerRoot.*\".*!ServerRoot \"${BLD_CFG_PREFIX}\"!
            s!^DocumentRoot.*\".*!DocumentRoot \"${BLD_WEB_PREFIX}\"!
            s!^LoadModulePath.*\".*!LoadModulePath \"${BLD_LIB_PREFIX}\"!
            s!^Listen.*!Listen ${BLD_HTTP_PORT}!
            s!^User .*!User ${username}!
            s!^Group .*!Group ${groupname}!" <"$conf" >"$conf.new"
        [ $? = 0 ] && mv "$conf.new" "$conf"
    fi

    if [ -f "$ssl" ] ; then
        sed -e "
            s![     ][  ]*Listen.*!    Listen ${BLD_SSL_PORT}!
            s!DocumentRoot.*!DocumentRoot \"${BLD_WEB_PREFIX}\"!
            s!<VirtualHost .*!<VirtualHost *:${BLD_SSL_PORT}>!" <"$ssl" >"$ssl.new"
        [ $? = 0 ] && mv "$ssl.new" "$ssl"
    fi
    
    if [ -f "$log" ] ; then
        sed -e "
            s!ErrorLog.*error.log\"!ErrorLog \"${BLD_LOG_PREFIX}/error.log\"!
            s!CustomLog.*access.log\"!CustomLog \"${BLD_LOG_PREFIX}/access.log\"!" "$log" >"$log.new"
        [ $? = 0 ] && mv "$log.new" "$log"
    fi

    if [ -f "$doc" ] ; then
        sed -e "
            s![     ][  ]*Alias /doc/.*!    Alias /doc/ \"${docPrefix}\/\"!" <"$doc" >"$doc.new"
        [ $? = 0 ] && mv "$doc.new" "$doc"
    fi

    if [ `uname | sed 's/CYGWIN.*/CYGWIN/'` = CYGWIN ] ; then
        if which unix2dos >/dev/null 2>&1 ; then
            unix2dos "$conf" >/dev/null 2>&1
            unix2dos "$ssl" >/dev/null 2>&1
            unix2dos "$log" >/dev/null 2>&1
            unix2dos "$doc" >/dev/null 2>&1
        fi
    else
        for f in "$conf" "$ssl" "$log" "$doc" ; do
            if [ -f "$f" ] ; then
                sed -e "s/$//" <"$f" >"$f".new
                [ $? = 0 ] && mv "$f.new" "$f"
            fi
        done
    fi
}

#
#   Modify service. Usage: configureService [start|stop|install|remove]
#
configureService() {
    local action=$1

    case $action in
    start|stop)
        if [ $BLD_HOST_OS = WIN ] ; then
            if [ $action = start ] ; then
                if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
                    "$BLD_BIN_PREFIX/angel" --start $BLD_BIN_PREFIX/$BLD_PRODUCT
                fi
                if [ -x "$BLD_BIN_PREFIX/${BLD_PRODUCT}Monitor.exe" ] ; then
                    sleep 5
                    "$BLD_BIN_PREFIX/${BLD_PRODUCT}Monitor" &
                fi
            else
                if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
                    "$BLD_BIN_PREFIX/angel" --stop $BLD_BIN_PREFIX/$BLD_PRODUCT
                fi
                if [ -x "$BLD_BIN_PREFIX/${BLD_PRODUCT}Monitor.exe" ] ; then
                    "$BLD_BIN_PREFIX/${BLD_PRODUCT}Monitor" --stop
                fi
            fi
        elif which launchctl >/dev/null 2>&1 ; then
            local company=`echo $BLD_COMPANY | tr '[:upper:]' '[:lower:']`
            if [ $action = start ] ; then
                launchctl start com.${company}.${BLD_PRODUCT}
            else
                launchctl stop com.${company}.${BLD_PRODUCT} 2>/dev/null
            fi
        elif which service >/dev/null 2>&1 ; then
            service $BLD_PRODUCT $action >/dev/null 2>&1
        elif which invoke-rc.d >/dev/null 2>&1 ; then
            invoke-rc.d $BLD_PRODUCT $action
        fi
        ;;

    install)
        if [ $BLD_HOST_OS = WIN ] ; then
            if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
                "$BLD_BIN_PREFIX/angel" --install $BLD_BIN_PREFIX/$BLD_PRODUCT
            fi
        elif which launchctl >/dev/null 2>&1 ; then
            local company=`echo $BLD_COMPANY | tr '[:upper:]' '[:lower:']`
            launchctl unload /Library/LaunchDaemons/com.${company}.${BLD_PRODUCT}.plist 2>/dev/null
            launchctl load -w /Library/LaunchDaemons/com.${company}.${BLD_PRODUCT}.plist
        elif which chkconfig >/dev/null 2>&1 ; then
            chkconfig --add $BLD_PRODUCT >/dev/null
            chkconfig --level 5 $BLD_PRODUCT on >/dev/null

        elif which update-rc.d >/dev/null 2>&1 ; then
            update-rc.d $BLD_PRODUCT defaults 90 10 >/dev/null
        fi
        ;;

    remove)
        if [ $BLD_HOST_OS = WIN ] ; then
            if [ -x "$BLD_BIN_PREFIX/angel.exe" ] ; then
                "$BLD_BIN_PREFIX/angel" --uninstall $BLD_BIN_PREFIX/$BLD_PRODUCT
            fi
        elif which launchctl >/dev/null 2>&1 ; then
            local company=`echo $BLD_COMPANY | tr '[:upper:]' '[:lower:']`
            launchctl unload -w /Library/LaunchDaemons/com.${company}.${BLD_PRODUCT}.plist 2>/dev/null
        elif which chkconfig >/dev/null 2>&1 ; then
            chkconfig --del $BLD_PRODUCT >/dev/null
        elif which update-rc.d >/dev/null 2>&1 ; then
            update-rc.d -f $BLD_PRODUCT remove >/dev/null
        fi
        ;;
    esac
}

installFiles() {
    local dir pkg doins NAME upper

    echo -e "\nExtracting files ...\n"

    for pkg in bin dev src ; do
        
        doins=`eval echo \\$install${pkg}`
        if [ "$doins" = Y ] ; then
            upper=`echo $pkg | tr '[:lower:]' '[:upper:]'`
            suffix="-${pkg}"
            #
            #   RPM doesn't give enough control on error codes. So best to
            #   keep going. 
            #
            NAME=`createPackageName ${BLD_PRODUCT}${suffix}`.$FMT
            if [ "$FMT" = "rpm" ] ; then
                echo -e "rpm -Uhv $NAME"
                rpm -Uhv $HOME/$NAME
            elif [ "$FMT" = "deb" ] ; then
                echo -e "dpkg -i $NAME"
                dpkg -i $HOME/$NAME >/dev/null
            else
                echo tar xzf "$HOME/${NAME}" --strip-components 1 -P -C /
                tar xzf "$HOME/${NAME}" --strip-components 1 -P -C /
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
        echo -e "\nSetting file permissions ..."
        find "$BLD_CFG_PREFIX" -type d -exec chmod 755 "{}" \;
        find "$BLD_CFG_PREFIX" -type f -exec chmod g+r,o+r "{}" \; 
        find "$BLD_WEB_PREFIX" -type d -exec chmod 755 "{}" \;
        find "$BLD_WEB_PREFIX" -type f -exec chmod g+r,o+r "{}" \;
        mkdir -p "$BLD_CFG_PREFIX/logs"
        chmod 777 "$BLD_CFG_PREFIX/logs"
        chmod 755 "$BLD_WEB_PREFIX"
        find "$BLD_BIN_PREFIX" -name '*.dll' -exec chmod 755 "{}" \;
        find "$BLD_BIN_PREFIX" -name '*.exe' -exec chmod 755 "{}" \;
    fi
    #
    #   TODO - Temp for Ejscript
    #
    chmod 777 "$BLD_WEB_PREFIX/test"
    echo
}

patchConfiguration() {
    configureService stop >/dev/null 2>&1
    if [ ! -f $BLD_PRODUCT.conf -a -f "$BLD_CFG_PREFIX/new.conf" ] ; then
        cp "$BLD_CFG_PREFIX/new.conf" "$BLD_CFG_PREFIX/$BLD_PRODUCT.conf"
    fi
    BLD_CFG_PREFIX="$BLD_CFG_PREFIX" BLD_WEB_PREFIX="$BLD_WEB_PREFIX" BLD_DOC_PREFIX="$BLD_DOC_PREFIX" \
        BLD_LIB_PREFIX="$BLD_LIB_PREFIX" BLD_LOG_PREFIX="$BLD_LOG_PREFIX" \
        BLD_SERVER=$HOSTNAME BLD_HTTP_PORT=$HTTP_PORT BLD_SSL_PORT=$SSL_PORT \
        patchAppwebConf "${BLD_CFG_PREFIX}/appweb.conf" "${BLD_CFG_PREFIX}/conf/hosts/ssl-default.conf" \
        "${BLD_CFG_PREFIX}/conf/log.conf" "${BLD_CFG_PREFIX}/conf/doc.conf"
}

startService() {
    configureService install
    configureService start
}

startBrowser() {

    if [ "$!!BLD_PRODUCT!!_HEADLESS" = 1 ] ; then
        return
    fi
    #
    #   Conservative delay to allow appweb to start and initialize
    #
    sleep 5
    echo -e "\nStarting browser to view the $BLD_NAME Home Page."
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
    configureService stop >/dev/null 2>&1
    configureService remove >/dev/null 2>&1
fi
installFiles $FMT
if [ "$installbin" = "Y" ] ; then
    patchConfiguration
    if [ "$runDaemon" = "Y" ] ; then
        startService
        startBrowser
    fi
fi
echo -e "\n$BLD_NAME installation successful.\n"
