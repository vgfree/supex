-- this test is based on luaio(https://github.com/coordcn/luaio)
local tcp = require('tcp')
local ERRNO = require('errno')
local WriteBuffer = require('write_buffer')
local bit = require('bit')
local fs = require('fs')
local color = require('color')

local options = {
  host = '127.0.0.1',
  port = 6688
}

local handshake1 = 'M=luaio&A=eoEl7P&N=luaio'
local handshake2 = 'M=2222222222222222222222222&A=eoEl7P&N=22222222222'
local handshake3 = 'M=2222222222222222222222222&A=eoEl7P&N=11111111111'
local handshake4 = 'M=1111111111111111111111111&A=eoEl7P&N=22222222222'
local tansfer = 'M=45456asdfserwerwefasdfsdf&A=eoEl7P&T=1458266656&B=160521142332&G=0,117123450,29123450,-1,0,864;1,117123460,29123460,-1,0,863;'
local heartbeat = '随便什么数据，反正心跳带的数据都会被忽略掉，心跳带数据，谁带谁傻逼，浪费带宽。。。'

local hs1_buf = WriteBuffer.new(512)
hs1_buf:write_uint8(0x10)
hs1_buf:write_uint8(0x00)
hs1_buf:write_uint8(0x00)
hs1_buf:write_uint32_be(#handshake1)
hs1_buf:write(handshake1)

local hs2_buf = WriteBuffer.new(512)
hs2_buf:write_uint8(0x10)
hs2_buf:write_uint8(0x00)
hs2_buf:write_uint8(0x00)
hs2_buf:write_uint32_be(#handshake2)
hs2_buf:write(handshake2)

local hs3_buf = WriteBuffer.new(512)
hs3_buf:write_uint8(0x10)
hs3_buf:write_uint8(0x00)
hs3_buf:write_uint8(0x00)
hs3_buf:write_uint32_be(#handshake3)
hs3_buf:write(handshake3)

local hs4_buf = WriteBuffer.new(512)
hs4_buf:write_uint8(0x10)
hs4_buf:write_uint8(0x00)
hs4_buf:write_uint8(0x00)
hs4_buf:write_uint32_be(#handshake3)
hs4_buf:write(handshake3)

local ts_buf = WriteBuffer.new(512)
ts_buf:write_uint8(0x10)
ts_buf:write_uint8(0x00)
ts_buf:write_uint8(0x02)
ts_buf:write_uint32_be(#tansfer)
ts_buf:write(tansfer)

local hb0_buf = WriteBuffer.new(512)
hb0_buf:write_uint8(0x10)
hb0_buf:write_uint8(0x00)
hb0_buf:write_uint8(0x01)
hb0_buf:write_uint32_be(0)

local hb_buf = WriteBuffer.new(512)
hb_buf:write_uint8(0x10)
hb_buf:write_uint8(0x00)
hb_buf:write_uint8(0x01)
hb_buf:write_uint32_be(#heartbeat)

local socket, err = tcp.connect(options)
if err < 0 then
  print(ERRNO.parse(err))
  return
end

print(color.green('测试：先握手后传输，开始。'))

local bytes
bytes, err = socket:write(hs1_buf)
if err < 0 then
  print(color.red('测试：先握手后传输，握手发送失败。错误原因：' .. ERRNO.parse(err)))
  return
end

for i = 1, 1 do
  bytes, err = socket:write(ts_buf)
  if err < 0 then
    print(color.red('测试：先握手后传输，数据发送失败。错误原因：' .. ERRNO.parse(err)))
    return
  end
end

print(color.green('测试：先握手后传输，通过。'))

print('--------------------\n')

print(color.green('测试：先握手后传输，心跳开始。'))

for i = 1, 1 do
  bytes, err = socket:write(hb0_buf)
  if err < 0 then
    print(color.red('测试：先握手后传输，心跳不带数据发送失败。错误原因：' .. ERRNO.parse(err)))
    return
  end
end

for i = 1, 1 do
  bytes, err = socket:write(ts_buf)
  if err < 0 then
    print(color.red('测试：先握手后传输，心跳后数据发送失败。错误原因：' .. ERRNO.parse(err)))
    return
  end
end

for i = 1, 1 do
  bytes, err = socket:write(hb_buf)
  if err < 0 then
    print(color.red('测试：先握手后传输，心跳带数据发送失败。错误原因：' .. ERRNO.parse(err)))
    return
  end
end

print(color.green('测试：先握手后传输，心跳通过。'))

print('--------------------\n')

print(color.green('测试：先握手后传输，重复握手开始。'))

bytes, err = socket:write(hs1_buf)
if err < 0 then
  print(color.red('测试：先握手后传输，重复握手发送失败。错误原因：' .. ERRNO.parse(err)))
  return
end

while true do
  local data, err = socket:read()
  if err < 0 then
    print(ERRNO.parse(err))
    break
  end
end

print(color.green('测试：先握手后传输，重复握手通过。'))

print('--------------------\n')

socket:close()

socket, err = tcp.connect(options)
if err < 0 then
  print(ERRNO.parse(err))
  return
end


print(color.green('测试：直接发送心跳，开始。'))

for i = 1, 1 do
  bytes, err = socket:write(hb0_buf)
  if err < 0 then
    print(color.red('测试：直接发送心跳，心跳不带数据发送失败。错误原因：' .. ERRNO.parse(err)))
    return
  end
end

for i = 1, 1 do
  bytes, err = socket:write(hb_buf)
  if err < 0 then
    print(color.red('测试：直接发送心跳，心跳带数据发送失败。错误原因：' .. ERRNO.parse(err)))
    return
  end
end

print(color.green('测试：直接发送心跳，通过。'))

print('--------------------\n')

print(color.green('测试：直接发送数据，开始。'))

bytes, err = socket:write(ts_buf)
if err < 0 then
  print(color.red('测试：直接发送数据，数据传输失败。错误原因：' .. ERRNO.parse(err)))
  return
end

while true do
  local data, err = socket:read()
  if err < 0 then
    print(ERRNO.parse(err))
    break
  end
end

print(color.green('测试：直接发送数据，通过。'))

print('--------------------\n')

socket:close()

socket, err = tcp.connect(options)
if err < 0 then
  print(ERRNO.parse(err))
  return
end

print(color.green('测试：握手，错误的mirrtalkID，开始。'))

bytes, err = socket:write(hs3_buf)
if err < 0 then
  print(color.red('测试：握手，错误的mirrtalkID，数据传输失败。错误原因：' .. ERRNO.parse(err)))
  return
end

while true do
  local data, err = socket:read()
  if err < 0 then
    print(ERRNO.parse(err))
    break
  end
end

print(color.green('测试：握手，错误的mirrtalkID，通过。'))

print('--------------------\n')

socket:close()

socket, err = tcp.connect(options)
if err < 0 then
  print(ERRNO.parse(err))
  return
end

print(color.green('测试：握手，错误的gpsToken，开始。'))

bytes, err = socket:write(hs4_buf)
if err < 0 then
  print(color.red('测试：握手，错误的gpsToken，数据传输失败。错误原因：' .. ERRNO.parse(err)))
  return
end

while true do
  local data, err = socket:read()
  if err < 0 then
    print(ERRNO.parse(err))
    break
  end
end

print(color.green('测试：握手，错误的gpsToken，通过。'))

print('--------------------\n')

socket:close()

socket, err = tcp.connect(options)
if err < 0 then
  print(ERRNO.parse(err))
  return
end

print(color.green('测试：握手，错误的mirrtalkID和gpsToken，开始。'))

bytes, err = socket:write(hs2_buf)
if err < 0 then
  print(color.red('测试：握手，错误的mirrtalkID和gpsToken，数据传输失败。错误原因：' .. ERRNO.parse(err)))
  return
end

while true do
  local data, err = socket:read()
  if err < 0 then
    print(ERRNO.parse(err))
    break
  end
end

print(color.green('测试：握手，错误的mirrtalkID和gpsToken，通过。'))

print('--------------------\n')
