#pragma once

#include <trx/game/menu/flat/types.h>

// Returns false when the focused item offers no actions.
bool InvFlatOptions_Open(INV_FLAT *flat, const INVENTORY_ITEM *inv_item);
INV_FLAT_ACTION InvFlatOptions_Control(INV_FLAT *flat);
void InvFlatOptions_Draw(INV_FLAT *flat);
void InvFlatOptions_Close(INV_FLAT *flat);

// Returns false when the focused weapon offers no ammo choice.
bool InvFlatAmmo_Open(INV_FLAT *flat, const INVENTORY_ITEM *inv_item);
INV_FLAT_ACTION InvFlatAmmo_Control(INV_FLAT *flat);
void InvFlatAmmo_Draw(INV_FLAT *flat);
void InvFlatAmmo_Close(INV_FLAT *flat);
