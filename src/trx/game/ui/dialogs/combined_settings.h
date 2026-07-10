#pragma once

// A single settings screen aggregating every option group from the
// settings catalog, used by the TR4-style menus.

typedef struct UI_COMBINED_SETTINGS_STATE UI_COMBINED_SETTINGS_STATE;

// state functions
UI_COMBINED_SETTINGS_STATE *UI_CombinedSettings_Init(void);
void UI_CombinedSettings_Free(UI_COMBINED_SETTINGS_STATE *s);
// Returns true when the dialog wants to close.
bool UI_CombinedSettings_Control(UI_COMBINED_SETTINGS_STATE *s);

// draw functions
void UI_CombinedSettings(UI_COMBINED_SETTINGS_STATE *s);
