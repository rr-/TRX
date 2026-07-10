#pragma once

#include <stdint.h>

// Credit roll requested by the gameflow credit_roll sequence event. The
// roll itself plays over the title menu (like the OG), so the event only
// records the request and the title strategy consumes it.

void CreditRoll_SetPending(const char *strings_key, int32_t music_track);
bool CreditRoll_IsPending(void);
// Claims the pending request; valid until the next CreditRoll_SetPending.
const char *CreditRoll_TakePending(int32_t *out_music_track);

// The roll widget: centered lines scrolling up 1 canvas pixel per logic
// frame, headings (lines starting with #) drawn with the HEADING text
// role and names with NORMAL. Strategy-independent; any title menu can
// host it.
typedef struct CREDIT_ROLL CREDIT_ROLL;

CREDIT_ROLL *CreditRoll_Create(const char *text);
void CreditRoll_Control(CREDIT_ROLL *roll);
void CreditRoll_Draw(CREDIT_ROLL *roll);
bool CreditRoll_IsDone(const CREDIT_ROLL *roll);
void CreditRoll_Free(CREDIT_ROLL *roll);
