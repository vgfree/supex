

local G_COLOR = {
	DEFAULT         = "\x1B[0m",
	BLACK           = "\x1B[30;2m",
	RED             = "\x1B[31;2m",
	GREEN           = "\x1B[32;2m",
	YELLOW          = "\x1B[33;2m",
	BLUE            = "\x1B[34;2m",
	PURPLE          = "\x1B[35;2m",
	SKYBLUE         = "\x1B[36;2m",
	GRAY            = "\x1B[37;2m",
}

local TONE_LIST = {
	DEBUG		= G_COLOR["GREEN"],
	INFO		= G_COLOR["BLUE"],
	WARNNING	= G_COLOR["YELLOW"],
	ERROR		= G_COLOR["RED"],
	SYSTEM		= G_COLOR["PURPLE"]
}

function handle( )
	local data = "2015-05-12 17:58:18 [DEBUG](1)-->hello world!\n2015-05-12 17:58:18 [ERROR](1)-->hello world!\n"
	local tone = G_COLOR["DEFAULT"]
	local head = ""
	repeat
		local st, ed, more, time, rank, info = string.find(data, "(.-)(%d%d%d%d%-%d%d%-%d%d %d%d%:%d%d%:%d%d) %[(%u+)%](.*)")
		if not st then break end

		if more then
			os.execute(string.format('echo -ne "%s%s%s"', tone, head, more))
		end

		tone = TONE_LIST[ rank ]
		head = string.sub(data, st + #more, ed - #info)

		data = info
	until false
	os.execute(string.format('echo -ne "%s%s%s"', tone, head, data))
end

handle()
