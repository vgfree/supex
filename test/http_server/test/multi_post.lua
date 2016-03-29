require 'socket'
local ip = '192.168.1.101'
local port = 1314

local req_fmt = 'POST /some_uri.json HTTP/1.1\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local tcp, err = socket.connect(ip, port)

if tcp == nil then
    print(string.format('connect server[%s:%d] fail: %s', ip, port, err))
    return
end

local body = --[[Just read up on the thread and I feel an answer is missing. A common way to eliminate branch prediction that I've found to work particularly good in managed languages is a table lookup instead of using a branch. (although I haven't tested it in this case) This approach works in general if:
It's a small table and is likely to be cached in the processor
You are running things in a quite tight loop and/or the processor can pre-load the data
Background and why Pfew, so what the hell is that supposed to mean?
From a processor perspective, your memory is slow. To compensate for the difference in speed, they build in a couple of caches in your processor (L1/L2 cache) that compensate for that. So imagine that you're doing your nice calculations and figure out that you need a piece of memory. The processor will get his 'load' operation and loads the piece of memory into cache - and then uses the cache to do the rest of the calculations. Because memory is relatively slow, this 'load' will slow down your program.
Like branch prediction, this was optimized in the Pentium processors: the processor predicts that it needs to load a piece of data and attempts to load that into the cache before the operation actually hits the cache. As we've already seen, branch prediction sometimes goes horribly wrong -- in the worst case scenario you need to go back and actually wait for a memory load, which will take forever (in other words: failing branch prediction is bad, a memory load after a branch prediction fail is just horrible!).
Fortunately for us, if the memory access pattern is predictable, the processor will load it in its fast cache and all is well.
First thing we need to know is what is small? While smaller is generally better, a rule of thumb is to stick to lookup tables that are <=4096 bytes in size. As an upper limit: if your lookup table is larger than 64K it's probably worth reconsidering.
Constructing a table]]

[[Just read up on the thread and I feel an answer is missing. A common way to eliminate branch prediction that I've found to work particularly good in managed languages is a table lookup instead of using a branch. (although I haven't tested it in this case) This approach works in general if:
It's a small table and is likely to be cached in the processor
You are running things in a quite tight loop and/or the processor can pre-load the data
Background and why Pfew, so what the hell is that supposed to mean?
From a processor perspective, your memory is slow. To compensate for the difference in speed, they build in a couple of caches in your processor (L1/L2 cache) that compensate for that. So imagine that you're doing your nice calculations and figure out that you need a piece of memory. The processor will get his 'load' operation and loads the piece of memory into cache - and then uses the cache to do the rest of the calculations. Because memory is relatively slow, this 'load' will slow down your program.
Like branch prediction, this was optimized in the Pentium processors: the processor predicts that it needs to load a piece of data and attempts to load that into the cache before the operation actually hits the cache. As we've already seen, branch prediction sometimes goes horribly wrong -- in the worst case scenario you need to go back and actually wait for a memory load, which will take forever (in other words: failing branch prediction is bad, a memory load after a branch prediction fail is just horrible!).
Fortunately for us, if the memory access pattern is predictable, the processor will load it in its fast cache and all is well.
First thing we need to know is what is small? While smaller is generally better, a rule of thumb is to stick to lookup tables that are <=4096 bytes in size. As an upper limit: if your lookup table is larger than 64K it's probably worth reconsidering.
Constructing a table
So we've figured out that we can create a small table. Next thing to do is get a lookup function in place. Lookup functions are usually small functions that use a couple of basic integer operations (and, or, xor, shift, add, remove and perhaps a multiply). What you want is to have your input translated by the lookup function to some kind of 'unique key' in your table, which then simply gives you the answer of all the work you wanted it to do.
In this case: >=128 means we can keep the value, <128 means we get rid of it. The easiest way to do that is by using an 'AND': if we keep it, we AND it with 7FFFFFFF ; if we want to get rid of it, we AND it with 0. Notice also that 128 is a power of 2 -- so we can go ahead and make a table of 32768/128 integers and fill it with one zero and a lot of 7FFFFFFFF's.
Managed languages
You might wonder why this works well in managed languages. After all, managed languages check the boundaries of the arrays with a branch to ensure you don't mess up...
Well, not exactly... :-)
There has been quite some work on eliminating this branch for managed languages. For example:
for (int i=0; i<array.Length; ++i)
       // use array[i]
       in this case it's obvious to the compiler that the boundary condition will never hit. At least the Microsoft JIT compiler (but I expect Java does similar things) will notice this and remove the check all together. WOW - that means no branch. Similarly, it will deal with other obvious cases.
       If you run into trouble with lookups on managed languages - the key is to add a & 0x[something]FFF to your lookup function to make the boundary check predictable - and watch it going faster.
       The result for this case
It's a small table and is likely to be cached in the processor
You are running things in a quite tight loop and/or the processor can pre-load the data
Background and why Pfew, so what the hell is that supposed to mean?
From a processor perspective, your memory is slow. To compensate for the difference in speed, they build in a couple of caches in your processor (L1/L2 cache) that compensate for that. So imagine that you're doing your nice calculations and figure out that you need a piece of memory. The processor will get his 'load' operation and loads the piece of memory into cache - and then uses the cache to do the rest of the calculations. Because memory is relatively slow, this 'load' will slow down your program.
Like branch prediction, this was optimized in the Pentium processors: the processor predicts that it needs to load a piece of data and attempts to load that into the cache before the operation actually hits the cache. As we've already seen, branch prediction sometimes goes horribly wrong -- in the worst case scenario you need to go back and actually wait for a memory load, which will take forever (in other words: failing branch prediction is bad, a memory load after a branch prediction fail is just horrible!).
Fortunately for us, if the memory access pattern is predictable, the processor will load it in its fast cache and all is well.
First thing we need to know is what is small? While smaller is generally better, a rule of thumb is to stick to lookup tables that are <=4096 bytes in size. As an upper limit: if your lookup table is larger than 64K it's probably worth reconsidering.
Constructing a table
So we've figured out that we can create a small table. Next thing to do is get a lookup function in place. Lookup functions are usually small functions that use a couple of basic integer operations (and, or, xor, shift, add, remove and perhaps a multiply). What you want is to have your input translated by the lookup function to some kind of 'unique key' in your table, which then simply gives you the answer of all the work you wanted it to do.
In this case: >=128 means we can keep the value, <128 means we get rid of it. The easiest way to do that is by using an 'AND': if we keep it, we AND it with 7FFFFFFF ; if we want to get rid of it, we AND it with 0. Notice also that 128 is a power of 2 -- so we can go ahead and make a table of 32768/128 integers and fill it with one zero and a lot of 7FFFFFFFF's.
Managed languages
You might wonder why this works well in managed languages. After all, managed languages check the boundaries of the arrays with a branch to ensure you don't mess up...
Well, not exactly... :-)
There has been quite some work on eliminating this branch for managed languages. For example:
for (int i=0; i<array.Length; ++i)
       // use array[i]
       in this case it's obvious to the compiler that the boundary condition will never hit. At least the Microsoft JIT compiler (but I expect Java does similar things) will notice this and remove the check all together. WOW - that means no branch. Similarly, it will deal with other obvious cases.
       If you run into trouble with lookups on managed languages - the key is to add a & 0x[something]FFF to your lookup function to make the boundary check predictable - and watch it going faster.
       The result for this case
It's a small table and is likely to be cached in the processor
You are running things in a quite tight loop and/or the processor can pre-load the data
Background and why Pfew, so what the hell is that supposed to mean?
From a processor perspective, your memory is slow. To compensate for the difference in speed, they build in a couple of caches in your processor (L1/L2 cache) that compensate for that. So imagine that you're doing your nice calculations and figure out that you need a piece of memory. The processor will get his 'load' operation and loads the piece of memory into cache - and then uses the cache to do the rest of the calculations. Because memory is relatively slow, this 'load' will slow down your program.
Like branch prediction, this was optimized in the Pentium processors: the processor predicts that it needs to load a piece of data and attempts to load that into the cache before the operation actually hits the cache. As we've already seen, branch prediction sometimes goes horribly wrong -- in the worst case scenario you need to go back and actually wait for a memory load, which will take forever (in other words: failing branch prediction is bad, a memory load after a branch prediction fail is just horrible!).
Fortunately for us, if the memory access pattern is predictable, the processor will load it in its fast cache and all is well.
First thing we need to know is what is small? While smaller is generally better, a rule of thumb is to stick to lookup tables that are <=4096 bytes in size. As an upper limit: if your lookup table is larger than 64K it's probably worth reconsidering.
Constructing a table
So we've figured out that we can create a small table. Next thing to do is get a lookup function in place. Lookup functions are usually small functions that use a couple of basic integer operations (and, or, xor, shift, add, remove and perhaps a multiply). What you want is to have your input translated by the lookup function to some kind of 'unique key' in your table, which then simply gives you the answer of all the work you wanted it to do.
In this case: >=128 means we can keep the value, <128 means we get rid of it. The easiest way to do that is by using an 'AND': if we keep it, we AND it with 7FFFFFFF ; if we want to get rid of it, we AND it with 0. Notice also that 128 is a power of 2 -- so we can go ahead and make a table of 32768/128 integers and fill it with one zero and a lot of 7FFFFFFFF's.
Managed languages
You might wonder why this works well in managed languages. After all, managed languages check the boundaries of the arrays with a branch to ensure you don't mess up...
Well, not exactly... :-)
There has been quite some work on eliminating this branch for managed languages. For example:
for (int i=0; i<array.Length; ++i)
       // use array[i]
       in this case it's obvious to the compiler that the boundary condition will never hit. At least the Microsoft JIT compiler (but I expect Java does similar things) will notice this and remove the check all together. WOW - that means no branch. Similarly, it will deal with other obvious cases.
       If you run into trouble with lookups on managed languages - the key is to add a & 0x[something]FFF to your lookup function to make the boundary check predictable - and watch it going faster.
       The result for this case
]]

while true do
    local req = string.format(req_fmt, ip, port, #body, body)
    req = req .. req

    --print(string.format('sending %s', req))
    local ok, err = tcp:send(req);

    if not ok then
        tcp:close('both')
        -- print('send fail')
        tcp, err = socket.connect(ip, port)
        if tcp == nil then
            print(string.format('connect server[%s:%d] fail: %s', ip, port, err))
            return
        end
    else
        ret = tcp:receive('*l')
    end

    --print(ret)
    --os.execute('sleep 1')
end
