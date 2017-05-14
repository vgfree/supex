package.path = '/data/nginx/open/lib/?.lua;;' .. package.path

local redis = require('redis')

local function handle()
        local file = io.open('./tag', 'r')

        local tab = {}
        for line in file:lines() do
                local i, j = string.find(line, '==')
                local tag = string.sub(line, j+1, -1)
                --print(line, tag)
                if not tab[tag] then
                        tab[tag] = 1
                else
                        tab[tag] = tab[tag] + 1
                        print(line .. ' ===> ' .. tab[tag])
                end
        end
end

local function split(string, char)
    if not string then return nil end 
    local str = string
    local tab = {}
    local idx 
    while true do
        idx = string.find(str, char)
        if idx == nil then
            tab[#tab + 1] = str 
            break
        end
        tab[#tab + 1] = string.sub(str, 1, idx - 1)
        str = string.sub(str, idx + 1)
    end 
    return tab 
end

local set_redis = 1

local function filter()

	local file = io.open('./filter1', 'r')
	
	conn = redis.connect('127.0.0.1', 8888)
	if not conn then
		print('connect failed ')
	end

if set_redis == 1 then
	local cnt = 0
	for line in file:lines() do
		local gps = split(line, ',')
		cnt = cnt + 1
		conn:sadd(gps[2], line .. ':' .. cnt)
	end
end
	local token = conn:keys('*')
	print(#token)
	local DETA = 1e-7
	for i =1, #token do
		local mem = conn:smembers(token[i])
		local tab = {}
		for j = 1, #mem do
			local tmp = split(mem[j], ':')
			local t = split(tmp[1], ',')
			table.insert(tab, t)
		end
		table.sort(tab, function(f1, f2) return tonumber(f1[1]) < tonumber(f2[1]) end)
		for k=2, #tab do
			if math.abs(tonumber(tab[k][1]) - tab[k-1][1]) <= DETA and math.abs(tonumber(tab[k][3]) - tab[k-1][3]) <= DETA  and math.abs(tonumber(tab[k][4]) - tab[k-1][4]) <= DETA then
				print(string.format('time:%s, tokenCode:%s, lon:%s, lat:%s, speed:%s, dir:%s', tab[k][1], tab[k][2], tab[k][3], tab[k][4], tab[k][5], tab[k][6]))
			end
		end
	end
end

--filter()
handle()


