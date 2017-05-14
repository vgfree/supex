#!/bin/bash

SLEEP_TIME=20

while [ 1 ]
do
	EXIST=`ps aux | grep "./timport" | grep -v grep`
        if [ -z "$EXIST" ]
        then
                ./start.sh
                echo "[`date +%Y%m%d-%H:%M:%S`] restart..." >> restart.log
                SLEEP_TIME=3
        else
                SLEEP_TIME=20
        fi
        sleep $SLEEP_TIME
done
