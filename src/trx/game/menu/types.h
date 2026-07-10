#pragma once

#include <trx/game/game_flow/types.h>
#include <trx/game/menu/enum.h>
#include <trx/game/ui/text.h>

// Opaque per-strategy menu state. Each strategy defines its own layout.
typedef struct INV_MENU INV_MENU;

typedef struct {
    bool needs_game_snapshot;
    bool needs_inventory_lighting;
    // The menu renders the running level itself instead of a background
    // image or a game snapshot.
    bool live_scene;
} INV_MENU_CAPS;

typedef enum {
    INV_PAUSE_NOOP,
    INV_PAUSE_RESUME,
    INV_PAUSE_EXIT_TO_TITLE,
} INV_PAUSE_CHOICE;

// Opaque pause menu state; strategies replacing the classic paused
// screen provide these ops.
typedef struct INV_PAUSE_MENU INV_PAUSE_MENU;

typedef struct {
    INV_PAUSE_MENU *(*init)(void);
    INV_PAUSE_CHOICE (*control)(INV_PAUSE_MENU *menu);
    void (*draw)(INV_PAUSE_MENU *menu);
    void (*free)(INV_PAUSE_MENU *menu);
} INV_PAUSE_MENU_OPS;

// Menu-wide UI styling implied by the strategy; consulted also outside of
// the inventory phase (dialogs, pause, HUD text).
typedef struct {
    // Text boxes, outlines and divider lines around UI elements.
    bool draw_menu_chrome;
    // Multiplier applied on top of the user text scale.
    float text_base_scale;
    // Vertical glyph stretch (0 = none); the OG TR4 draws its glyphs
    // noticeably taller than they are stored (capitals land at roughly a
    // 1.37 height/width ratio on screen).
    float text_v_stretch;
    // ui.json5 text style profile binding colors to the semantic text
    // roles; nullptr keeps the classic text palette.
    const char *text_style;
} INV_MENU_STYLE;

typedef struct {
    INV_MENU *(*open)(INVENTORY_MODE mode);
    GF_COMMAND (*control)(INV_MENU *menu);
    void (*draw)(INV_MENU *menu);
    void (*close)(INV_MENU *menu);
    bool (*is_done)(const INV_MENU *menu);
    INV_MENU_CAPS (*get_caps)(INVENTORY_MODE mode);
    INV_MENU_STYLE style;
    // Replaces the classic pause screen when set.
    const INV_PAUSE_MENU_OPS *pause_menu;
} INV_MENU_OPS;
