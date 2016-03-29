luakv = require('luakv-ffi')

do
        local kvhdl = luakv.create();

        for result in kvhdl:ask('set name zhoukai') do
                print("set success :", result);
        end

        for result in kvhdl:ask('get name') do
                print("get success :", result);
        end

        for result in kvhdl:ask('del name') do
                print("del success :", result);
        end
        
end
