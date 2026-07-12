local raw = trxc.config
local api = trx.api

api.module("config", {
  order = 8,
  description = "Module for reading, changing and declaring configuration options.\n\n"
    .. "A game can declare its own options here rather than in the engine: `config.declare` "
    .. "gives one storage, saves and loads it with the player's settings, and renders it in "
    .. "the settings menu. Options declared from a game's `scripts/_game.lua` exist only for "
    .. "that game, so nothing has to be hidden from the others.",
})

api.define("config.get", {
  description = "Reads an option's current value, as a string.",
  params = { { name = "key", type = "string", description = "Option name, e.g. `visuals.water_color`." } },
  returns = { type = "string" },
  impl = raw.get,
})

api.define("config.set", {
  description = "Changes an option's value. The value is parsed according to the option's type, "
    .. 'so a boolean takes `"true"` and a colour takes `"72FFFF"`.',
  params = {
    { name = "key", type = "string", description = "Option name." },
    { name = "value", type = "string", description = "New value." },
  },
  examples = {
    [[trx.config.set("visuals.water_color", "99B2FF")]],
  },
  impl = raw.set,
})

api.define("config.list", {
  description = "Returns every option available in the current game, as a table of name to value.",
  returns = { type = "table" },
  impl = raw.list,
})

api.define("config.declare", {
  description = "Declares a new configuration option.\n\n"
    .. "`spec.key` names it and `spec.type` is one of `bool`, `int` or `enum`. `spec.default` is "
    .. "the value it holds until the player changes it; an `enum` lists its `spec.values`, and an "
    .. "`int` gives its `spec.min` and `spec.max`.\n\n"
    .. "`spec.tab` puts it on a settings tab, e.g. `graphic_visuals`. To place the row, name "
    .. "another option in `spec.before` or `spec.after` - steadier than a number when the tab is "
    .. "reordered - or fall back to an explicit `spec.priority`.\n\n"
    .. "The declaration carries no text. The engine derives `settings/<key>/title`, "
    .. "`settings/<key>/description` and, for an enum, `settings/<key>/values/<value>`, and looks "
    .. "each up in the game strings - so a declared option is translated like any other setting. "
    .. "Write them in your game's `strings.json5`, where translators already work; `just lint` "
    .. "fails if one is missing.",
  params = { { name = "spec", type = "table", description = "The option's declaration." } },
  examples = {
    [[trx.config.declare({
  key = "visuals.water_color_mode",
  type = "enum",
  values = { "tombati", "dos", "custom" },
  default = "custom",
  tab = "graphic_visuals",
  before = "visuals.water_color",
})]],
  },
  impl = raw.declare,
})

api.define("config.set_enabled", {
  description = "Greys an option out in the settings menu: it stays visible, but is dimmed and "
    .. "cannot be changed. For one option gating another - a colour picker that only applies "
    .. "while its mode is set to `custom`, say.",
  params = {
    { name = "key", type = "string", description = "Option name." },
    { name = "enabled", type = "boolean", description = "`false` to grey the option out." },
  },
  impl = raw.set_enabled,
})

-- Per-key change watchers. The engine only reports that *something* changed, so
-- the routing lives here: remember each watched key's last value, and call back
-- only for the ones that actually moved.
local watchers = {}

local function dispatch()
  for _, watcher in ipairs(watchers) do
    local value = raw.get(watcher.key)
    if value ~= watcher.last then
      watcher.last = value
      watcher.fn(value)
    end
  end
end

api.define("config.on_change", {
  description = "Calls `fn(value)` whenever the option changes, and once at startup with the "
    .. "value loaded from the player's config - so a saved setting is applied on boot, not only "
    .. "when the player next touches it.",
  params = {
    { name = "key", type = "string", description = "Option name to watch." },
    { name = "fn", type = "function", description = "Called with the option's new value." },
  },
  examples = {
    [[trx.config.on_change("visuals.water_color_mode", function(mode)
  trx.config.set_enabled("visuals.water_color", mode == "custom")
end)]],
  },
  impl = function(key, fn)
    watchers[#watchers + 1] = { key = key, fn = fn, last = nil }
  end,
})

raw.on_change(dispatch)
