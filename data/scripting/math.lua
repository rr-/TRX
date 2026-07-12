local raw = trxc.math
local api = trx.api

api.module("math", {
  order = 17,
  description = "Fixed-point trigonometry, matching the engine's own tables.\n\n"
    .. "TRX angles are 16-bit units where 65536 is a full turn, not radians. Using these rather "
    .. "than Lua's `math` library guarantees a script places things exactly where the engine "
    .. "would.",
})

api.define("math.sin", {
  description = "Sine of an angle.",
  params = { { name = "angle", type = "integer", description = "Angle in TRX units." } },
  returns = { type = "number", description = "A value in [-1, 1]." },
  impl = raw.sin,
})

api.define("math.cos", {
  description = "Cosine of an angle.",
  params = { { name = "angle", type = "integer", description = "Angle in TRX units." } },
  returns = { type = "number", description = "A value in [-1, 1]." },
  impl = raw.cos,
})

api.define("math.atan", {
  description = "Angle of the vector (x, z), in TRX units.",
  params = {
    { name = "z", type = "integer" },
    { name = "x", type = "integer" },
  },
  returns = { type = "integer" },
  examples = {
    [[-- face an item towards Lara
local angle = trx.math.atan(lara.pos.z - pos.z, lara.pos.x - pos.x)]],
  },
  impl = raw.atan,
})

-- Constants, not functions: exposed directly on the module table.
trx.math.DEG_1 = raw.DEG_1
trx.math.DEG_45 = raw.DEG_45
trx.math.DEG_90 = raw.DEG_90
trx.math.WALL_L = raw.WALL_L
