#include <trx/game/menu/item_draw.h>

#include <trx/game/const.h>
#include <trx/game/game.h>
#include <trx/game/objects.h>
#include <trx/game/option/stats.h>
#include <trx/game/savegame.h>

void InvItem_DrawObject(
    const INVENTORY_ITEM *const inv_item, const ANIM_FRAME *const frame1,
    const ANIM_FRAME *const frame2, const int32_t frac, const int32_t rate)
{
    const OBJECT *const obj = Object_Get(inv_item->object_id);
    if (!obj->loaded || obj->mesh_count < 0) {
        return;
    }

    if (inv_item->object_id == O_COMPASS_OPTION) {
        const int16_t extra_rotation[1] = {
            Option_Stats_GetCompassNeedleAngle()
        };
        Object_GetBone(obj, 0)->rot.y = true;
        Object_DrawInterpolatedObject(
            obj, inv_item->meshes_drawn, extra_rotation, frame1, frame2, frac,
            rate);
    } else if (inv_item->object_id == O_STOPWATCH_OPTION) {
        const RESUME_INFO *const current_info =
            Savegame_GetCurrentInfo(Game_GetCurrentLevel());
        const int32_t total_seconds = current_info->stats.timer / LOGIC_FPS;
        const int32_t hours = (total_seconds % 43200) * DEG_1 * -360 / 43200;
        const int32_t minutes = (total_seconds % 3600) * DEG_1 * -360 / 3600;
        const int32_t seconds = (total_seconds % 60) * DEG_1 * -360 / 60;

        const int16_t extra_rotation[3] = { hours, minutes, seconds };
        Object_GetBone(obj, 3)->rot.z = true;
        Object_GetBone(obj, 4)->rot.z = true;
        Object_GetBone(obj, 5)->rot.z = true;
        Object_DrawInterpolatedObject(
            obj, inv_item->meshes_drawn, extra_rotation, frame1, frame2, frac,
            rate);
    } else {
        Object_DrawInterpolatedObject(
            obj, inv_item->meshes_drawn, nullptr, frame1, frame2, frac, rate);
    }
}
