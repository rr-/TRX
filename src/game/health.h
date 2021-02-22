#ifndef TOMB1MAIN_GAME_HEALTH_H
#define TOMB1MAIN_GAME_HEALTH_H

#include <stdint.h>

void __cdecl MakeAmmoString(char* string);
void __cdecl DrawAmmoInfo();
void __cdecl DrawGameInfo();
void __cdecl DrawHealthBar();
void __cdecl DrawAirBar();
void __cdecl AddDisplayPickup(int16_t objnum);
void __cdecl InitialisePickUpDisplay();
void __cdecl DrawPickups();

void Tomb1MInjectGameHealth();

#endif
