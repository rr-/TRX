#pragma once

#include <trx/game/game_flow/types.h>
#include <trx/game/menu/flat/types.h>

INV_FLAT *InvFlat_Open(INVENTORY_MODE mode);
GF_COMMAND InvFlat_Control(INV_FLAT *flat);
void InvFlat_Close(INV_FLAT *flat);
