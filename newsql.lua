
local write_list = {
	elementKey = {
		hashKey = (date +  elementKey)
		--hashKey = value
		typeKey = "set"
		cmd	= "srem/sadd"
		zkpKeysInfo = {}
	}


}

local read_list = {
	elementKey = {
		section = args[2]
		typeKey = "set"
		cmd	= "members"
		zkpKeysInfo = {}
		relationship = elementKey2[5]内部存放elementKey
	}


}


1.libkv引擎，数据多样化，统一数据模型,数据可以相互转化迁移
2.配置完成关系描述
3.zookper完成数据定位,智能导入和查询

列式的拆解可以按date分解/或分区的合集
