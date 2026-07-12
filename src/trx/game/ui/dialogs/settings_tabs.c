#include <trx/game/ui/dialogs/settings_tabs.h>

#include <trx/config/common.h>
#include <trx/config/dynamic_option.h>
#include <trx/core/memory.h>
#include <trx/core/vector.h>
#include <trx/game/ui/dialogs/config_presets.h>
#include <trx/game/ui/dialogs/settings.h>
#include <trx/game/ui/dialogs/settings_editor.h>

#include <string.h>

static bool M_EditorControl(
    void *const user_data, UI_SETTINGS_PHASE *const phase)
{
    return UI_SettingsEditor_Control(user_data, phase);
}

static void M_EditorDraw(
    void *const user_data, const UI_SETTINGS_PHASE phase, const float row_width)
{
    UI_SETTINGS_EDITOR_STATE *const editor = user_data;
    UI_SettingsEditor_Draw(
        editor, UI_SettingsEditor_GetScrollable(editor), phase, row_width);
}

static void M_EditorDrawFooter(
    void *const user_data, const UI_SETTINGS_PHASE phase)
{
    UI_SettingsEditor_DrawFooter(user_data, phase);
}

static void M_EditorDrawOverlay(void *const user_data)
{
    UI_SettingsEditor_DrawOverlay(user_data);
}

static void M_EditorFree(void *const user_data)
{
    UI_SettingsEditor_Free(user_data);
}

static UI_SCROLLABLE *M_EditorGetScrollable(void *const user_data)
{
    return UI_SettingsEditor_GetScrollable(user_data);
}

static void M_EditorRecompute(
    void *const user_data, const float max_content_height)
{
    UI_SettingsEditor_RecomputeSizes(user_data, max_content_height);
}

static float M_EditorGetContentWidth(void *const user_data)
{
    return UI_SettingsEditor_GetContentWidth(user_data);
}

static float M_EditorGetContentHeight(void *const user_data)
{
    return UI_SettingsEditor_GetContentHeight(user_data);
}

static int32_t M_EditorGetItemCount(void *const user_data)
{
    return UI_SettingsEditor_GetItemCount(user_data);
}

static const UI_SETTINGS_TAB_OPS m_EditorOps = {
    .control = M_EditorControl,
    .draw = M_EditorDraw,
    .draw_footer = M_EditorDrawFooter,
    .draw_overlay = M_EditorDrawOverlay,
    .free = M_EditorFree,
    .get_scrollable = M_EditorGetScrollable,
    .recompute = M_EditorRecompute,
    .get_content_width = M_EditorGetContentWidth,
    .get_content_height = M_EditorGetContentHeight,
    .get_item_count = M_EditorGetItemCount,
};

typedef struct {
    UI_SETTINGS_OPTION option;
    int32_t priority;
} M_ROW;

// Merges the tab's static rows with the options Lua declared for it. Returns a
// sentinel-terminated array the editor takes ownership of.
static UI_SETTINGS_OPTION *M_BuildTabOptions(
    const char *const tab_id, const UI_SETTINGS_OPTION *const static_options)
{
    VECTOR *const rows = Vector_Create(sizeof(M_ROW));

    int32_t static_count = 0;
    while (static_options[static_count].target != nullptr) {
        static_count++;
    }

    // The names of the static rows, in tab order: what an anchor resolves
    // against. A row whose option this game does not have has no name, and so
    // cannot be anchored to.
    const char **static_names =
        Memory_Alloc(sizeof(char *) * MAX(static_count, 1));
    for (int32_t i = 0; i < static_count; i++) {
        const CONFIG_OPTION *const option =
            Config_GetOption(static_options[i].target);
        static_names[i] = option != nullptr ? option->name : nullptr;
        Vector_Add(
            rows,
            &(M_ROW) {
                .option = static_options[i],
                .priority = i * CONFIG_STATIC_ROW_SPACING,
            });
    }

    const VECTOR *const dynamic_options = Config_GetDynamicOptions();
    for (int32_t i = 0;
         dynamic_options != nullptr && i < dynamic_options->count; i++) {
        const CONFIG_DYNAMIC_OPTION *const dyn =
            Vector_Get((VECTOR *)dynamic_options, i);
        if (dyn->ui.tab == nullptr || strcmp(dyn->ui.tab, tab_id) != 0) {
            continue;
        }
        Vector_Add(
            rows,
            &(M_ROW) {
                .option = {
                    .target = dyn->target,
                    .min_value = dyn->min_value,
                    .max_value = dyn->max_value,
                    .delta_slow = 1,
                    .delta_fast = 1,
                },
                .priority = Config_GetDynamicRowPriority(
                    dyn, static_names, static_count,
                    (static_count + i) * CONFIG_STATIC_ROW_SPACING),
            });
    }
    Memory_FreePointer(&static_names);

    // Insertion sort: it keeps rows of equal priority in the order they were
    // added, so a dynamic row never displaces a static one it did not name.
    for (int32_t i = 1; i < rows->count; i++) {
        const M_ROW row = *(M_ROW *)Vector_Get(rows, i);
        int32_t j = i - 1;
        while (j >= 0
               && ((M_ROW *)Vector_Get(rows, j))->priority > row.priority) {
            *(M_ROW *)Vector_Get(rows, j + 1) = *(M_ROW *)Vector_Get(rows, j);
            j--;
        }
        *(M_ROW *)Vector_Get(rows, j + 1) = row;
    }

    UI_SETTINGS_OPTION *const result =
        Memory_Alloc(sizeof(UI_SETTINGS_OPTION) * (rows->count + 1));
    for (int32_t i = 0; i < rows->count; i++) {
        result[i] = ((M_ROW *)Vector_Get(rows, i))->option;
    }
    result[rows->count] = (UI_SETTINGS_OPTION) { .target = nullptr };
    Vector_Free(rows);
    return result;
}

UI_SETTINGS_TAB UI_SettingsTab_MakeEditor(
    const GAME_STRING_ID header_gs, const char *const tab_id,
    const UI_SETTINGS_OPTION *const options)
{
    return (UI_SETTINGS_TAB) {
        .header_gs = header_gs,
        .ops = &m_EditorOps,
        .user_data = UI_SettingsEditor_Init(M_BuildTabOptions(tab_id, options)),
    };
}

static bool M_PresetsControl(
    void *const user_data, UI_SETTINGS_PHASE *const phase)
{
    if (UI_ConfigPresets_Control(user_data)) {
        *phase = UI_SETTINGS_PHASE_NAVIGATE_TABS;
    }
    return false;
}

static void M_PresetsDraw(
    void *const user_data, const UI_SETTINGS_PHASE, const float)
{
    UI_ConfigPresets(user_data);
}

static void M_PresetsDrawOverlay(void *const user_data)
{
    UI_ConfigPresetsApplyModal(user_data);
}

static void M_PresetsFree(void *const user_data)
{
    UI_ConfigPresets_Free(user_data);
}

static UI_SCROLLABLE *M_PresetsGetScrollable(void *const user_data)
{
    return UI_ConfigPresets_GetScrollable(user_data);
}

static void M_PresetsRecompute(
    void *const user_data, const float max_content_height)
{
    UI_ConfigPresets_RecomputeSizes(user_data, max_content_height);
}

static float M_PresetsGetContentWidth(void *const user_data)
{
    return UI_ConfigPresets_GetContentWidth(user_data);
}

static float M_PresetsGetContentHeight(void *const user_data)
{
    return UI_ConfigPresets_GetContentHeight(user_data);
}

static int32_t M_PresetsGetItemCount(void *const user_data)
{
    return UI_ConfigPresets_GetItemCount(user_data);
}

static const UI_SETTINGS_TAB_OPS m_PresetsOps = {
    .control = M_PresetsControl,
    .draw = M_PresetsDraw,
    .draw_overlay = M_PresetsDrawOverlay,
    .free = M_PresetsFree,
    .get_scrollable = M_PresetsGetScrollable,
    .recompute = M_PresetsRecompute,
    .get_content_width = M_PresetsGetContentWidth,
    .get_content_height = M_PresetsGetContentHeight,
    .get_item_count = M_PresetsGetItemCount,
};

UI_SETTINGS_TAB UI_SettingsTab_MakePresets(const GAME_STRING_ID header_gs)
{
    return (UI_SETTINGS_TAB) {
        .header_gs = header_gs,
        .ops = &m_PresetsOps,
        .user_data = UI_ConfigPresets_Init(),
    };
}
