#include <trx/game/menu/flat/menu_ops.h>

#include <trx/game/menu/flat/control.h>
#include <trx/game/menu/flat/draw.h>
#include <trx/game/menu/flat/pause_menu.h>

static INV_MENU *M_Open(const INVENTORY_MODE mode)
{
    return (INV_MENU *)InvFlat_Open(mode);
}

static GF_COMMAND M_Control(INV_MENU *const menu)
{
    return InvFlat_Control((INV_FLAT *)menu);
}

static void M_Draw(INV_MENU *const menu)
{
    InvFlat_Draw((INV_FLAT *)menu);
}

static void M_Close(INV_MENU *const menu)
{
    InvFlat_Close((INV_FLAT *)menu);
}

static bool M_IsDone(const INV_MENU *const menu)
{
    return ((const INV_FLAT *)menu)->is_done;
}

static INV_MENU_CAPS M_GetCaps(const INVENTORY_MODE mode)
{
    return (INV_MENU_CAPS) {
        // The flat menu draws over a desaturated freeze-frame of the game.
        .needs_game_snapshot = true,
        .needs_inventory_lighting = false,
        .live_scene = false,
    };
}

const INV_MENU_OPS *InvFlat_GetMenuOps(void)
{
    static INV_MENU_OPS ops = {
        .open = M_Open,
        .control = M_Control,
        .draw = M_Draw,
        .close = M_Close,
        .is_done = M_IsDone,
        .get_caps = M_GetCaps,
        .style = INV_FLAT_MENU_STYLE_INIT,
    };
    ops.pause_menu = InvFlatPause_GetOps();
    return &ops;
}
