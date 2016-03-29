
得到连续驾驶情景的接口文档
========================

### API编号

### 功能简介
* 对数据进行处理，判断是否属于连续驾驶情景，如果是则通过http请求把数据传给ptop

### 参数格式

### 传入参数

 参数             |参数说明         |  类型       |   示例         |是否允许为空|  
------------------|-----------------|-------------|----------------|------------|
 accountID        | 用户帐号编号    | string      |  aaaaaaaaaa    |否          | 
 direction        | 方向  	    | number	  |  0		   |否          | 
 GPSTime          | 时间	    | string      |  1231221	   |否          | 
 T_LONGDRI_MILEAGE| 连续驾驶特有属性| table       |  无		   |否          | 
 collect          |newStatus特有属性| string      |  true	   |否          | 
 speed            | 速度	    | string      |  100	   |否          |
 

### 返回结果参数

参数                | 参数说明
--------------------|-------------------------------------------
2                   | data_type,数据类型
0	            | useless_argument，多余的参数个数
actualMileage       | 实际驾驶里程
maxSpeed            | 最大速度
avgSpeed            | 平均速度
stopTime            | 停车时间


###返回结果示例：

* continuousDrivingCarryData: `{2:0:100:100:60:1231221}`


### 测试地址: 






