#include <trx/game/ui/elements/text_role.h>

#include <trx/game/ui/helpers.h>

static void M_Draw(const UI_NODE *const node)
{
    const UI_TEXT_ROLE role = *(UI_TEXT_ROLE *)node->data;
    const UI_TEXT_ROLE previous = UI_Text_SwapContextRole(role);
    UI_DrawWrapper(node);
    UI_Text_SwapContextRole(previous);
}

void UI_BeginTextRole(const UI_TEXT_ROLE role)
{
    UI_NODE *const node = UI_AllocNode(
        &(UI_WIDGET_OPS) {
            .measure = UI_MeasureWrapper,
            .layout = UI_LayoutWrapper,
            .draw = M_Draw,
        },
        sizeof(UI_TEXT_ROLE));
    *(UI_TEXT_ROLE *)node->data = role;
    UI_AddChild(node);
    UI_PushCurrent(node);
}

void UI_EndTextRole(void)
{
    UI_PopCurrent();
}
