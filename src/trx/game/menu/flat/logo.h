#pragma once

// TR4 title logo, read from the OG data/{us,uk,gr,fr}logo.pak next to the
// title level (4-byte size header + zlib stream of a 512x256 24-bit image,
// pure black = transparent).

// Idempotent; returns whether a logo is available.
bool InvFlatLogo_Load(void);
void InvFlatLogo_Draw(void);
