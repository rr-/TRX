-- TR1's own settings. Loaded before the config is read, so options declared
-- here are already in the map when the player's saved values land.

-- Water tint. TombATI and the DOS release picked different colors, and people
-- ask for both; Custom hands the choice back to the player.
local WATER_COLORS = {
  tombati = "72FFFF",
  dos = "99B2FF",
}

trx.config.declare({
  key = "visuals.water_color_mode",
  type = "enum",
  values = { "tombati", "dos", "custom" },
  default = "custom",
  tab = "graphic_visuals",
  before = "visuals.water_color",
})

trx.config.on_change("visuals.water_color_mode", function(mode)
  -- The picker below is only the player's to touch in Custom mode; in the two
  -- preset modes we own that value, so grey it out rather than let them edit
  -- something we are about to overwrite.
  trx.config.set_enabled("visuals.water_color", mode == "custom")

  local color = WATER_COLORS[mode]
  if color ~= nil then
    trx.config.set("visuals.water_color", color)
  end
end)
