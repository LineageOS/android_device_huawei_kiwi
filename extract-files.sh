#!/bin/bash

#set -e
export DEVICE=kiwi
export VENDOR=huawei

if [ $# -eq 0 ]; then
  SRC=adb
else
  if [ $# -eq 2 ]; then
    SRC=$1
    RAMDISK=$2
    SRC2=$1
    RAMDISK2=$2
  elif [ $# -eq 4 ]; then
    SRC=$1
    RAMDISK=$2
    SRC2=$3
    RAMDISK2=$4
  else
    echo "$0: bad number of arguments"
    echo ""
    echo "usage: $0 [PATH_TO_EXPANDED_ROM RAMDISK_TO_EXPAND_FROM [ALT_EXPANDED ALT_RAMDISK]]"
    echo ""
    echo "If PATH_TO_EXPANDED_ROM is not specified, blobs will be extracted from"
    echo "the device using adb pull."
    exit 1
  fi
fi

BASE=../../../vendor/$VENDOR/$DEVICE/proprietary
rm -rf $BASE/*

for FILE in `egrep -v '(^#|^$)' proprietary-files.txt`; do
  OLDIFS=$IFS IFS=":" PARSING_ARRAY=($FILE) IFS=$OLDIFS
  FILE=`echo ${PARSING_ARRAY[0]} | sed -e "s/^-//g"`
  DEST=${PARSING_ARRAY[1]}
  if [ -z $DEST ]
  then
    DEST=$FILE
  fi
  if [ "${FILE:0:6}" = "/sbin/" ]; then
    DIR_PREFIX=""
    LOCAL_DIR=$RAMDISK
    LOCAL_DIR2=$RAMDISK2
  else
    DIR_PREFIX=/system/
    LOCAL_DIR=$SRC
    LOCAL_DIR2=$SRC2
  fi
  DIR=`dirname $DEST`
  if [ ! -d $BASE/$DIR ]; then
    mkdir -p $BASE/$DIR
  fi
  # Try CM target first
  if [ "$SRC" = "adb" ]; then
    adb pull $DIR_PREFIX/$DEST $BASE/$DEST
    # if file does not exist try OEM target
    if [ "$?" != "0" ]; then
        adb pull $DIR_PREFIX/$FILE $BASE/$DEST
        if [ "$?" != "0" ]; then
            echo "Failed to load: $FILE"
            exit 1
        fi
    fi
  else
    if [ -r $LOCAL_DIR/$DIR_PREFIX/$DEST ]; then
        cp $LOCAL_DIR/$DIR_PREFIX/$DEST $BASE/$DEST
    elif [ -r $LOCAL_DIR/$DIR_PREFIX/$FILE ]; then
        cp $LOCAL_DIR/$DIR_PREFIX/$FILE $BASE/$DEST
    elif [ -r $LOCAL_DIR2/$DIR_PREFIX/$DEST ]; then
        cp $LOCAL_DIR2/$DIR_PREFIX/$DEST $BASE/$DEST
    elif [ -r $LOCAL_DIR2/$DIR_PREFIX/$FILE ]; then
        cp $LOCAL_DIR2/$DIR_PREFIX/$FILE $BASE/$DEST
    else
        echo "error: missing file $FILE:$DEST"
        exit 1
    fi
  fi
done

./setup-makefiles.sh
