local raw = trxc.objects

local function make_properties(object_id)
  return setmetatable({ object_id = object_id }, {
    __index = function(self, key)
      if type(key) ~= "string" then
        return nil
      end
      return raw.get_property(self.object_id, key)
    end,
    __newindex = function(self, key, value)
      raw.set_property(self.object_id, key, value)
    end,
    __pairs = function(self)
      local property_names = raw.get_property_names(self.object_id)
      local i = 0
      return function()
        i = i + 1
        local name = property_names[i]
        if name == nil then
          return nil
        end
        return name, raw.get_property(self.object_id, name)
      end
    end,
  })
end

local Object = {}

Object.__index = function(self, key)
  if key == "properties" then
    return make_properties(self.object_id)
  end
  return nil
end

local objects = {
  swap_mesh = raw.swap_mesh,
}

trx.objects = setmetatable(objects, {
  Object = Object,
  __index = function(_, key)
    if type(key) == "number" then
      return setmetatable({ object_id = key }, Object)
    elseif type(key) == "string" then
      local object_id = trx.catalog.objects[key]
      if object_id ~= nil then
        return setmetatable({ object_id = object_id }, Object)
      end
    end
    return nil
  end,
})

trx.api.define("objects.find_by_name", {
  description = "Fuzzy-matches a human-readable object name against the object catalog. "
    .. "This is what lets a console command accept `wolf` or `big medi`.",
  params = {
    { name = "name", type = "string", description = 'Name to match, e.g. `"wolf"`.' },
    {
      name = "filter",
      type = "string",
      optional = true,
      description = '`"creature"` to consider only targetable creatures, `"spawnable"` to consider '
        .. "only objects that can be spawned. Omit for no filter.",
    },
  },
  returns = { type = "table", description = "List of matching object IDs, possibly empty." },
  examples = {
    [[local ids = trx.objects.find_by_name("wolf", "creature")]],
  },
  impl = trxc.objects.ids_from_name,
})

trx.api.define("objects.is_type", {
  description = "Whether an object belongs to a given family.",
  params = {
    { name = "object_id", type = "integer", enum = "objects" },
    {
      name = "kind",
      type = "string",
      description = '`"creature"` or `"loyal"` (an ally that fights alongside Lara).',
    },
  },
  returns = { type = "boolean" },
  impl = trxc.objects.is_type,
})
