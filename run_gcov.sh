#! /bin/bash
EXECUTABLE_NAME=$1
shift
FILELIST=
for file in "$@"; do
    FILELIST="$FILELIST ${EXECUTABLE_NAME}-${file}"
done
gcov $FILELIST

