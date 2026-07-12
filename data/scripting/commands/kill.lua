-- The `kill` console command. Ported from src/trx/game/console/cmd/kill.c.
--
--   /kill        kill hostiles within one sector, else the nearest within five
--   /kill all    kill every hostile in the level
--   /kill wolf   kill every hostile matching an object name

-- Load order is not guaranteed, so state the dependencies.
require("trx.console")
require("trx.objects")
require("trx.math")
require("trx.strings")
require("trx.items")
require("trx.lara")
require("trx.sound")
require("trx.catalog")

local WALL_L = 1024

-- The kill cheat, composed from primitives rather than hidden behind a C
-- entrypoint: skip anything already dead, credit a betrayed ally, then blow it
-- up. Ported from Lara_Cheat_KillEnemy.
local function cheat_kill(item)
  if item.is_killed then
    return false
  end
  if not item.is_alive and item.status ~= trx.items.Status.ACTIVE then
    return false
  end

  if trx.objects.is_type(item.object_id, "loyal") then
    trx.lara.killed_loyal_item = true
  end

  trx.sound.play(trx.catalog.samples.explosion_1, { pos = item.pos })
  item:explode()
  return true
end

local function hostiles()
  local result = {}
  for i = 1, #trx.items do
    local item = trx.items[i]
    if item ~= nil and item.is_hostile then
      table.insert(result, item)
    end
  end
  return result
end

local function kill_all()
  -- A boss awaiting its scripted combat-end sequence must survive the cheat.
  local protected = trx.game.protected_boss_id()
  local killed = 0
  for _, item in ipairs(hostiles()) do
    if item.object_id ~= protected and cheat_kill(item) then
      killed = killed + 1
    end
  end
  if killed == 0 then
    return "failure", trx.strings.get("general/osd/kill_all_fail")
  end
  return "ok", trx.strings.format("general/osd/kill_all", killed)
end

local function kill_nearest()
  local lara_pos = trx.lara.item.pos

  -- One scan: kill everything within a sector, and remember the nearest within
  -- five in case nothing was that close.
  local killed = 0
  local best, best_dist
  for _, item in ipairs(hostiles()) do
    local dist = item:distance_to(lara_pos)
    if dist <= WALL_L then
      if cheat_kill(item) then
        killed = killed + 1
      end
    elseif dist <= 5 * WALL_L and (best_dist == nil or dist < best_dist) then
      best, best_dist = item, dist
    end
  end

  if killed == 0 and best ~= nil and cheat_kill(best) then
    killed = 1
  end

  if killed == 0 then
    return "failure", trx.strings.get("general/osd/kill_fail")
  end
  return "ok", trx.strings.get("general/osd/kill")
end

local function kill_type(name)
  local ids = trx.objects.find_by_name(name, "creature")
  if #ids == 0 then
    return "failure", trx.strings.format("general/osd/invalid_object", name)
  end

  local wanted = {}
  for _, id in ipairs(ids) do
    wanted[id] = true
  end

  local matched, killed = false, 0
  for i = 1, #trx.items do
    local item = trx.items[i]
    if item ~= nil and wanted[item.object_id] then
      matched = true
      if cheat_kill(item) then
        killed = killed + 1
      end
    end
  end

  if not matched then
    return "failure", trx.strings.format("general/osd/invalid_object", name)
  end
  if killed == 0 then
    return "failure", trx.strings.format("general/osd/object_not_found", name)
  end
  return "ok", trx.strings.format("general/osd/kill_all", killed)
end

trx.console.register({
  name = "kill",
  help = "console/cmd/kill/help",
  run = function(args)
    if not trx.game.is_loaded() then
      return "unavailable"
    end
    if args == "" then
      return kill_nearest()
    end
    if args:lower() == "all" then
      return kill_all()
    end
    return kill_type(args)
  end,
})
