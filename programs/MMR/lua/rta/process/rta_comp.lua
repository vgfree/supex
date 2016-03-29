local scan = require('scan')
local only = require('only')
local rta  = require('rta')
local supex = require('supex')
local DataPackageModule = require("data_package")
local DataPackage = DataPackageModule.DataPackage

module("rta_comp", package.seeall)


function handle()
	if (not supex.get_our_body_table()["collect"]) then
		return
	end
        --Get field from Body
        local req_body = supex.get_our_body_table()
	local data_pack = DataPackage:new(req_body['IMEI'], req_body['accountID'],req_body['tokenCode'])
	if not data_pack:init(req_body) then
		return 
	end
	rta.handle(data_pack)
end
