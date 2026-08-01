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

#ifndef _PreComp_
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#endif

#include <Base/Console.h>

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
        ? licenseItems.at(legacyIndex).identifier
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

SpdxLicenseList::SpdxLicenseList()
{
    QFile resource(QStringLiteral(":/licenses/licenses/spdx.json"));
    if (!resource.open(QIODevice::ReadOnly)) {
        Base::Console().warning("The SPDX license list is missing from the resources\n");
        return;
    }

    QJsonParseError parseError {};
    const QJsonDocument document = QJsonDocument::fromJson(resource.readAll(), &parseError);
    if (document.isNull()) {
        Base::Console().warning(
            "The SPDX license list could not be read: %s\n",
            parseError.errorString().toUtf8().constData()
        );
        return;
    }

    const QJsonObject root = document.object();
    _version = root.value(QStringLiteral("licenseListVersion")).toString().toStdString();

    const QJsonArray licenses = root.value(QStringLiteral("licenses")).toArray();
    _entries.reserve(licenses.size());
    for (const auto& value : licenses) {
        const QJsonObject license = value.toObject();
        Entry entry;
        entry.identifier = license.value(QStringLiteral("licenseId")).toString().toStdString();
        if (entry.identifier.empty()) {
            continue;
        }
        entry.name = license.value(QStringLiteral("name")).toString().toStdString();
        entry.deprecated = license.value(QStringLiteral("isDeprecatedLicenseId")).toBool();
        entry.osiApproved = license.value(QStringLiteral("isOsiApproved")).toBool();

        _byIdentifier.emplace(entry.identifier, _entries.size());
        _entries.push_back(std::move(entry));
    }
}

const SpdxLicenseList& SpdxLicenseList::instance()
{
    static const SpdxLicenseList list;
    return list;
}

const SpdxLicenseList::Entry* SpdxLicenseList::find(const std::string& identifier) const
{
    const auto found = _byIdentifier.find(identifier);
    return found == _byIdentifier.end() ? nullptr : &_entries.at(found->second);
}
