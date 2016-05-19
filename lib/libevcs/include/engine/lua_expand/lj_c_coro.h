#pragma once


#include "lj_base.h"

__BEGIN_DECLS

/**scco协程库切换*/
int lj_scco_switch(lua_State *L);
/**coro协程库切换*/
int lj_evcoro_switch(lua_State *L);

__END_DECLS

