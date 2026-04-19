# SPDX-License-Identifier: LGPL-2.1-or-later

"""
This file contains the tests for the src/Gui/ module.
"""

import FreeCAD
import FreeCADGui
import unittest


class ExplosionsNavigationStyle(FreeCADGui.PythonNavigationStyle):
    pass


class TestPythonNavigationStyleRegistry(unittest.TestCase):

    def _clear_styles(self):
        style_names = FreeCADGui.getNavigationStyles()
        for name in style_names:
            try:
                FreeCADGui.removeNavigationStyle(name)
            except NameError:
                pass  # Cannot remove C++ nav styles, that's OK

    def setUp(self):
        """A little stupid, since the setup and teardown basically also test the functionality of
        the class. But this is to prevent anything the tests do from leaking into the real run."""
        style_names = FreeCADGui.getNavigationStyles()
        self.nav_styles = {}
        for name in style_names:
            style_object = FreeCADGui.getNavigationStyle(name)
            if style_object:
                self.nav_styles[name] = style_object
        self._clear_styles()

    def tearDown(self):
        """Make sure we don't leave any leftover styles, and restore the originals."""
        self._clear_styles()
        for name, style in self.nav_styles.items():
            FreeCADGui.addNavigationStyle(name, style)

    def test_add_navigation_style_normal(self):
        """Given a name and a nav style, addNavigationStyle does so."""
        FreeCADGui.addNavigationStyle("Explosions all the time", ExplosionsNavigationStyle())

    def test_add_navigation_style_bad_type(self):
        """If you try to add a nav style with the wrong type, it raises a TypeError"""
        with self.assertRaises(TypeError):
            FreeCADGui.addNavigationStyle("No explosions at all", None)

    def test_remove_navigation_style_exists(self):
        """Given the name of an existing nav style, removeNavigationStyle does so."""
        FreeCADGui.addNavigationStyle("Explosions all the time", ExplosionsNavigationStyle())
        FreeCADGui.removeNavigationStyle("Explosions all the time")

    def test_get_navigation_styles(self):
        """getNavigationStyles returns the expected list of style names."""
        FreeCADGui.addNavigationStyle("Explosions all the time", ExplosionsNavigationStyle())
        FreeCADGui.addNavigationStyle("Explosions some of the time", ExplosionsNavigationStyle())
        FreeCADGui.addNavigationStyle("Explosions none of the time", ExplosionsNavigationStyle())
        styles = FreeCADGui.getNavigationStyles()
        self.assertGreaterEqual(len(styles), 3)
        self.assertIn("Explosions all the time", styles)
        self.assertIn("Explosions some of the time", styles)
        self.assertIn("Explosions none of the time", styles)

    def test_get_navigation_style(self):
        """getNavigationStyle returns the expected style object."""
        explosion1 = ExplosionsNavigationStyle()
        FreeCADGui.addNavigationStyle("Explosions all the time", explosion1)
        explosion2 = ExplosionsNavigationStyle()
        FreeCADGui.addNavigationStyle("Explosions some of the time", explosion2)
        explosion3 = ExplosionsNavigationStyle()
        FreeCADGui.addNavigationStyle("Explosions none of the time", explosion3)
        style = FreeCADGui.getNavigationStyle("Explosions some of the time")
        self.assertEqual(explosion2, style)

    def test_get_navigation_style_no_such_style(self):
        """getNavigationStyle returns None on a bad style name."""
        explosion1 = ExplosionsNavigationStyle()
        FreeCADGui.addNavigationStyle("Explosions all the time", explosion1)
        explosion2 = ExplosionsNavigationStyle()
        FreeCADGui.addNavigationStyle("Explosions some of the time", explosion2)
        explosion3 = ExplosionsNavigationStyle()
        FreeCADGui.addNavigationStyle("Explosions none of the time", explosion3)
        style = FreeCADGui.getNavigationStyle("I don't even like explosions")
        self.assertIsNone(style)
