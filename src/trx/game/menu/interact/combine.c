#include <trx/game/menu/interact/combine.h>

#include <trx/core/utils.h>
#include <trx/game/inventory.h>
#include <trx/game/lara/common.h>

// TODO: the OG also combines the mechanical scarab with the winding key
// into the clockwork beetle; those objects aren't cataloged for TR4 yet.

typedef struct {
    OBJECT_ID item1;
    OBJECT_ID item2;
    OBJECT_ID combined;
    void (*apply)(void);
    void (*separate)(void);
} M_COMBINE_ROW;

static void M_MountRevolverLasersight(void)
{
    Inv_RemoveItem(O_LASERSIGHT_OPTION);
    Lara_GetLaraInfo()->lasersight.revolver = true;
}

static void M_UnmountRevolverLasersight(void)
{
    Lara_GetLaraInfo()->lasersight.revolver = false;
    Inv_AddItem(O_LASERSIGHT_OPTION);
}

static void M_MountCrossbowLasersight(void)
{
    Inv_RemoveItem(O_LASERSIGHT_OPTION);
    Lara_GetLaraInfo()->lasersight.crossbow = true;
}

static void M_UnmountCrossbowLasersight(void)
{
    Lara_GetLaraInfo()->lasersight.crossbow = false;
    Inv_AddItem(O_LASERSIGHT_OPTION);
}

#define M_COMBO_ROW(prefix, n)                                                 \
    {                                                                          \
        .item1 = prefix##_##n##_COMBO_1,                                       \
        .item2 = prefix##_##n##_COMBO_2,                                       \
        .combined = prefix##_##n,                                              \
    }

static const M_COMBINE_ROW m_CombineTable[] = {
    {
        .item1 = O_REVOLVER_OPTION,
        .item2 = O_LASERSIGHT_OPTION,
        .combined = O_REVOLVER_OPTION,
        .apply = M_MountRevolverLasersight,
        .separate = M_UnmountRevolverLasersight,
    },
    {
        .item1 = O_CROSSBOW_OPTION,
        .item2 = O_LASERSIGHT_OPTION,
        .combined = O_CROSSBOW_OPTION,
        .apply = M_MountCrossbowLasersight,
        .separate = M_UnmountCrossbowLasersight,
    },
    M_COMBO_ROW(O_PUZZLE_OPTION, 1),
    M_COMBO_ROW(O_PUZZLE_OPTION, 2),
    M_COMBO_ROW(O_PUZZLE_OPTION, 3),
    M_COMBO_ROW(O_PUZZLE_OPTION, 4),
    M_COMBO_ROW(O_PUZZLE_OPTION, 5),
    M_COMBO_ROW(O_PUZZLE_OPTION, 6),
    M_COMBO_ROW(O_PUZZLE_OPTION, 7),
    M_COMBO_ROW(O_PUZZLE_OPTION, 8),
    M_COMBO_ROW(O_KEY_OPTION, 1),
    M_COMBO_ROW(O_KEY_OPTION, 2),
    M_COMBO_ROW(O_KEY_OPTION, 3),
    M_COMBO_ROW(O_KEY_OPTION, 4),
    M_COMBO_ROW(O_KEY_OPTION, 5),
    M_COMBO_ROW(O_KEY_OPTION, 6),
    M_COMBO_ROW(O_KEY_OPTION, 7),
    M_COMBO_ROW(O_KEY_OPTION, 8),
    M_COMBO_ROW(O_PICKUP_OPTION, 1),
    M_COMBO_ROW(O_PICKUP_OPTION, 2),
    M_COMBO_ROW(O_PICKUP_OPTION, 3),
    M_COMBO_ROW(O_PICKUP_OPTION, 4),
    { .item1 = NO_OBJECT },
};

static bool M_IsWaterskin(const OBJECT_ID obj)
{
    return obj == O_WATERSKIN_1_OPTION || obj == O_WATERSKIN_2_OPTION;
}

// Pours the selected skin into the other one, as much as fits. The
// receiving skin is reported for the menu to refocus on.
static bool M_PourWaterskin(const OBJECT_ID from, OBJECT_ID *const out_combined)
{
    LARA_INFO *const lara = Lara_GetLaraInfo();
    int8_t *small = &lara->small_water_skin;
    int8_t *big = &lara->big_water_skin;

    int8_t *src = from == O_WATERSKIN_1_OPTION ? small : big;
    int8_t *dst = from == O_WATERSKIN_1_OPTION ? big : small;
    const int8_t dst_capacity = (dst == small ? 3 : 5) - *dst;
    if (*src == 0 || dst_capacity == 0) {
        return false;
    }

    const int8_t moved = MIN(*src, dst_capacity);
    *src -= moved;
    *dst += moved;
    if (out_combined != nullptr) {
        *out_combined = from == O_WATERSKIN_1_OPTION ? O_WATERSKIN_2_OPTION
                                                     : O_WATERSKIN_1_OPTION;
    }
    return true;
}

static const M_COMBINE_ROW *M_FindPair(const OBJECT_ID a, const OBJECT_ID b)
{
    for (const M_COMBINE_ROW *row = m_CombineTable; row->item1 != NO_OBJECT;
         row++) {
        if ((row->item1 == a && row->item2 == b)
            || (row->item1 == b && row->item2 == a)) {
            return row;
        }
    }
    return nullptr;
}

bool InvInteract_CanCombine(const OBJECT_ID obj)
{
    return InvInteract_GetCombinePartners(obj, nullptr, 0) > 0;
}

int32_t InvInteract_GetCombinePartners(
    const OBJECT_ID obj, OBJECT_ID *const out_partners,
    const int32_t max_partners)
{
    int32_t count = 0;

#define M_ADD_PARTNER(partner)                                                 \
    do {                                                                       \
        if (out_partners != nullptr && count < max_partners) {                 \
            out_partners[count] = partner;                                     \
        }                                                                      \
        count++;                                                               \
    } while (0)

    if (M_IsWaterskin(obj)) {
        const OBJECT_ID other = obj == O_WATERSKIN_1_OPTION
            ? O_WATERSKIN_2_OPTION
            : O_WATERSKIN_1_OPTION;
        if (Inv_RequestItem(other) > 0) {
            M_ADD_PARTNER(other);
        }
        return count;
    }

    for (const M_COMBINE_ROW *row = m_CombineTable; row->item1 != NO_OBJECT;
         row++) {
        OBJECT_ID partner = NO_OBJECT;
        if (row->item1 == obj) {
            partner = row->item2;
        } else if (row->item2 == obj) {
            partner = row->item1;
        } else {
            continue;
        }
        if (Inv_RequestItem(partner) > 0) {
            M_ADD_PARTNER(partner);
        }
    }

#undef M_ADD_PARTNER
    return count;
}

bool InvInteract_Combine(
    const OBJECT_ID a, const OBJECT_ID b, OBJECT_ID *const out_combined)
{
    if (M_IsWaterskin(a) && M_IsWaterskin(b) && a != b) {
        return M_PourWaterskin(a, out_combined);
    }

    const M_COMBINE_ROW *const row = M_FindPair(a, b);
    if (row == nullptr || Inv_RequestItem(row->item1) <= 0
        || Inv_RequestItem(row->item2) <= 0) {
        return false;
    }

    if (row->apply != nullptr) {
        row->apply();
    } else {
        Inv_RemoveItem(row->item1);
        Inv_RemoveItem(row->item2);
        Inv_AddItem(row->combined);
    }
    if (out_combined != nullptr) {
        *out_combined = row->combined;
    }
    return true;
}

static const M_COMBINE_ROW *M_FindSeparable(const OBJECT_ID obj)
{
    const LARA_INFO *const lara = Lara_GetLaraInfo();
    for (const M_COMBINE_ROW *row = m_CombineTable; row->item1 != NO_OBJECT;
         row++) {
        // Like the OG, only the lasersight mounts separate back.
        if (row->separate == nullptr || row->combined != obj) {
            continue;
        }
        const bool mounted = obj == O_REVOLVER_OPTION
            ? lara->lasersight.revolver
            : lara->lasersight.crossbow;
        if (mounted) {
            return row;
        }
    }
    return nullptr;
}

bool InvInteract_CanSeparate(const OBJECT_ID obj)
{
    return M_FindSeparable(obj) != nullptr;
}

bool InvInteract_Separate(const OBJECT_ID obj, OBJECT_ID *const out_part)
{
    const M_COMBINE_ROW *const row = M_FindSeparable(obj);
    if (row == nullptr) {
        return false;
    }
    row->separate();
    if (out_part != nullptr) {
        *out_part = row->item1;
    }
    return true;
}
