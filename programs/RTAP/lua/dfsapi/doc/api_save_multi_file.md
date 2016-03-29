上传多文件
========================

### API编号
* 

### 功能简介
* 用于存储图片,多媒体文件,可批量上传

### 参数格式

* API以 **POST** 方式请求，且使用form表单方式提交,注意boundary

### 输入参数

 参数                               | 参数说明           | 类型    |   示例        | 是否允许为空 | 限制条件
------------------------------------|--------------------|---------|---------------|--------------|---------------------------
 appKey                             | 应用标识           | string  | 1111111111    | 否           | 长度不大于10
 sign                               | 安全签名           | string  | 无            | 否           | 长度为40 ( 计算签名只需要lenght,appKey,fileType)
 fileCount                          | 文件数量           | number  | 2             | 否           |  无
 fileTotalSize                      | 文件总大小         | number  | 3020          | 否           | 小于8M


### 示例代码/dfsapi/v2/saveMultifile

    POST /dfsapi/v2/saveMultifile HTTP/1.1
    Host: api.daoke.io:80
    Content-Length: 13085
    Content-Type: multipart/form-data; boundary=----WebKitFormBoundarymiA5o7JWrgF9Klcq

    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="appkey"

    1111111111

    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="fileCount"
    3
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="sign"

    4A61DE33C4078F80D00DFCBCC5AA25BACE1C9E67
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data; name="fileTotalSize"

    123124
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq
    Content-Disposition: form-data;name="mmFile";fileName="222.jpg";
    Content-Type: image/jpeg

    二进制文件内容
    Content-Disposition: form-data;name="mmFile";fileName="223.jpg";
    Content-Type: image/jpeg

    二进制文件内容
    ------WebKitFormBoundarymiA5o7JWrgF9Klcq



### 返回body示例

* 成功:  `{"ERRORCODE":"0", "RESULT":{"Totalcount":"2","successfulCount":2,"successFile":[{"fileID":"73df0a34b9be11e5a6e3000c29c75af0","fileUrl":"http://192.168.71.84:2222/dfsapi/v2/gainSound?group=dfsdb_1%26file=1452666334%3A73df0a34b9be11e5a6e3000c29c75af0.jpg","fileSize":5,"fileType":"jpg","fileName":"222.jpg"},{"fileID":"73df4f6cb9be11e5a6e3000c29c75af0","fileUrl":"http://192.168.71.84:2222/dfsapi/v2/gainSound?group=dfsdb_bak%26file=1452666334%3A73df4f6cb9be11e5a6e3000c29c75af0.jpg","fileSize":5,"fileType":"jpg","fileName":"223.jpg"}]}}`
* 失败: `{"ERRORCODE":"ME01002", "RESULT":"appKey error"}`


### 返回结果参数

 参数             | 参数说明
------------------|-------------------------------
Totalcount        |  总文件数              
successfulCount   |  成功的文件数             
fileID            |  文件标识ID   
fileName          |  文件名     
fileUrl           |  返回rul链接    
fileType          |  文件类型     
fileSize          |  文件大小

### 错误编码

 参数                 | 错误描述               | 解决办法     
----------------------|------------------------|---------------------------------------
 0                    | 调用成功               | 
 ME01002              | appKey error           | appKey需使用从语镜公司申请得到的appKey
 ME01019              | sign is not match      | 请阅读语镜公司提供的签名算法
 ME01023              | parameters is error    | 参数错误 请检查输入参数


### 测试地址: 192.168.71.84:2222/dfsapi/v2/saveMultiFile


