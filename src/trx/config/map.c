#include <trx/config/dynamic_option.h>
#include <trx/config/option.h>
#include <trx/config/priv.h>
#include <trx/config/types.h>
#include <trx/config/vars.h>
#include <trx/core/colors.h>
#include <trx/core/enum_map.h>
#include <trx/core/utils.h>
#include <trx/core/vector.h>
#include <trx/debug.h>
#include <trx/game/lara/const.h>
#include <trx/version.h>

#include <string.h>

#define X_CFG_BOOL(target_, default_value_)                                    \
    { .name = QUOTE(target_),                                                  \
      .type = COT_BOOL,                                                        \
      .target = &g_Config.target_,                                             \
      .default_value = &(bool) { default_value_ },                             \
      .param = nullptr },

#define X_CFG_INT32(target_, default_value_)                                   \
    { .name = QUOTE(target_),                                                  \
      .type = COT_INT32,                                                       \
      .target = &g_Config.target_,                                             \
      .default_value = &(int32_t) { default_value_ },                          \
      .param = nullptr },

#define X_CFG_FLOAT(target_, default_value_)                                   \
    { .name = QUOTE(target_),                                                  \
      .type = COT_FLOAT,                                                       \
      .target = &g_Config.target_,                                             \
      .default_value = &(float) { default_value_ },                            \
      .param = nullptr },

#define X_CFG_FLOAT_PERCENT(target_, default_value_)                           \
    { .name = QUOTE(target_),                                                  \
      .type = COT_FLOAT_PERCENT,                                               \
      .target = &g_Config.target_,                                             \
      .default_value = &(float) { default_value_ },                            \
      .param = nullptr },

#define X_CFG_DOUBLE(target_, default_value_)                                  \
    { .name = QUOTE(target_),                                                  \
      .type = COT_DOUBLE,                                                      \
      .target = &g_Config.target_,                                             \
      .default_value = &(double) { default_value_ },                           \
      .param = nullptr },

#define X_CFG_ENUM(target_, default_value_, enum_map)                          \
    X_CFG_ENUM_EX(QUOTE(target_), target_, default_value_, enum_map)

#define X_CFG_ENUM_EX(name_, target_, default_value_, enum_map)                \
    { .name = name_,                                                           \
      .type = COT_ENUM,                                                        \
      .target = &g_Config.target_,                                             \
      .default_value = &(int32_t) { default_value_ },                          \
      .param = ENUM_MAP_NAME(enum_map) },

#define X_CFG_RGB888(target_, default_r, default_g, default_b)                 \
    { .name = QUOTE(target_),                                                  \
      .type = COT_RGB888,                                                      \
      .target = &g_Config.target_,                                             \
      .default_value = &(RGB_888) { default_r, default_g, default_b },         \
      .param = nullptr },

#define X_CFG_STRING(target_, default_value_)                                  \
    { .name = QUOTE(target_),                                                  \
      .type = COT_STRING,                                                      \
      .target = &g_Config.target_,                                             \
      .default_value = default_value_,                                         \
      .param = nullptr },

#define X_CFG_STRING_EX(name_, target_, default_value_)                        \
    { .name = name_,                                                           \
      .type = COT_STRING,                                                      \
      .target = &g_Config.target_,                                             \
      .default_value = default_value_,                                         \
      .param = nullptr },

#define X_CFG_DYNAMIC_ENUM(target_, default_value_)                            \
    { .name = QUOTE(target_),                                                  \
      .type = COT_DYNAMIC_ENUM,                                                \
      .target = &g_Config.target_,                                             \
      .default_value = default_value_,                                         \
      .param = nullptr },

#define X_CFG_DYNAMIC_ENUM_EX(name_, target_, default_value_)                  \
    { .name = name_,                                                           \
      .type = COT_DYNAMIC_ENUM,                                                \
      .target = &g_Config.target_,                                             \
      .default_value = default_value_,                                         \
      .param = nullptr },

static const CONFIG_OPTION *m_ConfigOptionMap[TR_VERSION_COUNT] = {
    [0] =
        (CONFIG_OPTION[]) {
#include <trx/config/map_tr1.def>
            {}, // sentinel
        },
    [1] =
        (CONFIG_OPTION[]) {
#include <trx/config/map_tr2.def>
            {}, // sentinel
        },
    [2] =
        (CONFIG_OPTION[]) {
#include <trx/config/map_tr3.def>
            {}, // sentinel
        },
    [3] =
        (CONFIG_OPTION[]) {
#include <trx/config/map_tr4.def>
            {}, // sentinel
        },
};

#undef X_CFG_BOOL
#undef X_CFG_INT32
#undef X_CFG_FLOAT
#undef X_CFG_FLOAT_PERCENT
#undef X_CFG_DOUBLE
#undef X_CFG_ENUM
#undef X_CFG_ENUM_EX
#undef X_CFG_RGB888
#undef X_CFG_STRING
#undef X_CFG_STRING_EX
#undef X_CFG_DYNAMIC_ENUM
#undef X_CFG_DYNAMIC_ENUM_EX

// The map handed out at runtime: the static options for this game, plus every
// option Lua declared. Rebuilt lazily, so a declaration made before the game
// version is known still lands in the map.
static VECTOR *m_RuntimeMap = nullptr;
static int32_t m_RuntimeMapVersion = 0;
static bool m_RuntimeMapDirty = true;

void Config_InvalidateOptionMap(void)
{
    m_RuntimeMapDirty = true;
}

static void M_RebuildOptionMap(void)
{
    if (m_RuntimeMap == nullptr) {
        m_RuntimeMap = Vector_Create(sizeof(CONFIG_OPTION));
    }
    Vector_Clear(m_RuntimeMap);

    for (const CONFIG_OPTION *opt = m_ConfigOptionMap[g_TRVersion - 1];
         opt->name != nullptr; opt++) {
        Vector_Add(m_RuntimeMap, opt);
    }

    const VECTOR *const dynamic_options = Config_GetDynamicOptions();
    if (dynamic_options != nullptr) {
        for (int32_t i = 0; i < dynamic_options->count; i++) {
            const CONFIG_DYNAMIC_OPTION *const dyn =
                Vector_Get((VECTOR *)dynamic_options, i);
            const CONFIG_OPTION option = {
                .name = dyn->name,
                .type = dyn->type,
                .target = dyn->target,
                .default_value = dyn->default_value,
                .param = nullptr,
            };
            Vector_Add(m_RuntimeMap, &option);
        }
    }

    Vector_Add(m_RuntimeMap, &(CONFIG_OPTION) {}); // sentinel

    m_RuntimeMapVersion = g_TRVersion;
    m_RuntimeMapDirty = false;
}

const CONFIG_OPTION *Config_GetOptionMap(void)
{
    if (g_TRVersion < 1 || g_TRVersion > TR_VERSION_COUNT) {
        return nullptr;
    }
    if (m_RuntimeMapDirty || m_RuntimeMapVersion != g_TRVersion) {
        M_RebuildOptionMap();
    }
    return Vector_GetData(m_RuntimeMap);
}

const char *Config_ResolveOptionName(const char *option_name)
{
    const char *dot = strrchr(option_name, '.');
    if (dot) {
        return dot + 1;
    }
    return option_name;
}

const CONFIG_OPTION *Config_GetOptionByPath(const char *const path)
{
    if (path == nullptr) {
        return nullptr;
    }
    for (const CONFIG_OPTION *opt = Config_GetOptionMap(); opt->name != nullptr;
         opt++) {
        if (strcmp(opt->name, path) == 0
            || strcmp(Config_ResolveOptionName(opt->name), path) == 0) {
            return opt;
        }
    }
    return nullptr;
}
