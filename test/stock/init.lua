local FILENAME = "data.txt"--"~/data/hq/20161102/XXXXXX_STOCK"

-->> split string into table
function str_split(s, c)
	if not s then return nil end

	local m = string.format("([^%s]+)", c)
	local t = {}
	local k = 1
	for v in string.gmatch(s, m) do
		t[k] = v
		k = k + 1
	end
	return t
end

---> get max line
local lines = 0
local file = io.open(FILENAME, "r")
for _ in file:lines() do
	lines = lines + 1
end
print(lines)
file.close()

---> init shm
local stock = require('stock')
stock.shm_init(lines)


---> work start
local file = io.open(FILENAME, "r")
for line in file:lines() do
	print(line)
	local info = str_split(line, '|')
	local is_buy_in = (tonumber(info[11]) > 0) or
		(tonumber(info[13]) > 0) or
		(tonumber(info[15]) > 0) or
		(tonumber(info[17]) > 0) or
		(tonumber(info[19]) > 0) or
		(tonumber(info[31]) > 0) or
		(tonumber(info[33]) > 0) or
		(tonumber(info[35]) > 0) or
		(tonumber(info[37]) > 0) or
		(tonumber(info[39]) > 0)

	
	local mode = is_buy_in and true or false

	stock.shm_push(mode, tonumber(info[1]), tonumber(info[2]), tonumber(info[3]),
			tonumber(info[4]), tonumber(info[5]), tonumber(info[6]))
end
file.close()
