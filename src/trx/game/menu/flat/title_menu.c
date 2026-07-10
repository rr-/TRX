#include <trx/game/menu/flat/title_menu.h>

#include <trx/core/memory.h>
#include <trx/game/camera.h>
#include <trx/game/effects.h>
#include <trx/game/flyby_mode.h>
#include <trx/game/game.h>
#include <trx/game/game/draw.h>
#include <trx/game/game_flow.h>
#include <trx/game/game_strings/entries.h>
#include <trx/game/input.h>
#include <trx/game/interpolation.h>
#include <trx/game/inventory.h>
#include <trx/game/items/actions.h>
#include <trx/game/items/manager.h>
#include <trx/game/lara.h>
#include <trx/game/menu/flat/logo.h>
#include <trx/game/menu/flat/types.h>
#include <trx/game/output.h>
#include <trx/game/output/sky.h>
#include <trx/game/output/state.h>
#include <trx/game/overlay.h>
#include <trx/game/savegame.h>
#include <trx/game/shell.h>
#include <trx/game/sound.h>
#include <trx/game/sparks.h>
#include <trx/game/ui/dialogs/combined_settings.h>
#include <trx/game/ui/dialogs/controls.h>
#include <trx/game/ui/dialogs/save_slot.h>
#include <trx/game/ui/elements/anchor.h>
#include <trx/game/ui/elements/label.h>
#include <trx/game/ui/elements/modal.h>
#include <trx/game/ui/elements/requester.h>

typedef enum {
    TM_ROOT,
    TM_LOAD,
    TM_OPTIONS,
    TM_CONTROLS,
} M_PHASE;

typedef enum {
    M_ENTRY_NEW_GAME,
    M_ENTRY_LOAD_GAME,
    M_ENTRY_OPTIONS,
    M_ENTRY_CONTROLS,
    M_ENTRY_EXIT_GAME,
    M_ENTRY_COUNT,
} M_ENTRY;

static const GAME_STRING_ID m_Entries[M_ENTRY_COUNT] = {
    [M_ENTRY_NEW_GAME] = GS_ID("general/passport/new_game"),
    [M_ENTRY_LOAD_GAME] = GS_ID("general/passport/load_game"),
    [M_ENTRY_OPTIONS] = GS_ID("general/pause/options"),
    [M_ENTRY_CONTROLS] = GS_ID("general/pause/controls"),
    [M_ENTRY_EXIT_GAME] = GS_ID("general/passport/exit_game"),
};

typedef struct {
    M_PHASE phase;
    bool live_scene;
    UI_REQUESTER_STATE req;
    UI_SAVE_SLOT_DIALOG_STATE *save_slot;
    UI_COMBINED_SETTINGS_STATE *settings;
    UI_CONTROLS_STATE controls;
    GF_COMMAND result;
    bool is_done;
} M_TITLE_MENU;

static void M_SimTick(void)
{
    // A minimal simulation tick keeping the title level alive behind the
    // menu: items, effects, and the flyby camera — no Lara, no player
    // input, no HUD.
    Interpolation_Remember();
    Output_ResetDynamicLights();
    Sound_ResetAmbient();
    Item_Control();
    Effect_Control();
    Sparks_Control();
    FlybyMode_PostControl();
    Camera_Update();
    ItemAction_RunActive();
    Sound_UpdateEffects();
    Output_AnimateTextures(1);
    Output_Sky_Update();

    // Loop the flyby forever.
    if (!FlybyMode_IsActive() && g_GameFlow.title_flyby_sequence >= 0) {
        FlybyMode_Activate(g_GameFlow.title_flyby_sequence, false);
    }
}

static INV_MENU *M_Open(const INVENTORY_MODE mode)
{
    M_TITLE_MENU *const menu = Memory_Alloc(sizeof(M_TITLE_MENU));
    menu->phase = TM_ROOT;
    menu->result = (GF_COMMAND) { .action = GF_NOOP };
    UI_Requester_Init(&menu->req, M_ENTRY_COUNT, M_ENTRY_COUNT, true);

    Savegame_ScanSavedGames();

    menu->live_scene = g_GameFlow.title_flyby_sequence >= 0;
    if (menu->live_scene) {
        // The OG title hides Lara during the flyby.
        if (Lara_GetItem() != nullptr) {
            Lara_GetItem()->mesh_bits = 0;
        }
        FlybyMode_Activate(g_GameFlow.title_flyby_sequence, false);
        Interpolation_Remember();
    } else if (g_GameFlow.main_menu_background_path != nullptr) {
        Output_Overlay_LoadImage(g_GameFlow.main_menu_background_path);
    }

    g_Inv_Mode = mode;
    return (INV_MENU *)menu;
}

static void M_Finish(M_TITLE_MENU *const menu, const GF_COMMAND result)
{
    menu->result = result;
    menu->is_done = true;
}

static GF_COMMAND M_Control(INV_MENU *const raw_menu)
{
    M_TITLE_MENU *const menu = (M_TITLE_MENU *)raw_menu;

    Input_Update();
    Shell_ProcessInput();

    if (Shell_IsExiting()) {
        M_Finish(menu, (GF_COMMAND) { .action = GF_EXIT_GAME });
        return menu->result;
    }

    if (menu->live_scene && !menu->is_done) {
        M_SimTick();
    }

    switch (menu->phase) {
    case TM_ROOT: {
        const int32_t choice = UI_Requester_Control(&menu->req);
        switch (choice) {
        case M_ENTRY_NEW_GAME:
            // TODO: new game mode / play-any-level choices.
            Savegame_InitCurrentInfo();
            Savegame_UnbindSlot();
            M_Finish(
                menu,
                (GF_COMMAND) {
                    .action = GF_START_GAME,
                    .param = GF_GetFirstLevel()->num,
                });
            break;

        case M_ENTRY_LOAD_GAME:
            menu->save_slot = UI_SaveSlotDialog_Init(
                UI_SAVE_SLOT_DIALOG_LOAD_GAME,
                Savegame_GetMostRecentlyCreatedSlot());
            menu->phase = TM_LOAD;
            break;

        case M_ENTRY_OPTIONS:
            menu->settings = UI_CombinedSettings_Init();
            menu->phase = TM_OPTIONS;
            break;

        case M_ENTRY_CONTROLS:
            UI_Controls_Init(&menu->controls);
            menu->phase = TM_CONTROLS;
            break;

        case M_ENTRY_EXIT_GAME:
            M_Finish(menu, (GF_COMMAND) { .action = GF_EXIT_GAME });
            break;

        default:
            break;
        }
        break;
    }

    case TM_LOAD: {
        const UI_SAVE_SLOT_DIALOG_CHOICE choice =
            UI_SaveSlotDialog_Control(menu->save_slot);
        switch (choice.action) {
        case UI_SAVE_SLOT_DIALOG_CANCEL:
            UI_SaveSlotDialog_Free(menu->save_slot);
            menu->save_slot = nullptr;
            menu->phase = TM_ROOT;
            break;

        case UI_SAVE_SLOT_DIALOG_CONFIRM:
            UI_SaveSlotDialog_Free(menu->save_slot);
            menu->save_slot = nullptr;
            Inv_RemoveAllItems();
            M_Finish(
                menu,
                (GF_COMMAND) {
                    .action = GF_START_SAVED_GAME,
                    .param = Savegame_SlotToParam(choice.slot),
                });
            break;

        default:
            g_Input = (INPUT_STATE) {};
            g_InputDB = (INPUT_STATE) {};
            break;
        }
        break;
    }

    case TM_OPTIONS:
        if (UI_CombinedSettings_Control(menu->settings)) {
            UI_CombinedSettings_Free(menu->settings);
            menu->settings = nullptr;
            menu->phase = TM_ROOT;
        }
        break;

    case TM_CONTROLS:
        if (UI_Controls_Control(&menu->controls)) {
            UI_Controls_Free(&menu->controls);
            menu->phase = TM_ROOT;
        }
        break;
    }

    Overlay_Animate(1);
    return menu->is_done ? menu->result : (GF_COMMAND) { .action = GF_NOOP };
}

static void M_Draw(INV_MENU *const raw_menu)
{
    M_TITLE_MENU *const menu = (M_TITLE_MENU *)raw_menu;

    if (menu->live_scene) {
        Game_Draw(false);
    } else if (g_GameFlow.main_menu_background_path != nullptr) {
        Output_Overlay_DrawImageBilinear(g_GameFlow.main_menu_background_path);
        Interpolation_Interpolate();
        Output_Flush();
    }

    switch (menu->phase) {
    case TM_ROOT:
        InvFlatLogo_Draw();
        // Menu entries near the bottom of the screen, per the OG.
        UI_BeginModal(0.5f, 0.85f);
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

    case TM_LOAD:
        UI_SaveSlotDialog(menu->save_slot);
        break;

    case TM_OPTIONS:
        UI_CombinedSettings(menu->settings);
        break;

    case TM_CONTROLS:
        UI_Controls(&menu->controls);
        break;
    }
}

static void M_Close(INV_MENU *const raw_menu)
{
    M_TITLE_MENU *const menu = (M_TITLE_MENU *)raw_menu;
    switch (menu->phase) {
    case TM_LOAD:
        UI_SaveSlotDialog_Free(menu->save_slot);
        break;
    case TM_OPTIONS:
        UI_CombinedSettings_Free(menu->settings);
        break;
    case TM_CONTROLS:
        UI_Controls_Free(&menu->controls);
        break;
    default:
        break;
    }
    if (menu->live_scene) {
        FlybyMode_Deactivate();
    }
    UI_Requester_Free(&menu->req);
    Memory_Free(menu);
}

static bool M_IsDone(const INV_MENU *const menu)
{
    return ((const M_TITLE_MENU *)menu)->is_done;
}

static INV_MENU_CAPS M_GetCaps(const INVENTORY_MODE mode)
{
    return (INV_MENU_CAPS) {
        .needs_game_snapshot = false,
        .needs_inventory_lighting = false,
        .live_scene = g_GameFlow.title_flyby_sequence >= 0,
    };
}

const INV_MENU_OPS *InvFlatTitle_GetMenuOps(void)
{
    static const INV_MENU_OPS ops = {
        .open = M_Open,
        .control = M_Control,
        .draw = M_Draw,
        .close = M_Close,
        .is_done = M_IsDone,
        .get_caps = M_GetCaps,
        .style = INV_FLAT_MENU_STYLE_INIT,
    };
    return &ops;
}
