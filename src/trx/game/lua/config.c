#include <trx/config/common.h>
#include <trx/config/dynamic_enum.h>
#include <trx/config/dynamic_option.h>
#include <trx/core/log.h>
#include <trx/core/memory.h>
#include <trx/core/strings.h>
#include <trx/game/lua/common.h>

#include <lauxlib.h>
#include <string.h>

static lua_State *m_L = nullptr;
// The Lua-side dispatcher that routes config changes to per-key watchers.
static int m_DispatcherRef = LUA_NOREF;

// trxc.config.get(key)
static int M_L_ConfigGet(lua_State *const L)
{
    const char *const key = luaL_checkstring(L, 1);
    const CONFIG_OPTION *const opt = Config_GetOptionByPath(key);
    if (opt == nullptr) {
        return luaL_error(L, "Unknown option: %s", key);
    }
    const char *const value = Config_GetOptionValueAsString(opt, false);
    lua_pushstring(L, value);
    return 1;
}

// trxc.config.set(key, value)
static int M_L_ConfigSet(lua_State *const L)
{
    const char *const key = luaL_checkstring(L, 1);
    const char *const new_value = luaL_checkstring(L, 2);
    const CONFIG_OPTION *const opt = Config_GetOptionByPath(key);
    if (opt == nullptr) {
        return luaL_error(L, "Unknown option: %s", key);
    }
    const bool ok = Config_SetOptionValueFromString(opt, new_value);
    if (!ok) {
        return luaL_error(L, "Failed to set option %s to %s", key, new_value);
    }
    Config_Update();
    return 0;
}

// trxc.config.list()
static int M_L_ConfigList(lua_State *const L)
{
    lua_newtable(L);
    const CONFIG_OPTION *opt = Config_GetOptionMap();
    while (opt->name != nullptr) {
        const char *const value = Config_GetOptionValueAsString(opt, false);
        lua_pushstring(L, value);
        lua_setfield(L, -2, opt->name);
        opt++;
    }
    return 1;
}

// trxc.config.set_enabled(key, enabled)
static int M_L_ConfigSetEnabled(lua_State *const L)
{
    const char *const key = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    const CONFIG_OPTION *const opt = Config_GetOptionByPath(key);
    if (opt == nullptr) {
        return luaL_error(L, "Unknown option: %s", key);
    }
    Config_SetOptionDisabled(opt->target, !lua_toboolean(L, 2));
    return 0;
}

static CONFIG_OPTION_TYPE M_ParseOptionType(
    lua_State *const L, const char *const type)
{
    if (strcmp(type, "bool") == 0) {
        return COT_BOOL;
    }
    if (strcmp(type, "int") == 0) {
        return COT_INT32;
    }
    if (strcmp(type, "enum") == 0) {
        return COT_DYNAMIC_ENUM;
    }
    luaL_error(L, "Unknown option type: %s", type);
    return COT_BOOL;
}

static const char *M_GetOptStrField(
    lua_State *const L, const int32_t idx, const char *const name)
{
    lua_getfield(L, idx, name);
    const char *const result =
        lua_isnil(L, -1) ? nullptr : luaL_checkstring(L, -1);
    lua_pop(L, 1);
    return result;
}

// Seeds a declared enum's values. Each label is a game string key derived from
// the option, so the values are translatable like every other setting - see
// tools/update_game_strings.
static void M_DeclareEnumValues(
    lua_State *const L, const int32_t spec_idx, const char *const key,
    const CONFIG_OPTION *const option)
{
    lua_getfield(L, spec_idx, "values");
    if (!lua_istable(L, -1)) {
        luaL_error(L, "Option %s is an enum but declares no values", key);
    }
    Config_DynamicEnum_ResetValues(option);
    const int32_t count = (int32_t)lua_rawlen(L, -1);
    for (int32_t i = 1; i <= count; i++) {
        lua_rawgeti(L, -1, i);
        const char *const value = luaL_checkstring(L, -1);
        Config_DynamicEnum_AddValue(
            option, value,
            String_FormatStatic("settings/%s/values/%s", key, value));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

// trxc.config.declare(spec)
static int M_L_ConfigDeclare(lua_State *const L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "key");
    const char *const key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "type");
    const CONFIG_OPTION_TYPE type =
        M_ParseOptionType(L, luaL_checkstring(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, 1, "default");
    CONFIG_DYNAMIC_OPTION *option = nullptr;
    switch (type) {
    case COT_BOOL:
        option = Config_AddDynamicOption(
            key, type, &(bool) { lua_toboolean(L, -1) });
        break;
    case COT_INT32:
        option = Config_AddDynamicOption(
            key, type, &(int32_t) { (int32_t)luaL_checkinteger(L, -1) });
        break;
    default:
        option = Config_AddDynamicOption(key, type, luaL_checkstring(L, -1));
        break;
    }
    lua_pop(L, 1);

    if (option == nullptr) {
        return luaL_error(L, "Option already declared: %s", key);
    }

    lua_getfield(L, 1, "min");
    option->min_value = (int32_t)luaL_optinteger(L, -1, 0);
    lua_pop(L, 1);
    lua_getfield(L, 1, "max");
    option->max_value = (int32_t)luaL_optinteger(L, -1, 0);
    lua_pop(L, 1);

    const char *const tab = M_GetOptStrField(L, 1, "tab");
    const char *const before = M_GetOptStrField(L, 1, "before");
    const char *const after = M_GetOptStrField(L, 1, "after");
    option->ui.tab = tab == nullptr ? nullptr : Memory_DupStr(tab);
    option->ui.before = before == nullptr ? nullptr : Memory_DupStr(before);
    option->ui.after = after == nullptr ? nullptr : Memory_DupStr(after);

    lua_getfield(L, 1, "priority");
    if (!lua_isnil(L, -1)) {
        option->ui.priority = (int32_t)luaL_checkinteger(L, -1);
        option->ui.has_priority = true;
    }
    lua_pop(L, 1);

    if (type == COT_DYNAMIC_ENUM) {
        M_DeclareEnumValues(L, 1, key, Config_GetOptionByPath(key));
    }
    return 0;
}

// trxc.config.on_change(fn) - registers the single dispatcher that config.lua
// uses to route changes to per-key watchers. Keeping the routing in Lua means
// the engine only has to say "something changed".
static int M_L_ConfigOnChange(lua_State *const L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (m_DispatcherRef != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, m_DispatcherRef);
    }
    lua_pushvalue(L, 1);
    m_DispatcherRef = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

void LUA_Config_NotifyChanged(void)
{
    lua_State *const L = m_L;
    if (L == nullptr || m_DispatcherRef == LUA_NOREF) {
        return;
    }
    // A watcher that writes config - which is the whole point, eg. a mode
    // option driving a colour - fires this event again from inside itself.
    // The watcher's own writes are its business; it does not need telling
    // about them.
    static bool dispatching = false;
    if (dispatching) {
        return;
    }
    dispatching = true;
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_DispatcherRef);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        LOG_ERROR("config change handler failed: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    dispatching = false;
}

void LUA_CreateConfig(lua_State *const L)
{
    m_L = L;
    lua_getglobal(L, "trxc");
    lua_newtable(L);
    lua_pushcfunction(L, M_L_ConfigGet);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, M_L_ConfigSet);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, M_L_ConfigList);
    lua_setfield(L, -2, "list");
    lua_pushcfunction(L, M_L_ConfigDeclare);
    lua_setfield(L, -2, "declare");
    lua_pushcfunction(L, M_L_ConfigSetEnabled);
    lua_setfield(L, -2, "set_enabled");
    lua_pushcfunction(L, M_L_ConfigOnChange);
    lua_setfield(L, -2, "on_change");
    lua_setfield(L, -2, "config");
    lua_pop(L, 1);
}
