#include <trx/game/ui/dialogs/sound_settings.h>

#include <trx/game/ui/dialogs/settings_catalog.h>
#include <trx/game/ui/dialogs/settings_tabs.h>

static UI_SETTINGS_TAB M_MakeTab(const UI_SETTINGS_GROUP_ID group_id)
{
    const UI_SETTINGS_GROUP *const group =
        UI_SettingsCatalog_GetGroup(group_id);
    return UI_SettingsTab_MakeEditor(group->header_gs, group->options);
}

UI_SETTINGS_DIALOG_STATE *UI_SoundSettings_Init(void)
{
    const UI_SETTINGS_TAB tabs[] = {
        M_MakeTab(UI_SETTINGS_GROUP_SOUND_VOLUME),
        M_MakeTab(UI_SETTINGS_GROUP_SOUND_MISC),
    };

    return UI_SettingsDialog_Init(
        GS_ID("general/settings/sound/title"), ARRAY_SIZE(tabs), tabs);
}

void UI_SoundSettings_Free(UI_SETTINGS_DIALOG_STATE *const s)
{
    UI_SettingsDialog_Free(s);
}

bool UI_SoundSettings_Control(UI_SETTINGS_DIALOG_STATE *const s)
{
    return UI_SettingsDialog_Control(s);
}

void UI_SoundSettings(UI_SETTINGS_DIALOG_STATE *const s)
{
    UI_SettingsDialog(s);
}
