# SPDX-License-Identifier: LGPL-2.1-or-later

import unittest
from unittest import mock
from unittest.mock import patch


global AddonCatalogEntry
global AddonCatalog
global Version


class TestAddonCatalogEntry(unittest.TestCase):

    def setUp(self):
        """Start mock for Addon class."""
        global AddonCatalogEntry
        global AddonCatalog
        global Version
        self.addon_patch = mock.patch.dict("sys.modules", {"addonmanager_licenses": mock.Mock()})
        self.mock_addon_module = self.addon_patch.start()
        from AddonCatalog import AddonCatalogEntry, AddonCatalog
        from addonmanager_metadata import Version

    def tearDown(self):
        """Stop mock and remove posthog from modules cache."""
        self.addon_patch.stop()

    def test_version_match_without_restrictions(self):
        with patch("addonmanager_freecad_interface.Version") as mock_freecad:
            mock_freecad.Version = lambda: (1, 2, 3, "dev")
            ac = AddonCatalogEntry({})
            self.assertTrue(ac.is_compatible())

    def test_version_match_with_min_no_max_good_match(self):
        with patch("addonmanager_freecad_interface.Version", return_value=(1, 2, 3, "dev")):
            ac = AddonCatalogEntry({"freecad_min": Version(from_string="1.2")})
            self.assertTrue(ac.is_compatible())

    def test_version_match_with_max_no_min_good_match(self):
        with patch("addonmanager_freecad_interface.Version", return_value=(1, 2, 3, "dev")):
            ac = AddonCatalogEntry({"freecad_max": Version(from_string="1.3")})
            self.assertTrue(ac.is_compatible())

    def test_version_match_with_min_and_max_good_match(self):
        with patch("addonmanager_freecad_interface.Version", return_value=(1, 2, 3, "dev")):
            ac = AddonCatalogEntry(
                {
                    "freecad_min": Version(from_string="1.1"),
                    "freecad_max": Version(from_string="1.3"),
                }
            )
            self.assertTrue(ac.is_compatible())

    def test_version_match_with_min_and_max_bad_match_high(self):
        with patch("addonmanager_freecad_interface.Version", return_value=(1, 3, 3, "dev")):
            ac = AddonCatalogEntry(
                {
                    "freecad_min": Version(from_string="1.1"),
                    "freecad_max": Version(from_string="1.3"),
                }
            )
            self.assertFalse(ac.is_compatible())

    def test_version_match_with_min_and_max_bad_match_low(self):
        with patch("addonmanager_freecad_interface.Version", return_value=(1, 0, 3, "dev")):
            ac = AddonCatalogEntry(
                {
                    "freecad_min": Version(from_string="1.1"),
                    "freecad_max": Version(from_string="1.3"),
                }
            )
            self.assertFalse(ac.is_compatible())


class TestAddonCatalog(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass
