// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2023 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <array>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <Base/Tools.h>
#include <FCGlobal.h>

namespace App
{

/// Group a license belongs to, used to present related licenses together
enum class LicenseCategory
{
    /// Not a license at all, but the absence of any grant
    Reserved,
    /// Written specifically for hardware designs
    Hardware,
    /// Applicable to hardware, but not written for it
    GeneralPurpose,
    /// Kept so that existing documents keep working, but a newer version exists
    Superseded,
};

/**
 * One of the licenses FreeCAD offers for a document.
 *
 * The identifier is what preferences store, and must stay stable. The full name is what
 * Document::License stores and what the user sees, so it must stay stable too: an existing
 * document naming a license FreeCAD no longer spells the same way is treated as carrying an
 * unknown license. See also https://spdx.org/licenses/
 */
struct LicenseInfo
{
    const char* identifier;
    const char* fullName;
    const char* url;
    /// SPDX identifier, empty where SPDX defines none for this license
    const char* spdxIdentifier;
    LicenseCategory category;
};

constexpr int countOfLicenses {19};
// clang-format off
constexpr std::array<LicenseInfo, countOfLicenses> licenseItems {{
    { "AllRightsReserved", "All rights reserved",                                          "https://en.wikipedia.org/wiki/All_rights_reserved",  "",                LicenseCategory::Reserved       },
    { "CC_BY_40",          "Creative Commons Attribution 4.0",                             "https://creativecommons.org/licenses/by/4.0/",       "CC-BY-4.0",       LicenseCategory::GeneralPurpose },
    { "CC_BY_SA_40",       "Creative Commons Attribution-ShareAlike 4.0",                  "https://creativecommons.org/licenses/by-sa/4.0/",    "CC-BY-SA-4.0",    LicenseCategory::GeneralPurpose },
    { "CC_BY_ND_40",       "Creative Commons Attribution-NoDerivatives 4.0",               "https://creativecommons.org/licenses/by-nd/4.0/",    "CC-BY-ND-4.0",    LicenseCategory::GeneralPurpose },
    { "CC_BY_NC_40",       "Creative Commons Attribution-NonCommercial 4.0",               "https://creativecommons.org/licenses/by-nc/4.0/",    "CC-BY-NC-4.0",    LicenseCategory::GeneralPurpose },
    { "CC_BY_NC_SA_40",    "Creative Commons Attribution-NonCommercial-ShareAlike 4.0",    "https://creativecommons.org/licenses/by-nc-sa/4.0/", "CC-BY-NC-SA-4.0", LicenseCategory::GeneralPurpose },
    { "CC_BY_NC_ND_40",    "Creative Commons Attribution-NonCommercial-NoDerivatives 4.0", "https://creativecommons.org/licenses/by-nc-nd/4.0/", "CC-BY-NC-ND-4.0", LicenseCategory::GeneralPurpose },
    { "CC_BY_30",          "Creative Commons Attribution 3.0",                             "https://creativecommons.org/licenses/by/3.0/",       "CC-BY-3.0",       LicenseCategory::Superseded     },
    { "CC_BY_SA_30",       "Creative Commons Attribution-ShareAlike 3.0",                  "https://creativecommons.org/licenses/by-sa/3.0/",    "CC-BY-SA-3.0",    LicenseCategory::Superseded     },
    { "CC_BY_ND_30",       "Creative Commons Attribution-NoDerivatives 3.0",               "https://creativecommons.org/licenses/by-nd/3.0/",    "CC-BY-ND-3.0",    LicenseCategory::Superseded     },
    { "CC_BY_NC_30",       "Creative Commons Attribution-NonCommercial 3.0",               "https://creativecommons.org/licenses/by-nc/3.0/",    "CC-BY-NC-3.0",    LicenseCategory::Superseded     },
    { "CC_BY_NC_SA_30",    "Creative Commons Attribution-NonCommercial-ShareAlike 3.0",    "https://creativecommons.org/licenses/by-nc-sa/3.0/", "CC-BY-NC-SA-3.0", LicenseCategory::Superseded     },
    { "CC_BY_NC_ND_30",    "Creative Commons Attribution-NonCommercial-NoDerivatives 3.0", "https://creativecommons.org/licenses/by-nc-nd/3.0/", "CC-BY-NC-ND-3.0", LicenseCategory::Superseded     },
    { "PublicDomain",      "Public Domain",                                                "https://en.wikipedia.org/wiki/Public_domain",        "CC0-1.0",         LicenseCategory::GeneralPurpose },
    { "FreeArt",           "FreeArt",                                                      "https://artlibre.org/licence/lal",                   "LAL-1.3",         LicenseCategory::GeneralPurpose },
    { "CERN_OHS_S",        "CERN Open Hardware Licence strongly-reciprocal",               "https://cern-ohl.web.cern.ch/",                      "CERN-OHL-S-2.0",  LicenseCategory::Hardware       },
    { "CERN_OHS_W",        "CERN Open Hardware Licence weakly-reciprocal",                 "https://cern-ohl.web.cern.ch/",                      "CERN-OHL-W-2.0",  LicenseCategory::Hardware       },
    { "CERN_OHS_P",        "CERN Open Hardware Licence permissive",                        "https://cern-ohl.web.cern.ch/",                      "CERN-OHL-P-2.0",  LicenseCategory::Hardware       },
    { "GPL-3.0-or-later",  "GNU General Public License 3.0 or later",                      "https://www.gnu.org/licenses/gpl-3.0.html",          "GPL-3.0-or-later",LicenseCategory::GeneralPurpose },
}};
// clang-format on

int constexpr findLicense(const char* identifier)
{
    if (Base::Tools::isNullOrEmpty(identifier)) {
        return -1;
    }
    for (int i = 0; i < countOfLicenses; i++) {
        if (strcmp(licenseItems.at(i).identifier, identifier) == 0) {
            return i;
        }
    }
    return -1;
}

/// Identifier stored when the user chose a license that is not one of the known ones
constexpr const char* otherLicenseIdentifier = "Other";

/**
 * Rewrites the legacy license preference, which stored a position in licenseItems, as
 * the identifier of the license it referred to. Does nothing once that has happened, and
 * nothing if the legacy preference was never set. Reordering licenseItems is only safe
 * once every installation has been through this.
 */
AppExport void migrateLicensePreference();

/**
 * Position in licenseItems of the license configured as the default for new documents.
 * Returns -1 when the configured license is not one of the known ones, which callers
 * should treat as leaving the license unset.
 */
AppExport int getDefaultLicenseIndex();

/**
 * The SPDX license list, as published at https://spdx.org/licenses/ and shipped with
 * FreeCAD. Used to offer and check identifiers beyond the licenses FreeCAD presents
 * directly. The list is read once, on first use.
 */
class AppExport SpdxLicenseList
{
public:
    struct Entry
    {
        std::string identifier;
        std::string name;
        bool deprecated {false};
        bool osiApproved {false};
    };

    static const SpdxLicenseList& instance();

    /// Every entry, in the order the published list gives them
    const std::vector<Entry>& entries() const
    {
        return _entries;
    }

    /// The entry for an identifier, or nullptr when SPDX defines no such identifier
    const Entry* find(const std::string& identifier) const;

    bool contains(const std::string& identifier) const
    {
        return find(identifier) != nullptr;
    }

    /// Version of the published list the shipped copy was taken from
    const std::string& version() const
    {
        return _version;
    }

private:
    SpdxLicenseList();

    std::vector<Entry> _entries;
    std::map<std::string, std::size_t> _byIdentifier;
    std::string _version;
};
}  // namespace App
