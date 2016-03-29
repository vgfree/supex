-- auth: coanor
-- date: Wed Sep 25 10:28:10 CST 2013
-- generate test data
local Chars = {}
for Loop = 0, 255 do
   Chars[Loop+1] = string.char(Loop)
end
local String = table.concat(Chars)

local Built = {['.'] = Chars}

local AddLookup = function(CharSet)
   local Substitute = string.gsub(String, '[^'..CharSet..']', '')
   local Lookup = {}
   for Loop = 1, string.len(Substitute) do
       Lookup[Loop] = string.sub(Substitute, Loop, Loop)
   end
   Built[CharSet] = Lookup

   return Lookup
end

function string.random(Length, CharSet)
   -- Length (number)
   -- CharSet (string, optional); e.g. %l%d for lower case letters and digits

   local CharSet = CharSet or '.'

   if CharSet == '' then
      return ''
   else
      local Result = {}
      local Lookup = Built[CharSet] or AddLookup(CharSet)
      local Range = table.getn(Lookup)

      for Loop = 1,Length do
         Result[Loop] = Lookup[math.random(1, Range)]
      end

      return table.concat(Result)
   end
end

local function gen_kv(len, item_cnt)
	local file_name = string.format('k_%d_v_%d.ldb', len.key, len.val)

	local f = io.open(file_name, 'a')
	local str = ''
	local key, val = nil, string.random(len.val, '%l%d')
	local i = 1

	f:write(string.format('%d,%d,%d\n', len.key, len.val, item_cnt))

	for i=1, item_cnt do
		key = string.random(len.key, '%l%d')
		if i%100 == 0 then -- got duplicated val
			val = string.random(len.val, '%l%d')
		end
		str = string.format('%s,%s;', key, val)
		f:write(str)
	end
	f:flush()
	f:close()
end

local function gen_real_kv(len, minites, user_cnt, day_cnt)
	local str = ''
	local key, val =
		string.format('%s:%d:data', '0123456789', os.time()),
		string.random(len.val, '%l%d')

	local file_name = string.format('k_%d_v_%d_m_%d_u_%d_d_%d.ldb',
		#key, len.val*minites, minites, user_cnt, day_cnt)
	local f = io.open(file_name, 'a')
	local i = 1

	local file_name = string.format('k_%d_v_%d.ldb', len.key, len.val)
	local begin_time = os.time({year = 2013, month = 1, day = 1, hour=0, min=1})
	local end_time = begin_time + day_cnt*24*3600
	local month_key_cnt = math.floor((end_time - begin_time)/60/minites)
	print(string.format("data_cnt = %d", user_cnt * month_key_cnt))

	f:write(string.format('%d,%d,%d\n', #key, len.val*minites, user_cnt * month_key_cnt))

	local data_cnt = 0
	local userid = nil
	for i=1, user_cnt do
		userid = string.random(10, '%l%d')
		for j=0, month_key_cnt do
			key = string.format('%s:%d:data', userid, begin_time+j*60*minites)
			if data_cnt%100 == 0 then
				val = string.random(len.val*minites, '%l%d')
			end

			str = string.format('%s,%s;', key, val)
			f:write(str)
			data_cnt = data_cnt + 1;
		end
	end
	f:flush()
	f:close()
end

--gen_kv({key=arg[1], val=arg[2]}, arg[3])
gen_real_kv({key=arg[1], val=arg[2]}, arg[3], arg[4], arg[5])
