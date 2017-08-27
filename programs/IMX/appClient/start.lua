local CFG_LIST		= require('cfg')
local lualog		= require('lualog')
local only		= require('only')
local cutils		= require('cutils')
local utils		= require('utils')



local comm_send		= function (msg)
	_G.app_lua_send_message({msg})
end
local comm_recv		= function ()
	local msg = _G.app_lua_recv_message()[1]
	return msg
end

function app_init()
	--> init first
	lualog.setpath( CFG_LIST["DEFAULT_LOG_PATH"] )
	lualog.setlevel( CFG_LIST["LOGLV"] )

	lualog.open('access')
end

function app_hand_msg()
	while true do
		local msg = comm_recv()
		print("--------------", msg)
	end
end



function app_hand_ask()
	print("login [uid]")
	print("join [gid]")
	print("fsay [fid] [txt]")
	print("gsay [gid] [txt]")

	local uid
	while true do
		io.output():write("> ")
		local cmds = io.input():read("*l")
		local info = utils.str_split(cmds, ' ')
		if info or #info > 0 then
			local opt = info[1]
			--login
			if opt == "login" then
				uid = info[2]
				local msg = string.format('{"action":"login","accountID":"%s"}', uid)
				comm_send(msg)
			end
			--join
			if opt == "join" then
				local gid = info[2]
				local msg = string.format('{"action":"join","accountID":"%s","chatGroupID":"%s"}', uid, gid)
				comm_send(msg)
			end
			--groups
			if opt == "groups" then
				local gid = info[2]
				local msg = string.format('{"action":"groups","accountID":"%s"}', uid)
				comm_send(msg)
			end
			--chatAmigo
			if opt == "fsay" then
				local chatID = cutils.uuid()
				local fid = info[2]
				local txt = info[3]
				local msg = string.format('{"action":"chatAmigo","message":{"chatID":"%s","fromAccountID":"%s","toAccountID":"%s","content":"%s"}}', chatID, uid, fid, txt)
				comm_send(msg)
			end
			--chatGroup
			if opt == "gsay" then
				local chatID = cutils.uuid()
				local gid = info[2]
				local txt = info[3]
				local msg = string.format('{"action":"chatGroup","message":{"chatID":"%s","fromAccountID":"%s","chatGroupID":"%s","content":"%s"}}', chatID, uid, gid, txt)
				comm_send(msg)
			end
		end
	end
end
