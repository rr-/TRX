#include <trx/core/log.h>
#include <trx/core/memory.h>
#include <trx/core/strings.h>
#include <trx/game/console/common.h>
#include <trx/game/console/registry.h>
#include <trx/game/game_strings/entries.h>
#include <trx/game/lua/common.h>

#include <lauxlib.h>
#include <string.h>

// Matches the pattern in lua/events.c: the module caches the state it was
// created with, so C-side callbacks can re-enter Lua.
static lua_State *m_L = nullptr;

// trxc.console.log(...)
static int M_L_ConsoleLog(lua_State *const L)
{
    int num_args = lua_gettop(L);
    if (num_args < 2) {
        return 0;
    }

    const LOG_LEVEL log_level = (int)lua_tointeger(L, 1);
    const char *msg = nullptr;

    for (int32_t i = 2; i <= num_args; i++) {
        lua_getglobal(L, "tostring");
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char *arg = lua_tostring(L, -1);
        lua_pop(L, 1);
        msg = (i > 2) ? String_FormatStatic("%s %s", msg, arg)
                      : String_FormatStatic("%s", arg);
    }

    lua_Debug ar;
    const char *src = "?";
    const char *func = "?";
    int line = 0;
    if (lua_getstack(L, 2, &ar) && lua_getinfo(L, "nSl", &ar)) {
        src = ar.short_src;
        func = ar.name ? ar.name : "?";
        line = ar.currentline;
    }
    Console_LogEx(log_level, src, line, func, "%s", msg);
    return 0;
}

// trxc.console.clear()
static int M_L_ConsoleClear(lua_State *const L)
{
    Console_Clear();
    return 0;
}

// trxc.console.eval(cmd, { verbose = bool })
static int M_L_ConsoleEval(lua_State *const L)
{
    const char *cmd = luaL_checkstring(L, 1);
    bool verbose = false;
    if (lua_gettop(L) >= 2 && lua_istable(L, 2)) {
        lua_getfield(L, 2, "verbose");
        verbose = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }
    const bool old_verbose = Console_IsVerbose();
    Console_SetVerbose(verbose);
    COMMAND_RESULT res = Console_Eval(cmd);
    Console_SetVerbose(old_verbose);
    const char *err = "unknown error";
    switch (res) {
    case CR_BAD_INVOCATION:
        err = "bad invocation";
        break;
    case CR_UNAVAILABLE:
        err = "unavailable";
        break;
    case CR_FAILURE:
        err = "failure";
        break;
    case CR_SUCCESS:
        return 0;
    }
    return luaL_error(L, "console.eval %s: %s", err, cmd);
}

// Registry key for the table mapping command prefix -> Lua handler.
static const char M_HANDLERS_KEY[] = "trx.console.handlers";

static COMMAND_RESULT M_ResultFromString(const char *const str)
{
    if (str == nullptr || !strcmp(str, "ok")) {
        return CR_SUCCESS;
    }
    if (!strcmp(str, "unavailable")) {
        return CR_UNAVAILABLE;
    }
    if (!strcmp(str, "bad_invocation")) {
        return CR_BAD_INVOCATION;
    }
    return CR_FAILURE;
}

// Shared entrypoint for every Lua-defined console command. Dispatches on the
// command's prefix to the handler stored in the Lua registry, so a single C
// trampoline serves all of them.
static COMMAND_RESULT M_LuaCommandProc(const COMMAND_CONTEXT *const ctx)
{
    lua_State *const L = m_L;
    if (L == nullptr) {
        return CR_FAILURE;
    }

    const int32_t base = lua_gettop(L);
    if (lua_getfield(L, LUA_REGISTRYINDEX, M_HANDLERS_KEY) != LUA_TTABLE) {
        lua_settop(L, base);
        return CR_FAILURE;
    }
    if (lua_getfield(L, -1, ctx->prefix) != LUA_TFUNCTION) {
        lua_settop(L, base);
        return CR_FAILURE;
    }

    lua_pushstring(L, ctx->args != nullptr ? ctx->args : "");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        Console_LogError("%s: %s", ctx->prefix, lua_tostring(L, -1));
        lua_settop(L, base);
        return CR_FAILURE;
    }

    const COMMAND_RESULT result = M_ResultFromString(lua_tostring(L, -1));
    lua_settop(L, base);
    return result;
}

// trxc.console.register(name, help_id, fn)
static int M_L_ConsoleRegister(lua_State *const L)
{
    const char *const name = luaL_checkstring(L, 1);
    const char *const help_id = luaL_optstring(L, 2, nullptr);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    if (Console_Registry_Get(name) != nullptr) {
        return luaL_error(
            L, "console command '%s' is already registered", name);
    }

    if (lua_getfield(L, LUA_REGISTRYINDEX, M_HANDLERS_KEY) != LUA_TTABLE) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, M_HANDLERS_KEY);
    }
    lua_pushvalue(L, 3);
    lua_setfield(L, -2, name);
    lua_pop(L, 1);

    // The registry stores these by pointer and outlives this call.
    Console_Registry_Add((CONSOLE_COMMAND) {
        .prefix = Memory_DupStr(name),
        .proc = M_LuaCommandProc,
        .help_id = help_id != nullptr ? Memory_DupStr(help_id) : nullptr,
    });
    return 0;
}

void LUA_CreateConsole(lua_State *const L)
{
    m_L = L;
    lua_getglobal(L, "trxc");
    lua_newtable(L);
    lua_pushcfunction(L, M_L_ConsoleLog);
    lua_setfield(L, -2, "log");
    lua_pushcfunction(L, M_L_ConsoleEval);
    lua_setfield(L, -2, "eval");
    lua_pushcfunction(L, M_L_ConsoleClear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, M_L_ConsoleRegister);
    lua_setfield(L, -2, "register");
    lua_setfield(L, -2, "console");
    lua_pop(L, 1);
}
