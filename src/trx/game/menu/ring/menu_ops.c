#include <trx/game/menu/ring/menu_ops.h>

#include <trx/game/menu/ring/control.h>
#include <trx/game/menu/ring/draw.h>

static INV_MENU *M_Open(const INVENTORY_MODE mode)
{
    return (INV_MENU *)InvRing_Open(mode);
}

static GF_COMMAND M_Control(INV_MENU *const menu)
{
    return InvRing_Control((INV_RING *)menu);
}

static void M_Draw(INV_MENU *const menu)
{
    InvRing_Draw((INV_RING *)menu);
}

static void M_Close(INV_MENU *const menu)
{
    InvRing_Close((INV_RING *)menu);
}

static bool M_IsDone(const INV_MENU *const menu)
{
    return ((const INV_RING *)menu)->status == RNG_DONE;
}

static INV_MENU_CAPS M_GetCaps(const INVENTORY_MODE mode)
{
    return (INV_MENU_CAPS) {
        // Title mode draws its own background image, not a game snapshot;
        // the title level's room data isn't set up for a live scene render.
        .needs_game_snapshot = mode != INV_TITLE_MODE,
        .needs_inventory_lighting = true,
        .live_scene = false,
    };
}

const INV_MENU_OPS *InvRing_GetMenuOps(void)
{
    static const INV_MENU_OPS ops = {
        .open = M_Open,
        .control = M_Control,
        .draw = M_Draw,
        .close = M_Close,
        .is_done = M_IsDone,
        .get_caps = M_GetCaps,
        .style = {
            .draw_menu_chrome = true,
            .text_base_scale = 1.0f,
        },
    };
    return &ops;
}
