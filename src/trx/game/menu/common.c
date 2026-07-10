#include <trx/game/menu/common.h>

#include <trx/game/menu/ring/menu_ops.h>
#include <trx/version.h>

// TODO: once the TR4 flat menu strategy lands, this style moves into its
// menu ops and the strategy selection below routes TR4 to it.
static const INV_MENU_STYLE m_FlatStyle = {
    .draw_menu_chrome = false,
    // TR4 draws its UI with a much larger typeface than TR1-3
    // (originally 3/40th of the screen height per line).
    .text_base_scale = 1.5f,
};

const INV_MENU_OPS *InvMenu_GetOps(const INVENTORY_MODE mode)
{
    // The globe select is ring-only regardless of the game version.
    return InvRing_GetMenuOps();
}

const INV_MENU_STYLE *InvMenu_GetStyle(void)
{
    if (g_TRVersion == 4) {
        return &m_FlatStyle;
    }
    return &InvMenu_GetOps(INV_GAME_MODE)->style;
}
