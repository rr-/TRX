#include <trx/game/menu/credit_roll.h>

#include <trx/config.h>
#include <trx/core/memory.h>
#include <trx/core/utils.h>
#include <trx/core/vector.h>
#include <trx/game/interpolation.h>
#include <trx/game/ui/common.h>
#include <trx/game/ui/text.h>

#include <string.h>

// The OG scrolls one screen pixel per frame with ~36 px line spacing;
// expressed in canvas units this keeps a similar per-line pace.
#define M_SCROLL_SPEED 0.5f

typedef struct {
    char *text; // nullptr = blank spacer line
    bool heading;
} M_LINE;

struct CREDIT_ROLL {
    VECTOR *lines; // of M_LINE
    float scroll_pos;
    float prev_scroll_pos;
    float line_height;
};

static char *m_PendingStringsKey = nullptr;
static int32_t m_PendingMusicTrack = -1;
static bool m_Pending = false;

void CreditRoll_SetPending(
    const char *const strings_key, const int32_t music_track)
{
    Memory_FreePointer(&m_PendingStringsKey);
    m_PendingStringsKey = Memory_DupStr(strings_key);
    m_PendingMusicTrack = music_track;
    m_Pending = true;
}

bool CreditRoll_IsPending(void)
{
    return m_Pending;
}

const char *CreditRoll_TakePending(int32_t *const out_music_track)
{
    if (!m_Pending) {
        return nullptr;
    }
    m_Pending = false;
    if (out_music_track != nullptr) {
        *out_music_track = m_PendingMusicTrack;
    }
    return m_PendingStringsKey;
}

CREDIT_ROLL *CreditRoll_Create(const char *const text)
{
    if (text == nullptr) {
        return nullptr;
    }

    CREDIT_ROLL *const roll = Memory_Alloc(sizeof(CREDIT_ROLL));
    roll->lines = Vector_Create(sizeof(M_LINE));

    const char *line_start = text;
    while (true) {
        const char *const line_end = strchr(line_start, '\n');
        const size_t len = line_end != nullptr ? (size_t)(line_end - line_start)
                                               : strlen(line_start);

        M_LINE line = {};
        if (len > 0) {
            line.heading = line_start[0] == '#';
            const char *content = line_start + (line.heading ? 1 : 0);
            size_t content_len = len - (line.heading ? 1 : 0);
            while (content_len > 0 && *content == ' ') {
                content++;
                content_len--;
            }
            if (content_len > 0) {
                line.text = Memory_Alloc(content_len + 1);
                memcpy(line.text, content, content_len);
            }
        }
        Vector_Add(roll->lines, &line);

        if (line_end == nullptr) {
            break;
        }
        line_start = line_end + 1;
    }

    roll->line_height = UI_TEXT_HEIGHT * g_Config.ui.text_scale;
    roll->scroll_pos = UI_GetCanvasHeight() + roll->line_height;
    roll->prev_scroll_pos = roll->scroll_pos;
    return roll;
}

void CreditRoll_Control(CREDIT_ROLL *const roll)
{
    roll->prev_scroll_pos = roll->scroll_pos;
    roll->scroll_pos -= M_SCROLL_SPEED;
}

void CreditRoll_Draw(CREDIT_ROLL *const roll)
{
    const float canvas_w = UI_GetCanvasWidth();
    const float canvas_h = UI_GetCanvasHeight();
    const float base_y =
        LERP(roll->prev_scroll_pos, roll->scroll_pos, Interpolation_GetRate());

    for (int32_t i = 0; i < roll->lines->count; i++) {
        const M_LINE *const line = Vector_Get(roll->lines, i);
        if (line->text == nullptr) {
            continue;
        }
        const float y = base_y + i * roll->line_height;
        if (y < -roll->line_height || y > canvas_h) {
            continue;
        }

        const UI_TEXT_SETTINGS settings = {
            .scale = 1.0f,
            .role = line->heading ? UI_TEXT_ROLE_HEADING : UI_TEXT_ROLE_NORMAL,
        };
        float w = 0.0f;
        UI_Text_Measure(line->text, &w, nullptr, settings);
        UI_Text_Draw(line->text, (canvas_w - w) / 2.0f, y, settings);
    }
}

bool CreditRoll_IsDone(const CREDIT_ROLL *const roll)
{
    return roll->scroll_pos + roll->lines->count * roll->line_height < 0.0f;
}

void CreditRoll_Free(CREDIT_ROLL *const roll)
{
    if (roll == nullptr) {
        return;
    }
    for (int32_t i = 0; i < roll->lines->count; i++) {
        M_LINE *const line = Vector_Get(roll->lines, i);
        Memory_FreePointer(&line->text);
    }
    Vector_Free(roll->lines);
    Memory_Free(roll);
}
