// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 The FreeCAD project association AISBL
// SPDX-FileNotice: Part of the FreeCAD project.

/******************************************************************************
 *                                                                            *
 *   FreeCAD is free software: you can redistribute it and/or modify          *
 *   it under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1            *
 *   of the License, or (at your option) any later version.                   *
 *                                                                            *
 *   FreeCAD is distributed in the hope that it will be useful,               *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty              *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *   See the GNU Lesser General Public License for more details.              *
 *                                                                            *
 *   You should have received a copy of the GNU Lesser General Public         *
 *   License along with FreeCAD. If not, see https://www.gnu.org/licenses     *
 *                                                                            *
 ******************************************************************************/

#include "PreCompiled.h"

#include "Application.h"
#include "License.h"

using namespace App;

namespace
{
constexpr const char* documentParameterPath = "User parameter:BaseApp/Preferences/Document";
constexpr const char* licenseIdentifierEntry = "prefLicenseId";
constexpr const char* legacyLicenseIndexEntry = "prefLicenseType";

/// Used when nothing has been configured, and matches the first entry of licenseItems
constexpr int fallbackLicenseIndex = 0;

ParameterGrp::handle documentParameters()
{
    return GetApplication().GetParameterGroupByPath(documentParameterPath);
}
}  // namespace

void App::migrateLicensePreference()
{
    ParameterGrp::handle paramGrp = documentParameters();

    if (!paramGrp->GetASCII(licenseIdentifierEntry, "").empty()) {
        return;
    }

    constexpr long unset = -1;
    const auto legacyIndex = static_cast<int>(paramGrp->GetInt(legacyLicenseIndexEntry, unset));
    if (legacyIndex < 0) {
        return;
    }

    const char* identifier = legacyIndex < countOfLicenses
        ? licenseItems.at(legacyIndex).at(positionOfIdentifier)
        : otherLicenseIdentifier;
    paramGrp->SetASCII(licenseIdentifierEntry, identifier);
}

int App::getDefaultLicenseIndex()
{
    migrateLicensePreference();

    const std::string identifier = documentParameters()->GetASCII(licenseIdentifierEntry, "");
    if (identifier.empty()) {
        return fallbackLicenseIndex;
    }

    return findLicense(identifier.c_str());
}
