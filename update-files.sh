#!/bin/bash

#set -e
export DEVICE=kiwi
export VENDOR=huawei

if [ $# -eq 4 ]; then
  OLD_ROOT=$1
  OLD_RAMDISK=$2
  NEW_ROOT=$3
  NEW_RAMDISK=$4
else
  echo "$0: bad number of arguments"
  echo ""
  echo "usage: $0 OLD_ROOT OLD_RAMDISK NEW_ROOT NEW_RAMDISK"
  exit 1
fi

BASE=../../../vendor/$VENDOR/$DEVICE/proprietary

for FILE in `egrep -v '(^#|^$)' proprietary-files.txt`; do
  OLDIFS=$IFS IFS=":" PARSING_ARRAY=($FILE) IFS=$OLDIFS
  FILES=`echo ${PARSING_ARRAY[0]} | sed -e "s/^-//g"`
  FILE=${PARSING_ARRAY[1]}
  if [ -z $FILE ]
  then
    FILE=$FILES
  fi
  if [ "${FILE:0:6}" = "/sbin/" ]; then
    OLD_DIR=$OLD_RAMDISK
    NEW_DIR=$NEW_RAMDISK
  else
    OLD_DIR=$OLD_ROOT/system
    NEW_DIR=$NEW_ROOT/system
  fi
  if [ ! -f $OLD_DIR/$FILE ]; then
    echo "Not in old release: $FILE"
  elif [ ! -f $NEW_DIR/$FILE ]; then
    echo "Not in new release: $FILE"
  elif diff -q $OLD_DIR/$FILE $BASE/$FILE > /dev/null ; then
    cp $NEW_DIR/$FILE $BASE/$FILE
  else
    echo "Not from old release: $FILE"
  fi
done | sort
