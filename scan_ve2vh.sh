#!/bin/bash

kbscan="32 64 128 256 512"
mbscan="1 2 4 8 16 32 64 128 256 512 1024"

scan=$kbscan
for m in $mbscan; do
    scan="$scan "$((m*1024))
done

export LC_ALL=C
NPAR=${NPAR:-1}
HUGEARG=""
if [ "$HUGE" = "1" ]; then
    HUGEARG="-H"
fi
printf "%8s   %7s\n" "buff kb" "BW MiB/s"
for s in $scan; do
    bw=`./ve2vh -s $s -p $NPAR $HUGEARG 2>&1 | grep Total | egrep -o '[0-9\.]*'`
    printf "%8d   %7.0f\n" $s $bw
done

