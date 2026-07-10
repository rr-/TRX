#include <trx/game/menu/flat/options_menu.h>

#include <trx/game/game_strings/entries.h>
#include <trx/game/ui/elements/anchor.h>
#include <trx/game/ui/elements/label.h>
#include <trx/game/ui/elements/modal.h>
#include <trx/game/ui/elements/pad.h>

static void M_AddOption(
    INV_FLAT *const flat, const INV_FLAT_ACTION action,
    const GAME_STRING_ID label)
{
    if (flat->options.count >= INV_FLAT_MAX_OPTIONS) {
        return;
    }
    flat->options.actions[flat->options.count] = action;
    flat->options.labels[flat->options.count] = label;
    flat->options.count++;
}

static void M_BuildOptions(
    INV_FLAT *const flat, const INVENTORY_ITEM *const inv_item)
{
    flat->options.count = 0;

    switch (inv_item->object_id) {
    case O_PISTOL_OPTION:
    case O_SHOTGUN_OPTION:
    case O_MAGNUM_OPTION:
    case O_AUTOS_OPTION:
    case O_DESERT_EAGLE_OPTION:
    case O_UZI_OPTION:
    case O_HARPOON_OPTION:
    case O_M16_OPTION:
    case O_MP5_OPTION:
    case O_GRENADE_GUN_OPTION:
    case O_ROCKET_GUN_OPTION:
    case O_CROSSBOW_OPTION:
    case O_BINOCULARS_OPTION:
        M_AddOption(
            flat, IF_ACTION_EQUIP, GS_ID("general/inventory_flat/equip"));
        // TODO: IF_ACTION_CHOOSE_AMMO and IF_ACTION_COMBINE (lasersight)
        // once the ammo selector and combine tables land.
        break;

    case O_MEMCARD_LOAD_OPTION:
        M_AddOption(
            flat, IF_ACTION_LOAD_GAME, GS_ID("general/passport/load_game"));
        break;

    case O_MEMCARD_SAVE_OPTION:
        M_AddOption(
            flat, IF_ACTION_SAVE_GAME, GS_ID("general/passport/save_game"));
        break;

    case O_LASERSIGHT_OPTION:
    case O_WATERSKIN_1_OPTION:
    case O_WATERSKIN_2_OPTION:
        // TODO: combine-only items; nothing to offer until combining lands.
        break;

    case O_COMPASS_OPTION:
    case O_STOPWATCH_OPTION:
        break;

    default:
        // Medipacks, flares, keys, puzzles and other pickups.
        M_AddOption(flat, IF_ACTION_USE, GS_ID("general/inventory_flat/use"));
        // TODO: IF_ACTION_EXAMINE for examinable items.
        break;
    }
}

bool InvFlatOptions_Open(
    INV_FLAT *const flat, const INVENTORY_ITEM *const inv_item)
{
    M_BuildOptions(flat, inv_item);
    if (flat->options.count == 0) {
        return false;
    }
    UI_Requester_Init(
        &flat->options.req, flat->options.count, flat->options.count, true);
    return true;
}

INV_FLAT_ACTION InvFlatOptions_Control(INV_FLAT *const flat)
{
    const int32_t choice = UI_Requester_Control(&flat->options.req);
    if (choice == UI_REQUESTER_CANCEL) {
        return IF_ACTION_CANCEL;
    }
    if (choice >= 0 && choice < flat->options.count) {
        return flat->options.actions[choice];
    }
    return IF_ACTION_NONE;
}

void InvFlatOptions_Draw(INV_FLAT *const flat)
{
    UI_BeginModal(0.5f, 0.75f);
    UI_BeginRequester(&flat->options.req, nullptr);

    for (int32_t i = UI_Requester_GetFirstRow(&flat->options.req);
         i < UI_Requester_GetLastRow(&flat->options.req); i++) {
        UI_BeginRequesterRow(&flat->options.req, i);
        UI_BeginAnchor(0.5f, 0.5f);
        UI_Label(GameString_Get(flat->options.labels[i]));
        UI_EndAnchor();
        UI_EndRequesterRow(&flat->options.req, i);
    }

    UI_EndRequester(&flat->options.req);
    UI_EndModal();
}

void InvFlatOptions_Close(INV_FLAT *const flat)
{
    UI_Requester_Free(&flat->options.req);
    flat->options.count = 0;
}
