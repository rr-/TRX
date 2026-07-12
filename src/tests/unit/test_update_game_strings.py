#!/usr/bin/env python3
"""Unit tests for tools/update_game_strings. No engine, no binary, no game data."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from tools_helper import load

class TestGameStrings(unittest.TestCase):
    """A game string used only from Lua must not be pruned.

    kill.c and spawn.c moved to Lua, so their strings became Lua-only. The
    scanner read .c/.h/.def only, and pruning them would have silently stripped
    the commands' localized text at runtime.
    """

    def setUp(self):
        self.strings = load("update_game_strings")

    def test_lua_string_usages_are_found(self):
        source = (
            'trx.strings.get("general/osd/kill_all")\n'
            'trx.strings.format("general/osd/kill", n)\n'
            'trx.console.register({ help = "console/cmd/kill/help" })\n'
        )
        with tempfile.NamedTemporaryFile("w", suffix=".lua", delete=False) as fh:
            fh.write(source)
            path = Path(fh.name)

        found = {key for _, key in self.strings.get_used_lua_strings(path)}
        path.unlink()

        self.assertIn("general/osd/kill_all", found)
        self.assertIn("general/osd/kill", found)
        self.assertIn("console/cmd/kill/help", found)

    def test_unrelated_lua_strings_are_not_reported(self):
        with tempfile.NamedTemporaryFile("w", suffix=".lua", delete=False) as fh:
            fh.write('local x = "not a game string"\ntrx.log.info("hello")\n')
            path = Path(fh.name)

        found = {key for _, key in self.strings.get_used_lua_strings(path)}
        path.unlink()
        self.assertEqual(found, set())

if __name__ == "__main__":
    unittest.main(verbosity=2)
