#pragma once

// clang-format off
typedef enum {
    LA_RUN                                   = 0,
    LA_WALK_FORWARD                          = 1,
    LA_WALK_STOP_RIGHT                       = 2,
    LA_WALK_STOP_LEFT                        = 3,
    LA_WALK_TO_RUN_RIGHT                     = 4,
    LA_WALK_TO_RUN_LEFT                      = 5,
    LA_RUN_START                             = 6,
    LA_RUN_TO_WALK_RIGHT                     = 7,
    LA_RUN_TO_STAND_LEFT                     = 8,
    LA_RUN_TO_WALK_LEFT                      = 9,
    LA_RUN_TO_STAND_RIGHT                    = 10,
    LA_STAND_STILL                           = 11,
    LA_TURN_RIGHT_SLOW                       = 12,
    LA_TURN_LEFT_SLOW                        = 13,
    LA_JUMP_FORWARD_LAND_START               = 14,
    LA_JUMP_FORWARD_LAND_END_UNUSED          = 15,
    LA_RUN_JUMP_RIGHT_START                  = 16,
    LA_RUN_JUMP_RIGHT_CONTINUE               = 17,
    LA_RUN_JUMP_LEFT_START                   = 18,
    LA_RUN_JUMP_LEFT_CONTINUE                = 19,
    LA_WALK_FORWARD_START                    = 20,
    LA_WALK_FORWARD_START_CONTINUE           = 21,
    LA_JUMP_FORWARD_TO_FREEFALL              = 22,
    LA_FREEFALL                              = 23,
    LA_FREEFALL_LAND                         = 24,
    LA_FREEFALL_LAND_DEATH                   = 25,
    LA_STAND_TO_JUMP_UP                      = 26,
    LA_STAND_TO_JUMP_UP_CONTINUE             = 27,
    LA_JUMP_UP                               = 28,
    LA_JUMP_UP_TO_HANG                       = 29,
    LA_JUMP_UP_TO_FREEFALL                   = 30,
    LA_JUMP_UP_LAND                          = 31,
    LA_SMASH_JUMP                            = 32,
    LA_SMASH_JUMP_CONTINUE                   = 33,
    LA_FALL_START                            = 34,
    LA_FALL                                  = 35,
    LA_FALL_TO_FREEFALL                      = 36,
    LA_HANG_TO_FREEFALL                      = 37,
    LA_WALK_BACK_END_RIGHT                   = 38,
    LA_WALK_BACK_END_LEFT                    = 39,
    LA_WALK_BACK                             = 40,
    LA_WALK_BACK_START                       = 41,
    LA_CLIMB_3CLICK                          = 42,
    LA_CLIMB_3CLICK_END_TO_RUN               = 43,
    LA_TURN_RIGHT                            = 44,
    LA_JUMP_FORWARD_TO_FREEFALL_2            = 45,
    LA_REACH_TO_FREEFALL                     = 46,
    LA_ROLL_ALTERNATE                        = 47,
    LA_ROLL_END_ALTERNATE                    = 48,
    LA_JUMP_FORWARD_END_TO_FREEFALL          = 49,
    LA_CLIMB_2CLICK                          = 50,
    LA_CLIMB_2CLICK_END                      = 51,
    LA_CLIMB_2CLICK_END_TO_RUN               = 52,
    LA_WALL_SMASH_LEFT                       = 53,
    LA_WALL_SMASH_RIGHT                      = 54,
    LA_RUN_UP_STEP_RIGHT                     = 55,
    LA_RUN_UP_STEP_LEFT                      = 56,
    LA_WALK_UP_STEP_RIGHT                    = 57,
    LA_WALK_UP_STEP_LEFT                     = 58,
    LA_WALK_DOWN_LEFT                        = 59,
    LA_WALK_DOWN_RIGHT                       = 60,
    LA_WALK_DOWN_BACK_LEFT                   = 61,
    LA_WALK_DOWN_BACK_RIGHT                  = 62,
    LA_WALLSWITCH_DOWN                       = 63,
    LA_WALLSWITCH_UP                         = 64,
    LA_SIDESTEP_LEFT                         = 65,
    LA_SIDESTEP_LEFT_END                     = 66,
    LA_SIDESTEP_RIGHT                        = 67,
    LA_SIDESTEP_RIGHT_END                    = 68,
    LA_ROTATE_LEFT                           = 69,
    LA_SLIDE_FORWARD                         = 70,
    LA_SLIDE_FORWARD_END                     = 71,
    LA_SLIDE_FORWARD_STOP                    = 72,
    LA_STAND_TO_JUMP                         = 73,
    LA_JUMP_BACK_START                       = 74,
    LA_JUMP_BACK                             = 75,
    LA_JUMP_FORWARD_START                    = 76,
    LA_JUMP_FORWARD                          = 77,
    LA_JUMP_LEFT_START                       = 78,
    LA_JUMP_LEFT                             = 79,
    LA_JUMP_RIGHT_START                      = 80,
    LA_JUMP_RIGHT                            = 81,
    LA_LAND                                  = 82,
    LA_JUMP_BACK_TO_FREEFALL                 = 83,
    LA_JUMP_LEFT_TO_FREEFALL                 = 84,
    LA_JUMP_RIGHT_TO_FREEFALL                = 85,
    LA_UNDERWATER_SWIM_FORWARD               = 86,
    LA_UNDERWATER_SWIM_FORWARD_DRIFT         = 87,
    LA_SMALL_JUMP_BACK_START                 = 88,
    LA_SMALL_JUMP_BACK                       = 89,
    LA_SMALL_JUMP_BACK_END                   = 90,
    LA_JUMP_UP_START                         = 91,
    LA_LAND_TO_RUN                           = 92,
    LA_FALL_BACK                             = 93,
    LA_JUMP_FORWARD_TO_REACH                 = 94,
    LA_REACH                                 = 95,
    LA_REACH_TO_HANG                         = 96,
    LA_CLIMB_ON                              = 97,
    LA_REACH_TO_FREEFALL_2                   = 98,
    LA_FALL_CROUCHING_LANDING                = 99,
    LA_JUMP_FORWARD_TO_REACH_LATE            = 100,
    LA_JUMP_FORWARD_START_TO_REACH_ALTERNATE = 101,
    LA_CLIMB_ON_END                          = 102,
    LA_STAND_IDLE                            = 103,
    LA_SLIDE_BACKWARD_START                  = 104,
    LA_SLIDE_BACKWARD                        = 105,
    LA_SLIDE_BACKWARD_END                    = 106,
    LA_UNDERWATER_SWIM_TO_IDLE               = 107,
    LA_UNDERWATER_IDLE                       = 108,
    LA_UNDERWARER_IDLE_TO_SWIM               = 109,
    LA_ONWATER_IDLE                          = 110,
    LA_ONWATER_TO_STAND_HIGH                 = 111,
    LA_FREEFALL_TO_UNDERWATER                = 112,
    LA_ONWATER_DIVE_ALTERNATE                = 113,
    LA_UNDERWATER_TO_ONWATER                 = 114,
    LA_ONWATER_SWIM_FORWARD_DIVE             = 115,
    LA_ONWATER_SWIM_FORWARD                  = 116,
    LA_ONWATER_SWIM_FORWARD_TO_IDLE          = 117,
    LA_ONWATER_IDLE_TO_SWIM_FORWARD          = 118,
    LA_ONWATER_DIVE                          = 119,
    LA_PUSHABLE_GRAB                         = 120,
    LA_PUSHABLE_RELEASE                      = 121,
    LA_PUSHABLE_PULL                         = 122,
    LA_PUSHABLE_PUSH                         = 123,
    LA_UNDERWATER_DEATH                      = 124,
    LA_HIT_FRONT                             = 125,
    LA_HIT_BACK                              = 126,
    LA_HIT_LEFT                              = 127,
    LA_HIT_RIGHT                             = 128,
    LA_UNDERWATER_SWITCH                     = 129,
    LA_UNDERWATER_PICKUP                     = 130,
    LA_USE_KEY                               = 131,
    LA_ONWATER_DEATH                         = 132,
    LA_RUN_DEATH                             = 133,
    LA_USE_PUZZLE                            = 134,
    LA_PICKUP                                = 135,
    LA_SHIMMY_LEFT                           = 136,
    LA_SHIMMY_RIGHT                          = 137,
    LA_STAND_DEATH                           = 138,
    LA_BOULDER_DEATH                         = 139,
    LA_ONWATER_IDLE_TO_SWIM_BACK             = 140,
    LA_ONWATER_SWIM_BACK                     = 141,
    LA_ONWATER_SWIM_BACK_TO_IDLE             = 142,
    LA_ONWATER_SWIM_LEFT                     = 143,
    LA_ONWATER_SWIM_RIGHT                    = 144,
    LA_DEATH_JUMP                            = 145,
    LA_ROLL_START                            = 146,
    LA_ROLL_CONTINUE                         = 147,
    LA_ROLL_END                              = 148,
    LA_SPIKE_DEATH                           = 149,
    LA_REACH_TO_THIN_LEDGE                   = 150,
    LA_SWANDIVE_ROLL                         = 151,
    LA_SWANDIVE_TO_UNDERWATER                = 152,
    LA_FREEFALL_SWANDIVE                     = 153,
    LA_FREEFALL_SWANDIVE_TO_UNDERWATER       = 154,
    LA_SWANDIVE_DEATH                        = 155,
    LA_SWANDIVE_LEFT                         = 156,
    LA_SWANDIVE_RIGHT                        = 157,
    LA_SWANDIVE_START                        = 158,
    LA_CLIMB_ON_HANDSTAND                    = 159,
    LA_STAND_TO_LADDER                       = 160,
    LA_LADDER_UP                             = 161,
    LA_LADDER_UP_STOP_RIGHT                  = 162,
    LA_LADDER_UP_STOP_LEFT                   = 163,
    LA_LADDER_IDLE                           = 164,
    LA_LADDER_UP_START                       = 165,
    LA_LADDER_DOWN_STOP_LEFT                 = 166,
    LA_LADDER_DOWN_STOP_RIGHT                = 167,
    LA_LADDER_DOWN                           = 168,
    LA_LADDER_DOWN_START                     = 169,
    LA_LADDER_RIGHT                          = 170,
    LA_LADDER_LEFT                           = 171,
    LA_LADDER_HANG                           = 172,
    LA_LADDER_HANG_TO_IDLE                   = 173,
    LA_LADDER_CLIMB_ON                       = 174,
    LA_UNKNOWN                               = 175,
    LA_ONWATER_TO_WADE_SHALLOW               = 176,
    LA_WADE                                  = 177,
    LA_RUN_TO_WADE_LEFT                      = 178,
    LA_RUN_TO_WADE_RIGHT                     = 179,
    LA_WADE_TO_RUN_LEFT                      = 180,
    LA_WADE_TO_RUN_RIGHT                     = 181,
    LA_LADDER_BACKFLIP_START                 = 182,
    LA_LADDER_BACKFLIP_CONTINUE              = 183,
    LA_WADE_TO_STAND_RIGHT                   = 184,
    LA_WADE_TO_STAND_LEFT                    = 185,
    LA_STAND_TO_WADE                         = 186,
    LA_LADDER_UP_HANGING                     = 187,
    LA_LADDER_DOWN_HANGING                   = 188,
    LA_FLARE_THROW                           = 189,
    LA_ONWATER_TO_WADE                       = 190,
    LA_ONWATER_TO_STAND_MEDIUM               = 191,
    LA_UNDERWATER_TO_STAND                   = 192,
    LA_ONWATER_TO_WADE_LOW                   = 193,
    LA_LADDER_TO_HANG_DOWN                   = 194,
    LA_SWITCH_SMALL_DOWN                     = 195,
    LA_SWITCH_SMALL_UP                       = 196,
    LA_BUTTON_PUSH                           = 197,
    LA_UNDERWATER_SWIM_TO_STILL_HUDDLE       = 198,
    LA_UNDERWATER_SWIM_TO_STILL_SPRAWL       = 199,
    LA_UNDERWATER_SWIM_TO_STILL_MEDIUM       = 200,
    LA_LADDER_TO_HANG_RIGHT                  = 201,
    LA_LADDER_TO_HANG_LEFT                   = 202,
    LA_UNDERWATER_ROLL_START                 = 203,
    LA_FLARE_PICKUP                          = 204,
    LA_UNDERWATER_ROLL_END                   = 205,
    LA_UNDERWATER_FLARE_PICKUP               = 206,
    LA_RUN_JUMP_ROLL_START                   = 207,
    LA_SOMERSAULT                            = 208,
    LA_RUN_JUMP_ROLL_END                     = 209,
    LA_JUMP_FORWARD_ROLL_START               = 210,
    LA_JUMP_FORWARD_ROLL_END                 = 211,
    LA_JUMP_BACK_ROLL_START                  = 212,
    LA_JUMP_BACK_ROLL_END                    = 213,
    LA_KICK                                  = 214,
    LA_ZIPLINE_GRAB                          = 215,
    LA_ZIPLINE_RIDE                          = 216,
    LA_ZIPLINE_FALL                          = 217,
} LARA_ANIMATION;
// clang-format on

// clang-format off
typedef enum {
    LA_EXTRA_BREATH      = 0,
    LA_EXTRA_PLUNGER     = 1,
    LA_EXTRA_YETI_KILL   = 2,
    LA_EXTRA_SHARK_KILL  = 3,
    LA_EXTRA_AIRLOCK     = 4,
    LA_EXTRA_GONG_BONG   = 5,
    LA_EXTRA_DINO_KILL   = 6,
    LA_EXTRA_PULL_DAGGER = 7,
    LA_EXTRA_START_ANIM  = 8,
    LA_EXTRA_START_HOUSE = 9,
    LA_EXTRA_FINAL_ANIM  = 10,
} LARA_EXTRA_ANIMATION;
// clang-format on

// clang-format off
typedef enum {
    LS_WALK         = 0,
    LS_RUN          = 1,
    LS_STOP         = 2,
    LS_FORWARD_JUMP = 3,
    LS_POSE         = 4,
    LS_FAST_BACK    = 5,
    LS_TURN_RIGHT   = 6,
    LS_TURN_LEFT    = 7,
    LS_DEATH        = 8,
    LS_FAST_FALL    = 9,
    LS_HANG         = 10,
    LS_REACH        = 11,
    LS_SPLAT        = 12,
    LS_TREAD        = 13,
    LS_LAND         = 14,
    LS_COMPRESS     = 15,
    LS_BACK         = 16,
    LS_SWIM         = 17,
    LS_GLIDE        = 18,
    LS_NULL         = 19,
    LS_FAST_TURN    = 20,
    LS_STEP_RIGHT   = 21,
    LS_STEP_LEFT    = 22,
    LS_HIT          = 23,
    LS_SLIDE        = 24,
    LS_BACK_JUMP    = 25,
    LS_RIGHT_JUMP   = 26,
    LS_LEFT_JUMP    = 27,
    LS_UP_JUMP      = 28,
    LS_FALL_BACK    = 29,
    LS_HANG_LEFT    = 30,
    LS_HANG_RIGHT   = 31,
    LS_SLIDE_BACK   = 32,
    LS_SURF_TREAD   = 33,
    LS_SURF_SWIM    = 34,
    LS_DIVE         = 35,
    LS_PUSH_BLOCK   = 36,
    LS_PULL_BLOCK   = 37,
    LS_PP_READY     = 38,
    LS_PICKUP       = 39,
    LS_SWITCH_ON    = 40,
    LS_SWITCH_OFF   = 41,
    LS_USE_KEY      = 42,
    LS_USE_PUZZLE   = 43,
    LS_UW_DEATH     = 44,
    LS_ROLL         = 45,
    LS_SPECIAL      = 46,
    LS_SURF_BACK    = 47,
    LS_SURF_LEFT    = 48,
    LS_SURF_RIGHT   = 49,
    LS_USE_MIDAS    = 50,
    LS_DIE_MIDAS    = 51,
    LS_SWAN_DIVE    = 52,
    LS_FAST_DIVE    = 53,
    LS_GYMNAST      = 54,
    LS_WATER_OUT    = 55,
    LS_CLIMB_STANCE = 56,
    LS_CLIMBING     = 57,
    LS_CLIMB_LEFT   = 58,
    LS_CLIMB_END    = 59,
    LS_CLIMB_RIGHT  = 60,
    LS_CLIMB_DOWN   = 61,
    LS_LARA_TEST1   = 62,
    LS_LARA_TEST2   = 63,
    LS_LARA_TEST3   = 64,
    LS_WADE         = 65,
    LS_WATER_ROLL   = 66,
    LS_FLARE_PICKUP = 67,
    LS_TWIST        = 68,
    LS_KICK         = 69,
    LS_DEATH_SLIDE  = 70,
    LS_DUCK         = 71,
    LS_DUCK_ROLL    = 72,
    LS_DASH         = 73,
    LS_DASH_DIVE    = 74,
    LS_MONKEY_SWING = 75,
    LS_MONKEYF      = 76,
    LS_LAST         = 77,
} LARA_STATE;
// clang-format on

// clang-format off
typedef enum {
    LGT_UNARMED = 0,
    LGT_PISTOLS = 1,
    LGT_MAGNUMS = 2,
    LGT_UZIS    = 3,
    LGT_SHOTGUN = 4,
    LGT_M16     = 5,
    LGT_GRENADE = 6,
    LGT_HARPOON = 7,
    LGT_FLARE   = 8 ,
    LGT_SKIDOO  = 9,
    NUM_WEAPONS = 10,
} LARA_GUN_TYPE;
// clang-format on
