#include <trx/game/menu/common.h>

#include <trx/game/menu/flat/menu_ops.h>
#include <trx/game/menu/ring/menu_ops.h>
#include <trx/version.h>

const INV_MENU_OPS *InvMenu_GetOps(const INVENTORY_MODE mode)
{
    // The sole place deciding which menu strategy a game version uses.
    if (g_TRVersion == 4) {
        switch (mode) {
        case INV_GAME_MODE:
        case INV_KEYS_MODE:
        case INV_LOAD_MODE:
        case INV_SAVE_MODE:
        case INV_DEATH_MODE:
            return InvFlat_GetMenuOps();

        // TODO: route the title mode to the flat menu once the TR4
        // title menu comes online.
        default:
            break;
        }
    }
    return InvRing_GetMenuOps();
}

const INV_MENU_STYLE *InvMenu_GetStyle(void)
{
    if (g_TRVersion == 4) {
        return &InvFlat_GetMenuOps()->style;
    }
    return &InvRing_GetMenuOps()->style;
}
