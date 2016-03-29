
local switch	= require("config_half_full")

module("Half_and_Full", package.seeall)

function half_to_full(start_str)

	if not start_str then
		return nil
	end
	local len = string.len(start_str)
	local end_str = ''

	local ret = {}
	for i = 1, len do
		local ascii = string.byte(start_str, i)
		if ascii > 47 and ascii < 123 then
			ret[i] = switch.full_tab[ascii]
		else
			ret[i] = string.sub(start_str, i, i)
		end

		end_str = end_str .. string.format("%s", ret[i])
	end

	return end_str
end

function full_to_half(start_str)

	if not start_str then
		return nil
	end
	local len = string.len(start_str)
	local end_str = ''

	local ret = {}
	for i = 1, len do
		local ascii = string.byte(start_str, i)
		if ascii > 47 and ascii < 123 then
			ret[i] = switch.full_tab[ascii]
		else
			ret[i] = string.sub(start_str, i, i)
		end

		end_str = end_str .. string.format("%s", ret[i])
	end

	return end_str
end

