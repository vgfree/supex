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
	print("amigos")
	print("groups")
	print("ajoin [aid]")
	print("gjoin [gid]")
	print("asay [aid] [txt]")
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
			--ajoin
			if opt == "ajoin" then
				local aid = info[2]
				local msg = string.format('{"action":"amigoJoin","accountID":"%s","chatAmigoID":"%s"}', uid, aid)
				comm_send(msg)
			end
			--gjoin
			if opt == "gjoin" then
				local gid = info[2]
				local msg = string.format('{"action":"groupJoin","accountID":"%s","chatGroupID":"%s"}', uid, gid)
				comm_send(msg)
			end
			--amigos
			if opt == "amigos" then
				local msg = string.format('{"action":"amigos","accountID":"%s"}', uid)
				comm_send(msg)
			end
			--groups
			if opt == "groups" then
				local msg = string.format('{"action":"groups","accountID":"%s"}', uid)
				comm_send(msg)
			end
			--chatAmigo
			if opt == "asay" then
				local chatID = cutils.uuid()
				local aid = info[2]
				local txt = info[3]
				local msg = string.format('{"action":"chatAmigo","message":{"chatID":"%s","fromAccountID":"%s","chatAmigoID":"%s","content":"%s"}}', chatID, uid, aid, txt)
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
