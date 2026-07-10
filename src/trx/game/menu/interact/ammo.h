#pragma once

#include <trx/game/game_strings/entries.h>
#include <trx/game/lara/types.h>
#include <trx/game/objects/ids.h>

// Ammo variant selection model (TR4 semantics): the menu previews
// selections immediately and restores the stashed choice on cancel, like
// the OG. Strategy-independent.

typedef struct {
    LARA_GUN_TYPE gun_type;
    int32_t option_count;
    int8_t stashed_selected;
} INV_AMMO_SESSION;

// Whether the weapon offers an ammo choice at all.
bool InvInteract_HasAmmoOptions(OBJECT_ID weapon_obj);

bool InvInteract_BeginAmmoSession(
    OBJECT_ID weapon_obj, INV_AMMO_SESSION *out_session);

GAME_STRING_ID InvInteract_GetAmmoOptionName(
    const INV_AMMO_SESSION *session, int32_t idx);

// Rounds held for the variant.
int32_t InvInteract_GetAmmoOptionCount(
    const INV_AMMO_SESSION *session, int32_t idx);

int32_t InvInteract_GetSelectedAmmoOption(const INV_AMMO_SESSION *session);

// Activates the variant right away (OG preview semantics).
void InvInteract_SelectAmmoOption(const INV_AMMO_SESSION *session, int32_t idx);

// Restores the selection active when the session began.
void InvInteract_CancelAmmoSession(const INV_AMMO_SESSION *session);
