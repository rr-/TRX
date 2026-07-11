#include <trx/game/menu/flat/draw.h>

#include <trx/config.h>
#include <trx/core/math.h>
#include <trx/core/utils.h>
#include <trx/game/const.h>
#include <trx/game/game_strings/entries.h>
#include <trx/game/interpolation.h>
#include <trx/game/matrix.h>
#include <trx/game/menu/flat/options_menu.h>
#include <trx/game/menu/item_draw.h>
#include <trx/game/objects.h>
#include <trx/game/output.h>
#include <trx/game/output/scene_compositor.h>
#include <trx/game/ui/elements/label.h>
#include <trx/game/ui/elements/modal.h>
#include <trx/game/ui/elements/text_role.h>
#include <trx/game/viewport.h>

#include <math.h>

// Default camera distance for items without display parameters.
#define M_ROW_Z 1200
// Focal length in the OG's 640-wide screen units at the shared FOV of 80
// degrees; converts screen offsets to world offsets at a given distance.
#define M_FOCAL 381.0f
// The OG spaces items a quarter of the screen width apart.
#define M_SLOT_SPACING 160.0f
// Vertical screen placement of the two rows relative to the center.
#define M_ROW_SCREEN_Y (-26.0f)
#define M_SECOND_ROW_SCREEN_Y 61.0f
// How many slots away from the focus an item is still drawn.
#define M_VISIBLE_RANGE 3.5f

// The OG renders the prelit meshes with a flat per-item brightness and no
// light rig; 127 is neutral, the focused item ramps up to 160 and the
// rest sit at a dim 32.
#define M_BRIGHT_NORMAL 32.0f
#define M_BRIGHT_FOCUSED 160.0f
#define M_BRIGHT_NEUTRAL 127.0f

static void M_Light(void)
{
    Output_SetLightDivider(0x6000);
    Output_RotateLight(0, 0);
    const RGB_F ambient = { 1.0f, 1.0f, 1.0f };
    const RGB_F colors[3] = {};
    const XYZ_32 dirs_view[3] = {
        { .z = 0x4000 },
        { .z = 0x4000 },
        { .z = 0x4000 },
    };
    Output_SetTR3Light(ambient, colors, dirs_view);
}

static void M_SetBrightness(const float bright)
{
    Output_SetLightAdder(0x2000 - (int32_t)(0x1000 * bright / 127.0f));
}

// Screen placement of the corner compass.
#define M_COMPASS_SCREEN_X 117.0f
#define M_COMPASS_SCREEN_Y 65.0f

static float M_GetItemDist(const INVENTORY_ITEM *const inv_item)
{
    return inv_item->flat_dist > 0 ? inv_item->flat_dist : M_ROW_Z;
}

// Positions an item so it appears at the given screen offsets from the
// center regardless of its camera distance.
static void M_PlaceItem(
    const INVENTORY_ITEM *const inv_item, const float screen_x,
    const float screen_y)
{
    const float dist = M_GetItemDist(inv_item);
    Matrix_TranslateRel(
        screen_x * dist / M_FOCAL,
        (screen_y + inv_item->flat_y_off) * dist / M_FOCAL, dist);
}

static void M_DrawItemMeshes(const INVENTORY_ITEM *const inv_item)
{
    const OBJECT *const obj = Object_Get(inv_item->object_id);
    if (obj->loaded && obj->mesh_count >= 0) {
        const ANIM_FRAME *const frame =
            &obj->frame_base[inv_item->current_frame];
        InvItem_DrawObject(inv_item, frame, frame, 0, 1);
    }
}

static void M_DrawCompass(const INV_FLAT *const flat)
{
    if (flat->compass == nullptr) {
        return;
    }
    Matrix_Push();
    M_PlaceItem(flat->compass, M_COMPASS_SCREEN_X, M_COMPASS_SCREEN_Y);
    M_SetBrightness(M_BRIGHT_NEUTRAL);
    Matrix_Rot16(flat->compass->flat_rot);
    Matrix_RotX(flat->compass->x_rot);
    M_DrawItemMeshes(flat->compass);
    Matrix_Pop();
}

static void M_DrawItem(
    const INVENTORY_ITEM *const inv_item, const int32_t idx,
    const float scroll_pos, const int16_t spin_rot, const float row_screen_y)
{
    const float offset = idx - scroll_pos;

    Matrix_Push();
    M_PlaceItem(inv_item, offset * M_SLOT_SPACING, row_screen_y);

    const float focus = 1.0f - MIN(1.0f, fabsf(offset));
    M_SetBrightness(LERP(M_BRIGHT_NORMAL, M_BRIGHT_FOCUSED, focus));

    Matrix_RotY(focus > 0.5f ? spin_rot : 0);
    Matrix_Rot16(inv_item->flat_rot);
    M_DrawItemMeshes(inv_item);
    Matrix_Pop();
}

void InvFlat_Draw(INV_FLAT *const flat)
{
    const float opacity = g_Config.ui.inventory_fade_effects
        ? Fader_GetCurrentValue(&flat->back_fader)
        : flat->back_fader.args.target;
    Output_Overlay_DrawBackground(BK_MONOCHROME, opacity, nullptr);
    Output_Flush();

    if (flat->state == IF_DONE) {
        return;
    }

    const double interp_rate = Interpolation_GetRate();
    const float draw_scroll_pos =
        LERP(flat->prev_scroll_pos, flat->scroll_pos, interp_rate);
    const int16_t draw_spin_rot = (int16_t)(uint16_t)Math_AngleMean(
        (uint16_t)flat->prev_spin_rot, (uint16_t)flat->spin_rot, interp_rate);

    const int16_t old_fov = Viewport_GetSystemFOV();
    const FOV_MODE old_fov_mode = Viewport_GetFOVMode();
    Viewport_AlterFOV(FOV_VALUE_PASSPORT * DEG_1, FOV_MODE_PASSPORT);
    Output_ApplyFOV();

    const XYZ_32 view_pos = {};
    const XYZ_16 view_rot = {};
    Matrix_GenerateW2V(&view_pos, &view_rot);

    const int32_t old_fog_start = Output_GetFogStart();
    const int32_t old_fog_end = Output_GetFogEnd();
    Output_SetFogStart(20 * WALL_L);
    Output_SetFogEnd(100 * WALL_L);

    M_Light();

    if (flat->state == IF_EXAMINE) {
        // Close-up inspection: only the focused item, steered by input.
        const INVENTORY_ITEM *const inv_item = flat->items[flat->target_idx];
        Matrix_Push();
        Matrix_TranslateRel(0, 0, M_GetItemDist(inv_item) / 2);
        M_SetBrightness(M_BRIGHT_NEUTRAL);
        Matrix_RotY((int16_t)(uint16_t)Math_AngleMean(
            (uint16_t)flat->examine.prev_y_rot, (uint16_t)flat->examine.y_rot,
            interp_rate));
        Matrix_RotX((int16_t)(uint16_t)Math_AngleMean(
            (uint16_t)flat->examine.prev_x_rot, (uint16_t)flat->examine.x_rot,
            interp_rate));
        Matrix_Rot16(inv_item->flat_rot);
        M_DrawItemMeshes(inv_item);
        Matrix_Pop();
    } else if (flat->state != IF_LOADSAVE) {
        // While the partner row is open, the main row holds still and the
        // focused partner spins instead.
        const int16_t main_spin = flat->state == IF_COMBINE ? 0 : draw_spin_rot;
        for (int32_t i = 0; i < flat->item_count; i++) {
            if (fabsf(i - draw_scroll_pos) > M_VISIBLE_RANGE) {
                continue;
            }
            M_DrawItem(
                flat->items[i], i, draw_scroll_pos, main_spin, M_ROW_SCREEN_Y);
        }
        if (flat->state == IF_COMBINE) {
            const float second_scroll_pos = LERP(
                flat->second_row.prev_scroll_pos, flat->second_row.scroll_pos,
                interp_rate);
            for (int32_t i = 0; i < flat->second_row.count; i++) {
                if (fabsf(i - second_scroll_pos) > M_VISIBLE_RANGE) {
                    continue;
                }
                M_DrawItem(
                    flat->second_row.items[i], i, second_scroll_pos,
                    draw_spin_rot, M_SECOND_ROW_SCREEN_Y);
            }
        }
        M_DrawCompass(flat);
    }

    SceneCompositor_Flush();
    Output_SetFogStart(old_fog_start);
    Output_SetFogEnd(old_fog_end);
    Viewport_AlterFOV(old_fov, old_fov_mode);

    if (flat->state == IF_OPTION_MENU) {
        InvFlatOptions_Draw(flat);
    } else if (flat->state == IF_AMMO) {
        InvFlatAmmo_Draw(flat);
    } else if (flat->state == IF_COMBINE) {
        // "Combine with" header above the partner row.
        UI_BeginModal(0.5f, 0.62f);
        UI_BeginTextRole(UI_TEXT_ROLE_HEADING);
        UI_Label(GS("general/inventory_flat/combine_with"));
        UI_EndTextRole();
        UI_EndModal();
    } else if (flat->state == IF_LOADSAVE) {
        UI_SaveSlotDialog(flat->save_slot.dialog);
    }
}
