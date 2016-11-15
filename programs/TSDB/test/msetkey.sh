#!/bin/bash


i=300000000
while [ $i -gt 200000000 ]
do
	n=1000
	KV=""
        while [ $n -gt 0 ]
		do
		VALUES="ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		KV1=$VALUES"_"`date +%Y%m%d%H%M%S_%N`" "$VALUES
		n=$[ $n - 1]
	KV=$KV" "$KV1
	done
	#echo $KV >>kv.txt				
	/data/redis/bin/redis-cli -h 192.168.1.13 -p 6379 mset $KV  > /dev/null
i=$[ $i - 1000  ]
done
