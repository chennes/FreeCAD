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
#include <array>

#include <QApplication>
#include <QComboBox>
#include <QFont>
#include <QStandardItemModel>
#endif

#include <App/License.h>

#include "LicenseSelector.h"

using namespace Gui;

namespace
{
constexpr const char* unknownIdentifier = "__unknown__";

QString headingFor(App::LicenseCategory category)
{
    switch (category) {
        case App::LicenseCategory::Hardware:
            return QApplication::translate("Gui::LicenseSelector", "Hardware licenses");
        case App::LicenseCategory::GeneralPurpose:
            return QApplication::translate("Gui::LicenseSelector", "General purpose licenses");
        case App::LicenseCategory::Superseded:
            return QApplication::translate("Gui::LicenseSelector", "Superseded licenses");
        case App::LicenseCategory::Reserved:
            break;
    }
    return {};
}

void appendHeading(QComboBox* combo, const QString& text)
{
    combo->insertSeparator(combo->count());
    // Carries no item data, so findData() can never land on it
    combo->addItem(text);

    const int row = combo->count() - 1;
    QFont font = combo->font();
    font.setBold(true);
    combo->setItemData(row, font, Qt::FontRole);

    // A heading names the group that follows; it is not something to choose
    if (auto* model = qobject_cast<QStandardItemModel*>(combo->model())) {
        if (QStandardItem* item = model->item(row)) {
            item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
        }
    }
}

void appendLicense(QComboBox* combo, const App::LicenseInfo& license)
{
    const QString name =
        QApplication::translate("Gui::Dialog::DlgSettingsDocument", license.fullName);
    combo->addItem(name, QByteArray(license.identifier));
}

/// Categories in the order they are offered, excluding the ungrouped leading entries
constexpr std::array groupedCategories {
    App::LicenseCategory::Hardware,
    App::LicenseCategory::GeneralPurpose,
    App::LicenseCategory::Superseded,
};
}  // namespace

QByteArray Gui::unknownLicenseIdentifier()
{
    return QByteArray(unknownIdentifier);
}

void Gui::populateLicenseComboBox(
    QComboBox* combo,
    const QByteArray& selectedIdentifier,
    bool includeOther
)
{
    combo->clear();

    // Not a license at all, so it stands on its own ahead of the groups
    for (const auto& license : App::licenseItems) {
        if (license.category == App::LicenseCategory::Reserved) {
            appendLicense(combo, license);
        }
    }

    const int selected = App::findLicense(selectedIdentifier.constData());
    const bool superseded = selected >= 0
        && App::licenseItems.at(selected).category == App::LicenseCategory::Superseded;

    for (const auto category : groupedCategories) {
        // Superseded licenses stay available to documents already using one, but are not
        // offered otherwise
        if (category == App::LicenseCategory::Superseded && !superseded) {
            continue;
        }

        bool headingAdded = false;
        for (const auto& license : App::licenseItems) {
            if (license.category != category) {
                continue;
            }
            if (!headingAdded) {
                appendHeading(combo, headingFor(category));
                headingAdded = true;
            }
            appendLicense(combo, license);
        }
    }

    if (includeOther) {
        combo->insertSeparator(combo->count());
        combo->addItem(
            QApplication::translate("Gui::Dialog::DlgSettingsDocument", "Other"),
            QByteArray(App::otherLicenseIdentifier)
        );
    }

    const int row = combo->findData(selectedIdentifier);
    if (row >= 0) {
        combo->setCurrentIndex(row);
    }
}

void Gui::addUnknownLicenseToComboBox(QComboBox* combo, const QString& licenseName)
{
    combo->insertSeparator(combo->count());
    combo->addItem(licenseName, unknownLicenseIdentifier());
    combo->setCurrentIndex(combo->count() - 1);
}
