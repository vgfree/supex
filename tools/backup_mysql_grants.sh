#!/bin/bash
#author:xiaodong meng
#date:20131031

DBUSER='dbbackup'
DBPWD='0bQWotu1XrnCR25m8xcg'
DATE=`date +%Y%m%d`
DB_PATH=`/sbin/ifconfig | grep "inet addr" | awk '{print $2}' | sed s/addr:// | grep "^10\|^172.16.11\|^192.168.1" | head -n 1 | sed  's#\.#_#g'`
LOCAL_PATH="/home/mysql/dbbackup"
MYSQLDUMP='/data/mysql/bin/mysqldump'
MYSQLcmd='/data/mysql/bin/mysql'
MySQLOPT='-R --single-transaction --master-data=2 --default-character-set=utf8'

expgrants()
{
 "$MYSQLcmd" -u"$DBUSER" -p"$DBPWD" -B -N $@ -e "SELECT DISTINCT CONCAT(
    'SHOW GRANTS FOR ''', user, '''@''', host, ''';'
    ) AS query FROM mysql.user" | \
 "$MYSQLcmd" -u"$DBUSER" -p"$DBPWD" $@ | \
  sed 's/\(GRANT .*\)/\1;/;s/^\(Grants for .*\)/-- \1 /;/--/{x;p;x;}'
}
expgrants > "$LOCAL_PATH"/grants_"$DATE".sql
echo -ne "\nflush privileges;\n" >> "$LOCAL_PATH"/grants_"$DATE".sql

md5sum "$LOCAL_PATH"/grants_"$DATE".sql > "$LOCAL_PATH"/grants_"$DATE".sql.MD5
