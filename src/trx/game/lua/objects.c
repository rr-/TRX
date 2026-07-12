#include <trx/core/memory.h>
#include <trx/game/lua/utils.h>

#include <string.h>

static void M_PushPropertyValue(
    lua_State *const L, const OBJECT_PROPERTY_VALUE *const value)
{
    switch (value->type) {
    case OBJECT_PROPERTY_TYPE_INT:
        lua_pushinteger(L, value->as_int);
        break;
    case OBJECT_PROPERTY_TYPE_FLOAT:
        lua_pushnumber(L, value->as_float);
        break;
    case OBJECT_PROPERTY_TYPE_DOUBLE:
        lua_pushnumber(L, value->as_double);
        break;
    case OBJECT_PROPERTY_TYPE_BOOL:
        lua_pushboolean(L, value->as_bool);
        break;
    case OBJECT_PROPERTY_TYPE_XYZ:
        lua_newtable(L);
        lua_pushinteger(L, value->as_xyz.x);
        lua_setfield(L, -2, "x");
        lua_pushinteger(L, value->as_xyz.y);
        lua_setfield(L, -2, "y");
        lua_pushinteger(L, value->as_xyz.z);
        lua_setfield(L, -2, "z");
        break;
    }
}

// trxc.objects.swap_mesh(obj1_id, obj2_id, mesh1_num, mesh2_num)
static int M_L_ObjectsSwapMesh(lua_State *const L)
{
    const int32_t arg_count = lua_gettop(L);
    const OBJECT_ID obj1_id = luaL_checkinteger(L, 1);
    const OBJECT_ID obj2_id = luaL_checkinteger(L, 2);
    if (arg_count == 2) {
        Object_SwapAllMeshes(obj1_id, obj2_id);
    } else {
        const int32_t mesh1_num = luaL_checkinteger(L, 3);
        const int32_t mesh2_num = luaL_checkinteger(L, 4);
        Object_SwapMeshEx(obj1_id, obj2_id, mesh1_num, mesh2_num);
    }
    return 0;
}

// trxc.objects.get_property(object_id, name) → typed value or nil
static int M_L_ObjectsGetProperty(lua_State *const L)
{
    const OBJECT_ID object_id = luaL_checkinteger(L, 1);
    const char *const name = luaL_checkstring(L, 2);
    OBJECT_PROPERTY_VALUE value = {};
    if (!ObjectProperty_GetObjectValue(
            Object_TryGet(object_id), name, &value)) {
        lua_pushnil(L);
        return 1;
    }
    M_PushPropertyValue(L, &value);
    return 1;
}

// trxc.objects.set_property(object_id, name, value)
static int M_L_ObjectsSetProperty(lua_State *const L)
{
    const OBJECT_ID object_id = luaL_checkinteger(L, 1);
    const char *const name = luaL_checkstring(L, 2);
    OBJECT *const obj = Object_TryGet(object_id);
    if (obj == nullptr) {
        return luaL_error(L, "invalid object id %d", object_id);
    }
    const OBJECT_PROPERTY_VALUE value = LUA_CheckPropertyValue(L, 3);
    if (!ObjectProperty_SetObjectValueRaw(obj, name, value)) {
        return luaL_error(L, "unknown object property '%s'", name);
    }
    return 0;
}

// trxc.objects.get_property_names(object_id) → table
static int M_L_ObjectsGetPropertyNames(lua_State *const L)
{
    const OBJECT_ID object_id = luaL_checkinteger(L, 1);
    const OBJECT *const obj = Object_TryGet(object_id);
    lua_newtable(L);
    for (int32_t i = 0; i < ObjectProperty_GetObjectNameCount(obj); i++) {
        lua_pushinteger(L, i + 1);
        lua_pushstring(L, ObjectProperty_GetObjectName(obj, i));
        lua_settable(L, -3);
    }
    return 1;
}

static bool M_IsTargetableCreature(const OBJECT_ID obj_id)
{
    return (Object_IsType(obj_id, g_CreatureObjects)
            || Object_IsType(obj_id, g_LoyalObjects))
        && Object_Get(obj_id)->loaded;
}

static bool M_IsSpawnable(const OBJECT_ID obj_id)
{
    return !Object_IsType(obj_id, g_NullObjects)
        && !Object_IsType(obj_id, g_AnimObjects)
        && !Object_IsType(obj_id, g_InvObjects) && Object_Get(obj_id)->loaded;
}

// trxc.objects.is_type(object_id, kind) -> bool
static int M_L_ObjectsIsType(lua_State *const L)
{
    const OBJECT_ID object_id = luaL_checkinteger(L, 1);
    const char *const kind = luaL_checkstring(L, 2);
    if (!strcmp(kind, "creature")) {
        lua_pushboolean(L, Object_IsType(object_id, g_CreatureObjects));
    } else if (!strcmp(kind, "loyal")) {
        lua_pushboolean(L, Object_IsType(object_id, g_LoyalObjects));
    } else {
        return luaL_error(L, "unknown object kind '%s'", kind);
    }
    return 1;
}

// trxc.objects.ids_from_name(name, [filter]) -> { object_id, ... }
//
// Fuzzy-matches a human-readable object name ("wolf", "big medi") against the
// object name catalog. `filter` is "creature", "spawnable", or nil for no
// filter. This is what lets console commands accept object names.
static int M_L_ObjectsIdsFromName(lua_State *const L)
{
    const char *const name = luaL_checkstring(L, 1);
    const char *const filter_name = luaL_optstring(L, 2, nullptr);

    bool (*filter)(OBJECT_ID) = nullptr;
    if (filter_name != nullptr) {
        if (!strcmp(filter_name, "creature")) {
            filter = M_IsTargetableCreature;
        } else if (!strcmp(filter_name, "spawnable")) {
            filter = M_IsSpawnable;
        } else {
            return luaL_error(L, "unknown object filter '%s'", filter_name);
        }
    }

    int32_t match_count = 0;
    OBJECT_NAME_MATCH *const matches =
        Object_IdsFromName(name, &match_count, filter);

    lua_newtable(L);
    for (int32_t i = 0; i < match_count; i++) {
        lua_pushinteger(L, matches[i].object_id);
        lua_rawseti(L, -2, i + 1);
    }
    Memory_FreePointer(&matches);
    return 1;
}

void LUA_CreateObjects(lua_State *const L)
{
    lua_getglobal(L, "trxc");
    lua_newtable(L);
    lua_pushcfunction(L, M_L_ObjectsSwapMesh);
    lua_setfield(L, -2, "swap_mesh");
    lua_pushcfunction(L, M_L_ObjectsGetProperty);
    lua_setfield(L, -2, "get_property");
    lua_pushcfunction(L, M_L_ObjectsSetProperty);
    lua_setfield(L, -2, "set_property");
    lua_pushcfunction(L, M_L_ObjectsGetPropertyNames);
    lua_setfield(L, -2, "get_property_names");
    lua_pushcfunction(L, M_L_ObjectsIdsFromName);
    lua_setfield(L, -2, "ids_from_name");
    lua_pushcfunction(L, M_L_ObjectsIsType);
    lua_setfield(L, -2, "is_type");
    lua_setfield(L, -2, "objects");
    lua_pop(L, 1);
}
