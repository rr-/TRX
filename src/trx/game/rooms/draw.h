#pragma once

#include <stdint.h>

void Room_DrawReset(void);
void Room_MarkToBeDrawn(int16_t room_num);
int32_t Room_DrawGetCount(void);
int16_t Room_DrawGetRoom(int16_t idx);

void Room_DrawAllRooms(int16_t base_room, int16_t target_room);

// Controls whether the draw leaves out Lara, the creatures and the effects,
// so that a scripted viewpoint shows the level's scenery alone. The TR4
// loading screens draw this way.
void Room_SetSceneryOnly(bool enabled);

// Whether the last drawn frame had any outside-flagged room in view (i.e.
// the sky was visible), matching the OG engines' "outside" draw flag.
bool Room_IsSkyVisible(void);

// Manage per-room draw queue of items
void Room_AddDrawnItem(int16_t room_num, int16_t item_num);
void Room_RemoveDrawnItem(int16_t room_num, int16_t item_num);
