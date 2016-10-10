cd /home/baoxue/bin/mysql
chown -R baoxue:baoxue .
scripts/mysql_install_db --datadir=/home/baoxue/var/mysql/data --user=baoxue
chown -R root .
chown -R baoxue:baoxue /home/baoxue/var/mysql

sudo cp support-files/my-large.cnf /etc/my.cnf
sudo cp support-files/mysql.server /etc/init.d/mysqld
sudo chmod +x /etc/init.d/mysqld
sudo chkconfig --add mysqld
sudo chkconfig --list mysqld


vim /etc/my.cnf
	thread_concurrency = 4    #cpu的二倍, 在5.6版本中去掉了。
	datadir = /home/baoxue/var/mysql/data    #添加这一行，数据库位置。
	innodb_file_per_table = on  #独立的innodb表空间文件。

service mysqld start
ss -tnlp


vim /etc/profile.d/mysql.sh   
	export PATH=/home/baoxue/bin/mysql/bin:$PATH
. /etc/profile.d/mysql.sh
ln -s /home/baoxue/bin/mysql/include/mysql/ /usr/include/mysql
vim /etc/man.config 
	MANPATH /home/baoxue/bin/mysql/man
vim /etc/ld.so.conf.d/mysql-10.1.conf
	/home/baoxue/bin/mysql/lib
