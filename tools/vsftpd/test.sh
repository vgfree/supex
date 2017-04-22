
# 九.测试：
# 1.在虚拟用户目录中预先放入文件：
sudo touch /opt/vsftp/baoxue1/kc.test

# 2.从其他机器作为客户端登陆FTP：
sudo ftp
ftp> open 192.168.1.22
Connected to 192.168.1.22.
220 This Vsftp server supports virtual users ^_^
530 Please login with USER and PASS.
530 Please login with USER and PASS.
KERBEROS_V4 rejected as an authentication type
Name (192.168.1.22:root): baoxue1
331 Please specify the password.
Password: 123456
230 Login successful.
Remote system type is UNIX.
Using binary mode to transfer files.

3.测试列单操作
ftp> ls
227 Entering Passive Mode (192,168,1,22,220,24)
150 Here comes the directory listing.
-rw-r--r--    1 501      501             0 Sep 15 21:14 kc.test
226 Directory send OK.（目录列单成功）

4.测试上传操作：
ftp> put
(local-file) KC.repo
(remote-file) KC.repo
local: KC.repo remote: KC.repo
227 Entering Passive Mode (192,168,1,22,230,1)
150 Ok to send data.
226 File receive OK. （上传成功）
699 bytes sent in 0.024 seconds (29 Kbytes/s)
ftp>

5.测试建立目录操作：
ftp> mkdir test
257 "/opt/vsftp/baoxue1/test" created （目录建立成功）

6.测试下载操作：
ftp> get kc.test
local: kc.test remote: kc.test
227 Entering Passive Mode (192,168,1,22,164,178)
150 Opening BINARY mode data connection for kc.test (0 bytes).
226 File send OK.（下载成功）

7.测试超时：
ftp> dir
421 Timeout.（超时有效）
ftp> user
Not connected.注意:
在/etc/vsftpd/vsftpd.conf中，local_enable的选项必须打开为Yes，使得虚拟用户的访问成为可能，否则会出现以下现象：
----------------------------------
[root@KcentOS5 ~]# ftp
ftp> open 192.168.1.22
Connected to 192.168.1.22.
500 OOPS: vsftpd: both local and anonymous access disabled!
----------------------------------
原因：虚拟用户再丰富，其实也是基于它们的宿主用户overlord的，如果overlord这个虚拟用户的宿主被限制住了，那么虚拟用户也将受到限制。
补充：

500 OOPS:错误

有可能是你的vsftpd.con配置文件中有不能被实别的命令，还有一种可能是命令的YES 或 NO 后面有空格。

我遇到的是命令后面有空格。因为我是用GEDIT来编辑的配置文件

550 权限错误,不能创建目录和文件

解决方法: 关闭selinux

# vi /etc/selinux/config

将 SELINUX=XXX -->XXX 代表级别

改为

SELINUX=disabled

重启
