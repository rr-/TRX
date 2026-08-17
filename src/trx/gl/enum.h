#pragma once

typedef enum {
    TEXTURE_FILTER_POINT,
    TEXTURE_FILTER_BILINEAR,
    TEXTURE_FILTER_NUMBER_OF,
} TEXTURE_FILTER;

// The color depth the finished picture is reduced to, each with the ordered
// dither the hardware of the day used to cover the steps.
typedef enum {
    DITHER_MODE_OFF,
    DITHER_MODE_8_BIT,
    DITHER_MODE_15_BIT,
    DITHER_MODE_NUMBER_OF,
} DITHER_MODE;
