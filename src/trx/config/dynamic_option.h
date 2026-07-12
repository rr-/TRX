#pragma once

// Config options declared at runtime rather than baked into map*.def.
//
// A static option's target is a field of g_Config; a dynamic one owns its
// storage on the heap. Everything downstream - saving, loading, the console,
// the settings dialogs - reaches options through Config_GetOptionMap(), so a
// dynamic option is indistinguishable from a static one once registered.

#include <trx/config/option.h>
#include <trx/core/vector.h>

#include <stdint.h>

// Where a dynamic option wants to sit in its settings tab. The anchor names
// another option to sit next to; it wins over an explicit priority. Absent
// both, the option lands at the end of the tab.
typedef struct {
    char *tab;
    int32_t priority;
    char *before;
    char *after;
    bool has_priority;
} CONFIG_DYNAMIC_UI;

typedef struct {
    char *name;
    CONFIG_OPTION_TYPE type;
    void *target;
    void *default_value;
    // The last value Config_Update() saw. A static option's dirty check is a
    // memcmp of g_Config, which a heap-allocated value is not part of.
    void *saved_value;
    int32_t min_value;
    int32_t max_value;
    CONFIG_DYNAMIC_UI ui;
} CONFIG_DYNAMIC_OPTION;

// Declares an option. `default_value` is read according to `type` - for
// COT_DYNAMIC_ENUM and COT_STRING that is a const char *, otherwise a pointer
// to the value itself. Returns nullptr if the name is already taken.
CONFIG_DYNAMIC_OPTION *Config_AddDynamicOption(
    const char *name, CONFIG_OPTION_TYPE type, const void *default_value);

// The declarations, in declaration order. Items are CONFIG_DYNAMIC_OPTION.
const VECTOR *Config_GetDynamicOptions(void);

// Drops every dynamic option and its storage. Called when the mod changes, so
// one game's options never leak into the next.
void Config_ClearDynamicOptions(void);

// Whether any dynamic option's value has moved since the last call,
// snapshotting the current values as it goes.
//
// Config_Update() decides whether to fire its change event by memcmp'ing
// g_Config against a saved copy. A dynamic option's value is on the heap, not
// in that struct, so changing one is invisible to that check - the event would
// never fire, and nothing watching the option would ever hear about it.
bool Config_CommitDynamicOptions(void);

// Static rows in a settings tab are ordered by their index in the tab's .def
// file, spaced out so a dynamic row can slot between any two of them.
#define CONFIG_STATIC_ROW_SPACING 100

// Where a dynamic option's row goes, given the option names of the tab's static
// rows in their .def order. An anchor (before/after) wins over an explicit
// priority; an anchor naming a row that is not in this tab falls back, so a
// dynamic row can never displace rows it did not name.
int32_t Config_GetDynamicRowPriority(
    const CONFIG_DYNAMIC_OPTION *option, const char *const *static_names,
    int32_t static_count, int32_t fallback);
