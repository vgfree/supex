local cjson = require("cjson")

for i = 1, #msg do
	print(i, msg[i], string.len(msg[i]))
end

if (msg[1] == "status") then
	if (msg[2] == "connected") then
		print("第三帧cid : " .. msg[3])
		get_uid(msg[3])
	end

elseif (msg[1] == "upstream") then
	if (msg[3] == "bind") then
		local json_tab = cjson.decode(msg[4])
		local msg_tab = {}
		msg_tab[1] = "setting"
		msg_tab[2] = "uidmap"
		msg_tab[3] = msg[2]
		print(msg_tab[3])
		msg_tab[4] = json_tab["uid"]
		print(json_tab["uid"])
		set_uidmap(msg_tab)
	else
		local msg_tab = {}
		msg_tab[1] = "downstream"
		msg_tab[2] = msg[2]
		print(msg_tab[2])
		msg_tab[3] = msg[3]
		print(msg_tab[3])
		send_msg(msg_tab)
	end
end

return 1
