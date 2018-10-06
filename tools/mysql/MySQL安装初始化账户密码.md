MySQL安装初始化账户密码
2016年03月16日 16:37:58
阅读数：10586

MySQL登录的命令是mysql， mysql 的使用语法如下： mysql [-u username] [-h host] [-p[password]] [dbname] username 与 password 分别是 MySQL用户名与密码，mysql的初始管理帐号是root，没有密码，注意：这个root用户不是Linux的系统用户。MySQL默认用户是 root，由于初始没有密码，第一次进时只需键入mysql即可。

[root@test1 local]# mysql 
Welcome to the MySQL monitor. Commands end with ; or g. 
Your MySQL connection id is 1 to server version: 4.0.16-standard 
Type 'help;' or 'h' for help. Type 'c' to clear the buffer. 
mysql> 



出现了“mysql>”提示符，恭喜你，安装成功!

增加了密码后的登录格式如下：

mysql -u root -p 
Enter password: (输入密码) 



其中-u后跟的是用户名，-p要求输入密码，回车后在输入密码处输入密码。

可是我输入mysql却出错了：
初始密码是空的，我输入的也是空的还是错误!

ERROR 1045 (28000): Access denied for user 'root'@'localhost' (using password: NO) 



MySQL安装解决方法：重改密码

# /etc/init.d/mysqld stop 
# mysqld_safe --user=mysql --skip-grant-tables --skip-networking & 
# mysql -u root mysql 
mysql> UPDATE user SET Password=PASSWORD('newpassword') where USER='root'; 
mysql> FLUSH PRIVILEGES; 
mysql> quit 
# /etc/init.d/mysqld restart 
# mysql -uroot -p 
Enter password: 
mysql>搞定!



查看MYSQL数据库中所有用户

mysql> SELECT DISTINCT CONCAT('User: ''',user,'''@''',host,''';') AS query FROM mysql.user;
