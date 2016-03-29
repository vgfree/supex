local scan		= require('scan')
local only		= require('only')
local supex		= require('supex')
local rtpath		= require('rtpath')
local DataPackageModule = require("data_package")
local DataPackage	= DataPackageModule.DataPackage

module("rtpath_comp", package.seeall)


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
	--only.log('D',"data_pack" .. scan.dump(data_pack))
	rtpath.handle(data_pack)
end
