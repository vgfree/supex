
获取前方四公里poi情景的接口文档
========================

### API编号

### 功能简介
* 对数据进行处理，判断是否属于要获取的前方四公里poi情景，如果是则通过http请求把数据传给ptop

### 参数格式

### 传入参数

 参数             |参数说明         |  类型       |   示例         |是否允许为空|  
------------------|-----------------|-------------|----------------|------------|
 accountID        | 用户帐号编号    | string      |  aaaaaaaaaa    |否          | 
 longitude        | 经度	    | string      |  121.3605785   |否          | 
 latitude         | 纬度    	    | string      |  31.2239363	   |否          | 
 T_FRONT_ROAD_INFO| 前方道路信息    | table       |  无		   |否          | 
 T_FRONT_POI_SET  | 前方poi集合	    | table       |  无		   |否          | 
 collect          |newStatus特有属性| string      |  true	   |否          | 
 

### 返回结果参数

参数                | 参数说明
--------------------|-------------------------------------------
issued_poitype_table| 下发的poitype列表


###返回结果示例：

issued_poitype_table:'{
	        ["1123111"] = {
                	["1"] = {
                        	["longitude"] = 116.08883,
                        	["latitude"] = 43.93555,
                        	["receiverDirection"] = "[239,45]",
                        	["poiID"] = "P31702367",
                        	["receiverLatitude"] = 43.93555,
                        	["senderDirection"] = "[232]",
                        	["direction"] = 239,
                        	["receiverDistance"] = 200,
                        	["receiverLongitude"] = 116.08883,
                        	["senderLongitude"] = 116.0932518,
                        	["senderSpeed"] = "[0]",
                        	["level"] = 25,
                        	["senderLatitude"] = 43.9380915,
                        	["type"] = 1123111,
                        	["angle1"] = 239,
                        	["angle2"] = -1,
                        	["interval"] = 30,
                	},
                	["context"] = "前方有违章摄像头",
        		}
		}'
	

### 测试地址: 





