
文本转amr音频文件
========================

### API编号
* 

### 功能简介
* 用于文本转amr音频文件

### 参数格式

* API以 **POST** 方式请求，且传送方式为 **key-value键值对**.

### 输入参数

 参数                       | 参数说明           | 类型      |   示例             | 是否允许为空 | 限制条件
------------------------ ---|--------------------|-----------|--------------------|--------------|---------------------------
 appKey                     | 应用标识           | string    | 1111111111         | 否           | 长度不大于10
 sign                       | 安全签名           | string    | 无                 | 否           | 长度为40
 text                       | 待转的文本         | string    | 上海语镜汽车       | 否           | 长度< 300 (计算签名使用原始文本,之后转换为url编码POST)
 isStorage                  | 是否固化           | string    | true／false        | 是           | 长度大于0
 cacheTime                  | 过期时间           | string    | 600                | 是           | 单位为秒
 policy                     | 转换接口           | string    | 无                 | 是           | speech/unisound/lytts
 
# speech:思必驰
# unisound:云知声
# lytts:灵云tts

### wstxt:微软第三方接口tts 暂停使用



### 示例代码/spx_txt_to_voice

    POST /spx_txt_to_voice HTTP/1.0
    Host:api.daoke.io:80
    Content-Length:153
    Content-Type:application/x-www-form-urlencoded

    sign=58B0505FB67660E6CC69D212F03A837162287647&appKey=11111111111&text=%e9%81%93%e5%ae%a2%2c%e9%82%80%e8%af%b7%e6%82%a8%e5%8a%a0%e5%85%a5%e8%bd%a6%e9%98%9f


### 返回body示例

* 成功: `{"ERRORCODE":"0", "RESULT":{"url":"http://v1.mmfile.daoke.me/dfsapi/v2/gainSound?group=group_1&file=1449918331:42549fcca0c011e5a58900505697e0db.amr","fileID":"b64f7158da8b11e3ae3b000c29b90439","fileSize":"143305"}}`
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



### 测试地址: 172.16.51.58/spx_txt_to_voice


