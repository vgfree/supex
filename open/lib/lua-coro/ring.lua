local modename = "ring"
local Ring = {}
_G[modename] = Ring
package.loaded[modename] = Ring


local function newnode( data )
        local node = { _prev = nil, _data = data, _next = nil };
        return node;
end

-----------------------------
function Ring:new( ... )
        -- body
        local ring = {	_head = nil,	--可执行协程的头节点
			_obj = nil,	--当前正在执行的协程
			_taskcnt = 0,	--可执行的协程数
		};
        setmetatable(ring, self);
        self.__index = self;
        return ring;
end

function Ring:getdata( ... )
        -- body
        return self._obj and self._obj._data or nil;
end

function Ring:gettaskcnt( ... )
	--body
        return self._taskcnt;
end

function Ring:gethead()
	--body
	return self._head and self._head._data or nil;
end

--[[返回当前节点数据，并步进到下一节点]]
function Ring:forwardstep( ... )
        -- body
        if self._taskcnt > 0 then
                self._obj = self._obj._next;
                return self._obj._data;
        else
                return nil;
        end
end

--[[返回当前节点数据，并步进到上一节点]]
function Ring:backwardstep( ... )
        -- body
        if self._taskcnt > 0 then
                self._obj = self._obj._prev;
                return self._obj._data;
        else
                return nil;
        end
end

--[[插入数据增加节点，但不改变头节点]]
function Ring:append( data )
        -- body
        local node = newnode(data);
        if self._taskcnt > 0 then
                local cur = self._head;
                node._next = cur;
                node._prev = cur._prev;
                cur._prev._next = node;
                cur._prev = node;
        else
                node._next = node;
                node._prev = node;
                self._head = node;
		self._obj = self._head;
        end
        self._taskcnt = self._taskcnt + 1;
end

--[[插入数据增加节点，但改变头节点为新增加的节点]]
function Ring:add( data )
        -- body
        local node = newnode(data);
        if self._taskcnt > 0 then
                local cur = self._head;
                node._next = cur;
                node._prev = cur._prev;
                cur._prev._next = node;
                cur._prev = node;
        else
                node._next = node;
                node._prev = node;
		self._obj = node;
        end
        self._head = node;
        self._taskcnt = self._taskcnt + 1;
end

--[[删除当前节点返回其数据,并将obj回退一步]]
--[[使用此函数删除节点的时候，步进函数必须使用forwardstep()与之匹配步进]]
function Ring:deleteprevious( )
        -- body
        local cur = self._obj;
        assert(self._taskcnt > 0);
        if self._taskcnt == 1 then
                self._obj = nil;
		self._head = nil;
        else
		if cur == self._head then
			self._head = self._head._next;
		end
		self._obj._prev._next = self._obj._next;
		self._obj._next._prev = self._obj._prev;
		self._obj = self._obj._prev;
        end
        self._taskcnt = self._taskcnt - 1;

        return cur._data;
end


--[[删除当前节点返回其数据,并将obj向前进一步]]
--[[使用此函数删除节点的时候，步进函数必须使用backwardstep()与之匹配步进]]
function Ring:deletenext( )
	--body
        local cur = self._obj;
        assert(self._taskcnt > 0);
        if self._taskcnt == 1 then
                self._obj = nil;
		self._head = nil;
        else
		if cur == self._head then
			self._head = self._head._next;
		end
		self._obj._prev._next = self._obj._next;
		self._obj._next._prev = self._obj._prev;
		self._obj = self._obj._next;
        end
        self._taskcnt = self._taskcnt - 1;

        return cur._data;
end

--[[释放环]]
function Ring:free( callback )
        -- body
        if self._taskcnt < 1  then
                return;
        end

        local data = nil;
	while self._taskcnt > 0 do
                data = self:deleteprevious();
                if callback then
                        callback(data);
                end
	end

end

--[[遍历环，并以每个节点的数据调用回调函数]]
function Ring:travel( callback )
        -- body
        local head = self._head;
        local cur = nil;

        if not callback then
                return;
        end

        while head and cur ~= head do
                cur = cur and cur._next or head._next;
                local ok = pcall(callback, cur._prev._data);
                if not ok then
                        break;
                end
        end
end
