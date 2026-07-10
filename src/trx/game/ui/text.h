#pragma once

#include <trx/core/colors.h>

#include <stddef.h>
#include <stdint.h>

#define UI_TEXT_HEIGHT 15
#define UI_TEXT_BASE_SCALE 0x10000

// Semantic text purpose; the menu strategy style decides how each role
// looks. Roles only affect color, never metrics.
typedef enum {
    // Inherit the role from the enclosing UI_BeginTextRole() context;
    // NORMAL when there is none.
    UI_TEXT_ROLE_DEFAULT = 0,
    UI_TEXT_ROLE_NORMAL,
    UI_TEXT_ROLE_SELECTED,
    UI_TEXT_ROLE_HEADING,
    UI_TEXT_ROLE_VALUE,
    UI_TEXT_ROLE_NUMBER_OF,
} UI_TEXT_ROLE;

typedef struct {
    float scale;
    int32_t z;
    UI_TEXT_ROLE role;
} UI_TEXT_SETTINGS;

// Set the role inherited by texts with UI_TEXT_ROLE_DEFAULT; returns the
// previous role so callers can restore it.
UI_TEXT_ROLE UI_Text_SwapContextRole(UI_TEXT_ROLE role);

// Register (or update) an inline color marker token, e.g.
// "\{color gold}", resolving to the given UI settings text color index.
// Called by the UI settings loader for every named color and alias.
void UI_Text_RegisterColorMarker(const char *token, int32_t color_index);

// One line of text in canvas units, including the strategy style's
// vertical glyph stretch. Use this instead of UI_TEXT_HEIGHT wherever
// layout budgets rows of text.
float UI_Text_GetLineHeight(void);

// Initialize and shutdown UI text rendering cache.
void UI_InitText(void);
void UI_ShutdownText(void);

// Observe level load to establish glyph widths.
void UI_LoadText(void);

// Draw the given text at screen coordinates (x, y) with specified settings.
void UI_Text_Draw(
    const char *text, float x, float y, UI_TEXT_SETTINGS settings);

// Measure the width and height of the given text with specified settings.
void UI_Text_Measure(
    const char *text, float *out_w, float *out_h, UI_TEXT_SETTINGS settings);

// Wrap a text into multiple lines to fit a specific width in pixels.
char *UI_Text_WordWrap(
    const char *text, const float scale, const float max_width);

// Filter out any characters not present in the glyph map.
// Returns a newly-allocated string containing only known glyphs.
// Caller must free the result with Memory_Free*().
char *UI_Text_FilterGlyphs(const char *text);
