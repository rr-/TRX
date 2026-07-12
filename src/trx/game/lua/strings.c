#include <trx/game/game_strings/entries.h>
#include <trx/game/lua/common.h>

#include <lauxlib.h>

// trxc.strings.get(key) -> string or nil
//
// Returns the localized string for a game string key. Scripts format it
// themselves with Lua's string.format, since the underlying strings use
// printf-style placeholders.
static int M_L_StringsGet(lua_State *const L)
{
    const char *const value = GameString_Get(luaL_checkstring(L, 1));
    if (value == nullptr) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, value);
    }
    return 1;
}

void LUA_CreateStrings(lua_State *const L)
{
    lua_getglobal(L, "trxc");
    lua_newtable(L);
    lua_pushcfunction(L, M_L_StringsGet);
    lua_setfield(L, -2, "get");
    lua_setfield(L, -2, "strings");
    lua_pop(L, 1);
}
