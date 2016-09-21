--libexif binding
package.cpath = "?.so;" .. package.cpath
local ffi = require'ffi'
local C = ffi.load("./libexif.so")
require'libexif_h'

if not ... then
	local ed = C.exif_data_new_from_file('exif.jpg')
	if ed ~= nil then
		C.exif_data_dump(ed)
		C.exif_data_unref(ed)
	end
end

return C
