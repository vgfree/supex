local only = require('only')
local supex = require('supex')

module('sniff', package.seeall)


function handle()
	if supex.get_our_body_table() and supex.get_our_body_table()["APPLY"] and supex.get_our_body_table()["TIME"] then
		local mode = supex.get_our_body_table()["MODE"]
		local mode_list = {
			["push"] = 1,	--for lua VM
			["pull"] = 2,	--for lua VM

			["insert"] = 3,	--for task list
			["update"] = 4,	--for task list
			["select"] = 5,	--for task list
		}
		print("---------------")
		print(mode, mode_list[ mode ])

		if mode_list[ mode ] then
		print("+++++++++++++++++")
			local ok = supex.diffuse("/" .. supex.get_our_body_table()["APPLY"], supex.get_our_body_data(), supex.get_our_body_table()["TIME"], mode_list[ mode ])
			if not ok then
				only.log("E", "forward msmq msg failed!")
			end
		end
	end
end

