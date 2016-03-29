package.path = "../?.lua;" .. package.path
local shm_ffi = require("shm-ffi")


os.execute("sleep 2")
shm_ffi.push(2000, false, "0ello worldaaaaaaaaaaa")
shm_ffi.push(1000, false, "yes")
print(";;;;;;;;;;;;;;;;;")
