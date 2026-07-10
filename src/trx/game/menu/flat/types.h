#pragma once

#include <trx/game/fader.h>
#include <trx/game/game_flow/types.h>
#include <trx/game/game_strings/entries.h>
#include <trx/game/menu/enum.h>
#include <trx/game/menu/ring/types.h>
#include <trx/game/ui/dialogs/save_slot.h>
#include <trx/game/ui/elements/requester.h>

#define INV_FLAT_MAX_ITEMS 64
#define INV_FLAT_MAX_OPTIONS 4

typedef enum {
    IF_OPENING,
    IF_BROWSE,
    IF_OPTION_MENU,
    IF_LOADSAVE,
    IF_CLOSING,
    IF_DONE,
} INV_FLAT_STATE;

typedef enum {
    IF_ACTION_NONE,
    IF_ACTION_CANCEL,
    IF_ACTION_USE,
    IF_ACTION_EQUIP,
    IF_ACTION_EXAMINE,
    IF_ACTION_COMBINE,
    IF_ACTION_SEPARATE,
    IF_ACTION_CHOOSE_AMMO,
    IF_ACTION_LOAD_GAME,
    IF_ACTION_SAVE_GAME,
} INV_FLAT_ACTION;

typedef struct {
    INVENTORY_MODE mode;
    INV_FLAT_STATE state;

    INVENTORY_ITEM *items[INV_FLAT_MAX_ITEMS];
    int32_t item_count;

    // The item row scrolls towards target_idx; scroll_pos is expressed in
    // item slots.
    int32_t target_idx;
    float scroll_pos;
    float prev_scroll_pos;

    // Continuous spin of the focused item.
    int16_t spin_rot;
    int16_t prev_spin_rot;

    bool is_done;
    FADER back_fader;

    // The self-orienting compass in the screen corner.
    INVENTORY_ITEM *compass;

    // The vertical action menu shown under the focused item.
    struct {
        UI_REQUESTER_STATE req;
        INV_FLAT_ACTION actions[INV_FLAT_MAX_OPTIONS];
        GAME_STRING_ID labels[INV_FLAT_MAX_OPTIONS];
        int32_t count;
    } options;

    // The memcard load/save slot picker.
    struct {
        UI_SAVE_SLOT_DIALOG_STATE *dialog;
        UI_SAVE_SLOT_DIALOG_TYPE type;
    } save_slot;

    // Executed when the menu has fully closed.
    struct {
        INV_FLAT_ACTION action;
        SAVEGAME_SLOT_REF slot;
    } pending;
} INV_FLAT;
