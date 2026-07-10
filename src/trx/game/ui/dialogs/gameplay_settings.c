#include <trx/game/ui/dialogs/gameplay_settings.h>

#include <trx/game/ui/dialogs/settings_catalog.h>
#include <trx/game/ui/dialogs/settings_tabs.h>

static UI_SETTINGS_TAB M_MakeTab(const UI_SETTINGS_GROUP_ID group_id)
{
    const UI_SETTINGS_GROUP *const group =
        UI_SettingsCatalog_GetGroup(group_id);
    return UI_SettingsTab_MakeEditor(group->header_gs, group->options);
}

UI_SETTINGS_DIALOG_STATE *UI_GameplaySettings_Init(void)
{
    const UI_SETTINGS_TAB tabs[] = {
        M_MakeTab(UI_SETTINGS_GROUP_GAMEPLAY_GENERAL),
        M_MakeTab(UI_SETTINGS_GROUP_GAMEPLAY_CONTROLS),
        M_MakeTab(UI_SETTINGS_GROUP_GAMEPLAY_MODS),
        M_MakeTab(UI_SETTINGS_GROUP_GAMEPLAY_FIXES),
        UI_SettingsTab_MakePresets(
            GS_ID("general/settings/gameplay/tabs/presets")),
    };

    return UI_SettingsDialog_Init(
        GS_ID("general/settings/gameplay/title"), ARRAY_SIZE(tabs), tabs);
}

void UI_GameplaySettings_Free(UI_SETTINGS_DIALOG_STATE *const s)
{
    UI_SettingsDialog_Free(s);
}

bool UI_GameplaySettings_Control(UI_SETTINGS_DIALOG_STATE *const s)
{
    return UI_SettingsDialog_Control(s);
}

void UI_GameplaySettings(UI_SETTINGS_DIALOG_STATE *const s)
{
    UI_SettingsDialog(s);
}
