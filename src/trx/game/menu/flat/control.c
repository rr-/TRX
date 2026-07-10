#include <trx/game/menu/flat/control.h>

#include <trx/config.h>
#include <trx/core/memory.h>
#include <trx/game/game.h>
#include <trx/game/game_flow.h>
#include <trx/game/gym.h>
#include <trx/game/input.h>
#include <trx/game/interpolation.h>
#include <trx/game/inventory.h>
#include <trx/game/lara.h>
#include <trx/game/menu/flat/options_menu.h>
#include <trx/game/menu/interact/combine.h>
#include <trx/game/menu/ring/control.h>
#include <trx/game/menu/ring/priv.h>
#include <trx/game/menu/ring/vars.h>
#include <trx/game/music.h>
#include <trx/game/option/stats.h>
#include <trx/game/overlay.h>
#include <trx/game/savegame.h>
#include <trx/game/shell.h>
#include <trx/game/sound.h>
#include <trx/game/stats.h>

#include <math.h>

#define M_FADE_TIME 0.3f
// Slots scrolled per logic frame; OG scrolls a slot in 8 frames.
#define M_SCROLL_SPEED (1.0f / 8.0f)
// OG spins the focused item by 1022 per frame.
#define M_SPIN_SPEED 1022

static OBJECT_ID m_Chosen = NO_OBJECT;

static void M_AddItem(INV_FLAT *const flat, INVENTORY_ITEM *const inv_item)
{
    if (flat->item_count >= INV_FLAT_MAX_ITEMS || inv_item == nullptr) {
        return;
    }
    InvRing_InitInvItem(inv_item);
    flat->items[flat->item_count] = inv_item;
    flat->item_count++;
}

static void M_BuildItemList(INV_FLAT *const flat)
{
    // A single row: the main inventory, key items, then the memcards.
    // The compass is not part of the row; it sits in the screen corner.
    for (int32_t i = 0; i < g_InvRing_Source[RT_MAIN].count; i++) {
        INVENTORY_ITEM *const inv_item = g_InvRing_Source[RT_MAIN].items[i];
        if (inv_item->object_id == O_COMPASS_OPTION) {
            flat->compass = inv_item;
            InvRing_InitInvItem(inv_item);
            continue;
        }
        M_AddItem(flat, inv_item);
    }
    const int32_t first_key_idx = flat->item_count;
    for (int32_t i = 0; i < g_InvRing_Source[RT_KEYS].count; i++) {
        M_AddItem(flat, g_InvRing_Source[RT_KEYS].items[i]);
    }

    if (flat->compass == nullptr) {
        flat->compass = InvRing_GetByObjectID(O_COMPASS_OPTION);
        if (flat->compass != nullptr) {
            InvRing_InitInvItem(flat->compass);
        }
    }

    const int32_t load_idx = flat->item_count;
    if (flat->mode != INV_KEYS_MODE) {
        M_AddItem(flat, InvRing_GetByObjectID(O_MEMCARD_LOAD_OPTION));
        if (flat->mode != INV_DEATH_MODE) {
            M_AddItem(flat, InvRing_GetByObjectID(O_MEMCARD_SAVE_OPTION));
        }
    }

    switch (flat->mode) {
    case INV_KEYS_MODE:
        if (first_key_idx < flat->item_count) {
            flat->target_idx = first_key_idx;
        }
        break;

    case INV_LOAD_MODE:
    case INV_DEATH_MODE:
        flat->target_idx = load_idx;
        break;

    case INV_SAVE_MODE:
        flat->target_idx = MIN(load_idx + 1, flat->item_count - 1);
        break;

    default:
        break;
    }
}

// Rebuilds the row after the inventory changed, refocusing on the given
// object.
static void M_RefreshItems(INV_FLAT *const flat, const OBJECT_ID focus_obj)
{
    flat->item_count = 0;
    flat->compass = nullptr;
    flat->target_idx = 0;
    M_BuildItemList(flat);
    for (int32_t i = 0; i < flat->item_count; i++) {
        if (flat->items[i]->object_id == focus_obj) {
            flat->target_idx = i;
            break;
        }
    }
    flat->scroll_pos = flat->target_idx;
    flat->prev_scroll_pos = flat->scroll_pos;
}

static bool M_OpenCombineRow(INV_FLAT *const flat)
{
    const OBJECT_ID obj = flat->items[flat->target_idx]->object_id;
    OBJECT_ID partners[ARRAY_SIZE(flat->second_row.items)];
    const int32_t count =
        InvInteract_GetCombinePartners(obj, partners, ARRAY_SIZE(partners));
    if (count == 0) {
        return false;
    }

    flat->second_row.count = 0;
    for (int32_t i = 0; i < count; i++) {
        INVENTORY_ITEM *const inv_item = InvRing_GetByObjectID(partners[i]);
        if (inv_item != nullptr) {
            InvRing_InitInvItem(inv_item);
            flat->second_row.items[flat->second_row.count++] = inv_item;
        }
    }
    if (flat->second_row.count == 0) {
        return false;
    }

    flat->second_row.target_idx = 0;
    flat->second_row.scroll_pos = 0.0f;
    flat->second_row.prev_scroll_pos = 0.0f;
    return true;
}

static void M_ControlCombine(INV_FLAT *const flat)
{
    const bool settled = flat->second_row.target_idx
        == (int32_t)roundf(flat->second_row.scroll_pos);

    if (g_Input.menu_right && settled
        && flat->second_row.target_idx < flat->second_row.count - 1) {
        flat->second_row.target_idx++;
        Sound_Effect(SFX_MENU_ROTATE, nullptr, SPM_ALWAYS);
    } else if (
        g_Input.menu_left && settled && flat->second_row.target_idx > 0) {
        flat->second_row.target_idx--;
        Sound_Effect(SFX_MENU_ROTATE, nullptr, SPM_ALWAYS);
    } else if (g_InputDB.menu_back) {
        flat->state = IF_BROWSE;
        Sound_Effect(SFX_MENU_SPINOUT, nullptr, SPM_ALWAYS);
    } else if (g_InputDB.menu_confirm && settled) {
        const OBJECT_ID sel = flat->items[flat->target_idx]->object_id;
        const OBJECT_ID partner =
            flat->second_row.items[flat->second_row.target_idx]->object_id;
        OBJECT_ID combined = NO_OBJECT;
        if (InvInteract_Combine(sel, partner, &combined)) {
            // TODO: play the OG combine sample once it is cataloged.
            Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            M_RefreshItems(flat, combined);
        } else {
            Sound_Effect(SFX_LARA_NO, nullptr, SPM_ALWAYS);
        }
        flat->state = IF_BROWSE;
    }
    g_Input = (INPUT_STATE) {};
    g_InputDB = (INPUT_STATE) {};
}

static void M_OpenSaveSlotDialog(
    INV_FLAT *const flat, const UI_SAVE_SLOT_DIALOG_TYPE type)
{
    SAVEGAME_SLOT_REF initial_slot = Savegame_GetMostRecentlyUsedSlot();
    if (!Savegame_IsValidSlotRef(initial_slot)) {
        initial_slot = Savegame_GetMostRecentlyCreatedSlot();
    }
    if (!Savegame_IsValidSlotRef(initial_slot)) {
        initial_slot = Savegame_NormalSlot(0);
    }
    flat->save_slot.dialog = UI_SaveSlotDialog_Init(type, initial_slot);
    flat->save_slot.type = type;
    flat->state = IF_LOADSAVE;
}

static void M_ShowTexts(const INV_FLAT *const flat)
{
    if (flat->state == IF_COMBINE) {
        if (flat->second_row.target_idx
            == (int32_t)roundf(flat->second_row.scroll_pos)) {
            InvRing_ShowItemName(
                flat->second_row.items[flat->second_row.target_idx]);
        } else {
            InvRing_RemoveItemTexts();
        }
        return;
    }

    if ((flat->state != IF_BROWSE && flat->state != IF_OPTION_MENU
         && flat->state != IF_EXAMINE)
        || flat->target_idx != (int32_t)roundf(flat->scroll_pos)) {
        InvRing_RemoveItemTexts();
        return;
    }

    const INVENTORY_ITEM *const inv_item = flat->items[flat->target_idx];
    InvRing_ShowItemName(inv_item);

    const LARA_INFO *const lara = Lara_GetLaraInfo();
    const int32_t qty = Inv_RequestItem(inv_item->object_id);
    switch (inv_item->object_id) {
    case O_WATERSKIN_1_OPTION:
        InvRing_ShowItemQuantity("%dL", lara->small_water_skin);
        break;

    case O_WATERSKIN_2_OPTION:
        InvRing_ShowItemQuantity("%dL", lara->big_water_skin);
        break;

    case O_SMALL_MEDIPACK_OPTION:
    case O_LARGE_MEDIPACK_OPTION:
        Overlay_ForceHealthBar(true);
        if (qty > 1) {
            InvRing_ShowItemQuantity("%d", qty);
        }
        break;

    case O_FLAREBOX_OPTION:
        InvRing_ShowItemQuantity("%d", qty);
        break;

    default:
        if (Object_IsType(inv_item->object_id, g_GenericInvOptions)
            && qty > 1) {
            InvRing_ShowItemQuantity("%d", qty);
        }
        break;
    }
}

static GF_COMMAND M_Finish(INV_FLAT *const flat)
{
    if (Shell_IsExiting()) {
        return (GF_COMMAND) { .action = GF_EXIT_GAME };
    }
    if (GF_GetOverrideCommand().action != GF_NOOP) {
        return GF_GetOverrideCommand();
    }

    switch (flat->pending.action) {
    case IF_ACTION_LOAD_GAME:
        Inv_RemoveAllItems();
        return (GF_COMMAND) {
            .action = GF_START_SAVED_GAME,
            .param = Savegame_SlotToParam(flat->pending.slot),
        };

    case IF_ACTION_SAVE_GAME:
        Savegame_Save(flat->pending.slot);
        break;

    default:
        if (m_Chosen != NO_OBJECT) {
            // Weapon draws, medipacks, flares etc. are handled (or
            // ignored) by Lara_UseItem itself.
            Lara_UseItem(m_Chosen);
        }
        break;
    }

    Music_Unpause();
    Sound_UnpauseAll();
    return (GF_COMMAND) { .action = GF_NOOP };
}

INV_FLAT *InvFlat_Open(const INVENTORY_MODE mode)
{
    m_Chosen = NO_OBJECT;

    INV_FLAT *const flat = Memory_Alloc(sizeof(INV_FLAT));
    flat->mode = mode;
    flat->state = IF_OPENING;
    M_BuildItemList(flat);

    if (flat->item_count == 0) {
        Memory_Free(flat);
        return nullptr;
    }

    flat->scroll_pos = flat->target_idx;
    flat->prev_scroll_pos = flat->scroll_pos;

    g_Inv_Mode = mode;

    switch (mode) {
    case INV_LOAD_MODE:
    case INV_DEATH_MODE:
        M_OpenSaveSlotDialog(flat, UI_SAVE_SLOT_DIALOG_LOAD_GAME);
        break;
    case INV_SAVE_MODE:
        M_OpenSaveSlotDialog(flat, UI_SAVE_SLOT_DIALOG_SAVE_GAME);
        break;
    default:
        break;
    }

    if (!g_Config.audio.enable_music_in_inventory) {
        Music_Pause();
        Sound_PauseAll();
    }

    Sound_Effect(SFX_MENU_SPININ, nullptr, SPM_ALWAYS);
    Fader_InitTo(&flat->back_fader, 0.0f, 1.0f, M_FADE_TIME);
    Interpolation_Remember();
    return flat;
}

GF_COMMAND InvFlat_Control(INV_FLAT *const flat)
{
    flat->prev_scroll_pos = flat->scroll_pos;
    flat->prev_spin_rot = flat->spin_rot;

    Input_Update();
    Shell_ProcessInput();
    Game_ProcessInput();

    if (g_Config.gameplay.enable_timer_in_inventory
        && !(Game_IsInGym() && Gym_TrackManager_HasStats(GYM_TRACK_ASSAULT))) {
        Stats_UpdateTimer();
    }

    if (Shell_IsExiting()) {
        return (GF_COMMAND) { .action = GF_EXIT_GAME };
    }

    if (flat->compass != nullptr) {
        Option_Stats_UpdateCompassNeedle(flat->compass);
    }

    switch (flat->state) {
    case IF_OPENING:
        if (!Fader_IsActive(&flat->back_fader)) {
            flat->state = IF_BROWSE;
        }
        break;

    case IF_BROWSE:
        if (g_Input.menu_right && flat->target_idx < flat->item_count - 1
            && flat->target_idx == (int32_t)roundf(flat->scroll_pos)) {
            flat->target_idx++;
            Sound_Effect(SFX_MENU_ROTATE, nullptr, SPM_ALWAYS);
        } else if (
            g_Input.menu_left && flat->target_idx > 0
            && flat->target_idx == (int32_t)roundf(flat->scroll_pos)) {
            flat->target_idx--;
            Sound_Effect(SFX_MENU_ROTATE, nullptr, SPM_ALWAYS);
        } else if (g_InputDB.menu_back) {
            m_Chosen = NO_OBJECT;
            flat->state = IF_CLOSING;
            Sound_Effect(SFX_MENU_SPINOUT, nullptr, SPM_ALWAYS);
            Fader_InitFromCurrent(&flat->back_fader, 0.0f, M_FADE_TIME);
        } else if (
            g_InputDB.menu_confirm
            && flat->target_idx == (int32_t)roundf(flat->scroll_pos)) {
            if (InvFlatOptions_Open(flat, flat->items[flat->target_idx])) {
                flat->state = IF_OPTION_MENU;
                Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            } else {
                Sound_Effect(SFX_LARA_NO, nullptr, SPM_ALWAYS);
            }
        }
        g_Input = (INPUT_STATE) {};
        g_InputDB = (INPUT_STATE) {};
        break;

    case IF_OPTION_MENU:
        switch (InvFlatOptions_Control(flat)) {
        case IF_ACTION_CANCEL:
            InvFlatOptions_Close(flat);
            flat->state = IF_BROWSE;
            Sound_Effect(SFX_MENU_SPINOUT, nullptr, SPM_ALWAYS);
            break;

        case IF_ACTION_USE:
        case IF_ACTION_EQUIP:
            InvFlatOptions_Close(flat);
            m_Chosen = flat->items[flat->target_idx]->object_id;
            flat->state = IF_CLOSING;
            Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            Fader_InitFromCurrent(&flat->back_fader, 0.0f, M_FADE_TIME);
            break;

        case IF_ACTION_LOAD_GAME:
            InvFlatOptions_Close(flat);
            M_OpenSaveSlotDialog(flat, UI_SAVE_SLOT_DIALOG_LOAD_GAME);
            break;

        case IF_ACTION_SAVE_GAME:
            InvFlatOptions_Close(flat);
            M_OpenSaveSlotDialog(flat, UI_SAVE_SLOT_DIALOG_SAVE_GAME);
            break;

        case IF_ACTION_COMBINE:
            InvFlatOptions_Close(flat);
            if (M_OpenCombineRow(flat)) {
                flat->state = IF_COMBINE;
                Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            } else {
                flat->state = IF_BROWSE;
                Sound_Effect(SFX_LARA_NO, nullptr, SPM_ALWAYS);
            }
            break;

        case IF_ACTION_SEPARATE: {
            InvFlatOptions_Close(flat);
            const OBJECT_ID obj = flat->items[flat->target_idx]->object_id;
            OBJECT_ID part = NO_OBJECT;
            if (InvInteract_Separate(obj, &part)) {
                Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
                M_RefreshItems(flat, part);
            } else {
                Sound_Effect(SFX_LARA_NO, nullptr, SPM_ALWAYS);
            }
            flat->state = IF_BROWSE;
            break;
        }

        case IF_ACTION_CHOOSE_AMMO:
            InvFlatOptions_Close(flat);
            if (InvFlatAmmo_Open(flat, flat->items[flat->target_idx])) {
                flat->state = IF_AMMO;
                Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            } else {
                flat->state = IF_BROWSE;
                Sound_Effect(SFX_LARA_NO, nullptr, SPM_ALWAYS);
            }
            break;

        case IF_ACTION_EXAMINE:
            InvFlatOptions_Close(flat);
            flat->examine.x_rot = 0;
            flat->examine.y_rot = 0;
            flat->examine.prev_x_rot = 0;
            flat->examine.prev_y_rot = 0;
            flat->state = IF_EXAMINE;
            Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            break;

        default:
            break;
        }
        break;

    case IF_COMBINE:
        M_ControlCombine(flat);
        break;

    case IF_EXAMINE:
        flat->examine.prev_x_rot = flat->examine.x_rot;
        flat->examine.prev_y_rot = flat->examine.y_rot;
        if (g_Input.menu_left) {
            flat->examine.y_rot -= M_SPIN_SPEED;
        }
        if (g_Input.menu_right) {
            flat->examine.y_rot += M_SPIN_SPEED;
        }
        if (g_Input.menu_up) {
            flat->examine.x_rot -= M_SPIN_SPEED;
        }
        if (g_Input.menu_down) {
            flat->examine.x_rot += M_SPIN_SPEED;
        }
        if (g_InputDB.menu_back || g_InputDB.menu_confirm) {
            flat->state = IF_BROWSE;
            Sound_Effect(SFX_MENU_SPINOUT, nullptr, SPM_ALWAYS);
        }
        g_Input = (INPUT_STATE) {};
        g_InputDB = (INPUT_STATE) {};
        break;

    case IF_AMMO:
        switch (InvFlatAmmo_Control(flat)) {
        case IF_ACTION_CANCEL:
            InvFlatAmmo_Close(flat);
            flat->state = IF_BROWSE;
            Sound_Effect(SFX_MENU_SPINOUT, nullptr, SPM_ALWAYS);
            break;

        case IF_ACTION_CHOOSE_AMMO:
            InvFlatAmmo_Close(flat);
            flat->state = IF_BROWSE;
            Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            break;

        default:
            break;
        }
        break;

    case IF_LOADSAVE: {
        const UI_SAVE_SLOT_DIALOG_CHOICE choice =
            UI_SaveSlotDialog_Control(flat->save_slot.dialog);
        switch (choice.action) {
        case UI_SAVE_SLOT_DIALOG_NO_CHOICE:
            g_Input = (INPUT_STATE) {};
            g_InputDB = (INPUT_STATE) {};
            break;

        case UI_SAVE_SLOT_DIALOG_CANCEL:
            if (flat->mode == INV_DEATH_MODE) {
                // Death offers no way back into the game.
                break;
            }
            UI_SaveSlotDialog_Free(flat->save_slot.dialog);
            flat->save_slot.dialog = nullptr;
            if (flat->mode == INV_LOAD_MODE || flat->mode == INV_SAVE_MODE) {
                // Entered straight from the game; cancel returns to it.
                flat->state = IF_CLOSING;
                Fader_InitFromCurrent(&flat->back_fader, 0.0f, M_FADE_TIME);
            } else {
                flat->state = IF_BROWSE;
            }
            Sound_Effect(SFX_MENU_SPINOUT, nullptr, SPM_ALWAYS);
            break;

        case UI_SAVE_SLOT_DIALOG_CONFIRM:
            flat->pending.action =
                flat->save_slot.type == UI_SAVE_SLOT_DIALOG_LOAD_GAME
                ? IF_ACTION_LOAD_GAME
                : IF_ACTION_SAVE_GAME;
            flat->pending.slot = choice.slot;
            UI_SaveSlotDialog_Free(flat->save_slot.dialog);
            flat->save_slot.dialog = nullptr;
            flat->state = IF_CLOSING;
            Sound_Effect(SFX_MENU_CHOOSE, nullptr, SPM_ALWAYS);
            Fader_InitFromCurrent(&flat->back_fader, 0.0f, M_FADE_TIME);
            break;

        case UI_SAVE_SLOT_DIALOG_DELETE_FAILED:
            break;
        }
        break;
    }

    case IF_CLOSING:
        if (!Fader_IsActive(&flat->back_fader)) {
            flat->state = IF_DONE;
            flat->is_done = true;
            return M_Finish(flat);
        }
        break;

    case IF_DONE:
        break;
    }

    // Scroll towards the target slot and spin the focused item.
    const float scroll_delta = flat->target_idx - flat->scroll_pos;
    if (scroll_delta != 0.0f) {
        float step = scroll_delta < 0.0f ? -M_SCROLL_SPEED : M_SCROLL_SPEED;
        if (fabsf(scroll_delta) < M_SCROLL_SPEED) {
            step = scroll_delta;
        }
        flat->scroll_pos += step;
        flat->spin_rot = 0;
    } else if (
        flat->state == IF_BROWSE || flat->state == IF_OPTION_MENU
        || flat->state == IF_COMBINE || flat->state == IF_AMMO) {
        flat->spin_rot += M_SPIN_SPEED;
    }

    flat->second_row.prev_scroll_pos = flat->second_row.scroll_pos;
    const float second_delta =
        flat->second_row.target_idx - flat->second_row.scroll_pos;
    if (second_delta != 0.0f) {
        float step = second_delta < 0.0f ? -M_SCROLL_SPEED : M_SCROLL_SPEED;
        if (fabsf(second_delta) < M_SCROLL_SPEED) {
            step = second_delta;
        }
        flat->second_row.scroll_pos += step;
    }

    M_ShowTexts(flat);

    Interpolation_Remember();
    Overlay_Animate(1);
    return (GF_COMMAND) { .action = GF_NOOP };
}

void InvFlat_Close(INV_FLAT *const flat)
{
    if (flat->state == IF_OPTION_MENU) {
        InvFlatOptions_Close(flat);
    }
    if (flat->state == IF_AMMO) {
        InvInteract_CancelAmmoSession(&flat->ammo.session);
        InvFlatAmmo_Close(flat);
    }
    if (flat->save_slot.dialog != nullptr) {
        UI_SaveSlotDialog_Free(flat->save_slot.dialog);
        flat->save_slot.dialog = nullptr;
    }
    InvRing_RemoveItemTexts();

    if (g_Config.input.enable_buffering_inventory) {
        g_OldInputDB = (INPUT_STATE) {};
    }

    m_Chosen = NO_OBJECT;
    Memory_Free(flat);
}
