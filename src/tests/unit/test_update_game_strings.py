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


class TestDeclaredOptionStrings(unittest.TestCase):
    """An option a game declares in Lua needs strings C never defines.

    GS_DEFINE() in C is not the only way to author a translatable string. A game
    can declare a config option the engine has never heard of; its title,
    description and enum labels still have to reach translators. The tool has to
    know which keys such a declaration requires, or it would either reject them
    as undefined or prune them as unused.
    """

    def setUp(self):
        self.strings = load("update_game_strings")

    def _keys(self, source: str) -> list[str]:
        with tempfile.NamedTemporaryFile("w", suffix=".lua", delete=False) as fh:
            fh.write(source)
            path = Path(fh.name)
        try:
            return self.strings.get_lua_config_strings(path)
        finally:
            path.unlink()

    def test_a_declaration_requires_a_title_a_description_and_each_label(self):
        keys = self._keys(
            """
            trx.config.declare({
              key = "visuals.water_color_mode",
              type = "enum",
              values = { "tombati", "dos", "custom" },
              default = "custom",
            })
            """
        )
        self.assertIn("settings/visuals.water_color_mode/title", keys)
        self.assertIn("settings/visuals.water_color_mode/description", keys)
        # "Custom" is a word, not an id, so every value needs a key too.
        self.assertIn("settings/visuals.water_color_mode/values/tombati", keys)
        self.assertIn("settings/visuals.water_color_mode/values/dos", keys)
        self.assertIn("settings/visuals.water_color_mode/values/custom", keys)

    def test_a_declaration_spanning_a_call_is_read_whole(self):
        # The body is found by matching parens, not by a line-oriented regex: a
        # nested table would end the match early and the values would be lost.
        keys = self._keys(
            'trx.config.declare({ key = "a.b", type = "enum", '
            'values = { "x" }, default = "x" })'
        )
        self.assertIn("settings/a.b/values/x", keys)

    def test_a_lua_file_that_declares_nothing_requires_nothing(self):
        self.assertEqual(self._keys('trx.config.set("visuals.fov", "70")\n'), [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
