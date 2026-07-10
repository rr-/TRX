#include <trx/game/menu/flat/logo.h>

#include <trx/config.h>
#include <trx/core/filesystem.h>
#include <trx/core/log.h>
#include <trx/core/memory.h>
#include <trx/core/strings.h>
#include <trx/game/output.h>
#include <trx/game/shell/paths.h>
#include <trx/game/viewport.h>

#include <string.h>
#include <zlib.h>

#define M_CACHE_KEY "tr4:title-logo"
#define M_LOGO_WIDTH 512
#define M_LOGO_HEIGHT 256
// Placement over the title, normalized to the UI viewport; the OG spans
// most of the top of the screen.
#define M_DRAW_WIDTH 0.7f
#define M_DRAW_Y 0.06f

static bool m_Loaded = false;
static bool m_Available = false;

static bool M_TryLoadPak(const char *const path)
{
    char *data = nullptr;
    size_t size = 0;
    if (!File_Load(path, &data, &size) || size <= 4) {
        Memory_FreePointer(&data);
        return false;
    }

    const uint32_t raw_size = (uint8_t)data[0] | ((uint8_t)data[1] << 8)
        | ((uint8_t)data[2] << 16) | ((uint32_t)(uint8_t)data[3] << 24);
    const uint32_t expected_size = M_LOGO_WIDTH * M_LOGO_HEIGHT * 3;
    if (raw_size < expected_size) {
        LOG_WARNING("Unexpected logo size in %s", path);
        Memory_FreePointer(&data);
        return false;
    }

    uint8_t *const rgb = Memory_Alloc(raw_size);
    uLongf out_size = raw_size;
    const int32_t error =
        uncompress(rgb, &out_size, (const Bytef *)data + 4, size - 4);
    Memory_FreePointer(&data);
    if (error != Z_OK || out_size < expected_size) {
        LOG_WARNING("Failed to inflate %s", path);
        Memory_Free(rgb);
        return false;
    }

    uint8_t *const rgba = Memory_Alloc(M_LOGO_WIDTH * M_LOGO_HEIGHT * 4);
    for (int32_t i = 0; i < M_LOGO_WIDTH * M_LOGO_HEIGHT; i++) {
        const uint8_t r = rgb[i * 3 + 0];
        const uint8_t g = rgb[i * 3 + 1];
        const uint8_t b = rgb[i * 3 + 2];
        rgba[i * 4 + 0] = r;
        rgba[i * 4 + 1] = g;
        rgba[i * 4 + 2] = b;
        // The OG keys pure black out.
        rgba[i * 4 + 3] = (r == 0 && g == 0 && b == 0) ? 0 : 255;
    }
    Memory_Free(rgb);

    const bool ok = Output_Overlay_LoadImageFromMemory(
        M_CACHE_KEY, rgba, M_LOGO_WIDTH, M_LOGO_HEIGHT);
    Memory_Free(rgba);
    return ok;
}

bool InvFlatLogo_Load(void)
{
    if (m_Loaded) {
        return m_Available;
    }
    m_Loaded = true;

    // Language-specific logo first, then the OG fallback order.
    const char *candidates[4] = {};
    int32_t candidate_count = 0;
    if (String_Equivalent(g_Config.language, "de")) {
        candidates[candidate_count++] = "grlogo.pak";
    } else if (String_Equivalent(g_Config.language, "fr")) {
        candidates[candidate_count++] = "frlogo.pak";
    }
    candidates[candidate_count++] = "uklogo.pak";
    candidates[candidate_count++] = "uslogo.pak";

    for (int32_t i = 0; i < candidate_count; i++) {
        // The logo paks live in the game data directory.
        const char *const path = TRXPath_PeekResolve(
            TRX_DYNAMIC_PATH_CATALOG,
            String_FormatStatic("data/%s", candidates[i]));
        if (path != nullptr && M_TryLoadPak(path)) {
            m_Available = true;
            break;
        }
    }
    return m_Available;
}

void InvFlatLogo_Draw(void)
{
    if (!InvFlatLogo_Load()) {
        return;
    }
    const float vp_w = Viewport_GetWidth(VIEWPORT_UI);
    const float vp_h = Viewport_GetHeight(VIEWPORT_UI);
    if (vp_w <= 0.0f || vp_h <= 0.0f) {
        return;
    }
    const float h =
        M_DRAW_WIDTH * vp_w * ((float)M_LOGO_HEIGHT / M_LOGO_WIDTH) / vp_h;
    Output_Overlay_DrawImageRect(
        M_CACHE_KEY, (1.0f - M_DRAW_WIDTH) / 2.0f, M_DRAW_Y, M_DRAW_WIDTH, h);
}
