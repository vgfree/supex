#!/bin/sh
siege -c 1 -r 100 -T "application/json"  -q  'http://127.0.0.1:4170/releaseServerApply.json POST < data.txt'
