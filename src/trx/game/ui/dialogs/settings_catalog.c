#include <trx/game/ui/dialogs/settings_catalog.h>

#include <trx/config.h>
#include <trx/debug.h>
#include <trx/game/lara/const.h>
#include <trx/game/ui/dialogs/setting_helpers/enums.h>
#include <trx/game/ui/dialogs/setting_helpers/handlers.h>

static const UI_SETTINGS_OPTION m_GameplayGeneralOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/gameplay_general.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GameplayControlOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/gameplay_controls.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GameplayModOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/gameplay_mods.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GameplayFixOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/gameplay_fixes.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GraphicVisualsOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/graphic_visuals.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GraphicUIOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/graphic_ui.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GraphicUIStatsOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/graphic_ui_stats.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GraphicUIBarsOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/graphic_ui_bars.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_GraphicRenderOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/graphic_rendering.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_SoundVolumeOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/sound_volume.def>
    { .target = nullptr },
};

static const UI_SETTINGS_OPTION m_SoundMiscOptions[] = {
#include <trx/game/ui/dialogs/setting_tabs/sound_misc.def>
    { .target = nullptr },
};

static const UI_SETTINGS_GROUP m_Groups[UI_SETTINGS_GROUP_COUNT] = {
    [UI_SETTINGS_GROUP_GAMEPLAY_GENERAL] = {
        .header_gs = GS_ID("general/settings/gameplay/tabs/general"),
        .options = m_GameplayGeneralOptions,
    },
    [UI_SETTINGS_GROUP_GAMEPLAY_CONTROLS] = {
        .header_gs = GS_ID("general/settings/gameplay/tabs/controls"),
        .options = m_GameplayControlOptions,
    },
    [UI_SETTINGS_GROUP_GAMEPLAY_MODS] = {
        .header_gs = GS_ID("general/settings/gameplay/tabs/mods"),
        .options = m_GameplayModOptions,
    },
    [UI_SETTINGS_GROUP_GAMEPLAY_FIXES] = {
        .header_gs = GS_ID("general/settings/gameplay/tabs/fixes"),
        .options = m_GameplayFixOptions,
    },
    [UI_SETTINGS_GROUP_GRAPHIC_VISUALS] = {
        .header_gs = GS_ID("general/settings/graphic_settings/tabs/visuals"),
        .options = m_GraphicVisualsOptions,
    },
    [UI_SETTINGS_GROUP_GRAPHIC_UI] = {
        .header_gs = GS_ID("general/settings/graphic_settings/tabs/ui"),
        .options = m_GraphicUIOptions,
    },
    [UI_SETTINGS_GROUP_GRAPHIC_UI_STATS] = {
        .header_gs = GS_ID("general/settings/graphic_settings/tabs/stats"),
        .options = m_GraphicUIStatsOptions,
    },
    [UI_SETTINGS_GROUP_GRAPHIC_UI_BARS] = {
        .header_gs = GS_ID("general/settings/graphic_settings/tabs/bars"),
        .options = m_GraphicUIBarsOptions,
    },
    [UI_SETTINGS_GROUP_GRAPHIC_RENDERING] = {
        .header_gs = GS_ID("general/settings/graphic_settings/tabs/rendering"),
        .options = m_GraphicRenderOptions,
    },
    [UI_SETTINGS_GROUP_SOUND_VOLUME] = {
        .header_gs = GS_ID("general/settings/sound/tabs/volume"),
        .options = m_SoundVolumeOptions,
    },
    [UI_SETTINGS_GROUP_SOUND_MISC] = {
        .header_gs = GS_ID("general/settings/sound/tabs/misc"),
        .options = m_SoundMiscOptions,
    },
};

const UI_SETTINGS_GROUP *UI_SettingsCatalog_GetGroup(
    const UI_SETTINGS_GROUP_ID id)
{
    ASSERT(id >= 0 && id < UI_SETTINGS_GROUP_COUNT);
    return &m_Groups[id];
}
