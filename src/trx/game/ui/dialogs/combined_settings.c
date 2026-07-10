#include <trx/game/ui/dialogs/combined_settings.h>

#include <trx/config/common.h>
#include <trx/core/memory.h>
#include <trx/core/strings.h>
#include <trx/game/ui/dialogs/settings.h>
#include <trx/game/ui/dialogs/settings_catalog.h>
#include <trx/game/ui/dialogs/settings_tabs.h>
#include <trx/game/ui/elements/anchor.h>
#include <trx/game/ui/elements/prompt.h>

#include <string.h>

struct UI_COMBINED_SETTINGS_STATE {
    UI_PROMPT_STATE prompt;
    char *last_query;

    // All catalog groups concatenated, terminated by .target == nullptr.
    UI_SETTINGS_OPTION *all_options;
    // The subset matching the search query, same termination.
    UI_SETTINGS_OPTION *filtered_options;
    UI_SETTINGS_DIALOG_STATE *dialog;
};

static int32_t M_CountGroupOptions(const UI_SETTINGS_GROUP *const group)
{
    int32_t count = 0;
    while (group->options[count].target != nullptr) {
        count++;
    }
    return count;
}

static UI_SETTINGS_OPTION *M_BuildOptions(void)
{
    int32_t total = 0;
    for (int32_t i = 0; i < UI_SETTINGS_GROUP_COUNT; i++) {
        total += M_CountGroupOptions(UI_SettingsCatalog_GetGroup(i));
    }

    UI_SETTINGS_OPTION *const options =
        Memory_Alloc(sizeof(UI_SETTINGS_OPTION) * (total + 1));
    int32_t pos = 0;
    for (int32_t i = 0; i < UI_SETTINGS_GROUP_COUNT; i++) {
        const UI_SETTINGS_GROUP *const group = UI_SettingsCatalog_GetGroup(i);
        const int32_t count = M_CountGroupOptions(group);
        memcpy(
            &options[pos], group->options, sizeof(UI_SETTINGS_OPTION) * count);
        pos += count;
    }
    options[pos] = (UI_SETTINGS_OPTION) { .target = nullptr };
    return options;
}

static bool M_MatchesQuery(
    const UI_SETTINGS_OPTION *const option, const char *const query)
{
    if (String_IsEmpty(query)) {
        return true;
    }
    const CONFIG_OPTION *const cfg_option = Config_GetOption(option->target);
    if (cfg_option == nullptr) {
        return false;
    }
    const char *const title = Config_GetOptionTitle(cfg_option);
    return title != nullptr && String_CaseSubstring(title, query) != nullptr;
}

static void M_RebuildDialog(UI_COMBINED_SETTINGS_STATE *const s)
{
    if (s->dialog != nullptr) {
        UI_SettingsDialog_Free(s->dialog);
        s->dialog = nullptr;
    }
    Memory_FreePointer(&s->filtered_options);

    int32_t total = 0;
    while (s->all_options[total].target != nullptr) {
        total++;
    }

    const char *const query = s->prompt.current_text;
    s->filtered_options =
        Memory_Alloc(sizeof(UI_SETTINGS_OPTION) * (total + 1));
    int32_t pos = 0;
    for (int32_t i = 0; i < total; i++) {
        if (M_MatchesQuery(&s->all_options[i], query)) {
            s->filtered_options[pos] = s->all_options[i];
            pos++;
        }
    }
    s->filtered_options[pos] = (UI_SETTINGS_OPTION) { .target = nullptr };

    const UI_SETTINGS_TAB tabs[] = {
        UI_SettingsTab_MakeEditor(
            GS_ID("general/settings/combined/tab"), s->filtered_options),
    };
    s->dialog = UI_SettingsDialog_Init(
        GS_ID("general/settings/combined/title"), ARRAY_SIZE(tabs), tabs);

    Memory_FreePointer(&s->last_query);
    s->last_query = Memory_DupStr(query != nullptr ? query : "");
}

UI_COMBINED_SETTINGS_STATE *UI_CombinedSettings_Init(void)
{
    UI_COMBINED_SETTINGS_STATE *const s =
        Memory_Alloc(sizeof(UI_COMBINED_SETTINGS_STATE));
    s->all_options = M_BuildOptions();
    UI_Prompt_Init(&s->prompt);
    UI_Prompt_SetFocus(&s->prompt, true);
    M_RebuildDialog(s);
    return s;
}

void UI_CombinedSettings_Free(UI_COMBINED_SETTINGS_STATE *const s)
{
    UI_Prompt_SetFocus(&s->prompt, false);
    UI_Prompt_Free(&s->prompt);
    UI_SettingsDialog_Free(s->dialog);
    Memory_FreePointer(&s->filtered_options);
    Memory_FreePointer(&s->all_options);
    Memory_FreePointer(&s->last_query);
    Memory_Free(s);
}

bool UI_CombinedSettings_Control(UI_COMBINED_SETTINGS_STATE *const s)
{
    UI_Prompt_Control(&s->prompt);

    const char *const query =
        s->prompt.current_text != nullptr ? s->prompt.current_text : "";
    if (strcmp(query, s->last_query) != 0) {
        M_RebuildDialog(s);
    }

    return UI_SettingsDialog_Control(s->dialog);
}

void UI_CombinedSettings(UI_COMBINED_SETTINGS_STATE *const s)
{
    UI_SettingsDialog(s->dialog);

    // The search box floats above the dialog.
    UI_BeginAnchor(0.5f, 0.04f);
    UI_Prompt(&s->prompt);
    UI_EndAnchor();
}
