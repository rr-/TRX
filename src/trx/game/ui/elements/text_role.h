#pragma once

#include <trx/game/ui/text.h>

// Context wrapper assigning a semantic text role to all descendant texts
// that don't set an explicit role of their own.

void UI_BeginTextRole(UI_TEXT_ROLE role);
void UI_EndTextRole(void);
