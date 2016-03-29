## lua-FLIF 项目说明

### 文件及目录说明
* convert.cpp convert.h 封装FLIF接口及头文件
* exapmle 接口使用示例
* FLIF-master.zip FLIF源码压缩包
* image 示例图片(测试示例图片源及生成图片目录)
* lua_api Lua接口封装及测试用例

### 功能说明
lua-FLIF 可以实现将PNG图片无损转换为FLIF，并支持逆向操作，转换后FLIF文件占用空间较小
例如：/image 目录中 miku.png 经转换为 miku.flif  格式后， 体积从2277015bit变为1348750bit

本程序可实现转换：
>	PNG -> FLIF 
>	FLIF -> PNG
>	FLIF -> FLIF

其中前两种转换为无损转换，最后一种转换暂时只可实现无损转换(即默认100%质量及1：1转换,有损压缩转换功能未添加)

### 测试用例
图片源及输出目录在/image
* /example 	gcc convert.c -L../ -lflif-interface
* /lua_api/test	luajit test.lua

### 依赖环境
* Lua 5.1 luajit-2.0
* 安装 libpng-devel
* 安装 libsdl2-dev (viewfile 查看图片)

### 使用方法
1. 安装依赖环境
2. 执行make && make install
3. 执行测试用例
