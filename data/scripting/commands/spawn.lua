-- The `spawn` console command. Ported from src/trx/game/console/cmd/spawn.c.
--
--   /spawn wolf   spawn an object by name, one sector in front of Lara

-- Tries straight ahead first, then 45 degrees either side, and returns the
-- first spot that lands in valid room geometry.
-- Load order is not guaranteed, so state the dependencies.
require("trx.console")
require("trx.objects")
require("trx.math")
require("trx.strings")
require("trx.items")
require("trx.lara")
require("trx.rooms")

local function find_target_pos(lara)
  for _, offset in ipairs({ -trx.math.DEG_45, 0, trx.math.DEG_45 }) do
    local angle = lara.rot.y + offset
    local dist = trx.math.WALL_L
    local candidate = {
      x = lara.pos.x + math.floor(trx.math.sin(angle) * dist),
      y = lara.pos.y,
      z = lara.pos.z + math.floor(trx.math.cos(angle) * dist),
    }
    local pos, room_num = trx.rooms.find_valid_pos(candidate, lara.room_num)
    if pos ~= nil then
      return pos, room_num
    end
  end
  return nil
end

trx.console.register({
  name = "spawn",
  run = function(args)
    if not trx.game.is_playable() then
      return "unavailable"
    end

    local lara = trx.lara.item
    if lara.hit_points <= 0 then
      return "unavailable"
    end

    if args == "" then
      return "bad_invocation"
    end

    local pos = find_target_pos(lara)
    if pos == nil then
      return "failure", trx.strings.get("console/cmd/spawn/fail")
    end

    local ids = trx.objects.find_by_name(args, "spawnable")
    if #ids == 0 then
      return "failure", trx.strings.format("general/osd/invalid_item", args)
    end

    -- Face the spawned item back towards Lara.
    local angle = trx.math.atan(lara.pos.z - pos.z, lara.pos.x - pos.x)

    for _, object_id in ipairs(ids) do
      local item = trx.items.spawn(object_id, pos, angle, { activate = true })
      if item ~= nil then
        return "ok", trx.strings.get("console/cmd/spawn/success")
      end
    end

    return "failure"
  end,
})
