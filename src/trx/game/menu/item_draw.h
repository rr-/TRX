#pragma once

#include <trx/game/menu/ring/types.h>
#include <trx/game/objects/types.h>

// Draws an inventory item's 3D object at the current matrix position,
// covering the special-cased objects (compass needle, stopwatch hands).
// Shared between the menu strategies.
void InvItem_DrawObject(
    const INVENTORY_ITEM *inv_item, const ANIM_FRAME *frame1,
    const ANIM_FRAME *frame2, int32_t frac, int32_t rate);
