#!/bin/bash

HOST=192.168.1.10

SRC=/root/meng
DST=nginx

USER=backup

inotifywait -mrq --timefmt '%d/%m/%y %H:%M' --format '%T %w%f%e' -e modify,delete,create,attrib $SRC \
| while read files
	do
		rsync -vzrtopg --exclude=logs --delete --progress --password-file=/etc/server.pass $SRC/ $USER@$HOST::$DST
		echo "${files} was rsynced" >> /tmp/rsync.log 2>&1
	done
