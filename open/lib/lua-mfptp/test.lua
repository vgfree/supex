local mfptp_pack = require("mfptppack")
function compose_mfptp_json_request(data)
	local data = {}
	local body_lenth = 0
	local bodyData
	data[1] = 3
	data[2] = "jizhong"
	data[3] = "zhoukai"
	data[4] = "zhoukai"
	local more = data[1]
	local oldlen = 0
	-->> 封装body头
        local headString,dst= mfptp_pack.data_head(1,2,1)
	for i=2 ,#data do
		more = more -1
		print("i:" .. i)
		print("more:" .. more)
		-->> 封装body数据 前几贞数据
		if more > 0 then
			oldlen ,bodyData= mfptp_pack.data_body(data[i], #data[i], more, body_lenth,dst)
			body_lenth = oldlen +body_lenth
			print("bodyData:" .. bodyData)
		end
	
		-->>封装最后一贞数据数据
		if more == 0 then
			print("1111111111111")
			body_lenth = body_lenth+#data[i]+i
			bodyData = mfptp_pack.data_body(data[i], #data[i], more, body_lenth,dst)
			print("data:" .. bodyData)
			return 0
		end
	end
end
compose_mfptp_json_request()
