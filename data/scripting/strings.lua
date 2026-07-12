local raw = trxc.strings

local strings = {}

-- Returns the localized game string for a key, or the key itself if it is
-- missing, so a typo shows up on screen rather than as a nil error.
function strings.get(key)
  return raw.get(key) or key
end

-- Convenience: fetch and printf-format in one call, since most game strings
-- carry placeholders.
function strings.format(key, ...)
  return string.format(strings.get(key), ...)
end

trx.strings = strings
