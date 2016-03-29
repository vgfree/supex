package.path = "../?.lua;"

local Ring = require("ring")

do
        local ring = Ring:new();

        local tasks = {
                { cycle = 2, rc = true },
                { cycle = 4, rc = true },
                { cycle = 6, rc = true },
                { cycle = 8, rc = true },
                { cycle = 3, rc = true },
                { cycle = 5, rc = true },
        };

        for _,v in ipairs(tasks) do
                ring:append(v);
        end

        for i=1,10 do
                --ring:Add(i);
                -- ring:Append(i);
        end

        

        -- print(ring.obj);
        
        -- print(ring:ForwardStep());
        -- print(ring:GetData());



        -- for i=1,ring:GetCounter() do
        --         print(ring:ForwardStep());
        -- end

        -- print(ring:DeletePrevious());
        -- print(ring:DeletePrevious());
        -- print(ring:DeleteNext());
        -- print(ring:DeleteNext());
        -- print('------');

        ring:travel(print);
        ring:free();
end