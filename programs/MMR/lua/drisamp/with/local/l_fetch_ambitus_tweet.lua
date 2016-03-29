-- auth: baoxue
-- time: 2014.04.29

local utils 		= require('utils')
local only 		= require('only')
local redis_api 	= require('redis_pool_api')
local mysql_api 	= require('mysql_pool_api')
local cjson 		= require('cjson')
local link 		= require('link')
local weibo 		= require('weibo')
local supex 		= require('supex')

module('l_fetch_ambitus_tweet', package.seeall)

function bind()
	return '["collect", "accountID", "tokenCode"]'
end

function match()
	return true
end

function work()
	-->> check if has weibo not play
	local accountID = supex.get_our_body_table()["accountID"]
	local tokenCode = supex.get_our_body_table()["tokenCode"]
	local ok,card = redis_api.cmd("weibo", accountID, "zcard", accountID .. ':weiboPriority')
	if not ok then
		only.log('E','weibo redis failed!')
		return
	end
	if card and card > 0 then
		only.log('I', card)
		return
	end
	-->> fetch one weibo from map
	local ok,info = redis_api.cmd('mapFrontPosition', accountID, 'hget', accountID .. ':personalPOI', "1221000")
	if not ok then
		only.log('E','mapFrontPosition redis failed!')
		return
	end
	--info = '[{"positionType":"1221000","positionID":"60081592ca9011e3a0d9002219522239","geometryType":"2","appKey":"1957006319","accountID":"0w2psmawwj","longitude":"121.3605785","latitude":"31.2239363","content":"http://www.daoke.me/785412635211235____0w2psmawwj___1398220841310686208__recoding_2014_04_03_14_33_36.amr","title":"道客的吐槽"}]'
	if not info then return end
	-->> save one weibo record
	local ok,jo = pcall(cjson.decode, info)
	if not ok then return end
	--local keyct = tokenCode .. ":bullshitSet"
	local have = false
	local content = nil
	for i=1,#(jo or {}) do
		content = jo[i]["content"]
		if content then
			local ok,val = redis_api.cmd("private", accountID, "sismember", accountID .. ":bullshitSet", content)
			if not ok then
				only.log('E','private redis failed!')
				return
			end
			if not val then
				redis_api.cmd("private", accountID, "sadd", accountID .. ":bullshitSet", content)
				--redis_api.cmd("private",supex.get_our_body_table()["accountID"], "expire", keyct, 60*60*24*2)
				have = true
				break
			end
		end
	end
	-->> send one personal weibo
	if have then
		local wb = {
			multimediaURL = content,
			receiverAccountID = accountID,
			level = 25,
			interval = 60*5,
			senderType = 2,
		}

		local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
		local ok,err = weibo.send_weibo( server, "personal", wb, "2031222080", "382D7959C14D6EFD111A82530286CF868A58D94E" )
		if not ok then
			only.log("E", "send weibo failed : " .. err)
		end
	end
end
