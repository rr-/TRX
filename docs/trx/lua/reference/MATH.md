---
title: Math
order: 17
---

<!--
  GENERATED FILE - do not edit.
  Regenerate with: just lua-api-dump
  The public API is declared next to its implementation, in
  data/scripting/math.lua. Edit it there.
-->

## Math module

Fixed-point trigonometry, matching the engine's own tables.

TRX angles are 16-bit units where 65536 is a full turn, not radians. Using these rather than Lua's `math` library guarantees a script places things exactly where the engine would.

### Functions

- [lua]`trx.math.sin(angle)`  
  Sine of an angle.

  Parameters:
  - **`angle`** (integer). Angle in TRX units.

  Returns: number. A value in [-1, 1].

- [lua]`trx.math.cos(angle)`  
  Cosine of an angle.

  Parameters:
  - **`angle`** (integer). Angle in TRX units.

  Returns: number. A value in [-1, 1].

- [lua]`trx.math.atan(z, x)`  
  Angle of the vector (x, z), in TRX units.

  Parameters:
  - **`z`** (integer).
  - **`x`** (integer).

  Returns: integer.

  Example:
  ```lua
  -- face an item towards Lara
  local angle = trx.math.atan(lara.pos.z - pos.z, lara.pos.x - pos.x)
  ```
