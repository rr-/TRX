#pragma once

#include <lualib.h>
#include <stdint.h>

// Result of evaluating a Lua chunk.
typedef struct {
    int32_t code; // LUA_OK, LUA_ERRSYNTAX, LUA_ERRRUN, etc.
    char *message; // Error text (nullptr if code == LUA_OK).
} LUA_RESULT;

typedef enum {
    LUA_CONTEXT_GLOBAL,
    LUA_CONTEXT_LEVEL,
} LUA_CONTEXT;

void LUA_Init(void);
void LUA_Shutdown(void);

// Prints the full public API surface as JSON: the C-side FIELD_DESC tables plus
// the Lua-side trx.api registry. Used by --dump-lua-api.
void LUA_DumpAPI(void);

// Runs a Lua test script against the fully initialised scripting environment -
// the same one a level script sees. Returns true if it completed without error.
bool LUA_RunTest(const char *path);

// Set script context: level script vs global script
LUA_CONTEXT Lua_GetScriptContext(void);
void Lua_SetScriptContext(LUA_CONTEXT context);

// Evaluate a Lua code string. Caller must free the result with Lua_FreeResult.
LUA_RESULT Lua_Eval(const char *code);

// Free the LUA eval result.
void Lua_FreeResult(LUA_RESULT *result);

// Evaluate a Lua script file. Caller must free the result with Lua_FreeResult.
LUA_RESULT Lua_EvalFile(const char *path);

// Reload current level script and reset level-scoped listeners.
void Lua_ReloadLevelScript(void);

// Tells Lua a config option changed, so trx.config watchers can run. Also fired
// once after the config is first read, so a watcher applies the saved value at
// boot rather than only reacting to later edits.
void LUA_Config_NotifyChanged(void);

// Loads the per-game script (games/<mod>/scripts/_game.lua) if it exists. Runs
// before the config is read so that options it declares are already in the map.
void LUA_LoadGameScript(const char *mod_id);
