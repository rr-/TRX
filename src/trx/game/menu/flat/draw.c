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

// Distance of the item row from the camera and its vertical placement.
#define M_ROW_Z 1400
#define M_ROW_Y (-96)
// The "Combine with" partner row sits below the main row.
#define M_SECOND_ROW_Y 224
// Horizontal distance between two adjacent items; OG spaces them a quarter
// of the screen width apart.
#define M_ITEM_SPACING 352
// How many slots away from the focus an item is still drawn.
#define M_VISIBLE_RANGE 3.5f

#define M_SHADE_NORMAL SHADE_LOW
#define M_SHADE_FOCUSED SHADE_NEUTRAL

static XYZ_32 M_VectorViewFromWorld(const XYZ_32 v_world)
{
    return Matrix_MulVec32_M(&g_ViewMatrix, v_world);
}

static void M_Light(void)
{
    // Mirrors InvRing_Light for TR3+, with the ring's fixed light direction.
    int16_t angles[2];
    Math_GetVectorAngles(-1536, 256, 1024, angles);
    Output_SetLightDivider(0x6000);
    Output_RotateLight(angles[1], angles[0]);

    const float ambient_u8 = 32.0f / 255.0f;
    const RGB_F ambient = { ambient_u8, ambient_u8, ambient_u8 };
    const RGB_F colors[3] = {
        { .r = 3312.0f / 4096.0f, .g = 1664.0f / 4096.0f, .b = 0.0f },
        {
            .r = 3312.0f / 4096.0f,
            .g = 3312.0f / 4096.0f,
            .b = 3312.0f / 4096.0f,
        },
        { .r = 0.0f, .g = 0.0f, .b = 3072.0f / 4096.0f },
    };
    const XYZ_32 dirs_view[3] = {
        M_VectorViewFromWorld(
            (XYZ_32) { .x = 0x4000, .y = -0x4000, .z = 0x3000 }),
        M_VectorViewFromWorld(
            (XYZ_32) { .x = -0x4000, .y = -0x4000, .z = 0x3000 }),
        M_VectorViewFromWorld((XYZ_32) { .x = 0, .y = 0x2000, .z = 0x3000 }),
    };
    Output_SetTR3Light(ambient, colors, dirs_view);
}

// Placement of the corner compass.
#define M_COMPASS_X 430
#define M_COMPASS_Y 240
#define M_COMPASS_Z 1400

static void M_DrawCompass(const INV_FLAT *const flat)
{
    if (flat->compass == nullptr) {
        return;
    }
    Matrix_Push();
    Matrix_TranslateRel(M_COMPASS_X, M_COMPASS_Y, M_COMPASS_Z);
    Output_SetLightAdder(M_SHADE_FOCUSED);
    Matrix_RotX(flat->compass->x_rot);

    const OBJECT *const obj = Object_Get(flat->compass->object_id);
    if (obj->loaded && obj->mesh_count >= 0) {
        const ANIM_FRAME *const frame = &obj->frame_base[0];
        InvItem_DrawObject(flat->compass, frame, frame, 0, 1);
    }
    Matrix_Pop();
}

static void M_DrawItem(
    const INVENTORY_ITEM *const inv_item, const int32_t idx,
    const float scroll_pos, const int16_t spin_rot, const int32_t row_y)
{
    const float offset = idx - scroll_pos;

    Matrix_Push();
    Matrix_TranslateRel(offset * M_ITEM_SPACING, row_y, M_ROW_Z);

    const float focus = 1.0f - MIN(1.0f, fabsf(offset));
    Output_SetLightAdder(
        LERP((float)M_SHADE_NORMAL, (float)M_SHADE_FOCUSED, focus));

    Matrix_RotY(focus > 0.5f ? spin_rot : 0);
    Matrix_RotX(inv_item->x_rot);

    const OBJECT *const obj = Object_Get(inv_item->object_id);
    if (obj->loaded && obj->mesh_count >= 0) {
        const ANIM_FRAME *const frame =
            &obj->frame_base[inv_item->current_frame];
        InvItem_DrawObject(inv_item, frame, frame, 0, 1);
    }
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

    if (flat->state != IF_LOADSAVE) {
        // While the partner row is open, the main row holds still and the
        // focused partner spins instead.
        const int16_t main_spin = flat->state == IF_COMBINE ? 0 : draw_spin_rot;
        for (int32_t i = 0; i < flat->item_count; i++) {
            if (fabsf(i - draw_scroll_pos) > M_VISIBLE_RANGE) {
                continue;
            }
            M_DrawItem(flat->items[i], i, draw_scroll_pos, main_spin, M_ROW_Y);
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
                    draw_spin_rot, M_SECOND_ROW_Y);
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
