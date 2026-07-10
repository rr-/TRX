#pragma once

#include <trx/game/objects/ids.h>

#include <stdint.h>

// Inventory combine/separate model (TR4 semantics), independent of how a
// menu strategy presents it: the flat menu shows a second item row, a
// ring strategy could show a secondary ring. Operates purely on the
// inventory and Lara's state.

// Whether the item offers a Combine option right now (a partner is held).
bool InvInteract_CanCombine(OBJECT_ID obj);

// Collects all held partner objects the item can combine with.
int32_t InvInteract_GetCombinePartners(
    OBJECT_ID obj, OBJECT_ID *out_partners, int32_t max_partners);

// Performs the combination (either argument order). Returns the resulting
// object to refocus on via out_combined.
bool InvInteract_Combine(OBJECT_ID a, OBJECT_ID b, OBJECT_ID *out_combined);

// Whether the item can be separated back into its parts.
bool InvInteract_CanSeparate(OBJECT_ID obj);

// Splits the item; returns the first part to refocus on via out_part.
bool InvInteract_Separate(OBJECT_ID obj, OBJECT_ID *out_part);
