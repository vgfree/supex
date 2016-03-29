
图片加水印
=====================================

### API编号
*

### 功能简介
* 给图片添加文字和图片水印

### 参数格式

* API以 **http**方式请求,且传送方式为 **key-value键值对**.

### 输入参数

参数                        | 参数说明           | 类型    |   示例             			| 是否允许为空 | 限制条件
----------------------------|--------------------|---------|--------------------------------------------|--------------|---------------------------
 appKey                     | 应用标识           | string  | 1111111111         			| 否           | 长度不大于10
 sign                       | 安全签名           | string  | 无                 			| 否           | 长度为40 
 length    	 	    | 文件长度   	 | number  | 3020    	    				| 否           | 小于8M
 annotation                 | 文字水印文本       | string  | 上海语镜汽车\n上海道客 			| 是           | 不传或为空则不添加文字
 color	                    | 文字颜色	         | string  | red/green/white	 			| 是           | 不传或为空则默认为黑色
 font	                    | 字体	         | bumber  | 1			 			| 是           | 不传或为空则默认字体
 logoName                   | 添加logo的序号     | string  | 1      					| 是           | 不传或为空则添加默认水印
 locationX                  | logo添加位置的x坐标| string  | 123       					| 是           | 不传或为空则添加默认坐标
 locationY                  | logo添加位置的y坐标| string  | 150       					| 是           | 不传或为空则添加默认坐标
 isStorage     	            | 是否存储到dfsdb    | string  | true	            			| 是  	       | 不传或为空则默认false,表示存储到redis
 cacheTime 	            | 缓存时间   	 | number  | 60		    				| 是	       | 不传或为空表示dfsdb或redis不过期


### 示例代码/dfsapi/v2/picAppendWatermark

    POST /dfsapi/v2/picAppendWatermark HTTP/1.0
    Host:api.daoke.io:80
    Content-Length:153
    Content-Type:application/x-www-form-urlencoded

    sign=58B0505FB67660E6CC69D212F03A837162287647&appKey=1111111111&srcURL=%e9%8%7%e6%ae%a2%2c%e9%82%80%e8%hgbf%b7%e6%82%a8%e5%8a%a0%e5%522%a5250%e8%bd%a6%e9%98%9f


### 返回body示例

* 成功: `{"ERRORCODE":"0", "RESULT":{"url":"http://m9demo.daoke.me/group1/M00/1E/84/wKgBvlYh81iARg-eAAAGSjrczgg372.png","fileID":"b86c024a3aa911e5bcb500266cf02f24","fileSize":"6488"}}`
* 失败: `{"ERRORCODE":"ME01002", "RESULT":"appKey error"}`


### 返回结果参数

 参数   | 参数说明
----------|-------------------------------
 url      | 返回url链接
 fileID   | 文件标识ID
 fileSize | 文件大小

### 错误编码

 参数                 | 错误描述               | 解决办法     
----------------------|------------------------|---------------------------------------
 0                    | 调用成功               | 
 ME01002              | appKey error           | appKey需使用从语镜公司申请得到的appKey
 ME01019              | sign is not match      | 请阅读语镜公司提供的签名算法
 ME01023              | parameters is error    | 参数错误 请检查输入参数


### 测试地址: 192.168.71.84:2222/dfsapi/v2/picAppendWatermark

