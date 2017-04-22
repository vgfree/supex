# 二.系统帐户
# 1.建立Vsftpd服务的宿主用户：
# 默认的Vsftpd的服务宿主用户是root，但是这不符合安全性的需要。这里建立名字为vsftpd的用户，用他来作为支持Vsftpd的服务宿主用户。由于该用户仅用来支持Vsftpd服务用，因此没有许可他登陆系统的必要，并设定他为不能登陆系统的用户。
sudo useradd vsftpd -s /sbin/nologin
# 2.建立Vsftpd虚拟宿主用户：
# 本篇主要是介绍Vsftp的虚拟用户，虚拟用户并不是系统用户，也就是说这些FTP的用户在系统中是不存在的。
# 他们的总体权限其实是集中寄托在一个在系统中的某一个用户身上的，所谓Vsftpd的虚拟宿主用户，就是这样一个支持着所有虚拟用户的宿主用户。
# 由于他支撑了FTP的所有虚拟的用户，那么他本身的权限将会影响着这些虚拟的用户，
# 因此，处于安全性的考虑，也要非分注意对该用户的权限的控制，该用户也绝对没有登陆系统的必要，这里也设定他为不能登陆系统的用户。
#（这里插一句：原本在建立上面两个用户的时候，想连用户主路径也不打算给的。
# 本来想加上 -d /home/nowhere 的，据man useradd手册上讲述：
# “       -d, --home HOME_DIR
# The new user will be created using HOME_DIR as the value for the
# user's login directory. The default is to append the LOGIN name to
# BASE_DIR and use that as the login directory name. The directory
# HOME_DIR does not have to exist but will not be created if it is
# missing.
# 使用-d参数指定用户的主目录，用户主目录并不是必须存在的。如果没有存在指定的目录的话，那么它将不会被建立”。
sudo useradd overlord -s /sbin/nologin

# 三.调整Vsftpd的配置文件：
# 1.编辑配置文件前先备份
sudo cp /etc/vsftpd/vsftpd.conf /etc/vsftpd/vsftpd.conf.backup
# 2.编辑主配置文件Vsftpd.conf
sudo cp vsftpd.conf /etc/vsftpd/vsftpd.conf
# 3.建立Vsftpd的日志文件，并更该属主为Vsftpd的服务宿主用户：
sudo touch /var/log/vsftpd.log
sudo chown vsftpd.vsftpd /var/log/vsftpd.log
sudo chmod a=rw /var/log/vsftpd.log
# 4.建立虚拟用户配置文件存放路径：
sudo mkdir /etc/vsftpd/vconf/


# 四.制作虚拟用户数据库文件
# 1.先建立虚拟用户名单文件：
# 建立了一个虚拟用户名单文件，这个文件就是来记录vsftpd虚拟用户的用户名和口令的数据文件，我这里给它命名为virtusers。为了避免文件的混乱，我把这个名单文件就放置在/etc/vsftpd/下。
sudo touch /etc/vsftpd/virtusers
sudo chmod a=rw /etc/vsftpd/virtusers
# 2.编辑虚拟用户名单文件：
# 编辑这个虚拟用户名单文件，在其中加入用户的用户名和口令信息。格式很简单：“一行用户名，一行口令”。
sudo echo -e \
"baoxue1
123456
baoxue2
123456
baoxue3
123456" > /etc/vsftpd/virtusers
# 3.生成虚拟用户数据文件：
# 察看db4的db_load命令使用方法：
# usage: db_load [-nTV] [-c name=value] [-f file]
# [-h home] [-P password] [-t btree | hash | recno | queue] db_file
# usage: db_load -r lsn | fileid [-h home] [-P password] db_file
# 解释在本篇中，db_load命令几个相关选项很参数-T
# The -T option allows non-Berkeley DB applications to easily load text files into databases.
# If the database to be created is of type Btree or Hash, or the keyword keys is specified as set, the input must be paired lines of text, where the first line of the pair is the key item, and the second line of the pair is its corresponding data item. If the database to be created is of type Queue or Recno and the keywork keys is not set, the input must be lines of text, where each line is a new data item for the database.
# 选项-T允许应用程序能够将文本文件转译载入进数据库。由于我们之后是将虚拟用户的信息以文件方式存储在文件里的，为了让Vsftpd这个应用程序能够通过文本来载入用户数据，必须要使用这个选项。If the -T option is specified, the underlying access method type must be specified using the -t option.
# 如果指定了选项-T，那么一定要追跟子选项-t-t
# Specify the underlying access method. If no -t option is specified, the database will be loaded into a database of the same type as was dumped; for example, a Hash database will be created if a Hash database was dumped.
# Btree and Hash databases may be converted from one to the other. Queue and Recno databases may be converted from one to the other. If the -k option was specified on the call to db_dump then Queue and Recno databases may be converted to Btree or Hash, with the key being the integer record number.
# 子选项-t，追加在在-T选项后，用来指定转译载入的数据库类型。扩展介绍下，-t可以指定的数据类型有Btree、Hash、Queue和Recon数据库。这里，接下来我们需要指定的是Hash型。
# 需要特别注意的是，以后再要添加虚拟用户的时候，只需要按照“一行用户名，一行口令”的格式将新用户名和口令添加进虚拟用户名单文件。但是光这样做还不够，不会生效的哦！还要再执行一遍“ db_load -T -t hash -f 虚拟用户名单文件 虚拟用户数据库文件.db ”的命令使其生效才可以！
sudo db_load
sudo db_load -T -t hash -f /etc/vsftpd/virtusers /etc/vsftpd/virtusers.db
# 4.察看生成的虚拟用户数据文件
sudo ls -l /etc/vsftpd/virtusers.db


# 五.设定PAM验证文件，并指定虚拟用户数据库文件进行读取
# 1.察看原来的Vsftp的PAM验证配置文件：
sudo cat /etc/pam.d/vsftpd
# 2.在编辑前做好备份：
sudo cp /etc/pam.d/vsftpd /etc/pam.d/vsftpd.backup
# 3.编辑Vsftpd的PAM验证配置文件
sudo cp vsftpd /etc/pam.d/vsftpd


# 六.虚拟用户的配置
# 1.规划好虚拟用户的主路径：
sudo mkdir /opt/vsftp/
# 2.建立测试用户的FTP用户目录：
sudo mkdir /opt/vsftp/baoxue1/ /opt/vsftp/baoxue2/ /opt/vsftp/baoxue3/
# 3.更改虚拟用户的主目录的属主为虚拟宿主用户：
sudo chown -R overlord.overlord /opt/vsftp/
# 4.检查权限：
sudo ls -l /opt/vsftp/
# 5.建立虚拟用户配置文件模版：
sudo touch /etc/vsftpd/vconf/vconf.tmp
# 6.定制虚拟用户模版配置文件：
# 模版文件不起作用效果。
# 这里将原vsftpd.conf配置文件经过简化后保存作为虚拟用户配置文件的模版。这里将并不需要指定太多的配置内容，主要的框架和限制交由 Vsftpd的主配置文件vsftpd.conf来定义，即虚拟用户配置文件当中没有提到的配置项目将参考主配置文件中的设定。而在这里作为虚拟用户的配置文件模版只需要留一些和用户流量控制，访问方式控制的配置项目就可以了。这里的关键项是local_root这个配置，用来指定这个虚拟用户的FTP主路径。
sudo cp vconf.tmp /etc/vsftpd/vconf/vconf.tmp


# 七.给测试用户定制：
# 1.从虚拟用户模版配置文件复制：
sudo cp /etc/vsftpd/vconf/vconf.tmp /etc/vsftpd/vconf/baoxue1
sudo chmod a=rw /etc/vsftpd/vconf/baoxue1
# 2.针对具体用户进行定制：
sudo echo -e \
"local_root=/opt/vsftp/baoxue1
anonymous_enable=NO
write_enable=YES
local_umask=022
anon_upload_enable=NO
anon_mkdir_write_enable=NO
idle_session_timeout=300
data_connection_timeout=90
max_clients=1
max_per_ip=1
local_max_rate=25000" > /etc/vsftpd/vconf/baoxue1


# 八.启动服务：
sudo service vsftpd start
sudo netstat -ntpl | grep 21
