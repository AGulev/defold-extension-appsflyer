#include "lua_util.h"

namespace dmAppsflyer {

bool luaL_checkbool(lua_State *L, int numArg)
{
    bool b = false;
    if (lua_isboolean(L, numArg))
    {
        b = lua_toboolean(L, numArg);
    }
    else
    {
        luaL_typerror(L, numArg, lua_typename(L, LUA_TBOOLEAN));
    }
    return b;
}

} // namespace