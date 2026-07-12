-- Runs inside the real scripting environment - every trx.* module loaded,
-- exactly what a builder's level script sees - but with no level and no game
-- data. Invoked as: TRX --lua-test <this file>
--
-- These are the wiring invariants a unit test structurally cannot reach: they
-- depend on LUA_Init having done the right things in the right order.

local failures = 0

local function check(ok, what)
  if ok then
    print("  PASS  " .. what)
  else
    failures = failures + 1
    print("  FAIL  " .. what)
  end
end

-- === the sandbox =========================================================
--
-- Every one of these was reachable before, and each defeats the others: the
-- debug library alone recovers the raw C bridge from any trx.* closure, which
-- would make hiding trxc pure theatre.

check(trxc == nil, "the raw C bridge is not a global")
check(debug == nil, "debug is absent (getupvalue would recover the bridge)")
check(os == nil, "os is absent (a downloaded mod could run shell commands)")
check(io == nil, "io is absent (a downloaded mod could read and write files)")
check(require == nil, "require is absent after load")
check(package == nil, "package is absent after load")

-- The libraries a script legitimately needs must still be there.
check(type(math) == "table", "math is available")
check(type(string) == "table", "string is available")
check(type(table) == "table", "table is available")
check(type(pcall) == "function", "pcall is available")

-- === the declaration registry is sealed ==================================
--
-- api.type() reaches into the C struct binder. Left callable, a level script
-- could re-expose the very members the declarations deliberately withheld.

check(not pcall(trx.api.type, "items.Item", {
  backing = "ITEM",
  fields = { box_num = { from = "box_num", type = "integer" } },
}), "trx.api.type is sealed")
check(not pcall(trx.api.define, "items.evil", { impl = function() end }), "trx.api.define is sealed")

-- === the public surface matches what is documented =======================
--
-- Catches a forgotten `just lua-api-dump`: if the live registry has drifted from
-- the committed api.json, the reference is lying about the engine.

local surface = trx.api.describe()

local item_type
for _, t in ipairs(surface.types) do
  if t.path == "items.Item" then
    item_type = t
  end
end
check(item_type ~= nil, "items.Item is declared")

if item_type ~= nil then
  -- The bug that started all this: methods and computed members existed but
  -- appeared in no dump, so the generated docs silently omitted ten members.
  check(#item_type.fields > 0, "items.Item declares fields")
  check(#item_type.methods > 0, "items.Item declares methods")
  check(#item_type.extensions > 0, "items.Item declares computed members")

  local by_name = {}
  for _, f in ipairs(item_type.fields) do
    by_name[f.name] = f
  end
  check(by_name.hit_points ~= nil, "item.hit_points is public")
  check(by_name.box_num == nil, "item.box_num is NOT public (an engine internal)")
  check(by_name.next_item == nil, "item.next_item is NOT public (an engine internal)")

  -- Every declared member must carry documentation, or the generated reference
  -- would render a blank entry.
  for _, kind in ipairs({ "fields", "methods", "extensions" }) do
    for _, m in ipairs(item_type[kind]) do
      check(
        m.description ~= nil and m.description ~= "",
        ("items.Item %s '%s' is documented"):format(kind:sub(1, -2), m.name)
      )
    end
  end
end

-- === the modules the console commands depend on are public ===============
--
-- kill.lua and spawn.lua run with trxc gone, so every primitive they reach for
-- has to be real public API.
check(type(trx.math.sin) == "function", "trx.math.sin is public")
check(type(trx.math.WALL_L) == "number", "trx.math.WALL_L is public")
check(type(trx.objects.find_by_name) == "function", "trx.objects.find_by_name is public")
check(type(trx.objects.is_type) == "function", "trx.objects.is_type is public")
check(type(trx.rooms.find_valid_pos) == "function", "trx.rooms.find_valid_pos is public")
check(type(trx.game.is_loaded) == "function", "trx.game.is_loaded is public")
check(type(trx.items.Status.ACTIVE) == "number", "trx.items.Status is public")

print(("\n%d check(s) failed"):format(failures))
if failures > 0 then
  error(("%d sandbox invariant(s) violated"):format(failures), 0)
end
