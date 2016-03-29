--版权声明：无
--文件名称：realtime_comp.lua
--创建者  ：耿玄玄
--创建日期：2015-06-27
--文件描述：实时里程与途径计算入口
--修    改：2015-06-27 重构实时里程

local only = require('only')
local supex = require('supex')
local DataPackageModule = require("data_package")
local DataPackage = DataPackageModule.DataPackage

module('rtmiles_comp', package.seeall)

--名称：handle
--功能：入口函数
--参数：从pool中获取http请求体
--返回：无
--修改：2015-06-27 重构实时里程
function handle()
	if (not supex.get_our_body_table()["collect"]) then
		return
	end
        --Get field from Body
        only.log('D', 'handle begin')
        local req_body = supex.get_our_body_table()

        local data_pkg = DataPackage:new()
        local ok = data_pkg:init(req_body)
        data_pkg:process()

        only.log('D', 'handle end')
end
