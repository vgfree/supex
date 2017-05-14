#!/bin/bash

SLEEP_TIME=20

while [ 1 ]
do
	RESTARTED=0
	i=0
	while [ "$i" -lt "6" ]
	do
		EXIST=`ps aux | grep "../timport_00$i/timport" | grep -v grep`
	      	if [ -z "$EXIST" ]
        	then
			cd timport_00$i
                	./start.sh
			cd -
                	echo "[`date +%Y%m%d-%H:%M:%S`] restart timport_00$i..." >> restart.log
			RESTARTED=1
		fi
		(( i+= 1 ))
	done
	if [ "$RESTARTED" -eq "1" ]
	then
                SLEEP_TIME=3
        else
                SLEEP_TIME=20
        fi
        sleep $SLEEP_TIME
done
