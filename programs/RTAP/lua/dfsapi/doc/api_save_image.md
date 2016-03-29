
========================

### API编号
* 

### 功能简介
* 用于存储图片文件类型

### 参数格式

* API以 **POST** 方式请求，且使用form表单方式提交,注意boundary

### 输入参数

 参数                                 | 参数说明           | 类型      |   示例             | 是否允许为空 | 限制条件
--------------------------------------|--------------------|-----------|--------------------|--------------|---------------------------
 appKey    			      | 应用标识     	   | string    | 1111111111  	    | 否           | 长度不大于10
 length    			      | 文件长度   	   | number    | 3020    	    | 否           | 小于8M
 sign      			      | 安全签名     	   | string    | 无            	    | 否           | 长度为40 ( 计算签名只需要lenght,appKey)
 isStorage 			      | 是否存储到dfsdb    | string    | true	            | 是  	   | 不传或为空则默认false,表示存储到redis
 cacheTime 			      | 缓存时间   	   | number    | 60		    | 是	   | 不传或为空表示dfsdb或redis不过期
              

### 示例代码/dfsapi/v2/saveImage

    POST /dfsapi/v2/saveImage HTTP/1.1
    Host: image.mmfile.daoke.me
    Port: 80
    Content-Length: 13085
    Content-Type: multipart/form-data; boundary=----WebKitFormBoundarymiA5o7JWrgF9Klcq

    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="appkey"

    1111111111
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="length"

    1050
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="sign"

    4A61DE33C4078F80D00DFCBCC5AA25BACE1C9E67
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="mmfile"; filename="1.jpg"
    Content-Type: image/jpeg

    二进制文件内容
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq--


### 返回body示例

* 成功: `{"ERRORCODE":"0", "RESULT":{"url":"http://v1.mmfile.daoke.me/group1/M00/00/13/rBALdFM9BXiAKcAUAAW5LIgSrzk132.mp4","fileID":"b64f7158da8b11e3ae3b000c29b90439","fileSize":"143305"}}`
* 失败: `{"ERRORCODE":"ME01002", "RESULT":"appKey error"}`


### 返回结果参数

 参数   | 参数说明
----------|-------------------------------
 
url  	| 返回jpg链接
fileID  | 文件标识ID
Size  	| jpg文件大小

### 错误编码

 参数                 | 错误描述               | 解决办法     
----------------------|------------------------|---------------------------------------
 0                    | 调用成功               | 
 ME01002              | appKey error           | appKey需使用从语镜公司申请得到的appKey
 ME01019              | sign is not match      | 请阅读语镜公司提供的签名算法
 ME01023              | parameters is error    | 参数错误 请检查输入参数


### 测试地址: 192.168.71.84:2222/dfsapi/v2/saveImage
