#include <trx/game/menu/interact/ammo.h>

#include <trx/game/gun.h>
#include <trx/game/gun/ammo_types.h>
#include <trx/game/inventory.h>

static LARA_GUN_TYPE M_GetGunType(const OBJECT_ID weapon_obj)
{
    return Gun_GetType(Inv_GetItemPickup(weapon_obj));
}

bool InvInteract_HasAmmoOptions(const OBJECT_ID weapon_obj)
{
    return Gun_GetAmmoTypeCount(M_GetGunType(weapon_obj)) > 1;
}

bool InvInteract_BeginAmmoSession(
    const OBJECT_ID weapon_obj, INV_AMMO_SESSION *const out_session)
{
    const LARA_GUN_TYPE gun_type = M_GetGunType(weapon_obj);
    const int32_t count = Gun_GetAmmoTypeCount(gun_type);
    if (count <= 1) {
        return false;
    }
    Gun_SyncAmmoTypes(gun_type);
    out_session->gun_type = gun_type;
    out_session->option_count = count;
    out_session->stashed_selected = Gun_GetSelectedAmmoType(gun_type);
    return true;
}

GAME_STRING_ID InvInteract_GetAmmoOptionName(
    const INV_AMMO_SESSION *const session, const int32_t idx)
{
    return Gun_GetAmmoTypeName(session->gun_type, idx);
}

int32_t InvInteract_GetAmmoOptionCount(
    const INV_AMMO_SESSION *const session, const int32_t idx)
{
    const AMMO_INFO *const info = Gun_GetAmmoTypeInfo(session->gun_type, idx);
    return info != nullptr ? info->ammo : 0;
}

int32_t InvInteract_GetSelectedAmmoOption(const INV_AMMO_SESSION *const session)
{
    return Gun_GetSelectedAmmoType(session->gun_type);
}

void InvInteract_SelectAmmoOption(
    const INV_AMMO_SESSION *const session, const int32_t idx)
{
    Gun_CommitAmmoType(session->gun_type, idx);
}

void InvInteract_CancelAmmoSession(const INV_AMMO_SESSION *const session)
{
    Gun_CommitAmmoType(session->gun_type, session->stashed_selected);
}
