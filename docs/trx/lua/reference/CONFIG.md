---
title: Config
order: 8
---

<!--
  GENERATED FILE - do not edit.
  Regenerate with: just lua-api-dump
  The public API is declared next to its implementation, in
  data/scripting/config.lua. Edit it there.
-->

## Config module

Module for reading, changing and declaring configuration options.

A game can declare its own options here rather than in the engine: `config.declare` gives one storage, saves and loads it with the player's settings, and renders it in the settings menu. Options declared from a game's `scripts/_game.lua` exist only for that game, so nothing has to be hidden from the others.

### Functions

- [lua]`trx.config.get(key)`  
  Reads an option's current value, as a string.

  Parameters:
  - **`key`** (string). Option name, e.g. `visuals.water_color`.

  Returns: string.

- [lua]`trx.config.set(key, value)`  
  Changes an option's value. The value is parsed according to the option's type, so a boolean takes `"true"` and a colour takes `"72FFFF"`.

  Parameters:
  - **`key`** (string). Option name.
  - **`value`** (string). New value.

  Example:
  ```lua
  trx.config.set("visuals.water_color", "99B2FF")
  ```

- [lua]`trx.config.list()`  
  Returns every option available in the current game, as a table of name to value.

  Returns: table.

- [lua]`trx.config.declare(spec)`  
  Declares a new configuration option.

`spec.key` names it and `spec.type` is one of `bool`, `int` or `enum`. `spec.default` is the value it holds until the player changes it; an `enum` lists its `spec.values`, and an `int` gives its `spec.min` and `spec.max`.

`spec.tab` puts it on a settings tab, e.g. `graphic_visuals`. To place the row, name another option in `spec.before` or `spec.after` - steadier than a number when the tab is reordered - or fall back to an explicit `spec.priority`.

The declaration carries no text. The engine derives `settings/<key>/title`, `settings/<key>/description` and, for an enum, `settings/<key>/values/<value>`, and looks each up in the game strings - so a declared option is translated like any other setting. Write them in your game's `strings.json5`, where translators already work; `just lint` fails if one is missing.

  Parameters:
  - **`spec`** (table). The option's declaration.

  Example:
  ```lua
  trx.config.declare({
    key = "visuals.water_color_mode",
    type = "enum",
    values = { "tombati", "dos", "custom" },
    default = "custom",
    tab = "graphic_visuals",
    before = "visuals.water_color",
  })
  ```

- [lua]`trx.config.set_enabled(key, enabled)`  
  Greys an option out in the settings menu: it stays visible, but is dimmed and cannot be changed. For one option gating another - a colour picker that only applies while its mode is set to `custom`, say.

  Parameters:
  - **`key`** (string). Option name.
  - **`enabled`** (boolean). `false` to grey the option out.

- [lua]`trx.config.on_change(key, fn)`  
  Calls `fn(value)` whenever the option changes, and once at startup with the value loaded from the player's config - so a saved setting is applied on boot, not only when the player next touches it.

  Parameters:
  - **`key`** (string). Option name to watch.
  - **`fn`** (function). Called with the option's new value.

  Example:
  ```lua
  trx.config.on_change("visuals.water_color_mode", function(mode)
    trx.config.set_enabled("visuals.water_color", mode == "custom")
  end)
  ```
