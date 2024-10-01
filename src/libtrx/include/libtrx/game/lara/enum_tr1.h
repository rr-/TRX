#pragma once

// clang-format off
typedef enum {
    LF_SG_AIM_START           = 0,
    LF_SG_AIM_BEND            = 1,
    LF_SG_AIM_END             = 12,
    LF_SG_DRAW_START          = 13,
    LF_SG_DRAW_SFX            = 23,
    LF_SG_RECOIL_START        = 47,
    LF_SG_RECOILING           = 48,
    LF_SG_RECOIL_SFX          = 57,
    LF_SG_RECOIL_UNDRAW_RESET = 59,
    LF_SG_RECOIL_RESET_OG     = 60,
    LF_SG_RECOIL_RESET_FIX    = 63,
    LF_SG_RECOIL_END          = 79,
    LF_SG_UNDRAW_START        = 80,
    LF_SG_UNDRAW_SFX          = 100,
    LF_SG_UNDRAW_END          = 113,
    LF_SG_UNAIM_START         = 114,
    LF_SG_UNAIM_RAISE         = 126,
    LF_SG_UNAIM_END           = 127,
} LARA_SHOTGUN_ANIMATION_FRAME;
// clang-format on

// clang-format off
typedef enum {
    LF_G_AIM_START    = 0,
    LF_G_AIM_BEND     = 1,
    LF_G_AIM_EXTEND   = 3,
    LF_G_AIM_END      = 4,
    LF_G_UNDRAW_START = 5,
    LF_G_UNDRAW_BEND  = 6,
    LF_G_UNDRAW_END   = 12,
    LF_G_DRAW_START   = 13,
    LF_G_DRAW_END     = 23,
    LF_G_RECOIL_START = 24,
    LF_G_RECOIL_END   = 32,
} LARA_GUN_ANIMATION_FRAME;
// clang-format on

// clang-format off
typedef enum {
    LA_RUN                  = 0,
    LA_WALK_FORWARD         = 1,
    LA_RUN_START            = 6,
    LA_WALK_BACK            = 40,
    LA_VAULT_12             = 50,
    LA_VAULT_34             = 42,
    LA_FAST_FALL            = 32,
    LA_STOP                 = 11,
    LA_FALL_DOWN            = 34,
    LA_STOP_LEFT            = 2,
    LA_STOP_RIGHT           = 3,
    LA_HIT_WALL_LEFT        = 53,
    LA_HIT_WALL_RIGHT       = 54,
    LA_RUN_STEP_UP_LEFT     = 56,
    LA_RUN_STEP_UP_RIGHT    = 55,
    LA_WALK_STEP_UP_LEFT    = 57,
    LA_WALK_STEP_UP_RIGHT   = 58,
    LA_WALK_STEP_DOWN_LEFT  = 60,
    LA_WALK_STEP_DOWN_RIGHT = 59,
    LA_BACK_STEP_DOWN_LEFT  = 61,
    LA_BACK_STEP_DOWN_RIGHT = 62,
    LA_WALL_SWITCH_DOWN     = 63,
    LA_WALL_SWITCH_UP       = 64,
    LA_SIDE_STEP_LEFT       = 65,
    LA_SIDE_STEP_RIGHT      = 67,
    LA_LAND_FAR             = 24,
    LA_GRAB_LEDGE           = 96,
    LA_SWIM_GLIDE           = 87,
    LA_FALL_BACK            = 93,
    LA_HANG                 = 96,
    LA_STOP_HANG            = 28,
    LA_SLIDE                = 70,
    LA_SLIDE_BACK           = 104,
    LA_TREAD                = 108,
    LA_SURF_TREAD           = 114,
    LA_SURF_DIVE            = 119,
    LA_SURF_CLIMB           = 111,
    LA_JUMP_IN              = 112,
    LA_PUSHABLE_GRAB        = 120,
    LA_ROLL                 = 146,
    LA_PICKUP_UW            = 130,
    LA_PICKUP               = 135,
    LA_ROLLING_BALL_DEATH   = 139,
    LA_SPIKE_DEATH          = 149,
    LA_GRAB_LEDGE_IN        = 150,
    LA_SPAZ_FORWARD         = 125,
    LA_SPAZ_BACK            = 126,
    LA_SPAZ_RIGHT           = 127,
    LA_SPAZ_LEFT            = 128,
} LARA_ANIMATION;

// clang-format off
typedef enum {
    LS_WALK         = 0,
    LS_RUN          = 1,
    LS_STOP         = 2,
    LS_JUMP_FORWARD = 3,
    LS_POSE         = 4,
    LS_FAST_BACK    = 5,
    LS_TURN_R       = 6,
    LS_TURN_L       = 7,
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
    LS_CLIMB_UP     = 19,
    LS_FAST_TURN    = 20,
    LS_STEP_RIGHT   = 21,
    LS_STEP_LEFT    = 22,
    LS_HIT          = 23,
    LS_SLIDE        = 24,
    LS_JUMP_BACK    = 25,
    LS_JUMP_RIGHT   = 26,
    LS_JUMP_LEFT    = 27,
    LS_JUMP_UP      = 28,
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
    LS_CONTROLLED   = 56,
    LS_TWIST        = 57,
    LS_UW_ROLL      = 58,
} LARA_STATE;
// clang-format on

// clang-format off
typedef enum {
    LGT_UNKNOWN = -1, // for legacy saves
    LGT_UNARMED = 0,
    LGT_PISTOLS = 1,
    LGT_MAGNUMS = 2,
    LGT_UZIS = 3,
    LGT_SHOTGUN = 4,
    NUM_WEAPONS = 5
} LARA_GUN_TYPE;
// clang-format on
