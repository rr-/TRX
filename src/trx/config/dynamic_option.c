#include <trx/config/dynamic_option.h>

#include <trx/config/priv.h>
#include <trx/core/colors.h>
#include <trx/core/memory.h>
#include <trx/core/strings.h>
#include <trx/debug.h>

#include <string.h>

static VECTOR *m_DynamicOptions = nullptr;

static size_t M_GetValueSize(const CONFIG_OPTION_TYPE type)
{
    switch (type) {
    case COT_BOOL:
        return sizeof(bool);
    case COT_INT32:
    case COT_ENUM:
        return sizeof(int32_t);
    case COT_FLOAT:
    case COT_FLOAT_PERCENT:
        return sizeof(float);
    case COT_DOUBLE:
        return sizeof(double);
    case COT_RGB888:
        return sizeof(RGB_888);
    case COT_STRING:
    case COT_DYNAMIC_ENUM:
        return sizeof(char *);
    }
    return 0;
}

// COT_STRING and COT_DYNAMIC_ENUM hand their default around as a bare C string
// rather than a pointer to the stored value, matching X_CFG_STRING.
static bool M_IsStringLike(const CONFIG_OPTION_TYPE type)
{
    return type == COT_STRING || type == COT_DYNAMIC_ENUM;
}

static void M_SaveValue(CONFIG_DYNAMIC_OPTION *option);

CONFIG_DYNAMIC_OPTION *Config_AddDynamicOption(
    const char *const name, const CONFIG_OPTION_TYPE type,
    const void *const default_value)
{
    ASSERT(name != nullptr);

    if (m_DynamicOptions == nullptr) {
        m_DynamicOptions = Vector_Create(sizeof(CONFIG_DYNAMIC_OPTION));
    }

    for (int32_t i = 0; i < m_DynamicOptions->count; i++) {
        const CONFIG_DYNAMIC_OPTION *const other =
            Vector_Get(m_DynamicOptions, i);
        if (strcmp(other->name, name) == 0) {
            return nullptr;
        }
    }

    const size_t value_size = M_GetValueSize(type);
    ASSERT(value_size != 0);

    CONFIG_DYNAMIC_OPTION option = {
        .name = Memory_DupStr(name),
        .type = type,
        .target = Memory_Alloc(value_size),
        .saved_value = Memory_Alloc(value_size),
    };

    if (M_IsStringLike(type)) {
        // target and default_value are not symmetric here, and must not be made
        // so. The target is a `char *` box, the way a string field of g_Config
        // is - callers reach the value with *(char **)target. But the default
        // is the string itself: X_CFG_STRING passes a bare literal, and the
        // readers (config/file.c, Config_IsOptionAtDefault) cast default_value
        // straight to const char *. Boxing it too hands them a pointer to read
        // as text.
        //
        // The two must also not alias: restoring a default frees the target
        // first, which would take the default with it.
        option.default_value =
            default_value == nullptr ? nullptr : Memory_DupStr(default_value);
        *(char **)option.target =
            default_value == nullptr ? nullptr : Memory_DupStr(default_value);
    } else {
        option.default_value = Memory_Alloc(value_size);
        if (default_value != nullptr) {
            memcpy(option.default_value, default_value, value_size);
            memcpy(option.target, default_value, value_size);
        }
    }

    Vector_Add(m_DynamicOptions, &option);
    Config_InvalidateOptionMap();

    CONFIG_DYNAMIC_OPTION *const stored =
        Vector_Get(m_DynamicOptions, m_DynamicOptions->count - 1);
    // The option starts out unchanged, holding its default. Without this the
    // first Config_Update() would report a change that never happened.
    M_SaveValue(stored);
    return stored;
}

const VECTOR *Config_GetDynamicOptions(void)
{
    return m_DynamicOptions;
}

static bool M_ValueEquals(const CONFIG_DYNAMIC_OPTION *const option)
{
    if (M_IsStringLike(option->type)) {
        const char *const now = *(char **)option->target;
        const char *const before = *(char **)option->saved_value;
        if (now == nullptr || before == nullptr) {
            return now == before;
        }
        return strcmp(now, before) == 0;
    }
    return memcmp(
               option->target, option->saved_value,
               M_GetValueSize(option->type))
        == 0;
}

static void M_SaveValue(CONFIG_DYNAMIC_OPTION *const option)
{
    if (M_IsStringLike(option->type)) {
        Memory_FreePointer((char **)option->saved_value);
        const char *const now = *(char **)option->target;
        *(char **)option->saved_value =
            now == nullptr ? nullptr : Memory_DupStr(now);
    } else {
        memcpy(
            option->saved_value, option->target, M_GetValueSize(option->type));
    }
}

bool Config_CommitDynamicOptions(void)
{
    if (m_DynamicOptions == nullptr) {
        return false;
    }
    bool changed = false;
    for (int32_t i = 0; i < m_DynamicOptions->count; i++) {
        CONFIG_DYNAMIC_OPTION *const option = Vector_Get(m_DynamicOptions, i);
        if (!M_ValueEquals(option)) {
            M_SaveValue(option);
            changed = true;
        }
    }
    return changed;
}

static bool M_FindAnchorPriority(
    const char *const *const static_names, const int32_t static_count,
    const char *const name, int32_t *const out_priority)
{
    if (name == nullptr) {
        return false;
    }
    for (int32_t i = 0; i < static_count; i++) {
        if (static_names[i] != nullptr && strcmp(static_names[i], name) == 0) {
            *out_priority = i * CONFIG_STATIC_ROW_SPACING;
            return true;
        }
    }
    return false;
}

int32_t Config_GetDynamicRowPriority(
    const CONFIG_DYNAMIC_OPTION *const option,
    const char *const *const static_names, const int32_t static_count,
    const int32_t fallback)
{
    ASSERT(option != nullptr);
    int32_t anchor;
    if (M_FindAnchorPriority(
            static_names, static_count, option->ui.before, &anchor)) {
        return anchor - 1;
    }
    if (M_FindAnchorPriority(
            static_names, static_count, option->ui.after, &anchor)) {
        return anchor + 1;
    }
    if (option->ui.has_priority) {
        return option->ui.priority;
    }
    return fallback;
}

void Config_ClearDynamicOptions(void)
{
    if (m_DynamicOptions == nullptr) {
        return;
    }
    for (int32_t i = 0; i < m_DynamicOptions->count; i++) {
        CONFIG_DYNAMIC_OPTION *const option = Vector_Get(m_DynamicOptions, i);
        if (M_IsStringLike(option->type)) {
            // target and saved_value are boxes holding a string; the default
            // *is* the string.
            Memory_FreePointer((char **)option->target);
            Memory_FreePointer((char **)option->saved_value);
        }
        Memory_FreePointer(&option->target);
        Memory_FreePointer(&option->saved_value);
        Memory_FreePointer(&option->default_value);
        Memory_FreePointer(&option->name);
        Memory_FreePointer(&option->ui.tab);
        Memory_FreePointer(&option->ui.before);
        Memory_FreePointer(&option->ui.after);
    }
    Vector_Free(m_DynamicOptions);
    m_DynamicOptions = nullptr;
    Config_InvalidateOptionMap();
}
