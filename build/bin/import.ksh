#!/bin/bash

#   import.ksh -- Import files from a local build source package
#
#   Usage:
#       import.ksh [--diff] [--dir dir] [--ignore xx] [--patch preserveFeature] [--src dir] [--sync] fileList
#
#   Copyright (c) Embedthis Software Inc, 2003-2010. All Rights Reserved.
#
#   This file is for internal use only when importing shared source code.
#

ppArgs=
program=${0}
verbose=0
home=`pwd`

log() {
    tag=$1
    shift
    [ "$verbose" = "1" ] && printf "%12s %s\n" "[$tag]" "$*"
}

warn() {
    tag=$1
    shift
    printf "%12s %s\n" "[$tag]" "$*"
}

usage()
{
    echo "usage: ${program} --patch preserveFeatures --verbose fileList"
    exit 2
}

#
#   Get product settings into the local environment
#
getBuildConfig()
{
    if [ -f ./buildConfig.h ]
    then
        cat ./buildConfig.h | grep '#define' | sed 's/ *#define. *//' | 
            sed 's/ /=/' | sed 's/"/'\''/g' >/tmp/importConfig$$.sh ; \
        . /tmp/importConfig$$.sh ; rm -f /tmp/importConfig$$.sh
    fi
}


#
#   Build the pp command string
#
buildPatch()
{
    echo -e "# Stripping features according to: \n  # " >/dev/tty

    set | grep BLD_FEATURE | egrep -v "$preserve" | while read e
    do
        echo -e '#  ' $e >/dev/tty
        feature=${e%%=*}
        value=${e##*=}
        if [ "$value" = "1" ]
        then
            echo -ne " -iD${feature}"
        else
            echo -ne " -U${feature}"
        fi
    done
    echo -ne " -UUNUSED -iDBLD_DEBUG"
    echo "" >/dev/tty
}


importFiles()
{
    [ "$verbose" = "1" ] && echo -e "\n#\n# Importing from ${SRC} ...\n#"
    shift

    cat $FILE_LIST | while read pat ; do
        cd $SRC
        find . -type f -path "$pat" | while read file ; do
            target=${file##$STRIP}

            mkdir -p `dirname ${DIR}/${target}`
            [ -f ${DIR}/${target} ] && chmod +w ${DIR}/${target}
            if [ "$patch" = "1" ] ; then
                eval pp $ppArgs $SRC/$file > ${DIR}/${target}
            else
                [ "$verbose" ] && log "Import" "${DIR}/$target"
                cp $SRC/$file ${DIR}/${target}
            fi
            if [ `uname` != "CYGWIN_NT-5.1" ] ; then
                chmod -w ${DIR}/$target
            fi
        done 
    done
}


diffFiles()
{
    [ "$verbose" = "1" ] && echo -e "\n#\n# Comparing from ${SRC} ...\n#"
    shift

    cat $FILE_LIST | while read pat ; do
        cd $SRC
        find . -type f -path "$pat" | while read file ; do
            target=${file##$STRIP}

            [ ! -f $DIR/$file ] && continue
            diff $SRC/$file ${DIR}/${target} >/tmp/importDiff.tmp
            if [ $? != 0 ] ; then
                echo MODIFIED $DIR/$target
                [ "$verbose" = 1 ] && cat /tmp/importDiff.tmp
            fi
        done
    done
}


syncFiles()
{
    local status

    log "Sync" ${FILE_LIST}

    shift

    cat $FILE_LIST | while read pat ; do
        cd $SRC
        find . -type f -path "$pat" | while read file ; do
            target=${file##$STRIP}
            if [ "$ignore" != "" -a "${file/$ignore/}" != "${file}" ] ; then
                echo "#    ignore $file"
                continue
            fi

            if [ ! -f ${DIR}/${target} ] ; then
                status=1
            else
                diff $file ${DIR}/${target} 2>&1 >/dev/null
                status=$?
            fi
            if [ $status != 0 ] ; then
                if [ ${DIR}/${target} -nt $SRC/$file ] ; then
                    warn "WARNING" "${DIR}/${target} is newer than $SRC/$file. Skipping"
                    continue
                fi
                mkdir -p `dirname ${DIR}/${target}`
                [ -f ${DIR}/${target} ] && chmod +w "${DIR}/${target}"
                if [ "$patch" = "1" ] ; then
                    eval pp $ppArgs $SRC/$file > "${DIR}/${target}"
                else
                    [ "$verbose" ] && echo -e "#  import ${SRC}/$file"
                    cp $SRC/$file ${DIR}/${target}
                fi
                if [ `uname` != "CYGWIN_NT-5.1" ] ; then
                    chmod -w ${DIR}/$target
                fi
            fi
        done
    done
}


#
#   Main
#
DIR=`pwd`
ignore=""
diff=0
import=0
patch=0
sync=0

while :
do
    case "$1" in 
    --diff)
        diff=1
        ;;
    --dir|-d)
        DIR="$2"
        shift
        ;;
    --ignore)
        ignore="$2"
        shift
        ;;
    --import)
        import=1
        ;;
    --patch|-p)
        patch=1
        preserve="$2"
        shift
        ;;
    --src)
        SRC="$2"
        shift
        ;;
    --strip)
        STRIP="$2"
        shift
        ;;
    --sync)
        sync=1
        ;;
    --verbose|-v)
        verbose=1
        ;;
    --) usage
        ;;
    *)  break
        ;;
    esac
    shift
done


if [ "$#" -ne 1 ] ; then
    usage
    exit 2
fi
if [ "$SRC" = "" ] ; then
    echo "Must provide a --src argument"
    exit 2
fi

SRC=`getpath -a "$SRC"`
DIR=`getpath -a "$DIR"`
FILE_LIST=$1

if [ ! -f "$FILE_LIST" ] ; then
    if [ ! -f "../$FILE_LIST" ] ;then
        echo "Can't find $FILE_LIST"
    fi
    FILE_LIST="../$FILE_LIST"
fi


getBuildConfig

if [ "$patch" = "1" ] ; then
    ppArgs=`buildPatch`
    echo -e "  # Using pp args: $ppArgs\n"
fi


if [ "$import" = "1" ] ; then
    importFiles
elif [ "$diff" = "1" ] ; then
    diffFiles
elif [ "$sync" = "1" ] ; then
    syncFiles
fi

if [ "$patch" = "1" ]
then
    echo PATCHING copyrights not supported
    exit 255
    echo -en "\n  #\n  # "
    echo -e "Patch copyrights ...\n  #"
    cat $FILE_LIST | incPatch -l copyrights
fi

