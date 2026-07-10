#pragma once

// The single home of all data-driven setting option declarations,
// shared by the per-category settings dialogs and the combined
// settings screen.

#include <trx/game/ui/dialogs/settings.h>

typedef enum {
    UI_SETTINGS_GROUP_GAMEPLAY_GENERAL,
    UI_SETTINGS_GROUP_GAMEPLAY_CONTROLS,
    UI_SETTINGS_GROUP_GAMEPLAY_MODS,
    UI_SETTINGS_GROUP_GAMEPLAY_FIXES,
    UI_SETTINGS_GROUP_GRAPHIC_VISUALS,
    UI_SETTINGS_GROUP_GRAPHIC_UI,
    UI_SETTINGS_GROUP_GRAPHIC_UI_STATS,
    UI_SETTINGS_GROUP_GRAPHIC_UI_BARS,
    UI_SETTINGS_GROUP_GRAPHIC_RENDERING,
    UI_SETTINGS_GROUP_SOUND_VOLUME,
    UI_SETTINGS_GROUP_SOUND_MISC,
    UI_SETTINGS_GROUP_COUNT,
} UI_SETTINGS_GROUP_ID;

typedef struct {
    GAME_STRING_ID header_gs;
    // Terminated by an entry with .target == nullptr.
    const UI_SETTINGS_OPTION *options;
} UI_SETTINGS_GROUP;

const UI_SETTINGS_GROUP *UI_SettingsCatalog_GetGroup(UI_SETTINGS_GROUP_ID id);
