#pragma once

#include <trx/game/game_strings/entries.h>
#include <trx/game/lara/types.h>
#include <trx/game/objects/ids.h>

// TR4 ammo variants (wideshot shells, super/flash grenades,
// poison/explosive bolts). Each variant has its own pool; the weapon's
// classic AMMO_INFO always holds the active variant's count, so weapon
// firing code stays untouched.

#define GUN_MAX_AMMO_TYPES 3

// 1 for weapons without variants.
int32_t Gun_GetAmmoTypeCount(LARA_GUN_TYPE gun_type);

AMMO_INFO *Gun_GetAmmoTypeInfo(LARA_GUN_TYPE gun_type, int32_t type_idx);

int8_t Gun_GetSelectedAmmoType(LARA_GUN_TYPE gun_type);

// Absorbs gameplay ammo drain back into the selected variant's pool;
// call before reading the pools.
void Gun_SyncAmmoTypes(LARA_GUN_TYPE gun_type);

// Selects a variant and activates its pool.
void Gun_CommitAmmoType(LARA_GUN_TYPE gun_type, int32_t type_idx);

// Routes an ammo pickup into the right variant pool.
void Gun_AddAmmoFromPickup(
    LARA_GUN_TYPE gun_type, OBJECT_ID pickup_obj, int32_t qty);

// Restores the pools from persisted state; all-zero legacy data keeps the
// classic counter authoritative.
void Gun_SetAmmoTypePools(
    LARA_GUN_TYPE gun_type, const int32_t *counts, int32_t selected);

GAME_STRING_ID Gun_GetAmmoTypeName(LARA_GUN_TYPE gun_type, int32_t type_idx);
