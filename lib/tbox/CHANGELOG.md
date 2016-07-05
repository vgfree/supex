# Changelog

## master (unreleased)

### Bugs fixed

* Fix stdout compatibility issue for vs2015
* Fix process arguments length limit

## v1.5.2

### New features

* Add smallest configure option
* Add process operation interfaces

### Changes

* Improve envirnoment interfaces
* Modify xmake.lua for supporting xmake v2.x

### Bugs fixed

* Fix ltimer bug
* Fix asio memory leaks bug
* Fix asio httpd response bug on linux
* Fix path bug for windows

## v1.5.1

### New features

* Add automaticlly check libc interfaces
* Support custom allocator 
* Add trace for allocator in the debug mode
* Add `static_pool` module
* Add stream interfaces for reading all data to string
* Add adler32 hash algorithm
* Add `tb_memmem` interface
* Add regex module with pcre, pcre2 or posix regex 

### Changes

* Optimize stream and support read/write character device file
* Modify `tb_init` api and support allocator arguments
* Improve memory manager and use the allocator mode
* Redefine `assert` and will abort for debug mode 

### Bugs fixed

* Fix some bugs for android
* Fix seek bug for stream

# ������־

## master (������)

### Bugs�޸�

* �޸�stdout��vs2015���ϰ汾�ļ���������
* �޸����̲�����������

## v1.5.2

### ������

* ����smallest��������ѡ�ʵ��һ��������С�����룬����������չģ���������
* ���ӽ��̴����Ϳ��ƽӿ�

### �Ľ�

* ��ǿ�����������ýӿ�
* �޸�xmake.lua֧�����°�xmake v2.x, �򻯱�������

### Bugs�޸�

* �޸�ltimer��ʱ����׼����
* �޸�asio�����ڴ�й¶����
* �޸�asio/httpd��linux��keepaliveģʽ����Ӧ��������
* �޸�windows��·�������һЩbug

## v1.5.1

### ������

* �Զ��������ϵͳlibc�ӿڣ�����ʹ��ϵͳ�汾
* ֧���Զ����ڴ�������������ܹ���debugģʽ�£���ȡÿ�η���Ĵ���λ����Ϣ�������Զ���׷��
* ����������`static_pool`��ά������buffer���ڴ���䣬�ʺϾֲ��������ڴ棬pool��ȻҲ��ά�������ǵײ����`large_pool`���Ƚ����������ʺ�ȫ�ֹ����ڴ�
* ����stream���ٶ�ȡȫ�����ݵ�string�Ľӿ�
* ����adler32 hash�㷨
* ����`tb_memmem`�ӿ�
* ����pcre/pcre2/posix regexʵ��������ʽ��

### �Ľ�

* �Ż�stream��֧�ֶ��ַ��豸�ļ��Ķ�д
* �޸�`tb_init`�ӿڣ�����allocator�Զ����ڴ������������ʵ���û�������ʽ�ڴ����
* �ع��ڴ������ȫ���÷�����allocatorģʽ����������л��ڴ����֧��ԭ��ϵͳ�ڴ桢��̬buffer�ڴ桢�ڴ�صȸ��ַ��䷽ʽ
* �ض���assert��debugģʽ����assertֱ��abortִ��

### Bugs�޸�

* �޸�android�µ�һЩbug
* �޸�stream��seek����

## v1.5.0

### ������

* ���ӿ�ƽ̨�������������ӿ�

### �Ľ�

* �ؽ���������ܹ�������xmake��ƽ̨�Զ��������߽��й�������
* �Ż�.pkg�����������ƣ�֧��������ͽӿڵ��Զ���⣬���libc��libm����ʹ���Զ���⵽��ϵͳ��ӿ�ʵ�֣������ǰƽ̨û��ʵ����ʹ��tbox���Լ�ʵ�ְ汾��ʹ��������ܺͿ�ƽ̨�ԡ���
* ���ƺ��Ż�·���������������·��������·�����໥ת��

### Bugs�޸�

* �޸�strlcpy��һЩlibc�ӿڵ�ʵ��bug

## v1.4.8

### ������

* ����·�������ӿڣ�֧�����·��������·���໥ת��

### �Ľ�

* �ؽ�����makefile�ܹ�������*.pkg������ģʽģ�黯�Ե���������������������
* Ĭ�ϱ������ÿ����Զ�̽�⵱ǰƽ̨֧�ֵ���������ע���������������ǿ�ѡ�ģ����Ҫ��С�����룬������ȫ����
* �������ɵ����п��ͷ�ļ���Ҳ����װ�ɶ���*.pkg��ʽ�����㼯�ɵ���������ƽ̨��Ҳ����copy
* ��ǿobject·�������ӿڣ�֧��json, xml��·��������������ʵ��json�������ߣ�jcat
* ʵ��ͨ��ipaddr�ṹ��ͳһ�ӿڣ�ȫ��֧��ipv6/ipv4��stream/http��urlҲ��ȫ֧��ipv6��ʽ����
* ������hashΪ`hash_map`��������`hash_set`����

## v1.4.7

### �Ľ�

* ��ǿfixed16�������͵Ľӿڣ��Ż����ֽӿ����ܣ�����ģʽ�����Ӹ����������
* �Ż�����ƽ������ʵ�֣����Ӷ�64λ����ƽ�����Ŀ��ټ���

### Bugs�޸�

* �޸�string���ַ���bug
* �޸�windows��asio��һЩbug
* �޸�һЩ��������

## v1.4.7_rc1

### ������

* ����asioģ�飬֧�ָ����첽socket/file������֧���첽dns��ssl������polarssl/openssl����http
* ����http cookie֧�֣�����http�ͻ���Э��
* ����sql���ݿ�ģ�飬����sqlite3/mysql
* ����objectģ��
* ����min/max heap����������`list_entry`��`single_list_entry`��������������ʵ�֣���`list`��`single_list`��ͬ���ǣ�����Ҫά���ڲ��ڴ棬���Ҹ���������bloom_filter
* ����remove��walk��count��for�ȳ����㷨֧��
* �����̳߳ء���ʱ�����ź�������������atomic64�ȳ���ϵͳ����
* ����http��������http���桢http��������ʵ����demo

### �Ľ�

* �ع�streamģ�飬������`async_stream`��`async_transfer`��`transfer_pool`�������ԡ�
* �Ż�������libc��libm�Ľӿ�
* �ع������ڴ����ܹ��������ڴ����֧�֣��Ż��ڴ�ʹ�ú�Ч��

### Bugs�޸�

* �޸����Ż�xml����ģ��
