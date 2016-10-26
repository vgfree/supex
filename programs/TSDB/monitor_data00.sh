#!/bin/bash

TSDB_PIDFILE=var/tsdb.pid

SLEEP_TIME=20

while [ 1 ]
do
        TSDB_EXIST=`netstat -ano | grep 7601 | grep LISTEN`
        if [ -z "$TSDB_EXIST" ]
        then
                if [ -f $TSDB_PIDFILE ]
                then
                        rm -rf $TSDB_PIDFILE
                fi
                make run
                echo "[`date +%Y%m%d-%H:%M:%S`] restart..." >> restart.log
                SLEEP_TIME=3
        else
                SLEEP_TIME=10
        fi
        sleep $SLEEP_TIME
done
