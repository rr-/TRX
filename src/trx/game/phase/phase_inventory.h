#pragma once

#include <trx/game/menu/enum.h>
#include <trx/game/phase/types.h>

PHASE *Phase_Inventory_Create(INVENTORY_MODE mode);
void Phase_Inventory_Destroy(PHASE *phase);
