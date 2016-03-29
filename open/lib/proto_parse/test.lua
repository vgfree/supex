package.cpath = "./?.so;" .. package.cpath
local proto  = require('luaproto')

local function handle()
	local str1 = "hello"
	local str2 = "0100101010a***bc"
	local str3 = "21-00-233sdjksekfk**^&^^%"
	local str4 = 10;

	local head = proto.protoInit()
	proto.protoAppend(head, str1)
	proto.protoAppend(head, str2)
	proto.protoAppend(head, str3)
	proto.protoAppend(head, str4)
	
	local proto_str = proto.protoPack(head)
	proto.protoDestroy(head)
	
	local list = proto.protoInit()
	proto.protoParse(list, proto_str)
	proto.protoGet(list)
	
	proto.protoDestroy(list)
	proto.protoFree(proto_str)
end


do 
	handle()
end
	
		 
