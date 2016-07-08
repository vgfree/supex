-- Copyright (c) 2010 Aleksey Yeschenko <aleksey@yeschenko.com>
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.

local path_list ={
        "lua/core/?.lua;",
        "lua/code/?.lua;",


        "../../../open/lib/?.lua;",
        "../../../open/apply/?.lua;",
        "../../../open/spxonly/?.lua;",
        "../../../open/linkup/?.lua;",
        "../../../open/public/?.lua;",

        "open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../../open/lib/?.so;" .. "open/?.so;" .. package.cpath

require("zmq")

local ctx = zmq.init(1)
local s = ctx:socket(zmq.REQ)

s:connect("tcp://localhost:5558")

local data_cid = {
	"locating",
	"status",
	"cid",
	"192.168.71.142:325"
}

local data_uid = {
	"locating",
        "status",
        "uid",
        "a46feacc-446b-4df1-84b2-bdeb9ba34605"
}

local data_gid = {
	"locating",
        "uidmap",
        "gid",
        "gid0"
}

s:send_table(data_cid)
print(s:recv())
s:send_table(data_uid)
print(s:recv())

s:send_table(data_gid)
print(s:recv())

s:close()
ctx:term()
