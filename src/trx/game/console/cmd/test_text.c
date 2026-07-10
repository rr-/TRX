#include <trx/game/console/common.h>
#include <trx/game/console/registry.h>
#include <trx/game/ui/settings.h>

#include <stdio.h>

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    char buf[1500] = {};
    char *ptr = buf;

    // Every named color from the ui.json5 registry.
    const int32_t color_count = UI_Settings_GetTextColorCount();
    for (int32_t i = 0; i < color_count; i++) {
        const UI_TEXT_COLOR *const color = UI_Settings_GetTextColorByIndex(i);
        ptr += sprintf(
            ptr, "\\{color %s}%s\\{/color}   ", color->name, color->name);
        if (i % 4 == 3) {
            ptr += sprintf(ptr, "\n");
        }
    }
    ptr += sprintf(ptr, "\n");

    // The semantic roles under the active menu style profile.
    ptr += sprintf(
        ptr,
        "\\{role normal}normal\\{/role}   "
        "\\{role selected}selected\\{/role}   "
        "\\{role heading}heading\\{/role}   "
        "\\{role value}value\\{/role}\n");

    ptr += sprintf(ptr, "\\{dim}Dim\\{/dim}\n");
    ptr += sprintf(ptr, "Secrets: \\{secret 1}\\{secret 2}\\{secret 3}");
    Console_Log("%s", buf);
    return CR_SUCCESS;
}

REGISTER_CONSOLE_COMMAND("test-text", M_Entrypoint, nullptr)
