local raw = trxc.console
local LogLevel = trxc.log.LogLevel

local log = { LogLevel = LogLevel }
function log.generic(level, ...)
  raw.log(level, ...)
end
function log.info(...)
  raw.log(LogLevel.INFO, ...)
end
function log.warn(...)
  raw.log(LogLevel.WARNING, ...)
end
function log.warning(...)
  raw.log(LogLevel.WARNING, ...)
end
function log.error(...)
  raw.log(LogLevel.ERROR, ...)
end
function log.debug(...)
  raw.log(LogLevel.DEBUG, ...)
end

local console = {
  log = log,
}
setmetatable(console.log, {
  __call = function(_, ...)
    return console.log.info(...)
  end,
})

function console.clear()
  return raw.clear()
end

function console.eval(cmd, opts)
  return raw.eval(cmd, opts)
end

-- Registers a console command implemented in Lua.
--
-- spec.name    command prefix, e.g. "kill"
-- spec.help    game string key for the help text (optional)
-- spec.run     function(args) -> result[, message]
--
-- `args` is the raw argument string, trimmed. `result` is one of "ok",
-- "failure", "unavailable" or "bad_invocation"; returning nothing means "ok".
-- If `message` is given it is logged - as an error for any non-"ok" result.
function console.register(spec)
  assert(type(spec) == "table", "console.register expects a table")
  assert(type(spec.name) == "string", "console.register: name must be a string")
  assert(type(spec.run) == "function", "console.register: run must be a function")

  raw.register(spec.name, spec.help, function(args)
    local result, message = spec.run((args or ""):match("^%s*(.-)%s*$"))
    result = result or "ok"
    if message ~= nil then
      if result == "ok" then
        console.log.info(message)
      else
        console.log.error(message)
      end
    end
    return result
  end)
end

trx.console = console
