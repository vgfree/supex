-- 
-- file: only.lua
--


module("only", package.seeall)

function log(level, fmt, ...)
	print(string.format("log[%s] " .. fmt, level, ...))
end
