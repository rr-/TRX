#include <trx/game/gun/ammo_types.h>

#include <trx/core/utils.h>
#include <trx/game/gun.h>
#include <trx/game/lara/common.h>
#include <trx/game/menu/ring/types.h>

typedef struct {
    AMMO_INFO *counts;
    int8_t *selected;
} M_POOLS;

static bool M_GetPools(const LARA_GUN_TYPE gun_type, M_POOLS *const out)
{
    LARA_INFO *const lara = Lara_GetLaraInfo();
    if (lara == nullptr) {
        return false;
    }
    switch (gun_type) {
    case LGT_SHOTGUN:
        out->counts = lara->shotgun_ammo_types.counts;
        out->selected = &lara->shotgun_ammo_types.selected;
        return true;
    case LGT_GRENADE:
        out->counts = lara->grenade_ammo_types.counts;
        out->selected = &lara->grenade_ammo_types.selected;
        return true;
    case LGT_CROSSBOW:
        out->counts = lara->crossbow_ammo_types.counts;
        out->selected = &lara->crossbow_ammo_types.selected;
        return true;
    default:
        return false;
    }
}

int32_t Gun_GetAmmoTypeCount(const LARA_GUN_TYPE gun_type)
{
    switch (gun_type) {
    case LGT_SHOTGUN:
        return 2;
    case LGT_GRENADE:
    case LGT_CROSSBOW:
        return 3;
    default:
        return 1;
    }
}

AMMO_INFO *Gun_GetAmmoTypeInfo(
    const LARA_GUN_TYPE gun_type, const int32_t type_idx)
{
    M_POOLS pools;
    if (!M_GetPools(gun_type, &pools)
        || type_idx >= Gun_GetAmmoTypeCount(gun_type)) {
        return type_idx == 0 ? Gun_GetAmmoInfo(gun_type) : nullptr;
    }
    Gun_SyncAmmoTypes(gun_type);
    return &pools.counts[type_idx];
}

int8_t Gun_GetSelectedAmmoType(const LARA_GUN_TYPE gun_type)
{
    M_POOLS pools;
    return M_GetPools(gun_type, &pools) ? *pools.selected : 0;
}

void Gun_SyncAmmoTypes(const LARA_GUN_TYPE gun_type)
{
    M_POOLS pools;
    if (!M_GetPools(gun_type, &pools)) {
        return;
    }
    const AMMO_INFO *const active = Gun_GetAmmoInfo(gun_type);
    if (active != nullptr) {
        pools.counts[*pools.selected] = *active;
    }
}

void Gun_CommitAmmoType(const LARA_GUN_TYPE gun_type, const int32_t type_idx)
{
    M_POOLS pools;
    if (!M_GetPools(gun_type, &pools)
        || type_idx >= Gun_GetAmmoTypeCount(gun_type)) {
        return;
    }
    Gun_SyncAmmoTypes(gun_type);
    *pools.selected = type_idx;
    AMMO_INFO *const active = Gun_GetAmmoInfo(gun_type);
    if (active != nullptr) {
        *active = pools.counts[type_idx];
    }
}

static int32_t M_GetPickupVariant(const OBJECT_ID pickup_obj)
{
    switch (pickup_obj) {
    case O_SHOTGUN_AMMO_2_ITEM:
    case O_GRENADE_AMMO_2_ITEM:
    case O_CROSSBOW_AMMO_2_ITEM:
        return 1;
    case O_GRENADE_AMMO_3_ITEM:
    case O_CROSSBOW_AMMO_3_ITEM:
        return 2;
    default:
        return 0;
    }
}

void Gun_AddAmmoFromPickup(
    const LARA_GUN_TYPE gun_type, const OBJECT_ID pickup_obj, const int32_t qty)
{
    AMMO_INFO *const active = Gun_GetAmmoInfo(gun_type);
    const int32_t variant = M_GetPickupVariant(pickup_obj);

    M_POOLS pools;
    if (!M_GetPools(gun_type, &pools)) {
        if (active != nullptr) {
            active->ammo += qty;
            CLAMPG(active->ammo, MAX_QTY);
        }
        return;
    }

    Gun_SyncAmmoTypes(gun_type);
    pools.counts[variant].ammo += qty;
    CLAMPG(pools.counts[variant].ammo, MAX_QTY);
    if (variant == *pools.selected && active != nullptr) {
        *active = pools.counts[variant];
    }
}

void Gun_SetAmmoTypePools(
    const LARA_GUN_TYPE gun_type, const int32_t *const counts,
    const int32_t selected)
{
    M_POOLS pools;
    if (!M_GetPools(gun_type, &pools)) {
        return;
    }
    const int32_t type_count = Gun_GetAmmoTypeCount(gun_type);
    for (int32_t i = 0; i < type_count; i++) {
        pools.counts[i].ammo = counts[i];
    }
    int32_t clamped_selected = selected;
    CLAMP(clamped_selected, 0, type_count - 1);
    *pools.selected = clamped_selected;

    AMMO_INFO *const active = Gun_GetAmmoInfo(gun_type);
    if (active == nullptr) {
        return;
    }
    if (*pools.selected == 0 && counts[0] == 0) {
        // Data predating ammo variants: the classic counter stays
        // authoritative.
        pools.counts[0] = *active;
    } else {
        *active = pools.counts[*pools.selected];
    }
}

GAME_STRING_ID Gun_GetAmmoTypeName(
    const LARA_GUN_TYPE gun_type, const int32_t type_idx)
{
    switch (gun_type) {
    case LGT_SHOTGUN:
        switch (type_idx) {
        case 0:
            return GS_ID("general/ammo_types/normal");
        case 1:
            return GS_ID("general/ammo_types/wideshot");
        }
        break;
    case LGT_GRENADE:
        switch (type_idx) {
        case 0:
            return GS_ID("general/ammo_types/normal");
        case 1:
            return GS_ID("general/ammo_types/super");
        case 2:
            return GS_ID("general/ammo_types/flash");
        }
        break;
    case LGT_CROSSBOW:
        switch (type_idx) {
        case 0:
            return GS_ID("general/ammo_types/normal");
        case 1:
            return GS_ID("general/ammo_types/poison");
        case 2:
            return GS_ID("general/ammo_types/explosive");
        }
        break;
    default:
        break;
    }
    return nullptr;
}
