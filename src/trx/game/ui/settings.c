#include <trx/game/ui/settings.h>

#include <trx/config.h>
#include <trx/core/json/util/file.h>
#include <trx/core/json/util/read_io.h>
#include <trx/core/memory.h>
#include <trx/core/strings.h>
#include <trx/core/utils.h>
#include <trx/game/game_strings/entries.h>
#include <trx/game/shell.h>
#include <trx/game/ui/text.h>
#include <trx/version.h>

#include <string.h>
#include <uthash.h>

typedef struct {
    char *name;
    UI_BAR_THEME theme;
} M_THEME_ENTRY;

typedef struct M_THEME_LOOKUP {
    char *name;
    int32_t index;
    UT_hash_handle hh;
} M_THEME_LOOKUP;

typedef struct {
    int32_t color_count;
    M_THEME_ENTRY *colors;
    struct M_THEME_LOOKUP *lookup;
} M_THEME_GROUP;

typedef struct {
    char *name;
    char *name_gs;
    UI_BAR_THEME_KIND kind;
    M_THEME_GROUP group;
} M_BAR_THEME_ENTRY;

typedef struct M_BAR_THEME_LOOKUP {
    char *name;
    int32_t index;
    UT_hash_handle hh;
} M_BAR_THEME_LOOKUP;

typedef struct {
    int32_t bar_theme_count;
    M_BAR_THEME_ENTRY *bar_themes;
    struct M_BAR_THEME_LOOKUP *bar_lookup;
} M_SETTINGS;

typedef struct {
    char *const *const pc_color;
    char *const *const ps1_color;
} M_BAR_COLOR_SELECT;

static const M_BAR_COLOR_SELECT m_BarColorSelect[UI_BAR_NUMBER_OF] = {
    [UI_BAR_LARA_HP] = {
        .pc_color = &g_Config.ui.lara_health_bar.color,
        .ps1_color = &g_Config.ui.lara_health_bar.color_ps1,
    },
    [UI_BAR_LARA_HP_POISON] = {
        .pc_color = &g_Config.ui.lara_health_bar.poison_color,
        .ps1_color = &g_Config.ui.lara_health_bar.poison_color_ps1,
    },
    [UI_BAR_LARA_AIR] = {
        .pc_color = &g_Config.ui.lara_air_bar.color,
        .ps1_color = &g_Config.ui.lara_air_bar.color_ps1,
    },
    [UI_BAR_LARA_STAMINA] = {
        .pc_color = &g_Config.ui.lara_sprint_bar.color,
        .ps1_color = &g_Config.ui.lara_sprint_bar.color_ps1,
    },
    [UI_BAR_LARA_EXPOSURE] = {
        .pc_color = &g_Config.ui.lara_exposure_bar.color,
        .ps1_color = &g_Config.ui.lara_exposure_bar.color_ps1,
    },
    [UI_BAR_ENEMY_HP] = {
        .pc_color = &g_Config.ui.enemy_health_bar.color,
        .ps1_color = &g_Config.ui.enemy_health_bar.color_ps1,
    },
    [UI_BAR_ALLY_HP] = {
        .pc_color = &g_Config.ui.enemy_health_bar.color_allies,
        .ps1_color = &g_Config.ui.enemy_health_bar.color_allies_ps1,
    },
};

static M_SETTINGS m_Settings;

static UI_MENU_COLORS_PC m_MenuColorsPC[3]; // indexed [g_TRVersion - 1]
static UI_MENU_COLORS_PS1 m_MenuColorsPS1[3]; // indexed [g_TRVersion - 1]

typedef struct M_TEXT_COLOR_LOOKUP {
    char *name;
    int32_t index;
    UT_hash_handle hh;
} M_TEXT_COLOR_LOOKUP;

typedef struct {
    char *name;
    int32_t role_color[UI_TEXT_ROLE_NUMBER_OF]; // -1 = classic behavior
} M_TEXT_STYLE;

static struct {
    int32_t count;
    UI_TEXT_COLOR *colors;
    M_TEXT_COLOR_LOOKUP *lookup; // color names and aliases alike
    bool gradient[5]; // indexed [g_TRVersion - 1]
    int32_t style_count;
    M_TEXT_STYLE *styles;
} m_TextColors;

static const char *const m_TextRoleNames[UI_TEXT_ROLE_NUMBER_OF] = {
    [UI_TEXT_ROLE_NORMAL] = "normal",
    [UI_TEXT_ROLE_SELECTED] = "selected",
    [UI_TEXT_ROLE_HEADING] = "heading",
    [UI_TEXT_ROLE_VALUE] = "value",
};

// Compiled-in defaults matching the legacy palette, used when the
// settings file predates the "text" section.
typedef struct {
    const char *name;
    RGB_888 light;
    RGB_888 dark;
} M_TEXT_COLOR_DEFAULT;

static const M_TEXT_COLOR_DEFAULT m_TextColorDefaults[] = {
    // clang-format off
    { "default",     { 0xFF, 0xFF, 0xFF }, { 0x80, 0x80, 0x80 } },
    { "yellow",      { 0xB0, 0xB0, 0x00 }, { 0x50, 0x50, 0x00 } },
    { "grey",        { 0xA0, 0xA0, 0xA0 }, { 0x18, 0x18, 0x18 } },
    { "red",         { 0xFF, 0x60, 0x60 }, { 0x18, 0x00, 0x00 } },
    { "blue",        { 0x80, 0x80, 0xFF }, { 0x00, 0x00, 0x18 } },
    { "gold",        { 0xC0, 0x80, 0x40 }, { 0x40, 0x10, 0x00 } },
    { "green",       { 0xB6, 0xD1, 0x64 }, { 0xB6, 0x20, 0x13 } },
    { "pale_green",  { 0xC0, 0xFF, 0xC0 }, { 0xC0, 0xFF, 0xC0 } },
    { "white",       { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF } },
    { "placeholder", { 0xFF, 0x00, 0xFF }, { 0x3F, 0x00, 0x3F } },
    { "dim",         { 0x80, 0x80, 0x80 }, { 0x80, 0x80, 0x80 } },
    // clang-format on
};

static const char *const m_TextColorDefaultAliases[][2] = {
    // clang-format off
    { "0", "default" }, { "1", "yellow" },      { "2", "grey" },
    { "3", "red" },     { "4", "blue" },        { "5", "gold" },
    { "6", "green" },   { "7", "pale_green" },  { "8", "white" },
    { "9", "placeholder" }, { "10", "placeholder" },
    { "11", "placeholder" },
    // clang-format on
};

static void M_ExitWithJSONError(
    const char *const source_path, const JSON_READ_IO *const io)
{
    JSONFile_ExitWithReadIOError(
        io, String_FormatStatic("%s: ui settings parse error", source_path));
}

static void M_FreeThemeGroup(M_THEME_GROUP *const group)
{
    M_THEME_LOOKUP *entry = nullptr;
    M_THEME_LOOKUP *tmp = nullptr;
    HASH_ITER(hh, group->lookup, entry, tmp)
    {
        HASH_DEL(group->lookup, entry);
        Memory_FreePointer(&entry);
    }
    if (group->colors == nullptr) {
        return;
    }
    for (int32_t i = 0; i < group->color_count; i++) {
        Memory_FreePointer(&group->colors[i].name);
    }
    Memory_FreePointer(&group->colors);
    group->color_count = 0;
    group->lookup = nullptr;
}

static void M_ResetDynamicEnumValues(void)
{
    const CONFIG_OPTION *const bar_look_option =
        Config_GetOption(&g_Config.ui.bar_look);
    if (bar_look_option != nullptr) {
        Config_DynamicEnum_ResetValues(bar_look_option);
    }

    for (int32_t i = 0; i < UI_BAR_NUMBER_OF; i++) {
        const M_BAR_COLOR_SELECT *const select = &m_BarColorSelect[i];
        const CONFIG_OPTION *const pc_option =
            Config_GetOption(select->pc_color);
        if (pc_option != nullptr) {
            Config_DynamicEnum_ResetValues(pc_option);
        }
        const CONFIG_OPTION *const ps1_option =
            Config_GetOption(select->ps1_color);
        if (ps1_option != nullptr) {
            Config_DynamicEnum_ResetValues(ps1_option);
        }
    }
}

static bool M_IsBarColorNameEncountered(
    const UI_BAR_THEME_KIND kind, const char *const name, const int32_t stop_i,
    const int32_t stop_j)
{
    for (int32_t i = 0; i < m_Settings.bar_theme_count; i++) {
        M_BAR_THEME_ENTRY *const theme = &m_Settings.bar_themes[i];
        if (theme->kind != kind) {
            continue;
        }
        for (int32_t j = 0; j < theme->group.color_count; j++) {
            if (i == stop_i && j == stop_j) {
                return false;
            }
            if (String_Equivalent(theme->group.colors[j].name, name)) {
                return true;
            }
        }
    }
    return false;
}

static void M_SeedDynamicEnumBarColors(
    const CONFIG_OPTION *const option, const UI_BAR_THEME_KIND kind)
{
    Config_DynamicEnum_ResetValues(option);
    for (int32_t i = 0; i < m_Settings.bar_theme_count; i++) {
        const M_BAR_THEME_ENTRY *const theme = &m_Settings.bar_themes[i];
        if (theme->kind != kind) {
            continue;
        }
        for (int32_t j = 0; j < theme->group.color_count; j++) {
            const char *const name = theme->group.colors[j].name;
            if (M_IsBarColorNameEncountered(kind, name, i, j)) {
                continue;
            }
            Config_DynamicEnum_AddValue(option, name, nullptr);
        }
    }
}

static void M_SeedDynamicEnumValues(void)
{
    const CONFIG_OPTION *const bar_look_option =
        Config_GetOption(&g_Config.ui.bar_look);
    if (bar_look_option != nullptr) {
        Config_DynamicEnum_ResetValues(bar_look_option);
        for (int32_t i = 0; i < m_Settings.bar_theme_count; i++) {
            const M_BAR_THEME_ENTRY *const theme = &m_Settings.bar_themes[i];
            Config_DynamicEnum_AddValue(
                bar_look_option, theme->name, theme->name_gs);
        }
    }

    for (int32_t i = 0; i < UI_BAR_NUMBER_OF; i++) {
        const M_BAR_COLOR_SELECT *const select = &m_BarColorSelect[i];
        M_SeedDynamicEnumBarColors(
            Config_GetOption(select->pc_color), UI_BAR_THEME_PC_KIND);
        M_SeedDynamicEnumBarColors(
            Config_GetOption(select->ps1_color), UI_BAR_THEME_PS1_KIND);
    }
}

static void M_FreeBarThemes(void)
{
    M_ResetDynamicEnumValues();

    M_BAR_THEME_LOOKUP *entry = nullptr;
    M_BAR_THEME_LOOKUP *tmp = nullptr;
    HASH_ITER(hh, m_Settings.bar_lookup, entry, tmp)
    {
        HASH_DEL(m_Settings.bar_lookup, entry);
        Memory_FreePointer(&entry);
    }

    if (m_Settings.bar_themes == nullptr) {
        return;
    }

    for (int32_t i = 0; i < m_Settings.bar_theme_count; i++) {
        M_BAR_THEME_ENTRY *const theme = &m_Settings.bar_themes[i];
        Memory_FreePointer(&theme->name);
        Memory_FreePointer(&theme->name_gs);
        M_FreeThemeGroup(&theme->group);
    }

    Memory_FreePointer(&m_Settings.bar_themes);
    m_Settings.bar_theme_count = 0;
    m_Settings.bar_lookup = nullptr;
}

static bool M_ReadColorArray(
    JSON_READ_IO *const io, RGBA_8888 colors[UI_BAR_COLOR_STEPS])
{
    const int32_t count = JSON_ARRAY_LEN(io);
    if (count != UI_BAR_COLOR_STEPS) {
        JSON_ReadIO_SetError(
            io, "invalid color array (expected %d entries)",
            UI_BAR_COLOR_STEPS);
        JSON_FAIL();
    }

    for (int32_t i = 0; i < UI_BAR_COLOR_STEPS; i++) {
        RGB_888 rgb = {};
        JSON_MUST(JSON_READ_A(io, i, &rgb));
        colors[i] = Color_RGBToRGBA(rgb);
    }

    JSON_FINISH();
}

static bool M_LoadThemesPC(JSON_READ_IO *const io, M_THEME_GROUP *const group)
{
    float basic_scale = 1.0f;
    RGBA_8888 border_light = {};
    RGBA_8888 border_dark = {};

    JSON_READ_D(io, "scale", &basic_scale, 1.0f);

    RGB_888 border_light_rgb = {};
    JSON_MUST(JSON_READ(io, "border_light", &border_light_rgb));
    border_light = Color_RGBToRGBA(border_light_rgb);

    RGB_888 border_dark_rgb = {};
    JSON_MUST(JSON_READ(io, "border_dark", &border_dark_rgb));
    border_dark = Color_RGBToRGBA(border_dark_rgb);

    JSON_MUST(JSON_PUSH(io, "colors"));
    JSON_OBJECT *const colors_obj = JSON_ReadIO_GetCurrentObject(io);
    if (colors_obj == nullptr) {
        JSON_ReadIO_SetError(io, "'colors' must be an object");
        JSON_MUST(JSON_POP(io));
        JSON_FAIL();
    }

    size_t count = 0;
    for (JSON_OBJECT_ELEMENT *elem = colors_obj->start; elem != nullptr;
         elem = elem->next) {
        count++;
    }
    if (count == 0) {
        JSON_ReadIO_SetError(io, "'colors' cannot be empty");
        JSON_MUST(JSON_POP(io));
        JSON_FAIL();
    }

    M_FreeThemeGroup(group);
    group->colors = Memory_Alloc(sizeof(*group->colors) * count);
    group->color_count = (int32_t)count;
    group->lookup = nullptr;

    size_t idx = 0;
    for (JSON_OBJECT_ELEMENT *elem = colors_obj->start; elem != nullptr;
         elem = elem->next) {
        const char *const name = elem->name->string;
        JSON_MUST(JSON_PUSH(io, name));

        group->colors[idx].name = Memory_DupStr(name);
        M_THEME_LOOKUP *existing = nullptr;
        HASH_FIND_STR(group->lookup, group->colors[idx].name, existing);
        if (existing != nullptr) {
            JSON_ReadIO_SetError(io, "duplicate color '%s'", name);
            JSON_MUST(JSON_POP(io));
            JSON_MUST(JSON_POP(io));
            JSON_FAIL();
        }

        M_THEME_LOOKUP *const entry = Memory_Alloc(sizeof(*entry));
        entry->name = group->colors[idx].name;
        entry->index = (int32_t)idx;
        HASH_ADD_KEYPTR(
            hh, group->lookup, entry->name, strlen(entry->name), entry);

        UI_BAR_THEME *const theme = &group->colors[idx].theme;
        *theme = (UI_BAR_THEME) {
            .kind = UI_BAR_THEME_PC_KIND,
            .basic_scale = basic_scale,
            .border_light = border_light,
            .border_dark = border_dark,
        };
        JSON_MUST(M_ReadColorArray(io, theme->ramp));
        JSON_MUST(JSON_POP(io));
        idx++;
    }

    JSON_MUST(JSON_POP(io));
    JSON_FINISH();
}

static bool M_LoadThemesPS1(JSON_READ_IO *const io, M_THEME_GROUP *const group)
{
    float basic_scale = 1.0f;

    JSON_READ_D(io, "scale", &basic_scale, 1.0f);

    RGB_888 border_tl_rgb = {};
    RGB_888 border_tr_rgb = {};
    RGB_888 border_bl_rgb = {};
    RGB_888 border_br_rgb = {};
    JSON_MUST(JSON_READ(io, "border_tl", &border_tl_rgb));
    JSON_MUST(JSON_READ(io, "border_tr", &border_tr_rgb));
    JSON_MUST(JSON_READ(io, "border_bl", &border_bl_rgb));
    JSON_MUST(JSON_READ(io, "border_br", &border_br_rgb));
    const RGBA_8888 border_tl = Color_RGBToRGBA(border_tl_rgb);
    const RGBA_8888 border_tr = Color_RGBToRGBA(border_tr_rgb);
    const RGBA_8888 border_bl = Color_RGBToRGBA(border_bl_rgb);
    const RGBA_8888 border_br = Color_RGBToRGBA(border_br_rgb);

    JSON_MUST(JSON_PUSH(io, "colors"));
    JSON_OBJECT *const colors_obj = JSON_ReadIO_GetCurrentObject(io);
    if (colors_obj == nullptr) {
        JSON_ReadIO_SetError(io, "'colors' must be an object");
        JSON_MUST(JSON_POP(io));
        JSON_FAIL();
    }

    size_t count = 0;
    for (JSON_OBJECT_ELEMENT *elem = colors_obj->start; elem != nullptr;
         elem = elem->next) {
        count++;
    }
    if (count == 0) {
        JSON_ReadIO_SetError(io, "'colors' cannot be empty");
        JSON_MUST(JSON_POP(io));
        JSON_FAIL();
    }

    M_FreeThemeGroup(group);
    group->colors = Memory_Alloc(sizeof(*group->colors) * count);
    group->color_count = (int32_t)count;
    group->lookup = nullptr;

    size_t idx = 0;
    for (JSON_OBJECT_ELEMENT *elem = colors_obj->start; elem != nullptr;
         elem = elem->next) {
        const char *const name = elem->name->string;
        JSON_MUST(JSON_PUSH(io, name));

        const int32_t ramps_count = JSON_ARRAY_LEN(io);
        if (ramps_count != 2) {
            JSON_ReadIO_SetError(
                io, "invalid '%s' color definition (expected 2 arrays)", name);
            JSON_MUST(JSON_POP(io));
            JSON_MUST(JSON_POP(io));
            JSON_FAIL();
        }

        group->colors[idx].name = Memory_DupStr(name);
        M_THEME_LOOKUP *existing = nullptr;
        HASH_FIND_STR(group->lookup, group->colors[idx].name, existing);
        if (existing != nullptr) {
            JSON_ReadIO_SetError(io, "duplicate color '%s'", name);
            JSON_MUST(JSON_POP(io));
            JSON_MUST(JSON_POP(io));
            JSON_FAIL();
        }
        M_THEME_LOOKUP *const entry = Memory_Alloc(sizeof(*entry));
        entry->name = group->colors[idx].name;
        entry->index = (int32_t)idx;
        HASH_ADD_KEYPTR(
            hh, group->lookup, entry->name, strlen(entry->name), entry);

        UI_BAR_THEME *const theme = &group->colors[idx].theme;
        *theme = (UI_BAR_THEME) {
            .kind = UI_BAR_THEME_PS1_KIND,
            .basic_scale = basic_scale,
            .border_tl = border_tl,
            .border_tr = border_tr,
            .border_bl = border_bl,
            .border_br = border_br,
        };

        JSON_MUST(JSON_PUSH_INDEX(io, 0));
        JSON_MUST(M_ReadColorArray(io, theme->ramp_left));
        JSON_MUST(JSON_POP(io));

        JSON_MUST(JSON_PUSH_INDEX(io, 1));
        JSON_MUST(M_ReadColorArray(io, theme->ramp_right));
        JSON_MUST(JSON_POP(io));

        JSON_MUST(JSON_POP(io));
        idx++;
    }

    JSON_MUST(JSON_POP(io));
    JSON_FINISH();
}

static bool M_LoadTheme(JSON_READ_IO *const io, M_BAR_THEME_ENTRY *const theme)
{
    const char *name_gs = nullptr;
    JSON_MUST(JSON_READ(io, "name_gs", &name_gs));
    theme->name_gs = Memory_DupStr(name_gs);

    const char *style = nullptr;
    JSON_MUST(JSON_READ(io, "style", &style));
    if (String_Equivalent(style, "pc")) {
        theme->kind = UI_BAR_THEME_PC_KIND;
        JSON_MUST(M_LoadThemesPC(io, &theme->group));
    } else if (String_Equivalent(style, "ps1")) {
        theme->kind = UI_BAR_THEME_PS1_KIND;
        JSON_MUST(M_LoadThemesPS1(io, &theme->group));
    } else {
        JSON_ReadIO_SetError(io, "invalid 'style' value '%s'", style);
        JSON_FAIL();
    }

    JSON_FINISH();
}

static bool M_LoadBarThemes(JSON_READ_IO *const io)
{
    JSON_OBJECT *const root_obj = JSON_ReadIO_GetCurrentObject(io);
    if (root_obj == nullptr) {
        JSON_ReadIO_SetError(
            io, "invalid ui settings file: root must be object");
        JSON_FAIL();
    }

    size_t theme_count = 0;
    for (JSON_OBJECT_ELEMENT *elem = root_obj->start; elem != nullptr;
         elem = elem->next) {
        theme_count++;
    }
    if (theme_count == 0) {
        JSON_ReadIO_SetError(io, "ui settings file has no bar themes");
        JSON_FAIL();
    }

    m_Settings.bar_themes =
        Memory_Alloc(sizeof(*m_Settings.bar_themes) * theme_count);
    m_Settings.bar_theme_count = (int32_t)theme_count;
    m_Settings.bar_lookup = nullptr;

    size_t idx = 0;
    for (JSON_OBJECT_ELEMENT *elem = root_obj->start; elem != nullptr;
         elem = elem->next) {
        const char *const theme_name = elem->name->string;
        JSON_MUST(JSON_PUSH(io, theme_name));

        M_BAR_THEME_ENTRY *const theme = &m_Settings.bar_themes[idx];
        theme->name = Memory_DupStr(theme_name);
        theme->name_gs = nullptr;
        theme->kind = UI_BAR_THEME_PC_KIND;
        theme->group = (M_THEME_GROUP) {};

        M_BAR_THEME_LOOKUP *existing = nullptr;
        HASH_FIND_STR(m_Settings.bar_lookup, theme->name, existing);
        if (existing != nullptr) {
            JSON_ReadIO_SetError(io, "duplicate theme '%s'", theme_name);
            JSON_MUST(JSON_POP(io));
            JSON_FAIL();
        }

        JSON_MUST(M_LoadTheme(io, theme));

        M_BAR_THEME_LOOKUP *const entry = Memory_Alloc(sizeof(*entry));
        entry->name = theme->name;
        entry->index = (int32_t)idx;
        HASH_ADD_KEYPTR(
            hh, m_Settings.bar_lookup, entry->name, strlen(entry->name), entry);

        JSON_MUST(JSON_POP(io));
        idx++;
    }

    JSON_FINISH();
}

static M_BAR_THEME_ENTRY *M_FindBarThemeByName(const char *const name)
{
    if (name == nullptr) {
        return nullptr;
    }
    M_BAR_THEME_LOOKUP *entry = nullptr;
    HASH_FIND_STR(m_Settings.bar_lookup, name, entry);
    if (entry == nullptr) {
        return nullptr;
    }
    return &m_Settings.bar_themes[entry->index];
}

static M_BAR_THEME_ENTRY *M_GetCurrentBarTheme(void)
{
    M_BAR_THEME_ENTRY *theme = M_FindBarThemeByName(g_Config.ui.bar_look);
    if (theme != nullptr) {
        return theme;
    }
    if (m_Settings.bar_theme_count <= 0) {
        return nullptr;
    }
    return &m_Settings.bar_themes[0];
}

static const M_THEME_GROUP *M_GetCurrentBarGroup(void)
{
    M_BAR_THEME_ENTRY *const theme = M_GetCurrentBarTheme();
    if (theme == nullptr) {
        return nullptr;
    }
    return &theme->group;
}

static bool M_LoadMenuColorsPC(
    JSON_READ_IO *const io, UI_MENU_COLORS_PC *const c)
{
    JSON_MUST(JSON_PUSH(io, "background"));
    JSON_MUST(JSON_READ_A(io, 0, &c->background[0]));
    JSON_MUST(JSON_READ_A(io, 1, &c->background[1]));
    JSON_MUST(JSON_POP(io));

    JSON_MUST(JSON_PUSH(io, "background_heavy"));
    JSON_MUST(JSON_READ_A(io, 0, &c->background_heavy[0]));
    JSON_MUST(JSON_READ_A(io, 1, &c->background_heavy[1]));
    JSON_MUST(JSON_POP(io));

    JSON_MUST(JSON_READ(io, "outline_light", &c->outline_light));
    JSON_MUST(JSON_READ(io, "outline_dark", &c->outline_dark));

    JSON_FINISH();
}

static bool M_LoadMenuColorsPS1(
    JSON_READ_IO *const io, UI_MENU_COLORS_PS1 *const c)
{
    JSON_MUST(JSON_READ(io, "background_edge", &c->background_edge));
    JSON_MUST(JSON_READ(io, "background_center", &c->background_center));
    JSON_MUST(
        JSON_READ(io, "background_heavy_edge", &c->background_heavy_edge));
    JSON_MUST(
        JSON_READ(io, "background_heavy_center", &c->background_heavy_center));
    JSON_MUST(JSON_READ(io, "heading_edge", &c->heading_edge));
    JSON_MUST(JSON_READ(io, "heading_center", &c->heading_center));
    JSON_MUST(JSON_READ(io, "requested_edge", &c->requested_edge));
    JSON_MUST(JSON_READ(io, "requested_center", &c->requested_center));
    JSON_MUST(JSON_READ(io, "requested_outline_ch", &c->requested_outline_ch));
    JSON_MUST(JSON_READ(io, "requested_outline_cv", &c->requested_outline_cv));
    JSON_MUST(
        JSON_READ(io, "requested_outline_edge", &c->requested_outline_edge));
    JSON_MUST(JSON_READ(io, "outline_tl", &c->outline_tl));
    JSON_MUST(JSON_READ(io, "outline_tr", &c->outline_tr));
    JSON_MUST(JSON_READ(io, "outline_bl", &c->outline_bl));
    JSON_MUST(JSON_READ(io, "outline_br", &c->outline_br));
    JSON_MUST(JSON_READ(io, "heading_outline", &c->heading_outline));

    JSON_FINISH();
}

static bool M_LoadMenuColors(JSON_READ_IO *const io)
{
    static const char *const tr_keys[] = { "tr1", "tr2", "tr3" };

    for (int32_t i = 0; i < 3; i++) {
        JSON_MUST(JSON_PUSH(io, tr_keys[i]));

        JSON_MUST(JSON_PUSH(io, "pc"));
        JSON_MUST(M_LoadMenuColorsPC(io, &m_MenuColorsPC[i]));
        JSON_MUST(JSON_POP(io));

        JSON_MUST(JSON_PUSH(io, "ps1"));
        JSON_MUST(M_LoadMenuColorsPS1(io, &m_MenuColorsPS1[i]));
        JSON_MUST(JSON_POP(io));

        JSON_MUST(JSON_POP(io));
    }

    JSON_FINISH();
}

static RGBA_F M_TextColorToRGBAF(const RGB_888 color)
{
    return (RGBA_F) {
        .r = color.r / 255.0f,
        .g = color.g / 255.0f,
        .b = color.b / 255.0f,
        .a = 1.0f,
    };
}

static void M_FreeTextColors(void)
{
    M_TEXT_COLOR_LOOKUP *entry = nullptr;
    M_TEXT_COLOR_LOOKUP *tmp = nullptr;
    HASH_ITER(hh, m_TextColors.lookup, entry, tmp)
    {
        HASH_DEL(m_TextColors.lookup, entry);
        Memory_FreePointer(&entry->name);
        Memory_FreePointer(&entry);
    }
    for (int32_t i = 0; i < m_TextColors.count; i++) {
        Memory_FreePointer(&m_TextColors.colors[i].name);
    }
    Memory_FreePointer(&m_TextColors.colors);
    m_TextColors.count = 0;
    memset(m_TextColors.gradient, 0, sizeof(m_TextColors.gradient));
    for (int32_t i = 0; i < m_TextColors.style_count; i++) {
        Memory_FreePointer(&m_TextColors.styles[i].name);
    }
    Memory_FreePointer(&m_TextColors.styles);
    m_TextColors.style_count = 0;
}

static void M_AddTextColorAlias(const char *const name, const int32_t index)
{
    M_TEXT_COLOR_LOOKUP *entry = nullptr;
    HASH_FIND_STR(m_TextColors.lookup, name, entry);
    if (entry != nullptr) {
        entry->index = index;
        return;
    }
    entry = Memory_Alloc(sizeof(*entry));
    entry->name = Memory_DupStr(name);
    entry->index = index;
    HASH_ADD_KEYPTR(hh, m_TextColors.lookup, entry->name, strlen(name), entry);
}

static void M_LoadTextColorDefaults(void)
{
    m_TextColors.count = ARRAY_SIZE(m_TextColorDefaults);
    m_TextColors.colors =
        Memory_Alloc(sizeof(UI_TEXT_COLOR) * m_TextColors.count);
    for (int32_t i = 0; i < m_TextColors.count; i++) {
        const M_TEXT_COLOR_DEFAULT *const def = &m_TextColorDefaults[i];
        m_TextColors.colors[i] = (UI_TEXT_COLOR) {
            .name = Memory_DupStr(def->name),
            .light = M_TextColorToRGBAF(def->light),
            .dark = M_TextColorToRGBAF(def->dark),
            .pulse = false,
        };
        M_AddTextColorAlias(def->name, i);
    }
    for (size_t i = 0; i < ARRAY_SIZE(m_TextColorDefaultAliases); i++) {
        const UI_TEXT_COLOR *const target =
            UI_Settings_GetTextColorByName(m_TextColorDefaultAliases[i][1]);
        if (target != nullptr) {
            M_AddTextColorAlias(
                m_TextColorDefaultAliases[i][0],
                (int32_t)(target - m_TextColors.colors));
        }
    }
    m_TextColors.gradient[2] = true; // TR3
    m_TextColors.gradient[3] = true; // TR4
}

static bool M_LoadTextColors(JSON_READ_IO *const io)
{
    JSON_MUST(JSON_PUSH(io, "colors"));
    JSON_OBJECT *const colors_obj = JSON_ReadIO_GetCurrentObject(io);
    if (colors_obj == nullptr) {
        JSON_ReadIO_SetError(io, "'text.colors' must be an object");
        JSON_FAIL();
    }

    size_t color_count = 0;
    for (JSON_OBJECT_ELEMENT *elem = colors_obj->start; elem != nullptr;
         elem = elem->next) {
        color_count++;
    }
    if (color_count == 0) {
        JSON_ReadIO_SetError(io, "'text.colors' has no entries");
        JSON_FAIL();
    }

    m_TextColors.colors = Memory_Alloc(sizeof(UI_TEXT_COLOR) * color_count);
    m_TextColors.count = (int32_t)color_count;

    int32_t idx = 0;
    for (JSON_OBJECT_ELEMENT *elem = colors_obj->start; elem != nullptr;
         elem = elem->next) {
        const char *const color_name = elem->name->string;
        JSON_MUST(JSON_PUSH(io, color_name));

        RGB_888 light = {};
        RGB_888 dark = {};
        bool pulse = false;
        JSON_MUST(JSON_READ(io, "light", &light));
        dark = light;
        JSON_OPTIONAL(JSON_READ(io, "dark", &dark));
        JSON_OPTIONAL(JSON_READ(io, "pulse", &pulse));

        m_TextColors.colors[idx] = (UI_TEXT_COLOR) {
            .name = Memory_DupStr(color_name),
            .light = M_TextColorToRGBAF(light),
            .dark = M_TextColorToRGBAF(dark),
            .pulse = pulse,
        };
        M_AddTextColorAlias(color_name, idx);
        idx++;

        JSON_MUST(JSON_POP(io));
    }
    JSON_MUST(JSON_POP(io)); // colors

    if (JSON_PUSH(io, "aliases")) {
        JSON_OBJECT *const aliases_obj = JSON_ReadIO_GetCurrentObject(io);
        for (JSON_OBJECT_ELEMENT *elem =
                 aliases_obj != nullptr ? aliases_obj->start : nullptr;
             elem != nullptr; elem = elem->next) {
            const char *target_name = nullptr;
            JSON_MUST(JSON_READ(io, elem->name->string, &target_name));
            const UI_TEXT_COLOR *const target =
                UI_Settings_GetTextColorByName(target_name);
            if (target == nullptr) {
                JSON_ReadIO_SetError(
                    io, "'text.aliases.%s' references unknown color '%s'",
                    elem->name->string, target_name);
                JSON_FAIL();
            }
            M_AddTextColorAlias(
                elem->name->string, (int32_t)(target - m_TextColors.colors));
        }
        JSON_MUST(JSON_POP(io));
    }

    if (JSON_PUSH(io, "gradient_versions")) {
        const int32_t len = JSON_ARRAY_LEN(io);
        for (int32_t i = 0; i < len; i++) {
            int32_t version = 0;
            JSON_MUST(JSON_READ_A(io, i, &version));
            if (version >= 1 && version <= 5) {
                m_TextColors.gradient[version - 1] = true;
            }
        }
        JSON_MUST(JSON_POP(io));
    }

    if (JSON_PUSH(io, "styles")) {
        JSON_OBJECT *const styles_obj = JSON_ReadIO_GetCurrentObject(io);
        size_t style_count = 0;
        for (JSON_OBJECT_ELEMENT *elem =
                 styles_obj != nullptr ? styles_obj->start : nullptr;
             elem != nullptr; elem = elem->next) {
            style_count++;
        }
        if (style_count > 0) {
            m_TextColors.styles =
                Memory_Alloc(sizeof(M_TEXT_STYLE) * style_count);
            m_TextColors.style_count = (int32_t)style_count;
        }

        int32_t style_idx = 0;
        for (JSON_OBJECT_ELEMENT *elem =
                 styles_obj != nullptr ? styles_obj->start : nullptr;
             elem != nullptr; elem = elem->next) {
            M_TEXT_STYLE *const style = &m_TextColors.styles[style_idx++];
            style->name = Memory_DupStr(elem->name->string);
            for (int32_t i = 0; i < UI_TEXT_ROLE_NUMBER_OF; i++) {
                style->role_color[i] = -1;
            }

            JSON_MUST(JSON_PUSH(io, elem->name->string));
            for (int32_t role = 0; role < UI_TEXT_ROLE_NUMBER_OF; role++) {
                if (m_TextRoleNames[role] == nullptr) {
                    continue;
                }
                const char *color_name = nullptr;
                if (!JSON_OPTIONAL(
                        JSON_READ(io, m_TextRoleNames[role], &color_name))) {
                    continue;
                }
                const UI_TEXT_COLOR *const color =
                    UI_Settings_GetTextColorByName(color_name);
                if (color == nullptr) {
                    JSON_ReadIO_SetError(
                        io,
                        "'text.styles.%s.%s' references unknown color "
                        "'%s'",
                        style->name, m_TextRoleNames[role], color_name);
                    JSON_FAIL();
                }
                style->role_color[role] =
                    (int32_t)(color - m_TextColors.colors);
            }
            JSON_MUST(JSON_POP(io));
        }
        JSON_MUST(JSON_POP(io));
    }

    JSON_FINISH();
}

static void M_RegisterTextColorMarkers(void)
{
    M_TEXT_COLOR_LOOKUP *entry = nullptr;
    M_TEXT_COLOR_LOOKUP *tmp = nullptr;
    HASH_ITER(hh, m_TextColors.lookup, entry, tmp)
    {
        UI_Text_RegisterColorMarker(
            String_FormatStatic("\\{color %s}", entry->name), entry->index);
    }
}

void UI_Settings_LoadFromFile(const char *const path)
{
    JSON_VALUE *const root = JSONFile_ReadEx(path, true);
    JSON_READ_IO *const io = JSON_ReadIO_Create(root, 0, path);

    M_FreeBarThemes();
    if (!JSON_PUSH(io, "bars") || !M_LoadBarThemes(io) || !JSON_POP(io)) {
        M_ExitWithJSONError(path, io);
    }

    if (!JSON_PUSH(io, "ui") || !M_LoadMenuColors(io) || !JSON_POP(io)) {
        M_ExitWithJSONError(path, io);
    }

    M_FreeTextColors();
    if (JSON_PUSH(io, "text")) {
        if (!M_LoadTextColors(io) || !JSON_POP(io)) {
            M_ExitWithJSONError(path, io);
        }
    } else {
        M_LoadTextColorDefaults();
    }
    M_RegisterTextColorMarkers();

    M_SeedDynamicEnumValues();

    JSON_ReadIO_Destroy(io);
    JSON_ValueFree(root);
}

__attribute__((destructor)) static void M_Shutdown(void)
{
    M_FreeBarThemes();
}

static const char *M_GetBarColorName(const UI_BAR_TYPE type)
{
    if (type < 0 || type >= UI_BAR_NUMBER_OF) {
        return "gold";
    }

    const M_BAR_THEME_ENTRY *const theme = M_GetCurrentBarTheme();
    const bool use_ps1 =
        theme != nullptr && theme->kind == UI_BAR_THEME_PS1_KIND;
    const M_BAR_COLOR_SELECT *const select = &m_BarColorSelect[type];
    const char *value = nullptr;

    if (use_ps1 && select->ps1_color != nullptr) {
        value = *select->ps1_color;
    } else if (!use_ps1 && select->pc_color != nullptr) {
        value = *select->pc_color;
    }

    return value;
}

static const UI_BAR_THEME *M_FindThemeByName(
    const M_THEME_GROUP *const group, const char *const name)
{
    if (group == nullptr || group->colors == nullptr || group->color_count <= 0
        || name == nullptr) {
        return nullptr;
    }
    M_THEME_LOOKUP *entry = nullptr;
    HASH_FIND_STR(group->lookup, name, entry);
    if (entry != nullptr) {
        return &group->colors[entry->index].theme;
    }
    return nullptr;
}

bool UI_Settings_IsCurrentBarLookPS1(void)
{
    const M_BAR_THEME_ENTRY *const theme = M_GetCurrentBarTheme();
    return theme != nullptr && theme->kind == UI_BAR_THEME_PS1_KIND;
}

const UI_BAR_THEME *UI_Settings_GetBarTheme(const UI_BAR_TYPE type)
{
    if (type < 0 || type >= UI_BAR_NUMBER_OF) {
        return nullptr;
    }
    const M_THEME_GROUP *const group = M_GetCurrentBarGroup();
    if (group == nullptr || group->color_count <= 0) {
        return nullptr;
    }
    const char *const name = M_GetBarColorName(type);
    const UI_BAR_THEME *theme = M_FindThemeByName(group, name);
    if (theme != nullptr) {
        return theme;
    }
    return &group->colors[0].theme;
}

const UI_MENU_COLORS_PC *UI_Settings_GetMenuColorsPC(void)
{
    return &m_MenuColorsPC[g_TRVersion - 1];
}

const UI_MENU_COLORS_PS1 *UI_Settings_GetMenuColorsPS1(void)
{
    return &m_MenuColorsPS1[g_TRVersion - 1];
}

int32_t UI_Settings_GetTextColorCount(void)
{
    return m_TextColors.count;
}

const UI_TEXT_COLOR *UI_Settings_GetTextColorByIndex(const int32_t idx)
{
    if (idx < 0 || idx >= m_TextColors.count) {
        return nullptr;
    }
    return &m_TextColors.colors[idx];
}

const UI_TEXT_COLOR *UI_Settings_GetTextColorByName(const char *const name)
{
    if (name == nullptr) {
        return nullptr;
    }
    M_TEXT_COLOR_LOOKUP *entry = nullptr;
    HASH_FIND_STR(m_TextColors.lookup, name, entry);
    if (entry == nullptr) {
        return nullptr;
    }
    return UI_Settings_GetTextColorByIndex(entry->index);
}

bool UI_Settings_GetTextGradient(void)
{
    if (g_TRVersion < 1 || g_TRVersion > 5) {
        return false;
    }
    return m_TextColors.gradient[g_TRVersion - 1];
}

const UI_TEXT_COLOR *UI_Settings_GetTextStyleRoleColor(
    const char *const profile, const int32_t role)
{
    if (profile == nullptr || role < 0 || role >= UI_TEXT_ROLE_NUMBER_OF) {
        return nullptr;
    }
    for (int32_t i = 0; i < m_TextColors.style_count; i++) {
        if (String_Equivalent(m_TextColors.styles[i].name, profile)) {
            const int32_t color_idx = m_TextColors.styles[i].role_color[role];
            return UI_Settings_GetTextColorByIndex(color_idx);
        }
    }
    return nullptr;
}
