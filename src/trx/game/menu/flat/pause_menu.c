#include <trx/game/menu/flat/pause_menu.h>

#include <trx/config.h>
#include <trx/core/memory.h>
#include <trx/game/game.h>
#include <trx/game/game_strings/entries.h>
#include <trx/game/gym.h>
#include <trx/game/input.h>
#include <trx/game/savegame.h>
#include <trx/game/ui/dialogs/combined_settings.h>
#include <trx/game/ui/dialogs/controls.h>
#include <trx/game/ui/dialogs/stats.h>
#include <trx/game/ui/elements/anchor.h>
#include <trx/game/ui/elements/label.h>
#include <trx/game/ui/elements/modal.h>
#include <trx/game/ui/elements/requester.h>

typedef enum {
    PM_ROOT,
    PM_STATS,
    PM_OPTIONS,
    PM_CONTROLS,
} M_PHASE;

typedef enum {
    M_ENTRY_STATISTICS,
    M_ENTRY_OPTIONS,
    M_ENTRY_CONTROLS,
    M_ENTRY_EXIT_TO_TITLE,
    M_ENTRY_COUNT,
} M_ENTRY;

static const GAME_STRING_ID m_Entries[M_ENTRY_COUNT] = {
    [M_ENTRY_STATISTICS] = GS_ID("general/pause/statistics"),
    [M_ENTRY_OPTIONS] = GS_ID("general/pause/options"),
    [M_ENTRY_CONTROLS] = GS_ID("general/pause/controls"),
    [M_ENTRY_EXIT_TO_TITLE] = GS_ID("general/pause/exit_to_title_action"),
};

struct INV_PAUSE_MENU {
    M_PHASE phase;
    UI_REQUESTER_STATE req;
    UI_STATS_DIALOG_STATE *stats;
    UI_COMBINED_SETTINGS_STATE *settings;
    UI_CONTROLS_STATE controls;
};

static INV_PAUSE_MENU *M_Init(void)
{
    INV_PAUSE_MENU *const menu = Memory_Alloc(sizeof(INV_PAUSE_MENU));
    menu->phase = PM_ROOT;
    UI_Requester_Init(&menu->req, M_ENTRY_COUNT, M_ENTRY_COUNT, true);
    return menu;
}

static INV_PAUSE_CHOICE M_Control(INV_PAUSE_MENU *const menu)
{
    switch (menu->phase) {
    case PM_ROOT: {
        const int32_t choice = UI_Requester_Control(&menu->req);
        if (choice == UI_REQUESTER_CANCEL) {
            return INV_PAUSE_RESUME;
        }
        switch (choice) {
        case M_ENTRY_STATISTICS:
            menu->stats = UI_StatsDialog_Init((UI_STATS_DIALOG_ARGS) {
                .mode = Game_IsInGym()
                        && Gym_TrackManager_HasStats(GYM_TRACK_ASSAULT)
                    ? UI_STATS_DIALOG_MODE_ASSAULT_COURSE
                    : UI_STATS_DIALOG_MODE_LEVEL,
                .style = UI_STATS_DIALOG_STYLE_BARE,
                .level_num = Game_GetCurrentLevel()->num,
                .display_level_num = Savegame_GetCompletedLevelCount() + 1,
            });
            menu->phase = PM_STATS;
            break;

        case M_ENTRY_OPTIONS:
            menu->settings = UI_CombinedSettings_Init();
            menu->phase = PM_OPTIONS;
            break;

        case M_ENTRY_CONTROLS:
            UI_Controls_Init(&menu->controls);
            menu->phase = PM_CONTROLS;
            break;

        case M_ENTRY_EXIT_TO_TITLE:
            return INV_PAUSE_EXIT_TO_TITLE;
        }
        break;
    }

    case PM_STATS:
        UI_StatsDialog_Control(menu->stats);
        if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            UI_StatsDialog_Free(menu->stats);
            menu->stats = nullptr;
            menu->phase = PM_ROOT;
            g_Input = (INPUT_STATE) {};
            g_InputDB = (INPUT_STATE) {};
        }
        break;

    case PM_OPTIONS:
        if (UI_CombinedSettings_Control(menu->settings)) {
            UI_CombinedSettings_Free(menu->settings);
            menu->settings = nullptr;
            menu->phase = PM_ROOT;
        }
        break;

    case PM_CONTROLS:
        if (UI_Controls_Control(&menu->controls)) {
            UI_Controls_Free(&menu->controls);
            menu->phase = PM_ROOT;
        }
        break;
    }

    return INV_PAUSE_NOOP;
}

static void M_Draw(INV_PAUSE_MENU *const menu)
{
    switch (menu->phase) {
    case PM_ROOT:
        UI_BeginModal(0.5f, 0.5f);
        UI_BeginRequester(&menu->req, nullptr);
        for (int32_t i = UI_Requester_GetFirstRow(&menu->req);
             i < UI_Requester_GetLastRow(&menu->req); i++) {
            UI_BeginRequesterRow(&menu->req, i);
            UI_BeginAnchor(0.5f, 0.5f);
            UI_Label(GameString_Get(m_Entries[i]));
            UI_EndAnchor();
            UI_EndRequesterRow(&menu->req, i);
        }
        UI_EndRequester(&menu->req);
        UI_EndModal();
        break;

    case PM_STATS:
        UI_StatsDialog(menu->stats);
        break;

    case PM_OPTIONS:
        UI_CombinedSettings(menu->settings);
        break;

    case PM_CONTROLS:
        UI_Controls(&menu->controls);
        break;
    }
}

static void M_Free(INV_PAUSE_MENU *const menu)
{
    switch (menu->phase) {
    case PM_STATS:
        UI_StatsDialog_Free(menu->stats);
        break;
    case PM_OPTIONS:
        UI_CombinedSettings_Free(menu->settings);
        break;
    case PM_CONTROLS:
        UI_Controls_Free(&menu->controls);
        break;
    default:
        break;
    }
    UI_Requester_Free(&menu->req);
    Memory_Free(menu);
}

const INV_PAUSE_MENU_OPS *InvFlatPause_GetOps(void)
{
    static const INV_PAUSE_MENU_OPS ops = {
        .init = M_Init,
        .control = M_Control,
        .draw = M_Draw,
        .free = M_Free,
    };
    return &ops;
}
