#include <trx/game/ui/dialogs/graphic_settings.h>

#include <trx/game/ui/dialogs/settings_catalog.h>
#include <trx/game/ui/dialogs/settings_tabs.h>

static UI_SETTINGS_TAB M_MakeTab(const UI_SETTINGS_GROUP_ID group_id)
{
    const UI_SETTINGS_GROUP *const group =
        UI_SettingsCatalog_GetGroup(group_id);
    return UI_SettingsTab_MakeEditor(group->header_gs, group->options);
}

UI_SETTINGS_DIALOG_STATE *UI_GraphicSettings_Init(void)
{
    const UI_SETTINGS_TAB tabs[] = {
        M_MakeTab(UI_SETTINGS_GROUP_GRAPHIC_VISUALS),
        M_MakeTab(UI_SETTINGS_GROUP_GRAPHIC_UI),
        M_MakeTab(UI_SETTINGS_GROUP_GRAPHIC_UI_STATS),
        M_MakeTab(UI_SETTINGS_GROUP_GRAPHIC_UI_BARS),
        M_MakeTab(UI_SETTINGS_GROUP_GRAPHIC_RENDERING),
    };

    return UI_SettingsDialog_Init(
        GS_ID("general/settings/graphic_settings/title"), ARRAY_SIZE(tabs),
        tabs);
}

void UI_GraphicSettings_Free(UI_SETTINGS_DIALOG_STATE *const s)
{
    UI_SettingsDialog_Free(s);
}

bool UI_GraphicSettings_Control(UI_SETTINGS_DIALOG_STATE *const s)
{
    return UI_SettingsDialog_Control(s);
}

void UI_GraphicSettings(UI_SETTINGS_DIALOG_STATE *const s)
{
    UI_SettingsDialog(s);
}
