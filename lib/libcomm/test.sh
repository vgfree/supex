#!/bin/bash

circle1=1
while (( $circle1<=10))
do
	./client 127.0.0.1 10003 &
	echo $circle1
	circle1=`expr $circle1 + 1`
done
