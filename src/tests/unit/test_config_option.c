#include "harness.h"

#include <trx/config/dynamic_option.h>

#include <string.h>

// dynamic_option.c reaches out to exactly one thing: the map invalidation hook
// in map.c, which would drag in g_Config and the whole engine behind it. The
// declarations themselves are self-contained, so stub it and test them alone -
// no config file, no game, no level.
void Config_InvalidateOptionMap(void)
{
}

static void M_Reset(void)
{
    Config_ClearDynamicOptions();
}

TEST(declaring_an_option_gives_it_storage_holding_its_default)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.flag", COT_BOOL, &(bool) { true });
    CHECK_NOT_NULL(option);
    CHECK_EQ_STR(option->name, "test.flag");
    CHECK_EQ_INT(option->type, COT_BOOL);

    // A static option's target is a field of g_Config. A dynamic one has to own
    // its storage, and hold the default until the config is read over it.
    CHECK_NOT_NULL(option->target);
    CHECK(*(bool *)option->target == true);
    M_Reset();
}

TEST(a_declared_string_option_stores_its_default_the_way_the_engine_reads_it)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_DYNAMIC_ENUM, "ps1");
    CHECK_NOT_NULL(option);

    // The two are deliberately not symmetric, and this is not a typo. The
    // target is a `char *` box, like a string field of g_Config. The default is
    // the string itself - X_CFG_STRING passes a bare literal, and config/file.c
    // and Config_IsOptionAtDefault cast default_value straight to const char *.
    //
    // Boxing the default too handed those readers a pointer to read as text:
    // the option rendered as a stray glyph, never compared equal to any real
    // value, and wrote that garbage out to the player's config file.
    CHECK_EQ_STR(*(char **)option->target, "ps1");
    CHECK_EQ_STR((const char *)option->default_value, "ps1");

    // They must not alias either: restoring a default frees the target first,
    // which would take the default with it.
    CHECK((void *)*(char **)option->target != option->default_value);
    M_Reset();
}

TEST(an_option_cannot_be_declared_twice)
{
    M_Reset();
    CHECK_NOT_NULL(
        Config_AddDynamicOption("test.dupe", COT_BOOL, &(bool) { 0 }));
    // Two options with one name would both land in the map, and every lookup by
    // name would silently pick the first - so the second's row would edit a
    // value nothing reads.
    CHECK_NULL(Config_AddDynamicOption("test.dupe", COT_BOOL, &(bool) { 0 }));
    CHECK_EQ_INT(Config_GetDynamicOptions()->count, 1);
    M_Reset();
}

TEST(clearing_drops_every_declaration)
{
    M_Reset();
    Config_AddDynamicOption("test.a", COT_BOOL, &(bool) { 0 });
    Config_AddDynamicOption("test.b", COT_INT32, &(int32_t) { 7 });
    CHECK_EQ_INT(Config_GetDynamicOptions()->count, 2);

    // Switching game must not leave the previous game's options in the map.
    Config_ClearDynamicOptions();
    CHECK_NULL(Config_GetDynamicOptions());
}

// The tab's static rows, in .def order. Row 1 is what the water colour option
// anchors itself to in the real thing.
static const char *const m_StaticNames[] = {
    "visuals.fog_color",
    "visuals.water_color",
    "visuals.fov",
};
#define M_STATIC_COUNT 3
#define M_FALLBACK 9999

TEST(before_puts_a_row_directly_above_the_option_it_names)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_BOOL, &(bool) { 0 });
    option->ui.before = "visuals.water_color";

    // water_color is static row 1, so it sorts at 100. Landing at 99 puts the
    // new row between fog_color (0) and water_color (100) - and nowhere else.
    const int32_t priority = Config_GetDynamicRowPriority(
        option, m_StaticNames, M_STATIC_COUNT, M_FALLBACK);
    CHECK_EQ_INT(priority, 99);
    CHECK(priority > 0 * CONFIG_STATIC_ROW_SPACING);
    CHECK(priority < 1 * CONFIG_STATIC_ROW_SPACING);

    option->ui.before = nullptr; // not heap-owned here
    M_Reset();
}

TEST(after_puts_a_row_directly_below_the_option_it_names)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_BOOL, &(bool) { 0 });
    option->ui.after = "visuals.water_color";

    const int32_t priority = Config_GetDynamicRowPriority(
        option, m_StaticNames, M_STATIC_COUNT, M_FALLBACK);
    CHECK_EQ_INT(priority, 101);
    CHECK(priority > 1 * CONFIG_STATIC_ROW_SPACING);
    CHECK(priority < 2 * CONFIG_STATIC_ROW_SPACING);

    option->ui.after = nullptr;
    M_Reset();
}

TEST(an_anchor_beats_an_explicit_priority)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_BOOL, &(bool) { 0 });
    option->ui.before = "visuals.fov";
    option->ui.priority = 42;
    option->ui.has_priority = true;

    CHECK_EQ_INT(
        Config_GetDynamicRowPriority(
            option, m_StaticNames, M_STATIC_COUNT, M_FALLBACK),
        199);

    option->ui.before = nullptr;
    M_Reset();
}

TEST(an_explicit_priority_is_used_when_no_anchor_is_given)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_BOOL, &(bool) { 0 });
    option->ui.priority = 250;
    option->ui.has_priority = true;

    CHECK_EQ_INT(
        Config_GetDynamicRowPriority(
            option, m_StaticNames, M_STATIC_COUNT, M_FALLBACK),
        250);
    M_Reset();
}

TEST(an_anchor_naming_a_row_this_tab_does_not_have_falls_back)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_BOOL, &(bool) { 0 });

    // The named row is on another tab, or belongs to a game that is not this
    // one. Degrade to the end of the tab rather than reorder rows at random.
    option->ui.before = "audio.music_volume";
    CHECK_EQ_INT(
        Config_GetDynamicRowPriority(
            option, m_StaticNames, M_STATIC_COUNT, M_FALLBACK),
        M_FALLBACK);

    option->ui.before = nullptr;
    M_Reset();
}

TEST(a_row_with_neither_anchor_nor_priority_lands_at_the_fallback)
{
    M_Reset();
    CONFIG_DYNAMIC_OPTION *const option =
        Config_AddDynamicOption("test.mode", COT_BOOL, &(bool) { 0 });
    CHECK_EQ_INT(
        Config_GetDynamicRowPriority(
            option, m_StaticNames, M_STATIC_COUNT, M_FALLBACK),
        M_FALLBACK);
    M_Reset();
}
